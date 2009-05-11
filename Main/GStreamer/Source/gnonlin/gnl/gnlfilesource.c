/* Gnonlin
 * Copyright (C) <2005-2008> Edward Hervey <bilboed@bilboed.com>
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

#include "gnl.h"

/**
 * SECTION:element-gnlfilesource
 * @short_description: GNonLin File Source
 *
 * GnlFileSource is a #GnlSource which reads and decodes the contents
 * of a given file. The data in the file is decoded using any available
 * GStreamer plugins.
 */

static GstStaticPadTemplate gnl_filesource_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS_ANY);

GST_DEBUG_CATEGORY_STATIC (gnlfilesource);
#define GST_CAT_DEFAULT gnlfilesource


GST_BOILERPLATE (GnlFileSource, gnl_filesource, GnlSource, GNL_TYPE_SOURCE);

static GstElementDetails gnl_filesource_details = GST_ELEMENT_DETAILS
    ("GNonLin File Source",
    "Filter/Editor",
    "High-level File Source element",
    "Edward Hervey <bilboed@bilboed.com>");

enum
{
  ARG_0,
  ARG_LOCATION,
};

struct _GnlFileSourcePrivate
{
  gboolean dispose_has_run;
  GstElement *filesource;
};

static void gnl_filesource_dispose (GObject * object);

static void gnl_filesource_finalize (GObject * object);

static void
gnl_filesource_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);

static void
gnl_filesource_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static void
gnl_filesource_base_init (gpointer g_class)
{
  GstElementClass *gstclass = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (gstclass, &gnl_filesource_details);
}

static void
gnl_filesource_class_init (GnlFileSourceClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GnlObjectClass *gnlobject_class;
  GnlSourceClass *gnlsource_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gnlobject_class = (GnlObjectClass *) klass;
  gnlsource_class = (GnlSourceClass *) klass;

  parent_class = g_type_class_ref (GNL_TYPE_OBJECT);

  gnlsource_class->controls_one = FALSE;

  GST_DEBUG_CATEGORY_INIT (gnlfilesource, "gnlfilesource",
      GST_DEBUG_FG_BLUE | GST_DEBUG_BOLD, "GNonLin File Source Element");

  gobject_class->dispose = GST_DEBUG_FUNCPTR (gnl_filesource_dispose);
  gobject_class->finalize = GST_DEBUG_FUNCPTR (gnl_filesource_finalize);

  gobject_class->set_property = GST_DEBUG_FUNCPTR (gnl_filesource_set_property);
  gobject_class->get_property = GST_DEBUG_FUNCPTR (gnl_filesource_get_property);

  gst_element_class_install_std_props (GST_ELEMENT_CLASS (klass),
      "location", ARG_LOCATION, G_PARAM_READWRITE, NULL);

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&gnl_filesource_src_template));
}

static void
gnl_filesource_init (GnlFileSource * filesource, GnlFileSourceClass * klass)
{
  GstElement *filesrc, *decodebin;

  GST_OBJECT_FLAG_SET (filesource, GNL_OBJECT_SOURCE);
  filesource->private = g_new0 (GnlFileSourcePrivate, 1);

  /* We create a bin with source and decodebin within */

  if (!(filesrc =
          gst_element_factory_make ("gnomevfssrc", "internal-filesource")))
    if (!(filesrc =
            gst_element_factory_make ("filesrc", "internal-filesource")))
      g_warning
          ("Could not create a gnomevfssrc or filesource element, are you sure you have any of them installed ?");
  if (g_getenv ("USE_DECODEBIN2"))
    decodebin = gst_element_factory_make ("decodebin2", "internal-decodebin");
  else
    decodebin = gst_element_factory_make ("decodebin", "internal-decodebin");
  if (!decodebin)
    g_warning
        ("Could not create a decodebin element, are you sure you have decodebin installed ?");

  filesource->private->filesource = filesrc;

  gst_bin_add_many (GST_BIN (filesource), filesrc, decodebin, NULL);

  if (!(gst_element_link (filesrc, decodebin)))
    g_warning ("Could not link the file source element to decodebin");

  GNL_SOURCE_GET_CLASS (filesource)->control_element ((GnlSource*) filesource,
      decodebin);

  GST_DEBUG_OBJECT (filesource, "done");
}

static void
gnl_filesource_dispose (GObject * object)
{
  GnlFileSource *filesource = (GnlFileSource*) object;

  if (filesource->private->dispose_has_run)
    return;

  GST_INFO_OBJECT (object, "dispose");
  filesource->private->dispose_has_run = TRUE;

  G_OBJECT_CLASS (parent_class)->dispose (object);
  GST_INFO_OBJECT (object, "dispose END");
}

static void
gnl_filesource_finalize (GObject * object)
{
  GnlFileSource *filesource = (GnlFileSource*) object;

  GST_INFO_OBJECT (object, "finalize");
  g_free (filesource->private);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnl_filesource_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GnlFileSource *fs = (GnlFileSource*) object;

  switch (prop_id) {
    case ARG_LOCATION:
      /* proxy to gnomevfssrc */
      g_object_set_property (G_OBJECT (fs->private->filesource), "location",
          value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gnl_filesource_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GnlFileSource *fs = (GnlFileSource*) object;

  switch (prop_id) {
    case ARG_LOCATION:
      /* proxy from gnomevfssrc */
      g_object_get_property (G_OBJECT (fs->private->filesource), "location",
          value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }

}
