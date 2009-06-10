/* GStreamer
 * Copyright (C) 1999 Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) 2005 Edgard Lima <edgard.lima@indt.org.br>
 * Copyright (C) 2005 Nokia Corporation <kai.vehmanen@nokia.com>
 * Copyright (C) 2007,2008 Axis Communications <dev-gstreamer@axis.com>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <gst/rtp/gstrtpbuffer.h>

#include "gstrtpg726pay.h"

GST_DEBUG_CATEGORY_STATIC (rtpg726pay_debug);
#define GST_CAT_DEFAULT (rtpg726pay_debug)

#define DEFAULT_FORCE_AAL2      TRUE

enum
{
  PROP_0,
  PROP_FORCE_AAL2,
  PROP_LAST
};

static const GstElementDetails gst_rtp_g726_pay_details =
GST_ELEMENT_DETAILS ("RTP G.726 payloader",
    "Codec/Payloader/Network",
    "Payload-encodes G.726 audio into a RTP packet",
    "Axis Communications <dev-gstreamer@axis.com>");

static GstStaticPadTemplate gst_rtp_g726_pay_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-adpcm, "
        "channels = (int) 1, "
        "rate = (int) 8000, "
        "bitrate = (int) { 16000, 24000, 32000, 40000 }, "
        "layout = (string) \"g726\"")
    );

static GstStaticPadTemplate gst_rtp_g726_pay_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) \"audio\", "
        "payload = (int) " GST_RTP_PAYLOAD_DYNAMIC_STRING ", "
        "clock-rate = (int) 8000, "
        "encoding-name = (string) { \"G726-16\", \"G726-24\", \"G726-32\", \"G726-40\", "
        "    \"AAL2-G726-16\", \"AAL2-G726-24\", \"AAL2-G726-32\", \"AAL2-G726-40\" } ")
    );

static void gst_rtp_g726_pay_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static void gst_rtp_g726_pay_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);

static gboolean gst_rtp_g726_pay_setcaps (GstBaseRTPPayload * payload,
    GstCaps * caps);
static GstFlowReturn gst_rtp_g726_pay_handle_buffer (GstBaseRTPPayload *
    payload, GstBuffer * buffer);

GST_BOILERPLATE (GstRtpG726Pay, gst_rtp_g726_pay, GstBaseRTPAudioPayload,
    GST_TYPE_BASE_RTP_AUDIO_PAYLOAD);

static void
gst_rtp_g726_pay_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_g726_pay_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_g726_pay_src_template));
  gst_element_class_set_details (element_class, &gst_rtp_g726_pay_details);
}

static void
gst_rtp_g726_pay_class_init (GstRtpG726PayClass * klass)
{
  GObjectClass *gobject_class;
  GstBaseRTPPayloadClass *gstbasertppayload_class;

  gobject_class = (GObjectClass *) klass;
  gstbasertppayload_class = (GstBaseRTPPayloadClass *) klass;

  gobject_class->set_property = gst_rtp_g726_pay_set_property;
  gobject_class->get_property = gst_rtp_g726_pay_get_property;

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_FORCE_AAL2,
      g_param_spec_boolean ("force-aal2", "Force AAL2",
          "Force AAL2 encoding for compatibility with bad depayloaders",
          DEFAULT_FORCE_AAL2, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gstbasertppayload_class->set_caps = gst_rtp_g726_pay_setcaps;
  gstbasertppayload_class->handle_buffer = gst_rtp_g726_pay_handle_buffer;

  GST_DEBUG_CATEGORY_INIT (rtpg726pay_debug, "rtpg726pay", 0,
      "G.726 RTP Payloader");
}

static void
gst_rtp_g726_pay_init (GstRtpG726Pay * rtpg726pay, GstRtpG726PayClass * klass)
{
  GstBaseRTPAudioPayload *basertpaudiopayload;

  basertpaudiopayload = GST_BASE_RTP_AUDIO_PAYLOAD (rtpg726pay);

  GST_BASE_RTP_PAYLOAD (rtpg726pay)->clock_rate = 8000;

  rtpg726pay->force_aal2 = DEFAULT_FORCE_AAL2;

  /* sample based codec */
  gst_base_rtp_audio_payload_set_sample_based (basertpaudiopayload);
}

static gboolean
gst_rtp_g726_pay_setcaps (GstBaseRTPPayload * payload, GstCaps * caps)
{
  gchar *encoding_name;
  GstStructure *structure;
  GstBaseRTPAudioPayload *basertpaudiopayload;
  GstRtpG726Pay *pay;
  GstCaps *peercaps;

  basertpaudiopayload = GST_BASE_RTP_AUDIO_PAYLOAD (payload);
  pay = GST_RTP_G726_PAY (payload);

  structure = gst_caps_get_structure (caps, 0);

  if (!gst_structure_get_int (structure, "bitrate", &pay->bitrate))
    pay->bitrate = 32000;

  GST_DEBUG_OBJECT (payload, "using bitrate %d", pay->bitrate);

  pay->aal2 = FALSE;

  /* first see what we can do with the bitrate */
  switch (pay->bitrate) {
    case 16000:
      encoding_name = g_strdup ("G726-16");
      gst_base_rtp_audio_payload_set_samplebits_options (basertpaudiopayload,
          2);
      break;
    case 24000:
      encoding_name = g_strdup ("G726-24");
      gst_base_rtp_audio_payload_set_samplebits_options (basertpaudiopayload,
          3);
      break;
    case 32000:
      encoding_name = g_strdup ("G726-32");
      gst_base_rtp_audio_payload_set_samplebits_options (basertpaudiopayload,
          4);
      break;
    case 40000:
      encoding_name = g_strdup ("G726-40");
      gst_base_rtp_audio_payload_set_samplebits_options (basertpaudiopayload,
          5);
      break;
    default:
      goto invalid_bitrate;
  }

  GST_DEBUG_OBJECT (payload, "selected base encoding %s", encoding_name);

  /* now see if we need to produce AAL2 or not */
  peercaps = gst_pad_peer_get_caps (payload->srcpad);
  if (peercaps) {
    GstCaps *filter, *intersect;
    gchar *capsstr;

    GST_DEBUG_OBJECT (payload, "have peercaps %" GST_PTR_FORMAT, peercaps);

    capsstr = g_strdup_printf ("application/x-rtp, "
        "media = (string) \"audio\", "
        "clock-rate = (int) 8000, "
        "encoding-name = (string) %s; "
        "application/x-rtp, "
        "media = (string) \"audio\", "
        "clock-rate = (int) 8000, "
        "encoding-name = (string) AAL2-%s", encoding_name, encoding_name);
    filter = gst_caps_from_string (capsstr);
    g_free (capsstr);

    /* intersect to filter */
    intersect = gst_caps_intersect (peercaps, filter);
    gst_caps_unref (peercaps);

    GST_DEBUG_OBJECT (payload, "intersected to %" GST_PTR_FORMAT, intersect);

    if (!intersect)
      goto no_format;
    if (gst_caps_is_empty (intersect)) {
      gst_caps_unref (intersect);
      goto no_format;
    }

    structure = gst_caps_get_structure (intersect, 0);

    /* now see what encoding name we settled on, we need to dup because the
     * string goes away when we unref the intersection below. */
    encoding_name =
        g_strdup (gst_structure_get_string (structure, "encoding-name"));

    /* if we managed to negotiate to AAL2, we definatly are going to do AAL2
     * encoding. Else we only encode AAL2 when explicitly set by the
     * property. */
    if (g_str_has_prefix (encoding_name, "AAL2-"))
      pay->aal2 = TRUE;
    else
      pay->aal2 = pay->force_aal2;

    GST_DEBUG_OBJECT (payload, "final encoding %s, AAL2 %d", encoding_name,
        pay->aal2);

    gst_caps_unref (intersect);
  } else {
    /* downstream can do anything but we prefer the better supported non-AAL2 */
    pay->aal2 = pay->force_aal2;
    GST_DEBUG_OBJECT (payload, "no peer caps, AAL2 %d", pay->aal2);
  }

  gst_basertppayload_set_options (payload, "audio", TRUE, encoding_name, 8000);
  gst_basertppayload_set_outcaps (payload, NULL);

  g_free (encoding_name);

  return TRUE;

  /* ERRORS */
invalid_bitrate:
  {
    GST_ERROR_OBJECT (payload, "invalid bitrate %d specified", pay->bitrate);
    return FALSE;
  }
no_format:
  {
    GST_ERROR_OBJECT (payload, "could not negotiate format");
    return FALSE;
  }
}

static GstFlowReturn
gst_rtp_g726_pay_handle_buffer (GstBaseRTPPayload * payload, GstBuffer * buffer)
{
  GstFlowReturn res;
  GstRtpG726Pay *pay;

  pay = GST_RTP_G726_PAY (payload);

  if (!pay->aal2) {
    guint8 *data, tmp;
    guint len;

    /* for non AAL2, we need to reshuffle the bytes, we can do this in-place
     * when the buffer is writable. */
    buffer = gst_buffer_make_writable (buffer);

    data = GST_BUFFER_DATA (buffer);
    len = GST_BUFFER_SIZE (buffer);

    GST_LOG_OBJECT (pay, "packing %u bytes of data", len);

    /* we need to reshuffle the bytes, output is of the form:
     * A B C D .. with the number of bits depending on the bitrate. */
    switch (pay->bitrate) {
      case 16000:
      {
        /*  0               
         *  0 1 2 3 4 5 6 7 
         * +-+-+-+-+-+-+-+-+-
         * |D D|C C|B B|A A| ...
         * |0 1|0 1|0 1|0 1|
         * +-+-+-+-+-+-+-+-+-
         */
        while (len > 0) {
          tmp = *data;
          *data++ = ((tmp & 0xc0) >> 6) |
              ((tmp & 0x30) >> 2) | ((tmp & 0x0c) << 2) | ((tmp & 0x03) << 6);
          len--;
        }
        break;
      }
      case 24000:
      {
        /*  0                   1                   2
         *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
         * |C C|B B B|A A A|F|E E E|D D D|C|H H H|G G G|F F| ...
         * |1 2|0 1 2|0 1 2|2|0 1 2|0 1 2|0|0 1 2|0 1 2|0 1|
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
         */
        while (len > 2) {
          tmp = *data;
          *data++ = ((tmp & 0xc0) >> 6) |
              ((tmp & 0x38) >> 1) | ((tmp & 0x07) << 5);
          tmp = *data;
          *data++ = ((tmp & 0x80) >> 7) |
              ((tmp & 0x70) >> 3) | ((tmp & 0x0e) << 4) | ((tmp & 0x01) << 7);
          tmp = *data;
          *data++ = ((tmp & 0xe0) >> 5) |
              ((tmp & 0x1c) >> 2) | ((tmp & 0x03) << 6);
          len -= 3;
        }
        break;
      }
      case 32000:
      {
        /*  0                   1
         *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
         * |B B B B|A A A A|D D D D|C C C C| ...
         * |0 1 2 3|0 1 2 3|0 1 2 3|0 1 2 3|
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
         */
        while (len > 0) {
          tmp = *data;
          *data++ = ((tmp & 0xf0) >> 4) | ((tmp & 0x0f) << 4);
          len--;
        }
        break;
      }
      case 40000:
      {
        /*  0                   1                   2                   3                   4
         *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
         * |B B B|A A A A A|D|C C C C C|B B|E E E E|D D D D|G G|F F F F F|E|H H H H H|G G G|
         * |2 3 4|0 1 2 3 4|4|0 1 2 3 4|0 1|1 2 3 4|0 1 2 3|3 4|0 1 2 3 4|0|0 1 2 3 4|0 1 2|   
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
         */
        while (len > 4) {
          tmp = *data;
          *data++ = ((tmp & 0xe0) >> 5) | ((tmp & 0x1f) << 3);
          tmp = *data;
          *data++ = ((tmp & 0x80) >> 7) |
              ((tmp & 0x7c) >> 2) | ((tmp & 0x03) << 6);
          tmp = *data;
          *data++ = ((tmp & 0xf0) >> 4) | ((tmp & 0x0f) << 4);
          tmp = *data;
          *data++ = ((tmp & 0xc0) >> 6) |
              ((tmp & 0x3e) << 2) | ((tmp & 0x01) << 7);
          tmp = *data;
          *data++ = ((tmp & 0xf8) >> 3) | ((tmp & 0x07) << 5);
          len -= 5;
        }
        break;
      }
    }
  }

  res =
      GST_BASE_RTP_PAYLOAD_CLASS (parent_class)->handle_buffer (payload,
      buffer);

  return res;
}

static void
gst_rtp_g726_pay_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstRtpG726Pay *rtpg726pay;

  rtpg726pay = GST_RTP_G726_PAY (object);

  switch (prop_id) {
    case PROP_FORCE_AAL2:
      rtpg726pay->force_aal2 = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_rtp_g726_pay_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstRtpG726Pay *rtpg726pay;

  rtpg726pay = GST_RTP_G726_PAY (object);

  switch (prop_id) {
    case PROP_FORCE_AAL2:
      g_value_set_boolean (value, rtpg726pay->force_aal2);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

gboolean
gst_rtp_g726_pay_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "rtpg726pay",
      GST_RANK_NONE, GST_TYPE_RTP_G726_PAY);
}
