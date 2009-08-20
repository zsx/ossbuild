/* GStreamer
 * Copyright (C) 2001 Wim Taymans <wim.taymans@gmail.com>
 *               2004-2008 Edward Hervey <bilboed@bilboed.com>
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
 * SECTION:element-gnlcomposition
 * @short_description: Combines and controls GNonLin elements
 *
 * A GnlComposition contains GnlObjects such as GnlSources and GnlOperations,
 * and connects them dynamically to create a composition timeline.
 */

GST_BOILERPLATE (GnlComposition, gnl_composition, GnlObject, GNL_TYPE_OBJECT);

static GstElementDetails gnl_composition_details =
GST_ELEMENT_DETAILS ("GNonLin Composition",
    "Filter/Editor",
    "Combines GNL objects",
    "Wim Taymans <wim.taymans@gmail.com>, Edward Hervey <bilboed@bilboed.com>");

static GstStaticPadTemplate gnl_composition_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS_ANY);

GST_DEBUG_CATEGORY_STATIC (gnlcomposition);
#define GST_CAT_DEFAULT gnlcomposition

enum
{
  ARG_0,
  ARG_UPDATE,
};

struct _GnlCompositionPrivate
{
  gboolean dispose_has_run;

  /* 
     Sorted List of GnlObjects , ThreadSafe 
     objects_start : sorted by start-time then priority
     objects_stop : sorted by stop-time then priority
     objects_hash : contains signal handlers id for controlled objects
     objects_lock : mutex to acces/modify any of those lists/hashtable
   */
  GList *objects_start;
  GList *objects_stop;
  GHashTable *objects_hash;
  GMutex *objects_lock;

  /* Update properties
   * can_update: If True, updates should be taken into account immediatly, else
   *   they should be postponed until it is set to True again. 
   * update_required: Set to True if an update is required when the
   *   can_update property just above is set back to True. */
  gboolean can_update;
  gboolean update_required;

  /*
     thread-safe Seek handling.
     flushing_lock : mutex to access flushing and pending_idle
     flushing : 
     pending_idle :
   */
  GMutex *flushing_lock;
  gboolean flushing;
  guint pending_idle;

  /* source top-level ghostpad */
  GstPad *ghostpad;
  guint ghosteventprobe;

  /* current stack, list of GnlObject* */
  GNode *current;

  /* List of GnlObject whose start/duration will be the same as the composition */
  GList *expandables;

  /* TRUE if the stack is valid.
   * This is meant to prevent the top-level pad to be unblocked before the stack
   * is fully done. Protected by OBJECTS_LOCK */
  gboolean stackvalid;

  /*
     current segment seek start/stop time. 
     Reconstruct pipeline ONLY if seeking outside of those values
     FIXME : segment_start isn't always the earliest time before which the
     timeline doesn't need to be modified
   */
  GstClockTime segment_start;
  GstClockTime segment_stop;

  /* pending child seek */
  GstEvent *childseek;

  /* Seek segment handler */
  GstSegment *segment;

  /* number of pads we are waiting to appear so be can do proper linking */
  guint waitingpads;

  /*
     OUR sync_handler on the child_bus 
     We are called before gnl_object_sync_handler
   */
  GstPadEventFunction gnl_event_pad_func;

};

#define OBJECT_IN_ACTIVE_SEGMENT(comp,element)				\
  ((GNL_OBJECT_CAST(element)->start < comp->priv->segment_stop) &&	\
   (GNL_OBJECT_CAST(element)->stop >= comp->priv->segment_start))

static void gnl_composition_dispose (GObject * object);
static void gnl_composition_finalize (GObject * object);
static void gnl_composition_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspsec);
static void gnl_composition_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspsec);
static void gnl_composition_reset (GnlComposition * comp);

static gboolean gnl_composition_add_object (GstBin * bin, GstElement * element);

static void gnl_composition_handle_message (GstBin * bin, GstMessage * message);

static gboolean
gnl_composition_remove_object (GstBin * bin, GstElement * element);

static GstStateChangeReturn
gnl_composition_change_state (GstElement * element, GstStateChange transition);

static GstPad *get_src_pad (GstElement * element);
static void pad_blocked (GstPad * pad, gboolean blocked, GnlComposition * comp);

static gboolean
seek_handling (GnlComposition * comp, gboolean initial, gboolean update);
static gint objects_start_compare (GnlObject * a, GnlObject * b);
static gint objects_stop_compare (GnlObject * a, GnlObject * b);
static GstClockTime get_current_position (GnlComposition * comp);

static void gnl_composition_set_update (GnlComposition * comp, gboolean update);
static gboolean
update_pipeline (GnlComposition * comp, GstClockTime currenttime,
    gboolean initial, gboolean change_state, gboolean modify);


#define COMP_REAL_START(comp) \
  (MAX (comp->priv->segment->start, ((GnlObject*)comp)->start))

#define COMP_REAL_STOP(comp) \
  ((GST_CLOCK_TIME_IS_VALID (comp->priv->segment->stop) \
    ? (MIN (comp->priv->segment->stop, ((GnlObject*)comp)->stop))) \
   : (((GnlObject*)comp)->stop))

#define COMP_ENTRY(comp, object) \
  (g_hash_table_lookup (comp->priv->objects_hash, (gconstpointer) object))

#define COMP_OBJECTS_LOCK(comp) G_STMT_START {				\
    GST_LOG_OBJECT (comp, "locking objects_lock from thread %p",		\
      g_thread_self());							\
    g_mutex_lock (comp->priv->objects_lock);				\
    GST_LOG_OBJECT (comp, "locked objects_lock from thread %p",		\
		    g_thread_self());					\
  } G_STMT_END

#define COMP_OBJECTS_UNLOCK(comp) G_STMT_START {			\
    GST_LOG_OBJECT (comp, "unlocking objects_lock from thread %p",		\
		    g_thread_self());					\
    g_mutex_unlock (comp->priv->objects_lock);			\
  } G_STMT_END


#define COMP_FLUSHING_LOCK(comp) G_STMT_START {				\
    GST_LOG_OBJECT (comp, "locking flushing_lock from thread %p",		\
      g_thread_self());							\
    g_mutex_lock (comp->priv->flushing_lock);				\
    GST_LOG_OBJECT (comp, "locked flushing_lock from thread %p",		\
		    g_thread_self());					\
  } G_STMT_END

#define COMP_FLUSHING_UNLOCK(comp) G_STMT_START {			\
    GST_LOG_OBJECT (comp, "unlocking flushing_lock from thread %p",		\
		    g_thread_self());					\
    g_mutex_unlock (comp->priv->flushing_lock);			\
  } G_STMT_END


typedef struct _GnlCompositionEntry GnlCompositionEntry;

struct _GnlCompositionEntry
{
  GnlObject *object;

  /* handler ids for property notifications */
  gulong starthandler;
  gulong stophandler;
  gulong priorityhandler;
  gulong activehandler;

  /* handler id for 'no-more-pads' signal */
  gulong nomorepadshandler;
  gulong padaddedhandler;
  gulong padremovedhandler;
};

static void
gnl_composition_base_init (gpointer g_class)
{
  GstElementClass *gstclass = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (gstclass, &gnl_composition_details);
}

static void
gnl_composition_class_init (GnlCompositionClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBinClass *gstbin_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbin_class = (GstBinClass *) klass;

  GST_DEBUG_CATEGORY_INIT (gnlcomposition, "gnlcomposition",
      GST_DEBUG_FG_BLUE | GST_DEBUG_BOLD, "GNonLin Composition");

  gobject_class->dispose = GST_DEBUG_FUNCPTR (gnl_composition_dispose);
  gobject_class->finalize = GST_DEBUG_FUNCPTR (gnl_composition_finalize);
  gobject_class->set_property =
      GST_DEBUG_FUNCPTR (gnl_composition_set_property);
  gobject_class->get_property =
      GST_DEBUG_FUNCPTR (gnl_composition_get_property);

  gstelement_class->change_state = gnl_composition_change_state;

  gstbin_class->add_element = GST_DEBUG_FUNCPTR (gnl_composition_add_object);
  gstbin_class->remove_element =
      GST_DEBUG_FUNCPTR (gnl_composition_remove_object);
  gstbin_class->handle_message =
      GST_DEBUG_FUNCPTR (gnl_composition_handle_message);

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&gnl_composition_src_template));

  /**
   * GnlComposition:update:
   *
   * If %TRUE, then all modifications to objects within the composition will
   * cause a internal pipeline update (if required).
   * If %FALSE, then only the composition's start/duration/stop properties
   * will be updated, and the internal pipeline will only be updated when the
   * property is set back to %TRUE.
   *
   * It is recommended to temporarily set this property to %FALSE before doing
   * more than one modification in the composition (like adding/moving/removing
   * several objects at once) in order to speed up the process, and then setting
   * back the property to %TRUE when done.
   */

  g_object_class_install_property (gobject_class, ARG_UPDATE,
      g_param_spec_boolean ("update", "Update",
          "Update the internal pipeline on every modification", TRUE,
          G_PARAM_READWRITE));
}

static void
hash_value_destroy (GnlCompositionEntry * entry)
{
  if (entry->starthandler)
    g_signal_handler_disconnect (entry->object, entry->starthandler);
  if (entry->stophandler)
    g_signal_handler_disconnect (entry->object, entry->stophandler);
  if (entry->priorityhandler)
    g_signal_handler_disconnect (entry->object, entry->priorityhandler);
  g_signal_handler_disconnect (entry->object, entry->activehandler);
  g_signal_handler_disconnect (entry->object, entry->padremovedhandler);
  g_signal_handler_disconnect (entry->object, entry->padaddedhandler);

  if (entry->nomorepadshandler)
    g_signal_handler_disconnect (entry->object, entry->nomorepadshandler);
  g_free (entry);
}

static void
gnl_composition_init (GnlComposition * comp,
    GnlCompositionClass * klass G_GNUC_UNUSED)
{
  GST_OBJECT_FLAG_SET (comp, GNL_OBJECT_SOURCE);

  comp->priv = g_new0 (GnlCompositionPrivate, 1);
  comp->priv->objects_lock = g_mutex_new ();
  comp->priv->objects_start = NULL;
  comp->priv->objects_stop = NULL;

  comp->priv->can_update = TRUE;
  comp->priv->update_required = FALSE;

  comp->priv->flushing_lock = g_mutex_new ();
  comp->priv->flushing = FALSE;
  comp->priv->pending_idle = 0;

  comp->priv->segment = gst_segment_new ();

  comp->priv->waitingpads = 0;

  comp->priv->objects_hash = g_hash_table_new_full
      (g_direct_hash,
      g_direct_equal, NULL, (GDestroyNotify) hash_value_destroy);

  gnl_composition_reset (comp);
}

static void
gnl_composition_dispose (GObject * object)
{
  GnlComposition *comp = GNL_COMPOSITION (object);

  if (comp->priv->dispose_has_run)
    return;

  comp->priv->dispose_has_run = TRUE;

  comp->priv->can_update = TRUE;
  comp->priv->update_required = FALSE;

  if (comp->priv->ghostpad) {
    gnl_object_remove_ghost_pad ((GnlObject *) object, comp->priv->ghostpad);
    comp->priv->ghostpad = NULL;
    comp->priv->ghosteventprobe = 0;
  }

  if (comp->priv->childseek) {
    gst_event_unref (comp->priv->childseek);
    comp->priv->childseek = NULL;
  }

  if (comp->priv->current) {
    g_node_destroy (comp->priv->current);
    comp->priv->current = NULL;
  }

  if (comp->priv->expandables) {
    g_list_free (comp->priv->expandables);
    comp->priv->expandables = NULL;
  }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gnl_composition_finalize (GObject * object)
{
  GnlComposition *comp = GNL_COMPOSITION (object);

  GST_INFO ("finalize");

  COMP_OBJECTS_LOCK (comp);
  g_list_free (comp->priv->objects_start);
  g_list_free (comp->priv->objects_stop);
  if (comp->priv->current)
    g_node_destroy (comp->priv->current);
  g_hash_table_destroy (comp->priv->objects_hash);
  COMP_OBJECTS_UNLOCK (comp);

  g_mutex_free (comp->priv->objects_lock);
  gst_segment_free (comp->priv->segment);

  g_mutex_free (comp->priv->flushing_lock);


  g_free (comp->priv);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnl_composition_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GnlComposition *comp = (GnlComposition *) object;

  switch (prop_id) {
    case ARG_UPDATE:
      gnl_composition_set_update (comp, g_value_get_boolean (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gnl_composition_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GnlComposition *comp = (GnlComposition *) object;

  switch (prop_id) {
    case ARG_UPDATE:
      g_value_set_boolean (value, comp->priv->can_update);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* signal_duration_change
 * Creates a new GST_MESSAGE_DURATION with the currently configured
 * composition duration and sends that on the bus.
 */

static inline void
signal_duration_change (GnlComposition * comp)
{
  gst_element_post_message (GST_ELEMENT_CAST (comp),
      gst_message_new_duration (GST_OBJECT_CAST (comp),
          GST_FORMAT_TIME, ((GnlObject *) comp)->duration));
}

static gboolean
unblock_child_pads (GstElement * child, GValue * ret G_GNUC_UNUSED,
    GnlComposition * comp)
{
  GstPad *pad;

  GST_DEBUG_OBJECT (child, "unblocking pads");
  pad = get_src_pad (child);
  if (pad) {
    gst_pad_set_blocked_async (pad, FALSE, (GstPadBlockCallback) pad_blocked,
        comp);
    gst_object_unref (pad);
  }
  gst_object_unref (child);
  return TRUE;
}

static void
unblock_childs (GnlComposition * comp)
{
  GstIterator *childs;

  childs = gst_bin_iterate_elements (GST_BIN (comp));

retry:
  if (G_UNLIKELY (gst_iterator_fold (childs,
              (GstIteratorFoldFunction) unblock_child_pads, NULL,
              comp) == GST_ITERATOR_RESYNC)) {
    gst_iterator_resync (childs);
    goto retry;
  }
  gst_iterator_free (childs);
}


static gboolean
unlock_child_state (GstElement * child, GValue * ret G_GNUC_UNUSED,
    gpointer udata G_GNUC_UNUSED)
{
  GST_DEBUG_OBJECT (child, "unlocking state");
  gst_element_set_locked_state (child, FALSE);
  gst_object_unref (child);
  return TRUE;
}

static gboolean
lock_child_state (GstElement * child, GValue * ret G_GNUC_UNUSED,
    gpointer udata G_GNUC_UNUSED)
{
  GST_DEBUG_OBJECT (child, "locking state");
  gst_element_set_locked_state (child, TRUE);
  gst_object_unref (child);
  return TRUE;
}

static void
unlock_childs (GnlComposition * comp)
{
  GstIterator *childs;

  childs = gst_bin_iterate_elements (GST_BIN (comp));
retry:
  if (G_UNLIKELY (gst_iterator_fold (childs,
              (GstIteratorFoldFunction) unlock_child_state, NULL,
              NULL) == GST_ITERATOR_RESYNC)) {
    gst_iterator_resync (childs);
    goto retry;
  }
  gst_iterator_free (childs);
}

static void
gnl_composition_reset (GnlComposition * comp)
{
  GST_DEBUG_OBJECT (comp, "resetting");

  comp->priv->segment_start = GST_CLOCK_TIME_NONE;
  comp->priv->segment_stop = GST_CLOCK_TIME_NONE;

  gst_segment_init (comp->priv->segment, GST_FORMAT_TIME);

  if (comp->priv->current)
    g_node_destroy (comp->priv->current);
  comp->priv->current = NULL;

  comp->priv->stackvalid = FALSE;

  if (comp->priv->ghostpad) {
    gnl_object_remove_ghost_pad ((GnlObject *) comp, comp->priv->ghostpad);
    comp->priv->ghostpad = NULL;
    comp->priv->ghosteventprobe = 0;
  }

  if (comp->priv->childseek) {
    gst_event_unref (comp->priv->childseek);
    comp->priv->childseek = NULL;
  }

  comp->priv->waitingpads = 0;

  unlock_childs (comp);

  COMP_FLUSHING_LOCK (comp);
  if (comp->priv->pending_idle)
    g_source_remove (comp->priv->pending_idle);
  comp->priv->pending_idle = 0;
  comp->priv->flushing = FALSE;
  COMP_FLUSHING_UNLOCK (comp);

  comp->priv->update_required = FALSE;

  GST_DEBUG_OBJECT (comp, "Composition now resetted");
}

static gboolean
eos_main_thread (GnlComposition * comp)
{
  /* Set up a non-initial seek on segment_stop */
  GST_DEBUG_OBJECT (comp,
      "Setting segment->start to segment_stop:%" GST_TIME_FORMAT,
      GST_TIME_ARGS (comp->priv->segment_stop));
  comp->priv->segment->start = comp->priv->segment_stop;

  seek_handling (comp, TRUE, TRUE);

  if (!comp->priv->current) {
    /* If we're at the end, post SEGMENT_DONE, or push EOS */
    GST_DEBUG_OBJECT (comp, "Nothing else to play");

    if (!(comp->priv->segment->flags & GST_SEEK_FLAG_SEGMENT)
        && comp->priv->ghostpad) {
      GST_LOG_OBJECT (comp, "Pushing out EOS");
      gst_pad_push_event (comp->priv->ghostpad, gst_event_new_eos ());
    } else if (comp->priv->segment->flags & GST_SEEK_FLAG_SEGMENT) {
      gint64 epos;

      if (GST_CLOCK_TIME_IS_VALID (comp->priv->segment->stop))
        epos = (MIN (comp->priv->segment->stop, ((GnlObject *) comp)->stop));
      else
        epos = (((GnlObject *) comp)->stop);

      GST_LOG_OBJECT (comp, "Emitting segment done pos %" GST_TIME_FORMAT,
          GST_TIME_ARGS (epos));
      gst_element_post_message (GST_ELEMENT_CAST (comp),
          gst_message_new_segment_done (GST_OBJECT (comp),
              comp->priv->segment->format, epos));
    }
  }
  return FALSE;
}

static gboolean
ghost_event_probe_handler (GstPad * ghostpad G_GNUC_UNUSED, GstEvent * event,
    GnlComposition * comp)
{
  gboolean keepit = TRUE;

  GST_DEBUG_OBJECT (comp, "event: %s", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_NEWSEGMENT:{
      COMP_FLUSHING_LOCK (comp);
      if (comp->priv->pending_idle) {
        GST_DEBUG_OBJECT (comp, "removing pending seek for main thread");
        g_source_remove (comp->priv->pending_idle);
      }
      comp->priv->pending_idle = 0;
      comp->priv->flushing = FALSE;
      COMP_FLUSHING_UNLOCK (comp);
    }
      break;
    case GST_EVENT_EOS:{
      COMP_FLUSHING_LOCK (comp);
      if (comp->priv->flushing) {
        GST_DEBUG_OBJECT (comp, "flushing, bailing out");
        COMP_FLUSHING_UNLOCK (comp);
        keepit = FALSE;
        break;
      }
      COMP_FLUSHING_UNLOCK (comp);

      GST_DEBUG_OBJECT (comp, "Adding eos handling to main thread");
      if (comp->priv->pending_idle) {
        GST_WARNING_OBJECT (comp,
            "There was already a pending eos in main thread !");
        g_source_remove (comp->priv->pending_idle);
      }

      /* FIXME : This should be switched to using a g_thread_create() instead
       * of a g_idle_add(). EXTENSIVE TESTING AND ANALYSIS REQUIRED BEFORE
       * DOING THE SWITCH !!! */
      comp->priv->pending_idle =
          g_idle_add ((GSourceFunc) eos_main_thread, (gpointer) comp);

      keepit = FALSE;
    }
      break;
    default:
      break;
  }

  return keepit;
}



/* Warning : Don't take the objects lock in this method */
static void
gnl_composition_handle_message (GstBin * bin, GstMessage * message)
{
  GnlComposition *comp = (GnlComposition *) bin;
  gboolean dropit = FALSE;

  GST_DEBUG_OBJECT (comp, "message:%s from %s",
      gst_message_type_get_name (GST_MESSAGE_TYPE (message)),
      GST_MESSAGE_SRC (message) ? GST_ELEMENT_NAME (GST_MESSAGE_SRC (message)) :
      "UNKNOWN");

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:
    case GST_MESSAGE_WARNING:{
      /* FIXME / HACK
       * There is a massive issue with reverse negotiation and dynamic pipelines.
       *
       * Since we're not waiting for the pads of the previous stack to block before
       * re-switching, we might end up switching sources in the middle of a downstrea
       * negotiation which will do reverse negotiation... with the new source (which
       * is no longer the one that issues the request). That negotiation will fail
       * and the original source will emit an ERROR message.
       *
       * In order to avoid those issues, we just ignore error messages from elements
       * which aren't in the currently configured stack
       */
      if (GST_MESSAGE_SRC (message) && GNL_IS_OBJECT (GST_MESSAGE_SRC (message))
          && !OBJECT_IN_ACTIVE_SEGMENT (comp, GST_MESSAGE_SRC (message))) {
        GST_DEBUG_OBJECT (comp,
            "HACK Dropping error message from object not in currently configured stack !");
        dropit = TRUE;
      }
    }
    default:
      break;
  }

  if (dropit)
    gst_message_unref (message);
  else
    GST_BIN_CLASS (parent_class)->handle_message (bin, message);
}

static gint
priority_comp (GnlObject * a, GnlObject * b)
{
  if (a->priority < b->priority)
    return -1;
  if (a->priority > b->priority)
    return 1;
  return 0;
}

static inline gboolean
have_to_update_pipeline (GnlComposition * comp)
{
  GST_DEBUG_OBJECT (comp,
      "segment[%" GST_TIME_FORMAT "--%" GST_TIME_FORMAT "] current[%"
      GST_TIME_FORMAT "--%" GST_TIME_FORMAT "]",
      GST_TIME_ARGS (comp->priv->segment->start),
      GST_TIME_ARGS (comp->priv->segment->stop),
      GST_TIME_ARGS (comp->priv->segment_start),
      GST_TIME_ARGS (comp->priv->segment_stop));

  if (comp->priv->segment->start < comp->priv->segment_start)
    return TRUE;
  if (comp->priv->segment->start >= comp->priv->segment_stop)
    return TRUE;
  return FALSE;
}

static void
gnl_composition_set_update (GnlComposition * comp, gboolean update)
{
  if (G_UNLIKELY (update == comp->priv->can_update))
    return;

  GST_DEBUG_OBJECT (comp, "update:%d [currently %d], update_required:%d",
      update, comp->priv->can_update, comp->priv->update_required);

  COMP_OBJECTS_LOCK (comp);
  comp->priv->can_update = update;

  if (update && comp->priv->update_required) {
    GstClockTime curpos;

    /* Get current position */
    if ((curpos = get_current_position (comp)) == GST_CLOCK_TIME_NONE) {
      if (GST_CLOCK_TIME_IS_VALID (comp->priv->segment_start))
        curpos = comp->priv->segment->start = comp->priv->segment_start;
      else
        curpos = 0;
    }

    COMP_OBJECTS_UNLOCK (comp);

    /* update pipeline to that position */
    update_pipeline (comp, curpos, TRUE, TRUE, TRUE);
  } else
    COMP_OBJECTS_UNLOCK (comp);
}

/*
 * get_new_seek_event:
 *
 * Returns a seek event for the currently configured segment
 * and start/stop values
 *
 * The GstSegment and segment_start|stop must have been configured
 * before calling this function.
 */
static GstEvent *
get_new_seek_event (GnlComposition * comp, gboolean initial,
    gboolean updatestoponly)
{
  GstSeekFlags flags;
  gint64 start, stop;
  GstSeekType starttype = GST_SEEK_TYPE_SET;

  GST_DEBUG_OBJECT (comp, "initial:%d", initial);
  /* remove the seek flag */
  if (!(initial))
    flags = comp->priv->segment->flags;
  else
    flags = GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH;

  GST_DEBUG_OBJECT (comp,
      "private->segment->start:%" GST_TIME_FORMAT " segment_start%"
      GST_TIME_FORMAT, GST_TIME_ARGS (comp->priv->segment->start),
      GST_TIME_ARGS (comp->priv->segment_start));
  GST_DEBUG_OBJECT (comp,
      "private->segment->stop:%" GST_TIME_FORMAT " segment_stop%"
      GST_TIME_FORMAT, GST_TIME_ARGS (comp->priv->segment->stop),
      GST_TIME_ARGS (comp->priv->segment_stop));

  start = MAX (comp->priv->segment->start, comp->priv->segment_start);
  stop = GST_CLOCK_TIME_IS_VALID (comp->priv->segment->stop)
      ? MIN (comp->priv->segment->stop, comp->priv->segment_stop)
      : comp->priv->segment_stop;
  if (updatestoponly) {
    starttype = GST_SEEK_TYPE_NONE;
    start = GST_CLOCK_TIME_NONE;
  }

  GST_DEBUG_OBJECT (comp,
      "Created new seek event. Flags:%d, start:%" GST_TIME_FORMAT ", stop:%"
      GST_TIME_FORMAT, flags, GST_TIME_ARGS (start), GST_TIME_ARGS (stop));
  return gst_event_new_seek (comp->priv->segment->rate,
      comp->priv->segment->format, flags, starttype, start,
      GST_SEEK_TYPE_SET, stop);
}

/* OBJECTS LOCK must be taken when calling this ! */
static GstClockTime
get_current_position (GnlComposition * comp)
{
  GstFormat format = GST_FORMAT_TIME;
  gint64 value = GST_CLOCK_TIME_NONE;
  GstPad *pad;
  GnlObject *obj;
  gboolean res;

  /* 1. Try querying position downstream */
  if (comp->priv->ghostpad) {
    GstPad *peer = gst_pad_get_peer (comp->priv->ghostpad);
    if (peer) {
      res = gst_pad_query_position (peer, &format, &value);
      gst_object_unref (peer);
      if (res && (format == GST_FORMAT_TIME)) {
        GST_LOG_OBJECT (comp,
            "Successfully got downstream position %" GST_TIME_FORMAT,
            GST_TIME_ARGS ((guint64) value));
        goto beach;
      }
    }
    GST_DEBUG_OBJECT (comp, "Downstream position query failed");
    /* resetting format/value */
    format = GST_FORMAT_TIME;
    value = GST_CLOCK_TIME_NONE;
  }

  /* 2. If downstream fails , try within the current stack */
  if (!comp->priv->current) {
    GST_DEBUG_OBJECT (comp, "No current stack, can't send query");
    goto beach;
  }

  obj = (GnlObject *) comp->priv->current->data;

  if (!(pad = get_src_pad ((GstElement *) obj)))
    goto beach;

  res = gst_pad_query_position (pad, &format, &value);

  if (G_UNLIKELY ((res == FALSE) || (format != GST_FORMAT_TIME))) {
    GST_WARNING_OBJECT (comp,
        "query failed or returned a format different from TIME");
    value = GST_CLOCK_TIME_NONE;
  } else {
    GST_LOG_OBJECT (comp, "Query returned %" GST_TIME_FORMAT,
        GST_TIME_ARGS ((guint64) value));
  }

beach:
  return (guint64) value;
}

/*
  Figures out if pipeline needs updating.
  Updates it and sends the seek event.
  Sends flush events downstream if needed.
  can be called by user_seek or segment_done

  initial : FIXME : ???? Always seems to be TRUE
  update : TRUE from EOS, FALSE from seek
*/

static gboolean
seek_handling (GnlComposition * comp, gboolean initial, gboolean update)
{
  GST_DEBUG_OBJECT (comp, "initial:%d, update:%d", initial, update);

  COMP_FLUSHING_LOCK (comp);
  GST_DEBUG_OBJECT (comp, "Setting flushing to TRUE");
  comp->priv->flushing = TRUE;
  COMP_FLUSHING_UNLOCK (comp);

  if (update || have_to_update_pipeline (comp)) {
    update_pipeline (comp, comp->priv->segment->start, initial, TRUE, !update);
  }

  return TRUE;
}

static void
handle_seek_event (GnlComposition * comp, GstEvent * event)
{
  gboolean update;
  gdouble rate;
  GstFormat format;
  GstSeekFlags flags;
  GstSeekType cur_type, stop_type;
  gint64 cur, stop;

  gst_event_parse_seek (event, &rate, &format, &flags,
      &cur_type, &cur, &stop_type, &stop);

  GST_DEBUG_OBJECT (comp,
      "start:%" GST_TIME_FORMAT " -- stop:%" GST_TIME_FORMAT "  flags:%d",
      GST_TIME_ARGS (cur), GST_TIME_ARGS (stop), flags);

  gst_segment_set_seek (comp->priv->segment,
      rate, format, flags, cur_type, cur, stop_type, stop, &update);

  GST_DEBUG_OBJECT (comp, "Segment now has flags:%d",
      comp->priv->segment->flags);

  /* crop the segment start/stop values */
  /* Only crop segment start value if we don't have a default object */
  if (comp->priv->expandables == NULL)
    comp->priv->segment->start = MAX (comp->priv->segment->start,
        ((GnlObject *) comp)->start);
  comp->priv->segment->stop = MIN (comp->priv->segment->stop,
      ((GnlObject *) comp)->stop);

  seek_handling (comp, TRUE, FALSE);
}

static gboolean
gnl_composition_event_handler (GstPad * ghostpad, GstEvent * event)
{
  GnlComposition *comp = (GnlComposition *) gst_pad_get_parent (ghostpad);
  gboolean res = TRUE;

  GST_DEBUG_OBJECT (comp, "event type:%s", GST_EVENT_TYPE_NAME (event));
  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_SEEK:{
      GstEvent *nevent;

      handle_seek_event (comp, event);

      /* the incoming event might not be quite correct, we get a new proper
       * event to pass on to the childs. */
      COMP_OBJECTS_LOCK (comp);
      nevent = get_new_seek_event (comp, FALSE, FALSE);
      COMP_OBJECTS_UNLOCK (comp);
      gst_event_unref (event);
      event = nevent;
      break;
    }
    case GST_EVENT_QOS:{
      gdouble prop;
      GstClockTimeDiff diff;
      GstClockTime timestamp;

      gst_event_parse_qos (event, &prop, &diff, &timestamp);
      GST_INFO_OBJECT (comp, "timestamp:%" GST_TIME_FORMAT,
          GST_TIME_ARGS (timestamp));
      /* else we let it go through (gnlobject will take care of time-shifting) */
      break;
    }
    default:
      break;
  }

  /* FIXME : What should we do here if waitingpads != 0 ?? */
  /*            Delay ? Ignore ? Refuse ? */

  if (res && comp->priv->ghostpad) {
    GST_DEBUG_OBJECT (comp, "About to call gnl_event_pad_func()");
    COMP_OBJECTS_LOCK (comp);
    res = comp->priv->gnl_event_pad_func (comp->priv->ghostpad, event);
    COMP_OBJECTS_UNLOCK (comp);
    GST_DEBUG_OBJECT (comp, "Done calling gnl_event_pad_func() %d", res);
  }
  gst_object_unref (comp);
  return res;
}

static void
pad_blocked (GstPad * pad, gboolean blocked, GnlComposition * comp)
{
  GST_DEBUG_OBJECT (comp, "Pad : %s:%s , blocked:%d",
      GST_DEBUG_PAD_NAME (pad), blocked);
}

/* gnl_composition_ghost_pad_set_target:
 * target: The target #GstPad. The refcount will be decremented (given to the ghostpad).
 */

static void
gnl_composition_ghost_pad_set_target (GnlComposition * comp, GstPad * target)
{
  gboolean hadghost = (comp->priv->ghostpad) ? TRUE : FALSE;

  if (target)
    GST_DEBUG_OBJECT (comp, "%s:%s , hadghost:%d",
        GST_DEBUG_PAD_NAME (target), hadghost);
  else
    GST_DEBUG_OBJECT (comp, "Removing target, hadghost:%d", hadghost);

  if (!(hadghost)) {
    /* Create new ghostpad */
    comp->priv->ghostpad =
        gnl_object_ghost_pad_no_target ((GnlObject *) comp, "src", GST_PAD_SRC);
    if (!comp->priv->gnl_event_pad_func) {
      GST_DEBUG_OBJECT (comp->priv->ghostpad,
          "About to replace event_pad_func");
      comp->priv->gnl_event_pad_func = GST_PAD_EVENTFUNC (comp->priv->ghostpad);
    }
    gst_pad_set_event_function (comp->priv->ghostpad,
        GST_DEBUG_FUNCPTR (gnl_composition_event_handler));
    GST_DEBUG_OBJECT (comp->priv->ghostpad, "eventfunc is now %s",
        GST_DEBUG_FUNCPTR_NAME (GST_PAD_EVENTFUNC (comp->priv->ghostpad)));
  } else {
    GstPad *ptarget =
        gst_ghost_pad_get_target (GST_GHOST_PAD (comp->priv->ghostpad));

    if (ptarget && ptarget == target) {
      GST_DEBUG_OBJECT (comp,
          "Target of ghostpad is the same as existing one, not changing");
      gst_object_unref (ptarget);
      return;
    }

    /* Unset previous target */
    if (ptarget) {
      GST_DEBUG_OBJECT (comp, "Previous target was %s:%s, blocking that pad",
          GST_DEBUG_PAD_NAME (ptarget));
      gst_pad_set_blocked_async (ptarget, TRUE,
          (GstPadBlockCallback) pad_blocked, comp);
      /* remove event probe */
      if (comp->priv->ghosteventprobe) {
        gst_pad_remove_event_probe (ptarget, comp->priv->ghosteventprobe);
        comp->priv->ghosteventprobe = 0;
      }
      gst_object_unref (ptarget);
    }
  }

  gnl_object_ghost_pad_set_target ((GnlObject *) comp,
      comp->priv->ghostpad, target);

  if (target && (comp->priv->ghosteventprobe == 0)) {
    comp->priv->ghosteventprobe =
        gst_pad_add_event_probe (target, G_CALLBACK (ghost_event_probe_handler),
        comp);
    GST_DEBUG_OBJECT (comp, "added event probe %d",
        comp->priv->ghosteventprobe);
  }

  if (!(hadghost)) {
    gst_pad_set_active (comp->priv->ghostpad, TRUE);
    if (!(gst_element_add_pad (GST_ELEMENT (comp), comp->priv->ghostpad)))
      GST_WARNING ("Couldn't add the ghostpad");
    else {
      COMP_OBJECTS_UNLOCK (comp);
      gst_element_no_more_pads (GST_ELEMENT (comp));
      COMP_OBJECTS_LOCK (comp);
    }
  }
  GST_DEBUG_OBJECT (comp, "END");
}

static void
refine_start_stop_in_region_above_priority (GnlComposition * composition,
    GstClockTime timestamp, GstClockTime start,
    GstClockTime stop,
    GstClockTime * rstart, GstClockTime * rstop, guint32 priority)
{
  GList *tmp;
  GnlObject *object;
  GstClockTime nstart = start, nstop = stop;

  GST_DEBUG_OBJECT (composition,
      "timestamp:%" GST_TIME_FORMAT " start: %" GST_TIME_FORMAT " stop: %"
      GST_TIME_FORMAT " priority:%u", GST_TIME_ARGS (timestamp),
      GST_TIME_ARGS (start), GST_TIME_ARGS (stop), priority);

  for (tmp = composition->priv->objects_start; tmp; tmp = g_list_next (tmp)) {
    object = (GnlObject *) tmp->data;

    GST_LOG_OBJECT (object, "START %" GST_TIME_FORMAT "--%" GST_TIME_FORMAT,
        GST_TIME_ARGS (object->start), GST_TIME_ARGS (object->stop));

    if ((object->priority >= priority) || (!object->active))
      continue;

    if (object->start <= timestamp)
      continue;

    if (object->start >= nstop)
      continue;

    nstop = object->start;
    GST_DEBUG_OBJECT (composition,
        "START Found %s [prio:%u] at %" GST_TIME_FORMAT,
        GST_OBJECT_NAME (object), object->priority,
        GST_TIME_ARGS (object->start));
    break;
  }

  for (tmp = composition->priv->objects_stop; tmp; tmp = g_list_next (tmp)) {
    object = (GnlObject *) tmp->data;

    GST_LOG_OBJECT (object, "STOP %" GST_TIME_FORMAT "--%" GST_TIME_FORMAT,
        GST_TIME_ARGS (object->start), GST_TIME_ARGS (object->stop));

    if ((object->priority >= priority) || (!object->active))
      continue;

    if (object->stop >= timestamp)
      continue;

    if (object->stop <= nstart)
      continue;

    nstart = object->stop;
    GST_DEBUG_OBJECT (composition,
        "STOP Found %s [prio:%u] at %" GST_TIME_FORMAT,
        GST_OBJECT_NAME (object), object->priority,
        GST_TIME_ARGS (object->start));
    break;
  }

  if (*rstart)
    *rstart = nstart;
  if (*rstop)
    *rstop = nstop;
}


/*
 * Converts a sorted list to a tree
 * Recursive
 *
 * stack will be set to the next item to use in the parent.
 * If operations number of sinks is limited, it will only use that number.
 */

static GNode *
convert_list_to_tree (GList ** stack, GstClockTime * start,
    GstClockTime * stop, guint32 * highprio)
{
  GNode *ret;
  guint nbsinks;
  gboolean limit;
  GList *tmp;
  GnlObject *object;

  if (!stack || !*stack)
    return NULL;

  object = (GnlObject *) (*stack)->data;

  GST_DEBUG ("object:%s , *start:%" GST_TIME_FORMAT ", *stop:%"
      GST_TIME_FORMAT " highprio:%d",
      GST_ELEMENT_NAME (object), GST_TIME_ARGS (*start),
      GST_TIME_ARGS (*stop), *highprio);

  /* update earliest stop */
  if (GST_CLOCK_TIME_IS_VALID (*stop)) {
    if (GST_CLOCK_TIME_IS_VALID (object->stop) && (*stop > object->stop))
      *stop = object->stop;
  } else {
    *stop = object->stop;
  }

  if (GST_CLOCK_TIME_IS_VALID (*start)) {
    if (GST_CLOCK_TIME_IS_VALID (object->start) && (*start < object->start))
      *start = object->start;
  } else {
    *start = object->start;
  }

  if (GNL_IS_SOURCE (object)) {
    *stack = g_list_next (*stack);
    /* update highest priority.
     * We do this here, since it's only used with sources (leafs of the tree) */
    if (object->priority > *highprio)
      *highprio = object->priority;
    ret = g_node_new (object);
    goto beach;
  } else {                      /* GnlOperation */
    GnlOperation *oper = (GnlOperation *) object;

    GST_LOG_OBJECT (oper, "operation, num_sinks:%d", oper->num_sinks);
    ret = g_node_new (object);
    limit = (oper->dynamicsinks == FALSE);
    nbsinks = oper->num_sinks;

    /* FIXME : if num_sinks == -1 : request the proper number of pads */
    for (tmp = g_list_next (*stack); tmp && (!limit || nbsinks);) {
      g_node_append (ret, convert_list_to_tree (&tmp, start, stop, highprio));

      if (limit)
        nbsinks--;
    }
  }

beach:
  GST_DEBUG_OBJECT (object,
      "*start:%" GST_TIME_FORMAT " *stop:%" GST_TIME_FORMAT " priority:%u",
      GST_TIME_ARGS (*start), GST_TIME_ARGS (*stop), *highprio);

  return ret;
}

/*
 * get_stack_list:
 * @comp: The #GnlComposition
 * @timestamp: The #GstClockTime to look at
 * @priority: The priority level to start looking from
 * @activeonly: Only look for active elements if TRUE
 * @start: The biggest start time of the objects in the stack
 * @stop: The smallest stop time of the objects in the stack
 * @highprio: The highest priority in the stack
 *
 * Not MT-safe, you should take the objects lock before calling it.
 * Returns: A tree of #GNode sorted in priority order, corresponding
 * to the given search arguments. The returned value can be #NULL.
 */

static GNode *
get_stack_list (GnlComposition * comp, GstClockTime timestamp,
    guint32 priority, gboolean activeonly, GstClockTime * start,
    GstClockTime * stop, guint * highprio)
{
  GList *tmp = comp->priv->objects_start;
  GList *stack = NULL;
  GNode *ret = NULL;
  GstClockTime nstart = GST_CLOCK_TIME_NONE;
  GstClockTime nstop = GST_CLOCK_TIME_NONE;
  guint32 highest = 0;

  GST_DEBUG_OBJECT (comp,
      "timestamp:%" GST_TIME_FORMAT ", priority:%u, activeonly:%d",
      GST_TIME_ARGS (timestamp), priority, activeonly);

  GST_LOG ("objects_start:%p", comp->priv->objects_start);

  for (; tmp; tmp = g_list_next (tmp)) {
    GnlObject *object = (GnlObject *) tmp->data;

    GST_LOG_OBJECT (object,
        "start: %" GST_TIME_FORMAT " , stop:%" GST_TIME_FORMAT " , duration:%"
        GST_TIME_FORMAT ", priority:%u", GST_TIME_ARGS (object->start),
        GST_TIME_ARGS (object->stop), GST_TIME_ARGS (object->duration),
        object->priority);

    if (object->start <= timestamp) {
      if ((object->stop > timestamp) &&
          (object->priority >= priority) &&
          ((!activeonly) || (object->active))) {
        GST_LOG_OBJECT (comp, "adding %s: sorted to the stack",
            GST_OBJECT_NAME (object));
        stack = g_list_insert_sorted (stack, object,
            (GCompareFunc) priority_comp);
      }
    } else {
      GST_LOG_OBJECT (comp, "too far, stopping iteration");
      break;
    }
  }

  /* Insert the expandables */
  if (G_LIKELY (timestamp < ((GnlObject *) comp)->stop))
    for (tmp = comp->priv->expandables; tmp; tmp = g_list_next (tmp)) {
      GST_DEBUG_OBJECT (comp, "Adding expandable %s sorted to the list",
          GST_OBJECT_NAME (tmp->data));
      stack = g_list_insert_sorted (stack, tmp->data,
          (GCompareFunc) priority_comp);
    }

  /* convert that list to a stack */
  tmp = stack;
  ret = convert_list_to_tree (&tmp, &nstart, &nstop, &highest);

  GST_DEBUG ("nstart:%" GST_TIME_FORMAT ", nstop:%" GST_TIME_FORMAT,
      GST_TIME_ARGS (nstart), GST_TIME_ARGS (nstop));

  if (*stop)
    *stop = nstop;
  if (*start)
    *start = nstart;
  if (highprio)
    *highprio = highest;

  g_list_free (stack);

  return ret;
}

/*
 * get_clean_toplevel_stack:
 * @comp: The #GnlComposition
 * @timestamp: The #GstClockTime to look at
 * @stop_time: Pointer to a #GstClockTime for min stop time of returned stack
 * @start_time: Pointer to a #GstClockTime for greatest start time of returned stack
 *
 * Returns: The new current stack for the given #GnlComposition and @timestamp.
 */

static GNode *
get_clean_toplevel_stack (GnlComposition * comp, GstClockTime * timestamp,
    GstClockTime * start_time, GstClockTime * stop_time)
{
  GNode *stack = NULL;
  GList *tmp;
  GstClockTime start = G_MAXUINT64;
  GstClockTime stop = G_MAXUINT64;
  guint highprio;

  GST_DEBUG_OBJECT (comp, "timestamp:%" GST_TIME_FORMAT,
      GST_TIME_ARGS (*timestamp));

  stack = get_stack_list (comp, *timestamp, 0, TRUE, &start, &stop, &highprio);

  GST_DEBUG ("start:%" GST_TIME_FORMAT ", stop:%" GST_TIME_FORMAT,
      GST_TIME_ARGS (start), GST_TIME_ARGS (stop));

  if (!stack) {
    GnlObject *object = NULL;

    /* Case for gaps, therefore no objects at specified *timestamp */

    GST_DEBUG_OBJECT (comp,
        "Got empty stack, checking if it really was after the last object");
    /* Find the first active object just after *timestamp */
    for (tmp = comp->priv->objects_start; tmp; tmp = g_list_next (tmp)) {
      object = (GnlObject *) tmp->data;

      if ((object->start > *timestamp) && object->active)
        break;
    }

    if (tmp) {
      GST_DEBUG_OBJECT (comp,
          "Found a valid object after %" GST_TIME_FORMAT " : %s [%"
          GST_TIME_FORMAT "]", GST_TIME_ARGS (*timestamp),
          GST_ELEMENT_NAME (object), GST_TIME_ARGS (object->start));
      *timestamp = object->start;
      stack =
          get_stack_list (comp, *timestamp, 0, TRUE, &start, &stop, &highprio);
    }
  }

  GST_DEBUG ("start:%" GST_TIME_FORMAT ", stop:%" GST_TIME_FORMAT,
      GST_TIME_ARGS (start), GST_TIME_ARGS (stop));

  if (stack) {
    guint32 top_priority;

    top_priority = ((GnlObject *) stack->data)->priority;

    /* Figure out if there's anything blocking us with smaller priority */
    refine_start_stop_in_region_above_priority (comp, *timestamp, start, stop,
        &start, &stop, (highprio == 0) ? top_priority : highprio);
  }

  if (*stop_time) {
    if (stack)
      *stop_time = stop;
    else
      *stop_time = 0;
  }

  if (*start_time) {
    if (stack)
      *start_time = start;
    else
      *start_time = 0;
  }

  GST_DEBUG_OBJECT (comp,
      "Returning timestamp:%" GST_TIME_FORMAT " , start_time:%" GST_TIME_FORMAT
      " , stop_time:%" GST_TIME_FORMAT, GST_TIME_ARGS (*timestamp),
      GST_TIME_ARGS (*start_time), GST_TIME_ARGS (*stop_time));

  return stack;
}


/*
 *
 * UTILITY FUNCTIONS
 *
 */

/*
 * get_src_pad:
 * element: a #GstElement
 *
 * Returns: The src pad for the given element. A reference was added to the
 * returned pad, remove it when you don't need that pad anymore.
 * Returns NULL if there's no source pad.
 */
static GstPad *
get_src_pad (GstElement * element)
{
  GstIterator *it;
  GstIteratorResult itres;
  GstPad *srcpad;

  it = gst_element_iterate_src_pads (element);
  itres = gst_iterator_next (it, (gpointer) & srcpad);
  if (itres != GST_ITERATOR_OK) {
    GST_DEBUG ("%s doesn't have a src pad !", GST_ELEMENT_NAME (element));
    srcpad = NULL;
  }
  gst_iterator_free (it);
  return srcpad;
}


/*
 *
 * END OF UTILITY FUNCTIONS
 *
 */

static GstStateChangeReturn
gnl_composition_change_state (GstElement * element, GstStateChange transition)
{
  GnlComposition *comp = (GnlComposition *) element;
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:{
      GstIterator *childs;

      gnl_composition_reset (comp);
      /* state-lock all elements */
      GST_DEBUG_OBJECT (comp,
          "Setting all childs to READY and locking their state");
      childs = gst_bin_iterate_elements (GST_BIN (comp));
    retry:
      if (G_UNLIKELY (gst_iterator_fold (childs,
                  (GstIteratorFoldFunction) lock_child_state, NULL,
                  NULL) == GST_ITERATOR_RESYNC)) {
        gst_iterator_resync (childs);
        goto retry;
      }
      gst_iterator_free (childs);
    }

      /* set ghostpad target */
      if (!(update_pipeline (comp, COMP_REAL_START (comp), TRUE, FALSE, TRUE))) {
        ret = GST_STATE_CHANGE_FAILURE;
        goto beach;
      }
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
    case GST_STATE_CHANGE_READY_TO_NULL:
      gnl_composition_reset (comp);
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
    case GST_STATE_CHANGE_READY_TO_NULL:
      unblock_childs (comp);
      break;
    default:
      break;
  }

beach:
  return ret;
}

static gint
objects_start_compare (GnlObject * a, GnlObject * b)
{
  if (a->start == b->start) {
    if (a->priority < b->priority)
      return -1;
    if (a->priority > b->priority)
      return 1;
    return 0;
  }
  if (a->start < b->start)
    return -1;
  if (a->start > b->start)
    return 1;
  return 0;
}

static gint
objects_stop_compare (GnlObject * a, GnlObject * b)
{
  if (a->stop == b->stop) {
    if (a->priority < b->priority)
      return -1;
    if (a->priority > b->priority)
      return 1;
    return 0;
  }
  if (b->stop < a->stop)
    return -1;
  if (b->stop > a->stop)
    return 1;
  return 0;
}

static void
update_start_stop_duration (GnlComposition * comp)
{
  GnlObject *obj;
  GnlObject *cobj = (GnlObject *) comp;

  if (!(comp->priv->objects_start)) {
    GST_LOG ("no objects, resetting everything to 0");
    if (cobj->start) {
      cobj->start = 0;
      g_object_notify (G_OBJECT (cobj), "start");
    }
    if (cobj->duration) {
      cobj->duration = 0;
      g_object_notify (G_OBJECT (cobj), "duration");
      signal_duration_change (comp);
    }
    if (cobj->stop) {
      cobj->stop = 0;
      g_object_notify (G_OBJECT (cobj), "stop");
    }
    return;
  }

  /* If we have a default object, the start position is 0 */
  if (comp->priv->expandables) {
    GST_LOG_OBJECT (cobj,
        "Setting start to 0 because we have a default object");
    if (cobj->start != 0) {
      cobj->start = 0;
      g_object_notify (G_OBJECT (cobj), "start");
    }
  } else {
    /* Else it's the first object's start value */
    obj = (GnlObject *) comp->priv->objects_start->data;
    if (obj->start != cobj->start) {
      GST_LOG_OBJECT (obj, "setting start from %s to %" GST_TIME_FORMAT,
          GST_OBJECT_NAME (obj), GST_TIME_ARGS (obj->start));
      cobj->start = obj->start;
      g_object_notify (G_OBJECT (cobj), "start");
    }
  }

  obj = (GnlObject *) comp->priv->objects_stop->data;
  if (obj->stop != cobj->stop) {
    GST_LOG_OBJECT (obj, "setting stop from %s to %" GST_TIME_FORMAT,
        GST_OBJECT_NAME (obj), GST_TIME_ARGS (obj->stop));
    if (comp->priv->expandables) {
      GList *tmp = comp->priv->expandables;
      while (tmp) {
        g_object_set (tmp->data, "duration", obj->stop, NULL);
        g_object_set (tmp->data, "media-duration", obj->stop, NULL);
        tmp = g_list_next (tmp);
      }
    }
    comp->priv->segment->stop = obj->stop;
    cobj->stop = obj->stop;
    g_object_notify (G_OBJECT (cobj), "stop");
  }

  if ((cobj->stop - cobj->start) != cobj->duration) {
    cobj->duration = cobj->stop - cobj->start;
    g_object_notify (G_OBJECT (cobj), "duration");
    signal_duration_change (comp);
  }

  GST_LOG_OBJECT (comp,
      "start:%" GST_TIME_FORMAT
      " stop:%" GST_TIME_FORMAT
      " duration:%" GST_TIME_FORMAT,
      GST_TIME_ARGS (cobj->start),
      GST_TIME_ARGS (cobj->stop), GST_TIME_ARGS (cobj->duration));
}

static void
no_more_pads_object_cb (GstElement * element, GnlComposition * comp)
{
  GnlObject *object = (GnlObject *) element;
  GNode *tmp;
  GstPad *pad = NULL;
  GstPad *tpad = NULL;

  GST_LOG_OBJECT (element, "no more pads");
  if (!(pad = get_src_pad (element)))
    goto no_source;

  COMP_OBJECTS_LOCK (comp);

  if (comp->priv->current == NULL) {
    GST_DEBUG_OBJECT (comp, "current stack is empty !");
    goto done;
  }

  tmp = g_node_find (comp->priv->current, G_IN_ORDER, G_TRAVERSE_ALL, object);

  if (tmp) {
    GnlCompositionEntry *entry = COMP_ENTRY (comp, object);

    comp->priv->waitingpads--;
    GST_LOG_OBJECT (comp, "Number of waiting pads is now %d",
        comp->priv->waitingpads);

    if (tmp->parent) {
      /* child, link to parent */
      /* FIXME, shouldn't we check the order in which we link to the parent ? */
      if (!(gst_element_link (element, GST_ELEMENT (tmp->parent->data)))) {
        GST_WARNING_OBJECT (comp, "Couldn't link %s to %s",
            GST_ELEMENT_NAME (element),
            GST_ELEMENT_NAME (GST_ELEMENT (tmp->parent->data)));
        goto done;
      }
      gst_pad_set_blocked_async (pad, FALSE, (GstPadBlockCallback) pad_blocked,
          comp);
    }

    if (comp->priv->current && (comp->priv->waitingpads == 0)
        && comp->priv->stackvalid) {
      tpad = get_src_pad (GST_ELEMENT (comp->priv->current->data));

      /* There are no more waiting pads for the currently configured timeline */
      /* stack. */
      GST_LOG_OBJECT (comp,
          "top-level pad %s:%s, Setting target of ghostpad to it",
          GST_DEBUG_PAD_NAME (tpad));

      /* 1. set target of ghostpad to toplevel element src pad */
      gnl_composition_ghost_pad_set_target (comp, tpad);

      /* 2. send pending seek */
      if (comp->priv->childseek) {
        GstEvent *childseek = comp->priv->childseek;
        comp->priv->childseek = NULL;
        GST_INFO_OBJECT (comp, "Sending pending seek on %s:%s",
            GST_DEBUG_PAD_NAME (tpad));
        COMP_OBJECTS_UNLOCK (comp);
        if (!(gst_pad_send_event (tpad, childseek)))
          GST_ERROR_OBJECT (comp, "Sending seek event failed!");
        COMP_OBJECTS_LOCK (comp);
      }
      comp->priv->childseek = NULL;

      /* Check again if this element is still in the stack */
      if (comp->priv->current &&
          g_node_find (comp->priv->current, G_IN_ORDER, G_TRAVERSE_ALL,
              object)) {

        /* 3. unblock ghostpad */
        GST_LOG_OBJECT (comp, "About to unblock top-level pad : %s:%s",
            GST_DEBUG_PAD_NAME (tpad));
        gst_pad_set_blocked_async (tpad, FALSE,
            (GstPadBlockCallback) pad_blocked, comp);
        GST_LOG_OBJECT (comp, "Unblocked top-level pad");
      } else {
        GST_DEBUG ("Element went away from currently configured stack");
      }
    }

    /* deactivate nomorepads handler */
    g_signal_handler_disconnect (object, entry->nomorepadshandler);
    entry->nomorepadshandler = 0;
  } else {
    GST_LOG_OBJECT (comp,
        "The following object is not in currently configured stack : %s",
        GST_ELEMENT_NAME (object));
  }

done:
  COMP_OBJECTS_UNLOCK (comp);

  if (pad)
    gst_object_unref (pad);
  if (tpad)
    gst_object_unref (tpad);

  GST_DEBUG_OBJECT (comp, "end");

  return;

no_source:
  {
    GST_LOG_OBJECT (comp, "no source pad");
    return;
  }
}

/*
 * recursive depth-first relink stack function on new stack
 *
 * _ relink nodes with changed parent/order
 * _ links new nodes with parents
 * _ unblocks available source pads (except for toplevel)
 *
 * WITH OBJECTS LOCK TAKEN
 */

static void
compare_relink_single_node (GnlComposition * comp, GNode * node,
    GNode * oldstack)
{
  GNode *child;
  GNode *oldnode = NULL;
  GnlObject *newobj;
  GnlObject *newparent;
  GnlObject *oldparent = NULL;
  GstPad *srcpad = NULL;

  if (!node)
    return;

  newparent = G_NODE_IS_ROOT (node) ? NULL : (GnlObject *) node->parent->data;
  newobj = (GnlObject *) node->data;
  if (oldstack) {
    oldnode = g_node_find (oldstack, G_IN_ORDER, G_TRAVERSE_ALL, newobj);
    if (oldnode)
      oldparent =
          G_NODE_IS_ROOT (oldnode) ? NULL : (GnlObject *) oldnode->parent->data;
  }
  GST_DEBUG_OBJECT (comp, "newobj:%s",
      GST_ELEMENT_NAME ((GstElement *) newobj));

  srcpad = get_src_pad ((GstElement *) newobj);

  /* 1. Make sure the source pad is blocked for new objects */
  if (G_UNLIKELY (!oldnode && srcpad)) {
    GST_LOG_OBJECT (comp, "block_async(%s:%s, TRUE)",
        GST_DEBUG_PAD_NAME (srcpad));
    gst_pad_set_blocked_async (srcpad, TRUE, (GstPadBlockCallback) pad_blocked,
        comp);
  }

  /* 2. link to parent if needed */
  if (srcpad) {
    GST_LOG_OBJECT (comp, "has a valid source pad");
    /* POST PROCESSING */
    if ((oldparent != newparent) ||
        (oldparent && newparent &&
            (g_node_child_index (node, newobj) != g_node_child_index (oldnode,
                    newobj)))) {
      GST_LOG_OBJECT (comp,
          "not same parent, or same parent but in different order");

      /* relink to new parent in required order */
      if (newparent) {
        GST_LOG_OBJECT (comp, "Linking %s and %s",
            GST_ELEMENT_NAME (GST_ELEMENT (newobj)),
            GST_ELEMENT_NAME (GST_ELEMENT (newparent)));
        /* FIXME : do it in required order */
        if (!(gst_element_link ((GstElement *) newobj,
                    (GstElement *) newparent)))
          GST_ERROR_OBJECT (comp, "Couldn't link %s to %s",
              GST_ELEMENT_NAME (newobj), GST_ELEMENT_NAME (newparent));
      }
    } else
      GST_LOG_OBJECT (newobj, "Same parent and same position in the new stack");
  } else {
    GnlCompositionEntry *entry = COMP_ENTRY (comp, newobj);

    GST_LOG_OBJECT (newobj, "no existing pad, connecting to 'no-more-pads'");
    comp->priv->waitingpads++;
    if (!(entry->nomorepadshandler))
      entry->nomorepadshandler = g_signal_connect
          (G_OBJECT (newobj), "no-more-pads",
          G_CALLBACK (no_more_pads_object_cb), comp);
  }

  /* 3. Handle childs */
  if (GNL_IS_OPERATION (newobj)) {
    guint nbchilds = g_node_n_children (node);
    GnlOperation *oper = (GnlOperation *) newobj;

    GST_LOG_OBJECT (newobj, "is a %s operation, analyzing the %d childs",
        oper->dynamicsinks ? "dynamic" : "regular", nbchilds);

    /* Update the operation's number of sinks, that will make it have the proper
     * number of sink pads to connect the childs to. */
    if (oper->dynamicsinks)
      g_object_set (G_OBJECT (newobj), "sinks", nbchilds, NULL);

    for (child = node->children; child; child = child->next)
      compare_relink_single_node (comp, child, oldstack);

    if (G_UNLIKELY (nbchilds < oper->num_sinks))
      GST_ERROR
          ("Not enough sinkpads to link all objects to the operation ! %d / %d",
          oper->num_sinks, nbchilds);
    if (G_UNLIKELY (nbchilds == 0))
      GST_ERROR ("Operation has no child objects to be connected to !!!");
    /* Make sure we have enough sinkpads */
  } else {
    /* FIXME : do we need to do something specific for sources ? */
  }

  /* 4. Unblock source pad */
  if (srcpad && !G_NODE_IS_ROOT (node)) {
    GST_LOG_OBJECT (comp, "Unblocking pad %s:%s", GST_DEBUG_PAD_NAME (srcpad));
    gst_pad_set_blocked_async (srcpad, FALSE,
        (GstPadBlockCallback) pad_blocked, comp);
  }

  if (G_LIKELY (srcpad))
    gst_object_unref (srcpad);

  GST_LOG_OBJECT (comp, "done with object %s",
      GST_ELEMENT_NAME (GST_ELEMENT (newobj)));
}

/*
 * recursive depth-first compare stack function on old stack
 *
 * _ Add no-longer used objects to the deactivate list
 * _ unlink child-parent relations that have changed (not same parent, or not same order)
 * _ blocks available source pads
 *
 * FIXME : modify is only used for the root element.
 *    It is TRUE all the time except when the update is done from a seek
 *
 * WITH OBJECTS LOCK TAKEN
 */

static GList *
compare_deactivate_single_node (GnlComposition * comp, GNode * node,
    GNode * newstack, gboolean modify)
{
  GNode *child;
  GNode *newnode = NULL;        /* Same node in newstack */
  GnlObject *oldparent;
  GList *deactivate = NULL;
  GnlObject *oldobj = NULL;
  GstPad *srcpad = NULL;

  if (!node)
    return NULL;

  /* The former parent GnlObject (i.e. downstream) of the given node */
  oldparent = G_NODE_IS_ROOT (node) ? NULL : (GnlObject *) node->parent->data;

  /* The former GnlObject */
  oldobj = (GnlObject *) node->data;

  /* The node corresponding to oldobj in the new stack */
  if (newstack)
    newnode = g_node_find (newstack, G_IN_ORDER, G_TRAVERSE_ALL, oldobj);

  GST_DEBUG_OBJECT (comp, "oldobj:%s",
      GST_ELEMENT_NAME ((GstElement *) oldobj));

  srcpad = get_src_pad ((GstElement *) oldobj);

  if (G_LIKELY (srcpad)) {
    GstPad *peerpad = NULL;
    /* 1. Block source pad
     *   This makes sure that no data/event flow will come out of this element after this
     *   point. */
    GST_LOG_OBJECT (comp, "block_async(%s:%s, TRUE)",
        GST_DEBUG_PAD_NAME (srcpad));
    gst_pad_set_blocked_async (srcpad, TRUE, (GstPadBlockCallback) pad_blocked,
        comp);

    /* 2. If we have to modify or we have a parent, flush downstream 
     *   This ensures the streaming thread going through the current object has
     *   either stopped or is blocking against the source pad. */
    if ((modify || oldparent) && (peerpad = gst_pad_get_peer (srcpad))) {
      GST_LOG_OBJECT (comp, "Sending flush start/stop downstream ");

      gst_pad_send_event (peerpad, gst_event_new_flush_start ());
      gst_pad_send_event (peerpad, gst_event_new_flush_stop ());
      GST_DEBUG_OBJECT (comp, "DONE Sending flush events downstream");

      gst_object_unref (peerpad);
    }

  } else {
    GST_LOG_OBJECT (comp, "No source pad available");
  }

  /* 3. Unlink from the parent if we've changed position */

  GST_LOG_OBJECT (comp,
      "Checking if we need to unlink from downstream element");
  if (G_UNLIKELY (!oldparent)) {
    GST_LOG_OBJECT (comp, "Top-level object");
    /* for top-level objects we just set the ghostpad target to NULL */
    if (comp->priv->ghostpad) {
      GST_LOG_OBJECT (comp, "Setting ghostpad target to NULL");
      gnl_composition_ghost_pad_set_target (comp, NULL);
    } else
      GST_LOG_OBJECT (comp, "No ghostpad");
  } else {
    GnlObject *newparent = NULL;

    GST_LOG_OBJECT (comp, "non-toplevel object");

    if (newnode)
      newparent =
          G_NODE_IS_ROOT (newnode) ? NULL : (GnlObject *) newnode->parent->data;

    if ((!newnode) || (oldparent != newparent) ||
        (newparent &&
            (g_node_child_index (node, oldobj) != g_node_child_index (newnode,
                    oldobj)))) {
      GstPad *peerpad = NULL;
      GST_LOG_OBJECT (comp, "Topology changed, unlinking from downstream");
      if (srcpad && (peerpad = gst_pad_get_peer (srcpad))) {
        GST_LOG_OBJECT (peerpad, "Sending flush start/stop");
        gst_pad_send_event (peerpad, gst_event_new_flush_start ());
        gst_pad_send_event (peerpad, gst_event_new_flush_stop ());

        gst_pad_unlink (srcpad, peerpad);
        gst_object_unref (peerpad);
      }
    } else
      GST_LOG_OBJECT (comp, "Topology unchanged");
  }

  /* 4. If we're dealing with an operation, call this method recursively on it */
  if (G_UNLIKELY (GNL_IS_OPERATION (oldobj))) {
    GST_LOG_OBJECT (comp,
        "Object is an operation, recursively calling on childs");
    for (child = node->children; child; child = child->next) {
      GList *newdeac =
          compare_deactivate_single_node (comp, child, newstack, modify);

      if (newdeac)
        deactivate = g_list_concat (deactivate, newdeac);
    }
  }

  /* 5. If object isn't used anymore, add it to the list of objects to deactivate */
  if (G_LIKELY (!newnode)) {
    GST_LOG_OBJECT (comp, "Object doesn't exist in new stack");
    deactivate = g_list_prepend (deactivate, oldobj);
  }

  if (G_LIKELY (srcpad))
    gst_object_unref (srcpad);

  GST_LOG_OBJECT (comp, "done with object %s",
      GST_ELEMENT_NAME (GST_ELEMENT (oldobj)));

  return deactivate;
}

/*
 * compare_relink_stack:
 * @comp: The #GnlComposition
 * @stack: The new stack
 * @modify: TRUE if the timeline has changed and needs downstream flushes.
 *
 * Compares the given stack to the current one and relinks it if needed.
 *
 * WITH OBJECTS LOCK TAKEN
 *
 * Returns: The #GList of #GnlObject no longer used
 */

static GList *
compare_relink_stack (GnlComposition * comp, GNode * stack, gboolean modify)
{
  GList *deactivate = NULL;

  /* 1. reset waiting pads for new stack */
  comp->priv->waitingpads = 0;

  /* 2. Traverse old stack to deactivate no longer used objects */

  deactivate =
      compare_deactivate_single_node (comp, comp->priv->current, stack, modify);

  /* 3. Traverse new stack to do needed (re)links */

  compare_relink_single_node (comp, stack, comp->priv->current);

  return deactivate;
}

static void
unlock_activate_stack (GnlComposition * comp, GNode * node,
    gboolean change_state, GstState state)
{
  GNode *child;

  GST_LOG_OBJECT (comp, "object:%s",
      GST_ELEMENT_NAME ((GstElement *) (node->data)));

  gst_element_set_locked_state ((GstElement *) (node->data), FALSE);
  if (change_state)
    gst_element_set_state (GST_ELEMENT (node->data), state);
  for (child = node->children; child; child = child->next)
    unlock_activate_stack (comp, child, change_state, state);
}

static gboolean
are_same_stacks (GNode * stack1, GNode * stack2)
{
  gboolean res = FALSE;

  /* TODO : FIXME : we should also compare start/media-start */

  /* stacks are not equal if one of them is NULL but not the other */
  if ((!stack1 && stack2) || (stack1 && !stack2))
    goto beach;

  if (stack1 && stack2) {
    GNode *child1, *child2;

    /* if they don't contain the same source, not equal */
    if (!(stack1->data == stack2->data))
      goto beach;

    /* if they don't have the same number of childs, not equal */
    if (!(g_node_n_children (stack1) == g_node_n_children (stack2)))
      goto beach;

    child1 = stack1->children;
    child2 = stack2->children;
    while (child1 && child2) {
      if (!(are_same_stacks (child1, child2)))
        goto beach;
      child1 = g_node_next_sibling (child1);
      child2 = g_node_next_sibling (child2);
    }

    /* if there's a difference in child number, stacks are not equal */
    if (child1 || child2)
      goto beach;
  }

  /* if stack1 AND stack2 are NULL, then they're equal (both empty) */
  res = TRUE;

beach:
  GST_LOG ("Stacks are equal : %d", res);
  return res;
}

/*
 * update_pipeline:
 * @comp: The #GnlComposition
 * @currenttime: The #GstClockTime to update at, can be GST_CLOCK_TIME_NONE.
 * @initial: TRUE if this is the first setup
 * @change_state: Change the state of the (de)activated objects if TRUE.
 * @modify: Flush downstream if TRUE. Needed for modified timelines.
 *
 * Updates the internal pipeline and properties. If @currenttime is 
 * GST_CLOCK_TIME_NONE, it will not modify the current pipeline
 *
 * Returns: FALSE if there was an error updating the pipeline.
 */

static gboolean
update_pipeline (GnlComposition * comp, GstClockTime currenttime,
    gboolean initial, gboolean change_state, gboolean modify)
{
  gboolean ret = TRUE;

  GST_DEBUG_OBJECT (comp,
      "currenttime:%" GST_TIME_FORMAT
      " initial:%d , change_state:%d , modify:%d", GST_TIME_ARGS (currenttime),
      initial, change_state, modify);

  COMP_OBJECTS_LOCK (comp);

  if (G_UNLIKELY (!comp->priv->can_update)) {
    COMP_OBJECTS_UNLOCK (comp);
    return TRUE;
  }

  update_start_stop_duration (comp);

  if (GST_CLOCK_TIME_IS_VALID (currenttime)) {
    GstState state = GST_STATE (comp);
    GstState nextstate =
        (GST_STATE_NEXT (comp) ==
        GST_STATE_VOID_PENDING) ? GST_STATE (comp) : GST_STATE_NEXT (comp);
    GNode *stack = NULL;
    GList *deactivate = NULL;
    GstClockTime new_start = GST_CLOCK_TIME_NONE;
    GstClockTime new_stop = GST_CLOCK_TIME_NONE;
    gboolean samestack = FALSE;
    gboolean startchanged, stopchanged;

    GST_DEBUG_OBJECT (comp,
        "now really updating the pipeline, current-state:%s",
        gst_element_state_get_name (state));


    /* (re)build the stack and relink new elements */
    stack =
        get_clean_toplevel_stack (comp, &currenttime, &new_start, &new_stop);
    samestack = are_same_stacks (comp->priv->current, stack);

    if (!samestack)
      deactivate = compare_relink_stack (comp, stack, modify);

    startchanged = comp->priv->segment_start != currenttime;
    stopchanged = comp->priv->segment_stop != new_stop;

    /* set new segment_start/stop */
    comp->priv->segment_start = currenttime;
    comp->priv->segment_stop = new_stop;

    /* Clear pending child seek */
    if (comp->priv->childseek) {
      GST_DEBUG ("unreffing event %p", comp->priv->childseek);
      gst_event_unref (comp->priv->childseek);
      comp->priv->childseek = NULL;
    }

    /* activate new stack */
    if (comp->priv->current)
      g_node_destroy (comp->priv->current);
    comp->priv->current = NULL;

    /* invalidate the stack while modifying it */
    comp->priv->stackvalid = FALSE;

    COMP_OBJECTS_UNLOCK (comp);

    if (deactivate) {
      GList *tmp;

      GST_DEBUG_OBJECT (comp, "De-activating objects no longer used");

      /* state-lock elements no more used */
      for (tmp = deactivate; tmp; tmp = g_list_next (tmp)) {
        GST_LOG ("%p", tmp->data);

        if (change_state)
          gst_element_set_state (GST_ELEMENT (tmp->data), state);
        gst_element_set_locked_state (GST_ELEMENT (tmp->data), TRUE);
      }
      g_list_free (deactivate);

      GST_DEBUG_OBJECT (comp, "Finished de-activating objects no longer used");
    }

    comp->priv->current = stack;

    GST_DEBUG_OBJECT (comp, "activating objects in new stack to %s",
        gst_element_state_get_name (nextstate));

    if (!samestack && stack)
      unlock_activate_stack (comp, stack, change_state, nextstate);
    GST_DEBUG_OBJECT (comp, "Finished activating objects in new stack");

    if (comp->priv->current) {
      GstEvent *event;

      /* There is a valid timeline stack */

      COMP_OBJECTS_LOCK (comp);

      comp->priv->stackvalid = TRUE;

      /* 1. Create new seek event for newly configured timeline stack */
      if (samestack && (startchanged || stopchanged))
        event =
            get_new_seek_event (comp,
            (state == GST_STATE_PLAYING) ? FALSE : TRUE, !startchanged);
      else
        event = get_new_seek_event (comp, initial, FALSE);

      /* 2. Is the stack entirely ready ? */
      if (comp->priv->waitingpads == 0) {
        GstPad *pad = NULL;

        /* 2.a. Stack is entirely ready */

        /* 3. Get toplevel object source pad */
        if ((pad = get_src_pad (GST_ELEMENT (comp->priv->current->data)))) {

          GST_DEBUG_OBJECT (comp, "We have a valid toplevel element pad %s:%s",
              GST_DEBUG_PAD_NAME (pad));

          /* 4. Unconditionnaly set the ghostpad target to pad */
          GST_LOG_OBJECT (comp,
              "Setting the composition's ghostpad target to %s:%s",
              GST_DEBUG_PAD_NAME (pad));
          gnl_composition_ghost_pad_set_target (comp, pad);

          /* 5. send seek event */
          GST_LOG_OBJECT (comp, "sending seek event");
          if (!(gst_pad_send_event (pad, event))) {
            ret = FALSE;
          } else {
            /* 6. unblock top-level pad */
            GST_LOG_OBJECT (comp, "About to unblock top-level srcpad");
            gst_pad_set_blocked_async (pad, FALSE,
                (GstPadBlockCallback) pad_blocked, comp);
          }
          gst_object_unref (pad);
        } else {
          GST_WARNING_OBJECT (comp,
              "Timeline is entirely linked, but couldn't get top-level element's source pad");
          ret = FALSE;
        }
      } else {
        /* 2.b. Stack isn't entirely ready, save seek event for later on */
        GST_LOG_OBJECT (comp,
            "The timeline stack isn't entirely linked, delaying sending seek event (waitingpads:%d)",
            comp->priv->waitingpads);
        comp->priv->childseek = event;
        ret = TRUE;
      }
      COMP_OBJECTS_UNLOCK (comp);

    } else {
      if ((!comp->priv->objects_start) && comp->priv->ghostpad) {
        GST_DEBUG_OBJECT (comp, "composition is now empty, removing ghostpad");
        gnl_object_remove_ghost_pad ((GnlObject *) comp, comp->priv->ghostpad);
        comp->priv->ghostpad = NULL;
        comp->priv->ghosteventprobe = 0;
        comp->priv->segment_start = 0;
        comp->priv->segment_stop = GST_CLOCK_TIME_NONE;
      }
    }
  } else {
    COMP_OBJECTS_UNLOCK (comp);
  }

  GST_DEBUG_OBJECT (comp, "Returning %d", ret);
  return ret;
}

/* 
 * Child modification updates
 */

static void
object_start_stop_priority_changed (GnlObject * object,
    GParamSpec * arg G_GNUC_UNUSED, GnlComposition * comp)
{
  GST_DEBUG_OBJECT (object, "start/stop/priority  changed (%" GST_TIME_FORMAT
      "/%" GST_TIME_FORMAT "/%d), evaluating pipeline update",
      GST_TIME_ARGS (object->start),
      GST_TIME_ARGS (object->stop), object->priority);

  /* The topology of the ocmposition might have changed, update the lists */
  comp->priv->objects_start = g_list_sort
      (comp->priv->objects_start, (GCompareFunc) objects_start_compare);

  comp->priv->objects_stop = g_list_sort
      (comp->priv->objects_stop, (GCompareFunc) objects_stop_compare);

  if (!comp->priv->can_update) {
    comp->priv->update_required = TRUE;
    update_start_stop_duration (comp);
    return;
  }

  /* Update pipeline if needed */
  if (comp->priv->current &&
      (OBJECT_IN_ACTIVE_SEGMENT (comp, object) ||
          g_node_find (comp->priv->current, G_IN_ORDER, G_TRAVERSE_ALL,
              object))) {
    GstClockTime curpos = get_current_position (comp);
    if (curpos == GST_CLOCK_TIME_NONE)
      curpos = comp->priv->segment->start = comp->priv->segment_start;
    update_pipeline (comp, curpos, TRUE, TRUE, TRUE);
  } else
    update_start_stop_duration (comp);
}

static void
object_active_changed (GnlObject * object, GParamSpec * arg G_GNUC_UNUSED,
    GnlComposition * comp)
{
  GST_DEBUG_OBJECT (object,
      "active flag changed (%d), evaluating pipeline update", object->active);

  if (!comp->priv->can_update) {
    comp->priv->update_required = TRUE;
    return;
  }

  if (comp->priv->current && OBJECT_IN_ACTIVE_SEGMENT (comp, object)) {
    GstClockTime curpos = get_current_position (comp);
    if (curpos == GST_CLOCK_TIME_NONE)
      curpos = comp->priv->segment->start = comp->priv->segment_start;
    update_pipeline (comp, curpos, TRUE, TRUE, TRUE);
  } else
    update_start_stop_duration (comp);
}

static void
object_pad_removed (GnlObject * object, GstPad * pad, GnlComposition * comp)
{
  GST_DEBUG_OBJECT (comp, "pad %s:%s was removed", GST_DEBUG_PAD_NAME (pad));

  /* remove ghostpad if it's the current top stack object */
  if (GST_PAD_IS_SRC (pad) && comp->priv->current
      && ((GnlObject *) comp->priv->current->data == object)
      && comp->priv->ghostpad) {
    GST_DEBUG_OBJECT (comp, "Removing ghostpad");
    gnl_object_remove_ghost_pad ((GnlObject *) comp, comp->priv->ghostpad);
    comp->priv->ghostpad = NULL;
    comp->priv->ghosteventprobe = 0;
  } else {
    /* unblock it ! */
    gst_pad_set_blocked_async (pad, FALSE, (GstPadBlockCallback) pad_blocked,
        comp);
  }
}

static void
object_pad_added (GnlObject * object G_GNUC_UNUSED, GstPad * pad,
    GnlComposition * comp)
{
  if (GST_PAD_DIRECTION (pad) == GST_PAD_SINK)
    return;

  GST_DEBUG_OBJECT (comp, "pad %s:%s was added, blocking it",
      GST_DEBUG_PAD_NAME (pad));

  gst_pad_set_blocked_async (pad, TRUE, (GstPadBlockCallback) pad_blocked,
      comp);
}

static gboolean
gnl_composition_add_object (GstBin * bin, GstElement * element)
{
  gboolean ret;
  GnlCompositionEntry *entry;
  GnlComposition *comp = (GnlComposition *) bin;
  gboolean update_required;
  GstClockTime curpos = GST_CLOCK_TIME_NONE;

  GST_DEBUG_OBJECT (bin, "element %s", GST_OBJECT_NAME (element));

  /* we only accept GnlObject */
  g_return_val_if_fail (GNL_IS_OBJECT (element), FALSE);
  gst_object_ref (element);

  GST_DEBUG_OBJECT (element, "%" GST_TIME_FORMAT "--%" GST_TIME_FORMAT,
      GST_TIME_ARGS (((GnlObject *) element)->start),
      GST_TIME_ARGS (((GnlObject *) element)->stop));

  COMP_OBJECTS_LOCK (comp);

  if (((((GnlObject *) element)->priority == G_MAXUINT32) ||
          GNL_OBJECT_IS_EXPANDABLE (element)) &&
      g_list_find (comp->priv->expandables, element)) {
    GST_WARNING_OBJECT (comp,
        "We already have an expandable, remove it before adding new one");
    ret = FALSE;
    goto chiringuito;
  }

  ret = GST_BIN_CLASS (parent_class)->add_element (bin, element);

  if (!ret) {
    GST_WARNING_OBJECT (bin, "couldn't add element");
    goto chiringuito;
  }

  /* lock state of child ! */
  GST_LOG_OBJECT (bin, "Locking state of %s", GST_ELEMENT_NAME (element));
  gst_element_set_locked_state (element, TRUE);

  /* wrap new element in a GnlCompositionEntry ... */
  entry = g_new0 (GnlCompositionEntry, 1);
  entry->object = (GnlObject *) element;
  if (G_LIKELY ((((GnlObject *) element)->priority != G_MAXUINT32) &&
          !GNL_OBJECT_IS_EXPANDABLE (element))) {
    /* Only react on non-default objects properties */
    entry->starthandler = g_signal_connect (G_OBJECT (element),
        "notify::start", G_CALLBACK (object_start_stop_priority_changed), comp);
    entry->stophandler = g_signal_connect (G_OBJECT (element),
        "notify::stop", G_CALLBACK (object_start_stop_priority_changed), comp);
    entry->priorityhandler = g_signal_connect (G_OBJECT (element),
        "notify::priority", G_CALLBACK (object_start_stop_priority_changed),
        comp);
  } else {
    /* We set the default source start/stop values to 0 and composition-stop */
    g_object_set (element,
        "start", (GstClockTime) 0,
        "media-start", (GstClockTime) 0,
        "duration", (GstClockTimeDiff) ((GnlObject *) comp)->stop,
        "media-duration", (GstClockTimeDiff) ((GnlObject *) comp)->stop, NULL);
  }
  entry->activehandler = g_signal_connect (G_OBJECT (element),
      "notify::active", G_CALLBACK (object_active_changed), comp);
  entry->padremovedhandler = g_signal_connect (G_OBJECT (element),
      "pad-removed", G_CALLBACK (object_pad_removed), comp);
  entry->padaddedhandler = g_signal_connect (G_OBJECT (element),
      "pad-added", G_CALLBACK (object_pad_added), comp);

  /* ...and add it to the hash table */
  g_hash_table_insert (comp->priv->objects_hash, element, entry);

  /* Special case for default source. */
  if ((((GnlObject *) element)->priority == G_MAXUINT32) ||
      GNL_OBJECT_IS_EXPANDABLE (element)) {
    /* It doesn't get added to objects_start and objects_stop. */
    comp->priv->expandables = g_list_prepend (comp->priv->expandables, element);
    goto chiringuito;
  }

  /* add it sorted to the objects list */
  comp->priv->objects_start = g_list_insert_sorted
      (comp->priv->objects_start, element,
      (GCompareFunc) objects_start_compare);

  if (comp->priv->objects_start)
    GST_LOG_OBJECT (comp,
        "Head of objects_start is now %s [%" GST_TIME_FORMAT "--%"
        GST_TIME_FORMAT "]",
        GST_OBJECT_NAME (comp->priv->objects_start->data),
        GST_TIME_ARGS (((GnlObject *)
                comp->priv->objects_start->data)->start),
        GST_TIME_ARGS (((GnlObject *)
                comp->priv->objects_start->data)->stop));

  comp->priv->objects_stop = g_list_insert_sorted
      (comp->priv->objects_stop, element, (GCompareFunc) objects_stop_compare);

  if (comp->priv->objects_stop)
    GST_LOG_OBJECT (comp,
        "Head of objects_stop is now %s [%" GST_TIME_FORMAT "--%"
        GST_TIME_FORMAT "]",
        GST_OBJECT_NAME (comp->priv->objects_stop->data),
        GST_TIME_ARGS (((GnlObject *)
                comp->priv->objects_stop->data)->start),
        GST_TIME_ARGS (((GnlObject *)
                comp->priv->objects_stop->data)->stop));

  GST_DEBUG_OBJECT (comp,
      "segment_start:%" GST_TIME_FORMAT " segment_stop:%" GST_TIME_FORMAT,
      GST_TIME_ARGS (comp->priv->segment_start),
      GST_TIME_ARGS (comp->priv->segment_stop));

  update_required = OBJECT_IN_ACTIVE_SEGMENT (comp, element)
      || (!comp->priv->current);

  /* We only need the current position if we're going to update */
  if (update_required && comp->priv->can_update)
    if ((curpos = get_current_position (comp)) == GST_CLOCK_TIME_NONE)
      curpos = comp->priv->segment_start;

  COMP_OBJECTS_UNLOCK (comp);

  /* If we added within currently configured segment OR the pipeline was *
   * previously empty, THEN update pipeline */
  if (G_LIKELY (update_required && comp->priv->can_update))
    update_pipeline (comp, curpos, TRUE, TRUE, TRUE);
  else {
    if (!comp->priv->can_update)
      comp->priv->update_required |= update_required;
    update_start_stop_duration (comp);
  }

beach:
  gst_object_unref (element);
  return ret;

chiringuito:
  COMP_OBJECTS_UNLOCK (comp);
  update_start_stop_duration (comp);
  goto beach;
}


static gboolean
gnl_composition_remove_object (GstBin * bin, GstElement * element)
{
  GnlComposition *comp = (GnlComposition *) bin;
  GstClockTime curpos = GST_CLOCK_TIME_NONE;
  gboolean ret = GST_STATE_CHANGE_FAILURE;
  gboolean update_required;

  GST_DEBUG_OBJECT (bin, "element %s", GST_OBJECT_NAME (element));
  /* we only accept GnlObject */
  g_return_val_if_fail (GNL_IS_OBJECT (element), FALSE);

  COMP_OBJECTS_LOCK (comp);

  gst_object_ref (element);

  gst_element_set_locked_state (element, FALSE);

  /* handle default source */
  if ((((GnlObject *) element)->priority == G_MAXUINT32) ||
      GNL_OBJECT_IS_EXPANDABLE (element)) {
    /* Find it in the list */
    comp->priv->expandables = g_list_remove (comp->priv->expandables, element);
  } else {
    /* remove it from the objects list and resort the lists */
    comp->priv->objects_start = g_list_remove
        (comp->priv->objects_start, element);

    comp->priv->objects_stop = g_list_remove
        (comp->priv->objects_stop, element);

    GST_LOG_OBJECT (element, "Removed from the objects start/stop list");
  }

  if (!(g_hash_table_remove (comp->priv->objects_hash, element)))
    goto chiringuito;

  update_required = OBJECT_IN_ACTIVE_SEGMENT (comp, element) ||
      (((GnlObject *) element)->priority == G_MAXUINT32) ||
      GNL_OBJECT_IS_EXPANDABLE (element);

  if (update_required && comp->priv->can_update) {
    curpos = get_current_position (comp);
    if (G_UNLIKELY (curpos == GST_CLOCK_TIME_NONE))
      curpos = comp->priv->segment_start;
  }

  COMP_OBJECTS_UNLOCK (comp);

  /* If we removed within currently configured segment, or it was the default source, *
   * update pipeline */
  if (G_LIKELY (update_required))
    update_pipeline (comp, curpos, TRUE, TRUE, TRUE);
  else {
    if (!comp->priv->can_update)
      comp->priv->update_required |= update_required;
    update_start_stop_duration (comp);
  }

  ret = GST_BIN_CLASS (parent_class)->remove_element (bin, element);

  GST_LOG_OBJECT (element, "Done removing from the composition");

beach:
  /* unblock source pad */
  if (1) {
    GstPad *pad = get_src_pad (element);

    if (pad) {
      gst_pad_set_blocked_async (pad, FALSE, (GstPadBlockCallback) pad_blocked,
          comp);
      gst_object_unref (pad);
    }
  }

  gst_object_unref (element);
  return ret;

chiringuito:
  COMP_OBJECTS_UNLOCK (comp);
  goto beach;
}
