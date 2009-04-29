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

#ifndef __GST_GCONF_V4L2SRC_H
#define __GST_GCONF_V4L2SRC_H

#include <gst/gst.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

G_BEGIN_DECLS

#define GST_TYPE_GCONF_V4L2SRC \
  (gst_gconf_v4l2src_get_type())
#define GST_GCONF_V4L2SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_GCONF_V4L2SRC,GstGConfV4L2Src))
#define GST_GCONF_V4L2SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_GCONF_V4L2SRC,GstGConfV4L2SrcClass))
#define GST_IS_GCONF_V4L2SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_GCONF_V4L2SRC))
#define GST_IS_GCONF_V4L2SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_GCONF_V4L2SRC))

GType gst_gconf_v4l2src_get_type (void);

typedef struct _GstGConfV4L2Src GstGConfV4L2Src;
typedef struct _GstGConfV4L2SrcClass GstGConfV4L2SrcClass;

typedef struct _GstGConfV4L2SrcPrivate GstGConfV4L2SrcPrivate;

struct _GstGConfV4L2Src
{
  GstBin bin;                   /* we extend GstBin */
  GstGConfV4L2SrcPrivate *priv;
};

struct _GstGConfV4L2SrcClass
{
  GstBinClass parent_class;
};

G_END_DECLS

#endif /* __GST_GCONF_V4L2SRC_H */
