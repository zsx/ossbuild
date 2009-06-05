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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstsectionfilter.h"

#ifndef __always_inline
#if (__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#define __always_inline inline __attribute__((always_inline))
#else
#define __always_inline inline
#endif
#endif

#ifndef DISABLE_INLINE
#define FORCE_INLINE __always_inline
#else
#define FORCE_INLINE
#endif

GST_DEBUG_CATEGORY (gstflusectionfilter_debug);
#define GST_CAT_DEFAULT (gstflusectionfilter_debug)

void
gst_section_filter_init (GstSectionFilter *filter)
{
  g_return_if_fail (filter != NULL);
  filter->adapter = gst_adapter_new();
  /* continuity counter can at max be 15
   * we make 255 as an indication that
   * there is no last continuity counter */
  filter->last_continuity_counter = 255;
  filter->section_length = G_MAXUINT16;
}

void
gst_section_filter_uninit (GstSectionFilter *filter)
{
  g_return_if_fail (filter != NULL);
  if (filter->adapter)
    g_object_unref (filter->adapter);
  filter->adapter = NULL;
}

void
gst_section_filter_clear (GstSectionFilter *filter)
{
  g_return_if_fail (filter != NULL);
  if (filter->adapter) {
    gst_adapter_clear (filter->adapter);
    filter->last_continuity_counter = 255;
    filter->section_length = G_MAXUINT16;
  }
}

static FORCE_INLINE gboolean
gst_section_is_complete (GstSectionFilter *filter)
{
  /* section length measures size of section from 3 bytes into section
   * (ie after section length field finished) until end of section)
   */
  guint avail_bytes = gst_adapter_available(filter->adapter);
  if (filter->section_length == avail_bytes - 3) {
    return TRUE;
  } else if (filter->section_length < (int)(avail_bytes - 3)) {
    GST_DEBUG ("section length seems to be less than available bytes for "
      "rest of section.");
    return TRUE;
  }
  return FALSE;
}
/* returns True when section finished and ready to parse */
/* FIXME: especially for multi-section tables, we need to handle pusi correctly
 * and handle cases where a new section starts in the same transport packet.
 */
gboolean
gst_section_filter_push (GstSectionFilter *filter,
                         gboolean pusi, /* determines whether start or not */
                         guint8 continuity_counter,
                         GstBuffer *buf)

{
  g_return_val_if_fail (filter != NULL, FALSE);

  /* check if it's the first packet of a section or
   * if it continues the section */
  if (pusi) {
    const guint8* data = GST_BUFFER_DATA(buf);
    if (filter->last_continuity_counter != 255) {
      GST_WARNING ("section lost, last continuity counter: %d"
        "we now have a pusi at continuity counter: %d", 
        filter->last_continuity_counter,
        continuity_counter);
      gst_section_filter_clear (filter);
    }
    filter->section_length = GST_READ_UINT16_BE (data+1);
    filter->section_length &= 0x0fff;
    if (filter->section_length > 1021) {
      GST_DEBUG ("section length too big");
      return FALSE;
    }
    gst_adapter_push (filter->adapter, buf);
    filter->last_continuity_counter = continuity_counter;
    return gst_section_is_complete (filter);
  } else if (filter->last_continuity_counter == continuity_counter - 1 ||
             (filter->last_continuity_counter == 15 && 
             continuity_counter == 0)) {
    GST_DEBUG ("section still going, no pusi");
    gst_adapter_push (filter->adapter, buf);
    filter->last_continuity_counter = continuity_counter;
    return gst_section_is_complete (filter);
  }
  /* we have lost the section and we are not a start
   * section, so clear what was in it */
  else {
    GST_WARNING ("section lost, last continuity counter: %d"
      "new continuity counter but not pusi: %d", 
      filter->last_continuity_counter,
      continuity_counter);
    gst_section_filter_clear (filter);
    return FALSE;
  }
}

                      
