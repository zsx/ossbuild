/*
 * Farsight2 - Farsight MSN Session
 *
 * Copyright 2008 Richard Spiers <richard.spiers@gmail.com>
 * Copyright 2007 Nokia Corp.
 * Copyright 2007-2009 Collabora Ltd.
 *  @author: Olivier Crete <olivier.crete@collabora.co.uk>
 *  @author: Youness Alaoui <youness.alaoui@collabora.co.uk>
 *
 * fs-msn-session.c - A Farsight Msn Session gobject
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

/**
 * SECTION:fs-msn-session
 * @short_description: A  MSN session in a #FsMsnConference
 *
 * There can be only one stream per session.
 *
 * It can afterwards be modified to pause sending (or receiving) by modifying
 * the #FsMsnStream::direction property.
 *
 * The transmitter parameters to the fs_session_new_stream() function are
 * used to set the initial value of the construct properties of the stream
 * object. This plugin does not use transmitter plugins, so the transmitter
 * parameter itself is ignored.
 *
 * The codecs preferences can not be modified and the codec is a fixed value.
 * It is always "MIMIC".
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include <gst/gst.h>

#include "fs-msn-session.h"
#include "fs-msn-stream.h"
#include "fs-msn-participant.h"

#define GST_CAT_DEFAULT fsmsnconference_debug

/* Signals */
enum
{
  LAST_SIGNAL
};

/* props */
enum
{
  PROP_0,
  PROP_MEDIA_TYPE,
  PROP_ID,
  PROP_SINK_PAD,
  PROP_CODEC_PREFERENCES,
  PROP_CODECS,
  PROP_CODECS_WITHOUT_CONFIG,
  PROP_CURRENT_SEND_CODEC,
  PROP_CODECS_READY,
  PROP_CONFERENCE,
  PROP_TOS
};



struct _FsMsnSessionPrivate
{
  FsMediaType media_type;

  FsMsnConference *conference;
  FsMsnStream *stream;

  GError *construction_error;

  GstPad *media_sink_pad;

  guint tos; /* Protected by conf lock */

  GMutex *mutex; /* protects the conference */
};

G_DEFINE_TYPE (FsMsnSession, fs_msn_session, FS_TYPE_SESSION);

#define FS_MSN_SESSION_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), FS_TYPE_MSN_SESSION, FsMsnSessionPrivate))

static void fs_msn_session_dispose (GObject *object);
static void fs_msn_session_finalize (GObject *object);

static void fs_msn_session_get_property (GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec);
static void fs_msn_session_set_property (GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec);

static void fs_msn_session_constructed (GObject *object);

static FsStream *fs_msn_session_new_stream (FsSession *session,
    FsParticipant *participant,
    FsStreamDirection direction,
    const gchar *transmitter,
    guint n_parameters,
    GParameter *parameters,
    GError **error);

static GType
fs_msn_session_get_stream_transmitter_type (FsSession *session,
    const gchar *transmitter);

static void _remove_stream (gpointer user_data,
                            GObject *where_the_object_was);

static void
fs_msn_session_class_init (FsMsnSessionClass *klass)
{
  GObjectClass *gobject_class;
  FsSessionClass *session_class;

  gobject_class = (GObjectClass *) klass;
  session_class = FS_SESSION_CLASS (klass);

  gobject_class->set_property = fs_msn_session_set_property;
  gobject_class->get_property = fs_msn_session_get_property;
  gobject_class->constructed = fs_msn_session_constructed;

  session_class->new_stream = fs_msn_session_new_stream;
  session_class->get_stream_transmitter_type =
    fs_msn_session_get_stream_transmitter_type;

  g_object_class_override_property (gobject_class,
      PROP_MEDIA_TYPE, "media-type");
  g_object_class_override_property (gobject_class,
      PROP_ID, "id");
  g_object_class_override_property (gobject_class,
      PROP_SINK_PAD, "sink-pad");

  g_object_class_override_property (gobject_class,
    PROP_CODEC_PREFERENCES, "codec-preferences");
  g_object_class_override_property (gobject_class,
    PROP_CODECS, "codecs");
  g_object_class_override_property (gobject_class,
    PROP_CODECS_WITHOUT_CONFIG, "codecs-without-config");
  g_object_class_override_property (gobject_class,
    PROP_CURRENT_SEND_CODEC, "current-send-codec");
  g_object_class_override_property (gobject_class,
    PROP_CODECS_READY, "codecs-ready");
  g_object_class_override_property (gobject_class,
    PROP_TOS, "tos");

  g_object_class_install_property (gobject_class,
      PROP_CONFERENCE,
      g_param_spec_object ("conference",
          "The Conference this stream refers to",
          "This is a convience pointer for the Conference",
          FS_TYPE_MSN_CONFERENCE,
          G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gobject_class->dispose = fs_msn_session_dispose;
  gobject_class->finalize = fs_msn_session_finalize;

  g_type_class_add_private (klass, sizeof (FsMsnSessionPrivate));
}

static void
fs_msn_session_init (FsMsnSession *self)
{
  /* member init */
  self->priv = FS_MSN_SESSION_GET_PRIVATE (self);
  self->priv->construction_error = NULL;

  self->priv->mutex = g_mutex_new ();

  self->priv->media_type = FS_MEDIA_TYPE_LAST + 1;
}


static FsMsnConference *
fs_msn_session_get_conference (FsMsnSession *self, GError **error)
{
  FsMsnConference *conference;

  g_mutex_lock (self->priv->mutex);
  conference = self->priv->conference;
  if (conference)
    g_object_ref (conference);
  g_mutex_unlock (self->priv->mutex);

  if (!conference)
    g_set_error (error, FS_ERROR, FS_ERROR_DISPOSED,
        "Called function after session has been disposed");

  return conference;
}


static void
fs_msn_session_dispose (GObject *object)
{
  FsMsnSession *self = FS_MSN_SESSION (object);
  GstBin *conferencebin = NULL;
  FsMsnConference *conference = fs_msn_session_get_conference (self, NULL);
  GstElement *valve = NULL;

  g_mutex_lock (self->priv->mutex);
  self->priv->conference = NULL;
  g_mutex_unlock (self->priv->mutex);

  conferencebin = GST_BIN (conference);

  if (!conferencebin)
    goto out;

  if (self->priv->media_sink_pad)
    gst_pad_set_active (self->priv->media_sink_pad, FALSE);

  GST_OBJECT_LOCK (conference);
  valve = self->valve;
  self->valve = NULL;
  GST_OBJECT_UNLOCK (conference);

  if (valve)
  {
    gst_element_set_locked_state (valve, TRUE);
    gst_element_set_state (valve, GST_STATE_NULL);
    gst_bin_remove (conferencebin, valve);
  }

  if (self->priv->media_sink_pad)
    gst_element_remove_pad (GST_ELEMENT (conference),
        self->priv->media_sink_pad);
  self->priv->media_sink_pad = NULL;

  gst_object_unref (conferencebin);
  gst_object_unref (conference);

 out:

  G_OBJECT_CLASS (fs_msn_session_parent_class)->dispose (object);
}

static void
fs_msn_session_finalize (GObject *object)
{
  FsMsnSession *self = FS_MSN_SESSION (object);

  g_mutex_free (self->priv->mutex);

  G_OBJECT_CLASS (fs_msn_session_parent_class)->finalize (object);
}

static void
fs_msn_session_get_property (GObject *object,
                             guint prop_id,
                             GValue *value,
                             GParamSpec *pspec)
{
  FsMsnSession *self = FS_MSN_SESSION (object);
  FsMsnConference *conference = fs_msn_session_get_conference (self, NULL);

  if (!conference)
    return;

  switch (prop_id)
  {
    case PROP_MEDIA_TYPE:
      g_value_set_enum (value, self->priv->media_type);
      break;
    case PROP_ID:
      g_value_set_uint (value, 1);
      break;
    case PROP_CONFERENCE:
      g_value_set_object (value, self->priv->conference);
      break;
    case PROP_SINK_PAD:
      g_value_set_object (value, self->priv->media_sink_pad);
      break;
    case PROP_CODECS_READY:
      g_value_set_boolean (value, TRUE);
      break;
    case PROP_CODEC_PREFERENCES:
      /* There are no preferences, so return NULL */
      break;
    case PROP_CODECS:
    case PROP_CODECS_WITHOUT_CONFIG:
      {
        GList *codecs = NULL;
        FsCodec *mimic_codec = fs_codec_new (FS_CODEC_ID_ANY, "mimic",
          FS_MEDIA_TYPE_VIDEO, 0);
        codecs = g_list_append (codecs, mimic_codec);
        g_value_take_boxed (value, codecs);
      }
      break;
    case PROP_CURRENT_SEND_CODEC:
      {
        FsCodec *send_codec = fs_codec_new (FS_CODEC_ID_ANY, "mimic",
            FS_MEDIA_TYPE_VIDEO, 0);
        g_value_take_boxed (value, send_codec);
        break;
      }
    case PROP_TOS:
      GST_OBJECT_LOCK (conference);
      g_value_set_uint (value, self->priv->tos);
      GST_OBJECT_UNLOCK (conference);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }

  gst_object_unref (conference);
}

static void
fs_msn_session_set_property (GObject *object,
                             guint prop_id,
                             const GValue *value,
                             GParamSpec *pspec)
{
  FsMsnSession *self = FS_MSN_SESSION (object);
  FsMsnConference *conference = fs_msn_session_get_conference (self, NULL);

  if (!conference && !(pspec->flags & G_PARAM_CONSTRUCT_ONLY))
    return;

  switch (prop_id)
  {
    case PROP_MEDIA_TYPE:
      self->priv->media_type = g_value_get_enum (value);
      break;
    case PROP_ID:
      break;
    case PROP_CONFERENCE:
      self->priv->conference = FS_MSN_CONFERENCE (g_value_dup_object (value));
      break;
    case PROP_TOS:
      if (conference)
        GST_OBJECT_LOCK (conference);
      self->priv->tos = g_value_get_uint (value);
      if (self->priv->stream)
        fs_msn_stream_set_tos_locked (self->priv->stream, self->priv->tos);
      if (conference)
        GST_OBJECT_UNLOCK (conference);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }

  if (conference)
    gst_object_unref (conference);
}

static void
fs_msn_session_constructed (GObject *object)
{
  FsMsnSession *self = FS_MSN_SESSION (object);
  GstPad *pad;

  g_assert (self->priv->conference);

  self->valve = gst_element_factory_make ("valve", NULL);

  if (!self->valve)
  {
    self->priv->construction_error = g_error_new (FS_ERROR,
        FS_ERROR_CONSTRUCTION, "Could not make sink valve");
    return;
  }

  if (!gst_bin_add (GST_BIN (self->priv->conference), self->valve))
  {
    self->priv->construction_error = g_error_new (FS_ERROR,
        FS_ERROR_CONSTRUCTION, "Could not add valve to conference");
    return;
  }

  g_object_set (G_OBJECT (self->valve), "drop", TRUE, NULL);

  pad = gst_element_get_static_pad (self->valve, "sink");
  self->priv->media_sink_pad = gst_ghost_pad_new ("sink1", pad);
  gst_object_unref (pad);

  if (!self->priv->media_sink_pad)
  {
    self->priv->construction_error = g_error_new (FS_ERROR,
        FS_ERROR_CONSTRUCTION, "Could not create sink ghost pad");
    return;
  }

  gst_pad_set_active (self->priv->media_sink_pad, TRUE);
  if (!gst_element_add_pad (GST_ELEMENT (self->priv->conference),
          self->priv->media_sink_pad))
  {
    self->priv->construction_error = g_error_new (FS_ERROR,
        FS_ERROR_CONSTRUCTION, "Could not add sink pad to conference");
    gst_object_unref (self->priv->media_sink_pad);
    self->priv->media_sink_pad = NULL;
    return;
  }

  gst_element_sync_state_with_parent (self->valve);

  if (G_OBJECT_CLASS (fs_msn_session_parent_class)->constructed)
    G_OBJECT_CLASS (fs_msn_session_parent_class)->constructed (object);
}


static void
_remove_stream (gpointer user_data,
                GObject *where_the_object_was)
{
  FsMsnSession *self = FS_MSN_SESSION (user_data);
  FsMsnConference *conference = fs_msn_session_get_conference (self, NULL);

  if (!conference)
    return;

  GST_OBJECT_LOCK (conference);
  if (self->priv->stream == (FsMsnStream *) where_the_object_was)
    self->priv->stream = NULL;
  GST_OBJECT_UNLOCK (conference);
  gst_object_unref (conference);
}

/**
 * fs_msn_session_new_stream:
 * @session: an #FsMsnSession
 * @participant: #FsParticipant of a participant for the new stream
 * @direction: #FsStreamDirection describing the direction of the new stream
 * that will be created for this participant
 * @error: location of a #GError, or NULL if no error occured
 *
 * This function creates a stream for the given participant into the active
 * session.
 *
 * Returns: the new #FsStream that has been created. User must unref the
 * #FsStream when the stream is ended. If an error occured, returns NULL.
 */
static FsStream *
fs_msn_session_new_stream (FsSession *session,
                           FsParticipant *participant,
                           FsStreamDirection direction,
                           const gchar *transmitter,
                           guint n_parameters,
                           GParameter *parameters,
                           GError **error)
{
  FsMsnSession *self = FS_MSN_SESSION (session);
  FsMsnParticipant *msnparticipant = NULL;
  FsStream *new_stream = NULL;
  FsMsnConference *conference;

  if (!FS_IS_MSN_PARTICIPANT (participant))
  {
    g_set_error (error, FS_ERROR, FS_ERROR_INVALID_ARGUMENTS,
        "You have to provide a participant of type MSN");
    return NULL;
  }

  conference = fs_msn_session_get_conference (self, error);
  if (!conference)
    return FALSE;

  GST_OBJECT_LOCK (conference);
  if (self->priv->stream)
    goto already_have_stream;
  GST_OBJECT_UNLOCK (conference);

  msnparticipant = FS_MSN_PARTICIPANT (participant);

  new_stream = FS_STREAM_CAST (fs_msn_stream_new (self, msnparticipant,
          direction, conference, n_parameters, parameters, error));

  if (new_stream)
  {
    GST_OBJECT_LOCK (conference);
    if (self->priv->stream)
    {
      g_object_unref (new_stream);
      goto already_have_stream;
    }
    self->priv->stream = (FsMsnStream *) new_stream;
    g_object_weak_ref (G_OBJECT (new_stream), _remove_stream, self);

    if (self->priv->tos)
      fs_msn_stream_set_tos_locked (self->priv->stream, self->priv->tos);
    GST_OBJECT_UNLOCK (conference);
  }
  gst_object_unref (conference);


  return new_stream;

 already_have_stream:
  GST_OBJECT_UNLOCK (conference);
  gst_object_unref (conference);

  g_set_error (error, FS_ERROR, FS_ERROR_ALREADY_EXISTS,
        "There already is a stream in this session");
  return NULL;
}

FsMsnSession *
fs_msn_session_new (FsMediaType media_type,
    FsMsnConference *conference,
    GError **error)
{
  FsMsnSession *session = g_object_new (FS_TYPE_MSN_SESSION,
      "media-type", media_type,
      "conference", conference,
      NULL);

  if (!session)
  {
    *error = g_error_new (FS_ERROR, FS_ERROR_CONSTRUCTION,
        "Could not create object");
  }
  else if (session->priv->construction_error)
  {
    g_propagate_error (error, session->priv->construction_error);
    g_object_unref (session);
    return NULL;
  }

  return session;
}


static GType
fs_msn_session_get_stream_transmitter_type (FsSession *session,
    const gchar *transmitter)
{
  return FS_TYPE_MSN_STREAM;
}
