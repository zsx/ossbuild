#include <gst/gst.h>
#include <signal.h>
#include <string.h>
#include <strings.h>

#define GST_RTP_DTMF_TYPE_EVENT 1

static GstElement *pipeline, *dtmf_src, *network_sink;
static GMainLoop *main_loop;
static gboolean sending_dtmf;

static struct sigaction sig_action;

static void on_dtmf_event (gint dtmf_number, gint volume)
{
   GstEvent *event;
   GstStructure *structure;
   GstPad *pad;

   sending_dtmf = !sending_dtmf;

   structure = gst_structure_new ("dtmf-event",
                      "type", G_TYPE_INT, GST_RTP_DTMF_TYPE_EVENT,
                      "number", G_TYPE_INT, dtmf_number,
                      "volume", G_TYPE_INT, volume,
                      "start", G_TYPE_BOOLEAN, sending_dtmf, NULL);

   event = gst_event_new_custom (GST_EVENT_CUSTOM_UPSTREAM, structure);
   gst_element_send_event (pipeline, event);
}

static void
signal_handler (int sig_num)
{
  switch (sig_num) {
    case SIGINT:
      if (sending_dtmf) {
        on_dtmf_event (1, 20);
        g_main_loop_quit (main_loop);
        g_print ("stopped sending event '1'.\n");
      }

      else {
        on_dtmf_event (1, 25);
        g_print ("sending event '1' with volume '20'..\n");
      }  
      break;
    default:
      break;
  }
}

static gboolean
bus_call (GstBus * bus, GstMessage * msg, gpointer data)
{
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
      g_print ("End-of-stream\n");
      g_main_loop_quit (main_loop);
      break;
    case GST_MESSAGE_ERROR:{
      gchar *debug;
      GError *err;

      gst_message_parse_error (msg, &err, &debug);
      g_free (debug);

      g_print ("Error: %s\n", err->message);
      g_error_free (err);

      g_main_loop_quit (main_loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}

int
main (int argc, char *argv[])
{
  GstBus *bus;
  char *addr = NULL;
  int port = 0;
	
  /* initialize GStreamer */
  gst_init (&argc, &argv);

  if (argc >= 2)
  {
    if (strcmp ("--usage", argv[1]) == 0)
    {
      printf("Usage : dtmfsend destaddr port\n");
      return 0;
    }

    else {
        addr = argv[1];
        if (argc >= 3)
        {
          port = atoi (argv[2]);
        }
    }
  }
  

  /* create elements */
  pipeline = gst_pipeline_new ("dtmf-test-pipeline");
  dtmf_src = gst_element_factory_make ("rtpdtmfsrc", "dtmfsrc");
  network_sink = gst_element_factory_make ("udpsink", "networksink");
  
  if (!pipeline || !dtmf_src || !network_sink) {
    g_print ("One element could not be created\n");
    return -1;
  }

  /* videotestsrc should show a blank screen */
  //g_object_set (G_OBJECT (network_sink), "sync", FALSE, "dump", TRUE, NULL);
  g_object_set (G_OBJECT (network_sink), "sync", FALSE, NULL);
  if (addr)
    g_object_set (G_OBJECT (network_sink), "host", addr, NULL);
  if (port)
    g_object_set (G_OBJECT (network_sink), "port", port, NULL);
 
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, NULL);
  gst_object_unref (bus);
  
  /* put all elements in a bin */
  gst_bin_add_many (GST_BIN (pipeline), dtmf_src, network_sink, NULL);
  gst_element_link (dtmf_src, network_sink);

  /* Now set to playing and iterate. */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* set the unix signals */
  bzero(&sig_action, sizeof(sig_action));
  sig_action.sa_handler = signal_handler;
  sigaction(SIGINT, &sig_action, NULL);

  sending_dtmf = FALSE;

  g_print ("starting pipeline..\n");
  main_loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (main_loop);

  /* clean up nicely */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));
  g_main_loop_unref (main_loop);

  return 0;
}
