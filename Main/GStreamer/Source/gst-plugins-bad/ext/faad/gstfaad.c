/* GStreamer FAAD (Free AAC Decoder) plugin
 * Copyright (C) 2003 Ronald Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2006 Tim-Philipp Müller <tim centricular net>
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
#include <gst/audio/audio.h>
#include <gst/audio/multichannel.h>

/* These are the correct types for these functions, as defined in the source,
 * with types changed to match glib types, since those are defined for us.
 * However, upstream FAAD is distributed with a broken header file that defined
 * these wrongly (in a way which was broken on 64 bit systems).
 *
 * Upstream CVS still has the bug, but has also renamed all the public symbols
 * for Better Corporate Branding (or whatever), so we need to take that
 * (FAAD_IS_NEAAC) into account as well.
 *
 * We must call them using these definitions. Most distributions now have the
 * corrected header file (they distribute a patch along with the source), 
 * but not all, hence this Truly Evil Hack.
 *
 * Note: The prototypes don't need to be defined conditionaly, as the cpp will
 * do that for us.
 */
#if FAAD2_MINOR_VERSION < 7
#ifdef FAAD_IS_NEAAC
#define NeAACDecInit NeAACDecInit_no_definition
#define NeAACDecInit2 NeAACDecInit2_no_definition
#else
#define faacDecInit faacDecInit_no_definition
#define faacDecInit2 faacDecInit2_no_definition
#endif
#endif /* FAAD2_MINOR_VERSION < 7 */

#include "gstfaad.h"

#if FAAD2_MINOR_VERSION < 7
#ifdef FAAD_IS_NEAAC
#undef NeAACDecInit
#undef NeAACDecInit2
#else
#undef faacDecInit
#undef faacDecInit2
#endif

extern long faacDecInit (faacDecHandle, guint8 *, guint32, guint32 *, guint8 *);
extern gint8 faacDecInit2 (faacDecHandle, guint8 *, guint32,
    guint32 *, guint8 *);

#endif /* FAAD2_MINOR_VERSION < 7 */

GST_DEBUG_CATEGORY_STATIC (faad_debug);
#define GST_CAT_DEFAULT faad_debug

#define MAX_DECODE_ERRORS 5

static const GstElementDetails faad_details =
GST_ELEMENT_DETAILS ("AAC audio decoder",
    "Codec/Decoder/Audio",
    "Free MPEG-2/4 AAC decoder",
    "Ronald Bultje <rbultje@ronald.bitfreak.net>");

static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/mpeg, " "mpegversion = (int) { 2, 4 }")
    );

#define STATIC_INT_CAPS(bpp) \
  "audio/x-raw-int, " \
    "endianness = (int) BYTE_ORDER, " \
    "signed = (bool) TRUE, " \
    "width = (int) " G_STRINGIFY (bpp) ", " \
    "depth = (int) " G_STRINGIFY (bpp) ", " \
    "rate = (int) [ 8000, 96000 ], " \
    "channels = (int) [ 1, 8 ]"

#if 0
#define STATIC_FLOAT_CAPS(bpp) \
  "audio/x-raw-float, " \
    "endianness = (int) BYTE_ORDER, " \
    "depth = (int) " G_STRINGIFY (bpp) ", " \
    "rate = (int) [ 8000, 96000 ], " \
    "channels = (int) [ 1, 8 ]"
#endif

/*
 * All except 16-bit integer are disabled until someone fixes FAAD.
 * FAAD allocates approximately 8*1024*2 bytes bytes, which is enough
 * for 1 frame (1024 samples) of 6 channel (5.1) 16-bit integer 16bpp
 * audio, but not for any other. You'll get random segfaults, crashes
 * and even valgrind goes crazy.
 */

#define STATIC_CAPS \
  STATIC_INT_CAPS (16)
#if 0
#define NOTUSED "; " \
STATIC_INT_CAPS (24) \
    "; " \
STATIC_INT_CAPS (32) \
    "; " \
STATIC_FLOAT_CAPS (32) \
    "; " \
STATIC_FLOAT_CAPS (64)
#endif

static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (STATIC_CAPS)
    );

static void gst_faad_base_init (GstFaadClass * klass);
static void gst_faad_class_init (GstFaadClass * klass);
static void gst_faad_init (GstFaad * faad);
static void gst_faad_dispose (GObject * object);

static gboolean gst_faad_setcaps (GstPad * pad, GstCaps * caps);
static GstCaps *gst_faad_srcgetcaps (GstPad * pad);
static gboolean gst_faad_src_event (GstPad * pad, GstEvent * event);
static gboolean gst_faad_sink_event (GstPad * pad, GstEvent * event);
static gboolean gst_faad_src_query (GstPad * pad, GstQuery * query);
static GstFlowReturn gst_faad_chain (GstPad * pad, GstBuffer * buffer);
static GstStateChangeReturn gst_faad_change_state (GstElement * element,
    GstStateChange transition);
static gboolean gst_faad_src_convert (GstFaad * faad, GstFormat src_format,
    gint64 src_val, GstFormat dest_format, gint64 * dest_val);
static gboolean gst_faad_open_decoder (GstFaad * faad);
static void gst_faad_close_decoder (GstFaad * faad);

static GstElementClass *parent_class;   /* NULL */

GType
gst_faad_get_type (void)
{
  static GType gst_faad_type = 0;

  if (!gst_faad_type) {
    static const GTypeInfo gst_faad_info = {
      sizeof (GstFaadClass),
      (GBaseInitFunc) gst_faad_base_init,
      NULL,
      (GClassInitFunc) gst_faad_class_init,
      NULL,
      NULL,
      sizeof (GstFaad),
      0,
      (GInstanceInitFunc) gst_faad_init,
    };

    gst_faad_type = g_type_register_static (GST_TYPE_ELEMENT,
        "GstFaad", &gst_faad_info, 0);
  }

  return gst_faad_type;
}

static void
gst_faad_base_init (GstFaadClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_template));

  gst_element_class_set_details (element_class, &faad_details);

  GST_DEBUG_CATEGORY_INIT (faad_debug, "faad", 0, "AAC decoding");
}

static void
gst_faad_class_init (GstFaadClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->dispose = GST_DEBUG_FUNCPTR (gst_faad_dispose);

  gstelement_class->change_state = GST_DEBUG_FUNCPTR (gst_faad_change_state);
}

static void
gst_faad_init (GstFaad * faad)
{
  faad->handle = NULL;
  faad->samplerate = -1;
  faad->channels = -1;
  faad->tempbuf = NULL;
  faad->need_channel_setup = TRUE;
  faad->channel_positions = NULL;
  faad->init = FALSE;
  faad->next_ts = 0;
  faad->prev_ts = GST_CLOCK_TIME_NONE;
  faad->bytes_in = 0;
  faad->sum_dur_out = 0;
  faad->packetised = FALSE;
  faad->error_count = 0;
  faad->segment = gst_segment_new ();

  faad->sinkpad = gst_pad_new_from_static_template (&sink_template, "sink");
  gst_element_add_pad (GST_ELEMENT (faad), faad->sinkpad);
  gst_pad_set_event_function (faad->sinkpad,
      GST_DEBUG_FUNCPTR (gst_faad_sink_event));
  gst_pad_set_setcaps_function (faad->sinkpad,
      GST_DEBUG_FUNCPTR (gst_faad_setcaps));
  gst_pad_set_chain_function (faad->sinkpad,
      GST_DEBUG_FUNCPTR (gst_faad_chain));

  faad->srcpad = gst_pad_new_from_static_template (&src_template, "src");
  gst_pad_use_fixed_caps (faad->srcpad);
  gst_pad_set_getcaps_function (faad->srcpad,
      GST_DEBUG_FUNCPTR (gst_faad_srcgetcaps));
  gst_pad_set_query_function (faad->srcpad,
      GST_DEBUG_FUNCPTR (gst_faad_src_query));
  gst_pad_set_event_function (faad->srcpad,
      GST_DEBUG_FUNCPTR (gst_faad_src_event));
  gst_element_add_pad (GST_ELEMENT (faad), faad->srcpad);
}

static void
gst_faad_dispose (GObject * object)
{
  GstFaad *faad = GST_FAAD (object);

  if (faad->segment) {
    gst_segment_free (faad->segment);
    faad->segment = NULL;
  }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gst_faad_send_tags (GstFaad * faad)
{
  GstTagList *tags;

  tags = gst_tag_list_new ();

  gst_tag_list_add (tags, GST_TAG_MERGE_REPLACE,
      GST_TAG_AUDIO_CODEC, "MPEG-4 AAC audio", NULL);

  gst_element_found_tags (GST_ELEMENT (faad), tags);
}

static gint
aac_rate_idx (gint rate)
{
  if (92017 <= rate)
    return 0;
  else if (75132 <= rate)
    return 1;
  else if (55426 <= rate)
    return 2;
  else if (46009 <= rate)
    return 3;
  else if (37566 <= rate)
    return 4;
  else if (27713 <= rate)
    return 5;
  else if (23004 <= rate)
    return 6;
  else if (18783 <= rate)
    return 7;
  else if (13856 <= rate)
    return 8;
  else if (11502 <= rate)
    return 9;
  else if (9391 <= rate)
    return 10;
  else
    return 11;
}

static gboolean
gst_faad_setcaps (GstPad * pad, GstCaps * caps)
{
  GstFaad *faad = GST_FAAD (gst_pad_get_parent (pad));
  GstStructure *str = gst_caps_get_structure (caps, 0);
  GstBuffer *buf;
  const GValue *value;

  /* Assume raw stream */
  faad->packetised = FALSE;

  if ((value = gst_structure_get_value (str, "codec_data"))) {
#if FAAD2_MINOR_VERSION >= 7
    unsigned long samplerate;
#else
    guint32 samplerate;
#endif
    guint8 channels;
    guint8 *cdata;
    guint csize;

    /* We have codec data, means packetised stream */
    faad->packetised = TRUE;
    buf = gst_value_get_buffer (value);

    cdata = GST_BUFFER_DATA (buf);
    csize = GST_BUFFER_SIZE (buf);

    if (csize < 2)
      goto wrong_length;

    GST_DEBUG ("codec_data: object_type=%d, sample_rate=%d, channels=%d",
        ((cdata[0] & 0xf8) >> 3),
        (((cdata[0] & 0x07) << 1) | ((cdata[1] & 0x80) >> 7)),
        ((cdata[1] & 0x78) >> 3));

    /* someone forgot that char can be unsigned when writing the API */
    if ((gint8) faacDecInit2 (faad->handle, cdata, csize, &samplerate,
            &channels) < 0)
      goto init_failed;

    if (channels != ((cdata[1] & 0x78) >> 3)) {
      /* https://bugs.launchpad.net/ubuntu/+source/faad2/+bug/290259 */
      GST_WARNING_OBJECT (faad,
          "buggy faad version, wrong nr of channels %d instead of %d", channels,
          ((cdata[1] & 0x78) >> 3));
    }

    GST_DEBUG_OBJECT (faad, "codec_data init: channels=%u, rate=%u", channels,
        (guint32) samplerate);

    /* not updating these here, so they are updated in the
     * chain function, and new caps are created etc. */
    faad->samplerate = 0;
    faad->channels = 0;

    faad->init = TRUE;

    if (faad->tempbuf) {
      gst_buffer_unref (faad->tempbuf);
      faad->tempbuf = NULL;
    }
  } else if ((value = gst_structure_get_value (str, "framed")) &&
      g_value_get_boolean (value) == TRUE) {
    faad->packetised = TRUE;
    GST_DEBUG_OBJECT (faad, "we have packetized audio");
  } else {
    faad->init = FALSE;
  }

  faad->fake_codec_data[0] = 0;
  faad->fake_codec_data[1] = 0;

  if (faad->packetised) {
    gint rate, channels;

    if (gst_structure_get_int (str, "rate", &rate) &&
        gst_structure_get_int (str, "channels", &channels)) {
      gint rate_idx, profile;

      profile = 3;              /* 0=MAIN, 1=LC, 2=SSR, 3=LTP */
      rate_idx = aac_rate_idx (rate);

      faad->fake_codec_data[0] = ((profile + 1) << 3) | ((rate_idx & 0xE) >> 1);
      faad->fake_codec_data[1] = ((rate_idx & 0x1) << 7) | (channels << 3);
      GST_LOG_OBJECT (faad, "created fake codec data (%u,%u): 0x%x 0x%x", rate,
          channels, (int) faad->fake_codec_data[0],
          (int) faad->fake_codec_data[1]);
    }
  }

  faad->need_channel_setup = TRUE;

  if (!faad->packetised)
    gst_faad_send_tags (faad);

  gst_object_unref (faad);
  return TRUE;

  /* ERRORS */
wrong_length:
  {
    GST_DEBUG_OBJECT (faad, "codec_data less than 2 bytes long");
    gst_object_unref (faad);
    return FALSE;
  }
init_failed:
  {
    GST_DEBUG_OBJECT (faad, "faacDecInit2() failed");
    gst_object_unref (faad);
    return FALSE;
  }
}


/*
 * Channel identifier conversion - caller g_free()s result!
 */
/*
static guchar *
gst_faad_chanpos_from_gst (GstAudioChannelPosition * pos, guint num)
{
  guchar *fpos = g_new (guchar, num);
  guint n;

  for (n = 0; n < num; n++) {
    switch (pos[n]) {
      case GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT:
        fpos[n] = FRONT_CHANNEL_LEFT;
        break;
      case GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT:
        fpos[n] = FRONT_CHANNEL_RIGHT;
        break;
      case GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER:
      case GST_AUDIO_CHANNEL_POSITION_FRONT_MONO:
        fpos[n] = FRONT_CHANNEL_CENTER;
        break;
      case GST_AUDIO_CHANNEL_POSITION_SIDE_LEFT:
        fpos[n] = SIDE_CHANNEL_LEFT;
        break;
      case GST_AUDIO_CHANNEL_POSITION_SIDE_RIGHT:
        fpos[n] = SIDE_CHANNEL_RIGHT;
        break;
      case GST_AUDIO_CHANNEL_POSITION_REAR_LEFT:
        fpos[n] = BACK_CHANNEL_LEFT;
        break;
      case GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT:
        fpos[n] = BACK_CHANNEL_RIGHT;
        break;
      case GST_AUDIO_CHANNEL_POSITION_REAR_CENTER:
        fpos[n] = BACK_CHANNEL_CENTER;
        break;
      case GST_AUDIO_CHANNEL_POSITION_LFE:
        fpos[n] = LFE_CHANNEL;
        break;
      default:
        GST_WARNING ("Unsupported GST channel position 0x%x encountered",
            pos[n]);
        g_free (fpos);
        return NULL;
    }
  }

  return fpos;
}
*/

static GstAudioChannelPosition *
gst_faad_chanpos_to_gst (GstFaad * faad, guchar * fpos, guint num,
    gboolean * channel_map_failed)
{
  GstAudioChannelPosition *pos;
  guint n;
  gboolean unknown_channel = FALSE;

  *channel_map_failed = FALSE;

  /* special handling for the common cases for mono and stereo */
  if (num == 1 && fpos[0] == FRONT_CHANNEL_CENTER) {
    GST_DEBUG_OBJECT (faad, "mono common case; won't set channel positions");
    return NULL;
  } else if (num == 2 && fpos[0] == FRONT_CHANNEL_LEFT
      && fpos[1] == FRONT_CHANNEL_RIGHT) {
    GST_DEBUG_OBJECT (faad, "stereo common case; won't set channel positions");
    return NULL;
  }

  pos = g_new (GstAudioChannelPosition, num);
  for (n = 0; n < num; n++) {
    GST_DEBUG_OBJECT (faad, "faad channel %d as %d", n, fpos[n]);
    switch (fpos[n]) {
      case FRONT_CHANNEL_LEFT:
        pos[n] = GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT;
        break;
      case FRONT_CHANNEL_RIGHT:
        pos[n] = GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT;
        break;
      case FRONT_CHANNEL_CENTER:
        /* argh, mono = center */
        if (num == 1)
          pos[n] = GST_AUDIO_CHANNEL_POSITION_FRONT_MONO;
        else
          pos[n] = GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER;
        break;
      case SIDE_CHANNEL_LEFT:
        pos[n] = GST_AUDIO_CHANNEL_POSITION_SIDE_LEFT;
        break;
      case SIDE_CHANNEL_RIGHT:
        pos[n] = GST_AUDIO_CHANNEL_POSITION_SIDE_RIGHT;
        break;
      case BACK_CHANNEL_LEFT:
        pos[n] = GST_AUDIO_CHANNEL_POSITION_REAR_LEFT;
        break;
      case BACK_CHANNEL_RIGHT:
        pos[n] = GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT;
        break;
      case BACK_CHANNEL_CENTER:
        pos[n] = GST_AUDIO_CHANNEL_POSITION_REAR_CENTER;
        break;
      case LFE_CHANNEL:
        pos[n] = GST_AUDIO_CHANNEL_POSITION_LFE;
        break;
      default:
        GST_DEBUG_OBJECT (faad, "unknown channel %d at %d", fpos[n], n);
        unknown_channel = TRUE;
        break;
    }
  }
  if (unknown_channel) {
    g_free (pos);
    pos = NULL;
    switch (num) {
      case 1:{
        GST_DEBUG_OBJECT (faad,
            "FAAD reports unknown 1 channel mapping. Forcing to mono");
        break;
      }
      case 2:{
        GST_DEBUG_OBJECT (faad,
            "FAAD reports unknown 2 channel mapping. Forcing to stereo");
        break;
      }
      default:{
        GST_WARNING ("Unsupported FAAD channel position 0x%x encountered",
            fpos[n]);
        *channel_map_failed = TRUE;
        break;
      }
    }
  }

  return pos;
}

static GstCaps *
gst_faad_srcgetcaps (GstPad * pad)
{
  GstFaad *faad = GST_FAAD (gst_pad_get_parent (pad));
  static GstAudioChannelPosition *supported_positions = NULL;
  static gint num_supported_positions = LFE_CHANNEL - FRONT_CHANNEL_CENTER + 1;
  GstCaps *templ;

  if (!supported_positions) {
    guchar *supported_fpos = g_new0 (guchar, num_supported_positions);
    gint n;
    gboolean map_failed;

    for (n = 0; n < num_supported_positions; n++) {
      supported_fpos[n] = n + FRONT_CHANNEL_CENTER;
    }
    supported_positions = gst_faad_chanpos_to_gst (faad, supported_fpos,
        num_supported_positions, &map_failed);
    g_free (supported_fpos);
  }

  if (faad->handle != NULL && faad->channels != -1 && faad->samplerate != -1) {
    GstCaps *caps = gst_caps_new_empty ();
    GstStructure *str;
    gint fmt[] = {
      FAAD_FMT_16BIT,
#if 0
      FAAD_FMT_24BIT,
      FAAD_FMT_32BIT,
      FAAD_FMT_FLOAT,
      FAAD_FMT_DOUBLE,
#endif
      -1
    }
    , n;

    for (n = 0; fmt[n] != -1; n++) {
      switch (fmt[n]) {
        case FAAD_FMT_16BIT:
          str = gst_structure_new ("audio/x-raw-int",
              "signed", G_TYPE_BOOLEAN, TRUE,
              "width", G_TYPE_INT, 16, "depth", G_TYPE_INT, 16, NULL);
          break;
#if 0
        case FAAD_FMT_24BIT:
          str = gst_structure_new ("audio/x-raw-int",
              "signed", G_TYPE_BOOLEAN, TRUE,
              "width", G_TYPE_INT, 24, "depth", G_TYPE_INT, 24, NULL);
          break;
        case FAAD_FMT_32BIT:
          str = gst_structure_new ("audio/x-raw-int",
              "signed", G_TYPE_BOOLEAN, TRUE,
              "width", G_TYPE_INT, 32, "depth", G_TYPE_INT, 32, NULL);
          break;
        case FAAD_FMT_FLOAT:
          str = gst_structure_new ("audio/x-raw-float",
              "depth", G_TYPE_INT, 32, NULL);
          break;
        case FAAD_FMT_DOUBLE:
          str = gst_structure_new ("audio/x-raw-float",
              "depth", G_TYPE_INT, 64, NULL);
          break;
#endif
        default:
          str = NULL;
          break;
      }
      if (!str)
        continue;

      if (faad->samplerate > 0) {
        gst_structure_set (str, "rate", G_TYPE_INT, faad->samplerate, NULL);
      } else {
        gst_structure_set (str, "rate", GST_TYPE_INT_RANGE, 8000, 96000, NULL);
      }

      if (faad->channels > 0) {
        gst_structure_set (str, "channels", G_TYPE_INT, faad->channels, NULL);

        /* put channel information here */
        if (faad->channel_positions) {
          GstAudioChannelPosition *pos;
          gboolean map_failed;

          pos = gst_faad_chanpos_to_gst (faad, faad->channel_positions,
              faad->channels, &map_failed);
          if (map_failed) {
            gst_structure_free (str);
            continue;
          }
          if (pos) {
            gst_audio_set_channel_positions (str, pos);
            g_free (pos);
          }
        } else {
          gst_audio_set_structure_channel_positions_list (str,
              supported_positions, num_supported_positions);
        }
      } else {
        gst_structure_set (str, "channels", GST_TYPE_INT_RANGE, 1, 8, NULL);
        /* we set channel positions later */
      }

      gst_structure_set (str, "endianness", G_TYPE_INT, G_BYTE_ORDER, NULL);

      gst_caps_append_structure (caps, str);
    }

    if (faad->channels == -1) {
      gst_audio_set_caps_channel_positions_list (caps,
          supported_positions, num_supported_positions);
    }
    gst_object_unref (faad);
    return caps;
  }

  /* template with channel positions */
  templ = gst_caps_copy (GST_PAD_TEMPLATE_CAPS (GST_PAD_PAD_TEMPLATE (pad)));
  gst_audio_set_caps_channel_positions_list (templ,
      supported_positions, num_supported_positions);

  gst_object_unref (faad);
  return templ;
}

/*
static GstPadLinkReturn
gst_faad_srcconnect (GstPad * pad, const GstCaps * caps)
{
  GstStructure *structure;
  const gchar *mimetype;
  gint fmt = -1;
  gint depth, rate, channels;
  GstFaad *faad = GST_FAAD (gst_pad_get_parent (pad));

  structure = gst_caps_get_structure (caps, 0);

  if (!faad->handle || (faad->samplerate == -1 || faad->channels == -1) ||
      !faad->channel_positions) {
    return GST_PAD_LINK_DELAYED;
  }

  mimetype = gst_structure_get_name (structure);

  // Samplerate and channels are normally provided through
  // * the getcaps function 
  if (!gst_structure_get_int (structure, "channels", &channels) ||
      !gst_structure_get_int (structure, "rate", &rate) ||
      rate != faad->samplerate || channels != faad->channels) {
    return GST_PAD_LINK_REFUSED;
  }

  // Another internal checkup. 
  if (faad->need_channel_setup) {
    GstAudioChannelPosition *pos;
    guchar *fpos;
    guint i;

    pos = gst_audio_get_channel_positions (structure);
    if (!pos) {
      return GST_PAD_LINK_DELAYED;
    }
    fpos = gst_faad_chanpos_from_gst (pos, faad->channels);
    g_free (pos);
    if (!fpos)
      return GST_PAD_LINK_REFUSED;

    for (i = 0; i < faad->channels; i++) {
      if (fpos[i] != faad->channel_positions[i]) {
        g_free (fpos);
        return GST_PAD_LINK_REFUSED;
      }
    }
    g_free (fpos);
  }

  if (!strcmp (mimetype, "audio/x-raw-int")) {
    gint width;

    if (!gst_structure_get_int (structure, "depth", &depth) ||
        !gst_structure_get_int (structure, "width", &width))
      return GST_PAD_LINK_REFUSED;
    if (depth != width)
      return GST_PAD_LINK_REFUSED;

    switch (depth) {
      case 16:
        fmt = FAAD_FMT_16BIT;
        break;
#if 0
      case 24:
        fmt = FAAD_FMT_24BIT;
        break;
      case 32:
        fmt = FAAD_FMT_32BIT;
        break;
#endif
    }
  } else {
    if (!gst_structure_get_int (structure, "depth", &depth))
      return GST_PAD_LINK_REFUSED;

    switch (depth) {
#if 0
      case 32:
        fmt = FAAD_FMT_FLOAT;
        break;
      case 64:
        fmt = FAAD_FMT_DOUBLE;
        break;
#endif
    }
  }

  if (fmt != -1) {
    faacDecConfiguration *conf;

    conf = faacDecGetCurrentConfiguration (faad->handle);
    conf->outputFormat = fmt;
    if (faacDecSetConfiguration (faad->handle, conf) == 0)
      return GST_PAD_LINK_REFUSED;

    // FIXME: handle return value, how? 
    faad->bps = depth / 8;

    return GST_PAD_LINK_OK;
  }

  return GST_PAD_LINK_REFUSED;
}*/

static void
clear_queued (GstFaad * faad)
{
  g_list_foreach (faad->queued, (GFunc) gst_mini_object_unref, NULL);
  g_list_free (faad->queued);
  faad->queued = NULL;
}

static GstFlowReturn
flush_queued (GstFaad * faad)
{
  GstFlowReturn ret = GST_FLOW_OK;

  while (faad->queued) {
    GstBuffer *buf = GST_BUFFER_CAST (faad->queued->data);

    GST_LOG_OBJECT (faad, "pushing buffer %p, timestamp %"
        GST_TIME_FORMAT ", duration %" GST_TIME_FORMAT, buf,
        GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (buf)),
        GST_TIME_ARGS (GST_BUFFER_DURATION (buf)));

    /* iterate ouput queue an push downstream */
    ret = gst_pad_push (faad->srcpad, buf);

    faad->queued = g_list_delete_link (faad->queued, faad->queued);
  }
  return ret;
}

static GstFlowReturn
gst_faad_drain (GstFaad * faad)
{
  GstFlowReturn ret = GST_FLOW_OK;

  if (faad->segment->rate < 0.0) {
    /* if we have some queued frames for reverse playback, flush
     * them now */
    ret = flush_queued (faad);
  }
  return ret;
}

static gboolean
gst_faad_do_raw_seek (GstFaad * faad, GstEvent * event)
{
  GstSeekFlags flags;
  GstSeekType start_type, end_type;
  GstFormat format;
  gdouble rate;
  gint64 start, start_time;

  gst_event_parse_seek (event, &rate, &format, &flags, &start_type,
      &start_time, &end_type, NULL);

  if (rate != 1.0 ||
      format != GST_FORMAT_TIME ||
      start_type != GST_SEEK_TYPE_SET || end_type != GST_SEEK_TYPE_NONE) {
    return FALSE;
  }

  if (!gst_faad_src_convert (faad, GST_FORMAT_TIME, start_time,
          GST_FORMAT_BYTES, &start)) {
    return FALSE;
  }

  event = gst_event_new_seek (1.0, GST_FORMAT_BYTES, flags,
      GST_SEEK_TYPE_SET, start, GST_SEEK_TYPE_NONE, -1);

  GST_DEBUG_OBJECT (faad, "seeking to %" GST_TIME_FORMAT " at byte offset %"
      G_GINT64_FORMAT, GST_TIME_ARGS (start_time), start);

  return gst_pad_push_event (faad->sinkpad, event);
}

static gboolean
gst_faad_src_event (GstPad * pad, GstEvent * event)
{
  GstFaad *faad;
  gboolean res;

  faad = GST_FAAD (gst_pad_get_parent (pad));

  GST_LOG_OBJECT (faad, "Handling %s event", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_SEEK:{
      /* try upstream first, there might be a demuxer */
      gst_event_ref (event);
      if (!(res = gst_pad_push_event (faad->sinkpad, event))) {
        res = gst_faad_do_raw_seek (faad, event);
      }
      gst_event_unref (event);
      break;
    }
    default:
      res = gst_pad_push_event (faad->sinkpad, event);
      break;
  }

  gst_object_unref (faad);
  return res;
}

static gboolean
gst_faad_sink_event (GstPad * pad, GstEvent * event)
{
  GstFaad *faad;
  gboolean res = TRUE;

  faad = GST_FAAD (gst_pad_get_parent (pad));

  GST_LOG_OBJECT (faad, "Handling %s event", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_FLUSH_STOP:
      if (faad->tempbuf != NULL) {
        gst_buffer_unref (faad->tempbuf);
        faad->tempbuf = NULL;
      }
      clear_queued (faad);
      res = gst_pad_push_event (faad->srcpad, event);
      break;
    case GST_EVENT_EOS:
      gst_faad_drain (faad);
      if (faad->tempbuf != NULL) {
        gst_buffer_unref (faad->tempbuf);
        faad->tempbuf = NULL;
      }
      res = gst_pad_push_event (faad->srcpad, event);
      break;
    case GST_EVENT_NEWSEGMENT:
    {
      GstFormat fmt;
      gboolean is_update;
      gint64 start, end, base;
      gdouble rate;

      gst_event_parse_new_segment (event, &is_update, &rate, &fmt, &start,
          &end, &base);

      /* drain queued buffers before we activate the new segment */
      gst_faad_drain (faad);

      if (fmt == GST_FORMAT_TIME) {
        GST_DEBUG ("Got NEWSEGMENT event in GST_FORMAT_TIME, passing on (%"
            GST_TIME_FORMAT " - %" GST_TIME_FORMAT ")", GST_TIME_ARGS (start),
            GST_TIME_ARGS (end));
        gst_segment_set_newsegment (faad->segment, is_update, rate, fmt, start,
            end, base);
      } else if (fmt == GST_FORMAT_BYTES) {
        gint64 new_start = 0;
        gint64 new_end = -1;

        GST_DEBUG ("Got NEWSEGMENT event in GST_FORMAT_BYTES (%"
            G_GUINT64_FORMAT " - %" G_GUINT64_FORMAT ")", start, end);

        if (gst_faad_src_convert (faad, GST_FORMAT_BYTES, start,
                GST_FORMAT_TIME, &new_start)) {
          if (end != -1) {
            gst_faad_src_convert (faad, GST_FORMAT_BYTES, end,
                GST_FORMAT_TIME, &new_end);
          }
        } else {
          GST_DEBUG
              ("no average bitrate yet, sending newsegment with start at 0");
        }
        gst_event_unref (event);

        event = gst_event_new_new_segment (is_update, rate,
            GST_FORMAT_TIME, new_start, new_end, new_start);

        gst_segment_set_newsegment (faad->segment, is_update, rate,
            GST_FORMAT_TIME, new_start, new_end, new_start);

        GST_DEBUG ("Sending new NEWSEGMENT event, time %" GST_TIME_FORMAT
            " - %" GST_TIME_FORMAT, GST_TIME_ARGS (new_start),
            GST_TIME_ARGS (new_end));

        faad->next_ts = new_start;
        faad->prev_ts = GST_CLOCK_TIME_NONE;
      }

      res = gst_pad_push_event (faad->srcpad, event);
      break;
    }
    default:
      res = gst_pad_event_default (pad, event);
      break;
  }

  gst_object_unref (faad);
  return res;
}

static gboolean
gst_faad_src_convert (GstFaad * faad, GstFormat src_format, gint64 src_val,
    GstFormat dest_format, gint64 * dest_val)
{
  guint64 bytes_in, time_out, val;

  if (src_format == dest_format) {
    if (dest_val)
      *dest_val = src_val;
    return TRUE;
  }

  GST_OBJECT_LOCK (faad);
  bytes_in = faad->bytes_in;
  time_out = faad->sum_dur_out;
  GST_OBJECT_UNLOCK (faad);

  if (bytes_in == 0 || time_out == 0)
    return FALSE;

  /* convert based on the average bitrate so far */
  if (src_format == GST_FORMAT_BYTES && dest_format == GST_FORMAT_TIME) {
    val = gst_util_uint64_scale (src_val, time_out, bytes_in);
  } else if (src_format == GST_FORMAT_TIME && dest_format == GST_FORMAT_BYTES) {
    val = gst_util_uint64_scale (src_val, bytes_in, time_out);
  } else {
    return FALSE;
  }

  if (dest_val)
    *dest_val = (gint64) val;

  return TRUE;
}

static gboolean
gst_faad_src_query (GstPad * pad, GstQuery * query)
{
  gboolean res = FALSE;
  GstFaad *faad;
  GstPad *peer = NULL;

  faad = GST_FAAD (gst_pad_get_parent (pad));

  GST_LOG_OBJECT (faad, "processing %s query", GST_QUERY_TYPE_NAME (query));

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_DURATION:{
      GstFormat format;
      gint64 len_bytes, duration;

      /* try upstream first, in case there's a demuxer */
      if ((res = gst_pad_query_default (pad, query)))
        break;

      gst_query_parse_duration (query, &format, NULL);
      if (format != GST_FORMAT_TIME) {
        GST_DEBUG_OBJECT (faad, "query failed: can't handle format %s",
            gst_format_get_name (format));
        break;
      }

      peer = gst_pad_get_peer (faad->sinkpad);
      if (peer == NULL)
        break;

      format = GST_FORMAT_BYTES;
      if (!gst_pad_query_duration (peer, &format, &len_bytes)) {
        GST_DEBUG_OBJECT (faad, "query failed: failed to get upstream length");
        break;
      }

      res = gst_faad_src_convert (faad, GST_FORMAT_BYTES, len_bytes,
          GST_FORMAT_TIME, &duration);

      if (res) {
        gst_query_set_duration (query, GST_FORMAT_TIME, duration);

        GST_LOG_OBJECT (faad, "duration estimate: %" GST_TIME_FORMAT,
            GST_TIME_ARGS (duration));
      }
      break;
    }
    case GST_QUERY_POSITION:{
      GstFormat format;
      gint64 pos_bytes, pos;

      /* try upstream first, in case there's a demuxer */
      if ((res = gst_pad_query_default (pad, query)))
        break;

      gst_query_parse_position (query, &format, NULL);
      if (format != GST_FORMAT_TIME) {
        GST_DEBUG_OBJECT (faad, "query failed: can't handle format %s",
            gst_format_get_name (format));
        break;
      }

      peer = gst_pad_get_peer (faad->sinkpad);
      if (peer == NULL)
        break;

      format = GST_FORMAT_BYTES;
      if (!gst_pad_query_position (peer, &format, &pos_bytes)) {
        GST_OBJECT_LOCK (faad);
        pos = faad->next_ts;
        GST_OBJECT_UNLOCK (faad);
        res = TRUE;
      } else {
        res = gst_faad_src_convert (faad, GST_FORMAT_BYTES, pos_bytes,
            GST_FORMAT_TIME, &pos);
      }

      if (res) {
        gst_query_set_position (query, GST_FORMAT_TIME, pos);
      }
      break;
    }
    default:
      res = gst_pad_query_default (pad, query);
      break;
  }

  if (peer)
    gst_object_unref (peer);

  gst_object_unref (faad);
  return res;
}


static gboolean
gst_faad_update_caps (GstFaad * faad, faacDecFrameInfo * info)
{
  GstAudioChannelPosition *pos;
  gboolean ret;
  gboolean channel_map_failed;
  GstCaps *caps;

  /* store new negotiation information */
  faad->samplerate = info->samplerate;
  faad->channels = info->channels;
  g_free (faad->channel_positions);
  faad->channel_positions = g_memdup (info->channel_position, faad->channels);

  caps = gst_caps_new_simple ("audio/x-raw-int",
      "endianness", G_TYPE_INT, G_BYTE_ORDER,
      "signed", G_TYPE_BOOLEAN, TRUE,
      "width", G_TYPE_INT, 16,
      "depth", G_TYPE_INT, 16,
      "rate", G_TYPE_INT, faad->samplerate,
      "channels", G_TYPE_INT, faad->channels, NULL);

  faad->bps = 16 / 8;

  channel_map_failed = FALSE;
  pos =
      gst_faad_chanpos_to_gst (faad, faad->channel_positions, faad->channels,
      &channel_map_failed);
  if (channel_map_failed) {
    GST_DEBUG_OBJECT (faad, "Could not map channel positions");
    gst_caps_unref (caps);
    return FALSE;
  }
  if (pos) {
    gst_audio_set_channel_positions (gst_caps_get_structure (caps, 0), pos);
    g_free (pos);
  }

  GST_DEBUG_OBJECT (faad, "New output caps: %" GST_PTR_FORMAT, caps);

  ret = gst_pad_set_caps (faad->srcpad, caps);
  gst_caps_unref (caps);

  return ret;
}

/*
 * Find syncpoint in ADTS/ADIF stream. Doesn't work for raw,
 * packetized streams. Be careful when calling.
 * Returns FALSE on no-sync, fills offset/length if one/two
 * syncpoints are found, only returns TRUE when it finds two
 * subsequent syncpoints (similar to mp3 typefinding in
 * gst/typefind/) for ADTS because 12 bits isn't very reliable.
 */
static gboolean
gst_faad_sync (GstBuffer * buf, guint * off)
{
  guint8 *data = GST_BUFFER_DATA (buf);
  guint size = GST_BUFFER_SIZE (buf), n;
  gint snc;

  GST_DEBUG ("Finding syncpoint");

  /* check for too small a buffer */
  if (size < 3)
    return FALSE;

  /* FIXME: for no-sync, we go over the same data for every new buffer.
   * We should save the information somewhere. */
  for (n = 0; n < size - 3; n++) {
    snc = GST_READ_UINT16_BE (&data[n]);
    if ((snc & 0xfff6) == 0xfff0) {
      /* we have an ADTS syncpoint. Parse length and find
       * next syncpoint. */
      guint len;

      GST_DEBUG ("Found one ADTS syncpoint at offset 0x%x, tracing next...", n);

      if (size - n < 5) {
        GST_DEBUG ("Not enough data to parse ADTS header");
        return FALSE;
      }

      *off = n;
      len = ((data[n + 3] & 0x03) << 11) |
          (data[n + 4] << 3) | ((data[n + 5] & 0xe0) >> 5);
      if (n + len + 2 >= size) {
        GST_DEBUG ("Next frame is not within reach");
        return FALSE;
      }

      snc = GST_READ_UINT16_BE (&data[n + len]);
      if ((snc & 0xfff6) == 0xfff0) {
        GST_DEBUG ("Found ADTS syncpoint at offset 0x%x (framelen %u)", n, len);
        return TRUE;
      }

      GST_DEBUG ("No next frame found... (should be at 0x%x)", n + len);
    } else if (!memcmp (&data[n], "ADIF", 4)) {
      /* we have an ADIF syncpoint. 4 bytes is enough. */
      *off = n;
      GST_DEBUG ("Found ADIF syncpoint at offset 0x%x", n);
      return TRUE;
    }
  }

  GST_DEBUG ("Found no syncpoint");

  return FALSE;
}

static gboolean
looks_like_valid_header (guint8 * input_data, guint input_size)
{
  if (input_size < 4)
    return FALSE;

  if (input_data[0] == 'A'
      && input_data[1] == 'D' && input_data[2] == 'I' && input_data[3] == 'F')
    /* ADIF type header */
    return TRUE;

  if (input_data[0] == 0xff && (input_data[1] >> 4) == 0xf)
    /* ADTS type header */
    return TRUE;

  return FALSE;
}

static GstFlowReturn
gst_faad_chain (GstPad * pad, GstBuffer * buffer)
{
  GstFlowReturn ret = GST_FLOW_OK;
  guint input_size;
  guint skip_bytes = 0;
  guchar *input_data;
  GstFaad *faad;
  GstBuffer *outbuf;
  faacDecFrameInfo info;
  void *out;
  gboolean run_loop = TRUE;
  guint sync_off;

  faad = GST_FAAD (gst_pad_get_parent (pad));

  if (GST_BUFFER_IS_DISCONT (buffer)) {
    gst_faad_drain (faad);
    faacDecPostSeekReset (faad->handle, 0);
    if (faad->tempbuf != NULL) {
      gst_buffer_unref (faad->tempbuf);
      faad->tempbuf = NULL;
    }
    faad->discont = TRUE;
  }

  GST_OBJECT_LOCK (faad);
  faad->bytes_in += GST_BUFFER_SIZE (buffer);
  GST_OBJECT_UNLOCK (faad);

  if (GST_BUFFER_TIMESTAMP (buffer) != GST_CLOCK_TIME_NONE) {
    /* some demuxers send multiple buffers in a row
     *  with the same timestamp (e.g. matroskademux) */
    if (GST_BUFFER_TIMESTAMP (buffer) != faad->prev_ts) {
      faad->next_ts = GST_BUFFER_TIMESTAMP (buffer);
      faad->prev_ts = GST_BUFFER_TIMESTAMP (buffer);
    }
    GST_LOG_OBJECT (faad, "Timestamp on incoming buffer: %" GST_TIME_FORMAT
        ", next_ts: %" GST_TIME_FORMAT,
        GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (buffer)),
        GST_TIME_ARGS (faad->next_ts));
  }
  /* buffer + remaining data */
  if (faad->tempbuf) {
    buffer = gst_buffer_join (faad->tempbuf, buffer);
    faad->tempbuf = NULL;
  }

  input_data = GST_BUFFER_DATA (buffer);
  input_size = GST_BUFFER_SIZE (buffer);

  if (!faad->packetised) {
    if (!gst_faad_sync (buffer, &sync_off)) {
      goto next;
    } else {
      input_data += sync_off;
      input_size -= sync_off;
    }
  }

  /* init if not already done during capsnego */
  if (!faad->init) {
#if FAAD2_MINOR_VERSION >= 7
    unsigned long rate;
#else
    guint32 rate;
#endif
    guint8 ch;

    GST_DEBUG_OBJECT (faad, "initialising ...");
    /* We check if the first data looks like it might plausibly contain
     * appropriate initialisation info... if not, we use our fake_codec_data
     */
    if (looks_like_valid_header (input_data, input_size) || !faad->packetised) {
      if (faacDecInit (faad->handle, input_data, input_size, &rate, &ch) < 0)
        goto init_failed;

      GST_DEBUG_OBJECT (faad, "faacDecInit() ok: rate=%u,channels=%u",
          (guint32) rate, ch);
    } else {
      if ((gint8) faacDecInit2 (faad->handle, faad->fake_codec_data, 2,
              &rate, &ch) < 0) {
        goto init2_failed;
      }
      GST_DEBUG_OBJECT (faad, "faacDecInit2() ok: rate=%u,channels=%u",
          (guint32) rate, ch);
    }

    skip_bytes = 0;
    faad->init = TRUE;

    /* make sure we create new caps below */
    faad->samplerate = 0;
    faad->channels = 0;
    gst_faad_send_tags (faad);
  }

  /* decode cycle */
  info.bytesconsumed = input_size - skip_bytes;
  info.error = 0;

  if (!faad->packetised) {
    /* We must check that ourselves for raw stream */
    run_loop = (input_size >= FAAD_MIN_STREAMSIZE);
  }

  while ((input_size > 0) && run_loop) {

    if (faad->packetised) {
      /* Only one packet per buffer, no matter how much is really consumed */
      run_loop = FALSE;
    } else {
      if (input_size < FAAD_MIN_STREAMSIZE || info.bytesconsumed <= 0) {
        break;
      }
    }

    out = faacDecDecode (faad->handle, &info, input_data + skip_bytes,
        input_size - skip_bytes);

    if (info.error > 0) {
      GST_WARNING_OBJECT (faad, "decoding error: %s",
          faacDecGetErrorMessage (info.error));
      /* mark discont for the next buffer */
      faad->discont = TRUE;
      goto out;
    }

    if (info.bytesconsumed > input_size)
      info.bytesconsumed = input_size;

    input_size -= info.bytesconsumed;
    input_data += info.bytesconsumed;

    if (out && info.samples > 0) {
      gboolean fmt_change = FALSE;

      /* see if we need to renegotiate */
      if (info.samplerate != faad->samplerate ||
          info.channels != faad->channels || !faad->channel_positions) {
        fmt_change = TRUE;
      } else {
        gint i;

        for (i = 0; i < info.channels; i++) {
          if (info.channel_position[i] != faad->channel_positions[i])
            fmt_change = TRUE;
        }
      }

      if (fmt_change) {
        if (!gst_faad_update_caps (faad, &info)) {
          GST_ELEMENT_ERROR (faad, CORE, NEGOTIATION, (NULL),
              ("Setting caps on source pad failed"));
          ret = GST_FLOW_ERROR;
          goto out;
        }
      }

      if (info.samples > G_MAXUINT / faad->bps) {
        /* C's lovely propensity for int overflow.. */
        GST_ELEMENT_ERROR (faad, STREAM, DECODE, (NULL),
            ("Output buffer too large"));
        ret = GST_FLOW_ERROR;
        goto out;
      }

      /* play decoded data */
      if (info.samples > 0) {
        guint bufsize = info.samples * faad->bps;
        guint num_samples = info.samples / faad->channels;

        GST_DEBUG_OBJECT (faad, "decoded %d samples", (guint) info.samples);

        /* note: info.samples is total samples, not per channel */
        ret =
            gst_pad_alloc_buffer_and_set_caps (faad->srcpad, 0, bufsize,
            GST_PAD_CAPS (faad->srcpad), &outbuf);
        if (ret != GST_FLOW_OK)
          goto out;

        memcpy (GST_BUFFER_DATA (outbuf), out, GST_BUFFER_SIZE (outbuf));
        GST_BUFFER_OFFSET (outbuf) =
            GST_CLOCK_TIME_TO_FRAMES (faad->next_ts, faad->samplerate);
        GST_BUFFER_TIMESTAMP (outbuf) = faad->next_ts;
        GST_BUFFER_DURATION (outbuf) =
            GST_FRAMES_TO_CLOCK_TIME (num_samples, faad->samplerate);

        GST_OBJECT_LOCK (faad);
        faad->next_ts += GST_BUFFER_DURATION (outbuf);
        faad->sum_dur_out += GST_BUFFER_DURATION (outbuf);
        GST_OBJECT_UNLOCK (faad);

        if ((outbuf = gst_audio_buffer_clip (outbuf, faad->segment,
                    faad->samplerate, faad->bps * faad->channels))) {
          GST_LOG_OBJECT (faad,
              "pushing buffer, off=%" G_GUINT64_FORMAT ", ts=%" GST_TIME_FORMAT,
              GST_BUFFER_OFFSET (outbuf),
              GST_TIME_ARGS (GST_BUFFER_TIMESTAMP (outbuf)));

          if (faad->discont) {
            GST_BUFFER_FLAG_SET (outbuf, GST_BUFFER_FLAG_DISCONT);
            faad->discont = FALSE;
          }

          if (faad->segment->rate > 0.0) {
            ret = gst_pad_push (faad->srcpad, outbuf);
          } else {
            /* reverse playback, queue frame till later when we get a discont. */
            GST_DEBUG_OBJECT (faad, "queued frame");
            faad->queued = g_list_prepend (faad->queued, outbuf);
            ret = GST_FLOW_OK;
          }
          if (ret != GST_FLOW_OK)
            goto out;
        }
      }
    }
  }

next:

  /* Keep the leftovers in raw stream */
  if (input_size > 0 && !faad->packetised) {
    if (input_size < GST_BUFFER_SIZE (buffer)) {
      faad->tempbuf = gst_buffer_create_sub (buffer,
          GST_BUFFER_SIZE (buffer) - input_size, input_size);
    } else {
      faad->tempbuf = buffer;
      gst_buffer_ref (buffer);
    }
  }

out:

  gst_buffer_unref (buffer);
  gst_object_unref (faad);

  return ret;

/* ERRORS */
init_failed:
  {
    GST_ELEMENT_ERROR (faad, STREAM, DECODE, (NULL),
        ("Failed to init decoder from stream"));
    ret = GST_FLOW_ERROR;
    goto out;
  }
init2_failed:
  {
    GST_ELEMENT_ERROR (faad, STREAM, DECODE, (NULL),
        ("%s() failed", (faad->handle) ? "faacDecInit2" : "faacDecOpen"));
    ret = GST_FLOW_ERROR;
    goto out;
  }
}

static gboolean
gst_faad_open_decoder (GstFaad * faad)
{
  faacDecConfiguration *conf;

  faad->handle = faacDecOpen ();

  if (faad->handle == NULL) {
    GST_WARNING_OBJECT (faad, "faacDecOpen() failed");
    return FALSE;
  }

  conf = faacDecGetCurrentConfiguration (faad->handle);
  conf->defObjectType = LC;
  conf->dontUpSampleImplicitSBR = 1;
  conf->outputFormat = FAAD_FMT_16BIT;

  if (faacDecSetConfiguration (faad->handle, conf) == 0) {
    GST_WARNING_OBJECT (faad, "faacDecSetConfiguration() failed");
    return FALSE;
  }

  return TRUE;
}

static void
gst_faad_close_decoder (GstFaad * faad)
{
  if (faad->handle) {
    faacDecClose (faad->handle);
    faad->handle = NULL;
  }
}

static GstStateChangeReturn
gst_faad_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstFaad *faad = GST_FAAD (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      if (!gst_faad_open_decoder (faad))
        return GST_STATE_CHANGE_FAILURE;
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      gst_segment_init (faad->segment, GST_FORMAT_TIME);
      break;
    default:
      break;
  }

  if (GST_ELEMENT_CLASS (parent_class)->change_state)
    ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      faad->samplerate = -1;
      faad->channels = -1;
      faad->need_channel_setup = TRUE;
      faad->init = FALSE;
      g_free (faad->channel_positions);
      faad->channel_positions = NULL;
      faad->next_ts = 0;
      faad->prev_ts = GST_CLOCK_TIME_NONE;
      faad->bytes_in = 0;
      faad->sum_dur_out = 0;
      faad->error_count = 0;
      if (faad->tempbuf) {
        gst_buffer_unref (faad->tempbuf);
        faad->tempbuf = NULL;
      }
      clear_queued (faad);
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      gst_faad_close_decoder (faad);
      break;
    default:
      break;
  }

  return ret;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "faad", GST_RANK_PRIMARY, GST_TYPE_FAAD);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "faad",
    "Free AAC Decoder (FAAD)",
    plugin_init, VERSION, "GPL", GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
