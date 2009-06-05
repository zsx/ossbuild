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
 * Contributor(s): Jan Schmidt <jan@fluendo.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>

#include "flutspatinfo.h"

enum
{
  PROP_0,
  PROP_PROGRAM_NO,
  PROP_PID
};

static void fluts_pat_info_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * spec);
static void fluts_pat_info_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * spec);

GST_BOILERPLATE (FluTsPatInfo, fluts_pat_info, GObject, G_TYPE_OBJECT);

FluTsPatInfo *fluts_pat_info_new (guint16 program_no, guint16 pid)
{
  FluTsPatInfo *info;

  info = g_object_new (FLUTS_TYPE_PAT_INFO, NULL);

  info->program_no = program_no;
  info->pid = pid;

  return info;
}

static void
fluts_pat_info_base_init (gpointer klass)
{
}

static void
fluts_pat_info_class_init (FluTsPatInfoClass *klass)
{
  GObjectClass *gobject_klass = (GObjectClass *)klass;

  gobject_klass->set_property = fluts_pat_info_set_property;
  gobject_klass->get_property = fluts_pat_info_get_property;

  g_object_class_install_property (gobject_klass, PROP_PROGRAM_NO,
       g_param_spec_uint ("program-number", "Program Number",
       "Program Number for this program", 0, G_MAXUINT16, 1,
       G_PARAM_READABLE));

  g_object_class_install_property (gobject_klass, PROP_PID,
       g_param_spec_uint ("pid", "PID carrying PMT",
       "PID which carries the PMT for this program", 1, G_MAXUINT16, 1,
       G_PARAM_READABLE));
}

static void
fluts_pat_info_init (FluTsPatInfo *pat_info, FluTsPatInfoClass *klass)
{
}

static void fluts_pat_info_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * spec)
{
  g_return_if_fail (FLUTS_IS_PAT_INFO (object));

  /* No settable properties */
  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
}

static void fluts_pat_info_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * spec)
{
  FluTsPatInfo *pat_info;

  g_return_if_fail (FLUTS_IS_PAT_INFO (object));

  pat_info = FLUTS_PAT_INFO (object);

  switch (prop_id) {
    case PROP_PROGRAM_NO:
      g_value_set_uint (value, pat_info->program_no);
      break;
    case PROP_PID:
      g_value_set_uint (value, pat_info->pid);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
      break;
  }
}
