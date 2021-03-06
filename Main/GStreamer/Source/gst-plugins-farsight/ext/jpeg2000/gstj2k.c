/* 
 * GStreamer
 * Copyright (c) 2005 Philippe Khalaf <burger@speedy.org>
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
#include <config.h>
#endif

#include "gstj2kdec.h"
#include "gstj2kenc.h"
#include "gstyahooparse.h"
#include "gstyahooenc.h"

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_jpeg2000enc_plugin_init (plugin))
    return FALSE;
  if (!gst_jpeg2000dec_plugin_init (plugin))
    return FALSE;
  if (!gst_yahooparse_plugin_init (plugin))
    return FALSE;
  if (!gst_yahooenc_plugin_init (plugin))
    return FALSE;

  return TRUE;
}

GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "jpeg2000",
    "JPEG2000 codec and yahoo webcam",
    plugin_init,
    VERSION,
    "GPL",
    "Farsight",
    "http://farsight.sourceforge.net")
