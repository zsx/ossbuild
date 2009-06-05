/* 
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/.
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Fluendo MPEG Demuxer plugin.
 *
 * The Initial Developer of the Original Code is Fluendo, S.L.
 * Portions created by Fluendo, S.L. are Copyright (C) 2005
 * Fluendo, S.L. All Rights Reserved.
 *
 * Contributor(s): Jan Schmidt <jan@fluendo.com>
 */

#ifndef __FLUTS_PMT_INFO_H__
#define __FLUTS_PMT_INFO_H__

#include <glib.h>
#include "flutspmtstreaminfo.h"

G_BEGIN_DECLS

typedef struct FluTsPmtInfoClass {
  GObjectClass parent_class;
} FluTsPmtInfoClass;

typedef struct FluTsPmtInfo {
  GObject parent;

  guint16 program_no;
  guint16 pcr_pid;

  guint8 version_no;

  GValueArray *descriptors;
  GValueArray *streams;
} FluTsPmtInfo;

FluTsPmtInfo *fluts_pmt_info_new (guint16 program_no, guint16 pcr_pid, guint8 version);
void fluts_pmt_info_add_stream (FluTsPmtInfo *pmt_info, FluTsPmtStreamInfo *stream);
void fluts_pmt_info_add_descriptor (FluTsPmtInfo *pmt_info,	
  const gchar *descriptor, guint length);

GType fluts_pmt_info_get_type (void);

#define FLUTS_TYPE_PMT_INFO (fluts_pmt_info_get_type ())
#define FLUTS_IS_PMT_INFO(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), FLUTS_TYPE_PMT_INFO))
#define FLUTS_PMT_INFO(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),FLUTS_TYPE_PMT_INFO, FluTsPmtInfo))

G_END_DECLS

#endif
