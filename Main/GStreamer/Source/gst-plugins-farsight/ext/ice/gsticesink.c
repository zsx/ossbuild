/*
 * GStreamer
 * Farsight Voice+Video library
 *  Copyright 2006 Collabora Ltd, 
 *  Copyright 2006 Nokia Corporation
 *   @author: Philippe Khalaf <philippe.khalaf@collabora.co.uk>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gsticesink.h"
#include "jingle_c.h"

GST_DEBUG_CATEGORY (icesink_debug);
#define GST_CAT_DEFAULT (icesink_debug)

static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

static GstElementDetails gst_icesink_details =
GST_ELEMENT_DETAILS ("ICE packet sink",
    "Sink/Network",
    "Send data over the network via libjingle",
    "Philippe Kalaf <philippe.kalaf@collabora.co.uk>");

enum
{
  PROP_0,
  PROP_SOCKETCLIENT
  /* FILL ME */
};


static GstFlowReturn gst_icesink_render (GstBaseSink * bsink, GstBuffer *
        buffer);
static void gst_icesink_set_property (GObject * object, guint prop_id,
        const GValue * value, GParamSpec * pspec);
static void gst_icesink_get_property (GObject * object, guint prop_id,
        GValue * value, GParamSpec * pspec);

static void
_do_init (GType type)
{
  GST_DEBUG_CATEGORY_INIT (icesink_debug, "icesink", 0, "ICE sink");
}

GST_BOILERPLATE_FULL (GstIceSink, gst_icesink, GstBaseSink, GST_TYPE_BASE_SINK,
    _do_init);

static void
gst_icesink_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_template));

  gst_element_class_set_details (element_class, &gst_icesink_details);
}

static void
gst_icesink_class_init (GstIceSinkClass * klass)
{
  GObjectClass *gobject_class;
  GstBaseSinkClass *gstbasesink_class;

  gobject_class = (GObjectClass *) klass;
  gstbasesink_class = (GstBaseSinkClass *) klass;

  gobject_class->set_property = gst_icesink_set_property;
  gobject_class->get_property = gst_icesink_get_property;

  g_object_class_install_property (gobject_class, PROP_SOCKETCLIENT,
      g_param_spec_pointer ("socketclient", "socketclient pointer",
          "A pointer to the SocketClient object define in farsight rtp",
          G_PARAM_READWRITE));

  gstbasesink_class->render = gst_icesink_render;
}

static void
gst_icesink_init (GstIceSink * icesink, GstIceSinkClass * g_class)
{
    GST_DEBUG_OBJECT (icesink, "initialising %p sockclient %p", icesink, icesink->sockclient);
}

static GstFlowReturn
gst_icesink_render (GstBaseSink * bsink, GstBuffer * buffer)
{
    GstIceSink *sink = NULL;

    sink = GST_ICESINK(bsink);

    if (!GST_IS_NETBUFFER (buffer)) {
        GST_DEBUG_OBJECT (sink, "Received buffer is not a GstNetBuffer, skipping");
        return GST_FLOW_OK;
    }

    if (sink->sockclient)
    {
      if (GST_BUFFER_SIZE (buffer))
      {
        GST_DEBUG_OBJECT (sink, "sending from icesink %p %p", sink,
            g_thread_self());
        socketclient_send_packet(sink->sockclient, 
            (const gchar *)GST_BUFFER_DATA(buffer), 
            GST_BUFFER_SIZE(buffer));
      }
    }
    else
        GST_DEBUG_OBJECT (sink, "sockclient pointer not set!");

    return GST_FLOW_OK;
}

static void
gst_icesink_set_property (GObject * object, guint prop_id, const GValue * value,
    GParamSpec * pspec)
{
  GstIceSink *icesink = GST_ICESINK (object);

  switch (prop_id) {
    case PROP_SOCKETCLIENT:
      icesink->sockclient = g_value_get_pointer (value);
      break;
    default:
      break;
  }
}

static void
gst_icesink_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstIceSink *icesink = GST_ICESINK (object);

  switch (prop_id) {
    case PROP_SOCKETCLIENT:
      g_value_set_pointer (value, icesink->sockclient);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}
