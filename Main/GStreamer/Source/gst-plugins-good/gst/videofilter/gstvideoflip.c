/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) <2003> David Schleef <ds@schleef.org>
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
 * This file was (probably) generated from gstvideoflip.c,
 * gstvideoflip.c,v 1.7 2003/11/08 02:48:59 dschleef Exp 
 */
/**
 * SECTION:element-videoflip
 *
 * Flips and rotates video.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch videotestsrc ! videoflip method=clockwise ! ffmpegcolorspace ! ximagesink
 * ]| This pipeline flips the test image 90 degrees clockwise.
 * </refsect2>
 *
 * Last reviewed on 2006-03-03 (0.10.3)
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstvideoflip.h"

#include <string.h>

#include <gst/video/video.h>

/* GstVideoFlip signals and args */
enum
{
  ARG_0,
  ARG_METHOD
      /* FILL ME */
};

GST_DEBUG_CATEGORY_STATIC (video_flip_debug);
#define GST_CAT_DEFAULT video_flip_debug

static const GstElementDetails video_flip_details =
GST_ELEMENT_DETAILS ("Video flipper",
    "Filter/Effect/Video",
    "Flips and rotates video",
    "David Schleef <ds@schleef.org>");

static GstStaticPadTemplate gst_video_flip_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_YUV ("{ IYUV, I420, YV12 }"))
    );

static GstStaticPadTemplate gst_video_flip_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_YUV ("{ IYUV, I420, YV12 }"))
    );

static GstVideoFilterClass *parent_class = NULL;

#define GST_TYPE_VIDEO_FLIP_METHOD (gst_video_flip_method_get_type())

static GType
gst_video_flip_method_get_type (void)
{
  static GType video_flip_method_type = 0;
  static const GEnumValue video_flip_methods[] = {
    {GST_VIDEO_FLIP_METHOD_IDENTITY, "Identity (no rotation)", "none"},
    {GST_VIDEO_FLIP_METHOD_90R, "Rotate clockwise 90 degrees", "clockwise"},
    {GST_VIDEO_FLIP_METHOD_180, "Rotate 180 degrees", "rotate-180"},
    {GST_VIDEO_FLIP_METHOD_90L, "Rotate counter-clockwise 90 degrees",
        "counterclockwise"},
    {GST_VIDEO_FLIP_METHOD_HORIZ, "Flip horizontally", "horizontal-flip"},
    {GST_VIDEO_FLIP_METHOD_VERT, "Flip vertically", "vertical-flip"},
    {GST_VIDEO_FLIP_METHOD_TRANS,
        "Flip across upper left/lower right diagonal", "upper-left-diagonal"},
    {GST_VIDEO_FLIP_METHOD_OTHER,
        "Flip across upper right/lower left diagonal", "upper-right-diagonal"},
    {0, NULL, NULL},
  };

  if (!video_flip_method_type) {
    video_flip_method_type = g_enum_register_static ("GstVideoFlipMethod",
        video_flip_methods);
  }
  return video_flip_method_type;
}

static gboolean
gst_video_flip_set_caps (GstBaseTransform * btrans, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstVideoFlip *vf;
  GstStructure *in_s, *out_s;
  gboolean ret = FALSE;

  vf = GST_VIDEO_FLIP (btrans);

  in_s = gst_caps_get_structure (incaps, 0);
  out_s = gst_caps_get_structure (outcaps, 0);

  if (gst_structure_get_int (in_s, "width", &vf->from_width) &&
      gst_structure_get_int (in_s, "height", &vf->from_height) &&
      gst_structure_get_int (out_s, "width", &vf->to_width) &&
      gst_structure_get_int (out_s, "height", &vf->to_height)) {
    /* Check that they are correct */
    switch (vf->method) {
      case GST_VIDEO_FLIP_METHOD_90R:
      case GST_VIDEO_FLIP_METHOD_90L:
      case GST_VIDEO_FLIP_METHOD_TRANS:
      case GST_VIDEO_FLIP_METHOD_OTHER:
        if ((vf->from_width != vf->to_height) ||
            (vf->from_height != vf->to_width)) {
          GST_DEBUG_OBJECT (vf, "we are inverting width and height but caps "
              "are not correct : %dx%d to %dx%d", vf->from_width,
              vf->from_height, vf->to_width, vf->to_height);
          goto beach;
        }
        break;
      case GST_VIDEO_FLIP_METHOD_IDENTITY:

        break;
      case GST_VIDEO_FLIP_METHOD_180:
      case GST_VIDEO_FLIP_METHOD_HORIZ:
      case GST_VIDEO_FLIP_METHOD_VERT:
        if ((vf->from_width != vf->to_width) ||
            (vf->from_height != vf->to_height)) {
          GST_DEBUG_OBJECT (vf, "we are keeping width and height but caps "
              "are not correct : %dx%d to %dx%d", vf->from_width,
              vf->from_height, vf->to_width, vf->to_height);
          goto beach;
        }
        break;
      default:
        g_assert_not_reached ();
        break;
    }
  }

  ret = TRUE;

beach:
  return ret;
}

static GstCaps *
gst_video_flip_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps)
{
  GstVideoFlip *videoflip;
  GstCaps *ret;
  gint width, height, i;

  videoflip = GST_VIDEO_FLIP (trans);

  ret = gst_caps_copy (caps);

  for (i = 0; i < gst_caps_get_size (ret); i++) {
    GstStructure *structure = gst_caps_get_structure (ret, i);

    if (gst_structure_get_int (structure, "width", &width) &&
        gst_structure_get_int (structure, "height", &height)) {

      switch (videoflip->method) {
        case GST_VIDEO_FLIP_METHOD_90R:
        case GST_VIDEO_FLIP_METHOD_90L:
        case GST_VIDEO_FLIP_METHOD_TRANS:
        case GST_VIDEO_FLIP_METHOD_OTHER:
          gst_structure_set (structure, "width", G_TYPE_INT, height,
              "height", G_TYPE_INT, width, NULL);
          break;
        case GST_VIDEO_FLIP_METHOD_IDENTITY:
        case GST_VIDEO_FLIP_METHOD_180:
        case GST_VIDEO_FLIP_METHOD_HORIZ:
        case GST_VIDEO_FLIP_METHOD_VERT:
          gst_structure_set (structure, "width", G_TYPE_INT, width,
              "height", G_TYPE_INT, height, NULL);
          break;
        default:
          g_assert_not_reached ();
          break;
      }
    }
  }

  GST_DEBUG_OBJECT (videoflip, "transformed %" GST_PTR_FORMAT " to %"
      GST_PTR_FORMAT, caps, ret);

  return ret;
}

/* Useful macros */
#define GST_VIDEO_I420_Y_ROWSTRIDE(width) (GST_ROUND_UP_4(width))
#define GST_VIDEO_I420_U_ROWSTRIDE(width) (GST_ROUND_UP_8(width)/2)
#define GST_VIDEO_I420_V_ROWSTRIDE(width) ((GST_ROUND_UP_8(GST_VIDEO_I420_Y_ROWSTRIDE(width)))/2)

#define GST_VIDEO_I420_Y_OFFSET(w,h) (0)
#define GST_VIDEO_I420_U_OFFSET(w,h) (GST_VIDEO_I420_Y_OFFSET(w,h)+(GST_VIDEO_I420_Y_ROWSTRIDE(w)*GST_ROUND_UP_2(h)))
#define GST_VIDEO_I420_V_OFFSET(w,h) (GST_VIDEO_I420_U_OFFSET(w,h)+(GST_VIDEO_I420_U_ROWSTRIDE(w)*GST_ROUND_UP_2(h)/2))

#define GST_VIDEO_I420_SIZE(w,h)     (GST_VIDEO_I420_V_OFFSET(w,h)+(GST_VIDEO_I420_V_ROWSTRIDE(w)*GST_ROUND_UP_2(h)/2))

static gboolean
gst_video_flip_get_unit_size (GstBaseTransform * btrans, GstCaps * caps,
    guint * size)
{
  GstVideoFlip *videoflip;
  GstStructure *structure;
  gboolean ret = FALSE;
  gint width, height;

  videoflip = GST_VIDEO_FLIP (btrans);

  structure = gst_caps_get_structure (caps, 0);

  if (gst_structure_get_int (structure, "width", &width) &&
      gst_structure_get_int (structure, "height", &height)) {
    *size = GST_VIDEO_I420_SIZE (width, height);
    ret = TRUE;
    GST_DEBUG_OBJECT (videoflip, "our frame size is %d bytes (%dx%d)", *size,
        width, height);
  }

  return ret;
}

static GstFlowReturn
gst_video_flip_flip (GstVideoFlip * videoflip, guint8 * dest,
    guint8 * src, int sw, int sh, int dw, int dh)
{
  GstFlowReturn ret = GST_FLOW_OK;
  int x, y;
  guint8 *s = src, *d = dest;

  switch (videoflip->method) {
    case GST_VIDEO_FLIP_METHOD_90R:
      /* Flip Y */
      for (y = 0; y < dh; y++) {
        for (x = 0; x < dw; x++) {
          d[y * GST_VIDEO_I420_Y_ROWSTRIDE (dw) + x] =
              s[(sh - 1 - x) * GST_VIDEO_I420_Y_ROWSTRIDE (sw) + y];
        }
      }
      /* Flip U */
      s = src + GST_VIDEO_I420_U_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_U_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_U_ROWSTRIDE (dw) + x] =
              s[(sh / 2 - 1 - x) * GST_VIDEO_I420_U_ROWSTRIDE (sw) + y];
        }
      }
      /* Flip V */
      s = src + GST_VIDEO_I420_V_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_V_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_V_ROWSTRIDE (dw) + x] =
              s[(sh / 2 - 1 - x) * GST_VIDEO_I420_V_ROWSTRIDE (sw) + y];
        }
      }
      break;
    case GST_VIDEO_FLIP_METHOD_90L:
      /* Flip Y */
      for (y = 0; y < dh; y++) {
        for (x = 0; x < dw; x++) {
          d[y * GST_VIDEO_I420_Y_ROWSTRIDE (dw) + x] =
              s[x * GST_VIDEO_I420_Y_ROWSTRIDE (sw) + (sw - 1 - y)];
        }
      }
      /* Flip U */
      s = src + GST_VIDEO_I420_U_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_U_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_U_ROWSTRIDE (dw) + x] =
              s[x * GST_VIDEO_I420_U_ROWSTRIDE (sw) + (sw / 2 - 1 - y)];
        }
      }
      /* Flip V */
      s = src + GST_VIDEO_I420_V_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_V_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_V_ROWSTRIDE (dw) + x] =
              s[x * GST_VIDEO_I420_V_ROWSTRIDE (sw) + (sw / 2 - 1 - y)];
        }
      }
      break;
    case GST_VIDEO_FLIP_METHOD_180:
      /* Flip Y */
      for (y = 0; y < dh; y++) {
        for (x = 0; x < dw; x++) {
          d[y * GST_VIDEO_I420_Y_ROWSTRIDE (dw) + x] =
              s[(sh - 1 - y) * GST_VIDEO_I420_Y_ROWSTRIDE (sw) + (sw - 1 - x)];
        }
      }
      /* Flip U */
      s = src + GST_VIDEO_I420_U_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_U_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_U_ROWSTRIDE (dw) + x] =
              s[(sh / 2 - 1 - y) * GST_VIDEO_I420_U_ROWSTRIDE (sw) + (sw / 2 -
                  1 - x)];
        }
      }
      /* Flip V */
      s = src + GST_VIDEO_I420_V_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_V_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_V_ROWSTRIDE (dw) + x] =
              s[(sh / 2 - 1 - y) * GST_VIDEO_I420_V_ROWSTRIDE (sw) + (sw / 2 -
                  1 - x)];
        }
      }
      break;
    case GST_VIDEO_FLIP_METHOD_HORIZ:
      /* Flip Y */
      for (y = 0; y < dh; y++) {
        for (x = 0; x < dw; x++) {
          d[y * GST_VIDEO_I420_Y_ROWSTRIDE (dw) + x] =
              s[y * GST_VIDEO_I420_Y_ROWSTRIDE (sw) + (sw - 1 - x)];
        }
      }
      /* Flip U */
      s = src + GST_VIDEO_I420_U_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_U_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_U_ROWSTRIDE (dw) + x] =
              s[y * GST_VIDEO_I420_U_ROWSTRIDE (sw) + (sw / 2 - 1 - x)];
        }
      }
      /* Flip V */
      s = src + GST_VIDEO_I420_V_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_V_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_V_ROWSTRIDE (dw) + x] =
              s[y * GST_VIDEO_I420_V_ROWSTRIDE (sw) + (sw / 2 - 1 - x)];
        }
      }
      break;
    case GST_VIDEO_FLIP_METHOD_VERT:
      /* Flip Y */
      for (y = 0; y < dh; y++) {
        for (x = 0; x < dw; x++) {
          d[y * GST_VIDEO_I420_Y_ROWSTRIDE (dw) + x] =
              s[(sh - 1 - y) * GST_VIDEO_I420_Y_ROWSTRIDE (sw) + x];
        }
      }
      /* Flip U */
      s = src + GST_VIDEO_I420_U_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_U_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_U_ROWSTRIDE (dw) + x] =
              s[(sh / 2 - 1 - y) * GST_VIDEO_I420_U_ROWSTRIDE (sw) + x];
        }
      }
      /* Flip V */
      s = src + GST_VIDEO_I420_V_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_V_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_V_ROWSTRIDE (dw) + x] =
              s[(sh / 2 - 1 - y) * GST_VIDEO_I420_V_ROWSTRIDE (sw) + x];
        }
      }
      break;
    case GST_VIDEO_FLIP_METHOD_TRANS:
      /* Flip Y */
      for (y = 0; y < dh; y++) {
        for (x = 0; x < dw; x++) {
          d[y * GST_VIDEO_I420_Y_ROWSTRIDE (dw) + x] =
              s[x * GST_VIDEO_I420_Y_ROWSTRIDE (sw) + y];
        }
      }
      /* Flip U */
      s = src + GST_VIDEO_I420_U_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_U_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_U_ROWSTRIDE (dw) + x] =
              s[x * GST_VIDEO_I420_U_ROWSTRIDE (sw) + y];
        }
      }
      /* Flip V */
      s = src + GST_VIDEO_I420_V_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_V_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_V_ROWSTRIDE (dw) + x] =
              s[x * GST_VIDEO_I420_V_ROWSTRIDE (sw) + y];
        }
      }
      break;
    case GST_VIDEO_FLIP_METHOD_OTHER:
      /* Flip Y */
      for (y = 0; y < dh; y++) {
        for (x = 0; x < dw; x++) {
          d[y * GST_VIDEO_I420_Y_ROWSTRIDE (dw) + x] =
              s[(sh - 1 - x) * GST_VIDEO_I420_Y_ROWSTRIDE (sw) + (sw - 1 - y)];
        }
      }
      /* Flip U */
      s = src + GST_VIDEO_I420_U_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_U_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_U_ROWSTRIDE (dw) + x] =
              s[(sh / 2 - 1 - x) * GST_VIDEO_I420_U_ROWSTRIDE (sw) + (sw / 2 -
                  1 - y)];
        }
      }
      /* Flip V */
      s = src + GST_VIDEO_I420_V_OFFSET (sw, sh);
      d = dest + GST_VIDEO_I420_V_OFFSET (dw, dh);
      for (y = 0; y < dh / 2; y++) {
        for (x = 0; x < dw / 2; x++) {
          d[y * GST_VIDEO_I420_V_ROWSTRIDE (dw) + x] =
              s[(sh / 2 - 1 - x) * GST_VIDEO_I420_V_ROWSTRIDE (sw) + (sw / 2 -
                  1 - y)];
        }
      }
      break;
    case GST_VIDEO_FLIP_METHOD_IDENTITY:
      memcpy (d, s, GST_VIDEO_I420_SIZE (dw, dh));
      break;
    default:
      ret = GST_FLOW_ERROR;
      break;
  }

  return ret;
}

static GstFlowReturn
gst_video_flip_transform (GstBaseTransform * trans, GstBuffer * in,
    GstBuffer * out)
{
  GstVideoFlip *videoflip;
  gpointer dest, src;
  int sw, sh, dw, dh;
  GstFlowReturn ret = GST_FLOW_OK;

  videoflip = GST_VIDEO_FLIP (trans);

  gst_buffer_copy_metadata (out, in, GST_BUFFER_COPY_TIMESTAMPS);

  src = GST_BUFFER_DATA (in);
  dest = GST_BUFFER_DATA (out);
  sw = videoflip->from_width;
  sh = videoflip->from_height;
  dw = videoflip->to_width;
  dh = videoflip->to_height;

  GST_LOG_OBJECT (videoflip, "videoflip: scaling planar 4:1:1 %dx%d to %dx%d",
      sw, sh, dw, dh);

  ret = gst_video_flip_flip (videoflip, dest, src, sw, sh, dw, dh);

  return ret;
}

static gboolean
gst_video_flip_handle_src_event (GstPad * pad, GstEvent * event)
{
  GstVideoFlip *vf;
  gboolean ret;
  gdouble new_x, new_y, x, y;
  GstStructure *structure;

  vf = GST_VIDEO_FLIP (gst_pad_get_parent (pad));

  GST_DEBUG_OBJECT (vf, "handling %s event", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_NAVIGATION:
      event =
          GST_EVENT (gst_mini_object_make_writable (GST_MINI_OBJECT (event)));

      structure = (GstStructure *) gst_event_get_structure (event);
      if (gst_structure_get_double (structure, "pointer_x", &x) &&
          gst_structure_get_double (structure, "pointer_y", &y)) {
        GST_DEBUG_OBJECT (vf, "converting %fx%f", x, y);
        switch (vf->method) {
          case GST_VIDEO_FLIP_METHOD_90R:
            new_x = y;
            new_y = vf->to_width - x;
            break;
          case GST_VIDEO_FLIP_METHOD_90L:
            new_x = vf->to_height - y;
            new_y = x;
            break;
          case GST_VIDEO_FLIP_METHOD_OTHER:
            new_x = vf->to_height - y;
            new_y = vf->to_width - x;
            break;
          case GST_VIDEO_FLIP_METHOD_TRANS:
            new_x = y;
            new_y = x;
            break;
          case GST_VIDEO_FLIP_METHOD_180:
            new_x = vf->to_width - x;
            new_y = vf->to_height - y;
            break;
          case GST_VIDEO_FLIP_METHOD_HORIZ:
            new_x = vf->to_width - x;
            new_y = y;
            break;
          case GST_VIDEO_FLIP_METHOD_VERT:
            new_x = x;
            new_y = vf->to_height - y;
            break;
          default:
            new_x = x;
            new_y = y;
            break;
        }
        GST_DEBUG_OBJECT (vf, "to %fx%f", x, y);
        gst_structure_set (structure, "pointer_x", G_TYPE_DOUBLE, new_x,
            "pointer_y", G_TYPE_DOUBLE, new_y, NULL);
      }
      break;
    default:
      break;
  }

  ret = gst_pad_event_default (pad, event);

  gst_object_unref (vf);

  return ret;
}

static void
gst_video_flip_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstVideoFlip *videoflip;

  g_return_if_fail (GST_IS_VIDEO_FLIP (object));
  videoflip = GST_VIDEO_FLIP (object);

  switch (prop_id) {
    case ARG_METHOD:
    {
      GstVideoFlipMethod method;

      method = g_value_get_enum (value);
      if (method != videoflip->method) {
        GstBaseTransform *btrans = GST_BASE_TRANSFORM (videoflip);

        g_mutex_lock (btrans->transform_lock);
        gst_pad_set_caps (btrans->sinkpad, NULL);
        gst_pad_set_caps (btrans->srcpad, NULL);
        g_mutex_unlock (btrans->transform_lock);
        videoflip->method = method;
      }
    }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_video_flip_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstVideoFlip *videoflip;

  g_return_if_fail (GST_IS_VIDEO_FLIP (object));
  videoflip = GST_VIDEO_FLIP (object);

  switch (prop_id) {
    case ARG_METHOD:
      g_value_set_enum (value, videoflip->method);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_video_flip_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (element_class, &video_flip_details);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_video_flip_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_video_flip_src_template));
}

static void
gst_video_flip_class_init (gpointer klass, gpointer class_data)
{
  GObjectClass *gobject_class;
  GstBaseTransformClass *trans_class;

  gobject_class = (GObjectClass *) klass;
  trans_class = (GstBaseTransformClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->set_property = gst_video_flip_set_property;
  gobject_class->get_property = gst_video_flip_get_property;

  g_object_class_install_property (gobject_class, ARG_METHOD,
      g_param_spec_enum ("method", "method", "method",
          GST_TYPE_VIDEO_FLIP_METHOD, GST_VIDEO_FLIP_METHOD_90R,
          G_PARAM_READWRITE));

  trans_class->transform_caps =
      GST_DEBUG_FUNCPTR (gst_video_flip_transform_caps);
  trans_class->set_caps = GST_DEBUG_FUNCPTR (gst_video_flip_set_caps);
  trans_class->get_unit_size = GST_DEBUG_FUNCPTR (gst_video_flip_get_unit_size);
  trans_class->transform = GST_DEBUG_FUNCPTR (gst_video_flip_transform);
}

static void
gst_video_flip_init (GTypeInstance * instance, gpointer g_class)
{
  GstVideoFlip *videoflip = GST_VIDEO_FLIP (instance);
  GstBaseTransform *btrans = GST_BASE_TRANSFORM (instance);

  GST_DEBUG_OBJECT (videoflip, "gst_video_flip_init");

  videoflip->method = GST_VIDEO_FLIP_METHOD_90R;

  gst_pad_set_event_function (btrans->srcpad,
      GST_DEBUG_FUNCPTR (gst_video_flip_handle_src_event));
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (video_flip_debug, "videoflip", 0, "videoflip");

  return gst_element_register (plugin, "videoflip", GST_RANK_NONE,
      GST_TYPE_VIDEO_FLIP);
}

GType
gst_video_flip_get_type (void)
{
  static GType video_flip_type = 0;

  if (!video_flip_type) {
    static const GTypeInfo video_flip_info = {
      sizeof (GstVideoFlipClass),
      gst_video_flip_base_init,
      NULL,
      gst_video_flip_class_init,
      NULL,
      NULL,
      sizeof (GstVideoFlip),
      0,
      gst_video_flip_init,
    };

    video_flip_type = g_type_register_static (GST_TYPE_VIDEO_FILTER,
        "GstVideoFlip", &video_flip_info, 0);
  }
  return video_flip_type;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "videoflip",
    "Flips and rotates video",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
