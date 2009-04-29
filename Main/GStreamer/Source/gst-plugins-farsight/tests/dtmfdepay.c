/*
 * test7.c - Farsight tests
 *
 * Farsight Voice+Video library test suite
 *  Copyright 2005,2006 Collabora Ltd.
 *   @author: Youness Alaoui <youness.alaoui@collabora.co.uk>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <glib.h>
#include <unistd.h>
#include <gst/gst.h>

GMainLoop *mainloop = NULL;

gboolean dtmf_io_cb (GIOChannel *source, GIOCondition condition, gpointer data) {
  GstElement * pipeline = (GstElement *) data;
  GstStructure *structure = NULL;
  GstEvent *event = NULL;
  gchar c;
  gsize size;
  gint ev = 0;
  static gboolean sending = FALSE;

  g_io_channel_read_chars (source, &c, 1, &size, NULL);


  if (c >= '0' && c <= '9') {
    ev = c -'0';
  } else if (c < ' ') {
    return TRUE;
  } else {
    switch (c) {
      case '*':
        ev = 10;
        break;
      case '#':
        ev = 11;
        break;
      default:
        ev = -1;
        if (sending)
          break;
        else
          return TRUE;
    }
  }


  if (sending) {
    structure = gst_structure_new ("dtmf-event",
        "start", G_TYPE_BOOLEAN, FALSE,
        "type", G_TYPE_INT, 1,
        "method", G_TYPE_INT, 1,
        NULL);
    sending = FALSE;
    g_debug ("Stopping DTMF event");
    event = gst_event_new_custom (GST_EVENT_CUSTOM_UPSTREAM, structure);

    gst_element_send_event (pipeline, event);
  }
  if (ev >= 0) {
    structure = gst_structure_new ("dtmf-event",
        "number", G_TYPE_INT, ev,
        "volume", G_TYPE_INT, 30,
        "start", G_TYPE_BOOLEAN, TRUE,
        "type", G_TYPE_INT, 1,
        "method", G_TYPE_INT, 1,
        NULL);
    sending = TRUE;
    g_debug ("Sending DTMF event %d", ev);
  }

  event = gst_event_new_custom (GST_EVENT_CUSTOM_UPSTREAM, structure);

  gst_element_send_event (pipeline, event);

  return TRUE;
}



static gboolean
bus_watch_cb (GstBus *bus, GstMessage *message,
    gpointer user_data)
{

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_EOS:
      g_debug ("%s (%d): end of stream on stream pipeline",
          __FUNCTION__, __LINE__);
      break;
    case GST_MESSAGE_ERROR:
      {
      gchar *debug;
      GError *err;

      gst_message_parse_error (message, &err, &debug);
      g_free (debug);

      g_warning ("%s (%d): error on stream pipeline. "
                 "Error code=%d message=%s",
                 __FUNCTION__, __LINE__, err->code, err->message);
      g_error_free (err);

      g_main_loop_quit (mainloop);

      break;
      }
    default:
      break;
  }

  return TRUE;
}


/* Read usage */
int main(int argc, char **argv)
{
    GIOChannel *ioc = g_io_channel_unix_new (0);
    GstElement* bin = NULL;
    GstBus *pipe_bus;
    GError *err = NULL;


    gst_init (&argc, &argv);

    mainloop = g_main_loop_new (NULL, FALSE);


    bin = gst_parse_launch ("rtpdtmfsrc ! rtpdtmfdepay ! audioconvert ! audiorate ! autoaudiosink sync=true", &err);
    if (bin) {
      g_io_add_watch (ioc, G_IO_IN, dtmf_io_cb, (gpointer) bin);
      pipe_bus = gst_pipeline_get_bus (GST_PIPELINE (bin));
      gst_bus_add_watch (pipe_bus, bus_watch_cb, NULL);

      gst_element_set_state (bin, GST_STATE_PLAYING);

      g_main_loop_run(mainloop);
    }

    return 0;
}
