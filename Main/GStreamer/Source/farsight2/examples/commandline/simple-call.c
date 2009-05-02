/* Farsight 2 ad-hoc test for simple calls.
 *
 * Copyright (C) 2008 Collabora, Nokia
 * @author: Olivier Crete <olivier.crete@collabora.co.uk>
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

/*
 * WARNING:
 *
 * Do not use this as an example of a proper use of farsight, it assumes that
 * both ends have the EXACT same list of codec installed in the EXACT same order
 */


#include <stdlib.h>

#include <glib.h>
#include <gst/gst.h>
#include <gst/farsight/fs-conference-iface.h>

#define DEFAULT_AUDIOSRC       "alsasrc"
#define DEFAULT_AUDIOSINK      "audioconvert ! audioresample ! audioconvert ! alsasink"

typedef struct _TestSession
{
  FsSession *session;
  FsStream *stream;
} TestSession;


static void
print_error (GError *error)
{
  if (error)
  {
    g_error ("Error: %s:%d : %s", g_quark_to_string (error->domain),
        error->code, error->message);
  }
}

static void
src_pad_added_cb (FsStream *stream, GstPad *pad, FsCodec *codec,
    gpointer user_data)
{
  GstElement *pipeline = GST_ELEMENT_CAST (user_data);
  GstElement *sink = NULL;
  GError *error = NULL;
  GstPad *pad2;

  g_print ("Adding receive pipeline\n");

  if (g_getenv ("AUDIOSINK"))
    sink = gst_parse_bin_from_description (g_getenv ("AUDIOSINK"), TRUE,
        &error);
  else
    sink = gst_parse_bin_from_description (DEFAULT_AUDIOSINK, TRUE,
        &error);
  print_error (error);
  g_assert (sink);

  g_assert (gst_bin_add (GST_BIN (pipeline), sink));


  pad2 = gst_element_get_static_pad (sink, "sink");
  g_assert (pad2);

  g_assert (GST_PAD_LINK_SUCCESSFUL (gst_pad_link (pad, pad2)));

  g_assert (gst_element_set_state (sink, GST_STATE_PLAYING) !=
      GST_STATE_CHANGE_FAILURE);

  gst_object_unref (pad2);
}

static TestSession*
add_audio_session (GstElement *pipeline, FsConference *conf, guint id,
    FsParticipant *part, guint localport, const gchar *remoteip,
    guint remoteport)
{
  TestSession *ses = g_slice_new0 (TestSession);
  GError *error = NULL;
  GstPad *pad = NULL, *pad2 = NULL;
  GstElement *src = NULL;
  GList *cands = NULL;
  GList *codecs = NULL;
  GParameter param = {0};
  gboolean res;

  ses->session = fs_conference_new_session (conf, FS_MEDIA_TYPE_AUDIO, &error);
  print_error (error);
  g_assert (ses->session);

  g_object_get (ses->session, "sink-pad", &pad, NULL);

  if (g_getenv ("AUDIOSRC"))
    src = gst_parse_bin_from_description (g_getenv ("AUDIOSRC"), TRUE,
        &error);
  else
    src = gst_parse_bin_from_description (DEFAULT_AUDIOSRC, TRUE,
        &error);
  print_error (error);
  g_assert (src);

  g_assert (gst_bin_add (GST_BIN (pipeline), src));

  pad2 = gst_element_get_static_pad (src, "src");
  g_assert (pad2);

  g_assert (GST_PAD_LINK_SUCCESSFUL (gst_pad_link (pad2, pad)));

  gst_object_unref (pad2);
  gst_object_unref (pad);


  cands = g_list_prepend (NULL, fs_candidate_new ("", FS_COMPONENT_RTP,
          FS_CANDIDATE_TYPE_HOST, FS_NETWORK_PROTOCOL_UDP, NULL, localport));

  param.name = "preferred-local-candidates";
  g_value_init (&param.value, FS_TYPE_CANDIDATE_LIST);
  g_value_take_boxed (&param.value, cands);

  ses->stream = fs_session_new_stream (ses->session, part, FS_DIRECTION_BOTH,
      "rawudp", 1, &param, &error);
  print_error (error);
  g_assert (ses->stream);

  g_value_unset (&param.value);

  g_signal_connect (ses->stream, "src-pad-added",
      G_CALLBACK (src_pad_added_cb), pipeline);

  cands = g_list_prepend (NULL, fs_candidate_new ("", FS_COMPONENT_RTP,
          FS_CANDIDATE_TYPE_HOST, FS_NETWORK_PROTOCOL_UDP, remoteip,
          remoteport));

  res = fs_stream_set_remote_candidates (ses->stream, cands, &error);
  print_error (error);
  g_assert (res);

  fs_candidate_list_destroy (cands);

  codecs = g_list_prepend (NULL,
      fs_codec_new (FS_CODEC_ID_ANY, "PCMA", FS_MEDIA_TYPE_AUDIO, 0));
  codecs = g_list_prepend (codecs,
      fs_codec_new (FS_CODEC_ID_ANY, "PCMU", FS_MEDIA_TYPE_AUDIO, 0));

  res = fs_session_set_codec_preferences (ses->session, codecs, &error);
  print_error (error);
  fs_codec_list_destroy (codecs);


  g_object_get (ses->session, "codecs", &codecs, NULL);
  res = fs_stream_set_remote_codecs (ses->stream, codecs, &error);
  print_error (error);
  g_assert (res);

  return ses;
}

static gboolean
async_bus_cb (GstBus *bus, GstMessage *message, gpointer user_data)
{
  switch (GST_MESSAGE_TYPE(message))
  {
    case GST_MESSAGE_ERROR:
      {
        GError *error = NULL;
        gchar *debug_str = NULL;

        gst_message_parse_error (message, &error, &debug_str);
        g_error ("Got gst message: %s %s", error->message, debug_str);
      }
      break;
    case GST_MESSAGE_WARNING:
      {
        GError *error = NULL;
        gchar *debug_str = NULL;

        gst_message_parse_warning (message, &error, &debug_str);
        g_warning ("Got gst message: %s %s", error->message, debug_str);
      }
      break;
    case GST_MESSAGE_ELEMENT:
      {
        const GstStructure *s = gst_message_get_structure (message);

        if (gst_structure_has_name (s, "farsight-error"))
        {
          gint error;
          const gchar *error_msg = gst_structure_get_string (s, "error-msg");
          const gchar *debug_msg = gst_structure_get_string (s, "debug-msg");

          g_assert (gst_structure_get_enum (s, "error-no", FS_TYPE_ERROR,
                  &error));

          if (FS_ERROR_IS_FATAL (error))
            g_error ("Farsight fatal error: %d %s %s", error, error_msg,
                debug_msg);
          else
            g_warning ("Farsight non-fatal error: %d %s %s", error, error_msg,
                debug_msg);
        }
        else if (gst_structure_has_name (s, "farsight-new-local-candidate"))
        {
          const GValue *val = gst_structure_get_value (s, "candidate");
          FsCandidate *cand = NULL;

          g_assert (val);
          cand = g_value_get_boxed (val);

          g_print ("New candidate: %s %d\n", cand->ip, cand->port);
        }
        else if (gst_structure_has_name (s,
                "farsight-local-candidates-prepared"))
        {
          g_print ("Local candidates prepared\n");
        }
        else if (gst_structure_has_name (s, "farsight-recv-codecs-changed"))
        {
          const GValue *val = gst_structure_get_value (s, "codecs");
          GList *codecs = NULL;

          g_assert (val);
          codecs = g_value_get_boxed (val);

          g_print ("Recv codecs changed:\n");
          for (; codecs; codecs = g_list_next (codecs))
          {
            FsCodec *codec = codecs->data;
            gchar *tmp = fs_codec_to_string (codec);
            g_print ("%s\n", tmp);
            g_free (tmp);
          }
        }
        else if (gst_structure_has_name (s, "farsight-send-codec-changed"))
        {
          const GValue *val = gst_structure_get_value (s, "codec");
          FsCodec *codec = NULL;
          gchar *tmp;
          g_assert (val);
          codec = g_value_get_boxed (val);
          tmp = fs_codec_to_string (codec);

          g_print ("Send codec changed: %s\n", tmp);
          g_free (tmp);
        }
      }
      break;
    default:
      break;
  }

  return TRUE;
}

int main (int argc, char **argv)
{
  GMainLoop *loop = NULL;
  GstElement *pipeline = NULL;
  GstBus *bus = NULL;
  const gchar *remoteip;
  guint localport = 0;
  guint remoteport = 0;
  GstElement *conf = NULL;
  FsParticipant *part = NULL;
  GError *error = NULL;

  gst_init (&argc, &argv);

  if (argc != 4)
  {
    g_print ("Usage: %s <local port> <remoteip> <remoteport>\n",
        argv[0]);
    return 1;
  }

  localport = atoi (argv[1]);
  remoteip = argv[2];
  remoteport = atoi (argv[3]);

  if (!localport || !remoteip || !remoteport)
  {
    g_print ("Usage: %s <local port> <remoteip> <remoteport>\n",
        argv[0]);
    return 2;
  }

  loop = g_main_loop_new (NULL, FALSE);

  pipeline = gst_pipeline_new (NULL);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, async_bus_cb, pipeline);
  gst_object_unref (bus);

  conf = gst_element_factory_make ("fsrtpconference", NULL);
  g_assert (conf);

  part = fs_conference_new_participant (FS_CONFERENCE (conf), "test@ignore",
      &error);
  print_error (error);
  g_assert (part);

  g_assert (gst_bin_add (GST_BIN (pipeline), conf));


  add_audio_session (pipeline, FS_CONFERENCE (conf), 1, part, localport,
      remoteip, remoteport);


  g_assert (gst_element_set_state (pipeline, GST_STATE_PLAYING) !=
      GST_STATE_CHANGE_FAILURE);

  g_main_loop_run (loop);

  g_assert (gst_element_set_state (pipeline, GST_STATE_NULL) !=
      GST_STATE_CHANGE_FAILURE);

  g_object_unref (part);

  gst_object_unref (pipeline);
  g_main_loop_unref (loop);

  return 0;
}
