/*
 * Farsight
 * GStreamer msGSM decoder (uses WAV49 compiled libgsm)
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

#include "gstmsgsmdec.h"

GST_DEBUG_CATEGORY (msgsmdec_debug);
#define GST_CAT_DEFAULT (msgsmdec_debug)

/* elementfactory information */
GstElementDetails gst_msgsmdec_details = {
  "MS GSM audio decoder",
  "Codec/Decoder/Audio",
  "Decodes MS GSM encoded audio",
  "Philippe Khalaf <burger@speedy.org>",
};

/* MSGSMDec signals and args */
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

static void gst_msgsmdec_base_init (gpointer g_class);
static void gst_msgsmdec_class_init (GstMSGSMDec * klass);
static void gst_msgsmdec_init (GstMSGSMDec * msgsmdec);

static GstFlowReturn gst_msgsmdec_chain (GstPad * pad, GstBuffer * buf);

static GstElementClass *parent_class = NULL;

/*static guint gst_msgsmdec_signals[LAST_SIGNAL] = { 0 }; */

GType
gst_msgsmdec_get_type (void)
{
  static GType msgsmdec_type = 0;

  if (!msgsmdec_type) {
    static const GTypeInfo msgsmdec_info = {
      sizeof (GstMSGSMDecClass),
      gst_msgsmdec_base_init,
      NULL,
      (GClassInitFunc) gst_msgsmdec_class_init,
      NULL,
      NULL,
      sizeof (GstMSGSMDec),
      0,
      (GInstanceInitFunc) gst_msgsmdec_init,
    };

    msgsmdec_type =
        g_type_register_static (GST_TYPE_ELEMENT, "GstMSGSMDec", &msgsmdec_info, 0);
  }
  return msgsmdec_type;
}

static GstStaticPadTemplate msgsmdec_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-msgsm, " "rate = (int) 8000, " "channels = (int) 1")
    );

static GstStaticPadTemplate msgsmdec_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw-int, "
        "endianness = (int) BYTE_ORDER, "
        "signed = (boolean) true, "
        "width = (int) 16, "
        "depth = (int) 16, " "rate = (int) 8000, " "channels = (int) 1")
    );

static void
gst_msgsmdec_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&msgsmdec_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&msgsmdec_src_template));
  gst_element_class_set_details (element_class, &gst_msgsmdec_details);
}

static void
gst_msgsmdec_class_init (GstMSGSMDec * klass)
{
  GstElementClass *gstelement_class;

  gstelement_class = (GstElementClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  GST_DEBUG_CATEGORY_INIT (msgsmdec_debug, "msgsmdec", 0, "MSGSM Decoder");
}

static void
gst_msgsmdec_init (GstMSGSMDec * msgsmdec)
{
  // turn on WAN49 handling
  gint use_wav49 = 1;

  /* create the sink and src pads */
  msgsmdec->sinkpad =
      gst_pad_new_from_template (gst_static_pad_template_get
      (&msgsmdec_sink_template), "sink");
  gst_pad_set_chain_function (msgsmdec->sinkpad, gst_msgsmdec_chain);
  gst_element_add_pad (GST_ELEMENT (msgsmdec), msgsmdec->sinkpad);

  msgsmdec->srcpad =
      gst_pad_new_from_template (gst_static_pad_template_get
      (&msgsmdec_src_template), "src");
  gst_element_add_pad (GST_ELEMENT (msgsmdec), msgsmdec->srcpad);

  msgsmdec->state = gsm_create ();

  gsm_option (msgsmdec->state, GSM_OPT_WAV49, &use_wav49);

  msgsmdec->adapter = gst_adapter_new ();

  msgsmdec->next_of = 0;
}

static GstFlowReturn
gst_msgsmdec_chain (GstPad * pad, GstBuffer * buf)
{
  GstMSGSMDec *msgsmdec;
  gsm_byte *data;

  g_return_val_if_fail (GST_IS_PAD (pad), GST_FLOW_ERROR);

  msgsmdec = GST_MSGSMDEC (gst_pad_get_parent (pad));
  g_return_val_if_fail (GST_IS_MSGSMDEC (msgsmdec), GST_FLOW_ERROR);

  g_return_val_if_fail(GST_PAD_IS_LINKED(msgsmdec->srcpad), GST_FLOW_ERROR);

  gst_adapter_push (msgsmdec->adapter, buf);

  // do we have enough bytes to read a header
  while (gst_adapter_available (msgsmdec->adapter) >= 65) {

      GstBuffer *outbuf;

      gint value = 0;
      gsm_option(msgsmdec->state, GSM_OPT_FRAME_INDEX, & value);
      GST_DEBUG("Value is %d", value);

      outbuf = gst_buffer_new_and_alloc (2 * 160 * sizeof (gsm_signal));
      // TODO take new segment in consideration, if not given restart
      // timestamps at 0
      GST_BUFFER_TIMESTAMP (outbuf) = GST_BUFFER_TIMESTAMP (buf);
      GST_BUFFER_DURATION (outbuf) = 40 * GST_MSECOND;
      GST_BUFFER_OFFSET (outbuf) = msgsmdec->next_of;
      GST_BUFFER_OFFSET_END (outbuf) = msgsmdec->next_of + 320 - 1;
      gst_buffer_set_caps (outbuf, gst_pad_get_caps (msgsmdec->srcpad));
      msgsmdec->next_of += 320;

      data = (gsm_byte *) gst_adapter_peek (msgsmdec->adapter, 33);
      gsm_decode (msgsmdec->state, data, 
              (gsm_signal *) GST_BUFFER_DATA (outbuf));
      gst_adapter_flush (msgsmdec->adapter, 33);

      data = (gsm_byte *) gst_adapter_peek (msgsmdec->adapter, 32);
      gsm_decode (msgsmdec->state, data,
              (gsm_signal *) GST_BUFFER_DATA (outbuf) + 160);
      gst_adapter_flush (msgsmdec->adapter, 32);

      GST_DEBUG ("Pushing buffer of size %d ts %"GST_TIME_FORMAT, GST_BUFFER_SIZE (outbuf), 
              GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (outbuf)));
      //gst_util_dump_mem (GST_BUFFER_DATA(outbuf), GST_BUFFER_SIZE (outbuf));
      gst_pad_push (msgsmdec->srcpad, outbuf);
  }

  return GST_FLOW_OK;
}
