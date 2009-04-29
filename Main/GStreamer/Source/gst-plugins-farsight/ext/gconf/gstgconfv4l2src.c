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

#include "gstgconfv4l2src.h"

#include <gconf/gconf-client.h>

#include <gst/interfaces/videoorientation.h>

#define GCONF_CAM_DIR "/system/osso/af"
#define GCONF_CAM_FLIP_KEY "camera-has-turned"

#define V4L2SRC_ELEMENT "v4l2src"

static GstElementDetails gst_gconf_v4l2src_details = {
  "GConf wrapper for v4l2src video orientation",
  "Source/Video",
  "Apply gconf video orientation settings on v4l2src",
  "Philippe Khalaf <philippe.khalaf@collabora.co.uk>"
};

/* generic templates */
static GstStaticPadTemplate gst_gconf_v4l2src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY
    );

GST_DEBUG_CATEGORY_STATIC (gst_gconf_v4l2src_debug);
#define GST_CAT_DEFAULT gst_gconf_v4l2src_debug

struct _GstGConfV4L2SrcPrivate
{
  GstElement *v4l2src_element;
  GConfClient *client;
  guint notify_id;
};

#define GST_GCONF_V4L2SRC_GET_PRIVATE(o)  \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GST_TYPE_GCONF_V4L2SRC, \
                                GstGConfV4L2SrcPrivate))

static void gst_gconf_v4l2src_dispose (GObject * object);

static void gst_gconf_v4l2src_release_element (GstGConfV4L2Src *self);

static void flipped_cb (GConfClient * client, guint connection_id,
    GConfEntry * entry, gpointer data);

static void flip_it (GstGConfV4L2Src *self, gboolean flipped);

static GstStateChangeReturn gst_gconf_v4l2src_change_state (
    GstElement * element, GstStateChange transition);

GST_BOILERPLATE (GstGConfV4L2Src, gst_gconf_v4l2src, GstBin, GST_TYPE_BIN);

static void
gst_gconf_v4l2src_class_init (GstGConfV4L2SrcClass * klass)
{
  GObjectClass *gobject_klass = G_OBJECT_CLASS (klass);
  GstElementClass *gstelement_klass = GST_ELEMENT_CLASS (klass);

  gobject_klass->dispose = gst_gconf_v4l2src_dispose;
  gstelement_klass->change_state = gst_gconf_v4l2src_change_state;

  gst_element_class_add_pad_template (gstelement_klass,
      gst_static_pad_template_get (&gst_gconf_v4l2src_template));

  gst_element_class_set_details (gstelement_klass, &gst_gconf_v4l2src_details);

  GST_DEBUG_CATEGORY_INIT (gst_gconf_v4l2src_debug, "gconfv4l2src", 0,
      "GConf v4l2src");

  g_type_class_add_private (klass, sizeof (GstGConfV4L2SrcPrivate));
}

static void
gst_gconf_v4l2src_base_init (gpointer klass)
{
}

static void
gst_gconf_v4l2src_init (GstGConfV4L2Src *self, GstGConfV4L2SrcClass *g_class)
{
  self->priv = GST_GCONF_V4L2SRC_GET_PRIVATE (self);

  GError *err = NULL;

  /* Create gconf client and listen on flip key */
  self->priv->client = gconf_client_get_default ();
  gconf_client_add_dir (self->priv->client, GCONF_CAM_DIR,
      GCONF_CLIENT_PRELOAD_RECURSIVE, &err);
  if (err != NULL)
  {
    /* Report error to user, and free error */
    GST_DEBUG_OBJECT (self, "Could not listen on dir : %s", err->message);
    g_error_free (err);
    return;
  }
  self->priv->notify_id = gconf_client_notify_add (self->priv->client,
      GCONF_CAM_DIR "/" GCONF_CAM_FLIP_KEY,
      flipped_cb, self, NULL, &err);
  if (err != NULL)
  {
    /* Report error to user, and free error */
    GST_DEBUG_OBJECT (self, "Could not watch key : %s", err->message);
    g_error_free (err);
    return;
  }

  GstElement *v4l2src_element = NULL;
  GstPad *v4l2src_src_pad = NULL;

  v4l2src_element = gst_element_factory_make (V4L2SRC_ELEMENT, NULL);
  if (v4l2src_element == NULL)
  {
    GST_DEBUG_OBJECT (self, "Could not create video flip element %s",
        V4L2SRC_ELEMENT);
    return;
  }

  gst_bin_add (GST_BIN (self), v4l2src_element);
  GST_DEBUG_OBJECT (self,"added to bin");

  self->priv->v4l2src_element = v4l2src_element;

  v4l2src_src_pad = gst_element_get_pad (v4l2src_element, "src");
  if (v4l2src_src_pad)
  {
    GstPad *newpad = gst_ghost_pad_new ("src", v4l2src_src_pad);
    gst_element_add_pad (GST_ELEMENT (self), newpad);
  }
}

static void
gst_gconf_v4l2src_dispose (GObject * object)
{
  GstGConfV4L2Src *self = GST_GCONF_V4L2SRC (object);

  if (self->priv->client)
  {
    gconf_client_notify_remove(self->priv->client,
        self->priv->notify_id);  
    g_object_unref (G_OBJECT (self->priv->client));
    self->priv->client = NULL;
  }

  gst_gconf_v4l2src_release_element (self);
}

static void
gst_gconf_v4l2src_init_flip (GstGConfV4L2Src *self)
{
  gboolean flipped;
  flipped = gconf_client_get_bool (self->priv->client,
      GCONF_CAM_DIR "/" GCONF_CAM_FLIP_KEY, NULL);
  flip_it (self, flipped);
}

static void
gst_gconf_v4l2src_release_element (GstGConfV4L2Src *self)
{
  if (self->priv->v4l2src_element)
  {
    gst_bin_remove (GST_BIN (self), self->priv->v4l2src_element);
    self->priv->v4l2src_element = NULL;
  }
}

static void 
flipped_cb (GConfClient * client, guint connection_id, GConfEntry * entry,
    gpointer data)
{
  GstGConfV4L2Src *self = GST_GCONF_V4L2SRC (data);

  gboolean flipped =
    gconf_client_get_bool (client, gconf_entry_get_key (entry), NULL);

  flip_it (self, flipped);
}

static void
flip_it (GstGConfV4L2Src *self, gboolean flipped)
{
  /* The interface is implemented if the device is opened (and it will
   * abort() if its not implemented
   */
  if (gst_element_implements_interface (self->priv->v4l2src_element,
          GST_TYPE_VIDEO_ORIENTATION))
  {
    /* at this point we simply set the horizontal flip */
    GST_DEBUG_OBJECT (self, "Setting h/v flip to %d", !flipped);
    gst_video_orientation_set_hflip (GST_VIDEO_ORIENTATION
        (self->priv->v4l2src_element), flipped);
    gst_video_orientation_set_vflip (GST_VIDEO_ORIENTATION
        (self->priv->v4l2src_element), flipped);
  }
}

static GstStateChangeReturn
gst_gconf_v4l2src_change_state (GstElement * element,
    GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstGConfV4L2Src *self = GST_GCONF_V4L2SRC (element);

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
      gst_gconf_v4l2src_init_flip (self);
    default:
      break;
  }

  ret = GST_CALL_PARENT_WITH_DEFAULT (GST_ELEMENT_CLASS, change_state,
      (element, transition), GST_STATE_CHANGE_SUCCESS);

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_NULL:
      break;
    default:
      break;
  }

  return ret;
}
