/*
 * Farsight
 * GStreamer RTP Receive element using JRTPlib
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
#include <assert.h>

#include "gstrtprecv.h"

GST_DEBUG_CATEGORY (rtprecv_debug);
#define GST_CAT_DEFAULT (rtprecv_debug)

/* Filter signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_RTPSESSION,
  ARG_SILENT,
  ARG_PT_MAP
};

// takes in src info events interleaved with rtp packets
static GstStaticPadTemplate rtp_sink_factory =
GST_STATIC_PAD_TEMPLATE (
  "rtpsink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("application/x-rtp")
);

// takes in src info events interleaved with rtcp packets
static GstStaticPadTemplate rtcp_sink_factory =
GST_STATIC_PAD_TEMPLATE (
  "rtcpsink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("application/x-rtcp")
);

// gives out received RTP packets
static GstStaticPadTemplate data_src_factory =
GST_STATIC_PAD_TEMPLATE (
  "datasrc",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("ANY")
);

static void	gst_rtprecv_class_init	(GstRTPRecvClass *klass);
static void	gst_rtprecv_base_init	(GstRTPRecvClass *klass);
static void	gst_rtprecv_init	(GstRTPRecv *filter);
static void gst_rtprecv_finalize(GObject *object);

static GstFlowReturn gst_rtprecv_rtpsink_chain (GstPad *pad, GstBuffer *in);
static GstFlowReturn gst_rtprecv_rtcpsink_chain (GstPad *pad, GstBuffer *in);

static void	gst_rtprecv_set_property(GObject *object, guint prop_id,
        const GValue *value,
        GParamSpec *pspec);
static void	gst_rtprecv_get_property(GObject *object, guint prop_id,
        GValue *value,
        GParamSpec *pspec);

static GstElementClass *parent_class = NULL;

GType
gst_gst_rtprecv_get_type (void)
{
  static GType plugin_type = 0;

  if (!plugin_type)
  {
    static const GTypeInfo plugin_info =
    {
      sizeof (GstRTPRecvClass),
      (GBaseInitFunc) gst_rtprecv_base_init,
      NULL,
      (GClassInitFunc) gst_rtprecv_class_init,
      NULL,
      NULL,
      sizeof (GstRTPRecv),
      0,
      (GInstanceInitFunc) gst_rtprecv_init,
    };
    plugin_type = g_type_register_static (GST_TYPE_ELEMENT,
	                                  "GstRTPRecv",
	                                  &plugin_info, 0);
  }
  return plugin_type;
}

static void
gst_rtprecv_base_init (GstRTPRecvClass *klass)
{
  static GstElementDetails plugin_details = {
    "JRTP Session",
    "Manage/RTP",
    "RTP Send Element, all packets get processed through jrtplib",
    "Philippe Khalaf <burger@speedy.org>"
  };
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
          gst_static_pad_template_get (&rtp_sink_factory));
  gst_element_class_add_pad_template (element_class,
          gst_static_pad_template_get (&rtcp_sink_factory));
  gst_element_class_add_pad_template (element_class,
          gst_static_pad_template_get (&data_src_factory));
  gst_element_class_set_details (element_class, &plugin_details);
}

/* initialize the plugin's class */
static void
gst_rtprecv_class_init (GstRTPRecvClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*) klass;
  gstelement_class = (GstElementClass*) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  gobject_class->set_property = gst_rtprecv_set_property;
  gobject_class->get_property = gst_rtprecv_get_property;

  gobject_class->finalize = gst_rtprecv_finalize;
   
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_RTPSESSION,
          g_param_spec_pointer ("rtpsession_ptr", "RTPSession object pointer", 
              "A pointer to the RTPSession object created in the container",
              G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, ARG_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, ARG_PT_MAP,
      g_param_spec_pointer ("pt-map", "Payload-Type-map",
          "A Hash table, mapping payload-types to GstCaps object",
          G_PARAM_READWRITE));

  GST_DEBUG_CATEGORY_INIT (rtprecv_debug, "rtprecv", 0, "RTP Session");
}

/* initialize the new element
 * instantiate pads and add them to element
 * set functions
 * initialize structure
 */
static void
gst_rtprecv_init (GstRTPRecv *filter)
{
  GstElementClass *klass = GST_ELEMENT_GET_CLASS (filter);

  filter->datasrcpad = gst_pad_new_from_template (
          gst_element_class_get_pad_template (klass, "datasrc"), "datasrc");

  filter->rtpsinkpad = gst_pad_new_from_template (
          gst_element_class_get_pad_template (klass, "rtpsink"), "rtpsink");

  filter->rtcpsinkpad = gst_pad_new_from_template (
          gst_element_class_get_pad_template (klass, "rtcpsink"), "rtcpsink");

  gst_element_add_pad (GST_ELEMENT (filter), filter->datasrcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->rtpsinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->rtcpsinkpad);

  gst_pad_set_chain_function (filter->rtpsinkpad, gst_rtprecv_rtpsink_chain);
  gst_pad_set_chain_function (filter->rtcpsinkpad, gst_rtprecv_rtcpsink_chain);

  filter->silent = FALSE;

  filter->mutex = g_mutex_new();
  filter->pt_map_mutex = g_mutex_new();
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_rtprecv_rtpsink_chain (GstPad *pad, GstBuffer *in)
{
  GstRTPRecv *filter;
  GstBuffer *out_buf;
  static guint32 prev_ts = 0;
  GstCaps *caps;
  GstFlowReturn ret = GST_FLOW_OK;

  g_return_val_if_fail (GST_IS_PAD (pad), GST_FLOW_ERROR);
  g_return_val_if_fail (GST_BUFFER (in) != NULL, GST_FLOW_ERROR);

  filter = GST_RTPRECV (GST_OBJECT_PARENT (pad));
  g_return_val_if_fail (GST_IS_RTPRECV (filter), GST_FLOW_ERROR);
#ifdef HAVE_JRTP
  g_return_val_if_fail (filter->sess != NULL, GST_FLOW_ERROR);

  GST_DEBUG("1. Incoming RTP packet, giving to RTPSession for processing");
#else
  GST_DEBUG("1. Incoming RTP packet, sending without jrtplib");
#endif
  if (!gst_rtp_buffer_validate(in))
  {
    GST_DEBUG("Buffer not validated as RTP! skipping %d", GST_BUFFER_SIZE(in));
    //gst_util_dump_mem(GST_BUFFER_DATA(in), 16);
    gst_buffer_unref (in);
    return GST_FLOW_OK;
  }

  g_mutex_lock (filter->pt_map_mutex);
  if (filter->pt_map != NULL) {
    guint32 pt;
    pt = (guint32) gst_rtp_buffer_get_payload_type (in);
    caps = g_hash_table_lookup (filter->pt_map, GINT_TO_POINTER(pt));
    g_mutex_unlock (filter->pt_map_mutex);

    if (caps == NULL) {
      GST_DEBUG ("Caps unknown for payload-type %u, dropping buffer", pt);
      gst_buffer_unref (in);
      return GST_FLOW_OK;
    }
  }
  else
  {
    g_mutex_unlock (filter->pt_map_mutex);

    GST_DEBUG ("Caps<->payload-type mapping not provided, quitting");
    gst_buffer_unref (in);
    return GST_FLOW_ERROR;
  }

  out_buf = in;

  // let's give the buffer to jrtplib's gsttransmitter
#ifdef HAVE_JRTP
  g_mutex_lock (filter->mutex);
  if (!jrtpsession_setcurrentdata(filter->sess, (GstNetBuffer *)in, 1))
      return GST_FLOW_ERROR;

  // poll that same packet data, this will ensure the packet
  // goes through all the jrtplib processing
  jrtpsession_poll (filter->sess);
  g_mutex_unlock (filter->mutex);
  // the data is retreived and duplicated by poll
  // we can get rid of it now
  gst_buffer_unref (in);

  // get all packets in the queue
  while ((out_buf = jrtpsession_getpacket (filter->sess)) != NULL) {
    GST_DEBUG ("2. Incoming RTP packet return from jrtplib, pushing on rtprecv size %d seqnum %d tsdiff %u",
        GST_BUFFER_SIZE (out_buf), gst_rtp_buffer_get_seq (out_buf),
        gst_rtp_buffer_get_timestamp (out_buf) - prev_ts);
#endif

    gst_buffer_set_caps (out_buf, gst_caps_ref (caps));
    prev_ts = gst_rtp_buffer_get_timestamp (out_buf);

    ret = gst_pad_push (filter->datasrcpad, GST_BUFFER (out_buf));
#ifdef HAVE_JRTP
  }
#endif

  return ret;
}

static GstFlowReturn
gst_rtprecv_rtcpsink_chain (GstPad *pad, GstBuffer *in)
{
  GstRTPRecv *filter;

  g_return_val_if_fail (GST_IS_PAD (pad), GST_FLOW_ERROR);
  g_return_val_if_fail (GST_BUFFER (in) != NULL, GST_FLOW_ERROR);

  filter = GST_RTPRECV (GST_OBJECT_PARENT (pad));
  g_return_val_if_fail (GST_IS_RTPRECV (filter), GST_FLOW_ERROR);

#ifdef HAVE_JRTP
  g_return_val_if_fail (filter->sess != NULL, GST_FLOW_ERROR);

  GST_DEBUG("1. Incoming RTCP packet, giving to RTPSession");
  g_mutex_lock (filter->mutex);
  // let's give the buffer to jrtplib's gsttransmitter
  jrtpsession_setcurrentdata(filter->sess, (GstNetBuffer *)in, 0);

  // poll that same packet data, this will ensure the packet
  // goes through all the jrtplib processing
  jrtpsession_poll (filter->sess);
  g_mutex_unlock (filter->mutex);
#endif

  gst_buffer_unref (in);

  return GST_FLOW_OK;
}

static void
gst_rtprecv_set_property (GObject *object, guint prop_id,
                                  const GValue *value, GParamSpec *pspec)
{
  GstRTPRecv *filter;

  g_return_if_fail (GST_IS_RTPRECV (object));
  filter = GST_RTPRECV (object);

  switch (prop_id) {
    case ARG_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    case ARG_RTPSESSION:
#ifdef HAVE_JRTP
      filter->sess = g_value_get_pointer (value);
#endif
      break;
    case ARG_PT_MAP:
      g_mutex_lock (filter->pt_map_mutex);
      filter->pt_map = (GHashTable *) g_value_get_pointer (value);
      g_mutex_unlock (filter->pt_map_mutex);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_rtprecv_get_property (GObject *object, guint prop_id,
                                  GValue *value, GParamSpec *pspec)
{
  GstRTPRecv *filter;

  g_return_if_fail (GST_IS_RTPRECV (object));
  filter = GST_RTPRECV (object);

  switch (prop_id) {
    case ARG_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    case ARG_RTPSESSION:
#ifdef HAVE_JRTP
      g_value_set_pointer (value, filter->sess);
#endif
      break;
    case ARG_PT_MAP:
      g_mutex_lock (filter->pt_map_mutex);
      g_value_set_pointer (value, (gpointer) filter->pt_map);
      g_mutex_unlock (filter->pt_map_mutex);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_rtprecv_finalize(GObject *object)
{
	GstRTPRecv *filter = GST_RTPRECV(object);
	

   if (filter->mutex)
      g_mutex_free (filter->mutex);
   if (filter->pt_map_mutex)
      g_mutex_free (filter->pt_map_mutex);
   
	G_OBJECT_CLASS (parent_class)->finalize (object)	;
}
