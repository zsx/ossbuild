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
 * Contributor(s): Wim Taymans <wim@fluendo.com>
 */

#ifndef __GST_SECTION_FILTER_H__
#define __GST_SECTION_FILTER_H__

#include <gst/gst.h>
#include <gst/base/gstadapter.h>

G_BEGIN_DECLS

typedef struct _GstSectionFilter GstSectionFilter;

struct _GstSectionFilter {
  GstAdapter *adapter;
  guint8 last_continuity_counter;
  guint16 section_length;
};

void gst_section_filter_init (GstSectionFilter *filter);
void gst_section_filter_uninit (GstSectionFilter *filter);
gboolean gst_section_filter_push (GstSectionFilter *filter,
                                  gboolean pusi,
				  guint8 continuity_counter,
				  GstBuffer *buf);
void gst_section_filter_clear (GstSectionFilter *filter);				     
G_END_DECLS

#endif /* __GST_SECTION_FILTER_H__ */
