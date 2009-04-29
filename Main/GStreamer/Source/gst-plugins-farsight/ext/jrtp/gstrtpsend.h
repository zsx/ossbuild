/*
 * Farsight
 * GStreamer RTP Send element using JRTPlib
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

#ifndef __GST_RTPSEND_H__
#define __GST_RTPSEND_H__

#include <gst/gst.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_JRTP
#include "jrtplib_c.h"
#endif

G_BEGIN_DECLS

/* #define's don't like whitespacey bits */
#define GST_TYPE_RTPSEND \
  (gst_gst_rtpsend_get_type())
#define GST_RTPSEND(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RTPSEND,GstRTPSend))
#define GST_RTPSEND_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RTPSEND,GstRTPSend))
#define GST_IS_RTPSEND(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RTPSEND))
#define GST_IS_RTPSEND_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RTPSEND))

typedef struct _GstRTPSend      GstRTPSend;
typedef struct _GstRTPSendClass GstRTPSendClass;

struct _GstRTPSend
{
  GstElement element;

  GstPad *rtpsrcpad, *rtcpsrcpad, *datasinkpad;

#ifdef HAVE_JRTP
  // This holds our RTPSession
  jrtpsession_t sess;
#endif

  gboolean silent;
  guint prev_ts;
};

struct _GstRTPSendClass 
{
  GstElementClass parent_class;
};

GType gst_gst_rtpsend_get_type (void);

G_END_DECLS

#endif /* __GST_RTPSEND_H__ */
