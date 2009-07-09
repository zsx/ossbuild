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


#ifndef __GST_DSHOWAUDIO_MIXER_TRACK_H__
#define __GST_DSHOWAUDIO_MIXER_TRACK_H__

#include "gstdshowsrcwrapper.h"
#include <gst/interfaces/mixertrack.h>

G_BEGIN_DECLS


#define GST_DSHOWAUDIO_MIXER_TRACK_TYPE         (gst_dshowaudio_mixer_track_get_type ())
#define GST_DSHOWAUDIO_MIXER_TRACK(obj)         (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_DSHOWAUDIO_MIXER_TRACK,GstDshowAudioMixerTrack))
#define GST_DSHOWAUDIO_MIXER_TRACK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_DSHOWAUDIO_MIXER_TRACK,GstDshowAudioMixerTrackClass))
#define GST_IS_DSHOWAUDIO_MIXER_TRACK(obj)            (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_DSHOWAUDIO_MIXER_TRACK))
#define GST_IS_DSHOWAUDIO_MIXER_TRACK_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_DSHOWAUDIO_MIXER_TRACK))
#define GST_TYPE_DSHOWAUDIO_MIXER_TRACK               (gst_dshowaudio_mixer_track_get_type())

typedef struct _GstDshowAudioMixerTrack GstDshowAudioMixerTrack;
typedef struct _GstDshowAudioMixerTrackClass GstDshowAudioMixerTrackClass;


struct _GstDshowAudioMixerTrack {
  GstMixerTrack parent;
  IAMAudioInputMixer *dshow_audio_input_mixer;    /* the dshow audio mixer interface for this track */ 
};

struct _GstDshowAudioMixerTrackClass {
  GstMixerTrackClass parent;
};

GType           gst_dshowaudio_mixer_track_get_type   (void);
GstMixerTrack * gst_dshowaudio_mixer_track_new        (IAMAudioInputMixer * dshow_audio_input_mixer,
                                                       const gchar* label, const gchar* untranslated_label,
                                                       gint index,gint flags, gint num_channels);
void            gst_dshowaudio_mixer_track_free             (GstDshowAudioMixerTrack *mixer_track);

G_END_DECLS


#endif /* __GST_DSHOWAUDIO_MIXER_TRACK_H__ */
