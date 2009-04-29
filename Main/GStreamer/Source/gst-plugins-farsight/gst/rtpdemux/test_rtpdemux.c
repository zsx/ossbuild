/*
 * This file is part of the Sofia-SIP package
 *
 * Copyright (C) 2005 Nokia Corporation.
 * Contact: Kai Vehmanen <kai.vehmanen@nokia.com>
 *
 * Based on test application example from 
 * Gstreamer Plugin Writer's Guide (0.9.4.1) 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <glib.h>
#include <gst/gst.h>

struct rtpdemux_state_s { 
  GstElement *pipeline;
};

static gboolean bus_call (GstBus     *bus,
			  GstMessage *msg,
			  gpointer    data)
{
  GMainLoop *loop = data;

  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
      g_print ("End-of-stream\n");
      g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_ERROR: {
      gchar *debug;
      GError *err;

      gst_message_parse_error (msg, &err, &debug);
      g_free (debug);

      g_print ("Error: %s\n", err->message);
      g_error_free (err);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}

static void new_payload_type (GstElement * element, gint pt, GstPad *pad, gpointer data)
{
  struct rtpdemux_state_s *self = (struct rtpdemux_state_s *)data;
  GstElement *pipeline = self->pipeline;
  GstElement *decoder, *depayloader, *sink;
  gchar *padname = gst_pad_get_name (pad);

  g_debug("%s: adding depayloader chain.\n", __func__);

  /* pause the pipeline for to the add the pad */
  gst_element_set_state (pipeline, GST_STATE_PAUSED);

  decoder = gst_element_factory_make("gsmdec", "gsmdec");
  depayloader = gst_element_factory_make("rtpgsmparse", "rtpgsmparse");
  if (!decoder) {
      printf("unable to create a gsmdec.\n");
      exit(1);
  }  
  if (!depayloader) {
      printf("unable to create a rtpgsmparse.\n");
      exit(1);
  }  

  gst_bin_add_many(GST_BIN(pipeline), decoder, depayloader, NULL);
  gst_element_link_pads(element, padname, depayloader, "sink");
  gst_element_link_pads(depayloader, "src", decoder, "sink");

  sink = gst_element_factory_make("alsasink", "alsasink");
  g_object_set(G_OBJECT(sink), "sync", TRUE, NULL);
  gst_bin_add_many(GST_BIN(pipeline), sink, NULL);
  gst_element_link_pads(decoder, "src", sink , "sink");

  /* restart the pipeline */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
}

static void payload_type_change(GstElement * element, gint pt, gpointer data)
{
  g_debug("%s: active PT change to %d\n", __func__, pt);
}

gint
main (gint   argc,
      gchar *argv[])
{
  GstElement *pipeline;
  GstElement *encoder, *payloader, *src;
  GstElement *rtpdemux, *rtpbin, *sink;
  //filesrc, *decoder, *filter, *sink;
  GMainLoop *loop;
  struct rtpdemux_state_s *state = g_new0(struct rtpdemux_state_s, 1);

  /* make sure jrtplib accepts the packets we send via loopback */
  setenv("JRTPLIBC_ACCEPT_OWN", "1", 1);

  /* initialization */
  /* ------------------------ */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* create the pipeline */
  /* ------------------------ */
  pipeline = gst_pipeline_new ("my_pipeline");
  gst_bus_add_watch (gst_pipeline_get_bus (GST_PIPELINE (pipeline)),
		     bus_call, loop);
  state->pipeline = pipeline;

  /* rtpbin */
  /* -------------------- */
  rtpbin = gst_element_factory_make("rtpbin", "rtpbin");
  if (!rtpbin) {
    g_print("unable to create an rtpbin.\n");
    return -1;
  }  

  g_signal_emit_by_name(G_OBJECT(rtpbin), "add-destination", "127.0.0.1:5024");
  g_object_set(G_OBJECT(rtpbin), "rtcp-support", TRUE, NULL);
  g_object_set(G_OBJECT(rtpbin), "localport", 5024, NULL);

  /* send chain */
  /* -------------------- */
  encoder = gst_element_factory_make("gsmenc", "gsmenc");
  payloader = gst_element_factory_make("rtpgsmenc", "rtpgsmenc");

  gst_bin_add_many(GST_BIN(pipeline), rtpbin, encoder, payloader, NULL);
  gst_element_link_pads(encoder, "src", payloader, "sink");
  gst_element_link_pads(payloader, "src", rtpbin, "sink%d");

  // our test src and link it to the encoder
  src = gst_element_factory_make("alsasrc", "alsasrc");
  g_object_set(G_OBJECT(src), "blocksize", 320, NULL);
  gst_bin_add_many(GST_BIN(pipeline), src, NULL);
  gst_element_link_pads(src, "src", encoder, "sink");

  /* rtpdemux */
  /* -------------------- */
  rtpdemux  = gst_element_factory_make ("rtpdemux", "rtpdemux");
  if (!rtpdemux) {
    g_print ("rtpdemux not found, check your install\n");
    return -1;
  }
  g_signal_connect (G_OBJECT (rtpdemux), "new-payload-type", G_CALLBACK (new_payload_type), state);
  g_signal_connect (G_OBJECT (rtpdemux), "payload-type-change", G_CALLBACK (payload_type_change), state);


  /* fakesink */
  sink = gst_element_factory_make("fakesink", "sink");

  /* link everything together */
  /* ------------------------ */
  gst_bin_add_many (GST_BIN (pipeline), rtpdemux, NULL);
  gst_element_link_pads(rtpbin, "src%d", rtpdemux, "sink");

  /* run */
  /* ------------------------ */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  g_main_loop_run (loop);

  /* clean up */
  /* ------------------------ */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));

  g_free(state);

  return 0;
}
