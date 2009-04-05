/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) 2004 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
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
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>

#include "gstsubparse.h"
#include "gstssaparse.h"
#include "samiparse.h"
#include "tmplayerparse.h"
#include "mpl2parse.h"

GST_DEBUG_CATEGORY (sub_parse_debug);

#define DEFAULT_ENCODING   NULL

enum
{
  PROP_0,
  PROP_ENCODING
};

static void
gst_sub_parse_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void
gst_sub_parse_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);


static const GstElementDetails sub_parse_details =
GST_ELEMENT_DETAILS ("Subtitle parser",
    "Codec/Parser/Subtitle",
    "Parses subtitle (.sub) files into text streams",
    "Gustavo J. A. M. Carneiro <gjc@inescporto.pt>\n"
    "Ronald S. Bultje <rbultje@ronald.bitfreak.net>");

#ifndef GST_DISABLE_XML
static GstStaticPadTemplate sink_templ = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-subtitle; application/x-subtitle-sami; "
        "application/x-subtitle-tmplayer; application/x-subtitle-mpl2")
    );
#else
static GstStaticPadTemplate sink_templ = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-subtitle")
    );
#endif

static GstStaticPadTemplate src_templ = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("text/plain; text/x-pango-markup")
    );

static void gst_sub_parse_base_init (GstSubParseClass * klass);
static void gst_sub_parse_class_init (GstSubParseClass * klass);
static void gst_sub_parse_init (GstSubParse * subparse);

static gboolean gst_sub_parse_src_event (GstPad * pad, GstEvent * event);
static gboolean gst_sub_parse_sink_event (GstPad * pad, GstEvent * event);

static GstStateChangeReturn gst_sub_parse_change_state (GstElement * element,
    GstStateChange transition);

static GstFlowReturn gst_sub_parse_chain (GstPad * sinkpad, GstBuffer * buf);

static GstElementClass *parent_class = NULL;

GType
gst_sub_parse_get_type (void)
{
  static GType sub_parse_type = 0;

  if (!sub_parse_type) {
    static const GTypeInfo sub_parse_info = {
      sizeof (GstSubParseClass),
      (GBaseInitFunc) gst_sub_parse_base_init,
      NULL,
      (GClassInitFunc) gst_sub_parse_class_init,
      NULL,
      NULL,
      sizeof (GstSubParse),
      0,
      (GInstanceInitFunc) gst_sub_parse_init,
    };

    sub_parse_type = g_type_register_static (GST_TYPE_ELEMENT,
        "GstSubParse", &sub_parse_info, 0);
  }

  return sub_parse_type;
}

static void
gst_sub_parse_base_init (GstSubParseClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_templ));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_templ));
  gst_element_class_set_details (element_class, &sub_parse_details);
}

static void
gst_sub_parse_dispose (GObject * object)
{
  GstSubParse *subparse = GST_SUBPARSE (object);

  GST_DEBUG_OBJECT (subparse, "cleaning up subtitle parser");

  if (subparse->encoding) {
    g_free (subparse->encoding);
    subparse->encoding = NULL;
  }

  if (subparse->detected_encoding) {
    g_free (subparse->detected_encoding);
    subparse->detected_encoding = NULL;
  }

  if (subparse->adapter) {
    gst_object_unref (subparse->adapter);
    subparse->adapter = NULL;
  }

  if (subparse->textbuf) {
    g_string_free (subparse->textbuf, TRUE);
    subparse->textbuf = NULL;
  }
#ifndef GST_DISABLE_XML
  sami_context_deinit (&subparse->state);
#endif

  GST_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
gst_sub_parse_class_init (GstSubParseClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->dispose = gst_sub_parse_dispose;
  object_class->set_property = gst_sub_parse_set_property;
  object_class->get_property = gst_sub_parse_get_property;

  element_class->change_state = gst_sub_parse_change_state;

  g_object_class_install_property (object_class, PROP_ENCODING,
      g_param_spec_string ("subtitle-encoding", "subtitle charset encoding",
          "Encoding to assume if input subtitles are not in UTF-8 or any other "
          "Unicode encoding. If not set, the GST_SUBTITLE_ENCODING environment "
          "variable will be checked for an encoding to use. If that is not set "
          "either, ISO-8859-15 will be assumed.", DEFAULT_ENCODING,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static void
gst_sub_parse_init (GstSubParse * subparse)
{
  subparse->sinkpad = gst_pad_new_from_static_template (&sink_templ, "sink");
  gst_pad_set_chain_function (subparse->sinkpad,
      GST_DEBUG_FUNCPTR (gst_sub_parse_chain));
  gst_pad_set_event_function (subparse->sinkpad,
      GST_DEBUG_FUNCPTR (gst_sub_parse_sink_event));
  gst_element_add_pad (GST_ELEMENT (subparse), subparse->sinkpad);

  subparse->srcpad = gst_pad_new_from_static_template (&src_templ, "src");
  gst_pad_set_event_function (subparse->srcpad,
      GST_DEBUG_FUNCPTR (gst_sub_parse_src_event));
  gst_element_add_pad (GST_ELEMENT (subparse), subparse->srcpad);

  subparse->textbuf = g_string_new (NULL);
  subparse->parser_type = GST_SUB_PARSE_FORMAT_UNKNOWN;
  subparse->flushing = FALSE;
  gst_segment_init (&subparse->segment, GST_FORMAT_TIME);
  subparse->need_segment = TRUE;
  subparse->encoding = g_strdup (DEFAULT_ENCODING);
  subparse->detected_encoding = NULL;
  subparse->adapter = gst_adapter_new ();
}

/*
 * Source pad functions.
 */

static gboolean
gst_sub_parse_src_event (GstPad * pad, GstEvent * event)
{
  GstSubParse *self = GST_SUBPARSE (gst_pad_get_parent (pad));
  gboolean ret = FALSE;

  GST_DEBUG ("Handling %s event", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_SEEK:
    {
      GstFormat format;
      GstSeekType start_type, stop_type;
      gint64 start, stop;
      gdouble rate;
      gboolean update;

      gst_event_parse_seek (event, &rate, &format, &self->segment_flags,
          &start_type, &start, &stop_type, &stop);

      if (format != GST_FORMAT_TIME) {
        GST_WARNING_OBJECT (self, "we only support seeking in TIME format");
        gst_event_unref (event);
        goto beach;
      }

      /* Convert that seek to a seeking in bytes at position 0,
         FIXME: could use an index */
      ret = gst_pad_push_event (self->sinkpad,
          gst_event_new_seek (rate, GST_FORMAT_BYTES, self->segment_flags,
              GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_NONE, 0));

      if (ret) {
        /* Apply the seek to our segment */
        gst_segment_set_seek (&self->segment, rate, format, self->segment_flags,
            start_type, start, stop_type, stop, &update);

        GST_DEBUG_OBJECT (self, "segment after seek: %" GST_SEGMENT_FORMAT,
            &self->segment);

        self->next_offset = 0;

        self->need_segment = TRUE;
      } else {
        GST_WARNING_OBJECT (self, "seek to 0 bytes failed");
      }

      gst_event_unref (event);
      break;
    }
    default:
      ret = gst_pad_event_default (pad, event);
      break;
  }

beach:
  gst_object_unref (self);

  return ret;
}

static void
gst_sub_parse_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstSubParse *subparse = GST_SUBPARSE (object);

  GST_OBJECT_LOCK (subparse);
  switch (prop_id) {
    case PROP_ENCODING:
      g_free (subparse->encoding);
      subparse->encoding = g_value_dup_string (value);
      GST_LOG_OBJECT (object, "subtitle encoding set to %s",
          GST_STR_NULL (subparse->encoding));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
  GST_OBJECT_UNLOCK (subparse);
}

static void
gst_sub_parse_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstSubParse *subparse = GST_SUBPARSE (object);

  GST_OBJECT_LOCK (subparse);
  switch (prop_id) {
    case PROP_ENCODING:
      g_value_set_string (value, subparse->encoding);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
  GST_OBJECT_UNLOCK (subparse);
}

static gchar *
gst_convert_to_utf8 (const gchar * str, gsize len, const gchar * encoding,
    gsize * consumed, GError ** err)
{
  gchar *ret = NULL;

  *consumed = 0;
  ret =
      g_convert_with_fallback (str, len, "UTF-8", encoding, "*", consumed, NULL,
      err);
  if (ret == NULL)
    return ret;

  /* + 3 to skip UTF-8 BOM if it was added */
  len = strlen (ret);
  if (len >= 3 && (guint8) ret[0] == 0xEF && (guint8) ret[1] == 0xBB
      && (guint8) ret[2] == 0xBF)
    g_memmove (ret, ret + 3, len + 1 - 3);

  return ret;
}

static gchar *
detect_encoding (const gchar * str, gsize len)
{
  if (len >= 3 && (guint8) str[0] == 0xEF && (guint8) str[1] == 0xBB
      && (guint8) str[2] == 0xBF)
    return g_strdup ("UTF-8");

  if (len >= 2 && (guint8) str[0] == 0xFE && (guint8) str[1] == 0xFF)
    return g_strdup ("UTF-16BE");

  if (len >= 2 && (guint8) str[0] == 0xFF && (guint8) str[1] == 0xFE)
    return g_strdup ("UTF-16LE");

  if (len >= 4 && (guint8) str[0] == 0x00 && (guint8) str[1] == 0x00
      && (guint8) str[2] == 0xFE && (guint8) str[3] == 0xFF)
    return g_strdup ("UTF-32BE");

  if (len >= 4 && (guint8) str[0] == 0xFF && (guint8) str[1] == 0xFE
      && (guint8) str[2] == 0x00 && (guint8) str[3] == 0x00)
    return g_strdup ("UTF-32LE");

  return NULL;
}

static gchar *
convert_encoding (GstSubParse * self, const gchar * str, gsize len,
    gsize * consumed)
{
  const gchar *encoding;
  GError *err = NULL;
  gchar *ret = NULL;

  *consumed = 0;

  /* First try any detected encoding */
  if (self->detected_encoding) {
    ret =
        gst_convert_to_utf8 (str, len, self->detected_encoding, consumed, &err);

    if (!err)
      return ret;

    GST_WARNING_OBJECT (self, "could not convert string from '%s' to UTF-8: %s",
        encoding, err->message);
    g_free (self->detected_encoding);
    self->detected_encoding = NULL;
    g_error_free (err);
  }

  /* Otherwise check if it's UTF8 */
  if (self->valid_utf8) {
    if (g_utf8_validate (str, len, NULL)) {
      GST_LOG_OBJECT (self, "valid UTF-8, no conversion needed");
      *consumed = len;
      return g_strndup (str, len);
    }
    GST_INFO_OBJECT (self, "invalid UTF-8!");
    self->valid_utf8 = FALSE;
  }

  /* Else try fallback */
  encoding = self->encoding;
  if (encoding == NULL || *encoding == '\0') {
    encoding = g_getenv ("GST_SUBTITLE_ENCODING");
  }
  if (encoding == NULL || *encoding == '\0') {
    /* if local encoding is UTF-8 and no encoding specified
     * via the environment variable, assume ISO-8859-15 */
    if (g_get_charset (&encoding)) {
      encoding = "ISO-8859-15";
    }
  }

  ret = gst_convert_to_utf8 (str, len, encoding, consumed, &err);

  if (err) {
    GST_WARNING_OBJECT (self, "could not convert string from '%s' to UTF-8: %s",
        encoding, err->message);
    g_error_free (err);

    /* invalid input encoding, fall back to ISO-8859-15 (always succeeds) */
    ret = gst_convert_to_utf8 (str, len, "ISO-8859-15", consumed, NULL);
  }

  GST_LOG_OBJECT (self,
      "successfully converted %" G_GSIZE_FORMAT " characters from %s to UTF-8"
      "%s", len, encoding, (err) ? " , using ISO-8859-15 as fallback" : "");

  return ret;
}

static gchar *
get_next_line (GstSubParse * self)
{
  char *line = NULL;
  const char *line_end;
  int line_len;
  gboolean have_r = FALSE;

  line_end = strchr (self->textbuf->str, '\n');

  if (!line_end) {
    /* end-of-line not found; return for more data */
    return NULL;
  }

  /* get rid of '\r' */
  if (line_end != self->textbuf->str && *(line_end - 1) == '\r') {
    line_end--;
    have_r = TRUE;
  }

  line_len = line_end - self->textbuf->str;
  line = g_strndup (self->textbuf->str, line_len);
  self->textbuf = g_string_erase (self->textbuf, 0,
      line_len + (have_r ? 2 : 1));
  return line;
}

static gchar *
parse_mdvdsub (ParserState * state, const gchar * line)
{
  const gchar *line_split;
  gchar *line_chunk;
  guint start_frame, end_frame;
  gint64 clip_start = 0, clip_stop = 0;
  gboolean in_seg = FALSE;
  GString *markup;
  gchar *ret;

  /* style variables */
  gboolean italic;
  gboolean bold;
  guint fontsize;

  if (sscanf (line, "{%u}{%u}", &start_frame, &end_frame) != 2) {
    g_warning ("Parse of the following line, assumed to be in microdvd .sub"
        " format, failed:\n%s", line);
    return NULL;
  }

  /* skip the {%u}{%u} part */
  line = strchr (line, '}') + 1;
  line = strchr (line, '}') + 1;

  /* see if there's a first line with a framerate */
  if (state->fps == 0.0 && start_frame == 1 && end_frame == 1) {
    gchar *rest, *end = NULL;

    rest = g_strdup (line);
    g_strdelimit (rest, ",", '.');
    state->fps = g_ascii_strtod (rest, &end);
    if (end == rest)
      state->fps = 0.0;
    GST_INFO ("framerate from file: %f ('%s')", state->fps, rest);
    g_free (rest);
    return NULL;
  }

  if (state->fps == 0.0) {
    /* FIXME: hardcoded for now, is there a better way/assumption? */
    state->fps = 24000.0 / 1001.0;
    GST_INFO ("no framerate specified, assuming %f", state->fps);
  }

  state->start_time = start_frame / state->fps * GST_SECOND;
  state->duration = (end_frame - start_frame) / state->fps * GST_SECOND;

  /* Check our segment start/stop */
  in_seg = gst_segment_clip (state->segment, GST_FORMAT_TIME,
      state->start_time, state->start_time + state->duration, &clip_start,
      &clip_stop);

  /* No need to parse that text if it's out of segment */
  if (in_seg) {
    state->start_time = clip_start;
    state->duration = clip_stop - clip_start;
  } else {
    return NULL;
  }

  markup = g_string_new (NULL);
  while (1) {
    italic = FALSE;
    bold = FALSE;
    fontsize = 0;
    /* parse style markup */
    if (strncmp (line, "{y:i}", 5) == 0) {
      italic = TRUE;
      line = strchr (line, '}') + 1;
    }
    if (strncmp (line, "{y:b}", 5) == 0) {
      bold = TRUE;
      line = strchr (line, '}') + 1;
    }
    if (sscanf (line, "{s:%u}", &fontsize) == 1) {
      line = strchr (line, '}') + 1;
    }
    /* forward slashes at beginning/end signify italics too */
    if (g_str_has_prefix (line, "/")) {
      italic = TRUE;
      ++line;
    }
    if ((line_split = strchr (line, '|')))
      line_chunk = g_markup_escape_text (line, line_split - line);
    else
      line_chunk = g_markup_escape_text (line, strlen (line));

    /* Remove italics markers at end of line/stanza (CHECKME: are end slashes
     * always at the end of a line or can they span multiple lines?) */
    if (g_str_has_suffix (line_chunk, "/")) {
      line_chunk[strlen (line_chunk) - 1] = '\0';
    }

    markup = g_string_append (markup, "<span");
    if (italic)
      g_string_append (markup, " style=\"italic\"");
    if (bold)
      g_string_append (markup, " weight=\"bold\"");
    if (fontsize)
      g_string_append_printf (markup, " size=\"%u\"", fontsize * 1000);
    g_string_append_printf (markup, ">%s</span>", line_chunk);
    g_free (line_chunk);
    if (line_split) {
      g_string_append (markup, "\n");
      line = line_split + 1;
    } else {
      break;
    }
  }
  ret = markup->str;
  g_string_free (markup, FALSE);
  GST_DEBUG ("parse_mdvdsub returning (%f+%f): %s",
      state->start_time / (double) GST_SECOND,
      state->duration / (double) GST_SECOND, ret);
  return ret;
}

static void
strip_trailing_newlines (gchar * txt)
{
  if (txt) {
    guint len;

    len = strlen (txt);
    while (len > 1 && txt[len - 1] == '\n') {
      txt[len - 1] = '\0';
      --len;
    }
  }
}

/* we want to escape text in general, but retain basic markup like
 * <i></i>, <u></u>, and <b></b>. The easiest and safest way is to
 * just unescape a white list of allowed markups again after
 * escaping everything (the text between these simple markers isn't
 * necessarily escaped, so it seems best to do it like this) */
static void
subrip_unescape_formatting (gchar * txt)
{
  gchar *pos;

  for (pos = txt; pos != NULL && *pos != '\0'; ++pos) {
    if (g_ascii_strncasecmp (pos, "&lt;u&gt;", 9) == 0 ||
        g_ascii_strncasecmp (pos, "&lt;i&gt;", 9) == 0 ||
        g_ascii_strncasecmp (pos, "&lt;b&gt;", 9) == 0) {
      pos[0] = '<';
      pos[1] = g_ascii_tolower (pos[4]);
      pos[2] = '>';
      /* move NUL terminator as well */
      g_memmove (pos + 3, pos + 9, strlen (pos + 9) + 1);
      pos += 2;
    }
  }

  for (pos = txt; pos != NULL && *pos != '\0'; ++pos) {
    if (g_ascii_strncasecmp (pos, "&lt;/u&gt;", 10) == 0 ||
        g_ascii_strncasecmp (pos, "&lt;/i&gt;", 10) == 0 ||
        g_ascii_strncasecmp (pos, "&lt;/b&gt;", 10) == 0) {
      pos[0] = '<';
      pos[1] = '/';
      pos[2] = g_ascii_tolower (pos[5]);
      pos[3] = '>';
      /* move NUL terminator as well */
      g_memmove (pos + 4, pos + 10, strlen (pos + 10) + 1);
      pos += 3;
    }
  }
}


static gboolean
subrip_remove_unhandled_tag (gchar * start, gchar * stop)
{
  gchar *tag, saved;

  tag = start + strlen ("&lt;");
  if (*tag == '/')
    ++tag;

  if (g_ascii_tolower (*tag) < 'a' || g_ascii_tolower (*tag) > 'z')
    return FALSE;

  saved = *stop;
  *stop = '\0';
  GST_LOG ("removing unhandled tag '%s'", start);
  *stop = saved;
  g_memmove (start, stop, strlen (stop) + 1);
  return TRUE;
}

/* remove tags we haven't explicitly allowed earlier on, like font tags
 * for example */
static void
subrip_remove_unhandled_tags (gchar * txt)
{
  gchar *pos, *gt;

  for (pos = txt; pos != NULL && *pos != '\0'; ++pos) {
    if (strncmp (pos, "&lt;", 4) == 0 && (gt = strstr (pos + 4, "&gt;"))) {
      if (subrip_remove_unhandled_tag (pos, gt + strlen ("&gt;")))
        --pos;
    }
  }
}

/* we only allow <i>, <u> and <b>, so let's take a simple approach. This code
 * assumes the input has been escaped and subrip_unescape_formatting() has then
 * been run over the input! This function adds missing closing markup tags and
 * removes broken closing tags for tags that have never been opened. */
static void
subrip_fix_up_markup (gchar ** p_txt)
{
  gchar *cur, *next_tag;
  gchar open_tags[32];
  guint num_open_tags = 0;

  g_assert (*p_txt != NULL);

  cur = *p_txt;
  while (*cur != '\0') {
    next_tag = strchr (cur, '<');
    if (next_tag == NULL)
      break;
    ++next_tag;
    switch (*next_tag) {
      case '/':{
        ++next_tag;
        if (num_open_tags == 0 || open_tags[num_open_tags - 1] != *next_tag) {
          GST_LOG ("broken input, closing tag '%c' is not open", *next_tag);
          g_memmove (next_tag - 2, next_tag + 2, strlen (next_tag + 2) + 1);
          next_tag -= 2;
        } else {
          /* it's all good, closing tag which is open */
          --num_open_tags;
        }
        break;
      }
      case 'i':
      case 'b':
      case 'u':
        if (num_open_tags == G_N_ELEMENTS (open_tags))
          return;               /* something dodgy is going on, stop parsing */
        open_tags[num_open_tags] = *next_tag;
        ++num_open_tags;
        break;
      default:
        GST_ERROR ("unexpected tag '%c' (%s)", *next_tag, next_tag);
        g_assert_not_reached ();
        break;
    }
    cur = next_tag;
  }

  if (num_open_tags > 0) {
    GString *s;

    s = g_string_new (*p_txt);
    while (num_open_tags > 0) {
      GST_LOG ("adding missing closing tag '%c'", open_tags[num_open_tags - 1]);
      g_string_append_c (s, '<');
      g_string_append_c (s, '/');
      g_string_append_c (s, open_tags[num_open_tags - 1]);
      g_string_append_c (s, '>');
      --num_open_tags;
    }
    g_free (*p_txt);
    *p_txt = g_string_free (s, FALSE);
  }
}

static gchar *
parse_subrip (ParserState * state, const gchar * line)
{
  guint h1, m1, s1, ms1;
  guint h2, m2, s2, ms2;
  int subnum;
  gchar *ret;

  switch (state->state) {
    case 0:
      /* looking for a single integer */
      if (sscanf (line, "%u", &subnum) == 1)
        state->state = 1;
      return NULL;
    case 1:
      /* looking for start_time --> end_time */
      if (sscanf (line, "%u:%u:%u,%u --> %u:%u:%u,%u",
              &h1, &m1, &s1, &ms1, &h2, &m2, &s2, &ms2) == 8) {
        state->state = 2;
        state->start_time =
            (((guint64) h1) * 3600 + m1 * 60 + s1) * GST_SECOND +
            ms1 * GST_MSECOND;
        state->duration =
            (((guint64) h2) * 3600 + m2 * 60 + s2) * GST_SECOND +
            ms2 * GST_MSECOND - state->start_time;
      } else {
        GST_DEBUG ("error parsing subrip time line");
        state->state = 0;
      }
      return NULL;
    case 2:
    {
      /* No need to parse that text if it's out of segment */
      gint64 clip_start = 0, clip_stop = 0;
      gboolean in_seg = FALSE;

      /* Check our segment start/stop */
      in_seg = gst_segment_clip (state->segment, GST_FORMAT_TIME,
          state->start_time, state->start_time + state->duration,
          &clip_start, &clip_stop);

      if (in_seg) {
        state->start_time = clip_start;
        state->duration = clip_stop - clip_start;
      } else {
        state->state = 0;
        return NULL;
      }
    }
      /* looking for subtitle text; empty line ends this subtitle entry */
      if (state->buf->len)
        g_string_append_c (state->buf, '\n');
      g_string_append (state->buf, line);
      if (strlen (line) == 0) {
        ret = g_markup_escape_text (state->buf->str, state->buf->len);
        g_string_truncate (state->buf, 0);
        state->state = 0;
        subrip_unescape_formatting (ret);
        subrip_remove_unhandled_tags (ret);
        strip_trailing_newlines (ret);
        subrip_fix_up_markup (&ret);
        return ret;
      }
      return NULL;
    default:
      g_return_val_if_reached (NULL);
  }
}

static void
subviewer_unescape_newlines (gchar * read)
{
  gchar *write = read;

  /* Replace all occurences of '[br]' with a newline as version 2
   * of the subviewer format uses this for newlines */

  if (read[0] == '\0' || read[1] == '\0' || read[2] == '\0' || read[3] == '\0')
    return;

  do {
    if (strncmp (read, "[br]", 4) == 0) {
      *write = '\n';
      read += 4;
    } else {
      *write = *read;
      read++;
    }
    write++;
  } while (*read);

  *write = '\0';
}

static gchar *
parse_subviewer (ParserState * state, const gchar * line)
{
  guint h1, m1, s1, ms1;
  guint h2, m2, s2, ms2;
  gchar *ret;

  /* TODO: Maybe also parse the fields in the header, especially DELAY.
   * For examples see the unit test or
   * http://www.doom9.org/index.html?/sub.htm */

  switch (state->state) {
    case 0:
      /* looking for start_time,end_time */
      if (sscanf (line, "%u:%u:%u.%u,%u:%u:%u.%u",
              &h1, &m1, &s1, &ms1, &h2, &m2, &s2, &ms2) == 8) {
        state->state = 1;
        state->start_time =
            (((guint64) h1) * 3600 + m1 * 60 + s1) * GST_SECOND +
            ms1 * GST_MSECOND;
        state->duration =
            (((guint64) h2) * 3600 + m2 * 60 + s2) * GST_SECOND +
            ms2 * GST_MSECOND - state->start_time;
      }
      return NULL;
    case 1:
    {
      /* No need to parse that text if it's out of segment */
      gint64 clip_start = 0, clip_stop = 0;
      gboolean in_seg = FALSE;

      /* Check our segment start/stop */
      in_seg = gst_segment_clip (state->segment, GST_FORMAT_TIME,
          state->start_time, state->start_time + state->duration,
          &clip_start, &clip_stop);

      if (in_seg) {
        state->start_time = clip_start;
        state->duration = clip_stop - clip_start;
      } else {
        state->state = 0;
        return NULL;
      }
    }
      /* looking for subtitle text; empty line ends this subtitle entry */
      if (state->buf->len)
        g_string_append_c (state->buf, '\n');
      g_string_append (state->buf, line);
      if (strlen (line) == 0) {
        ret = g_strdup (state->buf->str);
        subviewer_unescape_newlines (ret);
        strip_trailing_newlines (ret);
        g_string_truncate (state->buf, 0);
        state->state = 0;
        return ret;
      }
      return NULL;
    default:
      g_assert_not_reached ();
      return NULL;
  }
}

static gchar *
parse_mpsub (ParserState * state, const gchar * line)
{
  gchar *ret;
  float t1, t2;

  switch (state->state) {
    case 0:
      /* looking for two floats (offset, duration) */
      if (sscanf (line, "%f %f", &t1, &t2) == 2) {
        state->state = 1;
        state->start_time += state->duration + GST_SECOND * t1;
        state->duration = GST_SECOND * t2;
      }
      return NULL;
    case 1:
    {                           /* No need to parse that text if it's out of segment */
      gint64 clip_start = 0, clip_stop = 0;
      gboolean in_seg = FALSE;

      /* Check our segment start/stop */
      in_seg = gst_segment_clip (state->segment, GST_FORMAT_TIME,
          state->start_time, state->start_time + state->duration,
          &clip_start, &clip_stop);

      if (in_seg) {
        state->start_time = clip_start;
        state->duration = clip_stop - clip_start;
      } else {
        state->state = 0;
        return NULL;
      }
    }
      /* looking for subtitle text; empty line ends this
       * subtitle entry */
      if (state->buf->len)
        g_string_append_c (state->buf, '\n');
      g_string_append (state->buf, line);
      if (strlen (line) == 0) {
        ret = g_strdup (state->buf->str);
        g_string_truncate (state->buf, 0);
        state->state = 0;
        return ret;
      }
      return NULL;
    default:
      g_assert_not_reached ();
      return NULL;
  }
}

static void
parser_state_init (ParserState * state)
{
  GST_DEBUG ("initialising parser");

  if (state->buf) {
    g_string_truncate (state->buf, 0);
  } else {
    state->buf = g_string_new (NULL);
  }

  state->start_time = 0;
  state->duration = 0;
  state->max_duration = 0;      /* no limit */
  state->state = 0;
  state->segment = NULL;
}

static void
parser_state_dispose (ParserState * state)
{
  if (state->buf) {
    g_string_free (state->buf, TRUE);
    state->buf = NULL;
  }
#ifndef GST_DISABLE_XML
  if (state->user_data) {
    sami_context_reset (state);
  }
#endif
}

/*
 * FIXME: maybe we should pass along a second argument, the preceding
 * text buffer, because that is how this originally worked, even though
 * I don't really see the use of that.
 */

static GstSubParseFormat
gst_sub_parse_data_format_autodetect (gchar * match_str)
{
  static gboolean need_init_regexps = TRUE;
  static regex_t mdvd_rx;
  static regex_t subrip_rx;
  guint n1, n2, n3;

  /* initialize the regexps used the first time around */
  if (need_init_regexps) {
    int err;
    char errstr[128];

    need_init_regexps = FALSE;
    if ((err = regcomp (&mdvd_rx, "^\\{[0-9]+\\}\\{[0-9]+\\}",
                REG_EXTENDED | REG_NEWLINE | REG_NOSUB) != 0) ||
        (err = regcomp (&subrip_rx, "^([ 0-9]){0,3}[0-9](\x0d)?\x0a"
                "[ 0-9][0-9]:[ 0-9][0-9]:[ 0-9][0-9],[ 0-9]{2}[0-9]"
                " --> ([ 0-9])?[0-9]:[ 0-9][0-9]:[ 0-9][0-9],[ 0-9]{2}[0-9]",
                REG_EXTENDED | REG_NEWLINE | REG_NOSUB)) != 0) {
      regerror (err, &subrip_rx, errstr, 127);
      GST_WARNING ("Compilation of subrip regex failed: %s", errstr);
    }
  }

  if (regexec (&mdvd_rx, match_str, 0, NULL, 0) == 0) {
    GST_LOG ("MicroDVD (frame based) format detected");
    return GST_SUB_PARSE_FORMAT_MDVDSUB;
  }
  if (regexec (&subrip_rx, match_str, 0, NULL, 0) == 0) {
    GST_LOG ("SubRip (time based) format detected");
    return GST_SUB_PARSE_FORMAT_SUBRIP;
  }

  if (!strncmp (match_str, "FORMAT=TIME", 11)) {
    GST_LOG ("MPSub (time based) format detected");
    return GST_SUB_PARSE_FORMAT_MPSUB;
  }
#ifndef GST_DISABLE_XML
  if (strstr (match_str, "<SAMI>") != NULL ||
      strstr (match_str, "<sami>") != NULL) {
    GST_LOG ("SAMI (time based) format detected");
    return GST_SUB_PARSE_FORMAT_SAMI;
  }
#endif
  /* we're boldly assuming the first subtitle appears within the first hour */
  if (sscanf (match_str, "0:%02u:%02u:", &n1, &n2) == 2 ||
      sscanf (match_str, "0:%02u:%02u=", &n1, &n2) == 2 ||
      sscanf (match_str, "00:%02u:%02u:", &n1, &n2) == 2 ||
      sscanf (match_str, "00:%02u:%02u=", &n1, &n2) == 2 ||
      sscanf (match_str, "00:%02u:%02u,%u=", &n1, &n2, &n3) == 3) {
    GST_LOG ("TMPlayer (time based) format detected");
    return GST_SUB_PARSE_FORMAT_TMPLAYER;
  }
  if (sscanf (match_str, "[%u][%u]", &n1, &n2) == 2) {
    GST_LOG ("MPL2 (time based) format detected");
    return GST_SUB_PARSE_FORMAT_MPL2;
  }
  if (strstr (match_str, "[INFORMATION]") != NULL) {
    GST_LOG ("SubViewer (time based) format detected");
    return GST_SUB_PARSE_FORMAT_SUBVIEWER;
  }

  GST_DEBUG ("no subtitle format detected");
  return GST_SUB_PARSE_FORMAT_UNKNOWN;
}

static GstCaps *
gst_sub_parse_format_autodetect (GstSubParse * self)
{
  gchar *data;
  GstSubParseFormat format;

  if (strlen (self->textbuf->str) < 35) {
    GST_DEBUG ("File too small to be a subtitles file");
    return NULL;
  }

  data = g_strndup (self->textbuf->str, 35);
  format = gst_sub_parse_data_format_autodetect (data);
  g_free (data);

  self->parser_type = format;
  parser_state_init (&self->state);

  switch (format) {
    case GST_SUB_PARSE_FORMAT_MDVDSUB:
      self->parse_line = parse_mdvdsub;
      return gst_caps_new_simple ("text/x-pango-markup", NULL);
    case GST_SUB_PARSE_FORMAT_SUBRIP:
      self->parse_line = parse_subrip;
      return gst_caps_new_simple ("text/x-pango-markup", NULL);
    case GST_SUB_PARSE_FORMAT_MPSUB:
      self->parse_line = parse_mpsub;
      return gst_caps_new_simple ("text/plain", NULL);
#ifndef GST_DISABLE_XML
    case GST_SUB_PARSE_FORMAT_SAMI:
      self->parse_line = parse_sami;
      sami_context_init (&self->state);
      return gst_caps_new_simple ("text/x-pango-markup", NULL);
#endif
    case GST_SUB_PARSE_FORMAT_TMPLAYER:
      self->parse_line = parse_tmplayer;
      self->state.max_duration = 5 * GST_SECOND;
      return gst_caps_new_simple ("text/plain", NULL);
    case GST_SUB_PARSE_FORMAT_MPL2:
      self->parse_line = parse_mpl2;
      return gst_caps_new_simple ("text/x-pango-markup", NULL);
    case GST_SUB_PARSE_FORMAT_SUBVIEWER:
      self->parse_line = parse_subviewer;
      return gst_caps_new_simple ("text/plain", NULL);
    case GST_SUB_PARSE_FORMAT_UNKNOWN:
    default:
      GST_DEBUG ("no subtitle format detected");
      GST_ELEMENT_ERROR (self, STREAM, WRONG_TYPE,
          ("The input is not a valid/supported subtitle file"), (NULL));
      return NULL;
  }
}

static void
feed_textbuf (GstSubParse * self, GstBuffer * buf)
{
  gboolean discont;
  gsize consumed;
  gchar *input = NULL;

  discont = GST_BUFFER_IS_DISCONT (buf);

  if (GST_BUFFER_OFFSET_IS_VALID (buf) &&
      GST_BUFFER_OFFSET (buf) != self->offset) {
    self->offset = GST_BUFFER_OFFSET (buf);
    discont = TRUE;
  }

  if (discont) {
    GST_INFO ("discontinuity");
    /* flush the parser state */
    parser_state_init (&self->state);
    g_string_truncate (self->textbuf, 0);
    gst_adapter_clear (self->adapter);
#ifndef GST_DISABLE_XML
    sami_context_reset (&self->state);
#endif
    /* we could set a flag to make sure that the next buffer we push out also
     * has the DISCONT flag set, but there's no point really given that it's
     * subtitles which are discontinuous by nature. */
  }

  self->offset = GST_BUFFER_OFFSET (buf) + GST_BUFFER_SIZE (buf);
  self->next_offset = self->offset;

  gst_adapter_push (self->adapter, buf);

  input =
      convert_encoding (self, (const gchar *) gst_adapter_peek (self->adapter,
          gst_adapter_available (self->adapter)),
      (gsize) gst_adapter_available (self->adapter), &consumed);

  if (input && consumed > 0) {
    self->textbuf = g_string_append (self->textbuf, input);
    gst_adapter_flush (self->adapter, consumed);
  }

  g_free (input);
}

static GstFlowReturn
handle_buffer (GstSubParse * self, GstBuffer * buf)
{
  GstFlowReturn ret = GST_FLOW_OK;
  GstCaps *caps = NULL;
  gchar *line, *subtitle;

  if (self->first_buffer) {
    self->detected_encoding =
        detect_encoding ((gchar *) GST_BUFFER_DATA (buf),
        GST_BUFFER_SIZE (buf));
    self->first_buffer = FALSE;
  }

  feed_textbuf (self, buf);

  /* make sure we know the format */
  if (G_UNLIKELY (self->parser_type == GST_SUB_PARSE_FORMAT_UNKNOWN)) {
    if (!(caps = gst_sub_parse_format_autodetect (self))) {
      return GST_FLOW_UNEXPECTED;
    }
    if (!gst_pad_set_caps (self->srcpad, caps)) {
      gst_caps_unref (caps);
      return GST_FLOW_UNEXPECTED;
    }
    gst_caps_unref (caps);
  }

  while ((line = get_next_line (self)) && !self->flushing) {
    guint offset = 0;

    /* Set segment on our parser state machine */
    self->state.segment = &self->segment;
    /* Now parse the line, out of segment lines will just return NULL */
    GST_LOG_OBJECT (self, "Parsing line '%s'", line + offset);
    subtitle = self->parse_line (&self->state, line + offset);
    g_free (line);

    if (subtitle) {
      guint subtitle_len = strlen (subtitle);

      /* +1 for terminating NUL character */
      ret = gst_pad_alloc_buffer_and_set_caps (self->srcpad,
          GST_BUFFER_OFFSET_NONE, subtitle_len + 1,
          GST_PAD_CAPS (self->srcpad), &buf);

      if (ret == GST_FLOW_OK) {
        /* copy terminating NUL character as well */
        memcpy (GST_BUFFER_DATA (buf), subtitle, subtitle_len + 1);
        GST_BUFFER_SIZE (buf) = subtitle_len;
        GST_BUFFER_TIMESTAMP (buf) = self->state.start_time;
        GST_BUFFER_DURATION (buf) = self->state.duration;

        /* in some cases (e.g. tmplayer) we can only determine the duration
         * of a text chunk from the timestamp of the next text chunk; in those
         * cases, we probably want to limit the duration to something
         * reasonable, so we don't end up showing some text for e.g. 40 seconds
         * just because nothing else is being said during that time */
        if (self->state.max_duration > 0 && GST_BUFFER_DURATION_IS_VALID (buf)) {
          if (GST_BUFFER_DURATION (buf) > self->state.max_duration)
            GST_BUFFER_DURATION (buf) = self->state.max_duration;
        }

        gst_segment_set_last_stop (&self->segment, GST_FORMAT_TIME,
            self->state.start_time);

        GST_DEBUG_OBJECT (self, "Sending text '%s', %" GST_TIME_FORMAT " + %"
            GST_TIME_FORMAT, subtitle, GST_TIME_ARGS (self->state.start_time),
            GST_TIME_ARGS (self->state.duration));

        ret = gst_pad_push (self->srcpad, buf);
      }

      /* move this forward (the tmplayer parser needs this) */
      if (self->state.duration != GST_CLOCK_TIME_NONE)
        self->state.start_time += self->state.duration;

      g_free (subtitle);
      subtitle = NULL;

      if (ret != GST_FLOW_OK) {
        GST_DEBUG_OBJECT (self, "flow: %s", gst_flow_get_name (ret));
        break;
      }
    }
  }

  return ret;
}

static GstFlowReturn
gst_sub_parse_chain (GstPad * sinkpad, GstBuffer * buf)
{
  GstFlowReturn ret;
  GstSubParse *self;

  self = GST_SUBPARSE (GST_PAD_PARENT (sinkpad));

  /* Push newsegment if needed */
  if (self->need_segment) {
    GST_LOG_OBJECT (self, "pushing newsegment event with %" GST_SEGMENT_FORMAT,
        &self->segment);

    gst_pad_push_event (self->srcpad, gst_event_new_new_segment (FALSE,
            self->segment.rate, self->segment.format,
            self->segment.last_stop, self->segment.stop, self->segment.time));
    self->need_segment = FALSE;
  }

  ret = handle_buffer (self, buf);

  return ret;
}

static gboolean
gst_sub_parse_sink_event (GstPad * pad, GstEvent * event)
{
  GstSubParse *self = GST_SUBPARSE (gst_pad_get_parent (pad));
  gboolean ret = FALSE;

  GST_DEBUG ("Handling %s event", GST_EVENT_TYPE_NAME (event));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_EOS:{
      /* Make sure the last subrip chunk is pushed out even
       * if the file does not have an empty line at the end */
      if (self->parser_type == GST_SUB_PARSE_FORMAT_SUBRIP ||
          self->parser_type == GST_SUB_PARSE_FORMAT_TMPLAYER ||
          self->parser_type == GST_SUB_PARSE_FORMAT_MPL2) {
        GstBuffer *buf = gst_buffer_new_and_alloc (2 + 1);

        GST_DEBUG ("EOS. Pushing remaining text (if any)");
        GST_BUFFER_DATA (buf)[0] = '\n';
        GST_BUFFER_DATA (buf)[1] = '\n';
        GST_BUFFER_DATA (buf)[2] = '\0';        /* play it safe */
        GST_BUFFER_SIZE (buf) = 2;
        GST_BUFFER_OFFSET (buf) = self->offset;
        gst_sub_parse_chain (pad, buf);
      }
      ret = gst_pad_event_default (pad, event);
      break;
    }
    case GST_EVENT_NEWSEGMENT:
    {
      GstFormat format;
      gdouble rate;
      gint64 start, stop, time;
      gboolean update;

      gst_event_parse_new_segment (event, &update, &rate, &format, &start,
          &stop, &time);

      GST_DEBUG_OBJECT (self, "newsegment (%s)", gst_format_get_name (format));

      if (format == GST_FORMAT_TIME) {
        gst_segment_set_newsegment (&self->segment, update, rate, format,
            start, stop, time);
      } else {
        /* if not time format, we'll either start with a 0 timestamp anyway or
         * it's following a seek in which case we'll have saved the requested
         * seek segment and don't want to overwrite it (remember that on a seek
         * we always just seek back to the start in BYTES format and just throw
         * away all text that's before the requested position; if the subtitles
         * come from an upstream demuxer, it won't be able to handle our BYTES
         * seek request and instead send us a newsegment from the seek request
         * it received via its video pads instead, so all is fine then too) */
      }

      ret = TRUE;
      gst_event_unref (event);
      break;
    }
    case GST_EVENT_FLUSH_START:
    {
      self->flushing = TRUE;

      ret = gst_pad_event_default (pad, event);
      break;
    }
    case GST_EVENT_FLUSH_STOP:
    {
      self->flushing = FALSE;

      ret = gst_pad_event_default (pad, event);
      break;
    }
    default:
      ret = gst_pad_event_default (pad, event);
      break;
  }

  gst_object_unref (self);

  return ret;
}


static GstStateChangeReturn
gst_sub_parse_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstSubParse *self = GST_SUBPARSE (element);

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      /* format detection will init the parser state */
      self->offset = 0;
      self->next_offset = 0;
      self->parser_type = GST_SUB_PARSE_FORMAT_UNKNOWN;
      self->valid_utf8 = TRUE;
      self->first_buffer = TRUE;
      g_free (self->detected_encoding);
      self->detected_encoding = NULL;
      g_string_truncate (self->textbuf, 0);
      gst_adapter_clear (self->adapter);
      break;
    default:
      break;
  }

  ret = parent_class->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      parser_state_dispose (&self->state);
      self->parser_type = GST_SUB_PARSE_FORMAT_UNKNOWN;
      break;
    default:
      break;
  }

  return ret;
}

/*
 * Typefind support.
 */

/* FIXME 0.11: these caps are ugly, use app/x-subtitle + type field or so;
 * also, give different  subtitle formats really different types */
static GstStaticCaps mpl2_caps =
GST_STATIC_CAPS ("application/x-subtitle-mpl2");
#define SUB_CAPS (gst_static_caps_get (&sub_caps))

static GstStaticCaps tmp_caps =
GST_STATIC_CAPS ("application/x-subtitle-tmplayer");
#define TMP_CAPS (gst_static_caps_get (&tmp_caps))

static GstStaticCaps sub_caps = GST_STATIC_CAPS ("application/x-subtitle");
#define MPL2_CAPS (gst_static_caps_get (&mpl2_caps))

#ifndef GST_DISABLE_XML
static GstStaticCaps smi_caps = GST_STATIC_CAPS ("application/x-subtitle-sami");
#define SAMI_CAPS (gst_static_caps_get (&smi_caps))
#endif

static void
gst_subparse_type_find (GstTypeFind * tf, gpointer private)
{
  GstSubParseFormat format;
  const guint8 *data;
  GstCaps *caps;
  gchar *str;
  gchar *encoding = NULL;
  const gchar *end;

  if (!(data = gst_type_find_peek (tf, 0, 129)))
    return;

  /* make sure string passed to _autodetect() is NUL-terminated */
  str = g_malloc0 (129);
  memcpy (str, data, 128);

  if ((encoding = detect_encoding (str, 128)) != NULL) {
    gchar *converted_str;
    GError *err = NULL;
    gsize tmp;

    converted_str = gst_convert_to_utf8 (str, 128, encoding, &tmp, &err);
    if (converted_str == NULL) {
      GST_DEBUG ("Encoding '%s' detected but conversion failed: %s", encoding,
          err->message);
      g_error_free (err);
      g_free (encoding);
    } else {
      g_free (str);
      str = converted_str;
      g_free (encoding);
    }
  }

  /* Check if at least the first 120 chars are valid UTF8,
   * otherwise convert as always */
  if (!g_utf8_validate (str, 128, &end) && (end - str) < 120) {
    gchar *converted_str;
    GError *err = NULL;
    gsize tmp;
    const gchar *enc;

    enc = g_getenv ("GST_SUBTITLE_ENCODING");
    if (enc == NULL || *enc == '\0') {
      /* if local encoding is UTF-8 and no encoding specified
       * via the environment variable, assume ISO-8859-15 */
      if (g_get_charset (&enc)) {
        enc = "ISO-8859-15";
      }
    }
    converted_str = gst_convert_to_utf8 (str, 128, enc, &tmp, &err);
    if (converted_str == NULL) {
      GST_DEBUG ("Charset conversion failed: %s", err->message);
      g_error_free (err);
      g_free (str);
      return;
    } else {
      g_free (str);
      str = converted_str;
    }
  }

  format = gst_sub_parse_data_format_autodetect (str);
  g_free (str);

  switch (format) {
    case GST_SUB_PARSE_FORMAT_MDVDSUB:
      GST_DEBUG ("MicroDVD format detected");
      caps = SUB_CAPS;
      break;
    case GST_SUB_PARSE_FORMAT_SUBRIP:
      GST_DEBUG ("SubRip format detected");
      caps = SUB_CAPS;
      break;
    case GST_SUB_PARSE_FORMAT_MPSUB:
      GST_DEBUG ("MPSub format detected");
      caps = SUB_CAPS;
      break;
#ifndef GST_DISABLE_XML
    case GST_SUB_PARSE_FORMAT_SAMI:
      GST_DEBUG ("SAMI (time-based) format detected");
      caps = SAMI_CAPS;
      break;
#endif
    case GST_SUB_PARSE_FORMAT_TMPLAYER:
      GST_DEBUG ("TMPlayer (time based) format detected");
      caps = TMP_CAPS;
      break;
      /* FIXME: our MPL2 typefinding is not really good enough to warrant
       * returning a high probability (however, since we registered our
       * typefinder here with a rank of MARGINAL we should pretty much only
       * be called if most other typefinders have already run */
    case GST_SUB_PARSE_FORMAT_MPL2:
      GST_DEBUG ("MPL2 (time based) format detected");
      caps = MPL2_CAPS;
      break;
    case GST_SUB_PARSE_FORMAT_SUBVIEWER:
      GST_DEBUG ("SubViewer format detected");
      caps = SUB_CAPS;
      break;
    default:
    case GST_SUB_PARSE_FORMAT_UNKNOWN:
      GST_DEBUG ("no subtitle format detected");
      return;
  }

  /* if we're here, it's ok */
  gst_type_find_suggest (tf, GST_TYPE_FIND_MAXIMUM, caps);
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  static gchar *sub_exts[] = { "srt", "sub", "mpsub", "mdvd", "smi", "txt",
    NULL
  };

  GST_DEBUG_CATEGORY_INIT (sub_parse_debug, "subparse", 0, ".sub parser");

  if (!gst_type_find_register (plugin, "subparse_typefind", GST_RANK_MARGINAL,
          gst_subparse_type_find, sub_exts, SUB_CAPS, NULL, NULL))
    return FALSE;

  if (!gst_element_register (plugin, "subparse",
          GST_RANK_PRIMARY, GST_TYPE_SUBPARSE) ||
      !gst_element_register (plugin, "ssaparse",
          GST_RANK_PRIMARY, GST_TYPE_SSA_PARSE)) {
    return FALSE;
  }

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "subparse",
    "Subtitle parsing",
    plugin_init, VERSION, "LGPL", GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
