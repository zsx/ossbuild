#include <glib.h>
#include <gst/gst.h>

int 
main (int   argc,
      char *argv[]) 
{
  GstElement *bin, *rtpbin, *decoder;
  GMainLoop *loop;
  gst_init (&argc, &argv);

  loop = g_main_loop_new(NULL, FALSE);

  bin = gst_pipeline_new("pipe");

  if (argc != 3)
  {
      printf("Usage : h263recv [rtplocalport] [destaddr:port]\n");
      return 0;
  }
  
  int port = atoi (argv[1]);
  char *addr = argv[2];

  // first we create our rtpbin object
  rtpbin = gst_element_factory_make("rtpbin", "rtpbin");
  g_signal_emit_by_name(G_OBJECT(rtpbin), "add-destination", addr);
  g_object_set(G_OBJECT(rtpbin), "rtcp-support", FALSE, NULL);
  g_object_set(G_OBJECT(rtpbin), "localport", port, NULL);
  
  
  // then our decoder / depayloader
  // TODO: fix ffdec_h263 
  // decoder leaks like hell (1MB/sec) for now, don't add it
  //decoder = gst_element_factory_make("ffdec_h263", "ffdec_h263");
  decoder = gst_element_factory_make("fakesink", "fakesink");
  //depayloader = gst_element_factory_make("r263depayloader", "r263depayloader");

  // add both to the piepline and link them
  gst_bin_add_many(GST_BIN(bin), rtpbin, decoder, NULL);
  //gst_element_link_pads(rtpbin, "src%d", depayloader, "sink");
  //gst_element_link_pads(depayloader, "src", decoder, "sink");
  gst_element_link_pads(rtpbin, "src%d", decoder, "sink");

  // our ximagesink and link it to the encoder
  /*
  GstElement *testsink = gst_element_factory_make("xvimagesink", "xvimagesink");
  GstElement *colorspace = gst_element_factory_make("ffmpegcolorspace", "ffmpegcolorspace");
  gst_bin_add_many(GST_BIN(bin), testsink, colorspace, NULL);
  gst_element_link_pads(decoder, "src", colorspace, "sink");
  gst_element_link_pads(colorspace, "src", testsink, "sink");
  */

  // let's start playing
  gst_element_set_state (bin, GST_STATE_PLAYING);

  gst_debug_set_threshold_for_name ("rtpbin", 1);
  gst_debug_set_threshold_for_name ("rtprecv", 5);

  g_main_loop_run(loop);

  return 0;
} 
