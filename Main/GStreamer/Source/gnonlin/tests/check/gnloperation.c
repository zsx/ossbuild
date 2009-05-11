#include "common.h"

GST_START_TEST (test_simple_operation)
{
  GstElement *pipeline;
  guint64 start, stop;
  gint64 duration;
  GstElement *comp, *oper, *source, *sink;
  CollectStructure *collect;
  GstBus *bus;
  GstMessage *message;
  gboolean carry_on = TRUE;
  GstPad *sinkpad;

  pipeline = gst_pipeline_new ("test_pipeline");
  comp =
      gst_element_factory_make_or_warn ("gnlcomposition", "test_composition");

  /*
     source
     Start : 0s
     Duration : 3s
     Priority : 1
   */

  source = videotest_gnl_src ("source", 0, 3 * GST_SECOND, 1, 1);
  fail_if (source == NULL);
  check_start_stop_duration (source, 0, 3 * GST_SECOND, 3 * GST_SECOND);

  /*
     operation
     Start : 1s
     Duration : 1s
     Priority : 0
   */

  oper = new_operation ("oper", "identity", 1 * GST_SECOND, 1 * GST_SECOND, 0);
  fail_if (oper == NULL);
  check_start_stop_duration (oper, 1 * GST_SECOND, 2 * GST_SECOND,
      1 * GST_SECOND);

  /* Add source */
  ASSERT_OBJECT_REFCOUNT (source, "source", 1);
  ASSERT_OBJECT_REFCOUNT (oper, "oper", 1);

  gst_bin_add (GST_BIN (comp), source);
  check_start_stop_duration (comp, 0, 3 * GST_SECOND, 3 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source, "source", 1);

  /* Add operaton */

  gst_bin_add (GST_BIN (comp), oper);
  check_start_stop_duration (comp, 0, 3 * GST_SECOND, 3 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (oper, "oper", 1);

  /* remove source */

  gst_object_ref (source);
  gst_bin_remove (GST_BIN (comp), source);
  check_start_stop_duration (comp, 1 * GST_SECOND, 2 * GST_SECOND,
      1 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source, "source", 1);

  /* re-add source */
  gst_bin_add (GST_BIN (comp), source);
  check_start_stop_duration (comp, 0, 3 * GST_SECOND, 3 * GST_SECOND);
  gst_object_unref (source);

  ASSERT_OBJECT_REFCOUNT (source, "source", 1);


  sink = gst_element_factory_make_or_warn ("fakesink", "sink");
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

  g_signal_connect (G_OBJECT (comp), "pad-added",
      G_CALLBACK (composition_pad_added_cb), collect);

  sinkpad = gst_element_get_pad (sink, "sink");
  gst_pad_add_event_probe (sinkpad, G_CALLBACK (sinkpad_event_probe), collect);
  gst_pad_add_buffer_probe (sinkpad, G_CALLBACK (sinkpad_buffer_probe),
      collect);

  bus = gst_element_get_bus (GST_ELEMENT (pipeline));

  GST_DEBUG ("Setting pipeline to PLAYING");
  ASSERT_OBJECT_REFCOUNT (source, "source", 1);

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

GST_START_TEST (test_pyramid_operations)
{
  GstElement *pipeline;
  guint64 start, stop;
  gint64 duration;
  GstElement *comp, *oper1, *oper2, *source, *sink;
  CollectStructure *collect;
  GstBus *bus;
  GstMessage *message;
  gboolean carry_on = TRUE;
  GstPad *sinkpad;

  pipeline = gst_pipeline_new ("test_pipeline");
  comp =
      gst_element_factory_make_or_warn ("gnlcomposition", "test_composition");

  /*
     source
     Start : 0s
     Duration : 10s
     Priority : 2
   */

  source = videotest_gnl_src ("source", 0, 10 * GST_SECOND, 1, 2);
  check_start_stop_duration (source, 0, 10 * GST_SECOND, 10 * GST_SECOND);

  /*
     operation1
     Start : 4s
     Duration : 2s
     Priority : 1
   */

  oper1 =
      new_operation ("oper1", "identity", 4 * GST_SECOND, 2 * GST_SECOND, 1);
  check_start_stop_duration (oper1, 4 * GST_SECOND, 6 * GST_SECOND,
      2 * GST_SECOND);

  /*
     operation2
     Start : 2s
     Duration : 6s
     Priority : 0
   */

  oper2 =
      new_operation ("oper2", "identity", 2 * GST_SECOND, 6 * GST_SECOND, 0);
  check_start_stop_duration (oper2, 2 * GST_SECOND, 8 * GST_SECOND,
      6 * GST_SECOND);

  /* Add source */
  ASSERT_OBJECT_REFCOUNT (source, "source", 1);
  ASSERT_OBJECT_REFCOUNT (oper1, "oper1", 1);
  ASSERT_OBJECT_REFCOUNT (oper2, "oper2", 1);

  gst_bin_add (GST_BIN (comp), source);
  check_start_stop_duration (comp, 0, 10 * GST_SECOND, 10 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source, "source", 1);

  /* Add operation 1 */

  gst_bin_add (GST_BIN (comp), oper1);
  check_start_stop_duration (comp, 0, 10 * GST_SECOND, 10 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (oper1, "oper1", 1);

  /* Add operation 2 */

  gst_bin_add (GST_BIN (comp), oper2);
  check_start_stop_duration (comp, 0, 10 * GST_SECOND, 10 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (oper1, "oper2", 1);



  sink = gst_element_factory_make_or_warn ("fakesink", "sink");
  gst_bin_add_many (GST_BIN (pipeline), comp, sink, NULL);

  /* Shared data */
  collect = g_new0 (CollectStructure, 1);
  collect->comp = comp;
  collect->sink = sink;

  /* Expected segments */
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME, 0, 2 * GST_SECOND, 0));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          2 * GST_SECOND, 4 * GST_SECOND, 2 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          4 * GST_SECOND, 6 * GST_SECOND, 4 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          6 * GST_SECOND, 8 * GST_SECOND, 6 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          8 * GST_SECOND, 10 * GST_SECOND, 8 * GST_SECOND));

  g_signal_connect (G_OBJECT (comp), "pad-added",
      G_CALLBACK (composition_pad_added_cb), collect);

  sinkpad = gst_element_get_pad (sink, "sink");
  gst_pad_add_event_probe (sinkpad, G_CALLBACK (sinkpad_event_probe), collect);
  gst_pad_add_buffer_probe (sinkpad, G_CALLBACK (sinkpad_buffer_probe),
      collect);

  bus = gst_element_get_bus (GST_ELEMENT (pipeline));

  GST_DEBUG ("Setting pipeline to PLAYING");
  ASSERT_OBJECT_REFCOUNT (source, "source", 1);

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
      segment_new (1.0, GST_FORMAT_TIME, 0, 2 * GST_SECOND, 0));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          2 * GST_SECOND, 4 * GST_SECOND, 2 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          4 * GST_SECOND, 6 * GST_SECOND, 4 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          6 * GST_SECOND, 8 * GST_SECOND, 6 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          8 * GST_SECOND, 10 * GST_SECOND, 8 * GST_SECOND));
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

GST_START_TEST (test_pyramid_operations2)
{
  GstElement *pipeline;
  guint64 start, stop;
  gint64 duration;
  GstElement *comp, *oper, *source1, *source2, *def, *sink;
  CollectStructure *collect;
  GstBus *bus;
  GstMessage *message;
  gboolean carry_on = TRUE;
  GstPad *sinkpad;

  pipeline = gst_pipeline_new ("test_pipeline");
  comp =
      gst_element_factory_make_or_warn ("gnlcomposition", "test_composition");

  /*
     source1
     Start : 0s
     Duration : 2s
     Priority : 2
   */

  source1 = videotest_gnl_src ("source1", 0, 2 * GST_SECOND, 1, 2);
  check_start_stop_duration (source1, 0, 2 * GST_SECOND, 2 * GST_SECOND);

  /*
     operation
     Start : 1s
     Duration : 4s
     Priority : 1
   */

  oper = new_operation ("oper", "identity", 1 * GST_SECOND, 4 * GST_SECOND, 1);
  check_start_stop_duration (oper, 1 * GST_SECOND, 5 * GST_SECOND,
      4 * GST_SECOND);

  /*
     source2
     Start : 4s
     Duration : 2s
     Priority : 2
   */

  source2 = videotest_gnl_src ("source2", 4 * GST_SECOND, 2 * GST_SECOND, 1, 2);
  check_start_stop_duration (source2, 4 * GST_SECOND, 6 * GST_SECOND,
      2 * GST_SECOND);

  /*
     def (default source)
     Priority = G_MAXUINT32
   */
  def =
      videotest_gnl_src ("default", 0 * GST_SECOND, 0 * GST_SECOND, 1,
      G_MAXUINT32);

  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);
  ASSERT_OBJECT_REFCOUNT (source2, "source2", 1);
  ASSERT_OBJECT_REFCOUNT (oper, "oper", 1);
  ASSERT_OBJECT_REFCOUNT (def, "default", 1);

  /* Add source 1 */

  gst_bin_add (GST_BIN (comp), source1);
  check_start_stop_duration (comp, 0, 2 * GST_SECOND, 2 * GST_SECOND);

  /* Add source 2 */

  gst_bin_add (GST_BIN (comp), source2);
  check_start_stop_duration (comp, 0, 6 * GST_SECOND, 6 * GST_SECOND);

  /* Add operation */

  gst_bin_add (GST_BIN (comp), oper);
  check_start_stop_duration (comp, 0, 6 * GST_SECOND, 6 * GST_SECOND);

  /* Add default */

  gst_bin_add (GST_BIN (comp), def);
  check_start_stop_duration (comp, 0, 6 * GST_SECOND, 6 * GST_SECOND);



  sink = gst_element_factory_make_or_warn ("fakesink", "sink");
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
          2 * GST_SECOND, 4 * GST_SECOND, 2 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          4 * GST_SECOND, 5 * GST_SECOND, 4 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          5 * GST_SECOND, 6 * GST_SECOND, 5 * GST_SECOND));

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
          1 * GST_SECOND, 2 * GST_SECOND, 1 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          2 * GST_SECOND, 4 * GST_SECOND, 2 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          4 * GST_SECOND, 5 * GST_SECOND, 4 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          5 * GST_SECOND, 6 * GST_SECOND, 5 * GST_SECOND));

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





GST_START_TEST (test_complex_operations)
{
  GstElement *pipeline;
  guint64 start, stop;
  gint64 duration;
  GstElement *comp, *oper, *source1, *source2, *sink;
  CollectStructure *collect;
  GstBus *bus;
  GstMessage *message;
  gboolean carry_on = TRUE;
  GstPad *sinkpad;

  pipeline = gst_pipeline_new ("test_pipeline");
  comp =
      gst_element_factory_make_or_warn ("gnlcomposition", "test_composition");

  /*
     source1
     Start : 0s
     Duration : 4s
     Priority : 3
   */

  source1 = videotest_in_bin_gnl_src ("source1", 0, 4 * GST_SECOND, 1, 3);
  fail_if (source1 == NULL);
  check_start_stop_duration (source1, 0, 4 * GST_SECOND, 4 * GST_SECOND);

  /*
     source2
     Start : 2s
     Duration : 4s
     Priority : 2
   */

  source2 =
      videotest_in_bin_gnl_src ("source2", 2 * GST_SECOND, 4 * GST_SECOND, 2,
      2);
  fail_if (source2 == NULL);
  check_start_stop_duration (source2, 2 * GST_SECOND, 6 * GST_SECOND,
      4 * GST_SECOND);

  /*
     operation
     Start : 2s
     Duration : 2s
     Priority : 1
   */

  oper =
      new_operation ("oper", "videomixer", 2 * GST_SECOND, 2 * GST_SECOND, 1);
  fail_if (oper == NULL);
  check_start_stop_duration (oper, 2 * GST_SECOND, 4 * GST_SECOND,
      2 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);
  ASSERT_OBJECT_REFCOUNT (source2, "source2", 1);
  ASSERT_OBJECT_REFCOUNT (oper, "oper", 1);

  /* Add source1 */
  gst_bin_add (GST_BIN (comp), source1);
  check_start_stop_duration (comp, 0, 4 * GST_SECOND, 4 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);

  /* Add source2 */
  gst_bin_add (GST_BIN (comp), source2);
  check_start_stop_duration (comp, 0, 6 * GST_SECOND, 6 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (source2, "source2", 1);

  /* Add operaton */

  gst_bin_add (GST_BIN (comp), oper);
  check_start_stop_duration (comp, 0, 6 * GST_SECOND, 6 * GST_SECOND);

  ASSERT_OBJECT_REFCOUNT (oper, "oper", 1);


  sink = gst_element_factory_make_or_warn ("fakesink", "sink");
  gst_bin_add_many (GST_BIN (pipeline), comp, sink, NULL);

  /* Shared data */
  collect = g_new0 (CollectStructure, 1);
  collect->comp = comp;
  collect->sink = sink;

  /* Expected segments */
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME, 0, 2 * GST_SECOND, 0));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          0 * GST_SECOND, 2 * GST_SECOND, 2 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          4 * GST_SECOND, 6 * GST_SECOND, 4 * GST_SECOND));

  g_signal_connect (G_OBJECT (comp), "pad-added",
      G_CALLBACK (composition_pad_added_cb), collect);

  sinkpad = gst_element_get_pad (sink, "sink");
  gst_pad_add_event_probe (sinkpad, G_CALLBACK (sinkpad_event_probe), collect);
  gst_pad_add_buffer_probe (sinkpad, G_CALLBACK (sinkpad_buffer_probe),
      collect);

  bus = gst_element_get_bus (GST_ELEMENT (pipeline));

  GST_DEBUG ("Setting pipeline to PLAYING");
  ASSERT_OBJECT_REFCOUNT (source1, "source1", 1);
  ASSERT_OBJECT_REFCOUNT (source2, "source2", 1);
  ASSERT_OBJECT_REFCOUNT (oper, "oper", 1);

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
      segment_new (1.0, GST_FORMAT_TIME, 0, 2 * GST_SECOND, 0));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          0 * GST_SECOND, 2 * GST_SECOND, 2 * GST_SECOND));
  collect->expected_segments = g_list_append (collect->expected_segments,
      segment_new (1.0, GST_FORMAT_TIME,
          4 * GST_SECOND, 6 * GST_SECOND, 4 * GST_SECOND));
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

Suite *
gnonlin_suite (void)
{
  Suite *s = suite_create ("gnonlin");
  TCase *tc_chain = tcase_create ("gnloperation");
  guint major, minor, micro, nano;

  suite_add_tcase (s, tc_chain);

  tcase_add_test (tc_chain, test_simple_operation);
  tcase_add_test (tc_chain, test_pyramid_operations);
  tcase_add_test (tc_chain, test_pyramid_operations2);
  tcase_add_test (tc_chain, test_complex_operations);

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
