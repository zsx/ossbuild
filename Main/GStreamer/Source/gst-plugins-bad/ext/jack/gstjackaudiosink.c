/* GStreamer
 * Copyright (C) 2006 Wim Taymans <wim@fluendo.com>
 *
 * gstjackaudiosink.c: jack audio sink implementation
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
 * SECTION:element-jackaudiosink
 * @see_also: #GstBaseAudioSink, #GstRingBuffer
 *
 * A Sink that outputs data to Jack ports.
 * 
 * It will create N Jack ports named out_&lt;name&gt;_&lt;num&gt; where 
 * &lt;name&gt; is the element name and &lt;num&gt; is starting from 1.
 * Each port corresponds to a gstreamer channel.
 * 
 * The samplerate as exposed on the caps is always the same as the samplerate of
 * the jack server.
 * 
 * When the #GstJackAudioSink:connect property is set to auto, this element
 * will try to connect each output port to a random physical jack input pin. In
 * this mode, the sink will expose the number of physical channels on its pad
 * caps.
 * 
 * When the #GstJackAudioSink:connect property is set to none, the element will
 * accept any number of input channels and will create (but not connect) an
 * output port for each channel.
 * 
 * The element will generate an error when the Jack server is shut down when it
 * was PAUSED or PLAYING. This element does not support dynamic rate and buffer
 * size changes at runtime.
 * 
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch audiotestsrc ! jackaudiosink
 * ]| Play a sine wave to using jack.
 * </refsect2>
 *
 * Last reviewed on 2006-11-30 (0.10.4)
 */

#include <stdlib.h>
#include <string.h>

#include "gstjackaudiosink.h"
#include "gstjackringbuffer.h"

GST_DEBUG_CATEGORY_STATIC (gst_jack_audio_sink_debug);
#define GST_CAT_DEFAULT gst_jack_audio_sink_debug

static gboolean
gst_jack_audio_sink_allocate_channels (GstJackAudioSink * sink, gint channels)
{
  jack_client_t *client;

  client = gst_jack_audio_client_get_client (sink->client);

  /* remove ports we don't need */
  while (sink->port_count > channels) {
    jack_port_unregister (client, sink->ports[--sink->port_count]);
  }

  /* alloc enough output ports */
  sink->ports = g_realloc (sink->ports, sizeof (jack_port_t *) * channels);

  /* create an output port for each channel */
  while (sink->port_count < channels) {
    gchar *name;

    /* port names start from 1 and are local to the element */
    name =
        g_strdup_printf ("out_%s_%d", GST_ELEMENT_NAME (sink),
        sink->port_count + 1);
    sink->ports[sink->port_count] =
        jack_port_register (client, name, JACK_DEFAULT_AUDIO_TYPE,
        JackPortIsOutput, 0);
    if (sink->ports[sink->port_count] == NULL)
      return FALSE;

    sink->port_count++;

    g_free (name);
  }
  return TRUE;
}

static void
gst_jack_audio_sink_free_channels (GstJackAudioSink * sink)
{
  gint res, i = 0;
  jack_client_t *client;

  client = gst_jack_audio_client_get_client (sink->client);

  /* get rid of all ports */
  while (sink->port_count) {
    GST_LOG_OBJECT (sink, "unregister port %d", i);
    if ((res = jack_port_unregister (client, sink->ports[i++]))) {
      GST_DEBUG_OBJECT (sink, "unregister of port failed (%d)", res);
    }
    sink->port_count--;
  }
  g_free (sink->ports);
  sink->ports = NULL;
}

/* ringbuffer abstract base class */
static GType
gst_jack_ring_buffer_get_type (void)
{
  static GType ringbuffer_type = 0;

  if (!ringbuffer_type) {
    static const GTypeInfo ringbuffer_info = {
      sizeof (GstJackRingBufferClass),
      NULL,
      NULL,
      (GClassInitFunc) gst_jack_ring_buffer_class_init,
      NULL,
      NULL,
      sizeof (GstJackRingBuffer),
      0,
      (GInstanceInitFunc) gst_jack_ring_buffer_init,
      NULL
    };

    ringbuffer_type =
        g_type_register_static (GST_TYPE_RING_BUFFER,
        "GstJackAudioSinkRingBuffer", &ringbuffer_info, 0);
  }
  return ringbuffer_type;
}

static void
gst_jack_ring_buffer_class_init (GstJackRingBufferClass * klass)
{
  GObjectClass *gobject_class;
  GstObjectClass *gstobject_class;
  GstRingBufferClass *gstringbuffer_class;

  gobject_class = (GObjectClass *) klass;
  gstobject_class = (GstObjectClass *) klass;
  gstringbuffer_class = (GstRingBufferClass *) klass;

  ring_parent_class = g_type_class_peek_parent (klass);

  gobject_class->dispose = GST_DEBUG_FUNCPTR (gst_jack_ring_buffer_dispose);
  gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_jack_ring_buffer_finalize);

  gstringbuffer_class->open_device =
      GST_DEBUG_FUNCPTR (gst_jack_ring_buffer_open_device);
  gstringbuffer_class->close_device =
      GST_DEBUG_FUNCPTR (gst_jack_ring_buffer_close_device);
  gstringbuffer_class->acquire =
      GST_DEBUG_FUNCPTR (gst_jack_ring_buffer_acquire);
  gstringbuffer_class->release =
      GST_DEBUG_FUNCPTR (gst_jack_ring_buffer_release);
  gstringbuffer_class->start = GST_DEBUG_FUNCPTR (gst_jack_ring_buffer_start);
  gstringbuffer_class->pause = GST_DEBUG_FUNCPTR (gst_jack_ring_buffer_pause);
  gstringbuffer_class->resume = GST_DEBUG_FUNCPTR (gst_jack_ring_buffer_start);
  gstringbuffer_class->stop = GST_DEBUG_FUNCPTR (gst_jack_ring_buffer_stop);

  gstringbuffer_class->delay = GST_DEBUG_FUNCPTR (gst_jack_ring_buffer_delay);
}

/* this is the callback of jack. This should RT-safe.
 */
static int
jack_process_cb (jack_nframes_t nframes, void *arg)
{
  GstJackAudioSink *sink;
  GstRingBuffer *buf;
  GstJackRingBuffer *abuf;
  gint readseg, len;
  guint8 *readptr;
  gint i, j, flen, channels;
  sample_t **buffers, *data;

  buf = GST_RING_BUFFER_CAST (arg);
  abuf = GST_JACK_RING_BUFFER_CAST (arg);
  sink = GST_JACK_AUDIO_SINK (GST_OBJECT_PARENT (buf));

  channels = buf->spec.channels;

  /* alloc pointers to samples */
  buffers = g_alloca (sizeof (sample_t *) * channels);

  /* get target buffers */
  for (i = 0; i < channels; i++) {
    buffers[i] = (sample_t *) jack_port_get_buffer (sink->ports[i], nframes);
  }

  if (gst_ring_buffer_prepare_read (buf, &readseg, &readptr, &len)) {
    flen = len / channels;

    /* the number of samples must be exactly the segment size */
    if (nframes * sizeof (sample_t) != flen)
      goto wrong_size;

    GST_DEBUG_OBJECT (sink, "copy %d frames: %p, %d bytes, %d channels",
        nframes, readptr, flen, channels);
    data = (sample_t *) readptr;

    /* the samples in the ringbuffer have the channels interleaved, we need to
     * deinterleave into the jack target buffers */
    for (i = 0; i < nframes; i++) {
      for (j = 0; j < channels; j++) {
        buffers[j][i] = *data++;
      }
    }

    /* clear written samples in the ringbuffer */
    gst_ring_buffer_clear (buf, readseg);

    /* we wrote one segment */
    gst_ring_buffer_advance (buf, 1);
  } else {
    GST_DEBUG_OBJECT (sink, "write %d frames silence", nframes);
    /* We are not allowed to read from the ringbuffer, write silence to all
     * jack output buffers */
    for (i = 0; i < channels; i++) {
      memset (buffers[i], 0, nframes * sizeof (sample_t));
    }
  }
  return 0;

  /* ERRORS */
wrong_size:
  {
    GST_ERROR_OBJECT (sink, "nbytes (%d) != flen (%d)",
        (gint) (nframes * sizeof (sample_t)), flen);
    return 1;
  }
}

/* we error out */
static int
jack_sample_rate_cb (jack_nframes_t nframes, void *arg)
{
  GstJackAudioSink *sink;
  GstJackRingBuffer *abuf;

  abuf = GST_JACK_RING_BUFFER_CAST (arg);
  sink = GST_JACK_AUDIO_SINK (GST_OBJECT_PARENT (arg));

  if (abuf->sample_rate != -1 && abuf->sample_rate != nframes)
    goto not_supported;

  return 0;

  /* ERRORS */
not_supported:
  {
    GST_ELEMENT_ERROR (sink, RESOURCE, SETTINGS,
        (NULL), ("Jack changed the sample rate, which is not supported"));
    return 1;
  }
}

/* we error out */
static int
jack_buffer_size_cb (jack_nframes_t nframes, void *arg)
{
  GstJackAudioSink *sink;
  GstJackRingBuffer *abuf;

  abuf = GST_JACK_RING_BUFFER_CAST (arg);
  sink = GST_JACK_AUDIO_SINK (GST_OBJECT_PARENT (arg));

  if (abuf->buffer_size != -1 && abuf->buffer_size != nframes)
    goto not_supported;

  return 0;

  /* ERRORS */
not_supported:
  {
    GST_ELEMENT_ERROR (sink, RESOURCE, SETTINGS,
        (NULL), ("Jack changed the buffer size, which is not supported"));
    return 1;
  }
}

static void
jack_shutdown_cb (void *arg)
{
  GstJackAudioSink *sink;

  sink = GST_JACK_AUDIO_SINK (GST_OBJECT_PARENT (arg));

  GST_DEBUG_OBJECT (sink, "shutdown");

  GST_ELEMENT_ERROR (sink, RESOURCE, NOT_FOUND,
      (NULL), ("Jack server shutdown"));
}

static void
gst_jack_ring_buffer_init (GstJackRingBuffer * buf,
    GstJackRingBufferClass * g_class)
{
  buf->channels = -1;
  buf->buffer_size = -1;
  buf->sample_rate = -1;
}

static void
gst_jack_ring_buffer_dispose (GObject * object)
{
  G_OBJECT_CLASS (ring_parent_class)->dispose (object);
}

static void
gst_jack_ring_buffer_finalize (GObject * object)
{
  GstJackRingBuffer *ringbuffer;

  ringbuffer = GST_JACK_RING_BUFFER_CAST (object);

  G_OBJECT_CLASS (ring_parent_class)->finalize (object);
}

/* the _open_device method should make a connection with the server
 */
static gboolean
gst_jack_ring_buffer_open_device (GstRingBuffer * buf)
{
  GstJackAudioSink *sink;
  jack_status_t status = 0;
  const gchar *name;

  sink = GST_JACK_AUDIO_SINK (GST_OBJECT_PARENT (buf));

  GST_DEBUG_OBJECT (sink, "open");

  name = g_get_application_name ();
  if (!name)
    name = "GStreamer";

  sink->client = gst_jack_audio_client_new (name, sink->server,
      GST_JACK_CLIENT_SINK,
      jack_shutdown_cb,
      jack_process_cb, jack_buffer_size_cb, jack_sample_rate_cb, buf, &status);
  if (sink->client == NULL)
    goto could_not_open;

  GST_DEBUG_OBJECT (sink, "opened");

  return TRUE;

  /* ERRORS */
could_not_open:
  {
    if (status & JackServerFailed) {
      GST_ELEMENT_ERROR (sink, RESOURCE, NOT_FOUND,
          (NULL), ("Cannot connect to the Jack server (status %d)", status));
    } else {
      GST_ELEMENT_ERROR (sink, RESOURCE, OPEN_WRITE,
          (NULL), ("Jack client open error (status %d)", status));
    }
    return FALSE;
  }
}

/* close the connection with the server
 */
static gboolean
gst_jack_ring_buffer_close_device (GstRingBuffer * buf)
{
  GstJackAudioSink *sink;

  sink = GST_JACK_AUDIO_SINK (GST_OBJECT_PARENT (buf));

  GST_DEBUG_OBJECT (sink, "close");

  gst_jack_audio_sink_free_channels (sink);
  gst_jack_audio_client_free (sink->client);
  sink->client = NULL;

  return TRUE;
}

/* allocate a buffer and setup resources to process the audio samples of
 * the format as specified in @spec.
 *
 * We allocate N jack ports, one for each channel. If we are asked to
 * automatically make a connection with physical ports, we connect as many
 * ports as there are physical ports, leaving leftover ports unconnected.
 *
 * It is assumed that samplerate and number of channels are acceptable since our
 * getcaps method will always provide correct values. If unacceptable caps are
 * received for some reason, we fail here.
 */
static gboolean
gst_jack_ring_buffer_acquire (GstRingBuffer * buf, GstRingBufferSpec * spec)
{
  GstJackAudioSink *sink;
  GstJackRingBuffer *abuf;
  const char **ports;
  gint sample_rate, buffer_size;
  gint i, channels, res;
  jack_client_t *client;

  sink = GST_JACK_AUDIO_SINK (GST_OBJECT_PARENT (buf));
  abuf = GST_JACK_RING_BUFFER_CAST (buf);

  GST_DEBUG_OBJECT (sink, "acquire");

  client = gst_jack_audio_client_get_client (sink->client);

  /* sample rate must be that of the server */
  sample_rate = jack_get_sample_rate (client);
  if (sample_rate != spec->rate)
    goto wrong_samplerate;

  channels = spec->channels;

  if (!gst_jack_audio_sink_allocate_channels (sink, channels))
    goto out_of_ports;

  buffer_size = jack_get_buffer_size (client);

  /* the segment size in bytes, this is large enough to hold a buffer of 32bit floats
   * for all channels  */
  spec->segsize = buffer_size * sizeof (gfloat) * channels;
  spec->latency_time = gst_util_uint64_scale (spec->segsize,
      (GST_SECOND / GST_USECOND), spec->rate * spec->bytes_per_sample);
  /* segtotal based on buffer-time latency */
  spec->segtotal = spec->buffer_time / spec->latency_time;

  GST_DEBUG_OBJECT (sink, "segsize %d, segtotal %d", spec->segsize,
      spec->segtotal);

  /* allocate the ringbuffer memory now */
  buf->data = gst_buffer_new_and_alloc (spec->segtotal * spec->segsize);
  memset (GST_BUFFER_DATA (buf->data), 0, GST_BUFFER_SIZE (buf->data));

  if ((res = gst_jack_audio_client_set_active (sink->client, TRUE)))
    goto could_not_activate;

  /* if we need to automatically connect the ports, do so now. We must do this
   * after activating the client. */
  if (sink->connect == GST_JACK_CONNECT_AUTO
      || sink->connect == GST_JACK_CONNECT_AUTO_FORCED) {
    /* find all the physical input ports. A physical input port is a port
     * associated with a hardware device. Someone needs connect to a physical
     * port in order to hear something. */
    ports = jack_get_ports (client, NULL, NULL,
        JackPortIsPhysical | JackPortIsInput);
    if (ports == NULL) {
      /* no ports? fine then we don't do anything except for posting a warning
       * message. */
      GST_ELEMENT_WARNING (sink, RESOURCE, NOT_FOUND, (NULL),
          ("No physical input ports found, leaving ports unconnected"));
      goto done;
    }

    for (i = 0; i < channels; i++) {
      /* stop when all input ports are exhausted */
      if (ports[i] == NULL) {
        /* post a warning that we could not connect all ports */
        GST_ELEMENT_WARNING (sink, RESOURCE, NOT_FOUND, (NULL),
            ("No more physical ports, leaving some ports unconnected"));
        break;
      }
      GST_DEBUG_OBJECT (sink, "try connecting to %s",
          jack_port_name (sink->ports[i]));
      /* connect the port to a physical port */
      res = jack_connect (client, jack_port_name (sink->ports[i]), ports[i]);
      if (res != 0 && res != EEXIST)
        goto cannot_connect;
    }
    free (ports);
  }
done:

  abuf->sample_rate = sample_rate;
  abuf->buffer_size = buffer_size;
  abuf->channels = spec->channels;

  return TRUE;

  /* ERRORS */
wrong_samplerate:
  {
    GST_ELEMENT_ERROR (sink, RESOURCE, SETTINGS, (NULL),
        ("Wrong samplerate, server is running at %d and we received %d",
            sample_rate, spec->rate));
    return FALSE;
  }
out_of_ports:
  {
    GST_ELEMENT_ERROR (sink, RESOURCE, SETTINGS, (NULL),
        ("Cannot allocate more Jack ports"));
    return FALSE;
  }
could_not_activate:
  {
    GST_ELEMENT_ERROR (sink, RESOURCE, SETTINGS, (NULL),
        ("Could not activate client (%d:%s)", res, g_strerror (res)));
    return FALSE;
  }
cannot_connect:
  {
    GST_ELEMENT_ERROR (sink, RESOURCE, SETTINGS, (NULL),
        ("Could not connect output ports to physical ports (%d:%s)",
            res, g_strerror (res)));
    free (ports);
    return FALSE;
  }
}

/* function is called with LOCK */
static gboolean
gst_jack_ring_buffer_release (GstRingBuffer * buf)
{
  GstJackAudioSink *sink;
  GstJackRingBuffer *abuf;
  gint res;

  abuf = GST_JACK_RING_BUFFER_CAST (buf);
  sink = GST_JACK_AUDIO_SINK (GST_OBJECT_PARENT (buf));

  GST_DEBUG_OBJECT (sink, "release");

  if ((res = gst_jack_audio_client_set_active (sink->client, FALSE))) {
    /* we only warn, this means the server is probably shut down and the client
     * is gone anyway. */
    GST_ELEMENT_WARNING (sink, RESOURCE, CLOSE, (NULL),
        ("Could not deactivate Jack client (%d)", res));
  }

  abuf->channels = -1;
  abuf->buffer_size = -1;
  abuf->sample_rate = -1;

  /* free the buffer */
  gst_buffer_unref (buf->data);
  buf->data = NULL;

  return TRUE;
}

static gboolean
gst_jack_ring_buffer_start (GstRingBuffer * buf)
{
  GstJackAudioSink *sink;

  sink = GST_JACK_AUDIO_SINK (GST_OBJECT_PARENT (buf));

  GST_DEBUG_OBJECT (sink, "start");

  return TRUE;
}

static gboolean
gst_jack_ring_buffer_pause (GstRingBuffer * buf)
{
  GstJackAudioSink *sink;

  sink = GST_JACK_AUDIO_SINK (GST_OBJECT_PARENT (buf));

  GST_DEBUG_OBJECT (sink, "pause");

  return TRUE;
}

static gboolean
gst_jack_ring_buffer_stop (GstRingBuffer * buf)
{
  GstJackAudioSink *sink;

  sink = GST_JACK_AUDIO_SINK (GST_OBJECT_PARENT (buf));

  GST_DEBUG_OBJECT (sink, "stop");

  return TRUE;
}

static guint
gst_jack_ring_buffer_delay (GstRingBuffer * buf)
{
  GstJackAudioSink *sink;
  guint i, res = 0, latency;
  jack_client_t *client;

  sink = GST_JACK_AUDIO_SINK (GST_OBJECT_PARENT (buf));
  client = gst_jack_audio_client_get_client (sink->client);

  for (i = 0; i < sink->port_count; i++) {
    latency = jack_port_get_total_latency (client, sink->ports[i]);
    if (latency > res)
      res = latency;
  }

  GST_LOG_OBJECT (sink, "delay %u", res);

  return res;
}

/* elementfactory information */
static const GstElementDetails gst_jack_audio_sink_details =
GST_ELEMENT_DETAILS ("Audio Sink (Jack)",
    "Sink/Audio",
    "Output to Jack",
    "Wim Taymans <wim@fluendo.com>");

static GstStaticPadTemplate jackaudiosink_sink_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw-float, "
        "endianness = (int) { " G_STRINGIFY (G_BYTE_ORDER) " }, "
        "width = (int) 32, "
        "rate = (int) [ 1, MAX ], " "channels = (int) [ 1, MAX ]")
    );

/* AudioSink signals and args */
enum
{
  /* FILL ME */
  SIGNAL_LAST
};

#define DEFAULT_PROP_CONNECT 	GST_JACK_CONNECT_AUTO
#define DEFAULT_PROP_SERVER 	NULL

enum
{
  PROP_0,
  PROP_CONNECT,
  PROP_SERVER,
  PROP_LAST
};

#define _do_init(bla) \
    GST_DEBUG_CATEGORY_INIT (gst_jack_audio_sink_debug, "jacksink", 0, "jacksink element");

GST_BOILERPLATE_FULL (GstJackAudioSink, gst_jack_audio_sink, GstBaseAudioSink,
    GST_TYPE_BASE_AUDIO_SINK, _do_init);

static void gst_jack_audio_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_jack_audio_sink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static GstCaps *gst_jack_audio_sink_getcaps (GstBaseSink * bsink);
static GstRingBuffer *gst_jack_audio_sink_create_ringbuffer (GstBaseAudioSink *
    sink);

static void
gst_jack_audio_sink_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (element_class, &gst_jack_audio_sink_details);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&jackaudiosink_sink_factory));
}

static void
gst_jack_audio_sink_class_init (GstJackAudioSinkClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSinkClass *gstbasesink_class;
  GstBaseAudioSinkClass *gstbaseaudiosink_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasesink_class = (GstBaseSinkClass *) klass;
  gstbaseaudiosink_class = (GstBaseAudioSinkClass *) klass;

  gobject_class->get_property =
      GST_DEBUG_FUNCPTR (gst_jack_audio_sink_get_property);
  gobject_class->set_property =
      GST_DEBUG_FUNCPTR (gst_jack_audio_sink_set_property);

  g_object_class_install_property (gobject_class, PROP_CONNECT,
      g_param_spec_enum ("connect", "Connect",
          "Specify how the output ports will be connected",
          GST_TYPE_JACK_CONNECT, DEFAULT_PROP_CONNECT, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_SERVER,
      g_param_spec_string ("server", "Server",
          "The Jack server to connect to (NULL = default)",
          DEFAULT_PROP_SERVER, G_PARAM_READWRITE));

  gstbasesink_class->get_caps = GST_DEBUG_FUNCPTR (gst_jack_audio_sink_getcaps);

  gstbaseaudiosink_class->create_ringbuffer =
      GST_DEBUG_FUNCPTR (gst_jack_audio_sink_create_ringbuffer);

  /* ref class from a thread-safe context to work around missing bit of
   * thread-safety in GObject */
  g_type_class_ref (GST_TYPE_JACK_RING_BUFFER);

  gst_jack_audio_client_init ();
}

static void
gst_jack_audio_sink_init (GstJackAudioSink * sink,
    GstJackAudioSinkClass * g_class)
{
  sink->connect = DEFAULT_PROP_CONNECT;
  sink->server = g_strdup (DEFAULT_PROP_SERVER);
  sink->ports = NULL;
  sink->port_count = 0;
}

static void
gst_jack_audio_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstJackAudioSink *sink;

  sink = GST_JACK_AUDIO_SINK (object);

  switch (prop_id) {
    case PROP_CONNECT:
      sink->connect = g_value_get_enum (value);
      break;
    case PROP_SERVER:
      g_free (sink->server);
      sink->server = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_jack_audio_sink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstJackAudioSink *sink;

  sink = GST_JACK_AUDIO_SINK (object);

  switch (prop_id) {
    case PROP_CONNECT:
      g_value_set_enum (value, sink->connect);
      break;
    case PROP_SERVER:
      g_value_set_string (value, sink->server);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstCaps *
gst_jack_audio_sink_getcaps (GstBaseSink * bsink)
{
  GstJackAudioSink *sink = GST_JACK_AUDIO_SINK (bsink);
  const char **ports;
  gint min, max;
  gint rate;
  jack_client_t *client;

  if (sink->client == NULL)
    goto no_client;

  client = gst_jack_audio_client_get_client (sink->client);

  if (sink->connect == GST_JACK_CONNECT_AUTO) {
    /* get a port count, this is the number of channels we can automatically
     * connect. */
    ports = jack_get_ports (client, NULL, NULL,
        JackPortIsPhysical | JackPortIsInput);
    max = 0;
    if (ports != NULL) {
      for (; ports[max]; max++);
      free (ports);
    } else
      max = 0;
  } else {
    /* we allow any number of pads, something else is going to connect the
     * pads. */
    max = G_MAXINT;
  }
  min = MIN (1, max);

  rate = jack_get_sample_rate (client);

  GST_DEBUG_OBJECT (sink, "got %d-%d ports, samplerate: %d", min, max, rate);

  if (!sink->caps) {
    sink->caps = gst_caps_new_simple ("audio/x-raw-float",
        "endianness", G_TYPE_INT, G_BYTE_ORDER,
        "width", G_TYPE_INT, 32,
        "rate", G_TYPE_INT, rate,
        "channels", GST_TYPE_INT_RANGE, min, max, NULL);
  }
  GST_INFO_OBJECT (sink, "returning caps %" GST_PTR_FORMAT, sink->caps);

  return gst_caps_ref (sink->caps);

  /* ERRORS */
no_client:
  {
    GST_DEBUG_OBJECT (sink, "device not open, using template caps");
    /* base class will get template caps for us when we return NULL */
    return NULL;
  }
}

static GstRingBuffer *
gst_jack_audio_sink_create_ringbuffer (GstBaseAudioSink * sink)
{
  GstRingBuffer *buffer;

  buffer = g_object_new (GST_TYPE_JACK_RING_BUFFER, NULL);
  GST_DEBUG_OBJECT (sink, "created ringbuffer @%p", buffer);

  return buffer;
}
