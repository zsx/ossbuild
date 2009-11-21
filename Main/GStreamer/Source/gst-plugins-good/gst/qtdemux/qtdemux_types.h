/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
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

#ifndef __GST_QTDEMUX_TYPES_H__
#define __GST_QTDEMUX_TYPES_H__

#include <gst/gst.h>

#include "qtatomparser.h"
#include "qtdemux.h"

G_BEGIN_DECLS

typedef gboolean (*QtDumpFunc) (GstQTDemux * qtdemux, QtAtomParser * data, int depth);

typedef struct _QtNodeType QtNodeType;

#define QT_UINT32(a)  (GST_READ_UINT32_BE(a))
#define QT_UINT24(a)  (GST_READ_UINT32_BE(a) >> 8)
#define QT_UINT16(a)  (GST_READ_UINT16_BE(a))
#define QT_UINT8(a)   (GST_READ_UINT8(a))
#define QT_FP32(a)    ((GST_READ_UINT32_BE(a))/65536.0)
#define QT_FP16(a)    ((GST_READ_UINT16_BE(a))/256.0)
#define QT_FOURCC(a)  (GST_READ_UINT32_LE(a))
#define QT_UINT64(a)  ((((guint64)QT_UINT32(a))<<32)|QT_UINT32(((guint8 *)a)+4))

typedef enum {
  QT_FLAG_NONE      = (0),
  QT_FLAG_CONTAINER = (1 << 0) 
} QtFlags;

struct _QtNodeType {
  guint32      fourcc;
  const gchar *name;
  QtFlags      flags;
  QtDumpFunc   dump;
};

const QtNodeType *qtdemux_type_get (guint32 fourcc);

G_END_DECLS

#endif /* __GST_QTDEMUX_TYPES_H__ */
