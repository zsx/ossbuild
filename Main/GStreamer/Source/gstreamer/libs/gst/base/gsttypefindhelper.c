/* GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) 2000,2005 Wim Taymans <wim@fluendo.com>
 * Copyright (C) 2006      Tim-Philipp Müller <tim centricular net>
 *
 * gsttypefindhelper.c:
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
 * SECTION:gsttypefindhelper
 * @short_description: Utility functions for typefinding 
 *
 * Utility functions for elements doing typefinding:
 * gst_type_find_helper() does typefinding in pull mode, while
 * gst_type_find_helper_for_buffer() is useful for elements needing to do
 * typefinding in push mode from a chain function.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "gsttypefindhelper.h"

static gint
type_find_factory_rank_cmp (gconstpointer fac1, gconstpointer fac2)
{
  if (GST_PLUGIN_FEATURE (fac1)->rank != GST_PLUGIN_FEATURE (fac2)->rank)
    return GST_PLUGIN_FEATURE (fac2)->rank - GST_PLUGIN_FEATURE (fac1)->rank;

  /* to make the order in which things happen more deterministic,
   * sort by name when the ranks are the same. */
  return strcmp (GST_PLUGIN_FEATURE_NAME (fac1),
      GST_PLUGIN_FEATURE_NAME (fac2));
}

/* ********************** typefinding in pull mode ************************ */

static void
helper_find_suggest (gpointer data, guint probability, const GstCaps * caps);

typedef struct
{
  GSList *buffers;              /* buffer cache */
  guint64 size;
  guint64 last_offset;
  GstTypeFindHelperGetRangeFunction func;
  guint best_probability;
  GstCaps *caps;
  GstTypeFindFactory *factory;  /* for logging */
  GstObject *obj;               /* for logging */
} GstTypeFindHelper;

/*
 * helper_find_peek:
 * @data: helper data struct
 * @off: stream offset
 * @size: block size
 *
 * Get data pointer within a stream. Keeps a cache of read buffers (partly
 * for performance reasons, but mostly because pointers returned by us need
 * to stay valid until typefinding has finished)
 *
 * Returns: address of the data or %NULL if buffer does not cover the
 * requested range.
 */
static guint8 *
helper_find_peek (gpointer data, gint64 offset, guint size)
{
  GstTypeFindHelper *helper;
  GstBuffer *buffer;
  GstFlowReturn ret;
  GSList *insert_pos = NULL;
  guint buf_size;
  guint64 buf_offset;
  GstCaps *caps;

  helper = (GstTypeFindHelper *) data;

  GST_LOG_OBJECT (helper->obj, "'%s' called peek (%" G_GINT64_FORMAT
      ", %u)", GST_PLUGIN_FEATURE_NAME (helper->factory), offset, size);

  if (size == 0)
    return NULL;

  if (offset < 0) {
    if (helper->size == -1 || helper->size < -offset)
      return NULL;

    offset += helper->size;
  }

  /* see if we have a matching buffer already in our list */
  if (size > 0 && offset <= helper->last_offset) {
    GSList *walk;

    for (walk = helper->buffers; walk; walk = walk->next) {
      GstBuffer *buf = GST_BUFFER_CAST (walk->data);
      guint64 buf_offset = GST_BUFFER_OFFSET (buf);
      guint buf_size = GST_BUFFER_SIZE (buf);

      /* buffers are kept sorted by end offset (highest first) in the list, so
       * at this point we save the current position and stop searching if 
       * we're after the searched end offset */
      if (buf_offset <= offset) {
        if ((offset + size) < (buf_offset + buf_size)) {
          return GST_BUFFER_DATA (buf) + (offset - buf_offset);
        }
      } else if (offset + size >= buf_offset + buf_size) {
        insert_pos = walk;
        break;
      }
    }
  }

  buffer = NULL;
  /* some typefinders go in 1 byte steps over 1k of data and request
   * small buffers. It is really inefficient to pull each time, and pulling
   * a larger chunk is almost free. Trying to pull a larger chunk at the end
   * of the file is also not a problem here, we'll just get a truncated buffer
   * in that case (and we'll have to double-check the size we actually get
   * anyway, see below) */
  ret = helper->func (helper->obj, offset, MAX (size, 4096), &buffer);

  if (ret != GST_FLOW_OK)
    goto error;

  caps = GST_BUFFER_CAPS (buffer);

  if (caps && !gst_caps_is_empty (caps) && !gst_caps_is_any (caps)) {
    GST_DEBUG ("buffer has caps %" GST_PTR_FORMAT ", suggest max probability",
        caps);

    gst_caps_replace (&helper->caps, caps);
    helper->best_probability = GST_TYPE_FIND_MAXIMUM;

    gst_buffer_unref (buffer);
    return NULL;
  }

  /* getrange might silently return shortened buffers at the end of a file,
   * we must, however, always return either the full requested data or NULL */
  buf_offset = GST_BUFFER_OFFSET (buffer);
  buf_size = GST_BUFFER_SIZE (buffer);

  if ((buf_offset != -1 && buf_offset != offset) || buf_size < size) {
    GST_DEBUG ("droping short buffer: %" G_GUINT64_FORMAT "-%" G_GUINT64_FORMAT
        " instead of %" G_GUINT64_FORMAT "-%" G_GUINT64_FORMAT,
        buf_offset, buf_offset + buf_size - 1, offset, offset + size - 1);
    gst_buffer_unref (buffer);
    return NULL;
  }

  if (insert_pos) {
    helper->buffers =
        g_slist_insert_before (helper->buffers, insert_pos, buffer);
  } else {
    /* if insert_pos is not set, our offset is bigger than the largest offset
     * we have so far; since we keep the list sorted with highest offsets
     * first, we need to prepend the buffer to the list */
    helper->last_offset = GST_BUFFER_OFFSET (buffer) + GST_BUFFER_SIZE (buffer);
    helper->buffers = g_slist_prepend (helper->buffers, buffer);
  }
  return GST_BUFFER_DATA (buffer);

error:
  {
    GST_INFO ("typefind function returned: %s", gst_flow_get_name (ret));
    return NULL;
  }
}

/*
 * helper_find_suggest:
 * @data: helper data struct
 * @probability: probability of the match
 * @caps: caps of the type
 *
 * If given @probability is higher, replace previously store caps.
 */
static void
helper_find_suggest (gpointer data, guint probability, const GstCaps * caps)
{
  GstTypeFindHelper *helper = (GstTypeFindHelper *) data;

  GST_LOG_OBJECT (helper->obj,
      "'%s' called called suggest (%u, %" GST_PTR_FORMAT ")",
      GST_PLUGIN_FEATURE_NAME (helper->factory), probability, caps);

  if (probability > helper->best_probability) {
    GstCaps *copy = gst_caps_copy (caps);

    gst_caps_replace (&helper->caps, copy);
    gst_caps_unref (copy);
    helper->best_probability = probability;
  }
}

static guint64
helper_find_get_length (gpointer data)
{
  GstTypeFindHelper *helper = (GstTypeFindHelper *) data;

  GST_LOG_OBJECT (helper->obj, "'%s' called called get_length, returning %"
      G_GUINT64_FORMAT, GST_PLUGIN_FEATURE_NAME (helper->factory),
      helper->size);

  return helper->size;
}

/**
 * gst_type_find_helper_get_range:
 * @obj: A #GstObject that will be passed as first argument to @func
 * @func: A generic #GstTypeFindHelperGetRangeFunction that will be used
 *        to access data at random offsets when doing the typefinding
 * @size: The length in bytes
 * @prob: location to store the probability of the found caps, or #NULL
 *
 * Utility function to do pull-based typefinding. Unlike gst_type_find_helper()
 * however, this function will use the specified function @func to obtain the
 * data needed by the typefind functions, rather than operating on a given
 * source pad. This is useful mostly for elements like tag demuxers which
 * strip off data at the beginning and/or end of a file and want to typefind
 * the stripped data stream before adding their own source pad (the specified
 * callback can then call the upstream peer pad with offsets adjusted for the
 * tag size, for example).
 *
 * Returns: The #GstCaps corresponding to the data stream.
 * Returns #NULL if no #GstCaps matches the data stream.
 */

GstCaps *
gst_type_find_helper_get_range (GstObject * obj,
    GstTypeFindHelperGetRangeFunction func, guint64 size,
    GstTypeFindProbability * prob)
{
  GstTypeFindHelper helper;
  GstTypeFind find;
  GSList *walk;
  GList *l, *type_list;
  GstCaps *result = NULL;

  g_return_val_if_fail (GST_IS_OBJECT (obj), NULL);
  g_return_val_if_fail (func != NULL, NULL);

  helper.buffers = NULL;
  helper.size = size;
  helper.last_offset = 0;
  helper.func = func;
  helper.best_probability = 0;
  helper.caps = NULL;
  helper.obj = obj;

  find.data = &helper;
  find.peek = helper_find_peek;
  find.suggest = helper_find_suggest;

  if (size == 0 || size == (guint64) - 1) {
    find.get_length = NULL;
  } else {
    find.get_length = helper_find_get_length;
  }

  /* FIXME: we need to keep this list within the registry */
  type_list = gst_type_find_factory_get_list ();
  type_list = g_list_sort (type_list, type_find_factory_rank_cmp);

  for (l = type_list; l; l = l->next) {
    helper.factory = GST_TYPE_FIND_FACTORY (l->data);
    gst_type_find_factory_call_function (helper.factory, &find);
    if (helper.best_probability >= GST_TYPE_FIND_MAXIMUM)
      break;
  }
  gst_plugin_feature_list_free (type_list);

  for (walk = helper.buffers; walk; walk = walk->next)
    gst_buffer_unref (GST_BUFFER_CAST (walk->data));
  g_slist_free (helper.buffers);

  if (helper.best_probability > 0)
    result = helper.caps;

  if (prob)
    *prob = helper.best_probability;

  GST_LOG_OBJECT (obj, "Returning %" GST_PTR_FORMAT " (probability = %u)",
      result, (guint) helper.best_probability);

  return result;
}

/**
 * gst_type_find_helper:
 * @src: A source #GstPad
 * @size: The length in bytes
 *
 * Tries to find what type of data is flowing from the given source #GstPad.
 *
 * Returns: The #GstCaps corresponding to the data stream.
 * Returns #NULL if no #GstCaps matches the data stream.
 */

GstCaps *
gst_type_find_helper (GstPad * src, guint64 size)
{
  GstTypeFindHelperGetRangeFunction func;

  g_return_val_if_fail (GST_IS_OBJECT (src), NULL);
  g_return_val_if_fail (GST_PAD_GETRANGEFUNC (src) != NULL, NULL);

  func = (GstTypeFindHelperGetRangeFunction) (GST_PAD_GETRANGEFUNC (src));

  return gst_type_find_helper_get_range (GST_OBJECT (src), func, size, NULL);
}

/* ********************** typefinding for buffers ************************* */

typedef struct
{
  guint8 *data;                 /* buffer data */
  guint size;
  guint best_probability;
  GstCaps *caps;
  GstTypeFindFactory *factory;  /* for logging */
  GstObject *obj;               /* for logging */
} GstTypeFindBufHelper;

/*
 * buf_helper_find_peek:
 * @data: helper data struct
 * @off: stream offset
 * @size: block size
 *
 * Get data pointer within a buffer.
 *
 * Returns: address inside the buffer or %NULL if buffer does not cover the
 * requested range.
 */
static guint8 *
buf_helper_find_peek (gpointer data, gint64 off, guint size)
{
  GstTypeFindBufHelper *helper;

  helper = (GstTypeFindBufHelper *) data;
  GST_LOG_OBJECT (helper->obj, "'%s' called peek (%" G_GINT64_FORMAT ", %u)",
      GST_PLUGIN_FEATURE_NAME (helper->factory), off, size);

  if (size == 0)
    return NULL;

  if (off < 0) {
    GST_LOG_OBJECT (helper->obj, "'%s' wanted to peek at end; not supported",
        GST_PLUGIN_FEATURE_NAME (helper->factory));
    return NULL;
  }

  if ((off + size) <= helper->size)
    return helper->data + off;

  return NULL;
}

/*
 * buf_helper_find_suggest:
 * @data: helper data struct
 * @probability: probability of the match
 * @caps: caps of the type
 *
 * If given @probability is higher, replace previously store caps.
 */
static void
buf_helper_find_suggest (gpointer data, guint probability, const GstCaps * caps)
{
  GstTypeFindBufHelper *helper = (GstTypeFindBufHelper *) data;

  GST_LOG_OBJECT (helper->obj,
      "'%s' called called suggest (%u, %" GST_PTR_FORMAT ")",
      GST_PLUGIN_FEATURE_NAME (helper->factory), probability, caps);

  /* Note: not >= as we call typefinders in order of rank, highest first */
  if (probability > helper->best_probability) {
    GstCaps *copy = gst_caps_copy (caps);

    gst_caps_replace (&helper->caps, copy);
    gst_caps_unref (copy);
    helper->best_probability = probability;
  }
}

/**
 * gst_type_find_helper_for_buffer:
 * @obj: object doing the typefinding, or NULL (used for logging)
 * @buf: a #GstBuffer with data to typefind
 * @prob: location to store the probability of the found caps, or #NULL
 *
 * Tries to find what type of data is contained in the given #GstBuffer, the
 * assumption being that the buffer represents the beginning of the stream or
 * file.
 *
 * All available typefinders will be called on the data in order of rank. If
 * a typefinding function returns a probability of #GST_TYPE_FIND_MAXIMUM,
 * typefinding is stopped immediately and the found caps will be returned
 * right away. Otherwise, all available typefind functions will the tried,
 * and the caps with the highest probability will be returned, or #NULL if
 * the content of the buffer could not be identified.
 *
 * Returns: The #GstCaps corresponding to the data, or #NULL if no type could
 * be found. The caller should free the caps returned with gst_caps_unref().
 */
GstCaps *
gst_type_find_helper_for_buffer (GstObject * obj, GstBuffer * buf,
    GstTypeFindProbability * prob)
{
  GstTypeFindBufHelper helper;
  GstTypeFind find;
  GList *l, *type_list;
  GstCaps *result = NULL;

  g_return_val_if_fail (buf != NULL, NULL);
  g_return_val_if_fail (GST_IS_BUFFER (buf), NULL);
  g_return_val_if_fail (GST_BUFFER_OFFSET (buf) == 0 ||
      GST_BUFFER_OFFSET (buf) == GST_BUFFER_OFFSET_NONE, NULL);

  helper.data = GST_BUFFER_DATA (buf);
  helper.size = GST_BUFFER_SIZE (buf);
  helper.best_probability = 0;
  helper.caps = NULL;
  helper.obj = obj;

  if (helper.data == NULL || helper.size == 0)
    return NULL;

  find.data = &helper;
  find.peek = buf_helper_find_peek;
  find.suggest = buf_helper_find_suggest;
  find.get_length = NULL;

  /* FIXME: we need to keep this list within the registry */
  type_list = gst_type_find_factory_get_list ();
  type_list = g_list_sort (type_list, type_find_factory_rank_cmp);

  for (l = type_list; l; l = l->next) {
    helper.factory = GST_TYPE_FIND_FACTORY (l->data);
    gst_type_find_factory_call_function (helper.factory, &find);
    if (helper.best_probability >= GST_TYPE_FIND_MAXIMUM)
      break;
  }
  gst_plugin_feature_list_free (type_list);

  if (helper.best_probability > 0)
    result = helper.caps;

  if (prob)
    *prob = helper.best_probability;

  GST_LOG_OBJECT (obj, "Returning %" GST_PTR_FORMAT " (probability = %u)",
      result, (guint) helper.best_probability);

  return result;
}
