/* GStreamer
 *
 *  Copyright 2007 Collabora Ltd
 *   @author: Olivier Crete <olivier.crete@collabora.co.uk>
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


#ifndef __FS_SELECTOR_H__
#define __FS_SELECTOR_H__

#include <gst/gst.h>
#include <gst/gstbin.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

G_BEGIN_DECLS
#define FS_TYPE_SELECTOR 		(fs_selector_get_type())
#define FS_SELECTOR(obj)	                (G_TYPE_CHECK_INSTANCE_CAST((obj), FS_TYPE_SELECTOR,FsSelector))
#define FS_SELECTOR_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST((klass), FS_TYPE_SELECTOR,FsSelectorClass))
#define FS_IS_SELECTOR(obj)              (G_TYPE_CHECK_INSTANCE_TYPE((obj), FS_TYPE_SELECTOR))
#define FS_IS_SELECTOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE((klass), FS_TYPE_SELECTOR))

typedef struct _FsSelector FsSelector;
typedef struct _FsSelectorClass FsSelectorClass;

struct _FsSelector
{
  GstBin bin;                   /* we extend GstBin */

  /* Protected by the object lock too */
  GList *factories;

  GstPad *sinkpad;
  GstPad *srcpad;

  /* Have to be set all at once
   * Protected by the object lock */
  GstElement *current_subelement;
  GstPad *current_internal_srcpad;
  GstPad *current_internal_sinkpad;
};

struct _FsSelectorClass
{
  GstBinClass parent_class;
};

G_END_DECLS

#endif /* __FS_SELECTOR_H__ */
