/* GStreamer
 * Copyright (C) <2007> Nokia Corporation (contact <stefan.kost@nokia.com>)
 *               <2007> Wim Taymans <wim.taymans@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gst/rtp/gstrtpbuffer.h>

#include <string.h>
#include "gstrtpmp4adepay.h"

GST_DEBUG_CATEGORY_STATIC (rtpmp4adepay_debug);
#define GST_CAT_DEFAULT (rtpmp4adepay_debug)

/* elementfactory information */
static const GstElementDetails gst_rtp_mp4adepay_details =
GST_ELEMENT_DETAILS ("RTP MPEG4 audio depayloader",
    "Codec/Depayloader/Network",
    "Extracts MPEG4 audio from RTP packets (RFC 3016)",
    "Nokia Corporation (contact <stefan.kost@nokia.com>), "
    "Wim Taymans <wim.taymans@gmail.com>");

static GstStaticPadTemplate gst_rtp_mp4a_depay_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/mpeg,"
        "mpegversion = (int) 4," "framed = (boolean) false")
    );

static GstStaticPadTemplate gst_rtp_mp4a_depay_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) \"audio\", "
        "payload = (int) " GST_RTP_PAYLOAD_DYNAMIC_STRING ", "
        "clock-rate = (int) [1, MAX ], "
        "encoding-name = (string) \"MP4A-LATM\""
        /* All optional parameters
         *
         * "profile-level-id=[1,MAX]"
         * "config=" 
         */
    )
    );

GST_BOILERPLATE (GstRtpMP4ADepay, gst_rtp_mp4a_depay, GstBaseRTPDepayload,
    GST_TYPE_BASE_RTP_DEPAYLOAD);

static void gst_rtp_mp4a_depay_finalize (GObject * object);

static gboolean gst_rtp_mp4a_depay_setcaps (GstBaseRTPDepayload * depayload,
    GstCaps * caps);
static GstBuffer *gst_rtp_mp4a_depay_process (GstBaseRTPDepayload * depayload,
    GstBuffer * buf);

static GstStateChangeReturn gst_rtp_mp4a_depay_change_state (GstElement *
    element, GstStateChange transition);


static void
gst_rtp_mp4a_depay_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_mp4a_depay_src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_mp4a_depay_sink_template));

  gst_element_class_set_details (element_class, &gst_rtp_mp4adepay_details);
}

static void
gst_rtp_mp4a_depay_class_init (GstRtpMP4ADepayClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseRTPDepayloadClass *gstbasertpdepayload_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasertpdepayload_class = (GstBaseRTPDepayloadClass *) klass;

  gobject_class->finalize = gst_rtp_mp4a_depay_finalize;

  gstelement_class->change_state = gst_rtp_mp4a_depay_change_state;

  gstbasertpdepayload_class->process = gst_rtp_mp4a_depay_process;
  gstbasertpdepayload_class->set_caps = gst_rtp_mp4a_depay_setcaps;

  GST_DEBUG_CATEGORY_INIT (rtpmp4adepay_debug, "rtpmp4adepay", 0,
      "MPEG4 audio RTP Depayloader");
}

static void
gst_rtp_mp4a_depay_init (GstRtpMP4ADepay * rtpmp4adepay,
    GstRtpMP4ADepayClass * klass)
{
  rtpmp4adepay->adapter = gst_adapter_new ();
}

static void
gst_rtp_mp4a_depay_finalize (GObject * object)
{
  GstRtpMP4ADepay *rtpmp4adepay;

  rtpmp4adepay = GST_RTP_MP4A_DEPAY (object);

  g_object_unref (rtpmp4adepay->adapter);
  rtpmp4adepay->adapter = NULL;

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gst_rtp_mp4a_depay_setcaps (GstBaseRTPDepayload * depayload, GstCaps * caps)
{
  GstStructure *structure;
  GstRtpMP4ADepay *rtpmp4adepay;
  GstCaps *srccaps;
  const gchar *str;
  gint clock_rate;
  gint object_type;
  gint channels = 2;            /* default */
  gboolean res;

  rtpmp4adepay = GST_RTP_MP4A_DEPAY (depayload);

  structure = gst_caps_get_structure (caps, 0);

  if (!gst_structure_get_int (structure, "clock-rate", &clock_rate))
    clock_rate = 90000;         /* default */
  depayload->clock_rate = clock_rate;

  if (!gst_structure_get_int (structure, "object", &object_type))
    object_type = 2;            /* AAC LC default */

  srccaps = gst_caps_new_simple ("audio/mpeg",
      "mpegversion", G_TYPE_INT, 4,
      "framed", G_TYPE_BOOLEAN, FALSE, "channels", G_TYPE_INT, channels, NULL);

  if ((str = gst_structure_get_string (structure, "config"))) {
    GValue v = { 0 };

    g_value_init (&v, GST_TYPE_BUFFER);
    if (gst_value_deserialize (&v, str)) {
      GstBuffer *buffer;
      guint8 *data;
      guint size;
      gint i;

      buffer = gst_value_get_buffer (&v);
      gst_buffer_ref (buffer);
      g_value_unset (&v);

      data = GST_BUFFER_DATA (buffer);
      size = GST_BUFFER_SIZE (buffer);

      if (size < 2) {
        GST_WARNING_OBJECT (depayload, "config too short (%d < 2)", size);
        goto bad_config;
      }

      /* Parse StreamMuxConfig according to ISO/IEC 14496-3:
       *
       * audioMuxVersion           == 0 (1 bit)
       * allStreamsSameTimeFraming == 1 (1 bit)
       * numSubFrames              == rtpmp4adepay->numSubFrames (6 bits)
       * numProgram                == 0 (4 bits)
       * numLayer                  == 0 (3 bits)
       *
       * We only require audioMuxVersion == 0;
       *
       * The remaining bit of the second byte and the rest of the bits are used
       * for audioSpecificConfig which we need to set in codec_info.
       */
      if ((data[0] & 0x80) != 0x00) {
        GST_WARNING_OBJECT (depayload, "unknown audioMuxVersion 1");
        goto bad_config;
      }

      rtpmp4adepay->numSubFrames = (data[0] & 0x3F);

      GST_LOG_OBJECT (rtpmp4adepay, "numSubFrames %d",
          rtpmp4adepay->numSubFrames);

      /* shift rest of string 15 bits down */
      size -= 2;
      for (i = 0; i < size; i++) {
        data[i] = ((data[i + 1] & 1) << 7) | ((data[i + 2] & 0xfe) >> 1);
      }
      /* last bit, this is probably not needed. */
      data[i] = ((data[i + 1] & 1) << 7);
      GST_BUFFER_SIZE (buffer) = size + 1;

      gst_caps_set_simple (srccaps,
          "codec_data", GST_TYPE_BUFFER, buffer, NULL);
      gst_buffer_unref (buffer);
    } else {
      g_warning ("cannot convert config to buffer");
    }
  }
bad_config:
  res = gst_pad_set_caps (depayload->srcpad, srccaps);
  gst_caps_unref (srccaps);

  return res;
}

static GstBuffer *
gst_rtp_mp4a_depay_process (GstBaseRTPDepayload * depayload, GstBuffer * buf)
{
  GstRtpMP4ADepay *rtpmp4adepay;
  GstBuffer *outbuf;

  rtpmp4adepay = GST_RTP_MP4A_DEPAY (depayload);

  /* flush remaining data on discont */
  if (GST_BUFFER_IS_DISCONT (buf)) {
    gst_adapter_clear (rtpmp4adepay->adapter);
  }

  outbuf = gst_rtp_buffer_get_payload_buffer (buf);

  gst_adapter_push (rtpmp4adepay->adapter, outbuf);

  /* RTP marker bit indicates the last packet of the AudioMuxElement => create
   * and push a buffer */
  if (gst_rtp_buffer_get_marker (buf)) {
    guint avail;
    guint i;
    guint8 *data;
    guint pos;

    avail = gst_adapter_available (rtpmp4adepay->adapter);

    GST_LOG_OBJECT (rtpmp4adepay, "have marker and %u available", avail);

    outbuf = gst_adapter_take_buffer (rtpmp4adepay->adapter, avail);
    data = GST_BUFFER_DATA (outbuf);
    /* position in data we are at */
    pos = 0;

    /* looping through the number of sub-frames in the audio payload */
    for (i = 0; i <= rtpmp4adepay->numSubFrames; i++) {
      /* determine payload length and set buffer data pointer accordingly */
      guint skip;
      guint data_len;
      guint32 timestamp;

      GstBuffer *tmp = NULL;

      timestamp = gst_rtp_buffer_get_timestamp (buf);

      /* each subframe starts with a variable length encoding */
      data_len = 0;
      for (skip = 0; skip < avail; skip++) {
        data_len += data[skip];
        if (data[skip] != 0xff)
          break;
      }
      skip++;

      /* this can not be possible, we have not enough data or the length
       * decoding failed because we ran out of data. */
      if (skip + data_len > avail)
        goto wrong_size;

      GST_LOG_OBJECT (rtpmp4adepay,
          "subframe %u, header len %u, data len %u, left %u", i, skip, data_len,
          avail);

      /* take data out, skip the header */
      pos += skip;
      tmp = gst_buffer_create_sub (outbuf, pos, data_len);

      /* skip data too */
      skip += data_len;
      pos += data_len;

      /* update our pointers whith what we consumed */
      data += skip;
      avail -= skip;

      gst_buffer_set_caps (tmp, GST_PAD_CAPS (depayload->srcpad));

      /* only apply the timestamp for the first buffer. Based on gstrtpmp4gdepay.c */
      if (i == 0)
        gst_base_rtp_depayload_push_ts (depayload, timestamp, tmp);
      else
        gst_base_rtp_depayload_push (depayload, tmp);
    }

    /* just a check that lengths match */
    if (avail) {
      GST_ELEMENT_WARNING (depayload, STREAM, DECODE,
          ("Packet invalid"), ("Not all payload consumed: "
              "possible wrongly encoded packet."));
    }
  }
  return NULL;

  /* ERRORS */
wrong_size:
  {
    GST_ELEMENT_WARNING (rtpmp4adepay, STREAM, DECODE,
        ("Packet did not validate"), ("wrong packet size"));
    return NULL;
  }
}

static GstStateChangeReturn
gst_rtp_mp4a_depay_change_state (GstElement * element,
    GstStateChange transition)
{
  GstRtpMP4ADepay *rtpmp4adepay;
  GstStateChangeReturn ret;

  rtpmp4adepay = GST_RTP_MP4A_DEPAY (element);

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      gst_adapter_clear (rtpmp4adepay->adapter);
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    default:
      break;
  }
  return ret;
}

gboolean
gst_rtp_mp4a_depay_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "rtpmp4adepay",
      GST_RANK_MARGINAL, GST_TYPE_RTP_MP4A_DEPAY);
}
