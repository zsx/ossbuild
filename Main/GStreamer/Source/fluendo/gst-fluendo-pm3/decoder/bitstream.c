/*
 * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.
 */
 /*********************************************************************
 * Adapted from dist10 reference code and used under the license therein:
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Decoder - Lower Sampling Frequency Extension
 **********************************************************************/

/* 
 * 2 Bitstream buffer implementations. 1 reading from a linked list 
 * of data buffers, the other from a fixed size ring buffer.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <gst/gst.h>

#include "common.h"
#include "bitstream.h"

GST_DEBUG_CATEGORY_EXTERN (flump3debug);
#define GST_CAT_DEFAULT flump3debug

/* Create and initialise a new bitstream reader with a buffer of size "size" */
Bit_stream_struc *
bs_new ()
{
  Bit_stream_struc *bs;

  bs = g_new0 (Bit_stream_struc, 1);
  g_return_val_if_fail (bs != NULL, NULL);

  bs->master.cur_bit = 8;
  bs->master.buf_size = 0;
  bs->master.cur_used = 0;
  bs->read.cur_bit = 8;
  bs->read.buf_size = 0;
  bs->read.cur_used = 0;
  return bs;
}

/* Release a bitstream reader */
void
bs_free (Bit_stream_struc * bs)
{
  GList *cur;

  g_return_if_fail (bs != NULL);

  /* Release all the queued buffers */
  cur = bs->master.buflist;
  while (cur) {
    BSBuffer *buf = (BSBuffer *) (cur->data);

    if (bs->free_cb)
      bs->free_cb (buf->ref);

    g_free (buf);
    cur = g_list_next (cur);
  }
  g_list_free (bs->master.buflist);

  g_free (bs);
}

void
bs_flush (Bit_stream_struc * bs)
{
  GList *cur;

  g_return_if_fail (bs != NULL);

  /* Release all the queued buffers */
  cur = bs->master.buflist;
  while (cur) {
    BSBuffer *buf = (BSBuffer *) (cur->data);

    if (bs->free_cb)
      bs->free_cb (buf->ref);

    g_free (buf);
    cur = g_list_next (cur);
  }
  g_list_free (bs->master.buflist);
  bs->master.buflist = NULL;

  bs->master.cur_bit  = 8;
  bs->master.buf_size = 0;
  bs->master.cur_used = 0;
  bs->master.cur_byte = NULL;
  bs->master.cur      = NULL;
  bs->master.bitpos = 0;

  bs_reset (bs);
}

/* Callback to be called to release a buffer when finished with it */
void
bs_set_release_func (Bit_stream_struc * bs, bs_release_func cb)
{
  g_return_if_fail (bs != NULL);
  bs->free_cb = cb;
}

/* Append a data buffer to the stream for processing */
gboolean
bs_add_buffer (Bit_stream_struc * bs, guchar * data, guint32 size, void *ref,
    GstClockTime ts)
{
  BSBuffer *buf;

  g_return_val_if_fail (bs != NULL, FALSE);
  g_return_val_if_fail (ref != NULL, FALSE);
  g_return_val_if_fail (size != 0, FALSE);

  buf = g_new (BSBuffer, 1);
  g_return_val_if_fail (buf != NULL, FALSE);
  buf->data = data;
  buf->size = size;
  buf->ref  = ref;
  buf->ts   = ts;

  /* Append a new buffer to the list */
  bs->master.buflist = g_list_append (bs->master.buflist, buf);
  bs->master.buf_size += size;

  bs_reset (bs);
  return TRUE;
}

void bs_reset (Bit_stream_struc *bs)
{
  memcpy (&bs->read, &bs->master, sizeof (BSReader));
}

/* Advance N bits on the indicated BSreader, 
 * freeing buffers if release = TRUE */
static void bs_eat (Bit_stream_struc *bs, BSReader *read, 
    guint32 Nbits, gboolean release)
{
  while (Nbits > 0) {
    gint k;

    /* If the current buffer is empty, move on to the next one */
    if ((read->cur == NULL) || (read->cur_used >=
        read->cur->size))  {
      bs_nextbuf (bs, read, release);
      if (!read->cur)
        return;
    }

    if (Nbits < 8 || read->cur_bit != 8)
    {
      /* Take as many bits as we can from the current byte */
      k = MIN (Nbits, read->cur_bit);

      /* Adjust our tracking vars */
      read->cur_bit -= k;
      Nbits -= k;
      read->bitpos += k;
  
      /* Move to the next byte if we consumed the current one */
      if (read->cur_bit == 0) {
        read->cur_bit = 8;
        read->cur_used++;
        read->cur_byte++;
      }
    } else {
      /* Take as many bytes as we can from current buffer */
      k = MIN (Nbits / 8, read->cur->size - read->cur_used);

      read->cur_used += k;
      read->cur_byte += k;
      
      /* convert to bits */
      k *= 8;
      read->bitpos += k;
      Nbits -= k;
    }
  }
}

/* Advance the master position by Nbits, freeing buffers as 
 * we go */
void bs_consume (Bit_stream_struc *bs, guint32 Nbits)
{
#if 0
  static gint n = 0;
  g_print ("%d Consumed %d bits to end at %lld\n", n++, Nbits, 
      bs_pos (bs) + Nbits);
#endif
  bs_eat (bs, &bs->master, Nbits, TRUE);

  /* Reset the read pointer, since we may have consumed its buffer */
  bs_reset (bs);
}

/* Advance the read position by Nbits */
void bs_skipbits (Bit_stream_struc *bs, guint32 Nbits)
{
  bs_eat (bs, &bs->read, Nbits, FALSE);
}

/* Move the reader to the next data buffer, freeing as indicated */
gboolean
bs_nextbuf (Bit_stream_struc *bs, BSReader *read, gboolean release)
{
  gboolean was_last_buf;
  if (G_LIKELY (read->cur)) {
    read->buf_size -= read->cur->size;
    if (release) {
      if (G_LIKELY (bs->free_cb))
        bs->free_cb (read->cur->ref);
      else
        g_warning ("Leaking data buffer - no release function set.");
      g_free (read->cur);
      read->buflist = g_list_remove (read->buflist, read->cur);
    }
    else {
      read->buflist = g_list_next (read->buflist);
    }
  }

  if (read->buflist) {
    read->cur = (BSBuffer *) (read->buflist->data);
    read->cur_byte = read->cur->data;
    was_last_buf = (g_list_next (read->buflist) == NULL);
  } else {
    read->cur = NULL;
    read->cur_byte = NULL;
    was_last_buf = TRUE;
  }
  read->cur_bit = 8;
  read->cur_used = 0;
  return was_last_buf;
}

gboolean
bs_seek_sync (Bit_stream_struc * bs, GstClockTime *frame_ts)
{
  gboolean last_buf;
  GstClockTime sync_ts;
  guint8 last_byte;
  guint8 *start_pos;

  /* Align to the start of the next byte */
  if (bs->read.cur && bs->read.cur_bit != BS_BYTE_SIZE) {
    bs->read.bitpos += (BS_BYTE_SIZE - bs->read.cur_bit);
    bs->read.cur_bit = BS_BYTE_SIZE;
    bs->read.cur_used++;
    bs->read.cur_byte++;
  }

  /* Ensure we have a buffer to start with */
  if (G_LIKELY (bs->read.cur != NULL && 
        bs->read.cur_used < bs->read.cur->size)) {
    last_buf = (g_list_next (bs->read.buflist) == NULL);
  } else {
    last_buf = bs_nextbuf (bs, &bs->read, FALSE);
    if (bs->read.cur == NULL) 
      return FALSE;
  }

  sync_ts = bs_buf_time (bs);
  start_pos = bs->read.cur_byte;
  while (bs->read.cur != NULL) {
    while (bs->read.cur_used < bs->read.cur->size-1) {
      last_byte = bs->read.cur_byte[0];
      bs->read.cur_used++;
      bs->read.cur_byte++;

      if (last_byte == 0xff && bs->read.cur_byte[0] >= 0xe0) {
        /* Found a sync word */
        goto found_sync;
      }
    }
    /* Ran out of current buffer */
    bs->read.bitpos += BS_BYTE_SIZE * (bs->read.cur_byte - start_pos);
    if (last_buf) {
      /* Leave the last byte in the buffer for next time */
      return FALSE;
    }
    last_byte = bs->read.cur_byte[0];
    bs->read.bitpos += BS_BYTE_SIZE; /* Eat the last byte in the buffer */

    last_buf = bs_nextbuf (bs, &bs->read, FALSE);
    start_pos = bs->read.cur_byte;
    g_assert (bs->read.cur != NULL); /* We checked for last_buf already */
    g_assert (bs->read.cur->data == start_pos); 
    if (last_byte == 0xff && bs->read.cur_byte[0] >= 0xe0) {
      /* Found a sync word */
      goto found_sync;
    }
    
    sync_ts = bs_buf_time (bs);
  }
  return FALSE;
found_sync:
  /* Move past the first 3 bits of 2nd sync byte */
  bs->read.cur_bit = 5;
  bs->read.bitpos += 3 + BS_BYTE_SIZE * (bs->read.cur_byte - start_pos);
  if (G_LIKELY (frame_ts))
    *frame_ts = sync_ts;
  return TRUE;
}

/* Return the clock time associated with the current data buffer */
GstClockTime bs_buf_time (Bit_stream_struc *bs)
{
  if (bs->read.cur) 
    return bs->read.cur->ts;

  return GST_CLOCK_TIME_NONE;
}

void 
bs_getbytes (Bit_stream_struc *bs, guint8 *out, guint32 N)
{
  gint j = N;
  gint to_take;

  while (j > 0) {
    /* Move to the next byte if we consumed any bits of the current one */
    if (bs->read.cur && bs->read.cur_bit != 8) {
      bs->read.cur_bit = 8;
      bs->read.cur_used++;
      bs->read.cur_byte++;
    }

    /* If the current buffer is empty, move on to the next one */
    if ((bs->read.cur == NULL) || (bs->read.cur_used >= bs->read.cur->size))  {
      bs_nextbuf (bs, &bs->read, FALSE);
      if (!bs->read.cur) {
        g_warning ("Attempted to read beyond buffer\n");
        return;
      } 
    }

    /* Take as many bytes as we can from the current buffer */
    to_take = MIN (j, (gint)(bs->read.cur->size - bs->read.cur_used));
    memcpy (out, bs->read.cur_byte, to_take);

    out += to_take;
    bs->read.cur_byte += to_take;
    bs->read.cur_used += to_take;
    j -= to_take;
    bs->read.bitpos += (to_take * 8);
  }
}

void
h_setbuf (huffdec_bitbuf *bb, guint8* buf, guint size)
{
  bb->avail = size;
  bb->totbit = 0;
  bb->buf_byte_idx = 0;
  bb->buf_bit_idx = 8;
  bb->buf = buf;
#if ENABLE_OPT_BS
  /* First load of the accumulator, assumes that size >= 4 */
  bb->buf_bit_idx = 32;
  bb->remaining = bb->avail - 4;

  /* we need reverse the byte order */
  bb->accumulator = (guint)bb->buf[bb->buf_byte_idx + 3];
  bb->accumulator |= (guint)(bb->buf[bb->buf_byte_idx + 2]) << 8;
  bb->accumulator |= (guint)(bb->buf[bb->buf_byte_idx + 1]) << 16;  
  bb->accumulator |= (guint)(bb->buf[bb->buf_byte_idx + 0]) << 24;  

  bb->buf_byte_idx+=4;
//  g_printf("h_setbuf: bb->accumulator=%8x, bb->remaining=%d\n", bb->accumulator, bb->remaining);
#endif  
}
#if ENABLE_OPT_BS
void
h_rewindNbits (huffdec_bitbuf *bb, guint N)
{
//  g_printf("h_rewindNbits N=%d bb->buf_bit_idx=%d, bb->accumulator=%8x, bb->remaining=%d\n", N, bb->buf_bit_idx, bb->accumulator, bb->remaining);
  guint bits = 0;
  guint bytes = 0;
  bb->totbit -= N;
  if (N <= (32-bb->buf_bit_idx))
    bb->buf_bit_idx += N;
  else {
    N -= (32-bb->buf_bit_idx);
    bb->buf_bit_idx = 0;
    bits = N % 8;
    bytes = N / 8;
    bb->buf -= bytes;
    bb->remaining += bytes;  
    h_getbits (bb, bits);
  }
//  g_printf("h_rewindNbits out:N=%d, bits=%d, bytes=%d, bb->buf_bit_idx=%d, bb->accumulator=%8x, bb->remaining=%d\n", N, bits, bytes, bb->buf_bit_idx, bb->accumulator, bb->remaining);
}
#else
void
h_rewindNbits (huffdec_bitbuf *bb, guint N)
{
  gulong byte_off;
  g_return_if_fail (bb->totbit >= N);

  byte_off = (bb->buf_bit_idx + N) / 8;

  g_return_if_fail (bb->buf_byte_idx >= byte_off);

  bb->totbit -= N;
  bb->buf_bit_idx += N;

  if (bb->buf_bit_idx >= 8) {
    bb->buf_bit_idx -= 8 * byte_off;
    bb->buf_byte_idx -= byte_off;
  }
}
#endif
void
h_rewindNbytes (huffdec_bitbuf *bb, guint N)
{
//  g_printf("h_rewindNbytes\n");
  g_return_if_fail (bb->totbit >= N * 8);
  g_return_if_fail (bb->buf_byte_idx >= N);

  bb->totbit -= N * 8;
  bb->buf_byte_idx -= N;

}

