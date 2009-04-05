/*
 *  GStreamer pulseaudio plugin
 *
 *  Copyright (c) 2004-2008 Lennart Poettering
 *
 *  gst-pulse is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1 of the
 *  License, or (at your option) any later version.
 *
 *  gst-pulse is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with gst-pulse; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 *  USA.
 */

#ifndef __GST_PULSEUTIL_H__
#define __GST_PULSEUTIL_H__

#include <gst/gst.h>
#include <pulse/pulseaudio.h>
#include <gst/audio/gstaudiosink.h>

gboolean gst_pulse_fill_sample_spec (GstRingBufferSpec * spec,
    pa_sample_spec * ss);

gchar *gst_pulse_client_name (void);

pa_channel_map *gst_pulse_gst_to_channel_map (pa_channel_map * map,
    const GstRingBufferSpec * spec);

GstRingBufferSpec *gst_pulse_channel_map_to_gst (const pa_channel_map * map,
    GstRingBufferSpec * spec);

void gst_pulse_cvolume_from_linear(pa_cvolume *v, unsigned channels, gdouble volume);

#if !HAVE_PULSE_0_9_11
static inline int PA_CONTEXT_IS_GOOD(pa_context_state_t x) {
    return
        x == PA_CONTEXT_CONNECTING ||
        x == PA_CONTEXT_AUTHORIZING ||
        x == PA_CONTEXT_SETTING_NAME ||
        x == PA_CONTEXT_READY;
}

/** Return non-zero if the passed state is one of the connected states */
static inline int PA_STREAM_IS_GOOD(pa_stream_state_t x) {
    return
        x == PA_STREAM_CREATING ||
        x == PA_STREAM_READY;
}

#endif

#endif
