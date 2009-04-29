/* Farsight
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

#include <string.h>
#include <gst/gst.h>

#include "gstmsgsmdepayload.h"

GST_DEBUG_CATEGORY (msgsmdepayload_debug);
#define GST_CAT_DEFAULT (msgsmdepayload_debug)

/* Filter signals and args */
enum {
    /* FILL ME */
    LAST_SIGNAL
};

enum {
    ARG_0,
};

static GstStaticPadTemplate src_factory =
GST_STATIC_PAD_TEMPLATE (
        "src",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS ("audio/x-msgsm")
        );

static GstStaticPadTemplate sink_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) \"audio\", "
        "payload = (int) [ 0, 255 ], "
        "clock-rate = (int) 8000, " "encoding-name = (string) \"MSGSM\"")
    );

static void	gst_msgsmdepayload_class_init (GstMSGSMDepayloadClass *klass);
static void	gst_msgsmdepayload_base_init (GstMSGSMDepayloadClass *klass);
static void	gst_msgsmdepayload_init (GstMSGSMDepayload *filter);
static void gst_msgsmdepayload_finalize (GObject * object);
static void	gst_msgsmdepayload_set_property (GObject *object, guint prop_id,
        const GValue *value,
        GParamSpec *pspec);
static void	gst_msgsmdepayload_get_property (GObject *object, guint prop_id,
        GValue *value,
        GParamSpec *pspec);
static GstBuffer *gst_msgsmdepayload_process (GstBaseRTPDepayload *base, 
        GstBuffer *in);

static GstElementClass *parent_class = NULL;

GType
gst_msgsmdepayload_get_type (void)
{
    static GType plugin_type = 0;

    if (!plugin_type)
    {
        static const GTypeInfo plugin_info =
        {
            sizeof (GstMSGSMDepayloadClass),
            (GBaseInitFunc) gst_msgsmdepayload_base_init,
            NULL,
            (GClassInitFunc) gst_msgsmdepayload_class_init,
            NULL,
            NULL,
            sizeof (GstMSGSMDepayload),
            0,
            (GInstanceInitFunc) gst_msgsmdepayload_init,
        };
        plugin_type = g_type_register_static (GST_TYPE_BASE_RTP_DEPAYLOAD,
                "GstMSGSMDepayload",
                &plugin_info, 0);
    }
    return plugin_type;
}

static void
gst_msgsmdepayload_base_init (GstMSGSMDepayloadClass *klass)
{
    static GstElementDetails plugin_details = {
        "MSGSM RTP Depayloader",
        "Codec/Depayr/Network",
        "Depayloader for MsGSM RTP streams using GstBaseRTPDepayload",
        "Philippe Khalaf <burger@speedy.org>"
    };
    GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

    gst_element_class_add_pad_template (element_class,
            gst_static_pad_template_get (&src_factory));
    gst_element_class_add_pad_template (element_class,
	    gst_static_pad_template_get (&sink_factory));

    gst_element_class_set_details (element_class, &plugin_details);
}

/* initialize the plugin's class */
static void
gst_msgsmdepayload_class_init (GstMSGSMDepayloadClass *klass)
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;
    GstBaseRTPDepayloadClass *gstbasertpdepayload_class;

    gobject_class = (GObjectClass*) klass;
    gstelement_class = (GstElementClass*) klass;
    gstbasertpdepayload_class = (GstBaseRTPDepayloadClass*) klass;

    parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

    gobject_class->set_property = gst_msgsmdepayload_set_property;
    gobject_class->get_property = gst_msgsmdepayload_get_property;

    gobject_class->finalize = gst_msgsmdepayload_finalize;

    gstbasertpdepayload_class->process = gst_msgsmdepayload_process;

    GST_DEBUG_CATEGORY_INIT (msgsmdepayload_debug, "msgsmdepayload", 0, "MSGSM RTP Depayloader");
}

/* initialize the new element
 * instantiate pads and add them to element
 * set functions
 * initialize structure
 */
static void
gst_msgsmdepayload_init (GstMSGSMDepayload *filter)
{
//    GstElementClass *klass = GST_ELEMENT_GET_CLASS (filter);
    GstBaseRTPDepayload *base = GST_BASE_RTP_DEPAYLOAD (filter);

    // clock rate for this payload type
    base->clock_rate = 8000;
}

static void gst_msgsmdepayload_finalize (GObject * object)
{
    if (G_OBJECT_CLASS (parent_class)->finalize)
        G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GstBuffer *gst_msgsmdepayload_process (GstBaseRTPDepayload *base, GstBuffer *in)
{
    GstBuffer *ret = NULL;
    guint8 *payload;
    GstBuffer *out_buf;

    GST_DEBUG ("process : got %d bytes, mark %d ts %u seqn %d", 
            GST_BUFFER_SIZE (in),
            gst_rtp_buffer_get_marker (in),
            gst_rtp_buffer_get_timestamp (in),
            gst_rtp_buffer_get_seq (in));

    // Dosen't get more simple than this, we simply remove the rtp headers
    // and give the buffer to our base class
    payload = gst_rtp_buffer_get_payload (in);
    gst_util_dump_mem (payload, gst_rtp_buffer_get_payload_len(in));

    out_buf = gst_buffer_new_and_alloc (65);
    memcpy (GST_BUFFER_DATA(out_buf), payload, 65);

    ret = out_buf;

    return ret;
}

static void
gst_msgsmdepayload_set_property (GObject *object, guint prop_id,
        const GValue *value, GParamSpec *pspec)
{
    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
gst_msgsmdepayload_get_property (GObject *object, guint prop_id,
        GValue *value, GParamSpec *pspec)
{
    switch (prop_id) 
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

gboolean
gst_msgsmdepayload_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "msgsmdepayloader",
      GST_RANK_NONE, GST_TYPE_MSGSMDEPAYLOAD);
}
