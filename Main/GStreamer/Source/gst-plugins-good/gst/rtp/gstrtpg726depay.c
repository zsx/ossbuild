/* GStreamer
 * Copyright (C) 1999 Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) 2005 Edgard Lima <edgard.lima@indt.org.br>
 * Copyright (C) 2005 Zeeshan Ali <zeenix@gmail.com>
 * Copyright (C) 2008 Axis Communications <dev-gstreamer@axis.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gst/rtp/gstrtpbuffer.h>

#include "gstrtpg726depay.h"

#define DEFAULT_BIT_RATE 32000
#define SAMPLE_RATE 8000
#define LAYOUT_G726 "g726"

/* elementfactory information */
static const GstElementDetails gst_rtp_g726depay_details =
GST_ELEMENT_DETAILS ("RTP G.726 depayloader",
    "Codec/Depayloader/Network",
    "Extracts G.726 audio from RTP packets",
    "Axis Communications <dev-gstreamer@axis.com>");

/* RtpG726Depay signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  ARG_0
};

static GstStaticPadTemplate gst_rtp_g726_depay_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) \"audio\", "
        "payload = (int) " GST_RTP_PAYLOAD_DYNAMIC_STRING ", "
        "encoding-name = (string) { \"G726\", \"G726-16\", \"G726-24\", \"G726-32\", \"G726-40\"}, "
        "clock-rate = (int) 8000;")
    );

static GstStaticPadTemplate gst_rtp_g726_depay_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-adpcm, "
        "channels = (int) 1, "
        "rate = (int) 8000, "
        "bitrate = (int) { 16000, 24000, 32000, 40000 }, "
        "layout = (string) \"g726\"")
    );

static GstBuffer *gst_rtp_g726_depay_process (GstBaseRTPDepayload * depayload,
    GstBuffer * buf);
static gboolean gst_rtp_g726_depay_setcaps (GstBaseRTPDepayload * depayload,
    GstCaps * caps);

GST_BOILERPLATE (GstRtpG726Depay, gst_rtp_g726_depay, GstBaseRTPDepayload,
    GST_TYPE_BASE_RTP_DEPAYLOAD);

static void
gst_rtp_g726_depay_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_g726_depay_src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_g726_depay_sink_template));
  gst_element_class_set_details (element_class, &gst_rtp_g726depay_details);
}

static void
gst_rtp_g726_depay_class_init (GstRtpG726DepayClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseRTPDepayloadClass *gstbasertpdepayload_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasertpdepayload_class = (GstBaseRTPDepayloadClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gstbasertpdepayload_class->process = gst_rtp_g726_depay_process;
  gstbasertpdepayload_class->set_caps = gst_rtp_g726_depay_setcaps;
}

static void
gst_rtp_g726_depay_init (GstRtpG726Depay * rtpG726depay,
    GstRtpG726DepayClass * klass)
{
  GstBaseRTPDepayload *depayload;

  depayload = GST_BASE_RTP_DEPAYLOAD (rtpG726depay);

  gst_pad_use_fixed_caps (GST_BASE_RTP_DEPAYLOAD_SRCPAD (depayload));
}

static gboolean
gst_rtp_g726_depay_setcaps (GstBaseRTPDepayload * depayload, GstCaps * caps)
{
  GstCaps *srccaps;
  GstStructure *structure;
  gboolean ret;
  gint clock_rate;
  const gchar *encoding_name;
  gint bitrate;

  structure = gst_caps_get_structure (caps, 0);

  if (!gst_structure_get_int (structure, "clock-rate", &clock_rate))
    clock_rate = 8000;          /* default */
  depayload->clock_rate = clock_rate;

  encoding_name = gst_structure_get_string (structure, "encoding-name");
  if (encoding_name == NULL || g_ascii_strcasecmp (encoding_name, "G726") == 0) {
    bitrate = DEFAULT_BIT_RATE;
  } else if (g_ascii_strcasecmp (encoding_name, "G726-16") == 0) {
    bitrate = 16000;
  } else if (g_ascii_strcasecmp (encoding_name, "G726-24") == 0) {
    bitrate = 24000;
  } else if (g_ascii_strcasecmp (encoding_name, "G726-32") == 0) {
    bitrate = 32000;
  } else if (g_ascii_strcasecmp (encoding_name, "G726-40") == 0) {
    bitrate = 40000;
  } else {
    GST_WARNING ("Could not determine bitrate from encoding-name (%s)",
        encoding_name);
    ret = FALSE;
    goto done;
  }
  GST_DEBUG ("RTP G.726 depayloader, bitrate set to %d\n", bitrate);

  srccaps = gst_caps_new_simple ("audio/x-adpcm",
      "channels", G_TYPE_INT, 1,
      "rate", G_TYPE_INT, clock_rate,
      "bitrate", G_TYPE_INT, bitrate,
      "layout", G_TYPE_STRING, LAYOUT_G726, NULL);

  ret = gst_pad_set_caps (GST_BASE_RTP_DEPAYLOAD_SRCPAD (depayload), srccaps);
  gst_caps_unref (srccaps);

done:
  return ret;
}


static GstBuffer *
gst_rtp_g726_depay_process (GstBaseRTPDepayload * depayload, GstBuffer * buf)
{
  GstBuffer *outbuf = NULL;
  gboolean marker;

  marker = gst_rtp_buffer_get_marker (buf);

  GST_DEBUG ("process : got %d bytes, mark %d ts %u seqn %d",
      GST_BUFFER_SIZE (buf), marker,
      gst_rtp_buffer_get_timestamp (buf), gst_rtp_buffer_get_seq (buf));

  outbuf = gst_rtp_buffer_get_payload_buffer (buf);

  if (marker) {
    /* mark start of talkspurt with discont */
    GST_BUFFER_FLAG_SET (outbuf, GST_BUFFER_FLAG_DISCONT);
  }

  return outbuf;
}

gboolean
gst_rtp_g726_depay_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "rtpg726depay",
      GST_RANK_MARGINAL, GST_TYPE_RTP_G726_DEPAY);
}
