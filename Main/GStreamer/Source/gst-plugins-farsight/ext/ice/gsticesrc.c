/*
 * GStreamer
 * Farsight Voice+Video library
 *  Copyright 2006 Collabora Ltd, 
 *  Copyright 2006 Nokia Corporation
 *   @author: Philippe Khalaf <philippe.khalaf@collabora.co.uk>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
#include <stdlib.h>
#include <sys/select.h>
*/
#include <stdio.h>
#include <sched.h>
#include <pthread.h>

#include "string.h"

#include "gsticesrc.h"
#include "jingle_c.h"

GST_DEBUG_CATEGORY (icesrc_debug);
#define GST_CAT_DEFAULT (icesrc_debug)

#define QUEUE_MAX_ITEMS 30
#define QUEUE_FLUSH_AFTER 90
#define MAX_INITIAL_POPS 10

static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

static GstElementDetails gst_icesrc_details =
GST_ELEMENT_DETAILS ("ICE packet source",
    "Source/Network",
    "Receive data over the network via libjingle",
    "Philippe Kalaf <philippe.kalaf@collabora.co.uk>");

enum
{
  PROP_0,
  PROP_SOCKETCLIENT
  /* FILL ME */
};

static GstFlowReturn gst_icesrc_create (GstPushSrc * psrc, GstBuffer ** buf);
static void gst_icesrc_dispose (GObject * icesrc);

static gboolean gst_icesrc_unlock (GstBaseSrc * bsrc);

static GstStateChangeReturn gst_icesrc_change_state (GstElement * element,
    GstStateChange transition);

static void gst_icesrc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_icesrc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
void gst_icesrc_packet_read (gpointer src, const gpointer data,
        guint len, const guint32 ip, const guint16 port);

static void
_do_init (GType type)
{
  GST_DEBUG_CATEGORY_INIT (icesrc_debug, "icesrc", 0, "ICE src");
}

GST_BOILERPLATE_FULL (GstIceSrc, gst_icesrc, GstPushSrc, GST_TYPE_PUSH_SRC,
    _do_init);

static void
gst_icesrc_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_template));

  gst_element_class_set_details (element_class, &gst_icesrc_details);
}

static void
gst_icesrc_class_init (GstIceSrcClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSrcClass *gstbasesrc_class;
  GstPushSrcClass *gstpushsrc_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasesrc_class = (GstBaseSrcClass *) klass;
  gstpushsrc_class = (GstPushSrcClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_PUSH_SRC);

  gobject_class->dispose = GST_DEBUG_FUNCPTR (gst_icesrc_dispose);

  gstbasesrc_class->unlock = GST_DEBUG_FUNCPTR (gst_icesrc_unlock);

  gstelement_class->change_state = gst_icesrc_change_state;

  gobject_class->set_property = gst_icesrc_set_property;
  gobject_class->get_property = gst_icesrc_get_property;

  g_object_class_install_property (gobject_class, PROP_SOCKETCLIENT,
      g_param_spec_pointer ("socketclient", "socketclient pointer",
          "A pointer to the SocketClient object define in farsight rtp",
          G_PARAM_READWRITE));

  gstpushsrc_class->create = gst_icesrc_create;
}

static void
gst_icesrc_init (GstIceSrc * icesrc, GstIceSrcClass * g_class)
{
  gst_base_src_set_live (GST_BASE_SRC (icesrc), TRUE);

  icesrc->data_queue = g_async_queue_new ();
  icesrc->flush_queue = 0;
  icesrc->initial_pops = 0;
  icesrc->initial_pops_done = FALSE;
}

static void
gst_icesrc_dispose (GObject * src)
{
  GstIceSrc *icesrc;

  icesrc = GST_ICESRC (src);
  g_async_queue_unref (icesrc->data_queue);

  G_OBJECT_CLASS (parent_class)->dispose (src);
}

static gboolean gst_icesrc_unlock (GstBaseSrc * bsrc)
{
  GstIceSrc *icesrc;

  icesrc = GST_ICESRC (bsrc);

  GstBuffer *signal_buffer;
  gchar *signal_string;

  /* We need to tell _create to return WRONG_STATE */
  signal_string = g_strdup("RETURN_WRONG_STATE");

  signal_buffer = gst_buffer_new();
  GST_BUFFER_FLAG_SET (signal_buffer, GST_BUFFER_FLAG_LAST);
  GST_BUFFER_DATA (signal_buffer) = (guint8 *)signal_string;
  GST_BUFFER_MALLOCDATA (signal_buffer) = (guint8 *)signal_string;
  GST_BUFFER_SIZE (signal_buffer) = strlen(signal_string);

  g_async_queue_push (icesrc->data_queue, (gpointer)signal_buffer);

  return TRUE;
}

#if 0
/**
 * gst_icesrc_worker_thread_started:
 *
 * Called when jingle's worker thread is started
 */
void gst_icesrc_worker_thread_started (src)
{
  GstIceSrc *icesrc;
  icesrc = GST_ICESRC (src);

  g_async_queue_ref (icesrc->data_queue);
}

/**
 * gst_icesrc_worker_thread_stopped:
 *
 * Called when jingle's worker thread is started
 */
void gst_icesrc_worker_thread_stopped (src)
{
  GstIceSrc *icesrc;
  icesrc = GST_ICESRC (src);

  g_async_queue_unref (icesrc->data_queue);
}
#endif

/*static void _set_priority ()
{
  static int prioset = 0;
  int schedpol;
  int minprio, maxprio;
  struct sched_param schedpar;

  if (!prioset)
  {
    g_print("\n\n\n *** read_packet, thread id: %u\n", (unsigned int) pthread_self());
    if (pthread_getschedparam(pthread_self(), &schedpol, &schedpar) != 0)
      perror("pthread_get_schedparam()");
    g_print("    policy: %i, priority %i\n", schedpol, schedpar.sched_priority);
    schedpol = SCHED_FIFO;
    minprio = sched_get_priority_min(schedpol);
    maxprio = sched_get_priority_max(schedpol);
    if (minprio >= 0 && maxprio >= 0)
      schedpar.sched_priority = minprio + (maxprio - minprio) / 2;
    g_print("    priority range %i - %i, new policy %i, new priority %i\n\n\n", minprio, maxprio, schedpol, schedpar.sched_priority);
    if (pthread_setschedparam(pthread_self(), schedpol, &schedpar) != 0)
      perror("pthread_set_schedparam()");
    prioset = 1;
  }
}*/

/**
 * gst_icesrc_packet_read:
 *
 * This is a threadsafe function to cause this src to push data from libjingle
 */
void gst_icesrc_packet_read (gpointer src, const gpointer data,
        guint len, const guint32 ip, const guint16 port)
{
  GstIceSrc *icesrc;
  icesrc = GST_ICESRC (src);
  guint8 *copied_data;
  /*struct timeval to;*/

  /*_set_priority();*/

  GST_DEBUG_OBJECT (icesrc, "packet read cb called %d bytes", len);
  //gst_util_dump_mem (data, 16);

  /* moved the buffer copy outside of the lock region,
   * as this is local to this function  -jl */
  GstNetBuffer *buffer;
  buffer = gst_netbuffer_new ();
  /* FIXME(jl): this should get the memory from pool and not through alloc */
  copied_data = (guint8 *)g_memdup (data, len);

  GST_BUFFER_DATA (buffer) = copied_data;
  GST_BUFFER_MALLOCDATA (buffer) = copied_data;
  GST_BUFFER_SIZE (buffer) = len;
  //gst_util_dump_mem (GST_BUFFER_DATA(buffer), 16);

  gst_netaddress_set_ip4_address (&(buffer->from), ip, port);

  /* keep the lock as short period of time as possible,
   * even though the push signal condition, so no one should be reading atm */
  g_async_queue_lock (icesrc->data_queue);
  /* let's drop oldest packet and push this one if the queue is already full */
  if (g_async_queue_length_unlocked (icesrc->data_queue) > QUEUE_MAX_ITEMS)
  {
    if (icesrc->initial_pops_done)
    {
      GST_DEBUG_OBJECT (icesrc, "Queue full, dropping old packet");
      GstBuffer *buf;
      buf = g_async_queue_pop_unlocked (icesrc->data_queue);
      gst_buffer_unref (buf);
    }
  }
  g_async_queue_push_unlocked (icesrc->data_queue, (gpointer)buffer);
  g_async_queue_unlock (icesrc->data_queue);

  /* FIXME(jl): is this really needed?
   *  this might be pretty much nop in linux,
   *  so do it known-to-work -way..   -jl */
  /*g_thread_yield();*/
  /* system may modify timeout struct, so better initialize it every time */
  /*
   * FIXME calling select this way crashes the application
  to.tv_sec = to.tv_usec = 0;
  select(0, NULL, NULL, NULL, &to);
  */
}

static GstFlowReturn
gst_icesrc_create (GstPushSrc * psrc, GstBuffer ** buf)
{
  GstIceSrc *icesrc;
  GstNetBuffer *outbuf;
  icesrc = GST_ICESRC (psrc);

  g_async_queue_lock (icesrc->data_queue);
  outbuf = (GstNetBuffer*) g_async_queue_pop_unlocked (icesrc->data_queue);
  if (!icesrc->initial_pops_done)
  {
    icesrc->initial_pops++;
  }
  if (icesrc->initial_pops > MAX_INITIAL_POPS)
  {
    icesrc->initial_pops_done = TRUE;
  }

  if (g_async_queue_length_unlocked (icesrc->data_queue) > 0)
  {
    icesrc->flush_queue++;
  }

  /* if we ran QUEUE_FLUSH_AFTER times and the queue is still not empty, let's
   * flush the queue */
  if (icesrc->flush_queue >= QUEUE_FLUSH_AFTER)
  {
    GST_DEBUG_OBJECT (icesrc, "Flushing queue");
    while (g_async_queue_length_unlocked (icesrc->data_queue) > 0)
    {
      GstBuffer *buf;
      buf = g_async_queue_pop_unlocked (icesrc->data_queue);
      gst_buffer_unref (buf);
    }
    icesrc->flush_queue = 0;
  }
  g_async_queue_unlock (icesrc->data_queue);

  if (GST_BUFFER_FLAG_IS_SET (outbuf, GST_BUFFER_FLAG_LAST))
  {
      GST_DEBUG_OBJECT (icesrc, "Flag set on buffer in queue, checking for signal message");
      if (g_ascii_strcasecmp ((gchar *)GST_BUFFER_DATA(outbuf), "RETURN_WRONG_STATE") == 0)
          return GST_FLOW_WRONG_STATE;
  }
  //gst_util_dump_mem (GST_BUFFER_DATA(outbuf), 16);

  //gst_buffer_ref(GST_BUFFER(outbuf));

  *buf = GST_BUFFER (outbuf);

  return GST_FLOW_OK;
}

static GstStateChangeReturn
gst_icesrc_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret;
  GstIceSrc *icesrc;
  icesrc = GST_ICESRC (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      if (icesrc->sockclient)
      {
        connect_signal_socket_read (icesrc->sockclient,
            gst_icesrc_packet_read, icesrc);
      }
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
      if (icesrc->sockclient)
      {
        disconnect_signal_socket_read(icesrc->sockclient, 
            gst_icesrc_packet_read);
      }
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      break;
    default:
      break;
  }
  return ret;
}

static void
gst_icesrc_set_property (GObject * object, guint prop_id, const GValue * value,
    GParamSpec * pspec)
{
  GstIceSrc *icesrc = GST_ICESRC (object);

  switch (prop_id) {
    case PROP_SOCKETCLIENT:
      icesrc->sockclient = g_value_get_pointer (value);
          break;
    default:
      break;
  }
}

static void
gst_icesrc_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstIceSrc *icesrc = GST_ICESRC (object);

  switch (prop_id) {
    case PROP_SOCKETCLIENT:
      g_value_set_pointer (value, icesrc->sockclient);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}
