/* GStreamer
 * Copyright (C) <2005> Wim Taymans <wim@fluendo.com>
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
 * SECTION:element-udpsink
 * @see_also: udpsrc, multifdsink
 *
 * udpsink is a network sink that sends UDP packets to the network.
 * It can be combined with RTP payloaders to implement RTP streaming.
 *
 * <refsect2>
 * <title>Examples</title>
 * |[
 * gst-launch -v audiotestsrc ! udpsink
 * ]|
 * </refsect2>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gstudpsink.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <string.h>

#define UDP_DEFAULT_HOST        "localhost"
#define UDP_DEFAULT_PORT        4951

/* elementfactory information */
static const GstElementDetails gst_udpsink_details =
GST_ELEMENT_DETAILS ("UDP packet sender",
    "Sink/Network",
    "Send data over the network via UDP",
    "Wim Taymans <wim@fluendo.com>");

/* UDPSink signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_HOST,
  PROP_PORT,
  PROP_URI,
  /* FILL ME */
};

static void gst_udpsink_base_init (gpointer g_class);
static void gst_udpsink_class_init (GstUDPSink * klass);
static void gst_udpsink_init (GstUDPSink * udpsink);
static void gst_udpsink_finalize (GstUDPSink * udpsink);

static void gst_udpsink_uri_handler_init (gpointer g_iface,
    gpointer iface_data);

static void gst_udpsink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_udpsink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static GstElementClass *parent_class = NULL;

/*static guint gst_udpsink_signals[LAST_SIGNAL] = { 0 }; */

GType
gst_udpsink_get_type (void)
{
  static GType udpsink_type = 0;

  if (!udpsink_type) {
    static const GTypeInfo udpsink_info = {
      sizeof (GstUDPSinkClass),
      gst_udpsink_base_init,
      NULL,
      (GClassInitFunc) gst_udpsink_class_init,
      NULL,
      NULL,
      sizeof (GstUDPSink),
      0,
      (GInstanceInitFunc) gst_udpsink_init,
      NULL
    };
    static const GInterfaceInfo urihandler_info = {
      gst_udpsink_uri_handler_init,
      NULL,
      NULL
    };

    udpsink_type =
        g_type_register_static (GST_TYPE_MULTIUDPSINK, "GstUDPSink",
        &udpsink_info, 0);

    g_type_add_interface_static (udpsink_type, GST_TYPE_URI_HANDLER,
        &urihandler_info);

  }
  return udpsink_type;
}

static void
gst_udpsink_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (element_class, &gst_udpsink_details);
}

static void
gst_udpsink_class_init (GstUDPSink * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSinkClass *gstbasesink_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasesink_class = (GstBaseSinkClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->set_property = gst_udpsink_set_property;
  gobject_class->get_property = gst_udpsink_get_property;

  gobject_class->finalize = (GObjectFinalizeFunc) gst_udpsink_finalize;

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_HOST,
      g_param_spec_string ("host", "host",
          "The host/IP/Multicast group to send the packets to",
          UDP_DEFAULT_HOST, G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_PORT,
      g_param_spec_int ("port", "port", "The port to send the packets to",
          0, 65535, UDP_DEFAULT_PORT, G_PARAM_READWRITE));
}


static void
gst_udpsink_init (GstUDPSink * udpsink)
{
  udpsink->host = g_strdup (UDP_DEFAULT_HOST);
  udpsink->port = UDP_DEFAULT_PORT;
  gst_multiudpsink_add (GST_MULTIUDPSINK (udpsink), udpsink->host,
      udpsink->port);
}

static void
gst_udpsink_finalize (GstUDPSink * udpsink)
{
  g_free (udpsink->host);
  g_free (udpsink->uri);

  G_OBJECT_CLASS (parent_class)->finalize ((GObject *) udpsink);
}

static void
gst_udpsink_update_uri (GstUDPSink * sink)
{
  g_free (sink->uri);
  sink->uri = g_strdup_printf ("udp://%s:%d", sink->host, sink->port);

  GST_DEBUG_OBJECT (sink, "updated uri to %s", sink->uri);
}

static gboolean
gst_udpsink_set_uri (GstUDPSink * sink, const gchar * uri)
{
  gchar *protocol;
  gchar *location;
  gchar *colptr;

  protocol = gst_uri_get_protocol (uri);
  if (strcmp (protocol, "udp") != 0)
    goto wrong_protocol;
  g_free (protocol);

  location = gst_uri_get_location (uri);
  if (!location)
    return FALSE;
  colptr = strstr (location, ":");

  gst_multiudpsink_remove (GST_MULTIUDPSINK (sink), sink->host, sink->port);

  if (colptr != NULL) {
    g_free (sink->host);
    sink->host = g_strndup (location, colptr - location);
    sink->port = atoi (colptr + 1);
  } else {
    g_free (sink->host);
    sink->host = g_strdup (location);
    sink->port = UDP_DEFAULT_PORT;
  }
  g_free (location);

  gst_multiudpsink_add (GST_MULTIUDPSINK (sink), sink->host, sink->port);

  gst_udpsink_update_uri (sink);

  return TRUE;

  /* ERRORS */
wrong_protocol:
  {
    g_free (protocol);
    GST_ELEMENT_ERROR (sink, RESOURCE, READ, (NULL),
        ("error parsing uri %s: wrong protocol (%s != udp)", uri, protocol));
    return FALSE;
  }
}

static void
gst_udpsink_set_property (GObject * object, guint prop_id, const GValue * value,
    GParamSpec * pspec)
{
  GstUDPSink *udpsink;

  udpsink = GST_UDPSINK (object);

  /* remove old host */
  gst_multiudpsink_remove (GST_MULTIUDPSINK (udpsink),
      udpsink->host, udpsink->port);

  switch (prop_id) {
    case PROP_HOST:
      g_free (udpsink->host);
      udpsink->host = g_value_dup_string (value);
      break;
    case PROP_PORT:
      udpsink->port = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
  /* add new host */
  gst_multiudpsink_add (GST_MULTIUDPSINK (udpsink),
      udpsink->host, udpsink->port);
}

static void
gst_udpsink_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstUDPSink *udpsink;

  udpsink = GST_UDPSINK (object);

  switch (prop_id) {
    case PROP_HOST:
      g_value_set_string (value, udpsink->host);
      break;
    case PROP_PORT:
      g_value_set_int (value, udpsink->port);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/*** GSTURIHANDLER INTERFACE *************************************************/

static GstURIType
gst_udpsink_uri_get_type (void)
{
  return GST_URI_SINK;
}

static gchar **
gst_udpsink_uri_get_protocols (void)
{
  static gchar *protocols[] = { "udp", NULL };

  return protocols;
}

static const gchar *
gst_udpsink_uri_get_uri (GstURIHandler * handler)
{
  GstUDPSink *sink = GST_UDPSINK (handler);

  return g_strdup (sink->uri);
}

static gboolean
gst_udpsink_uri_set_uri (GstURIHandler * handler, const gchar * uri)
{
  gboolean ret;
  GstUDPSink *sink = GST_UDPSINK (handler);

  ret = gst_udpsink_set_uri (sink, uri);

  return ret;
}

static void
gst_udpsink_uri_handler_init (gpointer g_iface, gpointer iface_data)
{
  GstURIHandlerInterface *iface = (GstURIHandlerInterface *) g_iface;

  iface->get_type = gst_udpsink_uri_get_type;
  iface->get_protocols = gst_udpsink_uri_get_protocols;
  iface->get_uri = gst_udpsink_uri_get_uri;
  iface->set_uri = gst_udpsink_uri_set_uri;
}
