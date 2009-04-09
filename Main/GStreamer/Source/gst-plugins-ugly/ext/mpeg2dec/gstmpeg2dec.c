/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
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

#include <inttypes.h>

#include "gstmpeg2dec.h"

/* 16byte-aligns a buffer for libmpeg2 */
#define ALIGN_16(p) ((void *)(((uintptr_t)(p) + 15) & ~((uintptr_t)15)))

/* mpeg2dec changed a struct name after 0.3.1, here's a workaround */
/* mpeg2dec also only defined MPEG2_RELEASE after 0.3.1
   #if MPEG2_RELEASE < MPEG2_VERSION(0,3,2)
*/
#ifndef MPEG2_RELEASE
#define MPEG2_VERSION(a,b,c) ((((a)&0xff)<<16)|(((b)&0xff)<<8)|((c)&0xff))
#define MPEG2_RELEASE MPEG2_VERSION(0,3,1)
typedef picture_t mpeg2_picture_t;
typedef gint mpeg2_state_t;

#define STATE_BUFFER 0
#endif

GST_DEBUG_CATEGORY_STATIC (mpeg2dec_debug);
#define GST_CAT_DEFAULT (mpeg2dec_debug)

/* elementfactory information */
static const GstElementDetails gst_mpeg2dec_details =
GST_ELEMENT_DETAILS ("mpeg1 and mpeg2 video decoder",
    "Codec/Decoder/Video",
    "Uses libmpeg2 to decode MPEG video streams",
    "Wim Taymans <wim.taymans@chello.be>");

/* Send a warning message about decoding errors after receiving this many
 * STATE_INVALID return values from mpeg2_parse. -1 means never.
 */
#define WARN_THRESHOLD (5)

//#define enable_user_data
#ifdef enable_user_data
static GstStaticPadTemplate user_data_template_factory =
GST_STATIC_PAD_TEMPLATE ("user_data",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);
#endif

static GstStaticPadTemplate sink_template_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/mpeg, "
        "mpegversion = (int) [ 1, 2 ], " "systemstream = (boolean) false")
    );

static GstStaticPadTemplate src_template_factory =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw-yuv, "
        "format = (fourcc) { YV12, I420, Y42B, Y444 }, "
        "width = (int) [ 16, 4096 ], "
        "height = (int) [ 16, 4096 ], "
        "framerate = (fraction) [ 0/1, 2147483647/1 ]")
    );

static void gst_mpeg2dec_base_init (gpointer g_class);
static void gst_mpeg2dec_class_init (GstMpeg2decClass * klass);
static void gst_mpeg2dec_init (GstMpeg2dec * mpeg2dec);

static void gst_mpeg2dec_finalize (GObject * object);
static void gst_mpeg2dec_reset (GstMpeg2dec * mpeg2dec);

static void gst_mpeg2dec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_mpeg2dec_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

#ifndef GST_DISABLE_INDEX
static void gst_mpeg2dec_set_index (GstElement * element, GstIndex * index);
static GstIndex *gst_mpeg2dec_get_index (GstElement * element);
#endif

static gboolean gst_mpeg2dec_src_event (GstPad * pad, GstEvent * event);
static const GstQueryType *gst_mpeg2dec_get_src_query_types (GstPad * pad);

static gboolean gst_mpeg2dec_src_query (GstPad * pad, GstQuery * query);

static gboolean gst_mpeg2dec_sink_convert (GstPad * pad, GstFormat src_format,
    gint64 src_value, GstFormat * dest_format, gint64 * dest_value);
static gboolean gst_mpeg2dec_src_convert (GstPad * pad, GstFormat src_format,
    gint64 src_value, GstFormat * dest_format, gint64 * dest_value);

static GstStateChangeReturn gst_mpeg2dec_change_state (GstElement * element,
    GstStateChange transition);

static gboolean gst_mpeg2dec_sink_event (GstPad * pad, GstEvent * event);
static gboolean gst_mpeg2dec_setcaps (GstPad * pad, GstCaps * caps);
static GstFlowReturn gst_mpeg2dec_chain (GstPad * pad, GstBuffer * buf);

static void clear_buffers (GstMpeg2dec * mpeg2dec);

//static gboolean gst_mpeg2dec_sink_query (GstPad * pad, GstQuery * query);

#if 0
static const GstFormat *gst_mpeg2dec_get_formats (GstPad * pad);
#endif

#if 0
static const GstEventMask *gst_mpeg2dec_get_event_masks (GstPad * pad);
#endif

static GstElementClass *parent_class = NULL;

static gboolean crop_buffer (GstMpeg2dec * mpeg2dec, GstBuffer ** buf);

/*static guint gst_mpeg2dec_signals[LAST_SIGNAL] = { 0 };*/

GType
gst_mpeg2dec_get_type (void)
{
  static GType mpeg2dec_type = 0;

  if (!mpeg2dec_type) {
    static const GTypeInfo mpeg2dec_info = {
      sizeof (GstMpeg2decClass),
      gst_mpeg2dec_base_init,
      NULL,
      (GClassInitFunc) gst_mpeg2dec_class_init,
      NULL,
      NULL,
      sizeof (GstMpeg2dec),
      0,
      (GInstanceInitFunc) gst_mpeg2dec_init,
    };

    mpeg2dec_type =
        g_type_register_static (GST_TYPE_ELEMENT, "GstMpeg2dec", &mpeg2dec_info,
        0);
  }

  GST_DEBUG_CATEGORY_INIT (mpeg2dec_debug, "mpeg2dec", 0,
      "MPEG2 decoder element");

  return mpeg2dec_type;
}

static void
gst_mpeg2dec_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_template_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_template_factory));
#ifdef enable_user_data
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&user_data_template_factory));
#endif
  gst_element_class_set_details (element_class, &gst_mpeg2dec_details);
}

static void
gst_mpeg2dec_class_init (GstMpeg2decClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->set_property = gst_mpeg2dec_set_property;
  gobject_class->get_property = gst_mpeg2dec_get_property;
  gobject_class->finalize = gst_mpeg2dec_finalize;

  gstelement_class->change_state = gst_mpeg2dec_change_state;
#ifndef GST_DISABLE_INDEX
  gstelement_class->set_index = gst_mpeg2dec_set_index;
  gstelement_class->get_index = gst_mpeg2dec_get_index;
#endif
}

static void
gst_mpeg2dec_init (GstMpeg2dec * mpeg2dec)
{
  /* create the sink and src pads */
  mpeg2dec->sinkpad =
      gst_pad_new_from_static_template (&sink_template_factory, "sink");
  gst_pad_set_chain_function (mpeg2dec->sinkpad,
      GST_DEBUG_FUNCPTR (gst_mpeg2dec_chain));
#if 0
  gst_pad_set_query_function (mpeg2dec->sinkpad,
      GST_DEBUG_FUNCPTR (gst_mpeg2dec_get_sink_query));
#endif
  gst_pad_set_event_function (mpeg2dec->sinkpad,
      GST_DEBUG_FUNCPTR (gst_mpeg2dec_sink_event));
  gst_pad_set_setcaps_function (mpeg2dec->sinkpad,
      GST_DEBUG_FUNCPTR (gst_mpeg2dec_setcaps));
  gst_element_add_pad (GST_ELEMENT (mpeg2dec), mpeg2dec->sinkpad);

  mpeg2dec->srcpad =
      gst_pad_new_from_static_template (&src_template_factory, "src");
  gst_pad_set_event_function (mpeg2dec->srcpad,
      GST_DEBUG_FUNCPTR (gst_mpeg2dec_src_event));
  gst_pad_set_query_type_function (mpeg2dec->srcpad,
      GST_DEBUG_FUNCPTR (gst_mpeg2dec_get_src_query_types));
  gst_pad_set_query_function (mpeg2dec->srcpad,
      GST_DEBUG_FUNCPTR (gst_mpeg2dec_src_query));
  gst_pad_use_fixed_caps (mpeg2dec->srcpad);
  gst_element_add_pad (GST_ELEMENT (mpeg2dec), mpeg2dec->srcpad);

#ifdef enable_user_data
  mpeg2dec->userdatapad =
      gst_pad_new_from_static_template (&user_data_template_factory,
      "user_data");
  gst_element_add_pad (GST_ELEMENT (mpeg2dec), mpeg2dec->userdatapad);
#endif

  mpeg2dec->error_count = 0;
  mpeg2dec->can_allocate_aligned = TRUE;

  /* initialize the mpeg2dec acceleration */
}

static void
gst_mpeg2dec_finalize (GObject * object)
{
  GstMpeg2dec *mpeg2dec = GST_MPEG2DEC (object);

  if (mpeg2dec->decoder) {
    GST_DEBUG_OBJECT (mpeg2dec, "closing decoder");
    mpeg2_close (mpeg2dec->decoder);
    mpeg2dec->decoder = NULL;
  }
  clear_buffers (mpeg2dec);
  g_free (mpeg2dec->dummybuf[3]);
  mpeg2dec->dummybuf[3] = NULL;

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_mpeg2dec_reset (GstMpeg2dec * mpeg2dec)
{
  /* reset the initial video state */
  mpeg2dec->format = MPEG2DEC_FORMAT_NONE;
  mpeg2dec->width = -1;
  mpeg2dec->height = -1;
  gst_segment_init (&mpeg2dec->segment, GST_FORMAT_UNDEFINED);
  mpeg2dec->discont_state = MPEG2DEC_DISC_NEW_PICTURE;
  mpeg2dec->frame_period = 0;
  mpeg2dec->need_sequence = TRUE;
  mpeg2dec->next_time = -1;
  mpeg2dec->offset = 0;
  mpeg2dec->error_count = 0;
  mpeg2dec->can_allocate_aligned = TRUE;
  mpeg2_reset (mpeg2dec->decoder, 1);
}

static void
gst_mpeg2dec_qos_reset (GstMpeg2dec * mpeg2dec)
{
  GST_OBJECT_LOCK (mpeg2dec);
  mpeg2dec->proportion = 1.0;
  mpeg2dec->earliest_time = -1;
  GST_OBJECT_UNLOCK (mpeg2dec);
}

#ifndef GST_DISABLE_INDEX
static void
gst_mpeg2dec_set_index (GstElement * element, GstIndex * index)
{
  GstMpeg2dec *mpeg2dec = GST_MPEG2DEC (element);

  mpeg2dec->index = index;

  gst_index_get_writer_id (index, GST_OBJECT (element), &mpeg2dec->index_id);
}

static GstIndex *
gst_mpeg2dec_get_index (GstElement * element)
{
  GstMpeg2dec *mpeg2dec = GST_MPEG2DEC (element);

  return mpeg2dec->index;
}
#endif

/* see gst-plugins/gst/games/gstvideoimage.c, paint_setup_I420() */
#define I420_Y_ROWSTRIDE(width) (GST_ROUND_UP_4(width))
#define I420_U_ROWSTRIDE(width) (GST_ROUND_UP_8(width)/2)
#define I420_V_ROWSTRIDE(width) ((GST_ROUND_UP_8(I420_Y_ROWSTRIDE(width)))/2)

#define I420_Y_OFFSET(w,h) (0)
#define I420_U_OFFSET(w,h) (I420_Y_OFFSET(w,h)+(I420_Y_ROWSTRIDE(w)*GST_ROUND_UP_2(h)))
#define I420_V_OFFSET(w,h) (I420_U_OFFSET(w,h)+(I420_U_ROWSTRIDE(w)*GST_ROUND_UP_2(h)/2))

#define I420_SIZE(w,h)     (I420_V_OFFSET(w,h)+(I420_V_ROWSTRIDE(w)*GST_ROUND_UP_2(h)/2))

static GstBuffer *
crop_copy_i420_buffer (GstMpeg2dec * mpeg2dec, GstBuffer * input)
{
  GstBuffer *outbuf;
  guint8 *dest, *src;
  guint outsize, line;

  outsize = I420_SIZE (mpeg2dec->width, mpeg2dec->height);
  GST_LOG_OBJECT (mpeg2dec, "Copying input buffer %ux%u (%u) to output buffer "
      "%ux%u (%u)", mpeg2dec->decoded_width, mpeg2dec->decoded_height,
      GST_BUFFER_SIZE (input), mpeg2dec->width, mpeg2dec->height, outsize);
  outbuf = gst_buffer_new_and_alloc (outsize);

  /* Copy Y first */
  src = GST_BUFFER_DATA (input);
  dest = GST_BUFFER_DATA (outbuf);
  for (line = 0; line < mpeg2dec->height; line++) {
    memcpy (dest, src, mpeg2dec->width);
    dest += I420_Y_ROWSTRIDE (mpeg2dec->width);
    src += I420_Y_ROWSTRIDE (mpeg2dec->decoded_width);
  }

  /* U */
  src = GST_BUFFER_DATA (input)
      + I420_U_OFFSET (mpeg2dec->decoded_width, mpeg2dec->decoded_height);
  dest = GST_BUFFER_DATA (outbuf)
      + I420_U_OFFSET (mpeg2dec->width, mpeg2dec->height);
  for (line = 0; line < mpeg2dec->height / 2; line++) {
    memcpy (dest, src, mpeg2dec->width / 2);
    dest += I420_U_ROWSTRIDE (mpeg2dec->width);
    src += I420_U_ROWSTRIDE (mpeg2dec->decoded_width);
  }

  /* V */
  src = GST_BUFFER_DATA (input)
      + I420_V_OFFSET (mpeg2dec->decoded_width, mpeg2dec->decoded_height);
  dest = GST_BUFFER_DATA (outbuf)
      + I420_V_OFFSET (mpeg2dec->width, mpeg2dec->height);
  for (line = 0; line < mpeg2dec->height / 2; line++) {
    memcpy (dest, src, mpeg2dec->width / 2);
    dest += I420_V_ROWSTRIDE (mpeg2dec->width);
    src += I420_V_ROWSTRIDE (mpeg2dec->decoded_width);
  }

  return outbuf;
}

  /* FIXME: this is unlikely to be right stride-wise and offset-wise */
static GstBuffer *
crop_copy_i422_buffer (GstMpeg2dec * mpeg2dec, GstBuffer * input)
{
  GstBuffer *outbuf;
  guint8 *in_data, *out_data;
  guint line;

  outbuf = gst_buffer_new_and_alloc (mpeg2dec->width * mpeg2dec->height * 2);

  /* Copy Y first */
  in_data = GST_BUFFER_DATA (input);
  out_data = GST_BUFFER_DATA (outbuf);
  for (line = 0; line < mpeg2dec->height; line++) {
    memcpy (out_data, in_data, mpeg2dec->width);
    out_data += mpeg2dec->width;
    in_data += mpeg2dec->decoded_width;
  }

  /* Now copy U & V */
  in_data = GST_BUFFER_DATA (input)
      + mpeg2dec->decoded_width * mpeg2dec->decoded_height;
  for (line = 0; line < mpeg2dec->height; line++) {
    memcpy (out_data, in_data, mpeg2dec->width / 2);
    memcpy (out_data + mpeg2dec->width * mpeg2dec->height / 2,
        in_data + mpeg2dec->decoded_width * mpeg2dec->decoded_height / 2,
        mpeg2dec->width / 2);
    out_data += mpeg2dec->width / 2;
    in_data += mpeg2dec->decoded_width / 2;
  }

  return outbuf;
}

static gboolean
crop_buffer (GstMpeg2dec * mpeg2dec, GstBuffer ** buf)
{
  gboolean result = TRUE;
  GstBuffer *input = *buf;
  GstBuffer *outbuf = input;

  /*We crop only if the target region is smaller than the input one */
  if ((mpeg2dec->decoded_width > mpeg2dec->width) ||
      (mpeg2dec->decoded_height > mpeg2dec->height)) {
    /* If we don't know about the format, we just return the original
     * buffer.
     */
    if (mpeg2dec->format == MPEG2DEC_FORMAT_I422 ||
        mpeg2dec->format == MPEG2DEC_FORMAT_I420 ||
        mpeg2dec->format == MPEG2DEC_FORMAT_YV12) {
      /*FIXME:  I have tried to use gst_buffer_copy_on_write, but it
       *        still have some artifact, so I'me allocating new buffer
       *        for each frame decoded...
       */
      if (mpeg2dec->format == MPEG2DEC_FORMAT_I422) {
        outbuf = crop_copy_i422_buffer (mpeg2dec, input);
      } else {
        outbuf = crop_copy_i420_buffer (mpeg2dec, input);
      }

      gst_buffer_set_caps (outbuf, GST_PAD_CAPS (mpeg2dec->srcpad));
      gst_buffer_copy_metadata (outbuf, input, GST_BUFFER_COPY_TIMESTAMPS);
      gst_buffer_unref (input);

      *buf = outbuf;
      result = TRUE;
    }
  }

  return result;
}

static GstFlowReturn
gst_mpeg2dec_alloc_sized_buf (GstMpeg2dec * mpeg2dec, guint size,
    GstBuffer ** obuf)
{
  if (mpeg2dec->can_allocate_aligned
      && mpeg2dec->decoded_width == mpeg2dec->width
      && mpeg2dec->decoded_height == mpeg2dec->height) {
    GstFlowReturn ret;

    ret = gst_pad_alloc_buffer_and_set_caps (mpeg2dec->srcpad,
        GST_BUFFER_OFFSET_NONE, size, GST_PAD_CAPS (mpeg2dec->srcpad), obuf);
    if (ret != GST_FLOW_OK) {
      return ret;
    }

    /* libmpeg2 needs 16 byte aligned buffers... test for this here
     * and if it fails only a single time create our own buffers from
     * there on below that are correctly aligned */
    if (((uintptr_t) GST_BUFFER_DATA (*obuf)) % 16 == 0) {
      GST_LOG_OBJECT (mpeg2dec, "return 16 byte aligned buffer");
      return ret;
    }

    GST_DEBUG_OBJECT (mpeg2dec,
        "can't get 16 byte aligned buffers, creating our own ones");
    gst_buffer_unref (*obuf);
    mpeg2dec->can_allocate_aligned = FALSE;
  }

  /* can't use gst_pad_alloc_buffer() here because the output buffer will
   * either be cropped later or be bigger than expected (for the alignment),
   * and basetransform-based elements will complain about the wrong unit size
   * when not operating in passthrough mode */
  *obuf = gst_buffer_new_and_alloc (size + 15);
  GST_BUFFER_DATA (*obuf) = (guint8 *) ALIGN_16 (GST_BUFFER_DATA (*obuf));
  GST_BUFFER_SIZE (*obuf) = size;
  gst_buffer_set_caps (*obuf, GST_PAD_CAPS (mpeg2dec->srcpad));

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_mpeg2dec_alloc_buffer (GstMpeg2dec * mpeg2dec, gint64 offset,
    GstBuffer ** obuf)
{
  GstBuffer *outbuf = NULL;
  guint8 *buf[3];
  GstFlowReturn ret = GST_FLOW_OK;

  ret = gst_mpeg2dec_alloc_sized_buf (mpeg2dec, mpeg2dec->size, &outbuf);
  if (ret != GST_FLOW_OK)
    goto no_buffer;

  buf[0] = GST_BUFFER_DATA (outbuf);
  buf[1] = buf[0] + mpeg2dec->u_offs;
  buf[2] = buf[0] + mpeg2dec->v_offs;

  GST_DEBUG_OBJECT (mpeg2dec, "set_buf: %p %p %p, outbuf %p",
      buf[0], buf[1], buf[2], outbuf);

  mpeg2_set_buf (mpeg2dec->decoder, buf, outbuf);

  /* we store the original byteoffset of this picture in the stream here
   * because we need it for indexing */
  GST_BUFFER_OFFSET (outbuf) = offset;

  *obuf = outbuf;
  return ret;

  /* ERRORS */
no_buffer:
  {
    if (GST_FLOW_IS_FATAL (ret)) {
      GST_ELEMENT_ERROR (mpeg2dec, RESOURCE, FAILED, (NULL),
          ("Failed to allocate memory for buffer, reason %s",
              gst_flow_get_name (ret)));
    }
    GST_DEBUG_OBJECT (mpeg2dec, "no output buffer, reason %s",
        gst_flow_get_name (ret));
    mpeg2_set_buf (mpeg2dec->decoder, mpeg2dec->dummybuf, NULL);
    *obuf = NULL;
    return ret;
  }
}

static gboolean
gst_mpeg2dec_negotiate_format (GstMpeg2dec * mpeg2dec)
{
  GstCaps *caps;
  guint32 fourcc;
  const mpeg2_info_t *info;
  const mpeg2_sequence_t *sequence;

  info = mpeg2_info (mpeg2dec->decoder);
  sequence = info->sequence;

  if (sequence->width != sequence->chroma_width &&
      sequence->height != sequence->chroma_height) {

    fourcc = GST_STR_FOURCC ("I420");
    mpeg2dec->format = MPEG2DEC_FORMAT_I420;
    mpeg2dec->size =
        I420_SIZE (mpeg2dec->decoded_width, mpeg2dec->decoded_height);

    mpeg2dec->u_offs =
        I420_U_OFFSET (mpeg2dec->decoded_width, mpeg2dec->decoded_height);
    mpeg2dec->v_offs =
        I420_V_OFFSET (mpeg2dec->decoded_width, mpeg2dec->decoded_height);

  } else if ((sequence->width == sequence->chroma_width &&
          sequence->height != sequence->chroma_height) ||
      (sequence->width != sequence->chroma_width &&
          sequence->height == sequence->chroma_height)) {
    gint halfsize;

    fourcc = GST_STR_FOURCC ("Y42B");
    mpeg2dec->format = MPEG2DEC_FORMAT_I422;
    halfsize = mpeg2dec->decoded_width * mpeg2dec->decoded_height;
    mpeg2dec->size = halfsize * 2;
    mpeg2dec->u_offs = halfsize;
    mpeg2dec->v_offs = halfsize + (halfsize / 2);
  } else {
    gint size;

    size = mpeg2dec->decoded_width * mpeg2dec->decoded_height;

    fourcc = GST_STR_FOURCC ("Y444");
    mpeg2dec->format = MPEG2DEC_FORMAT_Y444;
    mpeg2dec->size = size * 3;
    mpeg2dec->u_offs = size;
    mpeg2dec->v_offs = size * 2;
  }

  if (mpeg2dec->pixel_width == 0 || mpeg2dec->pixel_height == 0) {
    GValue par = { 0, }
    , dar = {
    0,}
    , dimensions = {
    0,};

    /* assume display aspect ratio (DAR) of 4:3 */
    g_value_init (&dar, GST_TYPE_FRACTION);
    gst_value_set_fraction (&dar, 4, 3);
    g_value_init (&dimensions, GST_TYPE_FRACTION);
    gst_value_set_fraction (&dimensions, mpeg2dec->height, mpeg2dec->width);

    g_value_init (&par, GST_TYPE_FRACTION);
    if (!gst_value_fraction_multiply (&par, &dar, &dimensions)) {
      gst_value_set_fraction (&dimensions, 1, 1);
    }

    mpeg2dec->pixel_width = gst_value_get_fraction_numerator (&par);
    mpeg2dec->pixel_height = gst_value_get_fraction_denominator (&par);

    GST_WARNING_OBJECT (mpeg2dec, "Unknown pixel-aspect-ratio, assuming %d:%d",
        mpeg2dec->pixel_width, mpeg2dec->pixel_height);

    g_value_unset (&par);
    g_value_unset (&dar);
    g_value_unset (&dimensions);
  }

  caps = gst_caps_new_simple ("video/x-raw-yuv",
      "format", GST_TYPE_FOURCC, fourcc,
      "width", G_TYPE_INT, mpeg2dec->width,
      "height", G_TYPE_INT, mpeg2dec->height,
      "pixel-aspect-ratio", GST_TYPE_FRACTION, mpeg2dec->pixel_width,
      mpeg2dec->pixel_height,
      "framerate", GST_TYPE_FRACTION, mpeg2dec->fps_n, mpeg2dec->fps_d, NULL);

  gst_pad_set_caps (mpeg2dec->srcpad, caps);
  gst_caps_unref (caps);

  return TRUE;
}

static void
init_dummybuf (GstMpeg2dec * mpeg2dec)
{
  g_free (mpeg2dec->dummybuf[3]);

  /* libmpeg2 needs 16 byte aligned buffers... care for this here */
  mpeg2dec->dummybuf[3] = g_malloc0 (mpeg2dec->size + 15);
  mpeg2dec->dummybuf[0] = ALIGN_16 (mpeg2dec->dummybuf[3]);
  mpeg2dec->dummybuf[1] = mpeg2dec->dummybuf[0] + mpeg2dec->u_offs;
  mpeg2dec->dummybuf[2] = mpeg2dec->dummybuf[0] + mpeg2dec->v_offs;
}

static GstFlowReturn
handle_sequence (GstMpeg2dec * mpeg2dec, const mpeg2_info_t * info)
{
  GstFlowReturn ret = GST_FLOW_OK;

  if (info->sequence->frame_period == 0) {
    GST_WARNING_OBJECT (mpeg2dec, "Frame period is 0!");
    ret = GST_FLOW_ERROR;
    goto done;
  }

  mpeg2dec->width = info->sequence->picture_width;
  mpeg2dec->height = info->sequence->picture_height;
  mpeg2dec->decoded_width = info->sequence->width;
  mpeg2dec->decoded_height = info->sequence->height;
  mpeg2dec->total_frames = 0;

  /* don't take the sequence PAR if we already have one from the sink caps */
  if (!mpeg2dec->have_par) {
    mpeg2dec->pixel_width = info->sequence->pixel_width;
    mpeg2dec->pixel_height = info->sequence->pixel_height;
  }

  /* mpeg2 video can only be from 16x16 to 4096x4096. Everything
   * else is a corrupted files */
  if (mpeg2dec->width > 4096 || mpeg2dec->width < 16 ||
      mpeg2dec->height > 4096 || mpeg2dec->height < 16) {
    GST_ERROR_OBJECT (mpeg2dec, "Invalid frame dimensions: %d x %d",
        mpeg2dec->width, mpeg2dec->height);
    return GST_FLOW_ERROR;
  }

  /* set framerate */
  mpeg2dec->fps_n = 27000000;
  mpeg2dec->fps_d = info->sequence->frame_period;
  mpeg2dec->frame_period = info->sequence->frame_period * GST_USECOND / 27;

  GST_DEBUG_OBJECT (mpeg2dec,
      "sequence flags: %d, frame period: %d (%g), frame rate: %d/%d",
      info->sequence->flags, info->sequence->frame_period,
      (double) (mpeg2dec->frame_period) / GST_SECOND, mpeg2dec->fps_n,
      mpeg2dec->fps_d);
  GST_DEBUG_OBJECT (mpeg2dec, "profile: %02x, colour_primaries: %d",
      info->sequence->profile_level_id, info->sequence->colour_primaries);
  GST_DEBUG_OBJECT (mpeg2dec, "transfer chars: %d, matrix coef: %d",
      info->sequence->transfer_characteristics,
      info->sequence->matrix_coefficients);

  if (!gst_mpeg2dec_negotiate_format (mpeg2dec))
    goto negotiate_failed;

  mpeg2_custom_fbuf (mpeg2dec->decoder, 1);

  init_dummybuf (mpeg2dec);

  /* Pump in some null buffers, because otherwise libmpeg2 doesn't 
   * initialise the discard_fbuf->id */
  mpeg2_set_buf (mpeg2dec->decoder, mpeg2dec->dummybuf, NULL);
  mpeg2_set_buf (mpeg2dec->decoder, mpeg2dec->dummybuf, NULL);
  mpeg2_set_buf (mpeg2dec->decoder, mpeg2dec->dummybuf, NULL);

  mpeg2dec->need_sequence = FALSE;

done:
  return ret;

negotiate_failed:
  {
    GST_ELEMENT_ERROR (mpeg2dec, CORE, NEGOTIATION, (NULL), (NULL));
    ret = GST_FLOW_NOT_NEGOTIATED;
    goto done;
  }
}

static void
clear_buffers (GstMpeg2dec * mpeg2dec)
{
  gint i;
  GstBuffer **bufpen;

  for (i = 0; i < 4; i++) {
    bufpen = &mpeg2dec->ip_buffers[i];
    if (*bufpen)
      gst_buffer_unref (*bufpen);
    *bufpen = NULL;
  }
  bufpen = &mpeg2dec->b_buffer;
  if (*bufpen)
    gst_buffer_unref (*bufpen);
  *bufpen = NULL;
}

static void
clear_queued (GstMpeg2dec * mpeg2dec)
{
  g_list_foreach (mpeg2dec->queued, (GFunc) gst_mini_object_unref, NULL);
  g_list_free (mpeg2dec->queued);
  mpeg2dec->queued = NULL;
}

static GstFlowReturn
flush_queued (GstMpeg2dec * mpeg2dec)
{
  GstFlowReturn res = GST_FLOW_OK;

  while (mpeg2dec->queued) {
    GstBuffer *buf = GST_BUFFER_CAST (mpeg2dec->queued->data);

    GST_LOG_OBJECT (mpeg2dec, "pushing buffer %p, timestamp %"
        GST_TIME_FORMAT ", duration %" GST_TIME_FORMAT, buf,
        GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (buf)),
        GST_TIME_ARGS (GST_BUFFER_DURATION (buf)));

    /* iterate ouput queue an push downstream */
    res = gst_pad_push (mpeg2dec->srcpad, buf);

    mpeg2dec->queued = g_list_delete_link (mpeg2dec->queued, mpeg2dec->queued);
  }
  return res;
}

static GstFlowReturn
handle_picture (GstMpeg2dec * mpeg2dec, const mpeg2_info_t * info)
{
  gboolean key_frame = FALSE;
  GstBuffer *outbuf, **bufpen;
  GstFlowReturn ret;
  gint type;

  ret = gst_mpeg2dec_alloc_buffer (mpeg2dec, mpeg2dec->offset, &outbuf);
  if (ret != GST_FLOW_OK)
    goto no_buffer;

  if (info->current_picture) {
    type = info->current_picture->flags & PIC_MASK_CODING_TYPE;
  } else {
    type = 0;
  }

  GST_DEBUG_OBJECT (mpeg2dec, "handle picture type %d", type);

  key_frame = type == PIC_FLAG_CODING_TYPE_I;

  switch (type) {
    case PIC_FLAG_CODING_TYPE_I:
      mpeg2_skip (mpeg2dec->decoder, 0);
      if (mpeg2dec->segment.rate < 0.0) {
        /* negative rate, flush the queued pictures in reverse */
        GST_DEBUG_OBJECT (mpeg2dec, "flushing queued buffers");
        flush_queued (mpeg2dec);
      }
    case PIC_FLAG_CODING_TYPE_P:
      bufpen = &mpeg2dec->ip_buffers[mpeg2dec->ip_bufpos];
      GST_DEBUG_OBJECT (mpeg2dec, "I/P unref %p, ref %p", *bufpen, outbuf);
      if (*bufpen)
        gst_buffer_unref (*bufpen);
      *bufpen = outbuf;
      mpeg2dec->ip_bufpos = (mpeg2dec->ip_bufpos + 1) & 3;
      break;
    case PIC_FLAG_CODING_TYPE_B:
      bufpen = &mpeg2dec->b_buffer;
      GST_DEBUG_OBJECT (mpeg2dec, "B unref %p, ref %p", *bufpen, outbuf);
      if (*bufpen)
        gst_buffer_unref (*bufpen);
      *bufpen = outbuf;
      break;
    default:
      break;
  }

  GST_DEBUG_OBJECT (mpeg2dec, "picture %s, outbuf %p, offset %"
      G_GINT64_FORMAT,
      key_frame ? ", kf," : "    ", outbuf, GST_BUFFER_OFFSET (outbuf)
      );

  if (mpeg2dec->discont_state == MPEG2DEC_DISC_NEW_PICTURE && key_frame) {
    mpeg2dec->discont_state = MPEG2DEC_DISC_NEW_KEYFRAME;
  }

  return ret;

no_buffer:
  {
    return ret;
  }
}

/* try to clip the buffer to the segment boundaries */
static gboolean
clip_buffer (GstMpeg2dec * dec, GstBuffer * buf)
{
  gboolean res = TRUE;
  GstClockTime in_ts, in_dur, stop;
  gint64 cstart, cstop;

  in_ts = GST_BUFFER_TIMESTAMP (buf);
  in_dur = GST_BUFFER_DURATION (buf);

  GST_LOG_OBJECT (dec,
      "timestamp:%" GST_TIME_FORMAT " , duration:%" GST_TIME_FORMAT,
      GST_TIME_ARGS (in_ts), GST_TIME_ARGS (in_dur));

  /* can't clip without TIME segment */
  if (dec->segment.format != GST_FORMAT_TIME)
    goto beach;

  /* we need a start time */
  if (!GST_CLOCK_TIME_IS_VALID (in_ts))
    goto beach;

  /* generate valid stop, if duration unknown, we have unknown stop */
  stop =
      GST_CLOCK_TIME_IS_VALID (in_dur) ? (in_ts + in_dur) : GST_CLOCK_TIME_NONE;

  /* now clip */
  if (!(res = gst_segment_clip (&dec->segment, GST_FORMAT_TIME,
              in_ts, stop, &cstart, &cstop)))
    goto beach;

  /* update timestamp and possibly duration if the clipped stop time is
   * valid */
  GST_BUFFER_TIMESTAMP (buf) = cstart;
  if (GST_CLOCK_TIME_IS_VALID (cstop))
    GST_BUFFER_DURATION (buf) = cstop - cstart;

beach:
  GST_LOG_OBJECT (dec, "%sdropping", (res ? "not " : ""));
  return res;
}

static GstFlowReturn
handle_slice (GstMpeg2dec * mpeg2dec, const mpeg2_info_t * info)
{
  GstBuffer *outbuf = NULL;
  GstFlowReturn ret = GST_FLOW_OK;
  const mpeg2_picture_t *picture;
  gboolean key_frame = FALSE;
  GstClockTime time;

  GST_DEBUG_OBJECT (mpeg2dec, "picture slice/end %p %p %p %p",
      info->display_fbuf,
      info->display_picture, info->current_picture,
      (info->display_fbuf ? info->display_fbuf->id : NULL));

  if (!info->display_fbuf || !info->display_fbuf->id)
    goto no_display;

  outbuf = GST_BUFFER (info->display_fbuf->id);

  picture = info->display_picture;

  key_frame = (picture->flags & PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_I;

  GST_DEBUG_OBJECT (mpeg2dec, "picture flags: %d, type: %d, keyframe: %d",
      picture->flags, picture->flags & PIC_MASK_CODING_TYPE, key_frame);

  if (key_frame) {
    GST_BUFFER_FLAG_UNSET (outbuf, GST_BUFFER_FLAG_DELTA_UNIT);
    mpeg2_skip (mpeg2dec->decoder, 0);
  } else {
    GST_BUFFER_FLAG_SET (outbuf, GST_BUFFER_FLAG_DELTA_UNIT);
  }

  if (mpeg2dec->discont_state == MPEG2DEC_DISC_NEW_KEYFRAME && key_frame)
    mpeg2dec->discont_state = MPEG2DEC_DISC_NONE;

  time = GST_CLOCK_TIME_NONE;

#if MPEG2_RELEASE < MPEG2_VERSION(0,4,0)
  if (picture->flags & PIC_FLAG_PTS) {
    time = MPEG_TIME_TO_GST_TIME (picture->pts);
    GST_DEBUG_OBJECT (mpeg2dec, "picture pts %" G_GUINT64_FORMAT
        ", time %" GST_TIME_FORMAT, picture->pts, GST_TIME_ARGS (time));
  }
#else
  if (picture->flags & PIC_FLAG_TAGS) {
    guint64 pts = (((guint64) picture->tag2) << 32) | picture->tag;

    time = MPEG_TIME_TO_GST_TIME (pts);
    GST_DEBUG_OBJECT (mpeg2dec, "picture tags %" G_GUINT64_FORMAT
        ", time %" GST_TIME_FORMAT, pts, GST_TIME_ARGS (time));
  }
#endif

  if (time == GST_CLOCK_TIME_NONE) {
    time = mpeg2dec->next_time;
    GST_DEBUG_OBJECT (mpeg2dec, "picture didn't have pts");
  } else {
    GST_DEBUG_OBJECT (mpeg2dec,
        "picture had pts %" GST_TIME_FORMAT ", we had %"
        GST_TIME_FORMAT, GST_TIME_ARGS (time),
        GST_TIME_ARGS (mpeg2dec->next_time));
    mpeg2dec->next_time = time;
  }
  GST_BUFFER_TIMESTAMP (outbuf) = time;

  /* TODO set correct offset here based on frame number */
  if (info->display_picture_2nd) {
    GST_BUFFER_DURATION (outbuf) = (picture->nb_fields +
        info->display_picture_2nd->nb_fields) * mpeg2dec->frame_period / 2;
  } else {
    GST_BUFFER_DURATION (outbuf) =
        picture->nb_fields * mpeg2dec->frame_period / 2;
  }
  mpeg2dec->next_time += GST_BUFFER_DURATION (outbuf);

  GST_DEBUG_OBJECT (mpeg2dec,
      "picture: %s %s fields:%d off:%" G_GINT64_FORMAT " ts:%"
      GST_TIME_FORMAT,
      (picture->flags & PIC_FLAG_TOP_FIELD_FIRST ? "tff " : "    "),
      (picture->flags & PIC_FLAG_PROGRESSIVE_FRAME ? "prog" : "    "),
      picture->nb_fields, GST_BUFFER_OFFSET (outbuf),
      GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (outbuf)));

#ifndef GST_DISABLE_INDEX
  if (mpeg2dec->index) {
    gst_index_add_association (mpeg2dec->index, mpeg2dec->index_id,
        (key_frame ? GST_ASSOCIATION_FLAG_KEY_UNIT : 0),
        GST_FORMAT_BYTES, GST_BUFFER_OFFSET (outbuf),
        GST_FORMAT_TIME, GST_BUFFER_TIMESTAMP (outbuf), 0);
  }
#endif

  if (picture->flags & PIC_FLAG_SKIP)
    goto skip;

  if (mpeg2dec->discont_state != MPEG2DEC_DISC_NONE)
    goto drop;

  /* check for clipping */
  if (!clip_buffer (mpeg2dec, outbuf))
    goto clipped;

  if (GST_CLOCK_TIME_IS_VALID (time)) {
    gboolean need_skip;
    GstClockTime qostime;

    /* qos needs to be done on running time */
    qostime = gst_segment_to_running_time (&mpeg2dec->segment, GST_FORMAT_TIME,
        time);

    GST_OBJECT_LOCK (mpeg2dec);
    /* check for QoS, don't perform the last steps of getting and
     * pushing the buffers that are known to be late. */
    /* FIXME, we can also entirely skip decoding if the next valid buffer is
     * known to be after a keyframe (using the granule_shift) */
    need_skip = mpeg2dec->earliest_time != -1
        && qostime <= mpeg2dec->earliest_time;
    GST_OBJECT_UNLOCK (mpeg2dec);

    if (need_skip)
      goto dropping_qos;
  }

  /* ref before pushing it out, so we still have the ref in our
   * array of buffers */
  gst_buffer_ref (outbuf);

  /* do cropping if needed */
  crop_buffer (mpeg2dec, &outbuf);

  if (mpeg2dec->segment.rate >= 0.0) {
    /* forward: push right away */
    GST_LOG_OBJECT (mpeg2dec, "pushing buffer %p, timestamp %"
        GST_TIME_FORMAT ", duration %" GST_TIME_FORMAT,
        outbuf,
        GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (outbuf)),
        GST_TIME_ARGS (GST_BUFFER_DURATION (outbuf)));

    ret = gst_pad_push (mpeg2dec->srcpad, outbuf);
    GST_DEBUG_OBJECT (mpeg2dec, "pushed with result %s",
        gst_flow_get_name (ret));
  } else {
    /* reverse: queue, we'll push in reverse when we receive the next (previous)
     * keyframe. */
    GST_DEBUG_OBJECT (mpeg2dec, "queued frame");
    mpeg2dec->queued = g_list_prepend (mpeg2dec->queued, outbuf);
    ret = GST_FLOW_OK;
  }

  return ret;

  /* special cases */
no_display:
  {
    GST_DEBUG_OBJECT (mpeg2dec, "no picture to display");
    return GST_FLOW_OK;
  }
skip:
  {
    GST_DEBUG_OBJECT (mpeg2dec, "dropping buffer because of skip flag");
    return GST_FLOW_OK;
  }
drop:
  {
    GST_DEBUG_OBJECT (mpeg2dec, "dropping buffer, discont state %d",
        mpeg2dec->discont_state);
    return GST_FLOW_OK;
  }
clipped:
  {
    GST_DEBUG_OBJECT (mpeg2dec, "dropping buffer, clipped");
    return GST_FLOW_OK;
  }
dropping_qos:
  {
    GST_DEBUG_OBJECT (mpeg2dec, "dropping buffer because of QoS");
    return GST_FLOW_OK;
  }
}

#if 0
static void
update_streaminfo (GstMpeg2dec * mpeg2dec)
{
  GstCaps *caps;
  GstProps *props;
  GstPropsEntry *entry;
  const mpeg2_info_t *info;

  info = mpeg2_info (mpeg2dec->decoder);

  props = gst_props_empty_new ();

  entry =
      gst_props_entry_new ("framerate",
      G_TYPE_DOUBLE (GST_SECOND / (float) mpeg2dec->frame_period));
  gst_props_add_entry (props, entry);
  entry =
      gst_props_entry_new ("bitrate",
      G_TYPE_INT (info->sequence->byte_rate * 8));
  gst_props_add_entry (props, entry);

  caps = gst_caps_new ("mpeg2dec_streaminfo",
      "application/x-gst-streaminfo", props);

  gst_caps_replace_sink (&mpeg2dec->streaminfo, caps);
  g_object_notify (G_OBJECT (mpeg2dec), "streaminfo");
}
#endif

static GstFlowReturn
gst_mpeg2dec_chain (GstPad * pad, GstBuffer * buf)
{
  GstMpeg2dec *mpeg2dec;
  guint32 size;
  guint8 *data, *end;
  GstClockTime pts;
  const mpeg2_info_t *info;
  mpeg2_state_t state;
  gboolean done = FALSE;
  GstFlowReturn ret = GST_FLOW_OK;

  mpeg2dec = GST_MPEG2DEC (GST_PAD_PARENT (pad));

  size = GST_BUFFER_SIZE (buf);
  data = GST_BUFFER_DATA (buf);
  pts = GST_BUFFER_TIMESTAMP (buf);

  if (GST_BUFFER_IS_DISCONT (buf)) {
    GST_LOG_OBJECT (mpeg2dec, "DISCONT, reset decoder");
    /* when we receive a discont, reset our state as to not create too much
     * distortion in the picture due to missing packets */
    mpeg2_reset (mpeg2dec->decoder, 0);
    mpeg2_skip (mpeg2dec->decoder, 1);
    mpeg2dec->discont_state = MPEG2DEC_DISC_NEW_PICTURE;
  }

  GST_LOG_OBJECT (mpeg2dec, "received buffer, timestamp %"
      GST_TIME_FORMAT ", duration %" GST_TIME_FORMAT,
      GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (buf)),
      GST_TIME_ARGS (GST_BUFFER_DURATION (buf)));

  info = mpeg2dec->info;
  end = data + size;

  mpeg2dec->offset = GST_BUFFER_OFFSET (buf);

  if (pts != GST_CLOCK_TIME_NONE) {
    gint64 mpeg_pts = GST_TIME_TO_MPEG_TIME (pts);

    GST_DEBUG_OBJECT (mpeg2dec,
        "have pts: %" G_GINT64_FORMAT " (%" GST_TIME_FORMAT ")",
        mpeg_pts, GST_TIME_ARGS (MPEG_TIME_TO_GST_TIME (mpeg_pts)));

#if MPEG2_RELEASE >= MPEG2_VERSION(0,4,0)
    mpeg2_tag_picture (mpeg2dec->decoder, mpeg_pts & 0xffffffff,
        mpeg_pts >> 32);
#else
    mpeg2_pts (mpeg2dec->decoder, mpeg_pts);
#endif
  } else {
    GST_LOG ("no pts");
  }

  GST_LOG_OBJECT (mpeg2dec, "calling mpeg2_buffer");
  mpeg2_buffer (mpeg2dec->decoder, data, end);
  GST_LOG_OBJECT (mpeg2dec, "calling mpeg2_buffer done");

  while (!done) {
    GST_LOG_OBJECT (mpeg2dec, "calling parse");
    state = mpeg2_parse (mpeg2dec->decoder);
    GST_DEBUG_OBJECT (mpeg2dec, "parse state %d", state);

    switch (state) {
#if MPEG2_RELEASE >= MPEG2_VERSION (0, 5, 0)
      case STATE_SEQUENCE_MODIFIED:
        GST_DEBUG_OBJECT (mpeg2dec, "sequence modified");
        /* fall through */
#endif
      case STATE_SEQUENCE:
        ret = handle_sequence (mpeg2dec, info);
        /* if there is an error handling the sequence
         * reset the decoder, maybe something more elegant
         * could be done.
         */
        if (ret == GST_FLOW_ERROR) {
          mpeg2dec->error_count++;
          GST_WARNING_OBJECT (mpeg2dec, "Decoding error #%d",
              mpeg2dec->error_count);
          if (mpeg2dec->error_count >= WARN_THRESHOLD && WARN_THRESHOLD > 0) {
            GST_ELEMENT_WARNING (mpeg2dec, STREAM, DECODE,
                ("%d consecutive decoding errors", mpeg2dec->error_count),
                (NULL));
          }
          mpeg2_reset (mpeg2dec->decoder, 0);
          mpeg2_skip (mpeg2dec->decoder, 1);
          mpeg2dec->discont_state = MPEG2DEC_DISC_NEW_PICTURE;

          goto exit;
        }
        break;
      case STATE_SEQUENCE_REPEATED:
        GST_DEBUG_OBJECT (mpeg2dec, "sequence repeated");
        break;
      case STATE_GOP:
        GST_DEBUG_OBJECT (mpeg2dec, "gop");
        break;
      case STATE_PICTURE:
        ret = handle_picture (mpeg2dec, info);
        break;
      case STATE_SLICE_1ST:
        GST_LOG_OBJECT (mpeg2dec, "1st slice of frame encountered");
        break;
      case STATE_PICTURE_2ND:
        GST_LOG_OBJECT (mpeg2dec,
            "Second picture header encountered. Decoding 2nd field");
        break;
#if MPEG2_RELEASE >= MPEG2_VERSION (0, 4, 0)
      case STATE_INVALID_END:
        GST_DEBUG_OBJECT (mpeg2dec, "invalid end");
#endif
      case STATE_END:
        GST_DEBUG_OBJECT (mpeg2dec, "end");
        mpeg2dec->need_sequence = TRUE;
      case STATE_SLICE:
        ret = handle_slice (mpeg2dec, info);
        break;
      case STATE_BUFFER:
        done = TRUE;
        break;
        /* error */
      case STATE_INVALID:
        /* FIXME: at some point we should probably send newsegment events to
         * let downstream know that parts of the stream are missing */
        mpeg2dec->error_count++;
        GST_WARNING_OBJECT (mpeg2dec, "Decoding error #%d",
            mpeg2dec->error_count);
        if (mpeg2dec->error_count >= WARN_THRESHOLD && WARN_THRESHOLD > 0) {
          GST_ELEMENT_WARNING (mpeg2dec, STREAM, DECODE,
              ("%d consecutive decoding errors", mpeg2dec->error_count),
              (NULL));
        }
        goto exit;
      default:
        GST_ERROR_OBJECT (mpeg2dec, "Unknown libmpeg2 state %d, FIXME", state);
        goto exit;
    }

    mpeg2dec->error_count = 0;

    /*
     * FIXME: should pass more information such as state the user data is from
     */
#ifdef enable_user_data
    if (info->user_data_len > 0) {
      GstBuffer *udbuf = gst_buffer_new_and_alloc (info->user_data_len);

      memcpy (GST_BUFFER_DATA (udbuf), info->user_data, info->user_data_len);

      gst_pad_push (mpeg2dec->userdatapad, udbuf);
    }
#endif

    if (ret != GST_FLOW_OK) {
      GST_DEBUG_OBJECT (mpeg2dec, "exit loop, reason %s",
          gst_flow_get_name (ret));
      break;
    }
  }
done:
  gst_buffer_unref (buf);
  return ret;

  /* errors */
exit:
  {
    ret = GST_FLOW_OK;
    goto done;
  }
}

static gboolean
gst_mpeg2dec_sink_event (GstPad * pad, GstEvent * event)
{
  GstMpeg2dec *mpeg2dec;
  gboolean ret = TRUE;

  mpeg2dec = GST_MPEG2DEC (gst_pad_get_parent (pad));

  GST_DEBUG_OBJECT (mpeg2dec, "Got %s event on sink pad",
      GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_NEWSEGMENT:
    {
      gboolean update;
      GstFormat format;
      gdouble rate, arate;
      gint64 start, stop, time;

      gst_event_parse_new_segment_full (event, &update, &rate, &arate, &format,
          &start, &stop, &time);

      /* we need TIME and a positive rate */
      if (format != GST_FORMAT_TIME)
        goto newseg_wrong_format;

      /* now configure the values */
      gst_segment_set_newsegment_full (&mpeg2dec->segment, update,
          rate, arate, format, start, stop, time);

      GST_DEBUG_OBJECT (mpeg2dec,
          "Pushing newseg rate %g, applied rate %g, format %d, start %"
          G_GINT64_FORMAT ", stop %" G_GINT64_FORMAT ", pos %" G_GINT64_FORMAT,
          rate, arate, format, start, stop, time);

      ret = gst_pad_push_event (mpeg2dec->srcpad, event);
      break;
    }
    case GST_EVENT_FLUSH_START:
      ret = gst_pad_push_event (mpeg2dec->srcpad, event);
      break;
    case GST_EVENT_FLUSH_STOP:
    {
      mpeg2dec->discont_state = MPEG2DEC_DISC_NEW_PICTURE;
      mpeg2dec->next_time = -1;;
      gst_mpeg2dec_qos_reset (mpeg2dec);
      mpeg2_reset (mpeg2dec->decoder, 0);
      mpeg2_skip (mpeg2dec->decoder, 1);
      clear_queued (mpeg2dec);
      ret = gst_pad_push_event (mpeg2dec->srcpad, event);
      break;
    }
    case GST_EVENT_EOS:
#ifndef GST_DISABLE_INDEX
      if (mpeg2dec->index && mpeg2dec->closed) {
        gst_index_commit (mpeg2dec->index, mpeg2dec->index_id);
      }
#endif
      ret = gst_pad_push_event (mpeg2dec->srcpad, event);
      break;
    default:
      ret = gst_pad_push_event (mpeg2dec->srcpad, event);
      break;
  }

done:
  gst_object_unref (mpeg2dec);

  return ret;

  /* ERRORS */
newseg_wrong_format:
  {
    GST_DEBUG_OBJECT (mpeg2dec, "received non TIME newsegment");
    gst_event_unref (event);
    goto done;
  }
}

static gboolean
gst_mpeg2dec_setcaps (GstPad * pad, GstCaps * caps)
{
  GstMpeg2dec *mpeg2dec;
  GstStructure *s;

  mpeg2dec = GST_MPEG2DEC (gst_pad_get_parent (pad));

  s = gst_caps_get_structure (caps, 0);

  /* parse the par, this overrides the encoded par */
  mpeg2dec->have_par = gst_structure_get_fraction (s, "pixel-aspect-ratio",
      &mpeg2dec->pixel_width, &mpeg2dec->pixel_height);

  gst_object_unref (mpeg2dec);

  return TRUE;
}

static gboolean
gst_mpeg2dec_sink_convert (GstPad * pad, GstFormat src_format, gint64 src_value,
    GstFormat * dest_format, gint64 * dest_value)
{
  gboolean res = TRUE;
  GstMpeg2dec *mpeg2dec;
  const mpeg2_info_t *info;

  mpeg2dec = GST_MPEG2DEC (GST_PAD_PARENT (pad));

  if (mpeg2dec->decoder == NULL)
    return FALSE;

  if (src_format == *dest_format) {
    *dest_value = src_value;
    return TRUE;
  }

  info = mpeg2_info (mpeg2dec->decoder);

  switch (src_format) {
    case GST_FORMAT_BYTES:
      switch (*dest_format) {
        case GST_FORMAT_TIME:
          if (info->sequence && info->sequence->byte_rate) {
            *dest_value =
                gst_util_uint64_scale_int (GST_SECOND, src_value,
                info->sequence->byte_rate);
            GST_WARNING_OBJECT (mpeg2dec, "dest_value:%" GST_TIME_FORMAT,
                GST_TIME_ARGS (*dest_value));
            break;
          } else if (info->sequence)
            GST_WARNING_OBJECT (mpeg2dec,
                "Cannot convert from BYTES to TIME since we don't know the bitrate at this point.");
        default:
          res = FALSE;
      }
      break;
    case GST_FORMAT_TIME:
      switch (*dest_format) {
        case GST_FORMAT_BYTES:
          if (info->sequence && info->sequence->byte_rate) {
            *dest_value =
                gst_util_uint64_scale_int (src_value, info->sequence->byte_rate,
                GST_SECOND);
            break;
          } else if (info->sequence)
            GST_WARNING_OBJECT (mpeg2dec,
                "Cannot convert from TIME to BYTES since we don't know the bitrate at this point.");
        default:
          res = FALSE;
      }
      break;
    default:
      res = FALSE;
  }
  return res;
}


static gboolean
gst_mpeg2dec_src_convert (GstPad * pad, GstFormat src_format, gint64 src_value,
    GstFormat * dest_format, gint64 * dest_value)
{
  gboolean res = TRUE;
  GstMpeg2dec *mpeg2dec;
  const mpeg2_info_t *info;
  guint64 scale = 1;

  mpeg2dec = GST_MPEG2DEC (GST_PAD_PARENT (pad));

  if (mpeg2dec->decoder == NULL)
    return FALSE;

  if (src_format == *dest_format) {
    *dest_value = src_value;
    return TRUE;
  }

  info = mpeg2_info (mpeg2dec->decoder);

  switch (src_format) {
    case GST_FORMAT_BYTES:
      switch (*dest_format) {
        case GST_FORMAT_TIME:
        default:
          res = FALSE;
      }
      break;
    case GST_FORMAT_TIME:
      switch (*dest_format) {
        case GST_FORMAT_BYTES:
          scale = 6 * (mpeg2dec->width * mpeg2dec->height >> 2);
        case GST_FORMAT_DEFAULT:
          if (info->sequence && mpeg2dec->frame_period) {
            *dest_value =
                gst_util_uint64_scale_int (src_value, scale,
                mpeg2dec->frame_period);
            break;
          }
        default:
          res = FALSE;
      }
      break;
    case GST_FORMAT_DEFAULT:
      switch (*dest_format) {
        case GST_FORMAT_TIME:
          *dest_value = src_value * mpeg2dec->frame_period;
          break;
        case GST_FORMAT_BYTES:
          *dest_value =
              src_value * 6 * ((mpeg2dec->width * mpeg2dec->height) >> 2);
          break;
        default:
          res = FALSE;
      }
      break;
    default:
      res = FALSE;
  }
  return res;
}

static const GstQueryType *
gst_mpeg2dec_get_src_query_types (GstPad * pad)
{
  static const GstQueryType types[] = {
    GST_QUERY_POSITION,
    GST_QUERY_DURATION,
    0
  };

  return types;
}

static gboolean
gst_mpeg2dec_src_query (GstPad * pad, GstQuery * query)
{
  gboolean res = TRUE;
  GstMpeg2dec *mpeg2dec;

  mpeg2dec = GST_MPEG2DEC (GST_PAD_PARENT (pad));

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_POSITION:
    {
      GstFormat format;
      gint64 cur;

      /* save requested format */
      gst_query_parse_position (query, &format, NULL);

      /* and convert to the requested format */
      if (!gst_mpeg2dec_src_convert (pad, GST_FORMAT_TIME,
              mpeg2dec->next_time, &format, &cur))
        goto error;

      gst_query_set_position (query, format, cur);

      GST_LOG_OBJECT (mpeg2dec,
          "position query: we return %llu (format %u)", cur, format);
      break;
    }
    case GST_QUERY_DURATION:
    {
      GstFormat format;
      GstFormat rformat;
      gint64 total, total_bytes;
      GstPad *peer;

      if ((peer = gst_pad_get_peer (mpeg2dec->sinkpad)) == NULL)
        goto error;

      /* save requested format */
      gst_query_parse_duration (query, &format, NULL);

      /* send to peer */
      if ((res = gst_pad_query (peer, query))) {
        gst_object_unref (peer);
        goto done;
      } else {
        GST_LOG_OBJECT (mpeg2dec, "query on peer pad failed, trying bytes");
      }

      /* query peer for total length in bytes */
      gst_query_set_duration (query, GST_FORMAT_BYTES, -1);

      if (!(res = gst_pad_query (peer, query))) {
        GST_LOG_OBJECT (mpeg2dec, "query on peer pad failed");
        gst_object_unref (peer);
        goto error;
      }
      gst_object_unref (peer);

      /* get the returned format */
      gst_query_parse_duration (query, &rformat, &total_bytes);
      GST_LOG_OBJECT (mpeg2dec, "peer pad returned total=%lld bytes",
          total_bytes);

      if (total_bytes != -1) {
        if (!gst_mpeg2dec_sink_convert (pad, GST_FORMAT_BYTES, total_bytes,
                &format, &total))
          goto error;
      } else {
        total = -1;
      }

      gst_query_set_duration (query, format, total);

      GST_LOG_OBJECT (mpeg2dec,
          "position query: we return %llu (format %u)", total, format);
      break;
    }
    default:
      res = gst_pad_query_default (pad, query);
      break;
  }
done:
  return res;

error:

  GST_DEBUG ("error handling query");
  return FALSE;
}


#if 0
static const GstEventMask *
gst_mpeg2dec_get_event_masks (GstPad * pad)
{
  static const GstEventMask masks[] = {
    {GST_EVENT_SEEK, GST_SEEK_METHOD_SET | GST_SEEK_FLAG_FLUSH},
    {GST_EVENT_NAVIGATION, GST_EVENT_FLAG_NONE},
    {0,}
  };

  return masks;
}
#endif

#ifndef GST_DISABLE_INDEX
static gboolean
index_seek (GstPad * pad, GstEvent * event)
{
  GstIndexEntry *entry;
  GstMpeg2dec *mpeg2dec;
  gdouble rate;
  GstFormat format;
  GstSeekFlags flags;
  GstSeekType cur_type, stop_type;
  gint64 cur, stop;

  mpeg2dec = GST_MPEG2DEC (GST_PAD_PARENT (pad));

  gst_event_parse_seek (event, &rate, &format, &flags,
      &cur_type, &cur, &stop_type, &stop);

  entry = gst_index_get_assoc_entry (mpeg2dec->index, mpeg2dec->index_id,
      GST_INDEX_LOOKUP_BEFORE, GST_ASSOCIATION_FLAG_KEY_UNIT, format, cur);

  if ((entry) && gst_pad_is_linked (mpeg2dec->sinkpad)) {
    const GstFormat *peer_formats, *try_formats;

    /* since we know the exact byteoffset of the frame, make sure to seek on bytes first */
    const GstFormat try_all_formats[] = {
      GST_FORMAT_BYTES,
      GST_FORMAT_TIME,
      0
    };

    try_formats = try_all_formats;

#if 0
    peer_formats = gst_pad_get_formats (GST_PAD_PEER (mpeg2dec->sinkpad));
#else
    peer_formats = try_all_formats;     /* FIXE ME */
#endif

    while (gst_formats_contains (peer_formats, *try_formats)) {
      gint64 value;

      if (gst_index_entry_assoc_map (entry, *try_formats, &value)) {
        GstEvent *seek_event;

        GST_DEBUG_OBJECT (mpeg2dec, "index %s %" G_GINT64_FORMAT
            " -> %s %" G_GINT64_FORMAT,
            gst_format_get_details (format)->nick,
            cur, gst_format_get_details (*try_formats)->nick, value);

        /* lookup succeeded, create the seek */
        seek_event =
            gst_event_new_seek (rate, *try_formats, flags, cur_type, value,
            stop_type, stop);
        /* do the seek */
        if (gst_pad_push_event (mpeg2dec->sinkpad, seek_event)) {
          /* seek worked, we're done, loop will exit */
#if 0
          mpeg2dec->segment_start = GST_EVENT_SEEK_OFFSET (event);
#endif
          return TRUE;
        }
      }
      try_formats++;
    }
  }
  return FALSE;
}
#endif

static gboolean
normal_seek (GstPad * pad, GstEvent * event)
{
  gdouble rate;
  GstFormat format, conv;
  GstSeekFlags flags;
  GstSeekType cur_type, stop_type;
  gint64 cur, stop;
  gint64 time_cur, bytes_cur;
  gint64 time_stop, bytes_stop;
  gboolean res;
  GstMpeg2dec *mpeg2dec;
  GstEvent *peer_event;

  mpeg2dec = GST_MPEG2DEC (GST_PAD_PARENT (pad));

  GST_DEBUG ("normal seek");

  gst_event_parse_seek (event, &rate, &format, &flags,
      &cur_type, &cur, &stop_type, &stop);

  conv = GST_FORMAT_TIME;
  if (!gst_mpeg2dec_src_convert (pad, format, cur, &conv, &time_cur))
    goto convert_failed;
  if (!gst_mpeg2dec_src_convert (pad, format, stop, &conv, &time_stop))
    goto convert_failed;

  GST_DEBUG ("seek to time %" GST_TIME_FORMAT "-%" GST_TIME_FORMAT,
      GST_TIME_ARGS (time_cur), GST_TIME_ARGS (time_stop));

  peer_event = gst_event_new_seek (rate, GST_FORMAT_TIME, flags,
      cur_type, time_cur, stop_type, time_stop);

  /* try seek on time then */
  if ((res = gst_pad_push_event (mpeg2dec->sinkpad, peer_event)))
    goto done;

  /* else we try to seek on bytes */
  conv = GST_FORMAT_BYTES;
  if (!gst_mpeg2dec_sink_convert (pad, GST_FORMAT_TIME, time_cur,
          &conv, &bytes_cur))
    goto convert_failed;
  if (!gst_mpeg2dec_sink_convert (pad, GST_FORMAT_TIME, time_stop,
          &conv, &bytes_stop))
    goto convert_failed;

  /* conversion succeeded, create the seek */
  peer_event =
      gst_event_new_seek (rate, GST_FORMAT_BYTES, flags,
      cur_type, bytes_cur, stop_type, bytes_stop);

  /* do the seek */
  res = gst_pad_push_event (mpeg2dec->sinkpad, peer_event);

done:
  return res;

  /* ERRORS */
convert_failed:
  {
    /* probably unsupported seek format */
    GST_DEBUG_OBJECT (mpeg2dec,
        "failed to convert format %u into GST_FORMAT_TIME", format);
    return FALSE;
  }
}


static gboolean
gst_mpeg2dec_src_event (GstPad * pad, GstEvent * event)
{
  gboolean res;
  GstMpeg2dec *mpeg2dec;

  mpeg2dec = GST_MPEG2DEC (GST_PAD_PARENT (pad));

  if (mpeg2dec->decoder == NULL)
    goto no_decoder;

  switch (GST_EVENT_TYPE (event)) {
      /* the all-formats seek logic */
    case GST_EVENT_SEEK:{
      gst_event_ref (event);
      if (!(res = gst_pad_push_event (mpeg2dec->sinkpad, event))) {
#ifndef GST_DISABLE_INDEX
        if (mpeg2dec->index)
          res = index_seek (pad, event);
        else
#endif
          res = normal_seek (pad, event);
      }
      gst_event_unref (event);
      break;
    }
    case GST_EVENT_QOS:
    {
      gdouble proportion;
      GstClockTimeDiff diff;
      GstClockTime timestamp;

      gst_event_parse_qos (event, &proportion, &diff, &timestamp);

      GST_OBJECT_LOCK (mpeg2dec);
      mpeg2dec->proportion = proportion;
      mpeg2dec->earliest_time = timestamp + diff;
      GST_OBJECT_UNLOCK (mpeg2dec);

      GST_DEBUG_OBJECT (mpeg2dec,
          "got QoS %" GST_TIME_FORMAT ", %" G_GINT64_FORMAT,
          GST_TIME_ARGS (timestamp), diff);

      res = gst_pad_push_event (mpeg2dec->sinkpad, event);
      break;
    }
    case GST_EVENT_NAVIGATION:
      /* Forward a navigation event unchanged */
    default:
      res = gst_pad_push_event (mpeg2dec->sinkpad, event);
      break;
  }
  return res;

no_decoder:
  {
    GST_DEBUG_OBJECT (mpeg2dec, "no decoder, cannot handle event");
    gst_event_unref (event);
    return FALSE;
  }
}

static GstStateChangeReturn
gst_mpeg2dec_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret;
  GstMpeg2dec *mpeg2dec = GST_MPEG2DEC (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      mpeg2_accel (MPEG2_ACCEL_DETECT);
      if ((mpeg2dec->decoder = mpeg2_init ()) == NULL)
        goto init_failed;
      mpeg2dec->info = mpeg2_info (mpeg2dec->decoder);
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      gst_mpeg2dec_reset (mpeg2dec);
      gst_mpeg2dec_qos_reset (mpeg2dec);
      break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      gst_mpeg2dec_qos_reset (mpeg2dec);
      clear_queued (mpeg2dec);
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      if (mpeg2dec->decoder) {
        mpeg2_close (mpeg2dec->decoder);
        mpeg2dec->decoder = NULL;
        mpeg2dec->info = NULL;
      }
      clear_buffers (mpeg2dec);
      break;
    default:
      break;
  }
  return ret;

  /* ERRORS */
init_failed:
  {
    GST_ELEMENT_ERROR (mpeg2dec, LIBRARY, INIT,
        (NULL), ("Failed to initialize libmpeg2 library"));
    return GST_STATE_CHANGE_FAILURE;
  }
}

static void
gst_mpeg2dec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstMpeg2dec *src;

  g_return_if_fail (GST_IS_MPEG2DEC (object));
  src = GST_MPEG2DEC (object);

  switch (prop_id) {
    default:
      break;
  }
}

static void
gst_mpeg2dec_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstMpeg2dec *mpeg2dec;

  g_return_if_fail (GST_IS_MPEG2DEC (object));
  mpeg2dec = GST_MPEG2DEC (object);

  switch (prop_id) {
    default:
      break;
  }
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "mpeg2dec", GST_RANK_SECONDARY,
          GST_TYPE_MPEG2DEC))
    return FALSE;

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "mpeg2dec",
    "LibMpeg2 decoder", plugin_init, VERSION, "GPL", GST_PACKAGE_NAME,
    GST_PACKAGE_ORIGIN);
