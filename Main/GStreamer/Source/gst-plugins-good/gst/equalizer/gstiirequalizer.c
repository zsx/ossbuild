/* GStreamer
 * Copyright (C) <2004> Benjamin Otte <otte@gnome.org>
 *               <2007> Stefan Kost <ensonic@users.sf.net>
 *               <2007> Sebastian Dröge <slomo@circular-chaos.org>
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

#include <math.h>
#include <string.h>

#include "gstiirequalizer.h"
#include "gstiirequalizernbands.h"
#include "gstiirequalizer3bands.h"
#include "gstiirequalizer10bands.h"

GST_DEBUG_CATEGORY (equalizer_debug);
#define GST_CAT_DEFAULT equalizer_debug

static void gst_iir_equalizer_child_proxy_interface_init (gpointer g_iface,
    gpointer iface_data);

static void gst_iir_equalizer_finalize (GObject * object);

static gboolean gst_iir_equalizer_setup (GstAudioFilter * filter,
    GstRingBufferSpec * fmt);
static GstFlowReturn gst_iir_equalizer_transform_ip (GstBaseTransform * btrans,
    GstBuffer * buf);

GST_DEBUG_CATEGORY_EXTERN (equalizer_debug);
#define GST_CAT_DEFAULT equalizer_debug

#define ALLOWED_CAPS \
    "audio/x-raw-int,"                                                \
    " depth=(int)16,"                                                 \
    " width=(int)16,"                                                 \
    " endianness=(int)BYTE_ORDER,"                                    \
    " signed=(bool)TRUE,"                                             \
    " rate=(int)[1000,MAX],"                                          \
    " channels=(int)[1,MAX]; "                                        \
    "audio/x-raw-float,"                                              \
    " width=(int) { 32, 64 } ,"                                       \
    " endianness=(int)BYTE_ORDER,"                                    \
    " rate=(int)[1000,MAX],"                                          \
    " channels=(int)[1,MAX]"

static void
_do_init (GType object_type)
{
  const GInterfaceInfo child_proxy_interface_info = {
    (GInterfaceInitFunc) gst_iir_equalizer_child_proxy_interface_init,
    NULL,                       /* interface_finalize */
    NULL                        /* interface_data */
  };

  g_type_add_interface_static (object_type, GST_TYPE_CHILD_PROXY,
      &child_proxy_interface_info);
}

GST_BOILERPLATE_FULL (GstIirEqualizer, gst_iir_equalizer,
    GstAudioFilter, GST_TYPE_AUDIO_FILTER, _do_init);

/* child object */

enum
{
  ARG_GAIN = 1,
  ARG_FREQ,
  ARG_BANDWIDTH
};

typedef struct _GstIirEqualizerBandClass GstIirEqualizerBandClass;

#define GST_TYPE_IIR_EQUALIZER_BAND \
  (gst_iir_equalizer_band_get_type())
#define GST_IIR_EQUALIZER_BAND(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_IIR_EQUALIZER_BAND,GstIirEqualizerBand))
#define GST_IIR_EQUALIZER_BAND_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_IIR_EQUALIZER_BAND,GstIirEqualizerBandClass))
#define GST_IS_IIR_EQUALIZER_BAND(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_IIR_EQUALIZER_BAND))
#define GST_IS_IIR_EQUALIZER_BAND_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_IIR_EQUALIZER_BAND))

struct _GstIirEqualizerBand
{
  GstObject object;

  /*< private > */
  /* center frequency and gain */
  gdouble freq;
  gdouble gain;
  gdouble width;

  /* second order iir filter */
  gdouble b1, b2;               /* IIR coefficients for outputs */
  gdouble a0, a1, a2;           /* IIR coefficients for inputs */
};

struct _GstIirEqualizerBandClass
{
  GstObjectClass parent_class;
};

static GType gst_iir_equalizer_band_get_type (void);

static void setup_filter (GstIirEqualizer * equ, GstIirEqualizerBand * band);

static void set_passthrough (GstIirEqualizer * equ);

static void
gst_iir_equalizer_band_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstIirEqualizerBand *band = GST_IIR_EQUALIZER_BAND (object);

  switch (prop_id) {
    case ARG_GAIN:{
      gdouble gain;

      gain = g_value_get_double (value);
      GST_DEBUG_OBJECT (band, "gain = %lf -> %lf", band->gain, gain);
      if (gain != band->gain) {
        GstIirEqualizer *equ =
            GST_IIR_EQUALIZER (gst_object_get_parent (GST_OBJECT (band)));

        equ->need_new_coefficients = TRUE;
        band->gain = gain;

        gst_object_unref (equ);
        GST_DEBUG_OBJECT (band, "changed gain = %lf ", band->gain);
      }
      break;
    }
    case ARG_FREQ:{
      gdouble freq;

      freq = g_value_get_double (value);
      GST_DEBUG_OBJECT (band, "freq = %lf -> %lf", band->freq, freq);
      if (freq != band->freq) {
        GstIirEqualizer *equ =
            GST_IIR_EQUALIZER (gst_object_get_parent (GST_OBJECT (band)));

        equ->need_new_coefficients = TRUE;
        band->freq = freq;
        gst_object_unref (equ);
        GST_DEBUG_OBJECT (band, "changed freq = %lf ", band->freq);
      }
      break;
    }
    case ARG_BANDWIDTH:{
      gdouble width;

      width = g_value_get_double (value);
      GST_DEBUG_OBJECT (band, "width = %lf -> %lf", band->width, width);
      if (width != band->width) {
        GstIirEqualizer *equ =
            GST_IIR_EQUALIZER (gst_object_get_parent (GST_OBJECT (band)));

        equ->need_new_coefficients = TRUE;
        band->width = width;
        gst_object_unref (equ);
        GST_DEBUG_OBJECT (band, "changed width = %lf ", band->width);
      }
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_iir_equalizer_band_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstIirEqualizerBand *band = GST_IIR_EQUALIZER_BAND (object);

  switch (prop_id) {
    case ARG_GAIN:
      g_value_set_double (value, band->gain);
      break;
    case ARG_FREQ:
      g_value_set_double (value, band->freq);
      break;
    case ARG_BANDWIDTH:
      g_value_set_double (value, band->width);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_iir_equalizer_band_class_init (GstIirEqualizerBandClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = gst_iir_equalizer_band_set_property;
  gobject_class->get_property = gst_iir_equalizer_band_get_property;

  g_object_class_install_property (gobject_class, ARG_GAIN,
      g_param_spec_double ("gain", "gain",
          "gain for the frequency band ranging from -24.0 dB to +12.0 dB",
          -24.0, 12.0, 0.0, G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE));

  g_object_class_install_property (gobject_class, ARG_FREQ,
      g_param_spec_double ("freq", "freq",
          "center frequency of the band",
          0.0, 100000.0, 0.0, G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE));

  g_object_class_install_property (gobject_class, ARG_BANDWIDTH,
      g_param_spec_double ("bandwidth", "bandwidth",
          "difference between bandedges in Hz",
          0.0, 100000.0, 1.0, G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE));
}

static void
gst_iir_equalizer_band_init (GstIirEqualizerBand * band,
    GstIirEqualizerBandClass * klass)
{
  band->freq = 0.0;
  band->gain = 0.0;
  band->width = 1.0;
}

static GType
gst_iir_equalizer_band_get_type (void)
{
  static GType type = 0;

  if (G_UNLIKELY (!type)) {
    const GTypeInfo type_info = {
      sizeof (GstIirEqualizerBandClass),
      NULL,
      NULL,
      (GClassInitFunc) gst_iir_equalizer_band_class_init,
      NULL,
      NULL,
      sizeof (GstIirEqualizerBand),
      0,
      (GInstanceInitFunc) gst_iir_equalizer_band_init,
    };
    type =
        g_type_register_static (GST_TYPE_OBJECT, "GstIirEqualizerBand",
        &type_info, 0);
  }
  return (type);
}


/* child proxy iface */
static GstObject *
gst_iir_equalizer_child_proxy_get_child_by_index (GstChildProxy * child_proxy,
    guint index)
{
  GstIirEqualizer *equ = GST_IIR_EQUALIZER (child_proxy);

  g_return_val_if_fail (index < equ->freq_band_count, NULL);

  GST_LOG ("return child[%d] '%s'", index, GST_OBJECT_NAME (equ->bands[index]));
  return (gst_object_ref (equ->bands[index]));
}

static guint
gst_iir_equalizer_child_proxy_get_children_count (GstChildProxy * child_proxy)
{
  GstIirEqualizer *equ = GST_IIR_EQUALIZER (child_proxy);

  GST_LOG ("we have %d children", equ->freq_band_count);
  return (equ->freq_band_count);
}

static void
gst_iir_equalizer_child_proxy_interface_init (gpointer g_iface,
    gpointer iface_data)
{
  GstChildProxyInterface *iface = g_iface;

  GST_DEBUG ("initializing iface");

  iface->get_child_by_index = gst_iir_equalizer_child_proxy_get_child_by_index;
  iface->get_children_count = gst_iir_equalizer_child_proxy_get_children_count;
}


/* equalizer implementation */

static void
gst_iir_equalizer_base_init (gpointer g_class)
{
  GstAudioFilterClass *audiofilter_class = GST_AUDIO_FILTER_CLASS (g_class);

  GstCaps *caps;

  caps = gst_caps_from_string (ALLOWED_CAPS);
  gst_audio_filter_class_add_pad_templates (audiofilter_class, caps);
  gst_caps_unref (caps);
}

static void
gst_iir_equalizer_class_init (GstIirEqualizerClass * klass)
{
  GstAudioFilterClass *audio_filter_class = (GstAudioFilterClass *) klass;

  GstBaseTransformClass *btrans_class = (GstBaseTransformClass *) klass;

  GObjectClass *gobject_class = (GObjectClass *) klass;

  gobject_class->finalize = gst_iir_equalizer_finalize;

  audio_filter_class->setup = gst_iir_equalizer_setup;
  btrans_class->transform_ip = gst_iir_equalizer_transform_ip;
}

static void
gst_iir_equalizer_init (GstIirEqualizer * eq, GstIirEqualizerClass * g_class)
{
  eq->need_new_coefficients = TRUE;
}

static void
gst_iir_equalizer_finalize (GObject * object)
{
  GstIirEqualizer *equ = GST_IIR_EQUALIZER (object);

  gint i;

  for (i = 0; i < equ->freq_band_count; i++) {
    if (equ->bands[i])
      gst_object_unparent (GST_OBJECT (equ->bands[i]));
    equ->bands[i] = NULL;
  }
  equ->freq_band_count = 0;

  g_free (equ->bands);
  g_free (equ->history);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static inline gdouble
arg_to_scale (gdouble arg)
{
  return (pow (10.0, arg / 20.0));
}

/* Filter taken from
 *
 * The Equivalence of Various Methods of Computing
 * Biquad Coefficients for Audio Parametric Equalizers
 *
 * by Robert Bristow-Johnson
 *
 * http://www.aes.org/e-lib/browse.cfm?elib=6326
 * http://www.musicdsp.org/files/EQ-Coefficients.pdf
 *
 * The bandwidth method that we use here is the preferred
 * one from this article transformed from octaves to frequency
 * in Hz.
 */
static void
setup_filter (GstIirEqualizer * equ, GstIirEqualizerBand * band)
{
  g_return_if_fail (GST_AUDIO_FILTER (equ)->format.rate);

  /* FIXME: we need better filters
   * - we need shelf-filter for 1st and last band
   */
  {
    gdouble gain, omega, bw;
    gdouble edge_gain, gamma;
    gdouble alpha, beta;

    gain = arg_to_scale (band->gain);

    if (band->freq / GST_AUDIO_FILTER (equ)->format.rate > 0.5)
      omega = M_PI;
    else if (band->freq < 0.0)
      omega = 0.0;
    else
      omega = 2.0 * M_PI * (band->freq / GST_AUDIO_FILTER (equ)->format.rate);

    if (band->width / GST_AUDIO_FILTER (equ)->format.rate >= 0.5) {
      /* If bandwidth == 0.5 the calculation below fails as tan(M_PI/2)
       * is undefined. So set the bandwidth to a slightly smaller value.
       */
      bw = M_PI - 0.00000001;
    } else if (band->width <= 0.0) {
      /* If bandwidth == 0 this band won't change anything so set
       * the coefficients accordingly. The coefficient calculation
       * below would create coefficients that for some reason amplify
       * the band.
       */
      band->a0 = 1.0;
      band->a1 = 0.0;
      band->a2 = 0.0;
      band->b1 = 0.0;
      band->b2 = 0.0;
      gain = 1.0;
      goto out;
    } else {
      bw = 2.0 * M_PI * (band->width / GST_AUDIO_FILTER (equ)->format.rate);
    }

    edge_gain = sqrt (gain);
    gamma = tan (bw / 2.0);

    alpha = gamma * edge_gain;
    beta = gamma / edge_gain;

    band->a0 = (1.0 + alpha) / (1.0 + beta);
    band->a1 = (-2.0 * cos (omega)) / (1.0 + beta);
    band->a2 = (1.0 - alpha) / (1.0 + beta);
    band->b1 = (2.0 * cos (omega)) / (1.0 + beta);
    band->b2 = -(1.0 - beta) / (1.0 + beta);

  out:
    GST_INFO
        ("gain = %5.1f, width= %7.2f, freq = %7.2f, a0 = %7.5g, a1 = %7.5g, a2=%7.5g b1 = %7.5g, b2 = %7.5g",
        band->gain, band->width, band->freq, band->a0, band->a1, band->a2,
        band->b1, band->b2);
  }
}

static void
set_passthrough (GstIirEqualizer * equ)
{
  gint i;

  gboolean passthrough = TRUE;

  for (i = 0; i < equ->freq_band_count; i++) {
    passthrough = passthrough && (equ->bands[i]->gain == 0.0);
  }

  gst_base_transform_set_passthrough (GST_BASE_TRANSFORM (equ), passthrough);
  GST_DEBUG ("Passthrough mode: %d\n", passthrough);
}

static void
update_coefficients (GstIirEqualizer * equ)
{
  gint i;

  for (i = 0; i < equ->freq_band_count; i++) {
    setup_filter (equ, equ->bands[i]);
  }
  equ->need_new_coefficients = FALSE;
}

static void
alloc_history (GstIirEqualizer * equ)
{
  /* free + alloc = no memcpy */
  g_free (equ->history);
  equ->history =
      g_malloc0 (equ->history_size * GST_AUDIO_FILTER (equ)->format.channels *
      equ->freq_band_count);
}

void
gst_iir_equalizer_compute_frequencies (GstIirEqualizer * equ, guint new_count)
{
  guint old_count, i;
  gdouble freq0, freq1, step;
  gchar name[20];

  if (equ->freq_band_count == new_count)
    return;

  old_count = equ->freq_band_count;
  equ->freq_band_count = new_count;
  GST_DEBUG ("bands %u -> %u", old_count, new_count);

  if (old_count < new_count) {
    /* add new bands */
    equ->bands = g_realloc (equ->bands, sizeof (GstObject *) * new_count);
    for (i = old_count; i < new_count; i++) {
      equ->bands[i] = g_object_new (GST_TYPE_IIR_EQUALIZER_BAND, NULL);
      /* otherwise they get names like 'iirequalizerband5' */
      sprintf (name, "band%u", i);
      gst_object_set_name (GST_OBJECT (equ->bands[i]), name);
      GST_DEBUG ("adding band[%d]=%p", i, equ->bands[i]);

      gst_object_set_parent (GST_OBJECT (equ->bands[i]), GST_OBJECT (equ));
      gst_child_proxy_child_added (GST_OBJECT (equ),
          GST_OBJECT (equ->bands[i]));
    }
  } else {
    /* free unused bands */
    for (i = new_count; i < old_count; i++) {
      GST_DEBUG ("removing band[%d]=%p", i, equ->bands[i]);
      gst_child_proxy_child_removed (GST_OBJECT (equ),
          GST_OBJECT (equ->bands[i]));
      gst_object_unparent (GST_OBJECT (equ->bands[i]));
      equ->bands[i] = NULL;
    }
  }

  alloc_history (equ);

  /* set center frequencies and name band objects
   * FIXME: arg! we can't change the name of parented objects :(
   *   application should read band->freq to get the name
   */

  step = pow (HIGHEST_FREQ / LOWEST_FREQ, 1.0 / new_count);
  freq0 = LOWEST_FREQ;
  for (i = 0; i < new_count; i++) {
    freq1 = freq0 * step;
    equ->bands[i]->freq = freq0 + ((freq1 - freq0) / 2.0);
    equ->bands[i]->width = freq1 - freq0;
    GST_DEBUG ("band[%2d] = '%lf'", i, equ->bands[i]->freq);
    /*
       if(equ->bands[i]->freq<10000.0)
       sprintf (name,"%dHz",(gint)equ->bands[i]->freq);
       else
       sprintf (name,"%dkHz",(gint)(equ->bands[i]->freq/1000.0));
       gst_object_set_name( GST_OBJECT (equ->bands[i]), name);
       GST_DEBUG ("band[%2d] = '%s'",i,name);
     */
    freq0 = freq1;
  }

  equ->need_new_coefficients = TRUE;
}

/* start of code that is type specific */

#define CREATE_OPTIMIZED_FUNCTIONS_INT(TYPE,BIG_TYPE,MIN_VAL,MAX_VAL)   \
typedef struct {                                                        \
  BIG_TYPE x1, x2;          /* history of input values for a filter */  \
  BIG_TYPE y1, y2;          /* history of output values for a filter */ \
} SecondOrderHistory ## TYPE;                                           \
                                                                        \
static inline BIG_TYPE                                                  \
one_step_ ## TYPE (GstIirEqualizerBand *filter,                         \
    SecondOrderHistory ## TYPE *history, BIG_TYPE input)                \
{                                                                       \
  /* calculate output */                                                \
  BIG_TYPE output = floor (filter->a0 * input +                         \
      filter->a1 * history->x1 + filter->a2 * history->x2 +             \
      filter->b1 * history->y1 + filter->b2 * history->y2 + 0.5);       \
  /* update history */                                                  \
  history->y2 = history->y1;                                            \
  history->y1 = output;                                                 \
  history->x2 = history->x1;                                            \
  history->x1 = input;                                                  \
                                                                        \
  return output;                                                        \
}                                                                       \
                                                                        \
static const guint                                                      \
history_size_ ## TYPE = sizeof (SecondOrderHistory ## TYPE);            \
                                                                        \
static void                                                             \
gst_iir_equ_process_ ## TYPE (GstIirEqualizer *equ, guint8 *data,       \
guint size, guint channels)                                             \
{                                                                       \
  guint frames = size / channels / sizeof (TYPE);                       \
  guint i, c, f;                                                        \
  BIG_TYPE cur;                                                         \
                                                                        \
  for (i = 0; i < frames; i++) {                                        \
    for (c = 0; c < channels; c++) {                                    \
      SecondOrderHistory ## TYPE *history = equ->history;               \
      cur = *((TYPE *) data);                                           \
      for (f = 0; f < equ->freq_band_count; f++) {                      \
        GstIirEqualizerBand *filter = equ->bands[f];                    \
                                                                        \
        cur = one_step_ ## TYPE (filter, history, cur);                 \
        history++;                                                      \
      }                                                                 \
      cur = CLAMP (cur, MIN_VAL, MAX_VAL);                              \
      *((TYPE *) data) = (TYPE) cur;                                    \
      data += sizeof (TYPE);                                            \
    }                                                                   \
  }                                                                     \
}

#define CREATE_OPTIMIZED_FUNCTIONS(TYPE)       \
typedef struct {                                                        \
  TYPE x1, x2;          /* history of input values for a filter */  \
  TYPE y1, y2;          /* history of output values for a filter */ \
} SecondOrderHistory ## TYPE;                                           \
                                                                        \
static inline TYPE                                                  \
one_step_ ## TYPE (GstIirEqualizerBand *filter,                         \
    SecondOrderHistory ## TYPE *history, TYPE input)                \
{                                                                       \
  /* calculate output */                                                \
  TYPE output = filter->a0 * input + filter->a1 * history->x1 +     \
      filter->a2 * history->x2 + filter->b1 * history->y1 +             \
      filter->b2 * history->y2;                                         \
  /* update history */                                                  \
  history->y2 = history->y1;                                            \
  history->y1 = output;                                                 \
  history->x2 = history->x1;                                            \
  history->x1 = input;                                                  \
                                                                        \
  return output;                                                        \
}                                                                       \
                                                                        \
static const guint                                                      \
history_size_ ## TYPE = sizeof (SecondOrderHistory ## TYPE);            \
                                                                        \
static void                                                             \
gst_iir_equ_process_ ## TYPE (GstIirEqualizer *equ, guint8 *data,       \
guint size, guint channels)                                             \
{                                                                       \
  guint frames = size / channels / sizeof (TYPE);                       \
  guint i, c, f;                                                        \
  TYPE cur;                                                         \
                                                                        \
  for (i = 0; i < frames; i++) {                                        \
    for (c = 0; c < channels; c++) {                                    \
      SecondOrderHistory ## TYPE *history = equ->history;               \
      cur = *((TYPE *) data);                                           \
      for (f = 0; f < equ->freq_band_count; f++) {                      \
        GstIirEqualizerBand *filter = equ->bands[f];                    \
                                                                        \
        cur = one_step_ ## TYPE (filter, history, cur);                 \
        history++;                                                      \
      }                                                                 \
      *((TYPE *) data) = (TYPE) cur;                                    \
      data += sizeof (TYPE);                                            \
    }                                                                   \
  }                                                                     \
}

CREATE_OPTIMIZED_FUNCTIONS_INT (gint16, gint32, -32768, 32767);
CREATE_OPTIMIZED_FUNCTIONS (gfloat);
CREATE_OPTIMIZED_FUNCTIONS (gdouble);

static GstFlowReturn
gst_iir_equalizer_transform_ip (GstBaseTransform * btrans, GstBuffer * buf)
{
  GstAudioFilter *filter = GST_AUDIO_FILTER (btrans);

  GstIirEqualizer *equ = GST_IIR_EQUALIZER (btrans);

  GstClockTime timestamp;

  if (G_UNLIKELY (filter->format.channels < 1 || equ->process == NULL))
    return GST_FLOW_NOT_NEGOTIATED;

  if (equ->need_new_coefficients) {
    update_coefficients (equ);
    set_passthrough (equ);
  }

  if (gst_base_transform_is_passthrough (btrans))
    return GST_FLOW_OK;

  timestamp = GST_BUFFER_TIMESTAMP (buf);
  timestamp =
      gst_segment_to_stream_time (&btrans->segment, GST_FORMAT_TIME, timestamp);

  if (GST_CLOCK_TIME_IS_VALID (timestamp))
    gst_object_sync_values (G_OBJECT (equ), timestamp);

  equ->process (equ, GST_BUFFER_DATA (buf), GST_BUFFER_SIZE (buf),
      filter->format.channels);

  return GST_FLOW_OK;
}

static gboolean
gst_iir_equalizer_setup (GstAudioFilter * audio, GstRingBufferSpec * fmt)
{
  GstIirEqualizer *equ = GST_IIR_EQUALIZER (audio);

  switch (fmt->type) {
    case GST_BUFTYPE_LINEAR:
      switch (fmt->width) {
        case 16:
          equ->history_size = history_size_gint16;
          equ->process = gst_iir_equ_process_gint16;
          break;
        default:
          return FALSE;
      }
      break;
    case GST_BUFTYPE_FLOAT:
      switch (fmt->width) {
        case 32:
          equ->history_size = history_size_gfloat;
          equ->process = gst_iir_equ_process_gfloat;
          break;
        case 64:
          equ->history_size = history_size_gdouble;
          equ->process = gst_iir_equ_process_gdouble;
          break;
        default:
          return FALSE;
      }
      break;
    default:
      return FALSE;
  }

  alloc_history (equ);
  return TRUE;
}


static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (equalizer_debug, "equalizer", 0, "equalizer");

  if (!(gst_element_register (plugin, "equalizer-nbands", GST_RANK_NONE,
              GST_TYPE_IIR_EQUALIZER_NBANDS)))
    return FALSE;

  if (!(gst_element_register (plugin, "equalizer-3bands", GST_RANK_NONE,
              GST_TYPE_IIR_EQUALIZER_3BANDS)))
    return FALSE;

  if (!(gst_element_register (plugin, "equalizer-10bands", GST_RANK_NONE,
              GST_TYPE_IIR_EQUALIZER_10BANDS)))
    return FALSE;

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "equalizer",
    "GStreamer audio equalizers",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
