/* GStreamer
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

#ifndef __GST_MSGSMENC_H__
#define __GST_MSGSMENC_H__

#include <gst/gst.h>
#include <gst/base/gstadapter.h>

#ifdef GSM_HEADER_IN_SUBDIR
#include <gsm/gsm.h>
#else
#include <gsm.h>
#endif

G_BEGIN_DECLS

#define GST_TYPE_MSGSMENC \
  (gst_msgsmenc_get_type())
#define GST_MSGSMENC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MSGSMENC,GstMSGSMEnc))
#define GST_MSGSMENC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_MSGSMENC,GstMSGSMEnc))
#define GST_IS_MSGSMENC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_MSGSMENC))
#define GST_IS_MSGSMENC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_MSGSMENC))

typedef struct _GstMSGSMEnc GstMSGSMEnc;
typedef struct _GstMSGSMEncClass GstMSGSMEncClass;

struct _GstMSGSMEnc
{
  GstElement element;

  /* pads */
  GstPad *sinkpad, *srcpad;
  GstAdapter *adapter;

  gsm state;
  GstClockTime next_ts;
};

struct _GstMSGSMEncClass
{
  GstElementClass parent_class;
};

GType gst_msgsmenc_get_type (void);

G_END_DECLS

#endif /* __GST_MSGSMENC_H__ */
