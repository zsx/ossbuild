


#include "gsttcp-enumtypes.h"

#include "gsttcp.h"

/* enumerations from "gsttcp.h" */
GType
gst_tcp_protocol_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_TCP_PROTOCOL_NONE, "GST_TCP_PROTOCOL_NONE", "none" },
      { GST_TCP_PROTOCOL_GDP, "GST_TCP_PROTOCOL_GDP", "gdp" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstTCPProtocol", values);
  }
  return etype;
}



