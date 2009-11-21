/* GStreamer
 * Copyright (C) 2004 Wim Taymans <wim@fluendo.com>
 * Copyright (C) 2006 Tim-Philipp Müller <tim centricular net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-speexdec
 * @see_also: speexenc, oggdemux
 *
 * This element decodes a Speex stream to raw integer audio.
 * <ulink url="http://www.speex.org/">Speex</ulink> is a royalty-free
 * audio codec maintained by the <ulink url="http://www.xiph.org/">Xiph.org
 * Foundation</ulink>.
 *
 * <refsect2>
 * <title>Example pipelines</title>
 * |[
 * gst-launch -v filesrc location=speex.ogg ! oggdemux ! speexdec ! audioconvert ! audioresample ! alsasink
 * ]| Decode an Ogg/Speex file. To create an Ogg/Speex file refer to the
 * documentation of speexenc.
 * </refsect2>
 *
 * Last reviewed on 2006-04-05 (0.10.2)
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "gstspeexdec.h"
#include <string.h>
#include <gst/tag/tag.h>

GST_DEBUG_CATEGORY_STATIC (speexdec_debug);
#define GST_CAT_DEFAULT speexdec_debug

static const GstElementDetails speex_dec_details =
GST_ELEMENT_DETAILS ("Speex audio decoder",
    "Codec/Decoder/Audio",
    "decode speex streams to audio",
    "Wim Taymans <wim@fluendo.com>");

#define DEFAULT_ENH   TRUE

enum
{
  ARG_0,
  ARG_ENH
};

static GstStaticPadTemplate speex_dec_src_factory =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw-int, "
        "rate = (int) [ 6000, 48000 ], "
        "channels = (int) [ 1, 2 ], "
        "endianness = (int) BYTE_ORDER, "
        "signed = (boolean) true, " "width = (int) 16, " "depth = (int) 16")
    );

static GstStaticPadTemplate speex_dec_sink_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-speex")
    );

GST_BOILERPLATE (GstSpeexDec, gst_speex_dec, GstElement, GST_TYPE_ELEMENT);

static gboolean speex_dec_sink_event (GstPad * pad, GstEvent * event);
static GstFlowReturn speex_dec_chain (GstPad * pad, GstBuffer * buf);
static GstStateChangeReturn speex_dec_change_state (GstElement * element,
    GstStateChange transition);

static gboolean speex_dec_src_event (GstPad * pad, GstEvent * event);
static gboolean speex_dec_src_query (GstPad * pad, GstQuery * query);
static gboolean speex_dec_sink_query (GstPad * pad, GstQuery * query);
static const GstQueryType *speex_get_src_query_types (GstPad * pad);
static const GstQueryType *speex_get_sink_query_types (GstPad * pad);
static gboolean speex_dec_convert (GstPad * pad,
    GstFormat src_format, gint64 src_value,
    GstFormat * dest_format, gint64 * dest_value);

static void gst_speex_dec_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static void gst_speex_dec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);

static GstFlowReturn speex_dec_chain_parse_data (GstSpeexDec * dec,
    GstBuffer * buf, GstClockTime timestamp, GstClockTime duration);

static void
gst_speex_dec_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&speex_dec_src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&speex_dec_sink_factory));
  gst_element_class_set_details (element_class, &speex_dec_details);
}

static void
gst_speex_dec_class_init (GstSpeexDecClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_speex_dec_set_property;
  gobject_class->get_property = gst_speex_dec_get_property;

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_ENH,
      g_param_spec_boolean ("enh", "Enh", "Enable perceptual enhancement",
          DEFAULT_ENH, G_PARAM_READWRITE));

  gstelement_class->change_state = GST_DEBUG_FUNCPTR (speex_dec_change_state);

  GST_DEBUG_CATEGORY_INIT (speexdec_debug, "speexdec", 0,
      "speex decoding element");
}

static void
gst_speex_dec_reset (GstSpeexDec * dec)
{
  gst_segment_init (&dec->segment, GST_FORMAT_UNDEFINED);
  dec->granulepos = -1;
  dec->packetno = 0;
  dec->frame_size = 0;
  dec->frame_duration = 0;
  dec->mode = NULL;
  dec->header = NULL;           /* FIXME: free ?! */
  if (dec->state) {
    speex_decoder_destroy (dec->state);
    dec->state = NULL;
  }
}

static void
gst_speex_dec_init (GstSpeexDec * dec, GstSpeexDecClass * g_class)
{
  dec->sinkpad =
      gst_pad_new_from_static_template (&speex_dec_sink_factory, "sink");
  gst_pad_set_chain_function (dec->sinkpad,
      GST_DEBUG_FUNCPTR (speex_dec_chain));
  gst_pad_set_event_function (dec->sinkpad,
      GST_DEBUG_FUNCPTR (speex_dec_sink_event));
  gst_pad_set_query_type_function (dec->sinkpad,
      GST_DEBUG_FUNCPTR (speex_get_sink_query_types));
  gst_pad_set_query_function (dec->sinkpad,
      GST_DEBUG_FUNCPTR (speex_dec_sink_query));
  gst_element_add_pad (GST_ELEMENT (dec), dec->sinkpad);

  dec->srcpad =
      gst_pad_new_from_static_template (&speex_dec_src_factory, "src");
  gst_pad_use_fixed_caps (dec->srcpad);
  gst_pad_set_event_function (dec->srcpad,
      GST_DEBUG_FUNCPTR (speex_dec_src_event));
  gst_pad_set_query_type_function (dec->srcpad,
      GST_DEBUG_FUNCPTR (speex_get_src_query_types));
  gst_pad_set_query_function (dec->srcpad,
      GST_DEBUG_FUNCPTR (speex_dec_src_query));
  gst_element_add_pad (GST_ELEMENT (dec), dec->srcpad);

  dec->enh = DEFAULT_ENH;

  gst_speex_dec_reset (dec);
}

static gboolean
speex_dec_convert (GstPad * pad,
    GstFormat src_format, gint64 src_value,
    GstFormat * dest_format, gint64 * dest_value)
{
  gboolean res = TRUE;
  GstSpeexDec *dec;
  guint64 scale = 1;

  dec = GST_SPEEX_DEC (gst_pad_get_parent (pad));

  if (src_format == *dest_format) {
    *dest_value = src_value;
    res = TRUE;
    goto cleanup;
  }

  if (dec->packetno < 1) {
    res = FALSE;
    goto cleanup;
  }

  if (pad == dec->sinkpad &&
      (src_format == GST_FORMAT_BYTES || *dest_format == GST_FORMAT_BYTES)) {
    res = FALSE;
    goto cleanup;
  }

  switch (src_format) {
    case GST_FORMAT_TIME:
      switch (*dest_format) {
        case GST_FORMAT_BYTES:
          scale = 2 * dec->header->nb_channels;
        case GST_FORMAT_DEFAULT:
          *dest_value =
              gst_util_uint64_scale_int (scale * src_value, dec->header->rate,
              GST_SECOND);
          break;
        default:
          res = FALSE;
          break;
      }
      break;
    case GST_FORMAT_DEFAULT:
      switch (*dest_format) {
        case GST_FORMAT_BYTES:
          *dest_value = src_value * 2 * dec->header->nb_channels;
          break;
        case GST_FORMAT_TIME:
          *dest_value =
              gst_util_uint64_scale_int (src_value, GST_SECOND,
              dec->header->rate);
          break;
        default:
          res = FALSE;
          break;
      }
      break;
    case GST_FORMAT_BYTES:
      switch (*dest_format) {
        case GST_FORMAT_DEFAULT:
          *dest_value = src_value / (2 * dec->header->nb_channels);
          break;
        case GST_FORMAT_TIME:
          *dest_value = gst_util_uint64_scale_int (src_value, GST_SECOND,
              dec->header->rate * 2 * dec->header->nb_channels);
          break;
        default:
          res = FALSE;
          break;
      }
      break;
    default:
      res = FALSE;
      break;
  }

cleanup:
  gst_object_unref (dec);
  return res;
}

static const GstQueryType *
speex_get_sink_query_types (GstPad * pad)
{
  static const GstQueryType speex_dec_sink_query_types[] = {
    GST_QUERY_CONVERT,
    0
  };

  return speex_dec_sink_query_types;
}

static gboolean
speex_dec_sink_query (GstPad * pad, GstQuery * query)
{
  GstSpeexDec *dec;
  gboolean res;

  dec = GST_SPEEX_DEC (gst_pad_get_parent (pad));

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_CONVERT:
    {
      GstFormat src_fmt, dest_fmt;
      gint64 src_val, dest_val;

      gst_query_parse_convert (query, &src_fmt, &src_val, &dest_fmt, &dest_val);
      res = speex_dec_convert (pad, src_fmt, src_val, &dest_fmt, &dest_val);
      if (res) {
        gst_query_set_convert (query, src_fmt, src_val, dest_fmt, dest_val);
      }
      break;
    }
    default:
      res = gst_pad_query_default (pad, query);
      break;
  }

  gst_object_unref (dec);
  return res;
}

static const GstQueryType *
speex_get_src_query_types (GstPad * pad)
{
  static const GstQueryType speex_dec_src_query_types[] = {
    GST_QUERY_POSITION,
    GST_QUERY_DURATION,
    0
  };

  return speex_dec_src_query_types;
}

static gboolean
speex_dec_src_query (GstPad * pad, GstQuery * query)
{
  GstSpeexDec *dec;
  gboolean res = FALSE;

  dec = GST_SPEEX_DEC (gst_pad_get_parent (pad));

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_POSITION:{
      GstSegment segment;
      GstFormat format;
      gint64 cur;

      gst_query_parse_position (query, &format, NULL);

      GST_PAD_STREAM_LOCK (dec->sinkpad);
      segment = dec->segment;
      GST_PAD_STREAM_UNLOCK (dec->sinkpad);

      if (segment.format != GST_FORMAT_TIME) {
        GST_DEBUG_OBJECT (dec, "segment not initialised yet");
        break;
      }

      if ((res = speex_dec_convert (dec->srcpad, GST_FORMAT_TIME,
                  segment.last_stop, &format, &cur))) {
        gst_query_set_position (query, format, cur);
      }
      break;
    }
    case GST_QUERY_DURATION:{
      GstFormat format = GST_FORMAT_TIME;
      gint64 dur;

      /* get duration from demuxer */
      if (!gst_pad_query_peer_duration (dec->sinkpad, &format, &dur))
        break;

      gst_query_parse_duration (query, &format, NULL);

      /* and convert it into the requested format */
      if ((res = speex_dec_convert (dec->srcpad, GST_FORMAT_TIME,
                  dur, &format, &dur))) {
        gst_query_set_duration (query, format, dur);
      }
      break;
    }
    default:
      res = gst_pad_query_default (pad, query);
      break;
  }

  gst_object_unref (dec);
  return res;
}

static gboolean
speex_dec_src_event (GstPad * pad, GstEvent * event)
{
  gboolean res = FALSE;
  GstSpeexDec *dec = GST_SPEEX_DEC (gst_pad_get_parent (pad));

  GST_LOG_OBJECT (dec, "handling %s event", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_SEEK:{
      GstFormat format, tformat;
      gdouble rate;
      GstEvent *real_seek;
      GstSeekFlags flags;
      GstSeekType cur_type, stop_type;
      gint64 cur, stop;
      gint64 tcur, tstop;

      gst_event_parse_seek (event, &rate, &format, &flags, &cur_type, &cur,
          &stop_type, &stop);

      /* we have to ask our peer to seek to time here as we know
       * nothing about how to generate a granulepos from the src
       * formats or anything.
       *
       * First bring the requested format to time
       */
      tformat = GST_FORMAT_TIME;
      if (!(res = speex_dec_convert (pad, format, cur, &tformat, &tcur)))
        break;
      if (!(res = speex_dec_convert (pad, format, stop, &tformat, &tstop)))
        break;

      /* then seek with time on the peer */
      real_seek = gst_event_new_seek (rate, GST_FORMAT_TIME,
          flags, cur_type, tcur, stop_type, tstop);

      GST_LOG_OBJECT (dec, "seek to %" GST_TIME_FORMAT, GST_TIME_ARGS (tcur));

      res = gst_pad_push_event (dec->sinkpad, real_seek);
      gst_event_unref (event);
      break;
    }
    default:
      res = gst_pad_event_default (pad, event);
      break;
  }

  gst_object_unref (dec);
  return res;
}

static gboolean
speex_dec_sink_event (GstPad * pad, GstEvent * event)
{
  GstSpeexDec *dec;
  gboolean ret = FALSE;

  dec = GST_SPEEX_DEC (gst_pad_get_parent (pad));

  GST_LOG_OBJECT (dec, "handling %s event", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_NEWSEGMENT:{
      GstFormat format;
      gdouble rate, arate;
      gint64 start, stop, time;
      gboolean update;

      gst_event_parse_new_segment_full (event, &update, &rate, &arate, &format,
          &start, &stop, &time);

      if (format != GST_FORMAT_TIME)
        goto newseg_wrong_format;

      if (rate <= 0.0)
        goto newseg_wrong_rate;

      if (update) {
        /* time progressed without data, see if we can fill the gap with
         * some concealment data */
        if (dec->segment.last_stop < start) {
          GstClockTime duration;

          duration = start - dec->segment.last_stop;
          speex_dec_chain_parse_data (dec, NULL, dec->segment.last_stop,
              duration);
        }
      }

      /* now configure the values */
      gst_segment_set_newsegment_full (&dec->segment, update,
          rate, arate, GST_FORMAT_TIME, start, stop, time);

      dec->granulepos = -1;

      GST_DEBUG_OBJECT (dec, "segment now: cur = %" GST_TIME_FORMAT " [%"
          GST_TIME_FORMAT " - %" GST_TIME_FORMAT "]",
          GST_TIME_ARGS (dec->segment.last_stop),
          GST_TIME_ARGS (dec->segment.start),
          GST_TIME_ARGS (dec->segment.stop));

      ret = gst_pad_push_event (dec->srcpad, event);
      break;
    }
    default:
      ret = gst_pad_event_default (pad, event);
      break;
  }

  gst_object_unref (dec);
  return ret;

  /* ERRORS */
newseg_wrong_format:
  {
    GST_DEBUG_OBJECT (dec, "received non TIME newsegment");
    gst_object_unref (dec);
    return FALSE;
  }
newseg_wrong_rate:
  {
    GST_DEBUG_OBJECT (dec, "negative rates not supported yet");
    gst_object_unref (dec);
    return FALSE;
  }
}

static GstFlowReturn
speex_dec_chain_parse_header (GstSpeexDec * dec, GstBuffer * buf)
{
  GstCaps *caps;

  /* get the header */
  dec->header = speex_packet_to_header ((char *) GST_BUFFER_DATA (buf),
      GST_BUFFER_SIZE (buf));

  if (!dec->header)
    goto no_header;

  if (dec->header->mode >= SPEEX_NB_MODES || dec->header->mode < 0)
    goto mode_too_old;

  dec->mode = (SpeexMode *) speex_mode_list[dec->header->mode];

  /* initialize the decoder */
  dec->state = speex_decoder_init (dec->mode);
  if (!dec->state)
    goto init_failed;

  speex_decoder_ctl (dec->state, SPEEX_SET_ENH, &dec->enh);
  speex_decoder_ctl (dec->state, SPEEX_GET_FRAME_SIZE, &dec->frame_size);

  if (dec->header->nb_channels != 1) {
    dec->callback.callback_id = SPEEX_INBAND_STEREO;
    dec->callback.func = speex_std_stereo_request_handler;
    dec->callback.data = &dec->stereo;
    speex_decoder_ctl (dec->state, SPEEX_SET_HANDLER, &dec->callback);
  }

  speex_decoder_ctl (dec->state, SPEEX_SET_SAMPLING_RATE, &dec->header->rate);

  dec->frame_duration = gst_util_uint64_scale_int (dec->frame_size,
      GST_SECOND, dec->header->rate);

  speex_bits_init (&dec->bits);

  /* set caps */
  caps = gst_caps_new_simple ("audio/x-raw-int",
      "rate", G_TYPE_INT, dec->header->rate,
      "channels", G_TYPE_INT, dec->header->nb_channels,
      "signed", G_TYPE_BOOLEAN, TRUE,
      "endianness", G_TYPE_INT, G_BYTE_ORDER,
      "width", G_TYPE_INT, 16, "depth", G_TYPE_INT, 16, NULL);

  if (!gst_pad_set_caps (dec->srcpad, caps))
    goto nego_failed;

  gst_caps_unref (caps);
  return GST_FLOW_OK;

  /* ERRORS */
no_header:
  {
    GST_ELEMENT_ERROR (GST_ELEMENT (dec), STREAM, DECODE,
        (NULL), ("couldn't read header"));
    return GST_FLOW_ERROR;
  }
mode_too_old:
  {
    GST_ELEMENT_ERROR (GST_ELEMENT (dec), STREAM, DECODE,
        (NULL),
        ("Mode number %d does not (yet/any longer) exist in this version",
            dec->header->mode));
    return GST_FLOW_ERROR;
  }
init_failed:
  {
    GST_ELEMENT_ERROR (GST_ELEMENT (dec), STREAM, DECODE,
        (NULL), ("couldn't initialize decoder"));
    return GST_FLOW_ERROR;
  }
nego_failed:
  {
    GST_ELEMENT_ERROR (GST_ELEMENT (dec), STREAM, DECODE,
        (NULL), ("couldn't negotiate format"));
    gst_caps_unref (caps);
    return GST_FLOW_NOT_NEGOTIATED;
  }
}

static GstFlowReturn
speex_dec_chain_parse_comments (GstSpeexDec * dec, GstBuffer * buf)
{
  GstTagList *list;
  gchar *ver, *encoder = NULL;

  list = gst_tag_list_from_vorbiscomment_buffer (buf, NULL, 0, &encoder);

  if (!list) {
    GST_WARNING_OBJECT (dec, "couldn't decode comments");
    list = gst_tag_list_new ();
  }

  if (encoder) {
    gst_tag_list_add (list, GST_TAG_MERGE_REPLACE,
        GST_TAG_ENCODER, encoder, NULL);
  }

  gst_tag_list_add (list, GST_TAG_MERGE_REPLACE,
      GST_TAG_AUDIO_CODEC, "Speex", NULL);

  ver = g_strndup (dec->header->speex_version, SPEEX_HEADER_VERSION_LENGTH);
  g_strstrip (ver);

  if (ver != NULL && *ver != '\0') {
    gst_tag_list_add (list, GST_TAG_MERGE_REPLACE,
        GST_TAG_ENCODER_VERSION, ver, NULL);
  }

  if (dec->header->bitrate > 0) {
    gst_tag_list_add (list, GST_TAG_MERGE_REPLACE,
        GST_TAG_BITRATE, (guint) dec->header->bitrate, NULL);
  }

  GST_INFO_OBJECT (dec, "tags: %" GST_PTR_FORMAT, list);

  gst_element_found_tags_for_pad (GST_ELEMENT (dec), dec->srcpad, list);

  g_free (encoder);
  g_free (ver);

  return GST_FLOW_OK;
}

static GstFlowReturn
speex_dec_chain_parse_data (GstSpeexDec * dec, GstBuffer * buf,
    GstClockTime timestamp, GstClockTime duration)
{
  GstFlowReturn res = GST_FLOW_OK;
  gint i, fpp;
  guint size;
  guint8 *data;
  SpeexBits *bits;

  if (timestamp != -1) {
    dec->segment.last_stop = timestamp;
    dec->granulepos = -1;
  }

  if (buf) {
    data = GST_BUFFER_DATA (buf);
    size = GST_BUFFER_SIZE (buf);

    /* send data to the bitstream */
    speex_bits_read_from (&dec->bits, (char *) data, size);

    fpp = 0;
    bits = &dec->bits;

    GST_DEBUG_OBJECT (dec, "received buffer of size %u, fpp %d", size, fpp);

    if (!GST_BUFFER_TIMESTAMP_IS_VALID (buf)
        && GST_BUFFER_OFFSET_END_IS_VALID (buf)) {
      dec->granulepos = GST_BUFFER_OFFSET_END (buf);
      GST_DEBUG_OBJECT (dec,
          "Taking granulepos from upstream: %" G_GUINT64_FORMAT,
          dec->granulepos);
    }

    /* copy timestamp */
  } else {
    /* concealment data, pass NULL as the bits parameters */
    GST_DEBUG_OBJECT (dec, "creating concealment data");
    fpp = dec->header->frames_per_packet;
    bits = NULL;
  }


  /* now decode each frame, catering for unknown number of them (e.g. rtp) */
  for (i = 0; (!fpp || i < fpp) && (!bits || speex_bits_remaining (bits) > 0);
      i++) {
    GstBuffer *outbuf;
    gint16 *out_data;
    gint ret;

    GST_LOG_OBJECT (dec, "decoding frame %d/%d", i, fpp);

    res = gst_pad_alloc_buffer_and_set_caps (dec->srcpad,
        GST_BUFFER_OFFSET_NONE, dec->frame_size * dec->header->nb_channels * 2,
        GST_PAD_CAPS (dec->srcpad), &outbuf);

    if (res != GST_FLOW_OK) {
      GST_DEBUG_OBJECT (dec, "buf alloc flow: %s", gst_flow_get_name (res));
      return res;
    }

    out_data = (gint16 *) GST_BUFFER_DATA (outbuf);

    ret = speex_decode_int (dec->state, bits, out_data);
    if (ret == -1) {
      /* uh? end of stream */
      GST_WARNING_OBJECT (dec, "Unexpected end of stream found");
      gst_buffer_unref (outbuf);
      outbuf = NULL;
      break;
    } else if (ret == -2) {
      GST_WARNING_OBJECT (dec, "Decoding error: corrupted stream?");
      gst_buffer_unref (outbuf);
      outbuf = NULL;
      break;
    }

    if (bits && speex_bits_remaining (bits) < 0) {
      GST_WARNING_OBJECT (dec, "Decoding overflow: corrupted stream?");
      gst_buffer_unref (outbuf);
      outbuf = NULL;
      break;
    }
    if (dec->header->nb_channels == 2)
      speex_decode_stereo_int (out_data, dec->frame_size, &dec->stereo);

    if (dec->granulepos == -1) {
      if (dec->segment.format != GST_FORMAT_TIME) {
        GST_WARNING_OBJECT (dec, "segment not initialized or not TIME format");
        dec->granulepos = dec->frame_size;
      } else {
        dec->granulepos = gst_util_uint64_scale_int (dec->segment.last_stop,
            dec->header->rate, GST_SECOND) + dec->frame_size;
      }
      GST_DEBUG_OBJECT (dec, "granulepos=%" G_GINT64_FORMAT, dec->granulepos);
    }

    if (timestamp == -1) {
      timestamp = gst_util_uint64_scale_int (dec->granulepos - dec->frame_size,
          GST_SECOND, dec->header->rate);
    }

    GST_BUFFER_OFFSET (outbuf) = dec->granulepos - dec->frame_size;
    GST_BUFFER_OFFSET_END (outbuf) = dec->granulepos;
    GST_BUFFER_TIMESTAMP (outbuf) = timestamp;
    GST_BUFFER_DURATION (outbuf) = dec->frame_duration;

    dec->granulepos += dec->frame_size;
    dec->segment.last_stop += dec->frame_duration;

    GST_LOG_OBJECT (dec, "pushing buffer with ts=%" GST_TIME_FORMAT ", dur=%"
        GST_TIME_FORMAT, GST_TIME_ARGS (timestamp),
        GST_TIME_ARGS (dec->frame_duration));

    res = gst_pad_push (dec->srcpad, outbuf);

    if (res != GST_FLOW_OK) {
      GST_DEBUG_OBJECT (dec, "flow: %s", gst_flow_get_name (res));
      break;
    }
    timestamp = -1;
  }

  return res;
}

static GstFlowReturn
speex_dec_chain (GstPad * pad, GstBuffer * buf)
{
  GstFlowReturn res;
  GstSpeexDec *dec;

  dec = GST_SPEEX_DEC (gst_pad_get_parent (pad));

  switch (dec->packetno) {
    case 0:
      res = speex_dec_chain_parse_header (dec, buf);
      break;
    case 1:
      res = speex_dec_chain_parse_comments (dec, buf);
      break;
    default:
      res =
          speex_dec_chain_parse_data (dec, buf, GST_BUFFER_TIMESTAMP (buf),
          GST_BUFFER_DURATION (buf));
      break;
  }

  dec->packetno++;

  gst_buffer_unref (buf);
  gst_object_unref (dec);

  return res;
}

static void
gst_speex_dec_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstSpeexDec *speexdec;

  speexdec = GST_SPEEX_DEC (object);

  switch (prop_id) {
    case ARG_ENH:
      g_value_set_boolean (value, speexdec->enh);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_speex_dec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstSpeexDec *speexdec;

  speexdec = GST_SPEEX_DEC (object);

  switch (prop_id) {
    case ARG_ENH:
      speexdec->enh = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static GstStateChangeReturn
speex_dec_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret;
  GstSpeexDec *dec = GST_SPEEX_DEC (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
    case GST_STATE_CHANGE_READY_TO_PAUSED:
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
    default:
      break;
  }

  ret = parent_class->change_state (element, transition);
  if (ret != GST_STATE_CHANGE_SUCCESS)
    return ret;

  switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      gst_speex_dec_reset (dec);
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      break;
    default:
      break;
  }

  return ret;
}
