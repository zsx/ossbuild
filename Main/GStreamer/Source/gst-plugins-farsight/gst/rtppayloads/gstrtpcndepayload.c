/* Farsight
 * Copyright (C) 2007 Nokia Corporation
 * Copyright (C) 2007 Collabora Ltd
 *  @author: Olivier Crete <olivier.crete@collabora.co.uk>
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

#include "gstrtpcndepayload.h"

#ifndef GST_RTP_PAYLOAD_CN_STRING
#define GST_RTP_PAYLOAD_CN_STRING "13"
#endif

#ifndef GST_RTP_PAYLOAD_CN
#define GST_RTP_PAYLOAD_CN 13
#endif

GST_DEBUG_CATEGORY (rtpcndepayload_debug);
#define GST_CAT_DEFAULT (rtpcndepayload_debug)

/* Elementfactory information */
static GstElementDetails gst_rtpcndepayload_details = {
  "RTP Confort Noise Depayloader",
  "Codec/Depayloader/Network",
  "Extracts CN from RTP packets",
  "Olivier Crete <olivier.crete@collabora.co.uk>"
};

static GstStaticPadTemplate gst_rtpcndepayload_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) \"audio\", "
        "payload = (int) " GST_RTP_PAYLOAD_CN_STRING ", "
        "clock-rate = (int) 8000, "
        "encoding-name = (string) \"CN\"; " 
        "application/x-rtp, "
        "media = (string) \"audio\", "
        "payload = (int) " GST_RTP_PAYLOAD_DYNAMIC_STRING ", "
        "clock-rate = (int) [ 1, MAX ], "
        "encoding-name = (string) \"CN\" ")
    );

static GstStaticPadTemplate gst_rtpcndepayload_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/CN, "
                     "rate = (int) [ 1, MAX ]")
    );

GST_BOILERPLATE (GstRTPCNDepayload, gst_rtpcndepayload, GstBaseRTPDepayload,
    GST_TYPE_BASE_RTP_DEPAYLOAD);


static gboolean gst_rtpcndepayload_setcaps (GstBaseRTPDepayload * payload,
    GstCaps * caps);
static GstBuffer *gst_rtpcndepayload_process (GstBaseRTPDepayload *depayload, GstBuffer * buf);

static void
gst_rtpcndepayload_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtpcndepayload_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtpcndepayload_src_template));
  gst_element_class_set_details (element_class, &gst_rtpcndepayload_details);
}

static void
gst_rtpcndepayload_class_init (GstRTPCNDepayloadClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseRTPDepayloadClass *gstbasertpdepayload_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasertpdepayload_class = (GstBaseRTPDepayloadClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_BASE_RTP_DEPAYLOAD);

  gstbasertpdepayload_class->set_caps = gst_rtpcndepayload_setcaps;
  gstbasertpdepayload_class->process = gst_rtpcndepayload_process;

  GST_DEBUG_CATEGORY_INIT (rtpcndepayload_debug, "rtpcndepay", 0, "CN RTP Depayloader");
}

static void
gst_rtpcndepayload_init (GstRTPCNDepayload * cndepayload, 
    GstRTPCNDepayloadClass * class)
{
  GstBaseRTPDepayload * depayload = GST_BASE_RTP_DEPAYLOAD (cndepayload);

  depayload->clock_rate = 8000;
}

static gboolean
gst_rtpcndepayload_setcaps (GstBaseRTPDepayload * depayload, GstCaps * caps)
{
  GstRTPCNDepayload *cndepayload;
  GstStructure *structure;
  GstCaps *srccaps;
  gint rate;
  gboolean ret;

  cndepayload = GST_RTPCNDEPAYLOAD (depayload);

  structure = gst_caps_get_structure (caps, 0);

  if (!gst_structure_get_int (structure, "clock-rate", &rate))
    rate = 8000;
  
  depayload->clock_rate = rate;

  srccaps = gst_caps_copy (gst_static_pad_template_get_caps
      (&gst_rtpcndepayload_src_template));
  structure = gst_caps_get_structure (srccaps, 0);
  gst_structure_set (structure, "rate", G_TYPE_INT, rate, NULL);

  ret = gst_pad_set_caps (GST_BASE_RTP_DEPAYLOAD_SRCPAD (depayload), srccaps);

  GST_DEBUG ("set caps on source: %" GST_PTR_FORMAT " (ret=%d)", srccaps, ret);

  gst_caps_unref (srccaps);

  return ret;
}


static GstBuffer *
gst_rtpcndepayload_process (GstBaseRTPDepayload *depayload, GstBuffer * buf)
{
  GstBuffer *outbuf = NULL;

  GST_DEBUG ("process : got %d bytes, mark %d ts %u seqn %d",
      GST_BUFFER_SIZE (buf),
      gst_rtp_buffer_get_marker (buf),
      gst_rtp_buffer_get_timestamp (buf), gst_rtp_buffer_get_seq (buf));

  outbuf = gst_rtp_buffer_get_payload_buffer (buf);

  return outbuf;
}


gboolean
gst_rtpcndepayload_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "rtpcndepay",
      GST_RANK_NONE, GST_TYPE_RTPCNDEPAYLOAD);
}
