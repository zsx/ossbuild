/*
 *  Copyright 2009 Nokia Corporation <multimedia@maemo.org>
 *            2006 Zeeshan Ali <zeeshan.ali@nokia.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * SECTION:element-fpsdisplaysink
 *
 * Can display the current and average framerate as a testoverlay or on stdout.
 *
 * <refsect2>
 * <title>Example launch lines</title>
 * |[
 * gst-launch videotestsrc ! fpsdisplaysink
 * gst-launch videotestsrc ! fpsdisplaysink text-overlay=false
 * gst-launch filesrc location=video.avi ! decodebin2 name=d ! queue ! fpsdisplaysink d. ! queue ! fakesink sync=true
 * ]|
 * </refsect2>
 */
/* FIXME:
 * - can we avoid plugging the textoverlay?
 * - we should use autovideosink as we are RANK_NONE and would not get plugged
 *   - but then we have to lookup the realsink to be able to set sync
 * - gst-seek 15 "videotestsrc ! fpsdisplaysink" dies when closing gst-seek
 * - if we make ourself RANK_PRIMARY+10 autovideosink asserts
 *
 * IDEAS:
 * - do we want to gather min/max fps and show in GST_STATE_CHANGE_READY_TO_NULL
 * - add another property for the FPS_DISPLAY_INTERVAL_MS
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "fpsdisplaysink.h"
#include <gst/interfaces/xoverlay.h>

#define FPS_DISPLAY_INTERVAL_MS 500     /* 500 ms */
#define DEFAULT_FONT "Sans 20"

static GstElementDetails fps_display_sink_details = {
  "Measure and show framerate on videosink",
  "Sink/Video",
  "Shows the current frame-rate and drop-rate of the videosink as overlay or text on stdout",
  "Zeeshan Ali <zeeshan.ali@nokia.com>, Stefan Kost <stefan.kost@nokia.com>"
};

/* generic templates */
static GstStaticPadTemplate fps_display_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

GST_DEBUG_CATEGORY_STATIC (fps_display_sink_debug);
#define GST_CAT_DEFAULT fps_display_sink_debug

enum
{
  ARG_0,
  ARG_SYNC,
  ARG_TEXT_OVERLAY,
  /* FILL ME */
};

static GstBinClass *parent_class = NULL;

static GstStateChangeReturn fps_display_sink_change_state (GstElement * element,
    GstStateChange transition);
static void fps_display_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void fps_display_sink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static void fps_display_sink_dispose (GObject * object);

static void
fps_display_sink_class_init (GstFPSDisplaySinkClass * klass)
{
  GObjectClass *gobject_klass = G_OBJECT_CLASS (klass);
  GstElementClass *gstelement_klass = GST_ELEMENT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  gobject_klass->set_property = fps_display_sink_set_property;
  gobject_klass->get_property = fps_display_sink_get_property;
  gobject_klass->dispose = fps_display_sink_dispose;

  g_object_class_install_property (gobject_klass, ARG_SYNC,
      g_param_spec_boolean ("sync",
          "Sync", "Sync on the clock", TRUE,
          G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));

  g_object_class_install_property (gobject_klass, ARG_TEXT_OVERLAY,
      g_param_spec_boolean ("text-overlay",
          "text-overlay",
          "Wether to use text-overlay", TRUE,
          G_PARAM_STATIC_STRINGS | G_PARAM_READWRITE));

  gstelement_klass->change_state = fps_display_sink_change_state;

  gst_element_class_add_pad_template (gstelement_klass,
      gst_static_pad_template_get (&fps_display_sink_template));

  gst_element_class_set_details (gstelement_klass, &fps_display_sink_details);
}

static gboolean
on_video_sink_data_flow (GstPad * pad, GstMiniObject * mini_obj,
    gpointer user_data)
{
  GstFPSDisplaySink *self = GST_FPS_DISPLAY_SINK (user_data);

#if 0
  if (GST_IS_BUFFER (mini_obj)) {
    GstBuffer *buf = GST_BUFFER_CAST (mini_obj);

    if (GST_CLOCK_TIME_IS_VALID (self->next_ts)) {
      if (GST_BUFFER_TIMESTAMP (buf) <= self->next_ts) {
        self->frames_rendered++;
      } else {
        GST_WARNING_OBJECT (self, "dropping frame : ts %" GST_TIME_FORMAT
            " < expected_ts %" GST_TIME_FORMAT,
            GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (buf)),
            GST_TIME_ARGS (self->next_ts));
        self->frames_dropped++;
      }
    } else {
      self->frames_rendered++;
    }
  } else
#endif
  if (GST_IS_EVENT (mini_obj)) {
    GstEvent *ev = GST_EVENT_CAST (mini_obj);

    if (GST_EVENT_TYPE (ev) == GST_EVENT_QOS) {
      GstClockTimeDiff diff;
      GstClockTime ts;

      gst_event_parse_qos (ev, NULL, &diff, &ts);
      self->next_ts = ts + diff;
      if (diff <= 0.0) {
        self->frames_rendered++;
      } else {
        self->frames_dropped++;
      }
    }
  }
  return TRUE;
}

static void
fps_display_sink_init (GstFPSDisplaySink * self,
    GstFPSDisplaySinkClass * g_class)
{
  GstPad *sink_pad;

  self->sync = FALSE;
  self->use_text_overlay = TRUE;

  /* create child elements */
  self->video_sink =
      gst_element_factory_make ("xvimagesink", "fps-display-video_sink");
  if (!self->video_sink) {
    GST_ERROR_OBJECT (self, "element could not be created");
    return;
  }

  g_object_set (self->video_sink, "sync", self->sync, NULL);

  /* take a ref before bin takes the ownership */
  gst_object_ref (self->video_sink);

  gst_bin_add (GST_BIN (self), self->video_sink);

  /* create ghost pad */
  self->ghost_pad = gst_ghost_pad_new_no_target ("sink_pad", GST_PAD_SINK);
  gst_element_add_pad (GST_ELEMENT (self), self->ghost_pad);

  /* attach or pad probe */
  sink_pad = gst_element_get_static_pad (self->video_sink, "sink");
  gst_pad_add_data_probe (sink_pad, G_CALLBACK (on_video_sink_data_flow),
      (gpointer) self);
  gst_object_unref (sink_pad);

  self->query = gst_query_new_position (GST_FORMAT_TIME);
}

static gboolean
display_current_fps (gpointer data)
{
  GstFPSDisplaySink *self = GST_FPS_DISPLAY_SINK (data);
  gint64 current_ts;

  gst_element_query (self->video_sink, self->query);
  gst_query_parse_position (self->query, NULL, &current_ts);

  if (GST_CLOCK_TIME_IS_VALID (self->last_ts)) {
    gdouble rr, dr, average_fps;
    gchar fps_message[256];
    gdouble time_diff = (gdouble) (current_ts - self->last_ts) / GST_SECOND;

    rr = (gdouble) (self->frames_rendered -
        self->last_frames_rendered) / time_diff;
    dr = (gdouble) (self->frames_dropped -
        self->last_frames_dropped) / time_diff;

    average_fps = self->frames_rendered / (gdouble) (current_ts / GST_SECOND);

    if (dr == 0.0) {
      g_snprintf (fps_message, 255, "current: %.2f\naverage: %.2f", rr,
          average_fps);
    } else {
      g_snprintf (fps_message, 255, "fps: %.2f\ndrop rate: %.2f", rr, dr);
    }

    if (self->use_text_overlay) {
      g_object_set (self->text_overlay, "text", fps_message, NULL);
    } else {
      g_print ("%s\n", fps_message);
    }
  }

  self->last_frames_rendered = self->frames_rendered;
  self->last_frames_dropped = self->frames_dropped;
  self->last_ts = current_ts;

  return TRUE;
}

static void
fps_display_sink_start (GstFPSDisplaySink * self)
{
  GstPad *target_pad = NULL;

  /* Init counters */
  self->next_ts = GST_CLOCK_TIME_NONE;
  self->last_ts = GST_CLOCK_TIME_NONE;
  self->frames_rendered = G_GUINT64_CONSTANT (0);
  self->frames_dropped = G_GUINT64_CONSTANT (0);

  GST_WARNING ("use text-overlay? %d", self->use_text_overlay);

  if (self->use_text_overlay) {
    if (!self->text_overlay) {
      self->text_overlay =
          gst_element_factory_make ("textoverlay", "fps-display-text-overlay");
      if (!self->text_overlay) {
        GST_WARNING_OBJECT (self, "text-overlay element could not be created");
        self->use_text_overlay = FALSE;
        goto no_text_overlay;
      }
      gst_object_ref (self->text_overlay);
      g_object_set (self->text_overlay,
          "font-desc", DEFAULT_FONT, "silent", FALSE, NULL);
      gst_bin_add (GST_BIN (self), self->text_overlay);

      if (!gst_element_link (self->text_overlay, self->video_sink)) {
        GST_ERROR_OBJECT (self, "Could not link elements");
      }
    }
    target_pad = gst_element_get_static_pad (self->text_overlay, "video_sink");
  }
no_text_overlay:
  if (!self->use_text_overlay) {
    if (self->text_overlay) {
      gst_element_unlink (self->text_overlay, self->video_sink);
      gst_bin_remove (GST_BIN (self), self->text_overlay);
      self->text_overlay = NULL;
    }
    target_pad = gst_element_get_static_pad (self->video_sink, "sink");
  }
  gst_ghost_pad_set_target (GST_GHOST_PAD (self->ghost_pad), target_pad);
  gst_object_unref (target_pad);

  /* Set a timeout for the fps display */
  self->timeout_id =
      g_timeout_add (FPS_DISPLAY_INTERVAL_MS,
      display_current_fps, (gpointer) self);
}

static void
fps_display_sink_stop (GstFPSDisplaySink * self)
{
  /* remove the timeout */
  if (self->timeout_id) {
    g_source_remove (self->timeout_id);
    self->timeout_id = 0;
  }

  if (self->text_overlay) {
    gst_element_unlink (self->text_overlay, self->video_sink);
    gst_bin_remove (GST_BIN (self), self->text_overlay);
    gst_object_unref (self->text_overlay);
    self->text_overlay = NULL;
  }
}

static void
fps_display_sink_dispose (GObject * object)
{
  GstFPSDisplaySink *self = GST_FPS_DISPLAY_SINK (object);

  if (self->query) {
    gst_query_unref (self->query);
    self->query = NULL;
  }

  if (self->video_sink) {
    gst_object_unref (self->video_sink);
    self->video_sink = NULL;
  }

  if (self->text_overlay) {
    gst_object_unref (self->text_overlay);
    self->text_overlay = NULL;
  }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
fps_display_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstFPSDisplaySink *self = GST_FPS_DISPLAY_SINK (object);

  switch (prop_id) {
    case ARG_SYNC:
      self->sync = g_value_get_boolean (value);
      g_object_set (self->video_sink, "sync", self->sync, NULL);
      break;
    case ARG_TEXT_OVERLAY:
      self->use_text_overlay = g_value_get_boolean (value);

      if (self->text_overlay) {
        if (!self->use_text_overlay) {
          GST_DEBUG_OBJECT (self, "text-overlay set to false");
          g_object_set (self->text_overlay, "text", "", "silent", TRUE, NULL);
        } else {
          GST_DEBUG_OBJECT (self, "text-overlay set to true");
          g_object_set (self->text_overlay, "silent", FALSE, NULL);
        }
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
fps_display_sink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstFPSDisplaySink *self = GST_FPS_DISPLAY_SINK (object);

  switch (prop_id) {
    case ARG_SYNC:
      g_value_set_boolean (value, self->sync);
      break;
    case ARG_TEXT_OVERLAY:
      g_value_set_boolean (value, self->use_text_overlay);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstStateChangeReturn
fps_display_sink_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstFPSDisplaySink *self = GST_FPS_DISPLAY_SINK (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      fps_display_sink_start (self);
      break;
    default:
      break;
  }

  ret = GST_CALL_PARENT_WITH_DEFAULT (GST_ELEMENT_CLASS, change_state,
      (element, transition), GST_STATE_CHANGE_SUCCESS);

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_NULL:
      fps_display_sink_stop (self);
      break;
    default:
      break;
  }

  return ret;
}

GType
fps_display_sink_get_type (void)
{
  static GType fps_display_sink_type = 0;

  if (!fps_display_sink_type) {
    static const GTypeInfo fps_display_sink_info = {
      sizeof (GstFPSDisplaySinkClass),
      NULL,
      NULL,
      (GClassInitFunc) fps_display_sink_class_init,
      NULL,
      NULL,
      sizeof (GstFPSDisplaySink),
      0,
      (GInstanceInitFunc) fps_display_sink_init,
    };

    fps_display_sink_type = g_type_register_static (GST_TYPE_BIN,
        "GstFPSDisplaySink", &fps_display_sink_info, 0);

    GST_DEBUG_CATEGORY_INIT (fps_display_sink_debug, "fpsdisplaysink", 0,
        "FPS Display Sink");
  }

  return fps_display_sink_type;
}
