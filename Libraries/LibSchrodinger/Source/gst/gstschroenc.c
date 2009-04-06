/* Schrodinger
 * Copyright (C) 2006 David Schleef <ds@schleef.org>
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

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstbasevideoencoder.h>
#include <string.h>

#include <schroedinger/schro.h>
#include <schroedinger/schrobitstream.h>
#include <schroedinger/schrovirtframe.h>
#include <math.h>
#include "gstschroutils.h"

GST_DEBUG_CATEGORY_EXTERN (schro_debug);
#define GST_CAT_DEFAULT schro_debug

#define GST_TYPE_SCHRO_ENC \
  (gst_schro_enc_get_type())
#define GST_SCHRO_ENC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_SCHRO_ENC,GstSchroEnc))
#define GST_SCHRO_ENC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_SCHRO_ENC,GstSchroEncClass))
#define GST_IS_SCHRO_ENC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_SCHRO_ENC))
#define GST_IS_SCHRO_ENC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_SCHRO_ENC))

typedef struct _GstSchroEnc GstSchroEnc;
typedef struct _GstSchroEncClass GstSchroEncClass;

typedef enum {
  GST_SCHRO_ENC_OUTPUT_OGG,
  GST_SCHRO_ENC_OUTPUT_QUICKTIME,
  GST_SCHRO_ENC_OUTPUT_AVI,
  GST_SCHRO_ENC_OUTPUT_MPEG_TS,
  GST_SCHRO_ENC_OUTPUT_MP4
} GstSchroEncOutputType;

struct _GstSchroEnc
{
  GstBaseVideoEncoder base_encoder;

  GstPad *sinkpad;
  GstPad *srcpad;

  /* video properties */
  GstSchroEncOutputType output_format;

  /* state */
  SchroEncoder *encoder;
  SchroVideoFormat *video_format;
  GstVideoFrame *eos_frame;
  GstBuffer *seq_header_buffer;

  guint64 last_granulepos;
};

struct _GstSchroEncClass
{
  GstBaseVideoEncoderClass parent_class;
};



enum
{
  LAST_SIGNAL
};

enum
{
  ARG_0
};

static void gst_schro_enc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_schro_enc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static GstFlowReturn gst_schro_enc_process (GstSchroEnc *schro_enc);

static gboolean gst_schro_enc_set_format (GstBaseVideoEncoder *base_video_encoder,
    GstVideoState *state);
static gboolean gst_schro_enc_start (GstBaseVideoEncoder *base_video_encoder);
static gboolean gst_schro_enc_stop (GstBaseVideoEncoder *base_video_encoder);
static gboolean gst_schro_enc_finish (GstBaseVideoEncoder *base_video_encoder,
    GstVideoFrame *frame);
static gboolean gst_schro_enc_handle_frame (GstBaseVideoEncoder *base_video_encoder,
    GstVideoFrame *frame);
static GstFlowReturn gst_schro_enc_shape_output (GstBaseVideoEncoder *base_video_encoder,
    GstVideoFrame *frame);
static GstCaps * gst_schro_enc_get_caps (GstBaseVideoEncoder *base_video_encoder);

static GstStaticPadTemplate gst_schro_enc_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (
      GST_VIDEO_CAPS_YUV ("{ I420, YV12, YUY2, UYVY, AYUV }"))
    );

static GstStaticPadTemplate gst_schro_enc_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-dirac;video/x-qt-part;video/x-mp4-part")
    );

GST_BOILERPLATE (GstSchroEnc, gst_schro_enc, GstBaseVideoEncoder,
    GST_TYPE_BASE_VIDEO_ENCODER);

static void
gst_schro_enc_base_init (gpointer g_class)
{
  static GstElementDetails schro_enc_details =
      GST_ELEMENT_DETAILS ("Dirac Encoder",
      "Codec/Encoder/Video",
      "Encode raw video into Dirac stream",
      "David Schleef <ds@schleef.org>");
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_schro_enc_src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_schro_enc_sink_template));

  gst_element_class_set_details (element_class, &schro_enc_details);
}

static void
gst_schro_enc_class_init (GstSchroEncClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseVideoEncoderClass *basevideocoder_class;
  int i;

  gobject_class = G_OBJECT_CLASS (klass);
  gstelement_class = GST_ELEMENT_CLASS (klass);
  basevideocoder_class = GST_BASE_VIDEO_ENCODER_CLASS (klass);

  gobject_class->set_property = gst_schro_enc_set_property;
  gobject_class->get_property = gst_schro_enc_get_property;

  for(i=0;i<schro_encoder_get_n_settings();i++){
    const SchroEncoderSetting *setting;

    setting = schro_encoder_get_setting_info (i);

    switch (setting->type) {
      case SCHRO_ENCODER_SETTING_TYPE_BOOLEAN:
        g_object_class_install_property (gobject_class, i + 1,
            g_param_spec_boolean (setting->name, setting->name, setting->name,
              setting->default_value, G_PARAM_READWRITE));
        break;
      case SCHRO_ENCODER_SETTING_TYPE_INT:
        g_object_class_install_property (gobject_class, i + 1,
            g_param_spec_int (setting->name, setting->name, setting->name,
              setting->min, setting->max, setting->default_value,
              G_PARAM_READWRITE));
        break;
      case SCHRO_ENCODER_SETTING_TYPE_ENUM:
        g_object_class_install_property (gobject_class, i + 1,
            g_param_spec_int (setting->name, setting->name, setting->name,
              setting->min, setting->max, setting->default_value,
              G_PARAM_READWRITE));
        break;
      case SCHRO_ENCODER_SETTING_TYPE_DOUBLE:
        g_object_class_install_property (gobject_class, i + 1,
            g_param_spec_double (setting->name, setting->name, setting->name,
              setting->min, setting->max, setting->default_value,
              G_PARAM_READWRITE));
        break;
      default:
        break;
    }
  }

  basevideocoder_class->set_format = GST_DEBUG_FUNCPTR(gst_schro_enc_set_format);
  basevideocoder_class->start = GST_DEBUG_FUNCPTR(gst_schro_enc_start);
  basevideocoder_class->stop = GST_DEBUG_FUNCPTR(gst_schro_enc_stop);
  basevideocoder_class->finish = GST_DEBUG_FUNCPTR(gst_schro_enc_finish);
  basevideocoder_class->handle_frame = GST_DEBUG_FUNCPTR(gst_schro_enc_handle_frame);
  basevideocoder_class->shape_output = GST_DEBUG_FUNCPTR(gst_schro_enc_shape_output);
  basevideocoder_class->get_caps = GST_DEBUG_FUNCPTR(gst_schro_enc_get_caps);
}

static void
gst_schro_enc_init (GstSchroEnc *schro_enc, GstSchroEncClass *klass)
{
  GST_DEBUG ("gst_schro_enc_init");

  /* Normally, we'd create the encoder in ->start(), but we use the
   * encoder to store object properties.  So it needs to be created
   * here. */
  schro_enc->encoder = schro_encoder_new ();
  schro_encoder_set_packet_assembly (schro_enc->encoder, TRUE);
  schro_enc->video_format =
    schro_encoder_get_video_format (schro_enc->encoder);
}



static gboolean
gst_schro_enc_set_format (GstBaseVideoEncoder *base_video_encoder,
    GstVideoState *state)
{
  GstSchroEnc *schro_enc = GST_SCHRO_ENC(base_video_encoder);

  schro_video_format_set_std_video_format (schro_enc->video_format,
      SCHRO_VIDEO_FORMAT_CUSTOM);

  switch (state->format) {
    case GST_VIDEO_FORMAT_I420:
    case GST_VIDEO_FORMAT_YV12:
      schro_enc->video_format->chroma_format = SCHRO_CHROMA_420;
      break;
    case GST_VIDEO_FORMAT_YUY2:
    case GST_VIDEO_FORMAT_UYVY:
      schro_enc->video_format->chroma_format = SCHRO_CHROMA_422;
      break;
    case GST_VIDEO_FORMAT_AYUV:
      schro_enc->video_format->chroma_format = SCHRO_CHROMA_444;
      break;
    case GST_VIDEO_FORMAT_ARGB:
      schro_enc->video_format->chroma_format = SCHRO_CHROMA_420;
      break;
    default:
      g_assert_not_reached();
  }

  schro_enc->video_format->frame_rate_numerator = state->fps_n;
  schro_enc->video_format->frame_rate_denominator = state->fps_d;

  schro_enc->video_format->width = state->width;
  schro_enc->video_format->height = state->height;
  schro_enc->video_format->clean_width = state->clean_width;
  schro_enc->video_format->clean_height = state->clean_height;
  schro_enc->video_format->left_offset = state->clean_offset_left;
  schro_enc->video_format->top_offset = state->clean_offset_top;

  schro_enc->video_format->aspect_ratio_numerator = state->par_n;
  schro_enc->video_format->aspect_ratio_denominator = state->par_d;

  schro_video_format_set_std_signal_range (schro_enc->video_format,
      SCHRO_SIGNAL_RANGE_8BIT_VIDEO);
  schro_video_format_set_std_colour_spec (schro_enc->video_format,
      SCHRO_COLOUR_SPEC_HDTV);

  schro_encoder_set_video_format (schro_enc->encoder, schro_enc->video_format);
  schro_encoder_start (schro_enc->encoder);

  schro_enc->seq_header_buffer = gst_schro_wrap_schro_buffer (
      schro_encoder_encode_sequence_header (schro_enc->encoder));

  return TRUE;
}

static void
gst_schro_enc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstSchroEnc *src;

  g_return_if_fail (GST_IS_SCHRO_ENC (object));
  src = GST_SCHRO_ENC (object);

  GST_DEBUG ("gst_schro_enc_set_property");

  if (prop_id >= 1) {
    const SchroEncoderSetting *setting;
    setting = schro_encoder_get_setting_info (prop_id - 1);
    switch(G_VALUE_TYPE(value)) {
      case G_TYPE_DOUBLE:
        schro_encoder_setting_set_double (src->encoder, setting->name,
            g_value_get_double(value));
        break;
      case G_TYPE_INT:
        schro_encoder_setting_set_double (src->encoder, setting->name,
            g_value_get_int(value));
        break;
      case G_TYPE_BOOLEAN:
        schro_encoder_setting_set_double (src->encoder, setting->name,
            g_value_get_boolean(value));
        break;
    }
  }
}

static void
gst_schro_enc_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstSchroEnc *src;

  g_return_if_fail (GST_IS_SCHRO_ENC (object));
  src = GST_SCHRO_ENC (object);

  if (prop_id >= 1) {
    const SchroEncoderSetting *setting;
    setting = schro_encoder_get_setting_info (prop_id - 1);
    switch(G_VALUE_TYPE(value)) {
      case G_TYPE_DOUBLE:
        g_value_set_double (value,
            schro_encoder_setting_get_double (src->encoder, setting->name));
        break;
      case G_TYPE_INT:
        g_value_set_int (value,
            schro_encoder_setting_get_double (src->encoder, setting->name));
        break;
      case G_TYPE_BOOLEAN:
        g_value_set_boolean (value,
            schro_encoder_setting_get_double (src->encoder, setting->name));
        break;
    }
  }
}

/*
 * start is called once the input format is known.  This function
 * must decide on an output format and negotiate it.
 */
static gboolean
gst_schro_enc_start (GstBaseVideoEncoder *base_video_encoder)
{
  GstSchroEnc *schro_enc = GST_SCHRO_ENC(base_video_encoder);
  GstCaps *caps;
  GstStructure *structure;

  GST_DEBUG("set_output_caps");
  caps = gst_pad_get_allowed_caps (GST_BASE_VIDEO_CODEC_SRC_PAD(base_video_encoder));

  if (gst_caps_is_empty (caps)) {
    gst_caps_unref (caps);
    return FALSE;
  }

  structure = gst_caps_get_structure (caps, 0);

  if (gst_structure_has_name (structure, "video/x-dirac")) {
    schro_enc->output_format = GST_SCHRO_ENC_OUTPUT_OGG;
  } else if (gst_structure_has_name (structure, "video/x-qt-part")) {
    schro_enc->output_format = GST_SCHRO_ENC_OUTPUT_QUICKTIME;
  } else if (gst_structure_has_name (structure, "video/x-avi-part")) {
    schro_enc->output_format = GST_SCHRO_ENC_OUTPUT_AVI;
  } else if (gst_structure_has_name (structure, "video/x-mp4-part")) {
    schro_enc->output_format = GST_SCHRO_ENC_OUTPUT_MP4;
  } else {
    return FALSE;
  }

  gst_caps_unref (caps);
  return TRUE;
}

static gboolean
gst_schro_enc_stop (GstBaseVideoEncoder *base_video_encoder)
{
  GstSchroEnc *schro_enc = GST_SCHRO_ENC(base_video_encoder);
  
  if (schro_enc->encoder) {
    schro_encoder_free (schro_enc->encoder);
    schro_enc->encoder = NULL;
  }

  return TRUE;
}

static gboolean
gst_schro_enc_finish (GstBaseVideoEncoder *base_video_encoder,
    GstVideoFrame *frame)
{
  GstSchroEnc *schro_enc = GST_SCHRO_ENC(base_video_encoder);

  GST_DEBUG("finish");

  schro_enc->eos_frame = frame;

  schro_encoder_end_of_stream (schro_enc->encoder);
  gst_schro_enc_process (schro_enc);

  return TRUE;
}

static gboolean
gst_schro_enc_handle_frame (GstBaseVideoEncoder *base_video_encoder,
    GstVideoFrame *frame)
{
  GstSchroEnc *schro_enc = GST_SCHRO_ENC(base_video_encoder);
  SchroFrame *schro_frame;
  GstFlowReturn ret;
  const GstVideoState *state;

  state = gst_base_video_encoder_get_state (base_video_encoder);

  schro_frame = gst_schro_buffer_wrap (frame->sink_buffer,
      state->format, state->width, state->height);

  GST_DEBUG ("pushing frame %p", frame);
  schro_encoder_push_frame_full (schro_enc->encoder, schro_frame, frame);

  ret = gst_schro_enc_process (schro_enc);

  return ret;
}

#if 0
static void
gst_caps_add_streamheader (GstCaps *caps, GList *list)
{
  GValue array = { 0 };
  GValue value = { 0 };
  GstBuffer *buf;
  GList *g;

  g_value_init (&array, GST_TYPE_ARRAY);

  for(g=g_list_first (list); g; g = g_list_next (list)) {
    g_value_init (&value, GST_TYPE_BUFFER);
    buf = gst_buffer_copy (GST_BUFFER(g->data));
    gst_value_set_buffer (&value, buf);
    gst_buffer_unref (buf);
    gst_value_array_append_value (&array, &value);
    g_value_unset (&value);
  }
  gst_structure_set_value (gst_caps_get_structure(caps,0),
      "streamheader", &array);
  g_value_unset (&array);
}
#endif

static GstCaps *
gst_schro_enc_get_caps (GstBaseVideoEncoder *base_video_encoder)
{
  GstCaps *caps;
  const GstVideoState *state;
  GstSchroEnc *schro_enc;
  
  schro_enc = GST_SCHRO_ENC(base_video_encoder);

  state = gst_base_video_encoder_get_state (base_video_encoder);

  if (schro_enc->output_format == GST_SCHRO_ENC_OUTPUT_OGG) {
    caps = gst_caps_new_simple ("video/x-dirac",
        "width", G_TYPE_INT, state->width,
        "height", G_TYPE_INT, state->height,
        "framerate", GST_TYPE_FRACTION, state->fps_n,
        state->fps_d,
        "pixel-aspect-ratio", GST_TYPE_FRACTION, state->par_n,
        state->par_d,
        NULL);

    GST_BUFFER_FLAG_SET (schro_enc->seq_header_buffer, GST_BUFFER_FLAG_IN_CAPS);

    {
      GValue array = { 0 };
      GValue value = { 0 };
      GstBuffer *buf;
      int size; 

      g_value_init (&array, GST_TYPE_ARRAY);
      g_value_init (&value, GST_TYPE_BUFFER);
      size = GST_BUFFER_SIZE(schro_enc->seq_header_buffer);
      buf = gst_buffer_new_and_alloc (size + SCHRO_PARSE_HEADER_SIZE);
      memcpy (GST_BUFFER_DATA(buf),
          GST_BUFFER_DATA(schro_enc->seq_header_buffer), size);
      GST_WRITE_UINT32_BE (GST_BUFFER_DATA(buf) + size + 0, 0x42424344);
      GST_WRITE_UINT8 (GST_BUFFER_DATA(buf) + size + 4, SCHRO_PARSE_CODE_END_OF_SEQUENCE);
      GST_WRITE_UINT32_BE (GST_BUFFER_DATA(buf) + size + 5, 0);
      GST_WRITE_UINT32_BE (GST_BUFFER_DATA(buf) + size + 9, size);
      gst_value_set_buffer (&value, buf);
      gst_buffer_unref (buf);
      gst_value_array_append_value (&array, &value);
      gst_structure_set_value (gst_caps_get_structure(caps,0),
          "streamheader", &array);
      g_value_unset (&value);
      g_value_unset (&array);
    }
  } else if (schro_enc->output_format == GST_SCHRO_ENC_OUTPUT_QUICKTIME) {
    caps = gst_caps_new_simple ("video/x-qt-part",
        "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC('d','r','a','c'),
        "width", G_TYPE_INT, state->width,
        "height", G_TYPE_INT, state->height,
        "framerate", GST_TYPE_FRACTION, state->fps_n,
        state->fps_d,
        "pixel-aspect-ratio", GST_TYPE_FRACTION, state->par_n,
        state->par_d, NULL);
  } else if (schro_enc->output_format == GST_SCHRO_ENC_OUTPUT_AVI) {
    caps = gst_caps_new_simple ("video/x-avi-part",
        "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC('d','r','a','c'),
        "width", G_TYPE_INT, state->width,
        "height", G_TYPE_INT, state->height,
        "framerate", GST_TYPE_FRACTION, state->fps_n,
        state->fps_d,
        "pixel-aspect-ratio", GST_TYPE_FRACTION, state->par_n,
        state->par_d, NULL);
  } else if (schro_enc->output_format == GST_SCHRO_ENC_OUTPUT_MPEG_TS) {
    caps = gst_caps_new_simple ("video/x-mpegts-part",
        "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC('d','r','a','c'),
        "width", G_TYPE_INT, state->width,
        "height", G_TYPE_INT, state->height,
        "framerate", GST_TYPE_FRACTION, state->fps_n,
        state->fps_d,
        "pixel-aspect-ratio", GST_TYPE_FRACTION, state->par_n,
        state->par_d, NULL);
  } else if (schro_enc->output_format == GST_SCHRO_ENC_OUTPUT_MP4) {
    caps = gst_caps_new_simple ("video/x-mp4-part",
        "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC('d','r','a','c'),
        "width", G_TYPE_INT, state->width,
        "height", G_TYPE_INT, state->height,
        "framerate", GST_TYPE_FRACTION, state->fps_n,
        state->fps_d,
        "pixel-aspect-ratio", GST_TYPE_FRACTION, state->par_n,
        state->par_d, NULL);
  } else {
    g_assert_not_reached ();
  }

  return caps;
}




static GstFlowReturn
gst_schro_enc_shape_output_ogg (GstBaseVideoEncoder *base_video_encoder,
    GstVideoFrame *frame)
{
  GstSchroEnc *schro_enc;
  int dpn;
  int delay;
  int dist;
  int pt;
  int dt;
  guint64 granulepos_hi;
  guint64 granulepos_low;
  GstBuffer *buf = frame->src_buffer;

  schro_enc = GST_SCHRO_ENC (base_video_encoder);

  dpn = frame->decode_frame_number;

  pt = frame->presentation_frame_number * 2;
  dt = frame->decode_frame_number * 2;
  delay = pt - dt;
  dist = frame->distance_from_sync;

  GST_DEBUG("sys %d dpn %d pt %d dt %d delay %d dist %d",
      (int)frame->system_frame_number,
      (int)frame->decode_frame_number,
      pt, dt, delay, dist);

  granulepos_hi = (((uint64_t)pt - delay)<<9) | ((dist>>8));
  granulepos_low = (delay << 9) | (dist & 0xff);
  GST_DEBUG("granulepos %lld:%lld", granulepos_hi, granulepos_low);

  if (frame->is_eos) {
    GST_BUFFER_OFFSET_END (buf) = schro_enc->last_granulepos;
  } else {
    schro_enc->last_granulepos = (granulepos_hi << 22) | (granulepos_low);
    GST_BUFFER_OFFSET_END (buf) = schro_enc->last_granulepos;
  }

  gst_buffer_set_caps (buf, base_video_encoder->caps);

  return gst_pad_push (GST_BASE_VIDEO_CODEC_SRC_PAD(base_video_encoder), buf);
}

static GstFlowReturn
gst_schro_enc_shape_output_quicktime (GstBaseVideoEncoder *base_video_encoder,
    GstVideoFrame *frame)
{
  GstBuffer *buf = frame->src_buffer;
  const GstVideoState *state;

  state = gst_base_video_encoder_get_state (base_video_encoder);

  GST_BUFFER_TIMESTAMP (buf) = gst_video_state_get_timestamp (state,
      frame->presentation_frame_number);
  GST_BUFFER_DURATION (buf) = gst_video_state_get_timestamp (state,
      frame->presentation_frame_number + 1) - GST_BUFFER_TIMESTAMP (buf);
  GST_BUFFER_OFFSET_END (buf) = gst_video_state_get_timestamp (state,
      frame->system_frame_number);
  GST_BUFFER_OFFSET (buf) = GST_CLOCK_TIME_NONE;

  if (frame->is_sync_point &&
      frame->presentation_frame_number == frame->system_frame_number) {
    GST_BUFFER_FLAG_UNSET (buf, GST_BUFFER_FLAG_DELTA_UNIT);
  } else {
    GST_BUFFER_FLAG_SET (buf, GST_BUFFER_FLAG_DELTA_UNIT);
  }

  gst_buffer_set_caps (buf, base_video_encoder->caps);

  return gst_pad_push (GST_BASE_VIDEO_CODEC_SRC_PAD(base_video_encoder), buf);
}

static GstFlowReturn
gst_schro_enc_shape_output_mp4 (GstBaseVideoEncoder *base_video_encoder,
    GstVideoFrame *frame)
{
  GstBuffer *buf = frame->src_buffer;
  const GstVideoState *state;

  state = gst_base_video_encoder_get_state (base_video_encoder);

  GST_BUFFER_TIMESTAMP (buf) = gst_video_state_get_timestamp (state,
      frame->presentation_frame_number);
  GST_BUFFER_DURATION (buf) = gst_video_state_get_timestamp (state,
      frame->presentation_frame_number + 1) - GST_BUFFER_TIMESTAMP (buf);
  GST_BUFFER_OFFSET_END (buf) = gst_video_state_get_timestamp (state,
      frame->decode_frame_number);
  GST_BUFFER_OFFSET (buf) = GST_CLOCK_TIME_NONE;

GST_BUFFER_OFFSET_END (buf) = gst_video_state_get_timestamp (state,
frame->system_frame_number);

  if (frame->is_sync_point &&
      frame->presentation_frame_number == frame->system_frame_number) {
    GST_BUFFER_FLAG_UNSET (buf, GST_BUFFER_FLAG_DELTA_UNIT);
  } else {
    GST_BUFFER_FLAG_SET (buf, GST_BUFFER_FLAG_DELTA_UNIT);
  }

  gst_buffer_set_caps (buf, base_video_encoder->caps);

  return gst_pad_push (GST_BASE_VIDEO_CODEC_SRC_PAD(base_video_encoder), buf);
}

static GstFlowReturn
gst_schro_enc_shape_output (GstBaseVideoEncoder *base_video_encoder,
    GstVideoFrame *frame)
{
  GstSchroEnc *schro_enc;

  schro_enc = GST_SCHRO_ENC (base_video_encoder);

  switch (schro_enc->output_format) {
    case GST_SCHRO_ENC_OUTPUT_OGG:
      return gst_schro_enc_shape_output_ogg (base_video_encoder, frame);
    case GST_SCHRO_ENC_OUTPUT_QUICKTIME:
      return gst_schro_enc_shape_output_quicktime (base_video_encoder, frame);
    case GST_SCHRO_ENC_OUTPUT_MP4:
      return gst_schro_enc_shape_output_mp4 (base_video_encoder, frame);
    default:
      g_assert_not_reached ();
      break;
  }

  return GST_FLOW_ERROR;
}

static GstFlowReturn
gst_schro_enc_process (GstSchroEnc *schro_enc)
{
  SchroBuffer *encoded_buffer;
  GstVideoFrame *frame;
  GstFlowReturn ret;
  int presentation_frame;
  void *voidptr;
  GstBaseVideoEncoder *base_video_encoder = GST_BASE_VIDEO_ENCODER(schro_enc);

  GST_DEBUG("process");

  while (1) {
    switch (schro_encoder_wait (schro_enc->encoder)) {
      case SCHRO_STATE_NEED_FRAME:
        return GST_FLOW_OK;
      case SCHRO_STATE_END_OF_STREAM:
        GST_DEBUG("EOS");
        return GST_FLOW_OK;
      case SCHRO_STATE_HAVE_BUFFER:
        voidptr = NULL;
        encoded_buffer = schro_encoder_pull_full (schro_enc->encoder,
            &presentation_frame, &voidptr);
        frame = voidptr;
        if (encoded_buffer == NULL) {
          GST_DEBUG("encoder_pull returned NULL");
          /* FIXME This shouldn't happen */
          return GST_FLOW_ERROR;
        }

        if (voidptr == NULL) {
          GST_DEBUG("got eos");
          frame = schro_enc->eos_frame;
        }

        if (SCHRO_PARSE_CODE_IS_SEQ_HEADER (encoded_buffer->data[4])) {
          frame->is_sync_point = TRUE;
        }
        
        frame->src_buffer = gst_schro_wrap_schro_buffer (encoded_buffer);

        ret = gst_base_video_encoder_finish_frame (base_video_encoder, frame);

        if (ret != GST_FLOW_OK) {
          GST_DEBUG("pad_push returned %d", ret);
          return ret;
        }
        break;
      case SCHRO_STATE_AGAIN:
        break;
    }
  }
  return GST_FLOW_OK;
}


