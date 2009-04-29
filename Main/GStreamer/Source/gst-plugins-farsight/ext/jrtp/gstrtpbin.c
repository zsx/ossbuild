/* GStreamer
 * Copyright (C) <2005> Philippe Khalaf <burger@speedy.org> 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Thanks to the following contributors :
 *
 * Martin Cizek
 *
 */

#include <stdlib.h>

#include <gst/gst.h>
#include <gst/netbuffer/gstnetbuffer.h>

#include <arpa/inet.h>

#include "gstrtpbin.h"

#include "default_pt.h"

static GstElementDetails gst_rtp_bin_details = {
  "RTP Bin",
  "Generic/Bin/RTP",
  "Encapsulates RTP session management and sending/receiving",
  "Philippe Khalaf <burger@speedy.org>"
};

/* generic templates */
static GstStaticPadTemplate rtp_bin_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink%d",
    GST_PAD_SINK,
    GST_PAD_REQUEST,
    GST_STATIC_CAPS("application/x-rtp, "
        "clock-rate = (int) [ 1, 2147483647 ]")
    );

static GstStaticPadTemplate rtp_bin_src_template =
GST_STATIC_PAD_TEMPLATE ("src%d",
    GST_PAD_SRC,
    GST_PAD_REQUEST,
    GST_STATIC_CAPS("application/x-rtp")
    );

GST_DEBUG_CATEGORY_STATIC (gst_rtp_bin_debug);
#define GST_CAT_DEFAULT gst_rtp_bin_debug

#define UDP_DEFAULT_CLOSEFD            TRUE

/* props */
enum
{
  ARG_0,
  ARG_RTCP_SUPPORT,
  ARG_LOCALPORT,
  ARG_DEFAULT_PT,
  ARG_DEFAULT_TS_INC,
  ARG_DEFAULT_MARK,
  ARG_DESTINATION,
  ARG_RTP_SOCKFD,
  ARG_RTCP_SOCKFD,
  ARG_PT_MAP,
  ARG_PT_CAPS,
  ARG_BYPASS_UDP,
  ARG_QUEUE_DELAY,
  ARG_CLOSE_FD
};


static void gst_rtp_bin_class_init (GstRTPBinClass * klass);
static void gst_rtp_bin_init (GstRTPBin * rtp_bin);
static void gst_rtp_bin_dispose (GObject * object);

static GHashTable *gst_rtp_bin_build_pt_map (const gchar *caps_string);

static GstPad * gst_rtp_bin_request_new_pad (GstElement * element,
    GstPadTemplate *templ, const gchar * name);

static void gst_rtp_bin_release_elements (GstRTPBin * rtp_bin);
static gboolean gst_rtp_bin_setup_elements (GstRTPBin *rtp_bin);

static GstElement * gst_rtp_bin_create_rtpsend (GstRTPBin * rtp_bin);
static GstElement * gst_rtp_bin_create_rtprecv (GstRTPBin * rtp_bin);
static GstElement * gst_rtp_bin_create_jitterbuffer (GstRTPBin * rtp_bin);
static GstPad *gst_rtp_bin_setup_rtpsend_pads(GstRTPBin * rtp_bin);
static GstPad *gst_rtp_bin_setup_rtprecv_pads(GstRTPBin * rtp_bin);

static gboolean gst_rtp_bin_setup_rtcp_elements (GstRTPBin * rtp_bin);
static gboolean gst_rtp_bin_setup_send_elements (GstRTPBin *rtp_bin);
static gboolean gst_rtp_bin_setup_recv_elements (GstRTPBin *rtp_bin);

//static GstClock *gst_rtp_bin_provide_clock (GstElement * element);

static GstStateChangeReturn
gst_rtp_bin_change_state (GstElement *element, GstStateChange transition);

static void gst_rtp_bin_set_destinations (GstRTPBin *rtp_bin,
    const gchar *destination);

static void gst_rtp_bin_set_property (GObject * object, guint prop_id, 
        const GValue * value, GParamSpec * spec);

static void gst_rtp_bin_get_property (GObject * object, guint prop_id, 
        GValue * value, GParamSpec * spec);

static GstElementClass *parent_class = NULL;

GType
gst_rtp_bin_get_type (void)
{
    static GType gst_rtp_bin_type = 0;

    if (!gst_rtp_bin_type) {
        static const GTypeInfo gst_rtp_bin_info = {
            sizeof (GstRTPBinClass),
            NULL,
            NULL,
            (GClassInitFunc) gst_rtp_bin_class_init,
            NULL,
            NULL,
            sizeof (GstRTPBin),
            0,
            (GInstanceInitFunc) gst_rtp_bin_init,
            NULL
        };

        gst_rtp_bin_type =
            g_type_register_static (GST_TYPE_BIN, "GstRTPBin",
                    &gst_rtp_bin_info, 0);
    }

    return gst_rtp_bin_type;
}

static void
gst_rtp_bin_class_init (GstRTPBinClass * klass)
{
    GObjectClass *gobject_klass;
    GstElementClass *gstelement_klass;
    GstBinClass *gstbin_klass;

    gobject_klass = (GObjectClass *) klass;
    gstelement_klass = (GstElementClass *) klass;
    gstbin_klass = (GstBinClass *) klass;

    parent_class = g_type_class_ref (GST_TYPE_BIN);

    gobject_klass->set_property = gst_rtp_bin_set_property;
    gobject_klass->get_property = gst_rtp_bin_get_property;

    gobject_klass->dispose = GST_DEBUG_FUNCPTR (gst_rtp_bin_dispose);

#ifdef HAVE_JRTP
    g_object_class_install_property (gobject_klass, ARG_RTCP_SUPPORT,
            g_param_spec_boolean ("rtcp_support", "RTCP Support",
                "Set to false to disable RTCP support",
                TRUE, G_PARAM_READWRITE));
#else
    g_object_class_install_property (gobject_klass, ARG_RTCP_SUPPORT,
            g_param_spec_boolean ("rtcp_support", "RTCP Support",
                "Set to false to disable RTCP support",
                FALSE, G_PARAM_READWRITE));
#endif

    g_object_class_install_property (gobject_klass, ARG_LOCALPORT,
            g_param_spec_uint ("localport", "Local Port", 
                "An even upd port for the rtp socket, rtcp is bound to +1",
                0, G_MAXUINT16, 0, G_PARAM_READWRITE));

    g_object_class_install_property (gobject_klass, ARG_DEFAULT_PT,
            g_param_spec_uint ("default_pt", "Default Payload Type", 
                "The default payload Type",
                0, G_MAXUINT8, 0, G_PARAM_READWRITE));

    g_object_class_install_property (gobject_klass, ARG_DEFAULT_TS_INC,
            g_param_spec_uint ("default_ts_inc", "Default Timestamp Increment", 
                "The default timestamp increment",
                0, G_MAXUINT32, 0, G_PARAM_READWRITE));

   g_object_class_install_property (gobject_klass, ARG_DESTINATION,
            g_param_spec_string ("destinations", "Destination addresses", 
                "The destination address to send to, seperated by ';'",
                NULL, G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_klass, ARG_DEFAULT_MARK,
            g_param_spec_boolean ("default_mark", "Default Mark", "The default mark",
                FALSE, G_PARAM_READWRITE));


    g_object_class_install_property (gobject_klass, ARG_RTP_SOCKFD,
            g_param_spec_int ("rtp_sockfd", "socket for RTP", 
                "Socket to use for RTP.",
                0, G_MAXINT16, 0, G_PARAM_READWRITE));

    g_object_class_install_property (gobject_klass, ARG_RTCP_SOCKFD,
            g_param_spec_int ("rtcp_sockfd", "socket for RTCP", 
                "Socket to use for RTCP.",
                0, G_MAXINT16, 0, G_PARAM_READWRITE));

    g_object_class_install_property (gobject_klass, ARG_PT_MAP,
            g_param_spec_pointer ("pt-map", "Payload-Type-map",
                "A Hash table, mapping payload-types to GstCaps object",
                G_PARAM_READWRITE));

    g_object_class_install_property (gobject_klass, ARG_PT_CAPS,
            g_param_spec_string ("pt-caps", "Caps for payload types", 
                "A string representing the caps for the supported payload types",
                NULL, G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_klass, ARG_BYPASS_UDP,
            g_param_spec_boolean ("bypass-udp", "Bypass udp elements",
                "When set to true, the udp sink/src elements are bypassed",
                FALSE, G_PARAM_READWRITE));

    g_object_class_install_property (gobject_klass, ARG_QUEUE_DELAY,
	    g_param_spec_uint ("queue-delay", "Queue Delay",
			       "Amount of ms to queue/buffer, or zero to disable", 0, G_MAXUINT, 0, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_klass, ARG_CLOSE_FD,
      g_param_spec_boolean ("closefd", "Close sockfd",
          "Close sockfd if passed as property on state change",
          UDP_DEFAULT_CLOSEFD, G_PARAM_READWRITE));

    gst_element_class_add_pad_template (gstelement_klass,
            gst_static_pad_template_get (&rtp_bin_sink_template));
    gst_element_class_add_pad_template (gstelement_klass,
            gst_static_pad_template_get (&rtp_bin_src_template));

    gst_element_class_set_details (gstelement_klass, &gst_rtp_bin_details);

    gstelement_klass->request_new_pad = gst_rtp_bin_request_new_pad;
    gstelement_klass->change_state = gst_rtp_bin_change_state;
    //gstelement_klass->provide_clock = gst_rtp_bin_provide_clock;

    GST_DEBUG_CATEGORY_INIT (gst_rtp_bin_debug, "rtpbin", 0, "RTP Bin");
    
    // this is not MT safe so it's exec only once here
    if (!GST_TYPE_NETBUFFER) { g_error ("eek! the impossible happened!"); }
    
}

static void
gst_rtp_bin_init (GstRTPBin * rtp_bin)
{
    rtp_bin->localport = 0; 
    rtp_bin->rtp_sockfd = -1;
    rtp_bin->rtcp_sockfd = -1;

    rtp_bin->rtpsend = NULL;
    rtp_bin->rtprecv = NULL;
    rtp_bin->jbuf = NULL;
    rtp_bin->rtpsrc = NULL;
    rtp_bin->rtpsink = NULL;
    rtp_bin->rtcpsrc = NULL;
    rtp_bin->rtcpsink = NULL;

    rtp_bin->clockrate = 100;

#ifdef HAVE_JRTP
    rtp_bin->rtcp_support = TRUE;
    // let's create our session
    rtp_bin->sess = jrtpsession_init();
    jrtpsession_create(rtp_bin->sess, rtp_bin->clockrate);
#else
    rtp_bin->rtcp_support = FALSE;
    rtp_bin->dest_ip = 0;
    rtp_bin->dest_port = 0;
#endif

    rtp_bin->bypass_udp = FALSE;

    rtp_bin->default_pt_map = gst_rtp_bin_build_pt_map (default_pt_table);

    rtp_bin->pt_map = NULL;

    rtp_bin->closefd = UDP_DEFAULT_CLOSEFD;
}

static void
gst_rtp_bin_dispose (GObject * object)
{
    GstRTPBin *rtp_bin;

    rtp_bin = GST_RTP_BIN (object);

    gst_rtp_bin_release_elements (rtp_bin);

    if (G_OBJECT_CLASS (parent_class)->dispose) {
        G_OBJECT_CLASS (parent_class)->dispose (object);
    }

    if (rtp_bin->default_pt_map)
    {
      g_hash_table_destroy (rtp_bin->default_pt_map);
    }
    if (rtp_bin->pt_map)
    {
      g_hash_table_destroy (rtp_bin->pt_map);
    }
}
/*
static GstClock *
gst_rtp_bin_provide_clock (GstElement * element)
{
    GstRTPBin *rtp_bin = GST_RTP_BIN (element);
    
    if (rtp_bin->jbuf != NULL)
	return gst_element_provide_clock (rtp_bin->jbuf);
    
    else
	return NULL;
}*/

static GHashTable *gst_rtp_bin_build_pt_map (const gchar *caps_string)
{
  GHashTable *pt_map = NULL;
  gint i;
  gint pt;
  GstCaps *caps = NULL;
  GstCaps *new_caps = NULL;
  GstStructure *structure = NULL;
  GstStaticCaps static_caps = GST_STATIC_CAPS (caps_string);

  caps = gst_static_caps_get (&static_caps);

  pt_map = g_hash_table_new_full (g_direct_hash,
      g_direct_equal,
      NULL,
      (GDestroyNotify) gst_caps_unref);
  for (i = 0; i < gst_caps_get_size (caps); i++)
  {
    structure = gst_caps_get_structure (caps, i);
    GST_DEBUG ("Adding struct %p %s", structure,
        gst_structure_to_string (structure));

    new_caps = gst_caps_new_full (gst_structure_copy (structure), NULL);
    GST_DEBUG ("Adding caps %p %s", new_caps, gst_caps_to_string (new_caps));
    if (gst_structure_get_int (structure, "payload", &pt))
    {
      g_hash_table_insert(pt_map, GINT_TO_POINTER(pt), (gpointer)
          new_caps);
    }
    else
    {
      GST_DEBUG ("Error building default pt-map hashtable");
    }
  }

  gst_caps_unref (caps);

  return pt_map;
}


static GstPad *
gst_rtp_bin_request_new_pad (GstElement * element, GstPadTemplate * templ,
    const gchar *name)
{
    GstRTPBin *rtp_bin;
    GstPad *newpad = NULL;

    g_return_val_if_fail (GST_IS_RTP_BIN (element), NULL);

    GST_DEBUG ("Pad %p %s requested!", templ, name);

    rtp_bin = GST_RTP_BIN (element);

    switch (templ->direction) {
        case GST_PAD_SINK:         /* Create rtpsend and its ghost pad. */
            if (rtp_bin->rtpsend != NULL)
                goto already_exist;

            if ((rtp_bin->rtpsend = gst_rtp_bin_create_rtpsend (rtp_bin))
                    == NULL) {
                goto error;
            }

            if (!gst_bin_add (GST_BIN (rtp_bin), rtp_bin->rtpsend)) {
                goto error;
            }

            gst_element_sync_state_with_parent (rtp_bin->rtpsend);

            newpad = gst_rtp_bin_setup_rtpsend_pads(rtp_bin);
            if (!newpad)
            {
                gst_bin_remove (GST_BIN (rtp_bin), rtp_bin->rtpsend);
                goto error;
            }

            /* Setup send elements if this pad was added after changing
               to the READY state. */
            if (GST_STATE(rtp_bin) >= GST_STATE_READY)
            {
                if (!rtp_bin->bypass_udp)
                {
                    if (!gst_rtp_bin_setup_send_elements(rtp_bin)) {
                        goto error;
                    }
                    /* Set children to the same state as parent so they
                       start running if rtpbin is already in PLAYING state. */
                    gst_element_sync_state_with_parent(rtp_bin->rtpsink);
                }
            }

            break;

        case GST_PAD_SRC:          /* Create rtprecv and its ghost pad. */
            if (rtp_bin->jbuf != NULL)
                goto already_exist;

            if ((rtp_bin->rtprecv = gst_rtp_bin_create_rtprecv (rtp_bin))
                    == NULL) {
                goto error;
            }

            if ((rtp_bin->jbuf = gst_rtp_bin_create_jitterbuffer (rtp_bin)) 
                    == NULL) {
                goto error;
            }
            g_object_set (rtp_bin->jbuf, "latency", rtp_bin->queue_delay,
                    NULL);
            g_object_set (rtp_bin->jbuf, "drop-on-latency", TRUE,
                    NULL);

            gst_bin_add_many (GST_BIN (rtp_bin), rtp_bin->rtprecv,
                    rtp_bin->jbuf, NULL);

            gst_element_sync_state_with_parent (rtp_bin->rtprecv);
            gst_element_sync_state_with_parent (rtp_bin->jbuf);


            if (!gst_element_link_pads (rtp_bin->rtprecv, "datasrc",
                        rtp_bin->jbuf, "sink")) {
                GST_ERROR ("Could not link rtprecv:datasrc to jitterbuffer:sink");
                goto error;
            }

            newpad = gst_rtp_bin_setup_rtprecv_pads(rtp_bin);
            if (!newpad)
            {
                gst_bin_remove (GST_BIN (rtp_bin), rtp_bin->rtprecv);
                gst_bin_remove (GST_BIN (rtp_bin), rtp_bin->jbuf);
                goto error;
            }

            /* Setup recv elements if this pad was added after changing
               to the READY state. */
            if (GST_STATE(rtp_bin) >= GST_STATE_READY)
            {
                if (!rtp_bin->bypass_udp)
                {
                    if (!gst_rtp_bin_setup_recv_elements(rtp_bin)) {
                        goto error;
                    }
                    /* Set children to the same state as parent so they
                       start running if rtpbin is already in PLAYING state. */
                    gst_element_sync_state_with_parent(rtp_bin->rtpsrc);
                    gst_element_sync_state_with_parent(rtp_bin->jbuf);
                }
            }

            break;

        default:
            goto bad_template;
    }

    return newpad;

error:
{
    GST_WARNING("some elements could not be created for rtpbin");
    if (rtp_bin->rtpsend)
        gst_object_unref (rtp_bin->rtpsend);
    if (rtp_bin->rtprecv)
        gst_object_unref (rtp_bin->rtprecv);
    if (rtp_bin->jbuf)
        gst_object_unref (rtp_bin->jbuf);
    rtp_bin->rtprecv = NULL;
    rtp_bin->jbuf = NULL;

    return NULL;
}

    /* errors that require special handling */
already_exist:
    {
        GST_WARNING("rtpbin %s can have only one src pad and only one sink pad",
                    GST_OBJECT_NAME(element));
        return NULL;
    }
bad_template:
    {
        GST_WARNING("rtpbin cannot create pad from template %s",
                GST_OBJECT_NAME(templ));
        return NULL;
    }
}

static void
gst_rtp_bin_release_elements (GstRTPBin * rtp_bin)
{
    if (rtp_bin->rtpsend != NULL) {
        gst_bin_remove (GST_BIN(rtp_bin), rtp_bin->rtpsend);
        rtp_bin->rtpsend = NULL;
    }
    if (rtp_bin->rtprecv != NULL) {
        gst_bin_remove (GST_BIN(rtp_bin), rtp_bin->rtprecv);
        rtp_bin->rtprecv = NULL;
    }
    if (rtp_bin->rtpsink != NULL) {
        gst_bin_remove (GST_BIN(rtp_bin), rtp_bin->rtpsink);
        rtp_bin->rtpsink = NULL;
    }
    if (rtp_bin->rtcpsink != NULL) {
        gst_bin_remove (GST_BIN(rtp_bin), rtp_bin->rtcpsink);
        rtp_bin->rtcpsink = NULL;
    }
    if (rtp_bin->rtpsrc != NULL) {
        gst_bin_remove (GST_BIN(rtp_bin), rtp_bin->rtpsrc);
        rtp_bin->rtpsrc = NULL;
    }
    if (rtp_bin->rtcpsrc != NULL) {
        gst_bin_remove (GST_BIN(rtp_bin), rtp_bin->rtcpsrc);
        rtp_bin->rtcpsrc = NULL;
    }
}

/* Setup rtpbin elements and RTPSession object. */
static gboolean
gst_rtp_bin_setup_elements (GstRTPBin * rtp_bin)
{
    GstPad * rtp_src_pad = NULL;
    GstPad * rtcp_src_pad = NULL;

    GST_DEBUG ("Setting up rtp bin");

    if (rtp_bin->rtpsend != NULL) {
        /* if bypassing udp elements let's not create them */
        if (!rtp_bin->bypass_udp)
        {
            GST_DEBUG ("Running gst_rtp_bin_setup_send_elements");
            if (!gst_rtp_bin_setup_send_elements (rtp_bin))
            {
                return FALSE;
            }
        }

        rtp_src_pad = gst_element_get_pad (rtp_bin->rtpsend, "rtpsrc");
    }

    if (rtp_bin->rtprecv != NULL) {
        /* if bypassing udp elements let's not create them */
        if (!rtp_bin->bypass_udp)
        {
            GST_DEBUG ("Running gst_rtp_bin_setup_recv_elements");
            if (!gst_rtp_bin_setup_recv_elements (rtp_bin))
            {
              goto error;
            }
        }
    }

    /* rtpsend and rtprecv elements have to be set up before rtcp element since
       rtcp may add rtprecv/rtpsend element. */
    if (rtp_bin->rtcp_support) {
        GST_DEBUG ("Creating rtcp elements");
        /* We need both rtpsend and rtprecv elements for RTCP. */
        if (rtp_bin->rtpsend == NULL) {
            rtp_bin->rtpsend = gst_rtp_bin_create_rtpsend (rtp_bin);
            gst_bin_add (GST_BIN(rtp_bin), rtp_bin->rtpsend);
            gst_element_sync_state_with_parent (rtp_bin->rtpsend);
        }
        if (rtp_bin->rtprecv == NULL) {
            rtp_bin->rtprecv = gst_rtp_bin_create_rtprecv (rtp_bin);
            gst_bin_add (GST_BIN(rtp_bin), rtp_bin->rtprecv);
            gst_element_sync_state_with_parent (rtp_bin->rtprecv);
        }

        /* if bypassing udp elements let's not create them */
        if (!rtp_bin->bypass_udp)
        {
            GST_DEBUG ("Running gst_rtp_bin_setup_rtcp_elements");
            if (!gst_rtp_bin_setup_rtcp_elements (rtp_bin))
            {
              goto error;
            }
            rtcp_src_pad = gst_element_get_pad (rtp_bin->rtpsend, "rtcpsrc");
        }
#if 0
        else
        {
            /* Let's just ghost pad rtcpsink on rtpbin */
            GstPad *newpad1 = NULL;
            GstPad *newpad2 = NULL;
            GstPad *tmppad = NULL;

            tmppad = gst_element_get_static_pad (rtp_bin->rtprecv, "rtcpsink");
            newpad1 = gst_ghost_pad_new ("rtcpsink", tmppad );
            gst_object_unref (tmppad);

            if (!gst_element_add_pad (GST_ELEMENT (rtp_bin), newpad1)) {
                gst_object_unref (newpad1);
                GST_DEBUG ("Could not setup rtcpsink ghost pad");
                return FALSE;
            }
            tmppad = gst_element_get_static_pad (rtp_bin->rtpsend, "rtcpsrc");
            newpad2 = gst_ghost_pad_new ("rtcpsrc", tmppad);
            gst_object_unref (tmppad);

            if (!gst_element_add_pad (GST_ELEMENT (rtp_bin), newpad2)) {
                gst_object_unref (newpad2);
                GST_DEBUG ("Could not setup rtcpsrc ghost pad");
                return FALSE;
            }
        }
#endif
    }

#ifdef HAVE_JRTP
    jrtpsession_setpads (rtp_bin->sess, rtp_src_pad, rtcp_src_pad);
#else
    if (rtp_src_pad)
      gst_object_unref(rtp_src_pad);
    if (rtcp_src_pad)
      gst_object_unref(rtcp_src_pad);
#endif

    GST_DEBUG("Elements setup properly");
    return TRUE;

 error:

    if (rtp_src_pad)
      gst_object_unref(rtp_src_pad);
    if (rtcp_src_pad)
      gst_object_unref(rtcp_src_pad);

    return FALSE;
}

static gboolean
gst_rtp_bin_setup_rtcp_elements (GstRTPBin * rtp_bin)
{
    /* We always send and receive RTCP packets. */
    rtp_bin->rtcpsink =
      gst_element_factory_make ("dynudpsink", "rtcpsinkelement");
    rtp_bin->rtcpsrc =
        gst_element_factory_make ("udpsrc", "rtcpsrcelement");
    if (rtp_bin->rtcpsink == NULL)
    {
        GST_ERROR("Could not add dynudpsink!");
        return FALSE;
    }
    if (rtp_bin->rtcpsrc == NULL)
    {
        GST_ERROR("Could not add udpsrc!");
        return FALSE;
    }

    /* If application provided a socket, use it */
    if (rtp_bin->rtcp_sockfd != -1) {
      g_object_set (G_OBJECT(rtp_bin->rtcpsrc), "sockfd",
          rtp_bin->rtcp_sockfd, NULL);
      g_object_set (G_OBJECT(rtp_bin->rtcpsrc), "closefd",
          rtp_bin->closefd, NULL);

      g_object_set (G_OBJECT(rtp_bin->rtcpsink), "sockfd",
          rtp_bin->rtcp_sockfd, NULL);
      g_object_set (G_OBJECT(rtp_bin->rtcpsink), "closefd",
          rtp_bin->closefd, NULL);
    }
    g_object_set (G_OBJECT(rtp_bin->rtcpsink), "sync", FALSE, NULL);

    gst_bin_add_many (GST_BIN(rtp_bin), rtp_bin->rtcpsink,
        rtp_bin->rtcpsrc, NULL);
    /* Set the local RTCP port */
    g_object_set (G_OBJECT(rtp_bin->rtcpsrc), "port", rtp_bin->localport+1,
        NULL);

        /* Link RTCP pads with pads of UDP elements.  */
    if (!gst_element_link_pads (rtp_bin->rtpsend, "rtcpsrc",
        rtp_bin->rtcpsink, "sink"))
    {
      GST_ERROR ("Could not link rtpsend:rtcpsrc to rtcpsink:sink");
      return FALSE;
    }
    if (!gst_element_link_pads (rtp_bin->rtcpsrc, "src",
        rtp_bin->rtprecv, "rtcpsink"))
    {
      GST_ERROR ("Could not link rtcpsrc:src to rtprecv:rtcpsink");
      return FALSE;
    }

    return TRUE;
}

/* Add UDP sink for rtpsend and link it. */
static gboolean
gst_rtp_bin_setup_send_elements (GstRTPBin * rtp_bin)
{
    rtp_bin->rtpsink
        = gst_element_factory_make ("dynudpsink", "rtpsinkelement");

    if (rtp_bin->rtpsink == NULL)
    {
        GST_ERROR("Could not add dynudpsink!");
        return FALSE;
    }

    /* If application provided a socket, use it */
    if (rtp_bin->rtp_sockfd != -1)
    {
      g_object_set (G_OBJECT(rtp_bin->rtpsink), "sockfd", rtp_bin->rtp_sockfd,
          NULL);
      g_object_set (G_OBJECT(rtp_bin->rtpsink), "closefd", rtp_bin->closefd,
          NULL);
    }

    g_object_set (G_OBJECT(rtp_bin->rtpsink), "sync", FALSE, NULL);

    gst_bin_add (GST_BIN (rtp_bin), rtp_bin->rtpsink);

    if (!gst_element_link_pads (rtp_bin->rtpsend, "rtpsrc", rtp_bin->rtpsink,
        "sink"))
    {
      GST_ERROR ("Could not link rtpsend:rtpsrc to rtpsink:sink");
      return FALSE;
    }

    return TRUE;
}

/* Add UDP src for rtprecv and link it. */
static gboolean 
gst_rtp_bin_setup_recv_elements (GstRTPBin * rtp_bin)
{
    rtp_bin->rtpsrc = 
        gst_element_factory_make ("udpsrc", "rtpsrcelement");

    if (rtp_bin->rtpsrc == NULL)
    {
        GST_ERROR("Could not add udpsrc!");
        return FALSE;
    }

    /* If application provided a socket, use it */
    if (rtp_bin->rtp_sockfd != -1)
    {
      g_object_set (G_OBJECT(rtp_bin->rtpsrc), "sockfd", rtp_bin->rtp_sockfd,
          NULL);
      g_object_set (G_OBJECT(rtp_bin->rtpsrc), "closefd", rtp_bin->closefd,
          NULL);
    }

    /* Set our local port. */
    g_object_set (G_OBJECT(rtp_bin->rtpsrc), "port", rtp_bin->localport, NULL);

    gst_bin_add (GST_BIN (rtp_bin), rtp_bin->rtpsrc);
    if (!gst_element_link_pads (rtp_bin->rtpsrc, "src", rtp_bin->rtprecv,
          "rtpsink"))
    {
      GST_ERROR ("Could not link rtpsrc:src to rtprecv:rtpsink");
      return FALSE;
    }

    return TRUE;
}


/* Create rtpsend and set it's attributes - no links. */
static GstElement *
gst_rtp_bin_create_rtpsend (GstRTPBin * rtp_bin)
{
    GstElement * rtpsend = NULL;

    rtpsend = gst_element_factory_make ("rtpsend", NULL);
    if (rtpsend == NULL) {
        GST_WARNING("Could not create rtpsend element");
        return NULL;
    }
#ifdef HAVE_JRTP
    /* Give rtpsend the RTPSession pointer. */
    g_object_set (G_OBJECT(rtpsend), "rtpsession_ptr", rtp_bin->sess, NULL);
#endif

    return rtpsend;
}

/* Create rtprecv and set it's attributes - no links. */
static GstElement *
gst_rtp_bin_create_rtprecv (GstRTPBin * rtp_bin)
{
    GstElement * rtprecv = NULL;

    rtprecv = gst_element_factory_make ("rtprecv", NULL);
    if (rtprecv == NULL) {
        GST_WARNING("Could not create rtprecv element");
        return NULL;
    }
#ifdef HAVE_JRTP
    /* Give rtprecv the RTPSession pointer. */
    g_object_set (G_OBJECT(rtprecv), "rtpsession_ptr", rtp_bin->sess, NULL);
#endif
    if (rtp_bin->pt_map)
    {
      g_object_set (G_OBJECT(rtprecv), "pt_map", rtp_bin->pt_map, NULL);
    }
    else if (rtp_bin->user_pt_map)
    {
      g_object_set (G_OBJECT(rtprecv), "pt_map", rtp_bin->user_pt_map, NULL);
    }
    else
    {
      GST_DEBUG_OBJECT (rtp_bin, "Using default pt-map table");
      g_object_set (G_OBJECT(rtprecv), "pt_map", rtp_bin->default_pt_map,
          NULL);
    }

    return rtprecv;
}

static GstElement *
gst_rtp_bin_create_jitterbuffer (GstRTPBin * rtp_bin)
{
    GstElement * rtpjitterbuffer = NULL;
    const gchar * jb_name;

    jb_name = g_getenv ("RTPBIN_JB");
    if (jb_name == NULL)
      jb_name = "rtpjitterbuffer";

    rtpjitterbuffer = gst_element_factory_make (jb_name,
            "jitterbuffer");
    if (rtpjitterbuffer == NULL) {
      GST_WARNING("Could not create %s element", jb_name);
      return NULL;
    }

    return rtpjitterbuffer;
}

static GstPad *
gst_rtp_bin_setup_rtpsend_pads(GstRTPBin * rtp_bin)
{
    GstPad *newpad = NULL;
    GstPad *internal_pad = NULL;

    internal_pad = gst_element_get_pad (rtp_bin->rtpsend, "datasink");
    newpad = gst_ghost_pad_new ("sink", internal_pad);

    /* This could break with gstreamer 0.10.9 */
    gst_pad_set_active (newpad, TRUE);

    gst_object_unref (internal_pad);

    if (!gst_element_add_pad (GST_ELEMENT (rtp_bin), newpad)) {
        gst_object_unref (newpad);
        newpad = NULL;
    }

    if (rtp_bin->bypass_udp)
    {
        /* Let's just ghost pad rtpsrc on rtpbin */
        GstPad *bypass_pad = NULL;
        GstPad *tmppad = NULL;

        GST_DEBUG ("Using bypass_udp to create ghostpads for rtpsend");
        tmppad = gst_element_get_static_pad (rtp_bin->rtpsend, "rtpsrc");
        bypass_pad = gst_ghost_pad_new ("rtpsrc", tmppad);
        gst_object_unref (tmppad);

        /* This could break with gstreamer 0.10.9 */
        gst_pad_set_active (bypass_pad, TRUE);

        if (!gst_element_add_pad (GST_ELEMENT (rtp_bin), bypass_pad)) {
            gst_object_unref (bypass_pad);
            GST_DEBUG ("Could not setup rtpsrc ghost pad");
            newpad = NULL;
        }
    }
    return newpad;
}

static GstPad *
gst_rtp_bin_setup_rtprecv_pads(GstRTPBin * rtp_bin)
{
    GstPad *newpad = NULL;
    GstPad *tmppad = NULL;

    tmppad = gst_element_get_static_pad (rtp_bin->jbuf, "src");
    newpad = gst_ghost_pad_new ("src",tmppad);
    gst_object_unref (tmppad);

    /* This could break with gstreamer 0.10.9 */
    gst_pad_set_active (newpad, TRUE);

    if (!gst_element_add_pad (GST_ELEMENT (rtp_bin), newpad)) {
        gst_object_unref (newpad);
        newpad = NULL;
    }

    if (rtp_bin->bypass_udp)
    {
        /* Let's just ghost pad rtpsink on rtpbin */
        GstPad *newpad = NULL;
        GstPad *tmppad = NULL;

        GST_DEBUG ("Using bypass_udp to create ghostpads on rtprecv");
        tmppad = gst_element_get_static_pad (rtp_bin->rtprecv, "rtpsink");
        newpad = gst_ghost_pad_new ("rtpsink",tmppad);
        gst_object_unref (tmppad);

        /* This could break with gstreamer 0.10.9 */
        gst_pad_set_active (newpad, TRUE);

        if (!gst_element_add_pad (GST_ELEMENT (rtp_bin), newpad)) {
            gst_object_unref (newpad);
            GST_DEBUG ("Could not setup rtpsink ghost pad");
            newpad = NULL;
        }
    }

    return newpad;
}

static GstStateChangeReturn
gst_rtp_bin_change_state (GstElement *element, GstStateChange transition)
{
    GstRTPBin *rtp_bin;
    GstStateChangeReturn ret;

    rtp_bin = GST_RTP_BIN (element);
    switch (transition) {
        case GST_STATE_CHANGE_NULL_TO_READY:
            if ((rtp_bin->rtpsend == NULL && rtp_bin->rtprecv == NULL)
                || (rtp_bin->localport == 0 && !rtp_bin->bypass_udp))
            {
                GST_WARNING("%s cannot enter READY state because %s",
                            GST_OBJECT_NAME (rtp_bin),
                            rtp_bin->localport == 0
                            ? "local port was not set"
                            : "no pad was requested");
                goto start_failed;
            }
            if (!gst_rtp_bin_setup_elements (rtp_bin))
            {
                GST_ERROR("Could not enter READY state because the elements "
                    "could not be setup");
                goto start_failed;
            }
            break;
        default:
            break;
    }

    ret =   GST_CALL_PARENT_WITH_DEFAULT (GST_ELEMENT_CLASS, change_state,
            (element, transition), GST_STATE_CHANGE_SUCCESS);
    
    switch (transition) {
        case GST_STATE_CHANGE_READY_TO_NULL:
            gst_rtp_bin_release_elements (rtp_bin);
            break;
        default:
            break;
    }
    
    return ret;

  /* ERRORS */
start_failed:
  {
    return GST_STATE_CHANGE_FAILURE;
  }
}

static void 
gst_rtp_bin_set_destinations (GstRTPBin *rtp_bin, const gchar *destination)
{
    gchar **ips;
    gint i;

    ips = g_strsplit (destination, ";", 0);

    for (i = 0; i < g_strv_length (ips); i++)
    {
        gchar **split_ip;
        split_ip = g_strsplit (ips[i], ":", 2);

        GST_DEBUG("Found one address %s", ips[i]);

        if (g_strv_length (split_ip) != 2)
        {
            GST_DEBUG("Badly formated address %s, use ip:port format", ips[i]);
            g_strfreev (split_ip);
            continue;
        }

#ifdef HAVE_JRTP
        jrtpsession_setdestinationaddrs (rtp_bin->sess, split_ip[0],
                atoi(split_ip[1]));
        g_strfreev (split_ip);
#else
        GST_DEBUG("Jrtplib not enabled, taking first destination only %s",
                ips[i]);
        rtp_bin->dest_ip = inet_addr(split_ip[0]);
        rtp_bin->dest_port = htons(atoi(split_ip[1]));
        g_strfreev (split_ip);
        break;
#endif
    }
    g_strfreev (ips);
}

static void
gst_rtp_bin_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstRTPBin *rtp_bin;

    g_return_if_fail (GST_IS_RTP_BIN (object));

    rtp_bin = GST_RTP_BIN (object);

    GST_DEBUG("Setting props: %d", prop_id);

    switch (prop_id) {
        case ARG_RTCP_SUPPORT:
            rtp_bin->rtcp_support = g_value_get_boolean (value);
#ifndef HAVE_JRTP
            rtp_bin->rtcp_support = FALSE;
#endif
            break;
        case ARG_LOCALPORT:
            rtp_bin->localport = g_value_get_uint (value);
#ifdef HAVE_JRTP
            jrtpsession_setport (rtp_bin->sess, rtp_bin->localport);
#endif
            break;
        case ARG_DEFAULT_PT:
            rtp_bin->defaultpt = g_value_get_uint (value);
#ifdef HAVE_JRTP
            jrtpsession_setdefaultpt (rtp_bin->sess, rtp_bin->defaultpt);
#endif
            break;
        case ARG_DEFAULT_TS_INC:
            rtp_bin->defaulttsinc = g_value_get_uint (value);
#ifdef HAVE_JRTP
            jrtpsession_setdefaultinc (rtp_bin->sess, rtp_bin->defaulttsinc);
#endif
            break;
        case ARG_DEFAULT_MARK:
            rtp_bin->defaultmark = g_value_get_boolean (value);
#ifdef HAVE_JRTP
            jrtpsession_setdefaultmark (rtp_bin->sess, rtp_bin->defaultmark);
#endif
            break;
        case ARG_DESTINATION:
            gst_rtp_bin_set_destinations (rtp_bin, g_value_get_string (value));
            break;
        case ARG_RTP_SOCKFD:
            rtp_bin->rtp_sockfd = g_value_get_int (value);
            if (rtp_bin->rtpsink != NULL)
            {
              g_object_set (G_OBJECT(rtp_bin->rtpsink), "sockfd",
                  rtp_bin->rtp_sockfd, NULL);
              g_object_set (G_OBJECT(rtp_bin->rtpsink), "closefd",
                  rtp_bin->closefd, NULL);
            }
            break;
        case ARG_RTCP_SOCKFD:
            rtp_bin->rtcp_sockfd = g_value_get_int (value);
            if (rtp_bin->rtcpsink != NULL)
            {
              g_object_set (G_OBJECT(rtp_bin->rtcpsink), "sockfd",
                  rtp_bin->rtcp_sockfd, NULL);
              g_object_set (G_OBJECT(rtp_bin->rtcpsink), "closefd",
                  rtp_bin->closefd, NULL);
            }
            break;
        case ARG_PT_MAP:
            rtp_bin->user_pt_map = (GHashTable *) g_value_get_pointer (value);
            if (rtp_bin->rtprecv != NULL)
            {
              g_object_set (rtp_bin->rtprecv, "pt_map", rtp_bin->user_pt_map,
                  NULL);
            }
            break;
        case ARG_PT_CAPS:
            if (rtp_bin->pt_map)
            {
              GST_DEBUG_OBJECT (rtp_bin, "Replacing old pt-map");
              g_hash_table_destroy (rtp_bin->pt_map);
            }
            rtp_bin->pt_map =
              gst_rtp_bin_build_pt_map (g_value_get_string (value));
            if (rtp_bin->rtprecv != NULL)
            {
              g_object_set (rtp_bin->rtprecv, "pt_map", rtp_bin->pt_map, NULL);
            }
            break;
        case ARG_BYPASS_UDP:
            rtp_bin->bypass_udp = g_value_get_boolean (value);
            break;
        case ARG_QUEUE_DELAY:
            rtp_bin->queue_delay = g_value_get_uint (value);
            if (rtp_bin->jbuf) 
              g_object_set (rtp_bin->jbuf, "latency", rtp_bin->queue_delay,
                  NULL);
            break;
        case ARG_CLOSE_FD:
            rtp_bin->closefd = g_value_get_boolean (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gst_rtp_bin_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
    GstRTPBin *rtp_bin;

    g_return_if_fail (GST_IS_RTP_BIN (object));

    rtp_bin = GST_RTP_BIN (object);

    switch (prop_id) {
        case ARG_RTCP_SUPPORT:
            g_value_set_boolean (value, rtp_bin->rtcp_support);
            break;
        case ARG_LOCALPORT:
            g_value_set_uint (value, rtp_bin->localport);
            break;
        case ARG_DEFAULT_PT:
            g_value_set_uint (value, rtp_bin->defaultpt);
            break;
        case ARG_DEFAULT_TS_INC:
            g_value_set_uint (value, rtp_bin->defaulttsinc);
            break;
        case ARG_DEFAULT_MARK:
            g_value_set_boolean (value, rtp_bin->defaultmark);
            break;
        case ARG_RTP_SOCKFD:
            g_value_set_int (value, rtp_bin->rtp_sockfd);
            break;
        case ARG_RTCP_SOCKFD:
            g_value_set_int (value, rtp_bin->rtcp_sockfd);
            break;
        case ARG_PT_MAP:
            g_value_set_pointer (value, (gpointer) rtp_bin->pt_map);
            break;
        case ARG_BYPASS_UDP:
            g_value_set_boolean (value, rtp_bin->bypass_udp);
            break;
        case ARG_QUEUE_DELAY:
	    g_value_set_uint (value, rtp_bin->queue_delay);
	    break;
        case ARG_CLOSE_FD:
	    g_value_set_boolean (value, rtp_bin->closefd);
	    break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}
