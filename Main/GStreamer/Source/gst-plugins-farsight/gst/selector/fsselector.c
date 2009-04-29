/* GStreamer
 *
 *  Copyright 2007 Collabora Ltd
 *   @author: Olivier Crete <olivier.crete@collabora.co.uk>
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
 * SECTION:element-fsselector:
 *
 * The #fsselector element has one sink and one source pad. It will look for
 * other elements that also have one sink and one source pad.
 * It will then pick an element that matches the caps on both sides.
 * If the caps change, it may change the selected element if the current one
 * no longer matches the caps.
 *
 * The list of element it will look into can be specified in the
 * #FsSelector::factories property, otherwise it will look at all available
 * elements.
 */

#include "fsselector.h"

#include <string.h>


GST_DEBUG_CATEGORY (fsselector_debug);
#define GST_CAT_DEFAULT (fsselector_debug)

/* elementfactory information */
static const GstElementDetails fs_selector_details =
GST_ELEMENT_DETAILS (
  "Element selector",
  "Generic/Bin",
  "Selects the right transform element from a list",
  "Olivier Crete <olivier.crete@collabora.co.uk>");


static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate srctemplate = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);


static GstStaticPadTemplate sink_internal_template = GST_STATIC_PAD_TEMPLATE (
    "sink_internal",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate src_internal_template = GST_STATIC_PAD_TEMPLATE (
    "src_internal",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

/* FsSelector signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_FACTORIES,
};


static void fs_selector_set_property (GObject *object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void fs_selector_get_property (GObject *object,
    guint prop_id, GValue *value, GParamSpec *pspec);
static void fs_selector_dispose (GObject *object);

static GstElement *fs_selector_get_subelement (FsSelector *selector);
static GstPad *fs_selector_get_internal_sinkpad (FsSelector *selector);
static GstPad *fs_selector_get_internal_srcpad (FsSelector *selector);

static gboolean fs_selector_sink_setcaps (GstPad *pad, GstCaps *caps);
static GstCaps* fs_selector_sink_getcaps (GstPad *pad);
static GstFlowReturn fs_selector_sink_chain (GstPad *pad, GstBuffer *buffer);
static gboolean fs_selector_sink_event (GstPad *pad, GstEvent *event);
static gboolean fs_selector_sink_query (GstPad *pad, GstQuery *query);
static const GstQueryType* fs_selector_sink_query_type (GstPad *pad);
static GstFlowReturn fs_selector_sink_buffer_alloc (GstPad *pad, guint64 offset,
    guint size, GstCaps *caps, GstBuffer **buf);
static void fs_selector_sink_fixatecaps (GstPad *pad, GstCaps *caps);

static gboolean fs_selector_src_event (GstPad *pad, GstEvent *event);
static gboolean fs_selector_src_query (GstPad *pad, GstQuery *query);
static const GstQueryType* fs_selector_src_query_type (GstPad *pad);


static GstFlowReturn fs_selector_internal_sink_chain (GstPad *pad,
    GstBuffer *buffer);
static gboolean fs_selector_internal_sink_event (GstPad *pad, GstEvent *event);
static gboolean fs_selector_internal_sink_query (GstPad *pad, GstQuery *query);
static const GstQueryType* fs_selector_internal_sink_query_type (GstPad *pad);
static GstCaps* fs_selector_internal_sink_getcaps (GstPad *pad);
static GstFlowReturn fs_selector_internal_sink_buffer_alloc (GstPad *pad,
    guint64 offset, guint size, GstCaps *caps, GstBuffer **buf);
static void fs_selector_internal_sink_fixatecaps (GstPad *pad, GstCaps *caps);

static gboolean fs_selector_internal_src_event (GstPad *pad, GstEvent *event);
static gboolean fs_selector_internal_src_query (GstPad *pad, GstQuery *query);
static const GstQueryType* fs_selector_internal_src_query_type (GstPad *pad);


static void fs_selector_load_factories (FsSelector *selector);

GQuark internal_srcpad_quark = 0;
GQuark internal_sinkpad_quark = 0;
GQuark parent_quark = 0;

static void
fs_selector_do_init (GType type)
{
  GST_DEBUG_CATEGORY_INIT
    (fsselector_debug, "fsselector", 0, "RTP logger");
}

GST_BOILERPLATE_FULL (FsSelector, fs_selector, GstBin,
    GST_TYPE_BIN, fs_selector_do_init);

static void
fs_selector_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&srctemplate));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sinktemplate));

  gst_element_class_set_details (element_class, &fs_selector_details);
}

static void
fs_selector_class_init (FsSelectorClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBinClass *gstbin_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbin_class = (GstBinClass *) klass;

  gobject_class->dispose = GST_DEBUG_FUNCPTR (fs_selector_dispose);

  gobject_class->set_property = GST_DEBUG_FUNCPTR (fs_selector_set_property);
  gobject_class->get_property = GST_DEBUG_FUNCPTR (fs_selector_get_property);

  g_object_class_install_property (gobject_class, PROP_FACTORIES,
      g_param_spec_pointer ("factories",
          "GList of GstElementFactory",
          "GList of GstElementFactory objects to pick from (the element takes"
          " ownership of the list (NULL means it will go through all possible"
          " elements), can only be set once",
          G_PARAM_READWRITE));

  parent_class = g_type_class_peek_parent (klass);

  internal_srcpad_quark = g_quark_from_static_string ("internal_srcpad");
  internal_sinkpad_quark = g_quark_from_static_string ("internal_sinkpad");
  parent_quark = g_quark_from_static_string ("parent");
}

static void
fs_selector_init (FsSelector *fsselector,
    FsSelectorClass *klass)
{

  fsselector->sinkpad = gst_pad_new_from_static_template (&sinktemplate, "sink");
  fsselector->srcpad = gst_pad_new_from_static_template (&srctemplate, "src");

  gst_element_add_pad (GST_ELEMENT (fsselector), fsselector->sinkpad);
  gst_element_add_pad (GST_ELEMENT (fsselector), fsselector->srcpad);

  gst_pad_set_setcaps_function (fsselector->sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_sink_setcaps));
  gst_pad_set_getcaps_function (fsselector->sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_sink_getcaps));
  gst_pad_set_chain_function (fsselector->sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_sink_chain));
  gst_pad_set_event_function (fsselector->sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_sink_event));
  gst_pad_set_query_function (fsselector->sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_sink_query));
  gst_pad_set_query_type_function (fsselector->sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_sink_query_type));
  gst_pad_set_bufferalloc_function (fsselector->sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_sink_buffer_alloc));

  gst_pad_set_event_function (fsselector->srcpad,
      GST_DEBUG_FUNCPTR (fs_selector_src_event));
  gst_pad_set_query_function (fsselector->srcpad,
      GST_DEBUG_FUNCPTR (fs_selector_src_query));
  gst_pad_set_query_type_function (fsselector->srcpad,
      GST_DEBUG_FUNCPTR (fs_selector_src_query_type));
}

static void
fs_selector_dispose (GObject *object)
{
  FsSelector *selector = FS_SELECTOR (object);

  gst_pad_set_fixatecaps_function (selector->sinkpad, NULL);

  GST_OBJECT_LOCK (object);
  if (selector->current_subelement) {
    gst_object_unref (selector->current_subelement);
    selector->current_subelement = NULL;
    selector->current_internal_sinkpad = NULL;
    selector->current_internal_srcpad = NULL;
  }
  GST_OBJECT_UNLOCK (object);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}


static void
fs_selector_set_property (GObject *object,
    guint prop_id, const GValue *value, GParamSpec *pspec)
{
  FsSelector *fsselector = FS_SELECTOR (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    case PROP_FACTORIES:
      GST_OBJECT_LOCK (fsselector);
      if (fsselector->factories == NULL)
        fsselector->factories = g_value_get_pointer (value);
      else
        GST_WARNING_OBJECT (object, "Can not reset factories after they"
            " have been set or auto-discovered");
      GST_OBJECT_UNLOCK (fsselector);
      break;
  }
}

static void
fs_selector_get_property (GObject *object,
    guint prop_id, GValue *value, GParamSpec *pspec)
{
  FsSelector *fsselector = FS_SELECTOR (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    case PROP_FACTORIES:
      GST_OBJECT_LOCK (fsselector);
      g_value_set_pointer (value, &fsselector->factories);
      GST_OBJECT_UNLOCK (fsselector);
      break;
  }
}

static GstElement *
fs_selector_get_element_by_type (FsSelector *selector, GType type)
{
  GstIterator *iter = NULL;
  GstElement *elem = NULL;
  gboolean done;

  g_return_val_if_fail (type != 0, NULL);

  iter = gst_bin_iterate_elements (GST_BIN (selector));

  if (!iter)
    return NULL;

  done = FALSE;
  while (!done) {
    switch (gst_iterator_next (iter, (gpointer) &elem)) {
      case GST_ITERATOR_OK:
        if (G_OBJECT_TYPE (elem) == type)
          done = TRUE;
        else
          gst_object_unref (elem);
        break;
      case GST_ITERATOR_RESYNC:
        gst_iterator_resync (iter);
        elem = NULL;
        break;
      case GST_ITERATOR_ERROR:
        GST_ERROR ("Error iterating elements in bin");
        elem = NULL;
        done = TRUE;
        break;
      case GST_ITERATOR_DONE:
        elem = NULL;
        done = TRUE;
        break;
    }
  }
  gst_iterator_free (iter);

  return elem;
}

/**
 * get_pad_by_direction:
 * @element: The Element
 * @direction: The direction
 *
 * Gets a #GstPad that goes in the requested direction. I will return NULL
 * if there is no pad or if there is more than one pad in this direction
 */

static GstPad *
get_pad_by_direction (GstElement *element, GstPadDirection direction)
{
  GstIterator *iter = gst_element_iterate_pads (element);
  GstPad *pad = NULL;
  GstPad *selected_pad = NULL;
  gboolean done;

  if (!iter)
    return NULL;

  done = FALSE;
  while (!done) {
    switch (gst_iterator_next (iter, (gpointer) &pad)) {
      case GST_ITERATOR_OK:
        if (gst_pad_get_direction (pad) == direction)
        {
          /* We check if there is more than one pad in this direction,
           * if there is, we return NULL so that the element is refused
           */
          if (selected_pad) {
            done = TRUE;
            gst_object_unref (selected_pad);
            selected_pad = NULL;
          }
          else
          {
            selected_pad = pad;
          }
        }
        else
        {
          gst_object_unref (pad);
        }
        break;
      case GST_ITERATOR_RESYNC:
        if (selected_pad)
        {
          gst_object_unref (selected_pad);
          selected_pad = NULL;
        }
        gst_iterator_resync (iter);
        break;
      case GST_ITERATOR_ERROR:
        GST_ERROR ("Error iterating pads of element %s",
            GST_OBJECT_NAME (element));
        gst_object_unref (selected_pad);
        selected_pad = NULL;
        done = TRUE;
        break;
      case GST_ITERATOR_DONE:
        done = TRUE;
        break;
    }
  }
  gst_iterator_free (iter);

  if (!selected_pad)
    GST_ERROR ("Did not find pad of direction %d in %s",
               direction, GST_OBJECT_NAME (element));

  return selected_pad;
}

static GstElement *
fs_selector_get_subelement (FsSelector *selector)
{
  GstElement *element = NULL;

  GST_OBJECT_LOCK (selector);
  if (selector->current_subelement)
    element = gst_object_ref (selector->current_subelement);
  GST_OBJECT_UNLOCK (selector);

  return element;
}

static GstPad *
fs_selector_get_internal_sinkpad (FsSelector *selector)
{
  GstPad *pad = NULL;

  GST_OBJECT_LOCK (selector);
  if (selector->current_internal_sinkpad)
    pad = gst_object_ref (selector->current_internal_sinkpad);
  GST_OBJECT_UNLOCK (selector);

  return pad;
}


static GstPad *
fs_selector_get_internal_srcpad (FsSelector *selector)
{
  GstPad *pad = NULL;

  GST_OBJECT_LOCK (selector);
  if (selector->current_internal_srcpad)
    pad = gst_object_ref (selector->current_internal_srcpad);
  GST_OBJECT_UNLOCK (selector);

  return pad;
}

/*
 * This function creates and adds an element to the FsSelector
 * it then creates the internal pads and links them
 *
 */

static GstElement *
fs_selector_add_element (FsSelector *selector, GstElementFactory *factory)
{
  GstElement *element = NULL;
  GstPad *internal_sinkpad = NULL;
  GstPad *internal_srcpad = NULL;
  GstPad *sinkpad;
  GstPad *srcpad;
  GstPadLinkReturn padlinkret;

  GST_DEBUG_OBJECT (selector, "Adding element %s to the selector bin",
      gst_plugin_feature_get_name (GST_PLUGIN_FEATURE (factory)));

  element = gst_element_factory_create (factory, NULL);
  if (!element)
    return NULL;

  if (!gst_bin_add (GST_BIN (selector), element)) {
    GST_ERROR_OBJECT (selector, "Could not add element %s to the bin",
        GST_OBJECT_NAME (element));
    gst_object_unref (element);
    return NULL;
  }

  gst_object_ref (element);

  srcpad = get_pad_by_direction (element, GST_PAD_SRC);
  if (!srcpad) {
    GST_ERROR_OBJECT (selector, "Could not find source in %s",
        GST_OBJECT_NAME (element));
    goto error;
  }

  sinkpad = get_pad_by_direction (element, GST_PAD_SINK);
  if (!sinkpad) {
    GST_ERROR_OBJECT (selector, "Could not find sink in %s",
        GST_OBJECT_NAME (element));
    goto error;
  }

  internal_sinkpad =
      gst_pad_new_from_static_template (&sink_internal_template,
          "sink_internal");
  internal_srcpad =
      gst_pad_new_from_static_template (&src_internal_template,
          "src_internal");

  if (!internal_sinkpad || !internal_srcpad) {
    GST_ERROR_OBJECT (selector, "Could not create internal pads");
    goto error;
  }

  g_object_weak_ref (G_OBJECT (element), (GWeakNotify) gst_object_unref,
      internal_sinkpad);
  g_object_weak_ref (G_OBJECT (element), (GWeakNotify) gst_object_unref,
      internal_srcpad);

  gst_pad_set_active (internal_sinkpad, TRUE);
  gst_pad_set_active (internal_srcpad, TRUE);

  g_object_set_qdata (G_OBJECT (internal_srcpad), parent_quark,
      selector);
  g_object_set_qdata (G_OBJECT (internal_sinkpad), parent_quark,
      selector);

  gst_pad_set_chain_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_internal_sink_chain));
  gst_pad_set_event_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_internal_sink_event));
  gst_pad_set_query_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_internal_sink_query));
  gst_pad_set_query_type_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_internal_sink_query_type));
  gst_pad_set_getcaps_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_internal_sink_getcaps));
  gst_pad_set_bufferalloc_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_internal_sink_buffer_alloc));
  gst_pad_set_fixatecaps_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (fs_selector_internal_sink_fixatecaps));

  gst_pad_set_event_function (internal_srcpad,
      GST_DEBUG_FUNCPTR (fs_selector_internal_src_event));
  gst_pad_set_query_function (internal_srcpad,
      GST_DEBUG_FUNCPTR (fs_selector_internal_src_query));
  gst_pad_set_query_type_function (internal_srcpad,
      GST_DEBUG_FUNCPTR (fs_selector_internal_src_query_type));

  padlinkret = gst_pad_link (internal_srcpad, sinkpad);
  if (GST_PAD_LINK_FAILED (padlinkret)) {
    GST_WARNING_OBJECT (selector, "Could not links pad %s:%s to %s:%s"
        " for reason %d",
        GST_DEBUG_PAD_NAME (internal_srcpad),
        GST_DEBUG_PAD_NAME (sinkpad),
        padlinkret);
    goto error;
  }

  padlinkret = gst_pad_link (srcpad, internal_sinkpad);
  if (GST_PAD_LINK_FAILED (padlinkret)) {
    GST_WARNING_OBJECT (selector, "Could not links pad %s:%s to %s:%s"
        " for reason %d",
        GST_DEBUG_PAD_NAME (internal_srcpad),
        GST_DEBUG_PAD_NAME (sinkpad),
        padlinkret);
    goto error;
  }

  g_object_set_qdata (G_OBJECT (element),
      internal_srcpad_quark, internal_srcpad);
  g_object_set_qdata (G_OBJECT (element),
      internal_sinkpad_quark, internal_sinkpad);

  /* Iffy */
  gst_element_sync_state_with_parent (element);

  return element;

 error:
  gst_bin_remove (GST_BIN (selector), element);
  gst_object_unref (element);

  return NULL;
}

static GstElement *
fs_selector_get_or_make_element_from_factory (FsSelector *selector,
    GstElementFactory *factory)
{
  GstElement *element = NULL;
  GstElementFactory *loaded_factory =
      GST_ELEMENT_FACTORY (gst_plugin_feature_load (
              GST_PLUGIN_FEATURE (factory)));

  if (!loaded_factory)
    return NULL;

  element = fs_selector_get_element_by_type (selector,
      gst_element_factory_get_element_type (loaded_factory));

  if (!element) {
    element = fs_selector_add_element (selector, loaded_factory);
  }

  gst_object_unref (loaded_factory);

  return element;
}

/*
 * This function checks if there is one and only one pad template on the
 * factory that can accept the given caps. If there is one and only one,
 * it returns TRUE, otherwise, its FALSE
 */

static gboolean
factory_can_intersect (GstElementFactory *factory, GstPadDirection direction,
    GstCaps *caps)
{
  GList *templates;
  gint has_direction = FALSE;
  gboolean ret = FALSE;

  g_return_val_if_fail (factory != NULL, FALSE);
  g_return_val_if_fail (caps != NULL, FALSE);

  templates = factory->staticpadtemplates;

  while (templates) {
    GstStaticPadTemplate *template = (GstStaticPadTemplate *) templates->data;

    if (template->direction == direction) {
      GstCaps *intersect = NULL;

      /* If there is more than one pad in this direction, we return FALSE
       * Only transform elements (with one sink and one source pad)
       * are accepted
       */
      if (has_direction)
        return FALSE;
      has_direction = TRUE;

      intersect = gst_caps_intersect (
          gst_static_caps_get (&template->static_caps), caps);

      if (intersect) {
        if (!gst_caps_is_empty (intersect))
          ret = TRUE;

        gst_caps_unref (intersect);
      }
    }
    templates = g_list_next (templates);
  }

  return ret;
}

/*
 * If there is already an internal element, it will try to call set_caps on it
 *
 * If there isn't an internal element or if the set_caps() on the internal
 * element failed, it will try to find another element where it would succeed
 * and will change the internal element.
 */

static gboolean
fs_selector_sink_setcaps (GstPad *pad, GstCaps *caps)
{
  GList *elem;
  FsSelector *selector = FS_SELECTOR (gst_pad_get_parent (pad));
  GstElement *subelement;
  GstCaps *other_caps = NULL;
  GstPad *peer;
  GList *factories;

  g_return_val_if_fail (selector != NULL, FALSE);

  subelement = fs_selector_get_subelement (selector);
  if (subelement) {
    if (gst_pad_set_caps (selector->current_internal_srcpad, caps)) {
      /* If we can set the new caps on the current element,
       * then we just get out
       */
      GST_DEBUG_OBJECT (selector, "Could set %s:%s to %" GST_PTR_FORMAT,
          GST_DEBUG_PAD_NAME (selector->current_internal_srcpad), caps);
      gst_object_unref (subelement);
      goto get_out;
    } else {
      /* If the current element doesn't work,
       * then we remove the current element before finding a new one.
       * By unsetting the fixatecaps function, we go back to the default one
       */
      gst_pad_set_fixatecaps_function (selector->sinkpad, NULL);
      GST_OBJECT_LOCK (selector);
      if (selector->current_subelement == subelement) {
        gst_object_unref (selector->current_subelement);
        selector->current_subelement = NULL;
        selector->current_internal_srcpad = NULL;
        selector->current_internal_sinkpad = NULL;
      }
      GST_OBJECT_UNLOCK (selector);
      gst_object_unref (subelement);
    }
  }

  peer = gst_pad_get_peer (selector->srcpad);
  if (peer) {
    other_caps = gst_pad_get_caps (peer);
    gst_object_unref (peer);
  }

  GST_OBJECT_LOCK (selector);
  factories = selector->factories;
  GST_OBJECT_UNLOCK (selector);

  if (!factories) {
    fs_selector_load_factories (selector);

    GST_OBJECT_LOCK (selector);
    factories = selector->factories;
    GST_OBJECT_UNLOCK (selector);
  }

  for (elem = factories; elem; elem = g_list_next (elem)) {
    GstElementFactory *factory = GST_ELEMENT_FACTORY (elem->data);
    GstElement *element;
    GstPad *internal_srcpad = NULL;
    GstPad *internal_sinkpad = NULL;

    /* Lets first check if according to the static pad templates on the factory
     * these caps have any chance of success
     */
    if (!factory_can_intersect (factory, GST_PAD_SINK, caps)) {
      GST_LOG_OBJECT (selector, "Factory %s does not accept sink caps %"
          GST_PTR_FORMAT,
          gst_plugin_feature_get_name (GST_PLUGIN_FEATURE (factory)), caps);
      continue;
    }
    if (other_caps != NULL) {
      if (!factory_can_intersect (factory, GST_PAD_SRC, other_caps)) {
      GST_LOG_OBJECT (selector, "Factory %s does not accept src caps %"
          GST_PTR_FORMAT,
          gst_plugin_feature_get_name (GST_PLUGIN_FEATURE (factory)), other_caps);
        continue;
      }
    }

    /* The element had a chance of success, lets make it */

    element = fs_selector_get_or_make_element_from_factory (selector, factory);

    if (!element) {
      continue;
    }

    internal_srcpad = g_object_get_qdata (G_OBJECT (element),
        internal_srcpad_quark);
    internal_sinkpad = g_object_get_qdata (G_OBJECT (element),
        internal_sinkpad_quark);

    /* Now we check if the element can really accept said caps */
    if (!gst_pad_set_caps (internal_srcpad, caps)) {
      GST_DEBUG_OBJECT (selector, "Could not set %s:%s to %" GST_PTR_FORMAT,
          GST_DEBUG_PAD_NAME (internal_srcpad), caps);
      goto next_element;
    }

    gst_pad_set_fixatecaps_function (selector->sinkpad,
        fs_selector_sink_fixatecaps);
    GST_OBJECT_LOCK (selector);
    selector->current_subelement = element;
    selector->current_internal_srcpad = internal_srcpad;
    selector->current_internal_sinkpad = internal_sinkpad;
    GST_OBJECT_UNLOCK (selector);

    GST_INFO_OBJECT (selector,
        "Selected element %s",
        GST_OBJECT_NAME (GST_OBJECT (selector->current_subelement)));

    break;

  next_element:
    continue;
  }

get_out:
  if (other_caps)
    gst_caps_unref (other_caps);
  gst_object_unref (selector);

  if (selector->current_subelement) {
    return TRUE;
  } else {
    GST_WARNING_OBJECT (selector, "Could not find a matching element for caps");
    return FALSE;
  }
}

/*
 * This function filters the pad pad templates, taking only transform element
 * (with one sink and one src pad)
 */

static gboolean
fs_selector_default_filter_func (GstPluginFeature *feature, gpointer user_data)
{
  GstElementFactory *factory = NULL;
  const GList *static_pad_templates, *tmp;
  GstStaticPadTemplate *src = NULL, *sink = NULL;

  if (!GST_IS_ELEMENT_FACTORY (feature))
    return FALSE;

  factory = GST_ELEMENT_FACTORY (feature);

  static_pad_templates = gst_element_factory_get_static_pad_templates (factory);

  for (tmp = static_pad_templates; tmp; tmp = g_list_next (tmp)) {
    GstStaticPadTemplate *template = tmp->data;
    GstCaps *caps;

    if (template->presence == GST_PAD_SOMETIMES)
      return FALSE;

    if (template->presence != GST_PAD_ALWAYS)
      continue;

    switch (template->direction) {
      case GST_PAD_SRC:
        if (src)
          return FALSE;
        src = template;
        break;
      case GST_PAD_SINK:
        if (sink)
          return FALSE;
        sink = template;
        break;
      default:
        return FALSE;
    }

    caps = gst_static_pad_template_get_caps (template);

    if (gst_caps_is_any (caps) || gst_caps_is_empty (caps))
      return FALSE;
  }

  if (!src || !sink)
    return FALSE;

  return TRUE;
}

/* function used to sort element features
 * Copy-pasted from decodebin */
static gint
compare_ranks (GstPluginFeature * f1, GstPluginFeature * f2)
{
  gint diff;
  const gchar *rname1, *rname2;

  diff =  gst_plugin_feature_get_rank (f2) - gst_plugin_feature_get_rank (f1);
  if (diff != 0)
    return diff;

  rname1 = gst_plugin_feature_get_name (f1);
  rname2 = gst_plugin_feature_get_name (f2);

  diff = strcmp (rname2, rname1);

  return diff;
}

static void
fs_selector_load_factories (FsSelector *selector)
{
  GList *all_factories;

  all_factories = gst_default_registry_feature_filter (
      fs_selector_default_filter_func, FALSE, NULL);

  all_factories = g_list_sort (all_factories, (GCompareFunc) compare_ranks);

  g_assert (all_factories);

  GST_OBJECT_LOCK (selector);
  if (selector->factories == NULL) {
    selector->factories = all_factories;
    all_factories = NULL;
  }
  GST_OBJECT_UNLOCK (selector);

  if (all_factories) {
    /* In this case, someone set the property while we were looking! */
    gst_plugin_feature_list_free (all_factories);
  }
}

/* In this case, we should almost always have an internal element, because
 * set_caps() should have been called first
 */

static GstFlowReturn
fs_selector_sink_chain (GstPad *pad, GstBuffer *buffer)
{
  GstFlowReturn ret = GST_FLOW_NOT_NEGOTIATED;
  FsSelector *selector = FS_SELECTOR (gst_pad_get_parent (pad));
  GstPad *internal_srcpad;

  internal_srcpad = fs_selector_get_internal_srcpad (selector);
  if (internal_srcpad) {
    ret = gst_pad_push (internal_srcpad, buffer);
    gst_object_unref (internal_srcpad);
  } else {
    GST_ERROR_OBJECT (selector, "Got buffer without an negotiated element,"
                      " returning not-negotiated");
  }

  gst_object_unref (selector);

  return ret;
}

static gboolean
fs_selector_sink_event (GstPad *pad, GstEvent *event)
{
  gboolean ret = TRUE;
  FsSelector *selector = FS_SELECTOR (gst_pad_get_parent (pad));
  GstPad *internal_srcpad;

  internal_srcpad = fs_selector_get_internal_srcpad (selector);
  if (internal_srcpad) {
    ret = gst_pad_push_event (internal_srcpad, event);
    gst_object_unref (internal_srcpad);
  } else {
    GST_WARNING_OBJECT (selector, "Got event while no element was selected,"
        "letting through");
    ret = gst_pad_push_event (selector->srcpad, event);
  }

  gst_object_unref (selector);

  return ret;
}

/* TODO Properly test that this code works well for queries */
static gboolean
fs_selector_sink_query (GstPad *pad, GstQuery *query)
{
  gboolean ret = TRUE;
  FsSelector *selector = FS_SELECTOR (gst_pad_get_parent (pad));
  GstElement *subelement;

  subelement = fs_selector_get_subelement (selector);
  if (subelement) {
    GstPad *sub_sinkpad = get_pad_by_direction (subelement, GST_PAD_SINK);

    ret = gst_pad_query (sub_sinkpad, query);

    gst_object_unref (sub_sinkpad);
    gst_object_unref (subelement);
  }
  else {
    GST_WARNING_OBJECT (selector, "Got query while no element was selected,"
        "letting through");
    ret = gst_pad_query_default (pad, query);
  }

  gst_object_unref (selector);

  return ret;
}

/* TODO Test that this code works properly for queries */
static const GstQueryType*
fs_selector_sink_query_type (GstPad *pad)
{
  const GstQueryType *ret = NULL;
  FsSelector *selector = FS_SELECTOR (gst_pad_get_parent (pad));
  GstElement *subelement;

  subelement = fs_selector_get_subelement (selector);
  if (subelement) {
    GstPad *sub_sinkpad = get_pad_by_direction (subelement, GST_PAD_SINK);

    ret = gst_pad_get_query_types (sub_sinkpad);

    gst_object_unref (sub_sinkpad);
    gst_object_unref (subelement);
  }
  else {
    ret = gst_pad_get_query_types_default (pad);
  }

  gst_object_unref (selector);

  return ret;
}

static void
fs_selector_sink_fixatecaps (GstPad *pad, GstCaps *caps)
{
  FsSelector *selector = FS_SELECTOR (gst_pad_get_parent (pad));
  GstElement *subelement;

  subelement = fs_selector_get_subelement (selector);
  if (subelement) {
    GstPad *sinkpad = get_pad_by_direction (subelement, GST_PAD_SINK);
    gst_pad_fixate_caps (sinkpad, caps);
    gst_object_unref (sinkpad);
    gst_object_unref (subelement);
  }
}

/**
 * fs_selector_sink_getcaps:
 * @pad: the sink #GstPad
 *
 * This function returns the union of the caps of all the possible element
 * factories, based on the static pad templates.
 * It also checks does a getcaps on the downstream element and ignores all
 * factories whose static caps can not satisfy it.
 *
 * It does not try to use each elements getcaps() function
 */

static GstCaps*
fs_selector_sink_getcaps (GstPad *pad)
{
  GstCaps *caps = NULL, *other_caps = NULL;
  FsSelector *selector = FS_SELECTOR (gst_pad_get_parent (pad));
  GstPad *peer;
  GList *elem, *factories;

  caps = gst_caps_new_empty ();

  peer = gst_pad_get_peer (selector->srcpad);
  if (peer) {
    other_caps = gst_pad_get_caps (peer);
    gst_object_unref (peer);
  }

  GST_DEBUG_OBJECT (selector,
      "Lets find all the element that can fit here with src caps %"
      GST_PTR_FORMAT, other_caps);

  if (other_caps && gst_caps_is_empty (other_caps)) {
    goto out;
  }

  GST_OBJECT_LOCK (selector);
  factories = selector->factories;
  GST_OBJECT_UNLOCK (selector);

  if (!factories) {
    fs_selector_load_factories (selector);

    GST_OBJECT_LOCK (selector);
    factories = selector->factories;
    GST_OBJECT_UNLOCK (selector);
  }

  for (elem = factories; elem; elem = g_list_next (elem)) {
    GstElementFactory *factory = GST_ELEMENT_FACTORY (elem->data);
    GstElement *element = NULL;
    GstCaps *element_caps;
    GstPad *internal_srcpad = NULL;

    if (other_caps != NULL) {
      if (!factory_can_intersect (factory, GST_PAD_SRC, other_caps)) {
      GST_LOG_OBJECT (selector, "Factory %s does not accept src caps %"
          GST_PTR_FORMAT,
          gst_plugin_feature_get_name (GST_PLUGIN_FEATURE (factory)), other_caps);
        continue;
      }
    }

    if (other_caps) {

      element = fs_selector_get_or_make_element_from_factory (selector, factory);

      if (!element) {
        continue;
      }

      internal_srcpad = g_object_get_qdata (G_OBJECT (element),
          internal_srcpad_quark);

      element_caps = gst_pad_peer_get_caps (internal_srcpad);

      if (element_caps) {
        if (!gst_caps_is_any (element_caps) &&
            !gst_caps_is_empty (element_caps)) {
          GstCaps *tmpcaps = NULL;

          tmpcaps = gst_caps_union (caps, element_caps);
          gst_caps_unref (caps);
          caps = tmpcaps;

        }
        gst_caps_unref (element_caps);
      }

      gst_object_unref (element);
    } else {
      const GList *tmp;

      for (tmp = gst_element_factory_get_static_pad_templates (factory);
           tmp;
           tmp = g_list_next (tmp)) {
        GstStaticPadTemplate *template = tmp->data;
        GstCaps *static_caps = gst_static_pad_template_get_caps (template);

        if (static_caps && !gst_caps_is_any (static_caps) &&
            !gst_caps_is_empty (static_caps)) {
          GstCaps *tmpcaps = NULL;

          tmpcaps = gst_caps_union (caps, static_caps);
          gst_caps_unref (caps);
          caps = tmpcaps;
        }
      }
    }
  }

  GST_DEBUG_OBJECT (selector, "Returning unioned caps %" GST_PTR_FORMAT, caps);

out:
  gst_object_unref (selector);

  if (other_caps)
    gst_caps_unref (other_caps);

  return caps;
}


static GstFlowReturn
fs_selector_sink_buffer_alloc (GstPad *pad, guint64 offset,
    guint size, GstCaps *caps, GstBuffer **buf)
{
  GstFlowReturn ret = GST_FLOW_OK;
  FsSelector *selector = FS_SELECTOR (gst_pad_get_parent (pad));
  GstPad *internal_srcpad;

  g_return_val_if_fail (selector != NULL,  GST_FLOW_ERROR);

  internal_srcpad = fs_selector_get_internal_srcpad (selector);
  if (internal_srcpad) {
    ret = gst_pad_alloc_buffer (internal_srcpad, offset, size, caps, buf);
    gst_object_unref (internal_srcpad);
  } else
    /* Fallback to the default */
    *buf = NULL;

  gst_object_unref (selector);

  return ret;
}

static gboolean
fs_selector_src_event (GstPad *pad, GstEvent *event)
{
  gboolean ret = TRUE;
  FsSelector *selector = FS_SELECTOR (gst_pad_get_parent (pad));
  GstPad *internal_sinkpad;

  internal_sinkpad = fs_selector_get_internal_sinkpad (selector);
  if (internal_sinkpad) {
    ret = gst_pad_push_event (internal_sinkpad, event);
    gst_object_unref (internal_sinkpad);
  } else {
    GST_WARNING_OBJECT (selector, "Got event while not element was selected,"
        "letting through");
    ret = gst_pad_push_event (selector->sinkpad, event);
  }

  gst_object_unref (selector);

  return ret;
}

/* TODO Properly test that this code works well for queries */
static gboolean
fs_selector_src_query (GstPad *pad, GstQuery *query)
{
  gboolean ret = TRUE;
  FsSelector *selector = FS_SELECTOR (gst_pad_get_parent (pad));
  GstElement *subelement;

  subelement = fs_selector_get_subelement (selector);
  if (subelement) {
    GstPad *sub_srcpad = get_pad_by_direction (subelement, GST_PAD_SRC);

    ret = gst_pad_query (sub_srcpad, query);

    gst_object_unref (sub_srcpad);
    gst_object_unref (subelement);
  }
  else {
    GST_WARNING_OBJECT (selector, "Got query while not element was selected,"
        "letting through");
    ret = gst_pad_query_default (pad, query);
  }

  gst_object_unref (selector);

  return ret;
}

/* TODO Properly test that this code works well for queries */
static const GstQueryType*
fs_selector_src_query_type (GstPad *pad)
{
  const GstQueryType *ret = NULL;
  FsSelector *selector = FS_SELECTOR (gst_pad_get_parent (pad));
  GstElement *subelement;

  subelement = fs_selector_get_subelement (selector);
  if (subelement) {
    GstPad *sub_srcpad = get_pad_by_direction (subelement, GST_PAD_SRC);

    ret = gst_pad_get_query_types (sub_srcpad);

    gst_object_unref (sub_srcpad);
    gst_object_unref (subelement);
  }
  else {
    ret = gst_pad_get_query_types_default (pad);
  }

  gst_object_unref (selector);

  return ret;
}

static GstFlowReturn
fs_selector_internal_sink_chain (GstPad *pad, GstBuffer *buffer)
{
  FsSelector *selector = FS_SELECTOR (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));

  return gst_pad_push (selector->srcpad, buffer);
}

static gboolean
fs_selector_internal_sink_event (GstPad *pad, GstEvent *event)
{
  FsSelector *selector = FS_SELECTOR (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));

  return gst_pad_push_event (selector->srcpad, event);
}

static gboolean
fs_selector_internal_sink_query (GstPad *pad, GstQuery *query)
{
  FsSelector *selector = FS_SELECTOR (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));
  GstPad *peerpad = gst_pad_get_peer (selector->srcpad);
  gboolean ret = FALSE;

  if (peerpad) {
    ret = gst_pad_query (peerpad, query);
    gst_object_unref (peerpad);
  }

  return ret;
}

static const GstQueryType *
fs_selector_internal_sink_query_type (GstPad *pad)
{
  FsSelector *selector = FS_SELECTOR (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));
  GstPad *peerpad = gst_pad_get_peer (selector->srcpad);
  const GstQueryType *ret = NULL;

  if (peerpad) {
    ret = gst_pad_get_query_types (peerpad);
    gst_object_unref (peerpad);
  } else
    ret = gst_pad_get_query_types_default (pad);

  return ret;
}

static GstCaps*
fs_selector_internal_sink_getcaps (GstPad *pad)
{
  FsSelector *selector = FS_SELECTOR (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));

  return gst_pad_peer_get_caps (selector->srcpad);
}

static void
fs_selector_internal_sink_fixatecaps (GstPad *pad, GstCaps *caps)
{
  FsSelector *selector = FS_SELECTOR (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));
  GstPad *peerpad = gst_pad_get_peer (selector->sinkpad);

  if (peerpad) {
    gst_pad_fixate_caps (peerpad, caps);
    gst_object_unref (peerpad);
  }
}

static GstFlowReturn
fs_selector_internal_sink_buffer_alloc (GstPad *pad, guint64 offset,
    guint size, GstCaps *caps, GstBuffer **buf)
{
  FsSelector *selector = FS_SELECTOR (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));

  return gst_pad_alloc_buffer (selector->srcpad, offset, size, caps, buf);
}

static gboolean
fs_selector_internal_src_event (GstPad *pad, GstEvent *event)
{
  FsSelector *selector = FS_SELECTOR (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));

  return gst_pad_push_event (selector->sinkpad, event);
}

static gboolean
fs_selector_internal_src_query (GstPad *pad, GstQuery *query)
{
  FsSelector *selector = FS_SELECTOR (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));
  GstPad *peerpad = gst_pad_get_peer (selector->sinkpad);
  gboolean ret = FALSE;

  if (peerpad) {
    ret = gst_pad_query (peerpad, query);
    gst_object_unref (peerpad);
  }

  return ret;
}

static const GstQueryType *
fs_selector_internal_src_query_type (GstPad *pad)
{
  FsSelector *selector = FS_SELECTOR (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));
  GstPad *peerpad = gst_pad_get_peer (selector->sinkpad);
  const GstQueryType* ret = NULL;

  if (peerpad) {
    ret = gst_pad_get_query_types (peerpad);
    gst_object_unref (peerpad);
  } else {
    ret = gst_pad_get_query_types_default (pad);
  }

  return ret;
}

gboolean
fs_selector_plugin_init (GstPlugin *plugin)
{
  return gst_element_register (plugin, "fsselector",
      GST_RANK_NONE, FS_TYPE_SELECTOR);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "fsselector",
    "Element selector",
    fs_selector_plugin_init, VERSION, "LGPL", "Farsight",
    "http://farsight.freedesktop.org")
