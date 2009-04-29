/*
 * Farsight
 * GStreamer msGSM encoder (uses WAV49 compiled libgsm)
 * Copyright (C) 2005 Philippe Khalaf <burger@speedy.org>
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

#include "gstmsgsmenc.h"

GST_DEBUG_CATEGORY (msgsmenc_debug);
#define GST_CAT_DEFAULT (msgsmenc_debug)

/* elementfactory information */
GstElementDetails gst_msgsmenc_details = {
  "MS GSM audio encoder",
  "Codec/Encoder/Audio",
  "Encodes MS GSM audio",
  "Philippe Khalaf <burger@speedy.org>"
};

/* MSGSMEnc signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
    /* FILL ME */
    ARG_0
};

static void gst_msgsmenc_base_init (gpointer g_class);
static void gst_msgsmenc_class_init (GstMSGSMEnc * klass);
static void gst_msgsmenc_init (GstMSGSMEnc * msgsmenc);

static GstFlowReturn gst_msgsmenc_chain (GstPad * pad, GstBuffer * buf);

static GstElementClass *parent_class = NULL;

GType
gst_msgsmenc_get_type (void)
{
  static GType msgsmenc_type = 0;

  if (!msgsmenc_type) {
    static const GTypeInfo msgsmenc_info = {
      sizeof (GstMSGSMEncClass),
      gst_msgsmenc_base_init,
      NULL,
      (GClassInitFunc) gst_msgsmenc_class_init,
      NULL,
      NULL,
      sizeof (GstMSGSMEnc),
      0,
      (GInstanceInitFunc) gst_msgsmenc_init,
    };

    msgsmenc_type =
        g_type_register_static (GST_TYPE_ELEMENT, "GstMSGSMEnc", &msgsmenc_info, 0);
  }
  return msgsmenc_type;
}

static GstStaticPadTemplate msgsmenc_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-msgsm, " "rate = (int) 8000, " "channels = (int) 1")
    );

static GstStaticPadTemplate msgsmenc_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw-int, "
        "endianness = (int) BYTE_ORDER, "
        "signed = (boolean) true, "
        "width = (int) 16, "
        "depth = (int) 16, " "rate = (int) 8000, " "channels = (int) 1")
    );

static void
gst_msgsmenc_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&msgsmenc_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&msgsmenc_src_template));
  gst_element_class_set_details (element_class, &gst_msgsmenc_details);
}

static void
gst_msgsmenc_class_init (GstMSGSMEnc * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);
  
  GST_DEBUG_CATEGORY_INIT (msgsmenc_debug, "msgsmenc", 0, "MSGSM Encoder");
}

static void
gst_msgsmenc_init (GstMSGSMEnc * msgsmenc)
{
  // turn on WAN49 handling
  gint use_wav49 = 1;

  /* create the sink and src pads */
  msgsmenc->sinkpad =
      gst_pad_new_from_template (gst_static_pad_template_get
      (&msgsmenc_sink_template), "sink");
  gst_element_add_pad (GST_ELEMENT (msgsmenc), msgsmenc->sinkpad);
  gst_pad_set_chain_function (msgsmenc->sinkpad, gst_msgsmenc_chain);

  msgsmenc->srcpad =
      gst_pad_new_from_template (gst_static_pad_template_get
      (&msgsmenc_src_template), "src");
  gst_element_add_pad (GST_ELEMENT (msgsmenc), msgsmenc->srcpad);

  msgsmenc->state = gsm_create ();

  gsm_option (msgsmenc->state, GSM_OPT_WAV49, &use_wav49);

  msgsmenc->adapter = gst_adapter_new ();

  msgsmenc->next_ts = 0;
}

static GstFlowReturn
gst_msgsmenc_chain (GstPad * pad, GstBuffer * buf)
{
  GstMSGSMEnc *msgsmenc;
  gsm_signal *data;

  g_return_val_if_fail (GST_IS_PAD (pad), GST_FLOW_ERROR);

  msgsmenc = GST_MSGSMENC (gst_pad_get_parent (pad));
  g_return_val_if_fail (GST_IS_MSGSMENC (msgsmenc), GST_FLOW_ERROR);

  g_return_val_if_fail(GST_PAD_IS_LINKED(msgsmenc->srcpad), GST_FLOW_ERROR);

  gst_adapter_push (msgsmenc->adapter, buf);

  while (gst_adapter_available (msgsmenc->adapter) >= 640) {

      GstBuffer *outbuf;

      gint value = 0;
      gsm_option(msgsmenc->state, GSM_OPT_FRAME_INDEX, & value);
      GST_DEBUG("Value is %d", value);

      outbuf = gst_buffer_new_and_alloc (65 * sizeof (gsm_byte));
      GST_BUFFER_TIMESTAMP (outbuf) = msgsmenc->next_ts;
      GST_BUFFER_DURATION (outbuf) = 40 * GST_MSECOND;
      msgsmenc->next_ts += 40 * GST_MSECOND;

      // encode first 160 16-bit samples into 32 bytes
      data = (gsm_signal *) gst_adapter_peek (msgsmenc->adapter, 320);
      gsm_encode (msgsmenc->state, data,
              (gsm_byte *) GST_BUFFER_DATA (outbuf));
      gst_adapter_flush (msgsmenc->adapter, 320);

      // encode the second 160 16-bit smaples into 33 bytes
      data = (gsm_signal *) gst_adapter_peek (msgsmenc->adapter, 320);
      gsm_encode (msgsmenc->state, data,
              (gsm_byte *) GST_BUFFER_DATA (outbuf) + 32);
      gst_adapter_flush (msgsmenc->adapter, 320);

      gst_buffer_set_caps (outbuf, gst_pad_get_caps (msgsmenc->srcpad));
      GST_DEBUG ("Pushing buffer of size %d", GST_BUFFER_SIZE (outbuf));
      //gst_util_dump_mem (GST_BUFFER_DATA(outbuf), GST_BUFFER_SIZE (outbuf));
      gst_pad_push (msgsmenc->srcpad, outbuf);
  }

  return GST_FLOW_OK;
}
