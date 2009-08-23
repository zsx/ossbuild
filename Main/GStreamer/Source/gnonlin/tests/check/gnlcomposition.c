/* Gnonlin
 * Copyright (C) <2009> Alessandro Decina <alessandro.decina@collabora.co.uk>
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
#include "common.h"

static int composition_pad_added;
static int composition_pad_removed;
static int seek_events;

static gboolean
on_source1_pad_event_cb (GstPad * pad, GstEvent * event, gpointer user_data)
{
  if (event->type == GST_EVENT_SEEK)
    ++seek_events;

  return TRUE;
}

static void
on_source1_pad_added_cb (GstElement * source, GstPad * pad, gpointer user_data)
{
  gst_pad_add_event_probe (pad, G_CALLBACK (on_source1_pad_event_cb), NULL);
}

static void
on_composition_pad_added_cb (GstElement * composition, GstPad * pad,
    GstElement * sink)
{
  GstPad *s = gst_element_get_pad (sink, "sink");
  gst_pad_link (pad, s);
  ++composition_pad_added;
}

static void
on_composition_pad_removed_cb (GstElement * composition, GstPad * pad,
    GstElement * sink)
{
  ++composition_pad_removed;
}

GST_START_TEST (test_change_object_start_stop_in_current_stack)
{
  GstElement *pipeline;
  guint64 start, stop;
  gint64 duration;
  GstElement *comp, *source1, *def, *sink;
  GstBus *bus;
  GstMessage *message;
  gboolean carry_on;
  int seek_events_before;

  pipeline = gst_pipeline_new ("test_pipeline");
  comp =
      gst_element_factory_make_or_warn ("gnlcomposition", "test_composition");

  sink = gst_element_factory_make_or_warn ("fakesink", "sink");
  gst_bin_add_many (GST_BIN (pipeline), comp, sink, NULL);

  /* connect to pad-added */
  g_object_connect (comp, "signal::pad-added",
      on_composition_pad_added_cb, sink, NULL);
  g_object_connect (comp, "signal::pad-removed",
      on_composition_pad_removed_cb, NULL, NULL);

  /*
     source1
     Start : 0s
     Duration : 2s
     Priority : 2
   */

  source1 = videotest_gnl_src ("source1", 0, 2 * GST_SECOND, 1, 2);
  g_object_connect (source1, "signal::pad-added",
      on_source1_pad_added_cb, NULL, NULL);

  check_start_stop_duration (source1, 0, 2 * GST_SECOND, 2 * GST_SECOND);

  /*
     def (default source)
     Priority = G_MAXUINT32
   */
  def =
      videotest_gnl_src ("default", 0 * GST_SECOND, 0 * GST_SECOND, 1,
      G_MAXUINT32);

  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);
  ASSERT_OBJECT_REFCOUNT (def, "default", 1);

  /* Add source 1 */

  /* keep an extra ref to source1 as we remove it from the bin */
  gst_object_ref (source1);
  gst_bin_add (GST_BIN (comp), source1);
  check_start_stop_duration (comp, 0, 2 * GST_SECOND, 2 * GST_SECOND);

  /* Add default */

  gst_bin_add (GST_BIN (comp), def);
  check_start_stop_duration (comp, 0, 2 * GST_SECOND, 2 * GST_SECOND);

  bus = gst_element_get_bus (GST_ELEMENT (pipeline));

  GST_DEBUG ("Setting pipeline to PLAYING");
  ASSERT_OBJECT_REFCOUNT (source1, "source1", 2);

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE);

  GST_DEBUG ("Let's poll the bus");

  carry_on = TRUE;
  while (carry_on) {
    message = gst_bus_poll (bus, GST_MESSAGE_ANY, GST_SECOND / 2);
    if (message) {
      switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_STATE_CHANGED:
        {
          GstState old, current, pending;
          gst_message_parse_state_changed (message, &old, &current, &pending);

          if (message->src == GST_OBJECT (pipeline)) {
            if (current == GST_STATE_PAUSED)
              carry_on = FALSE;
          }
          break;
        }
        case GST_MESSAGE_EOS:
        {
          GST_WARNING ("Saw EOS");

          fail_if (TRUE);
        }
        case GST_MESSAGE_ERROR:
        {
          GError *error;
          char *debug;

          gst_message_parse_error (message, &error, &debug);

          GST_WARNING ("Saw an ERROR %s: %s (%s)",
              gst_object_get_name (message->src), error->message, debug);

          g_error_free (error);
          fail_if (TRUE);
        }
        default:
          break;
      }
      gst_mini_object_unref (GST_MINI_OBJECT (message));
    }
  }

  fail_unless_equals_int (composition_pad_added, 1);
  fail_unless_equals_int (composition_pad_removed, 0);

  seek_events_before = seek_events;

  /* pipeline is paused at this point */

  /* move source1 out of the active segment */
  g_object_set (source1, "start", 4 * GST_SECOND, NULL);
  fail_unless (seek_events > seek_events_before);

  /* remove source1 from the composition, which will become empty and remove the
   * ghostpad */
  gst_bin_remove (GST_BIN (comp), source1);
  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);

  fail_unless_equals_int (composition_pad_added, 1);
  fail_unless_equals_int (composition_pad_removed, 1);

  g_object_set (source1, "start", 0 * GST_SECOND, NULL);
  /* add the source again and check that the ghostpad is added again */
  gst_bin_add (GST_BIN (comp), source1);

  fail_unless_equals_int (composition_pad_added, 2);
  fail_unless_equals_int (composition_pad_removed, 1);

  seek_events_before = seek_events;

  g_object_set (source1, "duration", 1 * GST_SECOND, NULL);
  fail_unless (seek_events > seek_events_before);

  GST_DEBUG ("Setting pipeline to NULL");

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE);
  gst_element_set_state (source1, GST_STATE_NULL);
  gst_object_unref (source1);

  GST_DEBUG ("Resetted pipeline to READY");

  ASSERT_OBJECT_REFCOUNT_BETWEEN (pipeline, "main pipeline", 1, 2);
  gst_object_unref (pipeline);
  ASSERT_OBJECT_REFCOUNT_BETWEEN (bus, "main bus", 1, 2);
  gst_object_unref (bus);
}

GST_END_TEST;

Suite *
gnonlin_suite (void)
{
  Suite *s = suite_create ("gnonlin");
  TCase *tc_chain = tcase_create ("gnlcomposition");

  suite_add_tcase (s, tc_chain);

  tcase_add_test (tc_chain, test_change_object_start_stop_in_current_stack);

  return s;
}

int
main (int argc, char **argv)
{
  int nf;

  Suite *s = gnonlin_suite ();
  SRunner *sr = srunner_create (s);

  gst_check_init (&argc, &argv);

  srunner_run_all (sr, CK_NORMAL);
  nf = srunner_ntests_failed (sr);
  srunner_free (sr);

  return nf;
}
