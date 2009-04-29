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

#include "gstrtpcnpayload.h"

#ifndef GST_RTP_PAYLOAD_CN_STRING
#define GST_RTP_PAYLOAD_CN_STRING "13"
#endif

#ifndef GST_RTP_PAYLOAD_CN
#define GST_RTP_PAYLOAD_CN 13
#endif

GST_DEBUG_CATEGORY (rtpcnpayload_debug);
#define GST_CAT_DEFAULT (rtpcnpayload_debug)

/* Elementfactory information */
static GstElementDetails gst_rtpcnpayload_details = {
  "RTP Confort Noise Payloader",
  "Codec/Payloader/Network",
  "Payloads CN into RTP packets",
  "Olivier Crete <olivier.crete@collabora.co.uk>"
};

static GstStaticPadTemplate gst_rtpcnpayload_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/CN, "
                     "rate = (int) [ 1, MAX ]")
    );

static GstStaticPadTemplate gst_rtpcnpayload_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
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

GST_BOILERPLATE (GstRTPCNPayload, gst_rtpcnpayload, GstBaseRTPPayload,
    GST_TYPE_BASE_RTP_PAYLOAD);


static gboolean gst_rtpcnpayload_setcaps (GstBaseRTPPayload * payload,
    GstCaps * caps);
static GstFlowReturn gst_rtpcnpayload_handle_buffer (GstBaseRTPPayload * payload,
    GstBuffer * buffer);

static void
gst_rtpcnpayload_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtpcnpayload_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtpcnpayload_src_template));
  gst_element_class_set_details (element_class, &gst_rtpcnpayload_details);
}

static void
gst_rtpcnpayload_class_init (GstRTPCNPayloadClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseRTPPayloadClass *gstbasertppayload_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasertppayload_class = (GstBaseRTPPayloadClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_BASE_RTP_PAYLOAD);

  gstbasertppayload_class->set_caps = gst_rtpcnpayload_setcaps;
  gstbasertppayload_class->handle_buffer = gst_rtpcnpayload_handle_buffer;

  GST_DEBUG_CATEGORY_INIT (rtpcnpayload_debug, "rtpcnpay", 0, "CN RTP Payloader");
}

static void
gst_rtpcnpayload_init (GstRTPCNPayload * cnpayload, 
    GstRTPCNPayloadClass * class)
{
  GstBaseRTPPayload * payload = GST_BASE_RTP_PAYLOAD (cnpayload);

  payload->pt = GST_RTP_PAYLOAD_CN;
}

static gboolean
gst_rtpcnpayload_setcaps (GstBaseRTPPayload * payload, GstCaps * caps)
{
  GstRTPCNPayload *cnpayload;
  GstStructure *structure;
  gint rate;

  cnpayload = GST_RTPCNPAYLOAD (payload);

  structure = gst_caps_get_structure (caps, 0);

  if (!gst_structure_get_int (structure, "rate", &rate))
    rate = 8000;
  
  if (payload->pt == GST_RTP_PAYLOAD_CN && rate != 8000) {
    GST_WARNING_OBJECT (cnpayload, 
        "Trying to set PT 13 with non-8000 rate (%d\n)", rate);
    return FALSE;
  }

  gst_basertppayload_set_options (payload, "audio", 
      payload->pt != GST_RTP_PAYLOAD_CN , "CN", rate);
  
  gst_basertppayload_set_outcaps (payload, NULL);

  return TRUE;
}

/* Get a CN frame... move it in a rtp buffer... ship it!
 */
static GstFlowReturn
gst_rtpcnpayload_handle_buffer (GstBaseRTPPayload * basepayload,
    GstBuffer * buffer)
{
  GstBuffer *outbuf;
  GstFlowReturn ret = GST_FLOW_OK;

  outbuf =  gst_rtp_buffer_new_allocate (GST_BUFFER_SIZE (buffer), 0, 0);

  GST_BUFFER_TIMESTAMP (outbuf) = GST_BUFFER_TIMESTAMP (buffer);
  GST_BUFFER_DURATION (outbuf) = GST_BUFFER_DURATION (buffer);
  GST_BUFFER_FLAGS (outbuf) = GST_BUFFER_FLAGS (buffer) &
      (GST_BUFFER_FLAG_DISCONT | GST_BUFFER_FLAG_DELTA_UNIT);

  memcpy(gst_rtp_buffer_get_payload(outbuf), GST_BUFFER_DATA(buffer), 
      GST_BUFFER_SIZE (buffer));

  
  ret = gst_basertppayload_push (basepayload, outbuf);
  gst_buffer_unref (buffer);
  return ret;
}

gboolean
gst_rtpcnpayload_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "rtpcnpay",
      GST_RANK_NONE, GST_TYPE_RTPCNPAYLOAD);
}
