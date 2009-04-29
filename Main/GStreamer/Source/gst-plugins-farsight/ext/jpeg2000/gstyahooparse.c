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

#include "gstyahooparse.h"

/* elementfactory information */
GstElementDetails gst_yahooparse_details = {
  "Yahoo webcam packet parser",
  "Codec/Decoder/Network",
  "Parses headers from yahoo webcam packets",
  "Philippe Khalaf <burger@speedy.org>"
};

GST_DEBUG_CATEGORY(yahooparse_debug);
#define GST_CAT_DEFAULT yahooparse_debug

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

static void gst_yahooparse_base_init(gpointer g_class);
static void gst_yahooparse_class_init(GstYahooParse *klass);
static void gst_yahooparse_init(GstYahooParse *yahooparse);
static void gst_yahooparse_finalize (GObject * object);

static GstFlowReturn gst_yahooparse_chain(GstPad *pad, GstBuffer *in);

static GstElementClass *parent_class = NULL;

/*static guint gst_yahooparse_signals[LAST_SIGNAL] = { 0 }; */

GType
gst_yahooparse_get_type(void)
{
  static GType yahooparse_type = 0;

  if (!yahooparse_type) {
    static const GTypeInfo yahooparse_info = {
      sizeof (GstYahooParseClass),
      gst_yahooparse_base_init,
      NULL,
      (GClassInitFunc) gst_yahooparse_class_init,
      NULL,
      NULL,
      sizeof (GstYahooParse),
      0,
      (GInstanceInitFunc) gst_yahooparse_init,
    };

    yahooparse_type =
        g_type_register_static(GST_TYPE_ELEMENT, "GstYahooParse", &yahooparse_info,
        0);
  }
  return yahooparse_type;
}

static GstStaticPadTemplate gst_yahooparse_src_pad_template =
GST_STATIC_PAD_TEMPLATE("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("image/jp2")
   );

static GstStaticPadTemplate gst_yahooparse_sink_pad_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("ANY")
    );

static void
gst_yahooparse_base_init(gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS(g_class);

  gst_element_class_add_pad_template(element_class,
          gst_static_pad_template_get(&gst_yahooparse_src_pad_template));
  gst_element_class_add_pad_template(element_class,
          gst_static_pad_template_get(&gst_yahooparse_sink_pad_template));

  gst_element_class_set_details(element_class, &gst_yahooparse_details);
}

static void
gst_yahooparse_class_init(GstYahooParse *klass)
{
  GstElementClass *gstelement_class;
  GObjectClass *gobject_class;

  gstelement_class = (GstElementClass *) klass;
  gobject_class = (GObjectClass *) klass;

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);

  gobject_class->finalize = gst_yahooparse_finalize;

  GST_DEBUG_CATEGORY_INIT(yahooparse_debug, "yahooparse", 0, "JPEG2000 decoder");
}

static void
gst_yahooparse_init(GstYahooParse *yahooparse)
{
  yahooparse->sinkpad = gst_pad_new_from_template(gst_static_pad_template_get(
              &gst_yahooparse_sink_pad_template), "sink");
  gst_element_add_pad(GST_ELEMENT(yahooparse), yahooparse->sinkpad);
  gst_pad_set_chain_function(yahooparse->sinkpad, gst_yahooparse_chain);


  yahooparse->srcpad = gst_pad_new_from_template(gst_static_pad_template_get(
              &gst_yahooparse_src_pad_template), "src");
  gst_element_add_pad(GST_ELEMENT(yahooparse), yahooparse->srcpad);

  yahooparse->adapter = gst_adapter_new ();

  yahooparse->have_header = FALSE;
}

static void 
gst_yahooparse_finalize (GObject * object)
{
    GstYahooParse *yahooparse = GST_YAHOOPARSE (object);

    gst_adapter_clear (yahooparse->adapter);
    g_object_unref (yahooparse->adapter);
}


static GstFlowReturn
gst_yahooparse_chain(GstPad *pad, GstBuffer *buf)
{
    GstYahooParse *yahooparse;
    guchar *data, *header, *small_chunk;
    guchar reason = 0;
    guint8 *peek_at_header_len;
    guint8 header_len;
    GstBuffer *outbuf;

    g_return_val_if_fail(pad != NULL, GST_FLOW_ERROR);
    g_return_val_if_fail(GST_IS_PAD(pad), GST_FLOW_ERROR);
    g_return_val_if_fail(buf != NULL, GST_FLOW_ERROR);

    yahooparse = GST_YAHOOPARSE(GST_OBJECT_PARENT(pad));

    //gst_util_dump_mem (GST_BUFFER_DATA(buf), GST_BUFFER_SIZE(buf));
    gst_adapter_push (yahooparse->adapter, buf);

    // if we have no header, we seek until the start of a header (header_len 00 05 00)
    if (!yahooparse->have_header)
    {
        gboolean found = FALSE;
        while (!found)
        {
            small_chunk = (guchar *) gst_adapter_peek (yahooparse->adapter, 4);
            if (small_chunk[2] == 0x05 && small_chunk[3] == 0x00)
            {
                found = TRUE;
            }
            else
            {
                if (gst_adapter_available (yahooparse->adapter) >= 1)
                    gst_adapter_flush (yahooparse->adapter, 1);
            }
        }
    }

    peek_at_header_len = (guint8 *) gst_adapter_peek (yahooparse->adapter, 1);

    // do we have enough bytes to read a header
    while (gst_adapter_available (yahooparse->adapter) >= 
            (yahooparse->have_header ? yahooparse->payload_size : *peek_at_header_len)) {
        if (!yahooparse->have_header) {
            header = (guchar *) gst_adapter_peek (yahooparse->adapter, *peek_at_header_len);
            header_len = header[0];

            if (header_len >= 8) {
                reason = header[1];
                /* next 2 bytes should always be 05 00 */
                yahooparse->payload_size = GUINT32_FROM_BE (*((guint32 *) (header + 4)));
            }
            if (header_len >= 13) {
                yahooparse->packet_type = header[8];
                yahooparse->timestamp = GUINT32_FROM_BE (*((guint32 *) (header + 9)));
            }

            gst_util_dump_mem (header, header_len);
            GST_DEBUG ("Got header info length %d packet_type %.2X reason %d payload_size %d timestamp %d\n",
                    header_len, yahooparse->packet_type, reason, yahooparse->payload_size, yahooparse->timestamp);

            gst_adapter_flush (yahooparse->adapter, header_len);

            yahooparse->have_header = TRUE;
        }

        if (yahooparse->packet_type == 0x07)
        {
            // he's gone
            // let's emit a signal to client app, or use the bus?
        }

        if (yahooparse->payload_size == 0 || yahooparse->packet_type != 0x02)
        {
            yahooparse->have_header = FALSE;
            gst_adapter_flush (yahooparse->adapter, yahooparse->payload_size);
            return GST_FLOW_OK;
        }

        if (gst_adapter_available (yahooparse->adapter) < yahooparse->payload_size)
        {
            return GST_FLOW_OK;
        }

        data = (guchar *) gst_adapter_peek (yahooparse->adapter, yahooparse->payload_size);
        outbuf = gst_buffer_new_and_alloc (yahooparse->payload_size);
        memcpy (GST_BUFFER_DATA (outbuf), data, yahooparse->payload_size);
        gst_pad_push (yahooparse->srcpad, outbuf);
        gst_adapter_flush (yahooparse->adapter, yahooparse->payload_size);
        yahooparse->have_header = FALSE;
    }
    return GST_FLOW_OK;
}

gboolean
gst_yahooparse_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "yahooparse",
      GST_RANK_NONE, GST_TYPE_YAHOOPARSE);
}
