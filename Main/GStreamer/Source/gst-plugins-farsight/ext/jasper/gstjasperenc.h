/* GStreamer
 * Copyright (C) <2005> Philippe Khalaf <burger@speedy.org> 
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

#ifndef __GST_JASPERENC_H__
#define __GST_JASPERENC_H__

#include <gst/gst.h>
#include <jasper/jasper.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GST_TYPE_JASPERENC         (gst_jasperenc_get_type())
#define GST_JASPERENC(obj)         (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_JASPERENC,GstJasperEnc))
#define GST_JASPERENC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_JASPERENC,GstJasperEnc))
#define GST_IS_JASPERENC(obj)      (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_JASPERENC))
#define GST_IS_JASPERENC_CLASS(obj)(G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_JASPERENC))

typedef struct _GstJasperEnc GstJasperEnc;
typedef struct _GstJasperEncClass GstJasperEncClass;

struct _GstJasperEnc
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  gint width;
  gint height;
  gint bpp;
};

struct _GstJasperEncClass
{
  GstElementClass parent_class;
};

GType gst_jasperenc_get_type(void);

gboolean gst_jasperenc_plugin_init (GstPlugin * plugin);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GST_JASPERENC_H__ */
