/*
 * GStreamer
 * Copyright (c) 2005 INdT.
 * @author Andre Moreira Magalhaes <andre.magalhaes@indt.org.br>
 * @author Rob Taylor <robtaylor@fastmail.fm>
 * @author Philippe Khalaf <burger@speedy.org>
 * @author Ole André Vadla Ravnås <oleavr@gmail.com>
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
 * SECTION:element-mimdec
 * @see_also: mimenc
 *
 * The MIMIC codec is used by MSN Messenger's webcam support. It consumes the
 * TCP header for the MIMIC codec.
 *
 * Its fourcc is ML20.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>

#include "gstmimdec.h"

GST_DEBUG_CATEGORY (mimdec_debug);
#define GST_CAT_DEFAULT (mimdec_debug)

static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-mimic")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw-rgb, "
        "bpp = (int) 24, "
        "depth = (int) 24, "
        "endianness = (int) 4321, "
        "framerate = (fraction) [ 0/1, 30/1 ], "
        "red_mask = (int) 16711680, "
        "green_mask = (int) 65280, "
        "blue_mask = (int) 255, "
        "height = (int) [16, 4096], " "width = (int) [16, 4096]")
    );

static void gst_mimdec_finalize (GObject * object);

static GstFlowReturn gst_mimdec_chain (GstPad * pad, GstBuffer * in);
static GstStateChangeReturn
gst_mimdec_change_state (GstElement * element, GstStateChange transition);

static gboolean gst_mimdec_sink_event (GstPad * pad, GstEvent * event);


GST_BOILERPLATE (GstMimDec, gst_mimdec, GstElement, GST_TYPE_ELEMENT);

static void
gst_mimdec_base_init (gpointer klass)
{
  static GstElementDetails plugin_details = {
    "MimDec",
    "Codec/Decoder/Video",
    "Mimic decoder",
    "Andre Moreira Magalhaes <andre.magalhaes@indt.org.br>, "
        "Rob Taylor <robtaylor@fastmail.fm>, "
        "Philippe Khalaf <burger@speedy.org>, "
        "Ole André Vadla Ravnås <oleavr@gmail.com>"
  };
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_factory));

  gst_element_class_set_details (element_class, &plugin_details);
}

static void
gst_mimdec_class_init (GstMimDecClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstelement_class->change_state = gst_mimdec_change_state;

  gobject_class->finalize = gst_mimdec_finalize;

  GST_DEBUG_CATEGORY_INIT (mimdec_debug, "mimdec", 0, "Mimic decoder plugin");
}

static void
gst_mimdec_init (GstMimDec * mimdec, GstMimDecClass * klass)
{
  mimdec->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_element_add_pad (GST_ELEMENT (mimdec), mimdec->sinkpad);
  gst_pad_set_chain_function (mimdec->sinkpad, gst_mimdec_chain);
  gst_pad_set_event_function (mimdec->sinkpad, gst_mimdec_sink_event);

  mimdec->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  gst_element_add_pad (GST_ELEMENT (mimdec), mimdec->srcpad);

  mimdec->adapter = gst_adapter_new ();

  mimdec->dec = NULL;
  mimdec->buffer_size = -1;
  mimdec->have_header = FALSE;
  mimdec->payload_size = -1;
  mimdec->current_ts = -1;
}

static void
gst_mimdec_finalize (GObject * object)
{
  GstMimDec *mimdec = GST_MIMDEC (object);

  gst_adapter_clear (mimdec->adapter);
  g_object_unref (mimdec->adapter);

  GST_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static GstFlowReturn
gst_mimdec_chain (GstPad * pad, GstBuffer * in)
{
  GstMimDec *mimdec;
  GstBuffer *out_buf, *buf;
  guchar *header, *frame_body;
  guint32 fourcc;
  guint16 header_size;
  gint width, height;
  GstCaps *caps;
  GstFlowReturn res = GST_FLOW_OK;
  GstClockTime in_time = GST_BUFFER_TIMESTAMP (in);

  GST_DEBUG ("in gst_mimdec_chain");

  g_return_val_if_fail (GST_IS_PAD (pad), GST_FLOW_ERROR);

  mimdec = GST_MIMDEC (gst_pad_get_parent (pad));
  g_return_val_if_fail (GST_IS_MIMDEC (mimdec), GST_FLOW_ERROR);

  buf = GST_BUFFER (in);
  gst_adapter_push (mimdec->adapter, buf);

  GST_OBJECT_LOCK (mimdec);

  // do we have enough bytes to read a header
  while (gst_adapter_available (mimdec->adapter) >=
      (mimdec->have_header ? mimdec->payload_size : 24)) {
    if (!mimdec->have_header) {
      header = (guchar *) gst_adapter_peek (mimdec->adapter, 24);
      header_size = header[0];
      if (header_size != 24) {
        GST_WARNING_OBJECT (mimdec,
            "invalid frame: header size %d incorrect", header_size);
        gst_adapter_flush (mimdec->adapter, 24);
        res = GST_FLOW_ERROR;
        goto out;
      }

      if (header[1] == 1) {
        /* This is a a paused frame, skip it */
        gst_adapter_flush (mimdec->adapter, 24);
        continue;
      }

      fourcc = GUINT32_FROM_LE (*((guint32 *) (header + 12)));
      if (GST_MAKE_FOURCC ('M', 'L', '2', '0') != fourcc) {
        GST_WARNING_OBJECT (mimdec, "invalid frame: unknown FOURCC code"
            " %X (%" GST_FOURCC_FORMAT ")", fourcc, GST_FOURCC_ARGS (fourcc));
        gst_adapter_flush (mimdec->adapter, 24);
        res = GST_FLOW_ERROR;
        goto out;
      }

      mimdec->payload_size = GUINT32_FROM_LE (*((guint32 *) (header + 8)));

      mimdec->current_ts = GUINT32_FROM_LE (*((guint32 *) (header + 20)));

      GST_DEBUG ("Got packet, payload size %d", mimdec->payload_size);

      gst_adapter_flush (mimdec->adapter, 24);

      mimdec->have_header = TRUE;
    }

    if (gst_adapter_available (mimdec->adapter) < mimdec->payload_size) {
      goto out;
    }

    frame_body =
        (guchar *) gst_adapter_peek (mimdec->adapter, mimdec->payload_size);

    if (mimdec->dec == NULL) {
      GstEvent *event = NULL;
      gboolean result = TRUE;

      /* Check if its a keyframe, otherwise skip it */
      if (GUINT32_FROM_LE (*((guint32 *) (frame_body + 12))) != 0) {
        gst_adapter_flush (mimdec->adapter, mimdec->payload_size);
        mimdec->have_header = FALSE;
        res = GST_FLOW_OK;
        goto out;
      }

      mimdec->dec = mimic_open ();
      if (mimdec->dec == NULL) {
        GST_WARNING_OBJECT (mimdec, "mimic_open error\n");

        gst_adapter_flush (mimdec->adapter, mimdec->payload_size);
        mimdec->have_header = FALSE;
        res = GST_FLOW_ERROR;
        goto out;
      }

      if (!mimic_decoder_init (mimdec->dec, frame_body)) {
        GST_WARNING_OBJECT (mimdec, "mimic_decoder_init error\n");
        mimic_close (mimdec->dec);
        mimdec->dec = NULL;

        gst_adapter_flush (mimdec->adapter, mimdec->payload_size);
        mimdec->have_header = FALSE;
        res = GST_FLOW_ERROR;
        goto out;
      }

      if (!mimic_get_property (mimdec->dec, "buffer_size",
              &mimdec->buffer_size)) {
        GST_WARNING_OBJECT (mimdec,
            "mimic_get_property('buffer_size') error\n");
        mimic_close (mimdec->dec);
        mimdec->dec = NULL;

        gst_adapter_flush (mimdec->adapter, mimdec->payload_size);
        mimdec->have_header = FALSE;
        res = GST_FLOW_ERROR;
        goto out;
      }

      if (mimdec->need_newsegment)
        event = gst_event_new_new_segment (FALSE, 1.0, GST_FORMAT_TIME,
            mimdec->current_ts * GST_MSECOND, -1, 0);
      mimdec->need_newsegment = FALSE;
      GST_OBJECT_UNLOCK (mimdec);
      if (event)
        result = gst_pad_push_event (mimdec->srcpad, event);
      GST_OBJECT_LOCK (mimdec);
      if (!result) {
        GST_WARNING_OBJECT (mimdec, "gst_pad_push_event failed");
        res = GST_FLOW_ERROR;
        goto out;
      }
    }

    out_buf = gst_buffer_new_and_alloc (mimdec->buffer_size);

    if (!mimic_decode_frame (mimdec->dec, frame_body,
            GST_BUFFER_DATA (out_buf))) {
      GST_WARNING_OBJECT (mimdec, "mimic_decode_frame error\n");

      gst_adapter_flush (mimdec->adapter, mimdec->payload_size);
      mimdec->have_header = FALSE;

      gst_buffer_unref (out_buf);
      res = GST_FLOW_ERROR;
      goto out;
    }

    if (GST_CLOCK_TIME_IS_VALID (in_time))
      GST_BUFFER_TIMESTAMP (out_buf) = in_time;
    else
      GST_BUFFER_TIMESTAMP (out_buf) = mimdec->current_ts * GST_MSECOND;

    mimic_get_property (mimdec->dec, "width", &width);
    mimic_get_property (mimdec->dec, "height", &height);
    GST_DEBUG_OBJECT (mimdec,
        "got WxH %d x %d payload size %d buffer_size %d",
        width, height, mimdec->payload_size, mimdec->buffer_size);
    caps = gst_caps_new_simple ("video/x-raw-rgb",
        "bpp", G_TYPE_INT, 24,
        "depth", G_TYPE_INT, 24,
        "endianness", G_TYPE_INT, 4321,
        "framerate", GST_TYPE_FRACTION, 7, 1,
        "red_mask", G_TYPE_INT, 16711680,
        "green_mask", G_TYPE_INT, 65280,
        "blue_mask", G_TYPE_INT, 255,
        "width", G_TYPE_INT, width, "height", G_TYPE_INT, height, NULL);
    gst_buffer_set_caps (out_buf, caps);
    gst_caps_unref (caps);
    GST_OBJECT_UNLOCK (mimdec);
    res = gst_pad_push (mimdec->srcpad, out_buf);
    GST_OBJECT_LOCK (mimdec);

    gst_adapter_flush (mimdec->adapter, mimdec->payload_size);
    mimdec->have_header = FALSE;
  }

out:
  GST_OBJECT_UNLOCK (mimdec);
  gst_object_unref (mimdec);

  return res;
}

static GstStateChangeReturn
gst_mimdec_change_state (GstElement * element, GstStateChange transition)
{
  GstMimDec *mimdec;

  mimdec = GST_MIMDEC (element);

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_NULL:
      GST_OBJECT_LOCK (element);
      if (mimdec->dec != NULL) {
        mimic_close (mimdec->dec);
        mimdec->dec = NULL;
        mimdec->buffer_size = -1;
        mimdec->have_header = FALSE;
        mimdec->payload_size = -1;
        mimdec->current_ts = -1;
      }
      GST_OBJECT_UNLOCK (element);
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      GST_OBJECT_LOCK (element);
      mimdec->need_newsegment = TRUE;
      GST_OBJECT_UNLOCK (element);
      break;
    default:
      break;
  }

  return GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
}

static gboolean
gst_mimdec_sink_event (GstPad * pad, GstEvent * event)
{
  gboolean res = TRUE;
  GstMimDec *mimdec = GST_MIMDEC (gst_pad_get_parent (pad));

  /*
   * Ignore upstream newsegment event, its EVIL, we should implement
   * proper seeking instead
   */
  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_NEWSEGMENT:
    {
      gboolean update;
      GstFormat format;
      gdouble rate, arate;
      gint64 start, stop, time;

      gst_event_parse_new_segment_full (event, &update, &rate, &arate,
          &format, &start, &stop, &time);

      /* we need TIME and a positive rate */
      if (format != GST_FORMAT_TIME)
        goto newseg_wrong_format;

      if (rate <= 0.0)
        goto newseg_wrong_rate;

      GST_OBJECT_LOCK (mimdec);
      mimdec->need_newsegment = FALSE;
      GST_OBJECT_UNLOCK (mimdec);

      res = gst_pad_push_event (mimdec->srcpad, event);
    }
      break;
    case GST_EVENT_FLUSH_STOP:
      GST_OBJECT_LOCK (mimdec);
      mimdec->need_newsegment = TRUE;
      GST_OBJECT_UNLOCK (mimdec);

      res = gst_pad_push_event (mimdec->srcpad, event);
      break;
    default:
      res = gst_pad_push_event (mimdec->srcpad, event);
      break;
  }

done:

  gst_object_unref (mimdec);

  return res;

newseg_wrong_format:
  {
    GST_DEBUG_OBJECT (mimdec, "received non TIME newsegment");
    gst_event_unref (event);
    goto done;
  }
newseg_wrong_rate:
  {
    GST_DEBUG_OBJECT (mimdec, "negative rates not supported yet");
    gst_event_unref (event);
    goto done;
  }


}
