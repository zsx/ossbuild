/* GStreamer
 * Copyright (C) <2006> Philippe Khalaf <philippe.kalaf@collabora.co.uk>
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
 * SECTION:gstbasertpaudiopayload
 * @short_description: Base class for audio RTP payloader
 *
 * <refsect2>
 * <para>
 * Provides a base class for audio RTP payloaders for frame or sample based
 * audio codecs (constant bitrate)
 * </para>
 * <para>
 * This class derives from GstBaseRTPPayload. It can be used for payloading
 * audio codecs. It will only work with constant bitrate codecs. It supports
 * both frame based and sample based codecs. It takes care of packing up the
 * audio data into RTP packets and filling up the headers accordingly. The
 * payloading is done based on the maximum MTU (mtu) and the maximum time per
 * packet (max-ptime). The general idea is to divide large data buffers into
 * smaller RTP packets. The RTP packet size is the minimum of either the MTU,
 * max-ptime (if set) or available data. The RTP packet size is always larger or
 * equal to min-ptime (if set). If min-ptime is not set, any residual data is
 * sent in a last RTP packet. In the case of frame based codecs, the resulting
 * RTP packets always contain full frames.
 * </para>
 * <title>Usage</title>
 * <para>
 * To use this base class, your child element needs to call either
 * gst_base_rtp_audio_payload_set_frame_based() or
 * gst_base_rtp_audio_payload_set_sample_based(). This is usually done in the
 * element's _init() function. Then, the child element must call either
 * gst_base_rtp_audio_payload_set_frame_options(),
 * gst_base_rtp_audio_payload_set_sample_options() or
 * gst_base_rtp_audio_payload_set_samplebits_options. Since
 * GstBaseRTPAudioPayload derives from GstBaseRTPPayload, the child element
 * must set any variables or call/override any functions required by that base
 * class. The child element does not need to override any other functions
 * specific to GstBaseRTPAudioPayload.
 * </para>
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <gst/rtp/gstrtpbuffer.h>
#include <gst/base/gstadapter.h>

#include "gstbasertpaudiopayload.h"

GST_DEBUG_CATEGORY_STATIC (basertpaudiopayload_debug);
#define GST_CAT_DEFAULT (basertpaudiopayload_debug)

/* function to convert bytes to a time */
typedef GstClockTime (*GetBytesToTimeFunc) (GstBaseRTPAudioPayload * payload,
    guint64 bytes);
/* function to convert bytes to a RTP time */
typedef guint32 (*GetBytesToRTPTimeFunc) (GstBaseRTPAudioPayload * payload,
    guint64 bytes);
/* function to convert time to bytes */
typedef guint64 (*GetTimeToBytesFunc) (GstBaseRTPAudioPayload * payload,
    GstClockTime time);

struct _GstBaseRTPAudioPayloadPrivate
{
  GetBytesToTimeFunc bytes_to_time;
  GetBytesToRTPTimeFunc bytes_to_rtptime;
  GetTimeToBytesFunc time_to_bytes;

  GstAdapter *adapter;
  guint fragment_size;
  GstClockTime frame_duration_ns;
  gboolean discont;
  guint64 offset;
  GstClockTime last_timestamp;
  guint32 last_rtptime;
  guint align;

  guint cached_mtu;
  guint cached_min_ptime;
  guint cached_max_ptime;
  guint cached_min_length;
  guint cached_max_length;
};


#define GST_BASE_RTP_AUDIO_PAYLOAD_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GST_TYPE_BASE_RTP_AUDIO_PAYLOAD, \
                                GstBaseRTPAudioPayloadPrivate))

static void gst_base_rtp_audio_payload_finalize (GObject * object);

/* bytes to time functions */
static GstClockTime
gst_base_rtp_audio_payload_frame_bytes_to_time (GstBaseRTPAudioPayload *
    payload, guint64 bytes);
static GstClockTime
gst_base_rtp_audio_payload_sample_bytes_to_time (GstBaseRTPAudioPayload *
    payload, guint64 bytes);

/* bytes to RTP time functions */
static guint32
gst_base_rtp_audio_payload_frame_bytes_to_rtptime (GstBaseRTPAudioPayload *
    payload, guint64 bytes);
static guint32
gst_base_rtp_audio_payload_sample_bytes_to_rtptime (GstBaseRTPAudioPayload *
    payload, guint64 bytes);

/* time to bytes functions */
static guint64
gst_base_rtp_audio_payload_frame_time_to_bytes (GstBaseRTPAudioPayload *
    payload, GstClockTime time);
static guint64
gst_base_rtp_audio_payload_sample_time_to_bytes (GstBaseRTPAudioPayload *
    payload, GstClockTime time);

static GstFlowReturn gst_base_rtp_audio_payload_handle_buffer (GstBaseRTPPayload
    * payload, GstBuffer * buffer);

static GstStateChangeReturn gst_base_rtp_payload_audio_change_state (GstElement
    * element, GstStateChange transition);

static gboolean gst_base_rtp_payload_audio_handle_event (GstPad * pad,
    GstEvent * event);

GST_BOILERPLATE (GstBaseRTPAudioPayload, gst_base_rtp_audio_payload,
    GstBaseRTPPayload, GST_TYPE_BASE_RTP_PAYLOAD);

static void
gst_base_rtp_audio_payload_base_init (gpointer klass)
{
}

static void
gst_base_rtp_audio_payload_class_init (GstBaseRTPAudioPayloadClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseRTPPayloadClass *gstbasertppayload_class;

  g_type_class_add_private (klass, sizeof (GstBaseRTPAudioPayloadPrivate));

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasertppayload_class = (GstBaseRTPPayloadClass *) klass;

  gobject_class->finalize =
      GST_DEBUG_FUNCPTR (gst_base_rtp_audio_payload_finalize);

  gstelement_class->change_state =
      GST_DEBUG_FUNCPTR (gst_base_rtp_payload_audio_change_state);

  gstbasertppayload_class->handle_buffer =
      GST_DEBUG_FUNCPTR (gst_base_rtp_audio_payload_handle_buffer);
  gstbasertppayload_class->handle_event =
      GST_DEBUG_FUNCPTR (gst_base_rtp_payload_audio_handle_event);

  GST_DEBUG_CATEGORY_INIT (basertpaudiopayload_debug, "basertpaudiopayload", 0,
      "base audio RTP payloader");
}

static void
gst_base_rtp_audio_payload_init (GstBaseRTPAudioPayload * payload,
    GstBaseRTPAudioPayloadClass * klass)
{
  payload->priv = GST_BASE_RTP_AUDIO_PAYLOAD_GET_PRIVATE (payload);

  /* these need to be set by child object if frame based */
  payload->frame_size = 0;
  payload->frame_duration = 0;

  /* these need to be set by child object if sample based */
  payload->sample_size = 0;

  payload->priv->adapter = gst_adapter_new ();
}

static void
gst_base_rtp_audio_payload_finalize (GObject * object)
{
  GstBaseRTPAudioPayload *payload;

  payload = GST_BASE_RTP_AUDIO_PAYLOAD (object);

  g_object_unref (payload->priv->adapter);

  GST_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

/**
 * gst_base_rtp_audio_payload_set_frame_based:
 * @basertpaudiopayload: a pointer to the element.
 *
 * Tells #GstBaseRTPAudioPayload that the child element is for a frame based
 * audio codec
 */
void
gst_base_rtp_audio_payload_set_frame_based (GstBaseRTPAudioPayload *
    basertpaudiopayload)
{
  g_return_if_fail (basertpaudiopayload != NULL);
  g_return_if_fail (basertpaudiopayload->priv->time_to_bytes == NULL);
  g_return_if_fail (basertpaudiopayload->priv->bytes_to_time == NULL);
  g_return_if_fail (basertpaudiopayload->priv->bytes_to_rtptime == NULL);

  basertpaudiopayload->priv->bytes_to_time =
      gst_base_rtp_audio_payload_frame_bytes_to_time;
  basertpaudiopayload->priv->bytes_to_rtptime =
      gst_base_rtp_audio_payload_frame_bytes_to_rtptime;
  basertpaudiopayload->priv->time_to_bytes =
      gst_base_rtp_audio_payload_frame_time_to_bytes;
}

/**
 * gst_base_rtp_audio_payload_set_sample_based:
 * @basertpaudiopayload: a pointer to the element.
 *
 * Tells #GstBaseRTPAudioPayload that the child element is for a sample based
 * audio codec
 */
void
gst_base_rtp_audio_payload_set_sample_based (GstBaseRTPAudioPayload *
    basertpaudiopayload)
{
  g_return_if_fail (basertpaudiopayload != NULL);
  g_return_if_fail (basertpaudiopayload->priv->time_to_bytes == NULL);
  g_return_if_fail (basertpaudiopayload->priv->bytes_to_time == NULL);
  g_return_if_fail (basertpaudiopayload->priv->bytes_to_rtptime == NULL);

  basertpaudiopayload->priv->bytes_to_time =
      gst_base_rtp_audio_payload_sample_bytes_to_time;
  basertpaudiopayload->priv->bytes_to_rtptime =
      gst_base_rtp_audio_payload_sample_bytes_to_rtptime;
  basertpaudiopayload->priv->time_to_bytes =
      gst_base_rtp_audio_payload_sample_time_to_bytes;
}

/**
 * gst_base_rtp_audio_payload_set_frame_options:
 * @basertpaudiopayload: a pointer to the element.
 * @frame_duration: The duraction of an audio frame in milliseconds.
 * @frame_size: The size of an audio frame in bytes.
 *
 * Sets the options for frame based audio codecs.
 *
 */
void
gst_base_rtp_audio_payload_set_frame_options (GstBaseRTPAudioPayload
    * basertpaudiopayload, gint frame_duration, gint frame_size)
{
  GstBaseRTPAudioPayloadPrivate *priv;

  g_return_if_fail (basertpaudiopayload != NULL);

  priv = basertpaudiopayload->priv;

  basertpaudiopayload->frame_duration = frame_duration;
  priv->frame_duration_ns = frame_duration * GST_MSECOND;
  basertpaudiopayload->frame_size = frame_size;
  priv->align = frame_size;

  gst_adapter_clear (priv->adapter);

  GST_DEBUG_OBJECT (basertpaudiopayload, "frame set to %d ms and size %d",
      frame_duration, frame_size);
}

/**
 * gst_base_rtp_audio_payload_set_sample_options:
 * @basertpaudiopayload: a pointer to the element.
 * @sample_size: Size per sample in bytes.
 *
 * Sets the options for sample based audio codecs.
 */
void
gst_base_rtp_audio_payload_set_sample_options (GstBaseRTPAudioPayload
    * basertpaudiopayload, gint sample_size)
{
  g_return_if_fail (basertpaudiopayload != NULL);

  /* sample_size is in bits internally */
  gst_base_rtp_audio_payload_set_samplebits_options (basertpaudiopayload,
      sample_size * 8);
}

/**
 * gst_base_rtp_audio_payload_set_samplebits_options:
 * @basertpaudiopayload: a pointer to the element.
 * @sample_size: Size per sample in bits.
 *
 * Sets the options for sample based audio codecs.
 *
 * Since: 0.10.18
 */
void
gst_base_rtp_audio_payload_set_samplebits_options (GstBaseRTPAudioPayload
    * basertpaudiopayload, gint sample_size)
{
  guint fragment_size;
  GstBaseRTPAudioPayloadPrivate *priv;

  g_return_if_fail (basertpaudiopayload != NULL);

  priv = basertpaudiopayload->priv;

  basertpaudiopayload->sample_size = sample_size;

  /* sample_size is in bits and is converted into multiple bytes */
  fragment_size = sample_size;
  while ((fragment_size % 8) != 0)
    fragment_size += fragment_size;
  priv->fragment_size = fragment_size / 8;
  priv->align = priv->fragment_size;

  gst_adapter_clear (priv->adapter);

  GST_DEBUG_OBJECT (basertpaudiopayload,
      "Samplebits set to sample size %d bits", sample_size);
}

static void
gst_base_rtp_audio_payload_set_meta (GstBaseRTPAudioPayload * payload,
    GstBuffer * buffer, guint payload_len, GstClockTime timestamp)
{
  GstBaseRTPPayload *basepayload;
  GstBaseRTPAudioPayloadPrivate *priv;

  basepayload = GST_BASE_RTP_PAYLOAD_CAST (payload);
  priv = payload->priv;

  /* set payload type */
  gst_rtp_buffer_set_payload_type (buffer, basepayload->pt);
  /* set marker bit for disconts */
  if (priv->discont) {
    GST_DEBUG_OBJECT (payload, "Setting marker and DISCONT");
    gst_rtp_buffer_set_marker (buffer, TRUE);
    GST_BUFFER_FLAG_SET (buffer, GST_BUFFER_FLAG_DISCONT);
    priv->discont = FALSE;
  }
  GST_BUFFER_TIMESTAMP (buffer) = timestamp;

  /* get the offset in RTP time */
  GST_BUFFER_OFFSET (buffer) = priv->bytes_to_rtptime (payload, priv->offset);

  priv->offset += payload_len;

  /* remember the last rtptime/timestamp pair. We will use this to realign our
   * RTP timestamp after a buffer discont */
  priv->last_rtptime = GST_BUFFER_OFFSET (buffer);
  priv->last_timestamp = timestamp;
}

/**
 * gst_base_rtp_audio_payload_push:
 * @baseaudiopayload: a #GstBaseRTPPayload
 * @data: data to set as payload
 * @payload_len: length of payload
 * @timestamp: a #GstClockTime
 *
 * Create an RTP buffer and store @payload_len bytes of @data as the
 * payload. Set the timestamp on the new buffer to @timestamp before pushing
 * the buffer downstream.
 *
 * Returns: a #GstFlowReturn
 *
 * Since: 0.10.13
 */
GstFlowReturn
gst_base_rtp_audio_payload_push (GstBaseRTPAudioPayload * baseaudiopayload,
    const guint8 * data, guint payload_len, GstClockTime timestamp)
{
  GstBaseRTPPayload *basepayload;
  GstBuffer *outbuf;
  guint8 *payload;
  GstFlowReturn ret;

  basepayload = GST_BASE_RTP_PAYLOAD (baseaudiopayload);

  GST_DEBUG_OBJECT (baseaudiopayload, "Pushing %d bytes ts %" GST_TIME_FORMAT,
      payload_len, GST_TIME_ARGS (timestamp));

  /* create buffer to hold the payload */
  outbuf = gst_rtp_buffer_new_allocate (payload_len, 0, 0);

  /* copy payload */
  payload = gst_rtp_buffer_get_payload (outbuf);
  memcpy (payload, data, payload_len);

  /* set metadata */
  gst_base_rtp_audio_payload_set_meta (baseaudiopayload, outbuf, payload_len,
      timestamp);

  ret = gst_basertppayload_push (basepayload, outbuf);

  return ret;
}

/**
 * gst_base_rtp_audio_payload_flush:
 * @baseaudiopayload: a #GstBaseRTPPayload
 * @payload_len: length of payload
 * @timestamp: a #GstClockTime
 *
 * Create an RTP buffer and store @payload_len bytes of the adapter as the
 * payload. Set the timestamp on the new buffer to @timestamp before pushing
 * the buffer downstream.
 *
 * If @payload_len is -1, all pending bytes will be flushed. If @timestamp is
 * -1, the timestamp will be calculated automatically.
 *
 * Returns: a #GstFlowReturn
 *
 * Since: 0.10.25
 */
GstFlowReturn
gst_base_rtp_audio_payload_flush (GstBaseRTPAudioPayload * baseaudiopayload,
    guint payload_len, GstClockTime timestamp)
{
  GstBaseRTPPayload *basepayload;
  GstBaseRTPAudioPayloadPrivate *priv;
  GstBuffer *outbuf;
  guint8 *payload;
  GstFlowReturn ret;
  GstAdapter *adapter;
  guint64 distance;

  priv = baseaudiopayload->priv;
  adapter = priv->adapter;

  basepayload = GST_BASE_RTP_PAYLOAD (baseaudiopayload);

  if (payload_len == -1)
    payload_len = gst_adapter_available (adapter);

  /* nothing to do, just return */
  if (payload_len == 0)
    return GST_FLOW_OK;

  if (timestamp == -1) {
    /* calculate the timestamp */
    timestamp = gst_adapter_prev_timestamp (adapter, &distance);

    GST_LOG_OBJECT (baseaudiopayload,
        "last timestamp %" GST_TIME_FORMAT ", distance %" G_GUINT64_FORMAT,
        GST_TIME_ARGS (timestamp), distance);

    if (GST_CLOCK_TIME_IS_VALID (timestamp) && distance > 0) {
      /* convert the number of bytes since the last timestamp to time and add to
       * the last seen timestamp */
      timestamp += priv->bytes_to_time (baseaudiopayload, distance);
    }
  }

  GST_DEBUG_OBJECT (baseaudiopayload, "Pushing %d bytes ts %" GST_TIME_FORMAT,
      payload_len, GST_TIME_ARGS (timestamp));

  /* create buffer to hold the payload */
  outbuf = gst_rtp_buffer_new_allocate (payload_len, 0, 0);

  payload = gst_rtp_buffer_get_payload (outbuf);
  gst_adapter_copy (adapter, payload, 0, payload_len);
  gst_adapter_flush (adapter, payload_len);

  /* set metadata */
  gst_base_rtp_audio_payload_set_meta (baseaudiopayload, outbuf, payload_len,
      timestamp);

  ret = gst_basertppayload_push (basepayload, outbuf);

  return ret;
}

#define ALIGN_DOWN(val,len) ((val) - ((val) % (len)))

/* calculate the min and max length of a packet. This depends on the configured
 * mtu and min/max_ptime values. We cache those so that we don't have to redo
 * all the calculations */
static gboolean
gst_base_rtp_audio_payload_get_lengths (GstBaseRTPPayload *
    basepayload, guint * min_payload_len, guint * max_payload_len,
    guint * align)
{
  GstBaseRTPAudioPayload *payload;
  GstBaseRTPAudioPayloadPrivate *priv;
  guint max_mtu, mtu;
  guint maxptime_octets;
  guint minptime_octets;

  payload = GST_BASE_RTP_AUDIO_PAYLOAD_CAST (basepayload);
  priv = payload->priv;

  if (priv->align == 0)
    return FALSE;

  *align = priv->align;

  mtu = GST_BASE_RTP_PAYLOAD_MTU (payload);

  /* check cached values */
  if (G_LIKELY (priv->cached_mtu == mtu
          && priv->cached_max_ptime == basepayload->max_ptime
          && priv->cached_min_ptime == basepayload->min_ptime)) {
    /* if nothing changed, return cached values */
    *min_payload_len = priv->cached_min_length;
    *max_payload_len = priv->cached_max_length;
    return TRUE;
  }

  /* ptime max */
  if (basepayload->max_ptime != -1) {
    maxptime_octets = priv->time_to_bytes (payload, basepayload->max_ptime);
  } else {
    maxptime_octets = G_MAXUINT;
  }
  /* MTU max */
  max_mtu = gst_rtp_buffer_calc_payload_len (mtu, 0, 0);
  /* round down to alignment */
  max_mtu = ALIGN_DOWN (max_mtu, *align);

  /* combine max ptime and max payload length */
  *max_payload_len = MIN (max_mtu, maxptime_octets);

  /* min number of bytes based on a given ptime */
  minptime_octets = priv->time_to_bytes (payload, basepayload->min_ptime);
  /* must be at least one frame size */
  *min_payload_len = MAX (minptime_octets, *align);

  if (*min_payload_len > *max_payload_len)
    *min_payload_len = *max_payload_len;

  /* cache values */
  priv->cached_mtu = mtu;
  priv->cached_min_ptime = basepayload->min_ptime;
  priv->cached_max_ptime = basepayload->max_ptime;
  priv->cached_min_length = *min_payload_len;
  priv->cached_max_length = *max_payload_len;

  return TRUE;
}

/* frame conversions functions */
static GstClockTime
gst_base_rtp_audio_payload_frame_bytes_to_time (GstBaseRTPAudioPayload *
    payload, guint64 bytes)
{
  return (bytes / payload->frame_size) * (payload->priv->frame_duration_ns);
}

static guint32
gst_base_rtp_audio_payload_frame_bytes_to_rtptime (GstBaseRTPAudioPayload *
    payload, guint64 bytes)
{
  guint64 time;

  time = (bytes / payload->frame_size) * (payload->priv->frame_duration_ns);

  return gst_util_uint64_scale_int (time,
      GST_BASE_RTP_PAYLOAD (payload)->clock_rate, GST_SECOND);
}

static guint64
gst_base_rtp_audio_payload_frame_time_to_bytes (GstBaseRTPAudioPayload *
    payload, GstClockTime time)
{
  return gst_util_uint64_scale (time, payload->frame_size,
      payload->priv->frame_duration_ns);
}

/* sample conversion functions */
static GstClockTime
gst_base_rtp_audio_payload_sample_bytes_to_time (GstBaseRTPAudioPayload *
    payload, guint64 bytes)
{
  guint64 rtptime;

  /* avoid division when we can */
  if (G_LIKELY (payload->sample_size != 8))
    rtptime = gst_util_uint64_scale_int (bytes, 8, payload->sample_size);
  else
    rtptime = bytes;

  return gst_util_uint64_scale_int (rtptime, GST_SECOND,
      GST_BASE_RTP_PAYLOAD (payload)->clock_rate);
}

static guint32
gst_base_rtp_audio_payload_sample_bytes_to_rtptime (GstBaseRTPAudioPayload *
    payload, guint64 bytes)
{
  /* avoid division when we can */
  if (G_LIKELY (payload->sample_size != 8))
    return gst_util_uint64_scale_int (bytes, 8, payload->sample_size);
  else
    return bytes;
}

static guint64
gst_base_rtp_audio_payload_sample_time_to_bytes (GstBaseRTPAudioPayload *
    payload, guint64 time)
{
  guint64 samples;

  samples = gst_util_uint64_scale_int (time,
      GST_BASE_RTP_PAYLOAD (payload)->clock_rate, GST_SECOND);

  /* avoid multiplication when we can */
  if (G_LIKELY (payload->sample_size != 8))
    return gst_util_uint64_scale_int (samples, payload->sample_size, 8);
  else
    return samples;
}

static GstFlowReturn
gst_base_rtp_audio_payload_handle_buffer (GstBaseRTPPayload *
    basepayload, GstBuffer * buffer)
{
  GstBaseRTPAudioPayload *payload;
  GstBaseRTPAudioPayloadPrivate *priv;
  guint payload_len;
  GstFlowReturn ret;
  guint available;
  guint min_payload_len;
  guint max_payload_len;
  guint align;
  guint size;
  gboolean discont;

  ret = GST_FLOW_OK;

  payload = GST_BASE_RTP_AUDIO_PAYLOAD_CAST (basepayload);
  priv = payload->priv;

  discont = GST_BUFFER_IS_DISCONT (buffer);
  if (discont) {
    GstClockTime timestamp;

    GST_DEBUG_OBJECT (payload, "Got DISCONT");
    /* flush everything out of the adapter, mark DISCONT */
    ret = gst_base_rtp_audio_payload_flush (payload, -1, -1);
    priv->discont = TRUE;

    timestamp = GST_BUFFER_TIMESTAMP (buffer);

    /* get the distance between the timestamp gap and produce the same gap in
     * the RTP timestamps */
    if (priv->last_timestamp != -1 && timestamp != -1) {
      /* we had a last timestamp, compare it to the new timestamp and update the
       * offset counter for RTP timestamps. The effect is that we will produce
       * output buffers containing the same RTP timestamp gap as the gap
       * between the GST timestamps. */
      if (timestamp > priv->last_timestamp) {
        GstClockTime diff;
        guint64 bytes;
        /* we're only going to apply a positive gap, otherwise we let the marker
         * bit do its thing. simply convert to bytes and add the the current
         * offset */
        diff = timestamp - priv->last_timestamp;
        bytes = priv->time_to_bytes (payload, diff);
        priv->offset += bytes;

        GST_DEBUG_OBJECT (payload,
            "elapsed time %" GST_TIME_FORMAT ", bytes %" G_GUINT64_FORMAT
            ", new offset %" G_GUINT64_FORMAT, GST_TIME_ARGS (diff), bytes,
            priv->offset);
      }
    }
  }

  if (!gst_base_rtp_audio_payload_get_lengths (basepayload, &min_payload_len,
          &max_payload_len, &align))
    goto config_error;

  GST_DEBUG_OBJECT (payload,
      "Calculated min_payload_len %u and max_payload_len %u",
      min_payload_len, max_payload_len);

  size = GST_BUFFER_SIZE (buffer);

  /* shortcut, we don't need to use the adapter when the packet can be pushed
   * through directly. */
  available = gst_adapter_available (priv->adapter);

  GST_DEBUG_OBJECT (payload, "got buffer size %u, available %u",
      size, available);

  if (available == 0 && (size >= min_payload_len && size <= max_payload_len)) {
    /* If buffer fits on an RTP packet, let's just push it through
     * this will check against max_ptime and max_mtu */
    GST_DEBUG_OBJECT (payload, "Fast packet push");
    ret = gst_base_rtp_audio_payload_push (payload,
        GST_BUFFER_DATA (buffer), size, GST_BUFFER_TIMESTAMP (buffer));
    gst_buffer_unref (buffer);
  } else {
    /* push the buffer in the adapter */
    gst_adapter_push (priv->adapter, buffer);
    available += size;

    GST_DEBUG_OBJECT (payload, "available now %u", available);

    /* as long as we have full frames */
    while (available >= min_payload_len) {
      /* get multiple of alignment */
      payload_len = ALIGN_DOWN (available, align);
      payload_len = MIN (max_payload_len, payload_len);

      /* and flush out the bytes from the adapter, automatically set the
       * timestamp. */
      ret = gst_base_rtp_audio_payload_flush (payload, payload_len, -1);

      available -= payload_len;
      GST_DEBUG_OBJECT (payload, "available after push %u", available);
    }
  }
  return ret;

  /* ERRORS */
config_error:
  {
    GST_ELEMENT_ERROR (payload, STREAM, NOT_IMPLEMENTED, (NULL),
        ("subclass did not configure us properly"));
    gst_buffer_unref (buffer);
    return GST_FLOW_ERROR;
  }
}

static GstStateChangeReturn
gst_base_rtp_payload_audio_change_state (GstElement * element,
    GstStateChange transition)
{
  GstBaseRTPAudioPayload *basertppayload;
  GstStateChangeReturn ret;

  basertppayload = GST_BASE_RTP_AUDIO_PAYLOAD (element);

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      basertppayload->priv->cached_mtu = -1;
      basertppayload->priv->last_rtptime = -1;
      basertppayload->priv->last_timestamp = -1;
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      gst_adapter_clear (basertppayload->priv->adapter);
      break;
    default:
      break;
  }

  return ret;
}

static gboolean
gst_base_rtp_payload_audio_handle_event (GstPad * pad, GstEvent * event)
{
  GstBaseRTPAudioPayload *payload;
  gboolean res = FALSE;

  payload = GST_BASE_RTP_AUDIO_PAYLOAD (gst_pad_get_parent (pad));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_EOS:
      /* flush remaining bytes in the adapter */
      gst_base_rtp_audio_payload_flush (payload, -1, -1);
      break;
    case GST_EVENT_FLUSH_STOP:
      gst_adapter_clear (payload->priv->adapter);
      break;
    default:
      break;
  }

  gst_object_unref (payload);

  /* return FALSE to let parent handle the remainder of the event */
  return res;
}

/**
 * gst_base_rtp_audio_payload_get_adapter:
 * @basertpaudiopayload: a #GstBaseRTPAudioPayload
 *
 * Gets the internal adapter used by the depayloader.
 *
 * Returns: a #GstAdapter.
 *
 * Since: 0.10.13
 */
GstAdapter *
gst_base_rtp_audio_payload_get_adapter (GstBaseRTPAudioPayload
    * basertpaudiopayload)
{
  GstAdapter *adapter;

  if ((adapter = basertpaudiopayload->priv->adapter))
    g_object_ref (adapter);

  return adapter;
}
