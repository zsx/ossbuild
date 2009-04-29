#include <glib.h>
#include <gst/gst.h>

int 
main (int   argc,
      char *argv[]) 
{
  GstElement *bin, *rtpbin, *encoder, *payloader;
  GMainLoop *loop;
  gst_init (&argc, &argv);

  loop = g_main_loop_new(NULL, FALSE);

  bin = gst_pipeline_new("pipe");

  fprintf(stderr,"bin: %p\n",bin);  

  if (argc != 3)
  {
      printf("Usage : gsmsend [rtcplocalport-1] [destaddr:port]\n");
      return 0;
  }
  
  int port = atoi (argv[1]);
  char *addr = argv[2];

  // first we create our rtpbin object
  rtpbin = gst_element_factory_make("rtpbin", "rtpbin");
  fprintf(stderr,"rtpbin: %p\n",rtpbin);  
  g_object_set(G_OBJECT(rtpbin), "destinations", addr, NULL);
  g_object_set(G_OBJECT(rtpbin), "rtcp-support", TRUE, NULL);
  g_object_set(G_OBJECT(rtpbin), "localport", port, NULL);
  
  // then our encoder/payloader
  encoder = gst_element_factory_make("alawenc", "alawenc");
  payloader = gst_element_factory_make("rtpg711pay", "rtpg711pay");

  // add both to the piepline and link them
  gst_bin_add_many(GST_BIN(bin), rtpbin, encoder, payloader, NULL);
  gst_element_link_pads(encoder, "src", payloader, "sink");
  gst_element_link_pads(payloader, "src", rtpbin, "sink%d");

  // our videotestsrc and link it to the encoder
  GstElement *testsrc = gst_element_factory_make("alsasrc", "alsasrc");
  g_object_set(G_OBJECT(testsrc), "blocksize", 320, NULL);
  gst_bin_add_many(GST_BIN(bin), testsrc, NULL);
  gst_element_link_pads(testsrc, "src", encoder, "sink");

  // let's start playing
  gst_element_set_state (bin, GST_STATE_PLAYING);

  //gst_debug_set_threshold_for_name ("rtpbin", 1);
  //gst_debug_set_threshold_for_name ("rtpsend", 5);
  //gst_debug_set_threshold_for_name ("*pad*", 5);

  g_main_loop_run(loop);

  return 0;
}
