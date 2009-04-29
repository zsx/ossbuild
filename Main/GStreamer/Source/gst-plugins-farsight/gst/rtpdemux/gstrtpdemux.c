/* 
 * RTP Demux element
 *
 * Copyright (C) 2005 Nokia Corporation.
 * @author Kai Vehmanen <kai.vehmanen@nokia.com>
 *
 * Loosely based on GStreamer gstdecodebin
 * Copyright (C) <2004> Wim Taymans <wim@fluendo.com>
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
 * Contributors:
 * Andre Moreira Magalhaes <andre.magalhaes@indt.org.br>
 */

/*
 * Status:
 *  - works with the test_rtpdemux.c tool
 *
 * Check:
 *  - is emitting a signal enough, or should we
 *    use GstEvent to notify downstream elements
 *    of the new packet... no?
 *
 * Notes:
 *  - emits event both for new PTs, and whenever
 *    a PT is changed
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <gst/gst.h>
#include <gst/rtp/gstrtpbuffer.h>

#include "gstrtpdemux-marshal.h"

/* generic templates */
static GstStaticPadTemplate rtp_demux_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "payload = (int) [ 0, 255 ], "
        "clock-rate = (int) [ 0, 2147483647 ]")
    );

static GstStaticPadTemplate rtp_demux_src_template =
GST_STATIC_PAD_TEMPLATE ("src%d",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS_ANY);

GST_DEBUG_CATEGORY_STATIC (gst_rtp_demux_debug);
#define GST_CAT_DEFAULT gst_rtp_demux_debug

#define GST_TYPE_RTP_DEMUX            (gst_rtp_demux_get_type())
#define GST_RTP_DEMUX(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RTP_DEMUX,GstRtpDemux))
#define GST_RTP_DEMUX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RTP_DEMUX,GstRtpDemuxClass))
#define GST_IS_RTP_DEMUX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RTP_DEMUX))
#define GST_IS_RTP_DEMUX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RTP_DEMUX))

typedef struct _GstRtpDemux GstRtpDemux;
typedef struct _GstRtpDemuxClass GstRtpDemuxClass;
typedef struct _GstRtpDemuxPad GstRtpDemuxPad;

struct _GstRtpDemux
{
  GstElement parent;  /**< parent class */

  GstPad *sink;       /**< the sink pad */
  guint16 last_pt;    /**< pt of the last packet 0xFFFF if none */
  GSList *srcpads;    /**< a linked list of GstRtpDemuxPad objects */
};

struct _GstRtpDemuxClass
{
  GstElementClass parent_class;

  /* signal emmited when a new PT is found from the incoming stream */
  void (* new_payload_type) (GstElement *element, gint pt, GstPad *pad);

  /* signal emitted when the payload type changes */
  void (* payload_type_change) (GstElement *element, gint pt);
};

/**
 * Item for storing GstPad<->pt pairs.
 */
struct _GstRtpDemuxPad {
  GstPad *pad;        /**< pointer to the actual pad */
  gint pt;             /**< RTP payload-type attached to pad */
};

/* signals */
enum
{
  SIGNAL_NEW_PAYLOAD_TYPE,
  SIGNAL_PAYLOAD_TYPE_CHANGE,
  LAST_SIGNAL
};

#define DEBUG_INIT(bla) \
  GST_DEBUG_CATEGORY_INIT (gst_rtp_demux_debug, "rtpdemux", 0, "RTP Demux");

GST_BOILERPLATE_FULL (GstRtpDemux, gst_rtp_demux, GstElement, GST_TYPE_ELEMENT, DEBUG_INIT);

static void                 gst_rtp_demux_dispose      (GObject       *object);

static void                 gst_rtp_demux_release      (GstElement    *element);
static gboolean             gst_rtp_demux_setup        (GstElement    *element);

static GstFlowReturn        gst_rtp_demux_chain        (GstPad        *pad,
                                                        GstBuffer     *buf);
static GstCaps             *gst_rtp_demux_getcaps      (GstPad        *pad);
static GstStateChangeReturn gst_rtp_demux_change_state (GstElement    *element,
                                                        GstStateChange transition);

static GstPad              *find_pad_for_pt            (GstRtpDemux   *rtpdemux,
                                                        guint8         pt);

static GstFlowReturn       gst_rtp_demux_pad_unlinked  (GstRtpDemux   *rtpdemux,
                                                        GstPad        *pad);

static void gst_rtp_demux_release_pad (GstElement *element, GstPad *pad);

static guint gst_rtp_demux_signals[LAST_SIGNAL] = { 0 };

static GstElementDetails gst_rtp_demux_details = {
  "RTP Demux",
  /* XXX: what's the correct hierarchy? */
  "Codec/Demux/Network",
  "Parses codec streams transmitted in the same RTP session",
  "Kai Vehmanen <kai.vehmanen@nokia.com>"
};

static void
gst_rtp_demux_base_init (gpointer g_class)
{
  GstElementClass *gstelement_klass = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template (gstelement_klass,
      gst_static_pad_template_get (&rtp_demux_sink_template));
  gst_element_class_add_pad_template (gstelement_klass,
      gst_static_pad_template_get (&rtp_demux_src_template));

  gst_element_class_set_details (gstelement_klass, &gst_rtp_demux_details);
}

static void
gst_rtp_demux_class_init (GstRtpDemuxClass * klass)
{
  GObjectClass *gobject_klass;
  GstElementClass *gstelement_klass;

  gobject_klass = (GObjectClass *) klass;
  gstelement_klass = (GstElementClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gst_rtp_demux_signals[SIGNAL_NEW_PAYLOAD_TYPE] =
      g_signal_new ("new-payload-type",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (GstRtpDemuxClass, new_payload_type),
            NULL, NULL,
            gst_rtp_demux_marshal_VOID__UINT_OBJECT,
            G_TYPE_NONE,
            2,
            G_TYPE_INT,
            GST_TYPE_PAD);
  gst_rtp_demux_signals[SIGNAL_PAYLOAD_TYPE_CHANGE] =
      g_signal_new ("payload-type-change",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_LAST,
            G_STRUCT_OFFSET (GstRtpDemuxClass, payload_type_change),
            NULL, NULL,
            gst_marshal_VOID__INT,
            G_TYPE_NONE,
            1,
            G_TYPE_INT);

  gobject_klass->dispose = GST_DEBUG_FUNCPTR (gst_rtp_demux_dispose);

  gstelement_klass->change_state =
      GST_DEBUG_FUNCPTR (gst_rtp_demux_change_state);

  gstelement_klass->release_pad =
      GST_DEBUG_FUNCPTR (gst_rtp_demux_release_pad);
}

static void
gst_rtp_demux_init (GstRtpDemux *rtp_demux,
                    GstRtpDemuxClass *g_class)
{
  GstElementClass *klass = GST_ELEMENT_GET_CLASS(rtp_demux);

  rtp_demux->sink = gst_pad_new_from_template (
          gst_element_class_get_pad_template (klass, "sink"), "sink");
  g_assert (rtp_demux->sink != NULL);

  gst_pad_set_chain_function (rtp_demux->sink, gst_rtp_demux_chain);

  gst_element_add_pad (GST_ELEMENT(rtp_demux), rtp_demux->sink);
}

static void
gst_rtp_demux_dispose (GObject *object)
{
  gst_rtp_demux_release (GST_ELEMENT (object));

  if (G_OBJECT_CLASS (parent_class)->dispose) {
    G_OBJECT_CLASS (parent_class)->dispose (object);
  }
}

static gint
compare_srcpad (gconstpointer a, gconstpointer b)
{
  GstRtpDemuxPad *first = (GstRtpDemuxPad *)a;
  GstPad *bpad = (GstPad *)b;
  return first->pad - bpad;
}

static void
gst_rtp_demux_release_pad (GstElement *element, GstPad *pad) {
  GST_DEBUG_OBJECT (element, "Releasing pad %p of element %p", pad, element);
  gst_rtp_demux_pad_unlinked (GST_RTP_DEMUX (element), pad);
}

static GstFlowReturn
gst_rtp_demux_pad_unlinked (GstRtpDemux *rtpdemux, GstPad *pad)
{
  GSList *found_pad;
  GstFlowReturn ret = GST_FLOW_OK;

  /* first let's remove it from the internal pad list */
  found_pad = g_slist_find_custom (rtpdemux->srcpads, pad, compare_srcpad);
  if (!found_pad)
  {
    GST_DEBUG_OBJECT (rtpdemux, "Couldn't find unlinked "
        "pad in internal pad list!, returning GST_FLOW_ERROR");
    ret = GST_FLOW_ERROR;
  }
  else
  {
    rtpdemux->srcpads = g_slist_remove_link (rtpdemux->srcpads, found_pad);
    g_free (found_pad->data);
    g_slist_free_1 (found_pad);

    /* now we remove the pad */
    if (!gst_pad_set_active (pad, FALSE))
    {
      GST_DEBUG_OBJECT (rtpdemux, "Couldn't deactive unlinked pad");
      ret = GST_FLOW_ERROR;
    }
    if (!gst_element_remove_pad (GST_ELEMENT (rtpdemux), pad))
    {
      GST_DEBUG_OBJECT (rtpdemux, "Couldn't remove unlinked pad");
      ret = GST_FLOW_ERROR;
    }
  }

  return ret;
}

static GstFlowReturn
gst_rtp_demux_chain (GstPad *pad, GstBuffer *buf)
{
  GstFlowReturn ret = GST_FLOW_OK;
  GstRtpDemux *rtpdemux;
  GstElement *element = GST_ELEMENT(GST_OBJECT_PARENT (pad));
  guint8 pt;
  GstPad *srcpad;
  gboolean first_try = TRUE;

  rtpdemux = GST_RTP_DEMUX (GST_OBJECT_PARENT (pad));

  if (!gst_rtp_buffer_validate (buf)) {
    /* We should never get here, invalid buffers should have been rejected
     * much earlier
     */
    GST_ELEMENT_WARNING (element, STREAM, DECODE, (NULL),
        ("Received invalid RTP payload"));
    gst_buffer_unref (buf);
    return GST_FLOW_OK;
  }
  pt = gst_rtp_buffer_get_payload_type (buf);

again:

  srcpad = find_pad_for_pt (rtpdemux, pt);
  if (srcpad == NULL) {
    /* new PT, create a src pad */
    GstElementClass *klass;
    GstPadTemplate *templ;
    gchar *padname;
    GstCaps *caps;
    GstRtpDemuxPad *rtpdemuxpad;

    klass = GST_ELEMENT_GET_CLASS (rtpdemux);
    templ = gst_element_class_get_pad_template (klass, "src%d");
    padname = g_strdup_printf ("src%d", pt);
    srcpad = gst_pad_new_from_template (templ, padname);
    g_free (padname);

    caps = gst_pad_get_caps (srcpad);
    caps = gst_caps_make_writable (caps);
    gst_caps_append_structure (caps,
            gst_structure_new ("payload",
                "payload", G_TYPE_INT, pt, NULL));
    gst_pad_set_caps (srcpad, caps);
    gst_caps_unref (caps);

    /* XXX: set _link () function */
    gst_pad_set_getcaps_function (srcpad, gst_rtp_demux_getcaps);
    /* This could break with gstreamer 0.10.9 */
    gst_pad_set_active (srcpad, TRUE);
    gst_element_add_pad (element, srcpad);

    if (srcpad) {
      GST_DEBUG ("Adding pt=%d to the list.", pt);
      rtpdemuxpad = g_new0 (GstRtpDemuxPad, 1);
      rtpdemuxpad->pt = pt;
      rtpdemuxpad->pad = srcpad;
      rtpdemux->srcpads = g_slist_append (rtpdemux->srcpads, rtpdemuxpad);

      GST_DEBUG ("emitting new-payload_type for pt %d", pt);
      g_signal_emit (G_OBJECT (rtpdemux),
              gst_rtp_demux_signals[SIGNAL_NEW_PAYLOAD_TYPE], 0, pt, srcpad);
      first_try = FALSE;
    }
  }

  if (pt != rtpdemux->last_pt) {
    gint emit_pt = pt;
    /* our own signal with an extra flag that this is the only pad */
    rtpdemux->last_pt = pt;
    GST_DEBUG ("emitting payload-type-changed for pt %d", emit_pt);
    g_signal_emit (G_OBJECT (rtpdemux),
            gst_rtp_demux_signals[SIGNAL_PAYLOAD_TYPE_CHANGE], 0, emit_pt);
  }

  /* push to srcpad */
  /* let's ref the buffer in case we need to resend it if it fails */
  gst_buffer_ref (buf);
  if (srcpad)
    ret = gst_pad_push (srcpad, GST_BUFFER (buf));

  if (ret == GST_FLOW_NOT_LINKED)
  {
    /* let's remove this unlinked pad */
    ret = gst_rtp_demux_pad_unlinked (rtpdemux, srcpad);

    if (first_try)
    {
      goto again;
    }
  }

  gst_buffer_unref (buf);
  return ret;
}

static GstCaps *
gst_rtp_demux_getcaps (GstPad *pad)
{
  GstCaps *caps;

  GST_OBJECT_LOCK (pad);
  if ((caps = GST_PAD_CAPS (pad)))
    caps = gst_caps_ref (caps);
  GST_OBJECT_UNLOCK (pad);

  return caps;
}

static GstPad *
find_pad_for_pt (GstRtpDemux *rtpdemux, guint8 pt)
{
  GstPad *respad = NULL;
  GSList *item = rtpdemux->srcpads;
  for (; item ; item = g_slist_next (item)) {
    GstRtpDemuxPad *pad = item->data;
    if (pad->pt == pt) {
      respad = pad->pad;
      break;
    }
  }

  return respad;
}

/**
 * Reserves resources for the object.
 */
static gboolean
gst_rtp_demux_setup (GstElement * element)
{
  GstRtpDemux *rtp_demux = GST_RTP_DEMUX (element);
  gboolean res = TRUE;

  if (rtp_demux) {
    rtp_demux->srcpads = NULL;
    rtp_demux->last_pt = 0xFFFF;
  }

  return res;
}

/**
 * Free resources for the object.
 */
static void
gst_rtp_demux_release (GstElement * element)
{
  GstRtpDemux *rtp_demux = GST_RTP_DEMUX (element);

  if (rtp_demux) {
    /* note: GstElement's dispose() will handle the pads */
    g_slist_free (rtp_demux->srcpads);
    rtp_demux->srcpads = NULL;
  }
}

static GstStateChangeReturn
gst_rtp_demux_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret;
  GstRtpDemux *rtp_demux;

  rtp_demux = GST_RTP_DEMUX (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      if (gst_rtp_demux_setup (element) != TRUE)
        ret = GST_STATE_CHANGE_FAILURE;
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      gst_rtp_demux_release (element);
      break;
    default:
      break;
  }

  return ret;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_rtp_demux_debug,
          "rtpdemux", 0, "RTP codec demuxer");

  return gst_element_register (plugin,
          "rtpdemux", GST_RANK_NONE, GST_TYPE_RTP_DEMUX);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "rtpdemux",
    "RTP codec demuxer",
    plugin_init,
    VERSION,
    "LGPL",
    "Farsight",
    "http://farsight.sf.net")
