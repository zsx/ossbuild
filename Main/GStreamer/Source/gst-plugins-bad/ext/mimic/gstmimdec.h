/*
 * GStreamer
 * Copyright (c) 2005 INdT.
 * @author Andre Moreira Magalhaes <andre.magalhaes@indt.org.br>
 * @author Philippe Khalaf <burger@speedy.org>
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

#ifndef __GST_MIMDEC_H__
#define __GST_MIMDEC_H__

#include <gst/gst.h>
#include <gst/base/gstadapter.h>
#include <mimic.h>

G_BEGIN_DECLS
#define GST_TYPE_MIMDEC \
  (gst_mimdec_get_type())
#define GST_MIMDEC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MIMDEC,GstMimDec))
#define GST_MIMDEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_MIMDEC,GstMimDec))
#define GST_IS_MIMDEC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_MIMDEC))
#define GST_IS_MIMDEC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_MIMDEC))
typedef struct _GstMimDec GstMimDec;
typedef struct _GstMimDecClass GstMimDecClass;

struct _GstMimDec
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  /* Protected by stream lock */
  GstAdapter *adapter;

  /* Protected by object lock */
  MimCtx *dec;

  gint buffer_size;
  gboolean have_header;
  guint32 payload_size;
  guint32 current_ts;

  gboolean need_newsegment;
};

struct _GstMimDecClass
{
  GstElementClass parent_class;
};

GType gst_mimdec_get_type (void);

G_END_DECLS
#endif /* __GST_MIMDEC_H__ */
