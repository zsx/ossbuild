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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gsticesrc.h"
#include "gsticesink.h"

static gboolean plugin_init (GstPlugin * plugin)
{
    return gst_element_register (plugin, "icesrc",
            GST_RANK_NONE, GST_TYPE_ICESRC)
    && gst_element_register (plugin, "icesink",
            GST_RANK_NONE, GST_TYPE_ICESINK);
}

GST_PLUGIN_DEFINE (
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "icesink/src",
  "Ice support through libjingleP2P",
  plugin_init,
  VERSION,
  "LGPL",
  "Farsight",
  "http://farsight.sf.net/"
)
