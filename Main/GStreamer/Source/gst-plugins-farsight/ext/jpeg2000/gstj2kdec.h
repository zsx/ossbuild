/* GStreamer
 * Copyright (C) 2005 Philippe Khalaf <burger@speedy.org>
 * Copyright (C) 2004 Tim Ringenbach <omarvo@hotmail.com>
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
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


#ifndef __GST_JPEG2000DEC_H__
#define __GST_JPEG2000DEC_H__


#include <gst/gst.h>
#include <j2k.h>

#define GST_TYPE_JPEG2000DEC \
  (gst_jpeg2000dec_get_type())
#define GST_JPEG2000DEC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_JPEG2000DEC,GstJpeg2000Dec))
#define GST_JPEG2000DEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_JPEG2000DEC,GstJpeg2000Dec))
#define GST_IS_JPEG2000DEC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_JPEG2000DEC))
#define GST_IS_JPEG2000DEC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_JPEG2000DEC))

typedef struct _GstJpeg2000Dec GstJpeg2000Dec;
typedef struct _GstJpeg2000DecClass GstJpeg2000DecClass;

struct _GstJpeg2000Dec {
  GstElement element;

  /* pads */
  GstPad *sinkpad,*srcpad;

  /* video state */
  gint width;
  gint height;
  gdouble fps;
  gint bpp;
  gint depth;
  gint endianness;
  gint red_mask, green_mask, blue_mask;
  gint red_loss, green_loss, blue_loss;
  gint red_shift, green_shift, blue_shift;
};

struct _GstJpeg2000DecClass {
  GstElementClass parent_class;
};

GType gst_jpeg2000dec_get_type(void);

gboolean gst_jpeg2000dec_plugin_init (GstPlugin * plugin);

#endif /* __GST_JPEG2000DEC_H__ */
