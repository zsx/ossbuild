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

#include "gstmpegdemux.h"
#include "gstmpegtsdemux.h"

GST_DEBUG_CATEGORY_EXTERN (gstflupesfilter_debug);
GST_DEBUG_CATEGORY_EXTERN (gstflusectionfilter_debug);
static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gstflupesfilter_debug, "flupesfilter", 0,
      "MPEG-TS/PS PES filter output");
  GST_DEBUG_CATEGORY_INIT (gstflusectionfilter_debug, "flusectionfilter", 0,
      "MPEG-TS Section filter output");

  if (!gst_flups_demux_plugin_init (plugin))
    return FALSE;
  if (!gst_fluts_demux_plugin_init (plugin))
    return FALSE;

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "flumpegdemux",
    "MPEG demuxers",
    plugin_init, VERSION,
    GST_LICENSE_UNKNOWN, "flumpegdemux", "http://www.fluendo.com/");
