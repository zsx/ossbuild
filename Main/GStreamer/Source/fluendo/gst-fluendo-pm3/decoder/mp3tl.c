/*
 * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.
 */
 /*********************************************************************
 * Adapted from dist10 reference code and used under the license therein:
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Decoder - Lower Sampling Frequency Extension
 **********************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include "mp3tl.h"
#include "mp3tl-priv.h"
#include "decode.h"

GST_DEBUG_CATEGORY_EXTERN (flump3debug);
#define GST_CAT_DEFAULT flump3debug

/* Minimum size in bytes of an MP3 frame */
#define MIN_FRAME_SIZE 24

mp3tl *
mp3tl_new (Bit_stream_struc * bs, Mp3TlMode mode)
{
  mp3tl *tl;

  g_return_val_if_fail (bs != NULL, NULL);
  g_return_val_if_fail (mode == MP3TL_MODE_16BIT, NULL);

  tl = g_new0 (mp3tl, 1);
  g_return_val_if_fail (tl != NULL, NULL);

  tl->bs = bs;
  tl->need_sync   = TRUE;
  tl->need_header = TRUE;
  tl->at_eos      = FALSE;
  tl->lost_sync   = TRUE;

  tl->frame_ts    = GST_CLOCK_TIME_NONE;

  tl->sample_size = 16;
  tl->sample_buf = NULL;
  tl->sample_w = 0;
  tl->stream_layer = 0;
  tl->error_count = 0;

  tl->fr_ps.alloc = NULL;
  init_syn_filter (&tl->fr_ps);

  tl->free_first = TRUE;
#ifdef USE_IPP
  if (!mp3_ipp_init (tl)) {
    g_free (tl);
    return NULL;
  }
#endif
  if (!mp3_c_init (tl)) {
    g_free (tl);
    return NULL;
  }

  return tl;
}

void
mp3tl_free (mp3tl * tl)
{
  g_return_if_fail (tl != NULL);

#ifdef USE_IPP
  mp3_ipp_close (tl);
#endif

  g_free (tl);
};

void mp3tl_set_eos (mp3tl *tl, gboolean more_data)
{   
  tl->at_eos = !more_data;
}

Mp3TlRetcode
mp3tl_sync (mp3tl *tl)
{
  g_return_val_if_fail (tl != NULL, FALSE);

  if (tl->need_sync) {
    guint64 sync_start;

    /* Find a sync word, with valid header */
    bs_reset (tl->bs);

    /* Need at least sync word + header bits */
    if (bs_bits_avail (tl->bs) < SYNC_WORD_LNGTH + HEADER_LNGTH)
      return MP3TL_ERR_NO_SYNC;

    sync_start = bs_pos (tl->bs);
    GST_LOG ("Starting sync search at %llu (byte %llu)", 
        sync_start, sync_start / 8);

    do {
      gboolean sync;
      guint64  offset;
      guint64  frame_start;
      fr_header *hdr = &tl->fr_ps.header;
      gboolean valid  = TRUE;
      guint64  total_offset;

      sync   = bs_seek_sync (tl->bs, &(tl->frame_ts));
      offset = bs_read_pos (tl->bs) - bs_pos (tl->bs);
      total_offset = bs_read_pos (tl->bs) - sync_start;

      if (!sync) {
        /* Leave the last byte in the stream, as it might be the first byte
         * of our sync word later */
        if (offset > 8)
          bs_consume (tl->bs, (guint32) (offset - 8));

        tl->lost_sync = TRUE;
        GST_LOG ("Not enough data in buffer for a sync sequence");
        return MP3TL_ERR_NO_SYNC;
      }
      g_assert (offset >= SYNC_WORD_LNGTH);

      /* Check if we skipped any data to find the sync word */
      if (offset != SYNC_WORD_LNGTH) {
        GST_DEBUG ("Skipped %" G_GUINT64_FORMAT " bits to find sync", offset);
        tl->lost_sync = TRUE;
      }

      /* Remember the start of frame */
      frame_start = bs_read_pos (tl->bs) - SYNC_WORD_LNGTH;

      /* Look ahead and check the header details */
      if (bs_bits_avail (tl->bs) < 20) {
        /* Consume bytes to the start of sync word and go get more data */
        bs_consume (tl->bs, (guint32) (offset - SYNC_WORD_LNGTH));
        tl->lost_sync = TRUE;

        GST_LOG ("Not enough data in buffer to read header");
        return MP3TL_ERR_NO_SYNC;
      }

      /* Read header bits */
      GST_LOG ("Reading header at %llu (byte %llu)", bs_read_pos (tl->bs), 
          bs_read_pos (tl->bs) / 8);
      if (!read_header (tl, hdr)) {
        valid = FALSE;
        GST_LOG ("Bad header");
      } else {
        /* Fill out the derived header details */
        hdr->sample_size = tl->sample_size;
        if (!set_hdr_data_slots (hdr)) {
          GST_LOG ("Bad header (slots)");
          valid = FALSE;
        }

        /* Data is not allowed to suddenly change to a different layer */
        if (tl->stream_layer != 0 && hdr->layer != tl->stream_layer) {
          GST_LOG ("Bad header (layer changed)");
          valid = FALSE;
        }
      }

      /* FIXME: Could check the CRC to confirm a sync point */

      /* If we skipped any to find the sync, and we have enough data, 
       * jump ahead to where we expect the next frame to be and confirm 
       * that there is a sync word there */
      if (valid && tl->lost_sync) {
        gint64 remain;

        remain = hdr->frame_bits - (bs_read_pos (tl->bs) - frame_start);
        if (hdr->frame_bits < (8*MIN_FRAME_SIZE)) {
          GST_LOG ("Header indicates a frame too small to be correct");
          valid = FALSE;
        }
        else if (bs_bits_avail (tl->bs) >= 
                 hdr->frame_bits) {
          guint32 sync_word;
          fr_header next_hdr;

          GST_DEBUG ("Peeking ahead %u bits to check sync (%" 
              G_GINT64_FORMAT ", %" G_GUINT64_FORMAT ", %" 
              G_GUINT64_FORMAT ")", hdr->frame_bits, remain, 
              (guint64) bs_read_pos (tl->bs), (guint64) frame_start);

          /* Skip 'remain' bits */
          bs_skipbits (tl->bs, (guint32) (remain - 1));

          /* Read a sync word and check */
          sync_word = bs_getbits_aligned (tl->bs, SYNC_WORD_LNGTH);

          if (sync_word != SYNC_WORD) {
            valid = FALSE;
            GST_LOG ("No next sync word %u bits later @ %" G_GUINT64_FORMAT 
                    ". Got 0x%03x", hdr->frame_bits, 
                    bs_read_pos (tl->bs) - SYNC_WORD_LNGTH, sync_word);
          }
          else if (!read_header (tl, &next_hdr)) {
            GST_LOG ("Invalid header at next indicated frame");
            valid = FALSE;
          }
          else {
            /* Check that the samplerate and layer for the next header is 
             * the same */
            if ((hdr->layer != next_hdr.layer) ||
               (hdr->sample_rate != next_hdr.sample_rate) ||
               (hdr->copyright != next_hdr.copyright) ||
               (hdr->original != next_hdr.original) ||
               (hdr->emphasis != next_hdr.emphasis)) {
              valid = FALSE;
              GST_LOG ("Invalid header at next indicated frame");
            }
          }

          if (valid)
            GST_LOG ("Good - found a valid frame %u bits later.", hdr->frame_bits);
        }
        else if (!tl->at_eos) {
          GST_LOG ("Not enough data in buffer to test next header");

          /* Not at the end of stream, so wait for more data to validate the 
           * frame with */
          /* Consume bytes to the start of sync word and go get more data */
          bs_consume (tl->bs, (guint32) (offset - SYNC_WORD_LNGTH));
          return MP3TL_ERR_NO_SYNC;
        }
      }

      if (!valid) {
        /* Move past the first byte of the sync word and keep looking */
        bs_consume (tl->bs, (guint32) (offset - SYNC_WORD_LNGTH + 8));
      }
      else {
        /* Consume everything up to the start of sync word */
        if (offset > SYNC_WORD_LNGTH)
          bs_consume (tl->bs, (guint32) (offset - SYNC_WORD_LNGTH));

        tl->need_sync = FALSE;
        GST_DEBUG ("OK after %d offset", (gint) total_offset - SYNC_WORD_LNGTH);
      }
    } while (tl->need_sync);
    
    if (bs_pos (tl->bs) != sync_start)
      GST_DEBUG("Skipped %llu bits, found sync", bs_pos (tl->bs) - sync_start);
  }

  return MP3TL_ERR_OK;
}

Mp3TlRetcode
mp3tl_decode_header (mp3tl *tl, const fr_header **ret_hdr)
{
  fr_header *hdr;
  Mp3TlRetcode ret;

  g_return_val_if_fail (tl != NULL, FALSE);
  hdr = &tl->fr_ps.header;
  if (G_LIKELY (ret_hdr != NULL))
    *ret_hdr = hdr;

  if (!tl->need_header)
    return MP3TL_ERR_OK;

  if ((ret = mp3tl_sync (tl)) != MP3TL_ERR_OK)
    return ret;

  /* Restart the read ptr and move past the sync word */
  bs_reset (tl->bs);
  bs_getbits (tl->bs, SYNC_WORD_LNGTH);

  /* If there are less than header bits available, something went 
   * wrong in the sync */
  g_assert (bs_bits_avail (tl->bs) >= HEADER_LNGTH);

  GST_DEBUG ("Frame is %d bytes (%d bits) with ts %" G_GUINT64_FORMAT, 
      hdr->frame_bits / 8, hdr->frame_bits, tl->frame_ts);

  /* Consume the header and sync word */
  bs_consume (tl->bs, SYNC_WORD_LNGTH + HEADER_LNGTH);

  tl->need_header = FALSE;
  return MP3TL_ERR_OK;
}

/*********************************************************************
 * Decode the current frame into the samples buffer
 *********************************************************************/
Mp3TlRetcode
mp3tl_decode_frame (mp3tl *tl, guint8 *samples, guint bufsize,
    GstClockTime *buf_time)
{
  fr_header *hdr;
  int i, j;
  int error_protection;
  guint new_crc;
  Mp3TlRetcode ret;
  gint64 frame_start_pos;

  g_return_val_if_fail (tl != NULL, MP3TL_ERR_PARAM);
  g_return_val_if_fail (samples != NULL, MP3TL_ERR_PARAM);

  hdr = &tl->fr_ps.header;

  if ((ret = mp3tl_decode_header (tl, NULL)) != MP3TL_ERR_OK)
    return ret;

  /* Check that the buffer is big enough to hold the decoded samples */
  if (bufsize < hdr->frame_samples * (hdr->sample_size / 8) * hdr->channels)
    return MP3TL_ERR_PARAM;

  bs_reset (tl->bs);

  GST_LOG ("Starting decode of frame size %u bits, with %u bits in buffer",
      hdr->frame_bits, bs_bits_avail (tl->bs));

  /* Got enough bits for the decode? (exclude the header) */
  if (bs_bits_avail (tl->bs) < 
      hdr->frame_bits - (SYNC_WORD_LNGTH + HEADER_LNGTH))
    return MP3TL_ERR_NEED_DATA;

  hdr_to_frps (&tl->fr_ps);

  if (hdr->version == MPEG_VERSION_1) 
    tl->n_granules = 2;
  else
    tl->n_granules = 1;

  tl->stream_layer = hdr->layer;

  error_protection = hdr->error_protection;
  
  /* We're about to start reading bits out of the stream, 
   * after which we'll need a new sync and header
   */
  tl->need_sync = TRUE;
  tl->need_header = TRUE;

  /* Set up the output buffer */
  tl->sample_w = 0;
  tl->sample_buf = (gint16 *) samples;

  /* Remember the start of the frame */
  frame_start_pos = bs_read_pos (tl->bs) - (SYNC_WORD_LNGTH + HEADER_LNGTH);

  /* Retrieve the CRC from the stream */
  if (error_protection) 
    buffer_CRC (tl->bs, &tl->old_crc);

  switch (hdr->layer) {
    case 1: {
      guint bit_alloc[2][SBLIMIT], scale_index[2][3][SBLIMIT];
      guint ch;

      I_decode_bitalloc (tl->bs, bit_alloc, &tl->fr_ps);
      I_decode_scale (tl->bs, bit_alloc, scale_index, &tl->fr_ps);

      /* Compute and check the CRC */
      if (error_protection) {
        I_CRC_calc (&tl->fr_ps, bit_alloc, &new_crc);
        if (new_crc != tl->old_crc) {
          tl->error_count++;
          GST_DEBUG ("CRC mismatch - Bad frame");
          return MP3TL_ERR_BAD_FRAME;
        } 
      }

      for (i = 0; i < SCALE_BLOCK; i++) {
        I_buffer_sample (tl->bs, tl->sample, bit_alloc, &tl->fr_ps);
        I_dequant_and_scale_sample (tl->sample, tl->fraction, bit_alloc,
            scale_index, &tl->fr_ps);

        for (ch = 0; ch < hdr->channels; ch++) {
          mp3_SubBandSynthesis (tl, &tl->fr_ps, &(tl->fraction[ch][0][0]), ch,
              &((tl->pcm_sample)[ch][0][0]));
        }
        out_fifo (tl->pcm_sample, 1, &tl->fr_ps, tl->sample_buf, &tl->sample_w,
            SAMPLE_BUF_SIZE);
      }
      break;
    }

    case 2: {
      guint bit_alloc[2][SBLIMIT], scfsi[2][SBLIMIT], scale_index[2][3][SBLIMIT];
      guint ch;

      /* Choose bit allocations table */
      II_pick_table (&tl->fr_ps);

      /* Read band bit allocations from the data and scale */
      II_decode_bitalloc (tl->bs, bit_alloc, &tl->fr_ps);
      II_decode_scale (tl->bs, scfsi, bit_alloc, scale_index, &tl->fr_ps);

      if (error_protection) {
        II_CRC_calc (&tl->fr_ps, bit_alloc, scfsi, &new_crc);
        if (new_crc != tl->old_crc) {
          tl->error_count++;
          GST_DEBUG ("CRC mismatch - Bad frame");
          return MP3TL_ERR_BAD_FRAME;
        } 
      }

      for (i = 0; i < SCALE_BLOCK; i++) {
        II_buffer_sample (tl->bs, (tl->sample), bit_alloc, &tl->fr_ps);
        II_dequant_and_scale_sample (tl->sample, bit_alloc, tl->fraction, 
           scale_index, i >> 2, &tl->fr_ps);

        for (j = 0; j < 3; j++)
          for (ch = 0; ch < hdr->channels; ch++) {
            mp3_SubBandSynthesis (tl, &tl->fr_ps, &((tl->fraction)[ch][j][0]), ch,
                &((tl->pcm_sample)[ch][j][0]));
          }
        out_fifo (tl->pcm_sample, 3, &tl->fr_ps, tl->sample_buf, &tl->sample_w,
            SAMPLE_BUF_SIZE);
      }
      break;
    }
    case 3:
      /* Decoder specific implementation */
#ifdef USE_IPP
      if (hdr->version == MPEG_VERSION_2_5)
        ret = c_decode_mp3 (tl);
      else
        ret = ipp_decode_mp3 (tl);
#else
        ret = c_decode_mp3 (tl);
#endif

      if (ret != MP3TL_ERR_OK)
        return ret;
      break;
    default:
      g_warning ("Unknown layer %d, invalid bitstream.", hdr->layer);
      return MP3TL_ERR_STREAM;
  }
  
  /* skip ancillary data   HP 22-nov-95 */
  if (hdr->bitrate_idx != 0) { /* if not free-format */
    /* Ancillary bits are any left in the frame that didn't get used */
    gint64 anc_len = hdr->frame_slots * hdr->bits_per_slot;

    anc_len -= bs_read_pos (tl->bs) - frame_start_pos;

    if (anc_len > 0) {
      GST_DEBUG ("Skipping %ld ancillary bits", anc_len);
      do {
        bs_getbits (tl->bs, (guint32) MIN (anc_len, MAX_LENGTH));
        anc_len -= MAX_LENGTH;
      } while (anc_len > 0);
    }
  }

  tl->frame_num++;
  tl->bits_used += hdr->frame_bits;

  /* Consume the data */
  bs_consume (tl->bs, hdr->frame_bits - (SYNC_WORD_LNGTH + HEADER_LNGTH));

  GST_DEBUG (
      "Used %u bits = %u slots plus %u", hdr->frame_bits, hdr->frame_slots,
      hdr->frame_bits % hdr->bits_per_slot);

  GST_DEBUG ("Avg slots/frame so far = %.3f; b/smp = %.2f; br = %.3f kbps",
      (FLOAT) tl->bits_used / (tl->frame_num * hdr->bits_per_slot),
      (FLOAT) tl->bits_used / (tl->frame_num * hdr->frame_samples),
      (FLOAT) (1000 * tl->bits_used) / (tl->frame_num * hdr->frame_samples) *
      s_rates[hdr->version][hdr->srate_idx]);

  /* Correctly decoded a frame, so assume we're synchronised */
  tl->lost_sync = FALSE;
  if (buf_time != NULL)
    *buf_time = tl->frame_ts;

  return MP3TL_ERR_OK;
}

void
mp3tl_flush (mp3tl *tl)
{
  GST_LOG ("Flush");
  /* Clear out the bytestreams */
  bs_flush (tl->bs);

  tl->need_header = TRUE;
  tl->need_sync   = TRUE;
  tl->lost_sync   = TRUE;

  tl->frame_ts    = GST_CLOCK_TIME_NONE;

  /* Call decoder specific flush routines */
#ifdef USE_IPP
  mp3_ipp_flush (tl);
#endif
  mp3_c_flush (tl);
}

Mp3TlRetcode
mp3tl_skip_frame (mp3tl *tl, GstClockTime *buf_time)
{
  fr_header *hdr;
  Mp3TlRetcode ret;

  g_return_val_if_fail (tl != NULL, MP3TL_ERR_PARAM);

  hdr = &tl->fr_ps.header;

  if ((ret = mp3tl_decode_header (tl, NULL)) != MP3TL_ERR_OK)
    return ret;

  bs_reset (tl->bs);

  /* Got enough bits to consume? (exclude the header) */
  if (bs_bits_avail (tl->bs) < hdr->frame_bits - (SYNC_WORD_LNGTH + HEADER_LNGTH))
    return MP3TL_ERR_NEED_DATA;

  hdr_to_frps (&tl->fr_ps);

  if (hdr->version == MPEG_VERSION_1) 
    tl->n_granules = 2;
  else
    tl->n_granules = 1;

  tl->stream_layer = hdr->layer;

  /* We're about to start reading bits out of the stream, 
   * after which we'll need a new sync and header
   */
  tl->need_sync = TRUE;
  tl->need_header = TRUE;

  tl->frame_num++;
  tl->bits_used += hdr->frame_bits;

  /* Consume the data */
  bs_consume (tl->bs, hdr->frame_bits - (SYNC_WORD_LNGTH + HEADER_LNGTH));

  GST_DEBUG (
      "Skipped %u bits = %u slots plus %u", hdr->frame_bits, hdr->frame_slots,
      hdr->frame_bits % hdr->bits_per_slot);

  GST_DEBUG ("Avg slots/frame so far = %.3f; b/smp = %.2f; br = %.3f kbps",
      (FLOAT) tl->bits_used / (tl->frame_num * hdr->bits_per_slot),
      (FLOAT) tl->bits_used / (tl->frame_num * hdr->frame_samples),
      (FLOAT) (1000 * tl->bits_used) / (tl->frame_num * hdr->frame_samples) *
      s_rates[hdr->version][hdr->srate_idx]);

  if (buf_time != NULL)
    *buf_time = tl->frame_ts;

  return MP3TL_ERR_OK;
}

const char *mp3tl_get_err_reason (mp3tl *tl)
{
  return tl->reason;
}
