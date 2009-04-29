/*
 * Farsight
 * GStreamer RTP Receive element using JRTPlib
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

#ifndef __GST_RTPRECV_H__
#define __GST_RTPRECV_H__

#include <gst/gst.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_JRTP
#include "jrtplib_c.h"
#endif

G_BEGIN_DECLS

/* #define's don't like whitespacey bits */
#define GST_TYPE_RTPRECV \
  (gst_gst_rtprecv_get_type())
#define GST_RTPRECV(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RTPRECV,GstRTPRecv))
#define GST_RTPRECV_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RTPRECV,GstRTPRecv))
#define GST_IS_RTPRECV(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RTPRECV))
#define GST_IS_RTPRECV_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RTPRECV))

typedef struct _GstRTPRecv      GstRTPRecv;
typedef struct _GstRTPRecvClass GstRTPRecvClass;

struct _GstRTPRecv
{
  GstElement element;

  GstPad *rtpsinkpad, *rtcpsinkpad, *datasrcpad;

#ifdef HAVE_JRTP
  // This holds our RTPSession
  jrtpsession_t sess;
#endif
  GMutex *mutex;
  gboolean silent;
  GHashTable *pt_map;
  GMutex *pt_map_mutex;
};

struct _GstRTPRecvClass 
{
  GstElementClass parent_class;
};

GType gst_gst_rtprecv_get_type (void);

G_END_DECLS

#endif /* __GST_RTPRECV_H__ */
