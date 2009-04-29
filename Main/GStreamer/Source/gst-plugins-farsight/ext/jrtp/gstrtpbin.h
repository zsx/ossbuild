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


#ifndef __GST_RTPBIN_H__
#define __GST_RTPBIN_H__

#include <gst/gst.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_JRTP
#include "jrtplib_c.h"
#endif

G_BEGIN_DECLS
#define GST_TYPE_RTP_BIN 		(gst_rtp_bin_get_type())
#define GST_RTP_BIN(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RTP_BIN,GstRTPBin))
#define GST_RTP_BIN_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RTP_BIN,GstRTPBinClass))
#define GST_IS_RTP_BIN(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RTP_BIN))
#define GST_IS_RTP_BIN_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RTP_BIN))

GType gst_rtp_bin_get_type (void);

typedef struct _GstRTPBin GstRTPBin;
typedef struct _GstRTPBinClass GstRTPBinClass;

struct _GstRTPBin
{
  GstBin bin;                   /* we extend GstBin */

  GstElement *rtpsend;
  GstElement *rtprecv;
  GstElement *rtpsink;
  GstElement *rtcpsink;
  GstElement *rtpsrc;
  GstElement *rtcpsrc;
  GstElement *jbuf;

  // task to run jrtpsession_poll()
  GstTask *task;

#ifdef HAVE_JRTP
  jrtpsession_t sess;
#else
  guint32       dest_ip;
  guint16       dest_port;
#endif

  gboolean      rtcp_support;
  guint         localport;
  guint         clockrate;
  guint         queue_delay;

  int           rtp_sockfd;
  int           rtcp_sockfd;

  guint8        defaultpt;
  guint         defaulttsinc;
  gboolean      defaultmark;

  GHashTable   *default_pt_map;
  GHashTable   *pt_map;
  GHashTable   *user_pt_map;

  gboolean      bypass_udp;
  gboolean 	closefd;
};

struct _GstRTPBinClass
{
  GstBinClass parent_class;
};

G_END_DECLS

#endif /* __GST_RTPBIN_H__ */
