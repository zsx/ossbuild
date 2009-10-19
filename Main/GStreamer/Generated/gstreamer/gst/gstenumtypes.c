


#include "gst_private.h"
#include <gst/gst.h>
#define C_ENUM(v) ((gint) v)
#define C_FLAGS(v) ((guint) v)
 

/* enumerations from "gstobject.h" */
static void
register_gst_object_flags (GType* id)
{
  static const GFlagsValue values[] = {
    { C_FLAGS(GST_OBJECT_DISPOSING), "GST_OBJECT_DISPOSING", "disposing" },
    { C_FLAGS(GST_OBJECT_FLOATING), "GST_OBJECT_FLOATING", "floating" },
    { C_FLAGS(GST_OBJECT_FLAG_LAST), "GST_OBJECT_FLAG_LAST", "flag-last" },
    { 0, NULL, NULL }
  };
  *id = g_flags_register_static ("GstObjectFlags", values);
}
GType
gst_object_flags_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_object_flags, &id);
  return id;
}

/* enumerations from "gstbin.h" */
static void
register_gst_bin_flags (GType* id)
{
  static const GFlagsValue values[] = {
    { C_FLAGS(GST_BIN_FLAG_LAST), "GST_BIN_FLAG_LAST", "last" },
    { 0, NULL, NULL }
  };
  *id = g_flags_register_static ("GstBinFlags", values);
}
GType
gst_bin_flags_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_bin_flags, &id);
  return id;
}

/* enumerations from "gstbuffer.h" */
static void
register_gst_buffer_flag (GType* id)
{
  static const GFlagsValue values[] = {
    { C_FLAGS(GST_BUFFER_FLAG_READONLY), "GST_BUFFER_FLAG_READONLY", "readonly" },
    { C_FLAGS(GST_BUFFER_FLAG_PREROLL), "GST_BUFFER_FLAG_PREROLL", "preroll" },
    { C_FLAGS(GST_BUFFER_FLAG_DISCONT), "GST_BUFFER_FLAG_DISCONT", "discont" },
    { C_FLAGS(GST_BUFFER_FLAG_IN_CAPS), "GST_BUFFER_FLAG_IN_CAPS", "in-caps" },
    { C_FLAGS(GST_BUFFER_FLAG_GAP), "GST_BUFFER_FLAG_GAP", "gap" },
    { C_FLAGS(GST_BUFFER_FLAG_DELTA_UNIT), "GST_BUFFER_FLAG_DELTA_UNIT", "delta-unit" },
    { C_FLAGS(GST_BUFFER_FLAG_MEDIA1), "GST_BUFFER_FLAG_MEDIA1", "media1" },
    { C_FLAGS(GST_BUFFER_FLAG_MEDIA2), "GST_BUFFER_FLAG_MEDIA2", "media2" },
    { C_FLAGS(GST_BUFFER_FLAG_MEDIA3), "GST_BUFFER_FLAG_MEDIA3", "media3" },
    { C_FLAGS(GST_BUFFER_FLAG_LAST), "GST_BUFFER_FLAG_LAST", "last" },
    { 0, NULL, NULL }
  };
  *id = g_flags_register_static ("GstBufferFlag", values);
}
GType
gst_buffer_flag_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_buffer_flag, &id);
  return id;
}
static void
register_gst_buffer_copy_flags (GType* id)
{
  static const GFlagsValue values[] = {
    { C_FLAGS(GST_BUFFER_COPY_FLAGS), "GST_BUFFER_COPY_FLAGS", "flags" },
    { C_FLAGS(GST_BUFFER_COPY_TIMESTAMPS), "GST_BUFFER_COPY_TIMESTAMPS", "timestamps" },
    { C_FLAGS(GST_BUFFER_COPY_CAPS), "GST_BUFFER_COPY_CAPS", "caps" },
    { 0, NULL, NULL }
  };
  *id = g_flags_register_static ("GstBufferCopyFlags", values);
}
GType
gst_buffer_copy_flags_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_buffer_copy_flags, &id);
  return id;
}

/* enumerations from "gstbufferlist.h" */
static void
register_gst_buffer_list_item (GType* id)
{
  static const GEnumValue values[] = {
    { C_ENUM(GST_BUFFER_LIST_CONTINUE), "GST_BUFFER_LIST_CONTINUE", "continue" },
    { C_ENUM(GST_BUFFER_LIST_SKIP_GROUP), "GST_BUFFER_LIST_SKIP_GROUP", "skip-group" },
    { C_ENUM(GST_BUFFER_LIST_END), "GST_BUFFER_LIST_END", "end" },
    { 0, NULL, NULL }
  };
  *id = g_enum_register_static ("GstBufferListItem", values);
}
GType
gst_buffer_list_item_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_buffer_list_item, &id);
  return id;
}

/* enumerations from "gstbus.h" */
static void
register_gst_bus_flags (GType* id)
{
  static const GFlagsValue values[] = {
    { C_FLAGS(GST_BUS_FLUSHING), "GST_BUS_FLUSHING", "flushing" },
    { C_FLAGS(GST_BUS_FLAG_LAST), "GST_BUS_FLAG_LAST", "flag-last" },
    { 0, NULL, NULL }
  };
  *id = g_flags_register_static ("GstBusFlags", values);
}
GType
gst_bus_flags_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_bus_flags, &id);
  return id;
}
static void
register_gst_bus_sync_reply (GType* id)
{
  static const GEnumValue values[] = {
    { C_ENUM(GST_BUS_DROP), "GST_BUS_DROP", "drop" },
    { C_ENUM(GST_BUS_PASS), "GST_BUS_PASS", "pass" },
    { C_ENUM(GST_BUS_ASYNC), "GST_BUS_ASYNC", "async" },
    { 0, NULL, NULL }
  };
  *id = g_enum_register_static ("GstBusSyncReply", values);
}
GType
gst_bus_sync_reply_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_bus_sync_reply, &id);
  return id;
}

/* enumerations from "gstcaps.h" */
static void
register_gst_caps_flags (GType* id)
{
  static const GFlagsValue values[] = {
    { C_FLAGS(GST_CAPS_FLAGS_ANY), "GST_CAPS_FLAGS_ANY", "any" },
    { 0, NULL, NULL }
  };
  *id = g_flags_register_static ("GstCapsFlags", values);
}
GType
gst_caps_flags_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_caps_flags, &id);
  return id;
}

/* enumerations from "gstclock.h" */
static void
register_gst_clock_return (GType* id)
{
  static const GEnumValue values[] = {
    { C_ENUM(GST_CLOCK_OK), "GST_CLOCK_OK", "ok" },
    { C_ENUM(GST_CLOCK_EARLY), "GST_CLOCK_EARLY", "early" },
    { C_ENUM(GST_CLOCK_UNSCHEDULED), "GST_CLOCK_UNSCHEDULED", "unscheduled" },
    { C_ENUM(GST_CLOCK_BUSY), "GST_CLOCK_BUSY", "busy" },
    { C_ENUM(GST_CLOCK_BADTIME), "GST_CLOCK_BADTIME", "badtime" },
    { C_ENUM(GST_CLOCK_ERROR), "GST_CLOCK_ERROR", "error" },
    { C_ENUM(GST_CLOCK_UNSUPPORTED), "GST_CLOCK_UNSUPPORTED", "unsupported" },
    { 0, NULL, NULL }
  };
  *id = g_enum_register_static ("GstClockReturn", values);
}
GType
gst_clock_return_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_clock_return, &id);
  return id;
}
static void
register_gst_clock_entry_type (GType* id)
{
  static const GEnumValue values[] = {
    { C_ENUM(GST_CLOCK_ENTRY_SINGLE), "GST_CLOCK_ENTRY_SINGLE", "single" },
    { C_ENUM(GST_CLOCK_ENTRY_PERIODIC), "GST_CLOCK_ENTRY_PERIODIC", "periodic" },
    { 0, NULL, NULL }
  };
  *id = g_enum_register_static ("GstClockEntryType", values);
}
GType
gst_clock_entry_type_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_clock_entry_type, &id);
  return id;
}
static void
register_gst_clock_flags (GType* id)
{
  static const GFlagsValue values[] = {
    { C_FLAGS(GST_CLOCK_FLAG_CAN_DO_SINGLE_SYNC), "GST_CLOCK_FLAG_CAN_DO_SINGLE_SYNC", "can-do-single-sync" },
    { C_FLAGS(GST_CLOCK_FLAG_CAN_DO_SINGLE_ASYNC), "GST_CLOCK_FLAG_CAN_DO_SINGLE_ASYNC", "can-do-single-async" },
    { C_FLAGS(GST_CLOCK_FLAG_CAN_DO_PERIODIC_SYNC), "GST_CLOCK_FLAG_CAN_DO_PERIODIC_SYNC", "can-do-periodic-sync" },
    { C_FLAGS(GST_CLOCK_FLAG_CAN_DO_PERIODIC_ASYNC), "GST_CLOCK_FLAG_CAN_DO_PERIODIC_ASYNC", "can-do-periodic-async" },
    { C_FLAGS(GST_CLOCK_FLAG_CAN_SET_RESOLUTION), "GST_CLOCK_FLAG_CAN_SET_RESOLUTION", "can-set-resolution" },
    { C_FLAGS(GST_CLOCK_FLAG_CAN_SET_MASTER), "GST_CLOCK_FLAG_CAN_SET_MASTER", "can-set-master" },
    { C_FLAGS(GST_CLOCK_FLAG_LAST), "GST_CLOCK_FLAG_LAST", "last" },
    { 0, NULL, NULL }
  };
  *id = g_flags_register_static ("GstClockFlags", values);
}
GType
gst_clock_flags_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_clock_flags, &id);
  return id;
}

/* enumerations from "gstdebugutils.h" */
static void
register_gst_debug_graph_details (GType* id)
{
  static const GFlagsValue values[] = {
    { C_FLAGS(GST_DEBUG_GRAPH_SHOW_MEDIA_TYPE), "GST_DEBUG_GRAPH_SHOW_MEDIA_TYPE", "media-type" },
    { C_FLAGS(GST_DEBUG_GRAPH_SHOW_CAPS_DETAILS), "GST_DEBUG_GRAPH_SHOW_CAPS_DETAILS", "caps-details" },
    { C_FLAGS(GST_DEBUG_GRAPH_SHOW_NON_DEFAULT_PARAMS), "GST_DEBUG_GRAPH_SHOW_NON_DEFAULT_PARAMS", "non-default-params" },
    { C_FLAGS(GST_DEBUG_GRAPH_SHOW_STATES), "GST_DEBUG_GRAPH_SHOW_STATES", "states" },
    { C_FLAGS(GST_DEBUG_GRAPH_SHOW_ALL), "GST_DEBUG_GRAPH_SHOW_ALL", "all" },
    { 0, NULL, NULL }
  };
  *id = g_flags_register_static ("GstDebugGraphDetails", values);
}
GType
gst_debug_graph_details_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_debug_graph_details, &id);
  return id;
}

/* enumerations from "gstelement.h" */
static void
register_gst_state (GType* id)
{
  static const GEnumValue values[] = {
    { C_ENUM(GST_STATE_VOID_PENDING), "GST_STATE_VOID_PENDING", "void-pending" },
    { C_ENUM(GST_STATE_NULL), "GST_STATE_NULL", "null" },
    { C_ENUM(GST_STATE_READY), "GST_STATE_READY", "ready" },
    { C_ENUM(GST_STATE_PAUSED), "GST_STATE_PAUSED", "paused" },
    { C_ENUM(GST_STATE_PLAYING), "GST_STATE_PLAYING", "playing" },
    { 0, NULL, NULL }
  };
  *id = g_enum_register_static ("GstState", values);
}
GType
gst_state_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_state, &id);
  return id;
}
static void
register_gst_state_change_return (GType* id)
{
  static const GEnumValue values[] = {
    { C_ENUM(GST_STATE_CHANGE_FAILURE), "GST_STATE_CHANGE_FAILURE", "failure" },
    { C_ENUM(GST_STATE_CHANGE_SUCCESS), "GST_STATE_CHANGE_SUCCESS", "success" },
    { C_ENUM(GST_STATE_CHANGE_ASYNC), "GST_STATE_CHANGE_ASYNC", "async" },
    { C_ENUM(GST_STATE_CHANGE_NO_PREROLL), "GST_STATE_CHANGE_NO_PREROLL", "no-preroll" },
    { 0, NULL, NULL }
  };
  *id = g_enum_register_static ("GstStateChangeReturn", values);
}
GType
gst_state_change_return_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_state_change_return, &id);
  return id;
}
static void
register_gst_state_change (GType* id)
{
  static const GEnumValue values[] = {
    { C_ENUM(GST_STATE_CHANGE_NULL_TO_READY), "GST_STATE_CHANGE_NULL_TO_READY", "null-to-ready" },
    { C_ENUM(GST_STATE_CHANGE_READY_TO_PAUSED), "GST_STATE_CHANGE_READY_TO_PAUSED", "ready-to-paused" },
    { C_ENUM(GST_STATE_CHANGE_PAUSED_TO_PLAYING), "GST_STATE_CHANGE_PAUSED_TO_PLAYING", "paused-to-playing" },
    { C_ENUM(GST_STATE_CHANGE_PLAYING_TO_PAUSED), "GST_STATE_CHANGE_PLAYING_TO_PAUSED", "playing-to-paused" },
    { C_ENUM(GST_STATE_CHANGE_PAUSED_TO_READY), "GST_STATE_CHANGE_PAUSED_TO_READY", "paused-to-ready" },
    { C_ENUM(GST_STATE_CHANGE_READY_TO_NULL), "GST_STATE_CHANGE_READY_TO_NULL", "ready-to-null" },
    { 0, NULL, NULL }
  };
  *id = g_enum_register_static ("GstStateChange", values);
}
GType
gst_state_change_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_state_change, &id);
  return id;
}
static void
register_gst_element_flags (GType* id)
{
  static const GFlagsValue values[] = {
    { C_FLAGS(GST_ELEMENT_LOCKED_STATE), "GST_ELEMENT_LOCKED_STATE", "locked-state" },
    { C_FLAGS(GST_ELEMENT_IS_SINK), "GST_ELEMENT_IS_SINK", "is-sink" },
    { C_FLAGS(GST_ELEMENT_UNPARENTING), "GST_ELEMENT_UNPARENTING", "unparenting" },
    { C_FLAGS(GST_ELEMENT_FLAG_LAST), "GST_ELEMENT_FLAG_LAST", "flag-last" },
    { 0, NULL, NULL }
  };
  *id = g_flags_register_static ("GstElementFlags", values);
}
GType
gst_element_flags_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_element_flags, &id);
  return id;
}

/* enumerations from "gsterror.h" */
static void
register_gst_core_error (GType* id)
{
  static const GEnumValue values[] = {
    { C_ENUM(GST_CORE_ERROR_FAILED), "GST_CORE_ERROR_FAILED", "failed" },
    { C_ENUM(GST_CORE_ERROR_TOO_LAZY), "GST_CORE_ERROR_TOO_LAZY", "too-lazy" },
    { C_ENUM(GST_CORE_ERROR_NOT_IMPLEMENTED), "GST_CORE_ERROR_NOT_IMPLEMENTED", "not-implemented" },
    { C_ENUM(GST_CORE_ERROR_STATE_CHANGE), "GST_CORE_ERROR_STATE_CHANGE", "state-change" },
    { C_ENUM(GST_CORE_ERROR_PAD), "GST_CORE_ERROR_PAD", "pad" },
    { C_ENUM(GST_CORE_ERROR_THREAD), "GST_CORE_ERROR_THREAD", "thread" },
    { C_ENUM(GST_CORE_ERROR_NEGOTIATION), "GST_CORE_ERROR_NEGOTIATION", "negotiation" },
    { C_ENUM(GST_CORE_ERROR_EVENT), "GST_CORE_ERROR_EVENT", "event" },
    { C_ENUM(GST_CORE_ERROR_SEEK), "GST_CORE_ERROR_SEEK", "seek" },
    { C_ENUM(GST_CORE_ERROR_CAPS), "GST_CORE_ERROR_CAPS", "caps" },
    { C_ENUM(GST_CORE_ERROR_TAG), "GST_CORE_ERROR_TAG", "tag" },
    { C_ENUM(GST_CORE_ERROR_MISSING_PLUGIN), "GST_CORE_ERROR_MISSING_PLUGIN", "missing-plugin" },
    { C_ENUM(GST_CORE_ERROR_CLOCK), "GST_CORE_ERROR_CLOCK", "clock" },
    { C_ENUM(GST_CORE_ERROR_DISABLED), "GST_CORE_ERROR_DISABLED", "disabled" },
    { C_ENUM(GST_CORE_ERROR_NUM_ERRORS), "GST_CORE_ERROR_NUM_ERRORS", "num-errors" },
    { 0, NULL, NULL }
  };
  *id = g_enum_register_static ("GstCoreError", values);
}
GType
gst_core_error_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_core_error, &id);
  return id;
}
static void
register_gst_library_error (GType* id)
{
  static const GEnumValue values[] = {
    { C_ENUM(GST_LIBRARY_ERROR_FAILED), "GST_LIBRARY_ERROR_FAILED", "failed" },
    { C_ENUM(GST_LIBRARY_ERROR_TOO_LAZY), "GST_LIBRARY_ERROR_TOO_LAZY", "too-lazy" },
    { C_ENUM(GST_LIBRARY_ERROR_INIT), "GST_LIBRARY_ERROR_INIT", "init" },
    { C_ENUM(GST_LIBRARY_ERROR_SHUTDOWN), "GST_LIBRARY_ERROR_SHUTDOWN", "shutdown" },
    { C_ENUM(GST_LIBRARY_ERROR_SETTINGS), "GST_LIBRARY_ERROR_SETTINGS", "settings" },
    { C_ENUM(GST_LIBRARY_ERROR_ENCODE), "GST_LIBRARY_ERROR_ENCODE", "encode" },
    { C_ENUM(GST_LIBRARY_ERROR_NUM_ERRORS), "GST_LIBRARY_ERROR_NUM_ERRORS", "num-errors" },
    { 0, NULL, NULL }
  };
  *id = g_enum_register_static ("GstLibraryError", values);
}
GType
gst_library_error_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_library_error, &id);
  return id;
}
static void
register_gst_resource_error (GType* id)
{
  static const GEnumValue values[] = {
    { C_ENUM(GST_RESOURCE_ERROR_FAILED), "GST_RESOURCE_ERROR_FAILED", "failed" },
    { C_ENUM(GST_RESOURCE_ERROR_TOO_LAZY), "GST_RESOURCE_ERROR_TOO_LAZY", "too-lazy" },
    { C_ENUM(GST_RESOURCE_ERROR_NOT_FOUND), "GST_RESOURCE_ERROR_NOT_FOUND", "not-found" },
    { C_ENUM(GST_RESOURCE_ERROR_BUSY), "GST_RESOURCE_ERROR_BUSY", "busy" },
    { C_ENUM(GST_RESOURCE_ERROR_OPEN_READ), "GST_RESOURCE_ERROR_OPEN_READ", "open-read" },
    { C_ENUM(GST_RESOURCE_ERROR_OPEN_WRITE), "GST_RESOURCE_ERROR_OPEN_WRITE", "open-write" },
    { C_ENUM(GST_RESOURCE_ERROR_OPEN_READ_WRITE), "GST_RESOURCE_ERROR_OPEN_READ_WRITE", "open-read-write" },
    { C_ENUM(GST_RESOURCE_ERROR_CLOSE), "GST_RESOURCE_ERROR_CLOSE", "close" },
    { C_ENUM(GST_RESOURCE_ERROR_READ), "GST_RESOURCE_ERROR_READ", "read" },
    { C_ENUM(GST_RESOURCE_ERROR_WRITE), "GST_RESOURCE_ERROR_WRITE", "write" },
    { C_ENUM(GST_RESOURCE_ERROR_SEEK), "GST_RESOURCE_ERROR_SEEK", "seek" },
    { C_ENUM(GST_RESOURCE_ERROR_SYNC), "GST_RESOURCE_ERROR_SYNC", "sync" },
    { C_ENUM(GST_RESOURCE_ERROR_SETTINGS), "GST_RESOURCE_ERROR_SETTINGS", "settings" },
    { C_ENUM(GST_RESOURCE_ERROR_NO_SPACE_LEFT), "GST_RESOURCE_ERROR_NO_SPACE_LEFT", "no-space-left" },
    { C_ENUM(GST_RESOURCE_ERROR_NUM_ERRORS), "GST_RESOURCE_ERROR_NUM_ERRORS", "num-errors" },
    { 0, NULL, NULL }
  };
  *id = g_enum_register_static ("GstResourceError", values);
}
GType
gst_resource_error_get_type (void)
{
  static GType id;
  static GOnce once = G_ONCE_INIT;

  g_once (&once, (GThreadFunc)register_gst_resource_error, &id);
  return id;
}
static void
register_gst_stream_error (GType* id)
{
  static const GEnumValue values[] = {
    { C_ENUM(GST_STREAM_ERROR_FAILED), "GST_STREAM_ERROR_FAILED", "failed" },
    { C_ENUM(GST_STREAM_ERROR_TOO_LAZY), "G