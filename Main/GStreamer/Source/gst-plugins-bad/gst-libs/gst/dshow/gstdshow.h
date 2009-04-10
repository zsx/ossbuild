/* GStreamer
 * Copyright (C) 2007 Sebastien Moutte <sebastien@moutte.net>
 *
 * gstdshow.h:
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

#ifndef _GSTDSHOW_
#define _GSTDSHOW_

#include <windows.h>

#include <glib.h>

#ifdef LIBDSHOW_EXPORTS
#include <streams.h>
#include <atlbase.h>
#define DSHOW_API __declspec(dllexport)
#else
#define DSHOW_API __declspec(dllimport)
#endif

typedef struct _GstCapturePinMediaType
{
  AM_MEDIA_TYPE *mediatype;
  IPin *capture_pin;
} GstCapturePinMediaType;

#ifdef  __cplusplus
extern "C" {
#endif

/* register fake filters as COM object and as Direct Show filters in the registry */
DSHOW_API BOOL gst_dshow_register_fakefilters ();

/* free memory of the input pin mediatype */
DSHOW_API void gst_dshow_free_pin_mediatype (gpointer pt);

/* free memory of the input dshow mediatype */
DSHOW_API void gst_dshow_free_mediatype (AM_MEDIA_TYPE *pmt);

/* free the memory of all mediatypes of the input list if pin mediatype */
DSHOW_API void gst_dshow_free_pins_mediatypes (GList *mediatypes);

/* get a pin from directshow filter */
DSHOW_API gboolean gst_dshow_get_pin_from_filter (IBaseFilter *filter, PIN_DIRECTION pindir, IPin **pin);

/* find and return a filter according to the input and output types */
DSHOW_API gboolean gst_dshow_find_filter(CLSID input_majortype, CLSID input_subtype, 
                               CLSID output_majortype, CLSID output_subtype,
                               gchar * prefered_filter_name, IBaseFilter **filter);

/* get the dshow device path from device friendly name. 
If friendly name is not set, it will return the first available device */
DSHOW_API gchar *gst_dshow_getdevice_from_devicename (GUID *device_category, gchar **device_name);

/* show the capture filter property page (generally used to setup the device). the page is modal*/
DSHOW_API gboolean gst_dshow_show_propertypage (IBaseFilter *base_filter);


#ifdef  __cplusplus
}
#endif

#endif /* _GSTDSHOW_ */