/* GStreamer
 * Copyright (C) 2005 Philippe Khalaf <burger@speedy.org>
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


#ifndef __GST_YAHOOPARSE_H__
#define __GST_YAHOOPARSE_H__


#include <gst/gst.h>
#include <gst/base/gstadapter.h>

#define GST_TYPE_YAHOOPARSE \
  (gst_yahooparse_get_type())
#define GST_YAHOOPARSE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_YAHOOPARSE,GstYahooParse))
#define GST_YAHOOPARSE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_YAHOOPARSE,GstYahooParse))
#define GST_IS_YAHOOPARSE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_YAHOOPARSE))
#define GST_IS_YAHOOPARSE_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_YAHOOPARSE))

typedef struct _GstYahooParse GstYahooParse;
typedef struct _GstYahooParseClass GstYahooParseClass;

struct _GstYahooParse {
  GstElement element;

  /* pads */
  GstPad *sinkpad,*srcpad;

  GstAdapter *adapter;

  gboolean have_header;

  guchar packet_type;
  guint payload_size;
  guint timestamp;
};

struct _GstYahooParseClass {
  GstElementClass parent_class;
};

GType gst_yahooparse_get_type(void);

gboolean gst_yahooparse_plugin_init (GstPlugin * plugin);

#endif /* __GST_YAHOOPARSE_H__ */
