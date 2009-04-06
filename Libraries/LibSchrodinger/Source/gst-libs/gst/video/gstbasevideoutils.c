/* GStreamer
 * Copyright (C) 2008 David Schleef <ds@schleef.org>
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
#include "config.h"
#endif

#include "gstbasevideoutils.h"

#include <string.h>


guint64
gst_base_video_convert_bytes_to_frames (GstVideoState *state,
    guint64 bytes)
{
  return gst_util_uint64_scale_int (bytes, 1, state->bytes_per_picture);
}

guint64
gst_base_video_convert_frames_to_bytes (GstVideoState *state,
    guint64 frames)
{
  return frames * state->bytes_per_picture;
}


gboolean
gst_base_video_rawvideo_convert (GstVideoState *state,
    GstFormat src_format, gint64 src_value,
    GstFormat * dest_format, gint64 *dest_value)
{
  gboolean res = FALSE;

  if (src_format == *dest_format) {
    *dest_value = src_value;
    return TRUE;
  }

  if (src_format == GST_FORMAT_BYTES &&
      *dest_format == GST_FORMAT_DEFAULT &&
      state->bytes_per_picture != 0) {
    /* convert bytes to frames */
    *dest_value = gst_util_uint64_scale_int (src_value, 1,
        state->bytes_per_picture);
    res = TRUE;
  } else if (src_format == GST_FORMAT_DEFAULT &&
      *dest_format == GST_FORMAT_BYTES &&
      state->bytes_per_picture != 0) {
    /* convert bytes to frames */
    *dest_value = src_value * state->bytes_per_picture;
    res = TRUE;
  } else if (src_format == GST_FORMAT_DEFAULT &&
      *dest_format == GST_FORMAT_TIME &&
      state->fps_n != 0) {
    /* convert frames to time */
    /* FIXME add segment time? */
    *dest_value = gst_util_uint64_scale (src_value,
        GST_SECOND * state->fps_d, state->fps_n);
    res = TRUE;
  } else if (src_format == GST_FORMAT_TIME &&
      *dest_format == GST_FORMAT_DEFAULT &&
      state->fps_d != 0) {
    /* convert time to frames */
    /* FIXME subtract segment time? */
    *dest_value = gst_util_uint64_scale (src_value, state->fps_n,
        GST_SECOND * state->fps_d);
    res = TRUE;
  }

  /* FIXME add bytes <--> time */

  return res;
}

gboolean
gst_base_video_encoded_video_convert (GstVideoState *state,
    GstFormat src_format, gint64 src_value,
    GstFormat * dest_format, gint64 *dest_value)
{ 
  gboolean res = FALSE;

  if (src_format == *dest_format) {
    *dest_value = src_value;
    return TRUE;
  } 

  GST_DEBUG("src convert");

#if 0
  if (src_format == GST_FORMAT_DEFAULT && *dest_format == GST_FORMAT_TIME) {
    if (dec->fps_d != 0) {
      *dest_value = gst_util_uint64_scale (granulepos_to_frame (src_value),
          dec->fps_d * GST_SECOND, dec->fps_n);
      res = TRUE;
    } else {
      res = FALSE;
    }
  } else {
    GST_WARNING("unhandled conversion from %d to %d", src_format, *dest_format);
    res = FALSE;
  }
#endif

  return res;
}

gboolean
gst_base_video_state_from_caps (GstVideoState *state, GstCaps *caps)
{

  gst_video_format_parse_caps (caps, &state->format,
      &state->width, &state->height);

  gst_video_parse_caps_framerate (caps, &state->fps_n, &state->fps_d);

  state->par_n = 1;
  state->par_d = 1;
  gst_video_parse_caps_pixel_aspect_ratio (caps, &state->par_n,
      &state->par_d);

  {
    GstStructure *structure = gst_caps_get_structure (caps, 0);
    state->interlaced = FALSE;
    gst_structure_get_boolean (structure, "interlaced", &state->interlaced);
  }

  state->clean_width = state->width;
  state->clean_height = state->height;
  state->clean_offset_left = 0;
  state->clean_offset_top = 0;

  /* FIXME need better error handling */
  return TRUE;
}

GstClockTime
gst_video_state_get_timestamp (const GstVideoState *state, int frame_number)
{
  if (frame_number < 0) {
    return state->segment.start -
      (gint64)gst_util_uint64_scale (-frame_number,
          state->fps_d * GST_SECOND, state->fps_n);
  } else {
    return state->segment.start +
      gst_util_uint64_scale (frame_number,
          state->fps_d * GST_SECOND, state->fps_n);
  }
}

/* gst adapter */

static GSList *
get_chunk (GstAdapter *adapter, int offset, int *skip)
{
  GSList *g;

#if 1
  if (skip) *skip = 0;
#endif

  g_return_val_if_fail (offset >= 0, NULL);
  g_return_val_if_fail (offset < adapter->size, NULL);

  offset += adapter->skip;
  g = adapter->buflist;
  while (g) {
    if (offset < GST_BUFFER_SIZE(GST_BUFFER(g->data))) {
      if (skip) *skip = offset;
      return g;
    }
    offset -= GST_BUFFER_SIZE (GST_BUFFER(g->data));
    g = g->next;
  }

  g_assert_not_reached ();
}

void
gst_adapter_copy_full (GstAdapter *adapter, void *dest, int offset,
    int size)
{
  int skip;
  guint8 *cdest = dest;
  int n_bytes;
  GSList *g;

  g_return_if_fail (offset >= 0);
  g_return_if_fail (offset + size <= adapter->size);

  g = get_chunk (adapter, offset, &skip);
  while (size > 0) {
    n_bytes = MIN (GST_BUFFER_SIZE(GST_BUFFER(g->data)) - skip,
        size);

    memcpy (cdest, GST_BUFFER_DATA(GST_BUFFER(g->data)) + skip,
        n_bytes);

    size -= n_bytes;
    cdest += n_bytes;
    skip = 0;

    g = g->next;
  }
}

static int
scan_fast (guint8 *data, guint32 pattern, guint32 mask, int n)
{
  int i;

  pattern &= mask;
  for(i=0;i<n;i++){
    if ((GST_READ_UINT32_BE (data + i) & mask) == pattern) {
      return i;
    }
  }
  return n;
}

static gboolean
scan_slow (GstAdapter *adapter, GSList *g, int skip, guint32 pattern,
    guint32 mask)
{
  guint8 tmp[4];
  int j;

  pattern &= mask;
  for(j=0;j<4;j++){
    tmp[j] = ((guint8 *)GST_BUFFER_DATA(GST_BUFFER(g->data)))[skip];
    skip++;
    if (skip >= GST_BUFFER_SIZE (GST_BUFFER(g->data))) {
      g = g->next;
      skip = 0;
    }
  }

  return ((GST_READ_UINT32_BE (tmp) & mask) == pattern);
}


int
gst_adapter_masked_scan_uint32 (GstAdapter *adapter,
    guint32 pattern, guint32 mask, int offset, int n)
{
  GSList *g;
  int j;
  int k;
  int skip;
  int m;

  g_return_val_if_fail (n >= 0, 0);
  g_return_val_if_fail (offset >= 0, 0);
  g_return_val_if_fail (offset + n + 4 <= adapter->size, 0);

  g = get_chunk (adapter, offset, &skip);
  j = 0;
  while (j < n) {
    m = MIN (GST_BUFFER_SIZE(GST_BUFFER(g->data)) - skip - 4, 0);
    if (m > 0) {
      k = scan_fast (GST_BUFFER_DATA(GST_BUFFER(g->data)) + skip,
          pattern, mask, m);
      if (k < m) {
        return offset+j+k;
      }
      j += m;
      skip += m;
    } else {
      if (scan_slow (adapter, g, skip, pattern, mask)) {
        return offset + j;
      }
      j++;
      skip++;
    }
    if (skip >= GST_BUFFER_SIZE (GST_BUFFER(g->data))) {
      g = g->next;
      skip = 0;
    }
  }

  return n;
}

GstBuffer *
gst_adapter_get_buffer (GstAdapter *adapter)
{
  return gst_buffer_ref (GST_BUFFER(adapter->buflist->data));

}

