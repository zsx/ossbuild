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

#include "gnl.h"

/**
 * SECTION:element-gnlsource
 * @short_description: GNonLin Source
 *
 * The GnlSource encapsulates a pipeline which produces data for processing
 * in a #GnlComposition.
 */

static GstStaticPadTemplate gnl_source_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS_ANY);

GST_DEBUG_CATEGORY_STATIC (gnlsource);
#define GST_CAT_DEFAULT gnlsource

GST_BOILERPLATE (GnlSource, gnl_source, GnlObject, GNL_TYPE_OBJECT);

static GstElementDetails gnl_source_details = GST_ELEMENT_DETAILS
    ("GNonLin Source",
    "Filter/Editor",
    "Manages source elements",
    "Wim Taymans <wim.taymans@gmail.com>, Edward Hervey <bilboed@bilboed.com>");

struct _GnlSourcePrivate
{
  gboolean dispose_has_run;

  gboolean dynamicpads;         /* TRUE if the controlled element has dynamic pads */
  GstPad *ghostpad;             /* The source ghostpad */
  GstEvent *event;              /* queued event */

  gulong padremovedid;          /* signal handler for element pad-removed signal */
  gulong padaddedid;            /* signal handler for element pad-added signal */

  gboolean pendingblock;        /* We have a pending pad_block */
  GstPad *ghostedpad;           /* Pad (to be) ghosted */
};

static gboolean gnl_source_prepare (GnlObject * object);

static gboolean gnl_source_add_element (GstBin * bin, GstElement * element);

static gboolean gnl_source_remove_element (GstBin * bin, GstElement * element);

static void gnl_source_dispose (GObject * object);
static void gnl_source_finalize (GObject * object);

static gboolean gnl_source_send_event (GstElement * element, GstEvent * event);

static GstStateChangeReturn
gnl_source_change_state (GstElement * element, GstStateChange transition);

static void pad_blocked_cb (GstPad * pad, gboolean blocked, GnlSource * source);

static gboolean
gnl_source_control_element_func (GnlSource * source, GstElement * element);

static void
gnl_source_base_init (gpointer g_class)
{
  GstElementClass *gstclass = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (gstclass, &gnl_source_details);
}

static void
gnl_source_class_init (GnlSourceClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBinClass *gstbin_class;
  GnlObjectClass *gnlobject_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbin_class = (GstBinClass *) klass;
  gnlobject_class = (GnlObjectClass *) klass;

  parent_class = g_type_class_ref (GNL_TYPE_OBJECT);

  GST_DEBUG_CATEGORY_INIT (gnlsource, "gnlsource",
      GST_DEBUG_FG_BLUE | GST_DEBUG_BOLD, "GNonLin Source Element");

  klass->controls_one = TRUE;
  klass->control_element = GST_DEBUG_FUNCPTR (gnl_source_control_element_func);

  gnlobject_class->prepare = GST_DEBUG_FUNCPTR (gnl_source_prepare);

  gstbin_class->add_element = GST_DEBUG_FUNCPTR (gnl_source_add_element);
  gstbin_class->remove_element = GST_DEBUG_FUNCPTR (gnl_source_remove_element);

  gstelement_class->send_event = GST_DEBUG_FUNCPTR (gnl_source_send_event);
  gstelement_class->change_state = GST_DEBUG_FUNCPTR (gnl_source_change_state);

  gobject_class->dispose = GST_DEBUG_FUNCPTR (gnl_source_dispose);
  gobject_class->finalize = GST_DEBUG_FUNCPTR (gnl_source_finalize);

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&gnl_source_src_template));

}


static void
gnl_source_init (GnlSource * source, GnlSourceClass * klass G_GNUC_UNUSED)
{
  GST_OBJECT_FLAG_SET (source, GNL_OBJECT_SOURCE);
  source->element = NULL;
  source->priv = g_new0 (GnlSourcePrivate, 1);

  if (g_object_class_find_property (G_OBJECT_CLASS (parent_class),
          "async-handling")) {
    GST_DEBUG_OBJECT (source, "Setting GstBin async-handling to TRUE");
    g_object_set (G_OBJECT (source), "async-handling", TRUE, NULL);
  }
}

static void
gnl_source_dispose (GObject * object)
{
  GnlSource *source = (GnlSource *) object;

  GST_DEBUG_OBJECT (object, "dispose");

  if (source->priv->dispose_has_run)
    return;

  if (source->element) {
    gst_object_unref (source->element);
    source->element = NULL;
  }

  source->priv->dispose_has_run = TRUE;
  if (source->priv->event)
    gst_event_unref (source->priv->event);

  if (source->priv->ghostpad)
    gnl_object_remove_ghost_pad ((GnlObject *) object, source->priv->ghostpad);
  source->priv->ghostpad = NULL;

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gnl_source_finalize (GObject * object)
{
  GnlSource *source = (GnlSource *) object;

  GST_DEBUG_OBJECT (object, "finalize");

  g_free (source->priv);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gnl_source_prepare (GnlObject * object)
{
  GnlSource *source = (GnlSource *) object;
  GstElement *parent =
      (GstElement *) gst_element_get_parent ((GstElement *) object);

  if (!GNL_IS_COMPOSITION (parent)) {

    /* Figure out if we're in a composition */
    if (source->priv->event)
      gst_event_unref (source->priv->event);

    GST_DEBUG_OBJECT (object, "Creating initial seek");

    source->priv->event = gst_event_new_seek (1.0, GST_FORMAT_TIME,
        GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH,
        GST_SEEK_TYPE_SET, object->start, GST_SEEK_TYPE_SET, object->stop);
  }

  gst_object_unref (parent);

  return TRUE;
}


static void
element_pad_added_cb (GstElement * element G_GNUC_UNUSED, GstPad * pad,
    GnlSource * source)
{
  GST_DEBUG_OBJECT (source, "pad %s:%s", GST_DEBUG_PAD_NAME (pad));

  if (source->priv->ghostpad || source->priv->pendingblock) {
    GST_WARNING_OBJECT (source,
        "We already have (pending) ghost-ed a valid source pad");
    return;
  }

  if (!(gst_pad_accept_caps (pad, GNL_OBJECT (source)->caps))) {
    GST_DEBUG_OBJECT (source, "Pad doesn't have valid caps, ignoring");
    return;
  }

  GST_DEBUG_OBJECT (pad, "valid pad, about to add event probe and pad block");


  if (!(gst_pad_set_blocked_async (pad, TRUE,
              (GstPadBlockCallback) pad_blocked_cb, source)))
    GST_WARNING_OBJECT (source, "Couldn't set Async pad blocking");
  else {
    source->priv->ghostedpad = pad;
    source->priv->pendingblock = TRUE;
  }

  GST_DEBUG_OBJECT (source, "Done handling pad %s:%s",
      GST_DEBUG_PAD_NAME (pad));
}

static void
element_pad_removed_cb (GstElement * element G_GNUC_UNUSED, GstPad * pad,
    GnlSource * source)
{
  GST_DEBUG_OBJECT (source, "pad %s:%s", GST_DEBUG_PAD_NAME (pad));

  if (source->priv->ghostpad) {
    GstPad *target =
        gst_ghost_pad_get_target (GST_GHOST_PAD (source->priv->ghostpad));

    if (target == pad) {
      gst_pad_set_blocked_async (target, FALSE,
          (GstPadBlockCallback) pad_blocked_cb, source);


      gnl_object_remove_ghost_pad ((GnlObject *) source,
          source->priv->ghostpad);
      source->priv->ghostpad = NULL;
      gst_object_unref (target);
    } else {
      GST_DEBUG_OBJECT (source, "The removed pad wasn't our ghostpad target");
    }
  }
}

static gint
compare_src_pad (GstPad * pad, GstCaps * caps)
{
  gint ret;

  if (gst_pad_accept_caps (pad, caps))
    ret = 0;
  else {
    gst_object_unref (pad);
    ret = 1;
  }
  return ret;
}

/*
  get_valid_src_pad

  Returns True if there's a src pad compatible with the GnlObject caps in the
  given element. Fills in pad if so. The returned pad has an incremented refcount
*/

static gboolean
get_valid_src_pad (GnlSource * source, GstElement * element, GstPad ** pad)
{
  GstIterator *srcpads;

  g_return_val_if_fail (pad, FALSE);

  srcpads = gst_element_iterate_src_pads (element);
  *pad = (GstPad *) gst_iterator_find_custom (srcpads,
      (GCompareFunc) compare_src_pad, GNL_OBJECT (source)->caps);
  gst_iterator_free (srcpads);

  if (*pad)
    return TRUE;
  return FALSE;
}

static gpointer
ghost_seek_pad (GnlSource * source)
{
  GstPad *pad = source->priv->ghostedpad;

  if (source->priv->ghostpad || !pad)
    goto beach;

  GST_DEBUG_OBJECT (source, "ghosting %s:%s", GST_DEBUG_PAD_NAME (pad));

  source->priv->ghostpad = gnl_object_ghost_pad_full
      ((GnlObject *) source, GST_PAD_NAME (pad), pad, TRUE);
  GST_DEBUG_OBJECT (source, "emitting no more pads");
  gst_pad_set_active (source->priv->ghostpad, TRUE);

  if (source->priv->event) {
    GST_DEBUG_OBJECT (source, "sending queued seek event");
    if (!(gst_pad_send_event (source->priv->ghostpad, source->priv->event)))
      GST_ELEMENT_ERROR (source, RESOURCE, SEEK,
          (NULL), ("Sending initial seek to upstream element failed"));
    else
      GST_DEBUG_OBJECT (source, "queued seek sent");
    source->priv->event = NULL;
  }

  GST_DEBUG_OBJECT (source, "about to unblock %s:%s", GST_DEBUG_PAD_NAME (pad));
  gst_pad_set_blocked_async (pad, FALSE,
      (GstPadBlockCallback) pad_blocked_cb, source);
  gst_element_no_more_pads (GST_ELEMENT (source));

  source->priv->pendingblock = FALSE;

beach:
  return NULL;
}

static void
pad_blocked_cb (GstPad * pad, gboolean blocked, GnlSource * source)
{
  GST_DEBUG_OBJECT (source, "blocked:%d pad:%s:%s",
      blocked, GST_DEBUG_PAD_NAME (pad));

  if (!(source->priv->ghostpad)) {
    if (blocked)
      g_thread_create ((GThreadFunc) ghost_seek_pad, source, FALSE, NULL);
  }
}


/*
 * has_dynamic_pads
 * Returns TRUE if the element has only dynamic pads.
 */

static gboolean
has_dynamic_srcpads (GstElement * element)
{
  gboolean ret = TRUE;
  GList *templates;
  GstPadTemplate *template;

  templates =
      gst_element_class_get_pad_template_list (GST_ELEMENT_GET_CLASS (element));

  while (templates) {
    template = (GstPadTemplate *) templates->data;

    if ((GST_PAD_TEMPLATE_DIRECTION (template) == GST_PAD_SRC)
        && (GST_PAD_TEMPLATE_PRESENCE (template) == GST_PAD_ALWAYS)) {
      ret = FALSE;
      break;
    }

    templates = g_list_next (templates);
  }

  return ret;
}

static gboolean
gnl_source_control_element_func (GnlSource * source, GstElement * element)
{
  GstPad *pad = NULL;

  g_return_val_if_fail (source->element == NULL, FALSE);

  GST_DEBUG_OBJECT (source, "element:%s, source->element:%p",
      GST_ELEMENT_NAME (element), source->element);

  source->element = element;
  gst_object_ref (element);

  if (get_valid_src_pad (source, source->element, &pad)) {
    gst_object_unref (pad);
    GST_DEBUG_OBJECT (source,
        "There is a valid source pad, we consider the object as NOT having dynamic pads");
    source->priv->dynamicpads = FALSE;
  } else {
    source->priv->dynamicpads = has_dynamic_srcpads (element);
    GST_DEBUG_OBJECT (source, "No valid source pad yet, dynamicpads:%d",
        source->priv->dynamicpads);
    if (source->priv->dynamicpads) {
      /* connect to pad-added/removed signals */
      source->priv->padremovedid = g_signal_connect
          (G_OBJECT (element), "pad-removed",
          G_CALLBACK (element_pad_removed_cb), source);
      source->priv->padaddedid =
          g_signal_connect (G_OBJECT (element), "pad-added",
          G_CALLBACK (element_pad_added_cb), source);
    }
  }

  return TRUE;
}

static gboolean
gnl_source_add_element (GstBin * bin, GstElement * element)
{
  GnlSource *source = (GnlSource *) bin;
  gboolean pret;

  GST_DEBUG_OBJECT (source, "Adding element %s", GST_ELEMENT_NAME (element));

  if (GNL_SOURCE_GET_CLASS (source)->controls_one && source->element) {
    GST_WARNING_OBJECT (bin, "GnlSource can only handle one element at a time");
    return FALSE;
  }

  /* call parent add_element */
  pret = GST_BIN_CLASS (parent_class)->add_element (bin, element);

  if (pret && GNL_SOURCE_GET_CLASS (source)->controls_one) {
    gnl_source_control_element_func (source, element);
  }
  return pret;
}

static gboolean
gnl_source_remove_element (GstBin * bin, GstElement * element)
{
  GnlSource *source = (GnlSource *) bin;
  gboolean pret;

  GST_DEBUG_OBJECT (source, "Removing element %s", GST_ELEMENT_NAME (element));

  /* try to remove it */
  pret = GST_BIN_CLASS (parent_class)->remove_element (bin, element);

  if ((!source->element) || (source->element != element)) {
    return TRUE;
  }

  if (pret) {
    /* remove ghostpad */
    if (source->priv->ghostpad) {
      gnl_object_remove_ghost_pad ((GnlObject *) bin, source->priv->ghostpad);
      source->priv->ghostpad = NULL;
    }

    /* discard events */
    if (source->priv->event) {
      gst_event_unref (source->priv->event);
      source->priv->event = NULL;
    }

    /* remove signal handlers */
    if (source->priv->padremovedid) {
      g_signal_handler_disconnect (source->element, source->priv->padremovedid);
      source->priv->padremovedid = 0;
    }
    if (source->priv->padaddedid) {
      g_signal_handler_disconnect (source->element, source->priv->padaddedid);
      source->priv->padaddedid = 0;
    }

    source->priv->dynamicpads = FALSE;
    gst_object_unref (element);
    source->element = NULL;
  }
  return pret;
}

static gboolean
gnl_source_send_event (GstElement * element, GstEvent * event)
{
  GnlSource *source = (GnlSource *) element;
  gboolean res = TRUE;

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_SEEK:
      if (source->priv->ghostpad)
        res = gst_pad_send_event (source->priv->ghostpad, event);
      else {
        if (source->priv->event)
          gst_event_unref (source->priv->event);
        source->priv->event = event;
      }
      break;
    default:
      res = GST_ELEMENT_CLASS (parent_class)->send_event (element, event);
      break;
  }

  return res;
}

static GstStateChangeReturn
gnl_source_change_state (GstElement * element, GstStateChange transition)
{
  GnlSource *source = (GnlSource *) element;
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      if (!source->element) {
        GST_WARNING_OBJECT (source,
            "GnlSource doesn't have an element to control !");
        ret = GST_STATE_CHANGE_FAILURE;
        break;
      }

      GST_LOG_OBJECT (source, "ghostpad:%p, dynamicpads:%d",
          source->priv->ghostpad, source->priv->dynamicpads);

      if (!(source->priv->ghostpad) && !source->priv->pendingblock) {
        GstPad *pad;

        GST_LOG_OBJECT (source, "no ghostpad and not dynamic pads");

        /* Do an async block on valid source pad */

        if (!(get_valid_src_pad (source, source->element, &pad))) {
          GST_WARNING_OBJECT (source, "Couldn't find a valid source pad");
        } else {
          GST_LOG_OBJECT (source, "Trying to async block source pad %s:%s",
              GST_DEBUG_PAD_NAME (pad));
          source->priv->ghostedpad = pad;
          gst_pad_set_blocked_async (pad, TRUE,
              (GstPadBlockCallback) pad_blocked_cb, source);
          gst_object_unref (pad);
        }
      }
      break;
    default:
      break;
  }

  if (ret == GST_STATE_CHANGE_FAILURE)
    goto beach;

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  if (ret == GST_STATE_CHANGE_FAILURE)
    goto beach;

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      if (source->priv->ghostpad) {
        GstPad *target =
            gst_ghost_pad_get_target ((GstGhostPad *) source->priv->ghostpad);

        if (target) {
          gst_pad_set_blocked_async (target, FALSE,
              (GstPadBlockCallback) pad_blocked_cb, source);
          gst_object_unref (target);
        }
        gnl_object_remove_ghost_pad ((GnlObject *) source,
            source->priv->ghostpad);
        source->priv->ghostpad = NULL;
        source->priv->ghostedpad = NULL;
      }
    default:
      break;
  }

beach:
  return ret;
}
