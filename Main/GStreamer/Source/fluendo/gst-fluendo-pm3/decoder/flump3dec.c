/*
 * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.
 */
 
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "flump3dec.h"

GST_DEBUG_CATEGORY (flump3debug);
#define GST_CAT_DEFAULT flump3debug

/* Forward declarations */
static void flump3dec_class_init (FluMp3Dec *klass);
static void flump3dec_base_init  (FluMp3DecClass *klass);
static void flump3dec_init       (FluMp3Dec *flump3dec);
static void flump3dec_dispose    (GObject *object);
static void flump3dec_reset (FluMp3Dec *flump3dec);

static gboolean flump3dec_sink_event (GstPad *pad, GstEvent *event);
static GstFlowReturn flump3dec_sink_chain (GstPad * pad, GstBuffer * buffer);
static GstFlowReturn flump3dec_drain_avail (GstPad *pad, gboolean more_data);
static GstStateChangeReturn flump3dec_change_state (GstElement *element, GstStateChange transition);
static const GstQueryType *flump3dec_get_query_types (GstPad *pad);
static gboolean flump3dec_src_query (GstPad *pad, GstQuery *query);
static gboolean flump3dec_src_convert (GstPad *pad, GstFormat src_format,
    gint64 src_value, GstFormat *dest_format, gint64 *dest_value);
static gboolean flump3dec_src_event (GstPad *pad, GstEvent *event);
static gboolean flump3_inbytes_to_time (FluMp3Dec *flump3dec, gint64 byteval, 
    gint64 *timeval);

/* static vars */
static GstElementClass *parent_class = NULL;

/* TODO: Add support for MPEG2 multichannel extension */
static GstStaticPadTemplate flump3dec_sink_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/mpeg, mpegversion=(int) 1, "
      "layer = (int) [ 1, 3 ], "
      "rate = (int) { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000 }, "
      "channels = (int) [ 1, 2 ]")
  );

/* TODO: higher resolution 24 bit decoding */
static GstStaticPadTemplate flump3dec_src_factory =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw-int,"
        "endianness = (int) " G_STRINGIFY (G_BYTE_ORDER) ", "
        "signed = (boolean) true, "
        "width = (int) 16, depth = (int) 16, "
        "rate = (int) { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000 }, "
        "channels = (int) [ 1, 2 ]")
  );

/* Function implementations */
GType
flump3dec_get_type (void)
{
  static GType flump3dec_type = 0;

  if (!flump3dec_type)
  {
    static const GTypeInfo flump3dec_info =
    {
      sizeof (FluMp3DecClass),
      (GBaseInitFunc) flump3dec_base_init,
      NULL,
      (GClassInitFunc) flump3dec_class_init,
      NULL,
      NULL,
      sizeof (FluMp3Dec),
      0,
      (GInstanceInitFunc) flump3dec_init,
      NULL
    };
    flump3dec_type = g_type_register_static (GST_TYPE_ELEMENT,
        "FluMp3Dec", &flump3dec_info, 0);
  }

  return flump3dec_type;
}

static void
flump3dec_class_init (FluMp3Dec *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass*) klass;
  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  gobject_class->dispose = flump3dec_dispose;

  gstelement_class->change_state = flump3dec_change_state;
}

static void
flump3dec_base_init (FluMp3DecClass *klass)
{
#ifdef USE_IPP
  #define LONGNAME "Fluendo MP3 Decoder (IPP build)"
#else
#ifdef USE_LIBOIL
  #define LONGNAME "Fluendo MP3 Decoder (liboil build)"
#else
  #define LONGNAME "Fluendo MP3 Decoder (C build)"
#endif
#endif
  /* FIXME: Which email address to put? */
  static GstElementDetails details = 
    GST_ELEMENT_DETAILS (
	  LONGNAME,
      "Codec/Decoder/Audio",
      "Decodes MPEG-1 Layer 1, 2 and 3 streams to raw audio frames",
      "Fluendo Support <support@fluendo.com>"
    );
#undef LONGNAME
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
        gst_static_pad_template_get (&flump3dec_src_factory));
  gst_element_class_add_pad_template (element_class,
        gst_static_pad_template_get (&flump3dec_sink_factory));

  gst_element_class_set_details (element_class, &details);
}

static void
flump3dec_init (FluMp3Dec *flump3dec)
{
  /* Create and add pads */
  flump3dec->sinkpad =
      gst_pad_new_from_static_template (&flump3dec_sink_factory, "sink");
  gst_pad_set_event_function (flump3dec->sinkpad, flump3dec_sink_event);
  gst_pad_set_chain_function (flump3dec->sinkpad, flump3dec_sink_chain);
  gst_element_add_pad (GST_ELEMENT (flump3dec), flump3dec->sinkpad);

  flump3dec->srcpad =
      gst_pad_new_from_static_template (&flump3dec_src_factory, "src");
  gst_pad_set_query_type_function (flump3dec->srcpad, flump3dec_get_query_types);
  gst_pad_set_query_function (flump3dec->srcpad, flump3dec_src_query);
  gst_pad_set_event_function (flump3dec->srcpad, flump3dec_src_event);
  gst_pad_use_fixed_caps (flump3dec->srcpad);
  gst_element_add_pad (GST_ELEMENT (flump3dec), flump3dec->srcpad);

  flump3dec->bs = bs_new ();
  g_return_if_fail (flump3dec->bs != NULL);
  bs_set_release_func (flump3dec->bs, (bs_release_func) (gst_mini_object_unref));

  flump3dec->dec = mp3tl_new (flump3dec->bs, MP3TL_MODE_16BIT);
  g_return_if_fail (flump3dec->dec != NULL);

  flump3dec_reset (flump3dec);
}

static void
flump3dec_reset (FluMp3Dec *flump3dec)
{
  flump3dec->rate        = 0;
  flump3dec->channels    = 0;
  flump3dec->next_ts     = 0;
  flump3dec->avg_bitrate = 0;
  flump3dec->bitrate_sum = 0;
  flump3dec->frame_count = 0;
  flump3dec->last_posted_bitrate = 0;

  flump3dec->bad = FALSE;
  flump3dec->pending_frame = NULL;

  flump3dec->xing_flags  = 0;
  flump3dec->last_dec_ts = GST_CLOCK_TIME_NONE;
}

static void
flump3dec_dispose (GObject *object)
{
  FluMp3Dec *flump3dec = FLUMP3DEC (object);
  
  if (flump3dec->dec)
    mp3tl_free (flump3dec->dec);
  flump3dec->dec = NULL;

  if (flump3dec->bs)
    bs_free (flump3dec->bs);
  flump3dec->bs = NULL;
    
  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void 
flump3dec_flush (FluMp3Dec *flump3dec)
{ 
  flump3dec->last_dec_ts = GST_CLOCK_TIME_NONE;

  mp3tl_flush (flump3dec->dec);
  if (flump3dec->pending_frame) {
    gst_buffer_unref (flump3dec->pending_frame);
    flump3dec->pending_frame = NULL;
  }
  
  flump3dec->need_discont = TRUE;
}

/* Decode a buffer */
static GstFlowReturn
flump3dec_sink_chain (GstPad *pad, GstBuffer *buf)
{
  GstFlowReturn ret = GST_FLOW_OK;
  FluMp3Dec * dec = NULL;
  GstClockTime new_ts;
  gboolean discont;
  
  dec = FLUMP3DEC (gst_pad_get_parent (pad));
  
  discont = GST_BUFFER_FLAG_IS_SET (buf, GST_BUFFER_FLAG_DISCONT);
  
  /* We flush on disconts */
  if (G_UNLIKELY (discont)) {
    GST_DEBUG_OBJECT (dec, "this buffer has a DISCONT flag, flushing");
    flump3dec_flush (dec);
  }
  
  /* We've got a new buffer. Decode it! */
  new_ts = GST_BUFFER_TIMESTAMP (buf);
  GST_DEBUG ("New input buffer with TS %" G_GUINT64_FORMAT, new_ts);

  /* Give the buffer over to the decoder */
  bs_add_buffer (dec->bs, GST_BUFFER_DATA (buf), GST_BUFFER_SIZE (buf),
      buf, new_ts);

  ret = flump3dec_drain_avail (pad, TRUE);
  
  gst_object_unref (dec);
  
  return ret;
}

#define XING_FRAMES_FLAG     0x0001
#define XING_BYTES_FLAG      0x0002
#define XING_TOC_FLAG        0x0004
#define XING_VBR_SCALE_FLAG  0x0008

static Mp3TlRetcode
flump3dec_check_for_xing (FluMp3Dec *flump3dec, const fr_header *mp3hdr) 
{
  const guint32 xing_id = 0x58696e67; /* 'Xing' in hex */
  const guint32 info_id = 0x496e666f; /* 'Info' in hex - found in LAME CBR files */
  const guint XING_HDR_MIN = 8;
  gint xing_offset;

  guint32 read_id;

  if (mp3hdr->version == MPEG_VERSION_1) { /* MPEG-1 file */
    if (mp3hdr->channels == 1)
      xing_offset = 0x11;
    else
      xing_offset = 0x20;
  } else { /* MPEG-2 header */
    if (mp3hdr->channels == 1)
      xing_offset = 0x09;
    else
      xing_offset = 0x11;
  }

  bs_reset (flump3dec->bs);

  if (bs_bits_avail (flump3dec->bs) < 8 * (xing_offset + XING_HDR_MIN)) {
    GST_DEBUG ("Not enough data to read Xing header");
    return MP3TL_ERR_NEED_DATA;
  }

  /* Read 4 bytes from the frame at the specified location */
  bs_skipbits (flump3dec->bs, 8 * xing_offset);
  read_id = bs_getbits (flump3dec->bs, 32);
  if (read_id == xing_id || read_id == info_id) {
    guint32 xing_flags;
    guint bytes_needed = 0;

    /* Read 4 base bytes of flags, big-endian */
    xing_flags = bs_getbits (flump3dec->bs, 32);
    if (xing_flags & XING_FRAMES_FLAG) 
      bytes_needed += 4;
    if (xing_flags & XING_BYTES_FLAG) 
      bytes_needed += 4;
    if (xing_flags & XING_TOC_FLAG) 
      bytes_needed += 100;
    if (xing_flags & XING_VBR_SCALE_FLAG) 
      bytes_needed += 4;
    if (bs_bits_avail (flump3dec->bs) < 8 * bytes_needed) {
      GST_DEBUG ("Not enough data to read Xing header (need %d)", bytes_needed);
      return MP3TL_ERR_NEED_DATA;
    }
            
    GST_DEBUG ("Reading Xing header");
    flump3dec->xing_flags = xing_flags;

    if (xing_flags & XING_FRAMES_FLAG) {
      flump3dec->xing_frames = bs_getbits (flump3dec->bs, 32);
      flump3dec->xing_total_time = gst_util_uint64_scale (GST_SECOND, 
          (guint64)(flump3dec->xing_frames) * (mp3hdr->frame_samples),
          mp3hdr->sample_rate);
    } else {
      flump3dec->xing_frames = 0;
      flump3dec->xing_total_time = 0;
    }

    if (xing_flags & XING_BYTES_FLAG)
      flump3dec->xing_bytes = bs_getbits (flump3dec->bs, 32);
    else 
      flump3dec->xing_bytes = 0;

    if (xing_flags & XING_TOC_FLAG) {
      gint i;
      for (i = 0; i < 100; i++)
        flump3dec->xing_seek_table[i] = bs_getbits (flump3dec->bs, 8);
    } else {
      memset (flump3dec->xing_seek_table, 0, 100);
    }

    if (xing_flags & XING_VBR_SCALE_FLAG)
      flump3dec->xing_vbr_scale = bs_getbits (flump3dec->bs, 32);
    else 
      flump3dec->xing_vbr_scale = 0;

    GST_DEBUG ("Xing header reported %u frames, time %" G_GUINT64_FORMAT 
        ", vbr scale %u\n", flump3dec->xing_frames, 
        flump3dec->xing_total_time, flump3dec->xing_vbr_scale);
  }
  else {
    GST_DEBUG ("No Xing header found");
  }

  return MP3TL_ERR_OK;
}

static void
gst_flump3dec_update_ts (FluMp3Dec *flump3dec, GstClockTime new_ts,
    const fr_header *mp3hdr)
{
  GstClockTimeDiff diff;
  GstClockTime out_ts = flump3dec->next_ts;
  GstClockTime frame_dur = gst_util_uint64_scale (GST_SECOND, 
                               mp3hdr->frame_samples, mp3hdr->sample_rate);

  /* Only take the new timestamp if it is more than half a frame from
   * our current timestamp */
  if (GST_CLOCK_TIME_IS_VALID (out_ts)) {
    diff = ABS ((GstClockTimeDiff)(new_ts - out_ts));
    if ((GstClockTime) diff > (frame_dur / 2)) {
      GST_DEBUG_OBJECT (flump3dec, "Got frame with new TS %" 
          G_GUINT64_FORMAT " - using.", new_ts);
      out_ts = new_ts;
    }
  }

  flump3dec->next_ts = out_ts;
}

static GstFlowReturn
flump3dec_drain_avail (GstPad *pad, gboolean more_data)
{
  Mp3TlRetcode    result;
  const fr_header *mp3hdr   = NULL;
  GstBuffer       *out_buf  = NULL;
  GstFlowReturn   ret = GST_FLOW_OK;
  FluMp3Dec        *flump3dec = FLUMP3DEC (GST_PAD_PARENT (pad));
  GstClockTime    dec_ts = GST_CLOCK_TIME_NONE;
  GstTagList      *taglist = NULL;

  GST_DEBUG_OBJECT (flump3dec, "draining, more: %d", more_data);

  mp3tl_set_eos (flump3dec->dec, more_data);
  
  while (bs_bits_avail (flump3dec->bs) > 0) {
    /* Find an mp3 header */
    result = mp3tl_sync (flump3dec->dec);
    if (result != MP3TL_ERR_OK)
      break ; /* Need more data */

    result = mp3tl_decode_header (flump3dec->dec, &mp3hdr);
    if (result != MP3TL_ERR_OK) {
      if (result == MP3TL_ERR_NEED_DATA) 
        break; /* Need more data */
      else if (result == MP3TL_ERR_STREAM)
        continue; /* Resync */
      else {
        /* Fatal decoder error */
        ret = GST_FLOW_ERROR;
        goto decode_error;
      }
    }

    g_return_val_if_fail (mp3hdr != NULL, GST_FLOW_ERROR);
    
    if (flump3dec->frame_count == 0) {
      gchar *codec;
      guint ver;
      /* For the first frame in the file, look for a Xing frame after 
       * the header */

      /* Set codec tag */
      if (mp3hdr->version == MPEG_VERSION_1)
        ver = 1;
      else
        ver = 2;

      if (mp3hdr->layer == 3) {
        codec = g_strdup_printf ("MPEG %d Audio, Layer %d (MP3)", 
            ver, mp3hdr->layer);
      }
      else {
        codec = g_strdup_printf ("MPEG %d Audio, Layer %d", 
            ver, mp3hdr->layer);
      }
      taglist = gst_tag_list_new();
      gst_tag_list_add(taglist, GST_TAG_MERGE_REPLACE, GST_TAG_AUDIO_CODEC,
          codec, NULL);
      gst_element_found_tags_for_pad(GST_ELEMENT(flump3dec), 
          flump3dec->srcpad, taglist);
      g_free (codec);
      /* end setting the tag */

      GST_DEBUG ("Checking first frame for Xing VBR header");
      result = flump3dec_check_for_xing (flump3dec, mp3hdr);
      
      if (result == MP3TL_ERR_NEED_DATA)
        break;
    }

    flump3dec->bitrate_sum += mp3hdr->bitrate;
    flump3dec->frame_count++;

    /* Round the bitrate to the nearest kbps */
    flump3dec->avg_bitrate = (guint)
        (flump3dec->bitrate_sum / flump3dec->frame_count + 500);
    flump3dec->avg_bitrate -= flump3dec->avg_bitrate % 1000;

    /* Change the output caps based on the header */
    if ((mp3hdr->sample_rate != flump3dec->rate ||
        mp3hdr->channels != flump3dec->channels)) {
      GstCaps *caps = gst_caps_new_simple ("audio/x-raw-int",
            "endianness", G_TYPE_INT, G_BYTE_ORDER,
            "signed", G_TYPE_BOOLEAN, TRUE,
            "width", G_TYPE_INT, 16,
            "depth", G_TYPE_INT, 16,
            "rate", G_TYPE_INT, mp3hdr->sample_rate,
            "channels", G_TYPE_INT, mp3hdr->channels, NULL);

      GST_DEBUG_OBJECT (flump3dec, "Caps change, rate: %d->%d channels %d->%d", 
	      flump3dec->rate, mp3hdr->sample_rate,
	      flump3dec->channels, mp3hdr->channels);
      if (!gst_pad_set_caps (flump3dec->srcpad, caps)) {
        gst_caps_unref (caps);
        GST_ELEMENT_ERROR (flump3dec, CORE, NEGOTIATION, (NULL), (NULL));

        return GST_FLOW_ERROR;
      }
      gst_caps_unref (caps);

      flump3dec->rate = mp3hdr->sample_rate;
      flump3dec->channels = mp3hdr->channels;
      flump3dec->bytes_per_sample = mp3hdr->channels * 
          mp3hdr->sample_size / 8;
    }

    /* Check whether the buffer has enough bits to decode the frame
     * minus the header that was already consumed */
    if (bs_bits_avail (flump3dec->bs) < mp3hdr->frame_bits - 32) {
      GST_INFO ("Need %" G_GINT64_FORMAT " more bits to decode this frame",
          (mp3hdr->frame_bits - 32) - bs_bits_avail (flump3dec->bs));
      break; /* Go get more data */
    }

    /* We have enough bytes in the store, decode a frame */
    ret = gst_pad_alloc_buffer (flump3dec->srcpad, GST_BUFFER_OFFSET_NONE, 
        mp3hdr->frame_samples * flump3dec->bytes_per_sample,
        GST_PAD_CAPS (flump3dec->srcpad), &out_buf);
    if (ret != GST_FLOW_OK) {
      GST_DEBUG_OBJECT (flump3dec, "Peer doesn't want buffer, skipping decode");
      /* Peer didn't want the buffer */
      result = mp3tl_skip_frame (flump3dec->dec, &dec_ts);
      if (GST_CLOCK_TIME_IS_VALID (dec_ts) && 
          dec_ts != flump3dec->last_dec_ts) {
        /* Use the new timestamp now, and store it so we don't repeat it. */
        gst_flump3dec_update_ts (flump3dec, dec_ts, mp3hdr);
        flump3dec->last_dec_ts = dec_ts;
      }
      goto no_buffer;
    }

    result = mp3tl_decode_frame (flump3dec->dec, GST_BUFFER_DATA (out_buf),
        GST_BUFFER_SIZE (out_buf), &dec_ts);

    if (result != MP3TL_ERR_OK) {
      /* Free up the buffer we allocated above */
      gst_buffer_unref (out_buf);
      out_buf = NULL;
 
      if (result == MP3TL_ERR_NEED_DATA) {
        /* Should never happen, since we checked we had enough bits */
        g_warning ("Decoder requested more data than it said it needed!");
        break;
      } 
      else if (result == MP3TL_ERR_BAD_FRAME) {
        /* Update time, and repeat the previous frame if we have one */
        flump3dec->bad = TRUE;

      	if (flump3dec->pending_frame != NULL) {
      	  GST_DEBUG_OBJECT (flump3dec, "Bad frame - using previous frame");
      	  out_buf = gst_buffer_create_sub (flump3dec->pending_frame, 0, 
              GST_BUFFER_SIZE (flump3dec->pending_frame));
      	  if (out_buf == NULL) {
      	    ret = GST_FLOW_ERROR;
      	    goto no_buffer;
      	  }
      	  gst_buffer_set_caps (out_buf,
              GST_BUFFER_CAPS (flump3dec->pending_frame));
      	}
      	else {
      	  GST_DEBUG_OBJECT (flump3dec, "Bad frame - no existing frame. Skipping");
      	  flump3dec->next_ts += gst_util_uint64_scale (GST_SECOND, 
              mp3hdr->frame_samples, mp3hdr->sample_rate);
      	  continue;
      	}
      }
      else {
        ret = GST_FLOW_ERROR;
        goto decode_error;
      }
    }

    /* Got a good frame */
    flump3dec->bad = FALSE;

    /* Set the bitrate tag if changed (only care about changes over 10kbps) */
    if ((flump3dec->last_posted_bitrate / 10240) != (flump3dec->avg_bitrate / 10240)) {
      flump3dec->last_posted_bitrate = flump3dec->avg_bitrate;
    	taglist = gst_tag_list_new();
    	gst_tag_list_add(taglist, GST_TAG_MERGE_REPLACE, GST_TAG_BITRATE,
          flump3dec->last_posted_bitrate, NULL);
    	gst_element_found_tags_for_pad(GST_ELEMENT(flump3dec),  flump3dec->srcpad,
          taglist);
    }
    

    if (GST_CLOCK_TIME_IS_VALID (dec_ts) && 
        dec_ts != flump3dec->last_dec_ts) {
      /* Use the new timestamp now, and store it so we don't repeat it. */
      gst_flump3dec_update_ts (flump3dec, dec_ts, mp3hdr);

      flump3dec->last_dec_ts = dec_ts;
    }
    
    if (G_UNLIKELY (flump3dec->need_discont)) {
      GST_BUFFER_FLAG_SET (out_buf, GST_BUFFER_FLAG_DISCONT);
      flump3dec->need_discont = FALSE;
    }
      
    GST_BUFFER_TIMESTAMP (out_buf) = flump3dec->next_ts;
    GST_BUFFER_DURATION (out_buf) = gst_util_uint64_scale (GST_SECOND, 
        mp3hdr->frame_samples, mp3hdr->sample_rate);
    flump3dec->next_ts += GST_BUFFER_DURATION (out_buf);

    GST_DEBUG_OBJECT (flump3dec, "Have new buffer, size %u, ts %" 
        GST_TIME_FORMAT, GST_BUFFER_SIZE (out_buf), 
        GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (out_buf)));

    if (flump3dec->pending_frame == NULL) {
      GST_DEBUG_OBJECT (flump3dec, "Storing as pending frame"); 
      flump3dec->pending_frame = out_buf;
    }
    else {
      /* push previous frame, queue current frame. */
      GST_DEBUG_OBJECT (flump3dec, "Pushing previous frame, ts %" \
          GST_TIME_FORMAT, 
          GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (flump3dec->pending_frame))); 

      ret = gst_pad_push (flump3dec->srcpad, flump3dec->pending_frame);
      flump3dec->pending_frame = out_buf;
      if (ret != GST_FLOW_OK) 
        goto error_out;
    }
  }

  /* Might need to flush out the pending decoded frame */
  if (!more_data && (flump3dec->pending_frame != NULL)) {
    GST_DEBUG_OBJECT (flump3dec, "Pushing pending frame"); 
    ret = gst_pad_push (flump3dec->srcpad, flump3dec->pending_frame);
    flump3dec->pending_frame = NULL;
    if (ret != GST_FLOW_OK) 
      goto error_out;
  }

  return GST_FLOW_OK;
no_buffer:
  if (GST_FLOW_IS_FATAL (ret)) {
    GST_ELEMENT_ERROR (flump3dec, RESOURCE, FAILED, (NULL),
        ("Failed to allocate output buffer: reason %s",
        gst_flow_get_name (ret)));
  }
  return ret;
error_out:
  return ret;
decode_error: 
  {
    const char *reason = mp3tl_get_err_reason (flump3dec->dec);

    /* Set element error */
    if (reason)
      GST_ELEMENT_ERROR (flump3dec, RESOURCE, FAILED, (NULL),
          ("Failed in mp3 stream decoding: %s", reason));
    else 
      GST_ELEMENT_ERROR (flump3dec, RESOURCE, FAILED, (NULL),
          ("Failed in mp3 stream decoding: Unknown reason"));
    return ret;
  }
}

/* Handle incoming events on the sink pad */
static gboolean 
flump3dec_sink_event (GstPad *pad, GstEvent *event)
{
  FluMp3Dec *flump3dec = FLUMP3DEC (GST_PAD_PARENT (pad));
  
  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_NEWSEGMENT:
    {
      gdouble rate;
      GstFormat format;
      gint64 start, end, base;
      gboolean result, update;
      gboolean converted = FALSE;

      gst_event_parse_new_segment (event, &update, &rate, &format,
          &start, &end, &base);

      GST_DEBUG_OBJECT (flump3dec,  "new segment, format=%d, base=%lld, " \
          "start = %lld, end=%lld", format, base, start, end);

      if (format == GST_FORMAT_BYTES) {
        gint64 disc_start, disc_end, disc_base;

        if (flump3_inbytes_to_time (flump3dec, start, &disc_start) &&
            flump3_inbytes_to_time (flump3dec, end, &disc_end) &&
            flump3_inbytes_to_time (flump3dec, base, &disc_base)) {
          gst_event_unref (event);
          event = gst_event_new_new_segment (FALSE, rate, GST_FORMAT_TIME,
              disc_start, disc_end, disc_base);
          flump3dec->next_ts = disc_start;

          GST_DEBUG_OBJECT (flump3dec,  "Converted to TIME, base=%lld, " \
              "start = %lld, end=%lld", disc_base, disc_start, disc_end);
          converted = TRUE;
        }
      }
      else if (format == GST_FORMAT_TIME) {
        GST_DEBUG_OBJECT (flump3dec, "Got segment in time format");
        flump3dec->next_ts = start;
      }

      if (!converted && format != GST_FORMAT_TIME) {
        gst_event_unref (event);
        GST_DEBUG_OBJECT (flump3dec, "creating new time segment");
        event = gst_event_new_new_segment (FALSE, rate, GST_FORMAT_TIME,
            0, GST_CLOCK_TIME_NONE, 0);
        flump3dec->next_ts = 0;
      }

      result = gst_pad_push_event (flump3dec->srcpad, event);

      return result;
    }
    case GST_EVENT_FLUSH_STOP:
      flump3dec_flush (flump3dec);
      break;
    case GST_EVENT_EOS:
      /* Output any remaining frames */
      flump3dec_drain_avail (pad, FALSE);
      break;
    default:
      break;
  }

  return gst_pad_event_default (pad, event);
}

static GstStateChangeReturn 
flump3dec_change_state (GstElement *element, GstStateChange transition)
{
  GstStateChangeReturn ret;
  FluMp3Dec *dec = FLUMP3DEC (element);

  g_return_val_if_fail (dec != NULL, GST_STATE_CHANGE_FAILURE);
  
  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      /* Prepare the decoder */
      flump3dec_reset (dec);
      break;
    default:
      break;
  }

  ret = parent_class->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      /* Clean up a bit */
      flump3dec_flush (dec);
      break;
    default:
      break;
  }
  return ret;
}

static const GstQueryType *
flump3dec_get_query_types (GstPad *pad ATTR_UNUSED)
{
  static const GstQueryType query_types[] = {
    GST_QUERY_POSITION,
    GST_QUERY_DURATION,
    0
  };

  return query_types;
}

static gboolean flump3_inbytes_to_time (FluMp3Dec *flump3dec,
    gint64 byteval, gint64 *timeval)
{
  if (flump3dec->avg_bitrate == 0 || timeval == NULL) 
    return FALSE;

  if (byteval == -1)
    *timeval = -1;
  else
    *timeval = gst_util_uint64_scale (GST_SECOND, byteval * 8, 
                   flump3dec->avg_bitrate);

  return TRUE;
}

static gboolean 
flump3dec_total_bytes (FluMp3Dec *flump3dec, gint64 *total)
{
  GstQuery *query;
  GstPad *peer;

  if ((peer = gst_pad_get_peer (flump3dec->sinkpad)) == NULL)
     return FALSE;

  query = gst_query_new_duration (GST_FORMAT_BYTES);
  gst_query_set_duration (query, GST_FORMAT_BYTES, -1);

  if (!gst_pad_query (peer, query)) {
    gst_object_unref (peer);
    return FALSE;
  }

  gst_object_unref (peer);
  
  gst_query_parse_duration (query, NULL, total);

  return TRUE;
}

static gboolean 
flump3dec_total_time (FluMp3Dec *flump3dec, gint64 *total)
{
  /* If we have a Xing header giving us the total number of frames,
   * use that to get total time */
  if (flump3dec->xing_flags & XING_FRAMES_FLAG) {
    *total = flump3dec->xing_total_time;
  }
  else {
    /* Calculate time from our bitrate */
    if (!flump3dec_total_bytes (flump3dec, total))
      return FALSE;

    if (*total != -1 && !flump3_inbytes_to_time (flump3dec, *total, total))
      return FALSE;
  }

  return TRUE;
}

static gboolean flump3dec_src_query (GstPad *pad, GstQuery *query)
{
  GstFormat format;
  gint64 cur, total;
  FluMp3Dec *flump3dec = FLUMP3DEC (gst_pad_get_parent (pad));
  GstPad *peer;
  gboolean res = FALSE;

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_POSITION:
    {
      /* Can't answer any queries if the upstream peer got unlinked */
      if ((peer = gst_pad_get_peer (flump3dec->sinkpad)) == NULL)
        goto error;

      gst_query_parse_position (query, &format, NULL);

      /* If the format is BYTES or SAMPLES (default), we'll calculate it from 
       * time, else let's see if upstream knows the position in the right 
       * format */
      if (format != GST_FORMAT_BYTES && format != GST_FORMAT_BYTES && 
          gst_pad_query (peer, query)) {
        gst_object_unref (peer);
        res = TRUE;
        goto out;
      }
      gst_object_unref (peer);

      /* Bring to time format, and from there to output format if needed */
      cur = flump3dec->next_ts;

      if (format != GST_FORMAT_TIME &&
          !flump3dec_src_convert (pad, GST_FORMAT_TIME, cur, &format, &cur)) {
        gst_query_set_position (query, format, -1);
        goto error;
      }
      gst_query_set_position (query, format, cur);
      break;
    }
    case GST_QUERY_DURATION:
    {
      /* Can't answer any queries if the upstream peer got unlinked */
      if ((peer = gst_pad_get_peer (flump3dec->sinkpad)) == NULL)
        goto error;

      gst_query_parse_duration (query, &format, NULL);

      /* If the format is BYTES or SAMPLES (default), we'll calculate it from 
       * time, else let's see if upstream knows the duration in the right 
       * format */
      if (format != GST_FORMAT_BYTES && format != GST_FORMAT_DEFAULT && 
          gst_pad_query (peer, query)) {
        gst_object_unref (peer);
        res = TRUE;
        goto out;
      }
      gst_object_unref (peer);
      peer = NULL;
      
      if (!flump3dec_total_time (flump3dec, &total))
        goto error;

      if (total != -1) {
        if (format != GST_FORMAT_TIME &&
            !flump3dec_src_convert (pad, GST_FORMAT_TIME, total, 
              &format, &total)) {
          gst_query_set_duration (query, format, -1);
          goto error;
        }
      }

      gst_query_set_duration (query, format, total);
      res = TRUE;
      break;
    }
    default:
      res = gst_pad_query_default (pad, query);
      break;
  }

out:
  gst_object_unref (flump3dec);
  return res;
error:
  gst_object_unref (flump3dec);
  return FALSE;
}

static gboolean flump3dec_src_convert (GstPad *pad, GstFormat src_format,
    gint64 src_value, GstFormat *dest_format, gint64 *dest_value)
{
  FluMp3Dec *flump3dec = FLUMP3DEC (gst_pad_get_parent (pad));
  gboolean res = FALSE;

  g_return_val_if_fail (flump3dec != NULL, FALSE);

  /* 0 always maps to 0, and -1 to -1 */
  if (src_value == 0 || src_value == -1) {
    *dest_value = src_value;
    return TRUE;
  }

  if (flump3dec->rate == 0 || flump3dec->bytes_per_sample == 0) {
    gst_object_unref (flump3dec);
    return FALSE;
  }
  
  switch (src_format) {
    case GST_FORMAT_BYTES:
      switch (*dest_format) {
        case GST_FORMAT_TIME: /* Bytes to time */
          *dest_value = gst_util_uint64_scale (src_value, GST_SECOND, 
                            (guint64)(flump3dec->bytes_per_sample) * 
                            flump3dec->rate);
          res = TRUE;
          break;
        case GST_FORMAT_DEFAULT: /* Bytes to samples */
          *dest_value = src_value / flump3dec->bytes_per_sample;
          res = TRUE;
          break;
        default:
          break;
      }
      break;
    case GST_FORMAT_TIME:
      switch (*dest_format) {
        case GST_FORMAT_BYTES: /* Time to bytes */
          *dest_value = gst_util_uint64_scale (src_value, 
                            (guint64)(flump3dec->bytes_per_sample) * 
                            flump3dec->rate, GST_SECOND);
          res = TRUE;
          break;
        case GST_FORMAT_DEFAULT: /* Time to samples */
          *dest_value = gst_util_uint64_scale (src_value, flump3dec->rate, 
                            GST_SECOND);
          res = TRUE;
          break;
        default:
          break;
      }
      break;
    case GST_FORMAT_DEFAULT: /* Samples */
      switch (*dest_format) {
        case GST_FORMAT_BYTES: /* Samples to bytes */
          *dest_value = src_value * flump3dec->bytes_per_sample;
          res = TRUE;
          break;
        case GST_FORMAT_TIME: /* Samples to time */
          *dest_value = gst_util_uint64_scale (src_value, GST_SECOND, 
                            flump3dec->rate);
          res = TRUE;
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  
  gst_object_unref (flump3dec);
  return res;
}

static gboolean
flump3dec_time_to_bytepos (FluMp3Dec *flump3dec, GstClockTime ts, 
    gint64 *bytepos)
{
  /* 0 always maps to 0, and -1 to -1 */
  if (ts == 0 || ts == (GstClockTime)(-1)) {
    *bytepos = (gint64) ts;
    return TRUE;
  }

  /* If we have a Xing seek table, determine a byte offset to seek to */
  if (flump3dec->xing_flags & XING_TOC_FLAG) {
    gdouble new_pos_percent;
    gint int_percent;
    gint64 total;
    gdouble fa, fb, fx;

    if (!flump3dec_total_time (flump3dec, &total))
      return FALSE;

    /* We need to know what point in the file to go to, from 0-100% */
    new_pos_percent = (100.0 * ts) / total;
    new_pos_percent = CLAMP (new_pos_percent, 0.0, 100.0);

    int_percent = CLAMP ((int)(new_pos_percent), 0, 99);

    fa = flump3dec->xing_seek_table[int_percent];
    if (int_percent < 99)
      fb = flump3dec->xing_seek_table[int_percent+1];
    else
      fb = 256.0;
    fx = fa + (fb-fa)*(new_pos_percent-int_percent);

    if (!flump3dec_total_bytes (flump3dec, &total))
      return FALSE;

    *bytepos = (gint64) ((fx / 256.0) * total);
    GST_DEBUG ("Xing seeking for %g percent time mapped to %g in bytes\n",
        new_pos_percent, fx * 100.0 / 256.0);
  }
  else {
    /* Otherwise, convert to bytes using bitrate and send upstream */
    if (flump3dec->avg_bitrate == 0)
      return FALSE;

    *bytepos = gst_util_uint64_scale (ts, flump3dec->avg_bitrate, 
                    (8 * GST_SECOND));
  }

  return TRUE;
}

static gboolean 
flump3dec_src_event (GstPad *pad, GstEvent *event)
{
  FluMp3Dec *flump3dec = FLUMP3DEC (gst_pad_get_parent (pad));
  gboolean result;

  g_return_val_if_fail (flump3dec != NULL, FALSE);

  if (GST_EVENT_TYPE (event) == GST_EVENT_SEEK)
  {
    GstFormat format;
    GstEvent *seek_event;
    gint64    start, stop;
    GstSeekType start_type, stop_type;
    GstSeekFlags in_flags;
    GstFormat in_format;
    gdouble in_rate;

    gst_event_parse_seek (event, &in_rate, &in_format, &in_flags, 
        &start_type, &start, &stop_type, &stop);
    gst_event_unref (event);
    event = NULL;
     
    GST_DEBUG_OBJECT (flump3dec, 
        "Seek, format %d, flags %d, start type %d start %" G_GINT64_FORMAT 
        " stop type %d stop %" G_GINT64_FORMAT, 
	in_format, in_flags, start_type, start, stop_type, stop);

    /* Convert request to time format if we can */
    if (in_format == GST_FORMAT_DEFAULT || in_format == GST_FORMAT_BYTES) {
      format = GST_FORMAT_TIME;
      if (!flump3dec_src_convert (pad, in_format, start, &format, &start))
        goto error;
      if (!flump3dec_src_convert (pad, in_format, stop, &format, &stop))
        goto error;
    }
    else {
      format = in_format;
    }

    /* See if upstream can seek by our converted time, or by whatever the 
     * input format is */
    seek_event = gst_event_new_seek (in_rate, format, in_flags, start_type, 
                    start, stop_type, stop);
    g_return_val_if_fail (seek_event != NULL, FALSE);

    if (gst_pad_push_event (flump3dec->sinkpad, seek_event)) {
      result = TRUE;
      goto out;
    }
    seek_event = NULL;

    /* From here on, we can only support seeks based on TIME format */
    if (format != GST_FORMAT_TIME) 
      goto error;

    /* Convert TIME to BYTES and send upstream */
    if (!flump3dec_time_to_bytepos (flump3dec, (GstClockTime) start, &start))
      goto error;
    if (!flump3dec_time_to_bytepos (flump3dec, (GstClockTime) stop, &stop))
      goto error;

    seek_event = gst_event_new_seek (in_rate, GST_FORMAT_BYTES, in_flags, 
                     start_type, start, stop_type, stop);
    if (!seek_event)
      goto error;

    result = gst_pad_push_event (flump3dec->sinkpad, seek_event);
  }
  else {
    result = gst_pad_event_default (pad, event);
  }

out:
  gst_object_unref (flump3dec);
  return result;

error:
  gst_object_unref (flump3dec);
  return FALSE;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (flump3debug, "flump3dec", 0, "Fluendo MP3 Decoder");

  if (!gst_element_register (plugin, "flump3dec", GST_RANK_PRIMARY,
          flump3dec_get_type()))
    return FALSE;

  return TRUE;
}

/*
 * FIXME: Fill in the license, requires new enums in GStreamer
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR,
  "flump3dec", "Fluendo MP3 decoder",
  plugin_init, VERSION, GST_LICENSE_UNKNOWN, "Fluendo MP3 Decoder",
  "http://www.fluendo.com"
)
