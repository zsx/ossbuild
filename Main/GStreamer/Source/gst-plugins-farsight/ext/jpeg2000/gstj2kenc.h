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


#ifndef __GST_JPEG2000ENC_H__
#define __GST_JPEG2000ENC_H__


#include <gst/gst.h>
#include <gst/base/gstadapter.h>
#include <j2k.h>

#define GST_TYPE_JPEG2000ENC \
  (gst_jpeg2000enc_get_type())
#define GST_JPEG2000ENC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_JPEG2000ENC,GstJpeg2000Enc))
#define GST_JPEG2000ENC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_JPEG2000ENC,GstJpeg2000Enc))
#define GST_IS_JPEG2000ENC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_JPEG2000ENC))
#define GST_IS_JPEG2000ENC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_JPEG2000ENC))

typedef struct _GstJpeg2000Enc GstJpeg2000Enc;
typedef struct _GstJpeg2000EncClass GstJpeg2000EncClass;

struct _GstJpeg2000Enc {
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
};

struct _GstJpeg2000EncClass {
  GstElementClass parent_class;
};

GType gst_jpeg2000enc_get_type(void);

gboolean gst_jpeg2000enc_plugin_init (GstPlugin * plugin);

#endif /* __GST_JPEG2000ENC_H__ */
