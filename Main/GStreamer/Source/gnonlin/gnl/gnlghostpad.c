/* Gnonlin
 * Copyright (C) <2009> Edward Hervey <bilboed@bilboed.com>
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

#include "gnl.h"

GST_DEBUG_CATEGORY_STATIC (gnlghostpad);
#define GST_CAT_DEFAULT gnlghostpad

typedef struct _GnlPadPrivate GnlPadPrivate;

struct _GnlPadPrivate
{
  GnlObject *object;
  GnlPadPrivate *ghostpriv;
  GstPadDirection dir;
  GstPadEventFunction eventfunc;
  GstPadQueryFunction queryfunc;
};


static GstEvent *
translate_incoming_qos (GnlObject * object, GstEvent * event)
{
  gdouble prop;
  GstClockTimeDiff diff;
  GstClockTime timestamp, timestamp2;
  GstEvent *event2 = NULL;

  gst_event_parse_qos (event, &prop, &diff, &timestamp);

  GST_DEBUG_OBJECT (object,
      "incoming qos prop:%f, diff:%" G_GINT64_FORMAT
      ", timestamp:%" GST_TIME_FORMAT, prop, diff, GST_TIME_ARGS (timestamp));

  if (!(gnl_object_to_media_time (object, timestamp, &timestamp2)) ||
      ((diff < 0) && (-diff > timestamp))) {
    GST_DEBUG ("Invalid timestamp, discarding event");
    gst_event_unref (event);
  } else {
    GST_DEBUG_OBJECT (object,
        "translated qos prop:%f, diff:%" G_GINT64_FORMAT
        ", timestamp:%" GST_TIME_FORMAT, prop, diff,
        GST_TIME_ARGS (timestamp2));
    event2 = gst_event_new_qos (prop, diff, timestamp2);
  }
  return event2;
}

static GstEvent *
translate_outgoing_qos (GnlObject * object, GstEvent * event)
{
  gdouble prop;
  GstClockTimeDiff diff;
  GstClockTime timestamp, timestamp2;
  GstEvent *event2 = NULL;

  gst_event_parse_qos (event, &prop, &diff, &timestamp);

  GST_DEBUG_OBJECT (object,
      "incoming qos prop:%f, diff:%" G_GINT64_FORMAT
      ", timestamp:%" GST_TIME_FORMAT, prop, diff, GST_TIME_ARGS (timestamp));

  if (!(gnl_media_to_object_time (object, timestamp, &timestamp2)) ||
      ((diff < 0) && (-diff > timestamp))) {
    GST_DEBUG ("Invalid timestamp, discarding event");
    gst_event_unref (event);
  } else {
    GST_DEBUG_OBJECT (object,
        "translated qos prop:%f, diff:%" G_GINT64_FORMAT
        ", timestamp:%" GST_TIME_FORMAT, prop, diff,
        GST_TIME_ARGS (timestamp2));
    event2 = gst_event_new_qos (prop, diff, timestamp2);
  }
  return event2;
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

  if (G_UNLIKELY (format != GST_FORMAT_TIME))
    goto invalid_format;

  /* convert rate */
  if (G_LIKELY (object->rate_1))
    nrate = rate;
  else
    nrate = rate * object->rate;
  GST_DEBUG ("nrate:%f , rate:%f, object->rate:%f", nrate, rate, object->rate);

  /* convert cur */
  ncurtype = GST_SEEK_TYPE_SET;
  if (G_LIKELY ((curtype == GST_SEEK_TYPE_SET)
          && (gnl_object_to_media_time (object, cur, &ncur)))) {
    /* cur is TYPE_SET and value is valid */
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
  if (G_LIKELY ((stoptype == GST_SEEK_TYPE_SET)
          && (gnl_object_to_media_time (object, stop, &nstop)))) {
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


  /* add accurate seekflags */
  if (G_UNLIKELY (!(flags & GST_SEEK_FLAG_ACCURATE))) {
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

  if (G_UNLIKELY (format != GST_FORMAT_TIME))
    goto invalid_format;

  /* convert rate */
  if (G_LIKELY (object->rate_1))
    nrate = rate;
  else
    nrate = rate / object->rate;
  GST_DEBUG ("nrate:%f , rate:%f, object->rate:%f", nrate, rate, object->rate);

  /* convert cur */
  ncurtype = GST_SEEK_TYPE_SET;
  if (G_LIKELY ((curtype == GST_SEEK_TYPE_SET)
          && (gnl_media_to_object_time (object, cur, &ncur)))) {
    /* cur is TYPE_SET and value is valid */
    if (ncur > G_MAXINT64)
      GST_WARNING_OBJECT (object, "return value too big...");
    GST_LOG_OBJECT (object, "Setting cur to %" GST_TIME_FORMAT,
        GST_TIME_ARGS (ncur));
  } else if ((curtype != GST_SEEK_TYPE_NONE)) {
    GST_DEBUG_OBJECT (object, "Limiting seek start to start");
    ncur = object->start;
  } else {
    GST_DEBUG_OBJECT (object, "leaving GST_SEEK_TYPE_NONE");
    ncur = cur;
    ncurtype = GST_SEEK_TYPE_NONE;
  }

  /* convert stop, we also need to limit it to object->stop */
  if (G_LIKELY ((stoptype == GST_SEEK_TYPE_SET)
          && (gnl_media_to_object_time (object, stop, &nstop)))) {
    if (nstop > G_MAXINT64)
      GST_WARNING_OBJECT (object, "return value too big...");
    GST_LOG_OBJECT (object, "Setting stop to %" GST_TIME_FORMAT,
        GST_TIME_ARGS (nstop));
  } else {
    GST_DEBUG_OBJECT (object, "Limiting end of seek to stop");
    nstop = object->stop;
    if (nstop > G_MAXINT64)
      GST_WARNING_OBJECT (object, "return value too big...");
    GST_LOG_OBJECT (object, "Setting stop to %" GST_TIME_FORMAT,
        GST_TIME_ARGS (nstop));
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

  if (G_UNLIKELY (format != GST_FORMAT_TIME)) {
    GST_WARNING_OBJECT (object,
        "Can't translate newsegments with format != GST_FORMAT_TIME");
    return event;
  }

  gnl_media_to_object_time (object, stream, &nstream);

  if (G_UNLIKELY (nstream > G_MAXINT64))
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

static GstEvent *
translate_incoming_new_segment (GnlObject * object, GstEvent * event)
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

  if (G_UNLIKELY (format != GST_FORMAT_TIME)) {
    GST_WARNING_OBJECT (object,
        "Can't translate newsegments with format != GST_FORMAT_TIME");
    return event;
  }

  if (!gnl_object_to_media_time (object, stream, &nstream)) {
    GST_DEBUG ("Can't convert media_time, using 0");
    nstream = 0;
  };

  if (G_UNLIKELY (nstream > G_MAXINT64))
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
  gboolean res;

  GST_DEBUG_OBJECT (internal, "event:%s", GST_EVENT_TYPE_NAME (event));

  if (G_UNLIKELY (!(priv->eventfunc))) {
    GST_WARNING_OBJECT (internal,
        "priv->eventfunc == NULL !! What is going on ?");
    return FALSE;
  }

  switch (priv->dir) {
    case GST_PAD_SRC:{
      switch (GST_EVENT_TYPE (event)) {
        case GST_EVENT_NEWSEGMENT:
          event = translate_outgoing_new_segment (object, event);
          break;
        default:
          break;
      }

      break;
    }
    case GST_PAD_SINK:{
      switch (GST_EVENT_TYPE (event)) {
        case GST_EVENT_SEEK:
          event = translate_outgoing_seek (object, event);
          break;
        case GST_EVENT_QOS:
          event = translate_outgoing_qos (object, event);
        default:
          break;
      }
      break;
    }
    default:
      break;
  }
  GST_DEBUG_OBJECT (internal, "Calling priv->eventfunc %p", priv->eventfunc);
  res = priv->eventfunc (internal, event);

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
  if (G_UNLIKELY (format != GST_FORMAT_TIME)) {
    GST_WARNING_OBJECT (object,
        "position query is in a format different from time, returning without modifying values");
    goto beach;
  }

  gnl_media_to_object_time (object, (guint64) cur, (guint64 *) & cur2);

  GST_DEBUG_OBJECT (object,
      "Adjust position from %" GST_TIME_FORMAT " to %" GST_TIME_FORMAT,
      GST_TIME_ARGS (cur), GST_TIME_ARGS (cur2));
  gst_query_set_position (query, GST_FORMAT_TIME, cur2);

beach:
  return TRUE;
}

static gboolean
translate_outgoing_position_query (GnlObject * object, GstQuery * query)
{
  GstFormat format;
  gint64 cur, cur2;

  gst_query_parse_position (query, &format, &cur);
  if (G_UNLIKELY (format != GST_FORMAT_TIME)) {
    GST_WARNING_OBJECT (object,
        "position query is in a format different from time, returning without modifying values");
    goto beach;
  }

  if (G_UNLIKELY (!(gnl_object_to_media_time (object, (guint64) cur,
                  (guint64 *) & cur2)))) {
    GST_WARNING_OBJECT (object,
        "Couldn't get media time for %" GST_TIME_FORMAT, GST_TIME_ARGS (cur));
    goto beach;
  }

  GST_DEBUG_OBJECT (object,
      "Adjust position from %" GST_TIME_FORMAT " to %" GST_TIME_FORMAT,
      GST_TIME_ARGS (cur), GST_TIME_ARGS (cur2));
  gst_query_set_position (query, GST_FORMAT_TIME, cur2);

beach:
  return TRUE;
}

static gboolean
translate_incoming_duration_query (GnlObject * object, GstQuery * query)
{
  GstFormat format;
  gint64 cur;

  gst_query_parse_duration (query, &format, &cur);
  if (G_UNLIKELY (format != GST_FORMAT_TIME)) {
    GST_WARNING_OBJECT (object,
        "We can only handle duration queries in GST_FORMAT_TIME");
    return FALSE;
  }

  gst_query_set_duration (query, GST_FORMAT_TIME, object->duration);

  return TRUE;
}

static gboolean
internalpad_query_function (GstPad * internal, GstQuery * query)
{
  GnlPadPrivate *priv = gst_pad_get_element_private (internal);
  GnlObject *object = priv->object;
  gboolean ret;

  GST_DEBUG_OBJECT (internal, "querytype:%d", GST_QUERY_TYPE (query));

  if (!(priv->queryfunc)) {
    GST_WARNING_OBJECT (internal,
        "priv->queryfunc == NULL !! What is going on ?");
    return FALSE;
  }

  if ((ret = priv->queryfunc (internal, query))) {

    switch (priv->dir) {
      case GST_PAD_SRC:
        break;
      case GST_PAD_SINK:
        switch (GST_QUERY_TYPE (query)) {
          case GST_QUERY_POSITION:
            ret = translate_outgoing_position_query (object, query);
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }
  return ret;
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
    {
      switch (GST_EVENT_TYPE (event)) {
        case GST_EVENT_SEEK:
          event = translate_incoming_seek (object, event);
          break;
        case GST_EVENT_QOS:
          event = translate_incoming_qos (object, event);
          break;
        default:
          break;
      }
    }
      break;
    case GST_PAD_SINK:{
      switch (GST_EVENT_TYPE (event)) {
        case GST_EVENT_NEWSEGMENT:
          event = translate_incoming_new_segment (object, event);
          break;
        default:
          break;
      }
    }
      break;
    default:
      break;
  }

  if (event) {
    GST_DEBUG_OBJECT (ghostpad, "Calling priv->eventfunc");
    ret = priv->eventfunc (ghostpad, event);
    GST_DEBUG_OBJECT (ghostpad, "Returned from calling priv->eventfunc : %d",
        ret);
  }

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
  gboolean pret = TRUE;

  GST_DEBUG_OBJECT (ghostpad, "querytype:%s", GST_QUERY_TYPE_NAME (query));

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_DURATION:
      /* skip duration upstream query, we'll fill it in ourselves */
      break;
    default:
      pret = priv->queryfunc (ghostpad, query);
  }

  if (pret) {
    /* translate result */
    switch (GST_QUERY_TYPE (query)) {
      case GST_QUERY_POSITION:
        pret = translate_incoming_position_query (object, query);
        break;
      case GST_QUERY_DURATION:
        pret = translate_incoming_duration_query (object, query);
        break;
      default:
        break;
    }
  }

  return pret;
}

/* internal pad going away */
static void
internal_pad_finalizing (GnlPadPrivate * priv, GObject * pad G_GNUC_UNUSED)
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

  if (G_UNLIKELY (!(priv = gst_pad_get_element_private (internal)))) {
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

  if (target) {
    GstCaps *negotiated_caps;

    /* if the target has negotiated caps, forward them to the ghost */
    if ((negotiated_caps = gst_pad_get_negotiated_caps (target))) {
      gst_pad_set_caps (ghost, negotiated_caps);
      gst_caps_unref (negotiated_caps);
    }
  }


  if (!GST_OBJECT_IS_FLOATING (ghost))
    control_internal_pad (ghost, object);

  return TRUE;
}

void
gnl_init_ghostpad_category ()
{
  GST_DEBUG_CATEGORY_INIT (gnlghostpad, "gnlghostpad",
      GST_DEBUG_FG_BLUE | GST_DEBUG_BOLD, "GNonLin GhostPad");

}
