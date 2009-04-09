/* GStreamer
 *
 * Copyright (C) 2008 Nokia Corporation <multimedia@maemo.org>
 *
 * photography.c: photography interface for digital imaging
 *
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

#include "photography.h"

/**
 * SECTION:photography
 * @short_description: Interface for elements having digital imaging controls
 *
 * The interface allows access to some common digital imaging controls
 */

static void gst_photography_iface_init (GstPhotographyInterface * iface);

GType
gst_photography_get_type (void)
{
  static GType gst_photography_type = 0;

  if (!gst_photography_type) {
    static const GTypeInfo gst_photography_info = {
      sizeof (GstPhotographyInterface),
      (GBaseInitFunc) gst_photography_iface_init,
      NULL,
      NULL,
      NULL,
      NULL,
      0,
      0,
      NULL,
    };

    gst_photography_type = g_type_register_static (G_TYPE_INTERFACE,
        "GstPhotography", &gst_photography_info, 0);
    g_type_interface_add_prerequisite (gst_photography_type,
        GST_TYPE_IMPLEMENTS_INTERFACE);
  }

  return gst_photography_type;
}

static void
gst_photography_iface_init (GstPhotographyInterface * iface)
{
  /* default virtual functions */
  iface->get_ev_compensation = NULL;
  iface->get_iso_speed = NULL;
  iface->get_aperture = NULL;
  iface->get_exposure = NULL;
  iface->get_white_balance_mode = NULL;
  iface->get_colour_tone_mode = NULL;
  iface->get_scene_mode = NULL;
  iface->get_flash_mode = NULL;
  iface->get_zoom = NULL;

  iface->set_ev_compensation = NULL;
  iface->set_iso_speed = NULL;
  iface->set_aperture = NULL;
  iface->set_exposure = NULL;
  iface->set_white_balance_mode = NULL;
  iface->set_colour_tone_mode = NULL;
  iface->set_scene_mode = NULL;
  iface->set_flash_mode = NULL;
  iface->set_zoom = NULL;

  iface->get_capabilities = NULL;
  iface->prepare_for_capture = NULL;
  iface->set_autofocus = NULL;
}

#define GST_PHOTOGRAPHY_FUNC_TEMPLATE(function_name, param_type) \
gboolean \
gst_photography_set_ ## function_name (GstPhotography * photo, param_type param) \
{ \
  GstPhotographyInterface *iface; \
  g_return_val_if_fail (photo != NULL, FALSE); \
  iface = GST_PHOTOGRAPHY_GET_IFACE (photo); \
  if (iface->set_ ## function_name) { \
    return iface->set_ ## function_name (photo, param); \
  } \
  return FALSE; \
} \
gboolean \
gst_photography_get_ ## function_name (GstPhotography * photo, param_type * param) \
{ \
  GstPhotographyInterface *iface; \
  g_return_val_if_fail (photo != NULL, FALSE); \
  iface = GST_PHOTOGRAPHY_GET_IFACE (photo); \
  if (iface->get_ ## function_name) { \
    return iface->get_ ## function_name (photo, param); \
  } \
  return FALSE; \
}


/**
 * gst_photography_set_ev_compensation:
 * @photo: #GstPhotography interface of a #GstElement
 * @ev_comp: ev compensation value to set
 *
 * Set the ev compensation value for the #GstElement
 *
 * Returns: %TRUE if setting the value succeeded, %FALSE otherwise
 */
/**
 * gst_photography_get_ev_compensation:
 * @photo: #GstPhotography interface of a #GstElement
 * @ev_comp: ev compensation value to get
 *
 * Get the ev compensation value for the #GstElement
 *
 * Returns: %TRUE if getting the value succeeded, %FALSE otherwise
 */
GST_PHOTOGRAPHY_FUNC_TEMPLATE (ev_compensation, gfloat);

/**
 * gst_photography_set_iso_speed:
 * @photo: #GstPhotography interface of a #GstElement
 * @iso_speed: ISO speed value to set
 *
 * Set the ISO value (light sensivity) for the #GstElement
 *
 * Returns: %TRUE if setting the value succeeded, %FALSE otherwise
 */
/**
 * gst_photography_get_iso_speed:
 * @photo: #GstPhotography interface of a #GstElement
 * @iso_speed: ISO speed value to get
 *
 * Get the ISO value (light sensivity) for the #GstElement
 *
 * Returns: %TRUE if getting the value succeeded, %FALSE otherwise
 */
GST_PHOTOGRAPHY_FUNC_TEMPLATE (iso_speed, guint);

/**
 * gst_photography_set_aperture:
 * @photo: #GstPhotography interface of a #GstElement
 * @aperture: aperture value to set
 *
 * Set the aperture value for the #GstElement
 *
 * Returns: %TRUE if setting the value succeeded, %FALSE otherwise
 */
/**
 * gst_photography_get_aperture:
 * @photo: #GstPhotography interface of a #GstElement
 * @aperture: aperture value to get
 *
 * Get the aperture value for the #GstElement
 *
 * Returns: %TRUE if getting the value succeeded, %FALSE otherwise
 */
GST_PHOTOGRAPHY_FUNC_TEMPLATE (aperture, guint);

/**
 * gst_photography_set_exposure:
 * @photo: #GstPhotography interface of a #GstElement
 * @exposure: exposure time to set
 *
 * Set the fixed exposure time (in us) for the #GstElement
 *
 * Returns: %TRUE if setting the value succeeded, %FALSE otherwise
 */
/**
 * gst_photography_get_exposure:
 * @photo: #GstPhotography interface of a #GstElement
 * @exposure: exposure time to get
 *
 * Get the fixed exposure time (in us) for the #GstElement
 *
 * Returns: %TRUE if getting the value succeeded, %FALSE otherwise
 */
GST_PHOTOGRAPHY_FUNC_TEMPLATE (exposure, guint32);

/**
 * gst_photography_set_white_balance_mode:
 * @photo: #GstPhotography interface of a #GstElement
 * @wb_mode: #GstWhiteBalanceMode to set
 *
 * Set the white balance mode for the #GstElement
 *
 * Returns: %TRUE if setting the value succeeded, %FALSE otherwise
 */
/**
 * gst_photography_get_white_balance_mode:
 * @photo: #GstPhotography interface of a #GstElement
 * @wb_mode: #GstWhiteBalanceMode to get
 *
 * Get the white balance mode for the #GstElement
 *
 * Returns: %TRUE if getting the value succeeded, %FALSE otherwise
 */
GST_PHOTOGRAPHY_FUNC_TEMPLATE (white_balance_mode, GstWhiteBalanceMode);

/**
 * gst_photography_set_colour_tone_mode:
 * @photo: #GstPhotography interface of a #GstElement
 * @tone_mode: #GstColourToneMode to set
 *
 * Set the colour tone mode for the #GstElement
 *
 * Returns: %TRUE if setting the value succeeded, %FALSE otherwise
 */
/**
 * gst_photography_get_colour_tone_mode:
 * @photo: #GstPhotography interface of a #GstElement
 * @tone_mode: #GstColourToneMode to get
 *
 * Get the colour tone mode for the #GstElement
 *
 * Returns: %TRUE if getting the value succeeded, %FALSE otherwise
 */
GST_PHOTOGRAPHY_FUNC_TEMPLATE (colour_tone_mode, GstColourToneMode);

/**
 * gst_photography_set_scene_mode:
 * @photo: #GstPhotography interface of a #GstElement
 * @scene_mode: #GstSceneMode to set
 *
 * Set the scene mode for the #GstElement
 *
 * Returns: %TRUE if setting the value succeeded, %FALSE otherwise
 */
/**
 * gst_photography_get_scene_mode:
 * @photo: #GstPhotography interface of a #GstElement
 * @scene_mode: #GstSceneMode to get
 *
 * Get the scene mode for the #GstElement
 *
 * Returns: %TRUE if getting the value succeeded, %FALSE otherwise
 */
GST_PHOTOGRAPHY_FUNC_TEMPLATE (scene_mode, GstSceneMode);

/**
 * gst_photography_set_flash_mode:
 * @photo: #GstPhotography interface of a #GstElement
 * @flash_mode: #GstFlashMode to set
 *
 * Set the flash mode for the #GstElement
 *
 * Returns: %TRUE if setting the value succeeded, %FALSE otherwise
 */
/**
 * gst_photography_get_flash_mode:
 * @photo: #GstPhotography interface of a #GstElement
 * @flash_mode: #GstFlashMode to get
 *
 * Get the flash mode for the #GstElement
 *
 * Returns: %TRUE if getting the value succeeded, %FALSE otherwise
 */
GST_PHOTOGRAPHY_FUNC_TEMPLATE (flash_mode, GstFlashMode);

/**
 * gst_photography_set_zoom:
 * @photo: #GstPhotography interface of a #GstElement
 * @zoom: zoom value to set
 *
 * Set the zoom value for the #GstElement.
 * E.g. 1.0 to get original image and 3.0 for 3x zoom and so on.
 *
 * Returns: %TRUE if setting the value succeeded, %FALSE otherwise
 */
/**
 * gst_photography_get_zoom:
 * @photo: #GstPhotography interface of a #GstElement
 * @zoom: zoom value to get
 *
 * Get the zoom value for the #GstElement
 *
 * Returns: %TRUE if getting the value succeeded, %FALSE otherwise
 */
GST_PHOTOGRAPHY_FUNC_TEMPLATE (zoom, gfloat);

/**
 * gst_photography_get_capabilities:
 * @photo: #GstPhotography interface of a #GstElement
 *
 * Get #GstPhotoCaps bitmask value that indicates what photography
 * interface features the #GstElement supports
 *
 * Returns: #GstPhotoCaps value
 */
GstPhotoCaps
gst_photography_get_capabilities (GstPhotography * photo)
{
  GstPhotographyInterface *iface;
  g_return_val_if_fail (photo != NULL, GST_PHOTOGRAPHY_CAPS_NONE);

  iface = GST_PHOTOGRAPHY_GET_IFACE (photo);
  if (iface->get_capabilities) {
    return iface->get_capabilities (photo);
  } else {
    return GST_PHOTOGRAPHY_CAPS_NONE;
  }
}

/**
 * gst_photography_prepare_for_capture:
 * @photo: #GstPhotography interface of a #GstElement
 * @func: callback that is called after capturing has been prepared
 * @caps: #GstCaps defining the desired format of the captured image
 * @user_data: user data that will be passed to the callback @func
 *
 * Start preparations for capture. @func callback is called after
 * preparations are done.
 *
 * Returns: TRUE if preparations were started (caps were OK), otherwise FALSE.
 */
gboolean
gst_photography_prepare_for_capture (GstPhotography * photo,
    GstPhotoCapturePrepared func, GstCaps * capture_caps, gpointer user_data)
{
  GstPhotographyInterface *iface;
  gboolean ret = TRUE;

  g_return_val_if_fail (photo != NULL, FALSE);

  iface = GST_PHOTOGRAPHY_GET_IFACE (photo);
  if (iface->prepare_for_capture) {
    ret = iface->prepare_for_capture (photo, func, capture_caps, user_data);
  }

  return ret;
}

/**
 * gst_photography_set_autofocus:
 * @photo: #GstPhotography interface of a #GstElement
 * @on: %TRUE to start autofocusing, %FALSE to stop autofocusing
 *
 * Start or stop autofocusing. %GST_PHOTOGRAPHY_AUTOFOCUS_DONE
 * message is posted to bus when autofocusing has finished.
 */
void
gst_photography_set_autofocus (GstPhotography * photo, gboolean on)
{
  GstPhotographyInterface *iface;
  g_return_if_fail (photo != NULL);

  iface = GST_PHOTOGRAPHY_GET_IFACE (photo);
  if (iface->set_autofocus) {
    iface->set_autofocus (photo, on);
  }
}
