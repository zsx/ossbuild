/* Farsight
 * Copyright (C) 2007 Nokia Corporation
 * Copyright (C) 2007 Collabora Ltd
 *  @author: Olivier Crete <olivier.crete@collabora.co.uk>
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


#ifndef __GST_RTPCNDEPAYLOAD_H__
#define __GST_RTPCNDEPAYLOAD_H__

#include <gst/gst.h>
#include <gst/rtp/gstbasertpdepayload.h>

G_BEGIN_DECLS

typedef struct _GstRTPCNDepayload GstRTPCNDepayload;
typedef struct _GstRTPCNDepayloadClass GstRTPCNDepayloadClass;

#define GST_TYPE_RTPCNDEPAYLOAD \
  (gst_rtpcndepayload_get_type())
#define GST_RTPCNDEPAYLOAD(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RTPCNDEPAYLOAD,GstRTPCNDepayload))
#define GST_RTPCNDEPAYLOAD_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RTPCNDEPAYLOAD,GstRTPCNDepayload))
#define GST_IS_RTPCNDEPAYLOAD(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RTPCNDEPAYLOAD))
#define GST_IS_RTPCNDEPAYLOAD_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RTPCNDEPAYLOAD))

struct _GstRTPCNDepayload
{
  GstBaseRTPDepayload depayload;
};

struct _GstRTPCNDepayloadClass
{
  GstBaseRTPDepayloadClass parent_class;
};

gboolean gst_rtpcndepayload_plugin_init (GstPlugin * plugin);

G_END_DECLS

#endif /* __GST_RTPCNDEPAYLOAD_H__ */
