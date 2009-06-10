#include "common.h"

GST_START_TEST (test_one_space_another)
{
  GstElement *pipeline;
  GstElement *comp, *sink, *source1, *source2;
  CollectStructure *collect;
  GstBus *bus;
  GstMessage *message;
  gboolean carry_on = TRUE;
  guint64 start, stop;
  gint64 duration;
  GstPad *sinkpad;

  pipeline = gst_pipeline_new ("test_pipeline");
  comp =
      gst_element_factory_make_or_warn ("gnlcomposition", "test_composition");
  fail_if (comp == NULL);

  /*
     Source 1
     Start : 0s
     Duration : 1s
     Priority : 1
   */
  source1 = videotest_gnl_src ("source1", 0, 1 * GST_SECOND, 1, 1);
  fail_if (source1 == NULL);
  check_start_stop_duration (source1, 0, 1 * GST_SECOND, 1 * GST_SECOND);

  /*
     Source 2
     Start : 2s
     Duration : 1s
     Priority : 1
   */
  source2 = videotest_gnl_src ("source2", 2 * GST_SECOND, 1 * GST_SECOND, 2, 1);
  fail_if (source2 == NULL);
  check_start_stop_duration (source2, 2 * GST_SECOND, 3 * GST_SECOND,
      1 * GST_SECOND);

  /* Add one source */

  gst_bin_add (GST_BIN (comp), source1);
  check_start_stop_duration (comp, 0, 1 * GST_SECOND, 1 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);

  /* Second source */

  gst_bin_add (GST_BIN (comp), source2);
  check_start_stop_duration (comp, 0, 3 * GST_SECOND, 3 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source2, "source2", 1);

  /* Remove first source */

  gst_object_ref (source1);
  gst_bin_remove (GST_BIN (comp), source1);
  check_start_stop_duration (comp, 2 * GST_SECOND, 3 * GST_SECOND,
      1 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);

  /* Re-add first source */

  gst_bin_add (GST_BIN (comp), source1);
  check_start_stop_duration (comp, 0, 3 * GST_SECOND, 3 * GST_SECOND);
  gst_object_unref (source1);

  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);

  sink = gst_element_factory_make_or_warn ("fakesink", "sink");
  fail_if (sink == NULL);

  gst_bin_add_many (GST_BIN (pipeline), comp, sink, NULL);

  /* Shared data */
  collect = g_new0 (CollectStructure, 1);
  collect->comp = comp;
  collect->sink = sink;

  /* Expected segments */
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME, 0, 1 * GST_SECOND, 0));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          2 * GST_SECOND, 3 * GST_SECOND, 2 * GST_SECOND));

  g_signal_connect (G_OBJECT (comp), "pad-added",
      G_CALLBACK (composition_pad_added_cb), collect);

  sinkpad = gst_element_get_pad (sink, "sink");
  gst_pad_add_event_probe (sinkpad, G_CALLBACK (sinkpad_event_probe), collect);
  gst_pad_add_buffer_probe (sinkpad, G_CALLBACK (sinkpad_buffer_probe),
      collect);

  bus = gst_element_get_bus (GST_ELEMENT (pipeline));

  GST_DEBUG ("Setting pipeline to PLAYING");
  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE);

  GST_DEBUG ("Let's poll the bus");

  while (carry_on) {
    message = gst_bus_poll (bus, GST_MESSAGE_ANY, GST_SECOND / 2);
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
          fail_if (TRUE);
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

  GST_DEBUG ("Resetted pipeline to READY");

  /* Expected segments */
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME, 0, 1 * GST_SECOND, 0));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          2 * GST_SECOND, 3 * GST_SECOND, 2 * GST_SECOND));
  collect->gotsegment = FALSE;


  GST_DEBUG ("Setting pipeline to PLAYING again");

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE);

  carry_on = TRUE;

  GST_DEBUG ("Let's poll the bus");

  while (carry_on) {
    message = gst_bus_poll (bus, GST_MESSAGE_ANY, GST_SECOND / 2);
    if (message) {
      switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_EOS:
          /* we should check if we really finished here */
          carry_on = FALSE;
          break;
        case GST_MESSAGE_SEGMENT_START:
        case GST_MESSAGE_SEGMENT_DONE:
          /* We shouldn't see any segement messages, since we didn't do a segment seek */
          GST_WARNING ("Saw a Segment start/stop");
          fail_if (TRUE);
          break;
        case GST_MESSAGE_ERROR:
          GST_ERROR ("Saw an ERROR");
          fail_if (TRUE);
        default:
          break;
      }
      gst_mini_object_unref (GST_MINI_OBJECT (message));
    } else {
      GST_DEBUG ("bus_poll responded, but there wasn't any message...");
    }
  }

  fail_if (collect->expected_segments != NULL);

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE);

  gst_object_unref (GST_OBJECT (sinkpad));
  ASSERT_OBJECT_REFCOUNT_BETWEEN (pipeline, "main pipeline", 1, 2);
  gst_object_unref (pipeline);
  ASSERT_OBJECT_REFCOUNT_BETWEEN (bus, "main bus", 1, 2);
  gst_object_unref (bus);

  g_free (collect);
}

GST_END_TEST;

GST_START_TEST (test_one_default_another)
{
  GstElement *pipeline;
  GstElement *comp, *sink, *source1, *source2, *source3, *defaultsrc;
  CollectStructure *collect;
  GstBus *bus;
  GstMessage *message;
  gboolean carry_on = TRUE;
  guint64 start, stop;
  gint64 duration;
  GstPad *sinkpad;

  pipeline = gst_pipeline_new ("test_pipeline");
  comp =
      gst_element_factory_make_or_warn ("gnlcomposition", "test_composition");
  fail_if (comp == NULL);

  /*
     defaultsrc source
     Start : 0s
     Duration : 5s
     Priority : 2
   */

  defaultsrc =
      videotest_gnl_src ("defaultsrc", 0, 5 * GST_SECOND, 2, G_MAXUINT32);
  fail_if (defaultsrc == NULL);
  check_start_stop_duration (defaultsrc, 0, 5 * GST_SECOND, 5 * GST_SECOND);

  /*
     Source 1
     Start : 1s
     Duration : 1s
     Priority : 1
   */
  source1 = videotest_gnl_src ("source1", 1 * GST_SECOND, 1 * GST_SECOND, 1, 1);
  fail_if (source1 == NULL);
  check_start_stop_duration (source1, GST_SECOND, 2 * GST_SECOND,
      1 * GST_SECOND);

  /*
     Source 2
     Start : 3s
     Duration : 1s
     Priority : 1
   */
  source2 = videotest_gnl_src ("source2", 3 * GST_SECOND, 1 * GST_SECOND, 2, 1);
  fail_if (source2 == NULL);
  check_start_stop_duration (source2, 3 * GST_SECOND, 4 * GST_SECOND,
      1 * GST_SECOND);

  /*
     Source 3
     Start : 4s
     Duration : 1s
     Priority : 1
   */
  source3 = videotest_gnl_src ("source3", 4 * GST_SECOND, 1 * GST_SECOND, 2, 1);
  fail_if (source3 == NULL);
  check_start_stop_duration (source3, 4 * GST_SECOND, 5 * GST_SECOND,
      1 * GST_SECOND);


  /* Add one source */

  gst_bin_add (GST_BIN (comp), source1);
  check_start_stop_duration (comp, GST_SECOND, 2 * GST_SECOND, 1 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);

  /* defaultsrc source */

  gst_bin_add (GST_BIN (comp), defaultsrc);
  check_start_stop_duration (comp, 0, 2 * GST_SECOND, 2 * GST_SECOND);
  check_start_stop_duration (defaultsrc, 0, 2 * GST_SECOND, 2 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (defaultsrc, "defaultsrc", 1);

  /* Second source */

  gst_bin_add (GST_BIN (comp), source2);
  check_start_stop_duration (comp, 0, 4 * GST_SECOND, 4 * GST_SECOND);
  check_start_stop_duration (defaultsrc, 0, 4 * GST_SECOND, 4 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source2, "source2", 1);


  /* Third source */

  gst_bin_add (GST_BIN (comp), source3);
  check_start_stop_duration (comp, 0, 5 * GST_SECOND, 5 * GST_SECOND);
  check_start_stop_duration (defaultsrc, 0, 5 * GST_SECOND, 5 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source3, "source3", 1);


  sink = gst_element_factory_make_or_warn ("fakesink", "sink");
  fail_if (sink == NULL);

  gst_bin_add_many (GST_BIN (pipeline), comp, sink, NULL);

  /* Shared data */
  collect = g_new0 (CollectStructure, 1);
  collect->comp = comp;
  collect->sink = sink;

  /* Expected segments */
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME, 0, 1 * GST_SECOND, 0));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          1 * GST_SECOND, 2 * GST_SECOND, 1 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          2 * GST_SECOND, 3 * GST_SECOND, 2 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          3 * GST_SECOND, 4 * GST_SECOND, 3 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          4 * GST_SECOND, 5 * GST_SECOND, 4 * GST_SECOND));

  g_signal_connect (G_OBJECT (comp), "pad-added",
      G_CALLBACK (composition_pad_added_cb), collect);

  sinkpad = gst_element_get_pad (sink, "sink");
  gst_pad_add_event_probe (sinkpad, G_CALLBACK (sinkpad_event_probe), collect);
  gst_pad_add_buffer_probe (sinkpad, G_CALLBACK (sinkpad_buffer_probe),
      collect);

  bus = gst_element_get_bus (GST_ELEMENT (pipeline));

  GST_DEBUG ("Setting pipeline to PLAYING");
  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE);

  GST_DEBUG ("Let's poll the bus");

  while (carry_on) {
    message = gst_bus_poll (bus, GST_MESSAGE_ANY, GST_SECOND / 2);
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
          fail_if (TRUE);
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

  GST_DEBUG ("Resetted pipeline to READY");

  /* Expected segments */
  /* Expected segments */
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME, 0, 1 * GST_SECOND, 0));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          1 * GST_SECOND, 2 * GST_SECOND, 1 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          2 * GST_SECOND, 3 * GST_SECOND, 2 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          3 * GST_SECOND, 4 * GST_SECOND, 3 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          4 * GST_SECOND, 5 * GST_SECOND, 4 * GST_SECOND));
  collect->gotsegment = FALSE;


  GST_DEBUG ("Setting pipeline to PLAYING again");

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE);

  carry_on = TRUE;

  GST_DEBUG ("Let's poll the bus");

  while (carry_on) {
    message = gst_bus_poll (bus, GST_MESSAGE_ANY, GST_SECOND / 2);
    if (message) {
      switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_EOS:
          /* we should check if we really finished here */
          carry_on = FALSE;
          break;
        case GST_MESSAGE_SEGMENT_START:
        case GST_MESSAGE_SEGMENT_DONE:
          /* We shouldn't see any segement messages, since we didn't do a segment seek */
          GST_WARNING ("Saw a Segment start/stop");
          fail_if (TRUE);
          break;
        case GST_MESSAGE_ERROR:
          GST_ERROR ("Saw an ERROR");
          fail_if (TRUE);
        default:
          break;
      }
      gst_mini_object_unref (GST_MINI_OBJECT (message));
    } else {
      GST_DEBUG ("bus_poll responded, but there wasn't any message...");
    }
  }

  fail_if (collect->expected_segments != NULL);

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE);

  gst_object_unref (GST_OBJECT (sinkpad));
  ASSERT_OBJECT_REFCOUNT_BETWEEN (pipeline, "main pipeline", 1, 2);
  gst_object_unref (pipeline);
  ASSERT_OBJECT_REFCOUNT_BETWEEN (bus, "main bus", 1, 2);
  gst_object_unref (bus);

  g_free (collect);
}

GST_END_TEST;



GST_START_TEST (test_renegotiation)
{
  GstElement *pipeline;
  GstElement *comp, *sink, *source1, *source2, *source3, *defaultsrc;
  GstElement *audioconvert;
  CollectStructure *collect;
  GstBus *bus;
  GstMessage *message;
  gboolean carry_on = TRUE;
  guint64 start, stop;
  gint64 duration;
  GstPad *sinkpad;
  GstCaps *caps;

  pipeline = gst_pipeline_new ("test_pipeline");
  comp =
      gst_element_factory_make_or_warn ("gnlcomposition", "test_composition");
  fail_if (comp == NULL);

  /*
     Source 1
     Start : 1s
     Duration : 1s
     Priority : 1
   */
  source1 =
      audiotest_bin_src ("source1", 0 * GST_SECOND, 1 * GST_SECOND, 1, FALSE);
  check_start_stop_duration (source1, 0 * GST_SECOND, 1 * GST_SECOND,
      1 * GST_SECOND);

  /*
     Source 2
     Start : 1s
     Duration : 1s
     Priority : 1
   */
  source2 =
      audiotest_bin_src ("source2", 1 * GST_SECOND, 1 * GST_SECOND, 1, TRUE);
  check_start_stop_duration (source2, 1 * GST_SECOND, 2 * GST_SECOND,
      1 * GST_SECOND);

  /*
     Source 3
     Start : 2s
     Duration : 1s
     Priority : 1
   */
  source3 =
      audiotest_bin_src ("source3", 2 * GST_SECOND, 1 * GST_SECOND, 1, FALSE);
  check_start_stop_duration (source3, 2 * GST_SECOND, 3 * GST_SECOND,
      1 * GST_SECOND);

  /* Add one source */

  gst_bin_add (GST_BIN (comp), source1);
  check_start_stop_duration (comp, 0, 1 * GST_SECOND, 1 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);

  /* Second source */

  gst_bin_add (GST_BIN (comp), source2);
  check_start_stop_duration (comp, 0, 2 * GST_SECOND, 2 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source2, "source2", 1);


  /* Third source */

  gst_bin_add (GST_BIN (comp), source3);
  check_start_stop_duration (comp, 0, 3 * GST_SECOND, 3 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source3, "source3", 1);


  sink = gst_element_factory_make_or_warn ("fakesink", "sink");
  audioconvert = gst_element_factory_make_or_warn ("audioconvert", "aconv");

  gst_bin_add_many (GST_BIN (pipeline), comp, audioconvert, sink, NULL);
  caps = gst_caps_from_string ("audio/x-raw-int");
  gst_element_link_filtered (audioconvert, sink, caps);
  gst_caps_unref (caps);

  /* Shared data */
  collect = g_new0 (CollectStructure, 1);
  collect->comp = comp;
  collect->sink = audioconvert;

  /* Expected segments */
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME, 0, 1 * GST_SECOND, 0));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          1 * GST_SECOND, 2 * GST_SECOND, 1 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          2 * GST_SECOND, 3 * GST_SECOND, 2 * GST_SECOND));

  g_signal_connect (G_OBJECT (comp), "pad-added",
      G_CALLBACK (composition_pad_added_cb), collect);

  sinkpad = gst_element_get_pad (sink, "sink");
  gst_pad_add_event_probe (sinkpad, G_CALLBACK (sinkpad_event_probe), collect);
  gst_pad_add_buffer_probe (sinkpad, G_CALLBACK (sinkpad_buffer_probe),
      collect);

  bus = gst_element_get_bus (GST_ELEMENT (pipeline));

  GST_DEBUG ("Setting pipeline to PLAYING");
  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE);

  GST_DEBUG ("Let's poll the bus");

  while (carry_on) {
    message = gst_bus_poll (bus, GST_MESSAGE_ANY, GST_SECOND / 2);
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
          fail_if (TRUE);
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

  GST_DEBUG ("Resetted pipeline to READY");

  /* Expected segments */
  /* Expected segments */
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME, 0, 1 * GST_SECOND, 0));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          1 * GST_SECOND, 2 * GST_SECOND, 1 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          2 * GST_SECOND, 3 * GST_SECOND, 2 * GST_SECOND));
  collect->gotsegment = FALSE;


  GST_DEBUG ("Setting pipeline to PLAYING again");

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE);

  carry_on = TRUE;

  GST_DEBUG ("Let's poll the bus");

  while (carry_on) {
    message = gst_bus_poll (bus, GST_MESSAGE_ANY, GST_SECOND / 2);
    if (message) {
      switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_EOS:
          /* we should check if we really finished here */
          carry_on = FALSE;
          break;
        case GST_MESSAGE_SEGMENT_START:
        case GST_MESSAGE_SEGMENT_DONE:
          /* We shouldn't see any segement messages, since we didn't do a segment seek */
          GST_WARNING ("Saw a Segment start/stop");
          fail_if (TRUE);
          break;
        case GST_MESSAGE_ERROR:
          GST_ERROR ("Saw an ERROR");
          fail_if (TRUE);
        default:
          break;
      }
      gst_mini_object_unref (GST_MINI_OBJECT (message));
    } else {
      GST_DEBUG ("bus_poll responded, but there wasn't any message...");
    }
  }

  fail_if (collect->expected_segments != NULL);

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE);

  gst_object_unref (GST_OBJECT (sinkpad));
  ASSERT_OBJECT_REFCOUNT_BETWEEN (pipeline, "main pipeline", 1, 2);
  gst_object_unref (pipeline);
  ASSERT_OBJECT_REFCOUNT_BETWEEN (bus, "main bus", 1, 2);
  gst_object_unref (bus);

  g_free (collect);
}

GST_END_TEST;


GST_START_TEST (test_one_bin_space_another)
{
  GstElement *pipeline;
  GstElement *comp, *sink, *source1, *source2;
  CollectStructure *collect;
  GstBus *bus;
  GstMessage *message;
  gboolean carry_on = TRUE;
  guint64 start, stop;
  gint64 duration;
  GstPad *sinkpad;

  pipeline = gst_pipeline_new ("test_pipeline");
  comp =
      gst_element_factory_make_or_warn ("gnlcomposition", "test_composition");
  fail_if (comp == NULL);

  /*
     Source 1
     Start : 0s
     Duration : 1s
     Priority : 1
   */
  source1 = videotest_in_bin_gnl_src ("source1", 0, 1 * GST_SECOND, 1, 1);
  if (source1 == NULL) {
    gst_object_unref (pipeline);
    gst_object_unref (comp);
    return;
  }
  check_start_stop_duration (source1, 0, 1 * GST_SECOND, 1 * GST_SECOND);

  /*
     Source 2
     Start : 2s
     Duration : 1s
     Priority : 1
   */
  source2 =
      videotest_in_bin_gnl_src ("source2", 2 * GST_SECOND, 1 * GST_SECOND, 1,
      1);
  fail_if (source2 == NULL);
  check_start_stop_duration (source2, 2 * GST_SECOND, 3 * GST_SECOND,
      1 * GST_SECOND);

  /* Add one source */

  gst_bin_add (GST_BIN (comp), source1);
  check_start_stop_duration (comp, 0, 1 * GST_SECOND, 1 * GST_SECOND);

  /* Second source */

  gst_bin_add (GST_BIN (comp), source2);
  check_start_stop_duration (comp, 0, 3 * GST_SECOND, 3 * GST_SECOND);

  /* Remove second source */

  gst_object_ref (source1);
  gst_bin_remove (GST_BIN (comp), source1);
  check_start_stop_duration (comp, 2 * GST_SECOND, 3 * GST_SECOND,
      1 * GST_SECOND);

  /* Re-add second source */

  gst_bin_add (GST_BIN (comp), source1);
  check_start_stop_duration (comp, 0, 3 * GST_SECOND, 3 * GST_SECOND);
  gst_object_unref (source1);

  sink = gst_element_factory_make_or_warn ("fakesink", "sink");
  fail_if (sink == NULL);

  gst_bin_add_many (GST_BIN (pipeline), comp, sink, NULL);

  /* Shared data */
  collect = g_new0 (CollectStructure, 1);
  collect->comp = comp;
  collect->sink = sink;

  /* Expected segments */
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME, 0, 1 * GST_SECOND, 0));

  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          2 * GST_SECOND, 3 * GST_SECOND, 2 * GST_SECOND));

  g_signal_connect (G_OBJECT (comp), "pad-added",
      G_CALLBACK (composition_pad_added_cb), collect);

  sinkpad = gst_element_get_pad (sink, "sink");
  gst_pad_add_event_probe (sinkpad, G_CALLBACK (sinkpad_event_probe), collect);
  gst_pad_add_buffer_probe (sinkpad, G_CALLBACK (sinkpad_buffer_probe),
      collect);

  bus = gst_element_get_bus (GST_ELEMENT (pipeline));

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE);

  while (carry_on) {
    message = gst_bus_poll (bus, GST_MESSAGE_ANY, GST_SECOND / 2);
    if (message) {
      switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_EOS:
          /* we should check if we really finished here */
          carry_on = FALSE;
          break;
        case GST_MESSAGE_SEGMENT_START:
        case GST_MESSAGE_SEGMENT_DONE:
          /* check if the segment is the correct one (0s-4s) */
          carry_on = FALSE;
          break;
        case GST_MESSAGE_ERROR:
          fail_if (TRUE);
        default:
          break;
      }
      gst_message_unref (message);
    }
  }

  fail_if (collect->expected_segments != NULL);

  fail_if (gst_element_set_state (GST_ELEMENT (pipeline),
          GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE);

  gst_object_unref (GST_OBJECT (sinkpad));
  gst_bus_poll (bus, GST_MESSAGE_ANY, GST_SECOND / 2);
  ASSERT_OBJECT_REFCOUNT_BETWEEN (pipeline, "main pipeline", 1, 2);
  gst_object_unref (pipeline);
  ASSERT_OBJECT_REFCOUNT_BETWEEN (bus, "main bus", 1, 2);
  gst_object_unref (bus);

  g_free (collect);
}

GST_END_TEST;

GST_START_TEST (test_one_above_another)
{
  GstElement *pipeline;
  GstElement *comp, *sink, *source1, *source2;
  CollectStructure *collect;
  GstBus *bus;
  GstMessage *message;
  gboolean carry_on = TRUE;
  guint64 start, stop;
  gint64 duration;
  GstPad *sinkpad;

  pipeline = gst_pipeline_new ("test_pipeline");
  comp =
      gst_element_factory_make_or_warn ("gnlcomposition", "test_composition");
  fail_if (comp == NULL);

  /*
     Source 1
     Start : 0s
     Duration : 2s
     Priority : 2
   */
  source1 = videotest_gnl_src ("source1", 0, 2 * GST_SECOND, 1, 2);
  fail_if (source1 == NULL);
  check_start_stop_duration (source1, 0, 2 * GST_SECOND, 2 * GST_SECOND);

  /*
     Source 2
     Start : 2s
     Duration : 2s
     Priority : 1
   */
  source2 = videotest_gnl_src ("source2", 1 * GST_SECOND, 2 * GST_SECOND, 2, 1);
  fail_if (source2 == NULL);
  check_start_stop_duration (source2, 1 * GST_SECOND, 3 * GST_SECOND,
      2 * GST_SECOND);

  /* Add one source */

  gst_bin_add (GST_BIN (comp), source1);
  check_start_stop_duration (comp, 0, 2 * GST_SECOND, 2 * GST_SECOND);

  /* Second source */

  gst_bin_add (GST_BIN (comp), source2);
  check_start_stop_duration (comp, 0, 3 * GST_SECOND, 3 * GST_SECOND);

  /* Remove second source */

  gst_object_ref (source1);
  gst_bin_remove (GST_BIN (comp), source1);
  check_start_stop_duration (comp, 1 * GST_SECOND, 3 * GST_SECOND,
      2 * GST_SECOND);

  /* Re-add second source */

  gst_bin_add (GST_BIN (comp), source1);
  check_start_stop_duration (comp, 0, 3 * GST_SECOND, 3 * GST_SECOND);
  gst_object_unref (source1);

  sink = gst_element_factory_make_or_warn ("fakesink", "sink");
  fail_if (sink == NULL);

  gst_bin_add_many (GST_BIN (pipeline), comp, sink, NULL);

  /* Shared data */
  collect = g_new0 (CollectStructure, 1);
  collect->comp = comp;
  collect->sink = sink;

  /* Expected segments */
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME, 0, 1 * GST_SECOND, 0));

  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          1 * GST_SECOND, 3 * GST_SECOND, 1 * GST_SECOND));

  g_signal_connect (G_OBJECT (comp), "pad-added",
      G_CALLBACK (composition_pad_added_cb), collect);

  sinkpad = gst_element_get_pad (sink, "sink");
  gst_pad_add_event_probe (sinkpad, G_CALLBACK (sinkpad_event_probe), collect);
  gst_pad_add_buffer_probe (sinkpad, G_CALLBACK (sinkpad_buffer_probe),
      collect);

  bus = gst_element_get_bus (GST_ELEMENT (pipeline));

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  while (carry_on) {
    message = gst_bus_poll (bus, GST_MESSAGE_ANY, GST_SECOND / 2);
    if (message) {
      switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_EOS:
          /* we should check if we really finished here */
          carry_on = FALSE;
          break;
        case GST_MESSAGE_SEGMENT_START:
        case GST_MESSAGE_SEGMENT_DONE:
          /* check if the segment is the correct one (0s-4s) */
          carry_on = FALSE;
          break;
        case GST_MESSAGE_ERROR:
          fail_if (TRUE);
        default:
          break;
      }
      gst_message_unref (message);
    }
  }

  fail_if (collect->expected_segments != NULL);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);

  gst_object_unref (GST_OBJECT (sinkpad));
  ASSERT_OBJECT_REFCOUNT_BETWEEN (pipeline, "main pipeline", 1, 2);
  gst_object_unref (pipeline);
  ASSERT_OBJECT_REFCOUNT_BETWEEN (bus, "main bus", 1, 2);
  gst_object_unref (bus);

  g_free (collect);
}

GST_END_TEST;

Suite *
gnonlin_suite (void)
{
  Suite *s = suite_create ("gnonlin");
  TCase *tc_chain = tcase_create ("complex");
  guint major, minor, micro, nano;

  suite_add_tcase (s, tc_chain);

  /* Only add the following test for core > 0.10.4 */
  gst_version (&major, &minor, &micro, &nano);
  if ((micro > 4) || (micro == 4 && nano > 0)) {
    tcase_add_test (tc_chain, test_one_space_another);
    tcase_add_test (tc_chain, test_one_default_another);
    tcase_add_test (tc_chain, test_renegotiation);
    tcase_add_test (tc_chain, test_one_bin_space_another);
    tcase_add_test (tc_chain, test_one_above_another);
  }
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
