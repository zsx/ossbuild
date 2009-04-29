/*
 * Farsight
 * GStreamer msGSM decoder (uses WAV49 compiled libgsm)
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

#ifndef __GST_MSGSMDEC_H__
#define __GST_MSGSMDEC_H__

#include <gst/gst.h>
#include <gst/base/gstadapter.h>

#ifdef GSM_HEADER_IN_SUBDIR
#include <gsm/gsm.h>
#else
#include <gsm.h>
#endif

G_BEGIN_DECLS

#define GST_TYPE_MSGSMDEC \
  (gst_msgsmdec_get_type())
#define GST_MSGSMDEC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MSGSMDEC,GstMSGSMDec))
#define GST_MSGSMDEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_MSGSMDEC,GstMSGSMDec))
#define GST_IS_MSGSMDEC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_MSGSMDEC))
#define GST_IS_MSGSMDEC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_MSGSMDEC))

typedef struct _GstMSGSMDec GstMSGSMDec;
typedef struct _GstMSGSMDecClass GstMSGSMDecClass;

struct _GstMSGSMDec
{
  GstElement element;

  /* pads */
  GstPad *sinkpad, *srcpad;
  GstAdapter *adapter;

  gsm state;
  gint64 next_of;
};

struct _GstMSGSMDecClass
{
  GstElementClass parent_class;
};

GType gst_msgsmdec_get_type (void);

G_END_DECLS

#endif /* __GST_MSGSMDEC_H__ */
