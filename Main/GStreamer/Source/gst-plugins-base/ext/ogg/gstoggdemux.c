/* GStreamer
 * Copyright (C) 2004 Wim Taymans <wim@fluendo.com>
 *
 * gstoggdemux.c: ogg stream demuxer
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
 * SECTION:element-oggdemux
 * @see_also: <link linkend="gst-plugins-base-plugins-oggmux">oggmux</link>
 *
 * This element demuxes ogg files into their encoded audio and video components.
 *
 * <refsect2>
 * <title>Example pipelines</title>
 * |[
 * gst-launch -v filesrc location=test.ogg ! oggdemux ! vorbisdec ! audioconvert ! alsasink
 * ]| Decodes the vorbis audio stored inside an ogg container.
 * </refsect2>
 *
 * Last reviewed on 2006-12-30 (0.10.5)
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>
#include <gst/gst-i18n-plugin.h>
#include <gst/base/gsttypefindhelper.h>

#include "gstoggdemux.h"

static const GstElementDetails gst_ogg_demux_details =
GST_ELEMENT_DETAILS ("Ogg demuxer",
    "Codec/Demuxer",
    "demux ogg streams (info about ogg: http://xiph.org)",
    "Wim Taymans <wim@fluendo.com>");

#define CHUNKSIZE (8500)        /* this is out of vorbisfile */
#define SKELETON_FISHEAD_SIZE 64
#define SKELETON_FISBONE_MIN_SIZE 52

#define GST_FLOW_LIMIT GST_FLOW_CUSTOM_ERROR

#define GST_CHAIN_LOCK(ogg)     g_mutex_lock((ogg)->chain_lock)
#define GST_CHAIN_UNLOCK(ogg)   g_mutex_unlock((ogg)->chain_lock)

GST_DEBUG_CATEGORY_STATIC (gst_ogg_demux_debug);
GST_DEBUG_CATEGORY_STATIC (gst_ogg_demux_setup_debug);
#define GST_CAT_DEFAULT gst_ogg_demux_debug

static ogg_page *
gst_ogg_page_copy (ogg_page * page)
{
  ogg_page *p = g_new0 (ogg_page, 1);

  /* make a copy of the page */
  p->header = g_memdup (page->header, page->header_len);
  p->header_len = page->header_len;
  p->body = g_memdup (page->body, page->body_len);
  p->body_len = page->body_len;

  return p;
}

static void
gst_ogg_page_free (ogg_page * page)
{
  g_free (page->header);
  g_free (page->body);
  g_free (page);
}

static GstStaticPadTemplate internaltemplate =
GST_STATIC_PAD_TEMPLATE ("internal",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

static gboolean gst_ogg_demux_collect_chain_info (GstOggDemux * ogg,
    GstOggChain * chain);
static gboolean gst_ogg_demux_activate_chain (GstOggDemux * ogg,
    GstOggChain * chain, GstEvent * event);
static void gst_ogg_chain_mark_discont (GstOggChain * chain);

static gboolean gst_ogg_demux_perform_seek (GstOggDemux * ogg,
    GstEvent * event);
static gboolean gst_ogg_demux_receive_event (GstElement * element,
    GstEvent * event);

static void gst_ogg_pad_class_init (GstOggPadClass * klass);
static void gst_ogg_pad_init (GstOggPad * pad);
static void gst_ogg_pad_dispose (GObject * object);
static void gst_ogg_pad_finalize (GObject * object);

#if 0
static const GstFormat *gst_ogg_pad_formats (GstPad * pad);
static const GstEventMask *gst_ogg_pad_event_masks (GstPad * pad);
#endif
static const GstQueryType *gst_ogg_pad_query_types (GstPad * pad);
static gboolean gst_ogg_pad_src_query (GstPad * pad, GstQuery * query);
static gboolean gst_ogg_pad_event (GstPad * pad, GstEvent * event);
static GstCaps *gst_ogg_pad_getcaps (GstPad * pad);
static GstOggPad *gst_ogg_chain_get_stream (GstOggChain * chain,
    glong serialno);

static gboolean gst_ogg_pad_query_convert (GstOggPad * pad,
    GstFormat src_format, gint64 src_val,
    GstFormat * dest_format, gint64 * dest_val);
static GstClockTime gst_annodex_granule_to_time (gint64 granulepos,
    gint64 granulerate_n, gint64 granulerate_d, guint8 granuleshift);

static GstFlowReturn gst_ogg_demux_combine_flows (GstOggDemux * ogg,
    GstOggPad * pad, GstFlowReturn ret);

G_DEFINE_TYPE (GstOggPad, gst_ogg_pad, GST_TYPE_PAD);

static void
gst_ogg_pad_class_init (GstOggPadClass * klass)
{
  GObjectClass *gobject_class;

  gobject_class = (GObjectClass *) klass;

  gobject_class->dispose = gst_ogg_pad_dispose;
  gobject_class->finalize = gst_ogg_pad_finalize;
}

static void
gst_ogg_pad_init (GstOggPad * pad)
{
  gst_pad_set_event_function (GST_PAD (pad),
      GST_DEBUG_FUNCPTR (gst_ogg_pad_event));
  gst_pad_set_getcaps_function (GST_PAD (pad),
      GST_DEBUG_FUNCPTR (gst_ogg_pad_getcaps));
  gst_pad_set_query_type_function (GST_PAD (pad),
      GST_DEBUG_FUNCPTR (gst_ogg_pad_query_types));
  gst_pad_set_query_function (GST_PAD (pad),
      GST_DEBUG_FUNCPTR (gst_ogg_pad_src_query));

  pad->mode = GST_OGG_PAD_MODE_INIT;

  pad->first_granule = -1;
  pad->current_granule = -1;

  pad->start_time = GST_CLOCK_TIME_NONE;
  pad->first_time = GST_CLOCK_TIME_NONE;

  pad->have_type = FALSE;
  pad->continued = NULL;
  pad->headers = NULL;
}

static void
gst_ogg_pad_dispose (GObject * object)
{
  GstOggPad *pad = GST_OGG_PAD (object);
  GstPad **elem_pad_p;
  GstElement **element_p;
  GstPad **elem_out_p;

  if (pad->element)
    gst_element_set_state (pad->element, GST_STATE_NULL);

  elem_pad_p = &pad->elem_pad;
  element_p = &pad->element;
  elem_out_p = &pad->elem_out;
  gst_object_replace ((GstObject **) elem_pad_p, NULL);
  gst_object_replace ((GstObject **) element_p, NULL);
  gst_object_replace ((GstObject **) elem_out_p, NULL);

  pad->chain = NULL;
  pad->ogg = NULL;

  g_list_foreach (pad->headers, (GFunc) gst_mini_object_unref, NULL);
  g_list_free (pad->headers);
  pad->headers = NULL;

  /* clear continued pages */
  g_list_foreach (pad->continued, (GFunc) gst_ogg_page_free, NULL);
  g_list_free (pad->continued);
  pad->continued = NULL;

  ogg_stream_reset (&pad->stream);

  G_OBJECT_CLASS (gst_ogg_pad_parent_class)->dispose (object);
}

static void
gst_ogg_pad_finalize (GObject * object)
{
  GstOggPad *pad = GST_OGG_PAD (object);

  ogg_stream_clear (&pad->stream);

  G_OBJECT_CLASS (gst_ogg_pad_parent_class)->finalize (object);
}

#if 0
static const GstFormat *
gst_ogg_pad_formats (GstPad * pad)
{
  static GstFormat src_formats[] = {
    GST_FORMAT_DEFAULT,         /* time */
    GST_FORMAT_TIME,            /* granulepos */
    0
  };
  static GstFormat sink_formats[] = {
    GST_FORMAT_BYTES,
    GST_FORMAT_DEFAULT,         /* bytes */
    0
  };

  return (GST_PAD_IS_SRC (pad) ? src_formats : sink_formats);
}
#endif

#if 0
static const GstEventMask *
gst_ogg_pad_event_masks (GstPad * pad)
{
  static const GstEventMask src_event_masks[] = {
    {GST_EVENT_SEEK, GST_SEEK_METHOD_SET | GST_SEEK_FLAG_FLUSH},
    {0,}
  };

  return src_event_masks;
}
#endif

static const GstQueryType *
gst_ogg_pad_query_types (GstPad * pad)
{
  static const GstQueryType query_types[] = {
    GST_QUERY_DURATION,
    GST_QUERY_SEEKING,
    0
  };

  return query_types;
}

static GstCaps *
gst_ogg_pad_getcaps (GstPad * pad)
{
  return gst_caps_ref (GST_PAD_CAPS (pad));
}

static gboolean
gst_ogg_pad_src_query (GstPad * pad, GstQuery * query)
{
  gboolean res = TRUE;
  GstOggDemux *ogg;
  GstOggPad *cur;

  ogg = GST_OGG_DEMUX (gst_pad_get_parent (pad));
  cur = GST_OGG_PAD (pad);

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_DURATION:
    {
      GstFormat format;
      gint64 total_time;

      gst_query_parse_duration (query, &format, NULL);
      /* can only get position in time */
      if (format != GST_FORMAT_TIME)
        goto wrong_format;

      if (ogg->seekable) {
        /* we must return the total seekable length */
        total_time = ogg->total_time;
      } else {
        /* in non-seek mode we can answer the query and we must return -1 */
        total_time = -1;
      }

      gst_query_set_duration (query, GST_FORMAT_TIME, total_time);
      break;
    }
    case GST_QUERY_SEEKING:
    {
      GstFormat format;

      gst_query_parse_seeking (query, &format, NULL, NULL, NULL);
      if (format == GST_FORMAT_TIME) {
        gst_query_set_seeking (query, GST_FORMAT_TIME, ogg->seekable,
            0, ogg->total_time);
      } else {
        res = FALSE;
      }
      break;
    }

    default:
      res = gst_pad_query_default (pad, query);
      break;
  }
done:
  gst_object_unref (ogg);

  return res;

  /* ERRORS */
wrong_format:
  {
    GST_DEBUG_OBJECT (ogg, "only query duration on TIME is supported");
    res = FALSE;
    goto done;
  }
}

static gboolean
gst_ogg_demux_receive_event (GstElement * element, GstEvent * event)
{
  gboolean res;
  GstOggDemux *ogg;

  ogg = GST_OGG_DEMUX (element);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_SEEK:
      /* can't seek if we are not seekable, FIXME could pass the
       * seek query upstream after converting it to bytes using
       * the average bitrate of the stream. */
      if (!ogg->seekable) {
        GST_DEBUG_OBJECT (ogg, "seek on non seekable stream");
        goto error;
      }

      /* now do the seek */
      res = gst_ogg_demux_perform_seek (ogg, event);
      gst_event_unref (event);
      break;
    default:
      GST_DEBUG_OBJECT (ogg, "We only handle seek events here");
      goto error;
  }

  return res;

  /* ERRORS */
error:
  {
    GST_DEBUG_OBJECT (ogg, "error handling event");
    gst_event_unref (event);
    return FALSE;
  }
}

static gboolean
gst_ogg_pad_event (GstPad * pad, GstEvent * event)
{
  gboolean res;
  GstOggDemux *ogg;
  GstOggPad *cur;

  ogg = GST_OGG_DEMUX (gst_pad_get_parent (pad));
  cur = GST_OGG_PAD (pad);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_SEEK:
      /* can't seek if we are not seekable, FIXME could pass the
       * seek query upstream after converting it to bytes using
       * the average bitrate of the stream. */
      if (!ogg->seekable) {
        GST_DEBUG_OBJECT (ogg, "seek on non seekable stream");
        goto error;
      }

      /* now do the seek */
      res = gst_ogg_demux_perform_seek (ogg, event);
      gst_event_unref (event);
      break;
    default:
      res = gst_pad_event_default (pad, event);
      break;
  }
done:
  gst_object_unref (ogg);

  return res;

  /* ERRORS */
error:
  {
    GST_DEBUG_OBJECT (ogg, "error handling event");
    gst_event_unref (event);
    res = FALSE;
    goto done;
  }
}

static void
gst_ogg_pad_reset (GstOggPad * pad)
{
  ogg_stream_reset (&pad->stream);

  GST_DEBUG_OBJECT (pad, "doing reset");

  /* clear continued pages */
  g_list_foreach (pad->continued, (GFunc) gst_ogg_page_free, NULL);
  g_list_free (pad->continued);
  pad->continued = NULL;

  pad->last_ret = GST_FLOW_OK;
}

/* the filter function for selecting the elements we can use in
 * autoplugging */
static gboolean
gst_ogg_demux_factory_filter (GstPluginFeature * feature, GstCaps * caps)
{
  guint rank;
  const gchar *klass;

  /* we only care about element factories */
  if (!GST_IS_ELEMENT_FACTORY (feature))
    return FALSE;

  klass = gst_element_factory_get_klass (GST_ELEMENT_FACTORY (feature));
  /* only demuxers and decoders can play */
  if (strstr (klass, "Demux") == NULL &&
      strstr (klass, "Decoder") == NULL && strstr (klass, "Parse") == NULL) {
    return FALSE;
  }

  /* only select elements with autoplugging rank */
  rank = gst_plugin_feature_get_rank (feature);
  if (rank < GST_RANK_MARGINAL)
    return FALSE;

  GST_DEBUG ("checking factory %s", GST_PLUGIN_FEATURE_NAME (feature));
  /* now see if it is compatible with the caps */
  {
    GstElementFactory *factory = GST_ELEMENT_FACTORY (feature);
    const GList *templates;
    GList *walk;

    /* get the templates from the element factory */
    templates = gst_element_factory_get_static_pad_templates (factory);

    for (walk = (GList *) templates; walk; walk = g_list_next (walk)) {
      GstStaticPadTemplate *templ = walk->data;

      /* we only care about the sink templates */
      if (templ->direction == GST_PAD_SINK) {
        GstCaps *intersect;
        GstCaps *scaps;
        gboolean empty;

        /* try to intersect the caps with the caps of the template */
        scaps = gst_static_caps_get (&templ->static_caps);
        intersect = gst_caps_intersect (caps, scaps);
        gst_caps_unref (scaps);

        empty = gst_caps_is_empty (intersect);
        gst_caps_unref (intersect);

        /* check if the intersection is empty */
        if (!empty) {
          /* non empty intersection, we can use this element */
          goto found;
        }
      }
    }
  }
  return FALSE;

found:
  return TRUE;
}

/* function used to sort element features */
static gint
compare_ranks (GstPluginFeature * f1, GstPluginFeature * f2)
{
  gint diff;

  diff = gst_plugin_feature_get_rank (f2) - gst_plugin_feature_get_rank (f1);
  if (diff != 0)
    return diff;
  return strcmp (gst_plugin_feature_get_name (f2),
      gst_plugin_feature_get_name (f1));
}

/* called when the skeleton fishead is found. Caller ensures the packet is
 * precisely the correct size; we don't re-check this here. */
static void
gst_ogg_pad_parse_skeleton_fishead (GstOggPad * pad, ogg_packet * packet)
{
  GstOggDemux *ogg = pad->ogg;
  guint8 *data = packet->packet;
  guint16 major, minor;
  gint64 prestime_n, prestime_d;
  gint64 basetime_n, basetime_d;

  /* skip "fishead\0" */
  data += 8;
  major = GST_READ_UINT16_LE (data);
  data += 2;
  minor = GST_READ_UINT16_LE (data);
  data += 2;
  prestime_n = (gint64) GST_READ_UINT64_LE (data);
  data += 8;
  prestime_d = (gint64) GST_READ_UINT64_LE (data);
  data += 8;
  basetime_n = (gint64) GST_READ_UINT64_LE (data);
  data += 8;
  basetime_d = (gint64) GST_READ_UINT64_LE (data);
  data += 8;

  ogg->basetime = gst_util_uint64_scale (GST_SECOND, basetime_n, basetime_d);
  ogg->prestime = gst_util_uint64_scale (GST_SECOND, prestime_n, prestime_d);
  ogg->have_fishead = TRUE;
  pad->is_skeleton = TRUE;
  pad->start_time = GST_CLOCK_TIME_NONE;
  pad->first_granule = -1;
  pad->first_time = GST_CLOCK_TIME_NONE;
  GST_INFO_OBJECT (ogg, "skeleton fishead parsed (basetime: %"
      GST_TIME_FORMAT ", prestime: %" GST_TIME_FORMAT ")",
      GST_TIME_ARGS (ogg->basetime), GST_TIME_ARGS (ogg->prestime));
}

/* function called when a skeleton fisbone is found. Caller ensures that
 * the packet length is sufficient */
static void
gst_ogg_pad_parse_skeleton_fisbone (GstOggPad * pad, ogg_packet * packet)
{
  GstOggDemux *ogg = pad->ogg;
  GstOggPad *fisbone_pad;
  gint64 start_granule;
  guint32 serialno;
  guint8 *data = packet->packet;

  /* skip "fisbone\0" */
  data += 8;
  /* skip headers offset */
  data += 4;
  serialno = GST_READ_UINT32_LE (data);

  fisbone_pad = gst_ogg_chain_get_stream (pad->chain, serialno);
  if (fisbone_pad) {
    if (fisbone_pad->have_fisbone)
      /* already parsed */
      return;

    fisbone_pad->have_fisbone = TRUE;

    data += 4;
    /* skip number of headers */
    data += 4;
    fisbone_pad->granulerate_n = GST_READ_UINT64_LE (data);
    data += 8;
    fisbone_pad->granulerate_d = GST_READ_UINT64_LE (data);
    data += 8;
    start_granule = GST_READ_UINT64_LE (data);
    data += 8;
    fisbone_pad->preroll = GST_READ_UINT32_LE (data);
    data += 4;
    fisbone_pad->granuleshift = GST_READ_UINT8 (data);
    data += 1;
    /* padding */
    data += 3;

    fisbone_pad->start_time = ogg->prestime - ogg->basetime;

    GST_INFO_OBJECT (pad->ogg, "skeleton fisbone parsed "
        "(serialno: %08x start time: %" GST_TIME_FORMAT
        " granulerate_n: %" G_GINT64_FORMAT " granulerate_d: %" G_GINT64_FORMAT
        " preroll: %" G_GUINT32_FORMAT " granuleshift: %d)",
        serialno, GST_TIME_ARGS (fisbone_pad->start_time),
        fisbone_pad->granulerate_n, fisbone_pad->granulerate_d,
        fisbone_pad->preroll, fisbone_pad->granuleshift);
  } else {
    GST_WARNING_OBJECT (pad->ogg,
        "found skeleton fisbone for an unknown stream %" G_GUINT32_FORMAT,
        serialno);
  }
}

/* function called to convert a granulepos to a timestamp */
static gboolean
gst_ogg_pad_query_convert (GstOggPad * pad, GstFormat src_format,
    gint64 src_val, GstFormat * dest_format, gint64 * dest_val)
{
  gboolean res;

  if (src_val == -1) {
    *dest_val = -1;
    return TRUE;
  }

  if (!pad->have_fisbone && pad->elem_pad == NULL)
    return FALSE;

  switch (src_format) {
    case GST_FORMAT_DEFAULT:
      if (pad->have_fisbone && *dest_format == GST_FORMAT_TIME) {
        *dest_val = gst_annodex_granule_to_time (src_val,
            pad->granulerate_n, pad->granulerate_d, pad->granuleshift);

        res = TRUE;
      } else {
        if (pad->elem_pad == NULL)
          res = FALSE;
        else
          res = gst_pad_query_convert (pad->elem_pad, src_format, src_val,
              dest_format, dest_val);
      }

      break;
    default:
      if (pad->elem_pad == NULL)
        res = FALSE;
      else
        res = gst_pad_query_convert (pad->elem_pad, src_format, src_val,
            dest_format, dest_val);
  }

  return res;
}

/* function called by the internal decoder elements when it outputs
 * a buffer. We use it to get the first timestamp of the stream 
 */
static GstFlowReturn
gst_ogg_pad_internal_chain (GstPad * pad, GstBuffer * buffer)
{
  GstOggPad *oggpad;
  GstOggDemux *ogg;
  GstClockTime timestamp;

  oggpad = gst_pad_get_element_private (pad);
  ogg = GST_OGG_DEMUX (oggpad->ogg);

  timestamp = GST_BUFFER_TIMESTAMP (buffer);
  GST_DEBUG_OBJECT (oggpad, "received buffer from internal pad, TS=%"
      GST_TIME_FORMAT "=%" G_GINT64_FORMAT, GST_TIME_ARGS (timestamp),
      timestamp);

  if (oggpad->start_time == GST_CLOCK_TIME_NONE) {
    oggpad->start_time = timestamp;
    GST_DEBUG_OBJECT (oggpad, "new start time: %" GST_TIME_FORMAT,
        GST_TIME_ARGS (timestamp));
  }

  gst_buffer_unref (buffer);

  return GST_FLOW_OK;
}

static void
internal_element_pad_added_cb (GstElement * element, GstPad * pad,
    GstOggPad * oggpad)
{
  if (GST_PAD_DIRECTION (pad) == GST_PAD_SRC) {
    if (!(gst_pad_link (pad, oggpad->elem_out) == GST_PAD_LINK_OK)) {
      GST_ERROR ("Really couldn't find a valid pad");
    }
    oggpad->dynamic = FALSE;
    g_signal_handler_disconnect (element, oggpad->padaddedid);
    oggpad->padaddedid = 0;
  }
}

/* runs typefind on the packet, which is assumed to be the first
 * packet in the stream.
 * 
 * Based on the type returned from the typefind function, an element
 * is created to help in conversion between granulepos and timestamps
 * so that we can do decent seeking.
 */
static gboolean
gst_ogg_pad_typefind (GstOggPad * pad, ogg_packet * packet)
{
  GstBuffer *buf;
  GstCaps *caps;
  GstElement *element = NULL;

#ifndef GST_DISABLE_GST_DEBUG
  GstOggDemux *ogg = pad->ogg;
#endif

  if (GST_PAD_CAPS (pad) != NULL)
    return TRUE;

  /* The ogg spec defines that the first packet of an ogg stream must identify
   * the stream. Therefore ogg can use a simplified approach to typefinding
   * and only needs to check the first packet */
  buf = gst_buffer_new ();
  GST_BUFFER_DATA (buf) = packet->packet;
  GST_BUFFER_SIZE (buf) = packet->bytes;
  GST_BUFFER_OFFSET (buf) = 0;

  caps = gst_type_find_helper_for_buffer (GST_OBJECT (pad), buf, NULL);
  gst_buffer_unref (buf);

  if (caps == NULL) {
    GST_WARNING_OBJECT (ogg,
        "couldn't find caps for stream with serial %08x", pad->serialno);
    caps = gst_caps_new_simple ("application/octet-stream", NULL);
  } else {
    /* ogg requires you to use a decoder element to define the
     * meaning of granulepos etc so we make one. We also do this if
     * we are in the streaming mode to calculate the first timestamp. */
    GList *factories;

    GST_LOG_OBJECT (ogg, "found caps: %" GST_PTR_FORMAT, caps);

    /* first filter out the interesting element factories */
    factories = gst_default_registry_feature_filter (
        (GstPluginFeatureFilter) gst_ogg_demux_factory_filter, FALSE, caps);

    /* sort them according to their ranks */
    factories = g_list_sort (factories, (GCompareFunc) compare_ranks);

    /* then pick the first factory to create an element */
    if (factories) {
      element =
          gst_element_factory_create (GST_ELEMENT_FACTORY (factories->data),
          NULL);
      if (element) {
        GstPadTemplate *template;

        /* this is ours */
        gst_object_ref (element);
        gst_object_sink (GST_OBJECT (element));

        /* FIXME, it might not be named "sink" */
        pad->elem_pad = gst_element_get_static_pad (element, "sink");
        gst_element_set_state (element, GST_STATE_PAUSED);
        template = gst_static_pad_template_get (&internaltemplate);
        pad->elem_out = gst_pad_new_from_template (template, "internal");
        gst_pad_set_chain_function (pad->elem_out, gst_ogg_pad_internal_chain);
        gst_pad_set_element_private (pad->elem_out, pad);
        gst_pad_set_active (pad->elem_out, TRUE);
        gst_object_unref (template);

        /* and this pad may not be named src.. */
        /* And it might also not exist at this time... */
        {
          GstPad *p;

          p = gst_element_get_static_pad (element, "src");
          if (p) {
            gst_pad_link (p, pad->elem_out);
            gst_object_unref (p);
          } else {
            pad->dynamic = TRUE;
            pad->padaddedid = g_signal_connect (G_OBJECT (element),
                "pad-added", G_CALLBACK (internal_element_pad_added_cb), pad);
          }
        }
      }
    }
    gst_plugin_feature_list_free (factories);
  }
  pad->element = element;

  gst_pad_set_caps (GST_PAD (pad), caps);
  gst_caps_unref (caps);

  return TRUE;
}

/* send packet to internal element */
static GstFlowReturn
gst_ogg_demux_chain_elem_pad (GstOggPad * pad, ogg_packet * packet)
{
  GstBuffer *buf;
  GstFlowReturn ret;

#ifndef GST_DISABLE_GST_DEBUG
  GstOggDemux *ogg = pad->ogg;
#endif

  /* initialize our internal decoder with packets */
  if (!pad->elem_pad)
    goto no_decoder;

  GST_DEBUG_OBJECT (ogg, "%p init decoder serial %08x", pad, pad->serialno);

  buf = gst_buffer_new_and_alloc (packet->bytes);
  memcpy (GST_BUFFER_DATA (buf), packet->packet, packet->bytes);
  gst_buffer_set_caps (buf, GST_PAD_CAPS (pad));
  GST_BUFFER_OFFSET (buf) = -1;
  GST_BUFFER_OFFSET_END (buf) = packet->granulepos;

  ret = gst_pad_chain (pad->elem_pad, buf);
  if (GST_FLOW_IS_FATAL (ret))
    goto decoder_error;

  return ret;

no_decoder:
  {
    GST_WARNING_OBJECT (ogg,
        "pad %p does not have elem_pad, no decoder ?", pad);
    return GST_FLOW_ERROR;
  }
decoder_error:
  {
    GST_WARNING_OBJECT (ogg, "internal decoder error");
    return ret;
  }
}

/* queue data, basically takes the packet, puts it in a buffer and store the
 * buffer in the headers list.
 */
static GstFlowReturn
gst_ogg_demux_queue_data (GstOggPad * pad, ogg_packet * packet)
{
  GstBuffer *buf;

#ifndef GST_DISABLE_GST_DEBUG
  GstOggDemux *ogg = pad->ogg;
#endif

  GST_DEBUG_OBJECT (ogg, "%p queueing data serial %08x", pad, pad->serialno);

  buf = gst_buffer_new_and_alloc (packet->bytes);
  memcpy (buf->data, packet->packet, packet->bytes);
  GST_BUFFER_OFFSET (buf) = -1;
  GST_BUFFER_OFFSET_END (buf) = packet->granulepos;
  pad->headers = g_list_append (pad->headers, buf);

  /* we are ok now */
  return GST_FLOW_OK;
}

/* send packet to internal element */
static GstFlowReturn
gst_ogg_demux_chain_peer (GstOggPad * pad, ogg_packet * packet)
{
  GstBuffer *buf;
  GstFlowReturn ret, cret;
  GstOggDemux *ogg = pad->ogg;
  GstFormat format;
  gint64 current_time;
  GstOggChain *chain;

  GST_DEBUG_OBJECT (ogg,
      "%p streaming to peer serial %08x", pad, pad->serialno);

  ret =
      gst_pad_alloc_buffer_and_set_caps (GST_PAD_CAST (pad),
      GST_BUFFER_OFFSET_NONE, packet->bytes, GST_PAD_CAPS (pad), &buf);

  /* combine flows */
  cret = gst_ogg_demux_combine_flows (ogg, pad, ret);
  if (ret != GST_FLOW_OK)
    goto no_buffer;

  /* copy packet in buffer */
  memcpy (buf->data, packet->packet, packet->bytes);

  GST_BUFFER_OFFSET (buf) = -1;
  GST_BUFFER_OFFSET_END (buf) = packet->granulepos;

  /* Mark discont on the buffer */
  if (pad->discont) {
    GST_BUFFER_FLAG_SET (buf, GST_BUFFER_FLAG_DISCONT);
    pad->discont = FALSE;
  }

  ret = gst_pad_push (GST_PAD_CAST (pad), buf);

  /* combine flows */
  cret = gst_ogg_demux_combine_flows (ogg, pad, ret);

  /* we're done with skeleton stuff */
  if (pad->is_skeleton)
    goto done;

  /* check if valid granulepos, then we can calculate the current
   * position */
  if (packet->granulepos < 0)
    goto done;

  /* store current granule pos */
  ogg->current_granule = packet->granulepos;

  /* convert to time */
  format = GST_FORMAT_TIME;
  if (!gst_ogg_pad_query_convert (pad,
          GST_FORMAT_DEFAULT, packet->granulepos, &format,
          (gint64 *) & current_time))
    goto convert_failed;

  /* convert to stream time */
  if ((chain = pad->chain)) {
    gint64 chain_start = 0;

    if (chain->segment_start != GST_CLOCK_TIME_NONE)
      chain_start = chain->segment_start;

    current_time = current_time - chain_start + chain->begin_time;
  }

  /* and store as the current position */
  gst_segment_set_last_stop (&ogg->segment, GST_FORMAT_TIME, current_time);

  GST_DEBUG_OBJECT (ogg, "ogg current time %" GST_TIME_FORMAT,
      GST_TIME_ARGS (current_time));

done:
  /* return combined flow result */
  return cret;

  /* special cases */
no_buffer:
  {
    GST_DEBUG_OBJECT (ogg,
        "%p could not get buffer from peer %08x, %d (%s), combined %d (%s)",
        pad, pad->serialno, ret, gst_flow_get_name (ret),
        cret, gst_flow_get_name (cret));
    goto done;
  }
convert_failed:
  {
    GST_WARNING_OBJECT (ogg, "could not convert granulepos to time");
    goto done;
  }
}

/* submit a packet to the oggpad, this function will run the
 * typefind code for the pad if this is the first packet for this
 * stream 
 */
static GstFlowReturn
gst_ogg_pad_submit_packet (GstOggPad * pad, ogg_packet * packet)
{
  gint64 granule;
  GstFlowReturn ret;

  GstOggDemux *ogg = pad->ogg;

  GST_DEBUG_OBJECT (ogg, "%p submit packet serial %08x", pad, pad->serialno);

  if (!pad->have_type) {
    if (!ogg->have_fishead && packet->bytes == SKELETON_FISHEAD_SIZE &&
        !memcmp (packet->packet, "fishead\0", 8)) {
      gst_ogg_pad_parse_skeleton_fishead (pad, packet);
    }
    gst_ogg_pad_typefind (pad, packet);
    pad->have_type = TRUE;
  }

  if (ogg->have_fishead && packet->bytes >= SKELETON_FISBONE_MIN_SIZE &&
      !memcmp (packet->packet, "fisbone\0", 8)) {
    gst_ogg_pad_parse_skeleton_fisbone (pad, packet);
  }

  granule = packet->granulepos;
  if (granule != -1) {
    GST_DEBUG_OBJECT (ogg, "%p has granulepos %" G_GINT64_FORMAT, pad, granule);
    ogg->current_granule = granule;
    pad->current_granule = granule;
    /* granulepos 0 and -1 are considered header packets.
     * Note that since theora is busted, it can create non-header
     * packets with 0 granulepos. We will correct for this when our
     * internal decoder produced a frame and we don't have a
     * granulepos because in that case the granulpos must have been 0 */
    if (pad->first_granule == -1 && granule != 0) {
      GST_DEBUG_OBJECT (ogg, "%p found first granulepos %" G_GINT64_FORMAT, pad,
          granule);
      pad->first_granule = granule;
    }
  }

  if (granule != -1 && memcmp (packet->packet, "KW-DIRAC", 8) == 0) {
    return GST_FLOW_OK;
  }

  /* no start time known, stream to internal plugin to
   * get time. always stream to the skeleton decoder */
  if (pad->start_time == GST_CLOCK_TIME_NONE || pad->is_skeleton) {
    ret = gst_ogg_demux_chain_elem_pad (pad, packet);
  }
  /* we know the start_time of the pad data, see if we
   * can activate the complete chain if this is a dynamic
   * chain. */
  if (pad->start_time != GST_CLOCK_TIME_NONE) {
    GstOggChain *chain = pad->chain;

    /* correction for busted ogg, if the internal decoder outputed
     * a timestamp but we did not get a granulepos, it must have
     * been 0 and the time is therefore also 0 */
    if (pad->first_granule == -1) {
      GST_DEBUG_OBJECT (ogg, "%p found start time without granulepos", pad);
      pad->first_granule = 0;
      pad->first_time = 0;
    }

    /* check if complete chain has start time */
    if (chain == ogg->building_chain) {

      /* see if we have enough info to activate the chain, we have enough info
       * when all streams have a valid start time. */
      if (gst_ogg_demux_collect_chain_info (ogg, chain)) {
        GstEvent *event;

        GST_DEBUG_OBJECT (ogg, "segment_start: %" GST_TIME_FORMAT,
            GST_TIME_ARGS (chain->segment_start));
        GST_DEBUG_OBJECT (ogg, "segment_stop:  %" GST_TIME_FORMAT,
            GST_TIME_ARGS (chain->segment_stop));
        GST_DEBUG_OBJECT (ogg, "segment_time:  %" GST_TIME_FORMAT,
            GST_TIME_ARGS (chain->begin_time));

        /* create the newsegment event we are going to send out */
        event = gst_event_new_new_segment (FALSE, ogg->segment.rate,
            GST_FORMAT_TIME, chain->segment_start, chain->segment_stop,
            chain->begin_time);
        gst_event_set_seqnum (event, ogg->seqnum);

        gst_ogg_demux_activate_chain (ogg, chain, event);

        ogg->building_chain = NULL;
      }
    }
  }

  /* if we are building a chain, store buffer for when we activate
   * it. This path is taken if we operate in streaming mode. */
  if (ogg->building_chain) {
    ret = gst_ogg_demux_queue_data (pad, packet);
  }
  /* else we are completely streaming to the peer */
  else {
    ret = gst_ogg_demux_chain_peer (pad, packet);
  }
  return ret;
}

/* flush at most @npackets from the stream layer. All packets if 
 * @npackets is 0;
 */
static GstFlowReturn
gst_ogg_pad_stream_out (GstOggPad * pad, gint npackets)
{
  GstFlowReturn result = GST_FLOW_OK;
  gboolean done = FALSE;
  GstOggDemux *ogg;

  ogg = pad->ogg;

  while (!done) {
    int ret;
    ogg_packet packet;

    ret = ogg_stream_packetout (&pad->stream, &packet);
    switch (ret) {
      case 0:
        GST_LOG_OBJECT (ogg, "packetout done");
        done = TRUE;
        break;
      case -1:
        GST_LOG_OBJECT (ogg, "packetout discont");
        gst_ogg_chain_mark_discont (pad->chain);
        break;
      case 1:
        GST_LOG_OBJECT (ogg, "packetout gave packet of size %ld", packet.bytes);
        result = gst_ogg_pad_submit_packet (pad, &packet);
        if (GST_FLOW_IS_FATAL (result))
          goto could_not_submit;
        break;
      default:
        GST_WARNING_OBJECT (ogg,
            "invalid return value %d for ogg_stream_packetout, resetting stream",
            ret);
        gst_ogg_pad_reset (pad);
        break;
    }
    if (npackets > 0) {
      npackets--;
      done = (npackets == 0);
    }
  }
  return result;

  /* ERRORS */
could_not_submit:
  {
    GST_WARNING_OBJECT (ogg,
        "could not submit packet for stream %08x, error: %d", pad->serialno,
        result);
    gst_ogg_pad_reset (pad);
    return result;
  }
}

/* submit a page to an oggpad, this function will then submit all
 * the packets in the page.
 */
static GstFlowReturn
gst_ogg_pad_submit_page (GstOggPad * pad, ogg_page * page)
{
  GstFlowReturn result = GST_FLOW_OK;
  GstOggDemux *ogg;
  gboolean continued = FALSE;

  ogg = pad->ogg;

  /* for negative rates we read pages backwards and must therefore be carefull
   * with continued pages */
  if (ogg->segment.rate < 0.0) {
    gint npackets;

    continued = ogg_page_continued (page);

    /* number of completed packets in the page */
    npackets = ogg_page_packets (page);
    if (!continued) {
      /* page is not continued so it contains at least one packet start. It's
       * possible that no packet ends on this page (npackets == 0). In that
       * case, the next (continued) page(s) we kept contain the remainder of the
       * packets. We mark npackets=1 to make us start decoding the pages in the
       * remainder of the algorithm. */
      if (npackets == 0)
        npackets = 1;
    }
    GST_LOG_OBJECT (ogg, "continued: %d, %d packets", continued, npackets);

    if (npackets == 0) {
      GST_LOG_OBJECT (ogg, "no decodable packets, we need a previous page");
      goto done;
    }
  }

  if (ogg_stream_pagein (&pad->stream, page) != 0)
    goto choked;

  /* flush all packets in the stream layer, this might not give a packet if
   * the page had no packets finishing on the page (npackets == 0). */
  result = gst_ogg_pad_stream_out (pad, 0);

  if (pad->continued) {
    ogg_packet packet;

    /* now send the continued pages to the stream layer */
    while (pad->continued) {
      ogg_page *p = (ogg_page *) pad->continued->data;

      GST_LOG_OBJECT (ogg, "submitting continued page %p", p);
      if (ogg_stream_pagein (&pad->stream, p) != 0)
        goto choked;

      pad->continued = g_list_delete_link (pad->continued, pad->continued);

      /* free the page */
      gst_ogg_page_free (p);
    }

    GST_LOG_OBJECT (ogg, "flushing last continued packet");
    /* flush 1 continued packet in the stream layer */
    result = gst_ogg_pad_stream_out (pad, 1);

    /* flush all remaining packets, we pushed them in the previous round.
     * We don't use _reset() because we still want to get the discont when
     * we submit a next page. */
    while (ogg_stream_packetout (&pad->stream, &packet) != 0);
  }

done:
  /* keep continued pages (only in reverse mode) */
  if (continued) {
    ogg_page *p = gst_ogg_page_copy (page);

    GST_LOG_OBJECT (ogg, "keeping continued page %p", p);
    pad->continued = g_list_prepend (pad->continued, p);
  }

  return result;

choked:
  {
    GST_WARNING_OBJECT (ogg,
        "ogg stream choked on page (serial %08x), resetting stream",
        pad->serialno);
    gst_ogg_pad_reset (pad);
    /* we continue to recover */
    return GST_FLOW_OK;
  }
}


static GstOggChain *
gst_ogg_chain_new (GstOggDemux * ogg)
{
  GstOggChain *chain = g_new0 (GstOggChain, 1);

  GST_DEBUG_OBJECT (ogg, "creating new chain %p", chain);
  chain->ogg = ogg;
  chain->offset = -1;
  chain->bytes = -1;
  chain->have_bos = FALSE;
  chain->streams = g_array_new (FALSE, TRUE, sizeof (GstOggPad *));
  chain->begin_time = GST_CLOCK_TIME_NONE;
  chain->segment_start = GST_CLOCK_TIME_NONE;
  chain->segment_stop = GST_CLOCK_TIME_NONE;
  chain->total_time = GST_CLOCK_TIME_NONE;

  return chain;
}

static void
gst_ogg_chain_free (GstOggChain * chain)
{
  gint i;

  for (i = 0; i < chain->streams->len; i++) {
    GstOggPad *pad = g_array_index (chain->streams, GstOggPad *, i);

    gst_object_unref (pad);
  }
  g_array_free (chain->streams, TRUE);
  g_free (chain);
}

static void
gst_ogg_chain_mark_discont (GstOggChain * chain)
{
  gint i;

  for (i = 0; i < chain->streams->len; i++) {
    GstOggPad *pad = g_array_index (chain->streams, GstOggPad *, i);

    pad->discont = TRUE;
  }
}

static void
gst_ogg_chain_reset (GstOggChain * chain)
{
  gint i;

  for (i = 0; i < chain->streams->len; i++) {
    GstOggPad *pad = g_array_index (chain->streams, GstOggPad *, i);

    gst_ogg_pad_reset (pad);
  }
}

static GstOggPad *
gst_ogg_chain_new_stream (GstOggChain * chain, glong serialno)
{
  GstOggPad *ret;
  GstTagList *list;
  gchar *name;

  GST_DEBUG_OBJECT (chain->ogg, "creating new stream %08lx in chain %p",
      serialno, chain);

  ret = g_object_new (GST_TYPE_OGG_PAD, NULL);
  /* we own this one */
  gst_object_ref (ret);
  gst_object_sink (ret);

  GST_PAD_DIRECTION (ret) = GST_PAD_SRC;
  ret->discont = TRUE;

  ret->chain = chain;
  ret->ogg = chain->ogg;

  ret->serialno = serialno;
  if (ogg_stream_init (&ret->stream, serialno) != 0)
    goto init_failed;

  name = g_strdup_printf ("serial_%08lx", serialno);
  gst_object_set_name (GST_OBJECT (ret), name);
  g_free (name);

  /* FIXME: either do something with it or remove it */
  list = gst_tag_list_new ();
  gst_tag_list_add (list, GST_TAG_MERGE_REPLACE, GST_TAG_SERIAL, serialno,
      NULL);
  gst_tag_list_free (list);

  GST_DEBUG_OBJECT (chain->ogg,
      "created new ogg src %p for stream with serial %08lx", ret, serialno);

  g_array_append_val (chain->streams, ret);

  return ret;

  /* ERRORS */
init_failed:
  {
    GST_ERROR ("Could not initialize ogg_stream struct for serial %08lx.",
        serialno);
    gst_object_unref (ret);
    return NULL;
  }
}

static GstOggPad *
gst_ogg_chain_get_stream (GstOggChain * chain, glong serialno)
{
  gint i;

  for (i = 0; i < chain->streams->len; i++) {
    GstOggPad *pad = g_array_index (chain->streams, GstOggPad *, i);

    if (pad->serialno == serialno)
      return pad;
  }
  return NULL;
}

static gboolean
gst_ogg_chain_has_stream (GstOggChain * chain, glong serialno)
{
  return gst_ogg_chain_get_stream (chain, serialno) != NULL;
}

#define CURRENT_CHAIN(ogg) (&g_array_index ((ogg)->chains, GstOggChain, (ogg)->current_chain))

/* signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  ARG_0
      /* FILL ME */
};

static GstStaticPadTemplate ogg_demux_src_template_factory =
GST_STATIC_PAD_TEMPLATE ("src_%d",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate ogg_demux_sink_template_factory =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/ogg; application/x-annodex")
    );

static void gst_ogg_demux_finalize (GObject * object);

//static const GstEventMask *gst_ogg_demux_get_event_masks (GstPad * pad);
//static const GstQueryType *gst_ogg_demux_get_query_types (GstPad * pad);
static GstFlowReturn gst_ogg_demux_read_chain (GstOggDemux * ogg,
    GstOggChain ** chain);
static GstFlowReturn gst_ogg_demux_read_end_chain (GstOggDemux * ogg,
    GstOggChain * chain);

static gboolean gst_ogg_demux_sink_event (GstPad * pad, GstEvent * event);
static void gst_ogg_demux_loop (GstOggPad * pad);
static GstFlowReturn gst_ogg_demux_chain (GstPad * pad, GstBuffer * buffer);
static gboolean gst_ogg_demux_sink_activate (GstPad * sinkpad);
static gboolean gst_ogg_demux_sink_activate_pull (GstPad * sinkpad,
    gboolean active);
static gboolean gst_ogg_demux_sink_activate_push (GstPad * sinkpad,
    gboolean active);
static GstStateChangeReturn gst_ogg_demux_change_state (GstElement * element,
    GstStateChange transition);
static void gst_ogg_demux_send_event (GstOggDemux * ogg, GstEvent * event);

static void gst_ogg_print (GstOggDemux * demux);

GST_BOILERPLATE (GstOggDemux, gst_ogg_demux, GstElement, GST_TYPE_ELEMENT);

static void
gst_ogg_demux_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (element_class, &gst_ogg_demux_details);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&ogg_demux_sink_template_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&ogg_demux_src_template_factory));
}

static void
gst_ogg_demux_class_init (GstOggDemuxClass * klass)
{
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gstelement_class->change_state = gst_ogg_demux_change_state;
  gstelement_class->send_event = gst_ogg_demux_receive_event;

  gobject_class->finalize = gst_ogg_demux_finalize;
}

static void
gst_ogg_demux_init (GstOggDemux * ogg, GstOggDemuxClass * g_class)
{
  /* create the sink pad */
  ogg->sinkpad =
      gst_pad_new_from_static_template (&ogg_demux_sink_template_factory,
      "sink");

  gst_pad_set_event_function (ogg->sinkpad, gst_ogg_demux_sink_event);
  gst_pad_set_chain_function (ogg->sinkpad, gst_ogg_demux_chain);
  gst_pad_set_activate_function (ogg->sinkpad, gst_ogg_demux_sink_activate);
  gst_pad_set_activatepull_function (ogg->sinkpad,
      gst_ogg_demux_sink_activate_pull);
  gst_pad_set_activatepush_function (ogg->sinkpad,
      gst_ogg_demux_sink_activate_push);
  gst_element_add_pad (GST_ELEMENT (ogg), ogg->sinkpad);

  ogg->chain_lock = g_mutex_new ();
  ogg->chains = g_array_new (FALSE, TRUE, sizeof (GstOggChain *));

  ogg->newsegment = NULL;
}

static void
gst_ogg_demux_finalize (GObject * object)
{
  GstOggDemux *ogg;

  ogg = GST_OGG_DEMUX (object);

  g_array_free (ogg->chains, TRUE);
  g_mutex_free (ogg->chain_lock);
  ogg_sync_clear (&ogg->sync);

  if (ogg->newsegment)
    gst_event_unref (ogg->newsegment);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gst_ogg_demux_sink_event (GstPad * pad, GstEvent * event)
{
  gboolean res;
  GstOggDemux *ogg;

  ogg = GST_OGG_DEMUX (gst_pad_get_parent (pad));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_NEWSEGMENT:
      /* FIXME */
      GST_DEBUG_OBJECT (ogg, "got a new segment event");
      ogg_sync_reset (&ogg->sync);
      gst_event_unref (event);
      res = TRUE;
      break;
    case GST_EVENT_EOS:
    {
      GST_DEBUG_OBJECT (ogg, "got an EOS event");
      res = gst_pad_event_default (pad, event);
      if (ogg->current_chain == NULL) {
        GST_ELEMENT_ERROR (ogg, STREAM, DEMUX, (NULL),
            ("can't get first chain"));
      }
      break;
    }
    default:
      res = gst_pad_event_default (pad, event);
      break;
  }
  gst_object_unref (ogg);

  return res;
}

/* submit the given buffer to the ogg sync.
 *
 * Returns the number of bytes submited.
 */
static GstFlowReturn
gst_ogg_demux_submit_buffer (GstOggDemux * ogg, GstBuffer * buffer)
{
  gint size;
  guint8 *data;
  gchar *oggbuffer;
  GstFlowReturn ret = GST_FLOW_OK;

  size = GST_BUFFER_SIZE (buffer);
  data = GST_BUFFER_DATA (buffer);

  GST_DEBUG_OBJECT (ogg, "submitting %u bytes", size);
  if (G_UNLIKELY (size == 0))
    goto done;

  oggbuffer = ogg_sync_buffer (&ogg->sync, size);
  if (G_UNLIKELY (oggbuffer == NULL))
    goto no_buffer;

  memcpy (oggbuffer, data, size);
  if (G_UNLIKELY (ogg_sync_wrote (&ogg->sync, size) < 0))
    goto write_failed;

done:
  gst_buffer_unref (buffer);

  return ret;

  /* ERRORS */
no_buffer:
  {
    GST_ELEMENT_ERROR (ogg, STREAM, DECODE,
        (NULL), ("failed to get ogg sync buffer"));
    ret = GST_FLOW_ERROR;
    goto done;
  }
write_failed:
  {
    GST_ELEMENT_ERROR (ogg, STREAM, DECODE,
        (NULL), ("failed to write %d bytes to the sync buffer", size));
    ret = GST_FLOW_ERROR;
    goto done;
  }
}

/* in random access mode this code updates the current read position
 * and resets the ogg sync buffer so that the next read will happen
 * from this new location.
 */
static void
gst_ogg_demux_seek (GstOggDemux * ogg, gint64 offset)
{
  GST_LOG_OBJECT (ogg, "seeking to %" G_GINT64_FORMAT, offset);

  ogg->offset = offset;
  ogg_sync_reset (&ogg->sync);
}

/* read more data from the current offset and submit to
 * the ogg sync layer.
 */
static GstFlowReturn
gst_ogg_demux_get_data (GstOggDemux * ogg)
{
  GstFlowReturn ret;
  GstBuffer *buffer;

  GST_LOG_OBJECT (ogg, "get data %" G_GINT64_FORMAT " %" G_GINT64_FORMAT,
      ogg->offset, ogg->length);
  if (ogg->offset == ogg->length)
    goto eos;

  ret = gst_pad_pull_range (ogg->sinkpad, ogg->offset, CHUNKSIZE, &buffer);
  if (ret != GST_FLOW_OK)
    goto error;

  ret = gst_ogg_demux_submit_buffer (ogg, buffer);

  return ret;

  /* ERROR */
eos:
  {
    GST_LOG_OBJECT (ogg, "reached EOS");
    return GST_FLOW_UNEXPECTED;
  }
error:
  {
    GST_WARNING_OBJECT (ogg, "got %d (%s) from pull range", ret,
        gst_flow_get_name (ret));
    return ret;
  }
}

/* Read the next page from the current offset.
 * boundary: number of bytes ahead we allow looking for;
 * -1 if no boundary
 *
 * @offset will contain the offset the next page starts at when this function
 * returns GST_FLOW_OK.
 *
 * GST_FLOW_UNEXPECTED is returned on EOS.
 *
 * GST_FLOW_LIMIT is returned when we did not find a page before the
 * boundary. If @boundary is -1, this is never returned.
 *
 * Any other error returned while retrieving data from the peer is returned as
 * is.
 */
static GstFlowReturn
gst_ogg_demux_get_next_page (GstOggDemux * ogg, ogg_page * og, gint64 boundary,
    gint64 * offset)
{
  gint64 end_offset = 0;
  GstFlowReturn ret;

  GST_LOG_OBJECT (ogg,
      "get next page, current offset %" G_GINT64_FORMAT ", bytes boundary %"
      G_GINT64_FORMAT, ogg->offset, boundary);

  if (boundary > 0)
    end_offset = ogg->offset + boundary;

  while (TRUE) {
    glong more;

    if (boundary > 0 && ogg->offset >= end_offset)
      goto boundary_reached;

    more = ogg_sync_pageseek (&ogg->sync, og);

    GST_LOG_OBJECT (ogg, "pageseek gave %ld", more);

    if (more < 0) {
      /* skipped n bytes */
      GST_LOG_OBJECT (ogg, "skipped %ld bytes", more);
      ogg->offset -= more;
    } else if (more == 0) {
      /* we need more data */
      if (boundary == 0)
        goto boundary_reached;

      GST_LOG_OBJECT (ogg, "need more data");
      ret = gst_ogg_demux_get_data (ogg);
      if (ret != GST_FLOW_OK)
        break;
    } else {
      gint64 res_offset = ogg->offset;

      /* got a page.  Return the offset at the page beginning,
         advance the internal offset past the page end */
      if (offset)
        *offset = res_offset;
      ret = GST_FLOW_OK;

      ogg->offset += more;
      /* need to reset as we do not keep track of the bytes we
       * sent to the sync layer */
      ogg_sync_reset (&ogg->sync);

      GST_LOG_OBJECT (ogg,
          "got page at %" G_GINT64_FORMAT ", serial %08x, end at %"
          G_GINT64_FORMAT ", granule %" G_GINT64_FORMAT, res_offset,
          ogg_page_serialno (og), ogg->offset, ogg_page_granulepos (og));
      break;
    }
  }
  GST_LOG_OBJECT (ogg, "returning %d", ret);

  return ret;

  /* ERRORS */
boundary_reached:
  {
    GST_LOG_OBJECT (ogg,
        "offset %" G_GINT64_FORMAT " >= end_offset %" G_GINT64_FORMAT,
        ogg->offset, end_offset);
    return GST_FLOW_LIMIT;
  }
}

/* from the current offset, find the previous page, seeking backwards
 * until we find the page. 
 */
static GstFlowReturn
gst_ogg_demux_get_prev_page (GstOggDemux * ogg, ogg_page * og, gint64 * offset)
{
  GstFlowReturn ret;
  gint64 begin = ogg->offset;
  gint64 end = begin;
  gint64 cur_offset = -1;

  while (cur_offset == -1) {
    begin -= CHUNKSIZE;
    if (begin < 0)
      begin = 0;

    /* seek CHUNKSIZE back */
    gst_ogg_demux_seek (ogg, begin);

    /* now continue reading until we run out of data, if we find a page
     * start, we save it. It might not be the final page as there could be
     * another page after this one. */
    while (ogg->offset < end) {
      gint64 new_offset;

      ret =
          gst_ogg_demux_get_next_page (ogg, og, end - ogg->offset, &new_offset);
      /* we hit the upper limit, offset contains the last page start */
      if (ret == GST_FLOW_LIMIT)
        break;
      /* something went wrong */
      if (ret == GST_FLOW_UNEXPECTED)
        new_offset = 0;
      else if (ret != GST_FLOW_OK)
        return ret;

      /* offset is next page start */
      cur_offset = new_offset;
    }
  }

  /* we have the offset.  Actually snork and hold the page now */
  gst_ogg_demux_seek (ogg, cur_offset);
  ret = gst_ogg_demux_get_next_page (ogg, og, -1, NULL);
  if (ret != GST_FLOW_OK)
    /* this shouldn't be possible */
    return ret;

  if (offset)
    *offset = cur_offset;

  return ret;
}

static gboolean
gst_ogg_demux_deactivate_current_chain (GstOggDemux * ogg)
{
  gint i;
  GstOggChain *chain = ogg->current_chain;

  if (chain == NULL)
    return TRUE;

  GST_DEBUG_OBJECT (ogg, "deactivating chain %p", chain);

  /* send EOS on all the pads */
  for (i = 0; i < chain->streams->len; i++) {
    GstOggPad *pad = g_array_index (chain->streams, GstOggPad *, i);
    GstEvent *event;

    if (pad->is_skeleton)
      continue;

    event = gst_event_new_eos ();
    gst_event_set_seqnum (event, ogg->seqnum);
    gst_pad_push_event (GST_PAD_CAST (pad), event);

    GST_DEBUG_OBJECT (ogg, "removing pad %" GST_PTR_FORMAT, pad);

    /* deactivate first */
    gst_pad_set_active (GST_PAD_CAST (pad), FALSE);

    gst_element_remove_pad (GST_ELEMENT (ogg), GST_PAD_CAST (pad));
  }
  /* if we cannot seek back to the chain, we can destroy the chain 
   * completely */
  if (!ogg->seekable) {
    gst_ogg_chain_free (chain);
  }
  ogg->current_chain = NULL;

  return TRUE;
}

static gboolean
gst_ogg_demux_activate_chain (GstOggDemux * ogg, GstOggChain * chain,
    GstEvent * event)
{
  gint i;

  if (chain == ogg->current_chain) {
    if (event)
      gst_event_unref (event);
    return TRUE;
  }

  /* FIXME, should not be called with NULL */
  if (chain != NULL) {
    GST_DEBUG_OBJECT (ogg, "activating chain %p", chain);

    /* first add the pads */
    for (i = 0; i < chain->streams->len; i++) {
      GstOggPad *pad;

      pad = g_array_index (chain->streams, GstOggPad *, i);

      if (pad->is_skeleton)
        continue;

      GST_DEBUG_OBJECT (ogg, "adding pad %" GST_PTR_FORMAT, pad);

      /* mark discont */
      pad->discont = TRUE;
      pad->last_ret = GST_FLOW_OK;

      /* activate first */
      gst_pad_set_active (GST_PAD_CAST (pad), TRUE);

      gst_element_add_pad (GST_ELEMENT (ogg), GST_PAD_CAST (pad));
    }
  }

  /* after adding the new pads, remove the old pads */
  gst_ogg_demux_deactivate_current_chain (ogg);

  ogg->current_chain = chain;

  /* we are finished now */
  gst_element_no_more_pads (GST_ELEMENT (ogg));

  /* FIXME, must be sent from the streaming thread */
  if (event)
    gst_ogg_demux_send_event (ogg, event);

  GST_DEBUG_OBJECT (ogg, "starting chain");

  /* then send out any queued buffers */
  for (i = 0; i < chain->streams->len; i++) {
    GList *headers;
    GstOggPad *pad;

    pad = g_array_index (chain->streams, GstOggPad *, i);

    for (headers = pad->headers; headers; headers = g_list_next (headers)) {
      GstBuffer *buffer = GST_BUFFER (headers->data);

      if (pad->discont) {
        GST_BUFFER_FLAG_SET (buffer, GST_BUFFER_FLAG_DISCONT);
        pad->discont = FALSE;
      }

      /* we don't care about the return value here */
      gst_pad_push (GST_PAD_CAST (pad), buffer);
    }
    /* and free the headers */
    g_list_free (pad->headers);
    pad->headers = NULL;
  }
  return TRUE;
}

/* 
 * do seek to time @position, return FALSE or chain and TRUE
 */
static gboolean
gst_ogg_demux_do_seek (GstOggDemux * ogg, gint64 position, gboolean accurate,
    GstOggChain ** rchain)
{
  GstOggChain *chain = NULL;
  gint64 begin, end;
  gint64 begintime, endtime;
  gint64 target;
  gint64 best;
  gint64 total;
  gint64 result = 0;
  GstFlowReturn ret;
  gint i;

  /* first find the chain to search in */
  total = ogg->total_time;
  if (ogg->chains->len == 0)
    goto no_chains;

  for (i = ogg->chains->len - 1; i >= 0; i--) {
    chain = g_array_index (ogg->chains, GstOggChain *, i);
    total -= chain->total_time;
    if (position >= total)
      break;
  }

  begin = chain->offset;
  end = chain->end_offset;
  begintime = chain->begin_time;
  endtime = chain->begin_time + chain->total_time;
  target = position - total + begintime;
  if (accurate) {
    /* FIXME, seek 4 seconds early to catch keyframes, better implement
     * keyframe detection. */
    target = target - (gint64) 4 *GST_SECOND;
  }
  target = MAX (target, 0);
  best = begin;

  GST_DEBUG_OBJECT (ogg,
      "seeking to %" GST_TIME_FORMAT " in chain %p",
      GST_TIME_ARGS (position), chain);
  GST_DEBUG_OBJECT (ogg,
      "chain offset %" G_GINT64_FORMAT ", end offset %" G_GINT64_FORMAT, begin,
      end);
  GST_DEBUG_OBJECT (ogg,
      "chain begin time %" GST_TIME_FORMAT ", end time %" GST_TIME_FORMAT,
      GST_TIME_ARGS (begintime), GST_TIME_ARGS (endtime));
  GST_DEBUG_OBJECT (ogg, "target %" GST_TIME_FORMAT, GST_TIME_ARGS (target));

  /* perform the seek */
  while (begin < end) {
    gint64 bisect;

    if ((end - begin < CHUNKSIZE) || (endtime == begintime)) {
      bisect = begin;
    } else {
      /* take a (pretty decent) guess, avoiding overflow */
      gint64 rate = (end - begin) * GST_MSECOND / (endtime - begintime);

      bisect = (target - begintime) / GST_MSECOND * rate + begin - CHUNKSIZE;

      if (bisect <= begin)
        bisect = begin;
      GST_DEBUG_OBJECT (ogg, "Initial guess: %" G_GINT64_FORMAT, bisect);
    }
    gst_ogg_demux_seek (ogg, bisect);

    while (begin < end) {
      ogg_page og;

      GST_DEBUG_OBJECT (ogg,
          "after seek, bisect %" G_GINT64_FORMAT ", begin %" G_GINT64_FORMAT
          ", end %" G_GINT64_FORMAT, bisect, begin, end);

      ret = gst_ogg_demux_get_next_page (ogg, &og, end - ogg->offset, &result);
      GST_LOG_OBJECT (ogg, "looking for next page returned %" G_GINT64_FORMAT,
          result);

      if (ret == GST_FLOW_LIMIT) {
        /* we hit the upper limit, go back a bit */
        if (bisect <= begin + 1) {
          end = begin;          /* found it */
        } else {
          if (bisect == 0)
            goto seek_error;

          bisect -= CHUNKSIZE;
          if (bisect <= begin)
            bisect = begin + 1;

          gst_ogg_demux_seek (ogg, bisect);
        }
      } else if (ret == GST_FLOW_OK) {
        /* found offset of next ogg page */
        gint64 granulepos;
        GstClockTime granuletime;
        GstFormat format;
        GstOggPad *pad;

        GST_LOG_OBJECT (ogg, "found next ogg page at %" G_GINT64_FORMAT,
            result);
        granulepos = ogg_page_granulepos (&og);
        if (granulepos == -1) {
          GST_LOG_OBJECT (ogg, "granulepos of next page is -1");
          continue;
        }

        pad = gst_ogg_chain_get_stream (chain, ogg_page_serialno (&og));
        if (pad == NULL || pad->is_skeleton)
          continue;

        format = GST_FORMAT_TIME;
        if (!gst_ogg_pad_query_convert (pad,
                GST_FORMAT_DEFAULT, granulepos, &format,
                (gint64 *) & granuletime)) {
          GST_WARNING_OBJECT (ogg, "could not convert granulepos to time");
          granuletime = target;
        } else {
          if (granuletime < pad->start_time)
            continue;
          GST_LOG_OBJECT (ogg, "granulepos %" G_GINT64_FORMAT " maps to time %"
              GST_TIME_FORMAT, granulepos, GST_TIME_ARGS (granuletime));

          granuletime -= pad->start_time;
        }

        GST_DEBUG_OBJECT (ogg,
            "found page with granule %" G_GINT64_FORMAT " and time %"
            GST_TIME_FORMAT, granulepos, GST_TIME_ARGS (granuletime));

        if (granuletime < target) {
          best = result;        /* raw offset of packet with granulepos */
          begin = ogg->offset;  /* raw offset of next page */
          begintime = granuletime;

          if (target - begintime > GST_SECOND)
            break;

          bisect = begin;       /* *not* begin + 1 */
        } else {
          if (bisect <= begin + 1) {
            end = begin;        /* found it */
          } else {
            if (end == ogg->offset) {   /* we're pretty close - we'd be stuck in */
              end = result;
              bisect -= CHUNKSIZE;      /* an endless loop otherwise. */
              if (bisect <= begin)
                bisect = begin + 1;
              gst_ogg_demux_seek (ogg, bisect);
            } else {
              end = result;
              endtime = granuletime;
              break;
            }
          }
        }
      } else
        goto seek_error;
    }
  }

  ogg->offset = best;
  *rchain = chain;

  return TRUE;

no_chains:
  {
    GST_DEBUG_OBJECT (ogg, "no chains");
    return FALSE;
  }
seek_error:
  {
    GST_DEBUG_OBJECT (ogg, "got a seek error");
    return FALSE;
  }
}

/* does not take ownership of the event */
static gboolean
gst_ogg_demux_perform_seek (GstOggDemux * ogg, GstEvent * event)
{
  GstOggChain *chain = NULL;
  gboolean res;
  gboolean flush, accurate;
  GstFormat format;
  gdouble rate;
  GstSeekFlags flags;
  GstSeekType cur_type, stop_type;
  gint64 cur, stop;
  gboolean update;
  guint32 seqnum;
  GstEvent *tevent;

  if (event) {
    GST_DEBUG_OBJECT (ogg, "seek with event");

    gst_event_parse_seek (event, &rate, &format, &flags,
        &cur_type, &cur, &stop_type, &stop);

    /* we can only seek on time */
    if (format != GST_FORMAT_TIME) {
      GST_DEBUG_OBJECT (ogg, "can only seek on TIME");
      goto error;
    }
    seqnum = gst_event_get_seqnum (event);
  } else {
    GST_DEBUG_OBJECT (ogg, "seek without event");

    flags = 0;
    rate = 1.0;
    seqnum = gst_util_seqnum_next ();
  }

  GST_DEBUG_OBJECT (ogg, "seek, rate %g", rate);

  flush = flags & GST_SEEK_FLAG_FLUSH;
  accurate = flags & GST_SEEK_FLAG_ACCURATE;

  /* first step is to unlock the streaming thread if it is
   * blocked in a chain call, we do this by starting the flush. because
   * we cannot yet hold any streaming lock, we have to protect the chains
   * with their own lock. */
  if (flush) {
    gint i;

    tevent = gst_event_new_flush_start ();
    gst_event_set_seqnum (tevent, seqnum);

    gst_event_ref (tevent);
    gst_pad_push_event (ogg->sinkpad, tevent);

    GST_CHAIN_LOCK (ogg);
    for (i = 0; i < ogg->chains->len; i++) {
      GstOggChain *chain = g_array_index (ogg->chains, GstOggChain *, i);
      gint j;

      for (j = 0; j < chain->streams->len; j++) {
        GstOggPad *pad = g_array_index (chain->streams, GstOggPad *, j);

        gst_event_ref (tevent);
        gst_pad_push_event (GST_PAD (pad), tevent);
      }
    }
    GST_CHAIN_UNLOCK (ogg);

    gst_event_unref (tevent);
  } else {
    gst_pad_pause_task (ogg->sinkpad);
  }

  /* now grab the stream lock so that streaming cannot continue, for
   * non flushing seeks when the element is in PAUSED this could block
   * forever. */
  GST_PAD_STREAM_LOCK (ogg->sinkpad);

  if (ogg->segment_running && !flush) {
    /* create the segment event to close the current segment */
    if ((chain = ogg->current_chain)) {
      GstEvent *newseg;
      gint64 chain_start = 0;

      if (chain->segment_start != GST_CLOCK_TIME_NONE)
        chain_start = chain->segment_start;

      newseg = gst_event_new_new_segment (TRUE, ogg->segment.rate,
          GST_FORMAT_TIME, ogg->segment.start + chain_start,
          ogg->segment.last_stop + chain_start, ogg->segment.time);
      /* set the seqnum of the running segment */
      gst_event_set_seqnum (newseg, ogg->seqnum);

      /* send segment on old chain, FIXME, must be sent from streaming thread. */
      gst_ogg_demux_send_event (ogg, newseg);
    }
  }

  if (event) {
    gst_segment_set_seek (&ogg->segment, rate, format, flags,
        cur_type, cur, stop_type, stop, &update);
  }

  GST_DEBUG_OBJECT (ogg, "segment positions set to %" GST_TIME_FORMAT "-%"
      GST_TIME_FORMAT, GST_TIME_ARGS (ogg->segment.start),
      GST_TIME_ARGS (ogg->segment.stop));

  /* we need to stop flushing on the srcpad as we're going to use it
   * next. We can do this as we have the STREAM lock now. */
  if (flush) {
    tevent = gst_event_new_flush_stop ();
    gst_event_set_seqnum (tevent, seqnum);
    gst_pad_push_event (ogg->sinkpad, tevent);
  }

  {
    gint i;

    /* reset all ogg streams now, need to do this from within the lock to
     * make sure the streaming thread is not messing with the stream */
    for (i = 0; i < ogg->chains->len; i++) {
      GstOggChain *chain = g_array_index (ogg->chains, GstOggChain *, i);

      gst_ogg_chain_reset (chain);
    }
  }

  /* for reverse we will already seek accurately */
  if (rate < 0.0)
    accurate = FALSE;

  res = gst_ogg_demux_do_seek (ogg, ogg->segment.last_stop, accurate, &chain);

  /* seek failed, make sure we continue the current chain */
  if (!res) {
    GST_DEBUG_OBJECT (ogg, "seek failed");
    chain = ogg->current_chain;
  } else {
    GST_DEBUG_OBJECT (ogg, "seek success");
  }

  if (!chain)
    goto no_chain;

  /* now we have a new position, prepare for streaming again */
  {
    GstEvent *event;
    gint64 stop;
    gint64 start;
    gint64 last_stop, begin_time;

    /* we have to send the flush to the old chain, not the new one */
    if (flush) {
      tevent = gst_event_new_flush_stop ();
      gst_event_set_seqnum (tevent, seqnum);
      gst_ogg_demux_send_event (ogg, tevent);
    }

    /* we need this to see how far inside the chain we need to start */
    if (chain->begin_time != GST_CLOCK_TIME_NONE)
      begin_time = chain->begin_time;
    else
      begin_time = 0;

    /* segment.start gives the start over all chains, we calculate the amount
     * of time into this chain we need to start */
    start = ogg->segment.start - begin_time;
    if (chain->segment_start != GST_CLOCK_TIME_NONE)
      start += chain->segment_start;

    if ((stop = ogg->segment.stop) == -1)
      stop = ogg->segment.duration;

    /* segment.stop gives the stop time over all chains, calculate the amount of
     * time we need to stop in this chain */
    if (stop != -1) {
      if (stop > begin_time)
        stop -= begin_time;
      else
        stop = 0;
      stop += chain->segment_start;
      /* we must stop when this chain ends and switch to the next chain to play
       * the remainder of the segment. */
      stop = MIN (stop, chain->segment_stop);
    }

    last_stop = ogg->segment.last_stop - begin_time;
    if (chain->segment_start != GST_CLOCK_TIME_NONE)
      last_stop += chain->segment_start;

    /* create the segment event we are going to send out */
    if (ogg->segment.rate >= 0.0)
      event = gst_event_new_new_segment (FALSE, ogg->segment.rate,
          ogg->segment.format, last_stop, stop, ogg->segment.time);
    else
      event = gst_event_new_new_segment (FALSE, ogg->segment.rate,
          ogg->segment.format, start, last_stop, ogg->segment.time);

    gst_event_set_seqnum (event, seqnum);

    if (chain != ogg->current_chain) {
      /* switch to different chain, send segment on new chain */
      gst_ogg_demux_activate_chain (ogg, chain, event);
    } else {
      /* mark discont and send segment on current chain */
      gst_ogg_chain_mark_discont (chain);
      /* This event should be sent from the streaming thread (sink pad task) */
      if (ogg->newsegment)
        gst_event_unref (ogg->newsegment);
      ogg->newsegment = event;
    }

    /* notify start of new segment */
    if (ogg->segment.flags & GST_SEEK_FLAG_SEGMENT) {
      GstMessage *message;

      message = gst_message_new_segment_start (GST_OBJECT (ogg),
          GST_FORMAT_TIME, ogg->segment.last_stop);
      gst_message_set_seqnum (message, seqnum);

      gst_element_post_message (GST_ELEMENT (ogg), message);
    }

    ogg->segment_running = TRUE;
    ogg->seqnum = seqnum;
    /* restart our task since it might have been stopped when we did the 
     * flush. */
    gst_pad_start_task (ogg->sinkpad, (GstTaskFunction) gst_ogg_demux_loop,
        ogg->sinkpad);
  }

  /* streaming can continue now */
  GST_PAD_STREAM_UNLOCK (ogg->sinkpad);

  return res;

error:
  {
    GST_DEBUG_OBJECT (ogg, "seek failed");
    return FALSE;
  }
no_chain:
  {
    GST_DEBUG_OBJECT (ogg, "no chain to seek in");
    GST_PAD_STREAM_UNLOCK (ogg->sinkpad);
    return FALSE;
  }
}

/* finds each bitstream link one at a time using a bisection search
 * (has to begin by knowing the offset of the lb's initial page).
 * Recurses for each link so it can alloc the link storage after
 * finding them all, then unroll and fill the cache at the same time 
 */
static GstFlowReturn
gst_ogg_demux_bisect_forward_serialno (GstOggDemux * ogg,
    gint64 begin, gint64 searched, gint64 end, GstOggChain * chain, glong m)
{
  gint64 endsearched = end;
  gint64 next = end;
  ogg_page og;
  GstFlowReturn ret;
  gint64 offset;
  GstOggChain *nextchain;

  GST_LOG_OBJECT (ogg,
      "bisect begin: %" G_GINT64_FORMAT ", searched: %" G_GINT64_FORMAT
      ", end %" G_GINT64_FORMAT ", chain: %p", begin, searched, end, chain);

  /* the below guards against garbage seperating the last and
   * first pages of two links. */
  while (searched < endsearched) {
    gint64 bisect;

    if (endsearched - searched < CHUNKSIZE) {
      bisect = searched;
    } else {
      bisect = (searched + endsearched) / 2;
    }

    gst_ogg_demux_seek (ogg, bisect);
    ret = gst_ogg_demux_get_next_page (ogg, &og, -1, &offset);

    if (ret == GST_FLOW_UNEXPECTED) {
      endsearched = bisect;
    } else if (ret == GST_FLOW_OK) {
      glong serial = ogg_page_serialno (&og);

      if (!gst_ogg_chain_has_stream (chain, serial)) {
        endsearched = bisect;
        next = offset;
      } else {
        searched = offset + og.header_len + og.body_len;
      }
    } else
      return ret;
  }

  GST_LOG_OBJECT (ogg, "current chain ends at %" G_GINT64_FORMAT, searched);

  chain->end_offset = searched;
  ret = gst_ogg_demux_read_end_chain (ogg, chain);
  if (ret != GST_FLOW_OK)
    return ret;

  GST_LOG_OBJECT (ogg, "found begin at %" G_GINT64_FORMAT, next);

  gst_ogg_demux_seek (ogg, next);
  ret = gst_ogg_demux_read_chain (ogg, &nextchain);
  if (ret == GST_FLOW_UNEXPECTED) {
    nextchain = NULL;
    ret = GST_FLOW_OK;
    GST_LOG_OBJECT (ogg, "no next chain");
  } else if (ret != GST_FLOW_OK)
    goto done;

  if (searched < end && nextchain != NULL) {
    ret = gst_ogg_demux_bisect_forward_serialno (ogg, next, ogg->offset,
        end, nextchain, m + 1);
    if (ret != GST_FLOW_OK)
      goto done;
  }
  GST_LOG_OBJECT (ogg, "adding chain %p", chain);

  g_array_insert_val (ogg->chains, 0, chain);

done:
  return ret;
}

/* read a chain from the ogg file. This code will
 * read all BOS pages and will create and return a GstOggChain 
 * structure with the results. 
 * 
 * This function will also read N pages from each stream in the
 * chain and submit them to the decoders. When the decoder has
 * decoded the first buffer, we know the timestamp of the first
 * page in the chain.
 */
static GstFlowReturn
gst_ogg_demux_read_chain (GstOggDemux * ogg, GstOggChain ** res_chain)
{
  GstFlowReturn ret;
  GstOggChain *chain = NULL;
  gint64 offset = ogg->offset;
  ogg_page op;
  gboolean done;
  gint i;

  GST_LOG_OBJECT (ogg, "reading chain at %" G_GINT64_FORMAT, offset);

  /* first read the BOS pages, do typefind on them, create
   * the decoders, send data to the decoders. */
  while (TRUE) {
    GstOggPad *pad;
    glong serial;

    ret = gst_ogg_demux_get_next_page (ogg, &op, -1, NULL);
    if (ret != GST_FLOW_OK) {
      GST_WARNING_OBJECT (ogg, "problem reading BOS page: ret=%d", ret);
      break;
    }
    if (!ogg_page_bos (&op)) {
      GST_WARNING_OBJECT (ogg, "page is not BOS page");
      break;
    }

    if (chain == NULL) {
      chain = gst_ogg_chain_new (ogg);
      chain->offset = offset;
    }

    serial = ogg_page_serialno (&op);
    if (gst_ogg_chain_get_stream (chain, serial) != NULL) {
      GST_WARNING_OBJECT (ogg, "found serial %08lx BOS page twice, ignoring",
          serial);
      continue;
    }

    pad = gst_ogg_chain_new_stream (chain, serial);
    gst_ogg_pad_submit_page (pad, &op);
  }

  if (ret != GST_FLOW_OK || chain == NULL) {
    if (ret == GST_FLOW_OK) {
      GST_WARNING_OBJECT (ogg, "no chain was found");
      ret = GST_FLOW_ERROR;
    } else if (ret != GST_FLOW_UNEXPECTED) {
      GST_WARNING_OBJECT (ogg, "failed to read chain");
    } else {
      GST_DEBUG_OBJECT (ogg, "done reading chains");
    }
    if (chain) {
      gst_ogg_chain_free (chain);
    }
    if (res_chain)
      *res_chain = NULL;
    return ret;
  }

  chain->have_bos = TRUE;
  GST_LOG_OBJECT (ogg, "read bos pages, init decoder now");

  /* now read pages until we receive a buffer from each of the
   * stream decoders, this will tell us the timestamp of the
   * first packet in the chain then */

  /* save the offset to the first non bos page in the chain: if searching for
   * pad->first_time we read past the end of the chain, we'll seek back to this
   * position
   */
  offset = ogg->offset;

  done = FALSE;
  while (!done) {
    glong serial;
    gboolean known_serial = FALSE;
    GstFlowReturn ret;

    serial = ogg_page_serialno (&op);
    done = TRUE;
    for (i = 0; i < chain->streams->len; i++) {
      GstOggPad *pad = g_array_index (chain->streams, GstOggPad *, i);

      GST_LOG_OBJECT (ogg, "serial %08x time %" GST_TIME_FORMAT, pad->serialno,
          GST_TIME_ARGS (pad->start_time));

      if (pad->serialno == serial) {
        known_serial = TRUE;

        /* submit the page now, this will fill in the start_time when the
         * internal decoder finds it */
        gst_ogg_pad_submit_page (pad, &op);

        if (!pad->is_skeleton && pad->start_time == -1 && ogg_page_eos (&op)) {
          /* got EOS on a pad before we could find its start_time.
           * We have no chance of finding a start_time for every pad so
           * stop searching for the other start_time(s).
           */
          done = TRUE;
          break;
        }
      }
      /* the timestamp will be filled in when we submit the pages */
      if (!pad->is_skeleton)
        done &= (pad->start_time != GST_CLOCK_TIME_NONE);

      GST_LOG_OBJECT (ogg, "done %08x now %d", pad->serialno, done);
    }

    /* we read a page not belonging to the current chain: seek back to the
     * beginning of the chain
     */
    if (!known_serial) {
      GST_LOG_OBJECT (ogg, "unknown serial %08lx", serial);
      gst_ogg_demux_seek (ogg, offset);
      break;
    }

    if (!done) {
      ret = gst_ogg_demux_get_next_page (ogg, &op, -1, NULL);
      if (ret != GST_FLOW_OK)
        break;
    }
  }
  GST_LOG_OBJECT (ogg, "done reading chain");
  /* now we can fill in the missing info using queries */
  for (i = 0; i < chain->streams->len; i++) {
    GstOggPad *pad = g_array_index (chain->streams, GstOggPad *, i);
    GstFormat target;

    if (pad->is_skeleton)
      continue;

    GST_LOG_OBJECT (ogg, "convert first granule %" G_GUINT64_FORMAT " to time ",
        pad->first_granule);

    target = GST_FORMAT_TIME;
    if (!gst_ogg_pad_query_convert (pad,
            GST_FORMAT_DEFAULT, pad->first_granule, &target,
            (gint64 *) & pad->first_time)) {
      GST_WARNING_OBJECT (ogg, "could not convert granulepos to time");
    } else {
      GST_LOG_OBJECT (ogg, "converted to first time %" GST_TIME_FORMAT,
          GST_TIME_ARGS (pad->first_time));
    }

    pad->mode = GST_OGG_PAD_MODE_STREAMING;
  }

  if (res_chain)
    *res_chain = chain;

  return GST_FLOW_OK;
}

/* read the last pages from the ogg stream to get the final
 * page end_offsets.
 */
static GstFlowReturn
gst_ogg_demux_read_end_chain (GstOggDemux * ogg, GstOggChain * chain)
{
  gint64 begin = chain->end_offset;
  gint64 end = begin;
  gint64 last_granule = -1;
  GstOggPad *last_pad = NULL;
  GstFormat target;
  GstFlowReturn ret;
  gboolean done = FALSE;
  ogg_page og;
  gint i;

  while (!done) {
    begin -= CHUNKSIZE;
    if (begin < 0)
      begin = 0;

    gst_ogg_demux_seek (ogg, begin);

    /* now continue reading until we run out of data, if we find a page
     * start, we save it. It might not be the final page as there could be
     * another page after this one. */
    while (ogg->offset < end) {
      ret = gst_ogg_demux_get_next_page (ogg, &og, end - ogg->offset, NULL);

      if (ret == GST_FLOW_LIMIT)
        break;
      if (ret != GST_FLOW_OK)
        return ret;

      for (i = 0; i < chain->streams->len; i++) {
        GstOggPad *pad = g_array_index (chain->streams, GstOggPad *, i);

        if (pad->is_skeleton)
          continue;

        if (pad->serialno == ogg_page_serialno (&og)) {
          gint64 granulepos = ogg_page_granulepos (&og);

          if (last_granule < granulepos) {
            last_granule = granulepos;
            last_pad = pad;
          }
          done = TRUE;
          break;
        }
      }
    }
  }

  target = GST_FORMAT_TIME;
  if (last_granule == -1 || !gst_ogg_pad_query_convert (last_pad,
          GST_FORMAT_DEFAULT, last_granule, &target,
          (gint64 *) & chain->segment_stop)) {
    GST_WARNING_OBJECT (ogg, "could not convert granulepos to time");
    chain->segment_stop = GST_CLOCK_TIME_NONE;
  }

  return GST_FLOW_OK;
}

/* find a pad with a given serial number
 */
static GstOggPad *
gst_ogg_demux_find_pad (GstOggDemux * ogg, int serialno)
{
  GstOggPad *pad;
  gint i;

  /* first look in building chain if any */
  if (ogg->building_chain) {
    pad = gst_ogg_chain_get_stream (ogg->building_chain, serialno);
    if (pad)
      return pad;
  }

  /* then look in current chain if any */
  if (ogg->current_chain) {
    pad = gst_ogg_chain_get_stream (ogg->current_chain, serialno);
    if (pad)
      return pad;
  }

  for (i = 0; i < ogg->chains->len; i++) {
    GstOggChain *chain = g_array_index (ogg->chains, GstOggChain *, i);

    pad = gst_ogg_chain_get_stream (chain, serialno);
    if (pad)
      return pad;
  }
  return NULL;
}

/* find a chain with a given serial number
 */
static GstOggChain *
gst_ogg_demux_find_chain (GstOggDemux * ogg, int serialno)
{
  GstOggPad *pad;

  pad = gst_ogg_demux_find_pad (ogg, serialno);
  if (pad) {
    return pad->chain;
  }
  return NULL;
}

/* returns TRUE if all streams have valid start time */
static gboolean
gst_ogg_demux_collect_chain_info (GstOggDemux * ogg, GstOggChain * chain)
{
  gint i;
  gboolean res = TRUE;

  chain->total_time = GST_CLOCK_TIME_NONE;
  chain->segment_start = G_MAXUINT64;

  GST_DEBUG_OBJECT (ogg, "trying to collect chain info");

  for (i = 0; i < chain->streams->len; i++) {
    GstOggPad *pad = g_array_index (chain->streams, GstOggPad *, i);

    if (pad->is_skeleton)
      continue;

    /*  can do this if the pad start time is not defined */
    if (pad->start_time == GST_CLOCK_TIME_NONE)
      res = FALSE;
    else
      chain->segment_start = MIN (chain->segment_start, pad->start_time);
  }

  if (chain->segment_stop != GST_CLOCK_TIME_NONE
      && chain->segment_start != G_MAXUINT64)
    chain->total_time = chain->segment_stop - chain->segment_start;

  GST_DEBUG_OBJECT (ogg, "return %d", res);

  return res;
}

static void
gst_ogg_demux_collect_info (GstOggDemux * ogg)
{
  gint i;

  /* collect all info */
  ogg->total_time = 0;

  for (i = 0; i < ogg->chains->len; i++) {
    GstOggChain *chain = g_array_index (ogg->chains, GstOggChain *, i);

    chain->begin_time = ogg->total_time;

    gst_ogg_demux_collect_chain_info (ogg, chain);

    ogg->total_time += chain->total_time;
  }
  gst_segment_set_duration (&ogg->segment, GST_FORMAT_TIME, ogg->total_time);
}

/* find all the chains in the ogg file, this reads the first and
 * last page of the ogg stream, if they match then the ogg file has
 * just one chain, else we do a binary search for all chains.
 */
static GstFlowReturn
gst_ogg_demux_find_chains (GstOggDemux * ogg)
{
  ogg_page og;
  GstPad *peer;
  GstFormat format;
  gboolean res;
  gulong serialno;
  GstOggChain *chain;
  GstFlowReturn ret;

  /* get peer to figure out length */
  if ((peer = gst_pad_get_peer (ogg->sinkpad)) == NULL)
    goto no_peer;

  /* find length to read last page, we store this for later use. */
  format = GST_FORMAT_BYTES;
  res = gst_pad_query_duration (peer, &format, &ogg->length);
  gst_object_unref (peer);
  if (!res || ogg->length <= 0)
    goto no_length;

  GST_DEBUG_OBJECT (ogg, "file length %" G_GINT64_FORMAT, ogg->length);

  /* read chain from offset 0, this is the first chain of the
   * ogg file. */
  gst_ogg_demux_seek (ogg, 0);
  ret = gst_ogg_demux_read_chain (ogg, &chain);
  if (ret != GST_FLOW_OK)
    goto no_first_chain;

  /* read page from end offset, we use this page to check if its serial
   * number is contained in the first chain. If this is the case then
   * this ogg is not a chained ogg and we can skip the scanning. */
  gst_ogg_demux_seek (ogg, ogg->length);
  ret = gst_ogg_demux_get_prev_page (ogg, &og, NULL);
  if (ret != GST_FLOW_OK)
    goto no_last_page;

  serialno = ogg_page_serialno (&og);

  if (!gst_ogg_chain_has_stream (chain, serialno)) {
    /* the last page is not in the first stream, this means we should
     * find all the chains in this chained ogg. */
    ret =
        gst_ogg_demux_bisect_forward_serialno (ogg, 0, 0, ogg->length, chain,
        0);
  } else {
    /* we still call this function here but with an empty range so that
     * we can reuse the setup code in this routine. */
    ret =
        gst_ogg_demux_bisect_forward_serialno (ogg, 0, ogg->length, ogg->length,
        chain, 0);
  }
  if (ret != GST_FLOW_OK)
    goto done;

  /* all fine, collect and print */
  gst_ogg_demux_collect_info (ogg);

  /* dump our chains and streams */
  gst_ogg_print (ogg);

done:
  return ret;

  /*** error cases ***/
no_peer:
  {
    GST_ELEMENT_ERROR (ogg, STREAM, DEMUX, (NULL), ("we don't have a peer"));
    return GST_FLOW_NOT_LINKED;
  }
no_length:
  {
    GST_ELEMENT_ERROR (ogg, STREAM, DEMUX, (NULL), ("can't get file length"));
    return GST_FLOW_NOT_SUPPORTED;
  }
no_first_chain:
  {
    GST_ELEMENT_ERROR (ogg, STREAM, DEMUX, (NULL), ("can't get first chain"));
    return GST_FLOW_ERROR;
  }
no_last_page:
  {
    GST_DEBUG_OBJECT (ogg, "can't get last page");
    if (chain)
      gst_ogg_chain_free (chain);
    return ret;
  }
}

static GstFlowReturn
gst_ogg_demux_handle_page (GstOggDemux * ogg, ogg_page * page)
{
  GstOggPad *pad;
  gint64 granule;
  guint serialno;
  GstFlowReturn result = GST_FLOW_OK;

  serialno = ogg_page_serialno (page);
  granule = ogg_page_granulepos (page);

  GST_LOG_OBJECT (ogg,
      "processing ogg page (serial %08x, pageno %ld, granulepos %"
      G_GINT64_FORMAT ", bos %d)",
      serialno, ogg_page_pageno (page), granule, ogg_page_bos (page));

  if (ogg_page_bos (page)) {
    GstOggChain *chain;

    /* first page */
    /* see if we know about the chain already */
    chain = gst_ogg_demux_find_chain (ogg, serialno);
    if (chain) {
      GstEvent *event;
      gint64 start = 0;

      if (chain->segment_start != GST_CLOCK_TIME_NONE)
        start = chain->segment_start;

      /* create the newsegment event we are going to send out */
      event = gst_event_new_new_segment (FALSE, ogg->segment.rate,
          GST_FORMAT_TIME, start, chain->segment_stop, chain->begin_time);
      gst_event_set_seqnum (event, ogg->seqnum);

      GST_DEBUG_OBJECT (ogg,
          "segment: start %" GST_TIME_FORMAT ", stop %" GST_TIME_FORMAT
          ", time %" GST_TIME_FORMAT, GST_TIME_ARGS (start),
          GST_TIME_ARGS (chain->segment_stop),
          GST_TIME_ARGS (chain->begin_time));

      /* activate it as it means we have a non-header, this will also deactivate
       * the currently running chain. */
      gst_ogg_demux_activate_chain (ogg, chain, event);
      pad = gst_ogg_demux_find_pad (ogg, serialno);
    } else {
      GstClockTime chain_time;
      GstOggChain *current_chain;
      gint64 current_time;

      /* this can only happen in non-seekabe mode */
      if (ogg->seekable)
        goto unknown_chain;

      current_chain = ogg->current_chain;
      current_time = ogg->segment.last_stop;

      /* time of new chain is current time */
      chain_time = current_time;

      if (ogg->building_chain == NULL) {
        GstOggChain *newchain;

        newchain = gst_ogg_chain_new (ogg);
        newchain->offset = 0;
        /* set new chain begin time aligned with end time of old chain */
        newchain->begin_time = chain_time;
        GST_DEBUG_OBJECT (ogg, "new chain, begin time %" GST_TIME_FORMAT,
            GST_TIME_ARGS (chain_time));

        /* and this is the one we are building now */
        ogg->building_chain = newchain;
      }
      pad = gst_ogg_chain_new_stream (ogg->building_chain, serialno);
    }
  } else {
    pad = gst_ogg_demux_find_pad (ogg, serialno);
  }
  if (pad) {
    result = gst_ogg_pad_submit_page (pad, page);
  } else {
    /* no pad, this is pretty fatal. This means an ogg page without bos
     * has been seen for this serialno. could just ignore it too... */
    goto unknown_pad;
  }
  return result;

  /* ERRORS */
unknown_chain:
  {
    GST_ELEMENT_ERROR (ogg, STREAM, DECODE,
        (NULL), ("unknown ogg chain for serial %08x detected", serialno));
    return GST_FLOW_ERROR;
  }
unknown_pad:
  {
    GST_ELEMENT_ERROR (ogg, STREAM, DECODE,
        (NULL), ("unknown ogg pad for serial %08x detected", serialno));
    return GST_FLOW_ERROR;
  }
}

/* streaming mode, receive a buffer, parse it, create pads for
 * the serialno, submit pages and packets to the oggpads
 */
static GstFlowReturn
gst_ogg_demux_chain (GstPad * pad, GstBuffer * buffer)
{
  GstOggDemux *ogg;
  gint ret;
  GstFlowReturn result = GST_FLOW_OK;

  ogg = GST_OGG_DEMUX (GST_OBJECT_PARENT (pad));

  GST_DEBUG_OBJECT (ogg, "chain");
  result = gst_ogg_demux_submit_buffer (ogg, buffer);

  while (result == GST_FLOW_OK) {
    ogg_page page;

    ret = ogg_sync_pageout (&ogg->sync, &page);
    if (ret == 0)
      /* need more data */
      break;
    if (ret == -1) {
      /* discontinuity in the pages */
      GST_DEBUG_OBJECT (ogg, "discont in page found, continuing");
    } else {
      result = gst_ogg_demux_handle_page (ogg, &page);
    }
  }
  return result;
}

static void
gst_ogg_demux_send_event (GstOggDemux * ogg, GstEvent * event)
{
  GstOggChain *chain = ogg->current_chain;

  if (chain) {
    gint i;

    for (i = 0; i < chain->streams->len; i++) {
      GstOggPad *pad = g_array_index (chain->streams, GstOggPad *, i);

      gst_event_ref (event);
      GST_DEBUG_OBJECT (ogg, "Pushing event on pad %p", pad);
      gst_pad_push_event (GST_PAD (pad), event);
    }
  }
  gst_event_unref (event);
}

static GstFlowReturn
gst_ogg_demux_combine_flows (GstOggDemux * ogg, GstOggPad * pad,
    GstFlowReturn ret)
{
  GstOggChain *chain;

  /* store the value */
  pad->last_ret = ret;

  /* any other error that is not-linked can be returned right
   * away */
  if (ret != GST_FLOW_NOT_LINKED)
    goto done;

  /* only return NOT_LINKED if all other pads returned NOT_LINKED */
  chain = ogg->current_chain;
  if (chain) {
    gint i;

    for (i = 0; i < chain->streams->len; i++) {
      GstOggPad *opad = g_array_index (chain->streams, GstOggPad *, i);

      ret = opad->last_ret;
      /* some other return value (must be SUCCESS but we can return
       * other values as well) */
      if (ret != GST_FLOW_NOT_LINKED)
        goto done;
    }
    /* if we get here, all other pads were unlinked and we return
     * NOT_LINKED then */
  }
done:
  return ret;
}

static GstFlowReturn
gst_ogg_demux_loop_forward (GstOggDemux * ogg)
{
  GstFlowReturn ret;
  GstBuffer *buffer;

  if (ogg->offset == ogg->length) {
    GST_LOG_OBJECT (ogg, "no more data to pull %" G_GINT64_FORMAT
        " == %" G_GINT64_FORMAT, ogg->offset, ogg->length);
    ret = GST_FLOW_UNEXPECTED;
    goto done;
  }

  GST_LOG_OBJECT (ogg, "pull data %" G_GINT64_FORMAT, ogg->offset);
  ret = gst_pad_pull_range (ogg->sinkpad, ogg->offset, CHUNKSIZE, &buffer);
  if (ret != GST_FLOW_OK) {
    GST_LOG_OBJECT (ogg, "Failed pull_range");
    goto done;
  }

  ogg->offset += GST_BUFFER_SIZE (buffer);

  if (G_UNLIKELY (ogg->newsegment)) {
    gst_ogg_demux_send_event (ogg, ogg->newsegment);
    ogg->newsegment = NULL;
  }

  ret = gst_ogg_demux_chain (ogg->sinkpad, buffer);
  if (ret != GST_FLOW_OK) {
    GST_LOG_OBJECT (ogg, "Failed demux_chain");
    goto done;
  }

  /* check for the end of the segment */
  if (ogg->segment.stop != -1 && ogg->segment.last_stop != -1) {
    if (ogg->segment.last_stop > ogg->segment.stop) {
      ret = GST_FLOW_UNEXPECTED;
      goto done;
    }
  }
done:
  return ret;
}

/* reverse mode.
 *
 * We read the pages backwards and send the packets forwards. The first packet
 * in the page will be pushed with the DISCONT flag set.
 *
 * Special care has to be taken for continued pages, which we can only decode
 * when we have the previous page(s).
 */
static GstFlowReturn
gst_ogg_demux_loop_reverse (GstOggDemux * ogg)
{
  GstFlowReturn ret;
  ogg_page page;
  gint64 offset;

  if (ogg->offset == 0) {
    GST_LOG_OBJECT (ogg, "no more data to pull %" G_GINT64_FORMAT
        " == 0", ogg->offset);
    ret = GST_FLOW_UNEXPECTED;
    goto done;
  }

  GST_LOG_OBJECT (ogg, "read page from %" G_GINT64_FORMAT, ogg->offset);
  ret = gst_ogg_demux_get_prev_page (ogg, &page, &offset);
  if (ret != GST_FLOW_OK)
    goto done;

  ogg->offset = offset;

  if (G_UNLIKELY (ogg->newsegment)) {
    gst_ogg_demux_send_event (ogg, ogg->newsegment);
    ogg->newsegment = NULL;
  }

  ret = gst_ogg_demux_handle_page (ogg, &page);
  if (ret != GST_FLOW_OK)
    goto done;

  /* check for the end of the segment */
  if (ogg->segment.start != -1 && ogg->segment.last_stop != -1) {
    if (ogg->segment.last_stop <= ogg->segment.start) {
      ret = GST_FLOW_UNEXPECTED;
      goto done;
    }
  }
done:
  return ret;
}

/* random access code
 *
 * - first find all the chains and streams by scanning the file.
 * - then get and chain buffers, just like the streaming case.
 * - when seeking, we can use the chain info to perform the seek.
 */
static void
gst_ogg_demux_loop (GstOggPad * pad)
{
  GstOggDemux *ogg;
  GstFlowReturn ret;
  GstEvent *event;

  ogg = GST_OGG_DEMUX (GST_OBJECT_PARENT (pad));

  if (ogg->need_chains) {
    gboolean res;

    /* this is the only place where we write chains and thus need to lock. */
    GST_CHAIN_LOCK (ogg);
    ret = gst_ogg_demux_find_chains (ogg);
    GST_CHAIN_UNLOCK (ogg);
    if (ret != GST_FLOW_OK)
      goto chain_read_failed;

    ogg->need_chains = FALSE;

    GST_OBJECT_LOCK (ogg);
    ogg->running = TRUE;
    event = ogg->event;
    ogg->event = NULL;
    GST_OBJECT_UNLOCK (ogg);

    /* and seek to configured positions without FLUSH */
    res = gst_ogg_demux_perform_seek (ogg, event);
    if (event)
      gst_event_unref (event);

    if (!res)
      goto seek_failed;
  }

  if (ogg->segment.rate >= 0.0)
    ret = gst_ogg_demux_loop_forward (ogg);
  else
    ret = gst_ogg_demux_loop_reverse (ogg);

  if (ret != GST_FLOW_OK)
    goto pause;

  return;

  /* ERRORS */
chain_read_failed:
  {
    /* error was posted */
    goto pause;
  }
seek_failed:
  {
    GST_ELEMENT_ERROR (ogg, STREAM, DEMUX, (NULL),
        ("failed to start demuxing ogg"));
    ret = GST_FLOW_ERROR;
    goto pause;
  }
pause:
  {
    const gchar *reason = gst_flow_get_name (ret);
    GstEvent *event = NULL;

    GST_LOG_OBJECT (ogg, "pausing task, reason %s", reason);
    ogg->segment_running = FALSE;
    gst_pad_pause_task (ogg->sinkpad);

    if (GST_FLOW_IS_FATAL (ret) || ret == GST_FLOW_NOT_LINKED) {
      if (ret == GST_FLOW_UNEXPECTED) {
        /* perform EOS logic */
        if (ogg->segment.flags & GST_SEEK_FLAG_SEGMENT) {
          gint64 stop;
          GstMessage *message;

          /* for segment playback we need to post when (in stream time)
           * we stopped, this is either stop (when set) or the duration. */
          if ((stop = ogg->segment.stop) == -1)
            stop = ogg->segment.duration;

          GST_LOG_OBJECT (ogg, "Sending segment done, at end of segment");
          message =
              gst_message_new_segment_done (GST_OBJECT (ogg), GST_FORMAT_TIME,
              stop);
          gst_message_set_seqnum (message, ogg->seqnum);

          gst_element_post_message (GST_ELEMENT (ogg), message);
        } else {
          /* normal playback, send EOS to all linked pads */
          GST_LOG_OBJECT (ogg, "Sending EOS, at end of stream");
          event = gst_event_new_eos ();
        }
      } else {
        GST_ELEMENT_ERROR (ogg, STREAM, FAILED,
            (_("Internal data stream error.")),
            ("stream stopped, reason %s", reason));
        event = gst_event_new_eos ();
      }
      if (event) {
        gst_event_set_seqnum (event, ogg->seqnum);
        gst_ogg_demux_send_event (ogg, event);
      }
    }
    return;
  }
}

static void
gst_ogg_demux_clear_chains (GstOggDemux * ogg)
{
  gint i;

  gst_ogg_demux_deactivate_current_chain (ogg);

  GST_CHAIN_LOCK (ogg);
  for (i = 0; i < ogg->chains->len; i++) {
    GstOggChain *chain = g_array_index (ogg->chains, GstOggChain *, i);

    gst_ogg_chain_free (chain);
  }
  ogg->chains = g_array_set_size (ogg->chains, 0);
  GST_CHAIN_UNLOCK (ogg);
}

/* this function is called when the pad is activated and should start
 * processing data.
 *
 * We check if we can do random access to decide if we work push or
 * pull based.
 */
static gboolean
gst_ogg_demux_sink_activate (GstPad * sinkpad)
{
  if (gst_pad_check_pull_range (sinkpad)) {
    GST_DEBUG_OBJECT (sinkpad, "activating pull");
    return gst_pad_activate_pull (sinkpad, TRUE);
  } else {
    GST_DEBUG_OBJECT (sinkpad, "activating push");
    return gst_pad_activate_push (sinkpad, TRUE);
  }
}

/* this function gets called when we activate ourselves in push mode.
 * We cannot seek (ourselves) in the stream */
static gboolean
gst_ogg_demux_sink_activate_push (GstPad * sinkpad, gboolean active)
{
  GstOggDemux *ogg;

  ogg = GST_OGG_DEMUX (GST_OBJECT_PARENT (sinkpad));

  ogg->seekable = FALSE;

  return TRUE;
}

/* this function gets called when we activate ourselves in pull mode.
 * We can perform  random access to the resource and we start a task
 * to start reading */
static gboolean
gst_ogg_demux_sink_activate_pull (GstPad * sinkpad, gboolean active)
{
  GstOggDemux *ogg;

  ogg = GST_OGG_DEMUX (GST_OBJECT_PARENT (sinkpad));

  if (active) {
    ogg->need_chains = TRUE;
    ogg->seekable = TRUE;

    return gst_pad_start_task (sinkpad, (GstTaskFunction) gst_ogg_demux_loop,
        sinkpad);
  } else {
    return gst_pad_stop_task (sinkpad);
  }
}

static GstStateChangeReturn
gst_ogg_demux_change_state (GstElement * element, GstStateChange transition)
{
  GstOggDemux *ogg;
  GstStateChangeReturn result = GST_STATE_CHANGE_FAILURE;

  ogg = GST_OGG_DEMUX (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      ogg->basetime = 0;
      ogg->have_fishead = FALSE;
      ogg_sync_init (&ogg->sync);
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      ogg_sync_reset (&ogg->sync);
      ogg->current_granule = -1;
      ogg->running = FALSE;
      ogg->segment_running = FALSE;
      gst_segment_init (&ogg->segment, GST_FORMAT_TIME);
      break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
      break;
    default:
      break;
  }

  result = parent_class->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      gst_ogg_demux_clear_chains (ogg);
      GST_OBJECT_LOCK (ogg);
      ogg->running = FALSE;
      ogg->segment_running = FALSE;
      ogg->have_fishead = FALSE;
      GST_OBJECT_UNLOCK (ogg);
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      ogg_sync_clear (&ogg->sync);
      break;
    default:
      break;
  }
  return result;
}

static GstClockTime
gst_annodex_granule_to_time (gint64 granulepos, gint64 granulerate_n,
    gint64 granulerate_d, guint8 granuleshift)
{
  gint64 keyindex, keyoffset;
  gint64 granulerate;
  GstClockTime res;

  if (granulepos == 0 || granulerate_n == 0 || granulerate_d == 0)
    return 0;

  if (granuleshift != 0) {
    keyindex = granulepos >> granuleshift;
    keyoffset = granulepos - (keyindex << granuleshift);
    granulepos = keyindex + keyoffset;
  }

  /* GST_SECOND / (granulerate_n / granulerate_d) */
  granulerate = gst_util_uint64_scale (GST_SECOND,
      granulerate_d, granulerate_n);
  res = gst_util_uint64_scale (granulepos, granulerate, 1);
  return res;
}

gboolean
gst_ogg_demux_plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_ogg_demux_debug, "oggdemux", 0, "ogg demuxer");
  GST_DEBUG_CATEGORY_INIT (gst_ogg_demux_setup_debug, "oggdemux_setup", 0,
      "ogg demuxer setup stage when parsing pipeline");

#if ENABLE_NLS
  GST_DEBUG ("binding text domain %s to locale dir %s", GETTEXT_PACKAGE,
      LOCALEDIR);
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif

  return gst_element_register (plugin, "oggdemux", GST_RANK_PRIMARY,
      GST_TYPE_OGG_DEMUX);
}

/* prints all info about the element */
#undef GST_CAT_DEFAULT
#define GST_CAT_DEFAULT gst_ogg_demux_setup_debug

#ifdef GST_DISABLE_GST_DEBUG

static void
gst_ogg_print (GstOggDemux * ogg)
{
  /* NOP */
}

#else /* !GST_DISABLE_GST_DEBUG */

static void
gst_ogg_print (GstOggDemux * ogg)
{
  guint j, i;

  GST_INFO_OBJECT (ogg, "%u chains", ogg->chains->len);
  GST_INFO_OBJECT (ogg, " total time: %" GST_TIME_FORMAT,
      GST_TIME_ARGS (ogg->total_time));

  for (i = 0; i < ogg->chains->len; i++) {
    GstOggChain *chain = g_array_index (ogg->chains, GstOggChain *, i);

    GST_INFO_OBJECT (ogg, " chain %d (%u streams):", i, chain->streams->len);
    GST_INFO_OBJECT (ogg, "  offset: %" G_GINT64_FORMAT " - %" G_GINT64_FORMAT,
        chain->offset, chain->end_offset);
    GST_INFO_OBJECT (ogg, "  begin time: %" GST_TIME_FORMAT,
        GST_TIME_ARGS (chain->begin_time));
    GST_INFO_OBJECT (ogg, "  total time: %" GST_TIME_FORMAT,
        GST_TIME_ARGS (chain->total_time));
    GST_INFO_OBJECT (ogg, "  segment start: %" GST_TIME_FORMAT,
        GST_TIME_ARGS (chain->segment_start));
    GST_INFO_OBJECT (ogg, "  segment stop:  %" GST_TIME_FORMAT,
        GST_TIME_ARGS (chain->segment_stop));

    for (j = 0; j < chain->streams->len; j++) {
      GstOggPad *stream = g_array_index (chain->streams, GstOggPad *, j);

      GST_INFO_OBJECT (ogg, "  stream %08x:", stream->serialno);
      GST_INFO_OBJECT (ogg, "   start time:       %" GST_TIME_FORMAT,
          GST_TIME_ARGS (stream->start_time));
      GST_INFO_OBJECT (ogg, "   first granulepos: %" G_GINT64_FORMAT,
          stream->first_granule);
      GST_INFO_OBJECT (ogg, "   first time:       %" GST_TIME_FORMAT,
          GST_TIME_ARGS (stream->first_time));
    }
  }
}
#endif /* GST_DISABLE_GST_DEBUG */
