/*
 * Farsight
 * GStreamer RTP Send element using JRTPlib
 * Copyright (C) 2005 Philippe Khalaf <burger@speedy.org>
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

#include <gst/gst.h>
#include <gst/rtp/gstrtpbuffer.h>

#ifndef HAVE_JRTP
#include <gst/netbuffer/gstnetbuffer.h>
#endif

#include "gstrtpsend.h"
#include "gstrtpbin.h"

GST_DEBUG_CATEGORY (rtpsend_debug);
#define GST_CAT_DEFAULT (rtpsend_debug)

/* Filter signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_RTPSESSION,
  ARG_SILENT
};

// takes in GstRTPBuffers to send out
static GstStaticPadTemplate data_sink_factory =
GST_STATIC_PAD_TEMPLATE (
  "datasink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS("application/x-rtp, "
      "clock-rate = (int) [ 1, 2147483647 ]")
);

// gives out dest info events interleaved with rtp packets
static GstStaticPadTemplate rtp_src_factory =
GST_STATIC_PAD_TEMPLATE (
  "rtpsrc",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS("application/x-rtp")
);

// gives out dest info events interleaved with rtcp packets
static GstStaticPadTemplate rtcp_src_factory =
GST_STATIC_PAD_TEMPLATE (
  "rtcpsrc",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS("application/x-rtcp")
);

static void	gst_rtpsend_class_init	(GstRTPSendClass *klass);
static void	gst_rtpsend_base_init	(GstRTPSendClass *klass);
static void	gst_rtpsend_init	(GstRTPSend *filter);

static void	gst_rtpsend_set_property(GObject *object, guint prop_id,
        const GValue *value,
        GParamSpec *pspec);
static void	gst_rtpsend_get_property(GObject *object, guint prop_id,
        GValue *value,
        GParamSpec *pspec);

static GstFlowReturn gst_rtpsend_datasink_chain (GstPad *pad, GstBuffer *in);
static GstPadLinkReturn gst_rtpsend_setcaps_func (GstPad * pad, GstCaps * caps);

static GstElementClass *parent_class = NULL;

GType
gst_gst_rtpsend_get_type (void)
{
  static GType plugin_type = 0;

  if (!plugin_type)
  {
    static const GTypeInfo plugin_info =
    {
      sizeof (GstRTPSendClass),
      (GBaseInitFunc) gst_rtpsend_base_init,
      NULL,
      (GClassInitFunc) gst_rtpsend_class_init,
      NULL,
      NULL,
      sizeof (GstRTPSend),
      0,
      (GInstanceInitFunc) gst_rtpsend_init,
    };
    plugin_type = g_type_register_static (GST_TYPE_ELEMENT,
	                                  "GstRTPSend",
	                                  &plugin_info, 0);
  }
  return plugin_type;
}

static void
gst_rtpsend_base_init (GstRTPSendClass *klass)
{
  static GstElementDetails plugin_details = {
    "JRTP Session",
    "Manage/RTP",
    "RTP Send Element, all packets get processed through jrtplib",
    "Philippe Khalaf <burger@speedy.org>"
  };
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
          gst_static_pad_template_get (&rtp_src_factory));
  gst_element_class_add_pad_template (element_class,
          gst_static_pad_template_get (&rtcp_src_factory));
  gst_element_class_add_pad_template (element_class,
          gst_static_pad_template_get (&data_sink_factory));
  gst_element_class_set_details (element_class, &plugin_details);
}

/* initialize the plugin's class */
static void
gst_rtpsend_class_init (GstRTPSendClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*) klass;
  gstelement_class = (GstElementClass*) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  gobject_class->set_property = gst_rtpsend_set_property;
  gobject_class->get_property = gst_rtpsend_get_property;

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_RTPSESSION,
          g_param_spec_pointer ("rtpsession_ptr", "RTPSession object pointer", 
              "A pointer to the RTPSession object created in the container",
              G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, ARG_SILENT,
          g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
              FALSE, G_PARAM_READWRITE));
 
  GST_DEBUG_CATEGORY_INIT (rtpsend_debug, "rtpsend", 0, "RTP Session");
}

/* initialize the new element
 * instantiate pads and add them to element
 * set functions
 * initialize structure
 */
static void
gst_rtpsend_init (GstRTPSend *filter)
{
  GstElementClass *klass = GST_ELEMENT_GET_CLASS (filter);

  filter->datasinkpad = gst_pad_new_from_template (
          gst_element_class_get_pad_template (klass, "datasink"), "datasink");

  filter->rtpsrcpad = gst_pad_new_from_template (
          gst_element_class_get_pad_template (klass, "rtpsrc"), "rtpsrc");

  filter->rtcpsrcpad = gst_pad_new_from_template (
          gst_element_class_get_pad_template (klass, "rtcpsrc"), "rtcpsrc");

  gst_element_add_pad (GST_ELEMENT (filter), filter->datasinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->rtpsrcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->rtcpsrcpad);

  gst_pad_set_chain_function (filter->datasinkpad, gst_rtpsend_datasink_chain);
  gst_pad_set_setcaps_function (filter->datasinkpad, gst_rtpsend_setcaps_func);

  filter->silent = FALSE;

  filter->prev_ts = 0;
}

static gboolean 
gst_rtpsend_setcaps_func (GstPad * pad, GstCaps * caps)
{
  GstRTPSend *filter;
  GstStructure *structure;
  gboolean ret;
  gint clock_rate;

  filter = GST_RTPSEND (gst_pad_get_parent (pad));
  structure = gst_caps_get_structure (caps, 0);

  ret = gst_structure_get_int (structure, "clock-rate", (gint *)&clock_rate);

  if (!ret) {
     gst_object_unref(filter);   
     return FALSE;
  }

#ifdef HAVE_JRTP
  /* Set timestamp unit now */
  jrtpsession_settimestampunit (filter->sess, (gdouble)(1/clock_rate));
#endif
  gst_object_unref(filter);
  return TRUE;
}

static GstFlowReturn 
gst_rtpsend_datasink_chain (GstPad *pad, GstBuffer *buf)
{
  GstRTPSend *filter;

  g_return_val_if_fail (GST_IS_PAD (pad), GST_FLOW_ERROR);
  g_return_val_if_fail (buf != NULL, GST_FLOW_ERROR);

  filter = GST_RTPSEND (GST_OBJECT_PARENT (pad));
  g_return_val_if_fail (GST_IS_RTPSEND (filter), GST_FLOW_ERROR);

#ifdef HAVE_JRTP
  g_return_val_if_fail (filter->sess != NULL, GST_FLOW_ERROR);

  // let us poll, this will allow for RTCP packets to be sent at the
  // appropriate time
  jrtpsession_poll (filter->sess);
#endif

  if (GST_BUFFER_DATA (buf))
  {
#ifdef HAVE_JRTP
      if (gst_rtp_buffer_validate (buf))
      {
          if (filter->prev_ts != 0)
          {
              guint timestampinc = gst_rtp_buffer_get_timestamp (buf) -
                      filter->prev_ts;

              jrtpsession_incrementtimestamp(filter->sess, timestampinc);
              GST_DEBUG_OBJECT(filter, "Timestamp incremented by %u",
                      timestampinc);
          }
          GST_DEBUG_OBJECT(filter, "1. Outgoing RTP packet received, giving to jrtplib buffer size %d pt %d mark %d ts %u", 
                  GST_BUFFER_SIZE (buf), gst_rtp_buffer_get_payload_type (buf),
                  gst_rtp_buffer_get_marker (buf), gst_rtp_buffer_get_timestamp (buf));
          jrtpsession_sendpacket (filter->sess, 
                  gst_rtp_buffer_get_payload (buf),
                  gst_rtp_buffer_get_payload_len (buf),
                  gst_rtp_buffer_get_payload_type (buf),
                  gst_rtp_buffer_get_marker (buf),
                  0);
          filter->prev_ts = gst_rtp_buffer_get_timestamp (buf);
      }
      else
      {
          GST_DEBUG_OBJECT (filter, "1. Outgoing RTP packet received, passing non rtp buffer to jrtplib for sending, using defaults");
          jrtpsession_sendpacket_default(filter->sess, GST_BUFFER_DATA (buf),
                  GST_BUFFER_SIZE (buf));
      }
      //GST_DEBUG_OBJECT (filter, "Finished Sending rtp packet %d bytes %p",
      //        GST_BUFFER_SIZE (buf), filter);
      //gst_util_dump_mem (GST_BUFFER_DATA(buf), 16);
      gst_buffer_unref (buf);
#else
      /* We send without going through jrtplib, we have to make sure it's a
       * valid RTP buffer */
      if (gst_rtp_buffer_validate (buf))
      {
          GstNetBuffer *out_buf;
          guint32 dest_ip;
          guint16 dest_port;
          dest_ip = GST_RTP_BIN(GST_ELEMENT_PARENT(filter))->dest_ip;
          dest_port = GST_RTP_BIN(GST_ELEMENT_PARENT(filter))->dest_port;
          if (dest_ip == 0 || dest_port == 0)
          {
              GST_WARNING_OBJECT(filter, "Destination not set! Don't know where to send! skipping");
              return GST_FLOW_OK;
          }
          out_buf = gst_netbuffer_new ();
          GST_BUFFER_DATA (out_buf) = GST_BUFFER_DATA(buf);
          GST_BUFFER_MALLOCDATA(out_buf) = GST_BUFFER_MALLOCDATA(buf);
          GST_BUFFER_SIZE (out_buf) = GST_BUFFER_SIZE(buf);
          buf = gst_buffer_make_metadata_writable (buf);
          GST_BUFFER_MALLOCDATA(buf) = NULL;
          gst_netaddress_set_ip4_address (&out_buf->to,
                  dest_ip, dest_port);
          GST_DEBUG_OBJECT(filter, "1. Outgoing RTP packet going straight out without jrtplib %p %d", 
                  GST_BUFFER_DATA(out_buf), GST_BUFFER_SIZE(out_buf));

          // push data
          gst_pad_push (filter->rtpsrcpad, GST_BUFFER(out_buf));
      }
      gst_buffer_unref (buf);

#endif

      return GST_FLOW_OK;
  }
  return GST_FLOW_ERROR;
}

static void
gst_rtpsend_set_property (GObject *object, guint prop_id,
                                  const GValue *value, GParamSpec *pspec)
{
  GstRTPSend *filter;

  g_return_if_fail (GST_IS_RTPSEND (object));
  filter = GST_RTPSEND (object);

  switch (prop_id)
  {
      case ARG_SILENT:
          filter->silent = g_value_get_boolean (value);
          break;
      case ARG_RTPSESSION:
#ifdef HAVE_JRTP
          filter->sess = g_value_get_pointer (value);
#endif
          break;
      default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
          break;
  }
}

static void
gst_rtpsend_get_property (GObject *object, guint prop_id,
                                  GValue *value, GParamSpec *pspec)
{
  GstRTPSend *filter;

  g_return_if_fail (GST_IS_RTPSEND (object));
  filter = GST_RTPSEND (object);

  switch (prop_id) {
      case ARG_SILENT:
          g_value_set_boolean (value, filter->silent);
          break;
      case ARG_RTPSESSION:
#ifdef HAVE_JRTP
          g_value_set_pointer (value, filter->sess);
#endif
          break;
      default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
          break;
  }
}
