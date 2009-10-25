/* GStreamer
 *
 *  Copyright 2007-2008 Collabora Ltd
 *   @author: Olivier Crete <olivier.crete@collabora.co.uk>
 *  Copyright 2007-2008 Nokia
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
 * SECTION:element-autoconvert
 *
 * The #autoconvert element has one sink and one source pad. It will look for
 * other elements that also have one sink and one source pad.
 * It will then pick an element that matches the caps on both sides.
 * If the caps change, it may change the selected element if the current one
 * no longer matches the caps.
 *
 * The list of element it will look into can be specified in the
 * #GstAutoConvert::factories property, otherwise it will look at all available
 * elements.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstautoconvert.h"

#include <string.h>

GST_DEBUG_CATEGORY (autoconvert_debug);
#define GST_CAT_DEFAULT (autoconvert_debug)

/* elementfactory information */
static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate srctemplate = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate sink_internal_template =
GST_STATIC_PAD_TEMPLATE ("sink_internal",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate src_internal_template =
GST_STATIC_PAD_TEMPLATE ("src_internal",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

/* GstAutoConvert signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_FACTORIES
};

static void gst_auto_convert_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_auto_convert_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);
static void gst_auto_convert_dispose (GObject * object);

static GstStateChangeReturn gst_auto_convert_change_state (GstElement * element,
    GstStateChange transition);

static GstElement *gst_auto_convert_get_subelement (GstAutoConvert *
    autoconvert);
static GstPad *gst_auto_convert_get_internal_sinkpad (GstAutoConvert *
    autoconvert);
static GstPad *gst_auto_convert_get_internal_srcpad (GstAutoConvert *
    autoconvert);

static gboolean gst_auto_convert_sink_setcaps (GstPad * pad, GstCaps * caps);
static GstCaps *gst_auto_convert_sink_getcaps (GstPad * pad);
static GstFlowReturn gst_auto_convert_sink_chain (GstPad * pad,
    GstBuffer * buffer);
static gboolean gst_auto_convert_sink_event (GstPad * pad, GstEvent * event);
static gboolean gst_auto_convert_sink_query (GstPad * pad, GstQuery * query);
static const GstQueryType *gst_auto_convert_sink_query_type (GstPad * pad);
static GstFlowReturn gst_auto_convert_sink_buffer_alloc (GstPad * pad,
    guint64 offset, guint size, GstCaps * caps, GstBuffer ** buf);
static void gst_auto_convert_sink_fixatecaps (GstPad * pad, GstCaps * caps);

static gboolean gst_auto_convert_src_event (GstPad * pad, GstEvent * event);
static gboolean gst_auto_convert_src_query (GstPad * pad, GstQuery * query);
static const GstQueryType *gst_auto_convert_src_query_type (GstPad * pad);

static GstFlowReturn gst_auto_convert_internal_sink_chain (GstPad * pad,
    GstBuffer * buffer);
static gboolean gst_auto_convert_internal_sink_event (GstPad * pad,
    GstEvent * event);
static gboolean gst_auto_convert_internal_sink_query (GstPad * pad,
    GstQuery * query);
static const GstQueryType *gst_auto_convert_internal_sink_query_type (GstPad *
    pad);
static GstCaps *gst_auto_convert_internal_sink_getcaps (GstPad * pad);
static GstFlowReturn gst_auto_convert_internal_sink_buffer_alloc (GstPad * pad,
    guint64 offset, guint size, GstCaps * caps, GstBuffer ** buf);
static void gst_auto_convert_internal_sink_fixatecaps (GstPad * pad,
    GstCaps * caps);

static gboolean gst_auto_convert_internal_src_event (GstPad * pad,
    GstEvent * event);
static gboolean gst_auto_convert_internal_src_query (GstPad * pad,
    GstQuery * query);
static const GstQueryType *gst_auto_convert_internal_src_query_type (GstPad *
    pad);

static GList *gst_auto_convert_load_factories (GstAutoConvert * autoconvert);

static GQuark internal_srcpad_quark = 0;
static GQuark internal_sinkpad_quark = 0;
static GQuark parent_quark = 0;

static void
gst_auto_convert_do_init (GType type)
{
  GST_DEBUG_CATEGORY_INIT (autoconvert_debug, "autoconvert", 0,
      "Auto convert based on caps");

  internal_srcpad_quark = g_quark_from_static_string ("internal_srcpad");
  internal_sinkpad_quark = g_quark_from_static_string ("internal_sinkpad");
  parent_quark = g_quark_from_static_string ("parent");
}

GST_BOILERPLATE_FULL (GstAutoConvert, gst_auto_convert, GstBin,
    GST_TYPE_BIN, gst_auto_convert_do_init);

static void
gst_auto_convert_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&srctemplate));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sinktemplate));

  gst_element_class_set_details_simple (element_class,
      "Select convertor based on caps", "Generic/Bin",
      "Selects the right transform element based on the caps",
      "Olivier Crete <olivier.crete@collabora.co.uk>");
}

static void
gst_auto_convert_class_init (GstAutoConvertClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GstElementClass *gstelement_class = (GstElementClass *) klass;

  gobject_class->dispose = gst_auto_convert_dispose;

  gobject_class->set_property = gst_auto_convert_set_property;
  gobject_class->get_property = gst_auto_convert_get_property;

  g_object_class_install_property (gobject_class, PROP_FACTORIES,
      g_param_spec_pointer ("factories",
          "GList of GstElementFactory",
          "GList of GstElementFactory objects to pick from (the element takes"
          " ownership of the list (NULL means it will go through all possible"
          " elements), can only be set once",
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gstelement_class->change_state =
      GST_DEBUG_FUNCPTR (gst_auto_convert_change_state);
}

static void
gst_auto_convert_init (GstAutoConvert * autoconvert,
    GstAutoConvertClass * klass)
{
  autoconvert->sinkpad =
      gst_pad_new_from_static_template (&sinktemplate, "sink");
  autoconvert->srcpad = gst_pad_new_from_static_template (&srctemplate, "src");

  gst_pad_set_setcaps_function (autoconvert->sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_sink_setcaps));
  gst_pad_set_getcaps_function (autoconvert->sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_sink_getcaps));
  gst_pad_set_chain_function (autoconvert->sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_sink_chain));
  gst_pad_set_event_function (autoconvert->sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_sink_event));
  gst_pad_set_query_function (autoconvert->sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_sink_query));
  gst_pad_set_query_type_function (autoconvert->sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_sink_query_type));
  gst_pad_set_bufferalloc_function (autoconvert->sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_sink_buffer_alloc));

  gst_pad_set_event_function (autoconvert->srcpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_src_event));
  gst_pad_set_query_function (autoconvert->srcpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_src_query));
  gst_pad_set_query_type_function (autoconvert->srcpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_src_query_type));

  gst_element_add_pad (GST_ELEMENT (autoconvert), autoconvert->sinkpad);
  gst_element_add_pad (GST_ELEMENT (autoconvert), autoconvert->srcpad);
}

static void
gst_auto_convert_dispose (GObject * object)
{
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (object);

  gst_pad_set_fixatecaps_function (autoconvert->sinkpad, NULL);

  GST_OBJECT_LOCK (object);
  if (autoconvert->current_subelement) {
    gst_object_unref (autoconvert->current_subelement);
    autoconvert->current_subelement = NULL;
    autoconvert->current_internal_sinkpad = NULL;
    autoconvert->current_internal_srcpad = NULL;
  }

  g_list_foreach (autoconvert->cached_events, (GFunc) gst_mini_object_unref,
      NULL);
  g_list_free (autoconvert->cached_events);
  autoconvert->cached_events = NULL;
  GST_OBJECT_UNLOCK (object);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}


static void
gst_auto_convert_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    case PROP_FACTORIES:
      GST_OBJECT_LOCK (autoconvert);
      if (autoconvert->factories == NULL)
        autoconvert->factories = g_value_get_pointer (value);
      else
        GST_WARNING_OBJECT (object, "Can not reset factories after they"
            " have been set or auto-discovered");
      GST_OBJECT_UNLOCK (autoconvert);
      break;
  }
}

static void
gst_auto_convert_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    case PROP_FACTORIES:
      GST_OBJECT_LOCK (autoconvert);
      g_value_set_pointer (value, &autoconvert->factories);
      GST_OBJECT_UNLOCK (autoconvert);
      break;
  }
}

static GstStateChangeReturn
gst_auto_convert_change_state (GstElement * element, GstStateChange transition)
{
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (element);
  GstStateChangeReturn ret;

  switch (transition) {
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      g_list_foreach (autoconvert->cached_events, (GFunc) gst_mini_object_unref,
          NULL);
      g_list_free (autoconvert->cached_events);
      autoconvert->cached_events = NULL;
      break;
    default:
      break;
  }

  return ret;
}

static GstElement *
gst_auto_convert_get_element_by_type (GstAutoConvert * autoconvert, GType type)
{
  GstIterator *iter = NULL;
  GstElement *elem = NULL;
  gboolean done;

  g_return_val_if_fail (type != 0, NULL);

  iter = gst_bin_iterate_elements (GST_BIN (autoconvert));

  if (!iter)
    return NULL;

  done = FALSE;
  while (!done) {
    switch (gst_iterator_next (iter, (gpointer) & elem)) {
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
get_pad_by_direction (GstElement * element, GstPadDirection direction)
{
  GstIterator *iter = gst_element_iterate_pads (element);
  GstPad *pad = NULL;
  GstPad *selected_pad = NULL;
  gboolean done;

  if (!iter)
    return NULL;

  done = FALSE;
  while (!done) {
    switch (gst_iterator_next (iter, (gpointer) & pad)) {
      case GST_ITERATOR_OK:
        if (gst_pad_get_direction (pad) == direction) {
          /* We check if there is more than one pad in this direction,
           * if there is, we return NULL so that the element is refused
           */
          if (selected_pad) {
            done = TRUE;
            gst_object_unref (selected_pad);
            selected_pad = NULL;
          } else {
            selected_pad = pad;
          }
        } else {
          gst_object_unref (pad);
        }
        break;
      case GST_ITERATOR_RESYNC:
        if (selected_pad) {
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
gst_auto_convert_get_subelement (GstAutoConvert * autoconvert)
{
  GstElement *element = NULL;

  GST_OBJECT_LOCK (autoconvert);
  if (autoconvert->current_subelement)
    element = gst_object_ref (autoconvert->current_subelement);
  GST_OBJECT_UNLOCK (autoconvert);

  return element;
}

static GstPad *
gst_auto_convert_get_internal_sinkpad (GstAutoConvert * autoconvert)
{
  GstPad *pad = NULL;

  GST_OBJECT_LOCK (autoconvert);
  if (autoconvert->current_internal_sinkpad)
    pad = gst_object_ref (autoconvert->current_internal_sinkpad);
  GST_OBJECT_UNLOCK (autoconvert);

  return pad;
}


static GstPad *
gst_auto_convert_get_internal_srcpad (GstAutoConvert * autoconvert)
{
  GstPad *pad = NULL;

  GST_OBJECT_LOCK (autoconvert);
  if (autoconvert->current_internal_srcpad)
    pad = gst_object_ref (autoconvert->current_internal_srcpad);
  GST_OBJECT_UNLOCK (autoconvert);

  return pad;
}

/*
 * This function creates and adds an element to the GstAutoConvert
 * it then creates the internal pads and links them
 *
 */

static GstElement *
gst_auto_convert_add_element (GstAutoConvert * autoconvert,
    GstElementFactory * factory)
{
  GstElement *element = NULL;
  GstPad *internal_sinkpad = NULL;
  GstPad *internal_srcpad = NULL;
  GstPad *sinkpad;
  GstPad *srcpad;
  GstPadLinkReturn padlinkret;

  GST_DEBUG_OBJECT (autoconvert, "Adding element %s to the autoconvert bin",
      gst_plugin_feature_get_name (GST_PLUGIN_FEATURE (factory)));

  element = gst_element_factory_create (factory, NULL);
  if (!element)
    return NULL;

  if (!gst_bin_add (GST_BIN (autoconvert), element)) {
    GST_ERROR_OBJECT (autoconvert, "Could not add element %s to the bin",
        GST_OBJECT_NAME (element));
    gst_object_unref (element);
    return NULL;
  }

  gst_object_ref (element);

  srcpad = get_pad_by_direction (element, GST_PAD_SRC);
  if (!srcpad) {
    GST_ERROR_OBJECT (autoconvert, "Could not find source in %s",
        GST_OBJECT_NAME (element));
    goto error;
  }

  sinkpad = get_pad_by_direction (element, GST_PAD_SINK);
  if (!sinkpad) {
    GST_ERROR_OBJECT (autoconvert, "Could not find sink in %s",
        GST_OBJECT_NAME (element));
    goto error;
  }

  internal_sinkpad =
      gst_pad_new_from_static_template (&sink_internal_template,
      "sink_internal");
  internal_srcpad =
      gst_pad_new_from_static_template (&src_internal_template, "src_internal");

  if (!internal_sinkpad || !internal_srcpad) {
    GST_ERROR_OBJECT (autoconvert, "Could not create internal pads");
    goto error;
  }

  g_object_weak_ref (G_OBJECT (element), (GWeakNotify) gst_object_unref,
      internal_sinkpad);
  g_object_weak_ref (G_OBJECT (element), (GWeakNotify) gst_object_unref,
      internal_srcpad);

  gst_pad_set_active (internal_sinkpad, TRUE);
  gst_pad_set_active (internal_srcpad, TRUE);

  g_object_set_qdata (G_OBJECT (internal_srcpad), parent_quark, autoconvert);
  g_object_set_qdata (G_OBJECT (internal_sinkpad), parent_quark, autoconvert);

  gst_pad_set_chain_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_internal_sink_chain));
  gst_pad_set_event_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_internal_sink_event));
  gst_pad_set_query_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_internal_sink_query));
  gst_pad_set_query_type_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_internal_sink_query_type));
  gst_pad_set_getcaps_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_internal_sink_getcaps));
  gst_pad_set_bufferalloc_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_internal_sink_buffer_alloc));
  gst_pad_set_fixatecaps_function (internal_sinkpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_internal_sink_fixatecaps));

  gst_pad_set_event_function (internal_srcpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_internal_src_event));
  gst_pad_set_query_function (internal_srcpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_internal_src_query));
  gst_pad_set_query_type_function (internal_srcpad,
      GST_DEBUG_FUNCPTR (gst_auto_convert_internal_src_query_type));

  padlinkret = gst_pad_link (internal_srcpad, sinkpad);
  if (GST_PAD_LINK_FAILED (padlinkret)) {
    GST_WARNING_OBJECT (autoconvert, "Could not links pad %s:%s to %s:%s"
        " for reason %d",
        GST_DEBUG_PAD_NAME (internal_srcpad),
        GST_DEBUG_PAD_NAME (sinkpad), padlinkret);
    goto error;
  }

  padlinkret = gst_pad_link (srcpad, internal_sinkpad);
  if (GST_PAD_LINK_FAILED (padlinkret)) {
    GST_WARNING_OBJECT (autoconvert, "Could not links pad %s:%s to %s:%s"
        " for reason %d",
        GST_DEBUG_PAD_NAME (internal_srcpad),
        GST_DEBUG_PAD_NAME (sinkpad), padlinkret);
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
  gst_bin_remove (GST_BIN (autoconvert), element);
  gst_object_unref (element);

  return NULL;
}

static GstElement *
gst_auto_convert_get_or_make_element_from_factory (GstAutoConvert * autoconvert,
    GstElementFactory * factory)
{
  GstElement *element = NULL;
  GstElementFactory *loaded_factory =
      GST_ELEMENT_FACTORY (gst_plugin_feature_load (GST_PLUGIN_FEATURE
          (factory)));

  if (!loaded_factory)
    return NULL;

  element = gst_auto_convert_get_element_by_type (autoconvert,
      gst_element_factory_get_element_type (loaded_factory));

  if (!element) {
    element = gst_auto_convert_add_element (autoconvert, loaded_factory);
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
factory_can_intersect (GstElementFactory * factory, GstPadDirection direction,
    GstCaps * caps)
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

      intersect =
          gst_caps_intersect (gst_static_caps_get (&template->static_caps),
          caps);

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
gst_auto_convert_sink_setcaps (GstPad * pad, GstCaps * caps)
{
  GList *elem;
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (gst_pad_get_parent (pad));
  GstElement *subelement;
  GstCaps *other_caps = NULL;
  GstPad *peer;
  GList *factories;

  g_return_val_if_fail (autoconvert != NULL, FALSE);

  subelement = gst_auto_convert_get_subelement (autoconvert);
  if (subelement) {
    if (gst_pad_peer_accept_caps (autoconvert->current_internal_srcpad, caps)) {
      /* If we can set the new caps on the current element,
       * then we just get out
       */
      GST_DEBUG_OBJECT (autoconvert, "Could set %s:%s to %" GST_PTR_FORMAT,
          GST_DEBUG_PAD_NAME (autoconvert->current_internal_srcpad), caps);
      gst_object_unref (subelement);
      goto get_out;
    } else {
      /* If the current element doesn't work,
       * then we remove the current element before finding a new one.
       * By unsetting the fixatecaps function, we go back to the default one
       */
      gst_pad_set_fixatecaps_function (autoconvert->sinkpad, NULL);
      GST_OBJECT_LOCK (autoconvert);
      if (autoconvert->current_subelement == subelement) {
        gst_object_unref (autoconvert->current_subelement);
        autoconvert->current_subelement = NULL;
        autoconvert->current_internal_srcpad = NULL;
        autoconvert->current_internal_sinkpad = NULL;
      }
      GST_OBJECT_UNLOCK (autoconvert);
      gst_object_unref (subelement);
    }
  }

  peer = gst_pad_get_peer (autoconvert->srcpad);
  if (peer) {
    other_caps = gst_pad_get_caps (peer);
    gst_object_unref (peer);
  }

  GST_OBJECT_LOCK (autoconvert);
  factories = autoconvert->factories;
  GST_OBJECT_UNLOCK (autoconvert);

  if (!factories)
    factories = gst_auto_convert_load_factories (autoconvert);

  for (elem = factories; elem; elem = g_list_next (elem)) {
    GstElementFactory *factory = GST_ELEMENT_FACTORY (elem->data);
    GstElement *element;
    GstPad *internal_srcpad = NULL;
    GstPad *internal_sinkpad = NULL;

    /* Lets first check if according to the static pad templates on the factory
     * these caps have any chance of success
     */
    if (!factory_can_intersect (factory, GST_PAD_SINK, caps)) {
      GST_LOG_OBJECT (autoconvert, "Factory %s does not accept sink caps %"
          GST_PTR_FORMAT,
          gst_plugin_feature_get_name (GST_PLUGIN_FEATURE (factory)), caps);
      continue;
    }
    if (other_caps != NULL) {
      if (!factory_can_intersect (factory, GST_PAD_SRC, other_caps)) {
        GST_LOG_OBJECT (autoconvert, "Factory %s does not accept src caps %"
            GST_PTR_FORMAT,
            gst_plugin_feature_get_name (GST_PLUGIN_FEATURE (factory)),
            other_caps);
        continue;
      }
    }

    /* The element had a chance of success, lets make it */

    element =
        gst_auto_convert_get_or_make_element_from_factory (autoconvert,
        factory);

    if (!element) {
      continue;
    }

    internal_srcpad = g_object_get_qdata (G_OBJECT (element),
        internal_srcpad_quark);
    internal_sinkpad = g_object_get_qdata (G_OBJECT (element),
        internal_sinkpad_quark);

    /* Now we check if the element can really accept said caps */
    if (!gst_pad_peer_accept_caps (internal_srcpad, caps)) {
      GST_DEBUG_OBJECT (autoconvert, "Could not set %s:%s to %" GST_PTR_FORMAT,
          GST_DEBUG_PAD_NAME (internal_srcpad), caps);
      goto next_element;
    }

    gst_pad_set_fixatecaps_function (autoconvert->sinkpad,
        gst_auto_convert_sink_fixatecaps);
    GST_OBJECT_LOCK (autoconvert);
    autoconvert->current_subelement = element;
    autoconvert->current_internal_srcpad = internal_srcpad;
    autoconvert->current_internal_sinkpad = internal_sinkpad;
    GST_OBJECT_UNLOCK (autoconvert);

    GST_INFO_OBJECT (autoconvert,
        "Selected element %s",
        GST_OBJECT_NAME (GST_OBJECT (autoconvert->current_subelement)));

    break;

  next_element:
    continue;
  }

get_out:
  if (other_caps)
    gst_caps_unref (other_caps);
  gst_object_unref (autoconvert);

  if (autoconvert->current_subelement) {
    return TRUE;
  } else {
    GST_WARNING_OBJECT (autoconvert,
        "Could not find a matching element for caps");
    return FALSE;
  }
}

/*
 * This function filters the pad pad templates, taking only transform element
 * (with one sink and one src pad)
 */

static gboolean
gst_auto_convert_default_filter_func (GstPluginFeature * feature,
    gpointer user_data)
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

  diff = gst_plugin_feature_get_rank (f2) - gst_plugin_feature_get_rank (f1);
  if (diff != 0)
    return diff;

  rname1 = gst_plugin_feature_get_name (f1);
  rname2 = gst_plugin_feature_get_name (f2);

  diff = strcmp (rname2, rname1);

  return diff;
}

static GList *
gst_auto_convert_load_factories (GstAutoConvert * autoconvert)
{
  GList *all_factories;
  GList *out_factories;

  all_factories =
      gst_default_registry_feature_filter (gst_auto_convert_default_filter_func,
      FALSE, NULL);

  all_factories = g_list_sort (all_factories, (GCompareFunc) compare_ranks);

  g_assert (all_factories);

  GST_OBJECT_LOCK (autoconvert);
  if (autoconvert->factories == NULL) {
    autoconvert->factories = all_factories;
    all_factories = NULL;
  }
  out_factories = autoconvert->factories;
  GST_OBJECT_UNLOCK (autoconvert);

  if (all_factories) {
    /* In this case, someone set the property while we were looking! */
    gst_plugin_feature_list_free (all_factories);
  }

  return out_factories;
}

/* In this case, we should almost always have an internal element, because
 * set_caps() should have been called first
 */

static GstFlowReturn
gst_auto_convert_sink_chain (GstPad * pad, GstBuffer * buffer)
{
  GstFlowReturn ret = GST_FLOW_NOT_NEGOTIATED;
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (gst_pad_get_parent (pad));
  GstPad *internal_srcpad;

  internal_srcpad = gst_auto_convert_get_internal_srcpad (autoconvert);
  if (internal_srcpad) {
    GList *events = NULL;
    GList *l;

    GST_OBJECT_LOCK (autoconvert);
    if (autoconvert->cached_events) {
      events = g_list_reverse (autoconvert->cached_events);
      autoconvert->cached_events = NULL;
    }
    GST_OBJECT_UNLOCK (autoconvert);

    if (events) {
      GST_DEBUG_OBJECT (autoconvert, "Sending cached events downstream");
      for (l = events; l; l = l->next)
        gst_pad_push_event (internal_srcpad, l->data);
      g_list_free (events);
    }

    ret = gst_pad_push (internal_srcpad, buffer);
    gst_object_unref (internal_srcpad);
  } else {
    GST_ERROR_OBJECT (autoconvert, "Got buffer without an negotiated element,"
        " returning not-negotiated");
  }

  gst_object_unref (autoconvert);

  return ret;
}

static gboolean
gst_auto_convert_sink_event (GstPad * pad, GstEvent * event)
{
  gboolean ret = TRUE;
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (gst_pad_get_parent (pad));
  GstPad *internal_srcpad;

  internal_srcpad = gst_auto_convert_get_internal_srcpad (autoconvert);
  if (internal_srcpad) {
    ret = gst_pad_push_event (internal_srcpad, event);
    gst_object_unref (internal_srcpad);
  } else {
    switch (GST_EVENT_TYPE (event)) {
      case GST_EVENT_FLUSH_STOP:
        GST_OBJECT_LOCK (autoconvert);
        g_list_foreach (autoconvert->cached_events,
            (GFunc) gst_mini_object_unref, NULL);
        g_list_free (autoconvert->cached_events);
        autoconvert->cached_events = NULL;
        GST_OBJECT_UNLOCK (autoconvert);
        /* fall through */
      case GST_EVENT_FLUSH_START:
        ret = gst_pad_push_event (autoconvert->srcpad, event);
        break;
      default:
        GST_OBJECT_LOCK (autoconvert);
        autoconvert->cached_events =
            g_list_prepend (autoconvert->cached_events, event);
        ret = TRUE;
        GST_OBJECT_UNLOCK (autoconvert);
        break;
    }
  }

  gst_object_unref (autoconvert);

  return ret;
}

/* TODO Properly test that this code works well for queries */
static gboolean
gst_auto_convert_sink_query (GstPad * pad, GstQuery * query)
{
  gboolean ret = TRUE;
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (gst_pad_get_parent (pad));
  GstElement *subelement;

  subelement = gst_auto_convert_get_subelement (autoconvert);
  if (subelement) {
    GstPad *sub_sinkpad = get_pad_by_direction (subelement, GST_PAD_SINK);

    ret = gst_pad_query (sub_sinkpad, query);

    gst_object_unref (sub_sinkpad);
    gst_object_unref (subelement);
  } else {
    GST_WARNING_OBJECT (autoconvert, "Got query while no element was selected,"
        "letting through");
    ret = gst_pad_peer_query (autoconvert->srcpad, query);
  }

  gst_object_unref (autoconvert);

  return ret;
}

/* TODO Test that this code works properly for queries */
static const GstQueryType *
gst_auto_convert_sink_query_type (GstPad * pad)
{
  const GstQueryType *ret = NULL;
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (gst_pad_get_parent (pad));
  GstElement *subelement;

  subelement = gst_auto_convert_get_subelement (autoconvert);
  if (subelement) {
    GstPad *sub_sinkpad = get_pad_by_direction (subelement, GST_PAD_SINK);

    ret = gst_pad_get_query_types (sub_sinkpad);

    gst_object_unref (sub_sinkpad);
    gst_object_unref (subelement);
  } else {
    ret = gst_pad_get_query_types_default (pad);
  }

  gst_object_unref (autoconvert);

  return ret;
}

static void
gst_auto_convert_sink_fixatecaps (GstPad * pad, GstCaps * caps)
{
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (gst_pad_get_parent (pad));
  GstElement *subelement;

  subelement = gst_auto_convert_get_subelement (autoconvert);
  if (subelement) {
    GstPad *sinkpad = get_pad_by_direction (subelement, GST_PAD_SINK);
    gst_pad_fixate_caps (sinkpad, caps);
    gst_object_unref (sinkpad);
    gst_object_unref (subelement);
  }
}

/**
 * gst_auto_convert_sink_getcaps:
 * @pad: the sink #GstPad
 *
 * This function returns the union of the caps of all the possible element
 * factories, based on the static pad templates.
 * It also checks does a getcaps on the downstream element and ignores all
 * factories whose static caps can not satisfy it.
 *
 * It does not try to use each elements getcaps() function
 */

static GstCaps *
gst_auto_convert_sink_getcaps (GstPad * pad)
{
  GstCaps *caps = NULL, *other_caps = NULL;
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (gst_pad_get_parent (pad));
  GstPad *peer;
  GList *elem, *factories;

  caps = gst_caps_new_empty ();

  peer = gst_pad_get_peer (autoconvert->srcpad);
  if (peer) {
    other_caps = gst_pad_get_caps (peer);
    gst_object_unref (peer);
  }

  GST_DEBUG_OBJECT (autoconvert,
      "Lets find all the element that can fit here with src caps %"
      GST_PTR_FORMAT, other_caps);

  if (other_caps && gst_caps_is_empty (other_caps)) {
    goto out;
  }

  GST_OBJECT_LOCK (autoconvert);
  factories = autoconvert->factories;
  GST_OBJECT_UNLOCK (autoconvert);

  if (!factories)
    factories = gst_auto_convert_load_factories (autoconvert);

  for (elem = factories; elem; elem = g_list_next (elem)) {
    GstElementFactory *factory = GST_ELEMENT_FACTORY (elem->data);
    GstElement *element = NULL;
    GstCaps *element_caps;
    GstPad *internal_srcpad = NULL;

    if (other_caps != NULL) {
      if (!factory_can_intersect (factory, GST_PAD_SRC, other_caps)) {
        GST_LOG_OBJECT (autoconvert, "Factory %s does not accept src caps %"
            GST_PTR_FORMAT,
            gst_plugin_feature_get_name (GST_PLUGIN_FEATURE (factory)),
            other_caps);
        continue;
      }
    }

    if (other_caps) {

      element =
          gst_auto_convert_get_or_make_element_from_factory (autoconvert,
          factory);

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
          tmp; tmp = g_list_next (tmp)) {
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

  GST_DEBUG_OBJECT (autoconvert, "Returning unioned caps %" GST_PTR_FORMAT,
      caps);

out:
  gst_object_unref (autoconvert);

  if (other_caps)
    gst_caps_unref (other_caps);

  return caps;
}


static GstFlowReturn
gst_auto_convert_sink_buffer_alloc (GstPad * pad, guint64 offset,
    guint size, GstCaps * caps, GstBuffer ** buf)
{
  GstFlowReturn ret = GST_FLOW_OK;
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (gst_pad_get_parent (pad));
  GstPad *internal_srcpad;

  g_return_val_if_fail (autoconvert != NULL, GST_FLOW_ERROR);

  internal_srcpad = gst_auto_convert_get_internal_srcpad (autoconvert);
  if (internal_srcpad) {
    ret = gst_pad_alloc_buffer (internal_srcpad, offset, size, caps, buf);
    gst_object_unref (internal_srcpad);
  } else
    /* Fallback to the default */
    *buf = NULL;

  gst_object_unref (autoconvert);

  return ret;
}

static gboolean
gst_auto_convert_src_event (GstPad * pad, GstEvent * event)
{
  gboolean ret = TRUE;
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (gst_pad_get_parent (pad));
  GstPad *internal_sinkpad;

  internal_sinkpad = gst_auto_convert_get_internal_sinkpad (autoconvert);
  if (internal_sinkpad) {
    ret = gst_pad_push_event (internal_sinkpad, event);
    gst_object_unref (internal_sinkpad);
  } else {
    GST_WARNING_OBJECT (autoconvert, "Got event while not element was selected,"
        "letting through");
    ret = gst_pad_push_event (autoconvert->sinkpad, event);
  }

  gst_object_unref (autoconvert);

  return ret;
}

/* TODO Properly test that this code works well for queries */
static gboolean
gst_auto_convert_src_query (GstPad * pad, GstQuery * query)
{
  gboolean ret = TRUE;
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (gst_pad_get_parent (pad));
  GstElement *subelement;

  subelement = gst_auto_convert_get_subelement (autoconvert);
  if (subelement) {
    GstPad *sub_srcpad = get_pad_by_direction (subelement, GST_PAD_SRC);

    ret = gst_pad_query (sub_srcpad, query);

    gst_object_unref (sub_srcpad);
    gst_object_unref (subelement);
  } else {
    GST_WARNING_OBJECT (autoconvert, "Got query while not element was selected,"
        "letting through");
    ret = gst_pad_peer_query (autoconvert->sinkpad, query);
  }

  gst_object_unref (autoconvert);

  return ret;
}

/* TODO Properly test that this code works well for queries */
static const GstQueryType *
gst_auto_convert_src_query_type (GstPad * pad)
{
  const GstQueryType *ret = NULL;
  GstAutoConvert *autoconvert = GST_AUTO_CONVERT (gst_pad_get_parent (pad));
  GstElement *subelement;

  subelement = gst_auto_convert_get_subelement (autoconvert);
  if (subelement) {
    GstPad *sub_srcpad = get_pad_by_direction (subelement, GST_PAD_SRC);

    ret = gst_pad_get_query_types (sub_srcpad);

    gst_object_unref (sub_srcpad);
    gst_object_unref (subelement);
  } else {
    ret = gst_pad_get_query_types_default (pad);
  }

  gst_object_unref (autoconvert);

  return ret;
}

static GstFlowReturn
gst_auto_convert_internal_sink_chain (GstPad * pad, GstBuffer * buffer)
{
  GstAutoConvert *autoconvert =
      GST_AUTO_CONVERT (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));

  return gst_pad_push (autoconvert->srcpad, buffer);
}

static gboolean
gst_auto_convert_internal_sink_event (GstPad * pad, GstEvent * event)
{
  GstAutoConvert *autoconvert =
      GST_AUTO_CONVERT (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));

  return gst_pad_push_event (autoconvert->srcpad, event);
}

static gboolean
gst_auto_convert_internal_sink_query (GstPad * pad, GstQuery * query)
{
  GstAutoConvert *autoconvert =
      GST_AUTO_CONVERT (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));
  GstPad *peerpad = gst_pad_get_peer (autoconvert->srcpad);
  gboolean ret = FALSE;

  if (peerpad) {
    ret = gst_pad_query (peerpad, query);
    gst_object_unref (peerpad);
  }

  return ret;
}

static const GstQueryType *
gst_auto_convert_internal_sink_query_type (GstPad * pad)
{
  GstAutoConvert *autoconvert =
      GST_AUTO_CONVERT (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));
  GstPad *peerpad = gst_pad_get_peer (autoconvert->srcpad);
  const GstQueryType *ret = NULL;

  if (peerpad) {
    ret = gst_pad_get_query_types (peerpad);
    gst_object_unref (peerpad);
  } else
    ret = gst_pad_get_query_types_default (pad);

  return ret;
}

static GstCaps *
gst_auto_convert_internal_sink_getcaps (GstPad * pad)
{
  GstAutoConvert *autoconvert =
      GST_AUTO_CONVERT (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));

  return gst_pad_peer_get_caps (autoconvert->srcpad);
}

static void
gst_auto_convert_internal_sink_fixatecaps (GstPad * pad, GstCaps * caps)
{
  GstAutoConvert *autoconvert =
      GST_AUTO_CONVERT (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));
  GstPad *peerpad = gst_pad_get_peer (autoconvert->sinkpad);

  if (peerpad) {
    gst_pad_fixate_caps (peerpad, caps);
    gst_object_unref (peerpad);
  }
}

static GstFlowReturn
gst_auto_convert_internal_sink_buffer_alloc (GstPad * pad, guint64 offset,
    guint size, GstCaps * caps, GstBuffer ** buf)
{
  GstAutoConvert *autoconvert =
      GST_AUTO_CONVERT (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));

  return gst_pad_alloc_buffer (autoconvert->srcpad, offset, size, caps, buf);
}

static gboolean
gst_auto_convert_internal_src_event (GstPad * pad, GstEvent * event)
{
  GstAutoConvert *autoconvert =
      GST_AUTO_CONVERT (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));

  return gst_pad_push_event (autoconvert->sinkpad, event);
}

static gboolean
gst_auto_convert_internal_src_query (GstPad * pad, GstQuery * query)
{
  GstAutoConvert *autoconvert =
      GST_AUTO_CONVERT (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));
  GstPad *peerpad = gst_pad_get_peer (autoconvert->sinkpad);
  gboolean ret = FALSE;

  if (peerpad) {
    ret = gst_pad_query (peerpad, query);
    gst_object_unref (peerpad);
  }

  return ret;
}

static const GstQueryType *
gst_auto_convert_internal_src_query_type (GstPad * pad)
{
  GstAutoConvert *autoconvert =
      GST_AUTO_CONVERT (g_object_get_qdata (G_OBJECT (pad),
          parent_quark));
  GstPad *peerpad = gst_pad_get_peer (autoconvert->sinkpad);
  const GstQueryType *ret = NULL;

  if (peerpad) {
    ret = gst_pad_get_query_types (peerpad);
    gst_object_unref (peerpad);
  } else {
    ret = gst_pad_get_query_types_default (pad);
  }

  return ret;
}

gboolean
gst_auto_convert_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "autoconvert",
      GST_RANK_NONE, GST_TYPE_AUTO_CONVERT);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "autoconvert",
    "Selects convertor element based on caps",
    gst_auto_convert_plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME,
    GST_PACKAGE_ORIGIN)
