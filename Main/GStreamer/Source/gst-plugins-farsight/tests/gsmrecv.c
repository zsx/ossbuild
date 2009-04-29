#include <glib.h>
#include <gst/gst.h>

int 
main (int   argc,
      char *argv[]) 
{
  GstElement *bin, *rtpbin, *decoder, *depayloader;
  GMainLoop *loop;
  gst_init (&argc, &argv);

  loop = g_main_loop_new(NULL, FALSE);

  bin = gst_pipeline_new("pipe");

  if (!bin) {
      printf("unable to create a pipe.\n");
      exit(1);
  }  

  if (argc < 3)
  {
      printf("Usage : gsmrecv [rtplocalport] [destaddr:port]\n");
      return 0;
  }
  int port = atoi (argv[1]);
  char *addr = argv[2];

  // first we create our rtpbin object
  rtpbin = gst_element_factory_make("rtpbin", "rtpbin");
  
  if (!rtpbin) {
      printf("unable to create an rtpbin.\n");
      exit(1);
  }  

  g_object_set(G_OBJECT(rtpbin), "destinations", addr, NULL);
  g_object_set(G_OBJECT(rtpbin), "rtcp-support", TRUE, NULL);
  g_object_set(G_OBJECT(rtpbin), "localport", port, NULL);
  
  
  decoder = gst_element_factory_make("gsmdec", "gsmdec");
  depayloader = gst_element_factory_make("rtpgsmdepay", "rtpgsmdepay");
  if (!decoder) {
      printf("unable to create a gsmdec.\n");
      exit(1);
  }  
  if (!depayloader) {
      printf("unable to create a rtpgsmdepay.\n");
      exit(1);
  }  
  //g_object_set(G_OBJECT(depayloader), "process-only", TRUE, NULL);

  // add both to the piepline and link them
  gst_bin_add_many(GST_BIN(bin), rtpbin, decoder, depayloader, NULL);
  gst_element_link_pads(rtpbin, "src%d", depayloader, "sink");
  gst_element_link_pads(depayloader, "src", decoder, "sink");

  // our alsasink and link it to the encoder
  GstElement *testsink = gst_element_factory_make("alsasink", "alsasink");
  g_object_set(G_OBJECT(testsink), "sync", TRUE, NULL);
  gst_bin_add_many(GST_BIN(bin), testsink, NULL);
  gst_element_link_pads(decoder, "src", testsink , "sink");

  // let's start playing
  gst_element_set_state (bin, GST_STATE_PLAYING);

  //gst_debug_set_threshold_for_name ("rtpbin", 5);
  //gst_debug_set_threshold_for_name ("rtprecv", 5);
  //gst_debug_set_threshold_for_name ("rtpgsmdepay", 5);
  //gst_debug_set_threshold_for_name ("basertpdepayload", 5);
  

  g_main_loop_run(loop);

  return 0;
}
