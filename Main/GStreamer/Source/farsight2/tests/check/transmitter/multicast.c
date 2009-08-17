/* Farsight 2 unit tests for FsMulticastTransmitter
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

#include <gst/check/gstcheck.h>
#include <gst/farsight/fs-transmitter.h>
#include <gst/farsight/fs-conference-iface.h>

#include "check-threadsafe.h"
#include "generic.h"
#include "fake-filter.h"
#include "testutils.h"

gint buffer_count[2] = {0, 0};
GMainLoop *loop = NULL;
gint candidates[2] = {0, 0};
GstElement *pipeline = NULL;
gboolean src_setup[2] = {FALSE, FALSE};

enum {
  FLAG_NOT_SENDING = 1<<0,
  FLAG_RECVONLY_FILTER = 1<<1
};


GST_START_TEST (test_multicasttransmitter_new)
{
  test_transmitter_creation ("multicast");
}
GST_END_TEST;



static void
_handoff_handler (GstElement *element, GstBuffer *buffer, GstPad *pad,
  gpointer user_data)
{
  gint component_id = GPOINTER_TO_INT (user_data);

  ts_fail_unless (GST_BUFFER_SIZE (buffer) == component_id * 10,
    "Buffer is size %d but component_id is %d", GST_BUFFER_SIZE (buffer),
    component_id);

  buffer_count[component_id-1]++;

  /*
  GST_DEBUG ("Buffer %d component: %d size: %u", buffer_count[component_id-1],
    component_id, GST_BUFFER_SIZE (buffer));
  */

  ts_fail_if (buffer_count[component_id-1] > 20,
    "Too many buffers %d > 20 for component",
    buffer_count[component_id-1], component_id);

  if (buffer_count[0] == 20 && buffer_count[1] == 20) {
    /* TEST OVER */
    g_main_loop_quit (loop);
  }
}

static void
_new_active_candidate_pair (FsStreamTransmitter *st, FsCandidate *local,
  FsCandidate *remote, gpointer user_data)
{
  ts_fail_if (local == NULL, "Local candidate NULL");
  ts_fail_if (remote == NULL, "Remote candidate NULL");

  ts_fail_unless (local->component_id == remote->component_id,
    "Local and remote candidates dont have the same component id");

  GST_DEBUG ("New active candidate pair for component %d", local->component_id);

  if (!src_setup[local->component_id-1])
    setup_fakesrc (user_data, pipeline, local->component_id);
  src_setup[local->component_id-1] = TRUE;
}

static gboolean
_start_pipeline (gpointer user_data)
{
  GstElement *pipeline = user_data;

  GST_DEBUG ("Starting pipeline");

  ts_fail_if (gst_element_set_state (pipeline, GST_STATE_PLAYING) ==
    GST_STATE_CHANGE_FAILURE, "Could not set the pipeline to playing");

  return FALSE;
}


static GstElement *
_get_recvonly_filter (FsTransmitter *trans, guint component, gpointer user_data)
{
  if (component == 1)
    return NULL;

  return gst_element_factory_make ("fsfakefilter", NULL);
}

static void
run_multicast_transmitter_test (gint n_parameters, GParameter *params,
                                gint flags)
{
  GError *error = NULL;
  FsTransmitter *trans;
  FsStreamTransmitter *st;
  FsCandidate *tmpcand = NULL;
  GList *candidates = NULL;
  GstBus *bus = NULL;
  guint tos;

  buffer_count[0] = 0;
  buffer_count[1] = 0;

  if (flags & FLAG_NOT_SENDING && flags & FLAG_RECVONLY_FILTER)
    buffer_count[0] = 20;

  if (flags & FLAG_RECVONLY_FILTER)
    fail_unless (fs_fake_filter_register ());

  loop = g_main_loop_new (NULL, FALSE);
  trans = fs_transmitter_new ("multicast", 2, 0, &error);

  if (error) {
    ts_fail ("Error creating transmitter: (%s:%d) %s",
      g_quark_to_string (error->domain), error->code, error->message);
  }

  ts_fail_if (trans == NULL, "No transmitter create, yet error is still NULL");

  g_object_set (trans, "tos", 2, NULL);
  g_object_get (trans, "tos", &tos, NULL);
  ts_fail_unless (tos == 2);

  if (flags & FLAG_RECVONLY_FILTER)
    ts_fail_unless (g_signal_connect (trans, "get-recvonly-filter",
            G_CALLBACK (_get_recvonly_filter), NULL));


  pipeline = setup_pipeline (trans, G_CALLBACK (_handoff_handler));

  st = fs_transmitter_new_stream_transmitter (trans, NULL, n_parameters, params,
    &error);

  if (error) {
    ts_fail ("Error creating stream transmitter: (%s:%d) %s",
        g_quark_to_string (error->domain), error->code, error->message);
  }

  ts_fail_if (st == NULL, "No stream transmitter created, yet error is NULL");

  g_object_set (st, "sending", !(flags & FLAG_NOT_SENDING), NULL);

  bus = gst_element_get_bus (pipeline);
  gst_bus_add_watch (bus, bus_error_callback, NULL);
  gst_object_unref (bus);

  ts_fail_unless (g_signal_connect (st, "new-active-candidate-pair",
      G_CALLBACK (_new_active_candidate_pair), trans),
    "Coult not connect new-active-candidate-pair signal");
  ts_fail_unless (g_signal_connect (st, "error",
      G_CALLBACK (stream_transmitter_error), NULL),
    "Could not connect error signal");

  g_idle_add (_start_pipeline, pipeline);

  tmpcand = fs_candidate_new ("L1", FS_COMPONENT_RTP,
      FS_CANDIDATE_TYPE_MULTICAST, FS_NETWORK_PROTOCOL_UDP,
      "224.0.0.110", 2322);
  tmpcand->ttl = 1;

  candidates = g_list_prepend (candidates, tmpcand);

  tmpcand = fs_candidate_new ("L2", FS_COMPONENT_RTCP,
      FS_CANDIDATE_TYPE_MULTICAST, FS_NETWORK_PROTOCOL_UDP,
      "224.0.0.110", 2323);
  tmpcand->ttl = 1;

  candidates = g_list_prepend (candidates, tmpcand);

  if (!fs_stream_transmitter_set_remote_candidates (st, candidates, &error))
    ts_fail ("Error setting the remote candidates: %p %s", error,
        error ? error->message : "NO ERROR SET");
  ts_fail_unless (error == NULL, "Error is not null after successful candidate"
      " addition");

  fs_candidate_list_destroy (candidates);

  g_main_loop_run (loop);

  g_object_unref (st);

  g_object_unref (trans);

  gst_element_set_state (pipeline, GST_STATE_NULL);

  gst_object_unref (pipeline);

  g_main_loop_unref (loop);
}

GST_START_TEST (test_multicasttransmitter_run)
{
  run_multicast_transmitter_test (0, NULL, 0);
}
GST_END_TEST;


GST_START_TEST (test_multicasttransmitter_run_local_candidates)
{
  GParameter params[1];
  GList *list = NULL;
  FsCandidate *candidate;
  gchar *address = find_multicast_capable_address ();

  if (address == NULL)
    return;

  memset (params, 0, sizeof (GParameter) * 1);

  candidate = fs_candidate_new ("L1", FS_COMPONENT_RTP, FS_CANDIDATE_TYPE_HOST,
      FS_NETWORK_PROTOCOL_UDP, address, 0);
  candidate->ttl = 2;
  list = g_list_prepend (list, candidate);

  candidate = fs_candidate_new ("L2", FS_COMPONENT_RTCP, FS_CANDIDATE_TYPE_HOST,
      FS_NETWORK_PROTOCOL_UDP, address, 0);
  candidate->ttl = 2;
  list = g_list_prepend (list, candidate);

  params[0].name = "preferred-local-candidates";
  g_value_init (&params[0].value, FS_TYPE_CANDIDATE_LIST);
  g_value_set_boxed (&params[0].value, list);

  run_multicast_transmitter_test (1, params, 0);

  g_value_reset (&params[0].value);

  g_free (address);
  fs_candidate_list_destroy (list);
}
GST_END_TEST;

GST_START_TEST (test_multicasttransmitter_with_filter)
{
  run_multicast_transmitter_test (0, NULL,
      FLAG_RECVONLY_FILTER);
}
GST_END_TEST;

GST_START_TEST (test_multicasttransmitter_sending_half)
{
  run_multicast_transmitter_test (0, NULL,
      FLAG_NOT_SENDING | FLAG_RECVONLY_FILTER);
}
GST_END_TEST;



static Suite *
multicasttransmitter_suite (void)
{
  Suite *s = suite_create ("multicasttransmitter");
  TCase *tc_chain;
  GLogLevelFlags fatal_mask;
  gchar *tmp_addr;

  tmp_addr = find_multicast_capable_address ();

  if (!tmp_addr)
    return s;
  else
    g_free (tmp_addr);

  fatal_mask = g_log_set_always_fatal (G_LOG_FATAL_MASK);
  fatal_mask |= G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL;
  g_log_set_always_fatal (fatal_mask);

  tc_chain = tcase_create ("multicast_transmitter");
  tcase_add_test (tc_chain, test_multicasttransmitter_new);
  tcase_add_test (tc_chain, test_multicasttransmitter_run);
  suite_add_tcase (s, tc_chain);

  tc_chain = tcase_create ("multicast_transmitter_local_candidates");
  tcase_add_test (tc_chain, test_multicasttransmitter_run_local_candidates);
  suite_add_tcase (s, tc_chain);

  tc_chain = tcase_create ("multicast_transmitter-with-filter");
  tcase_add_test (tc_chain, test_multicasttransmitter_with_filter);
  suite_add_tcase (s, tc_chain);

  tc_chain = tcase_create ("multicast_transmitter_sending_half");
  tcase_add_test (tc_chain, test_multicasttransmitter_sending_half);
  suite_add_tcase (s, tc_chain);

  return s;
}


GST_CHECK_MAIN (multicasttransmitter);
