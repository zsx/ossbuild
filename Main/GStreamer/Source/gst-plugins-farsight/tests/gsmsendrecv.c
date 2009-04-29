#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#include <glib.h>
#include <gst/gst.h>

#define DEST_MAXSIZE 128

int 
main (int   argc,
      char *argv[]) 
{
  GstElement *bin, *rtpbin, *encoder, *payloader, *decoder, *depayloader;
  GMainLoop *loop;
  gst_init (&argc, &argv);
  guint32 pt = 3;  /* PT=3 => GSM */
  char destination[DEST_MAXSIZE];
  int port, rtp_sockfd, rtcp_sockfd = -1;
  struct sockaddr_in rtp_addr;
  GstCaps *caps;
  GHashTable *pt_map = g_hash_table_new_full (g_direct_hash,
					      g_direct_equal,
					      NULL,
					      (GDestroyNotify) gst_caps_unref);

  /* initialize the PT<->caps hash table */
  caps = gst_caps_new_simple ("application/x-rtp",
      "clock-rate", G_TYPE_INT, 8000, NULL);
  g_hash_table_insert (pt_map, GINT_TO_POINTER(pt), (gpointer) caps);

  loop = g_main_loop_new(NULL, FALSE);

  if (argc != 3)
  {
      printf("Usage : gsmsendrecv <host> <port>\n");
      return 0;
  }

  /* set the local port */
  port = atoi (argv[2]);

  /* create and bind a socket for RTP and RTCP */
  memset(&rtp_addr, 0, sizeof(struct sockaddr_in));
  rtp_sockfd = socket (AF_INET, SOCK_DGRAM, 0);
  rtcp_sockfd = socket (AF_INET, SOCK_DGRAM, 0);
  if (rtp_sockfd > 0 && rtcp_sockfd > 0) {
    int res;
    rtp_addr.sin_family = AF_INET;
    rtp_addr.sin_port = htons (port);
    rtp_addr.sin_addr.s_addr = INADDR_ANY;

    res = bind(rtp_sockfd, (struct sockaddr*)&rtp_addr, sizeof(rtp_addr));
    if (res == 0) {
      fprintf(stderr, "Succesfully bound to local RTP port %d.\n", port);
    } else {
      fprintf(stderr, "Unable to bind to local RTP port %d.\n", port);
      rtp_sockfd = -1;
    }

    memset(&rtp_addr, 0, sizeof(struct sockaddr_in));
    rtp_addr.sin_family = AF_INET;
    rtp_addr.sin_port = htons (port + 1);
    rtp_addr.sin_addr.s_addr = INADDR_ANY;
    res = bind(rtcp_sockfd, (struct sockaddr*)&rtp_addr, sizeof(rtp_addr));
    if (res == 0) {
      fprintf(stderr, "Succesfully bound to local RTCP port %d.\n", port + 1);
    } else {
      fprintf(stderr, "Unable to bind to local RTCP port %d.\n", port + 1);
      rtcp_sockfd = -1;
    }
  }

  bin = gst_pipeline_new("pipe");
  fprintf(stderr,"bin: %p\n",bin);  
  
  // first we create our rtpbin object
  rtpbin = gst_element_factory_make("rtpbin", "rtpbin");
  fprintf(stderr,"rtpbin: %p\n",rtpbin);  
  snprintf(destination, DEST_MAXSIZE, "%s:%s", argv[1], argv[2]);
  g_object_set(G_OBJECT(rtpbin), "destinations", destination, NULL);
  g_object_set(G_OBJECT(rtpbin), "rtcp-support", TRUE, NULL);
  g_object_set(G_OBJECT(rtpbin), "localport", port, NULL);
  g_object_set(G_OBJECT(rtpbin), "pt-map", pt_map, NULL);
  if (rtp_sockfd != -1)
    g_object_set(G_OBJECT(rtpbin), "rtp_sockfd", rtp_sockfd, NULL);
  if (rtcp_sockfd != -1)
    g_object_set(G_OBJECT(rtpbin), "rtcp_sockfd", rtcp_sockfd, NULL);
  
  // then our encoder/payloader
  encoder = gst_element_factory_make("gsmenc", "gsmenc");
  payloader = gst_element_factory_make("rtpgsmpay", "rtpgsmpay");

  // add both to the piepline and link them
  gst_bin_add_many(GST_BIN(bin), rtpbin, encoder, payloader, NULL);
  gst_element_link_pads(encoder, "src", payloader, "sink");
  gst_element_link_pads(payloader, "src", rtpbin, "sink%d");

  // our test src and link it to the encoder
  GstElement *testsrc = gst_element_factory_make("alsasrc", "alsasrc");
  g_object_set(G_OBJECT(testsrc), "blocksize", 320, NULL);
  gst_bin_add_many(GST_BIN(bin), testsrc, NULL);
  gst_element_link_pads(testsrc, "src", encoder, "sink");

  // decoder and depayloader
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
  g_object_set(G_OBJECT(depayloader), "queue-delay", 0, NULL);

  // add receive elements to the pipeline and link them
  gst_bin_add_many(GST_BIN(bin), decoder, depayloader, NULL);
  gst_element_link_pads(rtpbin, "src%d", depayloader, "sink");
  gst_element_link_pads(depayloader, "src", decoder, "sink");

  // our alsasink and link it to the encoder
  GstElement *testsink = gst_element_factory_make("alsasink", "alsasink");
  g_object_set(G_OBJECT(testsink), "sync", TRUE, NULL);
  gst_bin_add_many(GST_BIN(bin), testsink, NULL);
  gst_element_link_pads(decoder, "src", testsink , "sink");

  // let's start playing
  gst_element_set_state (bin, GST_STATE_PLAYING);

  //gst_debug_set_threshold_for_name ("rtpbin", 1);
  //gst_debug_set_threshold_for_name ("rtpsend", 5);
  //gst_debug_set_threshold_for_name ("*pad*", 5);

  g_main_loop_run(loop);

  return 0;
}
