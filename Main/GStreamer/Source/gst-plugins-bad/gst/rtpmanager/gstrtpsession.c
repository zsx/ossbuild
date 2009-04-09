/* GStreamer
 * Copyright (C) <2007> Wim Taymans <wim.taymans@gmail.com>
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

/**
 * SECTION:element-gstrtpsession
 * @see_also: gstrtpjitterbuffer, gstrtpbin, gstrtpptdemux, gstrtpssrcdemux
 *
 * The RTP session manager models one participant with a unique SSRC in an RTP
 * session. This session can be used to send and receive RTP and RTCP packets.
 * Based on what REQUEST pads are requested from the session manager, specific
 * functionality can be activated.
 * 
 * The session manager currently implements RFC 3550 including:
 * <itemizedlist>
 *   <listitem>
 *     <para>RTP packet validation based on consecutive sequence numbers.</para>
 *   </listitem>
 *   <listitem>
 *     <para>Maintainance of the SSRC participant database.</para>
 *   </listitem>
 *   <listitem>
 *     <para>Keeping per participant statistics based on received RTCP packets.</para>
 *   </listitem>
 *   <listitem>
 *     <para>Scheduling of RR/SR RTCP packets.</para>
 *   </listitem>
 * </itemizedlist>
 * 
 * The gstrtpsession will not demux packets based on SSRC or payload type, nor will
 * it correct for packet reordering and jitter. Use #GstRtpsSrcDemux,
 * #GstRtpPtDemux and GstRtpJitterBuffer in addition to #GstRtpSession to
 * perform these tasks. It is usually a good idea to use #GstRtpBin, which
 * combines all these features in one element.
 * 
 * To use #GstRtpSession as an RTP receiver, request a recv_rtp_sink pad, which will
 * automatically create recv_rtp_src pad. Data received on the recv_rtp_sink pad
 * will be processed in the session and after being validated forwarded on the
 * recv_rtp_src pad.
 * 
 * To also use #GstRtpSession as an RTCP receiver, request a recv_rtcp_sink pad,
 * which will automatically create a sync_src pad. Packets received on the RTCP
 * pad will be used by the session manager to update the stats and database of
 * the other participants. SR packets will be forwarded on the sync_src pad
 * so that they can be used to perform inter-stream synchronisation when needed.
 * 
 * If you want the session manager to generate and send RTCP packets, request
 * the send_rtcp_src pad. Packet pushed on this pad contain SR/RR RTCP reports
 * that should be sent to all participants in the session.
 * 
 * To use #GstRtpSession as a sender, request a send_rtp_sink pad, which will
 * automatically create a send_rtp_src pad. The session manager will modify the
 * SSRC in the RTP packets to its own SSRC and wil forward the packets on the
 * send_rtp_src pad after updating its internal state.
 * 
 * The session manager needs the clock-rate of the payload types it is handling
 * and will signal the #GstRtpSession::request-pt-map signal when it needs such a
 * mapping. One can clear the cached values with the #GstRtpSession::clear-pt-map
 * signal.
 * 
 * <refsect2>
 * <title>Example pipelines</title>
 * |[
 * gst-launch udpsrc port=5000 caps="application/x-rtp, ..." ! .recv_rtp_sink gstrtpsession .recv_rtp_src ! rtptheoradepay ! theoradec ! xvimagesink
 * ]| Receive theora RTP packets from port 5000 and send them to the depayloader,
 * decoder and display. Note that the application/x-rtp caps on udpsrc should be
 * configured based on some negotiation process such as RTSP for this pipeline
 * to work correctly.
 * |[
 * gst-launch udpsrc port=5000 caps="application/x-rtp, ..." ! .recv_rtp_sink gstrtpsession name=session \
 *        .recv_rtp_src ! rtptheoradepay ! theoradec ! xvimagesink \
 *     udpsrc port=5001 caps="application/x-rtcp" ! session.recv_rtcp_sink
 * ]| Receive theora RTP packets from port 5000 and send them to the depayloader,
 * decoder and display. Receive RTCP packets from port 5001 and process them in
 * the session manager.
 * Note that the application/x-rtp caps on udpsrc should be
 * configured based on some negotiation process such as RTSP for this pipeline
 * to work correctly.
 * |[
 * gst-launch videotestsrc ! theoraenc ! rtptheorapay ! .send_rtp_sink gstrtpsession .send_rtp_src ! udpsink port=5000
 * ]| Send theora RTP packets through the session manager and out on UDP port
 * 5000.
 * |[
 * gst-launch videotestsrc ! theoraenc ! rtptheorapay ! .send_rtp_sink gstrtpsession name=session .send_rtp_src \
 *     ! udpsink port=5000  session.send_rtcp_src ! udpsink port=5001
 * ]| Send theora RTP packets through the session manager and out on UDP port
 * 5000. Send RTCP packets on port 5001. Note that this pipeline will not preroll
 * correctly because the second udpsink will not preroll correctly (no RTCP
 * packets are sent in the PAUSED state). Applications should manually set and
 * keep (see gst_element_set_locked_state()) the RTCP udpsink to the PLAYING state.
 * </refsect2>
 *
 * Last reviewed on 2007-05-28 (0.10.5)
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/rtp/gstrtpbuffer.h>

#include "gstrtpbin-marshal.h"
#include "gstrtpsession.h"
#include "rtpsession.h"

GST_DEBUG_CATEGORY_STATIC (gst_rtp_session_debug);
#define GST_CAT_DEFAULT gst_rtp_session_debug

/* elementfactory information */
static const GstElementDetails rtpsession_details =
GST_ELEMENT_DETAILS ("RTP Session",
    "Filter/Network/RTP",
    "Implement an RTP session",
    "Wim Taymans <wim.taymans@gmail.com>");

/* sink pads */
static GstStaticPadTemplate rtpsession_recv_rtp_sink_template =
GST_STATIC_PAD_TEMPLATE ("recv_rtp_sink",
    GST_PAD_SINK,
    GST_PAD_REQUEST,
    GST_STATIC_CAPS ("application/x-rtp")
    );

static GstStaticPadTemplate rtpsession_recv_rtcp_sink_template =
GST_STATIC_PAD_TEMPLATE ("recv_rtcp_sink",
    GST_PAD_SINK,
    GST_PAD_REQUEST,
    GST_STATIC_CAPS ("application/x-rtcp")
    );

static GstStaticPadTemplate rtpsession_send_rtp_sink_template =
GST_STATIC_PAD_TEMPLATE ("send_rtp_sink",
    GST_PAD_SINK,
    GST_PAD_REQUEST,
    GST_STATIC_CAPS ("application/x-rtp")
    );

/* src pads */
static GstStaticPadTemplate rtpsession_recv_rtp_src_template =
GST_STATIC_PAD_TEMPLATE ("recv_rtp_src",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS ("application/x-rtp")
    );

static GstStaticPadTemplate rtpsession_sync_src_template =
GST_STATIC_PAD_TEMPLATE ("sync_src",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS ("application/x-rtcp")
    );

static GstStaticPadTemplate rtpsession_send_rtp_src_template =
GST_STATIC_PAD_TEMPLATE ("send_rtp_src",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS ("application/x-rtp")
    );

static GstStaticPadTemplate rtpsession_send_rtcp_src_template =
GST_STATIC_PAD_TEMPLATE ("send_rtcp_src",
    GST_PAD_SRC,
    GST_PAD_REQUEST,
    GST_STATIC_CAPS ("application/x-rtcp")
    );

/* signals and args */
enum
{
  SIGNAL_REQUEST_PT_MAP,
  SIGNAL_CLEAR_PT_MAP,

  SIGNAL_ON_NEW_SSRC,
  SIGNAL_ON_SSRC_COLLISION,
  SIGNAL_ON_SSRC_VALIDATED,
  SIGNAL_ON_SSRC_ACTIVE,
  SIGNAL_ON_SSRC_SDES,
  SIGNAL_ON_BYE_SSRC,
  SIGNAL_ON_BYE_TIMEOUT,
  SIGNAL_ON_TIMEOUT,
  SIGNAL_ON_SENDER_TIMEOUT,
  LAST_SIGNAL
};

#define DEFAULT_NTP_NS_BASE          0
#define DEFAULT_BANDWIDTH            RTP_STATS_BANDWIDTH
#define DEFAULT_RTCP_FRACTION        RTP_STATS_RTCP_BANDWIDTH
#define DEFAULT_SDES_CNAME           NULL
#define DEFAULT_SDES_NAME            NULL
#define DEFAULT_SDES_EMAIL           NULL
#define DEFAULT_SDES_PHONE           NULL
#define DEFAULT_SDES_LOCATION        NULL
#define DEFAULT_SDES_TOOL            NULL
#define DEFAULT_SDES_NOTE            NULL
#define DEFAULT_NUM_SOURCES          0
#define DEFAULT_NUM_ACTIVE_SOURCES   0

enum
{
  PROP_0,
  PROP_NTP_NS_BASE,
  PROP_BANDWIDTH,
  PROP_RTCP_FRACTION,
  PROP_SDES_CNAME,
  PROP_SDES_NAME,
  PROP_SDES_EMAIL,
  PROP_SDES_PHONE,
  PROP_SDES_LOCATION,
  PROP_SDES_TOOL,
  PROP_SDES_NOTE,
  PROP_NUM_SOURCES,
  PROP_NUM_ACTIVE_SOURCES,
  PROP_INTERNAL_SESSION,
  PROP_LAST
};

#define GST_RTP_SESSION_GET_PRIVATE(obj)  \
	   (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GST_TYPE_RTP_SESSION, GstRtpSessionPrivate))

#define GST_RTP_SESSION_LOCK(sess)   g_mutex_lock ((sess)->priv->lock)
#define GST_RTP_SESSION_UNLOCK(sess) g_mutex_unlock ((sess)->priv->lock)

struct _GstRtpSessionPrivate
{
  GMutex *lock;
  GstClock *sysclock;

  RTPSession *session;

  /* thread for sending out RTCP */
  GstClockID id;
  gboolean stop_thread;
  GThread *thread;
  gboolean thread_stopped;

  /* caps mapping */
  GHashTable *ptmap;

  /* NTP base time */
  guint64 ntpnsbase;
};

/* callbacks to handle actions from the session manager */
static GstFlowReturn gst_rtp_session_process_rtp (RTPSession * sess,
    RTPSource * src, GstBuffer * buffer, gpointer user_data);
static GstFlowReturn gst_rtp_session_send_rtp (RTPSession * sess,
    RTPSource * src, GstBuffer * buffer, gpointer user_data);
static GstFlowReturn gst_rtp_session_send_rtcp (RTPSession * sess,
    RTPSource * src, GstBuffer * buffer, gboolean eos, gpointer user_data);
static GstFlowReturn gst_rtp_session_sync_rtcp (RTPSession * sess,
    RTPSource * src, GstBuffer * buffer, gpointer user_data);
static gint gst_rtp_session_clock_rate (RTPSession * sess, guint8 payload,
    gpointer user_data);
static void gst_rtp_session_reconsider (RTPSession * sess, gpointer user_data);

static RTPSessionCallbacks callbacks = {
  gst_rtp_session_process_rtp,
  gst_rtp_session_send_rtp,
  gst_rtp_session_sync_rtcp,
  gst_rtp_session_send_rtcp,
  gst_rtp_session_clock_rate,
  gst_rtp_session_reconsider
};

/* GObject vmethods */
static void gst_rtp_session_finalize (GObject * object);
static void gst_rtp_session_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_rtp_session_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

/* GstElement vmethods */
static GstStateChangeReturn gst_rtp_session_change_state (GstElement * element,
    GstStateChange transition);
static GstPad *gst_rtp_session_request_new_pad (GstElement * element,
    GstPadTemplate * templ, const gchar * name);
static void gst_rtp_session_release_pad (GstElement * element, GstPad * pad);

static void gst_rtp_session_clear_pt_map (GstRtpSession * rtpsession);

static guint gst_rtp_session_signals[LAST_SIGNAL] = { 0 };

static void
on_new_ssrc (RTPSession * session, RTPSource * src, GstRtpSession * sess)
{
  g_signal_emit (sess, gst_rtp_session_signals[SIGNAL_ON_NEW_SSRC], 0,
      src->ssrc);
}

static void
on_ssrc_collision (RTPSession * session, RTPSource * src, GstRtpSession * sess)
{
  g_signal_emit (sess, gst_rtp_session_signals[SIGNAL_ON_SSRC_COLLISION], 0,
      src->ssrc);
}

static void
on_ssrc_validated (RTPSession * session, RTPSource * src, GstRtpSession * sess)
{
  g_signal_emit (sess, gst_rtp_session_signals[SIGNAL_ON_SSRC_VALIDATED], 0,
      src->ssrc);
}

static void
on_ssrc_active (RTPSession * session, RTPSource * src, GstRtpSession * sess)
{
  g_signal_emit (sess, gst_rtp_session_signals[SIGNAL_ON_SSRC_ACTIVE], 0,
      src->ssrc);
}

static GstStructure *
source_get_sdes_structure (RTPSource * src)
{
  GstStructure *result;
  GValue val = { 0 };
  gchar *str;

  result = gst_structure_empty_new ("GstRTPSessionSDES");

  gst_structure_set (result, "ssrc", G_TYPE_UINT, src->ssrc, NULL);

  g_value_init (&val, G_TYPE_STRING);
  str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_CNAME);
  if (str) {
    g_value_take_string (&val, str);
    gst_structure_set_value (result, "cname", &val);
  }
  str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_NAME);
  if (str) {
    g_value_take_string (&val, str);
    gst_structure_set_value (result, "name", &val);
  }
  str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_EMAIL);
  if (str) {
    g_value_take_string (&val, str);
    gst_structure_set_value (result, "email", &val);
  }
  str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_PHONE);
  if (str) {
    g_value_take_string (&val, str);
    gst_structure_set_value (result, "phone", &val);
  }
  str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_LOC);
  if (str) {
    g_value_take_string (&val, str);
    gst_structure_set_value (result, "location", &val);
  }
  str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_TOOL);
  if (str) {
    g_value_take_string (&val, str);
    gst_structure_set_value (result, "tool", &val);
  }
  str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_NOTE);
  if (str) {
    g_value_take_string (&val, str);
    gst_structure_set_value (result, "note", &val);
  }
  str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_PRIV);
  if (str) {
    g_value_take_string (&val, str);
    gst_structure_set_value (result, "priv", &val);
  }
  g_value_unset (&val);

  return result;
}

static void
on_ssrc_sdes (RTPSession * session, RTPSource * src, GstRtpSession * sess)
{
  GstStructure *s;
  GstMessage *m;

  /* convert the new SDES info into a message */
  RTP_SESSION_LOCK (session);
  s = source_get_sdes_structure (src);
  RTP_SESSION_UNLOCK (session);
  m = gst_message_new_custom (GST_MESSAGE_ELEMENT, GST_OBJECT (sess), s);
  gst_element_post_message (GST_ELEMENT_CAST (sess), m);

  g_signal_emit (sess, gst_rtp_session_signals[SIGNAL_ON_SSRC_SDES], 0,
      src->ssrc);
}

static void
on_bye_ssrc (RTPSession * session, RTPSource * src, GstRtpSession * sess)
{
  g_signal_emit (sess, gst_rtp_session_signals[SIGNAL_ON_BYE_SSRC], 0,
      src->ssrc);
}

static void
on_bye_timeout (RTPSession * session, RTPSource * src, GstRtpSession * sess)
{
  g_signal_emit (sess, gst_rtp_session_signals[SIGNAL_ON_BYE_TIMEOUT], 0,
      src->ssrc);
}

static void
on_timeout (RTPSession * session, RTPSource * src, GstRtpSession * sess)
{
  g_signal_emit (sess, gst_rtp_session_signals[SIGNAL_ON_TIMEOUT], 0,
      src->ssrc);
}

static void
on_sender_timeout (RTPSession * session, RTPSource * src, GstRtpSession * sess)
{
  g_signal_emit (sess, gst_rtp_session_signals[SIGNAL_ON_SENDER_TIMEOUT], 0,
      src->ssrc);
}

GST_BOILERPLATE (GstRtpSession, gst_rtp_session, GstElement, GST_TYPE_ELEMENT);

static void
gst_rtp_session_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  /* sink pads */
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&rtpsession_recv_rtp_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&rtpsession_recv_rtcp_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&rtpsession_send_rtp_sink_template));

  /* src pads */
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&rtpsession_recv_rtp_src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&rtpsession_sync_src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&rtpsession_send_rtp_src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&rtpsession_send_rtcp_src_template));

  gst_element_class_set_details (element_class, &rtpsession_details);
}

static void
gst_rtp_session_class_init (GstRtpSessionClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  g_type_class_add_private (klass, sizeof (GstRtpSessionPrivate));

  gobject_class->finalize = gst_rtp_session_finalize;
  gobject_class->set_property = gst_rtp_session_set_property;
  gobject_class->get_property = gst_rtp_session_get_property;

  /**
   * GstRtpSession::request-pt-map:
   * @sess: the object which received the signal
   * @pt: the pt
   *
   * Request the payload type as #GstCaps for @pt.
   */
  gst_rtp_session_signals[SIGNAL_REQUEST_PT_MAP] =
      g_signal_new ("request-pt-map", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstRtpSessionClass, request_pt_map),
      NULL, NULL, gst_rtp_bin_marshal_BOXED__UINT, GST_TYPE_CAPS, 1,
      G_TYPE_UINT);
  /**
   * GstRtpSession::clear-pt-map:
   * @sess: the object which received the signal
   *
   * Clear the cached pt-maps requested with #GstRtpSession::request-pt-map.
   */
  gst_rtp_session_signals[SIGNAL_CLEAR_PT_MAP] =
      g_signal_new ("clear-pt-map", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_ACTION, G_STRUCT_OFFSET (GstRtpSessionClass, clear_pt_map),
      NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

  /**
   * GstRtpSession::on-new-ssrc:
   * @sess: the object which received the signal
   * @ssrc: the SSRC 
   *
   * Notify of a new SSRC that entered @session.
   */
  gst_rtp_session_signals[SIGNAL_ON_NEW_SSRC] =
      g_signal_new ("on-new-ssrc", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstRtpSessionClass, on_new_ssrc),
      NULL, NULL, g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);
  /**
   * GstRtpSession::on-ssrc_collision:
   * @sess: the object which received the signal
   * @ssrc: the SSRC 
   *
   * Notify when we have an SSRC collision
   */
  gst_rtp_session_signals[SIGNAL_ON_SSRC_COLLISION] =
      g_signal_new ("on-ssrc-collision", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstRtpSessionClass,
          on_ssrc_collision), NULL, NULL, g_cclosure_marshal_VOID__UINT,
      G_TYPE_NONE, 1, G_TYPE_UINT);
  /**
   * GstRtpSession::on-ssrc_validated:
   * @sess: the object which received the signal
   * @ssrc: the SSRC 
   *
   * Notify of a new SSRC that became validated.
   */
  gst_rtp_session_signals[SIGNAL_ON_SSRC_VALIDATED] =
      g_signal_new ("on-ssrc-validated", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstRtpSessionClass,
          on_ssrc_validated), NULL, NULL, g_cclosure_marshal_VOID__UINT,
      G_TYPE_NONE, 1, G_TYPE_UINT);
  /**
   * GstRtpSession::on-ssrc_active:
   * @sess: the object which received the signal
   * @ssrc: the SSRC
   *
   * Notify of a SSRC that is active, i.e., sending RTCP.
   */
  gst_rtp_session_signals[SIGNAL_ON_SSRC_ACTIVE] =
      g_signal_new ("on-ssrc-active", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstRtpSessionClass,
          on_ssrc_active), NULL, NULL, g_cclosure_marshal_VOID__UINT,
      G_TYPE_NONE, 1, G_TYPE_UINT);
  /**
   * GstRtpSession::on-ssrc-sdes:
   * @session: the object which received the signal
   * @src: the SSRC
   *
   * Notify that a new SDES was received for SSRC.
   */
  gst_rtp_session_signals[SIGNAL_ON_SSRC_SDES] =
      g_signal_new ("on-ssrc-sdes", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstRtpSessionClass, on_ssrc_sdes),
      NULL, NULL, g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);

  /**
   * GstRtpSession::on-bye-ssrc:
   * @sess: the object which received the signal
   * @ssrc: the SSRC 
   *
   * Notify of an SSRC that became inactive because of a BYE packet.
   */
  gst_rtp_session_signals[SIGNAL_ON_BYE_SSRC] =
      g_signal_new ("on-bye-ssrc", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstRtpSessionClass, on_bye_ssrc),
      NULL, NULL, g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);
  /**
   * GstRtpSession::on-bye-timeout:
   * @sess: the object which received the signal
   * @ssrc: the SSRC 
   *
   * Notify of an SSRC that has timed out because of BYE
   */
  gst_rtp_session_signals[SIGNAL_ON_BYE_TIMEOUT] =
      g_signal_new ("on-bye-timeout", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstRtpSessionClass, on_bye_timeout),
      NULL, NULL, g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);
  /**
   * GstRtpSession::on-timeout:
   * @sess: the object which received the signal
   * @ssrc: the SSRC 
   *
   * Notify of an SSRC that has timed out
   */
  gst_rtp_session_signals[SIGNAL_ON_TIMEOUT] =
      g_signal_new ("on-timeout", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstRtpSessionClass, on_timeout),
      NULL, NULL, g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);
  /**
   * GstRtpSession::on-sender-timeout:
   * @sess: the object which received the signal
   * @ssrc: the SSRC 
   *
   * Notify of a sender SSRC that has timed out and became a receiver
   */
  gst_rtp_session_signals[SIGNAL_ON_SENDER_TIMEOUT] =
      g_signal_new ("on-sender-timeout", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstRtpSessionClass,
          on_sender_timeout), NULL, NULL, g_cclosure_marshal_VOID__UINT,
      G_TYPE_NONE, 1, G_TYPE_UINT);

  g_object_class_install_property (gobject_class, PROP_NTP_NS_BASE,
      g_param_spec_uint64 ("ntp-ns-base", "NTP base time",
          "The NTP base time corresponding to running_time 0", 0,
          G_MAXUINT64, DEFAULT_NTP_NS_BASE, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_BANDWIDTH,
      g_param_spec_double ("bandwidth", "Bandwidth",
          "The bandwidth of the session",
          0.0, G_MAXDOUBLE, DEFAULT_BANDWIDTH, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_RTCP_FRACTION,
      g_param_spec_double ("rtcp-fraction", "RTCP Fraction",
          "The fraction of the bandwidth used for RTCP",
          0.0, G_MAXDOUBLE, DEFAULT_RTCP_FRACTION, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_SDES_CNAME,
      g_param_spec_string ("sdes-cname", "SDES CNAME",
          "The CNAME to put in SDES messages of this session",
          DEFAULT_SDES_CNAME, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_SDES_NAME,
      g_param_spec_string ("sdes-name", "SDES NAME",
          "The NAME to put in SDES messages of this session",
          DEFAULT_SDES_NAME, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_SDES_EMAIL,
      g_param_spec_string ("sdes-email", "SDES EMAIL",
          "The EMAIL to put in SDES messages of this session",
          DEFAULT_SDES_EMAIL, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_SDES_PHONE,
      g_param_spec_string ("sdes-phone", "SDES PHONE",
          "The PHONE to put in SDES messages of this session",
          DEFAULT_SDES_PHONE, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_SDES_LOCATION,
      g_param_spec_string ("sdes-location", "SDES LOCATION",
          "The LOCATION to put in SDES messages of this session",
          DEFAULT_SDES_LOCATION, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_SDES_TOOL,
      g_param_spec_string ("sdes-tool", "SDES TOOL",
          "The TOOL to put in SDES messages of this session",
          DEFAULT_SDES_TOOL, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_SDES_NOTE,
      g_param_spec_string ("sdes-note", "SDES NOTE",
          "The NOTE to put in SDES messages of this session",
          DEFAULT_SDES_NOTE, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_NUM_SOURCES,
      g_param_spec_uint ("num-sources", "Num Sources",
          "The number of sources in the session", 0, G_MAXUINT,
          DEFAULT_NUM_SOURCES, G_PARAM_READABLE));

  g_object_class_install_property (gobject_class, PROP_NUM_ACTIVE_SOURCES,
      g_param_spec_uint ("num-active-sources", "Num Active Sources",
          "The number of active sources in the session", 0, G_MAXUINT,
          DEFAULT_NUM_ACTIVE_SOURCES, G_PARAM_READABLE));

  g_object_class_install_property (gobject_class, PROP_INTERNAL_SESSION,
      g_param_spec_object ("internal-session", "Internal Session",
          "The internal RTPSession object", RTP_TYPE_SESSION,
          G_PARAM_READABLE));

  gstelement_class->change_state =
      GST_DEBUG_FUNCPTR (gst_rtp_session_change_state);
  gstelement_class->request_new_pad =
      GST_DEBUG_FUNCPTR (gst_rtp_session_request_new_pad);
  gstelement_class->release_pad =
      GST_DEBUG_FUNCPTR (gst_rtp_session_release_pad);

  klass->clear_pt_map = GST_DEBUG_FUNCPTR (gst_rtp_session_clear_pt_map);

  GST_DEBUG_CATEGORY_INIT (gst_rtp_session_debug,
      "rtpsession", 0, "RTP Session");
}

static void
gst_rtp_session_init (GstRtpSession * rtpsession, GstRtpSessionClass * klass)
{
  rtpsession->priv = GST_RTP_SESSION_GET_PRIVATE (rtpsession);
  rtpsession->priv->lock = g_mutex_new ();
  rtpsession->priv->sysclock = gst_system_clock_obtain ();
  rtpsession->priv->session = rtp_session_new ();

  /* configure callbacks */
  rtp_session_set_callbacks (rtpsession->priv->session, &callbacks, rtpsession);
  /* configure signals */
  g_signal_connect (rtpsession->priv->session, "on-new-ssrc",
      (GCallback) on_new_ssrc, rtpsession);
  g_signal_connect (rtpsession->priv->session, "on-ssrc-collision",
      (GCallback) on_ssrc_collision, rtpsession);
  g_signal_connect (rtpsession->priv->session, "on-ssrc-validated",
      (GCallback) on_ssrc_validated, rtpsession);
  g_signal_connect (rtpsession->priv->session, "on-ssrc-active",
      (GCallback) on_ssrc_active, rtpsession);
  g_signal_connect (rtpsession->priv->session, "on-ssrc-sdes",
      (GCallback) on_ssrc_sdes, rtpsession);
  g_signal_connect (rtpsession->priv->session, "on-bye-ssrc",
      (GCallback) on_bye_ssrc, rtpsession);
  g_signal_connect (rtpsession->priv->session, "on-bye-timeout",
      (GCallback) on_bye_timeout, rtpsession);
  g_signal_connect (rtpsession->priv->session, "on-timeout",
      (GCallback) on_timeout, rtpsession);
  g_signal_connect (rtpsession->priv->session, "on-sender-timeout",
      (GCallback) on_sender_timeout, rtpsession);
  rtpsession->priv->ptmap = g_hash_table_new_full (NULL, NULL, NULL,
      (GDestroyNotify) gst_caps_unref);

  gst_segment_init (&rtpsession->recv_rtp_seg, GST_FORMAT_UNDEFINED);
  gst_segment_init (&rtpsession->send_rtp_seg, GST_FORMAT_UNDEFINED);

  rtpsession->priv->thread_stopped = TRUE;
}

static void
gst_rtp_session_finalize (GObject * object)
{
  GstRtpSession *rtpsession;

  rtpsession = GST_RTP_SESSION (object);

  if (rtpsession->recv_rtp_sink != NULL)
    gst_object_unref (rtpsession->recv_rtp_sink);
  if (rtpsession->recv_rtcp_sink != NULL)
    gst_object_unref (rtpsession->recv_rtcp_sink);
  if (rtpsession->send_rtp_sink != NULL)
    gst_object_unref (rtpsession->send_rtp_sink);
  if (rtpsession->send_rtcp_src != NULL)
    gst_object_unref (rtpsession->send_rtcp_src);

  g_hash_table_destroy (rtpsession->priv->ptmap);
  g_mutex_free (rtpsession->priv->lock);
  g_object_unref (rtpsession->priv->sysclock);
  g_object_unref (rtpsession->priv->session);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_rtp_session_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;

  rtpsession = GST_RTP_SESSION (object);
  priv = rtpsession->priv;

  switch (prop_id) {
    case PROP_NTP_NS_BASE:
      GST_OBJECT_LOCK (rtpsession);
      priv->ntpnsbase = g_value_get_uint64 (value);
      GST_DEBUG_OBJECT (rtpsession, "setting NTP base to %" GST_TIME_FORMAT,
          GST_TIME_ARGS (priv->ntpnsbase));
      GST_OBJECT_UNLOCK (rtpsession);
      break;
    case PROP_BANDWIDTH:
      rtp_session_set_bandwidth (priv->session, g_value_get_double (value));
      break;
    case PROP_RTCP_FRACTION:
      rtp_session_set_rtcp_fraction (priv->session, g_value_get_double (value));
      break;
    case PROP_SDES_CNAME:
      rtp_session_set_sdes_string (priv->session, GST_RTCP_SDES_CNAME,
          g_value_get_string (value));
      break;
    case PROP_SDES_NAME:
      rtp_session_set_sdes_string (priv->session, GST_RTCP_SDES_NAME,
          g_value_get_string (value));
      break;
    case PROP_SDES_EMAIL:
      rtp_session_set_sdes_string (priv->session, GST_RTCP_SDES_EMAIL,
          g_value_get_string (value));
      break;
    case PROP_SDES_PHONE:
      rtp_session_set_sdes_string (priv->session, GST_RTCP_SDES_PHONE,
          g_value_get_string (value));
      break;
    case PROP_SDES_LOCATION:
      rtp_session_set_sdes_string (priv->session, GST_RTCP_SDES_LOC,
          g_value_get_string (value));
      break;
    case PROP_SDES_TOOL:
      rtp_session_set_sdes_string (priv->session, GST_RTCP_SDES_TOOL,
          g_value_get_string (value));
      break;
    case PROP_SDES_NOTE:
      rtp_session_set_sdes_string (priv->session, GST_RTCP_SDES_NOTE,
          g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_rtp_session_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;

  rtpsession = GST_RTP_SESSION (object);
  priv = rtpsession->priv;

  switch (prop_id) {
    case PROP_NTP_NS_BASE:
      GST_OBJECT_LOCK (rtpsession);
      g_value_set_uint64 (value, priv->ntpnsbase);
      GST_OBJECT_UNLOCK (rtpsession);
      break;
    case PROP_BANDWIDTH:
      g_value_set_double (value, rtp_session_get_bandwidth (priv->session));
      break;
    case PROP_RTCP_FRACTION:
      g_value_set_double (value, rtp_session_get_rtcp_fraction (priv->session));
      break;
    case PROP_SDES_CNAME:
      g_value_take_string (value, rtp_session_get_sdes_string (priv->session,
              GST_RTCP_SDES_CNAME));
      break;
    case PROP_SDES_NAME:
      g_value_take_string (value, rtp_session_get_sdes_string (priv->session,
              GST_RTCP_SDES_NAME));
      break;
    case PROP_SDES_EMAIL:
      g_value_take_string (value, rtp_session_get_sdes_string (priv->session,
              GST_RTCP_SDES_EMAIL));
      break;
    case PROP_SDES_PHONE:
      g_value_take_string (value, rtp_session_get_sdes_string (priv->session,
              GST_RTCP_SDES_PHONE));
      break;
    case PROP_SDES_LOCATION:
      g_value_take_string (value, rtp_session_get_sdes_string (priv->session,
              GST_RTCP_SDES_LOC));
      break;
    case PROP_SDES_TOOL:
      g_value_take_string (value, rtp_session_get_sdes_string (priv->session,
              GST_RTCP_SDES_TOOL));
      break;
    case PROP_SDES_NOTE:
      g_value_take_string (value, rtp_session_get_sdes_string (priv->session,
              GST_RTCP_SDES_NOTE));
      break;
    case PROP_NUM_SOURCES:
      g_value_set_uint (value, rtp_session_get_num_sources (priv->session));
      break;
    case PROP_NUM_ACTIVE_SOURCES:
      g_value_set_uint (value,
          rtp_session_get_num_active_sources (priv->session));
      break;
    case PROP_INTERNAL_SESSION:
      g_value_set_object (value, priv->session);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
get_current_times (GstRtpSession * rtpsession,
    GstClockTime * running_time, guint64 * ntpnstime)
{
  guint64 ntpns;
  GstClock *clock;
  GstClockTime base_time, ntpnsbase, rt;

  GST_OBJECT_LOCK (rtpsession);
  if ((clock = GST_ELEMENT_CLOCK (rtpsession))) {
    base_time = GST_ELEMENT_CAST (rtpsession)->base_time;
    ntpnsbase = rtpsession->priv->ntpnsbase;
    gst_object_ref (clock);
    GST_OBJECT_UNLOCK (rtpsession);

    /* get current clock time and convert to running time */
    rt = gst_clock_get_time (clock) - base_time;
    /* add NTP base offset to get NTP ns time */
    ntpns = rt + ntpnsbase;

    gst_object_unref (clock);
  } else {
    GST_OBJECT_UNLOCK (rtpsession);
    rt = -1;
    ntpns = -1;
  }
  if (running_time)
    *running_time = rt;
  if (ntpnstime)
    *ntpnstime = ntpns;
}

static void
rtcp_thread (GstRtpSession * rtpsession)
{
  GstClockID id;
  GstClockTime current_time;
  GstClockTime next_timeout;
  guint64 ntpnstime;

  GST_DEBUG_OBJECT (rtpsession, "entering RTCP thread");

  GST_RTP_SESSION_LOCK (rtpsession);

  current_time = gst_clock_get_time (rtpsession->priv->sysclock);

  while (!rtpsession->priv->stop_thread) {
    GstClockReturn res;

    /* get initial estimate */
    next_timeout =
        rtp_session_next_timeout (rtpsession->priv->session, current_time);

    GST_DEBUG_OBJECT (rtpsession, "next check time %" GST_TIME_FORMAT,
        GST_TIME_ARGS (next_timeout));

    /* leave if no more timeouts, the session ended */
    if (next_timeout == GST_CLOCK_TIME_NONE)
      break;

    id = rtpsession->priv->id =
        gst_clock_new_single_shot_id (rtpsession->priv->sysclock, next_timeout);
    GST_RTP_SESSION_UNLOCK (rtpsession);

    res = gst_clock_id_wait (id, NULL);

    GST_RTP_SESSION_LOCK (rtpsession);
    gst_clock_id_unref (id);
    rtpsession->priv->id = NULL;

    if (rtpsession->priv->stop_thread)
      break;

    /* update current time */
    current_time = gst_clock_get_time (rtpsession->priv->sysclock);

    /* get current NTP time */
    get_current_times (rtpsession, NULL, &ntpnstime);

    /* we get unlocked because we need to perform reconsideration, don't perform
     * the timeout but get a new reporting estimate. */
    GST_DEBUG_OBJECT (rtpsession, "unlocked %d, current %" GST_TIME_FORMAT,
        res, GST_TIME_ARGS (current_time));

    /* perform actions, we ignore result. Release lock because it might push. */
    GST_RTP_SESSION_UNLOCK (rtpsession);
    rtp_session_on_timeout (rtpsession->priv->session, current_time, ntpnstime);
    GST_RTP_SESSION_LOCK (rtpsession);
  }
  /* mark the thread as stopped now */
  rtpsession->priv->thread_stopped = TRUE;
  GST_RTP_SESSION_UNLOCK (rtpsession);

  GST_DEBUG_OBJECT (rtpsession, "leaving RTCP thread");
}

static gboolean
start_rtcp_thread (GstRtpSession * rtpsession)
{
  GError *error = NULL;
  gboolean res;

  GST_DEBUG_OBJECT (rtpsession, "starting RTCP thread");

  GST_RTP_SESSION_LOCK (rtpsession);
  rtpsession->priv->stop_thread = FALSE;
  if (rtpsession->priv->thread_stopped) {
    /* only create a new thread if the old one was stopped. Otherwise we can
     * just reuse the currently running one. */
    rtpsession->priv->thread =
        g_thread_create ((GThreadFunc) rtcp_thread, rtpsession, TRUE, &error);
    rtpsession->priv->thread_stopped = FALSE;
  }
  GST_RTP_SESSION_UNLOCK (rtpsession);

  if (error != NULL) {
    res = FALSE;
    GST_DEBUG_OBJECT (rtpsession, "failed to start thread, %s", error->message);
    g_error_free (error);
  } else {
    res = TRUE;
  }
  return res;
}

static void
stop_rtcp_thread (GstRtpSession * rtpsession)
{
  GST_DEBUG_OBJECT (rtpsession, "stopping RTCP thread");

  GST_RTP_SESSION_LOCK (rtpsession);
  rtpsession->priv->stop_thread = TRUE;
  if (rtpsession->priv->id)
    gst_clock_id_unschedule (rtpsession->priv->id);
  GST_RTP_SESSION_UNLOCK (rtpsession);
}

static void
join_rtcp_thread (GstRtpSession * rtpsession)
{
  GST_RTP_SESSION_LOCK (rtpsession);
  /* don't try to join when we have no thread */
  if (rtpsession->priv->thread != NULL) {
    GST_DEBUG_OBJECT (rtpsession, "joining RTCP thread");
    GST_RTP_SESSION_UNLOCK (rtpsession);

    g_thread_join (rtpsession->priv->thread);

    GST_RTP_SESSION_LOCK (rtpsession);
    /* after the join, take the lock and clear the thread structure. The caller
     * is supposed to not concurrently call start and join. */
    rtpsession->priv->thread = NULL;
  }
  GST_RTP_SESSION_UNLOCK (rtpsession);
}

static GstStateChangeReturn
gst_rtp_session_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn res;
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;

  rtpsession = GST_RTP_SESSION (element);
  priv = rtpsession->priv;

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
      break;
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      /* no need to join yet, we might want to continue later. Also, the
       * dataflow could block downstream so that a join could just block
       * forever. */
      stop_rtcp_thread (rtpsession);
      break;
    default:
      break;
  }

  res = parent_class->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
      if (!start_rtcp_thread (rtpsession))
        goto failed_thread;
      break;
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      /* downstream is now releasing the dataflow and we can join. */
      join_rtcp_thread (rtpsession);
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      break;
    default:
      break;
  }
  return res;

  /* ERRORS */
failed_thread:
  {
    return GST_STATE_CHANGE_FAILURE;
  }
}

static gboolean
return_true (gpointer key, gpointer value, gpointer user_data)
{
  return TRUE;
}

static void
gst_rtp_session_clear_pt_map (GstRtpSession * rtpsession)
{
  g_hash_table_foreach_remove (rtpsession->priv->ptmap, return_true, NULL);
}

/* called when the session manager has an RTP packet ready for further
 * processing */
static GstFlowReturn
gst_rtp_session_process_rtp (RTPSession * sess, RTPSource * src,
    GstBuffer * buffer, gpointer user_data)
{
  GstFlowReturn result;
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;

  rtpsession = GST_RTP_SESSION (user_data);
  priv = rtpsession->priv;

  if (rtpsession->recv_rtp_src) {
    GST_LOG_OBJECT (rtpsession, "pushing received RTP packet");
    result = gst_pad_push (rtpsession->recv_rtp_src, buffer);
  } else {
    GST_DEBUG_OBJECT (rtpsession, "dropping received RTP packet");
    gst_buffer_unref (buffer);
    result = GST_FLOW_OK;
  }
  return result;
}

/* called when the session manager has an RTP packet ready for further
 * sending */
static GstFlowReturn
gst_rtp_session_send_rtp (RTPSession * sess, RTPSource * src,
    GstBuffer * buffer, gpointer user_data)
{
  GstFlowReturn result;
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;

  rtpsession = GST_RTP_SESSION (user_data);
  priv = rtpsession->priv;

  GST_LOG_OBJECT (rtpsession, "sending RTP packet");

  if (rtpsession->send_rtp_src) {
    result = gst_pad_push (rtpsession->send_rtp_src, buffer);
  } else {
    gst_buffer_unref (buffer);
    result = GST_FLOW_OK;
  }
  return result;
}

/* called when the session manager has an RTCP packet ready for further
 * sending. The eos flag is set when an EOS event should be sent downstream as
 * well. */
static GstFlowReturn
gst_rtp_session_send_rtcp (RTPSession * sess, RTPSource * src,
    GstBuffer * buffer, gboolean eos, gpointer user_data)
{
  GstFlowReturn result;
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;

  rtpsession = GST_RTP_SESSION (user_data);
  priv = rtpsession->priv;

  if (rtpsession->send_rtcp_src) {
    GstCaps *caps;

    /* set rtcp caps on output pad */
    if (!(caps = GST_PAD_CAPS (rtpsession->send_rtcp_src))) {
      caps = gst_caps_new_simple ("application/x-rtcp", NULL);
      gst_pad_set_caps (rtpsession->send_rtcp_src, caps);
      gst_caps_unref (caps);
    }
    gst_buffer_set_caps (buffer, caps);
    GST_LOG_OBJECT (rtpsession, "sending RTCP");
    result = gst_pad_push (rtpsession->send_rtcp_src, buffer);

    /* we have to send EOS after this packet */
    if (eos) {
      GST_LOG_OBJECT (rtpsession, "sending EOS");
      gst_pad_push_event (rtpsession->send_rtcp_src, gst_event_new_eos ());
    }
  } else {
    GST_DEBUG_OBJECT (rtpsession, "not sending RTCP, no output pad");
    gst_buffer_unref (buffer);
    result = GST_FLOW_OK;
  }
  return result;
}

/* called when the session manager has an SR RTCP packet ready for handling
 * inter stream synchronisation */
static GstFlowReturn
gst_rtp_session_sync_rtcp (RTPSession * sess,
    RTPSource * src, GstBuffer * buffer, gpointer user_data)
{
  GstFlowReturn result;
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;

  rtpsession = GST_RTP_SESSION (user_data);
  priv = rtpsession->priv;

  if (rtpsession->sync_src) {
    GstCaps *caps;

    /* set rtcp caps on output pad */
    if (!(caps = GST_PAD_CAPS (rtpsession->sync_src))) {
      caps = gst_caps_new_simple ("application/x-rtcp", NULL);
      gst_pad_set_caps (rtpsession->sync_src, caps);
      gst_caps_unref (caps);
    }
    gst_buffer_set_caps (buffer, caps);
    GST_LOG_OBJECT (rtpsession, "sending Sync RTCP");
    result = gst_pad_push (rtpsession->sync_src, buffer);
  } else {
    GST_DEBUG_OBJECT (rtpsession, "not sending Sync RTCP, no output pad");
    gst_buffer_unref (buffer);
    result = GST_FLOW_OK;
  }
  return result;
}

static void
gst_rtp_session_cache_caps (GstRtpSession * rtpsession, GstCaps * caps)
{
  GstRtpSessionPrivate *priv;
  const GstStructure *s;
  gint payload;

  priv = rtpsession->priv;

  GST_DEBUG_OBJECT (rtpsession, "parsing caps");

  s = gst_caps_get_structure (caps, 0);
  if (!gst_structure_get_int (s, "payload", &payload))
    return;

  if (g_hash_table_lookup (priv->ptmap, GINT_TO_POINTER (payload)))
    return;

  g_hash_table_insert (priv->ptmap, GINT_TO_POINTER (payload),
      gst_caps_ref (caps));
}

/* called when the session manager needs the clock rate */
static gint
gst_rtp_session_clock_rate (RTPSession * sess, guint8 payload,
    gpointer user_data)
{
  gint ipayload, result = -1;
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;
  GValue ret = { 0 };
  GValue args[2] = { {0}, {0} };
  GstCaps *caps;
  const GstStructure *s;

  rtpsession = GST_RTP_SESSION_CAST (user_data);
  priv = rtpsession->priv;

  GST_RTP_SESSION_LOCK (rtpsession);
  ipayload = payload;           /* make compiler happy */
  caps = g_hash_table_lookup (priv->ptmap, GINT_TO_POINTER (ipayload));
  if (caps) {
    gst_caps_ref (caps);
    goto found;
  }

  /* not found in the cache, try to get it with a signal */
  g_value_init (&args[0], GST_TYPE_ELEMENT);
  g_value_set_object (&args[0], rtpsession);
  g_value_init (&args[1], G_TYPE_UINT);
  g_value_set_uint (&args[1], payload);

  g_value_init (&ret, GST_TYPE_CAPS);
  g_value_set_boxed (&ret, NULL);

  g_signal_emitv (args, gst_rtp_session_signals[SIGNAL_REQUEST_PT_MAP], 0,
      &ret);

  g_value_unset (&args[0]);
  g_value_unset (&args[1]);
  caps = (GstCaps *) g_value_dup_boxed (&ret);
  g_value_unset (&ret);
  if (!caps)
    goto no_caps;

  gst_rtp_session_cache_caps (rtpsession, caps);

found:
  s = gst_caps_get_structure (caps, 0);
  if (!gst_structure_get_int (s, "clock-rate", &result))
    goto no_clock_rate;

  gst_caps_unref (caps);

  GST_DEBUG_OBJECT (rtpsession, "parsed clock-rate %d", result);

done:
  GST_RTP_SESSION_UNLOCK (rtpsession);

  return result;

  /* ERRORS */
no_caps:
  {
    GST_DEBUG_OBJECT (rtpsession, "could not get caps");
    goto done;
  }
no_clock_rate:
  {
    gst_caps_unref (caps);
    GST_DEBUG_OBJECT (rtpsession, "No clock-rate in caps!");
    goto done;
  }
}

/* called when the session manager asks us to reconsider the timeout */
static void
gst_rtp_session_reconsider (RTPSession * sess, gpointer user_data)
{
  GstRtpSession *rtpsession;

  rtpsession = GST_RTP_SESSION_CAST (user_data);

  GST_RTP_SESSION_LOCK (rtpsession);
  GST_DEBUG_OBJECT (rtpsession, "unlock timer for reconsideration");
  if (rtpsession->priv->id)
    gst_clock_id_unschedule (rtpsession->priv->id);
  GST_RTP_SESSION_UNLOCK (rtpsession);
}

static gboolean
gst_rtp_session_event_recv_rtp_sink (GstPad * pad, GstEvent * event)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;
  gboolean ret = FALSE;

  rtpsession = GST_RTP_SESSION (gst_pad_get_parent (pad));
  priv = rtpsession->priv;

  GST_DEBUG_OBJECT (rtpsession, "received event %s",
      GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_FLUSH_STOP:
      gst_segment_init (&rtpsession->recv_rtp_seg, GST_FORMAT_UNDEFINED);
      ret = gst_pad_push_event (rtpsession->recv_rtp_src, event);
      break;
    case GST_EVENT_NEWSEGMENT:
    {
      gboolean update;
      gdouble rate, arate;
      GstFormat format;
      gint64 start, stop, time;
      GstSegment *segment;

      segment = &rtpsession->recv_rtp_seg;

      /* the newsegment event is needed to convert the RTP timestamp to
       * running_time, which is needed to generate a mapping from RTP to NTP
       * timestamps in SR reports */
      gst_event_parse_new_segment_full (event, &update, &rate, &arate, &format,
          &start, &stop, &time);

      GST_DEBUG_OBJECT (rtpsession,
          "configured NEWSEGMENT update %d, rate %lf, applied rate %lf, "
          "format GST_FORMAT_TIME, "
          "%" GST_TIME_FORMAT " -- %" GST_TIME_FORMAT
          ", time %" GST_TIME_FORMAT ", accum %" GST_TIME_FORMAT,
          update, rate, arate, GST_TIME_ARGS (segment->start),
          GST_TIME_ARGS (segment->stop), GST_TIME_ARGS (segment->time),
          GST_TIME_ARGS (segment->accum));

      gst_segment_set_newsegment_full (segment, update, rate,
          arate, format, start, stop, time);

      /* push event forward */
      ret = gst_pad_push_event (rtpsession->recv_rtp_src, event);
      break;
    }
    default:
      ret = gst_pad_push_event (rtpsession->recv_rtp_src, event);
      break;
  }
  gst_object_unref (rtpsession);

  return ret;

}

static GList *
gst_rtp_session_internal_links (GstPad * pad)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;
  GList *res = NULL;

  rtpsession = GST_RTP_SESSION (gst_pad_get_parent (pad));
  priv = rtpsession->priv;

  if (pad == rtpsession->recv_rtp_src) {
    res = g_list_prepend (res, rtpsession->recv_rtp_sink);
  } else if (pad == rtpsession->recv_rtp_sink) {
    res = g_list_prepend (res, rtpsession->recv_rtp_src);
  } else if (pad == rtpsession->send_rtp_src) {
    res = g_list_prepend (res, rtpsession->send_rtp_sink);
  } else if (pad == rtpsession->send_rtp_sink) {
    res = g_list_prepend (res, rtpsession->send_rtp_src);
  }

  gst_object_unref (rtpsession);

  return res;
}

static gboolean
gst_rtp_session_sink_setcaps (GstPad * pad, GstCaps * caps)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;

  rtpsession = GST_RTP_SESSION (gst_pad_get_parent (pad));
  priv = rtpsession->priv;

  GST_RTP_SESSION_LOCK (rtpsession);
  gst_rtp_session_cache_caps (rtpsession, caps);
  GST_RTP_SESSION_UNLOCK (rtpsession);

  gst_object_unref (rtpsession);

  return TRUE;
}

/* receive a packet from a sender, send it to the RTP session manager and
 * forward the packet on the rtp_src pad
 */
static GstFlowReturn
gst_rtp_session_chain_recv_rtp (GstPad * pad, GstBuffer * buffer)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;
  GstFlowReturn ret;
  GstClockTime current_time, running_time;
  guint64 ntpnstime;
  GstClockTime timestamp;

  rtpsession = GST_RTP_SESSION (gst_pad_get_parent (pad));
  priv = rtpsession->priv;

  GST_LOG_OBJECT (rtpsession, "received RTP packet");

  /* get NTP time when this packet was captured, this depends on the timestamp. */
  timestamp = GST_BUFFER_TIMESTAMP (buffer);
  if (GST_CLOCK_TIME_IS_VALID (timestamp)) {
    /* convert to running time using the segment values */
    running_time =
        gst_segment_to_running_time (&rtpsession->recv_rtp_seg, GST_FORMAT_TIME,
        timestamp);
    /* add constant to convert running time to NTP time */
    ntpnstime = running_time + priv->ntpnsbase;
  } else {
    get_current_times (rtpsession, &running_time, &ntpnstime);
  }
  current_time = gst_clock_get_time (priv->sysclock);

  ret = rtp_session_process_rtp (priv->session, buffer, current_time,
      running_time, ntpnstime);
  if (ret != GST_FLOW_OK)
    goto push_error;

done:
  gst_object_unref (rtpsession);

  return ret;

  /* ERRORS */
push_error:
  {
    GST_DEBUG_OBJECT (rtpsession, "process returned %s",
        gst_flow_get_name (ret));
    goto done;
  }
}

static gboolean
gst_rtp_session_event_recv_rtcp_sink (GstPad * pad, GstEvent * event)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;
  gboolean ret = FALSE;

  rtpsession = GST_RTP_SESSION (gst_pad_get_parent (pad));
  priv = rtpsession->priv;

  GST_DEBUG_OBJECT (rtpsession, "received event %s",
      GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    default:
      ret = gst_pad_push_event (rtpsession->sync_src, event);
      break;
  }
  gst_object_unref (rtpsession);

  return ret;
}

/* Receive an RTCP packet from a sender, send it to the RTP session manager and
 * forward the SR packets to the sync_src pad.
 */
static GstFlowReturn
gst_rtp_session_chain_recv_rtcp (GstPad * pad, GstBuffer * buffer)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;
  GstClockTime current_time;
  GstFlowReturn ret;

  rtpsession = GST_RTP_SESSION (gst_pad_get_parent (pad));
  priv = rtpsession->priv;

  GST_LOG_OBJECT (rtpsession, "received RTCP packet");

  current_time = gst_clock_get_time (priv->sysclock);
  ret = rtp_session_process_rtcp (priv->session, buffer, current_time);

  gst_object_unref (rtpsession);

  return GST_FLOW_OK;
}

static gboolean
gst_rtp_session_query_send_rtcp_src (GstPad * pad, GstQuery * query)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;
  gboolean ret = FALSE;

  rtpsession = GST_RTP_SESSION (gst_pad_get_parent (pad));
  priv = rtpsession->priv;

  GST_DEBUG_OBJECT (rtpsession, "received QUERY");

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_LATENCY:
      ret = TRUE;
      /* use the defaults for the latency query. */
      gst_query_set_latency (query, FALSE, 0, -1);
      break;
    default:
      /* other queries simply fail for now */
      break;
  }

  gst_object_unref (rtpsession);

  return ret;
}

static gboolean
gst_rtp_session_event_send_rtcp_src (GstPad * pad, GstEvent * event)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;
  gboolean ret;

  rtpsession = GST_RTP_SESSION (gst_pad_get_parent (pad));
  priv = rtpsession->priv;

  GST_DEBUG_OBJECT (rtpsession, "received EVENT");

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_SEEK:
    case GST_EVENT_LATENCY:
      gst_event_unref (event);
      ret = TRUE;
      break;
    default:
      /* other events simply fail for now */
      gst_event_unref (event);
      ret = FALSE;
      break;
  }

  gst_object_unref (rtpsession);

  return ret;
}


static gboolean
gst_rtp_session_event_send_rtp_sink (GstPad * pad, GstEvent * event)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;
  gboolean ret = FALSE;

  rtpsession = GST_RTP_SESSION (gst_pad_get_parent (pad));
  priv = rtpsession->priv;

  GST_DEBUG_OBJECT (rtpsession, "received event");

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_FLUSH_STOP:
      gst_segment_init (&rtpsession->send_rtp_seg, GST_FORMAT_UNDEFINED);
      ret = gst_pad_push_event (rtpsession->send_rtp_src, event);
      break;
    case GST_EVENT_NEWSEGMENT:{
      gboolean update;
      gdouble rate, arate;
      GstFormat format;
      gint64 start, stop, time;
      GstSegment *segment;

      segment = &rtpsession->send_rtp_seg;

      /* the newsegment event is needed to convert the RTP timestamp to
       * running_time, which is needed to generate a mapping from RTP to NTP
       * timestamps in SR reports */
      gst_event_parse_new_segment_full (event, &update, &rate, &arate, &format,
          &start, &stop, &time);

      GST_DEBUG_OBJECT (rtpsession,
          "configured NEWSEGMENT update %d, rate %lf, applied rate %lf, "
          "format GST_FORMAT_TIME, "
          "%" GST_TIME_FORMAT " -- %" GST_TIME_FORMAT
          ", time %" GST_TIME_FORMAT ", accum %" GST_TIME_FORMAT,
          update, rate, arate, GST_TIME_ARGS (segment->start),
          GST_TIME_ARGS (segment->stop), GST_TIME_ARGS (segment->time),
          GST_TIME_ARGS (segment->accum));

      gst_segment_set_newsegment_full (segment, update, rate,
          arate, format, start, stop, time);

      /* push event forward */
      ret = gst_pad_push_event (rtpsession->send_rtp_src, event);
      break;
    }
    case GST_EVENT_EOS:{
      GstClockTime current_time;

      /* push downstream FIXME, we are not supposed to leave the session just
       * because we stop sending. */
      ret = gst_pad_push_event (rtpsession->send_rtp_src, event);
      current_time = gst_clock_get_time (rtpsession->priv->sysclock);
      GST_DEBUG_OBJECT (rtpsession, "scheduling BYE message");
      rtp_session_schedule_bye (rtpsession->priv->session, "End of stream",
          current_time);
      break;
    }
    default:
      ret = gst_pad_push_event (rtpsession->send_rtp_src, event);
      break;
  }
  gst_object_unref (rtpsession);

  return ret;
}

static GstCaps *
gst_rtp_session_getcaps_send_rtp (GstPad * pad)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;
  GstCaps *result;
  GstStructure *s1, *s2;
  guint ssrc;

  rtpsession = GST_RTP_SESSION (gst_pad_get_parent (pad));
  priv = rtpsession->priv;

  ssrc = rtp_session_get_internal_ssrc (priv->session);

  /* we can basically accept anything but we prefer to receive packets with our
   * internal SSRC so that we don't have to patch it. Create a structure with
   * the SSRC and another one without. */
  s1 = gst_structure_new ("application/x-rtp", "ssrc", G_TYPE_UINT, ssrc, NULL);
  s2 = gst_structure_new ("application/x-rtp", NULL);

  result = gst_caps_new_full (s1, s2, NULL);

  GST_DEBUG_OBJECT (rtpsession, "getting caps %" GST_PTR_FORMAT, result);

  gst_object_unref (rtpsession);

  return result;
}

static gboolean
gst_rtp_session_setcaps_send_rtp (GstPad * pad, GstCaps * caps)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;
  GstStructure *s = gst_caps_get_structure (caps, 0);
  guint ssrc;

  rtpsession = GST_RTP_SESSION (gst_pad_get_parent (pad));
  priv = rtpsession->priv;

  if (gst_structure_get_uint (s, "ssrc", &ssrc)) {
    GST_DEBUG_OBJECT (rtpsession, "setting internal SSRC to %08x", ssrc);
    rtp_session_set_internal_ssrc (priv->session, ssrc);
  }

  gst_object_unref (rtpsession);

  return TRUE;
}

/* Recieve an RTP packet to be send to the receivers, send to RTP session
 * manager and forward to send_rtp_src.
 */
static GstFlowReturn
gst_rtp_session_chain_send_rtp (GstPad * pad, GstBuffer * buffer)
{
  GstRtpSession *rtpsession;
  GstRtpSessionPrivate *priv;
  GstFlowReturn ret;
  GstClockTime timestamp;
  GstClockTime current_time;
  guint64 ntpnstime;

  rtpsession = GST_RTP_SESSION (gst_pad_get_parent (pad));
  priv = rtpsession->priv;

  GST_LOG_OBJECT (rtpsession, "received RTP packet");

  /* get NTP time when this packet was captured, this depends on the timestamp. */
  timestamp = GST_BUFFER_TIMESTAMP (buffer);
  if (GST_CLOCK_TIME_IS_VALID (timestamp)) {
    /* convert to running time using the segment start value. */
    ntpnstime =
        gst_segment_to_running_time (&rtpsession->send_rtp_seg, GST_FORMAT_TIME,
        timestamp);
    /* convert to NTP time by adding the NTP base */
    ntpnstime += priv->ntpnsbase;
  } else {
    /* no timestamp, we could take the current running_time and convert it to
     * NTP time. */
    ntpnstime = -1;
  }

  current_time = gst_clock_get_time (priv->sysclock);
  ret = rtp_session_send_rtp (priv->session, buffer, current_time, ntpnstime);
  if (ret != GST_FLOW_OK)
    goto push_error;

done:
  gst_object_unref (rtpsession);

  return ret;

  /* ERRORS */
push_error:
  {
    GST_DEBUG_OBJECT (rtpsession, "process returned %s",
        gst_flow_get_name (ret));
    goto done;
  }
}

/* Create sinkpad to receive RTP packets from senders. This will also create a
 * srcpad for the RTP packets.
 */
static GstPad *
create_recv_rtp_sink (GstRtpSession * rtpsession)
{
  GST_DEBUG_OBJECT (rtpsession, "creating RTP sink pad");

  rtpsession->recv_rtp_sink =
      gst_pad_new_from_static_template (&rtpsession_recv_rtp_sink_template,
      "recv_rtp_sink");
  gst_pad_set_chain_function (rtpsession->recv_rtp_sink,
      gst_rtp_session_chain_recv_rtp);
  gst_pad_set_event_function (rtpsession->recv_rtp_sink,
      (GstPadEventFunction) gst_rtp_session_event_recv_rtp_sink);
  gst_pad_set_setcaps_function (rtpsession->recv_rtp_sink,
      gst_rtp_session_sink_setcaps);
  gst_pad_set_internal_link_function (rtpsession->recv_rtp_sink,
      gst_rtp_session_internal_links);
  gst_pad_set_active (rtpsession->recv_rtp_sink, TRUE);
  gst_element_add_pad (GST_ELEMENT_CAST (rtpsession),
      rtpsession->recv_rtp_sink);

  GST_DEBUG_OBJECT (rtpsession, "creating RTP src pad");
  rtpsession->recv_rtp_src =
      gst_pad_new_from_static_template (&rtpsession_recv_rtp_src_template,
      "recv_rtp_src");
  gst_pad_set_internal_link_function (rtpsession->recv_rtp_src,
      gst_rtp_session_internal_links);
  gst_pad_use_fixed_caps (rtpsession->recv_rtp_src);
  gst_pad_set_active (rtpsession->recv_rtp_src, TRUE);
  gst_element_add_pad (GST_ELEMENT_CAST (rtpsession), rtpsession->recv_rtp_src);

  return rtpsession->recv_rtp_sink;
}

/* Remove sinkpad to receive RTP packets from senders. This will also remove
 * the srcpad for the RTP packets.
 */
static void
remove_recv_rtp_sink (GstRtpSession * rtpsession)
{
  GST_DEBUG_OBJECT (rtpsession, "removing RTP sink pad");

  /* deactivate from source to sink */
  gst_pad_set_active (rtpsession->recv_rtp_src, FALSE);
  gst_pad_set_active (rtpsession->recv_rtp_sink, FALSE);

  /* remove pads */
  gst_element_remove_pad (GST_ELEMENT_CAST (rtpsession),
      rtpsession->recv_rtp_sink);
  rtpsession->recv_rtp_sink = NULL;

  GST_DEBUG_OBJECT (rtpsession, "removing RTP src pad");
  gst_element_remove_pad (GST_ELEMENT_CAST (rtpsession),
      rtpsession->recv_rtp_src);
  rtpsession->recv_rtp_src = NULL;
}

/* Create a sinkpad to receive RTCP messages from senders, this will also create a
 * sync_src pad for the SR packets.
 */
static GstPad *
create_recv_rtcp_sink (GstRtpSession * rtpsession)
{
  GST_DEBUG_OBJECT (rtpsession, "creating RTCP sink pad");

  rtpsession->recv_rtcp_sink =
      gst_pad_new_from_static_template (&rtpsession_recv_rtcp_sink_template,
      "recv_rtcp_sink");
  gst_pad_set_chain_function (rtpsession->recv_rtcp_sink,
      gst_rtp_session_chain_recv_rtcp);
  gst_pad_set_event_function (rtpsession->recv_rtcp_sink,
      (GstPadEventFunction) gst_rtp_session_event_recv_rtcp_sink);
  gst_pad_set_internal_link_function (rtpsession->recv_rtcp_sink,
      gst_rtp_session_internal_links);
  gst_pad_set_active (rtpsession->recv_rtcp_sink, TRUE);
  gst_element_add_pad (GST_ELEMENT_CAST (rtpsession),
      rtpsession->recv_rtcp_sink);

  GST_DEBUG_OBJECT (rtpsession, "creating sync src pad");
  rtpsession->sync_src =
      gst_pad_new_from_static_template (&rtpsession_sync_src_template,
      "sync_src");
  gst_pad_set_internal_link_function (rtpsession->sync_src,
      gst_rtp_session_internal_links);
  gst_pad_use_fixed_caps (rtpsession->sync_src);
  gst_pad_set_active (rtpsession->sync_src, TRUE);
  gst_element_add_pad (GST_ELEMENT_CAST (rtpsession), rtpsession->sync_src);

  return rtpsession->recv_rtcp_sink;
}

static void
remove_recv_rtcp_sink (GstRtpSession * rtpsession)
{
  GST_DEBUG_OBJECT (rtpsession, "removing RTCP sink pad");

  gst_pad_set_active (rtpsession->sync_src, FALSE);
  gst_pad_set_active (rtpsession->recv_rtcp_sink, FALSE);

  gst_element_remove_pad (GST_ELEMENT_CAST (rtpsession),
      rtpsession->recv_rtcp_sink);
  rtpsession->recv_rtcp_sink = NULL;

  GST_DEBUG_OBJECT (rtpsession, "removing sync src pad");
  gst_element_remove_pad (GST_ELEMENT_CAST (rtpsession), rtpsession->sync_src);
  rtpsession->sync_src = NULL;
}

/* Create a sinkpad to receive RTP packets for receivers. This will also create a
 * send_rtp_src pad.
 */
static GstPad *
create_send_rtp_sink (GstRtpSession * rtpsession)
{
  GST_DEBUG_OBJECT (rtpsession, "creating pad");

  rtpsession->send_rtp_sink =
      gst_pad_new_from_static_template (&rtpsession_send_rtp_sink_template,
      "send_rtp_sink");
  gst_pad_set_chain_function (rtpsession->send_rtp_sink,
      gst_rtp_session_chain_send_rtp);
  gst_pad_set_getcaps_function (rtpsession->send_rtp_sink,
      gst_rtp_session_getcaps_send_rtp);
  gst_pad_set_setcaps_function (rtpsession->send_rtp_sink,
      gst_rtp_session_setcaps_send_rtp);
  gst_pad_set_event_function (rtpsession->send_rtp_sink,
      (GstPadEventFunction) gst_rtp_session_event_send_rtp_sink);
  gst_pad_set_internal_link_function (rtpsession->send_rtp_sink,
      gst_rtp_session_internal_links);
  gst_pad_set_active (rtpsession->send_rtp_sink, TRUE);
  gst_element_add_pad (GST_ELEMENT_CAST (rtpsession),
      rtpsession->send_rtp_sink);

  rtpsession->send_rtp_src =
      gst_pad_new_from_static_template (&rtpsession_send_rtp_src_template,
      "send_rtp_src");
  gst_pad_set_internal_link_function (rtpsession->send_rtp_src,
      gst_rtp_session_internal_links);
  gst_pad_set_active (rtpsession->send_rtp_src, TRUE);
  gst_element_add_pad (GST_ELEMENT_CAST (rtpsession), rtpsession->send_rtp_src);

  return rtpsession->send_rtp_sink;
}

static void
remove_send_rtp_sink (GstRtpSession * rtpsession)
{
  GST_DEBUG_OBJECT (rtpsession, "removing pad");

  gst_pad_set_active (rtpsession->send_rtp_src, FALSE);
  gst_pad_set_active (rtpsession->send_rtp_sink, FALSE);

  gst_element_remove_pad (GST_ELEMENT_CAST (rtpsession),
      rtpsession->send_rtp_sink);
  rtpsession->send_rtp_sink = NULL;

  gst_element_remove_pad (GST_ELEMENT_CAST (rtpsession),
      rtpsession->send_rtp_src);
  rtpsession->send_rtp_src = NULL;
}

/* Create a srcpad with the RTCP packets to send out.
 * This pad will be driven by the RTP session manager when it wants to send out
 * RTCP packets.
 */
static GstPad *
create_send_rtcp_src (GstRtpSession * rtpsession)
{
  GST_DEBUG_OBJECT (rtpsession, "creating pad");

  rtpsession->send_rtcp_src =
      gst_pad_new_from_static_template (&rtpsession_send_rtcp_src_template,
      "send_rtcp_src");
  gst_pad_use_fixed_caps (rtpsession->send_rtcp_src);
  gst_pad_set_active (rtpsession->send_rtcp_src, TRUE);
  gst_pad_set_internal_link_function (rtpsession->send_rtcp_src,
      gst_rtp_session_internal_links);
  gst_pad_set_query_function (rtpsession->send_rtcp_src,
      gst_rtp_session_query_send_rtcp_src);
  gst_pad_set_event_function (rtpsession->send_rtcp_src,
      gst_rtp_session_event_send_rtcp_src);
  gst_element_add_pad (GST_ELEMENT_CAST (rtpsession),
      rtpsession->send_rtcp_src);

  return rtpsession->send_rtcp_src;
}

static void
remove_send_rtcp_src (GstRtpSession * rtpsession)
{
  GST_DEBUG_OBJECT (rtpsession, "removing pad");

  gst_pad_set_active (rtpsession->send_rtcp_src, FALSE);

  gst_element_remove_pad (GST_ELEMENT_CAST (rtpsession),
      rtpsession->send_rtcp_src);
  rtpsession->send_rtcp_src = NULL;
}

static GstPad *
gst_rtp_session_request_new_pad (GstElement * element,
    GstPadTemplate * templ, const gchar * name)
{
  GstRtpSession *rtpsession;
  GstElementClass *klass;
  GstPad *result;

  g_return_val_if_fail (templ != NULL, NULL);
  g_return_val_if_fail (GST_IS_RTP_SESSION (element), NULL);

  rtpsession = GST_RTP_SESSION (element);
  klass = GST_ELEMENT_GET_CLASS (element);

  GST_DEBUG_OBJECT (element, "requesting pad %s", GST_STR_NULL (name));

  GST_RTP_SESSION_LOCK (rtpsession);

  /* figure out the template */
  if (templ == gst_element_class_get_pad_template (klass, "recv_rtp_sink")) {
    if (rtpsession->recv_rtp_sink != NULL)
      goto exists;

    result = create_recv_rtp_sink (rtpsession);
  } else if (templ == gst_element_class_get_pad_template (klass,
          "recv_rtcp_sink")) {
    if (rtpsession->recv_rtcp_sink != NULL)
      goto exists;

    result = create_recv_rtcp_sink (rtpsession);
  } else if (templ == gst_element_class_get_pad_template (klass,
          "send_rtp_sink")) {
    if (rtpsession->send_rtp_sink != NULL)
      goto exists;

    result = create_send_rtp_sink (rtpsession);
  } else if (templ == gst_element_class_get_pad_template (klass,
          "send_rtcp_src")) {
    if (rtpsession->send_rtcp_src != NULL)
      goto exists;

    result = create_send_rtcp_src (rtpsession);
  } else
    goto wrong_template;

  GST_RTP_SESSION_UNLOCK (rtpsession);

  return result;

  /* ERRORS */
wrong_template:
  {
    GST_RTP_SESSION_UNLOCK (rtpsession);
    g_warning ("gstrtpsession: this is not our template");
    return NULL;
  }
exists:
  {
    GST_RTP_SESSION_UNLOCK (rtpsession);
    g_warning ("gstrtpsession: pad already requested");
    return NULL;
  }
}

static void
gst_rtp_session_release_pad (GstElement * element, GstPad * pad)
{
  GstRtpSession *rtpsession;

  g_return_if_fail (GST_IS_RTP_SESSION (element));
  g_return_if_fail (GST_IS_PAD (pad));

  rtpsession = GST_RTP_SESSION (element);

  GST_DEBUG_OBJECT (element, "releasing pad %s:%s", GST_DEBUG_PAD_NAME (pad));

  GST_RTP_SESSION_LOCK (rtpsession);

  if (rtpsession->recv_rtp_sink == pad) {
    remove_recv_rtp_sink (rtpsession);
  } else if (rtpsession->recv_rtcp_sink == pad) {
    remove_recv_rtcp_sink (rtpsession);
  } else if (rtpsession->send_rtp_sink == pad) {
    remove_send_rtp_sink (rtpsession);
  } else if (rtpsession->send_rtcp_src == pad) {
    remove_send_rtcp_src (rtpsession);
  } else
    goto wrong_pad;

  GST_RTP_SESSION_UNLOCK (rtpsession);

  return;

  /* ERRORS */
wrong_pad:
  {
    GST_RTP_SESSION_UNLOCK (rtpsession);
    g_warning ("gstrtpsession: asked to release an unknown pad");
    return;
  }
}
