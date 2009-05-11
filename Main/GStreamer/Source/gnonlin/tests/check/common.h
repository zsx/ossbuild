
#include <gst/check/gstcheck.h>

typedef struct _Segment {
  gdouble	rate;
  GstFormat	format;
  gint64	start, stop, position;
}	Segment;

typedef struct _CollectStructure {
  GstElement	*comp;
  GstElement	*sink;
  guint64	last_time;
  gboolean	gotsegment;
  GList		*expected_segments;
}	CollectStructure;

static GstElement *
gst_element_factory_make_or_warn (const gchar * factoryname, const gchar * name)
{
  GstElement * element;

  element = gst_element_factory_make (factoryname, name);
  fail_unless (element != NULL, "Failed to make element %s", factoryname);
  return element;
}

static void
composition_pad_added_cb (GstElement *composition, GstPad *pad, CollectStructure * collect)
{
  fail_if (!(gst_element_link (composition, collect->sink)));
}

/* return TRUE to discard the Segment */
static gboolean
compare_segments (Segment * segment, GstEvent * event)
{
  gboolean update;
  gdouble rate;
  GstFormat format;
  gint64 start, stop, position;

  gst_event_parse_new_segment (event, &update, &rate, &format, &start, &stop, &position);

  GST_DEBUG ("Got NewSegment update:%d, rate:%f, format:%d, start:%"GST_TIME_FORMAT
	     ", stop:%"GST_TIME_FORMAT", position:%"GST_TIME_FORMAT,
	     update, rate, format, GST_TIME_ARGS (start),
	     GST_TIME_ARGS (stop),
	     GST_TIME_ARGS (position));

  GST_DEBUG ("Expecting rate:%f, format:%d, start:%"GST_TIME_FORMAT
	     ", stop:%"GST_TIME_FORMAT", position:%"GST_TIME_FORMAT,
	     segment->rate, segment->format,
	     GST_TIME_ARGS (segment->start),
	     GST_TIME_ARGS (segment->stop),
	     GST_TIME_ARGS (segment->position));

  if (update) {
    GST_DEBUG ("was update, ignoring");
    return FALSE;
  }
  fail_if (rate != segment->rate);
  fail_if (format != segment->format);
  fail_if (start != segment->start);
  fail_if (stop != segment->stop);
  fail_if (position != segment->position);

  GST_DEBUG ("Segment was valid, discarding expected Segment");

  return TRUE;
}

static gboolean
sinkpad_event_probe (GstPad * sinkpad, GstEvent * event, CollectStructure * collect)
{
  Segment * segment;
  
  if (GST_EVENT_TYPE (event) == GST_EVENT_NEWSEGMENT) {
    fail_if (collect->expected_segments == NULL);
    segment = (Segment *) collect->expected_segments->data;

    if (compare_segments (segment, event)) {
      collect->expected_segments = g_list_remove (collect->expected_segments, segment);
      g_free (segment);
    }
    collect->gotsegment = TRUE;
  }

  return TRUE;
}

static gboolean
sinkpad_buffer_probe (GstPad * sinkpad, GstBuffer * buffer, CollectStructure * collect)
{
  fail_if(!collect->gotsegment);
  return TRUE;
}

static GstElement *
new_gnl_src (const gchar * name, guint64 start, gint64 duration, gint priority)
{
  GstElement * gnlsource = NULL;

  gnlsource = gst_element_factory_make_or_warn ("gnlsource", name);
  fail_if (gnlsource == NULL);

  g_object_set (G_OBJECT (gnlsource),
		"start", start,
		"duration", duration,
		"media-start", start,
		"media-duration", duration,
		"priority", priority,
		NULL);

  return gnlsource;
}

static GstElement *
videotest_gnl_src (const gchar * name, guint64 start, gint64 duration,
		   gint pattern, guint priority)
{
  GstElement * gnlsource = NULL;
  GstElement * videotestsrc = NULL;
  GstCaps * caps = gst_caps_from_string ("video/x-raw-yuv,format=(fourcc)I420");

  fail_if (caps == NULL);

  videotestsrc = gst_element_factory_make_or_warn ("videotestsrc", NULL);
  g_object_set (G_OBJECT (videotestsrc), "pattern", pattern, NULL);

  gnlsource = new_gnl_src (name, start, duration, priority);
  g_object_set (G_OBJECT (gnlsource), "caps", caps, NULL);
  gst_caps_unref(caps);

  gst_bin_add (GST_BIN (gnlsource), videotestsrc);
  
  return gnlsource;
}

static GstElement *
videotest_gnl_src_full (const gchar * name, guint64 start, gint64 duration,
			guint64 mediastart, gint64 mediaduration,
			gint pattern, guint priority)
{
  GstElement * gnls;

  gnls = videotest_gnl_src (name, start, duration, pattern, priority);
  if (gnls) {
    g_object_set (G_OBJECT (gnls),
		  "media-start", mediastart,
		  "media-duration", mediaduration,
		  NULL);
  }

  return gnls;
}

static GstElement *
videotest_in_bin_gnl_src (const gchar * name, guint64 start, gint64 duration, gint pattern, guint priority)
{
  GstElement * gnlsource = NULL;
  GstElement * videotestsrc = NULL;
  GstElement * bin = NULL;
  GstElement * alpha = NULL;
  GstPad *srcpad = NULL;

  videotestsrc = gst_element_factory_make_or_warn ("videotestsrc", NULL);
  g_object_set (G_OBJECT (videotestsrc), "pattern", pattern, NULL);
  bin = gst_bin_new (NULL);

  alpha = gst_element_factory_make_or_warn ("alpha", NULL);

  gnlsource = new_gnl_src (name, start, duration, priority);

  gst_bin_add (GST_BIN (bin), videotestsrc);
  gst_bin_add (GST_BIN (bin), alpha);

  gst_element_link (videotestsrc, alpha);

  gst_bin_add (GST_BIN (gnlsource), bin);
  
  srcpad = gst_element_get_pad (alpha, "src");

  gst_element_add_pad (bin, gst_ghost_pad_new ("src", srcpad));

  gst_object_unref (srcpad);

  return gnlsource;
}

static GstElement *
audiotest_bin_src (const gchar * name, guint64 start,
		   gint64 duration, guint priority, gboolean intaudio)
{
  GstElement * source = NULL;
  GstElement * identity = NULL;
  GstElement * audiotestsrc = NULL;
  GstElement * audioconvert = NULL;
  GstElement * bin = NULL;
  GstCaps * caps;
  GstPad *srcpad = NULL;

  audiotestsrc = gst_element_factory_make_or_warn ("audiotestsrc", NULL);
  identity = gst_element_factory_make_or_warn ("identity", NULL);
  bin = gst_bin_new (NULL);
  source = new_gnl_src (name, start, duration, priority);
  audioconvert = gst_element_factory_make_or_warn ("audioconvert", NULL);
  
  if (intaudio)
    caps = gst_caps_from_string ("audio/x-raw-int");
  else
    caps = gst_caps_from_string ("audio/x-raw-float");

  gst_bin_add_many (GST_BIN (bin), audiotestsrc, audioconvert, identity, NULL);
  gst_element_link (audiotestsrc, audioconvert);
  fail_if ((gst_element_link_filtered (audioconvert, identity, caps)) != TRUE);

  gst_caps_unref (caps);

  gst_bin_add (GST_BIN (source), bin);

  srcpad = gst_element_get_pad (identity, "src");

  gst_element_add_pad (bin, gst_ghost_pad_new ("src", srcpad));

  gst_object_unref (srcpad);

  return source;
}

static GstElement *
new_operation (const gchar * name, const gchar * factory, guint64 start, gint64 duration, guint priority)
{
  GstElement * gnloperation = NULL;
  GstElement * operation = NULL;

  operation = gst_element_factory_make_or_warn (factory, NULL);
  gnloperation = gst_element_factory_make_or_warn ("gnloperation", NULL);

  g_object_set (G_OBJECT (gnloperation),
		"start", start,
		"duration", duration,
		"priority", priority,
		NULL);

  gst_bin_add (GST_BIN (gnloperation), operation);

  return gnloperation;
}


static Segment *
segment_new (gdouble rate, GstFormat format, gint64 start, gint64 stop, gint64 position)
{
  Segment * segment;

  segment = g_new0 (Segment, 1);

  segment->rate = rate;
  segment->format = format;
  segment->start = start;
  segment->stop = stop;
  segment->position = position;

  return segment;
}

#define check_start_stop_duration(object, startval, stopval, durval)	\
  { \
    g_object_get (object, "start", &start, "stop", &stop, "duration", &duration, NULL); \
    fail_if (start != startval); \
    fail_if (stop != stopval); \
    fail_if (duration != durval); \
  }

