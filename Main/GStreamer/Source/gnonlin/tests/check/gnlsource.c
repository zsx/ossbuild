#include "common.h"

GST_START_TEST (test_simple_videotestsrc)
{
  GstElement *pipeline;
  GstElement *gnlsource, *sink;
  CollectStructure *collect;
  GstBus *bus;
  GstMessage *message;
  gboolean carry_on = TRUE;
  guint64 start, stop;
  gint64 duration;
  GstPad *sinkpad;

  pipeline = gst_pipeline_new ("test_pipeline");

  /*
     Source 1
     Start : 1s
     Duration : 1s
     Priority : 1
   */
  gnlsource =
      videotest_gnl_src ("source1", 1 * GST_SECOND, 1 * GST_SECOND, 1, 1);
  fail_if (gnlsource == NULL);
  check_start_stop_duration (gnlsource, 1 * GST_SECOND, 2 * GST_SECOND,
      1 * GST_SECOND);

  sink = gst_element_factory_make_or_warn ("fakesink", "sink");
  fail_if (sink == NULL);

  gst_bin_add_many (GST_BIN (pipeline), gnlsource, sink, NULL);

  /* Shared data */
  collect = g_new0 (CollectStructure, 1);
  collect->comp = gnlsource;
  collect->sink = sink;

  /* Expected segments */
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          1 * GST_SECOND, 2 * GST_SECOND, 1 * GST_SECOND));

  g_signal_connect (G_OBJECT (gnlsource), "pad-added",
      G_CALLBACK (composition_pad_added_cb), collect);

  sinkpad = gst_element_get_pad (sink, "sink");
  fail_if (sinkpad == NULL);
  gst_pad_add_event_probe (sinkpad, G_CALLBACK (sinkpad_event_probe), collect);
  gst_pad_add_buffer_probe (sinkpad, G_CALLBACK (sinkpad_buffer_probe),
      collect);

  bus = gst_element_get_bus (pipeline);

  GST_DEBUG ("Setting pipeline to PLAYING");
  ASSERT_OBJECT_REFCOUNT (gnlsource, "gnlsource", 1);

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE);

  GST_DEBUG ("Let's poll the bus");

  while (carry_on) {
    message = gst_bus_poll (bus, GST_MESSAGE_ANY, GST_SECOND / 2);
    GST_LOG ("poll");
    if (message) {
      switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_EOS:
          /* we should check if we really finished here */
          GST_WARNING ("Got an EOS");
          carry_on = FALSE;
          break;
        case GST_MESSAGE_SEGMENT_START:
        case GST_MESSAGE_SEGMENT_DONE:
          /* We shouldn't see any segement messages, since we didn't do a segment seek */
          GST_WARNING ("Saw a Segment start/stop");
          fail_if (FALSE);
          carry_on = FALSE;
          break;
        case GST_MESSAGE_ERROR:
          GST_WARNING ("Saw an ERROR");
          fail_if (TRUE);
        default:
          break;
      }
      gst_mini_object_unref (GST_MINI_OBJECT (message));
    }
  }

  GST_DEBUG ("Setting pipeline to NULL");

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_READY) == GST_STATE_CHANGE_FAILURE);

  fail_if (collect->expected_segments != NULL);

  gst_object_unref (GST_OBJECT (sinkpad));

  GST_DEBUG ("Resetted pipeline to READY");

  g_free (collect);
}

GST_END_TEST;

GST_START_TEST (test_videotestsrc_in_bin)
{
  GstElement *pipeline;
  GstElement *gnlsource, *sink;
  CollectStructure *collect;
  GstBus *bus;
  GstMessage *message;
  gboolean carry_on = TRUE;
  guint64 start, stop;
  gint64 duration;
  GstPad *sinkpad;

  pipeline = gst_pipeline_new ("test_pipeline");

  /*
     Source 1
     Start : 1s
     Duration : 1s
     Priority : 1
   */
  gnlsource = videotest_in_bin_gnl_src ("source1", 0, 1 * GST_SECOND, 1, 1);
  /* Handle systems which don't have alpha available */
  if (gnlsource == NULL)
    return;
  check_start_stop_duration (gnlsource, 0, 1 * GST_SECOND, 1 * GST_SECOND);

  sink = gst_element_factory_make_or_warn ("fakesink", "sink");
  fail_if (sink == NULL);

  gst_bin_add_many (GST_BIN (pipeline), gnlsource, sink, NULL);

  /* Shared data */
  collect = g_new0 (CollectStructure, 1);
  collect->comp = gnlsource;
  collect->sink = sink;

  /* Expected segments */
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME, 0, 1 * GST_SECOND, 0));

  g_signal_connect (G_OBJECT (gnlsource), "pad-added",
      G_CALLBACK (composition_pad_added_cb), collect);

  sinkpad = gst_element_get_pad (sink, "sink");
  fail_if (sinkpad == NULL);
  gst_pad_add_event_probe (sinkpad, G_CALLBACK (sinkpad_event_probe), collect);
  gst_pad_add_buffer_probe (sinkpad, G_CALLBACK (sinkpad_buffer_probe),
      collect);

  bus = gst_element_get_bus (pipeline);

  GST_DEBUG ("Setting pipeline to PLAYING");
  ASSERT_OBJECT_REFCOUNT (gnlsource, "gnlsource", 1);

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE);

  GST_DEBUG ("Let's poll the bus");

  while (carry_on) {
    message = gst_bus_poll (bus, GST_MESSAGE_ANY, GST_SECOND / 2);
    GST_LOG ("poll");
    if (message) {
      switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_EOS:
          /* we should check if we really finished here */
          GST_WARNING ("Got an EOS");
          carry_on = FALSE;
          break;
        case GST_MESSAGE_SEGMENT_START:
        case GST_MESSAGE_SEGMENT_DONE:
          /* We shouldn't see any segement messages, since we didn't do a segment seek */
          GST_WARNING ("Saw a Segment start/stop");
          fail_if (FALSE);
          carry_on = FALSE;
          break;
        case GST_MESSAGE_ERROR:
          GST_WARNING ("Saw an ERROR");
          fail_if (TRUE);
        default:
          break;
      }
      gst_mini_object_unref (GST_MINI_OBJECT (message));
    }
  }

  GST_DEBUG ("Setting pipeline to NULL");

  gst_object_unref (GST_OBJECT (sinkpad));

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_READY) == GST_STATE_CHANGE_FAILURE);

  fail_if (collect->expected_segments != NULL);

  GST_DEBUG ("Resetted pipeline to READY");

  g_free (collect);
}

GST_END_TEST;

Suite *
gnonlin_suite (void)
{
  Suite *s = suite_create ("gnonlin");
  TCase *tc_chain = tcase_create ("gnlsource");
  guint major, minor, micro, nano;

  suite_add_tcase (s, tc_chain);

  tcase_add_test (tc_chain, test_simple_videotestsrc);
  tcase_add_test (tc_chain, test_videotestsrc_in_bin);

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
