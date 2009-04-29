/* GStreamer
 * Copyright (C) 2004 Anatoly Yakovenko <aeyakovenko@yahoo.com>
 * Copyright (C) 2005 Philippe Khalaf <burger@speedy.org>
 *
 * gstjrtp.c: 
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstrtpbin.h"
#include "gstrtpsend.h"
#include "gstrtprecv.h"

static gboolean plugin_init (GstPlugin * plugin)
{
// return gst_element_register (plugin, "jrtpsrc",
//   GST_RANK_NONE, GST_TYPE_JRTPSRC) && 
//  gst_element_register (plugin, "jrtpsink",
//   GST_RANK_NONE, GST_TYPE_JRTPSINK) &&
//    return gst_element_register (plugin, "rtpsession",
//            GST_RANK_NONE, GST_TYPE_RTPSESSION) &&
//           gst_element_register (plugin, "rtpbin",
//            GST_RANK_NONE, GST_TYPE_RTP_BIN);

    return gst_element_register (plugin, "rtpsend",
            GST_RANK_NONE, GST_TYPE_RTPSEND) &&
        gst_element_register (plugin, "rtprecv",
                GST_RANK_NONE, GST_TYPE_RTPRECV) &&
    gst_element_register (plugin, "rtpbin",
            GST_RANK_NONE, GST_TYPE_RTP_BIN);
}

GST_PLUGIN_DEFINE (
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "jrtp",
  "Jrtplib plugin",
  plugin_init,
  VERSION,
  "LGPL",
  "Farsight",
  "http://farsight.sf.net/"
)

