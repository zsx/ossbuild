/* GStreamer
 * Copyright (C) <2007> Wim Taymans <wim.taymans@gmail.com>
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
 * SECTION:element-playbin2
 *
 * Playbin2 provides a stand-alone everything-in-one abstraction for an
 * audio and/or video player.
 *
 * At this stage, playbin2 is considered UNSTABLE. The API provided in the
 * signals and properties may yet change in the near future. When playbin2
 * is stable, it will probably replace #playbin
 *
 * It can handle both audio and video files and features
 * <itemizedlist>
 * <listitem>
 * automatic file type recognition and based on that automatic
 * selection and usage of the right audio/video/subtitle demuxers/decoders
 * </listitem>
 * <listitem>
 * visualisations for audio files
 * </listitem>
 * <listitem>
 * subtitle support for video files. Subtitles can be store in external
 * files.
 * </listitem>
 * <listitem>
 * stream selection between different video/audio/subtitles streams
 * </listitem>
 * <listitem>
 * meta info (tag) extraction
 * </listitem>
 * <listitem>
 * easy access to the last video frame
 * </listitem>
 * <listitem>
 * buffering when playing streams over a network
 * </listitem>
 * <listitem>
 * volume control with mute option
 * </listitem>
 * </itemizedlist>
 *
 * <refsect2>
 * <title>Usage</title>
 * <para>
 * A playbin element can be created just like any other element using
 * gst_element_factory_make(). The file/URI to play should be set via the #GstPlayBin2:uri
 * property. This must be an absolute URI, relative file paths are not allowed.
 * Example URIs are file:///home/joe/movie.avi or http://www.joedoe.com/foo.ogg
 *
 * Playbin is a #GstPipeline. It will notify the application of everything
 * that's happening (errors, end of stream, tags found, state changes, etc.)
 * by posting messages on its #GstBus. The application needs to watch the
 * bus.
 *
 * Playback can be initiated by setting the element to PLAYING state using
 * gst_element_set_state(). Note that the state change will take place in
 * the background in a separate thread, when the function returns playback
 * is probably not happening yet and any errors might not have occured yet.
 * Applications using playbin should ideally be written to deal with things
 * completely asynchroneous.
 *
 * When playback has finished (an EOS message has been received on the bus)
 * or an error has occured (an ERROR message has been received on the bus) or
 * the user wants to play a different track, playbin should be set back to
 * READY or NULL state, then the #GstPlayBin2:uri property should be set to the
 * new location and then playbin be set to PLAYING state again.
 *
 * Seeking can be done using gst_element_seek_simple() or gst_element_seek()
 * on the playbin element. Again, the seek will not be executed
 * instantaneously, but will be done in a background thread. When the seek
 * call returns the seek will most likely still be in process. An application
 * may wait for the seek to finish (or fail) using gst_element_get_state() with
 * -1 as the timeout, but this will block the user interface and is not
 * recommended at all.
 *
 * Applications may query the current position and duration of the stream
 * via gst_element_query_position() and gst_element_query_duration() and
 * setting the format passed to GST_FORMAT_TIME. If the query was successful,
 * the duration or position will have been returned in units of nanoseconds.
 * </para>
 * </refsect2>
 * <refsect2>
 * <title>Advanced Usage: specifying the audio and video sink</title>
 * <para>
 * By default, if no audio sink or video sink has been specified via the
 * #GstPlayBin2:audio-sink or #GstPlayBin2:video-sink property, playbin will use the autoaudiosink
 * and autovideosink elements to find the first-best available output method.
 * This should work in most cases, but is not always desirable. Often either
 * the user or application might want to specify more explicitly what to use
 * for audio and video output.
 *
 * If the application wants more control over how audio or video should be
 * output, it may create the audio/video sink elements itself (for example
 * using gst_element_factory_make()) and provide them to playbin using the
 * #GstPlayBin2:audio-sink or #GstPlayBin2:video-sink property.
 *
 * GNOME-based applications, for example, will usually want to create
 * gconfaudiosink and gconfvideosink elements and make playbin use those,
 * so that output happens to whatever the user has configured in the GNOME
 * Multimedia System Selector configuration dialog.
 *
 * The sink elements do not necessarily need to be ready-made sinks. It is
 * possible to create container elements that look like a sink to playbin,
 * but in reality contain a number of custom elements linked together. This
 * can be achieved by creating a #GstBin and putting elements in there and
 * linking them, and then creating a sink #GstGhostPad for the bin and pointing
 * it to the sink pad of the first element within the bin. This can be used
 * for a number of purposes, for example to force output to a particular
 * format or to modify or observe the data before it is output.
 *
 * It is also possible to 'suppress' audio and/or video output by using
 * 'fakesink' elements (or capture it from there using the fakesink element's
 * "handoff" signal, which, nota bene, is fired from the streaming thread!).
 * </para>
 * </refsect2>
 * <refsect2>
 * <title>Retrieving Tags and Other Meta Data</title>
 * <para>
 * Most of the common meta data (artist, title, etc.) can be retrieved by
 * watching for TAG messages on the pipeline's bus (see above).
 *
 * Other more specific meta information like width/height/framerate of video
 * streams or samplerate/number of channels of audio streams can be obtained
 * from the negotiated caps on the sink pads of the sinks.
 * </para>
 * </refsect2>
 * <refsect2>
 * <title>Buffering</title>
 * Playbin handles buffering automatically for the most part, but applications
 * need to handle parts of the buffering process as well. Whenever playbin is
 * buffering, it will post BUFFERING messages on the bus with a percentage
 * value that shows the progress of the buffering process. Applications need
 * to set playbin to PLAYING or PAUSED state in response to these messages.
 * They may also want to convey the buffering progress to the user in some
 * way. Here is how to extract the percentage information from the message
 * (requires GStreamer >= 0.10.11):
 * |[
 * switch (GST_MESSAGE_TYPE (msg)) {
 *   case GST_MESSAGE_BUFFERING: {
 *     gint percent = 0;
 *     gst_message_parse_buffering (msg, &amp;percent);
 *     g_print ("Buffering (%%u percent done)", percent);
 *     break;
 *   }
 *   ...
 * }
 * ]|
 * Note that applications should keep/set the pipeline in the PAUSED state when
 * a BUFFERING message is received with a buffer percent value < 100 and set
 * the pipeline back to PLAYING state when a BUFFERING message with a value
 * of 100 percent is received (if PLAYING is the desired state, that is).
 * </refsect2>
 * <refsect2>
 * <title>Embedding the video window in your application</title>
 * By default, playbin (or rather the video sinks used) will create their own
 * window. Applications will usually want to force output to a window of their
 * own, however. This can be done using the #GstXOverlay interface, which most
 * video sinks implement. See the documentation there for more details.
 * </refsect2>
 * <refsect2>
 * <title>Specifying which CD/DVD device to use</title>
 * The device to use for CDs/DVDs needs to be set on the source element
 * playbin creates before it is opened. The only way to do this at the moment
 * is to connect to playbin's "notify::source" signal, which will be emitted
 * by playbin when it has created the source element for a particular URI.
 * In the signal callback you can check if the source element has a "device"
 * property and set it appropriately. In future ways might be added to specify
 * the device as part of the URI, but at the time of writing this is not
 * possible yet.
 * </refsect2>
 * <refsect2>
 * <title>Handling redirects</title>
 * <para>
 * Some elements may post 'redirect' messages on the bus to tell the
 * application to open another location. These are element messages containing
 * a structure named 'redirect' along with a 'new-location' field of string
 * type. The new location may be a relative or an absolute URI. Examples
 * for such redirects can be found in many quicktime movie trailers.
 * </para>
 * </refsect2>
 * <refsect2>
 * <title>Examples</title>
 * |[
 * gst-launch -v playbin uri=file:///path/to/somefile.avi
 * ]| This will play back the given AVI video file, given that the video and
 * audio decoders required to decode the content are installed. Since no
 * special audio sink or video sink is supplied (not possible via gst-launch),
 * playbin will try to find a suitable audio and video sink automatically
 * using the autoaudiosink and autovideosink elements.
 * |[
 * gst-launch -v playbin uri=cdda://4
 * ]| This will play back track 4 on an audio CD in your disc drive (assuming
 * the drive is detected automatically by the plugin).
 * |[
 * gst-launch -v playbin uri=dvd://1
 * ]| This will play back title 1 of a DVD in your disc drive (assuming
 * the drive is detected automatically by the plugin).
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <gst/gst.h>

#include <gst/gst-i18n-plugin.h>
#include <gst/pbutils/pbutils.h>
#include <gst/interfaces/streamvolume.h>

#include "gstplay-enum.h"
#include "gstplay-marshal.h"
#include "gstplaysink.h"
#include "gstfactorylists.h"
#include "gstinputselector.h"
#include "gstscreenshot.h"

GST_DEBUG_CATEGORY_STATIC (gst_play_bin_debug);
#define GST_CAT_DEFAULT gst_play_bin_debug

#define GST_TYPE_PLAY_BIN               (gst_play_bin_get_type())
#define GST_PLAY_BIN(obj)               (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_PLAY_BIN,GstPlayBin))
#define GST_PLAY_BIN_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_PLAY_BIN,GstPlayBinClass))
#define GST_IS_PLAY_BIN(obj)            (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_PLAY_BIN))
#define GST_IS_PLAY_BIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_PLAY_BIN))

#define VOLUME_MAX_DOUBLE 10.0

typedef struct _GstPlayBin GstPlayBin;
typedef struct _GstPlayBinClass GstPlayBinClass;
typedef struct _GstSourceGroup GstSourceGroup;
typedef struct _GstSourceSelect GstSourceSelect;

/* has the info for a selector and provides the link to the sink */
struct _GstSourceSelect
{
  const gchar *media_list[3];   /* the media types for the selector */
  GstPlaySinkType type;         /* the sink pad type of the selector */

  GstElement *selector;         /* the selector */
  GPtrArray *channels;
  GstPad *srcpad;               /* the source pad of the selector */
  GstPad *sinkpad;              /* the sinkpad of the sink when the selector is linked */
};

#define GST_SOURCE_GROUP_GET_LOCK(group) (((GstSourceGroup*)(group))->lock)
#define GST_SOURCE_GROUP_LOCK(group) (g_mutex_lock (GST_SOURCE_GROUP_GET_LOCK(group)))
#define GST_SOURCE_GROUP_UNLOCK(group) (g_mutex_unlock (GST_SOURCE_GROUP_GET_LOCK(group)))

/* a structure to hold the objects for decoding a uri and the subtitle uri
 */
struct _GstSourceGroup
{
  GstPlayBin *playbin;

  GMutex *lock;

  gboolean valid;               /* the group has valid info to start playback */
  gboolean active;              /* the group is active */

  /* properties */
  gchar *uri;
  gchar *suburi;
  GValueArray *streaminfo;
  GstElement *source;

  GPtrArray *video_channels;    /* links to selector pads */
  GPtrArray *audio_channels;    /* links to selector pads */
  GPtrArray *text_channels;     /* links to selector pads */
  GPtrArray *subp_channels;     /* links to selector pads */

  GstElement *audio_sink;       /* autoplugged audio and video sinks */
  GstElement *video_sink;

  /* uridecodebins for uri and subtitle uri */
  GstElement *uridecodebin;
  GstElement *suburidecodebin;
  gint pending;

  gulong pad_added_id;
  gulong pad_removed_id;
  gulong no_more_pads_id;
  gulong notify_source_id;
  gulong drained_id;
  gulong autoplug_factories_id;
  gulong autoplug_select_id;

  gulong sub_pad_added_id;
  gulong sub_pad_removed_id;
  gulong sub_no_more_pads_id;

  /* selectors for different streams */
  GstSourceSelect selector[GST_PLAY_SINK_TYPE_LAST];
};

#define GST_PLAY_BIN_GET_LOCK(bin) (((GstPlayBin*)(bin))->lock)
#define GST_PLAY_BIN_LOCK(bin) (g_mutex_lock (GST_PLAY_BIN_GET_LOCK(bin)))
#define GST_PLAY_BIN_UNLOCK(bin) (g_mutex_unlock (GST_PLAY_BIN_GET_LOCK(bin)))

/* lock to protect dynamic callbacks, like no-more-pads */
#define GST_PLAY_BIN_DYN_LOCK(bin)    g_mutex_lock ((bin)->dyn_lock)
#define GST_PLAY_BIN_DYN_UNLOCK(bin)  g_mutex_unlock ((bin)->dyn_lock)

/* lock for shutdown */
#define GST_PLAY_BIN_SHUTDOWN_LOCK(bin,label)           \
G_STMT_START {                                          \
  if (G_UNLIKELY (g_atomic_int_get (&bin->shutdown)))   \
    goto label;                                         \
  GST_PLAY_BIN_DYN_LOCK (bin);                          \
  if (G_UNLIKELY (g_atomic_int_get (&bin->shutdown))) { \
    GST_PLAY_BIN_DYN_UNLOCK (bin);                      \
    goto label;                                         \
  }                                                     \
} G_STMT_END

/* unlock for shutdown */
#define GST_PLAY_BIN_SHUTDOWN_UNLOCK(bin)         \
  GST_PLAY_BIN_DYN_UNLOCK (bin);                  \

/**
 * GstPlayBin2:
 *
 * playbin element structure
 */
struct _GstPlayBin
{
  GstPipeline parent;

  GMutex *lock;                 /* to protect group switching */

  /* the groups, we use a double buffer to switch between current and next */
  GstSourceGroup groups[2];     /* array with group info */
  GstSourceGroup *curr_group;   /* pointer to the currently playing group */
  GstSourceGroup *next_group;   /* pointer to the next group */

  /* properties */
  guint connection_speed;       /* connection speed in bits/sec (0 = unknown) */
  gint current_video;           /* the currently selected stream */
  gint current_audio;           /* the currently selected stream */
  gint current_text;            /* the currently selected stream */
  gchar *encoding;              /* subtitle encoding */

  guint64 buffer_duration;      /* When buffering, the max buffer duration (ns) */
  guint buffer_size;            /* When buffering, the max buffer size (bytes) */

  /* our play sink */
  GstPlaySink *playsink;

  /* the last activated source */
  GstElement *source;

  /* lock protecting dynamic adding/removing */
  GMutex *dyn_lock;
  /* if we are shutting down or not */
  gint shutdown;

  GValueArray *elements;        /* factories we can use for selecting elements */

  gboolean have_selector;       /* set to FALSE when we fail to create an
                                 * input-selector, so that we only post a
                                 * warning once */

  GstElement *audio_sink;       /* configured audio sink, or NULL      */
  GstElement *video_sink;       /* configured video sink, or NULL      */
  GstElement *subpic_sink;      /* configured subpicture sink, or NULL */
  GstElement *text_sink;        /* configured text sink, or NULL       */
};

struct _GstPlayBinClass
{
  GstPipelineClass parent_class;

  /* notify app that the current uri finished decoding and it is possible to
   * queue a new one for gapless playback */
  void (*about_to_finish) (GstPlayBin * playbin);

  /* notify app that number of audio/video/text streams changed */
  void (*video_changed) (GstPlayBin * playbin);
  void (*audio_changed) (GstPlayBin * playbin);
  void (*text_changed) (GstPlayBin * playbin);

  /* notify app that the tags of audio/video/text streams changed */
  void (*video_tags_changed) (GstPlayBin * playbin, gint stream);
  void (*audio_tags_changed) (GstPlayBin * playbin, gint stream);
  void (*text_tags_changed) (GstPlayBin * playbin, gint stream);

  /* get audio/video/text tags for a stream */
  GstTagList *(*get_video_tags) (GstPlayBin * playbin, gint stream);
  GstTagList *(*get_audio_tags) (GstPlayBin * playbin, gint stream);
  GstTagList *(*get_text_tags) (GstPlayBin * playbin, gint stream);

  /* get the last video frame and convert it to the given caps */
  GstBuffer *(*convert_frame) (GstPlayBin * playbin, GstCaps * caps);

  /* get audio/video/text pad for a stream */
  GstPad *(*get_video_pad) (GstPlayBin * playbin, gint stream);
  GstPad *(*get_audio_pad) (GstPlayBin * playbin, gint stream);
  GstPad *(*get_text_pad) (GstPlayBin * playbin, gint stream);
};

/* props */
#define DEFAULT_URI               NULL
#define DEFAULT_SUBURI            NULL
#define DEFAULT_SOURCE            NULL
#define DEFAULT_FLAGS             GST_PLAY_FLAG_AUDIO | GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_TEXT | \
                                  GST_PLAY_FLAG_SOFT_VOLUME
#define DEFAULT_N_VIDEO           0
#define DEFAULT_CURRENT_VIDEO     -1
#define DEFAULT_N_AUDIO           0
#define DEFAULT_CURRENT_AUDIO     -1
#define DEFAULT_N_TEXT            0
#define DEFAULT_CURRENT_TEXT      -1
#define DEFAULT_SUBTITLE_ENCODING NULL
#define DEFAULT_AUDIO_SINK        NULL
#define DEFAULT_VIDEO_SINK        NULL
#define DEFAULT_VIS_PLUGIN        NULL
#define DEFAULT_TEXT_SINK         NULL
#define DEFAULT_SUBPIC_SINK       NULL
#define DEFAULT_VOLUME            1.0
#define DEFAULT_MUTE              FALSE
#define DEFAULT_FRAME             NULL
#define DEFAULT_FONT_DESC         NULL
#define DEFAULT_CONNECTION_SPEED  0
#define DEFAULT_BUFFER_DURATION   -1
#define DEFAULT_BUFFER_SIZE       -1

enum
{
  PROP_0,
  PROP_URI,
  PROP_SUBURI,
  PROP_SOURCE,
  PROP_FLAGS,
  PROP_N_VIDEO,
  PROP_CURRENT_VIDEO,
  PROP_N_AUDIO,
  PROP_CURRENT_AUDIO,
  PROP_N_TEXT,
  PROP_CURRENT_TEXT,
  PROP_SUBTITLE_ENCODING,
  PROP_AUDIO_SINK,
  PROP_VIDEO_SINK,
  PROP_VIS_PLUGIN,
  PROP_TEXT_SINK,
  PROP_SUBPIC_SINK,
  PROP_VOLUME,
  PROP_MUTE,
  PROP_FRAME,
  PROP_FONT_DESC,
  PROP_CONNECTION_SPEED,
  PROP_BUFFER_SIZE,
  PROP_BUFFER_DURATION,
  PROP_LAST
};

/* signals */
enum
{
  SIGNAL_ABOUT_TO_FINISH,
  SIGNAL_CONVERT_FRAME,
  SIGNAL_VIDEO_CHANGED,
  SIGNAL_AUDIO_CHANGED,
  SIGNAL_TEXT_CHANGED,
  SIGNAL_VIDEO_TAGS_CHANGED,
  SIGNAL_AUDIO_TAGS_CHANGED,
  SIGNAL_TEXT_TAGS_CHANGED,
  SIGNAL_GET_VIDEO_TAGS,
  SIGNAL_GET_AUDIO_TAGS,
  SIGNAL_GET_TEXT_TAGS,
  SIGNAL_GET_VIDEO_PAD,
  SIGNAL_GET_AUDIO_PAD,
  SIGNAL_GET_TEXT_PAD,
  LAST_SIGNAL
};

static void gst_play_bin_class_init (GstPlayBinClass * klass);
static void gst_play_bin_init (GstPlayBin * playbin);
static void gst_play_bin_finalize (GObject * object);

static void gst_play_bin_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * spec);
static void gst_play_bin_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * spec);

static GstStateChangeReturn gst_play_bin_change_state (GstElement * element,
    GstStateChange transition);

static void gst_play_bin_handle_message (GstBin * bin, GstMessage * message);

static GstTagList *gst_play_bin_get_video_tags (GstPlayBin * playbin,
    gint stream);
static GstTagList *gst_play_bin_get_audio_tags (GstPlayBin * playbin,
    gint stream);
static GstTagList *gst_play_bin_get_text_tags (GstPlayBin * playbin,
    gint stream);

static GstBuffer *gst_play_bin_convert_frame (GstPlayBin * playbin,
    GstCaps * caps);

static GstPad *gst_play_bin_get_video_pad (GstPlayBin * playbin, gint stream);
static GstPad *gst_play_bin_get_audio_pad (GstPlayBin * playbin, gint stream);
static GstPad *gst_play_bin_get_text_pad (GstPlayBin * playbin, gint stream);

static gboolean setup_next_source (GstPlayBin * playbin, GstState target);

static GstElementClass *parent_class;

static guint gst_play_bin_signals[LAST_SIGNAL] = { 0 };

static const GstElementDetails gst_play_bin_details =
GST_ELEMENT_DETAILS ("Player Bin 2",
    "Generic/Bin/Player",
    "Autoplug and play media from an uri",
    "Wim Taymans <wim.taymans@gmail.com>");

static void
gst_play_marshal_BUFFER__BOXED (GClosure * closure,
    GValue * return_value G_GNUC_UNUSED,
    guint n_param_values,
    const GValue * param_values,
    gpointer invocation_hint G_GNUC_UNUSED, gpointer marshal_data)
{
  typedef GstBuffer *(*GMarshalFunc_OBJECT__BOXED) (gpointer data1,
      gpointer arg_1, gpointer data2);
  register GMarshalFunc_OBJECT__BOXED callback;
  register GCClosure *cc = (GCClosure *) closure;
  register gpointer data1, data2;
  GstBuffer *v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 2);

  if (G_CCLOSURE_SWAP_DATA (closure)) {
    data1 = closure->data;
    data2 = g_value_peek_pointer (param_values + 0);
  } else {
    data1 = g_value_peek_pointer (param_values + 0);
    data2 = closure->data;
  }
  callback =
      (GMarshalFunc_OBJECT__BOXED) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1, g_value_get_boxed (param_values + 1), data2);

  gst_value_take_buffer (return_value, v_return);
}

static GType
gst_play_bin_get_type (void)
{
  static GType gst_play_bin_type = 0;

  if (!gst_play_bin_type) {
    static const GTypeInfo gst_play_bin_info = {
      sizeof (GstPlayBinClass),
      NULL,
      NULL,
      (GClassInitFunc) gst_play_bin_class_init,
      NULL,
      NULL,
      sizeof (GstPlayBin),
      0,
      (GInstanceInitFunc) gst_play_bin_init,
      NULL
    };
    static const GInterfaceInfo svol_info = {
      NULL, NULL, NULL
    };

    gst_play_bin_type = g_type_register_static (GST_TYPE_PIPELINE,
        "GstPlayBin2", &gst_play_bin_info, 0);

    g_type_add_interface_static (gst_play_bin_type, GST_TYPE_STREAM_VOLUME,
        &svol_info);
  }

  return gst_play_bin_type;
}

static void
gst_play_bin_class_init (GstPlayBinClass * klass)
{
  GObjectClass *gobject_klass;
  GstElementClass *gstelement_klass;
  GstBinClass *gstbin_klass;

  gobject_klass = (GObjectClass *) klass;
  gstelement_klass = (GstElementClass *) klass;
  gstbin_klass = (GstBinClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_klass->set_property = gst_play_bin_set_property;
  gobject_klass->get_property = gst_play_bin_get_property;

  gobject_klass->finalize = GST_DEBUG_FUNCPTR (gst_play_bin_finalize);

  /**
   * GstPlayBin2:uri
   *
   * Set the next URI that playbin will play. This property can be set from the
   * about-to-finish signal to queue the next media file.
   */
  g_object_class_install_property (gobject_klass, PROP_URI,
      g_param_spec_string ("uri", "URI", "URI of the media to play",
          NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstPlayBin2:suburi
   *
   * Set the next subtitle URI that playbin will play. This property can be
   * set from the about-to-finish signal to queue the next subtitle media file.
   */
  g_object_class_install_property (gobject_klass, PROP_SUBURI,
      g_param_spec_string ("suburi", ".sub-URI", "Optional URI of a subtitle",
          NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_klass, PROP_SOURCE,
      g_param_spec_object ("source", "Source", "Source element",
          GST_TYPE_ELEMENT, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  /**
   * GstPlayBin2:flags
   *
   * Control the behaviour of playbin.
   */
  g_object_class_install_property (gobject_klass, PROP_FLAGS,
      g_param_spec_flags ("flags", "Flags", "Flags to control behaviour",
          GST_TYPE_PLAY_FLAGS, DEFAULT_FLAGS,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstPlayBin2:n-video
   *
   * Get the total number of available video streams.
   */
  g_object_class_install_property (gobject_klass, PROP_N_VIDEO,
      g_param_spec_int ("n-video", "Number Video",
          "Total number of video streams", 0, G_MAXINT, 0,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  /**
   * GstPlayBin2:current-video
   *
   * Get or set the currently playing video stream. By default the first video
   * stream with data is played.
   */
  g_object_class_install_property (gobject_klass, PROP_CURRENT_VIDEO,
      g_param_spec_int ("current-video", "Current Video",
          "Currently playing video stream (-1 = auto)",
          -1, G_MAXINT, -1, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  /**
   * GstPlayBin2:n-audio
   *
   * Get the total number of available audio streams.
   */
  g_object_class_install_property (gobject_klass, PROP_N_AUDIO,
      g_param_spec_int ("n-audio", "Number Audio",
          "Total number of audio streams", 0, G_MAXINT, 0,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  /**
   * GstPlayBin2:current-audio
   *
   * Get or set the currently playing audio stream. By default the first audio
   * stream with data is played.
   */
  g_object_class_install_property (gobject_klass, PROP_CURRENT_AUDIO,
      g_param_spec_int ("current-audio", "Current audio",
          "Currently playing audio stream (-1 = auto)",
          -1, G_MAXINT, -1, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  /**
   * GstPlayBin2:n-text
   *
   * Get the total number of available subtitle streams.
   */
  g_object_class_install_property (gobject_klass, PROP_N_TEXT,
      g_param_spec_int ("n-text", "Number Text",
          "Total number of text streams", 0, G_MAXINT, 0,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  /**
   * GstPlayBin2:current-text:
   *
   * Get or set the currently playing subtitle stream. By default the first
   * subtitle stream with data is played.
   */
  g_object_class_install_property (gobject_klass, PROP_CURRENT_TEXT,
      g_param_spec_int ("current-text", "Current Text",
          "Currently playing text stream (-1 = auto)",
          -1, G_MAXINT, -1, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_klass, PROP_SUBTITLE_ENCODING,
      g_param_spec_string ("subtitle-encoding", "subtitle encoding",
          "Encoding to assume if input subtitles are not in UTF-8 encoding. "
          "If not set, the GST_SUBTITLE_ENCODING environment variable will "
          "be checked for an encoding to use. If that is not set either, "
          "ISO-8859-15 will be assumed.", NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_klass, PROP_VIDEO_SINK,
      g_param_spec_object ("video-sink", "Video Sink",
          "the video output element to use (NULL = default sink)",
          GST_TYPE_ELEMENT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_klass, PROP_AUDIO_SINK,
      g_param_spec_object ("audio-sink", "Audio Sink",
          "the audio output element to use (NULL = default sink)",
          GST_TYPE_ELEMENT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_klass, PROP_VIS_PLUGIN,
      g_param_spec_object ("vis-plugin", "Vis plugin",
          "the visualization element to use (NULL = default)",
          GST_TYPE_ELEMENT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_klass, PROP_TEXT_SINK,
      g_param_spec_object ("text-sink", "Text plugin",
          "the text output element to use (NULL = default textoverlay)",
          GST_TYPE_ELEMENT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_klass, PROP_SUBPIC_SINK,
      g_param_spec_object ("subpic-sink", "Subpicture plugin",
          "the subpicture output element to use (NULL = default dvdspu)",
          GST_TYPE_ELEMENT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstPlayBin2:volume:
   *
   * Get or set the current audio stream volume. 1.0 means 100%,
   * 0.0 means mute. This uses a linear volume scale.
   *
   */
  g_object_class_install_property (gobject_klass, PROP_VOLUME,
      g_param_spec_double ("volume", "Volume", "The audio volume, 1.0=100%",
          0.0, VOLUME_MAX_DOUBLE, 1.0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_klass, PROP_MUTE,
      g_param_spec_boolean ("mute", "Mute",
          "Mute the audio channel without changing the volume", FALSE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstPlayBin2:frame:
   * @playbin: a #GstPlayBin2
   *
   * Get the currently rendered or prerolled frame in the sink.
   * The #GstCaps on the buffer will describe the format of the buffer.
   */
  g_object_class_install_property (gobject_klass, PROP_FRAME,
      gst_param_spec_mini_object ("frame", "Frame",
          "The last frame (NULL = no video available)",
          GST_TYPE_BUFFER, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_klass, PROP_FONT_DESC,
      g_param_spec_string ("subtitle-font-desc",
          "Subtitle font description",
          "Pango font description of font "
          "to be used for subtitle rendering", NULL,
          G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_klass, PROP_CONNECTION_SPEED,
      g_param_spec_uint ("connection-speed", "Connection Speed",
          "Network connection speed in kbps (0 = unknown)",
          0, G_MAXUINT / 1000, DEFAULT_CONNECTION_SPEED,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_klass, PROP_BUFFER_SIZE,
      g_param_spec_int ("buffer-size", "Buffer size (bytes)",
          "Buffer size when buffering network streams",
          -1, G_MAXINT, DEFAULT_BUFFER_SIZE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_klass, PROP_BUFFER_DURATION,
      g_param_spec_int64 ("buffer-duration", "Buffer duration (ns)",
          "Buffer duration when buffering network streams",
          -1, G_MAXINT64, DEFAULT_BUFFER_DURATION,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * GstPlayBin2::about-to-finish
   * @playbin: a #GstPlayBin2
   *
   * This signal is emitted when the current uri is about to finish. You can
   * set the uri and suburi to make sure that playback continues.
   */
  gst_play_bin_signals[SIGNAL_ABOUT_TO_FINISH] =
      g_signal_new ("about-to-finish", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstPlayBinClass, about_to_finish), NULL, NULL,
      gst_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

  /**
   * GstPlayBin2::video-changed
   * @playbin: a #GstPlayBin2
   *
   * This signal is emitted whenever the number or order of the video
   * streams has changed. The application will most likely want to select
   * a new video stream.
   */
  gst_play_bin_signals[SIGNAL_VIDEO_CHANGED] =
      g_signal_new ("video-changed", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstPlayBinClass, video_changed), NULL, NULL,
      gst_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);
  /**
   * GstPlayBin2::audio-changed
   * @playbin: a #GstPlayBin2
   *
   * This signal is emitted whenever the number or order of the audio
   * streams has changed. The application will most likely want to select
   * a new audio stream.
   */
  gst_play_bin_signals[SIGNAL_AUDIO_CHANGED] =
      g_signal_new ("audio-changed", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstPlayBinClass, audio_changed), NULL, NULL,
      gst_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);
  /**
   * GstPlayBin2::text-changed
   * @playbin: a #GstPlayBin2
   *
   * This signal is emitted whenever the number or order of the text
   * streams has changed. The application will most likely want to select
   * a new text stream.
   */
  gst_play_bin_signals[SIGNAL_TEXT_CHANGED] =
      g_signal_new ("text-changed", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstPlayBinClass, text_changed), NULL, NULL,
      gst_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

  /**
   * GstPlayBin2::video-tags-changed
   * @playbin: a #GstPlayBin2
   * @stream: stream index with changed tags
   *
   * This signal is emitted whenever the tags of a video stream have changed.
   * The application will most likely want to get the new tags.
   *
   * Since: 0.10.24
   */
  gst_play_bin_signals[SIGNAL_VIDEO_TAGS_CHANGED] =
      g_signal_new ("video-tags-changed", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstPlayBinClass, video_tags_changed), NULL, NULL,
      gst_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

  /**
   * GstPlayBin2::audio-tags-changed
   * @playbin: a #GstPlayBin2
   * @stream: stream index with changed tags
   *
   * This signal is emitted whenever the tags of an audio stream have changed.
   * The application will most likely want to get the new tags.
   *
   * Since: 0.10.24
   */
  gst_play_bin_signals[SIGNAL_AUDIO_TAGS_CHANGED] =
      g_signal_new ("audio-tags-changed", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstPlayBinClass, audio_tags_changed), NULL, NULL,
      gst_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

  /**
   * GstPlayBin2::text-tags-changed
   * @playbin: a #GstPlayBin2
   * @stream: stream index with changed tags
   *
   * This signal is emitted whenever the tags of a text stream have changed.
   * The application will most likely want to get the new tags.
   *
   * Since: 0.10.24
   */
  gst_play_bin_signals[SIGNAL_TEXT_TAGS_CHANGED] =
      g_signal_new ("text-tags-changed", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstPlayBinClass, text_tags_changed), NULL, NULL,
      gst_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);

  /**
   * GstPlayBin2::get-video-tags
   * @playbin: a #GstPlayBin2
   * @stream: a video stream number
   *
   * Action signal to retrieve the tags of a specific video stream number.
   * This information can be used to select a stream.
   *
   * Returns: a GstTagList with tags or NULL when the stream number does not
   * exist.
   */
  gst_play_bin_signals[SIGNAL_GET_VIDEO_TAGS] =
      g_signal_new ("get-video-tags", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
      G_STRUCT_OFFSET (GstPlayBinClass, get_video_tags), NULL, NULL,
      gst_play_marshal_BOXED__INT, GST_TYPE_TAG_LIST, 1, G_TYPE_INT);
  /**
   * GstPlayBin2::get-audio-tags
   * @playbin: a #GstPlayBin2
   * @stream: an audio stream number
   *
   * Action signal to retrieve the tags of a specific audio stream number.
   * This information can be used to select a stream.
   *
   * Returns: a GstTagList with tags or NULL when the stream number does not
   * exist.
   */
  gst_play_bin_signals[SIGNAL_GET_AUDIO_TAGS] =
      g_signal_new ("get-audio-tags", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
      G_STRUCT_OFFSET (GstPlayBinClass, get_audio_tags), NULL, NULL,
      gst_play_marshal_BOXED__INT, GST_TYPE_TAG_LIST, 1, G_TYPE_INT);
  /**
   * GstPlayBin2::get-text-tags
   * @playbin: a #GstPlayBin2
   * @stream: a text stream number
   *
   * Action signal to retrieve the tags of a specific text stream number.
   * This information can be used to select a stream.
   *
   * Returns: a GstTagList with tags or NULL when the stream number does not
   * exist.
   */
  gst_play_bin_signals[SIGNAL_GET_TEXT_TAGS] =
      g_signal_new ("get-text-tags", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
      G_STRUCT_OFFSET (GstPlayBinClass, get_text_tags), NULL, NULL,
      gst_play_marshal_BOXED__INT, GST_TYPE_TAG_LIST, 1, G_TYPE_INT);
  /**
   * GstPlayBin2::convert-frame
   * @playbin: a #GstPlayBin2
   * @caps: the target format of the frame
   *
   * Action signal to retrieve the currently playing video frame in the format
   * specified by @caps.
   * If @caps is %NULL, no conversion will be performed and this function is
   * equivalent to the #GstPlayBin::frame property.
   *
   * Returns: a #GstBuffer of the current video frame converted to #caps.
   * The caps on the buffer will describe the final layout of the buffer data.
   * %NULL is returned when no current buffer can be retrieved or when the
   * conversion failed.
   */
  gst_play_bin_signals[SIGNAL_CONVERT_FRAME] =
      g_signal_new ("convert-frame", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
      G_STRUCT_OFFSET (GstPlayBinClass, convert_frame), NULL, NULL,
      gst_play_marshal_BUFFER__BOXED, GST_TYPE_BUFFER, 1, GST_TYPE_CAPS);

  /**
   * GstPlayBin2::get-video-pad
   * @playbin: a #GstPlayBin2
   * @stream: a video stream number
   *
   * Action signal to retrieve the stream-selector sinkpad for a specific
   * video stream.
   * This pad can be used for notifications of caps changes, stream-specific
   * queries, etc.
   *
   * Returns: a #GstPad, or NULL when the stream number does not exist.
   */
  gst_play_bin_signals[SIGNAL_GET_VIDEO_PAD] =
      g_signal_new ("get-video-pad", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
      G_STRUCT_OFFSET (GstPlayBinClass, get_video_pad), NULL, NULL,
      gst_play_marshal_OBJECT__INT, GST_TYPE_PAD, 1, G_TYPE_INT);
  /**
   * GstPlayBin2::get-audio-pad
   * @playbin: a #GstPlayBin2
   * @stream: an audio stream number
   *
   * Action signal to retrieve the stream-selector sinkpad for a specific
   * audio stream.
   * This pad can be used for notifications of caps changes, stream-specific
   * queries, etc.
   *
   * Returns: a #GstPad, or NULL when the stream number does not exist.
   */
  gst_play_bin_signals[SIGNAL_GET_AUDIO_PAD] =
      g_signal_new ("get-audio-pad", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
      G_STRUCT_OFFSET (GstPlayBinClass, get_audio_pad), NULL, NULL,
      gst_play_marshal_OBJECT__INT, GST_TYPE_PAD, 1, G_TYPE_INT);
  /**
   * GstPlayBin2::get-text-pad
   * @playbin: a #GstPlayBin2
   * @stream: a text stream number
   *
   * Action signal to retrieve the stream-selector sinkpad for a specific
   * text stream.
   * This pad can be used for notifications of caps changes, stream-specific
   * queries, etc.
   *
   * Returns: a #GstPad, or NULL when the stream number does not exist.
   */
  gst_play_bin_signals[SIGNAL_GET_TEXT_PAD] =
      g_signal_new ("get-text-pad", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
      G_STRUCT_OFFSET (GstPlayBinClass, get_text_pad), NULL, NULL,
      gst_play_marshal_OBJECT__INT, GST_TYPE_PAD, 1, G_TYPE_INT);

  klass->get_video_tags = gst_play_bin_get_video_tags;
  klass->get_audio_tags = gst_play_bin_get_audio_tags;
  klass->get_text_tags = gst_play_bin_get_text_tags;

  klass->convert_frame = gst_play_bin_convert_frame;

  klass->get_video_pad = gst_play_bin_get_video_pad;
  klass->get_audio_pad = gst_play_bin_get_audio_pad;
  klass->get_text_pad = gst_play_bin_get_text_pad;

  gst_element_class_set_details (gstelement_klass, &gst_play_bin_details);

  gstelement_klass->change_state =
      GST_DEBUG_FUNCPTR (gst_play_bin_change_state);

  gstbin_klass->handle_message =
      GST_DEBUG_FUNCPTR (gst_play_bin_handle_message);
}

static void
init_group (GstPlayBin * playbin, GstSourceGroup * group)
{
  /* store the array for the different channels */
  group->video_channels = g_ptr_array_new ();
  group->audio_channels = g_ptr_array_new ();
  group->text_channels = g_ptr_array_new ();
  group->subp_channels = g_ptr_array_new ();
  group->lock = g_mutex_new ();
  /* init selectors. The selector is found by finding the first prefix that
   * matches the media. */
  group->playbin = playbin;
  /* If you add any items to these lists, check that media_list[] is defined
   * above to be large enough to hold MAX(items)+1, so as to accomodate a
   * NULL terminator (set when the memory is zeroed on allocation) */
  group->selector[0].media_list[0] = "audio/x-raw-";
  group->selector[0].type = GST_PLAY_SINK_TYPE_AUDIO_RAW;
  group->selector[0].channels = group->audio_channels;
  group->selector[1].media_list[0] = "audio/";
  group->selector[1].type = GST_PLAY_SINK_TYPE_AUDIO;
  group->selector[1].channels = group->audio_channels;
  group->selector[2].media_list[0] = "video/x-raw-";
  group->selector[2].type = GST_PLAY_SINK_TYPE_VIDEO_RAW;
  group->selector[2].channels = group->video_channels;
  group->selector[3].media_list[0] = "video/x-dvd-subpicture";
  group->selector[3].media_list[1] = "subpicture/x-pgs";
  group->selector[3].type = GST_PLAY_SINK_TYPE_SUBPIC;
  group->selector[3].channels = group->subp_channels;
  group->selector[4].media_list[0] = "video/";
  group->selector[4].type = GST_PLAY_SINK_TYPE_VIDEO;
  group->selector[4].channels = group->video_channels;
  group->selector[5].media_list[0] = "text/";
  group->selector[5].type = GST_PLAY_SINK_TYPE_TEXT;
  group->selector[5].channels = group->text_channels;
}

static void
free_group (GstPlayBin * playbin, GstSourceGroup * group)
{
  g_free (group->uri);
  g_ptr_array_free (group->video_channels, TRUE);
  g_ptr_array_free (group->audio_channels, TRUE);
  g_ptr_array_free (group->text_channels, TRUE);
  g_ptr_array_free (group->subp_channels, TRUE);
  g_mutex_free (group->lock);
  if (group->audio_sink)
    gst_object_unref (group->audio_sink);
  group->audio_sink = NULL;
  if (group->video_sink)
    gst_object_unref (group->video_sink);
  group->video_sink = NULL;
}

static void
gst_play_bin_init (GstPlayBin * playbin)
{
  GstFactoryListType type;

  playbin->lock = g_mutex_new ();
  playbin->dyn_lock = g_mutex_new ();

  /* assume we can create a selector */
  playbin->have_selector = TRUE;

  /* init groups */
  playbin->curr_group = &playbin->groups[0];
  playbin->next_group = &playbin->groups[1];
  init_group (playbin, &playbin->groups[0]);
  init_group (playbin, &playbin->groups[1]);

  /* first filter out the interesting element factories */
  type = GST_FACTORY_LIST_DECODER | GST_FACTORY_LIST_SINK;
  playbin->elements = gst_factory_list_get_elements (type);
  gst_factory_list_debug (playbin->elements);

  /* add sink */
  playbin->playsink = g_object_new (GST_TYPE_PLAY_SINK, NULL);
  gst_bin_add (GST_BIN_CAST (playbin), GST_ELEMENT_CAST (playbin->playsink));
  gst_play_sink_set_flags (playbin->playsink, DEFAULT_FLAGS);

  playbin->encoding = g_strdup (DEFAULT_SUBTITLE_ENCODING);

  playbin->current_video = DEFAULT_CURRENT_VIDEO;
  playbin->current_audio = DEFAULT_CURRENT_AUDIO;
  playbin->current_text = DEFAULT_CURRENT_TEXT;

  playbin->buffer_duration = DEFAULT_BUFFER_DURATION;
  playbin->buffer_size = DEFAULT_BUFFER_SIZE;
}

static void
gst_play_bin_finalize (GObject * object)
{
  GstPlayBin *playbin;

  playbin = GST_PLAY_BIN (object);

  free_group (playbin, &playbin->groups[0]);
  free_group (playbin, &playbin->groups[1]);

  if (playbin->source)
    gst_object_unref (playbin->source);
  if (playbin->video_sink)
    gst_object_unref (playbin->video_sink);
  if (playbin->audio_sink)
    gst_object_unref (playbin->audio_sink);
  if (playbin->text_sink)
    gst_object_unref (playbin->text_sink);
  if (playbin->subpic_sink)
    gst_object_unref (playbin->subpic_sink);

  g_value_array_free (playbin->elements);
  g_free (playbin->encoding);
  g_mutex_free (playbin->lock);
  g_mutex_free (playbin->dyn_lock);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_play_bin_set_uri (GstPlayBin * playbin, const gchar * uri)
{
  GstSourceGroup *group;

  if (uri == NULL) {
    g_warning ("cannot set NULL uri");
    return;
  }

  GST_PLAY_BIN_LOCK (playbin);
  group = playbin->next_group;

  GST_SOURCE_GROUP_LOCK (group);
  /* store the uri in the next group we will play */
  g_free (group->uri);
  group->uri = g_strdup (uri);
  group->valid = TRUE;
  GST_SOURCE_GROUP_UNLOCK (group);

  GST_DEBUG ("set new uri to %s", uri);
  GST_PLAY_BIN_UNLOCK (playbin);
}

static void
gst_play_bin_set_suburi (GstPlayBin * playbin, const gchar * suburi)
{
  GstSourceGroup *group;

  GST_PLAY_BIN_LOCK (playbin);
  group = playbin->next_group;

  GST_SOURCE_GROUP_LOCK (group);
  g_free (group->suburi);
  group->suburi = g_strdup (suburi);
  GST_SOURCE_GROUP_UNLOCK (group);

  GST_DEBUG ("setting new .sub uri to %s", suburi);

  GST_PLAY_BIN_UNLOCK (playbin);
}

static void
gst_play_bin_set_flags (GstPlayBin * playbin, GstPlayFlags flags)
{
  gst_play_sink_set_flags (playbin->playsink, flags);
  gst_play_sink_reconfigure (playbin->playsink);
}

static GstPlayFlags
gst_play_bin_get_flags (GstPlayBin * playbin)
{
  GstPlayFlags flags;

  flags = gst_play_sink_get_flags (playbin->playsink);

  return flags;
}

/* get the currently playing group or if nothing is playing, the next
 * group. Must be called with the PLAY_BIN_LOCK. */
static GstSourceGroup *
get_group (GstPlayBin * playbin)
{
  GstSourceGroup *result;

  if (!(result = playbin->curr_group))
    result = playbin->next_group;

  return result;
}

static GstPad *
gst_play_bin_get_video_pad (GstPlayBin * playbin, gint stream)
{
  GstPad *sinkpad = NULL;
  GstSourceGroup *group;

  GST_PLAY_BIN_LOCK (playbin);
  group = get_group (playbin);
  if (stream < group->video_channels->len) {
    sinkpad = g_ptr_array_index (group->video_channels, stream);
    gst_object_ref (sinkpad);
  }
  GST_PLAY_BIN_UNLOCK (playbin);

  return sinkpad;
}

static GstPad *
gst_play_bin_get_audio_pad (GstPlayBin * playbin, gint stream)
{
  GstPad *sinkpad = NULL;
  GstSourceGroup *group;

  GST_PLAY_BIN_LOCK (playbin);
  group = get_group (playbin);
  if (stream < group->audio_channels->len) {
    sinkpad = g_ptr_array_index (group->audio_channels, stream);
    gst_object_ref (sinkpad);
  }
  GST_PLAY_BIN_UNLOCK (playbin);

  return sinkpad;
}

static GstPad *
gst_play_bin_get_text_pad (GstPlayBin * playbin, gint stream)
{
  GstPad *sinkpad = NULL;
  GstSourceGroup *group;

  GST_PLAY_BIN_LOCK (playbin);
  group = get_group (playbin);
  if (stream < group->text_channels->len) {
    sinkpad = g_ptr_array_index (group->text_channels, stream);
    gst_object_ref (sinkpad);
  }
  GST_PLAY_BIN_UNLOCK (playbin);

  return sinkpad;
}


static GstTagList *
get_tags (GstPlayBin * playbin, GPtrArray * channels, gint stream)
{
  GstTagList *result;
  GstPad *sinkpad;

  if (!channels || stream >= channels->len)
    return NULL;

  sinkpad = g_ptr_array_index (channels, stream);
  g_object_get (sinkpad, "tags", &result, NULL);

  return result;
}

static GstTagList *
gst_play_bin_get_video_tags (GstPlayBin * playbin, gint stream)
{
  GstTagList *result;
  GstSourceGroup *group;

  GST_PLAY_BIN_LOCK (playbin);
  group = get_group (playbin);
  result = get_tags (playbin, group->video_channels, stream);
  GST_PLAY_BIN_UNLOCK (playbin);

  return result;
}

static GstTagList *
gst_play_bin_get_audio_tags (GstPlayBin * playbin, gint stream)
{
  GstTagList *result;
  GstSourceGroup *group;

  GST_PLAY_BIN_LOCK (playbin);
  group = get_group (playbin);
  result = get_tags (playbin, group->audio_channels, stream);
  GST_PLAY_BIN_UNLOCK (playbin);

  return result;
}

static GstTagList *
gst_play_bin_get_text_tags (GstPlayBin * playbin, gint stream)
{
  GstTagList *result;
  GstSourceGroup *group;

  GST_PLAY_BIN_LOCK (playbin);
  group = get_group (playbin);
  result = get_tags (playbin, group->text_channels, stream);
  GST_PLAY_BIN_UNLOCK (playbin);

  return result;
}

static GstBuffer *
gst_play_bin_convert_frame (GstPlayBin * playbin, GstCaps * caps)
{
  GstBuffer *result;

  result = gst_play_sink_get_last_frame (playbin->playsink);
  if (result != NULL && caps != NULL) {
    GstBuffer *temp;

    temp = gst_play_frame_conv_convert (result, caps);
    gst_buffer_unref (result);
    result = temp;
  }
  return result;
}

/* Returns current stream number, or -1 if none has been selected yet */
static int
get_current_stream_number (GstPlayBin * playbin, GPtrArray * channels)
{
  /* Internal API cleanup would make this easier... */
  int i;
  GstPad *pad, *current;
  GstObject *selector = NULL;
  int ret = -1;

  for (i = 0; i < channels->len; i++) {
    pad = g_ptr_array_index (channels, i);
    if ((selector = gst_pad_get_parent (pad))) {
      g_object_get (selector, "active-pad", &current, NULL);
      gst_object_unref (selector);

      if (pad == current) {
        gst_object_unref (current);
        ret = i;
        break;
      }

      if (current)
        gst_object_unref (current);
    }
  }

  return ret;
}

static gboolean
gst_play_bin_set_current_video_stream (GstPlayBin * playbin, gint stream)
{
  GstSourceGroup *group;
  GPtrArray *channels;
  GstPad *sinkpad;

  GST_PLAY_BIN_LOCK (playbin);
  group = get_group (playbin);
  if (!(channels = group->video_channels))
    goto no_channels;

  if (stream == -1 || channels->len <= stream) {
    sinkpad = NULL;
  } else {
    /* take channel from selected stream */
    sinkpad = g_ptr_array_index (channels, stream);
  }

  if (sinkpad)
    gst_object_ref (sinkpad);
  GST_PLAY_BIN_UNLOCK (playbin);

  if (sinkpad) {
    GstObject *selector;

    if ((selector = gst_pad_get_parent (sinkpad))) {
      /* activate the selected pad */
      g_object_set (selector, "active-pad", sinkpad, NULL);
      gst_object_unref (selector);
    }
    gst_object_unref (sinkpad);
  }
  return TRUE;

no_channels:
  {
    GST_PLAY_BIN_UNLOCK (playbin);
    return FALSE;
  }
}

static gboolean
gst_play_bin_set_current_audio_stream (GstPlayBin * playbin, gint stream)
{
  GstSourceGroup *group;
  GPtrArray *channels;
  GstPad *sinkpad;

  GST_PLAY_BIN_LOCK (playbin);
  group = get_group (playbin);
  if (!(channels = group->audio_channels))
    goto no_channels;

  if (stream == -1 || channels->len <= stream) {
    sinkpad = NULL;
  } else {
    /* take channel from selected stream */
    sinkpad = g_ptr_array_index (channels, stream);
  }

  if (sinkpad)
    gst_object_ref (sinkpad);
  GST_PLAY_BIN_UNLOCK (playbin);

  if (sinkpad) {
    GstObject *selector;

    if ((selector = gst_pad_get_parent (sinkpad))) {
      /* activate the selected pad */
      g_object_set (selector, "active-pad", sinkpad, NULL);
      gst_object_unref (selector);
    }
    gst_object_unref (sinkpad);
  }
  return TRUE;

no_channels:
  {
    GST_PLAY_BIN_UNLOCK (playbin);
    return FALSE;
  }
}

static gboolean
gst_play_bin_set_current_text_stream (GstPlayBin * playbin, gint stream)
{
  GstSourceGroup *group;
  GPtrArray *channels;
  GstPad *sinkpad;

  GST_PLAY_BIN_LOCK (playbin);
  group = get_group (playbin);
  if (!(channels = group->text_channels))
    goto no_channels;

  if (stream == -1 || channels->len <= stream) {
    sinkpad = NULL;
  } else {
    /* take channel from selected stream */
    sinkpad = g_ptr_array_index (channels, stream);
  }

  if (sinkpad)
    gst_object_ref (sinkpad);
  GST_PLAY_BIN_UNLOCK (playbin);

  if (sinkpad) {
    GstObject *selector;

    if ((selector = gst_pad_get_parent (sinkpad))) {
      /* activate the selected pad */
      g_object_set (selector, "active-pad", sinkpad, NULL);
      gst_object_unref (selector);
    }
    gst_object_unref (sinkpad);
  }
  return TRUE;

no_channels:
  {
    GST_PLAY_BIN_UNLOCK (playbin);
    return FALSE;
  }
}

static void
gst_play_bin_set_encoding (GstPlayBin * playbin, const gchar * encoding)
{
  GstElement *elem;

  GST_PLAY_BIN_LOCK (playbin);
  g_free (playbin->encoding);
  playbin->encoding = g_strdup (encoding);

  /* set subtitles on all current and next decodebins. */
  if ((elem = playbin->groups[0].uridecodebin))
    g_object_set (G_OBJECT (elem), "subtitle-encoding", encoding, NULL);
  if ((elem = playbin->groups[0].suburidecodebin))
    g_object_set (G_OBJECT (elem), "subtitle-encoding", encoding, NULL);
  if ((elem = playbin->groups[1].uridecodebin))
    g_object_set (G_OBJECT (elem), "subtitle-encoding", encoding, NULL);
  if ((elem = playbin->groups[1].suburidecodebin))
    g_object_set (G_OBJECT (elem), "subtitle-encoding", encoding, NULL);
  GST_PLAY_BIN_UNLOCK (playbin);
}

static void
gst_play_bin_set_sink (GstPlayBin * playbin, GstElement ** elem,
    const gchar * dbg, GstElement * sink)
{
  GST_INFO_OBJECT (playbin, "Setting %s sink to %" GST_PTR_FORMAT, dbg, sink);

  GST_PLAY_BIN_LOCK (playbin);
  if (*elem != sink) {
    GstElement *old;

    old = *elem;
    if (sink) {
      gst_object_ref (sink);
      gst_object_sink (sink);
    }
    *elem = sink;
    if (old)
      gst_object_unref (old);
  }
  GST_LOG_OBJECT (playbin, "%s sink now %" GST_PTR_FORMAT, dbg, *elem);
  GST_PLAY_BIN_UNLOCK (playbin);
}

static void
gst_play_bin_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstPlayBin *playbin;

  playbin = GST_PLAY_BIN (object);

  switch (prop_id) {
    case PROP_URI:
      gst_play_bin_set_uri (playbin, g_value_get_string (value));
      break;
    case PROP_SUBURI:
      gst_play_bin_set_suburi (playbin, g_value_get_string (value));
      break;
    case PROP_FLAGS:
      gst_play_bin_set_flags (playbin, g_value_get_flags (value));
      break;
    case PROP_CURRENT_VIDEO:
      gst_play_bin_set_current_video_stream (playbin, g_value_get_int (value));
      break;
    case PROP_CURRENT_AUDIO:
      gst_play_bin_set_current_audio_stream (playbin, g_value_get_int (value));
      break;
    case PROP_CURRENT_TEXT:
      gst_play_bin_set_current_text_stream (playbin, g_value_get_int (value));
      break;
    case PROP_SUBTITLE_ENCODING:
      gst_play_bin_set_encoding (playbin, g_value_get_string (value));
      break;
    case PROP_VIDEO_SINK:
      gst_play_bin_set_sink (playbin, &playbin->video_sink, "video",
          g_value_get_object (value));
      break;
    case PROP_AUDIO_SINK:
      gst_play_bin_set_sink (playbin, &playbin->audio_sink, "audio",
          g_value_get_object (value));
      break;
    case PROP_VIS_PLUGIN:
      gst_play_sink_set_vis_plugin (playbin->playsink,
          g_value_get_object (value));
      break;
    case PROP_TEXT_SINK:
      gst_play_bin_set_sink (playbin, &playbin->text_sink, "text",
          g_value_get_object (value));
      break;
    case PROP_SUBPIC_SINK:
      gst_play_bin_set_sink (playbin, &playbin->subpic_sink, "subpicture",
          g_value_get_object (value));
      break;
    case PROP_VOLUME:
      gst_play_sink_set_volume (playbin->playsink, g_value_get_double (value));
      break;
    case PROP_MUTE:
      gst_play_sink_set_mute (playbin->playsink, g_value_get_boolean (value));
      break;
    case PROP_FONT_DESC:
      gst_play_sink_set_font_desc (playbin->playsink,
          g_value_get_string (value));
      break;
    case PROP_CONNECTION_SPEED:
      GST_PLAY_BIN_LOCK (playbin);
      playbin->connection_speed = g_value_get_uint (value) * 1000;
      GST_PLAY_BIN_UNLOCK (playbin);
      break;
    case PROP_BUFFER_SIZE:
      playbin->buffer_size = g_value_get_int (value);
      break;
    case PROP_BUFFER_DURATION:
      playbin->buffer_duration = g_value_get_int64 (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstElement *
gst_play_bin_get_current_sink (GstPlayBin * playbin, GstElement ** elem,
    const gchar * dbg, GstPlaySinkType type)
{
  GstElement *sink;

  sink = gst_play_sink_get_sink (playbin->playsink, type);

  GST_LOG_OBJECT (playbin, "play_sink_get_sink() returned %s sink %"
      GST_PTR_FORMAT ", the originally set %s sink is %" GST_PTR_FORMAT,
      dbg, sink, dbg, *elem);

  if (sink == NULL) {
    GST_PLAY_BIN_LOCK (playbin);
    if ((sink = *elem))
      gst_object_ref (sink);
    GST_PLAY_BIN_UNLOCK (playbin);
  }

  return sink;
}

static void
gst_play_bin_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstPlayBin *playbin;

  playbin = GST_PLAY_BIN (object);

  switch (prop_id) {
    case PROP_URI:
    {
      GstSourceGroup *group;

      GST_PLAY_BIN_LOCK (playbin);
      group = get_group (playbin);
      g_value_set_string (value, group->uri);
      GST_PLAY_BIN_UNLOCK (playbin);
      break;
    }
    case PROP_SUBURI:
    {
      GstSourceGroup *group;

      GST_PLAY_BIN_LOCK (playbin);
      group = get_group (playbin);
      g_value_set_string (value, group->suburi);
      GST_PLAY_BIN_UNLOCK (playbin);
      break;
    }
    case PROP_SOURCE:
    {
      GST_OBJECT_LOCK (playbin);
      g_value_set_object (value, playbin->source);
      GST_OBJECT_UNLOCK (playbin);
      break;
    }
    case PROP_FLAGS:
      g_value_set_flags (value, gst_play_bin_get_flags (playbin));
      break;
    case PROP_N_VIDEO:
    {
      GstSourceGroup *group;
      gint n_video;

      GST_PLAY_BIN_LOCK (playbin);
      group = get_group (playbin);
      n_video = (group->video_channels ? group->video_channels->len : 0);
      g_value_set_int (value, n_video);
      GST_PLAY_BIN_UNLOCK (playbin);
      break;
    }
    case PROP_CURRENT_VIDEO:
      GST_PLAY_BIN_LOCK (playbin);
      g_value_set_int (value, playbin->current_video);
      GST_PLAY_BIN_UNLOCK (playbin);
      break;
    case PROP_N_AUDIO:
    {
      GstSourceGroup *group;
      gint n_audio;

      GST_PLAY_BIN_LOCK (playbin);
      group = get_group (playbin);
      n_audio = (group->audio_channels ? group->audio_channels->len : 0);
      g_value_set_int (value, n_audio);
      GST_PLAY_BIN_UNLOCK (playbin);
      break;
    }
    case PROP_CURRENT_AUDIO:
      GST_PLAY_BIN_LOCK (playbin);
      g_value_set_int (value, playbin->current_audio);
      GST_PLAY_BIN_UNLOCK (playbin);
      break;
    case PROP_N_TEXT:
    {
      GstSourceGroup *group;
      gint n_text;

      GST_PLAY_BIN_LOCK (playbin);
      group = get_group (playbin);
      n_text = (group->text_channels ? group->text_channels->len : 0);
      g_value_set_int (value, n_text);
      GST_PLAY_BIN_UNLOCK (playbin);
      break;
    }
    case PROP_CURRENT_TEXT:
      GST_PLAY_BIN_LOCK (playbin);
      g_value_set_int (value, playbin->current_text);
      GST_PLAY_BIN_UNLOCK (playbin);
      break;
    case PROP_SUBTITLE_ENCODING:
      GST_PLAY_BIN_LOCK (playbin);
      g_value_set_string (value, playbin->encoding);
      GST_PLAY_BIN_UNLOCK (playbin);
      break;
    case PROP_VIDEO_SINK:
      g_value_take_object (value,
          gst_play_bin_get_current_sink (playbin, &playbin->video_sink,
              "video", GST_PLAY_SINK_TYPE_VIDEO));
      break;
    case PROP_AUDIO_SINK:
      g_value_take_object (value,
          gst_play_bin_get_current_sink (playbin, &playbin->audio_sink,
              "audio", GST_PLAY_SINK_TYPE_AUDIO));
      break;
    case PROP_VIS_PLUGIN:
      g_value_take_object (value,
          gst_play_sink_get_vis_plugin (playbin->playsink));
      break;
    case PROP_TEXT_SINK:
      g_value_take_object (value,
          gst_play_bin_get_current_sink (playbin, &playbin->text_sink,
              "text", GST_PLAY_SINK_TYPE_TEXT));
      break;
    case PROP_SUBPIC_SINK:
      g_value_take_object (value,
          gst_play_bin_get_current_sink (playbin, &playbin->subpic_sink,
              "subpicture", GST_PLAY_SINK_TYPE_SUBPIC));
      break;
    case PROP_VOLUME:
      g_value_set_double (value, gst_play_sink_get_volume (playbin->playsink));
      break;
    case PROP_MUTE:
      g_value_set_boolean (value, gst_play_sink_get_mute (playbin->playsink));
      break;
    case PROP_FRAME:
      gst_value_take_buffer (value, gst_play_bin_convert_frame (playbin, NULL));
      break;
    case PROP_FONT_DESC:
      g_value_take_string (value,
          gst_play_sink_get_font_desc (playbin->playsink));
      break;
    case PROP_CONNECTION_SPEED:
      GST_PLAY_BIN_LOCK (playbin);
      g_value_set_uint (value, playbin->connection_speed / 1000);
      GST_PLAY_BIN_UNLOCK (playbin);
      break;
    case PROP_BUFFER_SIZE:
      GST_OBJECT_LOCK (playbin);
      g_value_set_int (value, playbin->buffer_size);
      GST_OBJECT_UNLOCK (playbin);
      break;
    case PROP_BUFFER_DURATION:
      GST_OBJECT_LOCK (playbin);
      g_value_set_int64 (value, playbin->buffer_duration);
      GST_OBJECT_UNLOCK (playbin);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* mime types we are not handling on purpose right now, don't post a
 * missing-plugin message for these */
static const gchar *blacklisted_mimes[] = {
  "video/x-dvd-subpicture", "subpicture/x-pgs", NULL
};

static void
gst_play_bin_handle_message (GstBin * bin, GstMessage * msg)
{
  if (gst_is_missing_plugin_message (msg)) {
    gchar *detail;
    guint i;

    detail = gst_missing_plugin_message_get_installer_detail (msg);
    for (i = 0; detail != NULL && blacklisted_mimes[i] != NULL; ++i) {
      if (strstr (detail, "|decoder-") && strstr (detail, blacklisted_mimes[i])) {
        GST_LOG_OBJECT (bin, "suppressing message %" GST_PTR_FORMAT, msg);
        gst_message_unref (msg);
        g_free (detail);
        return;
      }
    }
    g_free (detail);
  }
  GST_BIN_CLASS (parent_class)->handle_message (bin, msg);
}

static void
selector_active_pad_changed (GObject * selector, GParamSpec * pspec,
    GstPlayBin * playbin)
{
  gchar *property;
  GstSourceGroup *group;
  GstSourceSelect *select = NULL;
  int i;

  GST_PLAY_BIN_LOCK (playbin);
  group = get_group (playbin);

  for (i = 0; i < GST_PLAY_SINK_TYPE_LAST; i++) {
    if (selector == G_OBJECT (group->selector[i].selector)) {
      select = &group->selector[i];
    }
  }

  /* We got a pad-change after our group got switched out; no need to notify */
  if (!select) {
    GST_PLAY_BIN_UNLOCK (playbin);
    return;
  }

  switch (select->type) {
    case GST_PLAY_SINK_TYPE_VIDEO:
    case GST_PLAY_SINK_TYPE_VIDEO_RAW:
      property = "current-video";
      playbin->current_video = get_current_stream_number (playbin,
          group->video_channels);
      break;
    case GST_PLAY_SINK_TYPE_AUDIO:
    case GST_PLAY_SINK_TYPE_AUDIO_RAW:
      property = "current-audio";
      playbin->current_audio = get_current_stream_number (playbin,
          group->audio_channels);
      break;
    case GST_PLAY_SINK_TYPE_TEXT:
      property = "current-text";
      playbin->current_text = get_current_stream_number (playbin,
          group->text_channels);
      break;
    default:
      property = NULL;
  }
  GST_PLAY_BIN_UNLOCK (playbin);

  if (property)
    g_object_notify (G_OBJECT (playbin), property);
}

static void
selector_blocked (GstPad * pad, gboolean blocked, gpointer user_data)
{
  /* no nothing */
  GST_DEBUG_OBJECT (pad, "blocked callback, blocked: %d", blocked);
}

/* helper function to lookup stuff in lists */
static gboolean
array_has_value (const gchar * values[], const gchar * value)
{
  gint i;

  for (i = 0; values[i]; i++) {
    if (g_str_has_prefix (value, values[i]))
      return TRUE;
  }
  return FALSE;
}

typedef struct
{
  GstPlayBin *playbin;
  gint stream_id;
  GstPlaySinkType type;
} NotifyTagsData;

static void
notify_tags_cb (GObject * object, GParamSpec * pspec, gpointer user_data)
{
  NotifyTagsData *ntdata = (NotifyTagsData *) user_data;
  gint signal;

  GST_DEBUG_OBJECT (ntdata->playbin, "Tags on pad %" GST_PTR_FORMAT
      " with stream id %d and type %d have changed",
      object, ntdata->stream_id, ntdata->type);

  switch (ntdata->type) {
    case GST_PLAY_SINK_TYPE_VIDEO:
    case GST_PLAY_SINK_TYPE_VIDEO_RAW:
      signal = SIGNAL_VIDEO_TAGS_CHANGED;
      break;
    case GST_PLAY_SINK_TYPE_AUDIO:
    case GST_PLAY_SINK_TYPE_AUDIO_RAW:
      signal = SIGNAL_AUDIO_TAGS_CHANGED;
      break;
    case GST_PLAY_SINK_TYPE_TEXT:
      signal = SIGNAL_TEXT_TAGS_CHANGED;
      break;
    default:
      signal = -1;
      break;
  }

  if (signal >= 0)
    g_signal_emit (G_OBJECT (ntdata->playbin), gst_play_bin_signals[signal], 0,
        ntdata->stream_id);
}

/* this function is called when a new pad is added to decodebin. We check the
 * type of the pad and add it to the selector element of the group. 
 */
static void
pad_added_cb (GstElement * decodebin, GstPad * pad, GstSourceGroup * group)
{
  GstPlayBin *playbin;
  GstCaps *caps;
  const GstStructure *s;
  const gchar *name;
  GstPad *sinkpad;
  GstPadLinkReturn res;
  GstSourceSelect *select = NULL;
  gint i;
  gboolean changed = FALSE;

  playbin = group->playbin;

  caps = gst_pad_get_caps (pad);
  s = gst_caps_get_structure (caps, 0);
  name = gst_structure_get_name (s);

  GST_DEBUG_OBJECT (playbin,
      "pad %s:%s with caps %" GST_PTR_FORMAT " added in group %p",
      GST_DEBUG_PAD_NAME (pad), caps, group);

  /* major type of the pad, this determines the selector to use */
  for (i = 0; i < GST_PLAY_SINK_TYPE_LAST; i++) {
    if (array_has_value (group->selector[i].media_list, name)) {
      select = &group->selector[i];
      break;
    }
  }
  /* no selector found for the media type, don't bother linking it to a
   * selector. This will leave the pad unlinked and thus ignored. */
  if (select == NULL)
    goto unknown_type;

  GST_SOURCE_GROUP_LOCK (group);
  if (select->selector == NULL && playbin->have_selector) {
    /* no selector, create one */
    GST_DEBUG_OBJECT (playbin, "creating new selector");
    select->selector = g_object_new (GST_TYPE_INPUT_SELECTOR, NULL);
    /* the above can't fail, but we keep the error handling around for when
     * the selector plugin has moved to -base or -good and we stop using an
     * internal copy of input-selector */
    if (select->selector == NULL) {
      /* post the missing selector message only once */
      playbin->have_selector = FALSE;
      gst_element_post_message (GST_ELEMENT_CAST (playbin),
          gst_missing_element_message_new (GST_ELEMENT_CAST (playbin),
              "input-selector"));
      GST_ELEMENT_WARNING (playbin, CORE, MISSING_PLUGIN,
          (_("Missing element '%s' - check your GStreamer installation."),
              "input-selector"), (NULL));
    } else {
      g_signal_connect (select->selector, "notify::active-pad",
          G_CALLBACK (selector_active_pad_changed), playbin);

      GST_DEBUG_OBJECT (playbin, "adding new selector %p", select->selector);
      gst_bin_add (GST_BIN_CAST (playbin), select->selector);
      gst_element_set_state (select->selector, GST_STATE_PAUSED);
    }
  }

  if (select->srcpad == NULL) {
    if (select->selector) {
      /* save source pad of the selector */
      select->srcpad = gst_element_get_static_pad (select->selector, "src");
    } else {
      /* no selector, use the pad as the source pad then */
      select->srcpad = gst_object_ref (pad);
    }
    /* block the selector srcpad. It's possible that multiple decodebins start
     * pushing data into the selectors before we have a chance to collect all
     * streams and connect the sinks, resulting in not-linked errors. After we
     * configured the sinks we will unblock them all. */
    gst_pad_set_blocked_async (select->srcpad, TRUE, selector_blocked, NULL);
  }

  /* get sinkpad for the new stream */
  if (select->selector) {
    if ((sinkpad = gst_element_get_request_pad (select->selector, "sink%d"))) {
      gulong notify_tags_handler = 0;
      NotifyTagsData *ntdata;

      GST_DEBUG_OBJECT (playbin, "got pad %s:%s from selector",
          GST_DEBUG_PAD_NAME (sinkpad));

      /* store the selector for the pad */
      g_object_set_data (G_OBJECT (sinkpad), "playbin2.select", select);

      /* connect to the notify::tags signal for our
       * own *-tags-changed signals
       */
      ntdata = g_new0 (NotifyTagsData, 1);
      ntdata->playbin = playbin;
      ntdata->stream_id = select->channels->len;
      ntdata->type = select->type;

      notify_tags_handler =
          g_signal_connect_data (G_OBJECT (sinkpad), "notify::tags",
          G_CALLBACK (notify_tags_cb), ntdata, (GClosureNotify) g_free,
          (GConnectFlags) 0);
      g_object_set_data (G_OBJECT (sinkpad), "playbin2.notify_tags_handler",
          (gpointer) notify_tags_handler);

      /* store the pad in the array */
      GST_DEBUG_OBJECT (playbin, "pad %p added to array", sinkpad);
      g_ptr_array_add (select->channels, sinkpad);

      res = gst_pad_link (pad, sinkpad);
      if (GST_PAD_LINK_FAILED (res))
        goto link_failed;

      /* store selector pad so we can release it */
      g_object_set_data (G_OBJECT (pad), "playbin2.sinkpad", sinkpad);

      changed = TRUE;
      GST_DEBUG_OBJECT (playbin, "linked pad %s:%s to selector %p",
          GST_DEBUG_PAD_NAME (pad), select->selector);
    }
  } else {
    /* no selector, don't configure anything, we'll link the new pad directly to
     * the sink. */
    changed = FALSE;
    sinkpad = NULL;
  }
  GST_SOURCE_GROUP_UNLOCK (group);

  if (changed) {
    int signal;
    switch (select->type) {
      case GST_PLAY_SINK_TYPE_VIDEO:
      case GST_PLAY_SINK_TYPE_VIDEO_RAW:
        /* we want to return NOT_LINKED for unselected pads but only for audio
         * and video pads because text pads might come from an external file. */
        g_object_set (sinkpad, "always-ok", FALSE, NULL);
        signal = SIGNAL_VIDEO_CHANGED;
        break;
      case GST_PLAY_SINK_TYPE_AUDIO:
      case GST_PLAY_SINK_TYPE_AUDIO_RAW:
        g_object_set (sinkpad, "always-ok", FALSE, NULL);
        signal = SIGNAL_AUDIO_CHANGED;
        break;
      case GST_PLAY_SINK_TYPE_TEXT:
        signal = SIGNAL_TEXT_CHANGED;
        break;
      case GST_PLAY_SINK_TYPE_SUBPIC:
      default:
        signal = -1;
    }

    if (signal >= 0)
      g_signal_emit (G_OBJECT (playbin), gst_play_bin_signals[signal], 0, NULL);
  }

done:
  gst_caps_unref (caps);
  return;

  /* ERRORS */
unknown_type:
  {
    GST_ERROR_OBJECT (playbin, "unknown type %s for pad %s:%s",
        name, GST_DEBUG_PAD_NAME (pad));
    goto done;
  }
link_failed:
  {
    GST_ERROR_OBJECT (playbin,
        "failed to link pad %s:%s to selector, reason %d",
        GST_DEBUG_PAD_NAME (pad), res);
    GST_SOURCE_GROUP_UNLOCK (group);
    goto done;
  }
}

/* called when a pad is removed from the uridecodebin. We unlink the pad from
 * the selector. This will make the selector select a new pad. */
static void
pad_removed_cb (GstElement * decodebin, GstPad * pad, GstSourceGroup * group)
{
  GstPlayBin *playbin;
  GstPad *peer;
  GstElement *selector;
  GstSourceSelect *select;

  playbin = group->playbin;

  GST_DEBUG_OBJECT (playbin,
      "pad %s:%s removed from group %p", GST_DEBUG_PAD_NAME (pad), group);

  GST_SOURCE_GROUP_LOCK (group);
  /* get the selector sinkpad */
  if (!(peer = g_object_get_data (G_OBJECT (pad), "playbin2.sinkpad")))
    goto not_linked;

  if ((select = g_object_get_data (G_OBJECT (peer), "playbin2.select"))) {
    gulong notify_tags_handler;

    notify_tags_handler =
        (gulong) g_object_get_data (G_OBJECT (peer),
        "playbin2.notify_tags_handler");
    if (notify_tags_handler != 0)
      g_signal_handler_disconnect (G_OBJECT (peer), notify_tags_handler);
    g_object_set_data (G_OBJECT (peer), "playbin2.notify_tags_handler", NULL);

    /* remove the pad from the array */
    g_ptr_array_remove (select->channels, peer);
    GST_DEBUG_OBJECT (playbin, "pad %p removed from array", peer);
  }

  /* unlink the pad now (can fail, the pad is unlinked before it's removed) */
  gst_pad_unlink (pad, peer);

  /* get selector, this can be NULL when the element is removing the pads
   * because it's being disposed. */
  selector = GST_ELEMENT_CAST (gst_pad_get_parent (peer));
  if (!selector) {
    gst_object_unref (peer);
    goto no_selector;
  }

  /* release the pad to the selector, this will make the selector choose a new
   * pad. */
  gst_element_release_request_pad (selector, peer);
  gst_object_unref (peer);

  gst_object_unref (selector);
  GST_SOURCE_GROUP_UNLOCK (group);

  return;

  /* ERRORS */
not_linked:
  {
    GST_DEBUG_OBJECT (playbin, "pad not linked");
    GST_SOURCE_GROUP_UNLOCK (group);
    return;
  }
no_selector:
  {
    GST_DEBUG_OBJECT (playbin, "selector not found");
    GST_SOURCE_GROUP_UNLOCK (group);
    return;
  }
}

/* we get called when all pads are available and we must connect the sinks to
 * them.
 * The main purpose of the code is to see if we have video/audio and subtitles
 * and pick the right pipelines to display them.
 *
 * The selectors installed on the group tell us about the presence of
 * audio/video and subtitle streams. This allows us to see if we need
 * visualisation, video or/and audio.
 */
static void
no_more_pads_cb (GstElement * decodebin, GstSourceGroup * group)
{
  GstPlayBin *playbin;
  GstPadLinkReturn res;
  gint i;
  gboolean configure;

  playbin = group->playbin;

  GST_DEBUG_OBJECT (playbin, "no more pads in group %p", group);

  GST_PLAY_BIN_SHUTDOWN_LOCK (playbin, shutdown);

  GST_SOURCE_GROUP_LOCK (group);
  for (i = 0; i < GST_PLAY_SINK_TYPE_LAST; i++) {
    GstSourceSelect *select = &group->selector[i];

    /* check if the specific media type was detected and thus has a selector
     * created for it. If there is the media type, get a sinkpad from the sink
     * and link it. We only do this if we have not yet requested the sinkpad
     * before. */
    if (select->srcpad && select->sinkpad == NULL) {
      GST_DEBUG_OBJECT (playbin, "requesting new sink pad %d", select->type);
      select->sinkpad =
          gst_play_sink_request_pad (playbin->playsink, select->type);
      res = gst_pad_link (select->srcpad, select->sinkpad);
      GST_DEBUG_OBJECT (playbin, "linked type %s, result: %d",
          select->media_list[0], res);
      if (res != GST_PAD_LINK_OK) {
        GST_ELEMENT_ERROR (playbin, CORE, PAD,
            ("Internal playbin error."),
            ("Failed to link selector to sink. Error %d", res));
      }
    }
  }
  GST_DEBUG_OBJECT (playbin, "pending %d > %d", group->pending,
      group->pending - 1);

  if (group->pending > 0)
    group->pending--;

  if (group->pending == 0) {
    /* we are the last group to complete, we will configure the output and then
     * signal the other waiters. */
    GST_LOG_OBJECT (playbin, "last group complete");
    configure = TRUE;
  } else {
    GST_LOG_OBJECT (playbin, "have more pending groups");
    configure = FALSE;
  }
  GST_SOURCE_GROUP_UNLOCK (group);

  if (configure) {
    /* if we have custom sinks, configure them now */
    GST_SOURCE_GROUP_LOCK (group);
    if (group->audio_sink) {
      GST_INFO_OBJECT (playbin, "setting custom audio sink %" GST_PTR_FORMAT,
          group->audio_sink);
      gst_play_sink_set_sink (playbin->playsink, GST_PLAY_SINK_TYPE_AUDIO,
          group->audio_sink);
    } else {
      GST_INFO_OBJECT (playbin, "setting default audio sink %" GST_PTR_FORMAT,
          playbin->audio_sink);
      gst_play_sink_set_sink (playbin->playsink, GST_PLAY_SINK_TYPE_AUDIO,
          playbin->audio_sink);
    }
    if (group->video_sink) {
      GST_INFO_OBJECT (playbin, "setting custom video sink %" GST_PTR_FORMAT,
          group->video_sink);
      gst_play_sink_set_sink (playbin->playsink, GST_PLAY_SINK_TYPE_VIDEO,
          group->video_sink);
    } else {
      GST_INFO_OBJECT (playbin, "setting default video sink %" GST_PTR_FORMAT,
          playbin->video_sink);
      gst_play_sink_set_sink (playbin->playsink, GST_PLAY_SINK_TYPE_VIDEO,
          playbin->video_sink);
    }
    gst_play_sink_set_sink (playbin->playsink, GST_PLAY_SINK_TYPE_TEXT,
        playbin->text_sink);
    gst_play_sink_set_sink (playbin->playsink, GST_PLAY_SINK_TYPE_SUBPIC,
        playbin->subpic_sink);
    GST_SOURCE_GROUP_UNLOCK (group);

    GST_LOG_OBJECT (playbin, "reconfigure sink");
    /* we configure the modes if we were the last decodebin to complete. */
    gst_play_sink_reconfigure (playbin->playsink);

    /* signal the other decodebins that they can continue now. */
    GST_SOURCE_GROUP_LOCK (group);
    /* unblock all selectors */
    for (i = 0; i < GST_PLAY_SINK_TYPE_LAST; i++) {
      GstSourceSelect *select = &group->selector[i];

      if (select->srcpad) {
        GST_DEBUG_OBJECT (playbin, "unblocking %" GST_PTR_FORMAT,
            select->srcpad);
        gst_pad_set_blocked_async (select->srcpad, FALSE, selector_blocked,
            NULL);
      }
    }
    GST_SOURCE_GROUP_UNLOCK (group);
  }

  GST_PLAY_BIN_SHUTDOWN_UNLOCK (playbin);

  return;

shutdown:
  {
    GST_DEBUG ("ignoring, we are shutting down");
    /* Request a flushing pad from playsink that we then link to the selector.
     * Then we unblock the selectors so that they stop with a WRONG_STATE
     * instead of a NOT_LINKED error.
     */
    GST_SOURCE_GROUP_LOCK (group);
    for (i = 0; i < GST_PLAY_SINK_TYPE_LAST; i++) {
      GstSourceSelect *select = &group->selector[i];

      if (select->srcpad) {
        if (select->sinkpad == NULL) {
          GST_DEBUG_OBJECT (playbin, "requesting new flushing sink pad");
          select->sinkpad =
              gst_play_sink_request_pad (playbin->playsink,
              GST_PLAY_SINK_TYPE_FLUSHING);
          res = gst_pad_link (select->srcpad, select->sinkpad);
          GST_DEBUG_OBJECT (playbin, "linked flushing, result: %d", res);
        }
        GST_DEBUG_OBJECT (playbin, "unblocking %" GST_PTR_FORMAT,
            select->srcpad);
        gst_pad_set_blocked_async (select->srcpad, FALSE, selector_blocked,
            NULL);
      }
    }
    GST_SOURCE_GROUP_UNLOCK (group);
    return;
  }
}

static void
drained_cb (GstElement * decodebin, GstSourceGroup * group)
{
  GstPlayBin *playbin;

  playbin = group->playbin;

  GST_DEBUG_OBJECT (playbin, "about to finish in group %p", group);

  /* after this call, we should have a next group to activate or we EOS */
  g_signal_emit (G_OBJECT (playbin),
      gst_play_bin_signals[SIGNAL_ABOUT_TO_FINISH], 0, NULL);

  /* now activate the next group. If the app did not set a uri, this will
   * fail and we can do EOS */
  setup_next_source (playbin, GST_STATE_PAUSED);
}

/* Called when we must provide a list of factories to plug to @pad with @caps.
 * We first check if we have a sink that can handle the format and if we do, we
 * return NULL, to expose the pad. If we have no sink (or the sink does not
 * work), we return the list of elements that can connect. */
static GValueArray *
autoplug_factories_cb (GstElement * decodebin, GstPad * pad,
    GstCaps * caps, GstSourceGroup * group)
{
  GstPlayBin *playbin;
  GValueArray *result;

  playbin = group->playbin;

  GST_DEBUG_OBJECT (playbin, "factories group %p for %s:%s, %" GST_PTR_FORMAT,
      group, GST_DEBUG_PAD_NAME (pad), caps);

  /* filter out the elements based on the caps. */
  result = gst_factory_list_filter (playbin->elements, caps);

  GST_DEBUG_OBJECT (playbin, "found factories %p", result);
  gst_factory_list_debug (result);

  return result;
}

/* We are asked to select an element. See if the next element to check
 * is a sink. If this is the case, we see if the sink works by setting it to
 * READY. If the sink works, we return SELECT_EXPOSE to make decodebin
 * expose the raw pad so that we can setup the mixers. */
static GstAutoplugSelectResult
autoplug_select_cb (GstElement * decodebin, GstPad * pad,
    GstCaps * caps, GstElementFactory * factory, GstSourceGroup * group)
{
  GstPlayBin *playbin;
  GstElement *element;
  const gchar *klass;
  GstPlaySinkType type;
  GstElement **sinkp;

  playbin = group->playbin;

  GST_DEBUG_OBJECT (playbin, "select group %p for %s:%s, %" GST_PTR_FORMAT,
      group, GST_DEBUG_PAD_NAME (pad), caps);

  GST_DEBUG_OBJECT (playbin, "checking factory %s",
      GST_PLUGIN_FEATURE_NAME (factory));

  /* if it's not a sink, we just make decodebin try it */
  if (!gst_factory_list_is_type (factory, GST_FACTORY_LIST_SINK))
    return GST_AUTOPLUG_SELECT_TRY;

  /* it's a sink, see if an instance of it actually works */
  GST_DEBUG_OBJECT (playbin, "we found a sink");

  klass = gst_element_factory_get_klass (factory);

  /* figure out the klass */
  if (strstr (klass, "Audio")) {
    GST_DEBUG_OBJECT (playbin, "we found an audio sink");
    type = GST_PLAY_SINK_TYPE_AUDIO;
    sinkp = &group->audio_sink;
  } else if (strstr (klass, "Video")) {
    GST_DEBUG_OBJECT (playbin, "we found a video sink");
    type = GST_PLAY_SINK_TYPE_VIDEO;
    sinkp = &group->video_sink;
  } else {
    /* unknown klass, skip this element */
    GST_WARNING_OBJECT (playbin, "unknown sink klass %s found", klass);
    return GST_AUTOPLUG_SELECT_SKIP;
  }

  /* if we are asked to do visualisations and it's an audio sink, skip the
   * element. We can only do visualisations with raw sinks */
  if (gst_play_sink_get_flags (playbin->playsink) & GST_PLAY_FLAG_VIS) {
    if (type == GST_PLAY_SINK_TYPE_AUDIO) {
      GST_DEBUG_OBJECT (playbin, "skip audio sink because of vis");
      return GST_AUTOPLUG_SELECT_SKIP;
    }
  }

  /* now see if we already have a sink element */
  GST_SOURCE_GROUP_LOCK (group);
  if (*sinkp) {
    GST_DEBUG_OBJECT (playbin, "we already have a pending sink, expose pad");
    /* for now, just assume that we can link the pad to this same sink. FIXME,
     * check that we can link this new pad to this sink as well. */
    GST_SOURCE_GROUP_UNLOCK (group);
    return GST_AUTOPLUG_SELECT_EXPOSE;
  }
  GST_DEBUG_OBJECT (playbin, "we have no pending sink, try to create one");
  GST_SOURCE_GROUP_UNLOCK (group);

  if ((element = gst_element_factory_create (factory, NULL)) == NULL) {
    GST_WARNING_OBJECT (playbin, "Could not create an element from %s",
        gst_plugin_feature_get_name (GST_PLUGIN_FEATURE (factory)));
    return GST_AUTOPLUG_SELECT_SKIP;
  }

  /* ... activate it ... We do this before adding it to the bin so that we
   * don't accidentally make it post error messages that will stop
   * everything. */
  if ((gst_element_set_state (element,
              GST_STATE_READY)) == GST_STATE_CHANGE_FAILURE) {
    GST_WARNING_OBJECT (playbin, "Couldn't set %s to READY",
        GST_ELEMENT_NAME (element));
    gst_object_unref (element);
    return GST_AUTOPLUG_SELECT_SKIP;
  }

  /* remember the sink in the group now, the element is floating, we take
   * ownership now */
  GST_SOURCE_GROUP_LOCK (group);
  if (*sinkp == NULL) {
    /* store the sink in the group, we will configure it later when we
     * reconfigure the sink */
    GST_DEBUG_OBJECT (playbin, "remember sink");
    gst_object_ref (element);
    gst_object_sink (element);
    *sinkp = element;
  } else {
    /* some other thread configured a sink while we were testing the sink, set
     * the sink back to NULL and assume we can use the other sink */
    GST_DEBUG_OBJECT (playbin, "another sink was found, expose pad");
    gst_element_set_state (element, GST_STATE_NULL);
    gst_object_unref (element);
  }
  GST_SOURCE_GROUP_UNLOCK (group);

  /* tell decodebin to expose the pad because we are going to use this
   * sink */
  GST_DEBUG_OBJECT (playbin, "we found a working sink, expose pad");

  return GST_AUTOPLUG_SELECT_EXPOSE;
}

static void
notify_source_cb (GstElement * uridecodebin, GParamSpec * pspec,
    GstSourceGroup * group)
{
  GstPlayBin *playbin;
  GstElement *source;

  playbin = group->playbin;

  g_object_get (group->uridecodebin, "source", &source, NULL);

  GST_OBJECT_LOCK (playbin);
  if (playbin->source)
    gst_object_unref (playbin->source);
  playbin->source = source;
  GST_OBJECT_UNLOCK (playbin);

  g_object_notify (G_OBJECT (playbin), "source");
}

/* must be called with the group lock */
static gboolean
group_set_locked_state_unlocked (GstPlayBin * playbin, GstSourceGroup * group,
    gboolean locked)
{
  GST_DEBUG_OBJECT (playbin, "locked_state %d on group %p", locked, group);

  if (group->uridecodebin)
    gst_element_set_locked_state (group->uridecodebin, locked);
  if (group->suburidecodebin)
    gst_element_set_locked_state (group->suburidecodebin, locked);

  return TRUE;
}

#define REMOVE_SIGNAL(obj,id)            \
if (id) {                                \
  g_signal_handler_disconnect (obj, id); \
  id = 0;                                \
}

/* must be called with PLAY_BIN_LOCK */
static gboolean
activate_group (GstPlayBin * playbin, GstSourceGroup * group, GstState target)
{
  GstElement *uridecodebin;
  GstElement *suburidecodebin = NULL;

  g_return_val_if_fail (group->valid, FALSE);
  g_return_val_if_fail (!group->active, FALSE);

  GST_DEBUG_OBJECT (playbin, "activating group %p", group);

  GST_SOURCE_GROUP_LOCK (group);
  if (group->uridecodebin) {
    GST_DEBUG_OBJECT (playbin, "reusing existing uridecodebin");
    REMOVE_SIGNAL (group->uridecodebin, group->pad_added_id);
    REMOVE_SIGNAL (group->uridecodebin, group->pad_removed_id);
    REMOVE_SIGNAL (group->uridecodebin, group->no_more_pads_id);
    REMOVE_SIGNAL (group->uridecodebin, group->notify_source_id);
    REMOVE_SIGNAL (group->uridecodebin, group->drained_id);
    REMOVE_SIGNAL (group->uridecodebin, group->autoplug_factories_id);
    REMOVE_SIGNAL (group->uridecodebin, group->autoplug_select_id);
    gst_element_set_state (group->uridecodebin, GST_STATE_NULL);
    uridecodebin = group->uridecodebin;
  } else {
    GST_DEBUG_OBJECT (playbin, "making new uridecodebin");
    uridecodebin = gst_element_factory_make ("uridecodebin", NULL);
    if (!uridecodebin)
      goto no_decodebin;
    gst_bin_add (GST_BIN_CAST (playbin), uridecodebin);
    group->uridecodebin = uridecodebin;
  }

  /* configure connection speed */
  g_object_set (uridecodebin, "connection-speed",
      playbin->connection_speed / 1000, NULL);
  if (gst_play_sink_get_flags (playbin->playsink) & GST_PLAY_FLAG_DOWNLOAD)
    g_object_set (uridecodebin, "download", TRUE, NULL);
  else
    g_object_set (uridecodebin, "download", FALSE, NULL);
  /* configure subtitle encoding */
  g_object_set (uridecodebin, "subtitle-encoding", playbin->encoding, NULL);
  /* configure uri */
  g_object_set (uridecodebin, "uri", group->uri, NULL);
  g_object_set (uridecodebin, "buffer-duration", playbin->buffer_duration,
      NULL);
  g_object_set (uridecodebin, "buffer-size", playbin->buffer_size, NULL);

  /* connect pads and other things */
  group->pad_added_id = g_signal_connect (uridecodebin, "pad-added",
      G_CALLBACK (pad_added_cb), group);
  group->pad_removed_id = g_signal_connect (uridecodebin, "pad-removed",
      G_CALLBACK (pad_removed_cb), group);
  group->no_more_pads_id = g_signal_connect (uridecodebin, "no-more-pads",
      G_CALLBACK (no_more_pads_cb), group);
  group->notify_source_id = g_signal_connect (uridecodebin, "notify::source",
      G_CALLBACK (notify_source_cb), group);
  /* we have 1 pending no-more-pads */
  group->pending = 1;

  /* is called when the uridecodebin is out of data and we can switch to the
   * next uri */
  group->drained_id =
      g_signal_connect (uridecodebin, "drained", G_CALLBACK (drained_cb),
      group);

  /* will be called when a new media type is found. We return a list of decoders
   * including sinks for decodebin to try */
  group->autoplug_factories_id =
      g_signal_connect (uridecodebin, "autoplug-factories",
      G_CALLBACK (autoplug_factories_cb), group);
  group->autoplug_select_id = g_signal_connect (uridecodebin, "autoplug-select",
      G_CALLBACK (autoplug_select_cb), group);

  if (group->suburi) {
    /* subtitles */
    if (group->suburidecodebin) {
      GST_DEBUG_OBJECT (playbin, "reusing existing suburidecodebin");
      REMOVE_SIGNAL (group->suburidecodebin, group->sub_pad_added_id);
      REMOVE_SIGNAL (group->suburidecodebin, group->sub_pad_removed_id);
      REMOVE_SIGNAL (group->suburidecodebin, group->sub_no_more_pads_id);
      gst_element_set_state (group->suburidecodebin, GST_STATE_NULL);
      suburidecodebin = group->suburidecodebin;
    } else {
      GST_DEBUG_OBJECT (playbin, "making new suburidecodebin");
      suburidecodebin = gst_element_factory_make ("uridecodebin", NULL);
      if (!suburidecodebin)
        goto no_decodebin;

      gst_bin_add (GST_BIN_CAST (playbin), suburidecodebin);
      group->suburidecodebin = suburidecodebin;
    }

    /* configure connection speed */
    g_object_set (suburidecodebin, "connection-speed",
        playbin->connection_speed, NULL);
    /* configure subtitle encoding */
    g_object_set (suburidecodebin, "subtitle-encoding", playbin->encoding,
        NULL);
    /* configure uri */
    g_object_set (suburidecodebin, "uri", group->suburi, NULL);

    /* connect pads and other things */
    group->sub_pad_added_id = g_signal_connect (suburidecodebin, "pad-added",
        G_CALLBACK (pad_added_cb), group);
    group->sub_pad_removed_id = g_signal_connect (suburidecodebin,
        "pad-removed", G_CALLBACK (pad_removed_cb), group);
    group->sub_no_more_pads_id = g_signal_connect (suburidecodebin,
        "no-more-pads", G_CALLBACK (no_more_pads_cb), group);

    /* we have 2 pending no-more-pads */
    group->pending = 2;
  }

  /* release the group lock before setting the state of the decodebins, they
   * might fire signals in this thread that we need to handle with the
   * group_lock taken. */
  GST_SOURCE_GROUP_UNLOCK (group);

  if (suburidecodebin) {
    if (gst_element_set_state (suburidecodebin,
            target) == GST_STATE_CHANGE_FAILURE)
      goto suburidecodebin_failure;
  }
  if (gst_element_set_state (uridecodebin, target) == GST_STATE_CHANGE_FAILURE)
    goto uridecodebin_failure;

  GST_SOURCE_GROUP_LOCK (group);
  /* alow state changes of the playbin2 affect the group elements now */
  group_set_locked_state_unlocked (playbin, group, FALSE);
  group->active = TRUE;
  GST_SOURCE_GROUP_UNLOCK (group);

  return TRUE;

  /* ERRORS */
no_decodebin:
  {
    GST_SOURCE_GROUP_UNLOCK (group);
    return FALSE;
  }
suburidecodebin_failure:
  {
    GST_DEBUG_OBJECT (playbin, "failed state change of subtitle uridecodebin");
    return FALSE;
  }
uridecodebin_failure:
  {
    GST_DEBUG_OBJECT (playbin, "failed state change of uridecodebin");
    return FALSE;
  }
}

/* unlink a group of uridecodebins from the sink.
 * must be called with PLAY_BIN_LOCK */
static gboolean
deactivate_group (GstPlayBin * playbin, GstSourceGroup * group)
{
  gint i;

  g_return_val_if_fail (group->valid, FALSE);
  g_return_val_if_fail (group->active, FALSE);

  GST_DEBUG_OBJECT (playbin, "unlinking group %p", group);

  GST_SOURCE_GROUP_LOCK (group);
  group->active = FALSE;
  for (i = 0; i < GST_PLAY_SINK_TYPE_LAST; i++) {
    GstSourceSelect *select = &group->selector[i];

    GST_DEBUG_OBJECT (playbin, "unlinking selector %s", select->media_list[0]);

    if (select->srcpad) {
      if (select->sinkpad) {
        GST_LOG_OBJECT (playbin, "unlinking from sink");
        gst_pad_unlink (select->srcpad, select->sinkpad);

        /* release back */
        GST_LOG_OBJECT (playbin, "release sink pad");
        gst_play_sink_release_pad (playbin->playsink, select->sinkpad);
        select->sinkpad = NULL;
      }
      gst_object_unref (select->srcpad);
      select->srcpad = NULL;
    }

    if (select->selector) {
      gst_element_set_state (select->selector, GST_STATE_NULL);
      gst_bin_remove (GST_BIN_CAST (playbin), select->selector);
      select->selector = NULL;
    }
  }
  /* delete any custom sinks we might have */
  if (group->audio_sink)
    gst_object_unref (group->audio_sink);
  group->audio_sink = NULL;
  if (group->video_sink)
    gst_object_unref (group->video_sink);
  group->video_sink = NULL;
  /* we still have the decodebins added to the playbin2 but we can't remove them
   * yet or change their state because this function might be called from the
   * streaming threads, instead block the state so that state changes on the
   * playbin2 don't affect us anymore */
  group_set_locked_state_unlocked (playbin, group, TRUE);
  GST_SOURCE_GROUP_UNLOCK (group);

  return TRUE;
}

/* setup the next group to play, this assumes the next_group is valid and
 * configured. It swaps out the current_group and activates the valid 
 * next_group. */
static gboolean
setup_next_source (GstPlayBin * playbin, GstState target)
{
  GstSourceGroup *new_group, *old_group;

  GST_DEBUG_OBJECT (playbin, "setup sources");

  /* see if there is a next group */
  GST_PLAY_BIN_LOCK (playbin);
  new_group = playbin->next_group;
  if (!new_group || !new_group->valid)
    goto no_next_group;

  /* first unlink the current source, if any */
  old_group = playbin->curr_group;
  if (old_group && old_group->valid) {
    /* unlink our pads with the sink */
    deactivate_group (playbin, old_group);
    old_group->valid = FALSE;
  }

  /* activate the new group */
  if (!activate_group (playbin, new_group, target))
    goto activate_failed;

  /* swap old and new */
  playbin->curr_group = new_group;
  playbin->next_group = old_group;
  GST_PLAY_BIN_UNLOCK (playbin);

  return TRUE;

  /* ERRORS */
no_next_group:
  {
    GST_DEBUG_OBJECT (playbin, "no next group");
    GST_PLAY_BIN_UNLOCK (playbin);
    return FALSE;
  }
activate_failed:
  {
    GST_DEBUG_OBJECT (playbin, "activate failed");
    GST_PLAY_BIN_UNLOCK (playbin);
    return FALSE;
  }
}

/* The group that is currently playing is copied again to the
 * next_group so that it will start playing the next time.
 */
static gboolean
save_current_group (GstPlayBin * playbin)
{
  GstSourceGroup *curr_group;

  GST_DEBUG_OBJECT (playbin, "save current group");

  /* see if there is a current group */
  GST_PLAY_BIN_LOCK (playbin);
  curr_group = playbin->curr_group;
  if (curr_group && curr_group->valid) {
    /* unlink our pads with the sink */
    deactivate_group (playbin, curr_group);
  }
  /* swap old and new */
  playbin->curr_group = playbin->next_group;
  playbin->next_group = curr_group;
  GST_PLAY_BIN_UNLOCK (playbin);

  return TRUE;
}

/* clear the locked state from all groups. This function is called before a
 * state change to NULL is performed on them. */
static gboolean
groups_set_locked_state (GstPlayBin * playbin, gboolean locked)
{
  GST_DEBUG_OBJECT (playbin, "setting locked state to %d on all groups",
      locked);

  GST_PLAY_BIN_LOCK (playbin);
  GST_SOURCE_GROUP_LOCK (playbin->curr_group);
  group_set_locked_state_unlocked (playbin, playbin->curr_group, locked);
  GST_SOURCE_GROUP_UNLOCK (playbin->curr_group);
  GST_SOURCE_GROUP_LOCK (playbin->next_group);
  group_set_locked_state_unlocked (playbin, playbin->next_group, locked);
  GST_SOURCE_GROUP_UNLOCK (playbin->next_group);
  GST_PLAY_BIN_UNLOCK (playbin);

  return TRUE;
}

static GstStateChangeReturn
gst_play_bin_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret;
  GstPlayBin *playbin;

  playbin = GST_PLAY_BIN (element);

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      GST_LOG_OBJECT (playbin, "clearing shutdown flag");
      g_atomic_int_set (&playbin->shutdown, 0);
      if (!setup_next_source (playbin, GST_STATE_READY))
        goto source_failed;
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      /* FIXME unlock our waiting groups */
      GST_LOG_OBJECT (playbin, "setting shutdown flag");
      g_atomic_int_set (&playbin->shutdown, 1);

      /* wait for all callbacks to end by taking the lock.
       * No dynamic (critical) new callbacks will
       * be able to happen as we set the shutdown flag. */
      GST_PLAY_BIN_DYN_LOCK (playbin);
      GST_LOG_OBJECT (playbin, "dynamic lock taken, we can continue shutdown");
      GST_PLAY_BIN_DYN_UNLOCK (playbin);
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      /* unlock so that all groups go to NULL */
      groups_set_locked_state (playbin, FALSE);
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    if (transition == GST_STATE_CHANGE_READY_TO_PAUSED) {
      GstSourceGroup *curr_group;

      curr_group = playbin->curr_group;
      if (curr_group && curr_group->valid) {
        /* unlink our pads with the sink */
        deactivate_group (playbin, curr_group);
      }

      /* Swap current and next group back */
      playbin->curr_group = playbin->next_group;
      playbin->next_group = curr_group;
    }
    return ret;
  }

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      break;
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      /* FIXME Release audio device when we implement that */
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      save_current_group (playbin);
      break;
    case GST_STATE_CHANGE_READY_TO_NULL:
      /* make sure the groups don't perform a state change anymore until we
       * enable them again */
      groups_set_locked_state (playbin, TRUE);
      break;
    default:
      break;
  }

  return ret;

  /* ERRORS */
source_failed:
  {
    return GST_STATE_CHANGE_FAILURE;
  }
}

gboolean
gst_play_bin2_plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_play_bin_debug, "playbin2", 0, "play bin");

  g_type_class_ref (gst_input_selector_get_type ());
  g_type_class_ref (gst_selector_pad_get_type ());

  return gst_element_register (plugin, "playbin2", GST_RANK_NONE,
      GST_TYPE_PLAY_BIN);
}
