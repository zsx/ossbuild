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

#include <string.h>

#include <gst/rtp/gstrtpbuffer.h>
#include <gst/rtp/gstrtcpbuffer.h>
#include <gst/netbuffer/gstnetbuffer.h>

#include "gstrtpbin-marshal.h"
#include "rtpsession.h"

GST_DEBUG_CATEGORY_STATIC (rtp_session_debug);
#define GST_CAT_DEFAULT rtp_session_debug

/* signals and args */
enum
{
  SIGNAL_GET_SOURCE_BY_SSRC,
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

#define DEFAULT_INTERNAL_SOURCE      NULL
#define DEFAULT_BANDWIDTH            RTP_STATS_BANDWIDTH
#define DEFAULT_RTCP_FRACTION        RTP_STATS_RTCP_BANDWIDTH
#define DEFAULT_RTCP_MTU             1400
#define DEFAULT_SDES_CNAME           NULL
#define DEFAULT_SDES_NAME            NULL
#define DEFAULT_SDES_EMAIL           NULL
#define DEFAULT_SDES_PHONE           NULL
#define DEFAULT_SDES_LOCATION        NULL
#define DEFAULT_SDES_TOOL            NULL
#define DEFAULT_SDES_NOTE            NULL
#define DEFAULT_NUM_SOURCES          0
#define DEFAULT_NUM_ACTIVE_SOURCES   0
#define DEFAULT_SOURCES              NULL

enum
{
  PROP_0,
  PROP_INTERNAL_SSRC,
  PROP_INTERNAL_SOURCE,
  PROP_BANDWIDTH,
  PROP_RTCP_FRACTION,
  PROP_RTCP_MTU,
  PROP_SDES_CNAME,
  PROP_SDES_NAME,
  PROP_SDES_EMAIL,
  PROP_SDES_PHONE,
  PROP_SDES_LOCATION,
  PROP_SDES_TOOL,
  PROP_SDES_NOTE,
  PROP_NUM_SOURCES,
  PROP_NUM_ACTIVE_SOURCES,
  PROP_SOURCES,
  PROP_LAST
};

/* update average packet size, we keep this scaled by 16 to keep enough
 * precision. */
#define UPDATE_AVG(avg, val)    	\
  if ((avg) == 0)			\
   (avg) = (val) << 4;          	\
  else 					\
   (avg) = ((val) + (15 * (avg))) >> 4;

/* The number RTCP intervals after which to timeout entries in the
 * collision table
 */
#define RTCP_INTERVAL_COLLISION_TIMEOUT 10

/* GObject vmethods */
static void rtp_session_finalize (GObject * object);
static void rtp_session_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void rtp_session_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static guint rtp_session_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (RTPSession, rtp_session, G_TYPE_OBJECT);

static RTPSource *obtain_source (RTPSession * sess, guint32 ssrc,
    gboolean * created, RTPArrivalStats * arrival, gboolean rtp);
static GstFlowReturn rtp_session_schedule_bye_locked (RTPSession * sess,
    const gchar * reason, GstClockTime current_time);
static GstClockTime calculate_rtcp_interval (RTPSession * sess,
    gboolean deterministic, gboolean first);

static void
rtp_session_class_init (RTPSessionClass * klass)
{
  GObjectClass *gobject_class;

  gobject_class = (GObjectClass *) klass;

  gobject_class->finalize = rtp_session_finalize;
  gobject_class->set_property = rtp_session_set_property;
  gobject_class->get_property = rtp_session_get_property;

  /**
   * RTPSession::get-source-by-ssrc:
   * @session: the object which received the signal
   * @ssrc: the SSRC of the RTPSource
   *
   * Request the #RTPSource object with SSRC @ssrc in @session.
   */
  rtp_session_signals[SIGNAL_GET_SOURCE_BY_SSRC] =
      g_signal_new ("get-source-by-ssrc", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION, G_STRUCT_OFFSET (RTPSessionClass,
          get_source_by_ssrc), NULL, NULL, gst_rtp_bin_marshal_OBJECT__UINT,
      RTP_TYPE_SOURCE, 1, G_TYPE_UINT);

  /**
   * RTPSession::on-new-ssrc:
   * @session: the object which received the signal
   * @src: the new RTPSource
   *
   * Notify of a new SSRC that entered @session.
   */
  rtp_session_signals[SIGNAL_ON_NEW_SSRC] =
      g_signal_new ("on-new-ssrc", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (RTPSessionClass, on_new_ssrc),
      NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1,
      RTP_TYPE_SOURCE);
  /**
   * RTPSession::on-ssrc-collision:
   * @session: the object which received the signal
   * @src: the #RTPSource that caused a collision
   *
   * Notify when we have an SSRC collision
   */
  rtp_session_signals[SIGNAL_ON_SSRC_COLLISION] =
      g_signal_new ("on-ssrc-collision", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (RTPSessionClass, on_ssrc_collision),
      NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1,
      RTP_TYPE_SOURCE);
  /**
   * RTPSession::on-ssrc-validated:
   * @session: the object which received the signal
   * @src: the new validated RTPSource
   *
   * Notify of a new SSRC that became validated.
   */
  rtp_session_signals[SIGNAL_ON_SSRC_VALIDATED] =
      g_signal_new ("on-ssrc-validated", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (RTPSessionClass, on_ssrc_validated),
      NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1,
      RTP_TYPE_SOURCE);
  /**
   * RTPSession::on-ssrc-active:
   * @session: the object which received the signal
   * @src: the active RTPSource
   *
   * Notify of a SSRC that is active, i.e., sending RTCP.
   */
  rtp_session_signals[SIGNAL_ON_SSRC_ACTIVE] =
      g_signal_new ("on-ssrc-active", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (RTPSessionClass, on_ssrc_active),
      NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1,
      RTP_TYPE_SOURCE);
  /**
   * RTPSession::on-ssrc-sdes:
   * @session: the object which received the signal
   * @src: the RTPSource
   *
   * Notify that a new SDES was received for SSRC.
   */
  rtp_session_signals[SIGNAL_ON_SSRC_SDES] =
      g_signal_new ("on-ssrc-sdes", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (RTPSessionClass, on_ssrc_sdes),
      NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1,
      RTP_TYPE_SOURCE);
  /**
   * RTPSession::on-bye-ssrc:
   * @session: the object which received the signal
   * @src: the RTPSource that went away
   *
   * Notify of an SSRC that became inactive because of a BYE packet.
   */
  rtp_session_signals[SIGNAL_ON_BYE_SSRC] =
      g_signal_new ("on-bye-ssrc", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (RTPSessionClass, on_bye_ssrc),
      NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1,
      RTP_TYPE_SOURCE);
  /**
   * RTPSession::on-bye-timeout:
   * @session: the object which received the signal
   * @src: the RTPSource that timed out
   *
   * Notify of an SSRC that has timed out because of BYE
   */
  rtp_session_signals[SIGNAL_ON_BYE_TIMEOUT] =
      g_signal_new ("on-bye-timeout", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (RTPSessionClass, on_bye_timeout),
      NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1,
      RTP_TYPE_SOURCE);
  /**
   * RTPSession::on-timeout:
   * @session: the object which received the signal
   * @src: the RTPSource that timed out
   *
   * Notify of an SSRC that has timed out
   */
  rtp_session_signals[SIGNAL_ON_TIMEOUT] =
      g_signal_new ("on-timeout", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (RTPSessionClass, on_timeout),
      NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1,
      RTP_TYPE_SOURCE);
  /**
   * RTPSession::on-sender-timeout:
   * @session: the object which received the signal
   * @src: the RTPSource that timed out
   *
   * Notify of an SSRC that was a sender but timed out and became a receiver.
   */
  rtp_session_signals[SIGNAL_ON_SENDER_TIMEOUT] =
      g_signal_new ("on-sender-timeout", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (RTPSessionClass, on_sender_timeout),
      NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1,
      RTP_TYPE_SOURCE);

  g_object_class_install_property (gobject_class, PROP_INTERNAL_SSRC,
      g_param_spec_uint ("internal-ssrc", "Internal SSRC",
          "The internal SSRC used for the session",
          0, G_MAXUINT, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_INTERNAL_SOURCE,
      g_param_spec_object ("internal-source", "Internal Source",
          "The internal source element of the session",
          RTP_TYPE_SOURCE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_BANDWIDTH,
      g_param_spec_double ("bandwidth", "Bandwidth",
          "The bandwidth of the session",
          0.0, G_MAXDOUBLE, DEFAULT_BANDWIDTH,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_RTCP_FRACTION,
      g_param_spec_double ("rtcp-fraction", "RTCP Fraction",
          "The fraction of the bandwidth used for RTCP",
          0.0, G_MAXDOUBLE, DEFAULT_RTCP_FRACTION,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_RTCP_MTU,
      g_param_spec_uint ("rtcp-mtu", "RTCP MTU",
          "The maximum size of the RTCP packets",
          16, G_MAXINT16, DEFAULT_RTCP_MTU,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SDES_CNAME,
      g_param_spec_string ("sdes-cname", "SDES CNAME",
          "The CNAME to put in SDES messages of this session",
          DEFAULT_SDES_CNAME, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SDES_NAME,
      g_param_spec_string ("sdes-name", "SDES NAME",
          "The NAME to put in SDES messages of this session",
          DEFAULT_SDES_NAME, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SDES_EMAIL,
      g_param_spec_string ("sdes-email", "SDES EMAIL",
          "The EMAIL to put in SDES messages of this session",
          DEFAULT_SDES_EMAIL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SDES_PHONE,
      g_param_spec_string ("sdes-phone", "SDES PHONE",
          "The PHONE to put in SDES messages of this session",
          DEFAULT_SDES_PHONE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SDES_LOCATION,
      g_param_spec_string ("sdes-location", "SDES LOCATION",
          "The LOCATION to put in SDES messages of this session",
          DEFAULT_SDES_LOCATION, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SDES_TOOL,
      g_param_spec_string ("sdes-tool", "SDES TOOL",
          "The TOOL to put in SDES messages of this session",
          DEFAULT_SDES_TOOL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SDES_NOTE,
      g_param_spec_string ("sdes-note", "SDES NOTE",
          "The NOTE to put in SDES messages of this session",
          DEFAULT_SDES_NOTE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_NUM_SOURCES,
      g_param_spec_uint ("num-sources", "Num Sources",
          "The number of sources in the session", 0, G_MAXUINT,
          DEFAULT_NUM_SOURCES, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_NUM_ACTIVE_SOURCES,
      g_param_spec_uint ("num-active-sources", "Num Active Sources",
          "The number of active sources in the session", 0, G_MAXUINT,
          DEFAULT_NUM_ACTIVE_SOURCES,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  /**
   * RTPSource::sources
   *
   * Get a GValue Array of all sources in the session.
   *
   * <example>
   * <title>Getting the #RTPSources of a session
   * <programlisting>
   * {
   *   GValueArray *arr;
   *   GValue *val;
   *   guint i;
   *
   *   g_object_get (sess, "sources", &arr, NULL);
   *
   *   for (i = 0; i < arr->n_values; i++) {
   *     RTPSource *source;
   *
   *     val = g_value_array_get_nth (arr, i);
   *     source = g_value_get_object (val);
   *   }
   *   g_value_array_free (arr);
   * }
   * </programlisting>
   * </example>
   */
  g_object_class_install_property (gobject_class, PROP_SOURCES,
      g_param_spec_boxed ("sources", "Sources",
          "An array of all known sources in the session",
          G_TYPE_VALUE_ARRAY, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  klass->get_source_by_ssrc =
      GST_DEBUG_FUNCPTR (rtp_session_get_source_by_ssrc);

  GST_DEBUG_CATEGORY_INIT (rtp_session_debug, "rtpsession", 0, "RTP Session");
}

static void
rtp_session_init (RTPSession * sess)
{
  gint i;
  gchar *str;

  sess->lock = g_mutex_new ();
  sess->key = g_random_int ();
  sess->mask_idx = 0;
  sess->mask = 0;

  for (i = 0; i < 32; i++) {
    sess->ssrcs[i] =
        g_hash_table_new_full (NULL, NULL, NULL,
        (GDestroyNotify) g_object_unref);
  }
  sess->cnames = g_hash_table_new_full (NULL, NULL, g_free, NULL);

  rtp_stats_init_defaults (&sess->stats);

  /* create an active SSRC for this session manager */
  sess->source = rtp_session_create_source (sess);
  sess->source->validated = TRUE;
  sess->source->internal = TRUE;
  sess->stats.active_sources++;

  /* default UDP header length */
  sess->header_len = 28;
  sess->mtu = DEFAULT_RTCP_MTU;

  /* some default SDES entries */
  str = g_strdup_printf ("%s@%s", g_get_user_name (), g_get_host_name ());
  rtp_source_set_sdes_string (sess->source, GST_RTCP_SDES_CNAME, str);
  g_free (str);

  rtp_source_set_sdes_string (sess->source, GST_RTCP_SDES_NAME,
      g_get_real_name ());
  rtp_source_set_sdes_string (sess->source, GST_RTCP_SDES_TOOL, "GStreamer");

  sess->first_rtcp = TRUE;

  GST_DEBUG ("%p: session using SSRC: %08x", sess, sess->source->ssrc);
}

static void
rtp_session_finalize (GObject * object)
{
  RTPSession *sess;
  gint i;

  sess = RTP_SESSION_CAST (object);

  g_mutex_free (sess->lock);
  for (i = 0; i < 32; i++)
    g_hash_table_destroy (sess->ssrcs[i]);

  g_free (sess->bye_reason);

  g_hash_table_destroy (sess->cnames);
  g_object_unref (sess->source);

  G_OBJECT_CLASS (rtp_session_parent_class)->finalize (object);
}

static void
copy_source (gpointer key, RTPSource * source, GValueArray * arr)
{
  GValue value = { 0 };

  g_value_init (&value, RTP_TYPE_SOURCE);
  g_value_take_object (&value, source);
  g_value_array_append (arr, &value);
}

static GValueArray *
rtp_session_create_sources (RTPSession * sess)
{
  GValueArray *res;
  guint size;

  RTP_SESSION_LOCK (sess);
  /* get number of elements in the table */
  size = g_hash_table_size (sess->ssrcs[sess->mask_idx]);
  /* create the result value array */
  res = g_value_array_new (size);

  /* and copy all values into the array */
  g_hash_table_foreach (sess->ssrcs[sess->mask_idx], (GHFunc) copy_source, res);
  RTP_SESSION_UNLOCK (sess);

  return res;
}

static void
rtp_session_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  RTPSession *sess;

  sess = RTP_SESSION (object);

  switch (prop_id) {
    case PROP_INTERNAL_SSRC:
      rtp_session_set_internal_ssrc (sess, g_value_get_uint (value));
      break;
    case PROP_BANDWIDTH:
      rtp_session_set_bandwidth (sess, g_value_get_double (value));
      break;
    case PROP_RTCP_FRACTION:
      rtp_session_set_rtcp_fraction (sess, g_value_get_double (value));
      break;
    case PROP_RTCP_MTU:
      sess->mtu = g_value_get_uint (value);
      break;
    case PROP_SDES_CNAME:
      rtp_session_set_sdes_string (sess, GST_RTCP_SDES_CNAME,
          g_value_get_string (value));
      break;
    case PROP_SDES_NAME:
      rtp_session_set_sdes_string (sess, GST_RTCP_SDES_NAME,
          g_value_get_string (value));
      break;
    case PROP_SDES_EMAIL:
      rtp_session_set_sdes_string (sess, GST_RTCP_SDES_EMAIL,
          g_value_get_string (value));
      break;
    case PROP_SDES_PHONE:
      rtp_session_set_sdes_string (sess, GST_RTCP_SDES_PHONE,
          g_value_get_string (value));
      break;
    case PROP_SDES_LOCATION:
      rtp_session_set_sdes_string (sess, GST_RTCP_SDES_LOC,
          g_value_get_string (value));
      break;
    case PROP_SDES_TOOL:
      rtp_session_set_sdes_string (sess, GST_RTCP_SDES_TOOL,
          g_value_get_string (value));
      break;
    case PROP_SDES_NOTE:
      rtp_session_set_sdes_string (sess, GST_RTCP_SDES_NOTE,
          g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
rtp_session_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  RTPSession *sess;

  sess = RTP_SESSION (object);

  switch (prop_id) {
    case PROP_INTERNAL_SSRC:
      g_value_set_uint (value, rtp_session_get_internal_ssrc (sess));
      break;
    case PROP_INTERNAL_SOURCE:
      g_value_take_object (value, rtp_session_get_internal_source (sess));
      break;
    case PROP_BANDWIDTH:
      g_value_set_double (value, rtp_session_get_bandwidth (sess));
      break;
    case PROP_RTCP_FRACTION:
      g_value_set_double (value, rtp_session_get_rtcp_fraction (sess));
      break;
    case PROP_RTCP_MTU:
      g_value_set_uint (value, sess->mtu);
      break;
    case PROP_SDES_CNAME:
      g_value_take_string (value, rtp_session_get_sdes_string (sess,
              GST_RTCP_SDES_CNAME));
      break;
    case PROP_SDES_NAME:
      g_value_take_string (value, rtp_session_get_sdes_string (sess,
              GST_RTCP_SDES_NAME));
      break;
    case PROP_SDES_EMAIL:
      g_value_take_string (value, rtp_session_get_sdes_string (sess,
              GST_RTCP_SDES_EMAIL));
      break;
    case PROP_SDES_PHONE:
      g_value_take_string (value, rtp_session_get_sdes_string (sess,
              GST_RTCP_SDES_PHONE));
      break;
    case PROP_SDES_LOCATION:
      g_value_take_string (value, rtp_session_get_sdes_string (sess,
              GST_RTCP_SDES_LOC));
      break;
    case PROP_SDES_TOOL:
      g_value_take_string (value, rtp_session_get_sdes_string (sess,
              GST_RTCP_SDES_TOOL));
      break;
    case PROP_SDES_NOTE:
      g_value_take_string (value, rtp_session_get_sdes_string (sess,
              GST_RTCP_SDES_NOTE));
      break;
    case PROP_NUM_SOURCES:
      g_value_set_uint (value, rtp_session_get_num_sources (sess));
      break;
    case PROP_NUM_ACTIVE_SOURCES:
      g_value_set_uint (value, rtp_session_get_num_active_sources (sess));
      break;
    case PROP_SOURCES:
      g_value_take_boxed (value, rtp_session_create_sources (sess));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
on_new_ssrc (RTPSession * sess, RTPSource * source)
{
  g_object_ref (source);
  RTP_SESSION_UNLOCK (sess);
  g_signal_emit (sess, rtp_session_signals[SIGNAL_ON_NEW_SSRC], 0, source);
  RTP_SESSION_LOCK (sess);
  g_object_unref (source);
}

static void
on_ssrc_collision (RTPSession * sess, RTPSource * source)
{
  g_object_ref (source);
  RTP_SESSION_UNLOCK (sess);
  g_signal_emit (sess, rtp_session_signals[SIGNAL_ON_SSRC_COLLISION], 0,
      source);
  RTP_SESSION_LOCK (sess);
  g_object_unref (source);
}

static void
on_ssrc_validated (RTPSession * sess, RTPSource * source)
{
  g_object_ref (source);
  RTP_SESSION_UNLOCK (sess);
  g_signal_emit (sess, rtp_session_signals[SIGNAL_ON_SSRC_VALIDATED], 0,
      source);
  RTP_SESSION_LOCK (sess);
  g_object_unref (source);
}

static void
on_ssrc_active (RTPSession * sess, RTPSource * source)
{
  g_object_ref (source);
  RTP_SESSION_UNLOCK (sess);
  g_signal_emit (sess, rtp_session_signals[SIGNAL_ON_SSRC_ACTIVE], 0, source);
  RTP_SESSION_LOCK (sess);
  g_object_unref (source);
}

static void
on_ssrc_sdes (RTPSession * sess, RTPSource * source)
{
  g_object_ref (source);
  GST_DEBUG ("SDES changed for SSRC %08x", source->ssrc);
  RTP_SESSION_UNLOCK (sess);
  g_signal_emit (sess, rtp_session_signals[SIGNAL_ON_SSRC_SDES], 0, source);
  RTP_SESSION_LOCK (sess);
  g_object_unref (source);
}

static void
on_bye_ssrc (RTPSession * sess, RTPSource * source)
{
  g_object_ref (source);
  RTP_SESSION_UNLOCK (sess);
  g_signal_emit (sess, rtp_session_signals[SIGNAL_ON_BYE_SSRC], 0, source);
  RTP_SESSION_LOCK (sess);
  g_object_unref (source);
}

static void
on_bye_timeout (RTPSession * sess, RTPSource * source)
{
  g_object_ref (source);
  RTP_SESSION_UNLOCK (sess);
  g_signal_emit (sess, rtp_session_signals[SIGNAL_ON_BYE_TIMEOUT], 0, source);
  RTP_SESSION_LOCK (sess);
  g_object_unref (source);
}

static void
on_timeout (RTPSession * sess, RTPSource * source)
{
  g_object_ref (source);
  RTP_SESSION_UNLOCK (sess);
  g_signal_emit (sess, rtp_session_signals[SIGNAL_ON_TIMEOUT], 0, source);
  RTP_SESSION_LOCK (sess);
  g_object_unref (source);
}

static void
on_sender_timeout (RTPSession * sess, RTPSource * source)
{
  g_object_ref (source);
  RTP_SESSION_UNLOCK (sess);
  g_signal_emit (sess, rtp_session_signals[SIGNAL_ON_SENDER_TIMEOUT], 0,
      source);
  RTP_SESSION_LOCK (sess);
  g_object_unref (source);
}

/**
 * rtp_session_new:
 *
 * Create a new session object.
 *
 * Returns: a new #RTPSession. g_object_unref() after usage.
 */
RTPSession *
rtp_session_new (void)
{
  RTPSession *sess;

  sess = g_object_new (RTP_TYPE_SESSION, NULL);

  return sess;
}

/**
 * rtp_session_set_callbacks:
 * @sess: an #RTPSession
 * @callbacks: callbacks to configure
 * @user_data: user data passed in the callbacks
 *
 * Configure a set of callbacks to be notified of actions.
 */
void
rtp_session_set_callbacks (RTPSession * sess, RTPSessionCallbacks * callbacks,
    gpointer user_data)
{
  g_return_if_fail (RTP_IS_SESSION (sess));

  if (callbacks->process_rtp) {
    sess->callbacks.process_rtp = callbacks->process_rtp;
    sess->process_rtp_user_data = user_data;
  }
  if (callbacks->send_rtp) {
    sess->callbacks.send_rtp = callbacks->send_rtp;
    sess->send_rtp_user_data = user_data;
  }
  if (callbacks->send_rtcp) {
    sess->callbacks.send_rtcp = callbacks->send_rtcp;
    sess->send_rtcp_user_data = user_data;
  }
  if (callbacks->sync_rtcp) {
    sess->callbacks.sync_rtcp = callbacks->sync_rtcp;
    sess->sync_rtcp_user_data = user_data;
  }
  if (callbacks->clock_rate) {
    sess->callbacks.clock_rate = callbacks->clock_rate;
    sess->clock_rate_user_data = user_data;
  }
  if (callbacks->reconsider) {
    sess->callbacks.reconsider = callbacks->reconsider;
    sess->reconsider_user_data = user_data;
  }
}

/**
 * rtp_session_set_process_rtp_callback:
 * @sess: an #RTPSession
 * @callback: callback to set
 * @user_data: user data passed in the callback
 *
 * Configure only the process_rtp callback to be notified of the process_rtp action.
 */
void
rtp_session_set_process_rtp_callback (RTPSession * sess,
    RTPSessionProcessRTP callback, gpointer user_data)
{
  g_return_if_fail (RTP_IS_SESSION (sess));

  sess->callbacks.process_rtp = callback;
  sess->process_rtp_user_data = user_data;
}

/**
 * rtp_session_set_send_rtp_callback:
 * @sess: an #RTPSession
 * @callback: callback to set
 * @user_data: user data passed in the callback
 *
 * Configure only the send_rtp callback to be notified of the send_rtp action.
 */
void
rtp_session_set_send_rtp_callback (RTPSession * sess,
    RTPSessionSendRTP callback, gpointer user_data)
{
  g_return_if_fail (RTP_IS_SESSION (sess));

  sess->callbacks.send_rtp = callback;
  sess->send_rtp_user_data = user_data;
}

/**
 * rtp_session_set_send_rtcp_callback:
 * @sess: an #RTPSession
 * @callback: callback to set
 * @user_data: user data passed in the callback
 *
 * Configure only the send_rtcp callback to be notified of the send_rtcp action.
 */
void
rtp_session_set_send_rtcp_callback (RTPSession * sess,
    RTPSessionSendRTCP callback, gpointer user_data)
{
  g_return_if_fail (RTP_IS_SESSION (sess));

  sess->callbacks.send_rtcp = callback;
  sess->send_rtcp_user_data = user_data;
}

/**
 * rtp_session_set_sync_rtcp_callback:
 * @sess: an #RTPSession
 * @callback: callback to set
 * @user_data: user data passed in the callback
 *
 * Configure only the sync_rtcp callback to be notified of the sync_rtcp action.
 */
void
rtp_session_set_sync_rtcp_callback (RTPSession * sess,
    RTPSessionSyncRTCP callback, gpointer user_data)
{
  g_return_if_fail (RTP_IS_SESSION (sess));

  sess->callbacks.sync_rtcp = callback;
  sess->sync_rtcp_user_data = user_data;
}

/**
 * rtp_session_set_clock_rate_callback:
 * @sess: an #RTPSession
 * @callback: callback to set
 * @user_data: user data passed in the callback
 *
 * Configure only the clock_rate callback to be notified of the clock_rate action.
 */
void
rtp_session_set_clock_rate_callback (RTPSession * sess,
    RTPSessionClockRate callback, gpointer user_data)
{
  g_return_if_fail (RTP_IS_SESSION (sess));

  sess->callbacks.clock_rate = callback;
  sess->clock_rate_user_data = user_data;
}

/**
 * rtp_session_set_reconsider_callback:
 * @sess: an #RTPSession
 * @callback: callback to set
 * @user_data: user data passed in the callback
 *
 * Configure only the reconsider callback to be notified of the reconsider action.
 */
void
rtp_session_set_reconsider_callback (RTPSession * sess,
    RTPSessionReconsider callback, gpointer user_data)
{
  g_return_if_fail (RTP_IS_SESSION (sess));

  sess->callbacks.reconsider = callback;
  sess->reconsider_user_data = user_data;
}

/**
 * rtp_session_set_bandwidth:
 * @sess: an #RTPSession
 * @bandwidth: the bandwidth allocated
 *
 * Set the session bandwidth in bytes per second.
 */
void
rtp_session_set_bandwidth (RTPSession * sess, gdouble bandwidth)
{
  g_return_if_fail (RTP_IS_SESSION (sess));

  RTP_SESSION_LOCK (sess);
  sess->stats.bandwidth = bandwidth;
  RTP_SESSION_UNLOCK (sess);
}

/**
 * rtp_session_get_bandwidth:
 * @sess: an #RTPSession
 *
 * Get the session bandwidth.
 *
 * Returns: the session bandwidth.
 */
gdouble
rtp_session_get_bandwidth (RTPSession * sess)
{
  gdouble result;

  g_return_val_if_fail (RTP_IS_SESSION (sess), 0);

  RTP_SESSION_LOCK (sess);
  result = sess->stats.bandwidth;
  RTP_SESSION_UNLOCK (sess);

  return result;
}

/**
 * rtp_session_set_rtcp_fraction:
 * @sess: an #RTPSession
 * @bandwidth: the RTCP bandwidth
 *
 * Set the bandwidth that should be used for RTCP
 * messages.
 */
void
rtp_session_set_rtcp_fraction (RTPSession * sess, gdouble bandwidth)
{
  g_return_if_fail (RTP_IS_SESSION (sess));

  RTP_SESSION_LOCK (sess);
  sess->stats.rtcp_bandwidth = bandwidth;
  RTP_SESSION_UNLOCK (sess);
}

/**
 * rtp_session_get_rtcp_fraction:
 * @sess: an #RTPSession
 *
 * Get the session bandwidth used for RTCP.
 *
 * Returns: The bandwidth used for RTCP messages.
 */
gdouble
rtp_session_get_rtcp_fraction (RTPSession * sess)
{
  gdouble result;

  g_return_val_if_fail (RTP_IS_SESSION (sess), 0.0);

  RTP_SESSION_LOCK (sess);
  result = sess->stats.rtcp_bandwidth;
  RTP_SESSION_UNLOCK (sess);

  return result;
}

/**
 * rtp_session_set_sdes_string:
 * @sess: an #RTPSession
 * @type: the type of the SDES item
 * @item: a null-terminated string to set. 
 *
 * Store an SDES item of @type in @sess. 
 *
 * Returns: %FALSE if the data was unchanged @type is invalid.
 */
gboolean
rtp_session_set_sdes_string (RTPSession * sess, GstRTCPSDESType type,
    const gchar * item)
{
  gboolean result;

  g_return_val_if_fail (RTP_IS_SESSION (sess), FALSE);

  RTP_SESSION_LOCK (sess);
  result = rtp_source_set_sdes_string (sess->source, type, item);
  RTP_SESSION_UNLOCK (sess);

  return result;
}

/**
 * rtp_session_get_sdes_string:
 * @sess: an #RTPSession
 * @type: the type of the SDES item
 *
 * Get the SDES item of @type from @sess. 
 *
 * Returns: a null-terminated copy of the SDES item or NULL when @type was not
 * valid. g_free() after usage.
 */
gchar *
rtp_session_get_sdes_string (RTPSession * sess, GstRTCPSDESType type)
{
  gchar *result;

  g_return_val_if_fail (RTP_IS_SESSION (sess), NULL);

  RTP_SESSION_LOCK (sess);
  result = rtp_source_get_sdes_string (sess->source, type);
  RTP_SESSION_UNLOCK (sess);

  return result;
}

static GstFlowReturn
source_push_rtp (RTPSource * source, GstBuffer * buffer, RTPSession * session)
{
  GstFlowReturn result = GST_FLOW_OK;

  if (source == session->source) {
    GST_LOG ("source %08x pushed sender RTP packet", source->ssrc);

    RTP_SESSION_UNLOCK (session);

    if (session->callbacks.send_rtp)
      result =
          session->callbacks.send_rtp (session, source, buffer,
          session->send_rtp_user_data);
    else
      gst_buffer_unref (buffer);

  } else {
    GST_LOG ("source %08x pushed receiver RTP packet", source->ssrc);
    RTP_SESSION_UNLOCK (session);

    if (session->callbacks.process_rtp)
      result =
          session->callbacks.process_rtp (session, source, buffer,
          session->process_rtp_user_data);
    else
      gst_buffer_unref (buffer);
  }
  RTP_SESSION_LOCK (session);

  return result;
}

static gint
source_clock_rate (RTPSource * source, guint8 pt, RTPSession * session)
{
  gint result;

  RTP_SESSION_UNLOCK (session);

  if (session->callbacks.clock_rate)
    result =
        session->callbacks.clock_rate (session, pt,
        session->clock_rate_user_data);
  else
    result = -1;

  RTP_SESSION_LOCK (session);

  GST_DEBUG ("got clock-rate %d for pt %d", result, pt);

  return result;
}

static RTPSourceCallbacks callbacks = {
  (RTPSourcePushRTP) source_push_rtp,
  (RTPSourceClockRate) source_clock_rate,
};

/**
 * find_add_conflicting_addresses:
 * @sess: The session to check in
 * @arrival: The arrival stats for the buffer
 *
 * Checks if an address which has a conflict is already known,
 *  otherwise remembers it to prevent loops.
 *
 * Returns: TRUE if it was a known conflict, FALSE otherwise
 */

static gboolean
find_add_conflicting_addresses (RTPSession * sess, RTPArrivalStats * arrival)
{
  GList *item;
  RTPConflictingAddress *new_conflict;

  for (item = g_list_first (sess->conflicting_addresses);
      item; item = g_list_next (item)) {
    RTPConflictingAddress *known_conflict = item->data;

    if (gst_netaddress_equal (&arrival->address, &known_conflict->address)) {
      known_conflict->time = arrival->time;
      return TRUE;
    }
  }

  new_conflict = g_new0 (RTPConflictingAddress, 1);

  memcpy (&new_conflict->address, &arrival->address, sizeof (GstNetAddress));
  new_conflict->time = arrival->time;

  sess->conflicting_addresses = g_list_prepend (sess->conflicting_addresses,
      new_conflict);

  return FALSE;
}

static gboolean
check_collision (RTPSession * sess, RTPSource * source,
    RTPArrivalStats * arrival, gboolean rtp)
{
  /* If we have no arrival address, we can't do collision checking */
  if (!arrival->have_address)
    return FALSE;

  if (sess->source != source) {
    /* This is not our local source, but lets check if two remote
     * source collide
     */
    if (rtp) {
      if (source->have_rtp_from) {
        if (gst_netaddress_equal (&source->rtp_from, &arrival->address))
          /* Address is the same */
          return FALSE;
      } else {
        /* We don't already have a from address for RTP, just set it */
        rtp_source_set_rtp_from (source, &arrival->address);
        return FALSE;
      }
    } else {
      if (source->have_rtcp_from) {
        if (gst_netaddress_equal (&source->rtcp_from, &arrival->address))
          /* Address is the same */
          return FALSE;
      } else {
        /* We don't already have a from address for RTCP, just set it */
        rtp_source_set_rtcp_from (source, &arrival->address);
        return FALSE;
      }
    }
    /* We received RTP or RTCP from this source before but the network address
     * changed. In this case, we have third-party collision or loop */
    GST_DEBUG ("we have a third-party collision or loop");

    /* FIXME: Log 3rd party collision somehow
     * Maybe should be done in upper layer, only the SDES can tell us
     * if its a collision or a loop
     */
  } else {
    /* This is sending with our ssrc, is it an address we already know */

    if (find_add_conflicting_addresses (sess, arrival)) {
      /* Its a known conflict, its probably a loop, not a collision
       * lets just drop the incoming packet
       */
      GST_DEBUG ("Our packets are being looped back to us, dropping");
    } else {
      /* Its a new collision, lets change our SSRC */

      GST_DEBUG ("Collision for SSRC %x", rtp_source_get_ssrc (source));
      on_ssrc_collision (sess, source);

      rtp_session_schedule_bye_locked (sess, "SSRC Collision", arrival->time);

      sess->change_ssrc = TRUE;
    }
  }

  return TRUE;
}


/* must be called with the session lock, the returned source needs to be
 * unreffed after usage. */
static RTPSource *
obtain_source (RTPSession * sess, guint32 ssrc, gboolean * created,
    RTPArrivalStats * arrival, gboolean rtp)
{
  RTPSource *source;

  source =
      g_hash_table_lookup (sess->ssrcs[sess->mask_idx], GINT_TO_POINTER (ssrc));
  if (source == NULL) {
    /* make new Source in probation and insert */
    source = rtp_source_new (ssrc);

    /* for RTP packets we need to set the source in probation. Receiving RTCP
     * packets of an SSRC, on the other hand, is a strong indication that we
     * are dealing with a valid source. */
    if (rtp)
      source->probation = RTP_DEFAULT_PROBATION;
    else
      source->probation = 0;

    /* store from address, if any */
    if (arrival->have_address) {
      if (rtp)
        rtp_source_set_rtp_from (source, &arrival->address);
      else
        rtp_source_set_rtcp_from (source, &arrival->address);
    }

    /* configure a callback on the source */
    rtp_source_set_callbacks (source, &callbacks, sess);

    g_hash_table_insert (sess->ssrcs[sess->mask_idx], GINT_TO_POINTER (ssrc),
        source);

    /* we have one more source now */
    sess->total_sources++;
    *created = TRUE;
  } else {
    *created = FALSE;
    /* check for collision, this updates the address when not previously set */
    if (check_collision (sess, source, arrival, rtp)) {
      return NULL;
    }
  }
  /* update last activity */
  source->last_activity = arrival->time;
  if (rtp)
    source->last_rtp_activity = arrival->time;
  g_object_ref (source);

  return source;
}

/**
 * rtp_session_get_internal_source:
 * @sess: a #RTPSession
 *
 * Get the internal #RTPSource of @sess.
 *
 * Returns: The internal #RTPSource. g_object_unref() after usage.
 */
RTPSource *
rtp_session_get_internal_source (RTPSession * sess)
{
  RTPSource *result;

  g_return_val_if_fail (RTP_IS_SESSION (sess), NULL);

  result = g_object_ref (sess->source);

  return result;
}

/**
 * rtp_session_set_internal_ssrc:
 * @sess: a #RTPSession
 * @ssrc: an SSRC
 *
 * Set the SSRC of @sess to @ssrc.
 */
void
rtp_session_set_internal_ssrc (RTPSession * sess, guint32 ssrc)
{
  RTP_SESSION_LOCK (sess);
  if (ssrc != sess->source->ssrc) {
    g_hash_table_steal (sess->ssrcs[sess->mask_idx],
        GINT_TO_POINTER (sess->source->ssrc));

    GST_DEBUG ("setting internal SSRC to %08x", ssrc);
    /* After this call, any receiver of the old SSRC either in RTP or RTCP
     * packets will timeout on the old SSRC, we could potentially schedule a
     * BYE RTCP for the old SSRC... */
    sess->source->ssrc = ssrc;
    rtp_source_reset (sess->source);

    /* rehash with the new SSRC */
    g_hash_table_insert (sess->ssrcs[sess->mask_idx],
        GINT_TO_POINTER (sess->source->ssrc), sess->source);
  }
  RTP_SESSION_UNLOCK (sess);
}

/**
 * rtp_session_get_internal_ssrc:
 * @sess: a #RTPSession
 *
 * Get the internal SSRC of @sess.
 *
 * Returns: The SSRC of the session. 
 */
guint32
rtp_session_get_internal_ssrc (RTPSession * sess)
{
  guint32 ssrc;

  RTP_SESSION_LOCK (sess);
  ssrc = sess->source->ssrc;
  RTP_SESSION_UNLOCK (sess);

  return ssrc;
}

/**
 * rtp_session_add_source:
 * @sess: a #RTPSession
 * @src: #RTPSource to add
 *
 * Add @src to @session.
 *
 * Returns: %TRUE on success, %FALSE if a source with the same SSRC already
 * existed in the session.
 */
gboolean
rtp_session_add_source (RTPSession * sess, RTPSource * src)
{
  gboolean result = FALSE;
  RTPSource *find;

  g_return_val_if_fail (RTP_IS_SESSION (sess), FALSE);
  g_return_val_if_fail (src != NULL, FALSE);

  RTP_SESSION_LOCK (sess);
  find =
      g_hash_table_lookup (sess->ssrcs[sess->mask_idx],
      GINT_TO_POINTER (src->ssrc));
  if (find == NULL) {
    g_hash_table_insert (sess->ssrcs[sess->mask_idx],
        GINT_TO_POINTER (src->ssrc), src);
    /* we have one more source now */
    sess->total_sources++;
    result = TRUE;
  }
  RTP_SESSION_UNLOCK (sess);

  return result;
}

/**
 * rtp_session_get_num_sources:
 * @sess: an #RTPSession
 *
 * Get the number of sources in @sess.
 *
 * Returns: The number of sources in @sess.
 */
guint
rtp_session_get_num_sources (RTPSession * sess)
{
  guint result;

  g_return_val_if_fail (RTP_IS_SESSION (sess), FALSE);

  RTP_SESSION_LOCK (sess);
  result = sess->total_sources;
  RTP_SESSION_UNLOCK (sess);

  return result;
}

/**
 * rtp_session_get_num_active_sources:
 * @sess: an #RTPSession
 *
 * Get the number of active sources in @sess. A source is considered active when
 * it has been validated and has not yet received a BYE RTCP message.
 *
 * Returns: The number of active sources in @sess.
 */
guint
rtp_session_get_num_active_sources (RTPSession * sess)
{
  guint result;

  g_return_val_if_fail (RTP_IS_SESSION (sess), 0);

  RTP_SESSION_LOCK (sess);
  result = sess->stats.active_sources;
  RTP_SESSION_UNLOCK (sess);

  return result;
}

/**
 * rtp_session_get_source_by_ssrc:
 * @sess: an #RTPSession
 * @ssrc: an SSRC
 *
 * Find the source with @ssrc in @sess.
 *
 * Returns: a #RTPSource with SSRC @ssrc or NULL if the source was not found.
 * g_object_unref() after usage.
 */
RTPSource *
rtp_session_get_source_by_ssrc (RTPSession * sess, guint32 ssrc)
{
  RTPSource *result;

  g_return_val_if_fail (RTP_IS_SESSION (sess), NULL);

  RTP_SESSION_LOCK (sess);
  result =
      g_hash_table_lookup (sess->ssrcs[sess->mask_idx], GINT_TO_POINTER (ssrc));
  if (result)
    g_object_ref (result);
  RTP_SESSION_UNLOCK (sess);

  return result;
}

/**
 * rtp_session_get_source_by_cname:
 * @sess: a #RTPSession
 * @cname: an CNAME
 *
 * Find the source with @cname in @sess.
 *
 * Returns: a #RTPSource with CNAME @cname or NULL if the source was not found.
 * g_object_unref() after usage.
 */
RTPSource *
rtp_session_get_source_by_cname (RTPSession * sess, const gchar * cname)
{
  RTPSource *result;

  g_return_val_if_fail (RTP_IS_SESSION (sess), NULL);
  g_return_val_if_fail (cname != NULL, NULL);

  RTP_SESSION_LOCK (sess);
  result = g_hash_table_lookup (sess->cnames, cname);
  if (result)
    g_object_ref (result);
  RTP_SESSION_UNLOCK (sess);

  return result;
}

static guint32
rtp_session_create_new_ssrc (RTPSession * sess)
{
  guint32 ssrc;

  while (TRUE) {
    ssrc = g_random_int ();

    /* see if it exists in the session, we're done if it doesn't */
    if (g_hash_table_lookup (sess->ssrcs[sess->mask_idx],
            GINT_TO_POINTER (ssrc)) == NULL)
      break;
  }

  return ssrc;
}


/**
 * rtp_session_create_source:
 * @sess: an #RTPSession
 *
 * Create an #RTPSource for use in @sess. This function will create a source
 * with an ssrc that is currently not used by any participants in the session.
 *
 * Returns: an #RTPSource.
 */
RTPSource *
rtp_session_create_source (RTPSession * sess)
{
  guint32 ssrc;
  RTPSource *source;

  RTP_SESSION_LOCK (sess);
  ssrc = rtp_session_create_new_ssrc (sess);
  source = rtp_source_new (ssrc);
  rtp_source_set_callbacks (source, &callbacks, sess);
  /* we need an additional ref for the source in the hashtable */
  g_object_ref (source);
  g_hash_table_insert (sess->ssrcs[sess->mask_idx], GINT_TO_POINTER (ssrc),
      source);
  /* we have one more source now */
  sess->total_sources++;
  RTP_SESSION_UNLOCK (sess);

  return source;
}

/* update the RTPArrivalStats structure with the current time and other bits
 * about the current buffer we are handling.
 * This function is typically called when a validated packet is received.
 * This function should be called with the SESSION_LOCK
 */
static void
update_arrival_stats (RTPSession * sess, RTPArrivalStats * arrival,
    gboolean rtp, GstBuffer * buffer, GstClockTime current_time,
    GstClockTime running_time, guint64 ntpnstime)
{
  /* get time of arrival */
  arrival->time = current_time;
  arrival->running_time = running_time;
  arrival->ntpnstime = ntpnstime;

  /* get packet size including header overhead */
  arrival->bytes = GST_BUFFER_SIZE (buffer) + sess->header_len;

  if (rtp) {
    arrival->payload_len = gst_rtp_buffer_get_payload_len (buffer);
  } else {
    arrival->payload_len = 0;
  }

  /* for netbuffer we can store the IP address to check for collisions */
  arrival->have_address = GST_IS_NETBUFFER (buffer);
  if (arrival->have_address) {
    GstNetBuffer *netbuf = (GstNetBuffer *) buffer;

    memcpy (&arrival->address, &netbuf->from, sizeof (GstNetAddress));
  }
}

/**
 * rtp_session_process_rtp:
 * @sess: and #RTPSession
 * @buffer: an RTP buffer
 * @current_time: the current system time
 * @ntpnstime: the NTP arrival time in nanoseconds
 *
 * Process an RTP buffer in the session manager. This function takes ownership
 * of @buffer.
 *
 * Returns: a #GstFlowReturn.
 */
GstFlowReturn
rtp_session_process_rtp (RTPSession * sess, GstBuffer * buffer,
    GstClockTime current_time, GstClockTime running_time, guint64 ntpnstime)
{
  GstFlowReturn result;
  guint32 ssrc;
  RTPSource *source;
  gboolean created;
  gboolean prevsender, prevactive;
  RTPArrivalStats arrival;

  g_return_val_if_fail (RTP_IS_SESSION (sess), GST_FLOW_ERROR);
  g_return_val_if_fail (GST_IS_BUFFER (buffer), GST_FLOW_ERROR);

  if (!gst_rtp_buffer_validate (buffer))
    goto invalid_packet;

  RTP_SESSION_LOCK (sess);
  /* update arrival stats */
  update_arrival_stats (sess, &arrival, TRUE, buffer, current_time,
      running_time, ntpnstime);

  /* ignore more RTP packets when we left the session */
  if (sess->source->received_bye)
    goto ignore;

  /* get SSRC and look up in session database */
  ssrc = gst_rtp_buffer_get_ssrc (buffer);
  source = obtain_source (sess, ssrc, &created, &arrival, TRUE);
  if (!source)
    goto collision;

  prevsender = RTP_SOURCE_IS_SENDER (source);
  prevactive = RTP_SOURCE_IS_ACTIVE (source);

  /* we need to ref so that we can process the CSRCs later */
  gst_buffer_ref (buffer);

  /* let source process the packet */
  result = rtp_source_process_rtp (source, buffer, &arrival);

  /* source became active */
  if (prevactive != RTP_SOURCE_IS_ACTIVE (source)) {
    sess->stats.active_sources++;
    GST_DEBUG ("source: %08x became active, %d active sources", ssrc,
        sess->stats.active_sources);
    on_ssrc_validated (sess, source);
  }
  if (prevsender != RTP_SOURCE_IS_SENDER (source)) {
    sess->stats.sender_sources++;
    GST_DEBUG ("source: %08x became sender, %d sender sources", ssrc,
        sess->stats.sender_sources);
  }

  if (created)
    on_new_ssrc (sess, source);

  if (source->validated) {
    guint8 i, count;
    gboolean created;

    /* for validated sources, we add the CSRCs as well */
    count = gst_rtp_buffer_get_csrc_count (buffer);

    for (i = 0; i < count; i++) {
      guint32 csrc;
      RTPSource *csrc_src;

      csrc = gst_rtp_buffer_get_csrc (buffer, i);

      /* get source */
      csrc_src = obtain_source (sess, csrc, &created, &arrival, TRUE);
      if (!csrc_src)
        continue;

      if (created) {
        GST_DEBUG ("created new CSRC: %08x", csrc);
        rtp_source_set_as_csrc (csrc_src);
        if (RTP_SOURCE_IS_ACTIVE (csrc_src))
          sess->stats.active_sources++;
        on_new_ssrc (sess, csrc_src);
      }
      g_object_unref (csrc_src);
    }
  }
  g_object_unref (source);
  gst_buffer_unref (buffer);

  RTP_SESSION_UNLOCK (sess);

  return result;

  /* ERRORS */
invalid_packet:
  {
    gst_buffer_unref (buffer);
    GST_DEBUG ("invalid RTP packet received");
    return GST_FLOW_OK;
  }
ignore:
  {
    gst_buffer_unref (buffer);
    RTP_SESSION_UNLOCK (sess);
    GST_DEBUG ("ignoring RTP packet because we are leaving");
    return GST_FLOW_OK;
  }
collision:
  {
    gst_buffer_unref (buffer);
    RTP_SESSION_UNLOCK (sess);
    GST_DEBUG ("ignoring packet because its collisioning");
    return GST_FLOW_OK;
  }
}

static void
rtp_session_process_rb (RTPSession * sess, RTPSource * source,
    GstRTCPPacket * packet, RTPArrivalStats * arrival)
{
  guint count, i;

  count = gst_rtcp_packet_get_rb_count (packet);
  for (i = 0; i < count; i++) {
    guint32 ssrc, exthighestseq, jitter, lsr, dlsr;
    guint8 fractionlost;
    gint32 packetslost;

    gst_rtcp_packet_get_rb (packet, i, &ssrc, &fractionlost,
        &packetslost, &exthighestseq, &jitter, &lsr, &dlsr);

    GST_DEBUG ("RB %d: SSRC %08x, jitter %" G_GUINT32_FORMAT, i, ssrc, jitter);

    if (ssrc == sess->source->ssrc) {
      /* only deal with report blocks for our session, we update the stats of
       * the sender of the RTCP message. We could also compare our stats against
       * the other sender to see if we are better or worse. */
      rtp_source_process_rb (source, arrival->time, fractionlost, packetslost,
          exthighestseq, jitter, lsr, dlsr);

      on_ssrc_active (sess, source);
    }
  }
}

/* A Sender report contains statistics about how the sender is doing. This
 * includes timing informataion such as the relation between RTP and NTP
 * timestamps and the number of packets/bytes it sent to us.
 *
 * In this report is also included a set of report blocks related to how this
 * sender is receiving data (in case we (or somebody else) is also sending stuff
 * to it). This info includes the packet loss, jitter and seqnum. It also
 * contains information to calculate the round trip time (LSR/DLSR).
 */
static void
rtp_session_process_sr (RTPSession * sess, GstRTCPPacket * packet,
    RTPArrivalStats * arrival)
{
  guint32 senderssrc, rtptime, packet_count, octet_count;
  guint64 ntptime;
  RTPSource *source;
  gboolean created, prevsender;

  gst_rtcp_packet_sr_get_sender_info (packet, &senderssrc, &ntptime, &rtptime,
      &packet_count, &octet_count);

  GST_DEBUG ("got SR packet: SSRC %08x, time %" GST_TIME_FORMAT,
      senderssrc, GST_TIME_ARGS (arrival->time));

  source = obtain_source (sess, senderssrc, &created, arrival, FALSE);
  if (!source)
    return;

  prevsender = RTP_SOURCE_IS_SENDER (source);

  /* first update the source */
  rtp_source_process_sr (source, arrival->time, ntptime, rtptime, packet_count,
      octet_count);

  if (prevsender != RTP_SOURCE_IS_SENDER (source)) {
    sess->stats.sender_sources++;
    GST_DEBUG ("source: %08x became sender, %d sender sources", senderssrc,
        sess->stats.sender_sources);
  }

  if (created)
    on_new_ssrc (sess, source);

  rtp_session_process_rb (sess, source, packet, arrival);
  g_object_unref (source);
}

/* A receiver report contains statistics about how a receiver is doing. It
 * includes stuff like packet loss, jitter and the seqnum it received last. It
 * also contains info to calculate the round trip time.
 *
 * We are only interested in how the sender of this report is doing wrt to us.
 */
static void
rtp_session_process_rr (RTPSession * sess, GstRTCPPacket * packet,
    RTPArrivalStats * arrival)
{
  guint32 senderssrc;
  RTPSource *source;
  gboolean created;

  senderssrc = gst_rtcp_packet_rr_get_ssrc (packet);

  GST_DEBUG ("got RR packet: SSRC %08x", senderssrc);

  source = obtain_source (sess, senderssrc, &created, arrival, FALSE);
  if (!source)
    return;

  if (created)
    on_new_ssrc (sess, source);

  rtp_session_process_rb (sess, source, packet, arrival);
  g_object_unref (source);
}

/* Get SDES items and store them in the SSRC */
static void
rtp_session_process_sdes (RTPSession * sess, GstRTCPPacket * packet,
    RTPArrivalStats * arrival)
{
  guint items, i, j;
  gboolean more_items, more_entries;

  items = gst_rtcp_packet_sdes_get_item_count (packet);
  GST_DEBUG ("got SDES packet with %d items", items);

  more_items = gst_rtcp_packet_sdes_first_item (packet);
  i = 0;
  while (more_items) {
    guint32 ssrc;
    gboolean changed, created;
    RTPSource *source;

    ssrc = gst_rtcp_packet_sdes_get_ssrc (packet);

    GST_DEBUG ("item %d, SSRC %08x", i, ssrc);

    changed = FALSE;

    /* find src, no probation when dealing with RTCP */
    source = obtain_source (sess, ssrc, &created, arrival, FALSE);
    if (!source)
      return;

    more_entries = gst_rtcp_packet_sdes_first_entry (packet);
    j = 0;
    while (more_entries) {
      GstRTCPSDESType type;
      guint8 len;
      guint8 *data;

      gst_rtcp_packet_sdes_get_entry (packet, &type, &len, &data);

      GST_DEBUG ("entry %d, type %d, len %d, data %.*s", j, type, len, len,
          data);

      changed |= rtp_source_set_sdes (source, type, data, len);

      more_entries = gst_rtcp_packet_sdes_next_entry (packet);
      j++;
    }

    source->validated = TRUE;

    if (created)
      on_new_ssrc (sess, source);
    if (changed)
      on_ssrc_sdes (sess, source);

    g_object_unref (source);

    more_items = gst_rtcp_packet_sdes_next_item (packet);
    i++;
  }
}

/* BYE is sent when a client leaves the session
 */
static void
rtp_session_process_bye (RTPSession * sess, GstRTCPPacket * packet,
    RTPArrivalStats * arrival)
{
  guint count, i;
  gchar *reason;

  reason = gst_rtcp_packet_bye_get_reason (packet);
  GST_DEBUG ("got BYE packet (reason: %s)", GST_STR_NULL (reason));

  count = gst_rtcp_packet_bye_get_ssrc_count (packet);
  for (i = 0; i < count; i++) {
    guint32 ssrc;
    RTPSource *source;
    gboolean created, prevactive, prevsender;
    guint pmembers, members;

    ssrc = gst_rtcp_packet_bye_get_nth_ssrc (packet, i);
    GST_DEBUG ("SSRC: %08x", ssrc);

    /* find src and mark bye, no probation when dealing with RTCP */
    source = obtain_source (sess, ssrc, &created, arrival, FALSE);
    if (!source)
      return;

    /* store time for when we need to time out this source */
    source->bye_time = arrival->time;

    prevactive = RTP_SOURCE_IS_ACTIVE (source);
    prevsender = RTP_SOURCE_IS_SENDER (source);

    /* let the source handle the rest */
    rtp_source_process_bye (source, reason);

    pmembers = sess->stats.active_sources;

    if (prevactive && !RTP_SOURCE_IS_ACTIVE (source)) {
      sess->stats.active_sources--;
      GST_DEBUG ("source: %08x became inactive, %d active sources", ssrc,
          sess->stats.active_sources);
    }
    if (prevsender && !RTP_SOURCE_IS_SENDER (source)) {
      sess->stats.sender_sources--;
      GST_DEBUG ("source: %08x became non sender, %d sender sources", ssrc,
          sess->stats.sender_sources);
    }
    members = sess->stats.active_sources;

    if (!sess->source->received_bye && members < pmembers) {
      /* some members went away since the previous timeout estimate.
       * Perform reverse reconsideration but only when we are not scheduling a
       * BYE ourselves. */
      if (arrival->time < sess->next_rtcp_check_time) {
        GstClockTime time_remaining;

        time_remaining = sess->next_rtcp_check_time - arrival->time;
        sess->next_rtcp_check_time =
            gst_util_uint64_scale (time_remaining, members, pmembers);

        GST_DEBUG ("reverse reconsideration %" GST_TIME_FORMAT,
            GST_TIME_ARGS (sess->next_rtcp_check_time));

        sess->next_rtcp_check_time += arrival->time;

        RTP_SESSION_UNLOCK (sess);
        /* notify app of reconsideration */
        if (sess->callbacks.reconsider)
          sess->callbacks.reconsider (sess, sess->reconsider_user_data);
        RTP_SESSION_LOCK (sess);
      }
    }

    if (created)
      on_new_ssrc (sess, source);

    on_bye_ssrc (sess, source);

    g_object_unref (source);
  }
  g_free (reason);
}

static void
rtp_session_process_app (RTPSession * sess, GstRTCPPacket * packet,
    RTPArrivalStats * arrival)
{
  GST_DEBUG ("received APP");
}

/**
 * rtp_session_process_rtcp:
 * @sess: and #RTPSession
 * @buffer: an RTCP buffer
 * @current_time: the current system time
 *
 * Process an RTCP buffer in the session manager. This function takes ownership
 * of @buffer.
 *
 * Returns: a #GstFlowReturn.
 */
GstFlowReturn
rtp_session_process_rtcp (RTPSession * sess, GstBuffer * buffer,
    GstClockTime current_time)
{
  GstRTCPPacket packet;
  gboolean more, is_bye = FALSE, is_sr = FALSE;
  RTPArrivalStats arrival;
  GstFlowReturn result = GST_FLOW_OK;

  g_return_val_if_fail (RTP_IS_SESSION (sess), GST_FLOW_ERROR);
  g_return_val_if_fail (GST_IS_BUFFER (buffer), GST_FLOW_ERROR);

  if (!gst_rtcp_buffer_validate (buffer))
    goto invalid_packet;

  GST_DEBUG ("received RTCP packet");

  RTP_SESSION_LOCK (sess);
  /* update arrival stats */
  update_arrival_stats (sess, &arrival, FALSE, buffer, current_time, -1, -1);

  if (sess->sent_bye)
    goto ignore;

  /* make writable, we might want to change the buffer */
  buffer = gst_buffer_make_metadata_writable (buffer);

  /* start processing the compound packet */
  more = gst_rtcp_buffer_get_first_packet (buffer, &packet);
  while (more) {
    GstRTCPType type;

    type = gst_rtcp_packet_get_type (&packet);

    /* when we are leaving the session, we should ignore all non-BYE messages */
    if (sess->source->received_bye && type != GST_RTCP_TYPE_BYE) {
      GST_DEBUG ("ignoring non-BYE RTCP packet because we are leaving");
      goto next;
    }

    switch (type) {
      case GST_RTCP_TYPE_SR:
        rtp_session_process_sr (sess, &packet, &arrival);
        is_sr = TRUE;
        break;
      case GST_RTCP_TYPE_RR:
        rtp_session_process_rr (sess, &packet, &arrival);
        break;
      case GST_RTCP_TYPE_SDES:
        rtp_session_process_sdes (sess, &packet, &arrival);
        break;
      case GST_RTCP_TYPE_BYE:
        is_bye = TRUE;
        rtp_session_process_bye (sess, &packet, &arrival);
        break;
      case GST_RTCP_TYPE_APP:
        rtp_session_process_app (sess, &packet, &arrival);
        break;
      default:
        GST_WARNING ("got unknown RTCP packet");
        break;
    }
  next:
    more = gst_rtcp_packet_move_to_next (&packet);
  }

  /* if we are scheduling a BYE, we only want to count bye packets, else we
   * count everything */
  if (sess->source->received_bye) {
    if (is_bye) {
      sess->stats.bye_members++;
      UPDATE_AVG (sess->stats.avg_rtcp_packet_size, arrival.bytes);
    }
  } else {
    /* keep track of average packet size */
    UPDATE_AVG (sess->stats.avg_rtcp_packet_size, arrival.bytes);
  }
  RTP_SESSION_UNLOCK (sess);

  /* notify caller of sr packets in the callback */
  if (is_sr && sess->callbacks.sync_rtcp)
    result = sess->callbacks.sync_rtcp (sess, sess->source, buffer,
        sess->sync_rtcp_user_data);
  else
    gst_buffer_unref (buffer);

  return result;

  /* ERRORS */
invalid_packet:
  {
    GST_DEBUG ("invalid RTCP packet received");
    gst_buffer_unref (buffer);
    return GST_FLOW_OK;
  }
ignore:
  {
    gst_buffer_unref (buffer);
    RTP_SESSION_UNLOCK (sess);
    GST_DEBUG ("ignoring RTP packet because we left");
    return GST_FLOW_OK;
  }
}

/**
 * rtp_session_send_rtp:
 * @sess: an #RTPSession
 * @buffer: an RTP buffer
 * @current_time: the current system time
 * @ntpnstime: the NTP time in nanoseconds of when this buffer was captured.
 * This is the buffer timestamp converted to NTP time.
 *
 * Send the RTP buffer in the session manager. This function takes ownership of
 * @buffer.
 *
 * Returns: a #GstFlowReturn.
 */
GstFlowReturn
rtp_session_send_rtp (RTPSession * sess, GstBuffer * buffer,
    GstClockTime current_time, guint64 ntpnstime)
{
  GstFlowReturn result;
  RTPSource *source;
  gboolean prevsender;

  g_return_val_if_fail (RTP_IS_SESSION (sess), GST_FLOW_ERROR);
  g_return_val_if_fail (GST_IS_BUFFER (buffer), GST_FLOW_ERROR);

  if (!gst_rtp_buffer_validate (buffer))
    goto invalid_packet;

  GST_LOG ("received RTP packet for sending");

  RTP_SESSION_LOCK (sess);
  source = sess->source;

  /* update last activity */
  source->last_rtp_activity = current_time;

  prevsender = RTP_SOURCE_IS_SENDER (source);

  /* we use our own source to send */
  result = rtp_source_send_rtp (source, buffer, ntpnstime);

  if (RTP_SOURCE_IS_SENDER (source) && !prevsender)
    sess->stats.sender_sources++;
  RTP_SESSION_UNLOCK (sess);

  return result;

  /* ERRORS */
invalid_packet:
  {
    gst_buffer_unref (buffer);
    GST_DEBUG ("invalid RTP packet received");
    return GST_FLOW_OK;
  }
}

static GstClockTime
calculate_rtcp_interval (RTPSession * sess, gboolean deterministic,
    gboolean first)
{
  GstClockTime result;

  if (sess->source->received_bye) {
    result = rtp_stats_calculate_bye_interval (&sess->stats);
  } else {
    result = rtp_stats_calculate_rtcp_interval (&sess->stats,
        RTP_SOURCE_IS_SENDER (sess->source), first);
  }

  GST_DEBUG ("next deterministic interval: %" GST_TIME_FORMAT ", first %d",
      GST_TIME_ARGS (result), first);

  if (!deterministic)
    result = rtp_stats_add_rtcp_jitter (&sess->stats, result);

  GST_DEBUG ("next interval: %" GST_TIME_FORMAT, GST_TIME_ARGS (result));

  return result;
}

/* Stop the current @sess and schedule a BYE message for the other members.
 * One must have the session lock to call this function
 */
static GstFlowReturn
rtp_session_schedule_bye_locked (RTPSession * sess, const gchar * reason,
    GstClockTime current_time)
{
  GstFlowReturn result = GST_FLOW_OK;
  RTPSource *source;
  GstClockTime interval;

  g_return_val_if_fail (RTP_IS_SESSION (sess), GST_FLOW_ERROR);

  source = sess->source;

  /* ignore more BYEs */
  if (source->received_bye)
    goto done;

  /* we have BYE now */
  source->received_bye = TRUE;
  /* at least one member wants to send a BYE */
  g_free (sess->bye_reason);
  sess->bye_reason = g_strdup (reason);
  sess->stats.avg_rtcp_packet_size = 100;
  sess->stats.bye_members = 1;
  sess->first_rtcp = TRUE;
  sess->sent_bye = FALSE;

  /* reschedule transmission */
  sess->last_rtcp_send_time = current_time;
  interval = calculate_rtcp_interval (sess, FALSE, TRUE);
  sess->next_rtcp_check_time = current_time + interval;

  GST_DEBUG ("Schedule BYE for %" GST_TIME_FORMAT ", %" GST_TIME_FORMAT,
      GST_TIME_ARGS (interval), GST_TIME_ARGS (sess->next_rtcp_check_time));

  RTP_SESSION_UNLOCK (sess);
  /* notify app of reconsideration */
  if (sess->callbacks.reconsider)
    sess->callbacks.reconsider (sess, sess->reconsider_user_data);
  RTP_SESSION_LOCK (sess);
done:

  return result;
}

/**
 * rtp_session_schedule_bye:
 * @sess: an #RTPSession
 * @reason: a reason or NULL
 * @current_time: the current system time
 *
 * Stop the current @sess and schedule a BYE message for the other members.
 *
 * Returns: a #GstFlowReturn.
 */
GstFlowReturn
rtp_session_schedule_bye (RTPSession * sess, const gchar * reason,
    GstClockTime current_time)
{
  GstFlowReturn result = GST_FLOW_OK;

  g_return_val_if_fail (RTP_IS_SESSION (sess), GST_FLOW_ERROR);

  RTP_SESSION_LOCK (sess);
  result = rtp_session_schedule_bye_locked (sess, reason, current_time);
  RTP_SESSION_UNLOCK (sess);

  return result;
}

/**
 * rtp_session_next_timeout:
 * @sess: an #RTPSession
 * @current_time: the current system time
 *
 * Get the next time we should perform session maintenance tasks.
 *
 * Returns: a time when rtp_session_on_timeout() should be called with the
 * current system time.
 */
GstClockTime
rtp_session_next_timeout (RTPSession * sess, GstClockTime current_time)
{
  GstClockTime result;

  g_return_val_if_fail (RTP_IS_SESSION (sess), GST_FLOW_ERROR);

  RTP_SESSION_LOCK (sess);

  result = sess->next_rtcp_check_time;

  GST_DEBUG ("current time: %" GST_TIME_FORMAT ", next :%" GST_TIME_FORMAT,
      GST_TIME_ARGS (current_time), GST_TIME_ARGS (result));

  if (result < current_time) {
    GST_DEBUG ("take current time as base");
    /* our previous check time expired, start counting from the current time
     * again. */
    result = current_time;
  }

  if (sess->source->received_bye) {
    if (sess->sent_bye) {
      GST_DEBUG ("we sent BYE already");
      result = GST_CLOCK_TIME_NONE;
    } else if (sess->stats.active_sources >= 50) {
      GST_DEBUG ("reconsider BYE, more than 50 sources");
      /* reconsider BYE if members >= 50 */
      result += calculate_rtcp_interval (sess, FALSE, TRUE);
    }
  } else {
    if (sess->first_rtcp) {
      GST_DEBUG ("first RTCP packet");
      /* we are called for the first time */
      result += calculate_rtcp_interval (sess, FALSE, TRUE);
    } else if (sess->next_rtcp_check_time < current_time) {
      GST_DEBUG ("old check time expired, getting new timeout");
      /* get a new timeout when we need to */
      result += calculate_rtcp_interval (sess, FALSE, FALSE);
    }
  }
  sess->next_rtcp_check_time = result;

  GST_DEBUG ("next timeout: %" GST_TIME_FORMAT, GST_TIME_ARGS (result));
  RTP_SESSION_UNLOCK (sess);

  return result;
}

typedef struct
{
  RTPSession *sess;
  GstBuffer *rtcp;
  GstClockTime current_time;
  guint64 ntpnstime;
  GstClockTime interval;
  GstRTCPPacket packet;
  gboolean is_bye;
  gboolean has_sdes;
} ReportData;

static void
session_start_rtcp (RTPSession * sess, ReportData * data)
{
  GstRTCPPacket *packet = &data->packet;
  RTPSource *own = sess->source;

  data->rtcp = gst_rtcp_buffer_new (sess->mtu);

  if (RTP_SOURCE_IS_SENDER (own)) {
    guint64 ntptime;
    guint32 rtptime;
    guint32 packet_count, octet_count;

    /* we are a sender, create SR */
    GST_DEBUG ("create SR for SSRC %08x", own->ssrc);
    gst_rtcp_buffer_add_packet (data->rtcp, GST_RTCP_TYPE_SR, packet);

    /* get latest stats */
    rtp_source_get_new_sr (own, data->ntpnstime, &ntptime, &rtptime,
        &packet_count, &octet_count);
    /* store stats */
    rtp_source_process_sr (own, data->current_time, ntptime, rtptime,
        packet_count, octet_count);

    /* fill in sender report info */
    gst_rtcp_packet_sr_set_sender_info (packet, own->ssrc,
        ntptime, rtptime, packet_count, octet_count);
  } else {
    /* we are only receiver, create RR */
    GST_DEBUG ("create RR for SSRC %08x", own->ssrc);
    gst_rtcp_buffer_add_packet (data->rtcp, GST_RTCP_TYPE_RR, packet);
    gst_rtcp_packet_rr_set_ssrc (packet, own->ssrc);
  }
}

/* construct a Sender or Receiver Report */
static void
session_report_blocks (const gchar * key, RTPSource * source, ReportData * data)
{
  RTPSession *sess = data->sess;
  GstRTCPPacket *packet = &data->packet;

  /* create a new buffer if needed */
  if (data->rtcp == NULL) {
    session_start_rtcp (sess, data);
  }
  if (gst_rtcp_packet_get_rb_count (packet) < GST_RTCP_MAX_RB_COUNT) {
    /* only report about other sender sources */
    if (source != sess->source && RTP_SOURCE_IS_SENDER (source)) {
      guint8 fractionlost;
      gint32 packetslost;
      guint32 exthighestseq, jitter;
      guint32 lsr, dlsr;

      /* get new stats */
      rtp_source_get_new_rb (source, data->current_time, &fractionlost,
          &packetslost, &exthighestseq, &jitter, &lsr, &dlsr);

      /* packet is not yet filled, add report block for this source. */
      gst_rtcp_packet_add_rb (packet, source->ssrc, fractionlost, packetslost,
          exthighestseq, jitter, lsr, dlsr);
    }
  }
}

/* perform cleanup of sources that timed out */
static gboolean
session_cleanup (const gchar * key, RTPSource * source, ReportData * data)
{
  gboolean remove = FALSE;
  gboolean byetimeout = FALSE;
  gboolean sendertimeout = FALSE;
  gboolean is_sender, is_active;
  RTPSession *sess = data->sess;
  GstClockTime interval;

  is_sender = RTP_SOURCE_IS_SENDER (source);
  is_active = RTP_SOURCE_IS_ACTIVE (source);

  /* check for our own source, we don't want to delete our own source. */
  if (!(source == sess->source)) {
    if (source->received_bye) {
      /* if we received a BYE from the source, remove the source after some
       * time. */
      if (data->current_time > source->bye_time &&
          data->current_time - source->bye_time > sess->stats.bye_timeout) {
        GST_DEBUG ("removing BYE source %08x", source->ssrc);
        remove = TRUE;
        byetimeout = TRUE;
      }
    }
    /* sources that were inactive for more than 5 times the deterministic reporting
     * interval get timed out. the min timeout is 5 seconds. */
    if (data->current_time > source->last_activity) {
      interval = MAX (data->interval * 5, 5 * GST_SECOND);
      if (data->current_time - source->last_activity > interval) {
        GST_DEBUG ("removing timeout source %08x, last %" GST_TIME_FORMAT,
            source->ssrc, GST_TIME_ARGS (source->last_activity));
        remove = TRUE;
      }
    }
  }

  /* senders that did not send for a long time become a receiver, this also
   * holds for our own source. */
  if (is_sender) {
    if (data->current_time > source->last_rtp_activity) {
      interval = MAX (data->interval * 2, 5 * GST_SECOND);
      if (data->current_time - source->last_rtp_activity > interval) {
        GST_DEBUG ("sender source %08x timed out and became receiver, last %"
            GST_TIME_FORMAT, source->ssrc,
            GST_TIME_ARGS (source->last_rtp_activity));
        source->is_sender = FALSE;
        sess->stats.sender_sources--;
        sendertimeout = TRUE;
      }
    }
  }

  if (remove) {
    sess->total_sources--;
    if (is_sender)
      sess->stats.sender_sources--;
    if (is_active)
      sess->stats.active_sources--;

    if (byetimeout)
      on_bye_timeout (sess, source);
    else
      on_timeout (sess, source);
  } else {
    if (sendertimeout)
      on_sender_timeout (sess, source);
  }
  return remove;
}

static void
session_sdes (RTPSession * sess, ReportData * data)
{
  GstRTCPPacket *packet = &data->packet;
  guint8 *sdes_data;
  guint sdes_len;

  /* add SDES packet */
  gst_rtcp_buffer_add_packet (data->rtcp, GST_RTCP_TYPE_SDES, packet);

  gst_rtcp_packet_sdes_add_item (packet, sess->source->ssrc);

  rtp_source_get_sdes (sess->source, GST_RTCP_SDES_CNAME, &sdes_data,
      &sdes_len);
  gst_rtcp_packet_sdes_add_entry (packet, GST_RTCP_SDES_CNAME, sdes_len,
      sdes_data);

  /* other SDES items must only be added at regular intervals and only when the
   * user requests to since it might be a privacy problem */
#if 0
  gst_rtcp_packet_sdes_add_entry (&packet, GST_RTCP_SDES_NAME,
      strlen (sess->name), (guint8 *) sess->name);
  gst_rtcp_packet_sdes_add_entry (&packet, GST_RTCP_SDES_TOOL,
      strlen (sess->tool), (guint8 *) sess->tool);
#endif

  data->has_sdes = TRUE;
}

/* schedule a BYE packet */
static void
session_bye (RTPSession * sess, ReportData * data)
{
  GstRTCPPacket *packet = &data->packet;

  /* open packet */
  session_start_rtcp (sess, data);

  /* add SDES */
  session_sdes (sess, data);

  /* add a BYE packet */
  gst_rtcp_buffer_add_packet (data->rtcp, GST_RTCP_TYPE_BYE, packet);
  gst_rtcp_packet_bye_add_ssrc (packet, sess->source->ssrc);
  if (sess->bye_reason)
    gst_rtcp_packet_bye_set_reason (packet, sess->bye_reason);

  /* we have a BYE packet now */
  data->is_bye = TRUE;
}

static gboolean
is_rtcp_time (RTPSession * sess, GstClockTime current_time, ReportData * data)
{
  GstClockTime new_send_time, elapsed;
  gboolean result;

  /* no need to check yet */
  if (sess->next_rtcp_check_time > current_time) {
    GST_DEBUG ("no check time yet, next %" GST_TIME_FORMAT " > now %"
        GST_TIME_FORMAT, GST_TIME_ARGS (sess->next_rtcp_check_time),
        GST_TIME_ARGS (current_time));
    return FALSE;
  }

  /* get elapsed time since we last reported */
  elapsed = current_time - sess->last_rtcp_send_time;

  /* perform forward reconsideration */
  new_send_time = rtp_stats_add_rtcp_jitter (&sess->stats, data->interval);

  GST_DEBUG ("forward reconsideration %" GST_TIME_FORMAT ", elapsed %"
      GST_TIME_FORMAT, GST_TIME_ARGS (new_send_time), GST_TIME_ARGS (elapsed));

  new_send_time += sess->last_rtcp_send_time;

  /* check if reconsideration */
  if (current_time < new_send_time) {
    GST_DEBUG ("reconsider RTCP for %" GST_TIME_FORMAT,
        GST_TIME_ARGS (new_send_time));
    result = FALSE;
    /* store new check time */
    sess->next_rtcp_check_time = new_send_time;
  } else {
    result = TRUE;
    new_send_time = calculate_rtcp_interval (sess, FALSE, FALSE);

    GST_DEBUG ("can send RTCP now, next interval %" GST_TIME_FORMAT,
        GST_TIME_ARGS (new_send_time));
    sess->next_rtcp_check_time = current_time + new_send_time;
  }
  return result;
}

/**
 * rtp_session_on_timeout:
 * @sess: an #RTPSession
 * @current_time: the current system time
 * @ntpnstime: the current NTP time in nanoseconds
 *
 * Perform maintenance actions after the timeout obtained with
 * rtp_session_next_timeout() expired.
 *
 * This function will perform timeouts of receivers and senders, send a BYE
 * packet or generate RTCP packets with current session stats.
 *
 * This function can call the #RTPSessionSendRTCP callback, possibly multiple
 * times, for each packet that should be processed.
 *
 * Returns: a #GstFlowReturn.
 */
GstFlowReturn
rtp_session_on_timeout (RTPSession * sess, GstClockTime current_time,
    guint64 ntpnstime)
{
  GstFlowReturn result = GST_FLOW_OK;
  GList *item;
  ReportData data;
  RTPSource *own;

  g_return_val_if_fail (RTP_IS_SESSION (sess), GST_FLOW_ERROR);

  GST_DEBUG ("reporting at %" GST_TIME_FORMAT ", NTP time %" GST_TIME_FORMAT,
      GST_TIME_ARGS (current_time), GST_TIME_ARGS (ntpnstime));

  data.sess = sess;
  data.rtcp = NULL;
  data.current_time = current_time;
  data.ntpnstime = ntpnstime;
  data.is_bye = FALSE;
  data.has_sdes = FALSE;

  own = sess->source;

  RTP_SESSION_LOCK (sess);
  /* get a new interval, we need this for various cleanups etc */
  data.interval = calculate_rtcp_interval (sess, TRUE, sess->first_rtcp);

  /* first perform cleanups */
  g_hash_table_foreach_remove (sess->ssrcs[sess->mask_idx],
      (GHRFunc) session_cleanup, &data);

  /* see if we need to generate SR or RR packets */
  if (is_rtcp_time (sess, current_time, &data)) {
    if (own->received_bye) {
      /* generate BYE instead */
      GST_DEBUG ("generating BYE message");
      session_bye (sess, &data);
      sess->sent_bye = TRUE;
    } else {
      /* loop over all known sources and do something */
      g_hash_table_foreach (sess->ssrcs[sess->mask_idx],
          (GHFunc) session_report_blocks, &data);
    }
  }

  if (data.rtcp) {
    guint size;

    /* we keep track of the last report time in order to timeout inactive
     * receivers or senders */
    sess->last_rtcp_send_time = data.current_time;
    sess->first_rtcp = FALSE;

    /* add SDES for this source when not already added */
    if (!data.has_sdes)
      session_sdes (sess, &data);

    /* update average RTCP size before sending */
    size = GST_BUFFER_SIZE (data.rtcp) + sess->header_len;
    UPDATE_AVG (sess->stats.avg_rtcp_packet_size, size);
  }

  /* check for outdated collisions */
  GST_DEBUG ("checking collision list");
  item = g_list_first (sess->conflicting_addresses);
  while (item) {
    RTPConflictingAddress *known_conflict = item->data;
    GList *next_item = g_list_next (item);

    if (known_conflict->time < current_time - (data.interval *
            RTCP_INTERVAL_COLLISION_TIMEOUT)) {
      sess->conflicting_addresses =
          g_list_delete_link (sess->conflicting_addresses, item);
      GST_DEBUG ("collision %p timed out", known_conflict);
      g_free (known_conflict);
    }
    item = next_item;
  }

  if (sess->change_ssrc) {
    GST_DEBUG ("need to change our SSRC (%08x)", own->ssrc);
    g_hash_table_steal (sess->ssrcs[sess->mask_idx],
        GINT_TO_POINTER (own->ssrc));

    own->ssrc = rtp_session_create_new_ssrc (sess);
    rtp_source_reset (own);

    g_hash_table_insert (sess->ssrcs[sess->mask_idx],
        GINT_TO_POINTER (own->ssrc), own);

    g_free (sess->bye_reason);
    sess->bye_reason = NULL;
    sess->sent_bye = FALSE;
    sess->change_ssrc = FALSE;
    GST_DEBUG ("changed our SSRC to %08x", own->ssrc);
  }
  RTP_SESSION_UNLOCK (sess);

  /* push out the RTCP packet */
  if (data.rtcp) {
    /* close the RTCP packet */
    gst_rtcp_buffer_end (data.rtcp);

    GST_DEBUG ("sending packet");
    if (sess->callbacks.send_rtcp)
      result = sess->callbacks.send_rtcp (sess, own, data.rtcp,
          sess->sent_bye, sess->send_rtcp_user_data);
    else {
      GST_DEBUG ("freeing packet");
      gst_buffer_unref (data.rtcp);
    }
  }

  return result;
}
