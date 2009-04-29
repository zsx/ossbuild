/*
 * GStreamer
 * Farsight Voice+Video library
 *  Copyright 2006 Collabora Ltd, 
 *  Copyright 2006 Nokia Corporation
 *   @author: Philippe Khalaf <philippe.khalaf@collabora.co.uk>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __GST_ICESRC_H__
#define __GST_ICESRC_H__

#include <gst/gst.h>
#include <gst/netbuffer/gstnetbuffer.h>
#include <gst/base/gstpushsrc.h>

G_BEGIN_DECLS

#define GST_TYPE_ICESRC \
  (gst_icesrc_get_type())
#define GST_ICESRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_ICESRC,GstIceSrc))
#define GST_ICESRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ICESRC,GstIceSrc))
#define GST_IS_ICESRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_ICESRC))
#define GST_IS_ICESRC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_ICESRC))

typedef struct _GstIceSrc GstIceSrc;
typedef struct _GstIceSrcClass GstIceSrcClass;

struct _GstIceSrc {
  GstPushSrc parent;

  GstCaps *caps;
  GAsyncQueue *data_queue;
  GstNetAddress from;

  void * sockclient;

  guint flush_queue;

  guint initial_pops;
  gboolean initial_pops_done;
};

struct _GstIceSrcClass {
  GstPushSrcClass parent_class;
};

GType gst_icesrc_get_type(void);

G_END_DECLS

#endif /* __GST_ICESRC_H__ */
