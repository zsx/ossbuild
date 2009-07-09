/* GStreamer
 * Copyright (C)  2009 Julien Isorce <julien.isorce@gmail.com>
 *
 * gstdshowaudiomixer.c: 
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

#include <gst/gst-i18n-plugin.h>

#include "gstdshowaudiomixertrack.h"

static void gst_dshowaudio_mixer_track_init (GstDshowAudioMixerTrack * dshowaudio_track);
static void gst_dshowaudio_mixer_track_class_init (gpointer g_class,
    gpointer class_data);

static GstMixerTrackClass *parent_class = NULL;

GType
gst_dshowaudio_mixer_track_get_type (void)
{
  static GType track_type = 0;

  if (!track_type) {
    static const GTypeInfo track_info = {
      sizeof (GstDshowAudioMixerTrackClass),
      NULL,
      NULL,
      gst_dshowaudio_mixer_track_class_init,
      NULL,
      NULL,
      sizeof (GstDshowAudioMixerTrack),
      0,
      (GInstanceInitFunc) gst_dshowaudio_mixer_track_init,
      NULL
    };

    track_type =
        g_type_register_static (GST_TYPE_MIXER_TRACK, "GstDshowAudioMixerTrack",
        &track_info, 0);
  }

  return track_type;
}

static void
gst_dshowaudio_mixer_track_class_init (gpointer g_class, gpointer class_data)
{
  parent_class = g_type_class_peek_parent (g_class);
}

static void
gst_dshowaudio_mixer_track_init (GstDshowAudioMixerTrack * dshowaudio_track)
{
}

GstMixerTrack *
gst_dshowaudio_mixer_track_new (IAMAudioInputMixer * dshow_audio_input_mixer,
                                const gchar* label, const gchar* untranslated_label, gint index, gint flags,
                                gint num_channels)
{
  GstDshowAudioMixerTrack *dshowaudio_track = NULL;
  GstMixerTrack *track = (GstMixerTrack *) g_object_new (GST_DSHOWAUDIO_MIXER_TRACK_TYPE,
    "untranslated-label", g_strdup(untranslated_label), "index", index, NULL);

  dshowaudio_track = (GstDshowAudioMixerTrack *) track;

  GST_LOG ("[%s] created new mixer track %p", label, track);

  /* This reflects the assumptions used for GstDshowAudioMixerTrack */
  if (!(!!(flags & GST_MIXER_TRACK_OUTPUT) ^ !!(flags & GST_MIXER_TRACK_INPUT))) {
    GST_ERROR ("Mixer track must be either output or input!");
    g_return_val_if_reached (NULL);
  }

  track->label = g_strdup(label);
  track->flags = flags;
  track->num_channels = num_channels;
  track->min_volume = 0;
  track->max_volume = 100;
  dshowaudio_track->dshow_audio_input_mixer = dshow_audio_input_mixer;

  return track;
}

void
gst_dshowaudio_mixer_track_free (GstDshowAudioMixerTrack * mixer_track)
{
  g_return_if_fail (mixer_track != NULL);

  if (mixer_track->dshow_audio_input_mixer) {
    IAMAudioInputMixer_Release (mixer_track->dshow_audio_input_mixer);
    mixer_track->dshow_audio_input_mixer = NULL;
  }

  g_print (" *** in gst_dshowaudio_mixer_track_free ***\n"); 
}

