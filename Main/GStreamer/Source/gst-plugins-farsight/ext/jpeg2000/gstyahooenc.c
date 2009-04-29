/* Farsight
 * Copyright (C) <2005> Philippe Khalaf <burger@speedy.org> 
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

#include "gstyahooenc.h"

/* elementfactory information */
GstElementDetails gst_yahooenc_details = {
  "Yahoo webcam packet encoder",
  "Codec/Encoder/Network",
  "Adds yahoo webcam headers to jpeg2000 frames",
  "Philippe Khalaf <burger@speedy.org>"
};

GST_DEBUG_CATEGORY(yahooenc_debug);
#define GST_CAT_DEFAULT yahooenc_debug

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

static void gst_yahooenc_base_init(gpointer g_class);
static void gst_yahooenc_class_init(GstYahooEnc *klass);
static void gst_yahooenc_init(GstYahooEnc *yahooenc);
static void gst_yahooenc_finalize(GObject *objet);

static GstFlowReturn gst_yahooenc_chain(GstPad *pad, GstBuffer *in);

static GstElementClass *parent_class = NULL;

/*static guint gst_yahooenc_signals[LAST_SIGNAL] = { 0 }; */

GType
gst_yahooenc_get_type(void)
{
  static GType yahooenc_type = 0;

  if (!yahooenc_type) {
    static const GTypeInfo yahooenc_info = {
      sizeof (GstYahooEncClass),
      gst_yahooenc_base_init,
      NULL,
      (GClassInitFunc) gst_yahooenc_class_init,
      NULL,
      NULL,
      sizeof (GstYahooEnc),
      0,
      (GInstanceInitFunc) gst_yahooenc_init,
    };

    yahooenc_type =
        g_type_register_static(GST_TYPE_ELEMENT, "GstYahooEnc", &yahooenc_info,
        0);
  }
  return yahooenc_type;
}

static GstStaticPadTemplate gst_yahooenc_src_pad_template =
GST_STATIC_PAD_TEMPLATE("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("ANY")
   );

static GstStaticPadTemplate gst_yahooenc_sink_pad_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("image/jp2")
    );

static void
gst_yahooenc_base_init(gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS(g_class);

  gst_element_class_add_pad_template(element_class,
          gst_static_pad_template_get(&gst_yahooenc_src_pad_template));
  gst_element_class_add_pad_template(element_class,
          gst_static_pad_template_get(&gst_yahooenc_sink_pad_template));

  gst_element_class_set_details(element_class, &gst_yahooenc_details);
}

static void
gst_yahooenc_class_init(GstYahooEnc *klass)
{
  GstElementClass *gstelement_class;
  GObjectClass *gobject_class;

  gstelement_class = (GstElementClass *) klass;
  gobject_class = (GObjectClass *) klass;

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);

  gobject_class->finalize = gst_yahooenc_finalize;

  GST_DEBUG_CATEGORY_INIT(yahooenc_debug, "yahooenc", 0, "Yahoo Webcam encoder");
}

static void
gst_yahooenc_init(GstYahooEnc *yahooenc)
{
  yahooenc->sinkpad = gst_pad_new_from_template(gst_static_pad_template_get(
              &gst_yahooenc_sink_pad_template), "sink");
  gst_element_add_pad(GST_ELEMENT(yahooenc), yahooenc->sinkpad);
  gst_pad_set_chain_function(yahooenc->sinkpad, gst_yahooenc_chain);


  yahooenc->srcpad = gst_pad_new_from_template(gst_static_pad_template_get(
              &gst_yahooenc_src_pad_template), "src");
  gst_element_add_pad(GST_ELEMENT(yahooenc), yahooenc->srcpad);

  yahooenc->ts_rand = g_rand_new();
  yahooenc->cur_ts = g_rand_int_range (yahooenc->ts_rand, 0, G_MAXUINT16);

  yahooenc->prev_ts = 0;
}

static void 
gst_yahooenc_finalize(GObject *objet)
{
    GstYahooEnc *yahooenc;
    yahooenc = GST_YAHOOENC (objet);

    g_rand_free (yahooenc->ts_rand);
    yahooenc->ts_rand = NULL;
}

static GstBuffer* 
gst_yahooenc_create_header (GstYahooEnc *yahooenc, GstBuffer *buf)
{
    // 13 bytes
    GstBuffer *buf_header = gst_buffer_new_and_alloc (13);
    guchar *p = (guchar *) GST_BUFFER_DATA(buf_header);

    *((guint8 *)  (p)) = 13;            // header size
    *((guchar *)  (p + 1)) = 0x00;      // reason always 0 for now
    *((guchar *)  (p + 2)) = 0x05;      // is always 0x05
    *((guchar *)  (p + 3)) = 0x00;      // is always 0x00
    *((guint32 *) (p + 4)) = GUINT32_TO_BE(GST_BUFFER_SIZE(buf));
    *((guchar *)  (p + 8)) = 0x02;      // packet_type is 0x02 because it is data 
    // calc new ts
    if (yahooenc->prev_ts == 0) yahooenc->prev_ts = GST_BUFFER_TIMESTAMP(buf);
    yahooenc->cur_ts += (guint32)((GST_BUFFER_TIMESTAMP(buf) - yahooenc->prev_ts) / (guint64)1000000);
    yahooenc->prev_ts = GST_BUFFER_TIMESTAMP(buf);
    *((guint32 *) (p + 9)) = GUINT32_TO_BE(yahooenc->cur_ts);

    gst_util_dump_mem (GST_BUFFER_DATA(buf_header), GST_BUFFER_SIZE(buf_header));

    return buf_header;
}

static GstFlowReturn
gst_yahooenc_chain(GstPad *pad, GstBuffer *buf)
{
    GstYahooEnc *yahooenc;
    GstBuffer *header;

    g_return_val_if_fail(pad != NULL, GST_FLOW_ERROR);
    g_return_val_if_fail(GST_IS_PAD(pad), GST_FLOW_ERROR);
    g_return_val_if_fail(buf != NULL, GST_FLOW_ERROR);

    yahooenc = GST_YAHOOENC(GST_OBJECT_PARENT(pad));

    header = gst_yahooenc_create_header (yahooenc, buf);

    gst_pad_push (yahooenc->srcpad, header);
    gst_pad_push (yahooenc->srcpad, buf);

    return GST_FLOW_OK;
}

gboolean
gst_yahooenc_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "yahooenc",
      GST_RANK_NONE, GST_TYPE_YAHOOENC);
}
