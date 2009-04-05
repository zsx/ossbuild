/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Library       <2002> Ronald Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2007 David A. Schleef <ds@schleef.org>
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

#include "video.h"

/**
 * SECTION:gstvideo
 * @short_description: Support library for video operations
 *
 * <refsect2>
 * <para>
 * This library contains some helper functions and includes the 
 * videosink and videofilter base classes.
 * </para>
 * </refsect2>
 */

static GstVideoFormat gst_video_format_from_rgb32_masks (int red_mask,
    int green_mask, int blue_mask);
static GstVideoFormat gst_video_format_from_rgba32_masks (int red_mask,
    int green_mask, int blue_mask, int alpha_mask);
static GstVideoFormat gst_video_format_from_rgb24_masks (int red_mask,
    int green_mask, int blue_mask);


/**
 * gst_video_frame_rate:
 * @pad: pointer to a #GstPad
 *
 * A convenience function to retrieve a GValue holding the framerate
 * from the caps on a pad.
 * 
 * The pad needs to have negotiated caps containing a framerate property.
 *
 * Returns: NULL if the pad has no configured caps or the configured caps
 * do not contain a framerate.
 *
 */
const GValue *
gst_video_frame_rate (GstPad * pad)
{
  const GValue *fps;
  gchar *fps_string;

  const GstCaps *caps = NULL;
  GstStructure *structure;

  /* get pad caps */
  caps = GST_PAD_CAPS (pad);
  if (caps == NULL) {
    g_warning ("gstvideo: failed to get caps of pad %s:%s",
        GST_DEBUG_PAD_NAME (pad));
    return NULL;
  }

  structure = gst_caps_get_structure (caps, 0);
  if ((fps = gst_structure_get_value (structure, "framerate")) == NULL) {
    g_warning ("gstvideo: failed to get framerate property of pad %s:%s",
        GST_DEBUG_PAD_NAME (pad));
    return NULL;
  }
  if (!GST_VALUE_HOLDS_FRACTION (fps)) {
    g_warning
        ("gstvideo: framerate property of pad %s:%s is not of type Fraction",
        GST_DEBUG_PAD_NAME (pad));
    return NULL;
  }

  fps_string = gst_value_serialize (fps);
  GST_DEBUG ("Framerate request on pad %s:%s: %s",
      GST_DEBUG_PAD_NAME (pad), fps_string);
  g_free (fps_string);

  return fps;
}

/**
 * gst_video_get_size:
 * @pad: pointer to a #GstPad
 * @width: pointer to integer to hold pixel width of the video frames (output)
 * @height: pointer to integer to hold pixel height of the video frames (output)
 *
 * Inspect the caps of the provided pad and retrieve the width and height of
 * the video frames it is configured for.
 * 
 * The pad needs to have negotiated caps containing width and height properties.
 *
 * Returns: TRUE if the width and height could be retrieved.
 *
 */
gboolean
gst_video_get_size (GstPad * pad, gint * width, gint * height)
{
  const GstCaps *caps = NULL;
  GstStructure *structure;
  gboolean ret;

  g_return_val_if_fail (pad != NULL, FALSE);
  g_return_val_if_fail (width != NULL, FALSE);
  g_return_val_if_fail (height != NULL, FALSE);

  caps = GST_PAD_CAPS (pad);

  if (caps == NULL) {
    g_warning ("gstvideo: failed to get caps of pad %s:%s",
        GST_DEBUG_PAD_NAME (pad));
    return FALSE;
  }

  structure = gst_caps_get_structure (caps, 0);
  ret = gst_structure_get_int (structure, "width", width);
  ret &= gst_structure_get_int (structure, "height", height);

  if (!ret) {
    g_warning ("gstvideo: failed to get size properties on pad %s:%s",
        GST_DEBUG_PAD_NAME (pad));
    return FALSE;
  }

  GST_DEBUG ("size request on pad %s:%s: %dx%d",
      GST_DEBUG_PAD_NAME (pad), width ? *width : -1, height ? *height : -1);

  return TRUE;
}

/**
 * gst_video_calculate_display_ratio:
 * @dar_n: Numerator of the calculated display_ratio
 * @dar_d: Denominator of the calculated display_ratio
 * @video_width: Width of the video frame in pixels
 * @video_height: Height of the video frame in pixels
 * @video_par_n: Numerator of the pixel aspect ratio of the input video.
 * @video_par_d: Denominator of the pixel aspect ratio of the input video.
 * @display_par_n: Numerator of the pixel aspect ratio of the display device
 * @display_par_d: Denominator of the pixel aspect ratio of the display device
 *
 * Given the Pixel Aspect Ratio and size of an input video frame, and the 
 * pixel aspect ratio of the intended display device, calculates the actual 
 * display ratio the video will be rendered with.
 *
 * Returns: A boolean indicating success and a calculated Display Ratio in the 
 * dar_n and dar_d parameters. 
 * The return value is FALSE in the case of integer overflow or other error. 
 *
 * Since: 0.10.7
 */
gboolean
gst_video_calculate_display_ratio (guint * dar_n, guint * dar_d,
    guint video_width, guint video_height,
    guint video_par_n, guint video_par_d,
    guint display_par_n, guint display_par_d)
{
  gint num, den;

  GValue display_ratio = { 0, };
  GValue tmp = { 0, };
  GValue tmp2 = { 0, };

  g_return_val_if_fail (dar_n != NULL, FALSE);
  g_return_val_if_fail (dar_d != NULL, FALSE);

  g_value_init (&display_ratio, GST_TYPE_FRACTION);
  g_value_init (&tmp, GST_TYPE_FRACTION);
  g_value_init (&tmp2, GST_TYPE_FRACTION);

  /* Calculate (video_width * video_par_n * display_par_d) /
   * (video_height * video_par_d * display_par_n) */
  gst_value_set_fraction (&display_ratio, video_width, video_height);
  gst_value_set_fraction (&tmp, video_par_n, video_par_d);

  if (!gst_value_fraction_multiply (&tmp2, &display_ratio, &tmp))
    goto error_overflow;

  gst_value_set_fraction (&tmp, display_par_d, display_par_n);

  if (!gst_value_fraction_multiply (&display_ratio, &tmp2, &tmp))
    goto error_overflow;

  num = gst_value_get_fraction_numerator (&display_ratio);
  den = gst_value_get_fraction_denominator (&display_ratio);

  g_value_unset (&display_ratio);
  g_value_unset (&tmp);
  g_value_unset (&tmp2);

  g_return_val_if_fail (num > 0, FALSE);
  g_return_val_if_fail (den > 0, FALSE);

  *dar_n = num;
  *dar_d = den;

  return TRUE;
error_overflow:
  g_value_unset (&display_ratio);
  g_value_unset (&tmp);
  g_value_unset (&tmp2);
  return FALSE;
}

/**
 * gst_video_format_parse_caps:
 * @caps: the #GstCaps to parse
 * @format: the #GstVideoFormat of the video represented by @caps (output)
 * @width: the width of the video represented by @caps, may be NULL (output)
 * @height: the height of the video represented by @caps, may be NULL (output)
 *
 * Determines the #GstVideoFormat of @caps and places it in the location
 * pointed to by @format.  Extracts the size of the video and places it
 * in the location pointed to by @width and @height.  If @caps does not
 * represent one of the raw video formats listed in #GstVideoFormat, the
 * function will fail and return FALSE.
 *
 * Since: 0.10.16
 *
 * Returns: TRUE if @caps was parsed correctly.
 */
gboolean
gst_video_format_parse_caps (GstCaps * caps, GstVideoFormat * format,
    int *width, int *height)
{
  GstStructure *structure;
  gboolean ok = TRUE;

  if (!gst_caps_is_fixed (caps))
    return FALSE;

  structure = gst_caps_get_structure (caps, 0);

  if (format) {
    if (gst_structure_has_name (structure, "video/x-raw-yuv")) {
      guint32 fourcc;

      ok &= gst_structure_get_fourcc (structure, "format", &fourcc);

      *format = gst_video_format_from_fourcc (fourcc);
      if (*format == GST_VIDEO_FORMAT_UNKNOWN) {
        ok = FALSE;
      }
    } else if (gst_structure_has_name (structure, "video/x-raw-rgb")) {
      int depth;
      int bpp;
      int endianness;
      int red_mask;
      int green_mask;
      int blue_mask;
      int alpha_mask;
      gboolean have_alpha;

      ok &= gst_structure_get_int (structure, "depth", &depth);
      ok &= gst_structure_get_int (structure, "bpp", &bpp);
      ok &= gst_structure_get_int (structure, "endianness", &endianness);
      ok &= gst_structure_get_int (structure, "red_mask", &red_mask);
      ok &= gst_structure_get_int (structure, "green_mask", &green_mask);
      ok &= gst_structure_get_int (structure, "blue_mask", &blue_mask);
      have_alpha = gst_structure_get_int (structure, "alpha_mask", &alpha_mask);

      if (depth == 24 && bpp == 32 && endianness == G_BIG_ENDIAN) {
        *format = gst_video_format_from_rgb32_masks (red_mask, green_mask,
            blue_mask);
        if (*format == GST_VIDEO_FORMAT_UNKNOWN) {
          ok = FALSE;
        }
      } else if (depth == 32 && bpp == 32 && endianness == G_BIG_ENDIAN &&
          have_alpha) {
        *format = gst_video_format_from_rgba32_masks (red_mask, green_mask,
            blue_mask, alpha_mask);
        if (*format == GST_VIDEO_FORMAT_UNKNOWN) {
          ok = FALSE;
        }
      } else if (depth == 24 && bpp == 24 && endianness == G_BIG_ENDIAN) {
        *format = gst_video_format_from_rgb24_masks (red_mask, green_mask,
            blue_mask);
        if (*format == GST_VIDEO_FORMAT_UNKNOWN) {
          ok = FALSE;
        }
      } else {
        ok = FALSE;
      }
    } else {
      ok = FALSE;
    }
  }

  if (width) {
    ok &= gst_structure_get_int (structure, "width", width);
  }

  if (height) {
    ok &= gst_structure_get_int (structure, "height", height);
  }

  return ok;
}

/**
 * gst_video_parse_caps_framerate:
 * @caps: pointer to a #GstCaps instance
 * @fps_n: pointer to integer to hold numerator of frame rate (output)
 * @fps_d: pointer to integer to hold denominator of frame rate (output)
 *
 * Extracts the frame rate from @caps and places the values in the locations
 * pointed to by @fps_n and @fps_d.  Returns TRUE if the values could be
 * parsed correctly, FALSE if not.
 *
 * This function can be used with #GstCaps that have any media type; it
 * is not limited to formats handled by #GstVideoFormat.
 *
 * Since: 0.10.16
 *
 * Returns: TRUE if @caps was parsed correctly.
 */
gboolean
gst_video_parse_caps_framerate (GstCaps * caps, int *fps_n, int *fps_d)
{
  GstStructure *structure;

  if (!gst_caps_is_fixed (caps))
    return FALSE;

  structure = gst_caps_get_structure (caps, 0);

  return gst_structure_get_fraction (structure, "framerate", fps_n, fps_d);
}

/**
 * gst_video_parse_caps_pixel_aspect_ratio:
 * @caps: pointer to a #GstCaps instance
 * @par_n: pointer to numerator of pixel aspect ratio (output)
 * @par_d: pointer to denominator of pixel aspect ratio (output)
 *
 * Extracts the pixel aspect ratio from @caps and places the values in
 * the locations pointed to by @par_n and @par_d.  Returns TRUE if the
 * values could be parsed correctly, FALSE if not.
 *
 * This function can be used with #GstCaps that have any media type; it
 * is not limited to formats handled by #GstVideoFormat.
 *
 * Since: 0.10.16
 *
 * Returns: TRUE if @caps was parsed correctly.
 */
gboolean
gst_video_parse_caps_pixel_aspect_ratio (GstCaps * caps, int *par_n, int *par_d)
{
  GstStructure *structure;

  if (!gst_caps_is_fixed (caps))
    return FALSE;

  structure = gst_caps_get_structure (caps, 0);

  if (!gst_structure_get_fraction (structure, "pixel-aspect-ratio",
          par_n, par_d)) {
    *par_n = 1;
    *par_d = 1;
  }
  return TRUE;
}

/**
 * gst_video_format_new_caps:
 * @format: the #GstVideoFormat describing the raw video format
 * @width: width of video
 * @height: height of video
 * @framerate_n: numerator of frame rate
 * @framerate_d: denominator of frame rate
 * @par_n: numerator of pixel aspect ratio
 * @par_d: denominator of pixel aspect ratio
 *
 * Creates a new #GstCaps object based on the parameters provided.
 *
 * Since: 0.10.16
 *
 * Returns: a new #GstCaps object, or NULL if there was an error
 */
GstCaps *
gst_video_format_new_caps (GstVideoFormat format, int width, int height,
    int framerate_n, int framerate_d, int par_n, int par_d)
{
  g_return_val_if_fail (format != GST_VIDEO_FORMAT_UNKNOWN, NULL);
  g_return_val_if_fail (width > 0 && height > 0, NULL);

  if (gst_video_format_is_yuv (format)) {
    return gst_caps_new_simple ("video/x-raw-yuv",
        "format", GST_TYPE_FOURCC, gst_video_format_to_fourcc (format),
        "width", G_TYPE_INT, width,
        "height", G_TYPE_INT, height,
        "framerate", GST_TYPE_FRACTION, framerate_n, framerate_d,
        "pixel-aspect-ratio", GST_TYPE_FRACTION, par_n, par_d, NULL);
  }
  if (gst_video_format_is_rgb (format)) {
    GstCaps *caps;
    int red_mask;
    int blue_mask;
    int green_mask;
    int alpha_mask;
    int depth;
    int bpp;
    gboolean have_alpha;
    unsigned int mask;

    switch (format) {
      case GST_VIDEO_FORMAT_RGBx:
      case GST_VIDEO_FORMAT_BGRx:
      case GST_VIDEO_FORMAT_xRGB:
      case GST_VIDEO_FORMAT_xBGR:
        bpp = 32;
        depth = 24;
        have_alpha = FALSE;
        break;
      case GST_VIDEO_FORMAT_RGBA:
      case GST_VIDEO_FORMAT_BGRA:
      case GST_VIDEO_FORMAT_ARGB:
      case GST_VIDEO_FORMAT_ABGR:
        bpp = 32;
        depth = 32;
        have_alpha = TRUE;
        break;
      case GST_VIDEO_FORMAT_RGB:
      case GST_VIDEO_FORMAT_BGR:
        bpp = 24;
        depth = 24;
        have_alpha = FALSE;
        break;
      default:
        return NULL;
    }
    if (bpp == 32) {
      mask = 0xff000000;
    } else {
      mask = 0xff0000;
    }
    red_mask =
        mask >> (8 * gst_video_format_get_component_offset (format, 0, width,
            height));
    green_mask =
        mask >> (8 * gst_video_format_get_component_offset (format, 1, width,
            height));
    blue_mask =
        mask >> (8 * gst_video_format_get_component_offset (format, 2, width,
            height));

    caps = gst_caps_new_simple ("video/x-raw-rgb",
        "bpp", G_TYPE_INT, bpp,
        "depth", G_TYPE_INT, depth,
        "endianness", G_TYPE_INT, G_BIG_ENDIAN,
        "red_mask", G_TYPE_INT, red_mask,
        "green_mask", G_TYPE_INT, green_mask,
        "blue_mask", G_TYPE_INT, blue_mask,
        "width", G_TYPE_INT, width,
        "height", G_TYPE_INT, height,
        "framerate", GST_TYPE_FRACTION, framerate_n, framerate_d,
        "pixel-aspect-ratio", GST_TYPE_FRACTION, par_n, par_d, NULL);
    if (have_alpha) {
      alpha_mask =
          mask >> (8 * gst_video_format_get_component_offset (format, 3, width,
              height));
      gst_caps_set_simple (caps, "alpha_mask", G_TYPE_INT, alpha_mask, NULL);
    }
    return caps;
  }
  return NULL;
}

/**
 * gst_video_format_from_fourcc:
 * @fourcc: a FOURCC value representing raw YUV video
 *
 * Converts a FOURCC value into the corresponding #GstVideoFormat.
 * If the FOURCC cannot be represented by #GstVideoFormat,
 * #GST_VIDEO_FORMAT_UNKNOWN is returned.
 *
 * Since: 0.10.16
 *
 * Returns: the #GstVideoFormat describing the FOURCC value
 */
GstVideoFormat
gst_video_format_from_fourcc (guint32 fourcc)
{
  switch (fourcc) {
    case GST_MAKE_FOURCC ('I', '4', '2', '0'):
      return GST_VIDEO_FORMAT_I420;
    case GST_MAKE_FOURCC ('Y', 'V', '1', '2'):
      return GST_VIDEO_FORMAT_YV12;
    case GST_MAKE_FOURCC ('Y', 'U', 'Y', '2'):
      return GST_VIDEO_FORMAT_YUY2;
    case GST_MAKE_FOURCC ('U', 'Y', 'V', 'Y'):
      return GST_VIDEO_FORMAT_UYVY;
    case GST_MAKE_FOURCC ('A', 'Y', 'U', 'V'):
      return GST_VIDEO_FORMAT_AYUV;
    case GST_MAKE_FOURCC ('Y', '4', '1', 'B'):
      return GST_VIDEO_FORMAT_Y41B;
    case GST_MAKE_FOURCC ('Y', '4', '2', 'B'):
      return GST_VIDEO_FORMAT_Y42B;
    default:
      return GST_VIDEO_FORMAT_UNKNOWN;
  }
}

/**
 * gst_video_format_to_fourcc:
 * @format: a #GstVideoFormat video format
 *
 * Converts a #GstVideoFormat value into the corresponding FOURCC.  Only
 * a few YUV formats have corresponding FOURCC values.  If @format has
 * no corresponding FOURCC value, 0 is returned.
 *
 * Since: 0.10.16
 *
 * Returns: the FOURCC corresponding to @format
 */
guint32
gst_video_format_to_fourcc (GstVideoFormat format)
{
  g_return_val_if_fail (format != GST_VIDEO_FORMAT_UNKNOWN, 0);

  switch (format) {
    case GST_VIDEO_FORMAT_I420:
      return GST_MAKE_FOURCC ('I', '4', '2', '0');
    case GST_VIDEO_FORMAT_YV12:
      return GST_MAKE_FOURCC ('Y', 'V', '1', '2');
    case GST_VIDEO_FORMAT_YUY2:
      return GST_MAKE_FOURCC ('Y', 'U', 'Y', '2');
    case GST_VIDEO_FORMAT_UYVY:
      return GST_MAKE_FOURCC ('U', 'Y', 'V', 'Y');
    case GST_VIDEO_FORMAT_AYUV:
      return GST_MAKE_FOURCC ('A', 'Y', 'U', 'V');
    case GST_VIDEO_FORMAT_Y41B:
      return GST_MAKE_FOURCC ('Y', '4', '1', 'B');
    case GST_VIDEO_FORMAT_Y42B:
      return GST_MAKE_FOURCC ('Y', '4', '2', 'B');
    default:
      return 0;
  }
}

/*
 * gst_video_format_from_rgb32_masks:
 * @red_mask: red bit mask
 * @green_mask: green bit mask
 * @blue_mask: blue bit mask
 *
 * Converts red, green, blue bit masks into the corresponding
 * #GstVideoFormat.  
 *
 * Since: 0.10.16
 *
 * Returns: the #GstVideoFormat corresponding to the bit masks
 */
static GstVideoFormat
gst_video_format_from_rgb32_masks (int red_mask, int green_mask, int blue_mask)
{
  if (red_mask == 0xff000000 && green_mask == 0x00ff0000 &&
      blue_mask == 0x0000ff00) {
    return GST_VIDEO_FORMAT_RGBx;
  }
  if (red_mask == 0x0000ff00 && green_mask == 0x00ff0000 &&
      blue_mask == 0xff000000) {
    return GST_VIDEO_FORMAT_BGRx;
  }
  if (red_mask == 0x00ff0000 && green_mask == 0x0000ff00 &&
      blue_mask == 0x000000ff) {
    return GST_VIDEO_FORMAT_xRGB;
  }
  if (red_mask == 0x000000ff && green_mask == 0x0000ff00 &&
      blue_mask == 0x00ff0000) {
    return GST_VIDEO_FORMAT_xBGR;
  }

  return GST_VIDEO_FORMAT_UNKNOWN;
}

static GstVideoFormat
gst_video_format_from_rgba32_masks (int red_mask, int green_mask, int blue_mask,
    int alpha_mask)
{
  if (red_mask == 0xff000000 && green_mask == 0x00ff0000 &&
      blue_mask == 0x0000ff00 && alpha_mask == 0x000000ff) {
    return GST_VIDEO_FORMAT_RGBA;
  }
  if (red_mask == 0x0000ff00 && green_mask == 0x00ff0000 &&
      blue_mask == 0xff000000 && alpha_mask == 0x000000ff) {
    return GST_VIDEO_FORMAT_BGRA;
  }
  if (red_mask == 0x00ff0000 && green_mask == 0x0000ff00 &&
      blue_mask == 0x000000ff && alpha_mask == 0xff000000) {
    return GST_VIDEO_FORMAT_ARGB;
  }
  if (red_mask == 0x000000ff && green_mask == 0x0000ff00 &&
      blue_mask == 0x00ff0000 && alpha_mask == 0xff000000) {
    return GST_VIDEO_FORMAT_ABGR;
  }

  return GST_VIDEO_FORMAT_UNKNOWN;
}

static GstVideoFormat
gst_video_format_from_rgb24_masks (int red_mask, int green_mask, int blue_mask)
{
  if (red_mask == 0xff0000 && green_mask == 0x00ff00 && blue_mask == 0x0000ff) {
    return GST_VIDEO_FORMAT_RGB;
  }
  if (red_mask == 0x0000ff && green_mask == 0x00ff00 && blue_mask == 0xff0000) {
    return GST_VIDEO_FORMAT_BGR;
  }

  return GST_VIDEO_FORMAT_UNKNOWN;
}

/**
 * gst_video_format_is_rgb:
 * @format: a #GstVideoFormat
 *
 * Determine whether the video format is an RGB format.
 *
 * Since: 0.10.16
 *
 * Returns: TRUE if @format represents RGB video
 */
gboolean
gst_video_format_is_rgb (GstVideoFormat format)
{
  switch (format) {
    case GST_VIDEO_FORMAT_I420:
    case GST_VIDEO_FORMAT_YV12:
    case GST_VIDEO_FORMAT_YUY2:
    case GST_VIDEO_FORMAT_UYVY:
    case GST_VIDEO_FORMAT_AYUV:
    case GST_VIDEO_FORMAT_Y41B:
    case GST_VIDEO_FORMAT_Y42B:
      return FALSE;
    case GST_VIDEO_FORMAT_RGBx:
    case GST_VIDEO_FORMAT_BGRx:
    case GST_VIDEO_FORMAT_xRGB:
    case GST_VIDEO_FORMAT_xBGR:
    case GST_VIDEO_FORMAT_RGBA:
    case GST_VIDEO_FORMAT_BGRA:
    case GST_VIDEO_FORMAT_ARGB:
    case GST_VIDEO_FORMAT_ABGR:
    case GST_VIDEO_FORMAT_RGB:
    case GST_VIDEO_FORMAT_BGR:
      return TRUE;
    default:
      return FALSE;
  }
}

/**
 * gst_video_format_is_yuv:
 * @format: a #GstVideoFormat
 *
 * Determine whether the video format is a YUV format.
 *
 * Since: 0.10.16
 *
 * Returns: TRUE if @format represents YUV video
 */
gboolean
gst_video_format_is_yuv (GstVideoFormat format)
{
  switch (format) {
    case GST_VIDEO_FORMAT_I420:
    case GST_VIDEO_FORMAT_YV12:
    case GST_VIDEO_FORMAT_YUY2:
    case GST_VIDEO_FORMAT_UYVY:
    case GST_VIDEO_FORMAT_AYUV:
    case GST_VIDEO_FORMAT_Y41B:
    case GST_VIDEO_FORMAT_Y42B:
      return TRUE;
    case GST_VIDEO_FORMAT_RGBx:
    case GST_VIDEO_FORMAT_BGRx:
    case GST_VIDEO_FORMAT_xRGB:
    case GST_VIDEO_FORMAT_xBGR:
    case GST_VIDEO_FORMAT_RGBA:
    case GST_VIDEO_FORMAT_BGRA:
    case GST_VIDEO_FORMAT_ARGB:
    case GST_VIDEO_FORMAT_ABGR:
    case GST_VIDEO_FORMAT_RGB:
    case GST_VIDEO_FORMAT_BGR:
      return FALSE;
    default:
      return FALSE;
  }
}

/**
 * gst_video_format_has_alpha:
 * @format: a #GstVideoFormat
 * 
 * Returns TRUE or FALSE depending on if the video format provides an
 * alpha channel.
 *
 * Since: 0.10.16
 *
 * Returns: TRUE if @format has an alpha channel
 */
gboolean
gst_video_format_has_alpha (GstVideoFormat format)
{
  switch (format) {
    case GST_VIDEO_FORMAT_I420:
    case GST_VIDEO_FORMAT_YV12:
    case GST_VIDEO_FORMAT_YUY2:
    case GST_VIDEO_FORMAT_UYVY:
    case GST_VIDEO_FORMAT_Y41B:
    case GST_VIDEO_FORMAT_Y42B:
      return FALSE;
    case GST_VIDEO_FORMAT_AYUV:
    case GST_VIDEO_FORMAT_RGBA:
    case GST_VIDEO_FORMAT_BGRA:
    case GST_VIDEO_FORMAT_ARGB:
    case GST_VIDEO_FORMAT_ABGR:
      return TRUE;
    case GST_VIDEO_FORMAT_RGBx:
    case GST_VIDEO_FORMAT_BGRx:
    case GST_VIDEO_FORMAT_xRGB:
    case GST_VIDEO_FORMAT_xBGR:
    case GST_VIDEO_FORMAT_RGB:
    case GST_VIDEO_FORMAT_BGR:
      return FALSE;
    default:
      return FALSE;
  }
}

/**
 * gst_video_format_get_row_stride:
 * @format: a #GstVideoFormat
 * @component: the component index
 * @width: the width of video
 *
 * Calculates the row stride (number of bytes from one row of pixels to
 * the next) for the video component with an index of @component.  For
 * YUV video, Y, U, and V have component indices of 0, 1, and 2,
 * respectively.  For RGB video, R, G, and B have component indicies of
 * 0, 1, and 2, respectively.  Alpha channels, if present, have a component
 * index of 3.  The @width parameter always represents the width of the
 * video, not the component.
 *
 * Since: 0.10.16
 *
 * Returns: row stride of component @component
 */
int
gst_video_format_get_row_stride (GstVideoFormat format, int component,
    int width)
{
  g_return_val_if_fail (format != GST_VIDEO_FORMAT_UNKNOWN, 0);
  g_return_val_if_fail (component >= 0 && component <= 3, 0);
  g_return_val_if_fail (width > 0, 0);

  switch (format) {
    case GST_VIDEO_FORMAT_I420:
    case GST_VIDEO_FORMAT_YV12:
      if (component == 0) {
        return GST_ROUND_UP_4 (width);
      } else {
        return GST_ROUND_UP_4 (GST_ROUND_UP_2 (width) / 2);
      }
    case GST_VIDEO_FORMAT_YUY2:
    case GST_VIDEO_FORMAT_UYVY:
      return GST_ROUND_UP_4 (width * 2);
    case GST_VIDEO_FORMAT_AYUV:
    case GST_VIDEO_FORMAT_RGBx:
    case GST_VIDEO_FORMAT_BGRx:
    case GST_VIDEO_FORMAT_xRGB:
    case GST_VIDEO_FORMAT_xBGR:
    case GST_VIDEO_FORMAT_RGBA:
    case GST_VIDEO_FORMAT_BGRA:
    case GST_VIDEO_FORMAT_ARGB:
    case GST_VIDEO_FORMAT_ABGR:
      return width * 4;
    case GST_VIDEO_FORMAT_RGB:
    case GST_VIDEO_FORMAT_BGR:
      return GST_ROUND_UP_4 (width * 3);
    case GST_VIDEO_FORMAT_Y41B:
      if (component == 0) {
        return GST_ROUND_UP_4 (width);
      } else {
        return GST_ROUND_UP_8 (width) / 4;
      }
    case GST_VIDEO_FORMAT_Y42B:
      if (component == 0) {
        return GST_ROUND_UP_4 (width);
      } else {
        return GST_ROUND_UP_8 (width) / 2;
      }
    default:
      return 0;
  }
}

/**
 * gst_video_format_get_pixel_stride:
 * @format: a #GstVideoFormat
 * @component: the component index
 *
 * Calculates the pixel stride (number of bytes from one pixel to the
 * pixel to its immediate left) for the video component with an index
 * of @component.  See @gst_video_format_get_row_stride for a description
 * of the component index.
 *
 * Since: 0.10.16
 *
 * Returns: pixel stride of component @component
 */
int
gst_video_format_get_pixel_stride (GstVideoFormat format, int component)
{
  g_return_val_if_fail (format != GST_VIDEO_FORMAT_UNKNOWN, 0);
  g_return_val_if_fail (component >= 0 && component <= 3, 0);

  switch (format) {
    case GST_VIDEO_FORMAT_I420:
    case GST_VIDEO_FORMAT_YV12:
    case GST_VIDEO_FORMAT_Y41B:
    case GST_VIDEO_FORMAT_Y42B:
      return 1;
    case GST_VIDEO_FORMAT_YUY2:
    case GST_VIDEO_FORMAT_UYVY:
      if (component == 0) {
        return 2;
      } else {
        return 4;
      }
    case GST_VIDEO_FORMAT_AYUV:
    case GST_VIDEO_FORMAT_RGBx:
    case GST_VIDEO_FORMAT_BGRx:
    case GST_VIDEO_FORMAT_xRGB:
    case GST_VIDEO_FORMAT_xBGR:
    case GST_VIDEO_FORMAT_RGBA:
    case GST_VIDEO_FORMAT_BGRA:
    case GST_VIDEO_FORMAT_ARGB:
    case GST_VIDEO_FORMAT_ABGR:
      return 4;
    case GST_VIDEO_FORMAT_RGB:
    case GST_VIDEO_FORMAT_BGR:
      return 3;
    default:
      return 0;
  }
}

/**
 * gst_video_format_get_component_width:
 * @format: a #GstVideoFormat
 * @component: the component index
 * @width: the width of video
 *
 * Calculates the width of the component.  See
 * @gst_video_format_get_row_stride for a description
 * of the component index.
 *
 * Since: 0.10.16
 *
 * Returns: width of component @component
 */
int
gst_video_format_get_component_width (GstVideoFormat format, int component,
    int width)
{
  g_return_val_if_fail (format != GST_VIDEO_FORMAT_UNKNOWN, 0);
  g_return_val_if_fail (component >= 0 && component <= 3, 0);
  g_return_val_if_fail (width > 0, 0);

  switch (format) {
    case GST_VIDEO_FORMAT_I420:
    case GST_VIDEO_FORMAT_YV12:
    case GST_VIDEO_FORMAT_YUY2:
    case GST_VIDEO_FORMAT_UYVY:
      if (component == 0) {
        return width;
      } else {
        return GST_ROUND_UP_2 (width) / 2;
      }
    case GST_VIDEO_FORMAT_Y41B:        /* CHECKME: component_width for Y41B */
      if (component == 0) {
        return width;
      } else {
        return GST_ROUND_UP_8 (width) / 4;
      }
    case GST_VIDEO_FORMAT_Y42B:        /* CHECKME: component_width for Y42B */
      if (component == 0) {
        return width;
      } else {
        return GST_ROUND_UP_8 (width) / 2;
      }
    case GST_VIDEO_FORMAT_AYUV:
    case GST_VIDEO_FORMAT_RGBx:
    case GST_VIDEO_FORMAT_BGRx:
    case GST_VIDEO_FORMAT_xRGB:
    case GST_VIDEO_FORMAT_xBGR:
    case GST_VIDEO_FORMAT_RGBA:
    case GST_VIDEO_FORMAT_BGRA:
    case GST_VIDEO_FORMAT_ARGB:
    case GST_VIDEO_FORMAT_ABGR:
    case GST_VIDEO_FORMAT_RGB:
    case GST_VIDEO_FORMAT_BGR:
      return width;
    default:
      return 0;
  }
}

/**
 * gst_video_format_get_component_height:
 * @format: a #GstVideoFormat
 * @component: the component index
 * @height: the height of video
 *
 * Calculates the height of the component.  See
 * @gst_video_format_get_row_stride for a description
 * of the component index.
 *
 * Since: 0.10.16
 *
 * Returns: height of component @component
 */
int
gst_video_format_get_component_height (GstVideoFormat format, int component,
    int height)
{
  g_return_val_if_fail (format != GST_VIDEO_FORMAT_UNKNOWN, 0);
  g_return_val_if_fail (component >= 0 && component <= 3, 0);
  g_return_val_if_fail (height > 0, 0);

  switch (format) {
    case GST_VIDEO_FORMAT_I420:
    case GST_VIDEO_FORMAT_YV12:
      if (component == 0) {
        return height;
      } else {
        return GST_ROUND_UP_2 (height) / 2;
      }
    case GST_VIDEO_FORMAT_Y41B:
    case GST_VIDEO_FORMAT_Y42B:
    case GST_VIDEO_FORMAT_YUY2:
    case GST_VIDEO_FORMAT_UYVY:
    case GST_VIDEO_FORMAT_AYUV:
    case GST_VIDEO_FORMAT_RGBx:
    case GST_VIDEO_FORMAT_BGRx:
    case GST_VIDEO_FORMAT_xRGB:
    case GST_VIDEO_FORMAT_xBGR:
    case GST_VIDEO_FORMAT_RGBA:
    case GST_VIDEO_FORMAT_BGRA:
    case GST_VIDEO_FORMAT_ARGB:
    case GST_VIDEO_FORMAT_ABGR:
    case GST_VIDEO_FORMAT_RGB:
    case GST_VIDEO_FORMAT_BGR:
      return height;
    default:
      return 0;
  }
}

/**
 * gst_video_format_get_component_offset:
 * @format: a #GstVideoFormat
 * @component: the component index
 * @width: the width of video
 * @height: the height of video
 *
 * Calculates the offset (in bytes) of the first pixel of the component
 * with index @component.  For packed formats, this will typically be a
 * small integer (0, 1, 2, 3).  For planar formats, this will be a
 * (relatively) large offset to the beginning of the second or third
 * component planes.  See @gst_video_format_get_row_stride for a description
 * of the component index.
 *
 * Since: 0.10.16
 *
 * Returns: offset of component @component
 */
int
gst_video_format_get_component_offset (GstVideoFormat format, int component,
    int width, int height)
{
  g_return_val_if_fail (format != GST_VIDEO_FORMAT_UNKNOWN, 0);
  g_return_val_if_fail (component >= 0 && component <= 3, 0);
  g_return_val_if_fail (width > 0 && height > 0, 0);

  switch (format) {
    case GST_VIDEO_FORMAT_I420:
      if (component == 0)
        return 0;
      if (component == 1)
        return GST_ROUND_UP_4 (width) * GST_ROUND_UP_2 (height);
      if (component == 2) {
        return GST_ROUND_UP_4 (width) * GST_ROUND_UP_2 (height) +
            GST_ROUND_UP_4 (GST_ROUND_UP_2 (width) / 2) *
            (GST_ROUND_UP_2 (height) / 2);
      }
      return 0;
    case GST_VIDEO_FORMAT_YV12:        /* same as I420, but components 1+2 swapped */
      if (component == 0)
        return 0;
      if (component == 2)
        return GST_ROUND_UP_4 (width) * GST_ROUND_UP_2 (height);
      if (component == 1) {
        return GST_ROUND_UP_4 (width) * GST_ROUND_UP_2 (height) +
            GST_ROUND_UP_4 (GST_ROUND_UP_2 (width) / 2) *
            (GST_ROUND_UP_2 (height) / 2);
      }
      return 0;
    case GST_VIDEO_FORMAT_YUY2:
      if (component == 0)
        return 0;
      if (component == 1)
        return 1;
      if (component == 2)
        return 3;
      return 0;
    case GST_VIDEO_FORMAT_UYVY:
      if (component == 0)
        return 1;
      if (component == 1)
        return 0;
      if (component == 2)
        return 2;
      return 0;
    case GST_VIDEO_FORMAT_AYUV:
      if (component == 0)
        return 1;
      if (component == 1)
        return 2;
      if (component == 2)
        return 3;
      if (component == 3)
        return 0;
      return 0;
    case GST_VIDEO_FORMAT_RGBx:
    case GST_VIDEO_FORMAT_RGBA:
      if (component == 0)
        return 0;
      if (component == 1)
        return 1;
      if (component == 2)
        return 2;
      if (component == 3)
        return 3;
      return 0;
    case GST_VIDEO_FORMAT_BGRx:
    case GST_VIDEO_FORMAT_BGRA:
      if (component == 0)
        return 2;
      if (component == 1)
        return 1;
      if (component == 2)
        return 0;
      if (component == 3)
        return 3;
      return 0;
    case GST_VIDEO_FORMAT_xRGB:
    case GST_VIDEO_FORMAT_ARGB:
      if (component == 0)
        return 1;
      if (component == 1)
        return 2;
      if (component == 2)
        return 3;
      if (component == 3)
        return 0;
      return 0;
    case GST_VIDEO_FORMAT_xBGR:
    case GST_VIDEO_FORMAT_ABGR:
      if (component == 0)
        return 3;
      if (component == 1)
        return 2;
      if (component == 2)
        return 1;
      if (component == 3)
        return 0;
      return 0;
    case GST_VIDEO_FORMAT_RGB:
      if (component == 0)
        return 0;
      if (component == 1)
        return 1;
      if (component == 2)
        return 2;
      return 0;
    case GST_VIDEO_FORMAT_BGR:
      if (component == 0)
        return 2;
      if (component == 1)
        return 1;
      if (component == 2)
        return 0;
      return 0;
    case GST_VIDEO_FORMAT_Y41B:
      if (component == 0)
        return 0;
      if (component == 1)
        return GST_ROUND_UP_4 (width) * height;
      if (component == 2)
        return (GST_ROUND_UP_4 (width) + (GST_ROUND_UP_8 (width) / 4)) * height;
      return 0;
    case GST_VIDEO_FORMAT_Y42B:
      if (component == 0)
        return 0;
      if (component == 1)
        return GST_ROUND_UP_4 (width) * height;
      if (component == 2)
        return (GST_ROUND_UP_4 (width) + (GST_ROUND_UP_8 (width) / 2)) * height;
      return 0;
    default:
      return 0;
  }
}

/**
 * gst_video_format_get_size:
 * @format: a #GstVideoFormat
 * @width: the width of video
 * @height: the height of video
 *
 * Calculates the total number of bytes in the raw video format.  This
 * number should be used when allocating a buffer for raw video.
 *
 * Since: 0.10.16
 *
 * Returns: size (in bytes) of raw video format
 */
int
gst_video_format_get_size (GstVideoFormat format, int width, int height)
{
  int size;

  g_return_val_if_fail (format != GST_VIDEO_FORMAT_UNKNOWN, 0);
  g_return_val_if_fail (width > 0 && height > 0, 0);

  switch (format) {
    case GST_VIDEO_FORMAT_I420:
    case GST_VIDEO_FORMAT_YV12:
      size = GST_ROUND_UP_4 (width) * GST_ROUND_UP_2 (height);
      size += GST_ROUND_UP_4 (GST_ROUND_UP_2 (width) / 2) *
          (GST_ROUND_UP_2 (height) / 2) * 2;
      return size;
    case GST_VIDEO_FORMAT_YUY2:
    case GST_VIDEO_FORMAT_UYVY:
      return GST_ROUND_UP_4 (width * 2) * height;
    case GST_VIDEO_FORMAT_AYUV:
    case GST_VIDEO_FORMAT_RGBx:
    case GST_VIDEO_FORMAT_BGRx:
    case GST_VIDEO_FORMAT_xRGB:
    case GST_VIDEO_FORMAT_xBGR:
    case GST_VIDEO_FORMAT_RGBA:
    case GST_VIDEO_FORMAT_BGRA:
    case GST_VIDEO_FORMAT_ARGB:
    case GST_VIDEO_FORMAT_ABGR:
      return width * 4 * height;
    case GST_VIDEO_FORMAT_RGB:
    case GST_VIDEO_FORMAT_BGR:
      return GST_ROUND_UP_4 (width * 3) * height;
    case GST_VIDEO_FORMAT_Y41B:
      /* simplification of ROUNDUP4(w)*h + 2*((ROUNDUP8(w)/4)*h */
      return (GST_ROUND_UP_4 (width) + (GST_ROUND_UP_8 (width) / 2)) * height;
    case GST_VIDEO_FORMAT_Y42B:
      /* simplification of ROUNDUP4(w)*h + 2*(ROUNDUP8(w)/2)*h: */
      return (GST_ROUND_UP_4 (width) + GST_ROUND_UP_8 (width)) * height;
    default:
      return 0;
  }
}

/**
 * gst_video_format_convert:
 * @format: a #GstVideoFormat
 * @width: the width of video
 * @height: the height of video
 * @fps_n: frame rate numerator
 * @fps_d: frame rate denominator
 * @src_format: #GstFormat of the @src_value
 * @src_value: value to convert
 * @dest_format: #GstFormat of the @dest_value
 * @dest_value: pointer to destination value
 *
 * Converts among various #GstFormat types.  This function handles
 * GST_FORMAT_BYTES, GST_FORMAT_TIME, and GST_FORMAT_DEFAULT.  For
 * raw video, GST_FORMAT_DEFAULT corresponds to video frames.  This
 * function can be to handle pad queries of the type GST_QUERY_CONVERT.
 *
 * Since: 0.10.16
 *
 * Returns: TRUE if the conversion was successful.
 */
gboolean
gst_video_format_convert (GstVideoFormat format, int width, int height,
    int fps_n, int fps_d,
    GstFormat src_format, gint64 src_value,
    GstFormat dest_format, gint64 * dest_value)
{
  gboolean ret = FALSE;
  int size;

  g_return_val_if_fail (format != GST_VIDEO_FORMAT_UNKNOWN, 0);
  g_return_val_if_fail (width > 0 && height > 0, 0);

  size = gst_video_format_get_size (format, width, height);

  GST_DEBUG ("converting value %" G_GINT64_FORMAT " from %s to %s",
      src_value, gst_format_get_name (src_format),
      gst_format_get_name (dest_format));

  if (src_format == dest_format) {
    *dest_value = src_value;
    ret = TRUE;
    goto done;
  }

  if (src_value == -1) {
    *dest_value = -1;
    ret = TRUE;
    goto done;
  }

  /* bytes to frames */
  if (src_format == GST_FORMAT_BYTES && dest_format == GST_FORMAT_DEFAULT) {
    if (size != 0) {
      *dest_value = gst_util_uint64_scale_int (src_value, 1, size);
    } else {
      GST_ERROR ("blocksize is 0");
      *dest_value = 0;
    }
    ret = TRUE;
    goto done;
  }

  /* frames to bytes */
  if (src_format == GST_FORMAT_DEFAULT && dest_format == GST_FORMAT_BYTES) {
    *dest_value = gst_util_uint64_scale_int (src_value, size, 1);
    ret = TRUE;
    goto done;
  }

  /* time to frames */
  if (src_format == GST_FORMAT_TIME && dest_format == GST_FORMAT_DEFAULT) {
    if (fps_d != 0) {
      *dest_value = gst_util_uint64_scale (src_value,
          fps_n, GST_SECOND * fps_d);
    } else {
      GST_ERROR ("framerate denominator is 0");
      *dest_value = 0;
    }
    ret = TRUE;
    goto done;
  }

  /* frames to time */
  if (src_format == GST_FORMAT_DEFAULT && dest_format == GST_FORMAT_TIME) {
    if (fps_n != 0) {
      *dest_value = gst_util_uint64_scale (src_value,
          GST_SECOND * fps_d, fps_n);
    } else {
      GST_ERROR ("framerate numerator is 0");
      *dest_value = 0;
    }
    ret = TRUE;
    goto done;
  }

  /* time to bytes */
  if (src_format == GST_FORMAT_TIME && dest_format == GST_FORMAT_BYTES) {
    if (fps_d != 0) {
      *dest_value = gst_util_uint64_scale (src_value,
          fps_n * size, GST_SECOND * fps_d);
    } else {
      GST_ERROR ("framerate denominator is 0");
      *dest_value = 0;
    }
    ret = TRUE;
    goto done;
  }

  /* bytes to time */
  if (src_format == GST_FORMAT_BYTES && dest_format == GST_FORMAT_TIME) {
    if (fps_n != 0 && size != 0) {
      *dest_value = gst_util_uint64_scale (src_value,
          GST_SECOND * fps_d, fps_n * size);
    } else {
      GST_ERROR ("framerate denominator and/or blocksize is 0");
      *dest_value = 0;
    }
    ret = TRUE;
  }

done:

  GST_DEBUG ("ret=%d result %" G_GINT64_FORMAT, ret, *dest_value);

  return ret;
}
