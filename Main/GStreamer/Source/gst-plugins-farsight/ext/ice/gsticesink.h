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

#ifndef __GST_ICESINK_H__
#define __GST_ICESINK_H__

#include <gst/gst.h>
#include <gst/netbuffer/gstnetbuffer.h>
#include <gst/base/gstbasesink.h>

G_BEGIN_DECLS

#define GST_TYPE_ICESINK \
  (gst_icesink_get_type())
#define GST_ICESINK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_ICESINK,GstIceSink))
#define GST_ICESINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ICESINK,GstIceSink))
#define GST_IS_ICESINK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_ICESINK))
#define GST_IS_ICESINK_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_ICESINK))

typedef struct _GstIceSink GstIceSink;
typedef struct _GstIceSinkClass GstIceSinkClass;

struct _GstIceSink {
  GstBaseSink parent;

  void* sockclient;
};

struct _GstIceSinkClass {
  GstBaseSinkClass parent_class;
};

GType gst_icesink_get_type(void);

G_END_DECLS

#endif /* __GST_ICESINK_H__ */
