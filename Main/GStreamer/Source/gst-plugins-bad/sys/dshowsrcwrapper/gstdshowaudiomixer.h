/* GStreamer
 * Copyright (C)  2009 Julien Isorce <julien.isorce@gmail.com>
 *
 * gstdshowaudiomixer.h: 
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


#ifndef __GST_DSHOWAUDIO_MIXER_H__
#define __GST_DSHOWAUDIO_MIXER_H__

#include <gst/interfaces/mixer.h>
#include <gst/interfaces/mixeroptions.h>
#include "gstdshowaudiomixertrack.h"


G_BEGIN_DECLS

/* This does not get you what you think it does, use obj->mixer   */
//#define GST_DSHOWAUDIO_MIXER(obj)             ((GstDshowAudioMixer*)(obj))

typedef struct _GstDshowAudioMixer GstDshowAudioMixer;

typedef enum {
  GST_DSHOWAUDIO_MIXER_CAPTURE = 1<<0,
  GST_DSHOWAUDIO_MIXER_PLAYBACK = 1<<1,
  GST_DSHOWAUDIO_MIXER_ALL = GST_DSHOWAUDIO_MIXER_CAPTURE | GST_DSHOWAUDIO_MIXER_PLAYBACK
} GstDshowAudioMixerDirection;

/**
 * GstDshowAudioMixer:
 *
 * Opaque data structure
 */
struct _GstDshowAudioMixer
{
  GList * tracklist;      /* list of available tracks */

  IBaseFilter * capture_filter;
  
  //IBaseFilter * renderer_filter;

  GStaticRecMutex *	rec_mutex;

  GstMixer *		mixer_interface;

  GstDshowAudioMixerDirection dir;
};


GstDshowAudioMixer*   gst_dshowaudio_mixer_new   (IBaseFilter *base_filter, GstDshowAudioMixerDirection dir);
void            gst_dshowaudio_mixer_free             (GstDshowAudioMixer *mixer);

const GList*    gst_dshowaudio_mixer_list_tracks      (GstDshowAudioMixer * mixer);
void            gst_dshowaudio_mixer_set_volume       (GstDshowAudioMixer * mixer,
                                                 GstMixerTrack * track,
                                                 gint * volumes);
void            gst_dshowaudio_mixer_get_volume       (GstDshowAudioMixer * mixer,
                                                 GstMixerTrack * track,
                                                 gint * volumes);
void            gst_dshowaudio_mixer_set_record       (GstDshowAudioMixer * mixer,
                                                 GstMixerTrack * track,
                                                 gboolean record);
void            gst_dshowaudio_mixer_set_mute         (GstDshowAudioMixer * mixer,
                                                 GstMixerTrack * track,
                                                 gboolean mute);
void            gst_dshowaudio_mixer_set_option       (GstDshowAudioMixer * mixer,
                                                 GstMixerOptions * opts,
                                                 gchar * value);
const gchar*    gst_dshowaudio_mixer_get_option       (GstDshowAudioMixer * mixer,
                                                 GstMixerOptions * opts);
void		_gst_dshowaudio_mixer_set_interface   (GstDshowAudioMixer * mixer,
						 GstMixer * mixer_interface);
GstMixerFlags   gst_dshowaudio_mixer_get_mixer_flags  (GstDshowAudioMixer *mixer);

#define GST_IMPLEMENT_DSHOWAUDIO_MIXER_METHODS(Type, interface_as_function)           \
static gboolean                                                                 \
interface_as_function ## _supported (Type *this, GType iface_type)              \
{                                                                               \
  g_assert (iface_type == GST_TYPE_MIXER);                                      \
                                                                                \
  return (this->mixer != NULL);                                                 \
}                                                                               \
                                                                                \
static const GList*                                                             \
interface_as_function ## _list_tracks (GstMixer * mixer)                        \
{                                                                               \
  Type *this = (Type*) mixer;                                                   \
                                                                                \
  g_return_val_if_fail (this != NULL, NULL);                                    \
  g_return_val_if_fail (this->mixer != NULL, NULL);                             \
                                                                                \
  return gst_dshowaudio_mixer_list_tracks (this->mixer);                              \
}                                                                               \
                                                                                \
static void                                                                     \
interface_as_function ## _set_volume (GstMixer * mixer, GstMixerTrack * track,  \
    gint * volumes)                                                             \
{                                                                               \
  Type *this = (Type*) mixer;                                                   \
                                                                                \
  g_return_if_fail (this != NULL);                                              \
  g_return_if_fail (this->mixer != NULL);                                       \
                                                                                \
  gst_dshowaudio_mixer_set_volume (this->mixer, track, volumes);                      \
}                                                                               \
                                                                                \
static void                                                                     \
interface_as_function ## _get_volume (GstMixer * mixer, GstMixerTrack * track,  \
    gint * volumes)                                                             \
{                                                                               \
  Type *this = (Type*) mixer;                                                   \
                                                                                \
  g_return_if_fail (this != NULL);                                              \
  g_return_if_fail (this->mixer != NULL);                                       \
                                                                                \
  gst_dshowaudio_mixer_get_volume (this->mixer, track, volumes);                      \
}                                                                               \
                                                                                \
static void                                                                     \
interface_as_function ## _set_record (GstMixer * mixer, GstMixerTrack * track,  \
    gboolean record)                                                            \
{                                                                               \
  Type *this = (Type*) mixer;                                                   \
                                                                                \
  g_return_if_fail (this != NULL);                                              \
  g_return_if_fail (this->mixer != NULL);                                       \
                                                                                \
  gst_dshowaudio_mixer_set_record (this->mixer, track, record);                       \
}                                                                               \
                                                                                \
static void                                                                     \
interface_as_function ## _set_mute (GstMixer * mixer, GstMixerTrack * track,    \
    gboolean mute)                                                              \
{                                                                               \
  Type *this = (Type*) mixer;                                                   \
                                                                                \
  g_return_if_fail (this != NULL);                                              \
  g_return_if_fail (this->mixer != NULL);                                       \
                                                                                \
  gst_dshowaudio_mixer_set_mute (this->mixer, track, mute);                           \
}                                                                               \
                                                                                \
static void                                                                     \
interface_as_function ## _set_option (GstMixer * mixer, GstMixerOptions * opts, \
    gchar * value)                                                              \
{                                                                               \
  Type *this = (Type*) mixer;                                                   \
                                                                                \
  g_return_if_fail (this != NULL);                                              \
  g_return_if_fail (this->mixer != NULL);                                       \
                                                                                \
  gst_dshowaudio_mixer_set_option (this->mixer, opts, value);                         \
}                                                                               \
                                                                                \
static const gchar*                                                             \
interface_as_function ## _get_option (GstMixer * mixer, GstMixerOptions * opts) \
{                                                                               \
  Type *this = (Type*) mixer;                                                   \
                                                                                \
  g_return_val_if_fail (this != NULL, NULL);                                    \
  g_return_val_if_fail (this->mixer != NULL, NULL);                             \
                                                                                \
  return gst_dshowaudio_mixer_get_option (this->mixer, opts);                         \
}                                                                               \
                                                                                \
static GstMixerFlags                                                            \
interface_as_function ## _get_mixer_flags (GstMixer * mixer)                    \
{                                                                               \
  Type *this = (Type*) mixer;                                                   \
                                                                                \
  g_return_val_if_fail (this != NULL, GST_MIXER_FLAG_NONE);                     \
  g_return_val_if_fail (this->mixer != NULL, GST_MIXER_FLAG_NONE);              \
                                                                                \
  return gst_dshowaudio_mixer_get_mixer_flags (this->mixer);                          \
}                                                                               \
                                                                                \
static void                                                                     \
interface_as_function ## _interface_init (GstMixerClass * klass)                \
{                                                                               \
  GST_MIXER_TYPE (klass) = GST_MIXER_HARDWARE;                                  \
                                                                                \
  /* set up the interface hooks */                                              \
  klass->list_tracks = interface_as_function ## _list_tracks;                   \
  klass->set_volume = interface_as_function ## _set_volume;                     \
  klass->get_volume = interface_as_function ## _get_volume;                     \
  klass->set_mute = interface_as_function ## _set_mute;                         \
  klass->set_record = interface_as_function ## _set_record;                     \
  klass->set_option = interface_as_function ## _set_option;                     \
  klass->get_option = interface_as_function ## _get_option;                     \
  klass->get_mixer_flags = interface_as_function ## _get_mixer_flags;           \
}


G_END_DECLS


#endif /* __GST_DSHOWAUDIO_MIXER_H__ */
