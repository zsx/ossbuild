/* GStreamer
 * Copyright (C) 2001 Wim Taymans <wim.taymans@gmail.com>
 *               2004-2008 Edward Hervey <bilboed@bilboed.com>
 *
 * gnlobject.h: Header for base GnlObject
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


#ifndef __GNL_OBJECT_H__
#define __GNL_OBJECT_H__

#include <gst/gst.h>

#include "gnltypes.h"

G_BEGIN_DECLS
#define GNL_TYPE_OBJECT \
  (gnl_object_get_type())
#define GNL_OBJECT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GNL_TYPE_OBJECT,GnlObject))
#define GNL_OBJECT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GNL_TYPE_OBJECT,GnlObjectClass))
#define GNL_OBJECT_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GNL_TYPE_OBJECT, GnlObjectClass))
#define GNL_IS_OBJECT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GNL_TYPE_OBJECT))
#define GNL_IS_OBJECT_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GNL_TYPE_OBJECT))

/**
 * GnlCoverType"
 * @GNL_COVER_ALL  : Covers all the content
 * @GNL_COVER_SOME : Covers some of the content
 * @GNL_COVER_START: Covers the beginning
 * @GNL_COVER_STOP : Covers the end
 *
 * Type of coverage for the given start/stop values
*/
    typedef enum
{
  GNL_COVER_ALL,
  GNL_COVER_SOME,
  GNL_COVER_START,
  GNL_COVER_STOP,
} GnlCoverType;

/**
 * GnlObjectFlags:
 * @GNL_OBJECT_IS_SOURCE:
 * @GNL_OBJECT_IS_OPERATION:
 * @GNL_OBJECT_LAST_FLAG:
*/

typedef enum
{
  GNL_OBJECT_SOURCE = (GST_BIN_FLAG_LAST << 0),
  GNL_OBJECT_OPERATION = (GST_BIN_FLAG_LAST << 1),
  /* padding */
  GNL_OBJECT_LAST_FLAG = (GST_BIN_FLAG_LAST << 16)
} GnlObjectFlags;


#define GNL_OBJECT_IS_SOURCE(obj) \
  (GST_OBJECT_FLAG_IS_SET(obj, GNL_OBJECT_SOURCE))
#define GNL_OBJECT_IS_OPERATION(obj) \
  (GST_OBJECT_FLAG_IS_SET(obj, GNL_OBJECT_OPERATION))

struct _GnlObject
{
  GstBin parent;

  /* Time positionning */
  GstClockTime start;
  GstClockTimeDiff duration;

  /* read-only */
  GstClockTime stop;

  GstClockTime media_start;
  GstClockTimeDiff media_duration;

  /* read-only */
  GstClockTime media_stop;

  /* read-only */
  gdouble rate;

  /* priority in parent */
  guint32 priority;

  /* active in parent */
  gboolean active;

  /* Filtering caps */
  GstCaps *caps;

  /* current segment seek <RO> */
  gdouble segment_rate;
  GstSeekFlags segment_flags;
  gint64 segment_start;
  gint64 segment_stop;
};

struct _GnlObjectClass
{
  GstBinClass parent_class;

  /* virtual methods for subclasses */
    gboolean (*covers) (GnlObject * object,
      GstClockTime start, GstClockTime stop, GnlCoverType type);
    gboolean (*prepare) (GnlObject * object);
    gboolean (*cleanup) (GnlObject * object);
};

GType gnl_object_get_type (void);

GstPad *gnl_object_ghost_pad (GnlObject * object,
    const gchar * name, GstPad * target);

GstPad *gnl_object_ghost_pad_full (GnlObject * object,
    const gchar * name, GstPad * target, gboolean flush_hack);


GstPad *gnl_object_ghost_pad_no_target (GnlObject * object,
    const gchar * name, GstPadDirection dir);

gboolean gnl_object_ghost_pad_set_target (GnlObject * object,
    GstPad * ghost, GstPad * target);

void gnl_object_remove_ghost_pad (GnlObject * object, GstPad * ghost);

gboolean gnl_object_covers (GnlObject * object,
    GstClockTime start, GstClockTime stop, GnlCoverType type);

gboolean
gnl_object_to_media_time (GnlObject * object, GstClockTime otime,
			  GstClockTime * mtime);

gboolean
gnl_media_to_object_time (GnlObject * object, GstClockTime mtime,
			  GstClockTime * otime);

G_END_DECLS
#endif /* __GNL_OBJECT_H__ */
