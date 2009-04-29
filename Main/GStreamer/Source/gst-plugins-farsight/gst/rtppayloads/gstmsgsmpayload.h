/* Farsight
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


#ifndef __GST_RTPMSGSMPAYLOAD_H__
#define __GST_RTPMSGSMPAYLOAD_H__

#include <gst/gst.h>
#include <gst/rtp/gstbasertppayload.h>

G_BEGIN_DECLS

typedef struct _GstMsGSMPayload GstMsGSMPayload;
typedef struct _GstMsGSMPayloadClass GstMsGSMPayloadClass;

#define GST_TYPE_RTPMSGSMPAYLOAD \
  (gst_msgsmpayload_get_type())
#define GST_RTPMSGSMPAYLOAD(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RTPMSGSMPAYLOAD,GstMsGSMPayload))
#define GST_RTPMSGSMPAYLOAD_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RTPMSGSMPAYLOAD,GstMsGSMPayload))
#define GST_IS_RTPMSGSMPAYLOAD(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RTPMSGSMPAYLOAD))
#define GST_IS_RTPMSGSMPAYLOAD_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RTPMSGSMPAYLOAD))

struct _GstMsGSMPayload
{
  GstBaseRTPPayload payload;
};

struct _GstMsGSMPayloadClass
{
  GstBaseRTPPayloadClass parent_class;
};

gboolean gst_msgsmpayload_plugin_init (GstPlugin * plugin);

G_END_DECLS

#endif /* __GST_RTPMSGSMPAYLOAD_H__ */
