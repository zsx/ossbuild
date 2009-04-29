/* Farsight
 * Copyright (C) <2005> Philippe Khalaf <burger@speedy.org> 
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

#include "gstmsgsmpayload.h"

GST_DEBUG_CATEGORY (msgsmpayload_debug);
#define GST_CAT_DEFAULT (msgsmpayload_debug)

/* elementfactory information */
static GstElementDetails gst_msgsmpayload_details = {
  "RTP MSGSM Audio Payloader",
  "Codec/Payloader/Network",
  "Payloads MSGSM audio into RTP packets",
  "Philippe Khalaf <burger@speedy.org>"
};

static GstStaticPadTemplate gst_msgsmpayload_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-msgsm, "
        "rate = (int) 8000")
    );

static GstStaticPadTemplate gst_msgsmpayload_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) \"audio\", "
        "payload = (int) 102, "
        "clock-rate = (int) 8000, "
        "encoding-name = (string) \"MSGSM\"")
    );


static void gst_msgsmpayload_class_init (GstMsGSMPayloadClass * klass);
static void gst_msgsmpayload_base_init (GstMsGSMPayloadClass * klass);
static void gst_msgsmpayload_init (GstMsGSMPayload * msgsmpayload);

static gboolean gst_msgsmpayload_setcaps (GstBaseRTPPayload * payload,
    GstCaps * caps);
static GstFlowReturn gst_msgsmpayload_handle_buffer (GstBaseRTPPayload * payload,
    GstBuffer * buffer);

static GstBaseRTPPayloadClass *parent_class = NULL;

static GType
gst_msgsmpayload_get_type (void)
{
  static GType msgsmpayload_type = 0;

  if (!msgsmpayload_type) {
    static const GTypeInfo msgsmpayload_info = {
      sizeof (GstMsGSMPayloadClass),
      (GBaseInitFunc) gst_msgsmpayload_base_init,
      NULL,
      (GClassInitFunc) gst_msgsmpayload_class_init,
      NULL,
      NULL,
      sizeof (GstMsGSMPayload),
      0,
      (GInstanceInitFunc) gst_msgsmpayload_init,
    };

    msgsmpayload_type =
        g_type_register_static (GST_TYPE_BASE_RTP_PAYLOAD, "GstMsGSMPayload",
        &msgsmpayload_info, 0);
  }
  return msgsmpayload_type;
}

static void
gst_msgsmpayload_base_init (GstMsGSMPayloadClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_msgsmpayload_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_msgsmpayload_src_template));
  gst_element_class_set_details (element_class, &gst_msgsmpayload_details);
}

static void
gst_msgsmpayload_class_init (GstMsGSMPayloadClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseRTPPayloadClass *gstbasertppayload_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasertppayload_class = (GstBaseRTPPayloadClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_BASE_RTP_PAYLOAD);

  gstbasertppayload_class->set_caps = gst_msgsmpayload_setcaps;
  gstbasertppayload_class->handle_buffer = gst_msgsmpayload_handle_buffer;

  GST_DEBUG_CATEGORY_INIT (msgsmpayload_debug, "msgsmpayload", 0, "MSGSM RTP Payloader");
}

static void
gst_msgsmpayload_init (GstMsGSMPayload * msgsmpayload)
{
}

static gboolean
gst_msgsmpayload_setcaps (GstBaseRTPPayload * payload, GstCaps * caps)
{
  GstMsGSMPayload *msgsmpayload;

  msgsmpayload = GST_RTPMSGSMPAYLOAD (payload);

  gst_basertppayload_set_options (payload, "audio", TRUE, "MSGSM", 8000);

  return TRUE;
}

static GstFlowReturn
gst_msgsmpayload_handle_buffer (GstBaseRTPPayload * basepayload,
    GstBuffer * buffer)
{
  GstMsGSMPayload *msgsmpayload;
  guint payload_len;
  GstBuffer *outbuf;
  guint8 *payload;
  GstFlowReturn ret = GST_FLOW_OK;

  msgsmpayload = GST_RTPMSGSMPAYLOAD (basepayload);

  GST_DEBUG("Payloader buffer handler called, got gst buffer with ts %" 
          GST_TIME_FORMAT, GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (buffer)));

  // Dosen't get more simple than this, all we need to do is pack our buffer
  // into an rtpbuffer
  payload_len = GST_BUFFER_SIZE (buffer);
  GST_DEBUG("Got frame, total size %d", payload_len);
  //gst_util_dump_mem (GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE(buffer));

  outbuf = gst_rtp_buffer_new_allocate (payload_len, 0, 0);
  GST_BUFFER_TIMESTAMP (outbuf) = GST_BUFFER_TIMESTAMP (buffer);
  /* FIXME, assert for now */
  g_assert (GST_BUFFER_SIZE (outbuf) < GST_BASE_RTP_PAYLOAD_MTU (msgsmpayload));

  /* get payload */
  payload = gst_rtp_buffer_get_payload (outbuf);
  /* copy data in payload */
  memcpy (&payload[0], GST_BUFFER_DATA(buffer), payload_len);
  GST_DEBUG ("pushing buffer of size %d", GST_BUFFER_SIZE (outbuf));

  ret = gst_basertppayload_push (basepayload, outbuf);
  gst_buffer_unref (buffer);

  return ret;
}

gboolean
gst_msgsmpayload_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "msgsmpayloader",
      GST_RANK_NONE, GST_TYPE_RTPMSGSMPAYLOAD);
}
