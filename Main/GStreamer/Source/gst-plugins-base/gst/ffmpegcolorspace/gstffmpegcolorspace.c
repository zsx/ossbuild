/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * This file:
 * Copyright (C) 2003 Ronald Bultje <rbultje@ronald.bitfreak.net>
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
 * SECTION:element-ffmpegcolorspace
 *
 * Convert video frames between a great variety of colorspace formats.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v videotestsrc ! video/x-raw-yuv,format=\(fourcc\)YUY2 ! ffmpegcolorspace ! ximagesink
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "gstffmpegcolorspace.h"
#include "gstffmpegcodecmap.h"

GST_DEBUG_CATEGORY_STATIC (ffmpegcolorspace_debug);
#define GST_CAT_DEFAULT ffmpegcolorspace_debug

/* elementfactory information */
static const GstElementDetails ffmpegcsp_details =
GST_ELEMENT_DETAILS ("FFMPEG Colorspace converter",
    "Filter/Converter/Video",
    "Converts video from one colorspace to another",
    "GStreamer maintainers <gstreamer-devel@lists.sourceforge.net>");


/* Stereo signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  ARG_0,
};

static GType gst_ffmpegcsp_get_type (void);

static void gst_ffmpegcsp_base_init (GstFFMpegCspClass * klass);
static void gst_ffmpegcsp_class_init (GstFFMpegCspClass * klass);
static void gst_ffmpegcsp_init (GstFFMpegCsp * space);

static gboolean gst_ffmpegcsp_set_caps (GstBaseTransform * btrans,
    GstCaps * incaps, GstCaps * outcaps);
static gboolean gst_ffmpegcsp_get_unit_size (GstBaseTransform * btrans,
    GstCaps * caps, guint * size);
static GstFlowReturn gst_ffmpegcsp_transform (GstBaseTransform * btrans,
    GstBuffer * inbuf, GstBuffer * outbuf);
#if 0
static GstFlowReturn gst_ffmpegcsp_transform_ip (GstBaseTransform * btrans,
    GstBuffer * inbuf);
#endif

static GstPadTemplate *sinktempl, *srctempl;
static GstElementClass *parent_class = NULL;

/*static guint gst_ffmpegcsp_signals[LAST_SIGNAL] = { 0 }; */

/* copies the given caps */
static GstCaps *
gst_ffmpegcsp_caps_remove_format_info (GstCaps * caps)
{
  int i;
  GstStructure *structure;
  GstCaps *rgbcaps;
  GstCaps *graycaps;

  caps = gst_caps_copy (caps);

  for (i = 0; i < gst_caps_get_size (caps); i++) {
    structure = gst_caps_get_structure (caps, i);

    gst_structure_set_name (structure, "video/x-raw-yuv");
    gst_structure_remove_field (structure, "format");
    gst_structure_remove_field (structure, "endianness");
    gst_structure_remove_field (structure, "depth");
    gst_structure_remove_field (structure, "bpp");
    gst_structure_remove_field (structure, "red_mask");
    gst_structure_remove_field (structure, "green_mask");
    gst_structure_remove_field (structure, "blue_mask");
    gst_structure_remove_field (structure, "alpha_mask");
    gst_structure_remove_field (structure, "palette_data");
  }

  gst_caps_do_simplify (caps);
  rgbcaps = gst_caps_copy (caps);

  for (i = 0; i < gst_caps_get_size (rgbcaps); i++) {
    structure = gst_caps_get_structure (rgbcaps, i);

    gst_structure_set_name (structure, "video/x-raw-rgb");
  }
  graycaps = gst_caps_copy (caps);

  for (i = 0; i < gst_caps_get_size (graycaps); i++) {
    structure = gst_caps_get_structure (graycaps, i);

    gst_structure_set_name (structure, "video/x-raw-gray");
  }

  gst_caps_append (caps, graycaps);
  gst_caps_append (caps, rgbcaps);

  return caps;
}

/* The caps can be transformed into any other caps with format info removed.
 * However, we should prefer passthrough, so if passthrough is possible,
 * put it first in the list. */
static GstCaps *
gst_ffmpegcsp_transform_caps (GstBaseTransform * btrans,
    GstPadDirection direction, GstCaps * caps)
{
  GstCaps *template;
  GstCaps *result;

  template = gst_ffmpegcsp_codectype_to_caps (CODEC_TYPE_VIDEO, NULL);
  result = gst_caps_intersect (caps, template);
  gst_caps_unref (template);

  gst_caps_append (result, gst_ffmpegcsp_caps_remove_format_info (caps));

  GST_DEBUG_OBJECT (btrans, "transformed %" GST_PTR_FORMAT " into %"
      GST_PTR_FORMAT, caps, result);

  return result;
}

static gboolean
gst_ffmpegcsp_set_caps (GstBaseTransform * btrans, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstFFMpegCsp *space;
  GstStructure *structure;
  gint in_height, in_width;
  gint out_height, out_width;
  const GValue *in_framerate = NULL;
  const GValue *out_framerate = NULL;
  const GValue *in_par = NULL;
  const GValue *out_par = NULL;
  AVCodecContext *ctx;
  gboolean res;

  space = GST_FFMPEGCSP (btrans);

  /* parse in and output values */
  structure = gst_caps_get_structure (incaps, 0);

  /* we have to have width and height */
  res = gst_structure_get_int (structure, "width", &in_width);
  res &= gst_structure_get_int (structure, "height", &in_height);
  if (!res)
    goto no_width_height;

  /* and framerate */
  in_framerate = gst_structure_get_value (structure, "framerate");
  if (in_framerate == NULL || !GST_VALUE_HOLDS_FRACTION (in_framerate))
    goto no_framerate;

  /* this is optional */
  in_par = gst_structure_get_value (structure, "pixel-aspect-ratio");

  structure = gst_caps_get_structure (outcaps, 0);

  /* we have to have width and height */
  res = gst_structure_get_int (structure, "width", &out_width);
  res &= gst_structure_get_int (structure, "height", &out_height);
  if (!res)
    goto no_width_height;

  /* and framerate */
  out_framerate = gst_structure_get_value (structure, "framerate");
  if (out_framerate == NULL || !GST_VALUE_HOLDS_FRACTION (out_framerate))
    goto no_framerate;

  /* this is optional */
  out_par = gst_structure_get_value (structure, "pixel-aspect-ratio");

  /* these must match */
  if (in_width != out_width || in_height != out_height ||
      gst_value_compare (in_framerate, out_framerate) != GST_VALUE_EQUAL)
    goto format_mismatch;

  /* if present, these must match too */
  if (in_par && out_par
      && gst_value_compare (in_par, out_par) != GST_VALUE_EQUAL)
    goto format_mismatch;

  ctx = avcodec_alloc_context ();

  space->width = ctx->width = in_width;
  space->height = ctx->height = in_height;

  space->interlaced = FALSE;
  gst_structure_get_boolean (structure, "interlaced", &space->interlaced);

  /* get from format */
  ctx->pix_fmt = PIX_FMT_NB;
  gst_ffmpegcsp_caps_with_codectype (CODEC_TYPE_VIDEO, incaps, ctx);
  if (ctx->pix_fmt == PIX_FMT_NB)
    goto invalid_in_caps;
  space->from_pixfmt = ctx->pix_fmt;

  /* palette, only for from data */
  if (space->palette)
    av_free (space->palette);
  space->palette = ctx->palctrl;
  ctx->palctrl = NULL;

  /* get to format */
  ctx->pix_fmt = PIX_FMT_NB;
  gst_ffmpegcsp_caps_with_codectype (CODEC_TYPE_VIDEO, outcaps, ctx);
  if (ctx->pix_fmt == PIX_FMT_NB)
    goto invalid_out_caps;
  space->to_pixfmt = ctx->pix_fmt;

  GST_DEBUG ("reconfigured %d %d", space->from_pixfmt, space->to_pixfmt);

  av_free (ctx);

  return TRUE;

  /* ERRORS */
no_width_height:
  {
    GST_DEBUG_OBJECT (space, "did not specify width or height");
    space->from_pixfmt = PIX_FMT_NB;
    space->to_pixfmt = PIX_FMT_NB;
    return FALSE;
  }
no_framerate:
  {
    GST_DEBUG_OBJECT (space, "did not specify framerate");
    space->from_pixfmt = PIX_FMT_NB;
    space->to_pixfmt = PIX_FMT_NB;
    return FALSE;
  }
format_mismatch:
  {
    GST_DEBUG_OBJECT (space, "input and output formats do not match");
    space->from_pixfmt = PIX_FMT_NB;
    space->to_pixfmt = PIX_FMT_NB;
    return FALSE;
  }
invalid_in_caps:
  {
    GST_DEBUG_OBJECT (space, "could not configure context for input format");
    av_free (ctx);
    space->from_pixfmt = PIX_FMT_NB;
    space->to_pixfmt = PIX_FMT_NB;
    return FALSE;
  }
invalid_out_caps:
  {
    GST_DEBUG_OBJECT (space, "could not configure context for output format");
    av_free (ctx);
    space->from_pixfmt = PIX_FMT_NB;
    space->to_pixfmt = PIX_FMT_NB;
    return FALSE;
  }
}

static GType
gst_ffmpegcsp_get_type (void)
{
  static GType ffmpegcsp_type = 0;

  if (!ffmpegcsp_type) {
    static const GTypeInfo ffmpegcsp_info = {
      sizeof (GstFFMpegCspClass),
      (GBaseInitFunc) gst_ffmpegcsp_base_init,
      NULL,
      (GClassInitFunc) gst_ffmpegcsp_class_init,
      NULL,
      NULL,
      sizeof (GstFFMpegCsp),
      0,
      (GInstanceInitFunc) gst_ffmpegcsp_init,
    };

    ffmpegcsp_type = g_type_register_static (GST_TYPE_BASE_TRANSFORM,
        "GstFFMpegCsp", &ffmpegcsp_info, 0);
  }

  return ffmpegcsp_type;
}

static void
gst_ffmpegcsp_base_init (GstFFMpegCspClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class, srctempl);
  gst_element_class_add_pad_template (element_class, sinktempl);
  gst_element_class_set_details (element_class, &ffmpegcsp_details);
}

static void
gst_ffmpegcsp_finalize (GObject * obj)
{
  GstFFMpegCsp *space = GST_FFMPEGCSP (obj);

  if (space->palette)
    av_free (space->palette);

  G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
gst_ffmpegcsp_class_init (GstFFMpegCspClass * klass)
{
  GObjectClass *gobject_class;
  GstBaseTransformClass *gstbasetransform_class;

  gobject_class = (GObjectClass *) klass;
  gstbasetransform_class = (GstBaseTransformClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_ffmpegcsp_finalize);

  gstbasetransform_class->transform_caps =
      GST_DEBUG_FUNCPTR (gst_ffmpegcsp_transform_caps);
  gstbasetransform_class->set_caps = GST_DEBUG_FUNCPTR (gst_ffmpegcsp_set_caps);
  gstbasetransform_class->get_unit_size =
      GST_DEBUG_FUNCPTR (gst_ffmpegcsp_get_unit_size);
  gstbasetransform_class->transform =
      GST_DEBUG_FUNCPTR (gst_ffmpegcsp_transform);
#if 0
  gstbasetransform_class->transform_ip =
      GST_DEBUG_FUNCPTR (gst_ffmpegcsp_transform_ip);
#endif

  gstbasetransform_class->passthrough_on_same_caps = TRUE;

  GST_DEBUG_CATEGORY_INIT (ffmpegcolorspace_debug, "ffmpegcolorspace", 0,
      "FFMPEG-based colorspace converter");
}

static void
gst_ffmpegcsp_init (GstFFMpegCsp * space)
{
  gst_base_transform_set_qos_enabled (GST_BASE_TRANSFORM (space), TRUE);
  space->from_pixfmt = space->to_pixfmt = PIX_FMT_NB;
  space->palette = NULL;
}

static gboolean
gst_ffmpegcsp_get_unit_size (GstBaseTransform * btrans, GstCaps * caps,
    guint * size)
{
  GstStructure *structure = NULL;
  AVCodecContext *ctx = NULL;
  gboolean ret = TRUE;
  gint width, height;

  g_assert (size);

  structure = gst_caps_get_structure (caps, 0);
  gst_structure_get_int (structure, "width", &width);
  gst_structure_get_int (structure, "height", &height);

  ctx = avcodec_alloc_context ();

  g_assert (ctx != NULL);

  ctx->pix_fmt = PIX_FMT_NB;

  gst_ffmpegcsp_caps_with_codectype (CODEC_TYPE_VIDEO, caps, ctx);

  if (G_UNLIKELY (ctx->pix_fmt == PIX_FMT_NB)) {
    ret = FALSE;
    goto beach;
  }

  *size = avpicture_get_size (ctx->pix_fmt, width, height);

  /* ffmpeg frames have the palette after the frame data, whereas
   * GStreamer currently puts it into the caps as 'palette_data' field,
   * so for paletted data the frame size avpicture_get_size() returns is
   * 1024 bytes larger than what GStreamer expects. */
  if (gst_structure_has_field (structure, "palette_data") &&
      ctx->pix_fmt == PIX_FMT_PAL8) {
    *size -= 4 * 256;           /* = AVPALETTE_SIZE */
  }

beach:

  if (ctx->palctrl)
    av_free (ctx->palctrl);
  av_free (ctx);

  return ret;
}

#if 0
/* FIXME: Could use transform_ip to implement endianness swap type operations */
static GstFlowReturn
gst_ffmpegcsp_transform_ip (GstBaseTransform * btrans, GstBuffer * inbuf)
{
  /* do nothing */
  return GST_FLOW_OK;
}
#endif

static GstFlowReturn
gst_ffmpegcsp_transform (GstBaseTransform * btrans, GstBuffer * inbuf,
    GstBuffer * outbuf)
{
  GstFFMpegCsp *space;
  gint result;

  space = GST_FFMPEGCSP (btrans);

  GST_DEBUG ("from %d -> to %d", space->from_pixfmt, space->to_pixfmt);
  if (space->from_pixfmt == PIX_FMT_NB || space->to_pixfmt == PIX_FMT_NB)
    goto unknown_format;

  /* fill from with source data */
  gst_ffmpegcsp_avpicture_fill (&space->from_frame,
      GST_BUFFER_DATA (inbuf), space->from_pixfmt, space->width, space->height,
      space->interlaced);

  /* fill optional palette */
  if (space->palette)
    space->from_frame.data[1] = (uint8_t *) space->palette->palette;

  /* fill target frame */
  gst_ffmpegcsp_avpicture_fill (&space->to_frame,
      GST_BUFFER_DATA (outbuf), space->to_pixfmt, space->width, space->height,
      space->interlaced);

  /* and convert */
  result = img_convert (&space->to_frame, space->to_pixfmt,
      &space->from_frame, space->from_pixfmt, space->width, space->height);
  if (result == -1)
    goto not_supported;

  /* baseclass copies timestamps */
  GST_DEBUG ("from %d -> to %d done", space->from_pixfmt, space->to_pixfmt);

  return GST_FLOW_OK;

  /* ERRORS */
unknown_format:
  {
    GST_ELEMENT_ERROR (space, CORE, NOT_IMPLEMENTED, (NULL),
        ("attempting to convert colorspaces between unknown formats"));
    return GST_FLOW_NOT_NEGOTIATED;
  }
not_supported:
  {
    GST_ELEMENT_ERROR (space, CORE, NOT_IMPLEMENTED, (NULL),
        ("cannot convert between formats"));
    return GST_FLOW_NOT_SUPPORTED;
  }
}

gboolean
gst_ffmpegcolorspace_register (GstPlugin * plugin)
{
  GstCaps *caps;

  /* template caps */
  caps = gst_ffmpegcsp_codectype_to_caps (CODEC_TYPE_VIDEO, NULL);

  /* build templates */
  srctempl = gst_pad_template_new ("src",
      GST_PAD_SRC, GST_PAD_ALWAYS, gst_caps_copy (caps));

  /* the sink template will do palette handling as well... */
  sinktempl = gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS, caps);

  return gst_element_register (plugin, "ffmpegcolorspace",
      GST_RANK_NONE, GST_TYPE_FFMPEGCSP);
}
