/* GStreamer
 * Copyright (C) <2007> Julien Moutte <julien@moutte.net>
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
 * SECTION:element-flvdemux
 *
 * flvdemux demuxes an FLV file into the different contained streams.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v filesrc location=/path/to/flv ! flvdemux ! audioconvert ! autoaudiosink
 * ]| This pipeline demuxes an FLV file and outputs the contained raw audio streams.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstflvdemux.h"
#include "gstflvparse.h"
#include "gstflvmux.h"

#include <string.h>

static GstStaticPadTemplate flv_sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-flv")
    );

static GstStaticPadTemplate audio_src_template =
GST_STATIC_PAD_TEMPLATE ("audio",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate video_src_template =
GST_STATIC_PAD_TEMPLATE ("video",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS_ANY);

GST_DEBUG_CATEGORY (flvdemux_debug);
#define GST_CAT_DEFAULT flvdemux_debug

GST_BOILERPLATE (GstFLVDemux, gst_flv_demux, GstElement, GST_TYPE_ELEMENT);

/* 9 bytes of header + 4 bytes of first previous tag size */
#define FLV_HEADER_SIZE 13
/* 1 byte of tag type + 3 bytes of tag data size */
#define FLV_TAG_TYPE_SIZE 4

static void
gst_flv_demux_flush (GstFLVDemux * demux, gboolean discont)
{
  GST_DEBUG_OBJECT (demux, "flushing queued data in the FLV demuxer");

  gst_adapter_clear (demux->adapter);

  demux->audio_need_discont = TRUE;
  demux->video_need_discont = TRUE;

  demux->flushing = FALSE;

  /* Only in push mode */
  if (!demux->random_access) {
    /* After a flush we expect a tag_type */
    demux->state = FLV_STATE_TAG_TYPE;
    /* We reset the offset and will get one from first push */
    demux->offset = 0;
  }
}

static void
gst_flv_demux_cleanup (GstFLVDemux * demux)
{
  GST_DEBUG_OBJECT (demux, "cleaning up FLV demuxer");

  demux->state = FLV_STATE_HEADER;

  demux->flushing = FALSE;
  demux->need_header = TRUE;
  demux->audio_need_segment = TRUE;
  demux->video_need_segment = TRUE;
  demux->audio_need_discont = TRUE;
  demux->video_need_discont = TRUE;

  /* By default we consider them as linked */
  demux->audio_linked = TRUE;
  demux->video_linked = TRUE;

  demux->has_audio = FALSE;
  demux->has_video = FALSE;
  demux->push_tags = FALSE;
  demux->got_par = FALSE;

  gst_segment_init (&demux->segment, GST_FORMAT_TIME);

  demux->w = demux->h = 0;
  demux->par_x = demux->par_y = 1;
  demux->video_offset = 0;
  demux->audio_offset = 0;
  demux->offset = demux->cur_tag_offset = 0;
  demux->tag_size = demux->tag_data_size = 0;
  demux->duration = GST_CLOCK_TIME_NONE;

  if (demux->new_seg_event) {
    gst_event_unref (demux->new_seg_event);
    demux->new_seg_event = NULL;
  }

  if (demux->close_seg_event) {
    gst_event_unref (demux->close_seg_event);
    demux->close_seg_event = NULL;
  }

  gst_adapter_clear (demux->adapter);

  if (demux->audio_codec_data) {
    gst_buffer_unref (demux->audio_codec_data);
    demux->audio_codec_data = NULL;
  }

  if (demux->video_codec_data) {
    gst_buffer_unref (demux->video_codec_data);
    demux->video_codec_data = NULL;
  }

  if (demux->audio_pad) {
    gst_element_remove_pad (GST_ELEMENT (demux), demux->audio_pad);
    gst_object_unref (demux->audio_pad);
    demux->audio_pad = NULL;
  }

  if (demux->video_pad) {
    gst_element_remove_pad (GST_ELEMENT (demux), demux->video_pad);
    gst_object_unref (demux->video_pad);
    demux->video_pad = NULL;
  }

  if (demux->times) {
    g_array_free (demux->times, TRUE);
    demux->times = NULL;
  }

  if (demux->filepositions) {
    g_array_free (demux->filepositions, TRUE);
    demux->filepositions = NULL;
  }
}

static GstFlowReturn
gst_flv_demux_chain (GstPad * pad, GstBuffer * buffer)
{
  GstFlowReturn ret = GST_FLOW_OK;
  GstFLVDemux *demux = NULL;

  demux = GST_FLV_DEMUX (gst_pad_get_parent (pad));

  GST_LOG_OBJECT (demux, "received buffer of %d bytes at offset %"
      G_GUINT64_FORMAT, GST_BUFFER_SIZE (buffer), GST_BUFFER_OFFSET (buffer));

  if (G_UNLIKELY (GST_BUFFER_OFFSET (buffer) == 0)) {
    GST_DEBUG_OBJECT (demux, "beginning of file, expect header");
    demux->state = FLV_STATE_HEADER;
    demux->offset = 0;
  }

  if (G_UNLIKELY (demux->offset == 0 && GST_BUFFER_OFFSET (buffer) != 0)) {
    GST_DEBUG_OBJECT (demux, "offset was zero, synchronizing with buffer's");
    demux->offset = GST_BUFFER_OFFSET (buffer);
  }

  gst_adapter_push (demux->adapter, buffer);

parse:
  if (G_UNLIKELY (ret != GST_FLOW_OK)) {
    if (ret == GST_FLOW_NOT_LINKED && (demux->audio_linked
            || demux->video_linked)) {
      ret = GST_FLOW_OK;
    } else {
      GST_DEBUG_OBJECT (demux, "got flow return %s", gst_flow_get_name (ret));
      goto beach;
    }
  }

  if (G_UNLIKELY (demux->flushing)) {
    GST_DEBUG_OBJECT (demux, "we are now flushing, exiting parser loop");
    ret = GST_FLOW_WRONG_STATE;
    goto beach;
  }

  switch (demux->state) {
    case FLV_STATE_HEADER:
    {
      if (gst_adapter_available (demux->adapter) >= FLV_HEADER_SIZE) {
        GstBuffer *buffer;

        buffer = gst_adapter_take_buffer (demux->adapter, FLV_HEADER_SIZE);

        ret = gst_flv_parse_header (demux, buffer);

        gst_buffer_unref (buffer);
        demux->offset += FLV_HEADER_SIZE;

        demux->state = FLV_STATE_TAG_TYPE;
        goto parse;
      } else {
        goto beach;
      }
    }
    case FLV_STATE_TAG_TYPE:
    {
      if (gst_adapter_available (demux->adapter) >= FLV_TAG_TYPE_SIZE) {
        GstBuffer *buffer;

        /* Remember the tag offset in bytes */
        demux->cur_tag_offset = demux->offset;

        buffer = gst_adapter_take_buffer (demux->adapter, FLV_TAG_TYPE_SIZE);

        ret = gst_flv_parse_tag_type (demux, buffer);

        gst_buffer_unref (buffer);
        demux->offset += FLV_TAG_TYPE_SIZE;

        goto parse;
      } else {
        goto beach;
      }
    }
    case FLV_STATE_TAG_VIDEO:
    {
      if (gst_adapter_available (demux->adapter) >= demux->tag_size) {
        GstBuffer *buffer;

        buffer = gst_adapter_take_buffer (demux->adapter, demux->tag_size);

        ret = gst_flv_parse_tag_video (demux, buffer);

        gst_buffer_unref (buffer);
        demux->offset += demux->tag_size;

        demux->state = FLV_STATE_TAG_TYPE;
        goto parse;
      } else {
        goto beach;
      }
    }
    case FLV_STATE_TAG_AUDIO:
    {
      if (gst_adapter_available (demux->adapter) >= demux->tag_size) {
        GstBuffer *buffer;

        buffer = gst_adapter_take_buffer (demux->adapter, demux->tag_size);

        ret = gst_flv_parse_tag_audio (demux, buffer);

        gst_buffer_unref (buffer);
        demux->offset += demux->tag_size;

        demux->state = FLV_STATE_TAG_TYPE;
        goto parse;
      } else {
        goto beach;
      }
    }
    case FLV_STATE_TAG_SCRIPT:
    {
      if (gst_adapter_available (demux->adapter) >= demux->tag_size) {
        GstBuffer *buffer;

        buffer = gst_adapter_take_buffer (demux->adapter, demux->tag_size);

        ret = gst_flv_parse_tag_script (demux, buffer);

        gst_buffer_unref (buffer);
        demux->offset += demux->tag_size;

        demux->state = FLV_STATE_TAG_TYPE;
        goto parse;
      } else {
        goto beach;
      }
    }
    default:
      GST_DEBUG_OBJECT (demux, "unexpected demuxer state");
  }

beach:
  if (G_UNLIKELY (ret == GST_FLOW_NOT_LINKED)) {
    /* If either audio or video is linked we return GST_FLOW_OK */
    if (demux->audio_linked || demux->video_linked) {
      ret = GST_FLOW_OK;
    }
  }

  gst_object_unref (demux);

  return ret;
}

static GstFlowReturn
gst_flv_demux_pull_range (GstFLVDemux * demux, GstPad * pad, guint64 offset,
    guint size, GstBuffer ** buffer)
{
  GstFlowReturn ret;

  ret = gst_pad_pull_range (pad, offset, size, buffer);
  if (G_UNLIKELY (ret != GST_FLOW_OK)) {
    GST_WARNING_OBJECT (demux,
        "failed when pulling %d bytes from offset %" G_GUINT64_FORMAT ": %s",
        size, offset, gst_flow_get_name (ret));
    *buffer = NULL;
    return ret;
  }

  if (G_UNLIKELY (*buffer && GST_BUFFER_SIZE (*buffer) != size)) {
    GST_WARNING_OBJECT (demux,
        "partial pull got %d when expecting %d from offset %" G_GUINT64_FORMAT,
        GST_BUFFER_SIZE (*buffer), size, offset);
    gst_buffer_unref (*buffer);
    ret = GST_FLOW_UNEXPECTED;
    *buffer = NULL;
    return ret;
  }

  return ret;
}

static GstFlowReturn
gst_flv_demux_pull_tag (GstPad * pad, GstFLVDemux * demux)
{
  GstBuffer *buffer = NULL;
  GstFlowReturn ret = GST_FLOW_OK;

  /* Store tag offset */
  demux->cur_tag_offset = demux->offset;

  /* Get the first 4 bytes to identify tag type and size */
  if (G_UNLIKELY ((ret = gst_flv_demux_pull_range (demux, pad, demux->offset,
                  FLV_TAG_TYPE_SIZE, &buffer)) != GST_FLOW_OK))
    goto beach;

  /* Identify tag type */
  ret = gst_flv_parse_tag_type (demux, buffer);

  gst_buffer_unref (buffer);

  if (G_UNLIKELY (ret != GST_FLOW_OK))
    goto beach;

  /* Jump over tag type + size */
  demux->offset += FLV_TAG_TYPE_SIZE;

  /* Pull the whole tag */
  if (G_UNLIKELY ((ret = gst_flv_demux_pull_range (demux, pad, demux->offset,
                  demux->tag_size, &buffer)) != GST_FLOW_OK))
    goto beach;

  switch (demux->state) {
    case FLV_STATE_TAG_VIDEO:
      ret = gst_flv_parse_tag_video (demux, buffer);
      break;
    case FLV_STATE_TAG_AUDIO:
      ret = gst_flv_parse_tag_audio (demux, buffer);
      break;
    case FLV_STATE_TAG_SCRIPT:
      ret = gst_flv_parse_tag_script (demux, buffer);
      break;
    default:
      GST_WARNING_OBJECT (demux, "unexpected state %d", demux->state);
  }

  gst_buffer_unref (buffer);

  /* Jump over that part we've just parsed */
  demux->offset += demux->tag_size;

  /* Make sure we reinitialize the tag size */
  demux->tag_size = 0;

  /* Ready for the next tag */
  demux->state = FLV_STATE_TAG_TYPE;

  if (G_UNLIKELY (ret == GST_FLOW_NOT_LINKED)) {
    /* If either audio or video is linked we return GST_FLOW_OK */
    if (demux->audio_linked || demux->video_linked) {
      ret = GST_FLOW_OK;
    } else {
      GST_WARNING_OBJECT (demux, "parsing this tag returned not-linked and "
          "neither video nor audio are linked");
    }
  }

beach:
  return ret;
}

static GstFlowReturn
gst_flv_demux_pull_header (GstPad * pad, GstFLVDemux * demux)
{
  GstBuffer *buffer = NULL;
  GstFlowReturn ret = GST_FLOW_OK;

  /* Get the first 9 bytes */
  if (G_UNLIKELY ((ret = gst_flv_demux_pull_range (demux, pad, demux->offset,
                  FLV_HEADER_SIZE, &buffer)) != GST_FLOW_OK))
    goto beach;

  ret = gst_flv_parse_header (demux, buffer);

  gst_buffer_unref (buffer);

  /* Jump over the header now */
  demux->offset += FLV_HEADER_SIZE;
  demux->state = FLV_STATE_TAG_TYPE;

beach:
  return ret;
}

static GstFlowReturn
gst_flv_demux_seek_to_prev_keyframe (GstFLVDemux * demux)
{
  return GST_FLOW_OK;
}

static gboolean
gst_flv_demux_push_src_event (GstFLVDemux * demux, GstEvent * event)
{
  gboolean ret = TRUE;

  if (demux->audio_pad)
    ret |= gst_pad_push_event (demux->audio_pad, gst_event_ref (event));

  if (demux->video_pad)
    ret |= gst_pad_push_event (demux->video_pad, gst_event_ref (event));

  gst_event_unref (event);

  return ret;
}

static void
gst_flv_demux_create_index (GstFLVDemux * demux)
{
  gint64 size;
  GstFormat fmt = GST_FORMAT_BYTES;
  size_t tag_size;
  guint64 old_offset;
  GstBuffer *buffer;
  GstFlowReturn ret;

  if (!gst_pad_query_peer_duration (demux->sinkpad, &fmt, &size) ||
      fmt != GST_FORMAT_BYTES)
    return;

  old_offset = demux->offset;

  while ((ret =
          gst_flv_demux_pull_range (demux, demux->sinkpad, demux->offset, 12,
              &buffer)) == GST_FLOW_OK) {
    if (gst_flv_parse_tag_timestamp (demux, buffer,
            &tag_size) == GST_CLOCK_TIME_NONE) {
      gst_buffer_unref (buffer);
      break;
    }

    gst_buffer_unref (buffer);
    demux->offset += tag_size;
  }

  demux->offset = old_offset;
}

static void
gst_flv_demux_loop (GstPad * pad)
{
  GstFLVDemux *demux = NULL;
  GstFlowReturn ret = GST_FLOW_OK;

  demux = GST_FLV_DEMUX (gst_pad_get_parent (pad));

  if (demux->segment.rate >= 0) {
    /* pull in data */
    switch (demux->state) {
      case FLV_STATE_TAG_TYPE:
        ret = gst_flv_demux_pull_tag (pad, demux);
        break;
      case FLV_STATE_DONE:
        ret = GST_FLOW_UNEXPECTED;
        break;
      default:
        ret = gst_flv_demux_pull_header (pad, demux);
        if (ret == GST_FLOW_OK)
          gst_flv_demux_create_index (demux);

    }

    /* pause if something went wrong */
    if (G_UNLIKELY (ret != GST_FLOW_OK))
      goto pause;

    /* check EOS condition */
    if ((demux->segment.flags & GST_SEEK_FLAG_SEGMENT) &&
        (demux->segment.stop != -1) &&
        (demux->segment.last_stop >= demux->segment.stop)) {
      ret = GST_FLOW_UNEXPECTED;
      goto pause;
    }
  } else {                      /* Reverse playback */
    /* pull in data */
    switch (demux->state) {
      case FLV_STATE_TAG_TYPE:
        ret = gst_flv_demux_pull_tag (pad, demux);
        /* When packet parsing returns UNEXPECTED that means we ve reached the
           point where we want to go to the previous keyframe. This is either
           the last FLV tag or the keyframe we used last time */
        if (ret == GST_FLOW_UNEXPECTED) {
          ret = gst_flv_demux_seek_to_prev_keyframe (demux);
          demux->state = FLV_STATE_TAG_TYPE;
        }
        break;
      default:
        ret = gst_flv_demux_pull_header (pad, demux);
        if (ret == GST_FLOW_OK)
          gst_flv_demux_create_index (demux);
    }

    /* pause if something went wrong */
    if (G_UNLIKELY (ret != GST_FLOW_OK))
      goto pause;

    /* check EOS condition */
    if (demux->segment.last_stop <= demux->segment.start) {
      ret = GST_FLOW_UNEXPECTED;
      goto pause;
    }
  }

  gst_object_unref (demux);

  return;

pause:
  {
    const gchar *reason = gst_flow_get_name (ret);

    GST_LOG_OBJECT (demux, "pausing task, reason %s", reason);
    gst_pad_pause_task (pad);

    if (GST_FLOW_IS_FATAL (ret) || ret == GST_FLOW_NOT_LINKED) {
      if (ret == GST_FLOW_UNEXPECTED) {
        /* perform EOS logic */
        gst_element_no_more_pads (GST_ELEMENT_CAST (demux));
        if (demux->segment.flags & GST_SEEK_FLAG_SEGMENT) {
          gint64 stop;

          /* for segment playback we need to post when (in stream time)
           * we stopped, this is either stop (when set) or the duration. */
          if ((stop = demux->segment.stop) == -1)
            stop = demux->segment.duration;

          if (demux->segment.rate >= 0) {
            GST_LOG_OBJECT (demux, "Sending segment done, at end of segment");
            gst_element_post_message (GST_ELEMENT_CAST (demux),
                gst_message_new_segment_done (GST_OBJECT_CAST (demux),
                    GST_FORMAT_TIME, stop));
          } else {              /* Reverse playback */
            GST_LOG_OBJECT (demux, "Sending segment done, at beginning of "
                "segment");
            gst_element_post_message (GST_ELEMENT_CAST (demux),
                gst_message_new_segment_done (GST_OBJECT_CAST (demux),
                    GST_FORMAT_TIME, demux->segment.start));
          }
        } else {
          /* normal playback, send EOS to all linked pads */
          gst_element_no_more_pads (GST_ELEMENT (demux));
          GST_LOG_OBJECT (demux, "Sending EOS, at end of stream");
          if (!gst_flv_demux_push_src_event (demux, gst_event_new_eos ()))
            GST_WARNING_OBJECT (demux, "failed pushing EOS on streams");
        }
      } else {
        GST_ELEMENT_ERROR (demux, STREAM, FAILED,
            ("Internal data stream error."),
            ("stream stopped, reason %s", reason));
        gst_flv_demux_push_src_event (demux, gst_event_new_eos ());
      }
    }
    gst_object_unref (demux);
    return;
  }
}

static guint64
gst_flv_demux_find_offset (GstFLVDemux * demux, GstSegment * segment)
{
  gint64 bytes = 0;
  gint64 time = 0;
  GstIndexEntry *entry;

  g_return_val_if_fail (segment != NULL, 0);

  time = segment->start;

  if (demux->index) {
    /* Let's check if we have an index entry for that seek time */
    entry = gst_index_get_assoc_entry (demux->index, demux->index_id,
        GST_INDEX_LOOKUP_BEFORE, GST_ASSOCIATION_FLAG_KEY_UNIT,
        GST_FORMAT_TIME, time);

    if (entry) {
      gst_index_entry_assoc_map (entry, GST_FORMAT_BYTES, &bytes);
      gst_index_entry_assoc_map (entry, GST_FORMAT_TIME, &time);

      GST_DEBUG_OBJECT (demux, "found index entry for %" GST_TIME_FORMAT
          " at %" GST_TIME_FORMAT ", seeking to %" G_GINT64_FORMAT,
          GST_TIME_ARGS (segment->start), GST_TIME_ARGS (time), bytes);

      /* Key frame seeking */
      if (segment->flags & GST_SEEK_FLAG_KEY_UNIT) {
        /* Adjust the segment so that the keyframe fits in */
        if (time < segment->start) {
          segment->start = segment->time = time;
        }
        segment->last_stop = time;
      }
    } else {
      GST_DEBUG_OBJECT (demux, "no index entry found for %" GST_TIME_FORMAT,
          GST_TIME_ARGS (segment->start));
    }
  }

  return bytes;
}

static gboolean
gst_flv_demux_handle_seek_push (GstFLVDemux * demux, GstEvent * event)
{
  GstFormat format;
  GstSeekFlags flags;
  GstSeekType start_type, stop_type;
  gint64 start, stop;
  gdouble rate;
  gboolean update, flush, keyframe, ret;
  GstSegment seeksegment;

  gst_event_parse_seek (event, &rate, &format, &flags,
      &start_type, &start, &stop_type, &stop);

  if (format != GST_FORMAT_TIME)
    goto wrong_format;

  flush = !!(flags & GST_SEEK_FLAG_FLUSH);
  keyframe = !!(flags & GST_SEEK_FLAG_KEY_UNIT);

  /* Work on a copy until we are sure the seek succeeded. */
  memcpy (&seeksegment, &demux->segment, sizeof (GstSegment));

  GST_DEBUG_OBJECT (demux, "segment before configure %" GST_SEGMENT_FORMAT,
      &demux->segment);

  /* Apply the seek to our segment */
  gst_segment_set_seek (&seeksegment, rate, format, flags,
      start_type, start, stop_type, stop, &update);

  GST_DEBUG_OBJECT (demux, "segment configured %" GST_SEGMENT_FORMAT,
      &seeksegment);

  if (flush || seeksegment.last_stop != demux->segment.last_stop) {
    /* Do the actual seeking */
    guint64 offset = gst_flv_demux_find_offset (demux, &seeksegment);

    GST_DEBUG_OBJECT (demux, "generating an upstream seek at position %"
        G_GUINT64_FORMAT, offset);
    ret = gst_pad_push_event (demux->sinkpad,
        gst_event_new_seek (seeksegment.rate, GST_FORMAT_BYTES,
            seeksegment.flags | GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET,
            offset, GST_SEEK_TYPE_NONE, 0));
    if (G_UNLIKELY (!ret)) {
      GST_WARNING_OBJECT (demux, "upstream seek failed");
    }

    /* Tell all the stream we moved to a different position (discont) */
    demux->audio_need_discont = TRUE;
    demux->video_need_discont = TRUE;
  } else {
    ret = TRUE;
  }

  if (ret) {
    /* Ok seek succeeded, take the newly configured segment */
    memcpy (&demux->segment, &seeksegment, sizeof (GstSegment));

    /* Tell all the stream a new segment is needed */
    demux->audio_need_segment = TRUE;
    demux->video_need_segment = TRUE;
    /* Clean any potential newsegment event kept for the streams. The first
     * stream needing a new segment will create a new one. */
    if (G_UNLIKELY (demux->new_seg_event)) {
      gst_event_unref (demux->new_seg_event);
      demux->new_seg_event = NULL;
    }
    gst_event_unref (event);
  } else {
    ret = gst_pad_push_event (demux->sinkpad, event);
  }

  return ret;

/* ERRORS */
wrong_format:
  {
    GST_WARNING_OBJECT (demux, "we only support seeking in TIME format");
    return gst_pad_push_event (demux->sinkpad, event);
  }
}

static gboolean
gst_flv_demux_handle_seek_pull (GstFLVDemux * demux, GstEvent * event)
{
  GstFormat format;
  GstSeekFlags flags;
  GstSeekType start_type, stop_type;
  gint64 start, stop;
  gdouble rate;
  gboolean update, flush, keyframe, ret;
  GstSegment seeksegment;

  gst_event_parse_seek (event, &rate, &format, &flags,
      &start_type, &start, &stop_type, &stop);

  gst_event_unref (event);

  if (format != GST_FORMAT_TIME)
    goto wrong_format;

  flush = !!(flags & GST_SEEK_FLAG_FLUSH);
  keyframe = !!(flags & GST_SEEK_FLAG_KEY_UNIT);

  if (flush) {
    /* Flush start up and downstream to make sure data flow and loops are
       idle */
    gst_flv_demux_push_src_event (demux, gst_event_new_flush_start ());
    gst_pad_push_event (demux->sinkpad, gst_event_new_flush_start ());
  } else {
    /* Pause the pulling task */
    gst_pad_pause_task (demux->sinkpad);
  }

  /* Take the stream lock */
  GST_PAD_STREAM_LOCK (demux->sinkpad);

  if (flush) {
    /* Stop flushing upstream we need to pull */
    gst_pad_push_event (demux->sinkpad, gst_event_new_flush_stop ());
  }

  /* Work on a copy until we are sure the seek succeeded. */
  memcpy (&seeksegment, &demux->segment, sizeof (GstSegment));

  GST_DEBUG_OBJECT (demux, "segment before configure %" GST_SEGMENT_FORMAT,
      &demux->segment);

  /* Apply the seek to our segment */
  gst_segment_set_seek (&seeksegment, rate, format, flags,
      start_type, start, stop_type, stop, &update);

  GST_DEBUG_OBJECT (demux, "segment configured %" GST_SEGMENT_FORMAT,
      &seeksegment);

  if (flush || seeksegment.last_stop != demux->segment.last_stop) {
    /* Do the actual seeking */
    demux->offset = gst_flv_demux_find_offset (demux, &seeksegment);

    /* Tell all the stream we moved to a different position (discont) */
    demux->audio_need_discont = TRUE;
    demux->video_need_discont = TRUE;

    /* If we seeked at the beginning of the file parse the header again */
    if (G_UNLIKELY (!demux->offset)) {
      demux->state = FLV_STATE_HEADER;
    } else {                    /* or parse a tag */
      demux->state = FLV_STATE_TAG_TYPE;
    }
    ret = TRUE;
  } else {
    ret = TRUE;
  }

  if (G_UNLIKELY (demux->close_seg_event)) {
    gst_event_unref (demux->close_seg_event);
    demux->close_seg_event = NULL;
  }

  if (flush) {
    /* Stop flushing, the sinks are at time 0 now */
    gst_flv_demux_push_src_event (demux, gst_event_new_flush_stop ());
  } else {
    GST_DEBUG_OBJECT (demux, "closing running segment %" GST_SEGMENT_FORMAT,
        &demux->segment);

    /* Close the current segment for a linear playback */
    if (demux->segment.rate >= 0) {
      /* for forward playback, we played from start to last_stop */
      demux->close_seg_event = gst_event_new_new_segment (TRUE,
          demux->segment.rate, demux->segment.format,
          demux->segment.start, demux->segment.last_stop, demux->segment.time);
    } else {
      gint64 stop;

      if ((stop = demux->segment.stop) == -1)
        stop = demux->segment.duration;

      /* for reverse playback, we played from stop to last_stop. */
      demux->close_seg_event = gst_event_new_new_segment (TRUE,
          demux->segment.rate, demux->segment.format,
          demux->segment.last_stop, stop, demux->segment.last_stop);
    }
  }

  if (ret) {
    /* Ok seek succeeded, take the newly configured segment */
    memcpy (&demux->segment, &seeksegment, sizeof (GstSegment));

    /* Notify about the start of a new segment */
    if (demux->segment.flags & GST_SEEK_FLAG_SEGMENT) {
      gst_element_post_message (GST_ELEMENT (demux),
          gst_message_new_segment_start (GST_OBJECT (demux),
              demux->segment.format, demux->segment.last_stop));
    }

    /* Tell all the stream a new segment is needed */
    demux->audio_need_segment = TRUE;
    demux->video_need_segment = TRUE;
    /* Clean any potential newsegment event kept for the streams. The first
     * stream needing a new segment will create a new one. */
    if (G_UNLIKELY (demux->new_seg_event)) {
      gst_event_unref (demux->new_seg_event);
      demux->new_seg_event = NULL;
    }
  }

  gst_pad_start_task (demux->sinkpad,
      (GstTaskFunction) gst_flv_demux_loop, demux->sinkpad);

  GST_PAD_STREAM_UNLOCK (demux->sinkpad);

  return ret;

  /* ERRORS */
wrong_format:
  {
    GST_WARNING_OBJECT (demux, "we only support seeking in TIME format");
    return FALSE;
  }
}

/* If we can pull that's prefered */
static gboolean
gst_flv_demux_sink_activate (GstPad * sinkpad)
{
  if (gst_pad_check_pull_range (sinkpad)) {
    return gst_pad_activate_pull (sinkpad, TRUE);
  } else {
    return gst_pad_activate_push (sinkpad, TRUE);
  }
}

/* This function gets called when we activate ourselves in push mode.
 * We cannot seek (ourselves) in the stream */
static gboolean
gst_flv_demux_sink_activate_push (GstPad * sinkpad, gboolean active)
{
  GstFLVDemux *demux;

  demux = GST_FLV_DEMUX (gst_pad_get_parent (sinkpad));

  demux->random_access = FALSE;

  gst_object_unref (demux);

  return TRUE;
}

/* this function gets called when we activate ourselves in pull mode.
 * We can perform  random access to the resource and we start a task
 * to start reading */
static gboolean
gst_flv_demux_sink_activate_pull (GstPad * sinkpad, gboolean active)
{
  GstFLVDemux *demux;

  demux = GST_FLV_DEMUX (gst_pad_get_parent (sinkpad));

  if (active) {
    demux->random_access = TRUE;
    gst_object_unref (demux);
    return gst_pad_start_task (sinkpad, (GstTaskFunction) gst_flv_demux_loop,
        sinkpad);
  } else {
    demux->random_access = FALSE;
    gst_object_unref (demux);
    return gst_pad_stop_task (sinkpad);
  }
}

static gboolean
gst_flv_demux_sink_event (GstPad * pad, GstEvent * event)
{
  GstFLVDemux *demux;
  gboolean ret = FALSE;

  demux = GST_FLV_DEMUX (gst_pad_get_parent (pad));

  GST_DEBUG_OBJECT (demux, "handling event %s", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_FLUSH_START:
      GST_DEBUG_OBJECT (demux, "trying to force chain function to exit");
      demux->flushing = TRUE;
      ret = gst_flv_demux_push_src_event (demux, event);
      break;
    case GST_EVENT_FLUSH_STOP:
      GST_DEBUG_OBJECT (demux, "flushing FLV demuxer");
      gst_flv_demux_flush (demux, TRUE);
      ret = gst_flv_demux_push_src_event (demux, event);
      break;
    case GST_EVENT_EOS:
      GST_DEBUG_OBJECT (demux, "received EOS");
      if (demux->index) {
        GST_DEBUG_OBJECT (demux, "committing index");
        gst_index_commit (demux->index, demux->index_id);
      }
      gst_element_no_more_pads (GST_ELEMENT (demux));
      if (!gst_flv_demux_push_src_event (demux, event))
        GST_WARNING_OBJECT (demux, "failed pushing EOS on streams");
      ret = TRUE;
      break;
    case GST_EVENT_NEWSEGMENT:
    {
      GstFormat format;
      gdouble rate;
      gint64 start, stop, time;
      gboolean update;

      GST_DEBUG_OBJECT (demux, "received new segment");

      gst_event_parse_new_segment (event, &update, &rate, &format, &start,
          &stop, &time);

      if (format == GST_FORMAT_TIME) {
        /* time segment, this is perfect, copy over the values. */
        gst_segment_set_newsegment (&demux->segment, update, rate, format,
            start, stop, time);

        GST_DEBUG_OBJECT (demux, "NEWSEGMENT: %" GST_SEGMENT_FORMAT,
            &demux->segment);

        /* and forward */
        ret = gst_flv_demux_push_src_event (demux, event);
      } else {
        /* non-time format */
        demux->audio_need_segment = TRUE;
        demux->video_need_segment = TRUE;
        ret = TRUE;
        gst_event_unref (event);
      }
      break;
    }
    default:
      ret = gst_flv_demux_push_src_event (demux, event);
      break;
  }

  gst_object_unref (demux);

  return ret;
}

gboolean
gst_flv_demux_src_event (GstPad * pad, GstEvent * event)
{
  GstFLVDemux *demux;
  gboolean ret = FALSE;

  demux = GST_FLV_DEMUX (gst_pad_get_parent (pad));

  GST_DEBUG_OBJECT (demux, "handling event %s", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_SEEK:
      if (demux->random_access) {
        ret = gst_flv_demux_handle_seek_pull (demux, event);
      } else {
        ret = gst_flv_demux_handle_seek_push (demux, event);
      }
      break;
    default:
      ret = gst_pad_push_event (demux->sinkpad, event);
      break;
  }

  gst_object_unref (demux);

  return ret;
}

gboolean
gst_flv_demux_query (GstPad * pad, GstQuery * query)
{
  gboolean res = TRUE;
  GstFLVDemux *demux;

  demux = GST_FLV_DEMUX (gst_pad_get_parent (pad));

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_DURATION:
    {
      GstFormat format;

      gst_query_parse_duration (query, &format, NULL);

      /* duration is time only */
      if (format != GST_FORMAT_TIME) {
        GST_DEBUG_OBJECT (demux, "duration query only supported for time "
            "format");
        res = FALSE;
        goto beach;
      }

      GST_DEBUG_OBJECT (pad, "duration query, replying %" GST_TIME_FORMAT,
          GST_TIME_ARGS (demux->duration));

      gst_query_set_duration (query, GST_FORMAT_TIME, demux->duration);

      break;
    }
    case GST_QUERY_POSITION:
    {
      GstFormat format;

      gst_query_parse_position (query, &format, NULL);

      /* position is time only */
      if (format != GST_FORMAT_TIME) {
        GST_DEBUG_OBJECT (demux, "position query only supported for time "
            "format");
        res = FALSE;
        goto beach;
      }

      GST_DEBUG_OBJECT (pad, "position query, replying %" GST_TIME_FORMAT,
          GST_TIME_ARGS (demux->segment.last_stop));

      gst_query_set_duration (query, GST_FORMAT_TIME, demux->segment.last_stop);

      break;
    }

    case GST_QUERY_LATENCY:
    default:
    {
      GstPad *peer;

      if ((peer = gst_pad_get_peer (demux->sinkpad))) {
        /* query latency on peer pad */
        res = gst_pad_query (peer, query);
        gst_object_unref (peer);
      } else {
        /* no peer, we don't know */
        res = FALSE;
      }
      break;
    }
  }

beach:
  gst_object_unref (demux);

  return res;
}

static GstStateChangeReturn
gst_flv_demux_change_state (GstElement * element, GstStateChange transition)
{
  GstFLVDemux *demux;
  GstStateChangeReturn ret;

  demux = GST_FLV_DEMUX (element);

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      /* If this is our own index destroy it as the
       * old entries might be wrong for the new stream */
      if (demux->own_index) {
        gst_object_unref (demux->index);
        demux->index = NULL;
        demux->own_index = FALSE;
      }

      /* If no index was created, generate one */
      if (G_UNLIKELY (!demux->index)) {
        GST_DEBUG_OBJECT (demux, "no index provided creating our own");

        demux->index = gst_index_factory_make ("memindex");

        gst_index_get_writer_id (demux->index, GST_OBJECT (demux),
            &demux->index_id);
        demux->own_index = TRUE;
      }
      gst_flv_demux_cleanup (demux);
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      gst_flv_demux_cleanup (demux);
      break;
    default:
      break;
  }

  return ret;
}

static void
gst_flv_demux_set_index (GstElement * element, GstIndex * index)
{
  GstFLVDemux *demux = GST_FLV_DEMUX (element);

  GST_OBJECT_LOCK (demux);
  if (demux->index)
    gst_object_unref (demux->index);
  demux->index = gst_object_ref (index);
  GST_OBJECT_UNLOCK (demux);

  gst_index_get_writer_id (index, GST_OBJECT (element), &demux->index_id);
  demux->own_index = FALSE;
}

static GstIndex *
gst_flv_demux_get_index (GstElement * element)
{
  GstIndex *result = NULL;

  GstFLVDemux *demux = GST_FLV_DEMUX (element);

  GST_OBJECT_LOCK (demux);
  if (demux->index)
    result = gst_object_ref (demux->index);
  GST_OBJECT_UNLOCK (demux);

  return result;
}

static void
gst_flv_demux_dispose (GObject * object)
{
  GstFLVDemux *demux = GST_FLV_DEMUX (object);

  GST_DEBUG_OBJECT (demux, "disposing FLV demuxer");

  if (demux->adapter) {
    gst_adapter_clear (demux->adapter);
    g_object_unref (demux->adapter);
    demux->adapter = NULL;
  }

  if (demux->taglist) {
    gst_tag_list_free (demux->taglist);
    demux->taglist = NULL;
  }

  if (demux->new_seg_event) {
    gst_event_unref (demux->new_seg_event);
    demux->new_seg_event = NULL;
  }

  if (demux->close_seg_event) {
    gst_event_unref (demux->close_seg_event);
    demux->close_seg_event = NULL;
  }

  if (demux->audio_codec_data) {
    gst_buffer_unref (demux->audio_codec_data);
    demux->audio_codec_data = NULL;
  }

  if (demux->video_codec_data) {
    gst_buffer_unref (demux->video_codec_data);
    demux->video_codec_data = NULL;
  }

  if (demux->audio_pad) {
    gst_object_unref (demux->audio_pad);
    demux->audio_pad = NULL;
  }

  if (demux->video_pad) {
    gst_object_unref (demux->video_pad);
    demux->video_pad = NULL;
  }

  if (demux->index) {
    gst_object_unref (demux->index);
    demux->index = NULL;
  }

  if (demux->times) {
    g_array_free (demux->times, TRUE);
    demux->times = NULL;
  }

  if (demux->filepositions) {
    g_array_free (demux->filepositions, TRUE);
    demux->filepositions = NULL;
  }

  GST_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
gst_flv_demux_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&flv_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&audio_src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&video_src_template));
  gst_element_class_set_details_simple (element_class, "FLV Demuxer",
      "Codec/Demuxer",
      "Demux FLV feeds into digital streams",
      "Julien Moutte <julien@moutte.net>");
}

static void
gst_flv_demux_class_init (GstFLVDemuxClass * klass)
{
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = GST_DEBUG_FUNCPTR (gst_flv_demux_dispose);

  gstelement_class->change_state =
      GST_DEBUG_FUNCPTR (gst_flv_demux_change_state);
  gstelement_class->set_index = GST_DEBUG_FUNCPTR (gst_flv_demux_set_index);
  gstelement_class->get_index = GST_DEBUG_FUNCPTR (gst_flv_demux_get_index);
}

static void
gst_flv_demux_init (GstFLVDemux * demux, GstFLVDemuxClass * g_class)
{
  demux->sinkpad =
      gst_pad_new_from_static_template (&flv_sink_template, "sink");

  gst_pad_set_event_function (demux->sinkpad,
      GST_DEBUG_FUNCPTR (gst_flv_demux_sink_event));
  gst_pad_set_chain_function (demux->sinkpad,
      GST_DEBUG_FUNCPTR (gst_flv_demux_chain));
  gst_pad_set_activate_function (demux->sinkpad,
      GST_DEBUG_FUNCPTR (gst_flv_demux_sink_activate));
  gst_pad_set_activatepull_function (demux->sinkpad,
      GST_DEBUG_FUNCPTR (gst_flv_demux_sink_activate_pull));
  gst_pad_set_activatepush_function (demux->sinkpad,
      GST_DEBUG_FUNCPTR (gst_flv_demux_sink_activate_push));

  gst_element_add_pad (GST_ELEMENT (demux), demux->sinkpad);

  demux->adapter = gst_adapter_new ();
  demux->taglist = gst_tag_list_new ();
  gst_segment_init (&demux->segment, GST_FORMAT_TIME);

  demux->own_index = FALSE;

  gst_flv_demux_cleanup (demux);
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (flvdemux_debug, "flvdemux", 0, "FLV demuxer");

  if (!gst_element_register (plugin, "flvdemux", GST_RANK_PRIMARY,
          gst_flv_demux_get_type ()) ||
      !gst_element_register (plugin, "flvmux", GST_RANK_PRIMARY,
          gst_flv_mux_get_type ()))
    return FALSE;

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR,
    "flv", "FLV muxing and demuxing plugin",
    plugin_init, VERSION, "LGPL", GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
