#include <glib.h>
#include <gst/gst.h>

int 
main (int   argc,
      char *argv[]) 
{
  GstElement *bin, *rtpbin, *encoder;
  GMainLoop *loop;
  gst_init (&argc, &argv);

  loop = g_main_loop_new(NULL, FALSE);

  bin = gst_pipeline_new("pipe");
  
  if (argc != 3)
  {
      printf("Usage : h263send [rtcplocalport-1] [destaddr:port]\n");
      return 0;
  }
  
  int port = atoi (argv[1]);
  char *addr = argv[2];

  // first we create our rtpbin object
  rtpbin = gst_element_factory_make("rtpbin", "rtpbin");
  g_signal_emit_by_name(G_OBJECT(rtpbin), "add-destination", addr);
  g_object_set(G_OBJECT(rtpbin), "rtcp-support", FALSE, NULL);
  g_object_set(G_OBJECT(rtpbin), "localport", port, NULL);
  
  // then our encoder/payloader
  encoder = gst_element_factory_make("r263enc", "h263enc");
  g_object_set(G_OBJECT(encoder), "rtp-support", TRUE, NULL);

  // add both to the piepline and link them
  gst_bin_add_many(GST_BIN(bin), rtpbin, encoder, NULL);
  gst_element_link_pads(encoder, "src", rtpbin, "sink%d");

  // our videotestsrc and link it to the encoder
  GstElement *testsrc = gst_element_factory_make("videotestsrc", "videotestsrc");
  gst_bin_add_many(GST_BIN(bin), testsrc, NULL);
  GstCaps *videocaps = gst_caps_new_simple ("video/x-raw-yuv", 
          "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('I', '4', '2', '0'),
          "width", G_TYPE_INT, 352,
          "height", G_TYPE_INT, 288, 
          NULL);
  gst_element_link_pads_filtered(testsrc, "src", encoder, "sink", videocaps);

  // let's start playing
  gst_element_set_state (bin, GST_STATE_PLAYING);

  gst_debug_set_threshold_for_name ("rtpbin", 1);
  gst_debug_set_threshold_for_name ("rtpsend", 1);

  g_main_loop_run(loop);

  return 0;
}
