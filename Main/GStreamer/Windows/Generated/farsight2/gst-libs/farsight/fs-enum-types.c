


#include "fs-enum-types.h"

#include "fs-candidate.h"
#include "fs-codec.h"
#include "fs-participant.h"
#include "fs-session.h"
#include "fs-stream.h"
#include "fs-conference-iface.h"

/* enumerations from "fs-candidate.h" */
GType
fs_candidate_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { FS_CANDIDATE_TYPE_HOST, "FS_CANDIDATE_TYPE_HOST", "host" },
      { FS_CANDIDATE_TYPE_SRFLX, "FS_CANDIDATE_TYPE_SRFLX", "srflx" },
      { FS_CANDIDATE_TYPE_PRFLX, "FS_CANDIDATE_TYPE_PRFLX", "prflx" },
      { FS_CANDIDATE_TYPE_RELAY, "FS_CANDIDATE_TYPE_RELAY", "relay" },
      { FS_CANDIDATE_TYPE_MULTICAST, "FS_CANDIDATE_TYPE_MULTICAST", "multicast" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("FsCandidateType", values);
  }
  return etype;
}
GType
fs_network_protocol_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { FS_NETWORK_PROTOCOL_UDP, "FS_NETWORK_PROTOCOL_UDP", "udp" },
      { FS_NETWORK_PROTOCOL_TCP, "FS_NETWORK_PROTOCOL_TCP", "tcp" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("FsNetworkProtocol", values);
  }
  return etype;
}
GType
fs_component_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { FS_COMPONENT_NONE, "FS_COMPONENT_NONE", "none" },
      { FS_COMPONENT_RTP, "FS_COMPONENT_RTP", "rtp" },
      { FS_COMPONENT_RTCP, "FS_COMPONENT_RTCP", "rtcp" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("FsComponentType", values);
  }
  return etype;
}

/* enumerations from "fs-codec.h" */
GType
fs_media_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { FS_MEDIA_TYPE_AUDIO, "FS_MEDIA_TYPE_AUDIO", "audio" },
      { FS_MEDIA_TYPE_VIDEO, "FS_MEDIA_TYPE_VIDEO", "video" },
      { FS_MEDIA_TYPE_LAST, "FS_MEDIA_TYPE_LAST", "last" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("FsMediaType", values);
  }
  return etype;
}

/* enumerations from "fs-session.h" */
GType
fs_dtmf_event_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { FS_DTMF_EVENT_0, "FS_DTMF_EVENT_0", "0" },
      { FS_DTMF_EVENT_1, "FS_DTMF_EVENT_1", "1" },
      { FS_DTMF_EVENT_2, "FS_DTMF_EVENT_2", "2" },
      { FS_DTMF_EVENT_3, "FS_DTMF_EVENT_3", "3" },
      { FS_DTMF_EVENT_4, "FS_DTMF_EVENT_4", "4" },
      { FS_DTMF_EVENT_5, "FS_DTMF_EVENT_5", "5" },
      { FS_DTMF_EVENT_6, "FS_DTMF_EVENT_6", "6" },
      { FS_DTMF_EVENT_7, "FS_DTMF_EVENT_7", "7" },
      { FS_DTMF_EVENT_8, "FS_DTMF_EVENT_8", "8" },
      { FS_DTMF_EVENT_9, "FS_DTMF_EVENT_9", "9" },
      { FS_DTMF_EVENT_STAR, "FS_DTMF_EVENT_STAR", "star" },
      { FS_DTMF_EVENT_POUND, "FS_DTMF_EVENT_POUND", "pound" },
      { FS_DTMF_EVENT_A, "FS_DTMF_EVENT_A", "a" },
      { FS_DTMF_EVENT_B, "FS_DTMF_EVENT_B", "b" },
      { FS_DTMF_EVENT_C, "FS_DTMF_EVENT_C", "c" },
      { FS_DTMF_EVENT_D, "FS_DTMF_EVENT_D", "d" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("FsDTMFEvent", values);
  }
  return etype;
}
GType
fs_dtmf_method_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { FS_DTMF_METHOD_AUTO, "FS_DTMF_METHOD_AUTO", "auto" },
      { FS_DTMF_METHOD_RTP_RFC4733, "FS_DTMF_METHOD_RTP_RFC4733", "rtp-rfc4733" },
      { FS_DTMF_METHOD_IN_BAND, "FS_DTMF_METHOD_IN_BAND", "in-band" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("FsDTMFMethod", values);
  }
  return etype;
}

/* enumerations from "fs-stream.h" */
GType
fs_stream_direction_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { FS_DIRECTION_NONE, "FS_DIRECTION_NONE", "none" },
      { FS_DIRECTION_SEND, "FS_DIRECTION_SEND", "send" },
      { FS_DIRECTION_RECV, "FS_DIRECTION_RECV", "recv" },
      { FS_DIRECTION_BOTH, "FS_DIRECTION_BOTH", "both" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("FsStreamDirection", values);
  }
  return etype;
}
GType
fs_stream_state_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { FS_STREAM_STATE_FAILED, "FS_STREAM_STATE_FAILED", "failed" },
      { FS_STREAM_STATE_DISCONNECTED, "FS_STREAM_STATE_DISCONNECTED", "disconnected" },
      { FS_STREAM_STATE_GATHERING, "FS_STREAM_STATE_GATHERING", "gathering" },
      { FS_STREAM_STATE_CONNECTING, "FS_STREAM_STATE_CONNECTING", "connecting" },
      { FS_STREAM_STATE_CONNECTED, "FS_STREAM_STATE_CONNECTED", "connected" },
      { FS_STREAM_STATE_READY, "FS_STREAM_STATE_READY", "ready" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("FsStreamState", values);
  }
  return etype;
}

/* enumerations from "fs-conference-iface.h" */
GType
fs_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { FS_ERROR_CONSTRUCTION, "FS_ERROR_CONSTRUCTION", "construction" },
      { FS_ERROR_INTERNAL, "FS_ERROR_INTERNAL", "internal" },
      { FS_ERROR_INVALID_ARGUMENTS, "FS_ERROR_INVALID_ARGUMENTS", "invalid-arguments" },
      { FS_ERROR_NETWORK, "FS_ERROR_NETWORK", "network" },
      { FS_ERROR_NOT_IMPLEMENTED, "FS_ERROR_NOT_IMPLEMENTED", "not-implemented" },
      { FS_ERROR_NEGOTIATION_FAILED, "FS_ERROR_NEGOTIATION_FAILED", "negotiation-failed" },
      { FS_ERROR_UNKNOWN_CODEC, "FS_ERROR_UNKNOWN_CODEC", "unknown-codec" },
      { FS_ERROR_UNKNOWN_CNAME, "FS_ERROR_UNKNOWN_CNAME", "unknown-cname" },
      { FS_ERROR_NO_CODECS, "FS_ERROR_NO_CODECS", "no-codecs" },
      { FS_ERROR_NO_CODECS_LEFT, "FS_ERROR_NO_CODECS_LEFT", "no-codecs-left" },
      { FS_ERROR_CONNECTION_FAILED, "FS_ERROR_CONNECTION_FAILED", "connection-failed" },
      { FS_ERROR_DISPOSED, "FS_ERROR_DISPOSED", "disposed" },
      { FS_ERROR_ALREADY_EXISTS, "FS_ERROR_ALREADY_EXISTS", "already-exists" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("FsError", values);
  }
  return etype;
}



