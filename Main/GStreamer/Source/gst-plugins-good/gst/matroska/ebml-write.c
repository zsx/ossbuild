/* GStreamer EBML I/O
 * (c) 2003 Ronald Bultje <rbultje@ronald.bitfreak.net>
 * (c) 2005 Michal Benes <michal.benes@xeris.cz>
 *
 * ebml-write.c: write EBML data to file/stream
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
#include <gst/floatcast/floatcast.h>

#include "ebml-write.h"
#include "ebml-ids.h"


GST_DEBUG_CATEGORY_STATIC (gst_ebml_write_debug);
#define GST_CAT_DEFAULT gst_ebml_write_debug

#define _do_init(thing) \
      GST_DEBUG_CATEGORY_INIT (gst_ebml_write_debug, "GstEbmlWrite", 0, "Write EBML structured data")
GST_BOILERPLATE_FULL (GstEbmlWrite, gst_ebml_write, GstObject, GST_TYPE_OBJECT,
    _do_init);

static void gst_ebml_write_finalize (GObject * object);


static void
gst_ebml_write_base_init (gpointer g_class)
{
}

static void
gst_ebml_write_class_init (GstEbmlWriteClass * klass)
{
  GObjectClass *object = G_OBJECT_CLASS (klass);

  object->finalize = gst_ebml_write_finalize;
}

static void
gst_ebml_write_init (GstEbmlWrite * ebml, GstEbmlWriteClass * klass)
{
  ebml->srcpad = NULL;
  ebml->pos = 0;

  ebml->cache = NULL;
  ebml->cache_size = 0;
}

static void
gst_ebml_write_finalize (GObject * object)
{
  GstEbmlWrite *ebml = GST_EBML_WRITE (object);

  gst_object_unref (ebml->srcpad);

  if (ebml->cache) {
    gst_buffer_unref (ebml->cache);
    ebml->cache = NULL;
  }

  GST_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}


/**
 * gst_ebml_write_new:
 * @srcpad: Source pad to which the output will be pushed.
 *
 * Creates a new #GstEbmlWrite.
 *
 * Returns: a new #GstEbmlWrite
 */
GstEbmlWrite *
gst_ebml_write_new (GstPad * srcpad)
{
  GstEbmlWrite *ebml =
      GST_EBML_WRITE (g_object_new (GST_TYPE_EBML_WRITE, NULL));

  ebml->srcpad = gst_object_ref (srcpad);
  ebml->timestamp = GST_CLOCK_TIME_NONE;

  gst_ebml_write_reset (ebml);

  return ebml;
}


/**
 * gst_ebml_write_reset:
 * @ebml: a #GstEbmlWrite.
 *
 * Reset internal state of #GstEbmlWrite.
 */
void
gst_ebml_write_reset (GstEbmlWrite * ebml)
{
  ebml->pos = 0;

  if (ebml->cache) {
    gst_buffer_unref (ebml->cache);
    ebml->cache = NULL;
  }
  ebml->cache_size = 0;
  ebml->last_write_result = GST_FLOW_OK;
  ebml->timestamp = GST_CLOCK_TIME_NONE;
  ebml->need_newsegment = TRUE;
}


/**
 * gst_ebml_last_write_result:
 * @ebml: a #GstEbmlWrite.
 *
 * Returns: GST_FLOW_OK if there was not write error since the last call of
 *          gst_ebml_last_write_result or code of the error.
 */
GstFlowReturn
gst_ebml_last_write_result (GstEbmlWrite * ebml)
{
  GstFlowReturn res = ebml->last_write_result;

  ebml->last_write_result = GST_FLOW_OK;

  return res;
}


/**
 * gst_ebml_write_set_cache:
 * @ebml: a #GstEbmlWrite.
 * @size: size of the cache.
 * Create a cache.
 *
 * The idea is that you use this for writing a lot
 * of small elements. This will just "queue" all of
 * them and they'll be pushed to the next element all
 * at once. This saves memory and time for buffer
 * allocation and init, and it looks better.
 */
void
gst_ebml_write_set_cache (GstEbmlWrite * ebml, guint size)
{
  /* FIXME: This is currently broken. I don't know why yet. */
  return;

  g_return_if_fail (ebml->cache == NULL);

  ebml->cache = gst_buffer_new_and_alloc (size);
  ebml->cache_size = size;
  GST_BUFFER_SIZE (ebml->cache) = 0;
  GST_BUFFER_OFFSET (ebml->cache) = ebml->pos;
  ebml->handled = 0;
}

/**
 * gst_ebml_write_flush_cache:
 * @ebml: a #GstEbmlWrite.
 *
 * Flush the cache.
 */
void
gst_ebml_write_flush_cache (GstEbmlWrite * ebml)
{
  if (!ebml->cache)
    return;

  /* this is very important. It may fail, in which case the client
   * programmer didn't use the cache somewhere. That's fatal. */
  g_assert (ebml->handled == GST_BUFFER_SIZE (ebml->cache));
  g_assert (GST_BUFFER_SIZE (ebml->cache) +
      GST_BUFFER_OFFSET (ebml->cache) == ebml->pos);

  if (ebml->last_write_result == GST_FLOW_OK) {
    if (ebml->need_newsegment) {
      GstEvent *ev;

      g_assert (ebml->handled == 0);
      ev = gst_event_new_new_segment (FALSE, 1.0, GST_FORMAT_BYTES, 0, -1, 0);
      if (gst_pad_push_event (ebml->srcpad, ev))
        ebml->need_newsegment = FALSE;
    }
    ebml->last_write_result = gst_pad_push (ebml->srcpad, ebml->cache);
  }

  ebml->cache = NULL;
  ebml->cache_size = 0;
  ebml->handled = 0;
}


/**
 * gst_ebml_write_element_new:
 * @ebml: a #GstEbmlWrite.
 * @size: size of the requested buffer.
 *
 * Create a buffer for one element. If there is
 * a cache, use that instead.
 *
 * Returns: A new #GstBuffer.
 */
static GstBuffer *
gst_ebml_write_element_new (GstEbmlWrite * ebml, guint size)
{
  /* Create new buffer of size + ID + length */
  GstBuffer *buf;

  /* length, ID */
  size += 12;

  /* prefer cache */
  if (ebml->cache) {
    if (ebml->cache_size - GST_BUFFER_SIZE (ebml->cache) < size) {
      GST_LOG ("Cache available, but too small. Clearing...");
      gst_ebml_write_flush_cache (ebml);
    } else {
      return ebml->cache;
    }
  }

  /* else, use a one-element buffer. This is slower */
  buf = gst_buffer_new_and_alloc (size);
  GST_BUFFER_SIZE (buf) = 0;
  GST_BUFFER_TIMESTAMP (buf) = ebml->timestamp;

  return buf;
}


/**
 * gst_ebml_write_element_id:
 * @buf: Buffer to which id should be written.
 * @id: Element ID that should be written.
 * 
 * Write element ID into a buffer.
 */
static void
gst_ebml_write_element_id (GstBuffer * buf, guint32 id)
{
  guint8 *data = GST_BUFFER_DATA (buf) + GST_BUFFER_SIZE (buf);
  guint bytes = 4, mask = 0x10;

  /* get ID length */
  while (!(id & (mask << ((bytes - 1) * 8))) && bytes > 0) {
    mask <<= 1;
    bytes--;
  }

  /* if invalid ID, use dummy */
  if (bytes == 0) {
    GST_WARNING ("Invalid ID, voiding");
    bytes = 1;
    id = GST_EBML_ID_VOID;
  }

  /* write out, BE */
  GST_BUFFER_SIZE (buf) += bytes;
  while (bytes--) {
    data[bytes] = id & 0xff;
    id >>= 8;
  }
}


/**
 * gst_ebml_write_element_size:
 * @buf: #GstBuffer to which size should be written.
 * @size: Element length.
 * 
 * Write element length into a buffer.
 */
static void
gst_ebml_write_element_size (GstBuffer * buf, guint64 size)
{
  guint8 *data = GST_BUFFER_DATA (buf) + GST_BUFFER_SIZE (buf);
  guint bytes = 1, mask = 0x80;

  if (size != GST_EBML_SIZE_UNKNOWN) {
    /* how many bytes? - use mask-1 because an all-1 bitset is not allowed */
    while ((size >> ((bytes - 1) * 8)) >= (mask - 1) && bytes <= 8) {
      mask >>= 1;
      bytes++;
    }

    /* if invalid size, use max. */
    if (bytes > 8) {
      GST_WARNING ("Invalid size, writing size unknown");
      mask = 0x01;
      bytes = 8;
      /* Now here's a real FIXME: we cannot read those yet! */
      size = GST_EBML_SIZE_UNKNOWN;
    }
  } else {
    mask = 0x01;
    bytes = 8;
  }

  /* write out, BE, with length size marker */
  GST_BUFFER_SIZE (buf) += bytes;
  while (bytes-- > 0) {
    data[bytes] = size & 0xff;
    size >>= 8;
    if (!bytes)
      *data |= mask;
  }
}


/**
 * gst_ebml_write_element_data:
 * @buf: #GstBuffer to which data should be written.
 * @write: Data that should be written.
 * @length: Length of the data.
 *
 * Write element data into a buffer.
 */
static void
gst_ebml_write_element_data (GstBuffer * buf, guint8 * write, guint64 length)
{
  guint8 *data = GST_BUFFER_DATA (buf) + GST_BUFFER_SIZE (buf);

  memcpy (data, write, length);
  GST_BUFFER_SIZE (buf) += length;
}


/**
 * gst_ebml_write_element_push:
 * @ebml: #GstEbmlWrite
 * @buf: #GstBuffer to be written.
 * 
 * Write out buffer by moving it to the next element.
 */
static void
gst_ebml_write_element_push (GstEbmlWrite * ebml, GstBuffer * buf)
{
  guint data_size = GST_BUFFER_SIZE (buf) - ebml->handled;

  ebml->pos += data_size;
  if (buf == ebml->cache) {
    ebml->handled += data_size;
  }

  /* if there's no cache, then don't push it! */
  if (ebml->cache) {
    g_assert (buf == ebml->cache);
    return;
  }

  if (ebml->last_write_result == GST_FLOW_OK) {
    if (ebml->need_newsegment) {
      GstEvent *ev;

      g_assert (ebml->handled == 0);
      ev = gst_event_new_new_segment (FALSE, 1.0, GST_FORMAT_BYTES, 0, -1, 0);
      if (gst_pad_push_event (ebml->srcpad, ev))
        ebml->need_newsegment = FALSE;
    }
    buf = gst_buffer_make_metadata_writable (buf);
    gst_buffer_set_caps (buf, GST_PAD_CAPS (ebml->srcpad));
    ebml->last_write_result = gst_pad_push (ebml->srcpad, buf);
  } else {
    if (buf != ebml->cache)
      gst_buffer_unref (buf);
  }
}


/**
 * gst_ebml_write_seek:
 * @ebml: #GstEbmlWrite
 * @pos: Seek position.
 * 
 * Seek.
 */
void
gst_ebml_write_seek (GstEbmlWrite * ebml, guint64 pos)
{
  GstEvent *event;

  /* Cache seeking. A bit dangerous, we assume the client writer
   * knows what he's doing... */
  if (ebml->cache) {
    /* within bounds? */
    if (pos >= GST_BUFFER_OFFSET (ebml->cache) &&
        pos < GST_BUFFER_OFFSET (ebml->cache) + ebml->cache_size) {
      GST_BUFFER_SIZE (ebml->cache) = pos - GST_BUFFER_OFFSET (ebml->cache);
      if (ebml->pos > pos)
        ebml->handled -= ebml->pos - pos;
      else
        ebml->handled += pos - ebml->pos;
      ebml->pos = pos;
    } else {
      GST_LOG ("Seek outside cache range. Clearing...");
      gst_ebml_write_flush_cache (ebml);
    }
  }

  event = gst_event_new_new_segment (FALSE, 1.0, GST_FORMAT_BYTES, pos, -1, 0);
  if (gst_pad_push_event (ebml->srcpad, event)) {
    GST_DEBUG ("Seek'd to offset %" G_GUINT64_FORMAT, pos);
  } else {
    GST_WARNING ("Seek to offset %" G_GUINT64_FORMAT " failed", pos);
  }
  ebml->pos = pos;
}


/**
 * gst_ebml_write_get_uint_size:
 * @num: Number to be encoded.
 * 
 * Get number of bytes needed to write a uint.
 *
 * Returns: Encoded uint length.
 */
static guint
gst_ebml_write_get_uint_size (guint64 num)
{
  guint size = 1;

  /* get size */
  while (num >= (G_GINT64_CONSTANT (1) << (size * 8)) && size < 8) {
    size++;
  }

  return size;
}


/**
 * gst_ebml_write_set_uint:
 * @buf: #GstBuffer to which ithe number should be written.
 * @num: Number to be written.
 * @size: Encoded number length.
 *
 * Write an uint into a buffer.
 */
static void
gst_ebml_write_set_uint (GstBuffer * buf, guint64 num, guint size)
{
  guint8 *data;

  data = GST_BUFFER_DATA (buf) + GST_BUFFER_SIZE (buf);
  GST_BUFFER_SIZE (buf) += size;
  while (size-- > 0) {
    data[size] = num & 0xff;
    num >>= 8;
  }
}


/**
 * gst_ebml_write_uint:
 * @ebml: #GstEbmlWrite
 * @id: Element ID.
 * @num: Number to be written.
 *
 * Write uint element.
 */
void
gst_ebml_write_uint (GstEbmlWrite * ebml, guint32 id, guint64 num)
{
  GstBuffer *buf = gst_ebml_write_element_new (ebml, sizeof (num));
  guint size = gst_ebml_write_get_uint_size (num);

  /* write */
  gst_ebml_write_element_id (buf, id);
  gst_ebml_write_element_size (buf, size);
  gst_ebml_write_set_uint (buf, num, size);
  gst_ebml_write_element_push (ebml, buf);
}


/**
 * gst_ebml_write_sint:
 * @ebml: #GstEbmlWrite
 * @id: Element ID.
 * @num: Number to be written.
 *
 * Write sint element.
 */
void
gst_ebml_write_sint (GstEbmlWrite * ebml, guint32 id, gint64 num)
{
  GstBuffer *buf = gst_ebml_write_element_new (ebml, sizeof (num));

  /* if the signed number is on the edge of a extra-byte,
   * then we'll fall over when detecting it. Example: if I
   * have a number (-)0x8000 (G_MINSHORT), then my abs()<<1
   * will be 0x10000; this is G_MAXUSHORT+1! So: if (<0) -1. */
  guint64 unum = (num < 0 ? (-num - 1) << 1 : num << 1);
  guint size = gst_ebml_write_get_uint_size (unum);

  /* make unsigned */
  if (num >= 0) {
    unum = num;
  } else {
    unum = 0x80 << (size - 1);
    unum += num;
    unum |= 0x80 << (size - 1);
  }

  /* write */
  gst_ebml_write_element_id (buf, id);
  gst_ebml_write_element_size (buf, size);
  gst_ebml_write_set_uint (buf, unum, size);
  gst_ebml_write_element_push (ebml, buf);
}


/**
 * gst_ebml_write_float:
 * @ebml: #GstEbmlWrite
 * @id: Element ID.
 * @num: Number to be written.
 *
 * Write float element.
 */
void
gst_ebml_write_float (GstEbmlWrite * ebml, guint32 id, gdouble num)
{
  GstBuffer *buf = gst_ebml_write_element_new (ebml, sizeof (num));

  gst_ebml_write_element_id (buf, id);
  gst_ebml_write_element_size (buf, 8);
  num = GDOUBLE_TO_BE (num);
  gst_ebml_write_element_data (buf, (guint8 *) & num, 8);
  gst_ebml_write_element_push (ebml, buf);
}


/**
 * gst_ebml_write_ascii:
 * @ebml: #GstEbmlWrite
 * @id: Element ID.
 * @str: String to be written.
 *
 * Write string element.
 */
void
gst_ebml_write_ascii (GstEbmlWrite * ebml, guint32 id, const gchar * str)
{
  gint len = strlen (str) + 1;  /* add trailing '\0' */
  GstBuffer *buf = gst_ebml_write_element_new (ebml, len);

  gst_ebml_write_element_id (buf, id);
  gst_ebml_write_element_size (buf, len);
  gst_ebml_write_element_data (buf, (guint8 *) str, len);
  gst_ebml_write_element_push (ebml, buf);
}


/**
 * gst_ebml_write_utf8:
 * @ebml: #GstEbmlWrite
 * @id: Element ID.
 * @str: String to be written.
 *
 * Write utf8 encoded string element.
 */
void
gst_ebml_write_utf8 (GstEbmlWrite * ebml, guint32 id, const gchar * str)
{
  gst_ebml_write_ascii (ebml, id, str);
}


/**
 * gst_ebml_write_date:
 * @ebml: #GstEbmlWrite
 * @id: Element ID.
 * @date: Date in seconds since the unix epoch.
 *
 * Write date element.
 */
void
gst_ebml_write_date (GstEbmlWrite * ebml, guint32 id, gint64 date)
{
  gst_ebml_write_sint (ebml, id, (date - GST_EBML_DATE_OFFSET) * GST_SECOND);
}

/**
 * gst_ebml_write_master_start:
 * @ebml: #GstEbmlWrite
 * @id: Element ID.
 *
 * Start wiriting mater element.
 *
 * Master writing is annoying. We use a size marker of
 * the max. allowed length, so that we can later fill it
 * in validly. 
 *
 * Returns: Master starting position.
 */
guint64
gst_ebml_write_master_start (GstEbmlWrite * ebml, guint32 id)
{
  guint64 pos = ebml->pos, t;
  GstBuffer *buf = gst_ebml_write_element_new (ebml, 0);

  t = GST_BUFFER_SIZE (buf);
  gst_ebml_write_element_id (buf, id);
  pos += GST_BUFFER_SIZE (buf) - t;
  gst_ebml_write_element_size (buf, GST_EBML_SIZE_UNKNOWN);
  gst_ebml_write_element_push (ebml, buf);

  return pos;
}


/**
 * gst_ebml_write_master_finish:
 * @ebml: #GstEbmlWrite
 * @startpos: Master starting position.
 *
 * Finish writing master element.
 */
void
gst_ebml_write_master_finish (GstEbmlWrite * ebml, guint64 startpos)
{
  guint64 pos = ebml->pos;
  GstBuffer *buf;

  gst_ebml_write_seek (ebml, startpos);
  buf = gst_ebml_write_element_new (ebml, 0);
  startpos =
      GUINT64_TO_BE ((G_GINT64_CONSTANT (1) << 56) | (pos - startpos - 8));
  memcpy (GST_BUFFER_DATA (buf) + GST_BUFFER_SIZE (buf), (guint8 *) & startpos,
      8);
  GST_BUFFER_SIZE (buf) += 8;
  gst_ebml_write_element_push (ebml, buf);
  gst_ebml_write_seek (ebml, pos);
}


/**
 * gst_ebml_write_binary:
 * @ebml: #GstEbmlWrite
 * @id: Element ID.
 * @binary: Data to be written.
 * @length: Length of the data
 *
 * Write an element with binary data.
 */
void
gst_ebml_write_binary (GstEbmlWrite * ebml,
    guint32 id, guint8 * binary, guint64 length)
{
  GstBuffer *buf = gst_ebml_write_element_new (ebml, length);

  gst_ebml_write_element_id (buf, id);
  gst_ebml_write_element_size (buf, length);
  gst_ebml_write_element_data (buf, binary, length);
  gst_ebml_write_element_push (ebml, buf);
}


/**
 * gst_ebml_write_buffer_header:
 * @ebml: #GstEbmlWrite
 * @id: Element ID.
 * @length: Length of the data
 * 
 * Write header of the binary element (use with gst_ebml_write_buffer function).
 * 
 * For things like video frames and audio samples,
 * you want to use this function, as it doesn't have
 * the overhead of memcpy() that other functions
 * such as write_binary() do have.
 */
void
gst_ebml_write_buffer_header (GstEbmlWrite * ebml, guint32 id, guint64 length)
{
  GstBuffer *buf = gst_ebml_write_element_new (ebml, 0);

  gst_ebml_write_element_id (buf, id);
  gst_ebml_write_element_size (buf, length);
  gst_ebml_write_element_push (ebml, buf);
}


/**
 * gst_ebml_write_buffer:
 * @ebml: #GstEbmlWrite
 * @data: #GstBuffer cointaining the data.
 *
 * Write  binary element (see gst_ebml_write_buffer_header).
 */
void
gst_ebml_write_buffer (GstEbmlWrite * ebml, GstBuffer * data)
{
  gst_ebml_write_element_push (ebml, data);
}


/**
 * gst_ebml_replace_uint:
 * @ebml: #GstEbmlWrite
 * @pos: Position of the uint that should be replaced.
 * @num: New value.
 *
 * Replace uint with a new value.
 * 
 * When replacing a uint, we assume that it is *always*
 * 8-byte, since that's the safest guess we can do. This
 * is just for simplicity.
 *
 * FIXME: this function needs to be replaced with something
 * proper. This is a crude hack.
 */
void
gst_ebml_replace_uint (GstEbmlWrite * ebml, guint64 pos, guint64 num)
{
  guint64 oldpos = ebml->pos;
  GstBuffer *buf = gst_buffer_new_and_alloc (8);

  gst_ebml_write_seek (ebml, pos);
  GST_BUFFER_SIZE (buf) = 0;
  gst_ebml_write_set_uint (buf, num, 8);
  gst_ebml_write_element_push (ebml, buf);
  gst_ebml_write_seek (ebml, oldpos);
}

/**
 * gst_ebml_write_header:
 * @ebml: #GstEbmlWrite
 * @doctype: Document type.
 * @version: Document type version.
 * 
 * Write EBML header.
 */
void
gst_ebml_write_header (GstEbmlWrite * ebml, gchar * doctype, guint version)
{
  guint64 pos;

  /* write the basic EBML header */
  gst_ebml_write_set_cache (ebml, 0x40);
  pos = gst_ebml_write_master_start (ebml, GST_EBML_ID_HEADER);
#if (GST_EBML_VERSION != 1)
  gst_ebml_write_uint (ebml, GST_EBML_ID_EBMLVERSION, GST_EBML_VERSION);
  gst_ebml_write_uint (ebml, GST_EBML_ID_EBMLREADVERSION, GST_EBML_VERSION);
#endif
#if 0
  /* we don't write these until they're "non-default" (never!) */
  gst_ebml_write_uint (ebml, GST_EBML_ID_EBMLMAXIDLENGTH, sizeof (guint32));
  gst_ebml_write_uint (ebml, GST_EBML_ID_EBMLMAXSIZELENGTH, sizeof (guint64));
#endif
  gst_ebml_write_ascii (ebml, GST_EBML_ID_DOCTYPE, doctype);
  gst_ebml_write_uint (ebml, GST_EBML_ID_DOCTYPEVERSION, version);
  gst_ebml_write_uint (ebml, GST_EBML_ID_DOCTYPEREADVERSION, version);
  gst_ebml_write_master_finish (ebml, pos);
  gst_ebml_write_flush_cache (ebml);
}
