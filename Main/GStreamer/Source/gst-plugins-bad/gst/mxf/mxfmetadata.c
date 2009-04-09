/* GStreamer
 * Copyright (C) 2008-2009 Sebastian Dröge <sebastian.droege@collabora.co.uk>
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

#include <gst/gst.h>
#include <string.h>

#include "mxfparse.h"
#include "mxfmetadata.h"
#include "mxfquark.h"

GST_DEBUG_CATEGORY_EXTERN (mxf_debug);
#define GST_CAT_DEFAULT mxf_debug

G_DEFINE_ABSTRACT_TYPE (MXFMetadataBase, mxf_metadata_base,
    GST_TYPE_MINI_OBJECT);

static void
mxf_metadata_base_finalize (GstMiniObject * object)
{
  MXFMetadataBase *self = MXF_METADATA_BASE (object);

  if (self->other_tags) {
    g_hash_table_destroy (self->other_tags);
    self->other_tags = NULL;
  }

  GST_MINI_OBJECT_CLASS (mxf_metadata_base_parent_class)->finalize (object);
}

static gboolean
mxf_metadata_base_handle_tag (MXFMetadataBase * self, MXFPrimerPack * primer,
    guint16 tag, const guint8 * tag_data, guint tag_size)
{
  return (mxf_local_tag_add_to_hash_table (primer, tag, tag_data, tag_size,
          &self->other_tags));
}

static gboolean
mxf_metadata_base_resolve_default (MXFMetadataBase * self,
    GHashTable * metadata)
{
  return TRUE;
}

#if !GLIB_CHECK_VERSION (2, 16, 0)
static void
build_values_in_hash_table (gpointer key, gpointer value, gpointer user_data)
{
  GList **valuelist = (GList **) (user_data);
  *valuelist = g_list_prepend (*valuelist, value);
}
#endif

static GstStructure *
mxf_metadata_base_to_structure_default (MXFMetadataBase * self)
{
  MXFMetadataBaseClass *klass = MXF_METADATA_BASE_GET_CLASS (self);
  GstStructure *ret;
  gchar str[48];

  g_return_val_if_fail (klass->name_quark != 0, NULL);

  ret = gst_structure_id_empty_new (klass->name_quark);

  if (!mxf_ul_is_zero (&self->instance_uid)) {
    mxf_ul_to_string (&self->instance_uid, str);
    gst_structure_id_set (ret, MXF_QUARK (INSTANCE_UID), G_TYPE_STRING, str,
        NULL);
  }

  if (!mxf_ul_is_zero (&self->generation_uid)) {
    mxf_ul_to_string (&self->generation_uid, str);
    gst_structure_id_set (ret, MXF_QUARK (GENERATION_UID), G_TYPE_STRING, str,
        NULL);
  }

  if (self->other_tags) {
    MXFLocalTag *tag;
    GValue va = { 0, };
    GValue v = { 0, };
    GstStructure *s;
    GstBuffer *buf;
#if GLIB_CHECK_VERSION (2, 16, 0)
    GHashTableIter iter;

    g_hash_table_iter_init (&iter, self->other_tags);
#else
    GList *l, *values = NULL;

    g_hash_table_foreach (self->other_tags, build_values_in_hash_table,
        &values);
#endif

    g_value_init (&va, GST_TYPE_ARRAY);

#if GLIB_CHECK_VERSION (2, 16, 0)
    while (g_hash_table_iter_next (&iter, NULL, (gpointer) & tag)) {
#else
    for (l = values; l; l = l->next) {
      tag = l->data;
#endif

      g_value_init (&v, GST_TYPE_STRUCTURE);
      s = gst_structure_id_empty_new (MXF_QUARK (TAG));

      mxf_ul_to_string (&tag->key, str);

      buf = gst_buffer_new_and_alloc (tag->size);
      memcpy (GST_BUFFER_DATA (buf), tag->data, tag->size);

      gst_structure_id_set (s, MXF_QUARK (NAME), G_TYPE_STRING, str,
          MXF_QUARK (DATA), GST_TYPE_BUFFER, buf, NULL);

      gst_value_set_structure (&v, s);
      gst_structure_free (s);
      gst_buffer_unref (buf);
      gst_value_array_append_value (&va, &v);
      g_value_unset (&v);
    }

    gst_structure_id_set_value (ret, MXF_QUARK (OTHER_TAGS), &va);
    g_value_unset (&va);

#if !GLIB_CHECK_VERSION (2, 16, 0)
    g_list_free (values);
#endif
  }

  return ret;
}

static void
mxf_metadata_base_init (MXFMetadataBase * self)
{

}

static void
mxf_metadata_base_class_init (MXFMetadataBaseClass * klass)
{
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;

  miniobject_class->finalize = mxf_metadata_base_finalize;
  klass->handle_tag = mxf_metadata_base_handle_tag;
  klass->resolve = mxf_metadata_base_resolve_default;
  klass->to_structure = mxf_metadata_base_to_structure_default;
}

gboolean
mxf_metadata_base_parse (MXFMetadataBase * self, MXFPrimerPack * primer,
    const guint8 * data, guint size)
{
  guint16 tag, tag_size;
  const guint8 *tag_data;

  g_return_val_if_fail (MXF_IS_METADATA_BASE (self), FALSE);
  g_return_val_if_fail (data != NULL, FALSE);
  g_return_val_if_fail (primer != NULL, FALSE);

  while (mxf_local_tag_parse (data, size, &tag, &tag_size, &tag_data)) {
    if (tag_size == 0 || tag == 0x0000)
      goto next;

    if (!MXF_METADATA_BASE_GET_CLASS (self)->handle_tag (self, primer, tag,
            tag_data, tag_size))
      return FALSE;
  next:
    data += 4 + tag_size;
    size -= 4 + tag_size;
  }

  return TRUE;
}

gboolean
mxf_metadata_base_resolve (MXFMetadataBase * self, GHashTable * metadata)
{
  MXFMetadataBaseClass *klass;
  gboolean ret = TRUE;

  g_return_val_if_fail (MXF_IS_METADATA_BASE (self), FALSE);
  g_return_val_if_fail (metadata != NULL, FALSE);

  if (self->resolved == MXF_METADATA_BASE_RESOLVE_STATE_SUCCESS)
    return TRUE;
  else if (self->resolved != MXF_METADATA_BASE_RESOLVE_STATE_NONE)
    return FALSE;

  self->resolved = MXF_METADATA_BASE_RESOLVE_STATE_RUNNING;

  klass = MXF_METADATA_BASE_GET_CLASS (self);

  if (klass->resolve)
    ret = klass->resolve (self, metadata);

  self->resolved =
      (ret) ? MXF_METADATA_BASE_RESOLVE_STATE_SUCCESS :
      MXF_METADATA_BASE_RESOLVE_STATE_FAILURE;

  return ret;
}

GstStructure *
mxf_metadata_base_to_structure (MXFMetadataBase * self)
{
  MXFMetadataBaseClass *klass;

  g_return_val_if_fail (MXF_IS_METADATA_BASE (self), NULL);

  g_return_val_if_fail (self->resolved ==
      MXF_METADATA_BASE_RESOLVE_STATE_SUCCESS, NULL);

  klass = MXF_METADATA_BASE_GET_CLASS (self);

  if (klass->to_structure)
    return klass->to_structure (self);

  return NULL;
}

G_DEFINE_ABSTRACT_TYPE (MXFMetadata, mxf_metadata, MXF_TYPE_METADATA_BASE);

static gboolean
mxf_metadata_handle_tag (MXFMetadataBase * metadata, MXFPrimerPack * primer,
    guint16 tag, const guint8 * tag_data, guint tag_size)
{
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif
  MXFMetadata *self = MXF_METADATA (metadata);
  gboolean ret = TRUE;

  switch (tag) {
    case 0x3c0a:
      if (tag_size != 16)
        goto error;
      memcpy (&self->parent.instance_uid, tag_data, 16);
      GST_DEBUG ("  instance uid = %s",
          mxf_ul_to_string (&self->parent.instance_uid, str));
      break;
    case 0x0102:
      if (tag_size != 16)
        goto error;
      memcpy (&self->parent.generation_uid, tag_data, 16);
      GST_DEBUG ("  generation uid = %s",
          mxf_ul_to_string (&self->parent.generation_uid, str));
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS (mxf_metadata_parent_class)->handle_tag
          (metadata, primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid metadata local tag 0x%04x of size %u", tag, tag_size);

  return FALSE;
}

static void
mxf_metadata_class_init (MXFMetadataClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;

  metadata_base_class->handle_tag = mxf_metadata_handle_tag;
}

static void
mxf_metadata_init (MXFMetadata * self)
{
}

static GArray *_mxf_metadata_registry = NULL;

#define _add_metadata_type(type) G_STMT_START { \
  GType t = type; \
  \
  g_array_append_val (_mxf_metadata_registry, t); \
} G_STMT_END

void
mxf_metadata_init_types (void)
{
  g_return_if_fail (_mxf_metadata_registry == NULL);

  _mxf_metadata_registry = g_array_new (FALSE, TRUE, sizeof (GType));

  _add_metadata_type (MXF_TYPE_METADATA_PREFACE);
  _add_metadata_type (MXF_TYPE_METADATA_IDENTIFICATION);
  _add_metadata_type (MXF_TYPE_METADATA_CONTENT_STORAGE);
  _add_metadata_type (MXF_TYPE_METADATA_ESSENCE_CONTAINER_DATA);
  _add_metadata_type (MXF_TYPE_METADATA_MATERIAL_PACKAGE);
  _add_metadata_type (MXF_TYPE_METADATA_SOURCE_PACKAGE);
  _add_metadata_type (MXF_TYPE_METADATA_TIMELINE_TRACK);
  _add_metadata_type (MXF_TYPE_METADATA_EVENT_TRACK);
  _add_metadata_type (MXF_TYPE_METADATA_STATIC_TRACK);
  _add_metadata_type (MXF_TYPE_METADATA_SEQUENCE);
  _add_metadata_type (MXF_TYPE_METADATA_SOURCE_CLIP);
  _add_metadata_type (MXF_TYPE_METADATA_TIMECODE_COMPONENT);
  _add_metadata_type (MXF_TYPE_METADATA_DM_SEGMENT);
  _add_metadata_type (MXF_TYPE_METADATA_DM_SOURCE_CLIP);
  _add_metadata_type (MXF_TYPE_METADATA_FILE_DESCRIPTOR);
  _add_metadata_type (MXF_TYPE_METADATA_GENERIC_PICTURE_ESSENCE_DESCRIPTOR);
  _add_metadata_type (MXF_TYPE_METADATA_CDCI_PICTURE_ESSENCE_DESCRIPTOR);
  _add_metadata_type (MXF_TYPE_METADATA_RGBA_PICTURE_ESSENCE_DESCRIPTOR);
  _add_metadata_type (MXF_TYPE_METADATA_GENERIC_SOUND_ESSENCE_DESCRIPTOR);
  _add_metadata_type (MXF_TYPE_METADATA_GENERIC_DATA_ESSENCE_DESCRIPTOR);
  _add_metadata_type (MXF_TYPE_METADATA_MULTIPLE_DESCRIPTOR);
  _add_metadata_type (MXF_TYPE_METADATA_NETWORK_LOCATOR);
  _add_metadata_type (MXF_TYPE_METADATA_TEXT_LOCATOR);
}

#undef _add_metadata_type

void
mxf_metadata_register (GType type)
{
  g_return_if_fail (g_type_is_a (type, MXF_TYPE_METADATA));

  g_array_append_val (_mxf_metadata_registry, type);
}

MXFMetadata *
mxf_metadata_new (guint16 type, MXFPrimerPack * primer, guint64 offset,
    const guint8 * data, guint size)
{
  guint i;
  GType t = G_TYPE_INVALID;
  MXFMetadata *ret = NULL;

  g_return_val_if_fail (type != 0, NULL);
  g_return_val_if_fail (primer != NULL, NULL);
  g_return_val_if_fail (_mxf_metadata_registry != NULL, NULL);

  for (i = 0; i < _mxf_metadata_registry->len; i++) {
    GType tmp = g_array_index (_mxf_metadata_registry, GType, i);
    MXFMetadataClass *klass = MXF_METADATA_CLASS (g_type_class_ref (tmp));

    if (klass->type == type) {
      g_type_class_unref (klass);
      t = tmp;
      break;
    }
    g_type_class_unref (klass);
  }

  if (t == G_TYPE_INVALID) {
    GST_WARNING
        ("No handler for type 0x%04x found -- using generic metadata parser",
        type);
    return NULL;
  }


  GST_DEBUG ("Metadata type 0x%06x is handled by type %s", type,
      g_type_name (t));

  ret = (MXFMetadata *) g_type_create_instance (t);
  if (!mxf_metadata_base_parse (MXF_METADATA_BASE (ret), primer, data, size)) {
    GST_ERROR ("Parsing metadata failed");
    gst_mini_object_unref ((GstMiniObject *) ret);
    return NULL;
  }

  ret->parent.offset = offset;
  return ret;
}

G_DEFINE_TYPE (MXFMetadataPreface, mxf_metadata_preface, MXF_TYPE_METADATA);

static void
mxf_metadata_preface_finalize (GstMiniObject * object)
{
  MXFMetadataPreface *self = MXF_METADATA_PREFACE (object);

  g_free (self->identifications_uids);
  self->identifications_uids = NULL;

  g_free (self->identifications);
  self->identifications = NULL;

  g_free (self->essence_containers);
  self->essence_containers = NULL;

  g_free (self->dm_schemes);
  self->dm_schemes = NULL;

  GST_MINI_OBJECT_CLASS (mxf_metadata_preface_parent_class)->finalize (object);
}

static gboolean
mxf_metadata_preface_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataPreface *self = MXF_METADATA_PREFACE (metadata);
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif
  gboolean ret = TRUE;

  switch (tag) {
    case 0x3b02:
      if (!mxf_timestamp_parse (&self->last_modified_date, tag_data, tag_size))
        goto error;
      GST_DEBUG ("  last modified date = %s",
          mxf_timestamp_to_string (&self->last_modified_date, str));
      break;
    case 0x3b05:
      if (tag_size != 2)
        goto error;
      self->version = GST_READ_UINT16_BE (tag_data);
      GST_DEBUG ("  version = %u.%u", (self->version >> 8),
          (self->version & 0x0f));
      break;
    case 0x3b07:
      if (tag_size != 4)
        goto error;
      self->object_model_version = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  object model version = %u", self->object_model_version);
      break;
    case 0x3b08:
      if (tag_size != 16)
        goto error;
      memcpy (&self->primary_package_uid, tag_data, 16);
      GST_DEBUG ("  primary package = %s",
          mxf_ul_to_string (&self->primary_package_uid, str));
      break;
    case 0x3b06:
      if (!mxf_ul_array_parse (&self->identifications_uids,
              &self->n_identifications, tag_data, tag_size))
        goto error;

      GST_DEBUG ("  number of identifications = %u", self->n_identifications);
#ifndef GST_DISABLE_GST_DEBUG
      {
        guint i;
        for (i = 0; i < self->n_identifications; i++) {
          GST_DEBUG ("  identification %u = %s", i,
              mxf_ul_to_string (&self->identifications_uids[i], str));
        }
      }
#endif
      break;
    case 0x3b03:
      if (tag_size != 16)
        goto error;
      memcpy (&self->content_storage_uid, tag_data, 16);
      GST_DEBUG ("  content storage = %s",
          mxf_ul_to_string (&self->content_storage_uid, str));
      break;
    case 0x3b09:
      if (tag_size != 16)
        goto error;
      memcpy (&self->operational_pattern, tag_data, 16);
      GST_DEBUG ("  operational pattern = %s",
          mxf_ul_to_string (&self->operational_pattern, str));
      break;
    case 0x3b0a:
      if (!mxf_ul_array_parse (&self->essence_containers,
              &self->n_essence_containers, tag_data, tag_size))
        goto error;

      GST_DEBUG ("  number of essence containers = %u",
          self->n_essence_containers);
#ifndef GST_DISABLE_GST_DEBUG
      {
        guint i;
        for (i = 0; i < self->n_essence_containers; i++) {
          GST_DEBUG ("  essence container %u = %s", i,
              mxf_ul_to_string (&self->essence_containers[i], str));
        }
      }
#endif
      break;
    case 0x3b0b:
      if (!mxf_ul_array_parse (&self->dm_schemes, &self->n_dm_schemes, tag_data,
              tag_size))
        goto error;
      GST_DEBUG ("  number of DM schemes = %u", self->n_dm_schemes);

#ifndef GST_DISABLE_GST_DEBUG
      {
        guint i;
        for (i = 0; i < self->n_dm_schemes; i++) {
          GST_DEBUG ("  DM schemes %u = %s", i,
              mxf_ul_to_string (&self->dm_schemes[i], str));
        }
      }
#endif
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_preface_parent_class)->handle_tag (metadata, primer,
          tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid preface local tag 0x%04x of size %u", tag, tag_size);

  return FALSE;
}

static gboolean
mxf_metadata_preface_resolve (MXFMetadataBase * m, GHashTable * metadata)
{
  MXFMetadataPreface *self = MXF_METADATA_PREFACE (m);
  MXFMetadataBase *current = NULL;
  guint i;

  if (!mxf_ul_is_zero (&self->primary_package_uid)) {
    current = g_hash_table_lookup (metadata, &self->primary_package_uid);
    if (!current || !MXF_IS_METADATA_GENERIC_PACKAGE (current)) {
      GST_ERROR ("Primary package not found");
    } else {
      if (mxf_metadata_base_resolve (current, metadata)) {
        self->primary_package = MXF_METADATA_GENERIC_PACKAGE (current);
      }
    }
  }
  current = NULL;

  current = g_hash_table_lookup (metadata, &self->content_storage_uid);
  if (!current || !MXF_IS_METADATA_CONTENT_STORAGE (current)) {
    GST_ERROR ("Content storage not found");
    return FALSE;
  } else {
    if (mxf_metadata_base_resolve (current, metadata)) {
      self->content_storage = MXF_METADATA_CONTENT_STORAGE (current);
    } else {
      GST_ERROR ("Couldn't resolve content storage");
      return FALSE;
    }
  }
  current = NULL;

  if (self->identifications)
    memset (self->identifications, 0,
        sizeof (gpointer) * self->n_identifications);
  else
    self->identifications =
        g_new0 (MXFMetadataIdentification *, self->n_identifications);
  for (i = 0; i < self->n_identifications; i++) {
    current = g_hash_table_lookup (metadata, &self->identifications_uids[i]);
    if (current && MXF_IS_METADATA_IDENTIFICATION (current)) {
      if (mxf_metadata_base_resolve (current, metadata))
        self->identifications[i] = MXF_METADATA_IDENTIFICATION (current);
    }
    current = NULL;
  }

  return
      MXF_METADATA_BASE_CLASS (mxf_metadata_preface_parent_class)->resolve (m,
      metadata);
}

static GstStructure *
mxf_metadata_preface_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS (mxf_metadata_preface_parent_class)->to_structure
      (m);
  MXFMetadataPreface *self = MXF_METADATA_PREFACE (m);
  gchar str[48];
  guint i;

  if (!mxf_timestamp_is_unknown (&self->last_modified_date)) {
    mxf_timestamp_to_string (&self->last_modified_date, str);
    gst_structure_id_set (ret, MXF_QUARK (LAST_MODIFIED_DATE), G_TYPE_STRING,
        str, NULL);
  }

  if (self->version != 0)
    gst_structure_id_set (ret, MXF_QUARK (VERSION), G_TYPE_UINT, self->version,
        NULL);

  if (self->object_model_version != 0)
    gst_structure_id_set (ret, MXF_QUARK (OBJECT_MODEL_VERSION), G_TYPE_UINT,
        self->object_model_version, NULL);

  if (!mxf_ul_is_zero (&self->primary_package_uid)) {
    mxf_ul_to_string (&self->primary_package_uid, str);
    gst_structure_id_set (ret, MXF_QUARK (PRIMARY_PACKAGE), G_TYPE_STRING, str,
        NULL);
  }

  if (self->n_identifications > 0) {
    GValue arr = { 0, }
    , val = {
    0,};

    g_value_init (&arr, GST_TYPE_ARRAY);

    for (i = 0; i < self->n_identifications; i++) {
      GstStructure *s;

      if (self->identifications[i] == NULL)
        continue;

      g_value_init (&val, GST_TYPE_STRUCTURE);

      s = mxf_metadata_base_to_structure (MXF_METADATA_BASE
          (self->identifications[i]));
      gst_value_set_structure (&val, s);
      gst_structure_free (s);
      gst_value_array_append_value (&arr, &val);
      g_value_unset (&val);
    }

    if (gst_value_array_get_size (&arr) > 0)
      gst_structure_id_set_value (ret, MXF_QUARK (IDENTIFICATIONS), &arr);

    g_value_unset (&arr);
  }

  if (self->content_storage) {
    GstStructure *s =
        mxf_metadata_base_to_structure (MXF_METADATA_BASE
        (self->content_storage));
    gst_structure_id_set (ret, MXF_QUARK (CONTENT_STORAGE), GST_TYPE_STRUCTURE,
        s, NULL);
    gst_structure_free (s);
  }

  if (!mxf_ul_is_zero (&self->operational_pattern)) {
    mxf_ul_to_string (&self->operational_pattern, str);
    gst_structure_id_set (ret, MXF_QUARK (OPERATIONAL_PATTERN), G_TYPE_STRING,
        str, NULL);
  }

  if (self->n_essence_containers > 0) {
    GValue arr = { 0, }
    , val = {
    0,};

    g_value_init (&arr, GST_TYPE_ARRAY);

    for (i = 0; i < self->n_essence_containers; i++) {
      if (mxf_ul_is_zero (&self->essence_containers[i]))
        continue;

      g_value_init (&val, G_TYPE_STRING);

      mxf_ul_to_string (&self->essence_containers[i], str);
      g_value_set_string (&val, str);

      gst_value_array_append_value (&arr, &val);
      g_value_unset (&val);
    }

    if (gst_value_array_get_size (&arr) > 0)
      gst_structure_id_set_value (ret, MXF_QUARK (ESSENCE_CONTAINERS), &arr);

    g_value_unset (&arr);
  }

  if (self->n_dm_schemes > 0) {
    GValue arr = { 0, }
    , val = {
    0,};

    g_value_init (&arr, GST_TYPE_ARRAY);

    for (i = 0; i < self->n_dm_schemes; i++) {
      if (mxf_ul_is_zero (&self->dm_schemes[i]))
        continue;

      g_value_init (&val, G_TYPE_STRING);

      mxf_ul_to_string (&self->dm_schemes[i], str);
      g_value_set_string (&val, str);

      gst_value_array_append_value (&arr, &val);
      g_value_unset (&val);
    }

    if (gst_value_array_get_size (&arr) > 0)
      gst_structure_id_set_value (ret, MXF_QUARK (DM_SCHEMES), &arr);

    g_value_unset (&arr);
  }

  return ret;
}

static void
mxf_metadata_preface_init (MXFMetadataPreface * self)
{

}

static void
mxf_metadata_preface_class_init (MXFMetadataPrefaceClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  miniobject_class->finalize = mxf_metadata_preface_finalize;
  metadata_base_class->handle_tag = mxf_metadata_preface_handle_tag;
  metadata_base_class->resolve = mxf_metadata_preface_resolve;
  metadata_base_class->to_structure = mxf_metadata_preface_to_structure;
  metadata_base_class->name_quark = MXF_QUARK (PREFACE);
  metadata_class->type = 0x012f;
}

G_DEFINE_TYPE (MXFMetadataIdentification, mxf_metadata_identification,
    MXF_TYPE_METADATA);

static void
mxf_metadata_identification_finalize (GstMiniObject * object)
{
  MXFMetadataIdentification *self = MXF_METADATA_IDENTIFICATION (object);

  g_free (self->company_name);
  self->company_name = NULL;

  g_free (self->product_name);
  self->product_name = NULL;

  g_free (self->version_string);
  self->version_string = NULL;

  g_free (self->platform);
  self->platform = NULL;

  GST_MINI_OBJECT_CLASS (mxf_metadata_identification_parent_class)->finalize
      (object);
}

static gboolean
mxf_metadata_identification_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataIdentification *self = MXF_METADATA_IDENTIFICATION (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif

  switch (tag) {
    case 0x3c09:
      if (tag_size != 16)
        goto error;
      memcpy (&self->this_generation_uid, tag_data, 16);
      GST_DEBUG ("  this generation uid = %s",
          mxf_ul_to_string (&self->this_generation_uid, str));
      break;
    case 0x3c01:
      self->company_name = mxf_utf16_to_utf8 (tag_data, tag_size);
      GST_DEBUG ("  company name = %s", GST_STR_NULL (self->company_name));
      break;
    case 0x3c02:
      self->product_name = mxf_utf16_to_utf8 (tag_data, tag_size);
      GST_DEBUG ("  product name = %s", GST_STR_NULL (self->product_name));
      break;
    case 0x3c03:
      if (!mxf_product_version_parse (&self->product_version,
              tag_data, tag_size))
        goto error;
      GST_DEBUG ("  product version = %u.%u.%u.%u.%u",
          self->product_version.major,
          self->product_version.minor,
          self->product_version.patch,
          self->product_version.build, self->product_version.release);
      break;
    case 0x3c04:
      self->version_string = mxf_utf16_to_utf8 (tag_data, tag_size);
      GST_DEBUG ("  version string = %s", GST_STR_NULL (self->version_string));
      break;
    case 0x3c05:
      if (tag_size != 16)
        goto error;
      memcpy (&self->product_uid, tag_data, 16);
      GST_DEBUG ("  product uid = %s",
          mxf_ul_to_string (&self->product_uid, str));
      break;
    case 0x3c06:
      if (!mxf_timestamp_parse (&self->modification_date, tag_data, tag_size))
        goto error;
      GST_DEBUG ("  modification date = %s",
          mxf_timestamp_to_string (&self->modification_date, str));
      break;
    case 0x3c07:
      if (!mxf_product_version_parse (&self->toolkit_version,
              tag_data, tag_size))
        goto error;
      GST_DEBUG ("  toolkit version = %u.%u.%u.%u.%u",
          self->toolkit_version.major,
          self->toolkit_version.minor,
          self->toolkit_version.patch,
          self->toolkit_version.build, self->toolkit_version.release);
      break;
    case 0x3c08:
      self->platform = mxf_utf16_to_utf8 (tag_data, tag_size);
      GST_DEBUG ("  platform = %s", GST_STR_NULL (self->platform));
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_identification_parent_class)->handle_tag (metadata,
          primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:
  GST_ERROR ("Invalid identification local tag 0x%04x of size %u", tag,
      tag_size);

  return FALSE;
}

static GstStructure *
mxf_metadata_identification_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_identification_parent_class)->to_structure (m);
  MXFMetadataIdentification *self = MXF_METADATA_IDENTIFICATION (m);
  gchar str[48];

  if (!mxf_ul_is_zero (&self->this_generation_uid)) {
    mxf_ul_to_string (&self->this_generation_uid, str);
    gst_structure_id_set (ret, MXF_QUARK (THIS_GENERATION_UID), G_TYPE_STRING,
        str, NULL);
  }

  if (self->company_name)
    gst_structure_id_set (ret, MXF_QUARK (COMPANY_NAME), G_TYPE_STRING,
        self->company_name, NULL);

  if (self->product_name)
    gst_structure_id_set (ret, MXF_QUARK (PRODUCT_NAME), G_TYPE_STRING,
        self->product_name, NULL);

  if (self->product_version.major ||
      self->product_version.minor ||
      self->product_version.patch ||
      self->product_version.build || self->product_version.release) {
    g_snprintf (str, 48, "%u.%u.%u.%u.%u", self->product_version.major,
        self->product_version.minor,
        self->product_version.patch,
        self->product_version.build, self->product_version.release);
    gst_structure_id_set (ret, MXF_QUARK (PRODUCT_VERSION), G_TYPE_STRING, str,
        NULL);
  }

  if (self->version_string)
    gst_structure_id_set (ret, MXF_QUARK (VERSION_STRING), G_TYPE_STRING,
        self->version_string, NULL);

  if (!mxf_ul_is_zero (&self->product_uid)) {
    mxf_ul_to_string (&self->product_uid, str);
    gst_structure_id_set (ret, MXF_QUARK (PRODUCT_UID), G_TYPE_STRING, str,
        NULL);
  }

  if (!mxf_timestamp_is_unknown (&self->modification_date)) {
    mxf_timestamp_to_string (&self->modification_date, str);
    gst_structure_id_set (ret, MXF_QUARK (MODIFICATION_DATE), G_TYPE_STRING,
        str, NULL);
  }

  if (self->toolkit_version.major ||
      self->toolkit_version.minor ||
      self->toolkit_version.patch ||
      self->toolkit_version.build || self->toolkit_version.release) {
    g_snprintf (str, 48, "%u.%u.%u.%u.%u", self->toolkit_version.major,
        self->toolkit_version.minor,
        self->toolkit_version.patch,
        self->toolkit_version.build, self->toolkit_version.release);
    gst_structure_id_set (ret, MXF_QUARK (TOOLKIT_VERSION), G_TYPE_STRING, str,
        NULL);
  }

  if (self->platform)
    gst_structure_id_set (ret, MXF_QUARK (PLATFORM), G_TYPE_STRING,
        self->platform, NULL);

  return ret;
}

static void
mxf_metadata_identification_init (MXFMetadataIdentification * self)
{

}

static void
mxf_metadata_identification_class_init (MXFMetadataIdentificationClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  miniobject_class->finalize = mxf_metadata_identification_finalize;
  metadata_base_class->handle_tag = mxf_metadata_identification_handle_tag;
  metadata_base_class->name_quark = MXF_QUARK (IDENTIFICATION);
  metadata_base_class->to_structure = mxf_metadata_identification_to_structure;
  metadata_class->type = 0x0130;
}

G_DEFINE_TYPE (MXFMetadataContentStorage, mxf_metadata_content_storage,
    MXF_TYPE_METADATA);

static void
mxf_metadata_content_storage_finalize (GstMiniObject * object)
{
  MXFMetadataContentStorage *self = MXF_METADATA_CONTENT_STORAGE (object);

  g_free (self->packages);
  self->packages = NULL;
  g_free (self->packages_uids);
  self->packages_uids = NULL;
  g_free (self->essence_container_data);
  self->essence_container_data = NULL;
  g_free (self->essence_container_data_uids);
  self->essence_container_data_uids = NULL;

  GST_MINI_OBJECT_CLASS (mxf_metadata_content_storage_parent_class)->finalize
      (object);
}

static gboolean
mxf_metadata_content_storage_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataContentStorage *self = MXF_METADATA_CONTENT_STORAGE (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif

  switch (tag) {
    case 0x1901:
      if (!mxf_ul_array_parse (&self->packages_uids, &self->n_packages,
              tag_data, tag_size))
        goto error;
      GST_DEBUG ("  number of packages = %u", self->n_packages);
#ifndef GST_DISABLE_GST_DEBUG
      {
        guint i;
        for (i = 0; i < self->n_packages; i++) {
          GST_DEBUG ("  package %u = %s", i,
              mxf_ul_to_string (&self->packages_uids[i], str));
        }
      }
#endif
      break;
    case 0x1902:
      if (!mxf_ul_array_parse (&self->essence_container_data_uids,
              &self->n_essence_container_data, tag_data, tag_size))
        goto error;

      GST_DEBUG ("  number of essence container data = %u",
          self->n_essence_container_data);
#ifndef GST_DISABLE_GST_DEBUG
      {
        guint i;
        for (i = 0; i < self->n_essence_container_data; i++) {
          GST_DEBUG ("  essence container data %u = %s", i,
              mxf_ul_to_string (&self->essence_container_data_uids[i], str));
        }
      }
#endif
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_content_storage_parent_class)->handle_tag (metadata,
          primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid content storage local tag 0x%04x of size %u", tag,
      tag_size);

  return FALSE;
}

static gboolean
mxf_metadata_content_storage_resolve (MXFMetadataBase * m,
    GHashTable * metadata)
{
  MXFMetadataContentStorage *self = MXF_METADATA_CONTENT_STORAGE (m);
  MXFMetadataBase *current = NULL;
  guint i;
  gboolean have_package = FALSE;
  gboolean have_ecd = FALSE;

  if (self->packages)
    memset (self->packages, 0, sizeof (gpointer) * self->n_packages);
  else
    self->packages = g_new0 (MXFMetadataGenericPackage *, self->n_packages);

  for (i = 0; i < self->n_packages; i++) {
    current = g_hash_table_lookup (metadata, &self->packages_uids[i]);
    if (current && MXF_IS_METADATA_GENERIC_PACKAGE (current)) {
      if (mxf_metadata_base_resolve (current, metadata)) {
        self->packages[i] = MXF_METADATA_GENERIC_PACKAGE (current);
        have_package = TRUE;
      } else {
        GST_ERROR ("Couldn't resolve package");
      }
    } else {
      GST_ERROR ("Package not found");
    }
  }

  if (self->essence_container_data)
    memset (self->essence_container_data, 0,
        sizeof (gpointer) * self->n_essence_container_data);
  else
    self->essence_container_data =
        g_new0 (MXFMetadataEssenceContainerData *,
        self->n_essence_container_data);
  for (i = 0; i < self->n_essence_container_data; i++) {
    current =
        g_hash_table_lookup (metadata, &self->essence_container_data_uids[i]);
    if (current && MXF_IS_METADATA_ESSENCE_CONTAINER_DATA (current)) {
      if (mxf_metadata_base_resolve (current, metadata)) {
        self->essence_container_data[i] =
            MXF_METADATA_ESSENCE_CONTAINER_DATA (current);
        have_ecd = TRUE;
      } else {
        GST_ERROR ("Couldn't resolve essence container data");
      }
    } else {
      GST_ERROR ("Essence container data not found");
    }
  }

  if (!have_package) {
    GST_ERROR ("Couldn't resolve any package");
    return FALSE;
  } else if (!have_ecd) {
    GST_ERROR ("Couldn't resolve any essence container data");
    return FALSE;
  }

  return
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_content_storage_parent_class)->resolve (m, metadata);
}

static GstStructure *
mxf_metadata_content_storage_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_content_storage_parent_class)->to_structure (m);
  MXFMetadataContentStorage *self = MXF_METADATA_CONTENT_STORAGE (m);
  guint i;

  if (self->n_packages > 0) {
    GValue arr = { 0, }
    , val = {
    0,};

    g_value_init (&arr, GST_TYPE_ARRAY);

    for (i = 0; i < self->n_packages; i++) {
      GstStructure *s;

      if (self->packages[i] == NULL)
        continue;

      g_value_init (&val, GST_TYPE_STRUCTURE);

      s = mxf_metadata_base_to_structure (MXF_METADATA_BASE (self->packages
              [i]));
      gst_value_set_structure (&val, s);
      gst_structure_free (s);
      gst_value_array_append_value (&arr, &val);
      g_value_unset (&val);
    }

    if (gst_value_array_get_size (&arr) > 0)
      gst_structure_id_set_value (ret, MXF_QUARK (PACKAGES), &arr);

    g_value_unset (&arr);
  }

  if (self->n_essence_container_data > 0) {
    GValue arr = { 0, }
    , val = {
    0,};

    g_value_init (&arr, GST_TYPE_ARRAY);

    for (i = 0; i < self->n_essence_container_data; i++) {
      GstStructure *s;

      if (self->essence_container_data[i] == NULL)
        continue;

      g_value_init (&val, GST_TYPE_STRUCTURE);

      s = mxf_metadata_base_to_structure (MXF_METADATA_BASE
          (self->essence_container_data[i]));
      gst_value_set_structure (&val, s);
      gst_structure_free (s);
      gst_value_array_append_value (&arr, &val);
      g_value_unset (&val);
    }

    if (gst_value_array_get_size (&arr) > 0)
      gst_structure_id_set_value (ret, MXF_QUARK (ESSENCE_CONTAINER_DATA),
          &arr);

    g_value_unset (&arr);
  }

  return ret;
}

static void
mxf_metadata_content_storage_init (MXFMetadataContentStorage * self)
{

}

static void
mxf_metadata_content_storage_class_init (MXFMetadataContentStorageClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  miniobject_class->finalize = mxf_metadata_content_storage_finalize;
  metadata_base_class->handle_tag = mxf_metadata_content_storage_handle_tag;
  metadata_base_class->resolve = mxf_metadata_content_storage_resolve;
  metadata_base_class->name_quark = MXF_QUARK (CONTENT_STORAGE);
  metadata_base_class->to_structure = mxf_metadata_content_storage_to_structure;
  metadata_class->type = 0x0118;
}

G_DEFINE_TYPE (MXFMetadataEssenceContainerData,
    mxf_metadata_essence_container_data, MXF_TYPE_METADATA);

static gboolean
mxf_metadata_essence_container_data_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataEssenceContainerData *self =
      MXF_METADATA_ESSENCE_CONTAINER_DATA (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[96];
#endif

  switch (tag) {
    case 0x2701:
      if (tag_size != 32)
        goto error;
      memcpy (&self->linked_package_uid, tag_data, 32);
      GST_DEBUG ("  linked package = %s",
          mxf_umid_to_string (&self->linked_package_uid, str));
      break;
    case 0x3f06:
      if (tag_size != 4)
        goto error;
      self->index_sid = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  index sid = %u", self->index_sid);
      break;
    case 0x3f07:
      if (tag_size != 4)
        goto error;
      self->body_sid = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  body sid = %u", self->body_sid);
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_essence_container_data_parent_class)->handle_tag
          (metadata, primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid essence container data local tag 0x%04x of size %u", tag,
      tag_size);

  return FALSE;
}

static gboolean
mxf_metadata_essence_container_data_resolve (MXFMetadataBase * m,
    GHashTable * metadata)
{
  MXFMetadataEssenceContainerData *self =
      MXF_METADATA_ESSENCE_CONTAINER_DATA (m);
  MXFMetadataBase *current = NULL;
#if GLIB_CHECK_VERSION (2, 16, 0)
  GHashTableIter iter;

  g_hash_table_iter_init (&iter, metadata);
#else
  GList *l, *values = NULL;

  g_hash_table_foreach (metadata, build_values_in_hash_table, &values);
#endif

#if GLIB_CHECK_VERSION (2, 16, 0)
  while (g_hash_table_iter_next (&iter, NULL, (gpointer) & current)) {
#else
  for (l = values; l; l = l->next) {
    current = l->data;
#endif
    if (MXF_IS_METADATA_SOURCE_PACKAGE (current)) {
      MXFMetadataSourcePackage *package = MXF_METADATA_SOURCE_PACKAGE (current);

      if (mxf_umid_is_equal (&package->parent.package_uid,
              &self->linked_package_uid)) {
        if (mxf_metadata_base_resolve (current, metadata)) {
          self->linked_package = package;
        }
        break;
      }
    }
  }

#if !GLIB_CHECK_VERSION (2, 16, 0)
  g_list_free (values);
#endif

  if (!self->linked_package) {
    GST_ERROR ("Couldn't resolve a package");
    return FALSE;
  }

  return
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_essence_container_data_parent_class)->resolve (m, metadata);
}

static GstStructure *
mxf_metadata_essence_container_data_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_essence_container_data_parent_class)->to_structure (m);
  MXFMetadataEssenceContainerData *self =
      MXF_METADATA_ESSENCE_CONTAINER_DATA (m);
  gchar str[96];

  if (!mxf_umid_is_zero (&self->linked_package_uid)) {
    mxf_umid_to_string (&self->linked_package_uid, str);
    gst_structure_id_set (ret, MXF_QUARK (LINKED_PACKAGE), G_TYPE_STRING, str,
        NULL);
  }

  gst_structure_id_set (ret, MXF_QUARK (INDEX_SID), G_TYPE_UINT,
      self->index_sid, MXF_QUARK (BODY_SID), G_TYPE_UINT, self->body_sid, NULL);


  return ret;
}

static void
mxf_metadata_essence_container_data_init (MXFMetadataEssenceContainerData *
    self)
{

}

static void
    mxf_metadata_essence_container_data_class_init
    (MXFMetadataEssenceContainerDataClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  metadata_base_class->handle_tag =
      mxf_metadata_essence_container_data_handle_tag;
  metadata_base_class->resolve = mxf_metadata_essence_container_data_resolve;
  metadata_base_class->name_quark = MXF_QUARK (ESSENCE_CONTAINER_DATA);
  metadata_base_class->to_structure =
      mxf_metadata_essence_container_data_to_structure;
  metadata_class->type = 0x0123;
}

G_DEFINE_ABSTRACT_TYPE (MXFMetadataGenericPackage, mxf_metadata_generic_package,
    MXF_TYPE_METADATA);

static void
mxf_metadata_generic_package_finalize (GstMiniObject * object)
{
  MXFMetadataGenericPackage *self = MXF_METADATA_GENERIC_PACKAGE (object);

  g_free (self->name);
  self->name = NULL;
  g_free (self->tracks_uids);
  self->tracks_uids = NULL;

  g_free (self->tracks);
  self->tracks = NULL;

  GST_MINI_OBJECT_CLASS (mxf_metadata_generic_package_parent_class)->finalize
      (object);
}

static gboolean
mxf_metadata_generic_package_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataGenericPackage *self = MXF_METADATA_GENERIC_PACKAGE (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[96];
#endif

  switch (tag) {
    case 0x4401:
      if (tag_size != 32)
        goto error;
      memcpy (&self->package_uid, tag_data, 32);
      GST_DEBUG ("  UMID = %s", mxf_umid_to_string (&self->package_uid, str));
      break;
    case 0x4402:
      self->name = mxf_utf16_to_utf8 (tag_data, tag_size);
      GST_DEBUG ("  name = %s", GST_STR_NULL (self->name));
      break;
    case 0x4405:
      if (!mxf_timestamp_parse (&self->package_creation_date,
              tag_data, tag_size))
        goto error;
      GST_DEBUG ("  creation date = %s",
          mxf_timestamp_to_string (&self->package_creation_date, str));
      break;
    case 0x4404:
      if (!mxf_timestamp_parse (&self->package_modified_date,
              tag_data, tag_size))
        goto error;
      GST_DEBUG ("  modification date = %s",
          mxf_timestamp_to_string (&self->package_modified_date, str));
      break;
    case 0x4403:
      if (!mxf_ul_array_parse (&self->tracks_uids, &self->n_tracks, tag_data,
              tag_size))
        goto error;

      GST_DEBUG ("  number of tracks = %u", self->n_tracks);
#ifndef GST_DISABLE_GST_DEBUG
      {
        guint i;
        for (i = 0; i < self->n_tracks; i++) {
          GST_DEBUG ("  track %u = %s", i,
              mxf_ul_to_string (&self->tracks_uids[i], str));
        }
      }
#endif
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_generic_package_parent_class)->handle_tag (metadata,
          primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid generic package local tag 0x%04x of size %u", tag,
      tag_size);

  return FALSE;
}

static gboolean
mxf_metadata_generic_package_resolve (MXFMetadataBase * m,
    GHashTable * metadata)
{
  MXFMetadataGenericPackage *self = MXF_METADATA_GENERIC_PACKAGE (m);
  MXFMetadataBase *current = NULL;
  guint i;
  gboolean have_track = FALSE;

  if (self->tracks)
    memset (self->tracks, 0, sizeof (gpointer) * self->n_tracks);
  else
    self->tracks = g_new0 (MXFMetadataTrack *, self->n_tracks);
  for (i = 0; i < self->n_tracks; i++) {
    current = g_hash_table_lookup (metadata, &self->tracks_uids[i]);
    if (current && MXF_IS_METADATA_TRACK (current)) {
      if (mxf_metadata_base_resolve (current, metadata)) {
        MXFMetadataTrack *track = MXF_METADATA_TRACK (current);

        self->tracks[i] = track;
        have_track = TRUE;
        if ((track->type & 0xf0) == 0x10)
          self->n_timecode_tracks++;
        else if ((track->type & 0xf0) == 0x20)
          self->n_metadata_tracks++;
        else if ((track->type & 0xf0) == 0x30)
          self->n_essence_tracks++;
        else if ((track->type & 0xf0) == 0x40)
          self->n_other_tracks++;
      } else {
        GST_ERROR ("Track couldn't be resolved");
      }
    } else {
      GST_ERROR ("Track not found");
    }
  }

  if (!have_track) {
    GST_ERROR ("Couldn't resolve a track");
    return FALSE;
  }

  return
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_generic_package_parent_class)->resolve (m, metadata);
}

static GstStructure *
mxf_metadata_generic_package_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_generic_package_parent_class)->to_structure (m);
  MXFMetadataGenericPackage *self = MXF_METADATA_GENERIC_PACKAGE (m);
  guint i;
  gchar str[96];

  mxf_umid_to_string (&self->package_uid, str);
  gst_structure_id_set (ret, MXF_QUARK (PACKAGE_UID), G_TYPE_STRING, str, NULL);

  if (self->name)
    gst_structure_id_set (ret, MXF_QUARK (NAME), G_TYPE_STRING, self->name,
        NULL);

  if (!mxf_timestamp_is_unknown (&self->package_creation_date)) {
    mxf_timestamp_to_string (&self->package_creation_date, str);
    gst_structure_id_set (ret, MXF_QUARK (PACKAGE_CREATION_DATE), G_TYPE_STRING,
        str, NULL);
  }

  if (!mxf_timestamp_is_unknown (&self->package_modified_date)) {
    mxf_timestamp_to_string (&self->package_modified_date, str);
    gst_structure_id_set (ret, MXF_QUARK (PACKAGE_MODIFIED_DATE), G_TYPE_STRING,
        str, NULL);
  }

  if (self->n_tracks > 0) {
    GValue arr = { 0, }
    , val = {
    0,};

    g_value_init (&arr, GST_TYPE_ARRAY);

    for (i = 0; i < self->n_tracks; i++) {
      GstStructure *s;

      if (self->tracks[i] == NULL)
        continue;

      g_value_init (&val, GST_TYPE_STRUCTURE);

      s = mxf_metadata_base_to_structure (MXF_METADATA_BASE (self->tracks[i]));
      gst_value_set_structure (&val, s);
      gst_structure_free (s);
      gst_value_array_append_value (&arr, &val);
      g_value_unset (&val);
    }

    if (gst_value_array_get_size (&arr) > 0)
      gst_structure_id_set_value (ret, MXF_QUARK (TRACKS), &arr);

    g_value_unset (&arr);
  }

  return ret;
}

static void
mxf_metadata_generic_package_init (MXFMetadataGenericPackage * self)
{

}

static void
mxf_metadata_generic_package_class_init (MXFMetadataGenericPackageClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;

  miniobject_class->finalize = mxf_metadata_generic_package_finalize;
  metadata_base_class->handle_tag = mxf_metadata_generic_package_handle_tag;
  metadata_base_class->resolve = mxf_metadata_generic_package_resolve;
  metadata_base_class->to_structure = mxf_metadata_generic_package_to_structure;
}

G_DEFINE_TYPE (MXFMetadataMaterialPackage, mxf_metadata_material_package,
    MXF_TYPE_METADATA_GENERIC_PACKAGE);

static gboolean
mxf_metadata_material_package_resolve (MXFMetadataBase * m,
    GHashTable * metadata)
{
  gboolean ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_material_package_parent_class)->resolve (m, metadata);
  MXFMetadataGenericPackage *self = MXF_METADATA_GENERIC_PACKAGE (m);
  guint i;
  guint ntracks = 0;

  if (!ret)
    return ret;

  for (i = 0; i < self->n_tracks; i++) {
    MXFMetadataTrack *track = self->tracks[i];
    MXFMetadataSequence *sequence;
    guint j;

    if (!track)
      continue;

    sequence = track->sequence;

    if (!sequence || !sequence->structural_components)
      continue;

    for (j = 0; j < sequence->n_structural_components; j++) {
      MXFMetadataSourceClip *sc;
      MXFMetadataTimelineTrack *st = NULL;
      guint k;

      if (!sequence->structural_components[j]
          || !MXF_IS_METADATA_SOURCE_CLIP (sequence->structural_components[j]))
        continue;

      sc = MXF_METADATA_SOURCE_CLIP (sequence->structural_components[j]);

      if (!sc->source_package) {
        GST_ERROR ("Material package track %u without resolved source package",
            i);
        track = NULL;
        break;
      }

      if (!mxf_metadata_base_resolve (MXF_METADATA_BASE (sc->source_package),
              metadata)) {
        GST_ERROR ("Couldn't resolve source package for track %u", i);
        track = NULL;
        break;
      }

      sc->source_package->top_level = TRUE;
      for (k = 0; k < sc->source_package->parent.n_tracks; k++) {
        MXFMetadataTimelineTrack *tmp;

        if (!sc->source_package->parent.tracks[k] ||
            !MXF_IS_METADATA_TIMELINE_TRACK (sc->source_package->parent.
                tracks[k]))
          continue;

        tmp =
            MXF_METADATA_TIMELINE_TRACK (sc->source_package->parent.tracks[k]);
        if (tmp->parent.track_id == sc->source_track_id) {
          st = tmp;
          break;
        }
      }

      if (!st) {
        GST_ERROR ("Material package track %u without resolved source track",
            i);
        track = NULL;
      }
    }

    if (track)
      ntracks++;
    else
      self->tracks[i] = NULL;
  }

  if (ntracks == 0) {
    GST_ERROR ("No tracks could be resolved");
    return FALSE;
  } else if (ntracks != self->n_tracks) {
    GST_WARNING ("Not all tracks could be resolved");
  }

  return TRUE;
}

static void
mxf_metadata_material_package_init (MXFMetadataMaterialPackage * self)
{
}

static void
mxf_metadata_material_package_class_init (MXFMetadataMaterialPackageClass *
    klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  metadata_base_class->resolve = mxf_metadata_material_package_resolve;
  metadata_base_class->name_quark = MXF_QUARK (MATERIAL_PACKAGE);
  metadata_class->type = 0x0136;
}

G_DEFINE_TYPE (MXFMetadataSourcePackage, mxf_metadata_source_package,
    MXF_TYPE_METADATA_GENERIC_PACKAGE);

static gboolean
mxf_metadata_source_package_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataSourcePackage *self = MXF_METADATA_SOURCE_PACKAGE (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif

  switch (tag) {
    case 0x4701:
      if (tag_size != 16)
        goto error;

      memcpy (&self->descriptor_uid, tag_data, 16);
      GST_DEBUG ("  descriptor = %s",
          mxf_ul_to_string (&self->descriptor_uid, str));
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_source_package_parent_class)->handle_tag (metadata,
          primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid source package local tag 0x%04x of size %u", tag,
      tag_size);

  return FALSE;
}

static gboolean
mxf_metadata_source_package_resolve (MXFMetadataBase * m, GHashTable * metadata)
{
  MXFMetadataSourcePackage *self = MXF_METADATA_SOURCE_PACKAGE (m);
  MXFMetadataGenericPackage *package = MXF_METADATA_GENERIC_PACKAGE (m);
  MXFMetadataBase *current = NULL;
  guint i;
  gboolean ret;
  MXFMetadataFileDescriptor *d;

  if (mxf_ul_is_zero (&self->descriptor_uid))
    return
        MXF_METADATA_BASE_CLASS
        (mxf_metadata_source_package_parent_class)->resolve (m, metadata);

  current = g_hash_table_lookup (metadata, &self->descriptor_uid);
  if (!current) {
    GST_ERROR ("Descriptor not found");
    return FALSE;
  }

  if (!mxf_metadata_base_resolve (MXF_METADATA_BASE (current), metadata)) {
    GST_ERROR ("Couldn't resolve descriptor");
    return FALSE;
  }

  self->descriptor = MXF_METADATA_GENERIC_DESCRIPTOR (current);

  ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_source_package_parent_class)->resolve (m, metadata);

  if (!MXF_IS_METADATA_FILE_DESCRIPTOR (self->descriptor))
    return ret;

  d = MXF_METADATA_FILE_DESCRIPTOR (current);

  for (i = 0; i < package->n_tracks; i++) {
    if (!MXF_IS_METADATA_MULTIPLE_DESCRIPTOR (d)) {
      if (d->linked_track_id == package->tracks[i]->track_id ||
          (d->linked_track_id == 0 && package->n_essence_tracks == 1 &&
              (package->tracks[i]->type & 0xf0) == 0x30)) {
        package->tracks[i]->descriptor =
            g_new0 (MXFMetadataFileDescriptor *, 1);
        package->tracks[i]->descriptor[0] = d;
        package->tracks[i]->n_descriptor = 1;
        break;
      }
    } else {
      guint n_descriptor = 0, j, k = 0;
      MXFMetadataMultipleDescriptor *md = MXF_METADATA_MULTIPLE_DESCRIPTOR (d);

      for (j = 0; j < md->n_sub_descriptors; j++) {
        MXFMetadataFileDescriptor *fd;

        if (!md->sub_descriptors[j] ||
            !MXF_METADATA_FILE_DESCRIPTOR (md->sub_descriptors[j]))
          continue;

        fd = MXF_METADATA_FILE_DESCRIPTOR (md->sub_descriptors[j]);

        if (fd->linked_track_id == package->tracks[i]->track_id ||
            (fd->linked_track_id == 0 && package->n_essence_tracks == 1 &&
                (package->tracks[i]->type & 0xf0) == 0x30))
          n_descriptor++;
      }

      package->tracks[i]->descriptor =
          g_new0 (MXFMetadataFileDescriptor *, n_descriptor);
      package->tracks[i]->n_descriptor = n_descriptor;

      for (j = 0; j < md->n_sub_descriptors; j++) {
        MXFMetadataFileDescriptor *fd;

        if (!md->sub_descriptors[j] ||
            !MXF_METADATA_FILE_DESCRIPTOR (md->sub_descriptors[j]))
          continue;

        fd = MXF_METADATA_FILE_DESCRIPTOR (md->sub_descriptors[j]);

        if (fd->linked_track_id == package->tracks[i]->track_id ||
            (fd->linked_track_id == 0 && package->n_essence_tracks == 1 &&
                (package->tracks[i]->type & 0xf0) == 0x30)) {
          package->tracks[i]->descriptor[k] = fd;
          k++;
        }
      }
    }
  }

  return ret;
}

static GstStructure *
mxf_metadata_source_package_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_source_package_parent_class)->to_structure (m);
  MXFMetadataSourcePackage *self = MXF_METADATA_SOURCE_PACKAGE (m);
  GstStructure *s;

  if (!self->descriptor)
    return ret;

  s = mxf_metadata_base_to_structure (MXF_METADATA_BASE (self->descriptor));
  gst_structure_id_set (ret, MXF_QUARK (DESCRIPTOR), GST_TYPE_STRUCTURE, s,
      NULL);
  gst_structure_free (s);

  return ret;
}

static void
mxf_metadata_source_package_init (MXFMetadataSourcePackage * self)
{

}

static void
mxf_metadata_source_package_class_init (MXFMetadataSourcePackageClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  metadata_base_class->handle_tag = mxf_metadata_source_package_handle_tag;
  metadata_base_class->resolve = mxf_metadata_source_package_resolve;
  metadata_base_class->name_quark = MXF_QUARK (SOURCE_PACKAGE);
  metadata_base_class->to_structure = mxf_metadata_source_package_to_structure;
  metadata_class->type = 0x0137;
}

G_DEFINE_ABSTRACT_TYPE (MXFMetadataTrack, mxf_metadata_track,
    MXF_TYPE_METADATA);

static void
mxf_metadata_track_finalize (GstMiniObject * object)
{
  MXFMetadataTrack *self = MXF_METADATA_TRACK (object);

  g_free (self->track_name);
  self->track_name = NULL;
  g_free (self->descriptor);
  self->descriptor = NULL;

  GST_MINI_OBJECT_CLASS (mxf_metadata_track_parent_class)->finalize (object);
}

static gboolean
mxf_metadata_track_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataTrack *self = MXF_METADATA_TRACK (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif

  switch (tag) {
    case 0x4801:
      if (tag_size != 4)
        goto error;
      self->track_id = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  track id = %u", self->track_id);
      break;
    case 0x4804:
      if (tag_size != 4)
        goto error;
      self->track_number = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  track number = %u", self->track_number);
      break;
    case 0x4802:
      self->track_name = mxf_utf16_to_utf8 (tag_data, tag_size);
      GST_DEBUG ("  track name = %s", GST_STR_NULL (self->track_name));
      break;
    case 0x4803:
      if (tag_size != 16)
        goto error;
      memcpy (&self->sequence_uid, tag_data, 16);
      GST_DEBUG ("  sequence uid = %s",
          mxf_ul_to_string (&self->sequence_uid, str));
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS (mxf_metadata_track_parent_class)->handle_tag
          (metadata, primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid track local tag 0x%04x of size %u", tag, tag_size);

  return FALSE;
}

static gboolean
mxf_metadata_track_resolve (MXFMetadataBase * m, GHashTable * metadata)
{
  MXFMetadataTrack *self = MXF_METADATA_TRACK (m);
  MXFMetadataBase *current = NULL;
  guint i;

  current = g_hash_table_lookup (metadata, &self->sequence_uid);
  if (current && MXF_IS_METADATA_SEQUENCE (current)) {
    if (mxf_metadata_base_resolve (current, metadata)) {
      self->sequence = MXF_METADATA_SEQUENCE (current);
    } else {
      GST_ERROR ("Couldn't resolve sequence");
      return FALSE;
    }
  } else {
    GST_ERROR ("Couldn't find sequence");
    return FALSE;
  }

  self->type =
      mxf_metadata_track_identifier_parse (&self->sequence->data_definition);
  if (self->type == MXF_METADATA_TRACK_UNKNOWN) {
    MXFMetadataSequence *sequence = self->sequence;

    for (i = 0; i < sequence->n_structural_components; i++) {
      MXFMetadataStructuralComponent *component =
          sequence->structural_components[i];

      if (!component)
        continue;

      self->type =
          mxf_metadata_track_identifier_parse (&component->data_definition);
      if (self->type != MXF_METADATA_TRACK_UNKNOWN)
        break;
    }
  }

  return MXF_METADATA_BASE_CLASS (mxf_metadata_track_parent_class)->resolve (m,
      metadata);
}

static GstStructure *
mxf_metadata_track_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS (mxf_metadata_track_parent_class)->to_structure
      (m);
  MXFMetadataTrack *self = MXF_METADATA_TRACK (m);

  gst_structure_id_set (ret, MXF_QUARK (TRACK_ID), G_TYPE_UINT, self->track_id,
      MXF_QUARK (TRACK_NUMBER), G_TYPE_UINT, self->track_number, NULL);

  if (self->track_name)
    gst_structure_id_set (ret, MXF_QUARK (TRACK_NAME), G_TYPE_STRING,
        self->track_name, NULL);

  if (self->sequence) {
    GstStructure *s =
        mxf_metadata_base_to_structure (MXF_METADATA_BASE (self->sequence));

    gst_structure_id_set (ret, MXF_QUARK (SEQUENCE), GST_TYPE_STRUCTURE, s,
        NULL);
    gst_structure_free (s);
  }


  return ret;
}

static void
mxf_metadata_track_init (MXFMetadataTrack * self)
{

}

static void
mxf_metadata_track_class_init (MXFMetadataTrackClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;

  miniobject_class->finalize = mxf_metadata_track_finalize;
  metadata_base_class->handle_tag = mxf_metadata_track_handle_tag;
  metadata_base_class->resolve = mxf_metadata_track_resolve;
  metadata_base_class->to_structure = mxf_metadata_track_to_structure;
}

/* SMPTE RP224 */
static const struct
{
  guint8 ul[16];
  MXFMetadataTrackType type;
} mxf_metadata_track_identifier[] = {
  { {
  0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01,
          0x01, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00},
      MXF_METADATA_TRACK_TIMECODE_12M_INACTIVE}, { {
  0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x01, 0x03, 0x02, 0x01,
          0x02, 0x00, 0x00, 0x00}, MXF_METADATA_TRACK_TIMECODE_12M_ACTIVE}, { {
  0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x01, 0x03, 0x02, 0x01,
          0x03, 0x00, 0x00, 0x00}, MXF_METADATA_TRACK_TIMECODE_309M}, { {
  0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x01, 0x03, 0x02, 0x01,
          0x10, 0x00, 0x00, 0x00}, MXF_METADATA_TRACK_METADATA}, { {
  0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x01, 0x03, 0x02, 0x02,
          0x01, 0x00, 0x00, 0x00}, MXF_METADATA_TRACK_PICTURE_ESSENCE}, { {
  0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x01, 0x03, 0x02, 0x02,
          0x02, 0x00, 0x00, 0x00}, MXF_METADATA_TRACK_SOUND_ESSENCE}, { {
  0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x01, 0x03, 0x02, 0x02,
          0x03, 0x00, 0x00, 0x00}, MXF_METADATA_TRACK_DATA_ESSENCE}, { {
  0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x01, 0x03, 0x02, 0x03,
          0x01, 0x00, 0x00, 0x00}, MXF_METADATA_TRACK_AUXILIARY_DATA}, { {
  0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x01, 0x03, 0x02, 0x03,
          0x02, 0x00, 0x00, 0x00}, MXF_METADATA_TRACK_PARSED_TEXT},
      /* Avid video? */
  { {
  0x80, 0x7d, 0x00, 0x60, 0x08, 0x14, 0x3e, 0x6f, 0x6f, 0x3c, 0x8c, 0xe1,
          0x6c, 0xef, 0x11, 0xd2}, MXF_METADATA_TRACK_PICTURE_ESSENCE}
};

MXFMetadataTrackType
mxf_metadata_track_identifier_parse (const MXFUL * track_identifier)
{
  guint i;

  for (i = 0; i < G_N_ELEMENTS (mxf_metadata_track_identifier); i++)
    if (memcmp (&mxf_metadata_track_identifier[i].ul, &track_identifier->u,
            16) == 0)
      return mxf_metadata_track_identifier[i].type;

  return MXF_METADATA_TRACK_UNKNOWN;
}

G_DEFINE_TYPE (MXFMetadataTimelineTrack, mxf_metadata_timeline_track,
    MXF_TYPE_METADATA_TRACK);

static gboolean
mxf_metadata_timeline_track_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataTimelineTrack *self = MXF_METADATA_TIMELINE_TRACK (metadata);
  gboolean ret = TRUE;

  switch (tag) {
    case 0x4b01:
      if (!mxf_fraction_parse (&self->edit_rate, tag_data, tag_size))
        goto error;
      GST_DEBUG ("  edit rate = %d/%d", self->edit_rate.n, self->edit_rate.d);
      break;
    case 0x4b02:
      if (tag_size != 8)
        goto error;
      self->origin = GST_READ_UINT64_BE (tag_data);
      GST_DEBUG ("  origin = %" G_GINT64_FORMAT, self->origin);
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_timeline_track_parent_class)->handle_tag (metadata,
          primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid timeline track local tag 0x%04x of size %u", tag,
      tag_size);

  return FALSE;
}

static GstStructure *
mxf_metadata_timeline_track_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_timeline_track_parent_class)->to_structure (m);
  MXFMetadataTimelineTrack *self = MXF_METADATA_TIMELINE_TRACK (m);

  gst_structure_id_set (ret, MXF_QUARK (EDIT_RATE), GST_TYPE_FRACTION,
      self->edit_rate.n, self->edit_rate.d, MXF_QUARK (ORIGIN), G_TYPE_INT64,
      self->origin, NULL);

  return ret;
}

static void
mxf_metadata_timeline_track_init (MXFMetadataTimelineTrack * self)
{

}

static void
mxf_metadata_timeline_track_class_init (MXFMetadataTimelineTrackClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  metadata_base_class->handle_tag = mxf_metadata_timeline_track_handle_tag;
  metadata_base_class->name_quark = MXF_QUARK (TIMELINE_TRACK);
  metadata_base_class->to_structure = mxf_metadata_timeline_track_to_structure;
  metadata_class->type = 0x013b;
}

G_DEFINE_TYPE (MXFMetadataEventTrack, mxf_metadata_event_track,
    MXF_TYPE_METADATA_TRACK);

static gboolean
mxf_metadata_event_track_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataEventTrack *self = MXF_METADATA_EVENT_TRACK (metadata);
  gboolean ret = TRUE;

  switch (tag) {
    case 0x4901:
      if (!mxf_fraction_parse (&self->event_edit_rate, tag_data, tag_size))
        goto error;
      GST_DEBUG ("  event edit rate = %d/%d", self->event_edit_rate.n,
          self->event_edit_rate.d);
      break;
    case 0x4902:
      if (tag_size != 8)
        goto error;
      self->event_origin = GST_READ_UINT64_BE (tag_data);
      GST_DEBUG ("  event origin = %" G_GINT64_FORMAT, self->event_origin);
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_event_track_parent_class)->handle_tag (metadata, primer,
          tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid event track local tag 0x%04x of size %u", tag, tag_size);

  return FALSE;
}

static GstStructure *
mxf_metadata_event_track_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_event_track_parent_class)->to_structure (m);
  MXFMetadataEventTrack *self = MXF_METADATA_EVENT_TRACK (m);

  gst_structure_id_set (ret, MXF_QUARK (EVENT_EDIT_RATE), GST_TYPE_FRACTION,
      self->event_edit_rate.n, self->event_edit_rate.d,
      MXF_QUARK (EVENT_ORIGIN), G_TYPE_INT64, self->event_origin, NULL);

  return ret;
}

static void
mxf_metadata_event_track_init (MXFMetadataEventTrack * self)
{

}

static void
mxf_metadata_event_track_class_init (MXFMetadataEventTrackClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  metadata_base_class->handle_tag = mxf_metadata_event_track_handle_tag;
  metadata_base_class->name_quark = MXF_QUARK (EVENT_TRACK);
  metadata_base_class->to_structure = mxf_metadata_event_track_to_structure;
  metadata_class->type = 0x0139;
}

G_DEFINE_TYPE (MXFMetadataStaticTrack, mxf_metadata_static_track,
    MXF_TYPE_METADATA_TRACK);

static void
mxf_metadata_static_track_init (MXFMetadataStaticTrack * self)
{
}

static void
mxf_metadata_static_track_class_init (MXFMetadataStaticTrackClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  metadata_base_class->name_quark = MXF_QUARK (STATIC_TRACK);
  metadata_class->type = 0x013a;
}

G_DEFINE_TYPE (MXFMetadataSequence, mxf_metadata_sequence, MXF_TYPE_METADATA);

static void
mxf_metadata_sequence_finalize (GstMiniObject * object)
{
  MXFMetadataSequence *self = MXF_METADATA_SEQUENCE (object);

  g_free (self->structural_components_uids);
  self->structural_components_uids = NULL;
  g_free (self->structural_components);
  self->structural_components = NULL;

  GST_MINI_OBJECT_CLASS (mxf_metadata_sequence_parent_class)->finalize (object);
}

static gboolean
mxf_metadata_sequence_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataSequence *self = MXF_METADATA_SEQUENCE (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif

  switch (tag) {
    case 0x0201:
      if (tag_size != 16)
        goto error;
      memcpy (&self->data_definition, tag_data, 16);
      GST_DEBUG ("  data definition = %s",
          mxf_ul_to_string (&self->data_definition, str));
      break;
    case 0x0202:
      if (tag_size != 8)
        goto error;
      self->duration = GST_READ_UINT64_BE (tag_data);
      GST_DEBUG ("  duration = %" G_GINT64_FORMAT, self->duration);
      break;
    case 0x1001:
      if (!mxf_ul_array_parse (&self->structural_components_uids,
              &self->n_structural_components, tag_data, tag_size))
        goto error;

      GST_DEBUG ("  number of structural components = %u",
          self->n_structural_components);
#ifndef GST_DISABLE_GST_DEBUG
      {
        guint i;
        for (i = 0; i < self->n_structural_components; i++) {
          GST_DEBUG ("  structural component %u = %s", i,
              mxf_ul_to_string (&self->structural_components_uids[i], str));
        }
      }
#endif
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_sequence_parent_class)->handle_tag (metadata, primer,
          tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid sequence local tag 0x%04x of size %u", tag, tag_size);

  return FALSE;
}

static gboolean
mxf_metadata_sequence_resolve (MXFMetadataBase * m, GHashTable * metadata)
{
  MXFMetadataSequence *self = MXF_METADATA_SEQUENCE (m);
  MXFMetadataBase *current = NULL;
  guint i;

  if (self->structural_components)
    memset (self->structural_components, 0,
        sizeof (gpointer) * self->n_structural_components);
  else
    self->structural_components =
        g_new0 (MXFMetadataStructuralComponent *,
        self->n_structural_components);
  for (i = 0; i < self->n_structural_components; i++) {
    current =
        g_hash_table_lookup (metadata, &self->structural_components_uids[i]);
    if (current && MXF_IS_METADATA_STRUCTURAL_COMPONENT (current)) {
      if (mxf_metadata_base_resolve (current, metadata)) {
        self->structural_components[i] =
            MXF_METADATA_STRUCTURAL_COMPONENT (current);
      } else {
        GST_ERROR ("Couldn't resolve structural component");
        return FALSE;
      }
    } else {
      GST_ERROR ("Structural component not found");
      return FALSE;
    }
  }

  return
      MXF_METADATA_BASE_CLASS (mxf_metadata_sequence_parent_class)->resolve (m,
      metadata);

}

static GstStructure *
mxf_metadata_sequence_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS (mxf_metadata_sequence_parent_class)->to_structure
      (m);
  MXFMetadataSequence *self = MXF_METADATA_SEQUENCE (m);
  guint i;
  gchar str[48];

  mxf_ul_to_string (&self->data_definition, str);
  gst_structure_id_set (ret, MXF_QUARK (DATA_DEFINITION), G_TYPE_STRING, str,
      MXF_QUARK (DURATION), G_TYPE_INT64, self->duration, NULL);

  if (self->n_structural_components > 0) {
    GValue arr = { 0, }
    , val = {
    0,};

    g_value_init (&arr, GST_TYPE_ARRAY);

    for (i = 0; i < self->n_structural_components; i++) {
      GstStructure *s;

      if (self->structural_components[i] == NULL)
        continue;

      g_value_init (&val, GST_TYPE_STRUCTURE);

      s = mxf_metadata_base_to_structure (MXF_METADATA_BASE
          (self->structural_components[i]));
      gst_value_set_structure (&val, s);
      gst_structure_free (s);
      gst_value_array_append_value (&arr, &val);
      g_value_unset (&val);
    }

    if (gst_value_array_get_size (&arr) > 0)
      gst_structure_id_set_value (ret, MXF_QUARK (STRUCTURAL_COMPONENTS), &arr);

    g_value_unset (&arr);
  }

  return ret;
}

static void
mxf_metadata_sequence_init (MXFMetadataSequence * self)
{
  self->duration = -1;
}

static void
mxf_metadata_sequence_class_init (MXFMetadataSequenceClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  miniobject_class->finalize = mxf_metadata_sequence_finalize;
  metadata_base_class->handle_tag = mxf_metadata_sequence_handle_tag;
  metadata_base_class->resolve = mxf_metadata_sequence_resolve;
  metadata_base_class->name_quark = MXF_QUARK (SEQUENCE);
  metadata_base_class->to_structure = mxf_metadata_sequence_to_structure;
  metadata_class->type = 0x010f;
}

G_DEFINE_TYPE (MXFMetadataStructuralComponent,
    mxf_metadata_structural_component, MXF_TYPE_METADATA);

static gboolean
mxf_metadata_structural_component_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataStructuralComponent *self =
      MXF_METADATA_STRUCTURAL_COMPONENT (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif

  switch (tag) {
    case 0x0201:
      if (tag_size != 16)
        goto error;
      memcpy (&self->data_definition, tag_data, 16);
      GST_DEBUG ("  data definition = %s",
          mxf_ul_to_string (&self->data_definition, str));
      break;
    case 0x0202:
      if (tag_size != 8)
        goto error;
      self->duration = GST_READ_UINT64_BE (tag_data);
      GST_DEBUG ("  duration = %" G_GINT64_FORMAT, self->duration);
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_structural_component_parent_class)->handle_tag
          (metadata, primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid structural component local tag 0x%04x of size %u", tag,
      tag_size);

  return FALSE;
}

static GstStructure *
mxf_metadata_structural_component_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_structural_component_parent_class)->to_structure (m);
  MXFMetadataStructuralComponent *self = MXF_METADATA_STRUCTURAL_COMPONENT (m);
  gchar str[48];

  mxf_ul_to_string (&self->data_definition, str);
  gst_structure_id_set (ret, MXF_QUARK (DATA_DEFINITION), G_TYPE_STRING, str,
      MXF_QUARK (DURATION), G_TYPE_INT64, self->duration, NULL);

  return ret;
}

static void
mxf_metadata_structural_component_init (MXFMetadataStructuralComponent * self)
{
  self->duration = -1;
}

static void
    mxf_metadata_structural_component_class_init
    (MXFMetadataStructuralComponentClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;

  metadata_base_class->handle_tag =
      mxf_metadata_structural_component_handle_tag;
  metadata_base_class->to_structure =
      mxf_metadata_structural_component_to_structure;
}

G_DEFINE_TYPE (MXFMetadataTimecodeComponent, mxf_metadata_timecode_component,
    MXF_TYPE_METADATA_STRUCTURAL_COMPONENT);

static gboolean
mxf_metadata_timecode_component_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataTimecodeComponent *self =
      MXF_METADATA_TIMECODE_COMPONENT (metadata);
  gboolean ret = TRUE;

  switch (tag) {
    case 0x1502:
      if (tag_size != 2)
        goto error;
      self->rounded_timecode_base = GST_READ_UINT16_BE (tag_data);
      GST_DEBUG ("  rounded timecode base = %u", self->rounded_timecode_base);
      break;
    case 0x1501:
      if (tag_size != 8)
        goto error;
      self->start_timecode = GST_READ_UINT64_BE (tag_data);
      GST_DEBUG ("  start timecode = %" G_GINT64_FORMAT, self->start_timecode);
      break;
    case 0x1503:
      if (tag_size != 1)
        goto error;
      self->drop_frame = (GST_READ_UINT8 (tag_data) != 0);
      GST_DEBUG ("  drop frame = %s", (self->drop_frame) ? "yes" : "no");
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_timecode_component_parent_class)->handle_tag (metadata,
          primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid timecode component local tag 0x%04x of size %u", tag,
      tag_size);

  return FALSE;
}

static GstStructure *
mxf_metadata_timecode_component_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_timecode_component_parent_class)->to_structure (m);
  MXFMetadataTimecodeComponent *self = MXF_METADATA_TIMECODE_COMPONENT (m);

  gst_structure_id_set (ret, MXF_QUARK (START_TIMECODE), G_TYPE_INT64,
      self->start_timecode, MXF_QUARK (ROUNDED_TIMECODE_BASE), G_TYPE_UINT,
      self->rounded_timecode_base, MXF_QUARK (DROP_FRAME), G_TYPE_BOOLEAN,
      self->drop_frame, NULL);

  return ret;
}

static void
mxf_metadata_timecode_component_init (MXFMetadataTimecodeComponent * self)
{

}

static void
mxf_metadata_timecode_component_class_init (MXFMetadataTimecodeComponentClass *
    klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  metadata_base_class->handle_tag = mxf_metadata_timecode_component_handle_tag;
  metadata_base_class->name_quark = MXF_QUARK (TIMECODE_COMPONENT);
  metadata_base_class->to_structure =
      mxf_metadata_timecode_component_to_structure;
  metadata_class->type = 0x0114;
}

G_DEFINE_TYPE (MXFMetadataSourceClip, mxf_metadata_source_clip,
    MXF_TYPE_METADATA_STRUCTURAL_COMPONENT);

static gboolean
mxf_metadata_source_clip_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataSourceClip *self = MXF_METADATA_SOURCE_CLIP (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[96];
#endif

  switch (tag) {
    case 0x1201:
      if (tag_size != 8)
        goto error;

      self->start_position = GST_READ_UINT64_BE (tag_data);
      GST_DEBUG ("  start position = %" G_GINT64_FORMAT, self->start_position);
      break;
    case 0x1101:
      if (tag_size != 32)
        goto error;

      memcpy (&self->source_package_id, tag_data, 32);
      GST_DEBUG ("  source package id = %s",
          mxf_umid_to_string (&self->source_package_id, str));
      break;
    case 0x1102:
      if (tag_size != 4)
        goto error;

      self->source_track_id = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  source track id = %u", self->source_track_id);
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_source_clip_parent_class)->handle_tag (metadata, primer,
          tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid source clip local tag 0x%04x of size %u", tag, tag_size);

  return FALSE;
}

static gboolean
mxf_metadata_source_clip_resolve (MXFMetadataBase * m, GHashTable * metadata)
{
  MXFMetadataSourceClip *self = MXF_METADATA_SOURCE_CLIP (m);
  MXFMetadataBase *current = NULL;
#if GLIB_CHECK_VERSION (2, 16, 0)
  GHashTableIter iter;

  g_hash_table_iter_init (&iter, metadata);
#else
  GList *l, *values = NULL;

  g_hash_table_foreach (metadata, build_values_in_hash_table, &values);
#endif

#if GLIB_CHECK_VERSION (2, 16, 0)
  while (g_hash_table_iter_next (&iter, NULL, (gpointer) & current)) {
#else
  for (l = values; l; l = l->next) {
    current = l->data;
#endif
    if (MXF_IS_METADATA_SOURCE_PACKAGE (current)) {
      MXFMetadataGenericPackage *p = MXF_METADATA_GENERIC_PACKAGE (current);

      if (mxf_umid_is_equal (&p->package_uid, &self->source_package_id)) {
        self->source_package = MXF_METADATA_SOURCE_PACKAGE (current);
        break;
      }
    }
  }

#if !GLIB_CHECK_VERSION (2, 16, 0)
  g_list_free (values);
#endif

  return
      MXF_METADATA_BASE_CLASS (mxf_metadata_source_clip_parent_class)->resolve
      (m, metadata);
}

static GstStructure *
mxf_metadata_source_clip_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_source_clip_parent_class)->to_structure (m);
  MXFMetadataSourceClip *self = MXF_METADATA_SOURCE_CLIP (m);
  gchar str[96];

  mxf_umid_to_string (&self->source_package_id, str);
  gst_structure_id_set (ret, MXF_QUARK (START_POSITION), G_TYPE_INT64,
      self->start_position, MXF_QUARK (SOURCE_PACKAGE), G_TYPE_STRING, str,
      MXF_QUARK (SOURCE_TRACK_ID), G_TYPE_UINT, self->source_track_id, NULL);

  return ret;
}


static void
mxf_metadata_source_clip_init (MXFMetadataSourceClip * self)
{

}

static void
mxf_metadata_source_clip_class_init (MXFMetadataSourceClipClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  metadata_base_class->handle_tag = mxf_metadata_source_clip_handle_tag;
  metadata_base_class->resolve = mxf_metadata_source_clip_resolve;
  metadata_base_class->name_quark = MXF_QUARK (SOURCE_CLIP);
  metadata_base_class->to_structure = mxf_metadata_source_clip_to_structure;
  metadata_class->type = 0x0111;
}

G_DEFINE_TYPE (MXFMetadataDMSourceClip, mxf_metadata_dm_source_clip,
    MXF_TYPE_METADATA_SOURCE_CLIP);

static void
mxf_metadata_dm_source_clip_finalize (GstMiniObject * object)
{
  MXFMetadataDMSourceClip *self = MXF_METADATA_DM_SOURCE_CLIP (object);

  g_free (self->track_ids);
  self->track_ids = NULL;

  GST_MINI_OBJECT_CLASS (mxf_metadata_dm_source_clip_parent_class)->finalize
      (object);
}

static gboolean
mxf_metadata_dm_source_clip_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataDMSourceClip *self = MXF_METADATA_DM_SOURCE_CLIP (metadata);
  gboolean ret = TRUE;

  switch (tag) {
    case 0x6103:
    {
      guint32 len;
      guint i;

      if (tag_size < 8)
        goto error;

      len = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  number of track ids = %u", len);
      if (len == 0)
        return TRUE;

      if (GST_READ_UINT32_BE (tag_data + 4) != 4)
        goto error;

      if (tag_size < 8 + 4 * len)
        goto error;

      tag_data += 8;
      tag_size -= 8;

      self->n_track_ids = len;
      self->track_ids = g_new0 (guint32, len);

      for (i = 0; i < len; i++) {
        self->track_ids[i] = GST_READ_UINT32_BE (tag_data);
        GST_DEBUG ("    track id %u = %u", i, self->track_ids[i]);
        tag_data += 4;
        tag_size -= 4;
      }
      break;
    }
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_dm_source_clip_parent_class)->handle_tag (metadata,
          primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid DM source clip local tag 0x%04x of size %u", tag,
      tag_size);

  return FALSE;
}

static GstStructure *
mxf_metadata_dm_source_clip_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_dm_source_clip_parent_class)->to_structure (m);
  MXFMetadataDMSourceClip *self = MXF_METADATA_DM_SOURCE_CLIP (m);
  guint i;

  if (self->n_track_ids > 0) {
    GValue arr = { 0, }
    , val = {
    0,};

    g_value_init (&arr, GST_TYPE_ARRAY);

    for (i = 0; i < self->n_track_ids; i++) {
      g_value_init (&val, G_TYPE_UINT);

      g_value_set_uint (&val, self->track_ids[i]);
      gst_value_array_append_value (&arr, &val);
      g_value_unset (&val);
    }

    if (gst_value_array_get_size (&arr) > 0)
      gst_structure_id_set_value (ret, MXF_QUARK (TRACK_IDS), &arr);

    g_value_unset (&arr);
  }

  return ret;
}

static void
mxf_metadata_dm_source_clip_init (MXFMetadataDMSourceClip * self)
{

}

static void
mxf_metadata_dm_source_clip_class_init (MXFMetadataDMSourceClipClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  miniobject_class->finalize = mxf_metadata_dm_source_clip_finalize;
  metadata_base_class->handle_tag = mxf_metadata_dm_source_clip_handle_tag;
  metadata_base_class->name_quark = MXF_QUARK (DM_SOURCE_CLIP);
  metadata_base_class->to_structure = mxf_metadata_dm_source_clip_to_structure;
  metadata_class->type = 0x0145;
}

G_DEFINE_TYPE (MXFMetadataDMSegment, mxf_metadata_dm_segment,
    MXF_TYPE_METADATA_STRUCTURAL_COMPONENT);

static void
mxf_metadata_dm_segment_finalize (GstMiniObject * object)
{
  MXFMetadataDMSegment *self = MXF_METADATA_DM_SEGMENT (object);

  g_free (self->track_ids);
  self->track_ids = NULL;

  g_free (self->event_comment);
  self->event_comment = NULL;

  GST_MINI_OBJECT_CLASS (mxf_metadata_dm_segment_parent_class)->finalize
      (object);
}

static gboolean
mxf_metadata_dm_segment_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataDMSegment *self = MXF_METADATA_DM_SEGMENT (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif

  switch (tag) {
    case 0x0601:
      if (tag_size != 8)
        goto error;
      self->event_start_position = GST_READ_UINT64_BE (tag_data);
      GST_DEBUG ("  event start position = %" G_GINT64_FORMAT,
          self->event_start_position);
      break;
    case 0x0602:
      self->event_comment = mxf_utf16_to_utf8 (tag_data, tag_size);
      GST_DEBUG ("  event comment = %s", GST_STR_NULL (self->event_comment));
      break;
    case 0x6102:
    {
      guint32 len;
      guint i;

      if (tag_size < 8)
        goto error;
      len = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  number of track ids = %u", len);
      if (len == 0)
        return TRUE;

      if (GST_READ_UINT32_BE (tag_data + 4) != 4)
        goto error;

      if (len * 4 + 8 < tag_size)
        goto error;

      self->n_track_ids = len;
      self->track_ids = g_new0 (guint32, len);

      tag_data += 8;
      tag_size -= 8;

      for (i = 0; i < len; i++) {
        self->track_ids[i] = GST_READ_UINT32_BE (tag_data);
        GST_DEBUG ("    track id %u = %u", i, self->track_ids[i]);
        tag_data += 4;
        tag_size -= 4;
      }
      break;
    }
    case 0x6101:
      if (tag_size != 16)
        goto error;

      memcpy (&self->dm_framework_uid, tag_data, 16);
      GST_DEBUG ("  DM framework = %s",
          mxf_ul_to_string (&self->dm_framework_uid, str));
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_dm_segment_parent_class)->handle_tag (metadata, primer,
          tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid DM segment local tag 0x%04x of size %u", tag, tag_size);

  return FALSE;
}

static gboolean
mxf_metadata_dm_segment_resolve (MXFMetadataBase * m, GHashTable * metadata)
{
  MXFMetadataDMSegment *self = MXF_METADATA_DM_SEGMENT (m);
  MXFMetadataBase *current = NULL;

  current = g_hash_table_lookup (metadata, &self->dm_framework_uid);
  if (current && MXF_IS_DESCRIPTIVE_METADATA_FRAMEWORK (current)) {
    if (mxf_metadata_base_resolve (current, metadata)) {
      self->dm_framework = MXF_DESCRIPTIVE_METADATA_FRAMEWORK (current);
    } else {
      GST_ERROR ("Couldn't resolve DM framework");
      return FALSE;
    }
  } else {
    GST_ERROR ("Couldn't find DM framework");
    return FALSE;
  }


  return
      MXF_METADATA_BASE_CLASS (mxf_metadata_dm_segment_parent_class)->resolve
      (m, metadata);
}

static GstStructure *
mxf_metadata_dm_segment_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_dm_segment_parent_class)->to_structure (m);
  MXFMetadataDMSegment *self = MXF_METADATA_DM_SEGMENT (m);
  guint i;

  gst_structure_id_set (ret, MXF_QUARK (EVENT_START_POSITION), G_TYPE_INT64,
      self->event_start_position, NULL);

  if (self->event_comment)
    gst_structure_id_set (ret, MXF_QUARK (EVENT_COMMENT), G_TYPE_STRING,
        self->event_comment, NULL);
  /* FIXME: DMS1 doesn't support serializing to a structure yet */
#if 0
  if (self->dm_framework) {
    GstStructure *s =
        mxf_metadata_base_to_structure (MXF_METADATA_BASE (self->dm_framework));

    gst_structure_id_set (ret, MXF_QUARK (DM_FRAMEWORK), GST_TYPE_STRUCTURE,
        s, NULL);
    gst_structure_free (s);
  }
#endif

  if (self->n_track_ids > 0) {
    GValue arr = { 0, }
    , val = {
    0,};

    g_value_init (&arr, GST_TYPE_ARRAY);

    for (i = 0; i < self->n_track_ids; i++) {
      g_value_init (&val, G_TYPE_UINT);

      g_value_set_uint (&val, self->track_ids[i]);
      gst_value_array_append_value (&arr, &val);
      g_value_unset (&val);
    }

    if (gst_value_array_get_size (&arr) > 0)
      gst_structure_id_set_value (ret, MXF_QUARK (TRACK_IDS), &arr);

    g_value_unset (&arr);
  }

  return ret;
}

static void
mxf_metadata_dm_segment_init (MXFMetadataDMSegment * self)
{

}

static void
mxf_metadata_dm_segment_class_init (MXFMetadataDMSegmentClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  miniobject_class->finalize = mxf_metadata_dm_segment_finalize;
  metadata_base_class->handle_tag = mxf_metadata_dm_segment_handle_tag;
  metadata_base_class->resolve = mxf_metadata_dm_segment_resolve;
  metadata_base_class->name_quark = MXF_QUARK (DM_SEGMENT);
  metadata_base_class->to_structure = mxf_metadata_dm_segment_to_structure;
  metadata_class->type = 0x0141;
}

G_DEFINE_ABSTRACT_TYPE (MXFMetadataGenericDescriptor,
    mxf_metadata_generic_descriptor, MXF_TYPE_METADATA);

static void
mxf_metadata_generic_descriptor_finalize (GstMiniObject * object)
{
  MXFMetadataGenericDescriptor *self = MXF_METADATA_GENERIC_DESCRIPTOR (object);

  g_free (self->locators_uids);
  self->locators_uids = NULL;

  g_free (self->locators);
  self->locators = NULL;

  GST_MINI_OBJECT_CLASS (mxf_metadata_generic_descriptor_parent_class)->finalize
      (object);
}

static gboolean
mxf_metadata_generic_descriptor_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataGenericDescriptor *self =
      MXF_METADATA_GENERIC_DESCRIPTOR (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif

  switch (tag) {
    case 0x2f01:
      if (!mxf_ul_array_parse (&self->locators_uids, &self->n_locators,
              tag_data, tag_size))
        goto error;

      GST_DEBUG ("  number of locators = %u", self->n_locators);
#ifndef GST_DISABLE_GST_DEBUG
      {
        guint i;
        for (i = 0; i < self->n_locators; i++) {
          GST_DEBUG ("  locator %u = %s", i,
              mxf_ul_to_string (&self->locators_uids[i], str));
        }
      }
#endif
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_generic_descriptor_parent_class)->handle_tag (metadata,
          primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid generic descriptor local tag 0x%04x of size %u", tag,
      tag_size);

  return FALSE;
}

static gboolean
mxf_metadata_generic_descriptor_resolve (MXFMetadataBase * m,
    GHashTable * metadata)
{
  MXFMetadataGenericDescriptor *self = MXF_METADATA_GENERIC_DESCRIPTOR (m);
  MXFMetadataBase *current = NULL;
  guint i;
  gboolean have_locator = FALSE;

  if (self->locators)
    memset (self->locators, 0, sizeof (gpointer) * self->n_locators);
  else
    self->locators = g_new0 (MXFMetadataLocator *, self->n_locators);
  for (i = 0; i < self->n_locators; i++) {
    current = g_hash_table_lookup (metadata, &self->locators_uids[i]);
    if (current && MXF_IS_METADATA_LOCATOR (current)) {
      if (mxf_metadata_base_resolve (current, metadata)) {
        self->locators[i] = MXF_METADATA_LOCATOR (current);
        have_locator = TRUE;
      } else {
        GST_ERROR ("Couldn't resolve locator");
      }
    } else {
      GST_ERROR ("Locator not found");
    }
  }

  if (!have_locator && self->n_locators > 0) {
    GST_ERROR ("Couldn't resolve a locator");
    return FALSE;
  }

  return
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_generic_descriptor_parent_class)->resolve (m, metadata);
}

static GstStructure *
mxf_metadata_generic_descriptor_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_generic_descriptor_parent_class)->to_structure (m);
  MXFMetadataGenericDescriptor *self = MXF_METADATA_GENERIC_DESCRIPTOR (m);
  guint i;

  if (self->n_locators > 0) {
    GValue arr = { 0, }
    , val = {
    0,};

    g_value_init (&arr, GST_TYPE_ARRAY);

    for (i = 0; i < self->n_locators; i++) {
      GstStructure *s;

      if (self->locators[i] == NULL)
        continue;

      g_value_init (&val, GST_TYPE_STRUCTURE);

      s = mxf_metadata_base_to_structure (MXF_METADATA_BASE (self->locators
              [i]));
      gst_value_set_structure (&val, s);
      gst_structure_free (s);
      gst_value_array_append_value (&arr, &val);
      g_value_unset (&val);
    }

    if (gst_value_array_get_size (&arr) > 0)
      gst_structure_id_set_value (ret, MXF_QUARK (LOCATORS), &arr);

    g_value_unset (&arr);
  }

  return ret;
}

static void
mxf_metadata_generic_descriptor_init (MXFMetadataGenericDescriptor * self)
{

}

static void
mxf_metadata_generic_descriptor_class_init (MXFMetadataGenericDescriptorClass *
    klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;

  miniobject_class->finalize = mxf_metadata_generic_descriptor_finalize;
  metadata_base_class->handle_tag = mxf_metadata_generic_descriptor_handle_tag;
  metadata_base_class->resolve = mxf_metadata_generic_descriptor_resolve;
  metadata_base_class->to_structure =
      mxf_metadata_generic_descriptor_to_structure;
}

G_DEFINE_TYPE (MXFMetadataFileDescriptor, mxf_metadata_file_descriptor,
    MXF_TYPE_METADATA_GENERIC_DESCRIPTOR);

static gboolean
mxf_metadata_file_descriptor_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataFileDescriptor *self = MXF_METADATA_FILE_DESCRIPTOR (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif

  switch (tag) {
    case 0x3006:
      if (tag_size != 4)
        goto error;
      self->linked_track_id = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  linked track id = %u", self->linked_track_id);
      break;
    case 0x3001:
      if (!mxf_fraction_parse (&self->sample_rate, tag_data, tag_size))
        goto error;
      GST_DEBUG ("  sample rate = %d/%d", self->sample_rate.n,
          self->sample_rate.d);
      break;
    case 0x3002:
      if (tag_size != 8)
        goto error;
      self->container_duration = GST_READ_UINT64_BE (tag_data);
      GST_DEBUG ("  container duration = %" G_GINT64_FORMAT,
          self->container_duration);
      break;
    case 0x3004:
      if (tag_size != 16)
        goto error;
      memcpy (&self->essence_container, tag_data, 16);
      GST_DEBUG ("  essence container = %s",
          mxf_ul_to_string (&self->essence_container, str));
      break;
    case 0x3005:
      if (tag_size != 16)
        goto error;
      memcpy (&self->codec, tag_data, 16);
      GST_DEBUG ("  codec = %s", mxf_ul_to_string (&self->codec, str));
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_file_descriptor_parent_class)->handle_tag (metadata,
          primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid file descriptor local tag 0x%04x of size %u", tag,
      tag_size);

  return FALSE;
}

static GstStructure *
mxf_metadata_file_descriptor_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_file_descriptor_parent_class)->to_structure (m);
  MXFMetadataFileDescriptor *self = MXF_METADATA_FILE_DESCRIPTOR (m);
  gchar str[48];

  if (self->linked_track_id)
    gst_structure_id_set (ret, MXF_QUARK (LINKED_TRACK_ID), G_TYPE_UINT,
        self->linked_track_id, NULL);

  if (self->sample_rate.n && self->sample_rate.d)
    gst_structure_id_set (ret, MXF_QUARK (SAMPLE_RATE), GST_TYPE_FRACTION,
        self->sample_rate.n, self->sample_rate.d, NULL);

  if (self->container_duration)
    gst_structure_id_set (ret, MXF_QUARK (CONTAINER_DURATION), G_TYPE_INT64,
        self->container_duration, NULL);

  mxf_ul_to_string (&self->essence_container, str);
  gst_structure_id_set (ret, MXF_QUARK (ESSENCE_CONTAINER), G_TYPE_STRING, str,
      NULL);

  if (!mxf_ul_is_zero (&self->codec)) {
    mxf_ul_to_string (&self->codec, str);
    gst_structure_id_set (ret, MXF_QUARK (CODEC), G_TYPE_STRING, str, NULL);
  }

  return ret;
}

static void
mxf_metadata_file_descriptor_init (MXFMetadataFileDescriptor * self)
{

}

static void
mxf_metadata_file_descriptor_class_init (MXFMetadataFileDescriptorClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  metadata_base_class->handle_tag = mxf_metadata_file_descriptor_handle_tag;
  metadata_base_class->name_quark = MXF_QUARK (FILE_DESCRIPTOR);
  metadata_base_class->to_structure = mxf_metadata_file_descriptor_to_structure;
  metadata_class->type = 0x0125;
}

G_DEFINE_TYPE (MXFMetadataGenericPictureEssenceDescriptor,
    mxf_metadata_generic_picture_essence_descriptor,
    MXF_TYPE_METADATA_FILE_DESCRIPTOR);

static gboolean
mxf_metadata_generic_picture_essence_descriptor_handle_tag (MXFMetadataBase *
    metadata, MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataGenericPictureEssenceDescriptor *self =
      MXF_METADATA_GENERIC_PICTURE_ESSENCE_DESCRIPTOR (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif

  switch (tag) {
    case 0x3215:
      if (tag_size != 1)
        goto error;
      self->signal_standard = GST_READ_UINT8 (tag_data);
      GST_DEBUG ("  signal standard = %u", self->signal_standard);
      break;
    case 0x320c:
      if (tag_size != 1)
        goto error;
      self->frame_layout = GST_READ_UINT8 (tag_data);
      GST_DEBUG ("  frame layout = %u", self->frame_layout);
      break;
    case 0x3203:
      if (tag_size != 4)
        goto error;
      self->stored_width = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  stored width = %u", self->stored_width);
      break;
    case 0x3202:
      if (tag_size != 4)
        goto error;
      self->stored_height = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  stored height = %u", self->stored_height);
      break;
    case 0x3216:
      if (tag_size != 4)
        goto error;
      self->stored_f2_offset = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  stored f2 offset = %d", self->stored_f2_offset);
      break;
    case 0x3205:
      if (tag_size != 4)
        goto error;
      self->sampled_width = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  sampled width = %u", self->sampled_width);
      break;
    case 0x3204:
      if (tag_size != 4)
        goto error;
      self->sampled_height = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  sampled height = %u", self->sampled_height);
      break;
    case 0x3206:
      if (tag_size != 4)
        goto error;
      self->sampled_x_offset = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  sampled x offset = %d", self->sampled_x_offset);
      break;
    case 0x3207:
      if (tag_size != 4)
        goto error;
      self->sampled_y_offset = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  sampled y offset = %d", self->sampled_y_offset);
      break;
    case 0x3208:
      if (tag_size != 4)
        goto error;
      self->display_height = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  display height = %u", self->display_height);
      break;
    case 0x3209:
      if (tag_size != 4)
        goto error;
      self->display_width = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  display width = %u", self->display_width);
      break;
    case 0x320a:
      if (tag_size != 4)
        goto error;
      self->display_x_offset = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  display x offset = %d", self->display_x_offset);
      break;
    case 0x320b:
      if (tag_size != 4)
        goto error;
      self->display_y_offset = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  display y offset = %d", self->display_y_offset);
      break;
    case 0x3217:
      if (tag_size != 4)
        goto error;
      self->display_f2_offset = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  display f2 offset = %d", self->display_f2_offset);
      break;
    case 0x320e:
      if (!mxf_fraction_parse (&self->aspect_ratio, tag_data, tag_size))
        goto error;
      GST_DEBUG ("  aspect ratio = %d/%d", self->aspect_ratio.n,
          self->aspect_ratio.d);
      break;
    case 0x3218:
      if (tag_size != 1)
        goto error;
      self->active_format_descriptor = GST_READ_UINT8 (tag_data);
      GST_DEBUG ("  active format descriptor = %u",
          self->active_format_descriptor);
      break;
    case 0x320d:
      if (tag_size < 8)
        goto error;

      if (GST_READ_UINT32_BE (tag_data) == 0)
        return TRUE;

      if (GST_READ_UINT32_BE (tag_data) != 2 &&
          GST_READ_UINT32_BE (tag_data + 4) != 4)
        goto error;

      if (tag_size != 16)
        goto error;

      self->video_line_map[0] = GST_READ_UINT32_BE (tag_data + 8);
      self->video_line_map[1] = GST_READ_UINT32_BE (tag_data + 12);
      GST_DEBUG ("  video line map = {%i, %i}", self->video_line_map[0],
          self->video_line_map[1]);
      break;
    case 0x320f:
      if (tag_size != 1)
        goto error;
      self->alpha_transparency = GST_READ_UINT8 (tag_data);
      GST_DEBUG ("  alpha transparency = %u", self->alpha_transparency);
      break;
    case 0x3210:
      if (tag_size != 16)
        goto error;
      memcpy (&self->capture_gamma, tag_data, 16);
      GST_DEBUG ("  capture gamma = %s",
          mxf_ul_to_string (&self->capture_gamma, str));
      break;
    case 0x3211:
      if (tag_size != 4)
        goto error;
      self->image_alignment_offset = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  image alignment offset = %u", self->image_alignment_offset);
      break;
    case 0x3213:
      if (tag_size != 4)
        goto error;
      self->image_start_offset = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  image start offset = %u", self->image_start_offset);
      break;
    case 0x3214:
      if (tag_size != 4)
        goto error;
      self->image_end_offset = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  image end offset = %u", self->image_end_offset);
      break;
    case 0x3212:
      if (tag_size != 1)
        goto error;
      self->field_dominance = GST_READ_UINT8 (tag_data);
      GST_DEBUG ("  field dominance = %u", self->field_dominance);
      break;
    case 0x3201:
      if (tag_size != 16)
        goto error;
      memcpy (&self->picture_essence_coding, tag_data, 16);
      GST_DEBUG ("  picture essence coding = %s",
          mxf_ul_to_string (&self->picture_essence_coding, str));
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_generic_picture_essence_descriptor_parent_class)->handle_tag
          (metadata, primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR
      ("Invalid generic picture essence descriptor local tag 0x%04x of size %u",
      tag, tag_size);

  return FALSE;
}

static GstStructure *
mxf_metadata_generic_picture_essence_descriptor_to_structure (MXFMetadataBase *
    m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_generic_picture_essence_descriptor_parent_class)->to_structure
      (m);
  MXFMetadataGenericPictureEssenceDescriptor *self =
      MXF_METADATA_GENERIC_PICTURE_ESSENCE_DESCRIPTOR (m);
  gchar str[48];

  gst_structure_id_set (ret, MXF_QUARK (SIGNAL_STANDARD), G_TYPE_UCHAR,
      self->signal_standard, NULL);

  if (self->frame_layout != 255)
    gst_structure_id_set (ret, MXF_QUARK (FRAME_LAYOUT), G_TYPE_UCHAR,
        self->frame_layout, NULL);

  if (self->stored_width != 0 && self->stored_height != 0)
    gst_structure_id_set (ret, MXF_QUARK (STORED_WIDTH), G_TYPE_UINT,
        self->stored_width, MXF_QUARK (STORED_HEIGHT), G_TYPE_UINT,
        self->stored_height, NULL);

  if (self->stored_f2_offset != 0)
    gst_structure_id_set (ret, MXF_QUARK (STORED_F2_OFFSET), G_TYPE_INT,
        self->stored_f2_offset, NULL);

  if (self->sampled_width != 0 && self->sampled_height != 0)
    gst_structure_id_set (ret, MXF_QUARK (SAMPLED_WIDTH), G_TYPE_UINT,
        self->sampled_width, MXF_QUARK (SAMPLED_HEIGHT), G_TYPE_UINT,
        self->sampled_height, NULL);

  if (self->sampled_x_offset != 0)
    gst_structure_id_set (ret, MXF_QUARK (SAMPLED_X_OFFSET), G_TYPE_INT,
        self->sampled_x_offset, NULL);

  if (self->sampled_y_offset != 0)
    gst_structure_id_set (ret, MXF_QUARK (SAMPLED_Y_OFFSET), G_TYPE_INT,
        self->sampled_y_offset, NULL);

  if (self->display_width != 0 && self->display_height != 0)
    gst_structure_id_set (ret, MXF_QUARK (DISPLAY_WIDTH), G_TYPE_UINT,
        self->display_width, MXF_QUARK (DISPLAY_HEIGHT), G_TYPE_UINT,
        self->display_height, NULL);

  if (self->display_x_offset != 0)
    gst_structure_id_set (ret, MXF_QUARK (DISPLAY_X_OFFSET), G_TYPE_INT,
        self->display_x_offset, NULL);

  if (self->display_y_offset != 0)
    gst_structure_id_set (ret, MXF_QUARK (DISPLAY_Y_OFFSET), G_TYPE_INT,
        self->display_y_offset, NULL);

  if (self->display_f2_offset != 0)
    gst_structure_id_set (ret, MXF_QUARK (DISPLAY_F2_OFFSET), G_TYPE_INT,
        self->display_f2_offset, NULL);

  if (self->aspect_ratio.n != 0 && self->aspect_ratio.d != 0)
    gst_structure_id_set (ret, MXF_QUARK (ASPECT_RATIO), GST_TYPE_FRACTION,
        self->aspect_ratio.n, self->aspect_ratio.d, NULL);

  if (self->active_format_descriptor)
    gst_structure_id_set (ret, MXF_QUARK (ACTIVE_FORMAT_DESCRIPTOR),
        G_TYPE_UCHAR, self->active_format_descriptor, NULL);

  if (self->video_line_map[0] != 0 && self->video_line_map[1] != 0)
    gst_structure_id_set (ret, MXF_QUARK (VIDEO_LINE_MAP_0), G_TYPE_UINT,
        self->video_line_map[0], MXF_QUARK (VIDEO_LINE_MAP_1), G_TYPE_UINT,
        self->video_line_map[1], NULL);

  if (self->alpha_transparency != 0)
    gst_structure_id_set (ret, MXF_QUARK (ALPHA_TRANSPARENCY), G_TYPE_UCHAR,
        self->alpha_transparency, NULL);

  if (!mxf_ul_is_zero (&self->capture_gamma)) {
    mxf_ul_to_string (&self->capture_gamma, str);
    gst_structure_id_set (ret, MXF_QUARK (CAPTURE_GAMMA), G_TYPE_STRING, str,
        NULL);
  }

  if (self->image_alignment_offset != 0)
    gst_structure_id_set (ret, MXF_QUARK (IMAGE_ALIGNMENT_OFFSET), G_TYPE_UINT,
        self->image_alignment_offset, NULL);

  if (self->image_start_offset != 0)
    gst_structure_id_set (ret, MXF_QUARK (IMAGE_START_OFFSET), G_TYPE_UINT,
        self->image_start_offset, NULL);

  if (self->image_end_offset != 0)
    gst_structure_id_set (ret, MXF_QUARK (IMAGE_END_OFFSET), G_TYPE_UINT,
        self->image_end_offset, NULL);

  if (self->field_dominance != 0)
    gst_structure_id_set (ret, MXF_QUARK (FIELD_DOMINANCE), G_TYPE_UCHAR,
        self->field_dominance, NULL);

  if (!mxf_ul_is_zero (&self->picture_essence_coding)) {
    mxf_ul_to_string (&self->picture_essence_coding, str);
    gst_structure_id_set (ret, MXF_QUARK (PICTURE_ESSENCE_CODING),
        G_TYPE_STRING, str, NULL);
  }

  return ret;
}

static void
    mxf_metadata_generic_picture_essence_descriptor_init
    (MXFMetadataGenericPictureEssenceDescriptor * self)
{
  self->signal_standard = 1;
  self->frame_layout = 255;
}

static void
    mxf_metadata_generic_picture_essence_descriptor_class_init
    (MXFMetadataGenericPictureEssenceDescriptorClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  metadata_base_class->handle_tag =
      mxf_metadata_generic_picture_essence_descriptor_handle_tag;
  metadata_base_class->name_quark =
      MXF_QUARK (GENERIC_PICTURE_ESSENCE_DESCRIPTOR);
  metadata_base_class->to_structure =
      mxf_metadata_generic_picture_essence_descriptor_to_structure;
  metadata_class->type = 0x0127;
}

void mxf_metadata_generic_picture_essence_descriptor_set_caps
    (MXFMetadataGenericPictureEssenceDescriptor * self, GstCaps * caps)
{
  guint par_n, par_d;
  guint width, height;
  MXFMetadataFileDescriptor *f = (MXFMetadataFileDescriptor *) self;

  g_return_if_fail (MXF_IS_METADATA_GENERIC_PICTURE_ESSENCE_DESCRIPTOR (self));
  g_return_if_fail (GST_IS_CAPS (caps));

  if (f->sample_rate.d == 0) {
    GST_ERROR ("Invalid framerate");
  } else {
    gst_caps_set_simple (caps, "framerate", GST_TYPE_FRACTION, f->sample_rate.n,
        f->sample_rate.d, NULL);
  }

  width = self->stored_width;
  height = self->stored_height;

  /* If the video is stored as separate fields the
   * height is only the height of one field, i.e.
   * half the height of the frame.
   *
   * See SMPTE 377M E2.2 and E1.2
   */
  if (self->frame_layout == 1 || self->frame_layout == 2
      || self->frame_layout == 4)
    height *= 2;

  if (width == 0 || height == 0) {
    GST_ERROR ("Invalid width/height");
    return;
  }

  gst_caps_set_simple (caps, "width", G_TYPE_INT, width, "height", G_TYPE_INT,
      height, NULL);

  if (self->aspect_ratio.n == 0 || self->aspect_ratio.d == 0) {
    GST_ERROR ("Invalid aspect ratio");
    return;
  }

  par_n = height * self->aspect_ratio.n;
  par_d = width * self->aspect_ratio.d;

  gst_caps_set_simple (caps, "pixel-aspect-ratio", GST_TYPE_FRACTION,
      par_n, par_d, NULL);
}

G_DEFINE_TYPE (MXFMetadataGenericSoundEssenceDescriptor,
    mxf_metadata_generic_sound_essence_descriptor,
    MXF_TYPE_METADATA_FILE_DESCRIPTOR);

static gboolean
mxf_metadata_generic_sound_essence_descriptor_handle_tag (MXFMetadataBase *
    metadata, MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataGenericSoundEssenceDescriptor *self =
      MXF_METADATA_GENERIC_SOUND_ESSENCE_DESCRIPTOR (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif

  switch (tag) {
    case 0x3d03:
      if (!mxf_fraction_parse (&self->audio_sampling_rate, tag_data, tag_size))
        goto error;
      GST_DEBUG ("  audio sampling rate = %d/%d",
          self->audio_sampling_rate.n, self->audio_sampling_rate.d);
      break;
    case 0x3d02:
      if (tag_size != 1)
        goto error;
      self->locked = (GST_READ_UINT8 (tag_data) != 0);
      GST_DEBUG ("  locked = %s", (self->locked) ? "yes" : "no");
      break;
    case 0x3d04:
      if (tag_size != 1)
        goto error;
      self->audio_ref_level = GST_READ_UINT8 (tag_data);
      GST_DEBUG ("  audio ref level = %d", self->audio_ref_level);
      break;
    case 0x3d05:
      if (tag_size != 1)
        goto error;
      self->electro_spatial_formulation = GST_READ_UINT8 (tag_data);
      GST_DEBUG ("  electro spatial formulation = %u",
          self->electro_spatial_formulation);
      break;
    case 0x3d07:
      if (tag_size != 4)
        goto error;
      self->channel_count = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  channel count = %u", self->channel_count);
      break;
    case 0x3d01:
      if (tag_size != 4)
        goto error;
      self->quantization_bits = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  quantization bits = %u", self->quantization_bits);
      break;
    case 0x3d0c:
      if (tag_size != 1)
        goto error;
      self->dial_norm = GST_READ_UINT8 (tag_data);
      GST_DEBUG ("  dial norm = %d", self->dial_norm);
      break;
    case 0x3d06:
      if (tag_size != 16)
        goto error;
      memcpy (&self->sound_essence_compression, tag_data, 16);
      GST_DEBUG ("  sound essence compression = %s",
          mxf_ul_to_string (&self->sound_essence_compression, str));
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_generic_sound_essence_descriptor_parent_class)->handle_tag
          (metadata, primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR
      ("Invalid generic sound essence descriptor local tag 0x%04x of size %u",
      tag, tag_size);

  return FALSE;
}

static GstStructure *
mxf_metadata_generic_sound_essence_descriptor_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_generic_sound_essence_descriptor_parent_class)->to_structure
      (m);
  MXFMetadataGenericSoundEssenceDescriptor *self =
      MXF_METADATA_GENERIC_SOUND_ESSENCE_DESCRIPTOR (m);

  gst_structure_id_set (ret, MXF_QUARK (AUDIO_SAMPLING_RATE), GST_TYPE_FRACTION,
      self->audio_sampling_rate.n, self->audio_sampling_rate.d, NULL);

  gst_structure_id_set (ret, MXF_QUARK (LOCKED), G_TYPE_BOOLEAN, self->locked,
      NULL);

  if (self->electro_spatial_formulation != 0)
    gst_structure_id_set (ret, MXF_QUARK (ELECTRO_SPATIAL_FORMULATION),
        G_TYPE_UCHAR, self->electro_spatial_formulation, NULL);

  if (self->channel_count != 0)
    gst_structure_id_set (ret, MXF_QUARK (CHANNEL_COUNT), G_TYPE_UINT,
        self->channel_count, NULL);

  if (self->quantization_bits != 0)
    gst_structure_id_set (ret, MXF_QUARK (QUANTIZATION_BITS), G_TYPE_UINT,
        self->quantization_bits, NULL);

  if (self->dial_norm != 0)
    gst_structure_id_set (ret, MXF_QUARK (DIAL_NORM), G_TYPE_CHAR,
        self->dial_norm, NULL);

  if (!mxf_ul_is_zero (&self->sound_essence_compression)) {
    gchar str[48];

    mxf_ul_to_string (&self->sound_essence_compression, str);
    gst_structure_id_set (ret, MXF_QUARK (SOUND_ESSENCE_COMPRESSION),
        G_TYPE_STRING, str, NULL);
  }

  return ret;
}

static void
    mxf_metadata_generic_sound_essence_descriptor_init
    (MXFMetadataGenericSoundEssenceDescriptor * self)
{
  self->audio_sampling_rate.n = 48000;
  self->audio_sampling_rate.d = 1;
}

static void
    mxf_metadata_generic_sound_essence_descriptor_class_init
    (MXFMetadataGenericSoundEssenceDescriptorClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  metadata_base_class->handle_tag =
      mxf_metadata_generic_sound_essence_descriptor_handle_tag;
  metadata_base_class->name_quark =
      MXF_QUARK (GENERIC_SOUND_ESSENCE_DESCRIPTOR);
  metadata_base_class->to_structure =
      mxf_metadata_generic_sound_essence_descriptor_to_structure;
  metadata_class->type = 0x0142;
}

void mxf_metadata_generic_sound_essence_descriptor_set_caps
    (MXFMetadataGenericSoundEssenceDescriptor * self, GstCaps * caps)
{
  g_return_if_fail (MXF_IS_METADATA_GENERIC_SOUND_ESSENCE_DESCRIPTOR (self));
  g_return_if_fail (GST_IS_CAPS (caps));

  if (self->audio_sampling_rate.n == 0 || self->audio_sampling_rate.d == 0) {
    GST_ERROR ("Invalid audio sampling rate");
  } else {
    gst_caps_set_simple (caps,
        "rate", G_TYPE_INT,
        (gint) (mxf_fraction_to_double (&self->audio_sampling_rate)
            + 0.5), NULL);
  }

  if (self->channel_count == 0) {
    GST_ERROR ("Invalid number of channels (0)");
  } else {
    gst_caps_set_simple (caps, "channels", G_TYPE_INT, self->channel_count,
        NULL);
  }
}

G_DEFINE_TYPE (MXFMetadataCDCIPictureEssenceDescriptor,
    mxf_metadata_cdci_picture_essence_descriptor,
    MXF_TYPE_METADATA_GENERIC_PICTURE_ESSENCE_DESCRIPTOR);

static gboolean
mxf_metadata_cdci_picture_essence_descriptor_handle_tag (MXFMetadataBase *
    metadata, MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataCDCIPictureEssenceDescriptor *self =
      MXF_METADATA_CDCI_PICTURE_ESSENCE_DESCRIPTOR (metadata);
  gboolean ret = TRUE;

  switch (tag) {
    case 0x3301:
      if (tag_size != 4)
        goto error;
      self->component_depth = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  component depth = %u", self->component_depth);
      break;
    case 0x3302:
      if (tag_size != 4)
        goto error;
      self->horizontal_subsampling = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  horizontal subsampling = %u", self->horizontal_subsampling);
      break;
    case 0x3308:
      if (tag_size != 4)
        goto error;
      self->vertical_subsampling = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  vertical subsampling = %u", self->vertical_subsampling);
      break;
    case 0x3303:
      if (tag_size != 1)
        goto error;
      self->color_siting = GST_READ_UINT8 (tag_data);
      GST_DEBUG ("  color siting = %u", self->color_siting);
      break;
    case 0x330b:
      if (tag_size != 1)
        goto error;
      self->reversed_byte_order = GST_READ_UINT8 (tag_data);
      GST_DEBUG ("  reversed byte order = %s",
          (self->reversed_byte_order) ? "yes" : "no");
      break;
    case 0x3307:
      if (tag_size != 2)
        goto error;
      self->padding_bits = GST_READ_UINT16_BE (tag_data);
      GST_DEBUG ("  padding bits = %d", self->padding_bits);
      break;
    case 0x3309:
      if (tag_size != 4)
        goto error;
      self->alpha_sample_depth = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  alpha sample depth = %u", self->alpha_sample_depth);
      break;
    case 0x3304:
      if (tag_size != 4)
        goto error;
      self->black_ref_level = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  black ref level = %u", self->black_ref_level);
      break;
    case 0x3305:
      if (tag_size != 4)
        goto error;
      self->white_ref_level = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  white ref level = %u", self->white_ref_level);
      break;
    case 0x3306:
      if (tag_size != 4)
        goto error;
      self->color_range = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  color range = %u", self->color_range);
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_cdci_picture_essence_descriptor_parent_class)->handle_tag
          (metadata, primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR
      ("Invalid CDCI picture essence descriptor local tag 0x%04x of size %u",
      tag, tag_size);

  return FALSE;
}

static GstStructure *
mxf_metadata_cdci_picture_essence_descriptor_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_cdci_picture_essence_descriptor_parent_class)->to_structure
      (m);
  MXFMetadataCDCIPictureEssenceDescriptor *self =
      MXF_METADATA_CDCI_PICTURE_ESSENCE_DESCRIPTOR (m);

  if (self->component_depth != 0)
    gst_structure_id_set (ret, MXF_QUARK (COMPONENT_DEPTH), G_TYPE_UINT,
        self->component_depth, NULL);

  if (self->horizontal_subsampling != 0)
    gst_structure_id_set (ret, MXF_QUARK (HORIZONTAL_SUBSAMPLING), G_TYPE_UINT,
        self->horizontal_subsampling, NULL);

  if (self->vertical_subsampling != 0)
    gst_structure_id_set (ret, MXF_QUARK (VERTICAL_SUBSAMPLING), G_TYPE_UINT,
        self->vertical_subsampling, NULL);

  if (self->color_siting != 0)
    gst_structure_id_set (ret, MXF_QUARK (COLOR_SITING), G_TYPE_UCHAR,
        self->color_siting, NULL);

  gst_structure_id_set (ret, MXF_QUARK (REVERSED_BYTE_ORDER), G_TYPE_BOOLEAN,
      self->reversed_byte_order, NULL);

  if (self->padding_bits != 0)
    gst_structure_id_set (ret, MXF_QUARK (PADDING_BITS), G_TYPE_INT,
        self->padding_bits, NULL);

  if (self->alpha_sample_depth != 0)
    gst_structure_id_set (ret, MXF_QUARK (ALPHA_SAMPLE_DEPTH), G_TYPE_UINT,
        self->alpha_sample_depth, NULL);

  if (self->black_ref_level != 0)
    gst_structure_id_set (ret, MXF_QUARK (BLACK_REF_LEVEL), G_TYPE_UINT,
        self->black_ref_level, NULL);

  if (self->white_ref_level != 0)
    gst_structure_id_set (ret, MXF_QUARK (WHITE_REF_LEVEL), G_TYPE_UINT,
        self->white_ref_level, NULL);

  if (self->color_range != 0)
    gst_structure_id_set (ret, MXF_QUARK (COLOR_RANGE), G_TYPE_UINT,
        self->color_range, NULL);

  return ret;
}

static void
    mxf_metadata_cdci_picture_essence_descriptor_init
    (MXFMetadataCDCIPictureEssenceDescriptor * self)
{

}

static void
    mxf_metadata_cdci_picture_essence_descriptor_class_init
    (MXFMetadataCDCIPictureEssenceDescriptorClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  metadata_base_class->handle_tag =
      mxf_metadata_cdci_picture_essence_descriptor_handle_tag;
  metadata_base_class->name_quark = MXF_QUARK (CDCI_PICTURE_ESSENCE_DESCRIPTOR);
  metadata_base_class->to_structure =
      mxf_metadata_cdci_picture_essence_descriptor_to_structure;
  metadata_class->type = 0x0128;
}

G_DEFINE_TYPE (MXFMetadataRGBAPictureEssenceDescriptor,
    mxf_metadata_rgba_picture_essence_descriptor,
    MXF_TYPE_METADATA_GENERIC_PICTURE_ESSENCE_DESCRIPTOR);

static void
mxf_metadata_rgba_picture_essence_descriptor_finalize (GstMiniObject * object)
{
  MXFMetadataRGBAPictureEssenceDescriptor *self =
      MXF_METADATA_RGBA_PICTURE_ESSENCE_DESCRIPTOR (object);

  g_free (self->pixel_layout);
  self->pixel_layout = NULL;

  GST_MINI_OBJECT_CLASS
      (mxf_metadata_rgba_picture_essence_descriptor_parent_class)->finalize
      (object);
}

static gboolean
mxf_metadata_rgba_picture_essence_descriptor_handle_tag (MXFMetadataBase *
    metadata, MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataRGBAPictureEssenceDescriptor *self =
      MXF_METADATA_RGBA_PICTURE_ESSENCE_DESCRIPTOR (metadata);
  gboolean ret = TRUE;

  switch (tag) {
    case 0x3406:
      if (tag_size != 4)
        goto error;
      self->component_max_ref = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  component max ref = %u", self->component_max_ref);
      break;
    case 0x3407:
      if (tag_size != 4)
        goto error;
      self->component_min_ref = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  component min ref = %u", self->component_min_ref);
      break;
    case 0x3408:
      if (tag_size != 4)
        goto error;
      self->alpha_max_ref = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  alpha max ref = %u", self->alpha_max_ref);
      break;
    case 0x3409:
      if (tag_size != 4)
        goto error;
      self->alpha_min_ref = GST_READ_UINT32_BE (tag_data);
      GST_DEBUG ("  alpha min ref = %u", self->alpha_min_ref);
      break;
    case 0x3405:
      if (tag_size != 1)
        goto error;
      self->scanning_direction = GST_READ_UINT8 (tag_data);
      GST_DEBUG ("  scanning direction = %u", self->scanning_direction);
      break;
    case 0x3401:{
      guint i, len;

      if (tag_size % 2 != 0)
        goto error;

      i = 0;
      while (tag_data[i] != 0 && tag_data[i + 1] != 0 && i + 2 <= tag_size)
        i += 2;
      len = i / 2;

      self->n_pixel_layout = len;
      GST_DEBUG ("  number of pixel layouts = %u", len);
      if (len == 0)
        return TRUE;

      self->pixel_layout = g_malloc0 (2 * len);

      for (i = 0; i < len; i++) {
        self->pixel_layout[2 * i] = tag_data[2 * i];
        self->pixel_layout[2 * i + 1] = tag_data[2 * i + 1];
        GST_DEBUG ("    pixel layout %u = %c : %u", i,
            (gchar) self->pixel_layout[2 * i], self->pixel_layout[2 * i + 1]);
      }

      break;
    }
    case 0x3403:
    case 0x3404:
      /* TODO: handle this */
      GST_WARNING ("  tag 0x%04x not implemented yet", tag);
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_rgba_picture_essence_descriptor_parent_class)->handle_tag
          (metadata, primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR
      ("Invalid RGBA picture essence descriptor local tag 0x%04x of size %u",
      tag, tag_size);

  return FALSE;
}

static GstStructure *
mxf_metadata_rgba_picture_essence_descriptor_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_rgba_picture_essence_descriptor_parent_class)->to_structure
      (m);
  MXFMetadataRGBAPictureEssenceDescriptor *self =
      MXF_METADATA_RGBA_PICTURE_ESSENCE_DESCRIPTOR (m);

  if (self->component_max_ref != 255)
    gst_structure_id_set (ret, MXF_QUARK (COMPONENT_MAX_REF), G_TYPE_UINT,
        self->component_max_ref, NULL);

  if (self->component_min_ref != 0)
    gst_structure_id_set (ret, MXF_QUARK (COMPONENT_MIN_REF), G_TYPE_UINT,
        self->component_min_ref, NULL);

  if (self->alpha_max_ref != 255)
    gst_structure_id_set (ret, MXF_QUARK (ALPHA_MAX_REF), G_TYPE_UINT,
        self->alpha_max_ref, NULL);

  if (self->alpha_min_ref != 0)
    gst_structure_id_set (ret, MXF_QUARK (ALPHA_MIN_REF), G_TYPE_UINT,
        self->alpha_min_ref, NULL);

  if (self->scanning_direction != 0)
    gst_structure_id_set (ret, MXF_QUARK (SCANNING_DIRECTION), G_TYPE_UCHAR,
        self->scanning_direction, NULL);

  if (self->n_pixel_layout != 0) {
    gchar *pl = g_new0 (gchar, self->n_pixel_layout * 2 + 1);

    memcpy (pl, self->pixel_layout, self->n_pixel_layout * 2);

    gst_structure_id_set (ret, MXF_QUARK (PIXEL_LAYOUT), G_TYPE_STRING, pl,
        NULL);

    g_free (pl);
  }

  return ret;
}

static void
    mxf_metadata_rgba_picture_essence_descriptor_init
    (MXFMetadataRGBAPictureEssenceDescriptor * self)
{
  self->component_max_ref = 255;
  self->alpha_max_ref = 255;
}

static void
    mxf_metadata_rgba_picture_essence_descriptor_class_init
    (MXFMetadataRGBAPictureEssenceDescriptorClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  miniobject_class->finalize =
      mxf_metadata_rgba_picture_essence_descriptor_finalize;
  metadata_base_class->handle_tag =
      mxf_metadata_rgba_picture_essence_descriptor_handle_tag;
  metadata_base_class->name_quark = MXF_QUARK (RGBA_PICTURE_ESSENCE_DESCRIPTOR);
  metadata_base_class->to_structure =
      mxf_metadata_rgba_picture_essence_descriptor_to_structure;
  metadata_class->type = 0x0129;
}

G_DEFINE_TYPE (MXFMetadataGenericDataEssenceDescriptor,
    mxf_metadata_generic_data_essence_descriptor,
    MXF_TYPE_METADATA_FILE_DESCRIPTOR);

static gboolean
mxf_metadata_generic_data_essence_descriptor_handle_tag (MXFMetadataBase *
    metadata, MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataGenericDataEssenceDescriptor *self =
      MXF_METADATA_GENERIC_DATA_ESSENCE_DESCRIPTOR (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif

  switch (tag) {
    case 0x3e01:
      if (tag_size != 16)
        goto error;
      memcpy (&self->data_essence_coding, tag_data, 16);
      GST_DEBUG ("  data essence coding = %s",
          mxf_ul_to_string (&self->data_essence_coding, str));
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_generic_data_essence_descriptor_parent_class)->handle_tag
          (metadata, primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR
      ("Invalid generic data essence descriptor local tag 0x%04x of size %u",
      tag, tag_size);

  return FALSE;
}

static GstStructure *
mxf_metadata_generic_data_essence_descriptor_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_generic_data_essence_descriptor_parent_class)->to_structure
      (m);
  MXFMetadataGenericDataEssenceDescriptor *self =
      MXF_METADATA_GENERIC_DATA_ESSENCE_DESCRIPTOR (m);
  gchar str[48];

  if (!mxf_ul_is_zero (&self->data_essence_coding)) {
    mxf_ul_to_string (&self->data_essence_coding, str);
    gst_structure_id_set (ret, MXF_QUARK (DATA_ESSENCE_CODING), G_TYPE_STRING,
        str, NULL);
  }

  return ret;
}

static void
    mxf_metadata_generic_data_essence_descriptor_init
    (MXFMetadataGenericDataEssenceDescriptor * self)
{

}

static void
    mxf_metadata_generic_data_essence_descriptor_class_init
    (MXFMetadataGenericDataEssenceDescriptorClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  metadata_base_class->handle_tag =
      mxf_metadata_generic_data_essence_descriptor_handle_tag;
  metadata_base_class->name_quark = MXF_QUARK (GENERIC_DATA_ESSENCE_DESCRIPTOR);
  metadata_base_class->to_structure =
      mxf_metadata_generic_data_essence_descriptor_to_structure;
  metadata_class->type = 0x0143;
}

G_DEFINE_TYPE (MXFMetadataMultipleDescriptor, mxf_metadata_multiple_descriptor,
    MXF_TYPE_METADATA_FILE_DESCRIPTOR);

static void
mxf_metadata_multiple_descriptor_finalize (GstMiniObject * object)
{
  MXFMetadataMultipleDescriptor *self =
      MXF_METADATA_MULTIPLE_DESCRIPTOR (object);

  g_free (self->sub_descriptors_uids);
  self->sub_descriptors_uids = NULL;
  g_free (self->sub_descriptors);
  self->sub_descriptors = NULL;

  GST_MINI_OBJECT_CLASS
      (mxf_metadata_multiple_descriptor_parent_class)->finalize (object);
}

static gboolean
mxf_metadata_multiple_descriptor_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataMultipleDescriptor *self =
      MXF_METADATA_MULTIPLE_DESCRIPTOR (metadata);
  gboolean ret = TRUE;
#ifndef GST_DISABLE_GST_DEBUG
  gchar str[48];
#endif

  switch (tag) {
    case 0x3f01:
      if (!mxf_ul_array_parse (&self->sub_descriptors_uids,
              &self->n_sub_descriptors, tag_data, tag_size))
        goto error;

      GST_DEBUG ("  number of sub descriptors = %u", self->n_sub_descriptors);
#ifndef GST_DISABLE_GST_DEBUG
      {
        guint i;
        for (i = 0; i < self->n_sub_descriptors; i++) {
          GST_DEBUG ("    sub descriptor %u = %s", i,
              mxf_ul_to_string (&self->sub_descriptors_uids[i], str));
        }
      }
#endif

      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_multiple_descriptor_parent_class)->handle_tag (metadata,
          primer, tag, tag_data, tag_size);
      break;
  }

  return ret;

error:

  GST_ERROR ("Invalid multiple descriptor local tag 0x%04x of size %u", tag,
      tag_size);

  return FALSE;
}

static gboolean
mxf_metadata_multiple_descriptor_resolve (MXFMetadataBase * m,
    GHashTable * metadata)
{
  MXFMetadataMultipleDescriptor *self = MXF_METADATA_MULTIPLE_DESCRIPTOR (m);
  MXFMetadataBase *current = NULL;
  guint i, have_subdescriptors = 0;

  if (self->sub_descriptors)
    memset (self->sub_descriptors, 0,
        sizeof (gpointer) * self->n_sub_descriptors);
  else
    self->sub_descriptors =
        g_new0 (MXFMetadataGenericDescriptor *, self->n_sub_descriptors);
  for (i = 0; i < self->n_sub_descriptors; i++) {
    current = g_hash_table_lookup (metadata, &self->sub_descriptors_uids[i]);
    if (current && MXF_IS_METADATA_GENERIC_DESCRIPTOR (current)) {
      if (mxf_metadata_base_resolve (current, metadata)) {
        self->sub_descriptors[i] = MXF_METADATA_GENERIC_DESCRIPTOR (current);
        have_subdescriptors++;
      } else {
        GST_ERROR ("Couldn't resolve descriptor");
        return FALSE;
      }
    } else {
      GST_ERROR ("Descriptor not found");
      return FALSE;
    }
  }

  return
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_multiple_descriptor_parent_class)->resolve (m, metadata);
}

static GstStructure *
mxf_metadata_multiple_descriptor_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_multiple_descriptor_parent_class)->to_structure (m);
  MXFMetadataMultipleDescriptor *self = MXF_METADATA_MULTIPLE_DESCRIPTOR (m);
  guint i;

  if (self->n_sub_descriptors > 0) {
    GValue arr = { 0, }
    , val = {
    0,};

    g_value_init (&arr, GST_TYPE_ARRAY);

    for (i = 0; i < self->n_sub_descriptors; i++) {
      GstStructure *s;

      if (self->sub_descriptors[i] == NULL)
        continue;

      g_value_init (&val, GST_TYPE_STRUCTURE);

      s = mxf_metadata_base_to_structure (MXF_METADATA_BASE
          (self->sub_descriptors[i]));
      gst_value_set_structure (&val, s);
      gst_structure_free (s);
      gst_value_array_append_value (&arr, &val);
      g_value_unset (&val);
    }

    if (gst_value_array_get_size (&arr) > 0)
      gst_structure_id_set_value (ret, MXF_QUARK (SUB_DESCRIPTORS), &arr);

    g_value_unset (&arr);
  }

  return ret;
}

static void
mxf_metadata_multiple_descriptor_init (MXFMetadataMultipleDescriptor * self)
{

}

static void
mxf_metadata_multiple_descriptor_class_init (MXFMetadataMultipleDescriptorClass
    * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  miniobject_class->finalize = mxf_metadata_multiple_descriptor_finalize;
  metadata_base_class->handle_tag = mxf_metadata_multiple_descriptor_handle_tag;
  metadata_base_class->resolve = mxf_metadata_multiple_descriptor_resolve;
  metadata_base_class->name_quark = MXF_QUARK (MULTIPLE_DESCRIPTOR);
  metadata_base_class->to_structure =
      mxf_metadata_multiple_descriptor_to_structure;
  metadata_class->type = 0x0144;
}

G_DEFINE_ABSTRACT_TYPE (MXFMetadataLocator, mxf_metadata_locator,
    MXF_TYPE_METADATA);

static void
mxf_metadata_locator_init (MXFMetadataLocator * self)
{
}

static void
mxf_metadata_locator_class_init (MXFMetadataLocatorClass * klass)
{
}

G_DEFINE_TYPE (MXFMetadataTextLocator, mxf_metadata_text_locator,
    MXF_TYPE_METADATA_LOCATOR);

static void
mxf_metadata_text_locator_finalize (GstMiniObject * object)
{
  MXFMetadataTextLocator *self = MXF_METADATA_TEXT_LOCATOR (object);

  g_free (self->locator_name);
  self->locator_name = NULL;

  GST_MINI_OBJECT_CLASS (mxf_metadata_text_locator_parent_class)->finalize
      (object);
}

static gboolean
mxf_metadata_text_locator_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataTextLocator *self = MXF_METADATA_TEXT_LOCATOR (metadata);
  gboolean ret = TRUE;

  switch (tag) {
    case 0x4101:
      self->locator_name = mxf_utf16_to_utf8 (tag_data, tag_size);
      GST_DEBUG ("  text locator = %s", GST_STR_NULL (self->locator_name));
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_text_locator_parent_class)->handle_tag (metadata,
          primer, tag, tag_data, tag_size);
      break;
  }

  return ret;
}

static GstStructure *
mxf_metadata_text_locator_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_text_locator_parent_class)->to_structure (m);
  MXFMetadataTextLocator *self = MXF_METADATA_TEXT_LOCATOR (m);

  gst_structure_id_set (ret, MXF_QUARK (LOCATOR_NAME), G_TYPE_STRING,
      self->locator_name, NULL);

  return ret;
}

static void
mxf_metadata_text_locator_init (MXFMetadataTextLocator * self)
{

}

static void
mxf_metadata_text_locator_class_init (MXFMetadataTextLocatorClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  miniobject_class->finalize = mxf_metadata_text_locator_finalize;
  metadata_base_class->handle_tag = mxf_metadata_text_locator_handle_tag;
  metadata_base_class->name_quark = MXF_QUARK (TEXT_LOCATOR);
  metadata_base_class->to_structure = mxf_metadata_text_locator_to_structure;
  metadata_class->type = 0x0133;
}

G_DEFINE_TYPE (MXFMetadataNetworkLocator, mxf_metadata_network_locator,
    MXF_TYPE_METADATA_LOCATOR);

static void
mxf_metadata_network_locator_finalize (GstMiniObject * object)
{
  MXFMetadataNetworkLocator *self = MXF_METADATA_NETWORK_LOCATOR (object);

  g_free (self->url_string);
  self->url_string = NULL;

  GST_MINI_OBJECT_CLASS (mxf_metadata_network_locator_parent_class)->finalize
      (object);
}

static gboolean
mxf_metadata_network_locator_handle_tag (MXFMetadataBase * metadata,
    MXFPrimerPack * primer, guint16 tag, const guint8 * tag_data,
    guint tag_size)
{
  MXFMetadataNetworkLocator *self = MXF_METADATA_NETWORK_LOCATOR (metadata);
  gboolean ret = TRUE;

  switch (tag) {
    case 0x4101:
      self->url_string = mxf_utf16_to_utf8 (tag_data, tag_size);
      GST_DEBUG ("  url string = %s", GST_STR_NULL (self->url_string));
      break;
    default:
      ret =
          MXF_METADATA_BASE_CLASS
          (mxf_metadata_network_locator_parent_class)->handle_tag (metadata,
          primer, tag, tag_data, tag_size);
      break;
  }

  return ret;
}

static GstStructure *
mxf_metadata_network_locator_to_structure (MXFMetadataBase * m)
{
  GstStructure *ret =
      MXF_METADATA_BASE_CLASS
      (mxf_metadata_network_locator_parent_class)->to_structure (m);
  MXFMetadataNetworkLocator *self = MXF_METADATA_NETWORK_LOCATOR (m);

  gst_structure_id_set (ret, MXF_QUARK (URL_STRING), G_TYPE_STRING,
      self->url_string, NULL);

  return ret;
}

static void
mxf_metadata_network_locator_init (MXFMetadataNetworkLocator * self)
{
}

static void
mxf_metadata_network_locator_class_init (MXFMetadataNetworkLocatorClass * klass)
{
  MXFMetadataBaseClass *metadata_base_class = (MXFMetadataBaseClass *) klass;
  GstMiniObjectClass *miniobject_class = (GstMiniObjectClass *) klass;
  MXFMetadataClass *metadata_class = (MXFMetadataClass *) klass;

  miniobject_class->finalize = mxf_metadata_network_locator_finalize;
  metadata_base_class->handle_tag = mxf_metadata_network_locator_handle_tag;
  metadata_base_class->name_quark = MXF_QUARK (NETWORK_LOCATOR);
  metadata_base_class->to_structure = mxf_metadata_network_locator_to_structure;
  metadata_class->type = 0x0133;
}

G_DEFINE_ABSTRACT_TYPE (MXFDescriptiveMetadata, mxf_descriptive_metadata,
    MXF_TYPE_METADATA_BASE);

static void
mxf_descriptive_metadata_init (MXFDescriptiveMetadata * self)
{
}

static void
mxf_descriptive_metadata_class_init (MXFDescriptiveMetadataClass * klass)
{
}

typedef struct
{
  guint8 scheme;
  GType *types;
} _MXFDescriptiveMetadataScheme;

static GArray *_dm_schemes = NULL;

void
mxf_descriptive_metadata_register (guint8 scheme, GType * types)
{
  _MXFDescriptiveMetadataScheme s;

  if (!_dm_schemes)
    _dm_schemes =
        g_array_new (FALSE, TRUE, sizeof (_MXFDescriptiveMetadataScheme));

  s.scheme = scheme;
  s.types = types;

  g_array_append_val (_dm_schemes, s);
}

MXFDescriptiveMetadata *
mxf_descriptive_metadata_new (guint8 scheme, guint32 type,
    MXFPrimerPack * primer, guint64 offset, const guint8 * data, guint size)
{
  guint i;
  GType t = G_TYPE_INVALID, *p;
  _MXFDescriptiveMetadataScheme *s = NULL;
  MXFDescriptiveMetadata *ret = NULL;

  g_return_val_if_fail (type != 0, NULL);
  g_return_val_if_fail (primer != NULL, NULL);

  for (i = 0; i < _dm_schemes->len; i++) {
    _MXFDescriptiveMetadataScheme *data =
        &g_array_index (_dm_schemes, _MXFDescriptiveMetadataScheme, i);

    if (data->scheme == scheme) {
      s = data;
      break;
    }
  }

  if (s == NULL) {
    GST_WARNING ("Descriptive metadata scheme 0x%02x not supported", scheme);
    return NULL;
  }

  p = s->types;
  while (*p) {
    GType tmp = *p;
    MXFDescriptiveMetadataClass *klass =
        MXF_DESCRIPTIVE_METADATA_CLASS (g_type_class_ref (tmp));

    if (klass->type == type) {
      g_type_class_unref (klass);
      t = tmp;
      break;
    }
    g_type_class_unref (klass);
    p++;
  }

  if (t == G_TYPE_INVALID) {
    GST_WARNING
        ("No handler for type 0x%06x of descriptive metadata scheme 0x%02x found",
        type, scheme);
    return NULL;
  }

  GST_DEBUG ("DM scheme 0x%02x type 0x%06x is handled by type %s", scheme, type,
      g_type_name (t));

  ret = (MXFDescriptiveMetadata *) g_type_create_instance (t);
  if (!mxf_metadata_base_parse (MXF_METADATA_BASE (ret), primer, data, size)) {
    GST_ERROR ("Parsing metadata failed");
    gst_mini_object_unref ((GstMiniObject *) ret);
    return NULL;
  }

  ret->parent.offset = offset;

  return ret;
}

/* TODO: Remove this once we depend on GLib 2.14 */
#if GLIB_CHECK_VERSION (2, 14, 0)
#define __gst_once_init_enter(val) (g_once_init_enter (val))
#define __gst_once_init_leave(val,newval) (g_once_init_leave (val, newval))
#endif

GType
mxf_descriptive_metadata_framework_get_type (void)
{
  static volatile gsize type = 0;
  if (__gst_once_init_enter (&type)) {
    GType _type = 0;
    static const GTypeInfo info = {
      sizeof (MXFDescriptiveMetadataFrameworkInterface),
      NULL,                     /* base_init */
      NULL,                     /* base_finalize */
      NULL,                     /* class_init */
      NULL,                     /* class_finalize */
      NULL,                     /* class_data */
      0,                        /* instance_size */
      0,                        /* n_preallocs */
      NULL                      /* instance_init */
    };
    _type = g_type_register_static (G_TYPE_INTERFACE,
        "MXFDescriptiveMetadataFrameworkInterface", &info, 0);

    g_type_interface_add_prerequisite (_type, MXF_TYPE_DESCRIPTIVE_METADATA);

    __gst_once_init_leave (&type, (gsize) _type);
  }

  return (GType) type;
}

GHashTable *
mxf_metadata_hash_table_new (void)
{
  return g_hash_table_new_full ((GHashFunc) mxf_ul_hash,
      (GEqualFunc) mxf_ul_is_equal, (GDestroyNotify) NULL,
      (GDestroyNotify) gst_mini_object_unref);
}
