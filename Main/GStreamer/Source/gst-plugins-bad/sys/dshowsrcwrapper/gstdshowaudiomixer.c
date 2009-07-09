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

#include "gstdshowaudiomixer.h"

GstDshowAudioMixer *
gst_dshowaudio_mixer_new (IBaseFilter *base_filter, GstDshowAudioMixerDirection dir)
{
  GstDshowAudioMixer *ret = NULL;

  g_return_val_if_fail (base_filter != NULL, NULL);

  ret = g_new0 (GstDshowAudioMixer, 1);

  ret->rec_mutex = g_new (GStaticRecMutex, 1);
  g_static_rec_mutex_init (ret->rec_mutex);

  ret->dir = dir;

  if (!base_filter)
    goto error;

  ret->capture_filter = base_filter;

  return ret;

  /* ERRORS */
error:
  {
    gst_dshowaudio_mixer_free (ret);
    return NULL;
  }
}

void
gst_dshowaudio_mixer_free (GstDshowAudioMixer * mixer)
{
  g_return_if_fail (mixer != NULL);

  if (mixer->mixer_interface) {
    g_object_unref (G_OBJECT (mixer->mixer_interface));
    mixer->mixer_interface = NULL;
  }

  if (mixer->tracklist) {
    g_list_foreach (mixer->tracklist, (GFunc) g_object_unref, NULL);
    g_list_free (mixer->tracklist);
    mixer->tracklist = NULL;
  }

  if (mixer->capture_filter) {
    mixer->capture_filter = NULL;
  }

  g_static_rec_mutex_free (mixer->rec_mutex);
  g_free (mixer->rec_mutex);
  mixer->rec_mutex = NULL;

  g_free (mixer);
}

const GList *
gst_dshowaudio_mixer_list_tracks (GstDshowAudioMixer * mixer)
{
  IPin *pPin = NULL;
  IEnumPins *enumpins = NULL;
  HRESULT hres;
  
  g_return_val_if_fail (mixer->capture_filter != NULL, NULL);

  hres = IBaseFilter_EnumPins (mixer->capture_filter, &enumpins);
  if (SUCCEEDED (hres)) 
  {
    int index = 0;
    while (IEnumPins_Next (enumpins, 1, &pPin, NULL) == S_OK) 
    {
      PIN_DIRECTION pinDir;

      IPin_QueryDirection(pPin, &pinDir);
      if (pinDir == PINDIR_INPUT)
      {
        GstMixerTrack *cap_track = NULL;
        PIN_INFO pInfo;
        LPWSTR pinId;
        gchar label[256];
        gchar untranslated_label[256];
        IAMAudioInputMixer* dshow_audio_input_mixer = NULL;

        IPin_QueryPinInfo(pPin, &pInfo);
        IPin_QueryId(pPin, &pinId);

        wcstombs(label, pInfo.achName, sizeof(pInfo.achName));
        wcstombs(untranslated_label, pinId, sizeof(pinId));

        GST_INFO("one more input pin name = %s and id = %ls\n", label, pinId);

        if (SUCCEEDED (IPin_QueryInterface (pPin,
                      &IID_IAMAudioInputMixer, (void **) &dshow_audio_input_mixer))) 
        {

          cap_track = gst_dshowaudio_mixer_track_new (dshow_audio_input_mixer, label, untranslated_label, index, GST_MIXER_TRACK_INPUT, 2); //FIXME: 2

          if (cap_track)
            mixer->tracklist = g_list_append (mixer->tracklist, cap_track);
        }
      }
      ++index;
    }
  }
  else
    GST_DEBUG("failed to enumerate pins 0x%x", hres);

  return (const GList *) mixer->tracklist;
}

void
gst_dshowaudio_mixer_get_volume (GstDshowAudioMixer * mixer, GstMixerTrack * track,
    gint * volumes)
{
  GstDshowAudioMixerTrack *dshowaudio_track = GST_DSHOWAUDIO_MIXER_TRACK (track);
  gint i = 0;

  g_return_if_fail (mixer != NULL);
  g_return_if_fail (track != NULL);

  //dshow only support mono or stereo
  g_return_if_fail (track->num_channels <= 2 || track->num_channels >= 1);

  //sink
  if (track->flags & GST_MIXER_TRACK_OUTPUT) 
  {
    GST_WARNING ("not yet implemented");
  } 
  //src
  else if (track->flags & GST_MIXER_TRACK_INPUT) 
  {
    g_return_if_fail (mixer->capture_filter != NULL);
    g_return_if_fail (dshowaudio_track->dshow_audio_input_mixer != NULL);
    
    //mono
    if (track->num_channels < 2)
    {
      gdouble volume = 0.0;
      IAMAudioInputMixer_get_MixLevel(dshowaudio_track->dshow_audio_input_mixer, &volume);
      volumes[0] = (gint) (volume * 100);
    }
    //stereo
    else
    {
      gdouble balance = 0.0;
      gdouble volume = 0.0;

      IAMAudioInputMixer_get_MixLevel(dshowaudio_track->dshow_audio_input_mixer, &volume);
      IAMAudioInputMixer_get_Pan(dshowaudio_track->dshow_audio_input_mixer, &balance);

      //same volume on each side
      if (balance == 0)
      {
        volumes[0] = (gint) (volume * 100);
        volumes[1] = (gint) (volume * 100);
      }
      else
      {
        //balance is used to have different level on each side
        gdouble min = volume * (1 - (balance < 0 ? -balance : balance ));

        g_return_if_fail (min >= 0 || min <= 100);
        
        //left higher than right
        if (balance < 0)
        {
          volumes[0] = (gint) (volume * 100);
          volumes[1] = (gint) (min * 100);

        }
        else
        {
          volumes[0] = (gint) (min * 100);
          volumes[1] = (gint) (volume * 100);
        }
      }
    }
  }

  //dshow volumes must be double in [0, 1] but gst interface wants integers, so [0, 100]
  for (i = 0; i < track->num_channels; ++i)
    g_return_if_fail (volumes[i] <= 100 || volumes[i] >= 0);
}


void
gst_dshowaudio_mixer_set_volume (GstDshowAudioMixer * mixer, GstMixerTrack * track,
    gint * volumes)
{
  GstDshowAudioMixerTrack *dshowaudio_track = GST_DSHOWAUDIO_MIXER_TRACK (track);
  gint i = 0;

  g_return_if_fail (mixer != NULL);
  g_return_if_fail (track != NULL);

  //dshow only support mono or stereo
  g_return_if_fail (track->num_channels >= 1 || track->num_channels <= 2);

  //dshow volumes must be double in [0, 1]
  for (i = 0; i < track->num_channels; ++i)
    g_return_if_fail (volumes[i] <= 100 || volumes[i] >= 0);

  //sink
  if (track->flags & GST_MIXER_TRACK_OUTPUT) 
  {
    GST_WARNING ("not yet implemented");
  } 
  //src
  else if (track->flags & GST_MIXER_TRACK_INPUT) 
  {
    HRESULT hres = 0;
    
    g_return_if_fail (mixer->capture_filter != NULL);
    g_return_if_fail (dshowaudio_track->dshow_audio_input_mixer != NULL);
    
    //mono or (stereo and same volume)
    if (track->num_channels < 2 || (track->num_channels > 1 && volumes[0] == volumes[1]))
    {
      //dhsow volume is in [0, 1]
      hres = IAMAudioInputMixer_put_MixLevel(dshowaudio_track->dshow_audio_input_mixer, volumes[0] / 100.0);

      if (FAILED(hres))
        GST_DEBUG("failed to put_MixLevel 0x%x", hres);

      hres = IAMAudioInputMixer_put_Pan(dshowaudio_track->dshow_audio_input_mixer, 0.0);

      if (FAILED(hres))
        GST_DEBUG("failed to put_Pan 0x%x", hres);
    }
    else
    {
      //dhsow volume is in [0, 1]
      gdouble max_volume = max(volumes[0], volumes[1]) / 100.0;
      gdouble percent = 0;
      gint side = 0;

      //can't happen because in this case L and R volume are equals, and this is the if
      g_return_if_fail(max_volume != 0);

      //set level for the 2 channels
      hres = IAMAudioInputMixer_put_MixLevel(dshowaudio_track->dshow_audio_input_mixer, max_volume);

      if (FAILED(hres))
        GST_DEBUG("failed to put_MixLevel 0x%x", hres);

      //percent can't be 1 because can't be equals
      percent = (min(volumes[0], volumes[1]) / 100.0) / max_volume;

      //left greater than right ?
      side = volumes[0] > volumes[1] ? -1 : 1;

      //set balance to have different volume on each side
      hres = IAMAudioInputMixer_put_Pan(dshowaudio_track->dshow_audio_input_mixer, side * (1.0 - percent));

      if (FAILED(hres))
        GST_DEBUG("failed to put_Pan 0x%x", hres);
    }
  }
}

void
gst_dshowaudio_mixer_set_mute (GstDshowAudioMixer * mixer, GstMixerTrack * track,
    gboolean mute)
{
  GstDshowAudioMixerTrack *dshowaudio_track = GST_DSHOWAUDIO_MIXER_TRACK (track);

  gint volumes[2] = {0, 0};
  gint i = 0;

  g_return_if_fail (mixer != NULL);
  g_return_if_fail (track != NULL);

  //dshow only support mono or stereo
  g_return_if_fail (track->num_channels <= 2 || track->num_channels >= 1);

  //mute all channels of the given track
  gst_dshowaudio_mixer_set_volume (mixer, track, volumes);
}

void
gst_dshowaudio_mixer_set_record (GstDshowAudioMixer * mixer,
    GstMixerTrack * track, gboolean record)
{

  GstDshowAudioMixerTrack *dshowaudio_track = GST_DSHOWAUDIO_MIXER_TRACK (track);

  g_return_if_fail (mixer != NULL);
  g_return_if_fail (track != NULL);

  //sink
  if (track->flags & GST_MIXER_TRACK_OUTPUT) 
  {
    GST_WARNING ("not yet implemented");
  } 
  //src
  else if (track->flags & GST_MIXER_TRACK_INPUT) 
  {
    HRESULT hres = 0;
    
    g_return_if_fail (mixer->capture_filter != NULL);
    g_return_if_fail (dshowaudio_track->dshow_audio_input_mixer != NULL);

    hres = IAMAudioInputMixer_put_Enable(dshowaudio_track->dshow_audio_input_mixer, record);

    if (FAILED(hres))
      GST_DEBUG("failed to start recording track 0x%x", hres);
  }
}

void
gst_dshowaudio_mixer_set_option (GstDshowAudioMixer * mixer,
    GstMixerOptions * opts, gchar * value)
{
  GST_WARNING ("not yet implemented");
}

const gchar *
gst_dshowaudio_mixer_get_option (GstDshowAudioMixer * mixer, GstMixerOptions * opts)
{
  GST_WARNING ("not yet implemented");

  return NULL;
}

GstMixerFlags
gst_dshowaudio_mixer_get_mixer_flags (GstDshowAudioMixer * mixer)
{
  g_return_val_if_fail (mixer != NULL, GST_MIXER_FLAG_NONE);

  return GST_MIXER_FLAG_AUTO_NOTIFICATIONS;
}

/* utility function for gstdshowaudiomixerelement to set the interface */
void
_gst_dshowaudio_mixer_set_interface (GstDshowAudioMixer * mixer, GstMixer * mixer_interface)
{
  g_return_if_fail (mixer != NULL && mixer->mixer_interface == NULL);
  g_return_if_fail (mixer_interface != NULL);

  mixer->mixer_interface = g_object_ref (G_OBJECT (mixer_interface));
}
