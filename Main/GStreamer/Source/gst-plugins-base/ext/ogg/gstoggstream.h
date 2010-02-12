/* GStreamer
 * Copyright (C) 2009 David Schleef <ds@schleef.org>
 *
 * gstoggstream.h: header for GstOggStream
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

#ifndef __GST_OGG_STREAM_H__
#define __GST_OGG_STREAM_H__

#include <ogg/ogg.h>

#include <gst/gst.h>

G_BEGIN_DECLS

typedef struct _GstOggStream GstOggStream;

struct _GstOggStream
{
  ogg_stream_state stream;

  glong serialno;
  GList *headers;
  gboolean have_headers;
  GList *queued;

  /* for oggparse */
  gboolean in_headers;
  GList *unknown_pages;

  gint map;
  gboolean is_skeleton;
  gboolean have_fisbone;
  gint granulerate_n;
  gint granulerate_d;
  guint32 preroll;
  guint granuleshift;
  gint n_header_packets;
  gint n_header_packets_seen;
  gint64 accumulated_granule;
  gint frame_size;

  GstCaps *caps;
  
  /* vorbis stuff */
  int nln_increments[4];
  int nsn_increment;
  int short_size;
  int long_size;
  int vorbis_log2_num_modes;
  int vorbis_mode_sizes[256];
  int last_size;
  /* theora stuff */
  gboolean theora_has_zero_keyoffset;
  /* OGM stuff */
  gboolean is_ogm;
  gboolean is_ogm_text;
};


gboolean gst_ogg_stream_setup_map (GstOggStream * pad, ogg_packet *packet);
GstClockTime gst_ogg_stream_get_end_time_for_granulepos (GstOggStream *pad,
    gint64 granulepos);
GstClockTime gst_ogg_stream_get_start_time_for_granulepos (GstOggStream *pad,
    gint64 granulepos);
GstClockTime gst_ogg_stream_granule_to_time (GstOggStream *pad, gint64 granule);
gint64 gst_ogg_stream_granulepos_to_granule (GstOggStream * pad, gint64 granulepos);
gint64 gst_ogg_stream_granulepos_to_key_granule (GstOggStream * pad, gint64 granulepos);
gint64 gst_ogg_stream_granule_to_granulepos (GstOggStream * pad, gint64 granule, gint64 keyframe_granule);
GstClockTime gst_ogg_stream_get_packet_start_time (GstOggStream *pad,
    ogg_packet *packet);
gboolean gst_ogg_stream_granulepos_is_key_frame (GstOggStream *pad,
    gint64 granulepos);
gboolean gst_ogg_stream_packet_is_header (GstOggStream *pad, ogg_packet *packet);
gint64 gst_ogg_stream_get_packet_duration (GstOggStream * pad, ogg_packet *packet);


G_END_DECLS

#endif /* __GST_OGG_STREAM_H__ */
