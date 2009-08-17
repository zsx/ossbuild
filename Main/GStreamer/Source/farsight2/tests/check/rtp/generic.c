/* Farsigh2 generic unit tests for conferences
 *
 * Copyright (C) 2007 Collabora, Nokia
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "generic.h"

#include <gst/check/gstcheck.h>
#include <gst/farsight/fs-conference-iface.h>

struct SimpleTestConference *
setup_simple_conference (
    gint id,
    gchar *conference_elem,
    gchar *cname)
{
  struct SimpleTestConference *dat = g_new0 (struct SimpleTestConference, 1);
  GError *error = NULL;
  guint tos;

  dat->id = id;
  dat->cname = g_strdup (cname);

  dat->pipeline = gst_pipeline_new ("pipeline");
  fail_if (dat->pipeline == NULL);

  dat->conference = gst_element_factory_make (conference_elem, NULL);
  fail_if (dat->conference == NULL, "Could not build %s", conference_elem);
  fail_unless (gst_bin_add (GST_BIN (dat->pipeline), dat->conference),
      "Could not add conference to the pipeline");

  g_object_set (dat->conference, "sdes-cname", cname, NULL);

  dat->session = fs_conference_new_session (FS_CONFERENCE (dat->conference),
      FS_MEDIA_TYPE_AUDIO, &error);
  if (error)
    fail ("Error while creating new session (%d): %s",
        error->code, error->message);
  fail_if (dat->session == NULL, "Could not make session, but no GError!");

  g_object_set (dat->session, "tos", 2, NULL);
  g_object_get (dat->session, "tos", &tos, NULL);
  fail_unless (tos == 2);


  g_object_set_data (G_OBJECT (dat->conference), "dat", dat);

  return dat;
}


struct SimpleTestStream *
simple_conference_add_stream (
    struct SimpleTestConference *dat,
    struct SimpleTestConference *target,
    const gchar *transmitter,
    guint st_param_count,
    GParameter *st_params)
{
  struct SimpleTestStream *st = g_new0 (struct SimpleTestStream, 1);
  GError *error = NULL;

  st->dat = dat;
  st->target = target;

  st->participant = fs_conference_new_participant (
      FS_CONFERENCE (dat->conference), NULL, &error);
  if (error)
    fail ("Error while creating new participant (%d): %s",
        error->code, error->message);
  fail_if (st->participant == NULL, "Could not make participant, but no GError!");

  st->stream = fs_session_new_stream (dat->session, st->participant,
      FS_DIRECTION_BOTH, transmitter, st_param_count, st_params, &error);
  if (error)
    fail ("Error while creating new stream (%d): %s",
        error->code, error->message);
  fail_if (st->stream == NULL, "Could not make stream, but no GError!");

  g_object_set_data (G_OBJECT (st->stream), "SimpleTestStream", st);

  dat->streams = g_list_append (dat->streams, st);

  return st;
}


void
cleanup_simple_stream (struct SimpleTestStream *st)
{
  if (st->stream)
    g_object_unref (st->stream);
  g_object_unref (st->participant);
  g_free (st);
}

void
cleanup_simple_conference (struct SimpleTestConference *dat)
{

  g_list_foreach (dat->streams, (GFunc) cleanup_simple_stream, NULL);
  g_list_free (dat->streams);

  if (dat->session)
    g_object_unref (dat->session);
  gst_object_unref (dat->pipeline);
  g_free (dat->cname);
  g_free (dat);
}


void
setup_fakesrc (struct SimpleTestConference *dat)
{
  GstPad *sinkpad = NULL, *srcpad = NULL;

  GST_DEBUG ("Adding fakesrc");


  g_object_get (dat->session, "sink-pad", &sinkpad, NULL);
  fail_if (sinkpad == NULL, "Could not get session sinkpad");

  dat->fakesrc = gst_element_factory_make ("audiotestsrc", NULL);
  fail_if (dat->fakesrc == NULL, "Could not make audiotestsrc");
  gst_bin_add (GST_BIN (dat->pipeline), dat->fakesrc);

  g_object_set (dat->fakesrc,
      "blocksize", 10,
      "is-live", TRUE,
      NULL);

  srcpad = gst_element_get_static_pad (dat->fakesrc, "src");

  fail_unless (gst_pad_link (srcpad, sinkpad) == GST_PAD_LINK_OK,
      "Could not link the capsfilter and the fsrtpconference");

  gst_object_unref (sinkpad);
  gst_object_unref (srcpad);

  if (dat->started)
    gst_element_set_state (dat->pipeline, GST_STATE_PLAYING);
}
