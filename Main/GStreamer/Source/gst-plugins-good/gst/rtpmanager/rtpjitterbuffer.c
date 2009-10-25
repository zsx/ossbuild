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
#include <stdlib.h>

#include <gst/rtp/gstrtpbuffer.h>
#include <gst/rtp/gstrtcpbuffer.h>

#include "rtpjitterbuffer.h"

GST_DEBUG_CATEGORY_STATIC (rtp_jitter_buffer_debug);
#define GST_CAT_DEFAULT rtp_jitter_buffer_debug

#define MAX_WINDOW	RTP_JITTER_BUFFER_MAX_WINDOW
#define MAX_TIME	(2 * GST_SECOND)

/* signals and args */
enum
{
  LAST_SIGNAL
};

enum
{
  PROP_0
};

/* GObject vmethods */
static void rtp_jitter_buffer_finalize (GObject * object);

/* static guint rtp_jitter_buffer_signals[LAST_SIGNAL] = { 0 }; */

G_DEFINE_TYPE (RTPJitterBuffer, rtp_jitter_buffer, G_TYPE_OBJECT);

static void
rtp_jitter_buffer_class_init (RTPJitterBufferClass * klass)
{
  GObjectClass *gobject_class;

  gobject_class = (GObjectClass *) klass;

  gobject_class->finalize = rtp_jitter_buffer_finalize;

  GST_DEBUG_CATEGORY_INIT (rtp_jitter_buffer_debug, "rtpjitterbuffer", 0,
      "RTP Jitter Buffer");
}

static void
rtp_jitter_buffer_init (RTPJitterBuffer * jbuf)
{
  jbuf->packets = g_queue_new ();

  rtp_jitter_buffer_reset_skew (jbuf);
}

static void
rtp_jitter_buffer_finalize (GObject * object)
{
  RTPJitterBuffer *jbuf;

  jbuf = RTP_JITTER_BUFFER_CAST (object);

  rtp_jitter_buffer_flush (jbuf);
  g_queue_free (jbuf->packets);

  G_OBJECT_CLASS (rtp_jitter_buffer_parent_class)->finalize (object);
}

/**
 * rtp_jitter_buffer_new:
 *
 * Create an #RTPJitterBuffer.
 *
 * Returns: a new #RTPJitterBuffer. Use g_object_unref() after usage.
 */
RTPJitterBuffer *
rtp_jitter_buffer_new (void)
{
  RTPJitterBuffer *jbuf;

  jbuf = g_object_new (RTP_TYPE_JITTER_BUFFER, NULL);

  return jbuf;
}

void
rtp_jitter_buffer_reset_skew (RTPJitterBuffer * jbuf)
{
  jbuf->base_time = -1;
  jbuf->base_rtptime = -1;
  jbuf->base_extrtp = -1;
  jbuf->clock_rate = -1;
  jbuf->ext_rtptime = -1;
  jbuf->last_rtptime = -1;
  jbuf->window_pos = 0;
  jbuf->window_filling = TRUE;
  jbuf->window_min = 0;
  jbuf->skew = 0;
  jbuf->prev_send_diff = -1;
  jbuf->prev_out_time = -1;
  GST_DEBUG ("reset skew correction");
}

/* For the clock skew we use a windowed low point averaging algorithm as can be
 * found in http://www.grame.fr/pub/TR-050601.pdf. The idea is that the jitter is
 * composed of:
 *
 *  J = N + n
 *
 *   N   : a constant network delay.
 *   n   : random added noise. The noise is concentrated around 0
 *
 * In the receiver we can track the elapsed time at the sender with:
 *
 *  send_diff(i) = (Tsi - Ts0);
 *
 *   Tsi : The time at the sender at packet i
 *   Ts0 : The time at the sender at the first packet
 *
 * This is the difference between the RTP timestamp in the first received packet
 * and the current packet.
 *
 * At the receiver we have to deal with the jitter introduced by the network.
 *
 *  recv_diff(i) = (Tri - Tr0)
 *
 *   Tri : The time at the receiver at packet i
 *   Tr0 : The time at the receiver at the first packet
 *
 * Both of these values contain a jitter Ji, a jitter for packet i, so we can
 * write:
 *
 *  recv_diff(i) = (Cri + D + ni) - (Cr0 + D + n0))
 *
 *    Cri    : The time of the clock at the receiver for packet i
 *    D + ni : The jitter when receiving packet i
 *
 * We see that the network delay is irrelevant here as we can elliminate D:
 *
 *  recv_diff(i) = (Cri + ni) - (Cr0 + n0))
 *
 * The drift is now expressed as:
 *
 *  Drift(i) = recv_diff(i) - send_diff(i);
 *
 * We now keep the W latest values of Drift and find the minimum (this is the
 * one with the lowest network jitter and thus the one which is least affected
 * by it). We average this lowest value to smooth out the resulting network skew.
 *
 * Both the window and the weighting used for averaging influence the accuracy
 * of the drift estimation. Finding the correct parameters turns out to be a
 * compromise between accuracy and inertia. 
 *
 * We use a 2 second window or up to 512 data points, which is statistically big
 * enough to catch spikes (FIXME, detect spikes).
 * We also use a rather large weighting factor (125) to smoothly adapt. During
 * startup, when filling the window, we use a parabolic weighting factor, the
 * more the window is filled, the faster we move to the detected possible skew.
 *
 * Returns: @time adjusted with the clock skew.
 */
static GstClockTime
calculate_skew (RTPJitterBuffer * jbuf, guint32 rtptime, GstClockTime time,
    guint32 clock_rate)
{
  guint64 ext_rtptime;
  guint64 send_diff, recv_diff;
  gint64 delta;
  gint64 old;
  gint pos, i;
  GstClockTime gstrtptime, out_time;

  ext_rtptime = gst_rtp_buffer_ext_timestamp (&jbuf->ext_rtptime, rtptime);

  gstrtptime = gst_util_uint64_scale_int (ext_rtptime, GST_SECOND, clock_rate);

  /* keep track of the last extended rtptime */
  jbuf->last_rtptime = ext_rtptime;

  if (jbuf->clock_rate != clock_rate) {
    GST_WARNING ("Clock rate changed from %" G_GUINT32_FORMAT " to %"
        G_GUINT32_FORMAT, jbuf->clock_rate, clock_rate);
    jbuf->base_time = -1;
    jbuf->base_rtptime = -1;
    jbuf->clock_rate = clock_rate;
    jbuf->prev_out_time = -1;
    jbuf->prev_send_diff = -1;
  }

  /* first time, lock on to time and gstrtptime */
  if (G_UNLIKELY (jbuf->base_time == -1)) {
    jbuf->base_time = time;
    jbuf->prev_out_time = -1;
    GST_DEBUG ("Taking new base time %" GST_TIME_FORMAT, GST_TIME_ARGS (time));
  }
  if (G_UNLIKELY (jbuf->base_rtptime == -1)) {
    jbuf->base_rtptime = gstrtptime;
    jbuf->base_extrtp = ext_rtptime;
    jbuf->prev_send_diff = -1;
    GST_DEBUG ("Taking new base rtptime %" GST_TIME_FORMAT,
        GST_TIME_ARGS (gstrtptime));
  }

  if (G_LIKELY (gstrtptime >= jbuf->base_rtptime))
    send_diff = gstrtptime - jbuf->base_rtptime;
  else {
    /* elapsed time at sender, timestamps can go backwards and thus be smaller
     * than our base time, take a new base time in that case. */
    GST_WARNING ("backward timestamps at server, taking new base time");
    jbuf->base_time = time;
    jbuf->base_rtptime = gstrtptime;
    jbuf->base_extrtp = ext_rtptime;
    jbuf->prev_out_time = -1;
    jbuf->prev_send_diff = -1;
    send_diff = 0;
  }

  GST_DEBUG ("extrtp %" G_GUINT64_FORMAT ", gstrtp %" GST_TIME_FORMAT ", base %"
      GST_TIME_FORMAT ", send_diff %" GST_TIME_FORMAT, ext_rtptime,
      GST_TIME_ARGS (gstrtptime), GST_TIME_ARGS (jbuf->base_rtptime),
      GST_TIME_ARGS (send_diff));

  /* we don't have an arrival timestamp so we can't do skew detection. we
   * should still apply a timestamp based on RTP timestamp and base_time */
  if (time == -1 || jbuf->base_time == -1)
    goto no_skew;

  /* elapsed time at receiver, includes the jitter */
  recv_diff = time - jbuf->base_time;

  GST_DEBUG ("time %" GST_TIME_FORMAT ", base %" GST_TIME_FORMAT ", recv_diff %"
      GST_TIME_FORMAT, GST_TIME_ARGS (time), GST_TIME_ARGS (jbuf->base_time),
      GST_TIME_ARGS (recv_diff));

  /* measure the diff */
  delta = ((gint64) recv_diff) - ((gint64) send_diff);

  /* if the difference between the sender timeline and the receiver timeline
   * changed too quickly we have to resync because the server likely restarted
   * its timestamps. */
  if (ABS (delta - jbuf->skew) > GST_SECOND) {
    GST_WARNING ("delta %" GST_TIME_FORMAT " too big, reset skew",
        GST_TIME_ARGS (delta - jbuf->skew));
    jbuf->base_time = time;
    jbuf->base_rtptime = gstrtptime;
    jbuf->base_extrtp = ext_rtptime;
    jbuf->prev_out_time = -1;
    jbuf->prev_send_diff = -1;
    send_diff = 0;
    delta = 0;
  }

  pos = jbuf->window_pos;

  if (G_UNLIKELY (jbuf->window_filling)) {
    /* we are filling the window */
    GST_DEBUG ("filling %d, delta %" G_GINT64_FORMAT, pos, delta);
    jbuf->window[pos++] = delta;
    /* calc the min delta we observed */
    if (G_UNLIKELY (pos == 1 || delta < jbuf->window_min))
      jbuf->window_min = delta;

    if (G_UNLIKELY (send_diff >= MAX_TIME || pos >= MAX_WINDOW)) {
      jbuf->window_size = pos;

      /* window filled */
      GST_DEBUG ("min %" G_GINT64_FORMAT, jbuf->window_min);

      /* the skew is now the min */
      jbuf->skew = jbuf->window_min;
      jbuf->window_filling = FALSE;
    } else {
      gint perc_time, perc_window, perc;

      /* figure out how much we filled the window, this depends on the amount of
       * time we have or the max number of points we keep. */
      perc_time = send_diff * 100 / MAX_TIME;
      perc_window = pos * 100 / MAX_WINDOW;
      perc = MAX (perc_time, perc_window);

      /* make a parabolic function, the closer we get to the MAX, the more value
       * we give to the scaling factor of the new value */
      perc = perc * perc;

      /* quickly go to the min value when we are filling up, slowly when we are
       * just starting because we're not sure it's a good value yet. */
      jbuf->skew =
          (perc * jbuf->window_min + ((10000 - perc) * jbuf->skew)) / 10000;
      jbuf->window_size = pos + 1;
    }
  } else {
    /* pick old value and store new value. We keep the previous value in order
     * to quickly check if the min of the window changed */
    old = jbuf->window[pos];
    jbuf->window[pos++] = delta;

    if (G_UNLIKELY (delta <= jbuf->window_min)) {
      /* if the new value we inserted is smaller or equal to the current min,
       * it becomes the new min */
      jbuf->window_min = delta;
    } else if (G_UNLIKELY (old == jbuf->window_min)) {
      gint64 min = G_MAXINT64;

      /* if we removed the old min, we have to find a new min */
      for (i = 0; i < jbuf->window_size; i++) {
        /* we found another value equal to the old min, we can stop searching now */
        if (jbuf->window[i] == old) {
          min = old;
          break;
        }
        if (jbuf->window[i] < min)
          min = jbuf->window[i];
      }
      jbuf->window_min = min;
    }
    /* average the min values */
    jbuf->skew = (jbuf->window_min + (124 * jbuf->skew)) / 125;
    GST_DEBUG ("delta %" G_GINT64_FORMAT ", new min: %" G_GINT64_FORMAT,
        delta, jbuf->window_min);
  }
  /* wrap around in the window */
  if (G_UNLIKELY (pos >= jbuf->window_size))
    pos = 0;
  jbuf->window_pos = pos;

no_skew:
  /* the output time is defined as the base timestamp plus the RTP time
   * adjusted for the clock skew .*/
  if (jbuf->base_time != -1) {
    out_time = jbuf->base_time + send_diff + jbuf->skew;
    /* check if timestamps are not going backwards, we can only check this if we
     * have a previous out time and a previous send_diff */
    if (G_LIKELY (jbuf->prev_out_time != -1 && jbuf->prev_send_diff != -1)) {
      /* now check for backwards timestamps */
      if (G_UNLIKELY (
              /* if the server timestamps went up and the out_time backwards */
              (send_diff > jbuf->prev_send_diff
                  && out_time < jbuf->prev_out_time) ||
              /* if the server timestamps went backwards and the out_time forwards */
              (send_diff < jbuf->prev_send_diff
                  && out_time > jbuf->prev_out_time) ||
              /* if the server timestamps did not change */
              send_diff == jbuf->prev_send_diff)) {
        GST_DEBUG ("backwards timestamps, using previous time");
        out_time = jbuf->prev_out_time;
      }
    }
  } else
    out_time = -1;

  jbuf->prev_out_time = out_time;
  jbuf->prev_send_diff = send_diff;

  GST_DEBUG ("skew %" G_GINT64_FORMAT ", out %" GST_TIME_FORMAT,
      jbuf->skew, GST_TIME_ARGS (out_time));

  return out_time;
}

/**
 * rtp_jitter_buffer_insert:
 * @jbuf: an #RTPJitterBuffer
 * @buf: a buffer
 * @time: a running_time when this buffer was received in nanoseconds
 * @clock_rate: the clock-rate of the payload of @buf
 * @tail: TRUE when the tail element changed.
 *
 * Inserts @buf into the packet queue of @jbuf. The sequence number of the
 * packet will be used to sort the packets. This function takes ownerhip of
 * @buf when the function returns %TRUE.
 * @buf should have writable metadata when calling this function.
 *
 * Returns: %FALSE if a packet with the same number already existed.
 */
gboolean
rtp_jitter_buffer_insert (RTPJitterBuffer * jbuf, GstBuffer * buf,
    GstClockTime time, guint32 clock_rate, gboolean * tail)
{
  GList *list;
  guint32 rtptime;
  guint16 seqnum;

  g_return_val_if_fail (jbuf != NULL, FALSE);
  g_return_val_if_fail (buf != NULL, FALSE);

  seqnum = gst_rtp_buffer_get_seq (buf);

  /* loop the list to skip strictly smaller seqnum buffers */
  for (list = jbuf->packets->head; list; list = g_list_next (list)) {
    guint16 qseq;
    gint gap;

    qseq = gst_rtp_buffer_get_seq (GST_BUFFER_CAST (list->data));

    /* compare the new seqnum to the one in the buffer */
    gap = gst_rtp_buffer_compare_seqnum (seqnum, qseq);

    /* we hit a packet with the same seqnum, notify a duplicate */
    if (G_UNLIKELY (gap == 0))
      goto duplicate;

    /* seqnum > qseq, we can stop looking */
    if (G_LIKELY (gap < 0))
      break;
  }

  /* do skew calculation by measuring the difference between rtptime and the
   * receive time, this function will retimestamp @buf with the skew corrected
   * running time. */
  rtptime = gst_rtp_buffer_get_timestamp (buf);
  time = calculate_skew (jbuf, rtptime, time, clock_rate);
  GST_BUFFER_TIMESTAMP (buf) = time;

  /* It's more likely that the packet was inserted in the front of the buffer */
  if (G_LIKELY (list))
    g_queue_insert_before (jbuf->packets, list, buf);
  else
    g_queue_push_tail (jbuf->packets, buf);

  /* tail was changed when we did not find a previous packet, we set the return
   * flag when requested. */
  if (G_LIKELY (tail))
    *tail = (list == NULL);

  return TRUE;

  /* ERRORS */
duplicate:
  {
    GST_WARNING ("duplicate packet %d found", (gint) seqnum);
    return FALSE;
  }
}

/**
 * rtp_jitter_buffer_pop:
 * @jbuf: an #RTPJitterBuffer
 *
 * Pops the oldest buffer from the packet queue of @jbuf. The popped buffer will
 * have its timestamp adjusted with the incomming running_time and the detected
 * clock skew.
 *
 * Returns: a #GstBuffer or %NULL when there was no packet in the queue.
 */
GstBuffer *
rtp_jitter_buffer_pop (RTPJitterBuffer * jbuf)
{
  GstBuffer *buf;

  g_return_val_if_fail (jbuf != NULL, FALSE);

  buf = g_queue_pop_tail (jbuf->packets);

  return buf;
}

/**
 * rtp_jitter_buffer_peek:
 * @jbuf: an #RTPJitterBuffer
 *
 * Peek the oldest buffer from the packet queue of @jbuf. Register a callback
 * with rtp_jitter_buffer_set_tail_changed() to be notified when an older packet
 * was inserted in the queue.
 *
 * Returns: a #GstBuffer or %NULL when there was no packet in the queue.
 */
GstBuffer *
rtp_jitter_buffer_peek (RTPJitterBuffer * jbuf)
{
  GstBuffer *buf;

  g_return_val_if_fail (jbuf != NULL, FALSE);

  buf = g_queue_peek_tail (jbuf->packets);

  return buf;
}

/**
 * rtp_jitter_buffer_flush:
 * @jbuf: an #RTPJitterBuffer
 *
 * Flush all packets from the jitterbuffer.
 */
void
rtp_jitter_buffer_flush (RTPJitterBuffer * jbuf)
{
  GstBuffer *buffer;

  g_return_if_fail (jbuf != NULL);

  while ((buffer = g_queue_pop_head (jbuf->packets)))
    gst_buffer_unref (buffer);
}

/**
 * rtp_jitter_buffer_num_packets:
 * @jbuf: an #RTPJitterBuffer
 *
 * Get the number of packets currently in "jbuf.
 *
 * Returns: The number of packets in @jbuf.
 */
guint
rtp_jitter_buffer_num_packets (RTPJitterBuffer * jbuf)
{
  g_return_val_if_fail (jbuf != NULL, 0);

  return jbuf->packets->length;
}

/**
 * rtp_jitter_buffer_get_ts_diff:
 * @jbuf: an #RTPJitterBuffer
 *
 * Get the difference between the timestamps of first and last packet in the
 * jitterbuffer.
 *
 * Returns: The difference expressed in the timestamp units of the packets.
 */
guint32
rtp_jitter_buffer_get_ts_diff (RTPJitterBuffer * jbuf)
{
  guint64 high_ts, low_ts;
  GstBuffer *high_buf, *low_buf;
  guint32 result;

  g_return_val_if_fail (jbuf != NULL, 0);

  high_buf = g_queue_peek_head (jbuf->packets);
  low_buf = g_queue_peek_tail (jbuf->packets);

  if (!high_buf || !low_buf || high_buf == low_buf)
    return 0;

  high_ts = gst_rtp_buffer_get_timestamp (high_buf);
  low_ts = gst_rtp_buffer_get_timestamp (low_buf);

  /* it needs to work if ts wraps */
  if (high_ts >= low_ts) {
    result = (guint32) (high_ts - low_ts);
  } else {
    result = (guint32) (high_ts + G_MAXUINT32 + 1 - low_ts);
  }
  return result;
}

/**
 * rtp_jitter_buffer_get_sync:
 * @jbuf: an #RTPJitterBuffer
 * @rtptime: result RTP time
 * @timestamp: result GStreamer timestamp
 * @clock_rate: clock-rate of @rtptime
 * @last_rtptime: last seen rtptime.
 *
 * Calculates the relation between the RTP timestamp and the GStreamer timestamp
 * used for constructing timestamps.
 *
 * For extended RTP timestamp @rtptime with a clock-rate of @clock_rate,
 * the GStreamer timestamp is currently @timestamp.
 *
 * The last seen extended RTP timestamp with clock-rate @clock-rate is returned in
 * @last_rtptime.
 */
void
rtp_jitter_buffer_get_sync (RTPJitterBuffer * jbuf, guint64 * rtptime,
    guint64 * timestamp, guint32 * clock_rate, guint64 * last_rtptime)
{
  if (rtptime)
    *rtptime = jbuf->base_extrtp;
  if (timestamp)
    *timestamp = jbuf->base_time + jbuf->skew;
  if (clock_rate)
    *clock_rate = jbuf->clock_rate;
  if (last_rtptime)
    *last_rtptime = jbuf->last_rtptime;
}
