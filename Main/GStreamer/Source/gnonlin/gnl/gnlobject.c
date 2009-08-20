/* Gnonlin
 * Copyright (C) <2001> Wim Taymans <wim.taymans@gmail.com>
 *               <2004-2008> Edward Hervey <bilboed@bilboed.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "gnl.h"

/**
 * SECTION:gnlobject
 * @short_description: Base class for GNonLin elements
 * 
 * <refsect2>
 * <para>
 * GnlObject encapsulates default behaviour and implements standard
 * properties provided by all the GNonLin elements.
 * </para>
 * </refsect2>
 *  
 */


GST_BOILERPLATE (GnlObject, gnl_object, GstBin, GST_TYPE_BIN);

GST_DEBUG_CATEGORY_STATIC (gnlobject);
#define GST_CAT_DEFAULT gnlobject

enum
{
  ARG_0,
  ARG_START,
  ARG_DURATION,
  ARG_STOP,
  ARG_MEDIA_START,
  ARG_MEDIA_DURATION,
  ARG_MEDIA_STOP,
  ARG_RATE,
  ARG_PRIORITY,
  ARG_ACTIVE,
  ARG_CAPS,
  ARG_EXPANDABLE
};

static void gnl_object_dispose (GObject * object);
static void
gnl_object_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);

static void
gnl_object_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static GstStateChangeReturn
gnl_object_change_state (GstElement * element, GstStateChange transition);

static gboolean gnl_object_prepare_func (GnlObject * object);
static gboolean gnl_object_cleanup_func (GnlObject * object);

static GstStateChangeReturn gnl_object_prepare (GnlObject * object);

static void
gnl_object_base_init (gpointer g_class G_GNUC_UNUSED)
{

};

static void
gnl_object_class_init (GnlObjectClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GnlObjectClass *gnlobject_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gnlobject_class = (GnlObjectClass *) klass;

  GST_DEBUG_CATEGORY_INIT (gnlobject, "gnlobject",
      GST_DEBUG_FG_BLUE | GST_DEBUG_BOLD, "GNonLin Object base class");

  gobject_class->set_property = GST_DEBUG_FUNCPTR (gnl_object_set_property);
  gobject_class->get_property = GST_DEBUG_FUNCPTR (gnl_object_get_property);
  gobject_class->dispose = GST_DEBUG_FUNCPTR (gnl_object_dispose);

  gstelement_class->change_state = GST_DEBUG_FUNCPTR (gnl_object_change_state);

  gnlobject_class->prepare = GST_DEBUG_FUNCPTR (gnl_object_prepare_func);
  gnlobject_class->cleanup = GST_DEBUG_FUNCPTR (gnl_object_cleanup_func);

  /**
   * GnlObject:start
   *
   * The start position relative to the parent in nanoseconds.
   */
  g_object_class_install_property (gobject_class, ARG_START,
      g_param_spec_uint64 ("start", "Start",
          "The start position relative to the parent",
          0, G_MAXUINT64, 0, G_PARAM_READWRITE));

  /**
   * GnlObject:duration
   *
   * The outgoing duration in nanoseconds.
   */
  g_object_class_install_property (gobject_class, ARG_DURATION,
      g_param_spec_int64 ("duration", "Duration",
          "Outgoing duration", 0, G_MAXINT64, 0, G_PARAM_READWRITE));

  /**
   * GnlObject:stop
   *
   * The stop position relative to the parent in nanoseconds.
   *
   * This value is computed based on the values of start and duration.
   */
  g_object_class_install_property (gobject_class, ARG_STOP,
      g_param_spec_uint64 ("stop", "Stop",
          "The stop position relative to the parent",
          0, G_MAXUINT64, 0, G_PARAM_READABLE));

  /**
   * GnlObject:media_start
   *
   * The media start position in nanoseconds.
   *
   * Also called 'in-point' in video-editing, this corresponds to
   * what position in the 'contained' object we should start outputting from.
   */
  g_object_class_install_property (gobject_class, ARG_MEDIA_START,
      g_param_spec_uint64 ("media_start", "Media start",
          "The media start position",
          0, G_MAXUINT64, GST_CLOCK_TIME_NONE, G_PARAM_READWRITE));

  /**
   * GnlObject:media_duration
   *
   * The media's duration in nanoseconds.
   *
   * This correspond to the 'contained' object's duration we will be
   * outputting for.
   */
  g_object_class_install_property (gobject_class, ARG_MEDIA_DURATION,
      g_param_spec_int64 ("media_duration", "Media duration",
          "Duration of the media, can be negative",
          G_MININT64, G_MAXINT64, 0, G_PARAM_READWRITE));

  /**
   * GnlObject:media_stop
   *
   * The media stop position in nanoseconds.
   *
   * Also called 'out-point' in video-editing, this corresponds to the
   * position in the 'contained' object we should output until.
   *
   * This value is read-only, and is computed from the media_start and
   * media_duration property.
   */
  g_object_class_install_property (gobject_class, ARG_MEDIA_STOP,
      g_param_spec_uint64 ("media_stop", "Media stop",
          "The media stop position",
          0, G_MAXUINT64, GST_CLOCK_TIME_NONE, G_PARAM_READABLE));

  /**
   * GnlObject:rate
   *
   * The rate applied to the controlled output.
   *
   * This is a read-only value computed from duration and media_duration.
   */
  g_object_class_install_property (gobject_class, ARG_RATE,
      g_param_spec_double ("rate", "Rate",
          "Playback rate of the media",
          -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, G_PARAM_READABLE));

  /**
   * GnlObject:priority
   *
   * The priority of the object in the container.
   *
   * The highest priority is 0, meaning this object will be selected over
   * any other between start and stop.
   *
   * The lowest priority is G_MAXUINT32.
   *
   * Objects whose priority is (-1) will be considered as 'default' objects
   * in GnlComposition and their start/stop values will be modified as to
   * fit the whole duration of the composition.
   */
  g_object_class_install_property (gobject_class, ARG_PRIORITY,
      g_param_spec_uint ("priority", "Priority",
          "The priority of the object (0 = highest priority)", 0, G_MAXUINT, 0,
          G_PARAM_READWRITE));

  /**
   * GnlObject:active
   *
   * Indicates whether this object should be used by its container.
   *
   * Set to #TRUE to temporarily disable this object in a #GnlComposition.
   */
  g_object_class_install_property (gobject_class, ARG_ACTIVE,
      g_param_spec_boolean ("active", "Active",
          "Use this object in the GnlComposition", TRUE, G_PARAM_READWRITE));

  /**
   * GnlObject:caps
   *
   * Caps used to filter/choose the output stream.
   *
   * If the controlled object produces several stream, you can set this
   * property to choose a specific stream.
   *
   * If nothing is specified then a source pad will be chosen at random.
   */
  g_object_class_install_property (gobject_class, ARG_CAPS,
      g_param_spec_boxed ("caps", "Caps",
          "Caps used to filter/choose the output stream",
          GST_TYPE_CAPS, G_PARAM_READWRITE));

  /**
   * GnlObject:expandable
   *
   * Indicates whether this object should expand to the full duration of its
   * container #GnlComposition.
   */
  g_object_class_install_property (gobject_class, ARG_EXPANDABLE,
      g_param_spec_boolean ("expandable", "Expandable",
          "Expand to the full duration of the container composition", FALSE,
          G_PARAM_READWRITE));
}

static void
gnl_object_init (GnlObject * object, GnlObjectClass * klass)
{
  object->start = 0;
  object->duration = 0;
  object->stop = 0;

  object->media_start = GST_CLOCK_TIME_NONE;
  object->media_duration = 0;
  object->media_stop = GST_CLOCK_TIME_NONE;

  object->rate = 1.0;
  object->rate_1 = TRUE;
  object->priority = 0;
  object->active = TRUE;

  object->caps = gst_caps_new_any ();

  object->segment_rate = 1.0;
  object->segment_start = -1;
  object->segment_stop = -1;
}

static void
gnl_object_dispose (GObject * object)
{
  GnlObject *gnl = (GnlObject *) object;

  if (gnl->caps) {
    gst_caps_unref (gnl->caps);
    gnl->caps = NULL;
  }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

/**
 * gnl_object_to_media_time:
 * @object: a #GnlObject
 * @objecttime: The #GstClockTime we want to convert
 * @mediatime: A pointer on a #GstClockTime to fill
 *
 * Converts a #GstClockTime from the object (container) context to the media context
 *
 * Returns: TRUE if @objecttime was within the limits of the @object start/stop time,
 * FALSE otherwise
 */
gboolean
gnl_object_to_media_time (GnlObject * object, GstClockTime otime,
    GstClockTime * mtime)
{
  g_return_val_if_fail (mtime, FALSE);

  GST_DEBUG_OBJECT (object, "ObjectTime : %" GST_TIME_FORMAT,
      GST_TIME_ARGS (otime));

  GST_DEBUG_OBJECT (object,
      "Start/Stop:[%" GST_TIME_FORMAT " -- %" GST_TIME_FORMAT "] "
      "Media [%" GST_TIME_FORMAT " -- %" GST_TIME_FORMAT "]",
      GST_TIME_ARGS (object->start),
      GST_TIME_ARGS (object->stop),
      GST_TIME_ARGS (object->media_start), GST_TIME_ARGS (object->media_stop));

  /* limit check */
  if (G_UNLIKELY ((otime < object->start))) {
    GST_DEBUG_OBJECT (object, "ObjectTime is before start");
    *mtime =
        (object->media_start == GST_CLOCK_TIME_NONE) ? 0 : object->media_start;
    return FALSE;
  }
  if (G_UNLIKELY ((otime >= object->stop))) {
    GST_DEBUG_OBJECT (object, "ObjectTime is after stop");
    if (G_LIKELY (GST_CLOCK_TIME_IS_VALID (object->media_stop)))
      *mtime = object->media_stop;
    else if (GST_CLOCK_TIME_IS_VALID (object->media_start))
      *mtime = object->media_start + object->duration;
    else
      *mtime = object->stop - object->start;
    return FALSE;
  }

  if (G_UNLIKELY (object->media_start == GST_CLOCK_TIME_NONE)) {
    /* no time shifting, for live sources ? */
    *mtime = otime - object->start;
  } else if (G_LIKELY (object->rate_1)) {
    *mtime = otime - object->start + object->media_start;
  } else {
    *mtime = (otime - object->start) * object->rate + object->media_start;
  }

  GST_DEBUG_OBJECT (object, "Returning MediaTime : %" GST_TIME_FORMAT,
      GST_TIME_ARGS (*mtime));

  return TRUE;
}

/**
 * gnl_media_to_object_time:
 * @object:
 * @mediatime: The #GstClockTime we want to convert
 * @objecttime: A pointer on a #GstClockTime to fill
 *
 * Converts a #GstClockTime from the media context to the object (container) context
 *
 * Returns: TRUE if @objecttime was within the limits of the @object media start/stop time,
 * FALSE otherwise
 */

gboolean
gnl_media_to_object_time (GnlObject * object, GstClockTime mtime,
    GstClockTime * otime)
{
  g_return_val_if_fail (otime, FALSE);

  GST_DEBUG_OBJECT (object, "MediaTime : %" GST_TIME_FORMAT,
      GST_TIME_ARGS (mtime));

  GST_DEBUG_OBJECT (object,
      "Start/Stop:[%" GST_TIME_FORMAT " -- %" GST_TIME_FORMAT "] "
      "Media [%" GST_TIME_FORMAT " -- %" GST_TIME_FORMAT "]",
      GST_TIME_ARGS (object->start),
      GST_TIME_ARGS (object->stop),
      GST_TIME_ARGS (object->media_start), GST_TIME_ARGS (object->media_stop));


  /* limit check */
  if (G_UNLIKELY ((object->media_start != GST_CLOCK_TIME_NONE)
          && (mtime < object->media_start))) {
    GST_DEBUG_OBJECT (object,
        "media time is before media_start, forcing to start");
    *otime = object->start;
    return FALSE;
  }
  if (G_UNLIKELY ((object->media_stop != GST_CLOCK_TIME_NONE)
          && (mtime >= object->media_stop))) {
    GST_DEBUG_OBJECT (object,
        "media time is at or after media_stop, forcing to stop");
    *otime = object->stop;
    return FALSE;
  }

  if (G_LIKELY (object->rate_1 && (object->media_start != GST_CLOCK_TIME_NONE))) {
    *otime = mtime - object->media_start + object->start;
  } else if (object->media_start == GST_CLOCK_TIME_NONE)
    *otime = mtime + object->start;
  else
    *otime = (mtime - object->media_start) / object->rate + object->start;

  GST_DEBUG_OBJECT (object, "Returning ObjectTime : %" GST_TIME_FORMAT,
      GST_TIME_ARGS (*otime));
  return TRUE;
}

static gboolean
gnl_object_prepare_func (GnlObject * object)
{
  GST_DEBUG_OBJECT (object, "default prepare function, returning TRUE");

  return TRUE;
}

static GstStateChangeReturn
gnl_object_prepare (GnlObject * object)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

  GST_DEBUG_OBJECT (object, "preparing");

  if (!(GNL_OBJECT_GET_CLASS (object)->prepare (object)))
    ret = GST_STATE_CHANGE_FAILURE;

  GST_DEBUG_OBJECT (object, "finished preparing, returning %d", ret);

  return ret;
}

static gboolean
gnl_object_cleanup_func (GnlObject * object)
{
  GST_DEBUG_OBJECT (object, "default cleanup function, returning TRUE");

  return TRUE;
}

static GstStateChangeReturn
gnl_object_cleanup (GnlObject * object)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

  GST_DEBUG_OBJECT (object, "cleaning-up");

  if (!(GNL_OBJECT_GET_CLASS (object)->cleanup (object)))
    ret = GST_STATE_CHANGE_FAILURE;

  GST_DEBUG_OBJECT (object, "finished preparing, returning %d", ret);

  return ret;
}

static void
gnl_object_set_caps (GnlObject * object, const GstCaps * caps)
{
  if (object->caps)
    gst_caps_unref (object->caps);

  object->caps = gst_caps_copy (caps);
}

static void
update_values (GnlObject * object)
{
  /* check if start/duration has changed */
  if ((object->start + object->duration) != object->stop) {
    object->stop = object->start + object->duration;
    GST_LOG_OBJECT (object,
        "Updating stop value : %" GST_TIME_FORMAT " [start:%" GST_TIME_FORMAT
        ", duration:%" GST_TIME_FORMAT "]", GST_TIME_ARGS (object->stop),
        GST_TIME_ARGS (object->start), GST_TIME_ARGS (object->duration));
    g_object_notify (G_OBJECT (object), "stop");
  }

  /* check if media start/duration has changed */
  if ((object->media_start != GST_CLOCK_TIME_NONE)
      && ((object->media_start + object->media_duration) != object->media_stop)) {
    object->media_stop = object->media_start + object->media_duration;
    GST_LOG_OBJECT (object,
        "Updated media_stop value : %" GST_TIME_FORMAT
        " [mstart:%" GST_TIME_FORMAT ", mduration:%" GST_TIME_FORMAT "]",
        GST_TIME_ARGS (object->media_stop),
        GST_TIME_ARGS (object->media_start),
        GST_TIME_ARGS (object->media_duration));
    g_object_notify (G_OBJECT (object), "media_stop");
  }

  /* check if rate has changed */
  if ((object->media_duration != GST_CLOCK_TIME_NONE)
      && (object->duration)
      && (object->media_duration)
      && (((gdouble) object->media_duration / (gdouble) object->duration) !=
          object->rate)) {
    object->rate =
        (gdouble) object->media_duration / (gdouble) object->duration;
    /* update the speedup rate_1 variable */
    object->rate_1 = (object->media_duration == object->duration);
    GST_LOG_OBJECT (object,
        "Updated rate : %f [mduration:%" GST_TIME_FORMAT ", duration:%"
        GST_TIME_FORMAT "] rate_1:%d", object->rate,
        GST_TIME_ARGS (object->media_duration),
        GST_TIME_ARGS (object->duration), object->rate_1);
    g_object_notify (G_OBJECT (object), "rate");
  }
}

static void
gnl_object_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GnlObject *gnlobject = (GnlObject *) object;

  g_return_if_fail (GNL_IS_OBJECT (object));

  switch (prop_id) {
    case ARG_START:
      gnlobject->start = g_value_get_uint64 (value);
      update_values (gnlobject);
      break;
    case ARG_DURATION:
      gnlobject->duration = g_value_get_int64 (value);
      update_values (gnlobject);
      break;
    case ARG_MEDIA_START:
      gnlobject->media_start = g_value_get_uint64 (value);
      break;
    case ARG_MEDIA_DURATION:
      gnlobject->media_duration = g_value_get_int64 (value);
      update_values (gnlobject);
      break;
    case ARG_PRIORITY:
      gnlobject->priority = g_value_get_uint (value);
      break;
    case ARG_ACTIVE:
      gnlobject->active = g_value_get_boolean (value);
      break;
    case ARG_CAPS:
      gnl_object_set_caps (gnlobject, gst_value_get_caps (value));
      break;
    case ARG_EXPANDABLE:
      if (g_value_get_boolean (value))
        GST_OBJECT_FLAG_SET (gnlobject, GNL_OBJECT_EXPANDABLE);
      else
        GST_OBJECT_FLAG_UNSET (gnlobject, GNL_OBJECT_EXPANDABLE);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gnl_object_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GnlObject *gnlobject = (GnlObject *) object;

  switch (prop_id) {
    case ARG_START:
      g_value_set_uint64 (value, gnlobject->start);
      break;
    case ARG_DURATION:
      g_value_set_int64 (value, gnlobject->duration);
      break;
    case ARG_STOP:
      g_value_set_uint64 (value, gnlobject->stop);
      break;
    case ARG_MEDIA_START:
      g_value_set_uint64 (value, gnlobject->media_start);
      break;
    case ARG_MEDIA_DURATION:
      g_value_set_int64 (value, gnlobject->media_duration);
      break;
    case ARG_MEDIA_STOP:
      g_value_set_uint64 (value, gnlobject->media_stop);
      break;
    case ARG_RATE:
      g_value_set_double (value, gnlobject->rate);
      break;
    case ARG_PRIORITY:
      g_value_set_uint (value, gnlobject->priority);
      break;
    case ARG_ACTIVE:
      g_value_set_boolean (value, gnlobject->active);
      break;
    case ARG_CAPS:
      gst_value_set_caps (value, gnlobject->caps);
      break;
    case ARG_EXPANDABLE:
      g_value_set_boolean (value, GNL_OBJECT_IS_EXPANDABLE (object));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstStateChangeReturn
gnl_object_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      if (gnl_object_prepare (GNL_OBJECT (element)) == GST_STATE_CHANGE_FAILURE) {
        ret = GST_STATE_CHANGE_FAILURE;
        goto beach;
      }
      break;
    default:
      break;
  }

  GST_DEBUG_OBJECT (element, "Calling parent change_state");

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  GST_DEBUG_OBJECT (element, "Return from parent change_state was %d", ret);

  if (ret == GST_STATE_CHANGE_FAILURE)
    goto beach;

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      /* cleanup gnlobject */
      if (gnl_object_cleanup (GNL_OBJECT (element)) == GST_STATE_CHANGE_FAILURE)
        ret = GST_STATE_CHANGE_FAILURE;
      break;
    default:
      break;
  }

beach:
  return ret;
}
