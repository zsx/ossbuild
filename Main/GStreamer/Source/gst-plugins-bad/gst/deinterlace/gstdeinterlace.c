/* GStreamer simple deinterlacing plugin
 * Copyright (C) 1999 Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) 2006-2008 Tim-Philipp Müller <tim centricular net>
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
/* based on the Area Based Deinterlacer (for RGB frames)        */
/* (a VirtualDub filter) from Gunnar Thalin <guth@home.se>      */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "gstdeinterlace.h"
#include <gst/video/video.h>

/**
 * SECTION:element-deinterlace
 *
 * Adaptively deinterlaces video frames by detecting interlacing artifacts.
 * An edge detection matrix is used, with a threshold value. Pixels detected
 * as 'interlaced' are replaced with pixels blended from the pixels above and
 * below.
 * 
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v videotestsrc ! deinterlace ! ffmpegcolorspace ! xvimagesink
 * ]|
 * </refsect2>
 */

GST_DEBUG_CATEGORY_STATIC (deinterlace_debug);
#define GST_CAT_DEFAULT deinterlace_debug

#define DEFAULT_DI_AREA_ONLY  FALSE
#define DEFAULT_NI_AREA_ONLY  FALSE
#define DEFAULT_BLEND         FALSE
#define DEFAULT_DEINTERLACE   TRUE
#define DEFAULT_THRESHOLD     20
#define DEFAULT_EDGE_DETECT   25

enum
{
  ARG_0,
  ARG_DI_ONLY,
  ARG_NI_ONLY,
  ARG_BLEND,
  ARG_THRESHOLD,
  ARG_EDGE_DETECT,
  ARG_DEINTERLACE
};

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_YUV ("{ I420, Y42B }"))
    );

static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_YUV ("{ I420, Y42B }"))
    );

GST_BOILERPLATE (GstDeinterlace, gst_deinterlace, GstBaseTransform,
    GST_TYPE_BASE_TRANSFORM);

static void gst_deinterlace_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_deinterlace_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static GstFlowReturn gst_deinterlace_transform_ip (GstBaseTransform * trans,
    GstBuffer * buf);
static gboolean gst_deinterlace_stop (GstBaseTransform * trans);
static gboolean gst_deinterlace_set_caps (GstBaseTransform * trans,
    GstCaps * incaps, GstCaps * outcaps);
static GstCaps *gst_deinterlace_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * incaps);

static void
gst_deinterlace_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_factory));

  gst_element_class_set_details_simple (element_class, "Deinterlace",
      "Filter/Effect/Video", "Deinterlace video",
      "Wim Taymans <wim.taymans@gmail.com>");
}

static void
gst_deinterlace_class_init (GstDeinterlaceClass * klass)
{
  GObjectClass *gobject_class;
  GstBaseTransformClass *basetransform_class;

  gobject_class = (GObjectClass *) klass;
  basetransform_class = (GstBaseTransformClass *) klass;

  gobject_class->set_property = gst_deinterlace_set_property;
  gobject_class->get_property = gst_deinterlace_get_property;

  /**
   * GstDeinterlace:deinterlace:
   *
   * Turn processing on/off. When false, no modification of the 
   * video frames occurs and they pass through intact.
   */
  g_object_class_install_property (gobject_class, ARG_DEINTERLACE,
      g_param_spec_boolean ("deinterlace", "deinterlace",
          "turn deinterlacing on/off", DEFAULT_DEINTERLACE, G_PARAM_READWRITE));
  /**
   * GstDeinterlace:di-area-only:
   *
   * When set to true, only areas affected by the deinterlacing are output,
   * making it easy to see which regions are being modified.
   *
   * See Also: #GstDeinterlace:ni-area-only
   */
  g_object_class_install_property (gobject_class, ARG_DI_ONLY,
      g_param_spec_boolean ("di-area-only", "di-area-only",
          "displays deinterlaced areas only", DEFAULT_DI_AREA_ONLY,
          G_PARAM_READWRITE));
  /**
   * GstDeinterlace:ni-area-only:
   *
   * When set to true, only areas unaffected by the deinterlacing are output,
   * making it easy to see which regions are being preserved intact.
   *
   * See Also: #GstDeinterlace:di-area-only
   */
  g_object_class_install_property (gobject_class, ARG_NI_ONLY,
      g_param_spec_boolean ("ni-area-only", "ni-area-only",
          "displays non-interlaced areas only", DEFAULT_DI_AREA_ONLY,
          G_PARAM_READWRITE));
  /**
   * GstDeinterlace:blend:
   *
   * Change the blending for pixels which are detected as
   * 'interlacing artifacts'. When true, the output pixel is a weighted
   * average (1,2,1) of the pixel and the pixels above and below it.
   * When false, the odd field lines are preserved, and the even field lines
   * are averaged from the surrounding pixels above and below (the odd field).
   */
  g_object_class_install_property (gobject_class, ARG_BLEND,
      g_param_spec_boolean ("blend", "blend", "blend", DEFAULT_BLEND,
          G_PARAM_READWRITE));
  /**
   * GstDeinterlace:threshold:
   *
   * Affects the threshold of the edge-detection function used for detecting
   * interlacing artifacts.
   */
  g_object_class_install_property (gobject_class, ARG_THRESHOLD,
      g_param_spec_int ("threshold", "Edge-detection threshold",
          "Threshold value for the interlacing artifacts in the output "
          "of the edge detection", G_MININT, G_MAXINT, 0, G_PARAM_READWRITE));
  /**
   * GstDeinterlace:edge-detect:
   *
   * Affects the weighting of the edge-detection function used for detecting
   * interlacing artifacts.
   */
  g_object_class_install_property (gobject_class, ARG_EDGE_DETECT,
      g_param_spec_int ("edge-detect", "edge detection weighting",
          "Weighting value used for calculating the edge detection matrix",
          G_MININT, G_MAXINT, 0, G_PARAM_READWRITE));

  basetransform_class->transform_ip =
      GST_DEBUG_FUNCPTR (gst_deinterlace_transform_ip);
  basetransform_class->transform_caps =
      GST_DEBUG_FUNCPTR (gst_deinterlace_transform_caps);
  basetransform_class->stop = GST_DEBUG_FUNCPTR (gst_deinterlace_stop);
  basetransform_class->set_caps = GST_DEBUG_FUNCPTR (gst_deinterlace_set_caps);
}

static void
gst_deinterlace_init (GstDeinterlace * filter, GstDeinterlaceClass * klass)
{
  filter->show_deinterlaced_area_only = DEFAULT_DI_AREA_ONLY;
  filter->show_noninterlaced_area_only = DEFAULT_NI_AREA_ONLY;
  filter->blend = DEFAULT_BLEND;
  filter->deinterlace = DEFAULT_DEINTERLACE;
  filter->threshold = DEFAULT_THRESHOLD;
  filter->edge_detect = DEFAULT_EDGE_DETECT;
  /*filter->threshold_blend = 0;  */

  filter->src = NULL;
  filter->picsize = 0;
}

static gboolean
gst_deinterlace_stop (GstBaseTransform * trans)
{
  GstDeinterlace *filter;

  filter = GST_DEINTERLACE (trans);

  g_free (filter->src);
  filter->src = NULL;
  filter->picsize = 0;
  filter->width = 0;
  filter->height = 0;

  return TRUE;
}

static GstCaps *
gst_deinterlace_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * incaps)
{
  return gst_caps_ref (incaps);
}

static gboolean
gst_deinterlace_set_caps (GstBaseTransform * trans, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstDeinterlace *filter;
  GstVideoFormat fmt;
  GstStructure *s;
  guint32 fourcc;
  gint picsize, w, h;

  filter = GST_DEINTERLACE (trans);

  g_assert (gst_caps_is_equal_fixed (incaps, outcaps));

  s = gst_caps_get_structure (incaps, 0);
  if (!gst_structure_get_int (s, "width", &w) ||
      !gst_structure_get_int (s, "height", &h) ||
      !gst_structure_get_fourcc (s, "format", &fourcc)) {
    return FALSE;
  }

  filter->width = w;
  filter->height = h;
  filter->fourcc = fourcc;

  GST_DEBUG_OBJECT (filter, "width x height = %d x %d, fourcc: %"
      GST_FOURCC_FORMAT, w, h, GST_FOURCC_ARGS (fourcc));

  fmt = gst_video_format_from_fourcc (fourcc);

  filter->y_stride = gst_video_format_get_row_stride (fmt, 0, w);
  filter->u_stride = gst_video_format_get_row_stride (fmt, 1, w);
  filter->v_stride = gst_video_format_get_row_stride (fmt, 2, w);

  filter->uv_height = gst_video_format_get_component_height (fmt, 1, h);

  filter->y_off = gst_video_format_get_component_offset (fmt, 0, w, h);
  filter->u_off = gst_video_format_get_component_offset (fmt, 1, w, h);
  filter->v_off = gst_video_format_get_component_offset (fmt, 2, w, h);

  picsize = gst_video_format_get_size (fmt, w, h);

  if (filter->picsize != picsize) {
    filter->picsize = picsize;
    g_free (filter->src);       /* free + alloc avoids memcpy */
    filter->src = g_malloc0 (filter->picsize);
    GST_LOG_OBJECT (filter, "temp buffer size %d", filter->picsize);
  }

  return TRUE;
}

static GstFlowReturn
gst_deinterlace_transform_ip (GstBaseTransform * trans, GstBuffer * buf)
{
  GstDeinterlace *filter;
  gboolean bShowDeinterlacedAreaOnly;
  gboolean bShowNoninterlacedAreaOnly;
  gint y0, y1, y2, y3;
  guchar *psrc1, *pdst1, *yuvptr, *src;
  gint iInterlaceValue0, iInterlaceValue1, iInterlaceValue2;
  gint x, y, p;
  gint y_line;
  guchar *y_dst, *y_src;
  guchar fill_value;
  gboolean bBlend;
  gboolean bDeinterlace;
  gint iThreshold;
  gint iEdgeDetect;
  gint width, height;

  /* g_assert (gst_buffer_is_writable (buf)); */

  filter = GST_DEINTERLACE (trans);

  GST_OBJECT_LOCK (filter);
  bBlend = filter->blend;
  bDeinterlace = filter->deinterlace;
  iThreshold = filter->threshold;
  iEdgeDetect = filter->edge_detect;
  bShowDeinterlacedAreaOnly = filter->show_deinterlaced_area_only;
  bShowNoninterlacedAreaOnly = filter->show_noninterlaced_area_only;
  GST_OBJECT_UNLOCK (filter);

  src = filter->src;
  yuvptr = GST_BUFFER_DATA (buf);

  memcpy (filter->src, yuvptr, filter->picsize);


  iThreshold = iThreshold * iThreshold * 4;
  /* We don't want an integer overflow in the interlace calculation. */
  if (iEdgeDetect > 180)
    iEdgeDetect = 180;
  iEdgeDetect = iEdgeDetect * iEdgeDetect;

  for (p = 0; p < 3; p++) {
    switch (p) {
      case 0:
        y_dst = yuvptr + filter->y_off; /* dst y pointer */
        y_line = filter->y_stride;
        y_src = src + filter->y_off;
        width = filter->width;
        height = filter->height;
        fill_value = 0;
        break;
      case 1:
        y_dst = yuvptr + filter->u_off; /* dst U pointer */
        y_line = filter->u_stride;
        y_src = src + filter->u_off;
        width = filter->width / 2;
        height = filter->uv_height;
        fill_value = 128;
        break;
      case 2:
        y_dst = yuvptr + filter->v_off; /* dst V pointer */
        y_line = filter->v_stride;
        y_src = src + filter->v_off;
        width = filter->width / 2;
        height = filter->uv_height;
        fill_value = 128;
        break;
    }

    for (x = 0; x < width; x++) {
      pdst1 = y_dst + x;
      psrc1 = y_src + x;
      iInterlaceValue1 = iInterlaceValue2 = 0;

      for (y = 0; y < height; y++, psrc1 += y_line, pdst1 += y_line) {
        /* current line is 1 */
        y0 = y1 = y2 = y3 = *psrc1;
        if (y > 0)
          y0 = *(psrc1 - y_line);
        if (y < (height - 1))
          y2 = *(psrc1 + y_line);
        if (y < (height - 2))
          y3 = *(psrc1 + 2 * y_line);

        iInterlaceValue0 = iInterlaceValue1;
        iInterlaceValue1 = iInterlaceValue2;

        if (y < height - 1)
          iInterlaceValue2 =
              (ABS (y1 - y2) * ABS (y3 - y2) - ((iEdgeDetect * (y1 - y3) * (y1 -
                          y3)) >> 12)) * 10;
        else
          iInterlaceValue2 = 0;

        if ((iInterlaceValue0 + 2 * iInterlaceValue1 + iInterlaceValue2 >
                iThreshold) && (y > 0)) {
          if (bShowNoninterlacedAreaOnly) {
            *pdst1 = fill_value;        /* blank the point and so the interlac area */
          } else {
            if (bDeinterlace) {
              if (bBlend) {
                *pdst1 = (unsigned char) ((y0 + 2 * y1 + y2) >> 2);
              } else {
                /* this method seems to work better than blending if the */
                /* quality is pretty bad and the half pics don't fit together */
                if ((y % 2) == 1) {     /* if odd simply copy the value */
                  *pdst1 = *psrc1;
                } else {        /* if even interpolate the line (upper + lower)/2 */
                  *pdst1 = (unsigned char) ((y0 + y2) >> 1);
                }
              }
            } else {
              *pdst1 = *psrc1;
            }
          }

        } else {
          /* so we went below the treshold and therefore we don't have to  */
          /* change anything */
          if (bShowDeinterlacedAreaOnly) {
            /* this is for testing to see how we should tune the treshhold */
            /* and shows as the things that haven't change because the  */
            /* threshold was to low?? (or shows that everything is ok :-) */
            *pdst1 = fill_value;        /* blank the point and so the non-interlac area */
          } else {
            *pdst1 = *psrc1;
          }
        }
      }
    }
  }

  return GST_FLOW_OK;
}

static void
gst_deinterlace_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstDeinterlace *filter;

  filter = GST_DEINTERLACE (object);

  GST_OBJECT_LOCK (filter);
  switch (prop_id) {
    case ARG_DEINTERLACE:
      filter->deinterlace = g_value_get_boolean (value);
      break;
    case ARG_DI_ONLY:
      filter->show_deinterlaced_area_only = g_value_get_boolean (value);
      break;
    case ARG_NI_ONLY:
      filter->show_noninterlaced_area_only = g_value_get_boolean (value);
      break;
    case ARG_BLEND:
      filter->blend = g_value_get_boolean (value);
      break;
    case ARG_THRESHOLD:
      filter->threshold = g_value_get_int (value);
      break;
    case ARG_EDGE_DETECT:
      filter->edge_detect = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
  GST_OBJECT_UNLOCK (filter);
}

static void
gst_deinterlace_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstDeinterlace *filter;

  filter = GST_DEINTERLACE (object);

  GST_OBJECT_LOCK (filter);
  switch (prop_id) {
    case ARG_DEINTERLACE:
      g_value_set_boolean (value, filter->deinterlace);
      break;
    case ARG_DI_ONLY:
      g_value_set_boolean (value, filter->show_deinterlaced_area_only);
      break;
    case ARG_NI_ONLY:
      g_value_set_boolean (value, filter->show_noninterlaced_area_only);
      break;
    case ARG_BLEND:
      g_value_set_boolean (value, filter->blend);
      break;
    case ARG_THRESHOLD:
      g_value_set_int (value, filter->threshold);
      break;
    case ARG_EDGE_DETECT:
      g_value_set_int (value, filter->edge_detect);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
  GST_OBJECT_UNLOCK (filter);
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (deinterlace_debug, "deinterlace", 0,
      "deinterlace element");

  if (!gst_element_register (plugin, "deinterlace", GST_RANK_NONE,
          gst_deinterlace_get_type ()))
    return FALSE;

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "gstinterlace",
    "Deinterlace video", plugin_init, PACKAGE_VERSION, "LGPL", GST_PACKAGE_NAME,
    GST_PACKAGE_ORIGIN);
