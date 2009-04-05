/* GStreamer
 * Copyright (C) 2008 Sebastian Dröge <sebastian.droege@collabora.co.uk>
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

/* Handling of MXF structural metadata */

#ifndef __MXF_METADATA_H__
#define __MXF_METADATA_H__

#include <gst/gst.h>
#include "mxftypes.h"

#define MXF_TYPE_METADATA_BASE \
  (mxf_metadata_base_get_type())
#define MXF_METADATA_BASE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_BASE, MXFMetadataBase))
#define MXF_IS_METADATA_BASE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_BASE))
#define MXF_METADATA_BASE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MXF_TYPE_METADATA_BASE, MXFMetadataBaseClass))
#define MXF_METADATA_BASE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),MXF_TYPE_METADATA_BASE,MXFMetadataBaseClass))
typedef struct _MXFMetadataBase MXFMetadataBase;
typedef struct _MXFMetadataBaseClass MXFMetadataBaseClass;
GType mxf_metadata_base_get_type (void);

#define MXF_TYPE_METADATA \
  (mxf_metadata_get_type())
#define MXF_METADATA(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA, MXFMetadata))
#define MXF_IS_METADATA(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA))
#define MXF_METADATA_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MXF_TYPE_METADATA, MXFMetadataClass))
#define MXF_METADATA_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),MXF_TYPE_METADATA,MXFMetadataClass))
typedef struct _MXFMetadata MXFMetadata;
typedef MXFMetadataBaseClass MXFMetadataClass;
GType mxf_metadata_get_type (void);

#define MXF_TYPE_METADATA_PREFACE \
  (mxf_metadata_preface_get_type())
#define MXF_METADATA_PREFACE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_PREFACE,MXFMetadataPreface))
#define MXF_IS_METADATA_PREFACE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_PREFACE))
typedef struct _MXFMetadataPreface MXFMetadataPreface;
typedef MXFMetadataBaseClass MXFMetadataPrefaceClass;
GType mxf_metadata_preface_get_type (void);

#define MXF_TYPE_METADATA_IDENTIFICATION \
  (mxf_metadata_identification_get_type())
#define MXF_METADATA_IDENTIFICATION(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_IDENTIFICATION,MXFMetadataIdentification))
#define MXF_IS_METADATA_IDENTIFICATION(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_IDENTIFICATION))
typedef struct _MXFMetadataIdentification MXFMetadataIdentification;
typedef MXFMetadataBaseClass MXFMetadataIdentificationClass;
GType mxf_metadata_identification_get_type (void);

#define MXF_TYPE_METADATA_CONTENT_STORAGE \
  (mxf_metadata_content_storage_get_type())
#define MXF_METADATA_CONTENT_STORAGE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_CONTENT_STORAGE, MXFMetadataContentStorage))
#define MXF_IS_METADATA_CONTENT_STORAGE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_CONTENT_STORAGE))
typedef struct _MXFMetadataContentStorage MXFMetadataContentStorage;
typedef MXFMetadataBaseClass MXFMetadataContentStorageClass;
GType mxf_metadata_content_storage_get_type (void);

#define MXF_TYPE_METADATA_ESSENCE_CONTAINER_DATA \
  (mxf_metadata_essence_container_data_get_type())
#define MXF_METADATA_ESSENCE_CONTAINER_DATA(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_ESSENCE_CONTAINER_DATA, MXFMetadataEssenceContainerData))
#define MXF_IS_METADATA_ESSENCE_CONTAINER_DATA(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_ESSENCE_CONTAINER_DATA))
typedef struct _MXFMetadataEssenceContainerData MXFMetadataEssenceContainerData;
typedef MXFMetadataBaseClass MXFMetadataEssenceContainerDataClass;
GType mxf_metadata_essence_container_data_get_type (void);

#define MXF_TYPE_METADATA_GENERIC_PACKAGE \
  (mxf_metadata_generic_package_get_type())
#define MXF_METADATA_GENERIC_PACKAGE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_GENERIC_PACKAGE, MXFMetadataGenericPackage))
#define MXF_IS_METADATA_GENERIC_PACKAGE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_GENERIC_PACKAGE))
typedef struct _MXFMetadataGenericPackage MXFMetadataGenericPackage;
typedef MXFMetadataBaseClass MXFMetadataGenericPackageClass;
GType mxf_metadata_generic_package_get_type (void);

#define MXF_TYPE_METADATA_MATERIAL_PACKAGE \
  (mxf_metadata_material_package_get_type())
#define MXF_METADATA_MATERIAL_PACKAGE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_MATERIAL_PACKAGE, MXFMetadataMaterialPackage))
#define MXF_IS_METADATA_MATERIAL_PACKAGE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_MATERIAL_PACKAGE))
typedef MXFMetadataGenericPackage MXFMetadataMaterialPackage;
typedef MXFMetadataBaseClass MXFMetadataMaterialPackageClass;
GType mxf_metadata_material_package_get_type (void);

#define MXF_TYPE_METADATA_SOURCE_PACKAGE \
  (mxf_metadata_source_package_get_type())
#define MXF_METADATA_SOURCE_PACKAGE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_SOURCE_PACKAGE, MXFMetadataSourcePackage))
#define MXF_IS_METADATA_SOURCE_PACKAGE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_SOURCE_PACKAGE))
typedef struct _MXFMetadataSourcePackage MXFMetadataSourcePackage;
typedef MXFMetadataBaseClass MXFMetadataSourcePackageClass;
GType mxf_metadata_source_package_get_type (void);

#define MXF_TYPE_METADATA_TRACK \
  (mxf_metadata_track_get_type())
#define MXF_METADATA_TRACK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_TRACK, MXFMetadataTrack))
#define MXF_IS_METADATA_TRACK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_TRACK))
typedef struct _MXFMetadataTrack MXFMetadataTrack;
typedef MXFMetadataBaseClass MXFMetadataTrackClass;
GType mxf_metadata_track_get_type (void);

#define MXF_TYPE_METADATA_TIMELINE_TRACK \
  (mxf_metadata_timeline_track_get_type())
#define MXF_METADATA_TIMELINE_TRACK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_TIMELINE_TRACK, MXFMetadataTimelineTrack))
#define MXF_IS_METADATA_TIMELINE_TRACK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_TIMELINE_TRACK))
typedef struct _MXFMetadataTimelineTrack MXFMetadataTimelineTrack;
typedef MXFMetadataBaseClass MXFMetadataTimelineTrackClass;
GType mxf_metadata_timeline_track_get_type (void);

#define MXF_TYPE_METADATA_EVENT_TRACK \
  (mxf_metadata_event_track_get_type())
#define MXF_METADATA_EVENT_TRACK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_EVENT_TRACK, MXFMetadataEventTrack))
#define MXF_IS_METADATA_EVENT_TRACK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_EVENT_TRACK))
typedef struct _MXFMetadataEventTrack MXFMetadataEventTrack;
typedef MXFMetadataBaseClass MXFMetadataEventTrackClass;
GType mxf_metadata_event_track_get_type (void);

#define MXF_TYPE_METADATA_STATIC_TRACK \
  (mxf_metadata_static_track_get_type())
#define MXF_METADATA_STATIC_TRACK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_STATIC_TRACK, MXFMetadataStaticTrack))
#define MXF_IS_METADATA_STATIC_TRACK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_STATIC_TRACK))
typedef MXFMetadataTrack MXFMetadataStaticTrack;
typedef MXFMetadataBaseClass MXFMetadataStaticTrackClass;
GType mxf_metadata_static_track_get_type (void);

#define MXF_TYPE_METADATA_SEQUENCE \
  (mxf_metadata_sequence_get_type())
#define MXF_METADATA_SEQUENCE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_SEQUENCE, MXFMetadataSequence))
#define MXF_IS_METADATA_SEQUENCE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_SEQUENCE))
typedef struct _MXFMetadataSequence MXFMetadataSequence;
typedef MXFMetadataBaseClass MXFMetadataSequenceClass;
GType mxf_metadata_sequence_get_type (void);

#define MXF_TYPE_METADATA_STRUCTURAL_COMPONENT \
  (mxf_metadata_structural_component_get_type())
#define MXF_METADATA_STRUCTURAL_COMPONENT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_STRUCTURAL_COMPONENT, MXFMetadataStructuralComponent))
#define MXF_IS_METADATA_STRUCTURAL_COMPONENT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_STRUCTURAL_COMPONENT))
typedef struct _MXFMetadataStructuralComponent MXFMetadataStructuralComponent;
typedef MXFMetadataBaseClass MXFMetadataStructuralComponentClass;
GType mxf_metadata_structural_component_get_type (void);

#define MXF_TYPE_METADATA_SOURCE_CLIP \
  (mxf_metadata_source_clip_get_type())
#define MXF_METADATA_SOURCE_CLIP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_SOURCE_CLIP, MXFMetadataSourceClip))
#define MXF_IS_METADATA_SOURCE_CLIP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_SOURCE_CLIP))
typedef struct _MXFMetadataSourceClip MXFMetadataSourceClip;
typedef MXFMetadataBaseClass MXFMetadataSourceClipClass;
GType mxf_metadata_source_clip_get_type (void);

#define MXF_TYPE_METADATA_TIMECODE_COMPONENT \
  (mxf_metadata_timecode_component_get_type())
#define MXF_METADATA_TIMECODE_COMPONENT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_TIMECODE_COMPONENT, MXFMetadataTimecodeComponent))
#define MXF_IS_METADATA_TIMECODE_COMPONENT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_TIMECODE_COMPONENT))
typedef struct _MXFMetadataTimecodeComponent MXFMetadataTimecodeComponent;
typedef MXFMetadataBaseClass MXFMetadataTimecodeComponentClass;
GType mxf_metadata_timecode_component_get_type (void);

#define MXF_TYPE_METADATA_DM_SOURCE_CLIP \
  (mxf_metadata_dm_source_clip_get_type())
#define MXF_METADATA_DM_SOURCE_CLIP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_DM_SOURCE_CLIP, MXFMetadataDMSourceClip))
#define MXF_IS_METADATA_DM_SOURCE_CLIP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_DM_SOURCE_CLIP))
typedef struct _MXFMetadataDMSourceClip MXFMetadataDMSourceClip;
typedef MXFMetadataBaseClass MXFMetadataDMSourceClipClass;
GType mxf_metadata_dm_source_clip_get_type (void);

#define MXF_TYPE_METADATA_DM_SEGMENT \
  (mxf_metadata_dm_segment_get_type())
#define MXF_METADATA_DM_SEGMENT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_DM_SEGMENT, MXFMetadataDMSegment))
#define MXF_IS_METADATA_DM_SEGMENT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_DM_SEGMENT))
typedef struct _MXFMetadataDMSegment MXFMetadataDMSegment;
typedef MXFMetadataBaseClass MXFMetadataDMSegmentClass;
GType mxf_metadata_dm_segment_get_type (void);

#define MXF_TYPE_METADATA_GENERIC_DESCRIPTOR \
  (mxf_metadata_generic_descriptor_get_type())
#define MXF_METADATA_GENERIC_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_GENERIC_DESCRIPTOR, MXFMetadataGenericDescriptor))
#define MXF_IS_METADATA_GENERIC_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_GENERIC_DESCRIPTOR))
typedef struct _MXFMetadataGenericDescriptor MXFMetadataGenericDescriptor;
typedef MXFMetadataBaseClass MXFMetadataGenericDescriptorClass;
GType mxf_metadata_generic_descriptor_get_type (void);

#define MXF_TYPE_METADATA_FILE_DESCRIPTOR \
  (mxf_metadata_file_descriptor_get_type())
#define MXF_METADATA_FILE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_FILE_DESCRIPTOR, MXFMetadataFileDescriptor))
#define MXF_IS_METADATA_FILE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_FILE_DESCRIPTOR))
typedef struct _MXFMetadataFileDescriptor MXFMetadataFileDescriptor;
typedef MXFMetadataBaseClass MXFMetadataFileDescriptorClass;
GType mxf_metadata_file_descriptor_get_type (void);

#define MXF_TYPE_METADATA_GENERIC_PICTURE_ESSENCE_DESCRIPTOR \
  (mxf_metadata_generic_picture_essence_descriptor_get_type())
#define MXF_METADATA_GENERIC_PICTURE_ESSENCE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_GENERIC_PICTURE_ESSENCE_DESCRIPTOR, MXFMetadataGenericPictureEssenceDescriptor))
#define MXF_IS_METADATA_GENERIC_PICTURE_ESSENCE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_GENERIC_PICTURE_ESSENCE_DESCRIPTOR))
typedef struct _MXFMetadataGenericPictureEssenceDescriptor MXFMetadataGenericPictureEssenceDescriptor;
typedef MXFMetadataBaseClass MXFMetadataGenericPictureEssenceDescriptorClass;
GType mxf_metadata_generic_picture_essence_descriptor_get_type (void);

#define MXF_TYPE_METADATA_CDCI_PICTURE_ESSENCE_DESCRIPTOR \
  (mxf_metadata_cdci_picture_essence_descriptor_get_type())
#define MXF_METADATA_CDCI_PICTURE_ESSENCE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_CDCI_PICTURE_ESSENCE_DESCRIPTOR, MXFMetadataCDCIPictureEssenceDescriptor))
#define MXF_IS_METADATA_CDCI_PICTURE_ESSENCE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_CDCI_PICTURE_ESSENCE_DESCRIPTOR))
typedef struct _MXFMetadataCDCIPictureEssenceDescriptor MXFMetadataCDCIPictureEssenceDescriptor;
typedef MXFMetadataBaseClass MXFMetadataCDCIPictureEssenceDescriptorClass;
GType mxf_metadata_cdci_picture_essence_descriptor_get_type (void);

#define MXF_TYPE_METADATA_RGBA_PICTURE_ESSENCE_DESCRIPTOR \
  (mxf_metadata_rgba_picture_essence_descriptor_get_type())
#define MXF_METADATA_RGBA_PICTURE_ESSENCE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_RGBA_PICTURE_ESSENCE_DESCRIPTOR, MXFMetadataRGBAPictureEssenceDescriptor))
#define MXF_IS_METADATA_RGBA_PICTURE_ESSENCE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_RGBA_PICTURE_ESSENCE_DESCRIPTOR))
typedef struct _MXFMetadataRGBAPictureEssenceDescriptor MXFMetadataRGBAPictureEssenceDescriptor;
typedef MXFMetadataBaseClass MXFMetadataRGBAPictureEssenceDescriptorClass;
GType mxf_metadata_rgba_picture_essence_descriptor_get_type (void);

#define MXF_TYPE_METADATA_GENERIC_SOUND_ESSENCE_DESCRIPTOR \
  (mxf_metadata_generic_sound_essence_descriptor_get_type())
#define MXF_METADATA_GENERIC_SOUND_ESSENCE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_GENERIC_SOUND_ESSENCE_DESCRIPTOR, MXFMetadataGenericSoundEssenceDescriptor))
#define MXF_IS_METADATA_GENERIC_SOUND_ESSENCE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_GENERIC_SOUND_ESSENCE_DESCRIPTOR))
typedef struct _MXFMetadataGenericSoundEssenceDescriptor MXFMetadataGenericSoundEssenceDescriptor;
typedef MXFMetadataBaseClass MXFMetadataGenericSoundEssenceDescriptorClass;
GType mxf_metadata_generic_sound_essence_descriptor_get_type (void);

#define MXF_TYPE_METADATA_GENERIC_DATA_ESSENCE_DESCRIPTOR \
  (mxf_metadata_generic_data_essence_descriptor_get_type())
#define MXF_METADATA_GENERIC_DATA_ESSENCE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_GENERIC_DATA_ESSENCE_DESCRIPTOR, MXFMetadataGenericDataEssenceDescriptor))
#define MXF_IS_METADATA_GENERIC_DATA_ESSENCE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_GENERIC_DATA_ESSENCE_DESCRIPTOR))
typedef struct _MXFMetadataGenericDataEssenceDescriptor MXFMetadataGenericDataEssenceDescriptor;
typedef MXFMetadataBaseClass MXFMetadataGenericDataEssenceDescriptorClass;
GType mxf_metadata_generic_data_essence_descriptor_get_type (void);

#define MXF_TYPE_METADATA_MULTIPLE_DESCRIPTOR \
  (mxf_metadata_multiple_descriptor_get_type())
#define MXF_METADATA_MULTIPLE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_MULTIPLE_DESCRIPTOR, MXFMetadataMultipleDescriptor))
#define MXF_IS_METADATA_MULTIPLE_DESCRIPTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_MULTIPLE_DESCRIPTOR))
typedef struct _MXFMetadataMultipleDescriptor MXFMetadataMultipleDescriptor;
typedef MXFMetadataBaseClass MXFMetadataMultipleDescriptorClass;
GType mxf_metadata_multiple_descriptor_get_type (void);

#define MXF_TYPE_METADATA_LOCATOR \
  (mxf_metadata_locator_get_type())
#define MXF_METADATA_LOCATOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_LOCATOR, MXFMetadataLocator))
#define MXF_IS_METADATA_LOCATOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_LOCATOR))
typedef struct _MXFMetadataLocator MXFMetadataLocator;
typedef MXFMetadataBaseClass MXFMetadataLocatorClass;
GType mxf_metadata_locator_get_type (void);

#define MXF_TYPE_METADATA_NETWORK_LOCATOR \
  (mxf_metadata_network_locator_get_type())
#define MXF_METADATA_NETWORK_LOCATOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_NETWORK_LOCATOR, MXFMetadataNetworkLocator))
#define MXF_IS_METADATA_NETWORK_LOCATOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_NETWORK_LOCATOR))
typedef struct _MXFMetadataNetworkLocator MXFMetadataNetworkLocator;
typedef MXFMetadataBaseClass MXFMetadataNetworkLocatorClass;
GType mxf_metadata_network_locator_get_type (void);

#define MXF_TYPE_METADATA_TEXT_LOCATOR \
  (mxf_metadata_text_locator_get_type())
#define MXF_METADATA_TEXT_LOCATOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),MXF_TYPE_METADATA_TEXT_LOCATOR, MXFMetadataTextLocator))
#define MXF_IS_METADATA_TEXT_LOCATOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),MXF_TYPE_METADATA_TEXT_LOCATOR))
typedef struct _MXFMetadataTextLocator MXFMetadataTextLocator;
typedef MXFMetadataBaseClass MXFMetadataTextLocatorClass;
GType mxf_metadata_text_locator_get_type (void);

typedef enum {
  MXF_METADATA_BASE_RESOLVE_STATE_NONE = 0,
  MXF_METADATA_BASE_RESOLVE_STATE_SUCCESS,
  MXF_METADATA_BASE_RESOLVE_STATE_FAILURE
} MXFMetadataBaseResolveState;

struct _MXFMetadataBase {
  GstMiniObject parent;

  MXFUL instance_uid;

  MXFMetadataBaseResolveState resolved;

  GHashTable *other_tags;
};

struct _MXFMetadataBaseClass {
  GstMiniObjectClass parent;

  gboolean (*handle_tag) (MXFMetadataBase *self, MXFPrimerPack *primer, guint16 tag, const guint8 *tag_data, guint tag_size);
  gboolean (*resolve) (MXFMetadataBase *self, MXFMetadataBase **metadata);
};

struct _MXFMetadata {
  MXFMetadataBase parent;
  
  guint16 type;
};

struct _MXFMetadataPreface {
  MXFMetadata parent;

  MXFUL generation_uid;

  MXFTimestamp last_modified_date;
  guint16 version;

  guint32 object_model_version;

  MXFUL primary_package_uid;
  MXFMetadataGenericPackage *primary_package;

  guint32 n_identifications;
  MXFUL *identifications_uids;
  MXFMetadataIdentification **identifications;

  MXFUL content_storage_uid;
  MXFMetadataContentStorage *content_storage;

  MXFUL operational_pattern;

  guint32 n_essence_containers;
  MXFUL *essence_containers;

  guint32 n_dm_schemes;
  MXFUL *dm_schemes;
};

struct _MXFMetadataIdentification {
  MXFMetadata parent;

  MXFUL this_generation_uid;

  gchar *company_name;

  gchar *product_name;
  MXFProductVersion product_version;
  
  gchar *version_string;

  MXFUL product_uid;

  MXFTimestamp modification_date;

  MXFProductVersion toolkit_version;

  gchar *platform;
};

struct _MXFMetadataContentStorage {
  MXFMetadata parent;

  MXFUL generation_uid;

  guint32 n_packages;
  MXFUL *packages_uids;
  MXFMetadataGenericPackage **packages;

  guint32 n_essence_container_data;
  MXFUL *essence_container_data_uids;
  MXFMetadataEssenceContainerData **essence_container_data;
};

struct _MXFMetadataEssenceContainerData {
  MXFMetadata parent;

  MXFUL generation_uid;

  MXFUMID linked_package_uid;
  MXFMetadataSourcePackage *linked_package;

  guint32 index_sid;
  guint32 body_sid;
};

struct _MXFMetadataGenericPackage {
  MXFMetadata parent;

  MXFUL generation_uid;

  MXFUMID package_uid;

  gchar *name;
  MXFTimestamp package_creation_date;
  MXFTimestamp package_modified_date;

  guint32 n_tracks;
  MXFUL *tracks_uids;
  MXFMetadataTrack **tracks;

  guint n_timecode_tracks;
  guint n_metadata_tracks;
  guint n_essence_tracks;
  guint n_other_tracks;
};

struct _MXFMetadataSourcePackage
{
  MXFMetadataGenericPackage parent;

  MXFUL descriptors_uid;
  guint32 n_descriptors;
  MXFMetadataGenericDescriptor **descriptors;

  gboolean top_level;
};

typedef enum {
  MXF_METADATA_TRACK_UNKNOWN               = 0x00,
  MXF_METADATA_TRACK_TIMECODE_12M_INACTIVE = 0x10,
  MXF_METADATA_TRACK_TIMECODE_12M_ACTIVE   = 0x11,
  MXF_METADATA_TRACK_TIMECODE_309M         = 0x12,
  MXF_METADATA_TRACK_METADATA              = 0x20,
  MXF_METADATA_TRACK_PICTURE_ESSENCE       = 0x30,
  MXF_METADATA_TRACK_SOUND_ESSENCE         = 0x31,
  MXF_METADATA_TRACK_DATA_ESSENCE          = 0x32,
  MXF_METADATA_TRACK_AUXILIARY_DATA        = 0x40,
  MXF_METADATA_TRACK_PARSED_TEXT           = 0x41
} MXFMetadataTrackType;

struct _MXFMetadataTrack {
  MXFMetadata parent;

  MXFUL generation_uid;

  guint32 track_id;
  guint32 track_number;

  gchar *track_name;

  MXFUL sequence_uid;
  MXFMetadataSequence *sequence;

  MXFMetadataTrackType type;

  MXFMetadataFileDescriptor **descriptor;
  guint n_descriptor;
};

struct _MXFMetadataTimelineTrack {
  MXFMetadataTrack parent;

  MXFFraction edit_rate;
  gint64 origin;
};

struct _MXFMetadataEventTrack {
  MXFMetadataTrack parent;

  MXFFraction event_edit_rate;
  gint64 event_origin;
};

struct _MXFMetadataSequence {
  MXFMetadata parent;

  MXFUL generation_uid;

  MXFUL data_definition;

  gint64 duration;

  guint32 n_structural_components;
  MXFUL *structural_components_uids;
  MXFMetadataStructuralComponent **structural_components;
};

struct _MXFMetadataStructuralComponent {
  MXFMetadata parent;

  MXFUL generation_uid;

  MXFUL data_definition;
  gint64 duration;
};

struct _MXFMetadataTimecodeComponent {
  MXFMetadataStructuralComponent parent;

  gint64 start_timecode;
  guint16 rounded_timecode_base;
  gboolean drop_frame;
};

struct _MXFMetadataSourceClip {
  MXFMetadataStructuralComponent parent;

  gint64 start_position;
  MXFUMID source_package_id;
  MXFMetadataSourcePackage *source_package;

  guint32 source_track_id;
};

struct _MXFMetadataDMSourceClip {
  MXFMetadataSourceClip parent;

  guint32 n_track_ids;
  guint32 *track_ids;
};

struct _MXFMetadataDMSegment {
  MXFMetadataStructuralComponent parent;

  gint64 event_start_position;
  gchar *event_comment;

  guint32 n_track_ids;
  guint32 *track_ids;
      
  MXFUL dm_framework_uid;
  MXFMetadataBase *dm_framework;
};

struct _MXFMetadataGenericDescriptor {
  MXFMetadata parent;

  MXFUL generation_uid;

  guint32 n_locators;
  MXFUL *locators_uids;
  MXFMetadataLocator **locators;
};

struct _MXFMetadataFileDescriptor {
  MXFMetadataGenericDescriptor parent;

  guint32 linked_track_id;

  MXFFraction sample_rate;
  gint64 container_duration;

  MXFUL essence_container;
  MXFUL codec;
};

struct _MXFMetadataGenericPictureEssenceDescriptor {
  MXFMetadataFileDescriptor parent;

  guint8 signal_standard;
  guint8 frame_layout;

  guint32 stored_width;
  guint32 stored_height;
  gint32 stored_f2_offset;
  guint32 sampled_width;
  guint32 sampled_height;
  gint32 sampled_x_offset;
  gint32 sampled_y_offset;
  guint32 display_height;
  guint32 display_width;
  gint32 display_x_offset;
  gint32 display_y_offset;
  gint32 display_f2_offset;
  MXFFraction aspect_ratio;

  guint8 active_format_descriptor;
  gint32 video_line_map[2];
  guint8 alpha_transparency;
  MXFUL capture_gamma;

  guint32 image_alignment_offset;
  guint32 image_start_offset;
  guint32 image_end_offset;

  guint8 field_dominance;

  MXFUL picture_essence_coding;
};

struct _MXFMetadataCDCIPictureEssenceDescriptor {
  MXFMetadataGenericPictureEssenceDescriptor parent;

  guint32 component_depth;
  guint32 horizontal_subsampling;
  guint32 vertical_subsampling;
  guint8 color_siting;
  gboolean reversed_byte_order;
  gint16 padding_bits;
  guint32 alpha_sample_depth;
  guint32 black_ref_level;
  guint32 white_ref_level;
  guint32 color_range;
};

struct _MXFMetadataRGBAPictureEssenceDescriptor {
  MXFMetadataGenericPictureEssenceDescriptor parent;

  guint32 component_max_ref;
  guint32 component_min_ref;
  guint32 alpha_max_ref;
  guint32 alpha_min_ref;
  guint8 scanning_direction;

  guint32 n_pixel_layout;
  guint8 *pixel_layout;

  /* TODO: palette & palette layout */
};

struct _MXFMetadataGenericSoundEssenceDescriptor {
  MXFMetadataFileDescriptor parent;

  MXFFraction audio_sampling_rate;

  gboolean locked;

  gint8 audio_ref_level;

  guint8 electro_spatial_formulation;

  guint32 channel_count;
  guint32 quantization_bits;

  gint8 dial_norm;

  MXFUL sound_essence_compression;
};

struct _MXFMetadataGenericDataEssenceDescriptor {
  MXFMetadataFileDescriptor parent;

  MXFUL data_essence_compression;
};

struct _MXFMetadataMultipleDescriptor {
  MXFMetadataFileDescriptor parent;
  
  MXFUL *sub_descriptors_uids;
  guint32 n_sub_descriptors;
  MXFMetadataGenericDescriptor **sub_descriptors;
};

struct _MXFMetadataLocator {
  MXFMetadata parent;

  MXFUL generation_uid;
};

struct _MXFMetadataNetworkLocator {
  MXFMetadataLocator parent;

  gchar *url_string;
};

struct _MXFMetadataTextLocator {
  MXFMetadataLocator parent;

  gchar *locator_name;
};

gboolean mxf_metadata_base_parse (MXFMetadataBase *self, MXFPrimerPack *primer, const guint8 *data, guint size);
gboolean mxf_metadata_base_resolve (MXFMetadataBase *self, MXFMetadataBase **metadata);

MXFMetadata *mxf_metadata_new (guint16 type, MXFPrimerPack *primer, const guint8 *data, guint size);
void mxf_metadata_register (guint16 type_id, GType type);
void mxf_metadata_init_types (void);

MXFMetadataTrackType mxf_metadata_track_identifier_parse (const MXFUL * track_identifier);

void mxf_metadata_generic_picture_essence_descriptor_set_caps (MXFMetadataGenericPictureEssenceDescriptor * self, GstCaps * caps);
void mxf_metadata_generic_sound_essence_descriptor_set_caps (MXFMetadataGenericSoundEssenceDescriptor * self, GstCaps * caps);

#endif /* __MXF_METADATA_H__ */
