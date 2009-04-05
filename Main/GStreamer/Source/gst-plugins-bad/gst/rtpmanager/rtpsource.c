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

#include "rtpsource.h"

GST_DEBUG_CATEGORY_STATIC (rtp_source_debug);
#define GST_CAT_DEFAULT rtp_source_debug

#define RTP_MAX_PROBATION_LEN	32

/* signals and args */
enum
{
  LAST_SIGNAL
};

#define DEFAULT_SSRC                 0
#define DEFAULT_IS_CSRC              FALSE
#define DEFAULT_IS_VALIDATED         FALSE
#define DEFAULT_IS_SENDER            FALSE
#define DEFAULT_SDES                 NULL

enum
{
  PROP_0,
  PROP_SSRC,
  PROP_IS_CSRC,
  PROP_IS_VALIDATED,
  PROP_IS_SENDER,
  PROP_SDES,
  PROP_STATS,
  PROP_LAST
};

/* GObject vmethods */
static void rtp_source_finalize (GObject * object);
static void rtp_source_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void rtp_source_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

/* static guint rtp_source_signals[LAST_SIGNAL] = { 0 }; */

G_DEFINE_TYPE (RTPSource, rtp_source, G_TYPE_OBJECT);

static void
rtp_source_class_init (RTPSourceClass * klass)
{
  GObjectClass *gobject_class;

  gobject_class = (GObjectClass *) klass;

  gobject_class->finalize = rtp_source_finalize;

  gobject_class->set_property = rtp_source_set_property;
  gobject_class->get_property = rtp_source_get_property;

  g_object_class_install_property (gobject_class, PROP_SSRC,
      g_param_spec_uint ("ssrc", "SSRC",
          "The SSRC of this source", 0, G_MAXUINT, DEFAULT_SSRC,
          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_IS_CSRC,
      g_param_spec_boolean ("is-csrc", "Is CSRC",
          "If this SSRC is acting as a contributing source",
          DEFAULT_IS_CSRC, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_IS_VALIDATED,
      g_param_spec_boolean ("is-validated", "Is Validated",
          "If this SSRC is validated", DEFAULT_IS_VALIDATED,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_IS_SENDER,
      g_param_spec_boolean ("is-sender", "Is Sender",
          "If this SSRC is a sender", DEFAULT_IS_SENDER,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  /**
   * RTPSource::sdes
   *
   * The current SDES items of the source. Returns a structure with the
   * following fields:
   *
   *  'cname'    G_TYPE_STRING  : The canonical name 
   *  'name'     G_TYPE_STRING  : The user name 
   *  'email'    G_TYPE_STRING  : The user's electronic mail address
   *  'phone'    G_TYPE_STRING  : The user's phone number
   *  'location' G_TYPE_STRING  : The geographic user location
   *  'tool'     G_TYPE_STRING  : The name of application or tool
   *  'note'     G_TYPE_STRING  : A notice about the source
   */
  g_object_class_install_property (gobject_class, PROP_SDES,
      g_param_spec_boxed ("sdes", "SDES",
          "The SDES information for this source",
          GST_TYPE_STRUCTURE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  /**
   * RTPSource::stats
   *
   * The statistics of the source. This property returns a GstStructure with
   * name application/x-rtp-source-stats with the following fields:
   * 
   */
  g_object_class_install_property (gobject_class, PROP_STATS,
      g_param_spec_boxed ("stats", "Stats",
          "The stats of this source", GST_TYPE_STRUCTURE,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  GST_DEBUG_CATEGORY_INIT (rtp_source_debug, "rtpsource", 0, "RTP Source");
}

/**
 * rtp_source_reset:
 * @src: an #RTPSource
 *
 * Reset the stats of @src.
 */
void
rtp_source_reset (RTPSource * src)
{
  src->received_bye = FALSE;

  src->stats.cycles = -1;
  src->stats.jitter = 0;
  src->stats.transit = -1;
  src->stats.curr_sr = 0;
  src->stats.curr_rr = 0;
}

static void
rtp_source_init (RTPSource * src)
{
  /* sources are initialy on probation until we receive enough valid RTP
   * packets or a valid RTCP packet */
  src->validated = FALSE;
  src->internal = FALSE;
  src->probation = RTP_DEFAULT_PROBATION;

  src->payload = -1;
  src->clock_rate = -1;
  src->packets = g_queue_new ();
  src->seqnum_base = -1;
  src->last_rtptime = -1;

  rtp_source_reset (src);
}

static void
rtp_source_finalize (GObject * object)
{
  RTPSource *src;
  GstBuffer *buffer;
  gint i;

  src = RTP_SOURCE_CAST (object);

  while ((buffer = g_queue_pop_head (src->packets)))
    gst_buffer_unref (buffer);
  g_queue_free (src->packets);

  for (i = 0; i < 9; i++)
    g_free (src->sdes[i]);

  g_free (src->bye_reason);

  gst_caps_replace (&src->caps, NULL);

  G_OBJECT_CLASS (rtp_source_parent_class)->finalize (object);
}

static GstStructure *
rtp_source_create_stats (RTPSource * src)
{
  GstStructure *s;
  gboolean is_sender = src->is_sender;
  gboolean internal = src->internal;

  /* common data for all types of sources */
  s = gst_structure_new ("application/x-rtp-source-stats",
      "ssrc", G_TYPE_UINT, (guint) src->ssrc,
      "internal", G_TYPE_BOOLEAN, internal,
      "validated", G_TYPE_BOOLEAN, src->validated,
      "received-bye", G_TYPE_BOOLEAN, src->received_bye,
      "is-csrc", G_TYPE_BOOLEAN, src->is_csrc,
      "is-sender", G_TYPE_BOOLEAN, is_sender, NULL);

  if (internal) {
    /* our internal source */
    if (is_sender) {
      /* if we are sending, report about how much we sent, other sources will
       * have a RB with info on reception. */
      gst_structure_set (s,
          "octets-sent", G_TYPE_UINT64, src->stats.octets_sent,
          "packets-sent", G_TYPE_UINT64, src->stats.packets_sent,
          "bitrate", G_TYPE_UINT64, src->bitrate, NULL);
    } else {
      /* if we are not sending we have nothing more to report */
    }
  } else {
    gboolean have_rb;
    guint8 fractionlost = 0;
    gint32 packetslost = 0;
    guint32 exthighestseq = 0;
    guint32 jitter = 0;
    guint32 lsr = 0;
    guint32 dlsr = 0;
    guint32 round_trip = 0;

    /* other sources */
    if (is_sender) {
      gboolean have_sr;
      GstClockTime time = 0;
      guint64 ntptime = 0;
      guint32 rtptime = 0;
      guint32 packet_count = 0;
      guint32 octet_count = 0;

      /* this source is sending to us, get the last SR. */
      have_sr = rtp_source_get_last_sr (src, &time, &ntptime, &rtptime,
          &packet_count, &octet_count);
      gst_structure_set (s,
          "octets-received", G_TYPE_UINT64, src->stats.octets_received,
          "packets-received", G_TYPE_UINT64, src->stats.packets_received,
          "have-sr", G_TYPE_BOOLEAN, have_sr,
          "sr-ntptime", G_TYPE_UINT64, ntptime,
          "sr-rtptime", G_TYPE_UINT, (guint) rtptime,
          "sr-octet-count", G_TYPE_UINT, (guint) octet_count,
          "sr-packet-count", G_TYPE_UINT, (guint) packet_count, NULL);
    }
    /* we might be sending to this SSRC so we report about how it is
     * receiving our data */
    have_rb = rtp_source_get_last_rb (src, &fractionlost, &packetslost,
        &exthighestseq, &jitter, &lsr, &dlsr, &round_trip);

    gst_structure_set (s,
        "have-rb", G_TYPE_BOOLEAN, have_rb,
        "rb-fractionlost", G_TYPE_UINT, (guint) fractionlost,
        "rb-packetslost", G_TYPE_INT, (gint) packetslost,
        "rb-exthighestseq", G_TYPE_UINT, (guint) exthighestseq,
        "rb-jitter", G_TYPE_UINT, (guint) jitter,
        "rb-lsr", G_TYPE_UINT, (guint) lsr,
        "rb-dlsr", G_TYPE_UINT, (guint) dlsr,
        "rb-round-trip", G_TYPE_UINT, (guint) round_trip, NULL);
  }

  return s;
}

static GstStructure *
rtp_source_create_sdes (RTPSource * src)
{
  GstStructure *s;
  gchar *str;

  s = gst_structure_new ("application/x-rtp-source-sdes", NULL);

  if ((str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_CNAME))) {
    gst_structure_set (s, "cname", G_TYPE_STRING, str, NULL);
    g_free (str);
  }
  if ((str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_NAME))) {
    gst_structure_set (s, "name", G_TYPE_STRING, str, NULL);
    g_free (str);
  }
  if ((str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_EMAIL))) {
    gst_structure_set (s, "email", G_TYPE_STRING, str, NULL);
    g_free (str);
  }
  if ((str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_PHONE))) {
    gst_structure_set (s, "phone", G_TYPE_STRING, str, NULL);
    g_free (str);
  }
  if ((str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_LOC))) {
    gst_structure_set (s, "location", G_TYPE_STRING, str, NULL);
    g_free (str);
  }
  if ((str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_TOOL))) {
    gst_structure_set (s, "tool", G_TYPE_STRING, str, NULL);
    g_free (str);
  }
  if ((str = rtp_source_get_sdes_string (src, GST_RTCP_SDES_NOTE))) {
    gst_structure_set (s, "note", G_TYPE_STRING, str, NULL);
    g_free (str);
  }
  return s;
}

static void
rtp_source_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  RTPSource *src;

  src = RTP_SOURCE (object);

  switch (prop_id) {
    case PROP_SSRC:
      src->ssrc = g_value_get_uint (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
rtp_source_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  RTPSource *src;

  src = RTP_SOURCE (object);

  switch (prop_id) {
    case PROP_SSRC:
      g_value_set_uint (value, rtp_source_get_ssrc (src));
      break;
    case PROP_IS_CSRC:
      g_value_set_boolean (value, rtp_source_is_as_csrc (src));
      break;
    case PROP_IS_VALIDATED:
      g_value_set_boolean (value, rtp_source_is_validated (src));
      break;
    case PROP_IS_SENDER:
      g_value_set_boolean (value, rtp_source_is_sender (src));
      break;
    case PROP_SDES:
      g_value_take_boxed (value, rtp_source_create_sdes (src));
      break;
    case PROP_STATS:
      g_value_take_boxed (value, rtp_source_create_stats (src));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/**
 * rtp_source_new:
 * @ssrc: an SSRC
 *
 * Create a #RTPSource with @ssrc.
 *
 * Returns: a new #RTPSource. Use g_object_unref() after usage.
 */
RTPSource *
rtp_source_new (guint32 ssrc)
{
  RTPSource *src;

  src = g_object_new (RTP_TYPE_SOURCE, NULL);
  src->ssrc = ssrc;

  return src;
}

/**
 * rtp_source_set_callbacks:
 * @src: an #RTPSource
 * @cb: callback functions
 * @user_data: user data
 *
 * Set the callbacks for the source.
 */
void
rtp_source_set_callbacks (RTPSource * src, RTPSourceCallbacks * cb,
    gpointer user_data)
{
  g_return_if_fail (RTP_IS_SOURCE (src));

  src->callbacks.push_rtp = cb->push_rtp;
  src->callbacks.clock_rate = cb->clock_rate;
  src->user_data = user_data;
}

/**
 * rtp_source_get_ssrc:
 * @src: an #RTPSource
 *
 * Get the SSRC of @source.
 *
 * Returns: the SSRC of src.
 */
guint32
rtp_source_get_ssrc (RTPSource * src)
{
  guint32 result;

  g_return_val_if_fail (RTP_IS_SOURCE (src), 0);

  result = src->ssrc;

  return result;
}

/**
 * rtp_source_set_as_csrc:
 * @src: an #RTPSource
 *
 * Configure @src as a CSRC, this will also validate @src.
 */
void
rtp_source_set_as_csrc (RTPSource * src)
{
  g_return_if_fail (RTP_IS_SOURCE (src));

  src->validated = TRUE;
  src->is_csrc = TRUE;
}

/**
 * rtp_source_is_as_csrc:
 * @src: an #RTPSource
 *
 * Check if @src is a contributing source.
 *
 * Returns: %TRUE if @src is acting as a contributing source.
 */
gboolean
rtp_source_is_as_csrc (RTPSource * src)
{
  gboolean result;

  g_return_val_if_fail (RTP_IS_SOURCE (src), FALSE);

  result = src->is_csrc;

  return result;
}

/**
 * rtp_source_is_active:
 * @src: an #RTPSource
 *
 * Check if @src is an active source. A source is active if it has been
 * validated and has not yet received a BYE packet
 *
 * Returns: %TRUE if @src is an qactive source.
 */
gboolean
rtp_source_is_active (RTPSource * src)
{
  gboolean result;

  g_return_val_if_fail (RTP_IS_SOURCE (src), FALSE);

  result = RTP_SOURCE_IS_ACTIVE (src);

  return result;
}

/**
 * rtp_source_is_validated:
 * @src: an #RTPSource
 *
 * Check if @src is a validated source.
 *
 * Returns: %TRUE if @src is a validated source.
 */
gboolean
rtp_source_is_validated (RTPSource * src)
{
  gboolean result;

  g_return_val_if_fail (RTP_IS_SOURCE (src), FALSE);

  result = src->validated;

  return result;
}

/**
 * rtp_source_is_sender:
 * @src: an #RTPSource
 *
 * Check if @src is a sending source.
 *
 * Returns: %TRUE if @src is a sending source.
 */
gboolean
rtp_source_is_sender (RTPSource * src)
{
  gboolean result;

  g_return_val_if_fail (RTP_IS_SOURCE (src), FALSE);

  result = RTP_SOURCE_IS_SENDER (src);

  return result;
}

/**
 * rtp_source_received_bye:
 * @src: an #RTPSource
 *
 * Check if @src has receoved a BYE packet.
 *
 * Returns: %TRUE if @src has received a BYE packet.
 */
gboolean
rtp_source_received_bye (RTPSource * src)
{
  gboolean result;

  g_return_val_if_fail (RTP_IS_SOURCE (src), FALSE);

  result = src->received_bye;

  return result;
}


/**
 * rtp_source_get_bye_reason:
 * @src: an #RTPSource
 *
 * Get the BYE reason for @src. Check if the source receoved a BYE message first
 * with rtp_source_received_bye().
 *
 * Returns: The BYE reason or NULL when no reason was given or the source did
 * not receive a BYE message yet. g_fee() after usage.
 */
gchar *
rtp_source_get_bye_reason (RTPSource * src)
{
  gchar *result;

  g_return_val_if_fail (RTP_IS_SOURCE (src), NULL);

  result = g_strdup (src->bye_reason);

  return result;
}

/**
 * rtp_source_update_caps:
 * @src: an #RTPSource
 * @caps: a #GstCaps
 *
 * Parse @caps and store all relevant information in @source.
 */
void
rtp_source_update_caps (RTPSource * src, GstCaps * caps)
{
  GstStructure *s;
  guint val;
  gint ival;

  /* nothing changed, return */
  if (src->caps == caps)
    return;

  s = gst_caps_get_structure (caps, 0);

  if (gst_structure_get_int (s, "payload", &ival))
    src->payload = ival;
  else
    src->payload = -1;
  GST_DEBUG ("got payload %d", src->payload);

  if (gst_structure_get_int (s, "clock-rate", &ival))
    src->clock_rate = ival;
  else
    src->clock_rate = -1;

  GST_DEBUG ("got clock-rate %d", src->clock_rate);

  if (gst_structure_get_uint (s, "seqnum-base", &val))
    src->seqnum_base = val;
  else
    src->seqnum_base = -1;

  GST_DEBUG ("got seqnum-base %" G_GINT32_FORMAT, src->seqnum_base);

  gst_caps_replace (&src->caps, caps);
}

/**
 * rtp_source_set_sdes:
 * @src: an #RTPSource
 * @type: the type of the SDES item
 * @data: the SDES data
 * @len: the SDES length
 *
 * Store an SDES item of @type in @src. 
 *
 * Returns: %FALSE if the SDES item was unchanged or @type is unknown.
 */
gboolean
rtp_source_set_sdes (RTPSource * src, GstRTCPSDESType type,
    const guint8 * data, guint len)
{
  guint8 *old;

  g_return_val_if_fail (RTP_IS_SOURCE (src), FALSE);

  if (type < 0 || type > GST_RTCP_SDES_PRIV)
    return FALSE;

  old = src->sdes[type];

  /* lengths are the same, check if the data is the same */
  if ((src->sdes_len[type] == len))
    if (data != NULL && old != NULL && (memcmp (old, data, len) == 0))
      return FALSE;

  /* NULL data, make sure we store 0 length or if no length is given,
   * take strlen */
  if (data == NULL)
    len = 0;

  g_free (src->sdes[type]);
  src->sdes[type] = g_memdup (data, len);
  src->sdes_len[type] = len;

  return TRUE;
}

/**
 * rtp_source_set_sdes_string:
 * @src: an #RTPSource
 * @type: the type of the SDES item
 * @data: the SDES data
 *
 * Store an SDES item of @type in @src. This function is similar to
 * rtp_source_set_sdes() but takes a null-terminated string for convenience.
 *
 * Returns: %FALSE if the SDES item was unchanged or @type is unknown.
 */
gboolean
rtp_source_set_sdes_string (RTPSource * src, GstRTCPSDESType type,
    const gchar * data)
{
  guint len;
  gboolean result;

  if (data)
    len = strlen (data);
  else
    len = 0;

  result = rtp_source_set_sdes (src, type, (guint8 *) data, len);

  return result;
}

/**
 * rtp_source_get_sdes:
 * @src: an #RTPSource
 * @type: the type of the SDES item
 * @data: location to store the SDES data or NULL
 * @len: location to store the SDES length or NULL
 *
 * Get the SDES item of @type from @src. Note that @data does not always point
 * to a null-terminated string, use rtp_source_get_sdes_string() to retrieve a
 * null-terminated string instead.
 *
 * @data remains valid until the next call to rtp_source_set_sdes().
 *
 * Returns: %TRUE if @type was valid and @data and @len contain valid
 * data. @data can be NULL when the item was unset.
 */
gboolean
rtp_source_get_sdes (RTPSource * src, GstRTCPSDESType type, guint8 ** data,
    guint * len)
{
  g_return_val_if_fail (RTP_IS_SOURCE (src), FALSE);

  if (type < 0 || type > GST_RTCP_SDES_PRIV)
    return FALSE;

  if (data)
    *data = src->sdes[type];
  if (len)
    *len = src->sdes_len[type];

  return TRUE;
}

/**
 * rtp_source_get_sdes_string:
 * @src: an #RTPSource
 * @type: the type of the SDES item
 *
 * Get the SDES item of @type from @src. 
 *
 * Returns: a null-terminated copy of the SDES item or NULL when @type was not
 * valid or the SDES item was unset. g_free() after usage.
 */
gchar *
rtp_source_get_sdes_string (RTPSource * src, GstRTCPSDESType type)
{
  gchar *result;

  g_return_val_if_fail (RTP_IS_SOURCE (src), NULL);

  if (type < 0 || type > GST_RTCP_SDES_PRIV)
    return NULL;

  result = g_strndup ((const gchar *) src->sdes[type], src->sdes_len[type]);

  return result;
}

/**
 * rtp_source_set_rtp_from:
 * @src: an #RTPSource
 * @address: the RTP address to set
 *
 * Set that @src is receiving RTP packets from @address. This is used for
 * collistion checking.
 */
void
rtp_source_set_rtp_from (RTPSource * src, GstNetAddress * address)
{
  g_return_if_fail (RTP_IS_SOURCE (src));

  src->have_rtp_from = TRUE;
  memcpy (&src->rtp_from, address, sizeof (GstNetAddress));
}

/**
 * rtp_source_set_rtcp_from:
 * @src: an #RTPSource
 * @address: the RTCP address to set
 *
 * Set that @src is receiving RTCP packets from @address. This is used for
 * collistion checking.
 */
void
rtp_source_set_rtcp_from (RTPSource * src, GstNetAddress * address)
{
  g_return_if_fail (RTP_IS_SOURCE (src));

  src->have_rtcp_from = TRUE;
  memcpy (&src->rtcp_from, address, sizeof (GstNetAddress));
}

static GstFlowReturn
push_packet (RTPSource * src, GstBuffer * buffer)
{
  GstFlowReturn ret = GST_FLOW_OK;

  /* push queued packets first if any */
  while (!g_queue_is_empty (src->packets)) {
    GstBuffer *buffer = GST_BUFFER_CAST (g_queue_pop_head (src->packets));

    GST_LOG ("pushing queued packet");
    if (src->callbacks.push_rtp)
      src->callbacks.push_rtp (src, buffer, src->user_data);
    else
      gst_buffer_unref (buffer);
  }
  GST_LOG ("pushing new packet");
  /* push packet */
  if (src->callbacks.push_rtp)
    ret = src->callbacks.push_rtp (src, buffer, src->user_data);
  else
    gst_buffer_unref (buffer);

  return ret;
}

static gint
get_clock_rate (RTPSource * src, guint8 payload)
{
  if (src->payload == -1) {
    /* first payload received, nothing was in the caps, lock on to this payload */
    src->payload = payload;
    GST_DEBUG ("first payload %d", payload);
  } else if (payload != src->payload) {
    /* we have a different payload than before, reset the clock-rate */
    GST_DEBUG ("new payload %d", payload);
    src->payload = payload;
    src->clock_rate = -1;
    src->stats.transit = -1;
  }

  if (src->clock_rate == -1) {
    gint clock_rate = -1;

    if (src->callbacks.clock_rate)
      clock_rate = src->callbacks.clock_rate (src, payload, src->user_data);

    GST_DEBUG ("got clock-rate %d", clock_rate);

    src->clock_rate = clock_rate;
  }
  return src->clock_rate;
}

/* Jitter is the variation in the delay of received packets in a flow. It is
 * measured by comparing the interval when RTP packets were sent to the interval
 * at which they were received. For instance, if packet #1 and packet #2 leave
 * 50 milliseconds apart and arrive 60 milliseconds apart, then the jitter is 10
 * milliseconds. */
static void
calculate_jitter (RTPSource * src, GstBuffer * buffer,
    RTPArrivalStats * arrival)
{
  guint64 ntpnstime;
  guint32 rtparrival, transit, rtptime;
  gint32 diff;
  gint clock_rate;
  guint8 pt;

  /* get arrival time */
  if ((ntpnstime = arrival->ntpnstime) == GST_CLOCK_TIME_NONE)
    goto no_time;

  pt = gst_rtp_buffer_get_payload_type (buffer);

  GST_LOG ("SSRC %08x got payload %d", src->ssrc, pt);

  /* get clockrate */
  if ((clock_rate = get_clock_rate (src, pt)) == -1)
    goto no_clock_rate;

  rtptime = gst_rtp_buffer_get_timestamp (buffer);

  /* convert arrival time to RTP timestamp units, truncate to 32 bits, we don't
   * care about the absolute value, just the difference. */
  rtparrival = gst_util_uint64_scale_int (ntpnstime, clock_rate, GST_SECOND);

  /* transit time is difference with RTP timestamp */
  transit = rtparrival - rtptime;

  /* get ABS diff with previous transit time */
  if (src->stats.transit != -1) {
    if (transit > src->stats.transit)
      diff = transit - src->stats.transit;
    else
      diff = src->stats.transit - transit;
  } else
    diff = 0;

  src->stats.transit = transit;

  /* update jitter, the value we store is scaled up so we can keep precision. */
  src->stats.jitter += diff - ((src->stats.jitter + 8) >> 4);

  src->stats.prev_rtptime = src->stats.last_rtptime;
  src->stats.last_rtptime = rtparrival;

  GST_LOG ("rtparrival %u, rtptime %u, clock-rate %d, diff %d, jitter: %f",
      rtparrival, rtptime, clock_rate, diff, (src->stats.jitter) / 16.0);

  return;

  /* ERRORS */
no_time:
  {
    GST_WARNING ("cannot get current time");
    return;
  }
no_clock_rate:
  {
    GST_WARNING ("cannot get clock-rate for pt %d", pt);
    return;
  }
}

static void
init_seq (RTPSource * src, guint16 seq)
{
  src->stats.base_seq = seq;
  src->stats.max_seq = seq;
  src->stats.bad_seq = RTP_SEQ_MOD + 1; /* so seq == bad_seq is false */
  src->stats.cycles = 0;
  src->stats.packets_received = 0;
  src->stats.octets_received = 0;
  src->stats.bytes_received = 0;
  src->stats.prev_received = 0;
  src->stats.prev_expected = 0;

  GST_DEBUG ("base_seq %d", seq);
}

/**
 * rtp_source_process_rtp:
 * @src: an #RTPSource
 * @buffer: an RTP buffer
 *
 * Let @src handle the incomming RTP @buffer.
 *
 * Returns: a #GstFlowReturn.
 */
GstFlowReturn
rtp_source_process_rtp (RTPSource * src, GstBuffer * buffer,
    RTPArrivalStats * arrival)
{
  GstFlowReturn result = GST_FLOW_OK;
  guint16 seqnr, udelta;
  RTPSourceStats *stats;

  g_return_val_if_fail (RTP_IS_SOURCE (src), GST_FLOW_ERROR);
  g_return_val_if_fail (GST_IS_BUFFER (buffer), GST_FLOW_ERROR);

  stats = &src->stats;

  seqnr = gst_rtp_buffer_get_seq (buffer);

  rtp_source_update_caps (src, GST_BUFFER_CAPS (buffer));

  if (stats->cycles == -1) {
    GST_DEBUG ("received first buffer");
    /* first time we heard of this source */
    init_seq (src, seqnr);
    src->stats.max_seq = seqnr - 1;
    src->probation = RTP_DEFAULT_PROBATION;
  }

  udelta = seqnr - stats->max_seq;

  /* if we are still on probation, check seqnum */
  if (src->probation) {
    guint16 expected;

    expected = src->stats.max_seq + 1;

    /* when in probation, we require consecutive seqnums */
    if (seqnr == expected) {
      /* expected packet */
      GST_DEBUG ("probation: seqnr %d == expected %d", seqnr, expected);
      src->probation--;
      src->stats.max_seq = seqnr;
      if (src->probation == 0) {
        GST_DEBUG ("probation done!");
        init_seq (src, seqnr);
      } else {
        GstBuffer *q;

        GST_DEBUG ("probation %d: queue buffer", src->probation);
        /* when still in probation, keep packets in a list. */
        g_queue_push_tail (src->packets, buffer);
        /* remove packets from queue if there are too many */
        while (g_queue_get_length (src->packets) > RTP_MAX_PROBATION_LEN) {
          q = g_queue_pop_head (src->packets);
          gst_buffer_unref (q);
        }
        goto done;
      }
    } else {
      GST_DEBUG ("probation: seqnr %d != expected %d", seqnr, expected);
      src->probation = RTP_DEFAULT_PROBATION;
      src->stats.max_seq = seqnr;
      goto done;
    }
  } else if (udelta < RTP_MAX_DROPOUT) {
    /* in order, with permissible gap */
    if (seqnr < stats->max_seq) {
      /* sequence number wrapped - count another 64K cycle. */
      stats->cycles += RTP_SEQ_MOD;
    }
    stats->max_seq = seqnr;
  } else if (udelta <= RTP_SEQ_MOD - RTP_MAX_MISORDER) {
    /* the sequence number made a very large jump */
    if (seqnr == stats->bad_seq) {
      /* two sequential packets -- assume that the other side
       * restarted without telling us so just re-sync
       * (i.e., pretend this was the first packet).  */
      init_seq (src, seqnr);
    } else {
      /* unacceptable jump */
      stats->bad_seq = (seqnr + 1) & (RTP_SEQ_MOD - 1);
      goto bad_sequence;
    }
  } else {
    /* duplicate or reordered packet, will be filtered by jitterbuffer. */
    GST_WARNING ("duplicate or reordered packet");
  }

  src->stats.octets_received += arrival->payload_len;
  src->stats.bytes_received += arrival->bytes;
  src->stats.packets_received++;
  /* the source that sent the packet must be a sender */
  src->is_sender = TRUE;
  src->validated = TRUE;

  GST_LOG ("seq %d, PC: %" G_GUINT64_FORMAT ", OC: %" G_GUINT64_FORMAT,
      seqnr, src->stats.packets_received, src->stats.octets_received);

  /* calculate jitter for the stats */
  calculate_jitter (src, buffer, arrival);

  /* we're ready to push the RTP packet now */
  result = push_packet (src, buffer);

done:
  return result;

  /* ERRORS */
bad_sequence:
  {
    GST_WARNING ("unacceptable seqnum received");
    return GST_FLOW_OK;
  }
}

/**
 * rtp_source_process_bye:
 * @src: an #RTPSource
 * @reason: the reason for leaving
 *
 * Notify @src that a BYE packet has been received. This will make the source
 * inactive.
 */
void
rtp_source_process_bye (RTPSource * src, const gchar * reason)
{
  g_return_if_fail (RTP_IS_SOURCE (src));

  GST_DEBUG ("marking SSRC %08x as BYE, reason: %s", src->ssrc,
      GST_STR_NULL (reason));

  /* copy the reason and mark as received_bye */
  g_free (src->bye_reason);
  src->bye_reason = g_strdup (reason);
  src->received_bye = TRUE;
}

/**
 * rtp_source_send_rtp:
 * @src: an #RTPSource
 * @buffer: an RTP buffer
 * @ntpnstime: the NTP time when this buffer was captured in nanoseconds. This
 * is the buffer timestamp converted to NTP time.
 *
 * Send an RTP @buffer originating from @src. This will make @src a sender.
 * This function takes ownership of @buffer and modifies the SSRC in the RTP
 * packet to that of @src when needed.
 *
 * Returns: a #GstFlowReturn.
 */
GstFlowReturn
rtp_source_send_rtp (RTPSource * src, GstBuffer * buffer, guint64 ntpnstime)
{
  GstFlowReturn result = GST_FLOW_OK;
  guint len;
  guint32 rtptime;
  guint64 ext_rtptime;
  guint64 ntp_diff, rtp_diff;
  guint64 elapsed;

  g_return_val_if_fail (RTP_IS_SOURCE (src), GST_FLOW_ERROR);
  g_return_val_if_fail (GST_IS_BUFFER (buffer), GST_FLOW_ERROR);

  len = gst_rtp_buffer_get_payload_len (buffer);

  rtp_source_update_caps (src, GST_BUFFER_CAPS (buffer));

  /* we are a sender now */
  src->is_sender = TRUE;

  /* update stats for the SR */
  src->stats.packets_sent++;
  src->stats.octets_sent += len;
  src->bytes_sent += len;

  if (src->prev_ntpnstime) {
    elapsed = ntpnstime - src->prev_ntpnstime;

    if (elapsed > (G_GINT64_CONSTANT (1) << 31)) {
      guint64 rate;

      rate =
          gst_util_uint64_scale (src->bytes_sent, elapsed,
          (G_GINT64_CONSTANT (1) << 29));

      GST_LOG ("Elapsed %" G_GUINT64_FORMAT ", bytes %" G_GUINT64_FORMAT
          ", rate %" G_GUINT64_FORMAT, elapsed, src->bytes_sent, rate);

      if (src->bitrate == 0)
        src->bitrate = rate;
      else
        src->bitrate = ((src->bitrate * 3) + rate) / 4;

      src->prev_ntpnstime = ntpnstime;
      src->bytes_sent = 0;
    }
  } else {
    GST_LOG ("Reset bitrate measurement");
    src->prev_ntpnstime = ntpnstime;
    src->bitrate = 0;
  }

  rtptime = gst_rtp_buffer_get_timestamp (buffer);
  ext_rtptime = src->last_rtptime;
  ext_rtptime = gst_rtp_buffer_ext_timestamp (&ext_rtptime, rtptime);

  GST_LOG ("SSRC %08x, RTP %" G_GUINT64_FORMAT ", NTP %" GST_TIME_FORMAT,
      src->ssrc, ext_rtptime, GST_TIME_ARGS (ntpnstime));

  if (ext_rtptime > src->last_rtptime) {
    rtp_diff = ext_rtptime - src->last_rtptime;
    ntp_diff = ntpnstime - src->last_ntpnstime;

    /* calc the diff so we can detect drift at the sender. This can also be used
     * to guestimate the clock rate if the NTP time is locked to the RTP
     * timestamps (as is the case when the capture device is providing the clock). */
    GST_LOG ("SSRC %08x, diff RTP %" G_GUINT64_FORMAT ", diff NTP %"
        GST_TIME_FORMAT, src->ssrc, rtp_diff, GST_TIME_ARGS (ntp_diff));
  }

  /* we keep track of the last received RTP timestamp and the corresponding
   * NTP timestamp so that we can use this info when constructing SR reports */
  src->last_rtptime = ext_rtptime;
  src->last_ntpnstime = ntpnstime;

  /* push packet */
  if (src->callbacks.push_rtp) {
    guint32 ssrc;

    ssrc = gst_rtp_buffer_get_ssrc (buffer);
    if (ssrc != src->ssrc) {
      /* the SSRC of the packet is not correct, make a writable buffer and
       * update the SSRC. This could involve a complete copy of the packet when
       * it is not writable. Usually the payloader will use caps negotiation to
       * get the correct SSRC from the session manager before pushing anything. */
      buffer = gst_buffer_make_writable (buffer);

      GST_WARNING ("updating SSRC from %08x to %08x, fix the payloader", ssrc,
          src->ssrc);
      gst_rtp_buffer_set_ssrc (buffer, src->ssrc);
    }
    GST_LOG ("pushing RTP packet %" G_GUINT64_FORMAT, src->stats.packets_sent);
    result = src->callbacks.push_rtp (src, buffer, src->user_data);
  } else {
    GST_WARNING ("no callback installed, dropping packet");
    gst_buffer_unref (buffer);
  }

  return result;
}

/**
 * rtp_source_process_sr:
 * @src: an #RTPSource
 * @time: time of packet arrival
 * @ntptime: the NTP time in 32.32 fixed point
 * @rtptime: the RTP time
 * @packet_count: the packet count
 * @octet_count: the octect count
 *
 * Update the sender report in @src.
 */
void
rtp_source_process_sr (RTPSource * src, GstClockTime time, guint64 ntptime,
    guint32 rtptime, guint32 packet_count, guint32 octet_count)
{
  RTPSenderReport *curr;
  gint curridx;

  g_return_if_fail (RTP_IS_SOURCE (src));

  GST_DEBUG ("got SR packet: SSRC %08x, NTP %08x:%08x, RTP %" G_GUINT32_FORMAT
      ", PC %" G_GUINT32_FORMAT ", OC %" G_GUINT32_FORMAT, src->ssrc,
      (guint32) (ntptime >> 32), (guint32) (ntptime & 0xffffffff), rtptime,
      packet_count, octet_count);

  curridx = src->stats.curr_sr ^ 1;
  curr = &src->stats.sr[curridx];

  /* this is a sender now */
  src->is_sender = TRUE;

  /* update current */
  curr->is_valid = TRUE;
  curr->ntptime = ntptime;
  curr->rtptime = rtptime;
  curr->packet_count = packet_count;
  curr->octet_count = octet_count;
  curr->time = time;

  /* make current */
  src->stats.curr_sr = curridx;
}

/**
 * rtp_source_process_rb:
 * @src: an #RTPSource
 * @time: the current time in nanoseconds since 1970
 * @fractionlost: fraction lost since last SR/RR
 * @packetslost: the cumululative number of packets lost
 * @exthighestseq: the extended last sequence number received
 * @jitter: the interarrival jitter
 * @lsr: the last SR packet from this source
 * @dlsr: the delay since last SR packet
 *
 * Update the report block in @src.
 */
void
rtp_source_process_rb (RTPSource * src, GstClockTime time, guint8 fractionlost,
    gint32 packetslost, guint32 exthighestseq, guint32 jitter, guint32 lsr,
    guint32 dlsr)
{
  RTPReceiverReport *curr;
  gint curridx;
  guint32 ntp, A;

  g_return_if_fail (RTP_IS_SOURCE (src));

  GST_DEBUG ("got RB packet: SSRC %08x, FL %2x, PL %d, HS %" G_GUINT32_FORMAT
      ", jitter %" G_GUINT32_FORMAT ", LSR %04x:%04x, DLSR %04x:%04x",
      src->ssrc, fractionlost, packetslost, exthighestseq, jitter, lsr >> 16,
      lsr & 0xffff, dlsr >> 16, dlsr & 0xffff);

  curridx = src->stats.curr_rr ^ 1;
  curr = &src->stats.rr[curridx];

  /* update current */
  curr->is_valid = TRUE;
  curr->fractionlost = fractionlost;
  curr->packetslost = packetslost;
  curr->exthighestseq = exthighestseq;
  curr->jitter = jitter;
  curr->lsr = lsr;
  curr->dlsr = dlsr;

  /* calculate round trip, round the time up */
  ntp = ((gst_rtcp_unix_to_ntp (time) + 0xffff) >> 16) & 0xffffffff;
  A = dlsr + lsr;
  if (A > 0 && ntp > A)
    A = ntp - A;
  else
    A = 0;
  curr->round_trip = A;

  GST_DEBUG ("NTP %04x:%04x, round trip %04x:%04x", ntp >> 16, ntp & 0xffff,
      A >> 16, A & 0xffff);

  /* make current */
  src->stats.curr_rr = curridx;
}

/**
 * rtp_source_get_new_sr:
 * @src: an #RTPSource
 * @ntpnstime: the current time in nanoseconds since 1970
 * @ntptime: the NTP time in 32.32 fixed point
 * @rtptime: the RTP time corresponding to @ntptime
 * @packet_count: the packet count
 * @octet_count: the octect count
 *
 * Get new values to put into a new SR report from this source.
 *
 * Returns: %TRUE on success.
 */
gboolean
rtp_source_get_new_sr (RTPSource * src, guint64 ntpnstime,
    guint64 * ntptime, guint32 * rtptime, guint32 * packet_count,
    guint32 * octet_count)
{
  guint64 t_rtp;
  guint64 t_current_ntp;
  GstClockTimeDiff diff;

  g_return_val_if_fail (RTP_IS_SOURCE (src), FALSE);

  /* use the sync params to interpolate the date->time member to rtptime. We
   * use the last sent timestamp and rtptime as reference points. We assume
   * that the slope of the rtptime vs timestamp curve is 1, which is certainly
   * sufficient for the frequency at which we report SR and the rate we send
   * out RTP packets. */
  t_rtp = src->last_rtptime;

  GST_DEBUG ("last_ntpnstime %" GST_TIME_FORMAT ", last_rtptime %"
      G_GUINT64_FORMAT, GST_TIME_ARGS (src->last_ntpnstime), t_rtp);

  if (src->clock_rate != -1) {
    /* get the diff with the SR time */
    diff = GST_CLOCK_DIFF (src->last_ntpnstime, ntpnstime);

    /* now translate the diff to RTP time, handle positive and negative cases.
     * If there is no diff, we already set rtptime correctly above. */
    if (diff > 0) {
      GST_DEBUG ("ntpnstime %" GST_TIME_FORMAT ", diff %" GST_TIME_FORMAT,
          GST_TIME_ARGS (ntpnstime), GST_TIME_ARGS (diff));
      t_rtp += gst_util_uint64_scale_int (diff, src->clock_rate, GST_SECOND);
    } else {
      diff = -diff;
      GST_DEBUG ("ntpnstime %" GST_TIME_FORMAT ", diff -%" GST_TIME_FORMAT,
          GST_TIME_ARGS (ntpnstime), GST_TIME_ARGS (diff));
      t_rtp -= gst_util_uint64_scale_int (diff, src->clock_rate, GST_SECOND);
    }
  } else {
    GST_WARNING ("no clock-rate, cannot interpolate rtp time");
  }

  /* convert the NTP time in nanoseconds to 32.32 fixed point */
  t_current_ntp = gst_util_uint64_scale (ntpnstime, (1LL << 32), GST_SECOND);

  GST_DEBUG ("NTP %08x:%08x, RTP %" G_GUINT32_FORMAT,
      (guint32) (t_current_ntp >> 32), (guint32) (t_current_ntp & 0xffffffff),
      (guint32) t_rtp);

  if (ntptime)
    *ntptime = t_current_ntp;
  if (rtptime)
    *rtptime = t_rtp;
  if (packet_count)
    *packet_count = src->stats.packets_sent;
  if (octet_count)
    *octet_count = src->stats.octets_sent;

  return TRUE;
}

/**
 * rtp_source_get_new_rb:
 * @src: an #RTPSource
 * @time: the current time of the system clock
 * @fractionlost: fraction lost since last SR/RR
 * @packetslost: the cumululative number of packets lost
 * @exthighestseq: the extended last sequence number received
 * @jitter: the interarrival jitter
 * @lsr: the last SR packet from this source
 * @dlsr: the delay since last SR packet
 *
 * Get new values to put into a new report block from this source.
 *
 * Returns: %TRUE on success.
 */
gboolean
rtp_source_get_new_rb (RTPSource * src, GstClockTime time,
    guint8 * fractionlost, gint32 * packetslost, guint32 * exthighestseq,
    guint32 * jitter, guint32 * lsr, guint32 * dlsr)
{
  RTPSourceStats *stats;
  guint64 extended_max, expected;
  guint64 expected_interval, received_interval, ntptime;
  gint64 lost, lost_interval;
  guint32 fraction, LSR, DLSR;
  GstClockTime sr_time;

  stats = &src->stats;

  extended_max = stats->cycles + stats->max_seq;
  expected = extended_max - stats->base_seq + 1;

  GST_DEBUG ("ext_max %" G_GUINT64_FORMAT ", expected %" G_GUINT64_FORMAT
      ", received %" G_GUINT64_FORMAT ", base_seq %" G_GUINT32_FORMAT,
      extended_max, expected, stats->packets_received, stats->base_seq);

  lost = expected - stats->packets_received;
  lost = CLAMP (lost, -0x800000, 0x7fffff);

  expected_interval = expected - stats->prev_expected;
  stats->prev_expected = expected;
  received_interval = stats->packets_received - stats->prev_received;
  stats->prev_received = stats->packets_received;

  lost_interval = expected_interval - received_interval;

  if (expected_interval == 0 || lost_interval <= 0)
    fraction = 0;
  else
    fraction = (lost_interval << 8) / expected_interval;

  GST_DEBUG ("add RR for SSRC %08x", src->ssrc);
  /* we scaled the jitter up for additional precision */
  GST_DEBUG ("fraction %" G_GUINT32_FORMAT ", lost %" G_GINT64_FORMAT
      ", extseq %" G_GUINT64_FORMAT ", jitter %d", fraction, lost,
      extended_max, stats->jitter >> 4);

  if (rtp_source_get_last_sr (src, &sr_time, &ntptime, NULL, NULL, NULL)) {
    GstClockTime diff;

    /* LSR is middle 32 bits of the last ntptime */
    LSR = (ntptime >> 16) & 0xffffffff;
    diff = time - sr_time;
    GST_DEBUG ("last SR time diff %" GST_TIME_FORMAT, GST_TIME_ARGS (diff));
    /* DLSR, delay since last SR is expressed in 1/65536 second units */
    DLSR = gst_util_uint64_scale_int (diff, 65536, GST_SECOND);
  } else {
    /* No valid SR received, LSR/DLSR are set to 0 then */
    GST_DEBUG ("no valid SR received");
    LSR = 0;
    DLSR = 0;
  }
  GST_DEBUG ("LSR %04x:%04x, DLSR %04x:%04x", LSR >> 16, LSR & 0xffff,
      DLSR >> 16, DLSR & 0xffff);

  if (fractionlost)
    *fractionlost = fraction;
  if (packetslost)
    *packetslost = lost;
  if (exthighestseq)
    *exthighestseq = extended_max;
  if (jitter)
    *jitter = stats->jitter >> 4;
  if (lsr)
    *lsr = LSR;
  if (dlsr)
    *dlsr = DLSR;

  return TRUE;
}

/**
 * rtp_source_get_last_sr:
 * @src: an #RTPSource
 * @time: time of packet arrival
 * @ntptime: the NTP time in 32.32 fixed point
 * @rtptime: the RTP time
 * @packet_count: the packet count
 * @octet_count: the octect count
 *
 * Get the values of the last sender report as set with rtp_source_process_sr().
 *
 * Returns: %TRUE if there was a valid SR report.
 */
gboolean
rtp_source_get_last_sr (RTPSource * src, GstClockTime * time, guint64 * ntptime,
    guint32 * rtptime, guint32 * packet_count, guint32 * octet_count)
{
  RTPSenderReport *curr;

  g_return_val_if_fail (RTP_IS_SOURCE (src), FALSE);

  curr = &src->stats.sr[src->stats.curr_sr];
  if (!curr->is_valid)
    return FALSE;

  if (ntptime)
    *ntptime = curr->ntptime;
  if (rtptime)
    *rtptime = curr->rtptime;
  if (packet_count)
    *packet_count = curr->packet_count;
  if (octet_count)
    *octet_count = curr->octet_count;
  if (time)
    *time = curr->time;

  return TRUE;
}

/**
 * rtp_source_get_last_rb:
 * @src: an #RTPSource
 * @fractionlost: fraction lost since last SR/RR
 * @packetslost: the cumululative number of packets lost
 * @exthighestseq: the extended last sequence number received
 * @jitter: the interarrival jitter
 * @lsr: the last SR packet from this source
 * @dlsr: the delay since last SR packet
 * @round_trip: the round trip time
 *
 * Get the values of the last RB report set with rtp_source_process_rb().
 *
 * Returns: %TRUE if there was a valid SB report.
 */
gboolean
rtp_source_get_last_rb (RTPSource * src, guint8 * fractionlost,
    gint32 * packetslost, guint32 * exthighestseq, guint32 * jitter,
    guint32 * lsr, guint32 * dlsr, guint32 * round_trip)
{
  RTPReceiverReport *curr;

  g_return_val_if_fail (RTP_IS_SOURCE (src), FALSE);

  curr = &src->stats.rr[src->stats.curr_rr];
  if (!curr->is_valid)
    return FALSE;

  if (fractionlost)
    *fractionlost = curr->fractionlost;
  if (packetslost)
    *packetslost = curr->packetslost;
  if (exthighestseq)
    *exthighestseq = curr->exthighestseq;
  if (jitter)
    *jitter = curr->jitter;
  if (lsr)
    *lsr = curr->lsr;
  if (dlsr)
    *dlsr = curr->dlsr;
  if (round_trip)
    *round_trip = curr->round_trip;

  return TRUE;
}
