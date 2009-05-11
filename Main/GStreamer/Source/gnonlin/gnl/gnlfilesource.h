/* GStreamer
 * Copyright (C) 2001 Wim Taymans <wim.taymans@gmail.com>
 *               2004-2008 Edward Hervey <bilboed@bilboed.com>
 *
 * gnlfilesource.h: Header for GnlFileSource
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


#ifndef __GNL_FILESOURCE_H__
#define __GNL_FILESOURCE_H__

#include <gst/gst.h>
#include "gnlobject.h"

G_BEGIN_DECLS
#define GNL_TYPE_FILESOURCE \
  (gnl_filesource_get_type())
#define GNL_FILESOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GNL_TYPE_FILESOURCE,GnlFileSource))
#define GNL_FILESOURCE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GNL_TYPE_FILESOURCE,GnlFileSourceClass))
#define GNL_IS_FILESOURCE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GNL_TYPE_FILESOURCE))
#define GNL_IS_FILESOURCE_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GNL_TYPE_FILESOURCE))
typedef struct _GnlFileSourcePrivate GnlFileSourcePrivate;

struct _GnlFileSource
{
  GnlSource parent;

  /*< private >*/

  GnlFileSourcePrivate *private;
};

struct _GnlFileSourceClass
{
  GnlSourceClass parent_class;
};

GType gnl_filesource_get_type (void);

G_END_DECLS
#endif /* __GNL_FILESOURCE_H__ */
