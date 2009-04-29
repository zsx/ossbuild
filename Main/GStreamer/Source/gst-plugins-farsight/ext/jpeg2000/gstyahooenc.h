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


#ifndef __GST_YAHOOENC_H__
#define __GST_YAHOOENC_H__


#include <gst/gst.h>
#include <gst/base/gstadapter.h>

#define GST_TYPE_YAHOOENC \
  (gst_yahooenc_get_type())
#define GST_YAHOOENC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_YAHOOENC,GstYahooEnc))
#define GST_YAHOOENC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_YAHOOENC,GstYahooEnc))
#define GST_IS_YAHOOENC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_YAHOOENC))
#define GST_IS_YAHOOENC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_YAHOOENC))

typedef struct _GstYahooEnc GstYahooEnc;
typedef struct _GstYahooEncClass GstYahooEncClass;

struct _GstYahooEnc {
  GstElement element;

  /* pads */
  GstPad *sinkpad,*srcpad;

  GRand *ts_rand;
  GstClockTime prev_ts;
  guint32 cur_ts;
};

struct _GstYahooEncClass {
  GstElementClass parent_class;
};

GType gst_yahooenc_get_type(void);

gboolean gst_yahooenc_plugin_init (GstPlugin * plugin);

#endif /* __GST_YAHOOENC_H__ */
