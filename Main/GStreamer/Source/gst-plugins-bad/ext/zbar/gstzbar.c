/* GStreamer
 * Copyright (C) 2009 Stefan Kost <ensonic@users.sf.net>
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

/*
 * This file was (probably) generated from
 * gstvideotemplate.c,v 1.12 2004/01/07 21:07:12 ds Exp 
 * and
 * make_filter,v 1.6 2004/01/07 21:33:01 ds Exp 
 */

/**
 * SECTION:element-zbar
 *
 * Detect bar codes in the video streams and send them as element messages to
 * the #GstBus if .#GstZBar:message property is %TRUE.
 *
 * The element generate messages named
 * <classname>&quot;barcode&quot;</classname>. The structure containes these
 * fields:
 * <itemizedlist>
 * <listitem>
 *   <para>
 *   #GstClockTime
 *   <classname>&quot;timestamp&quot;</classname>:
 *   the timestamp of the buffer that triggered the message.
 *   </para>
 * </listitem>
 * <listitem>
 *   <para>
 *   gchar*
 *   <classname>&quot;type&quot;</classname>:
 *   the symbol type.
 *   </para>
 * </listitem>
 * <listitem>
 *   <para>
 *   gchar*
 *   <classname>&quot;symbol&quot;</classname>:
 *   the deteted bar code data.
 *   </para>
 * </listitem>
 * <listitem>
 *   <para>
 *   gint
 *   <classname>&quot;quality&quot;</classname>:
 *   an unscaled, relative quantity: larger values are better than smaller
 *   values.
 *   </para>
 * </listitem>
 * </itemizedlist>
 *
 * <refsect2>
 * <title>Example launch lines</title>
 * |[
 * gst-launch -m v4l2src ! ffmpegcolorspace ! zbar ! ffmpegcolorspace ! xvimagesink
 * ]| This pipeline will detect barcodes and send them as messages.
 * |[
 * gst-launch -m v4l2src ! tee name=t ! queue ! ffmpegcolorspace ! zbar ! fakesink t. ! queue ! xvimagesink
 * ]| Same as above, but running the filter on a branch to keep the display in color
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstzbar.h"

#include <string.h>
#include <math.h>

#include <gst/video/video.h>


GST_DEBUG_CATEGORY_STATIC (zbar_debug);
#define GST_CAT_DEFAULT zbar_debug

/* GstZBar signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_MESSAGE,
};

#define DEFAULT_PROP_ZBAR  1

static const GstElementDetails zbar_details =
GST_ELEMENT_DETAILS ("Barcode detector",
    "Filter/Analyzer/Video",
    "Detect bar codes in the video streams",
    "Stefan Kost <ensonic@users.sf.net>");

static GstStaticPadTemplate gst_zbar_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_YUV ("{ Y800 }"))
    );

static GstStaticPadTemplate gst_zbar_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_YUV ("{ Y800 }"))
    );

static void gst_zbar_finalize (GObject * object);
static void gst_zbar_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_zbar_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_zbar_set_caps (GstBaseTransform * base, GstCaps * incaps,
    GstCaps * outcaps);
static GstFlowReturn gst_zbar_transform_ip (GstBaseTransform * transform,
    GstBuffer * buf);
static gboolean gst_zbar_start (GstBaseTransform * base);
static gboolean gst_zbar_stop (GstBaseTransform * base);

GST_BOILERPLATE (GstZBar, gst_zbar, GstVideoFilter, GST_TYPE_VIDEO_FILTER);


static void
gst_zbar_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (element_class, &zbar_details);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_zbar_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_zbar_src_template));
}

static void
gst_zbar_class_init (GstZBarClass * g_class)
{
  GObjectClass *gobject_class;
  GstBaseTransformClass *trans_class;

  gobject_class = G_OBJECT_CLASS (g_class);
  trans_class = GST_BASE_TRANSFORM_CLASS (g_class);

  gobject_class->set_property = gst_zbar_set_property;
  gobject_class->get_property = gst_zbar_get_property;
  gobject_class->finalize = gst_zbar_finalize;

  g_object_class_install_property (gobject_class, PROP_MESSAGE,
      g_param_spec_boolean ("message", "mesage",
          "Post a barcode message for each detected code",
          TRUE, G_PARAM_READWRITE));

  trans_class->set_caps = GST_DEBUG_FUNCPTR (gst_zbar_set_caps);
  trans_class->transform_ip = GST_DEBUG_FUNCPTR (gst_zbar_transform_ip);
  trans_class->start = GST_DEBUG_FUNCPTR (gst_zbar_start);
  trans_class->stop = GST_DEBUG_FUNCPTR (gst_zbar_stop);
}

static void
gst_zbar_init (GstZBar * zbar, GstZBarClass * g_class)
{
  zbar->message = TRUE;

  zbar->scanner = zbar_image_scanner_create ();
}

static void
gst_zbar_finalize (GObject * object)
{
  GstZBar *zbar = GST_ZBAR (object);

  zbar_image_scanner_destroy (zbar->scanner);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_zbar_set_property (GObject * object, guint prop_id, const GValue * value,
    GParamSpec * pspec)
{
  GstZBar *zbar;

  g_return_if_fail (GST_IS_ZBAR (object));
  zbar = GST_ZBAR (object);

  switch (prop_id) {
    case PROP_MESSAGE:
      zbar->message = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_zbar_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstZBar *zbar;

  g_return_if_fail (GST_IS_ZBAR (object));
  zbar = GST_ZBAR (object);

  switch (prop_id) {
    case PROP_MESSAGE:
      g_value_set_boolean (value, zbar->message);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_zbar_set_caps (GstBaseTransform * base, GstCaps * incaps, GstCaps * outcaps)
{
  GstZBar *zbar = GST_ZBAR (base);
  GstStructure *structure;
  gboolean res;

  GST_DEBUG_OBJECT (zbar,
      "set_caps: in %" GST_PTR_FORMAT " out %" GST_PTR_FORMAT, incaps, outcaps);

  structure = gst_caps_get_structure (incaps, 0);

  res = gst_structure_get_int (structure, "width", &zbar->width);
  res &= gst_structure_get_int (structure, "height", &zbar->height);

  return res;
}

static GstFlowReturn
gst_zbar_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  GstZBar *zbar = GST_ZBAR (base);
  guint8 *data;
  guint size;
  zbar_image_t *image;
  const zbar_symbol_t *symbol;
  int n;

  if (base->passthrough)
    goto done;

  data = GST_BUFFER_DATA (outbuf);
  size = GST_BUFFER_SIZE (outbuf);

  image = zbar_image_create ();
  zbar_image_set_format (image, *(gint *) "Y800");
  zbar_image_set_size (image, zbar->width, zbar->height);
  zbar_image_set_data (image, (gpointer) data, zbar->width * zbar->height,
      NULL);

  /* scan the image for barcodes */
  n = zbar_scan_image (zbar->scanner, image);

  /* extract results */
  symbol = zbar_image_first_symbol (image);
  for (; symbol; symbol = zbar_symbol_next (symbol)) {
    zbar_symbol_type_t typ = zbar_symbol_get_type (symbol);
    const char *data = zbar_symbol_get_data (symbol);
    gint quality = zbar_symbol_get_quality (symbol);

    GST_DEBUG_OBJECT (zbar, "decoded %s symbol \"%s\" at quality %d",
        zbar_get_symbol_name (typ), data, quality);

    if (zbar_symbol_get_count (symbol) != 0)
      continue;

    if (zbar->message) {
      GstMessage *m;
      GstStructure *s;

      /* post a message */
      s = gst_structure_new ("barcode",
          "timestamp", G_TYPE_UINT64, GST_BUFFER_TIMESTAMP (outbuf),
          "type", G_TYPE_STRING, zbar_get_symbol_name (typ),
          "symbol", G_TYPE_STRING, data, "quality", G_TYPE_INT, quality, NULL);
      m = gst_message_new_element (GST_OBJECT (zbar), s);
      gst_element_post_message (GST_ELEMENT (zbar), m);
    }
  }

  /* clean up */
  zbar_image_destroy (image);

done:
  return GST_FLOW_OK;

/* ERRORS */
}

static gboolean
gst_zbar_start (GstBaseTransform * base)
{
  GstZBar *zbar = GST_ZBAR (base);

  /* start the cache (e.g. for filtering dupes) */
  zbar_image_scanner_enable_cache (zbar->scanner, TRUE);

  return TRUE;
}

static gboolean
gst_zbar_stop (GstBaseTransform * base)
{
  GstZBar *zbar = GST_ZBAR (base);

  /* stop the cache (e.g. for filtering dupes) */
  zbar_image_scanner_enable_cache (zbar->scanner, FALSE);

  return TRUE;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (zbar_debug, "zbar", 0, "zbar");

  return gst_element_register (plugin, "zbar", GST_RANK_NONE, GST_TYPE_ZBAR);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "zbar",
    "zbar barcode scanner",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
