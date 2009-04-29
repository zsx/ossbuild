/*
 * Farsight Voice+Video library
 *
 *  Copyright 2006 Collabora Ltd, 
 *  Copyright 2006 Nokia Corporation
 *   @author: Philippe Kalaf <philippe.kalaf@collabora.co.uk>.
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
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "gstnetsim.h"

GST_DEBUG_CATEGORY (netsim_debug);
#define GST_CAT_DEFAULT (netsim_debug)

/* elementfactory information */
static const GstElementDetails gst_net_sim_details =
GST_ELEMENT_DETAILS (
  "Network Simulator",
  "Filter/Network",
  "An element that simulates network jitter, packet loss and packet duplication",
  "Philippe Kalaf <philippe.kalaf@collabora.co.uk>");

/* NetSim signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  ARG_0,
  ARG_MIN_DELAY,
  ARG_MAX_DELAY,
  ARG_DELAY_PROBABILITY,
  ARG_DROP_PROBABILITY,
  ARG_DUPLICATE_PROBABILITY
};

struct _GstNetSimPrivate
{
  GstPad *sinkpad, *srcpad;

  GMainLoop *main_loop;
  GMainContext *main_context;
  GRand *rand_seed;
  gint min_delay;
  gint max_delay;
  gfloat delay_probability;
  gfloat drop_probability;
  gfloat duplicate_probability;
};

typedef struct
{
  GstNetSim *netsim;
  GstBuffer *buffer;
} SourceInfo;

/* these numbers are nothing but wild guesses and dont reflect any reality */
#define DEFAULT_MIN_DELAY 200
#define DEFAULT_MAX_DELAY 400
#define DEFAULT_DELAY_PROBABILITY 0.5
#define DEFAULT_DROP_PROBABILITY 0.2
#define DEFAULT_DUPLICATE_PROBABILITY 0.1

#define GST_NET_SIM_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GST_TYPE_NET_SIM, \
                                GstNetSimPrivate))

static GstStaticPadTemplate gst_net_sim_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY
    /*
        "payload = (int) , "
        "clock-rate = (int) , "
        "encoding-name = (string) ")
*/
    );

static GstStaticPadTemplate gst_net_sim_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY 
    /*
        "payload = (int) , "
        "clock-rate = (int) , "
        "encoding-name = (string) ")
*/
    );

static void gst_net_sim_set_property (GObject *object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_net_sim_get_property (GObject *object,
    guint prop_id, GValue *value, GParamSpec *pspec);
static void gst_net_sim_dispose (GObject *object);

static gboolean
gst_net_sim_src_activate_push (GstPad * pad, gboolean active);

static GstStateChangeReturn gst_net_sim_change_state (
    GstElement *element, GstStateChange transition);
static GstFlowReturn gst_net_sim_chain (GstPad *pad,
    GstBuffer *buffer);
static void gst_net_sim_loop (GstNetSim *netsim);

static void
_do_init (GType type)
{
  GST_DEBUG_CATEGORY_INIT
    (netsim_debug, "netsim", 0, "Network simulator");
}

GST_BOILERPLATE_FULL (GstNetSim, gst_net_sim, GstElement,
    GST_TYPE_ELEMENT, _do_init);

static void
gst_net_sim_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_net_sim_src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_net_sim_sink_template));
  gst_element_class_set_details (element_class, &gst_net_sim_details);
}

static void
gst_net_sim_class_init (GstNetSimClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->dispose = GST_DEBUG_FUNCPTR (gst_net_sim_dispose);

  gobject_class->set_property = gst_net_sim_set_property;
  gobject_class->get_property = gst_net_sim_get_property;

  g_object_class_install_property (gobject_class, ARG_MIN_DELAY,
      g_param_spec_int ("min_delay",
        "Minimum delay (ms)",
        "The minimum delay in ms to apply to buffers",
          G_MININT, G_MAXINT, DEFAULT_MIN_DELAY, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, ARG_MAX_DELAY,
      g_param_spec_int ("max_delay",
        "Maximum delay (ms)",
        "The maximum delay in ms to apply to buffers",
          G_MININT, G_MAXINT, DEFAULT_MAX_DELAY, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
      ARG_DELAY_PROBABILITY, g_param_spec_float ("delay_probability",
          "Delay Probability",
          "The Probability a buffer is delayed", 0.0, 1.0,
          DEFAULT_DELAY_PROBABILITY, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
      ARG_DROP_PROBABILITY, g_param_spec_float ("drop_probability",
          "Drop Probability",
          "The Probability a buffer is dropped", 0.0, 1.0,
          DEFAULT_DROP_PROBABILITY, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
      ARG_DUPLICATE_PROBABILITY, g_param_spec_float ("duplicate_probability",
          "Duplicate Probability",
          "The Probability a buffer is duplicated", 0.0, 1.0,
          DEFAULT_DUPLICATE_PROBABILITY, G_PARAM_READWRITE));

  gstelement_class->change_state = gst_net_sim_change_state;

  g_type_class_add_private (klass, sizeof (GstNetSimPrivate));

  parent_class = g_type_class_peek_parent (klass);
}

static void
gst_net_sim_init (GstNetSim *netsim,
    GstNetSimClass *klass)
{
  GstNetSimPrivate *priv =
    GST_NET_SIM_GET_PRIVATE (netsim);
  netsim->priv = priv;

  priv->srcpad =
    gst_pad_new_from_static_template (&gst_net_sim_src_template, "src");
  priv->sinkpad =
    gst_pad_new_from_static_template (&gst_net_sim_sink_template, "sink");

  gst_element_add_pad (GST_ELEMENT (netsim), priv->srcpad);
  gst_element_add_pad (GST_ELEMENT (netsim), priv->sinkpad);

  priv->min_delay = DEFAULT_MIN_DELAY;
  priv->max_delay = DEFAULT_MAX_DELAY;
  priv->delay_probability = DEFAULT_DELAY_PROBABILITY;
  priv->drop_probability = DEFAULT_DROP_PROBABILITY;
  priv->duplicate_probability = DEFAULT_DUPLICATE_PROBABILITY;
 
  priv->rand_seed = g_rand_new ();

  gst_pad_set_chain_function (priv->sinkpad, gst_net_sim_chain);

  gst_pad_set_activatepush_function (priv->srcpad,
      GST_DEBUG_FUNCPTR (gst_net_sim_src_activate_push));
}

static void
gst_net_sim_dispose (GObject *object)
{
  GstNetSim *netsim = NULL;
  netsim = GST_NET_SIM (netsim);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static gboolean
gst_net_sim_src_activate_push (GstPad * pad, gboolean active)
{
  gboolean result = TRUE;
  GstNetSim *netsim = NULL;

  netsim = GST_NET_SIM (gst_pad_get_parent (pad));

  if (active) {
#if 0
    /* we do not start the task yet if the pad is not connected */
    if (gst_pad_is_linked (pad))
      result = gst_pad_start_task (pad, (GstTaskFunction) gst_queue_loop, pad);
    else {
      GST_DEBUG_OBJECT (queue, "not starting task as pad is not linked");
      result = TRUE;
    }
#endif
  } else {
    g_main_loop_quit (netsim->priv->main_loop);
    /* NOTE this will hardlock if the state change is called from the src pad
     * task thread */
    GST_DEBUG_OBJECT (netsim, "Stopping task on srcpad");
    result = gst_pad_stop_task (pad);
  }

  gst_object_unref (netsim);

  return result;
}

static GstStateChangeReturn
gst_net_sim_change_state (GstElement * element,
    GstStateChange transition)
{
  GstNetSim *netsim;
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

  netsim = GST_NET_SIM (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      GST_DEBUG_OBJECT (netsim, "Starting task on srcpad");
      gst_pad_start_task (netsim->priv->srcpad,
          (GstTaskFunction) gst_net_sim_loop, netsim);
      break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      break;
    default:
      break;
  }

  return ret;
}

static gboolean
push_buffer (SourceInfo *info)
{
  GST_DEBUG_OBJECT (info->netsim, "Pushing buffer now");
  gst_pad_push (info->netsim->priv->srcpad, info->buffer);
  g_free (info);

  return FALSE;
}

static GstFlowReturn
gst_net_sim_delay_buffer (GstNetSim *netsim, GstPad *pad, GstBuffer *buffer)
{
  GstNetSimPrivate *priv = netsim->priv;
  GSource *source;

  if (priv->delay_probability > 0)
  {
    if (g_rand_double_range (priv->rand_seed, 0, 1) < priv->delay_probability)
    {
      SourceInfo *info = g_new0 (SourceInfo, 1);
      gint delay;
      info->netsim = netsim;
      info->buffer = buffer;
      delay = g_rand_int_range (priv->rand_seed, priv->min_delay,
          priv->max_delay);
      GST_DEBUG_OBJECT (netsim, "Delaying packet by %d", delay);
      source = g_timeout_source_new (delay);
      g_source_set_callback (source, (GSourceFunc)push_buffer, info, NULL);
      g_source_attach (source, priv->main_context);

      return GST_FLOW_OK;
    }
  }

  return gst_pad_push (pad, buffer);
}

static GstFlowReturn
gst_net_sim_chain (GstPad *pad, GstBuffer *buffer)
{
  GstNetSim *netsim;
  GstNetSimPrivate *priv;

  netsim = GST_NET_SIM (gst_pad_get_parent (pad));
  priv = netsim->priv;

  if (priv->drop_probability > 0)
  {
    if ((gfloat)g_rand_double_range (priv->rand_seed, 0, 1) <
        priv->drop_probability)
    {
      GST_DEBUG_OBJECT (netsim, "Dropping packet");
      gst_buffer_unref (buffer);
      return GST_FLOW_OK;
    }
  }
  if (priv->duplicate_probability > 0)
  {
    if ((gfloat)g_rand_double_range (priv->rand_seed, 0, 1) <
        priv->duplicate_probability)
    {
      GST_DEBUG_OBJECT (netsim, "Duplicating packet");
      gst_buffer_ref (buffer);
      gst_net_sim_delay_buffer (netsim, priv->srcpad, buffer);
    }
  }

  return gst_net_sim_delay_buffer (netsim, priv->srcpad, buffer);
}


static void
gst_net_sim_loop (GstNetSim *netsim)
{
  GstNetSimPrivate *priv;

  priv = netsim->priv;

  GST_DEBUG_OBJECT (netsim, "Creating mainloop and context");
  priv->main_context = g_main_context_new ();
  priv->main_loop = g_main_loop_new (priv->main_context, FALSE);

  g_main_loop_run (priv->main_loop);
}

static void
gst_net_sim_set_property (GObject *object,
    guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GstNetSim *netsim = GST_NET_SIM (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    case ARG_MIN_DELAY:
      netsim->priv->min_delay = g_value_get_int (value);
      break;
    case ARG_MAX_DELAY:
      netsim->priv->max_delay = g_value_get_int (value);
      break;
    case ARG_DELAY_PROBABILITY:
      netsim->priv->delay_probability = g_value_get_float (value);
      break;
    case ARG_DROP_PROBABILITY:
      netsim->priv->drop_probability = g_value_get_float (value);
      break;
    case ARG_DUPLICATE_PROBABILITY:
      netsim->priv->duplicate_probability = g_value_get_float (value);
      break;
  }
}

static void
gst_net_sim_get_property (GObject *object,
    guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstNetSim *netsim = GST_NET_SIM (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    case ARG_MIN_DELAY:
      g_value_set_int (value, netsim->priv->min_delay);
      break;
    case ARG_MAX_DELAY:
      g_value_set_int (value, netsim->priv->max_delay);
      break;
    case ARG_DELAY_PROBABILITY:
      g_value_set_float (value, netsim->priv->delay_probability);
      break;
    case ARG_DROP_PROBABILITY:
      g_value_set_float (value, netsim->priv->drop_probability);
      break;
    case ARG_DUPLICATE_PROBABILITY:
      g_value_set_float (value, netsim->priv->duplicate_probability);
      break;
  }
}

gboolean
gst_net_sim_plugin_init (GstPlugin *plugin)
{
  return gst_element_register (plugin, "netsim",
      GST_RANK_MARGINAL, GST_TYPE_NET_SIM);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "netsim",
    "Network Simulator",
    gst_net_sim_plugin_init, VERSION, "LGPL", "Farsight", "http://farsight.sf.net")
