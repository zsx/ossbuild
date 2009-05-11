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

typedef struct _GnlPadPrivate GnlPadPrivate;

struct _GnlPadPrivate
{
  GnlObject *object;
  GnlPadPrivate *ghostpriv;
  GstPadDirection dir;
  GstPadEventFunction eventfunc;
  GstPadQueryFunction queryfunc;
};

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
};

static void gnl_object_dispose (GObject * object);
static void
gnl_object_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);

static void
gnl_object_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean
gnl_object_covers_func (GnlObject * object,
    GstClockTime start, GstClockTime stop, GnlCoverType type);
static GstStateChangeReturn
gnl_object_change_state (GstElement * element, GstStateChange transition);

static gboolean gnl_object_prepare_func (GnlObject * object);
static gboolean gnl_object_cleanup_func (GnlObject * object);

static GstStateChangeReturn gnl_object_prepare (GnlObject * object);

static void gnl_object_handle_message (GstBin * bin, GstMessage * message);

static void
gnl_object_base_init (gpointer g_class)
{

};

static void
gnl_object_class_init (GnlObjectClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBinClass *gstbin_class;
  GnlObjectClass *gnlobject_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbin_class = (GstBinClass *) klass;
  gnlobject_class = (GnlObjectClass *) klass;

  GST_DEBUG_CATEGORY_INIT (gnlobject, "gnlobject",
      GST_DEBUG_FG_BLUE | GST_DEBUG_BOLD, "GNonLin Object base class");

  gobject_class->set_property = GST_DEBUG_FUNCPTR (gnl_object_set_property);
  gobject_class->get_property = GST_DEBUG_FUNCPTR (gnl_object_get_property);
  gobject_class->dispose = GST_DEBUG_FUNCPTR (gnl_object_dispose);

  gstelement_class->change_state = GST_DEBUG_FUNCPTR (gnl_object_change_state);

  gstbin_class->handle_message = GST_DEBUG_FUNCPTR (gnl_object_handle_message);

  gnlobject_class->covers = GST_DEBUG_FUNCPTR (gnl_object_covers_func);
  gnlobject_class->prepare = GST_DEBUG_FUNCPTR (gnl_object_prepare_func);
  gnlobject_class->cleanup = GST_DEBUG_FUNCPTR (gnl_object_cleanup_func);

  /**
   * GnlObject:start:
   *
   */
  g_object_class_install_property (gobject_class, ARG_START,
      g_param_spec_uint64 ("start", "Start",
          "The start position relative to the parent",
          0, G_MAXUINT64, 0, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, ARG_DURATION,
      g_param_spec_int64 ("duration", "Duration",
          "Outgoing duration", 0, G_MAXINT64, 0, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, ARG_STOP,
      g_param_spec_uint64 ("stop", "Stop",
          "The stop position relative to the parent",
          0, G_MAXUINT64, 0, G_PARAM_READABLE));

  g_object_class_install_property (gobject_class, ARG_MEDIA_START,
      g_param_spec_uint64 ("media_start", "Media start",
          "The media start position",
          0, G_MAXUINT64, GST_CLOCK_TIME_NONE, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, ARG_MEDIA_DURATION,
      g_param_spec_int64 ("media_duration", "Media duration",
          "Duration of the media, can be negative",
          G_MININT64, G_MAXINT64, 0, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, ARG_MEDIA_STOP,
      g_param_spec_uint64 ("media_stop", "Media stop",
          "The media stop position",
          0, G_MAXUINT64, GST_CLOCK_TIME_NONE, G_PARAM_READABLE));

  g_object_class_install_property (gobject_class, ARG_RATE,
      g_param_spec_double ("rate", "Rate",
          "Playback rate of the media",
          -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, G_PARAM_READABLE));

  g_object_class_install_property (gobject_class, ARG_PRIORITY,
      g_param_spec_uint ("priority", "Priority",
          "The priority of the object (0 = highest priority)", 0, G_MAXUINT, 0,
          G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, ARG_ACTIVE,
      g_param_spec_boolean ("active", "Active", "Render this object", TRUE,
          G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, ARG_CAPS,
      g_param_spec_boxed ("caps", "Caps",
          "Caps used to filter/choose the output stream",
          GST_TYPE_CAPS, G_PARAM_READWRITE));
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
  if ((otime < object->start) || (otime >= object->stop)) {
    GST_DEBUG_OBJECT (object, "ObjectTime is outside object start/stop times");
    if (otime < object->start) {
      *mtime =
          (object->media_start ==
          GST_CLOCK_TIME_NONE) ? object->start : object->media_start;
    } else {
      if (GST_CLOCK_TIME_IS_VALID (object->media_stop))
        *mtime = object->media_stop;
      else if (GST_CLOCK_TIME_IS_VALID (object->media_start))
        *mtime = object->media_start + object->duration;
      else
        *mtime = object->stop;
    }
    return FALSE;
  }

  if (object->media_start == GST_CLOCK_TIME_NONE) {
    /* no time shifting, for live sources ? */
    *mtime = otime;
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
  if (object->media_start == GST_CLOCK_TIME_NONE)
    return gnl_object_to_media_time (object, mtime, otime);

  if (mtime < object->media_start) {
    GST_DEBUG_OBJECT (object,
        "media time is before media_start, forcing to start");
    *otime = object->start;
    return FALSE;
  } else if ((object->media_stop != GST_CLOCK_TIME_NONE)
      && (mtime >= object->media_stop)) {
    GST_DEBUG_OBJECT (object,
        "media time is at or after media_stop, forcing to stop");
    *otime = object->stop;
  } else
    *otime = (mtime - object->media_start) / object->rate + object->start;

  GST_DEBUG_OBJECT (object, "Returning ObjectTime : %" GST_TIME_FORMAT,
      GST_TIME_ARGS (*otime));
  return TRUE;
}

static gboolean
gnl_object_covers_func (GnlObject * object,
    GstClockTime start, GstClockTime stop, GnlCoverType type)
{
  gboolean ret = FALSE;

  GST_DEBUG_OBJECT (object,
      "start:%" GST_TIME_FORMAT ", stop:%" GST_TIME_FORMAT ", type:%d",
      GST_TIME_ARGS (start), GST_TIME_ARGS (stop), type);

  /* FIXME: BOGUS, REMOVE */
  gnl_media_to_object_time (object, 0, NULL);

  switch (type) {
    case GNL_COVER_ALL:
    case GNL_COVER_SOME:
      if ((start <= object->start) && (stop >= object->stop))
        ret = TRUE;
      break;
    case GNL_COVER_START:
      if ((start >= object->start) && (start < object->stop))
        ret = TRUE;
      break;
    case GNL_COVER_STOP:
      if ((stop >= object->start) && (stop < object->stop))
        ret = TRUE;
      break;
    default:
      break;
  }
  return ret;
};

gboolean
gnl_object_covers (GnlObject * object, GstClockTime start,
    GstClockTime stop, GnlCoverType type)
{
  return GNL_OBJECT_GET_CLASS (object)->covers (object, start, stop, type);
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

static GstEvent *
translate_incoming_seek (GnlObject * object, GstEvent * event)
{
  GstEvent *event2;
  GstFormat format;
  gdouble rate, nrate;
  GstSeekFlags flags;
  GstSeekType curtype, stoptype;
  GstSeekType ncurtype;
  gint64 cur;
  guint64 ncur;
  gint64 stop;
  guint64 nstop;

  gst_event_parse_seek (event, &rate, &format, &flags,
      &curtype, &cur, &stoptype, &stop);

  GST_DEBUG_OBJECT (object,
      "GOT SEEK rate:%f, format:%d, flags:%d, curtype:%d, stoptype:%d, %"
      GST_TIME_FORMAT " -- %" GST_TIME_FORMAT, rate, format, flags, curtype,
      stoptype, GST_TIME_ARGS (cur), GST_TIME_ARGS (stop));

  if (format != GST_FORMAT_TIME)
    goto invalid_format;

  /* convert rate */
  nrate = rate * object->rate;
  GST_DEBUG ("nrate:%f , rate:%f, object->rate:%f", nrate, rate, object->rate);

  /* convert cur */
  ncurtype = GST_SEEK_TYPE_SET;
  if ((curtype == GST_SEEK_TYPE_SET)
      && (gnl_object_to_media_time (object, cur, &ncur))) {
    if (ncur > G_MAXINT64)
      GST_WARNING_OBJECT (object, "return value too big...");
    GST_LOG_OBJECT (object, "Setting cur to %" GST_TIME_FORMAT,
        GST_TIME_ARGS (ncur));
  } else if ((curtype != GST_SEEK_TYPE_NONE)) {
    GST_DEBUG_OBJECT (object, "Limiting seek start to media_start");
    ncur = object->media_start;
  } else {
    GST_DEBUG_OBJECT (object, "leaving GST_SEEK_TYPE_NONE");
    ncur = cur;
    ncurtype = GST_SEEK_TYPE_NONE;
  }

  /* convert stop, we also need to limit it to object->stop */
  if ((stoptype == GST_SEEK_TYPE_SET)
      && (gnl_object_to_media_time (object, stop, &nstop))) {
    if (nstop > G_MAXINT64)
      GST_WARNING_OBJECT (object, "return value too big...");
    GST_LOG_OBJECT (object, "Setting stop to %" GST_TIME_FORMAT,
        GST_TIME_ARGS (nstop));
  } else {
    GST_DEBUG_OBJECT (object, "Limiting end of seek to media_stop");
    gnl_object_to_media_time (object, object->stop, &nstop);
    if (nstop > G_MAXINT64)
      GST_WARNING_OBJECT (object, "return value too big...");
    GST_LOG_OBJECT (object, "Setting stop to %" GST_TIME_FORMAT,
        GST_TIME_ARGS (nstop));
  }

  /* add segment seekflags */
  if (!(flags & GST_SEEK_FLAG_SEGMENT)) {
    GST_DEBUG_OBJECT (object, "Adding GST_SEEK_FLAG_SEGMENT");
    flags |= GST_SEEK_FLAG_SEGMENT;
  } else {
    GST_DEBUG_OBJECT (object,
        "event already has GST_SEEK_FLAG_SEGMENT : %d", flags);
  }

  /* add accurate seekflags */
  if (!(flags & GST_SEEK_FLAG_ACCURATE)) {
    GST_DEBUG_OBJECT (object, "Adding GST_SEEK_FLAG_ACCURATE");
    flags |= GST_SEEK_FLAG_ACCURATE;
  } else {
    GST_DEBUG_OBJECT (object,
        "event already has GST_SEEK_FLAG_ACCURATE : %d", flags);
  }



  GST_DEBUG_OBJECT (object,
      "SENDING SEEK rate:%f, format:TIME, flags:%d, curtype:%d, stoptype:SET, %"
      GST_TIME_FORMAT " -- %" GST_TIME_FORMAT, nrate, flags, ncurtype,
      GST_TIME_ARGS (ncur), GST_TIME_ARGS (nstop));

  event2 = gst_event_new_seek (nrate, GST_FORMAT_TIME, flags,
      ncurtype, (gint64) ncur, GST_SEEK_TYPE_SET, (gint64) nstop);

  gst_event_unref (event);

  return event2;

  /* ERRORS */
invalid_format:
  {
    GST_WARNING ("GNonLin time shifting only works with GST_FORMAT_TIME");
    return event;
  }
}

static GstEvent *
translate_outgoing_seek (GnlObject * object, GstEvent * event)
{
  GST_DEBUG_OBJECT (object,
      "TODO shifting cur/stop/rate of seek event to container time domain");

  return event;
}

static GstEvent *
translate_outgoing_new_segment (GnlObject * object, GstEvent * event)
{
  GstEvent *event2;
  gboolean update;
  gdouble rate;
  GstFormat format;
  gint64 start, stop, stream;
  guint64 nstream;

  /* only modify the streamtime */
  gst_event_parse_new_segment (event, &update, &rate, &format,
      &start, &stop, &stream);

  GST_DEBUG_OBJECT (object,
      "Got NEWSEGMENT %" GST_TIME_FORMAT " -- %" GST_TIME_FORMAT " // %"
      GST_TIME_FORMAT, GST_TIME_ARGS (start), GST_TIME_ARGS (stop),
      GST_TIME_ARGS (stream));

  if (format != GST_FORMAT_TIME) {
    GST_WARNING_OBJECT (object,
        "Can't translate newsegments with format != GST_FORMAT_TIME");
    return event;
  }

  gnl_media_to_object_time (object, stream, &nstream);

  if (nstream > G_MAXINT64)
    GST_WARNING_OBJECT (object, "Return value too big...");

  GST_DEBUG_OBJECT (object,
      "Sending NEWSEGMENT %" GST_TIME_FORMAT " -- %" GST_TIME_FORMAT " // %"
      GST_TIME_FORMAT, GST_TIME_ARGS (start), GST_TIME_ARGS (stop),
      GST_TIME_ARGS (nstream));
  event2 =
      gst_event_new_new_segment (update, rate, format, start, stop,
      (gint64) nstream);
  gst_event_unref (event);

  return event2;
}

static gboolean
internalpad_event_function (GstPad * internal, GstEvent * event)
{
  GnlPadPrivate *priv = gst_pad_get_element_private (internal);
  GnlObject *object = priv->object;
  GstMessage *message = NULL;
  gboolean res;

  GST_DEBUG_OBJECT (internal, "event:%s", GST_EVENT_TYPE_NAME (event));

  if (!(priv->eventfunc)) {
    GST_WARNING_OBJECT (internal,
        "priv->eventfunc == NULL !! What is going on ?");
    return FALSE;
  }

  switch (priv->dir) {
    case GST_PAD_SRC:
      if (GST_EVENT_TYPE (event) == GST_EVENT_NEWSEGMENT) {
        event = translate_outgoing_new_segment (object, event);
        message = gst_message_new_segment_start (GST_OBJECT (object),
            GST_FORMAT_TIME, (gint64) object->start);
      }

      break;
    case GST_PAD_SINK:
      if (GST_EVENT_TYPE (event) == GST_EVENT_SEEK) {
        event = translate_outgoing_seek (object, event);
      }
      break;
    default:
      break;
  }
  GST_DEBUG_OBJECT (internal, "Calling priv->eventfunc %p", priv->eventfunc);
  res = priv->eventfunc (internal, event);
  if (message)
    gst_element_post_message (GST_ELEMENT (object), message);

  return res;
}

/*
  translate_outgoing_position_query

  Should only be called:
  _ if the query is a GST_QUERY_POSITION
  _ after the query was sent upstream
  _ if the upstream query returned TRUE
*/

static gboolean
translate_incoming_position_query (GnlObject * object, GstQuery * query)
{
  GstFormat format;
  gint64 cur, cur2;

  gst_query_parse_position (query, &format, &cur);
  if (format != GST_FORMAT_TIME) {
    GST_WARNING_OBJECT (object,
        "position query is in a format different from time, returning without modifying values");
    goto beach;
  }

  if (!(gnl_media_to_object_time (object, (guint64) cur, (guint64 *) & cur2))) {
    GST_WARNING_OBJECT (object,
        "Couldn't get object time for %" GST_TIME_FORMAT, GST_TIME_ARGS (cur));
    goto beach;
  }

  gst_query_set_position (query, GST_FORMAT_TIME, cur2);

beach:
  return TRUE;
}

static gboolean
internalpad_query_function (GstPad * internal, GstQuery * query)
{
  GnlPadPrivate *priv = gst_pad_get_element_private (internal);

/*   GnlObject	*object = GNL_OBJECT (GST_PAD_PARENT (internal)); */

  GST_DEBUG_OBJECT (internal, "querytype:%d", GST_QUERY_TYPE (query));

  if (!(priv->queryfunc)) {
    GST_WARNING_OBJECT (internal,
        "priv->queryfunc == NULL !! What is going on ?");
    return FALSE;
  }

  switch (priv->dir) {
    case GST_PAD_SRC:
      break;
    case GST_PAD_SINK:
      break;
    default:
      break;
  }
  return priv->queryfunc (internal, query);
}

static gboolean
ghostpad_event_function (GstPad * ghostpad, GstEvent * event)
{
  GnlPadPrivate *priv;
  GnlObject *object;
  gboolean ret = FALSE;

  priv = gst_pad_get_element_private (ghostpad);
  object = priv->object;

  GST_DEBUG_OBJECT (ghostpad, "event:%s", GST_EVENT_TYPE_NAME (event));

  if (G_UNLIKELY (priv->eventfunc == NULL))
    goto no_function;

  switch (priv->dir) {
    case GST_PAD_SRC:
      if (GST_EVENT_TYPE (event) == GST_EVENT_SEEK) {

        event = translate_incoming_seek (object, event);
      } else if (GST_EVENT_TYPE (event) == GST_EVENT_QOS) {

        /* FIXME : Implement proper QoS time-shifting, for the time being it's
         * just ignored. See #398453 */
        gst_event_unref (event);
        goto beach;
      }
      break;
    default:
      break;
  }

  GST_DEBUG_OBJECT (ghostpad, "Calling priv->eventfunc");
  ret = priv->eventfunc (ghostpad, event);
  GST_DEBUG_OBJECT (ghostpad, "Returned from calling priv->eventfunc : %d",
      ret);

beach:
  return ret;

  /* ERRORS */
no_function:
  {
    GST_WARNING_OBJECT (ghostpad,
        "priv->eventfunc == NULL !! What's going on ?");
    return FALSE;
  }
}

static gboolean
ghostpad_query_function (GstPad * ghostpad, GstQuery * query)
{
  GnlPadPrivate *priv = gst_pad_get_element_private (ghostpad);
  GnlObject *object = GNL_OBJECT (GST_PAD_PARENT (ghostpad));
  gboolean pret;

  GST_DEBUG_OBJECT (ghostpad, "querytype:%d", GST_QUERY_TYPE (query));

  pret = priv->queryfunc (ghostpad, query);

  if (pret) {
    /* translate result */
    switch (GST_QUERY_TYPE (query)) {
      case GST_QUERY_POSITION:
        pret = translate_incoming_position_query (object, query);
        break;
      default:
        break;
    }
  }

  return pret;
}

/* internal pad going away */
static void
internal_pad_finalizing (GnlPadPrivate * priv, GObject * pad)
{
  g_free (priv);
}

static void
control_internal_pad (GstPad * ghostpad, GnlObject * object)
{
  GnlPadPrivate *priv;
  GnlPadPrivate *privghost;
  GstPad *target;
  GstPad *internal;

  if (!ghostpad) {
    GST_DEBUG_OBJECT (object, "We don't have a valid ghostpad !");
    return;
  }
  privghost = gst_pad_get_element_private (ghostpad);
  target = gst_ghost_pad_get_target (GST_GHOST_PAD (ghostpad));

  if (!target) {
    GST_DEBUG_OBJECT (ghostpad,
        "ghostpad doesn't have a target, no need to control the internal pad");
    return;
  }

  GST_LOG_OBJECT (ghostpad, "overriding ghostpad's internal pad function");

  internal = gst_pad_get_peer (target);
  gst_object_unref (target);

  if (!(priv = gst_pad_get_element_private (internal))) {
    GST_DEBUG_OBJECT (internal,
        "Creating a GnlPadPrivate to put in element_private");
    priv = g_new0 (GnlPadPrivate, 1);

    /* Remember existing pad functions */
    priv->eventfunc = GST_PAD_EVENTFUNC (internal);
    priv->queryfunc = GST_PAD_QUERYFUNC (internal);
    gst_pad_set_element_private (internal, priv);

    g_object_weak_ref ((GObject *) internal,
        (GWeakNotify) internal_pad_finalizing, priv);

    /* add query/event function overrides on internal pad */
    gst_pad_set_event_function (internal,
        GST_DEBUG_FUNCPTR (internalpad_event_function));
    gst_pad_set_query_function (internal,
        GST_DEBUG_FUNCPTR (internalpad_query_function));
  } else {
    GST_WARNING_OBJECT (internal,
        "internal pad already had an element_private");
  }

  priv->object = object;
  priv->ghostpriv = privghost;
  priv->dir = GST_PAD_DIRECTION (ghostpad);
  gst_object_unref (internal);
}


/**
 * gnl_object_ghost_pad:
 * @object: #GnlObject to add the ghostpad to
 * @name: Name for the new pad
 * @target: Target #GstPad to ghost
 *
 * Adds a #GstGhostPad overridding the correct pad [query|event]_function so 
 * that time shifting is done correctly
 * The #GstGhostPad is added to the #GnlObject
 *
 * /!\ This function doesn't check if the existing [src|sink] pad was removed
 * first, so you might end up with more pads than wanted
 *
 * Returns: The #GstPad if everything went correctly, else NULL.
 */

GstPad *
gnl_object_ghost_pad_full (GnlObject * object, const gchar * name,
    GstPad * target, gboolean flush_hack)
{
  GstPadDirection dir = GST_PAD_DIRECTION (target);
  GstPad *ghost;

  GST_DEBUG_OBJECT (object, "name:%s, target:%p, flush_hack:%d",
      name, target, flush_hack);

  g_return_val_if_fail (target, FALSE);
  g_return_val_if_fail ((dir != GST_PAD_UNKNOWN), FALSE);

  ghost = gnl_object_ghost_pad_no_target (object, name, dir);
  if (ghost && (!(gnl_object_ghost_pad_set_target (object, ghost, target)))) {
    GST_WARNING_OBJECT (object,
        "Couldn't set the target pad... removing ghostpad");
    gst_object_unref (ghost);
    return NULL;
  }


  /* activate pad */
  gst_pad_set_active (ghost, TRUE);
  /* add it to element */
  if (!(gst_element_add_pad (GST_ELEMENT (object), ghost))) {
    GST_WARNING ("couldn't add newly created ghostpad");
    return NULL;
  }
  control_internal_pad (ghost, object);

  return ghost;
}

GstPad *
gnl_object_ghost_pad (GnlObject * object, const gchar * name, GstPad * target)
{
  return gnl_object_ghost_pad_full (object, name, target, FALSE);
}

/*
 * gnl_object_ghost_pad_no_target:
 * /!\ Doesn't add the pad to the GnlObject....
 */
GstPad *
gnl_object_ghost_pad_no_target (GnlObject * object, const gchar * name,
    GstPadDirection dir)
{
  GstPad *ghost;
  GnlPadPrivate *priv;

  /* create a no_target ghostpad */
  ghost = gst_ghost_pad_new_no_target (name, dir);
  if (!ghost)
    return NULL;

  GST_DEBUG ("grabbing existing pad functions");

  /* remember the existing ghostpad event/query/link/unlink functions */
  priv = g_new0 (GnlPadPrivate, 1);
  priv->dir = dir;
  priv->object = object;

  /* grab/replace event/query functions */
  GST_DEBUG_OBJECT (ghost, "Setting priv->eventfunc to %p",
      GST_PAD_EVENTFUNC (ghost));
  priv->eventfunc = GST_PAD_EVENTFUNC (ghost);
  priv->queryfunc = GST_PAD_QUERYFUNC (ghost);

  gst_pad_set_event_function (ghost,
      GST_DEBUG_FUNCPTR (ghostpad_event_function));
  gst_pad_set_query_function (ghost,
      GST_DEBUG_FUNCPTR (ghostpad_query_function));

  gst_pad_set_element_private (ghost, priv);

  return ghost;
}

void
gnl_object_remove_ghost_pad (GnlObject * object, GstPad * ghost)
{
  GnlPadPrivate *priv;

  GST_DEBUG_OBJECT (object, "ghostpad %s:%s", GST_DEBUG_PAD_NAME (ghost));

  priv = gst_pad_get_element_private (ghost);
  gst_ghost_pad_set_target (GST_GHOST_PAD (ghost), NULL);
  gst_element_remove_pad (GST_ELEMENT (object), ghost);
  if (priv)
    g_free (priv);
}

gboolean
gnl_object_ghost_pad_set_target (GnlObject * object, GstPad * ghost,
    GstPad * target)
{
  GnlPadPrivate *priv = gst_pad_get_element_private (ghost);

  g_return_val_if_fail (priv, FALSE);

  if (target)
    GST_DEBUG_OBJECT (object, "setting target %s:%s on ghostpad",
        GST_DEBUG_PAD_NAME (target));
  else
    GST_DEBUG_OBJECT (object, "removing target from ghostpad");

  /* set target */
  if (!(gst_ghost_pad_set_target (GST_GHOST_PAD (ghost), target)))
    return FALSE;

  if (!GST_OBJECT_IS_FLOATING (ghost))
    control_internal_pad (ghost, object);

  return TRUE;
}

static GstMessage *
translate_message_segment_start (GnlObject * object, GstMessage * message)
{
  GstFormat format;
  gint64 position;
  guint64 pos2;
  GstMessage *message2;

  gst_message_parse_segment_start (message, &format, &position);
  if (format != GST_FORMAT_TIME)
    return message;
  GST_LOG_OBJECT (object, "format:%d, position:%" GST_TIME_FORMAT,
      format, GST_TIME_ARGS (position));
  gnl_media_to_object_time (object, position, &pos2);
  if (pos2 > G_MAXINT64) {
    g_warning ("getting values too big...");
    return message;
  }
  message2 = gst_message_new_segment_start (GST_MESSAGE_SRC (message),
      format, (gint64) pos2);
  gst_message_unref (message);
  return message2;
}

static GstMessage *
translate_message_segment_done (GnlObject * object, GstMessage * message)
{
  GstFormat format;
  gint64 position;
  guint64 pos2;
  GstMessage *message2;

  gst_message_parse_segment_done (message, &format, &position);

  GST_LOG_OBJECT (object, "format:%d, position:%" GST_TIME_FORMAT,
      format, GST_TIME_ARGS (position));

  if (format != GST_FORMAT_TIME) {
    GST_WARNING_OBJECT (object,
        "Got SEGMENT_DONE with format different from TIME");

    if (GST_CLOCK_TIME_IS_VALID (object->media_stop)) {
      GST_WARNING_OBJECT (object, "Bumping to object->media_stop");
      position = (gint64) object->media_stop;
      format = GST_FORMAT_TIME;
    } else {
      GST_WARNING_OBJECT (object, "Bumping to object->stop");
      message2 = gst_message_new_segment_done (GST_MESSAGE_SRC (message),
          GST_FORMAT_TIME, (gint64) object->stop);
      goto beach;
    }
  }

  gnl_media_to_object_time (object, position, &pos2);
  if (pos2 > G_MAXINT64) {
    g_warning ("getting values too big...");
    return message;
  }
  message2 = gst_message_new_segment_done (GST_MESSAGE_SRC (message),
      format, (gint64) pos2);
beach:
  gst_message_unref (message);
  return message2;

}

static void
gnl_object_handle_message (GstBin * bin, GstMessage * message)
{
  GnlObject *object = (GnlObject *) bin;

  GST_DEBUG_OBJECT (object, "message:%s",
      gst_message_type_get_name (GST_MESSAGE_TYPE (message)));

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_SEGMENT_START:
      /* translate outgoing segment_start */
      message = translate_message_segment_start (object, message);
      break;
    case GST_MESSAGE_SEGMENT_DONE:
      /* translate outgoing segment_done */
      message = translate_message_segment_done (object, message);
      break;
    default:
      break;
  }

  GST_BIN_CLASS (parent_class)->handle_message (bin, message);
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
    GST_LOG_OBJECT (object,
        "Updated rate : %f [mduration:%" GST_TIME_FORMAT ", duration:%"
        GST_TIME_FORMAT "]", object->rate,
        GST_TIME_ARGS (object->media_duration),
        GST_TIME_ARGS (object->duration));
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
