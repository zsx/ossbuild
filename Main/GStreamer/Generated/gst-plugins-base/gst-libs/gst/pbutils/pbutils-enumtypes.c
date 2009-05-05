


#include "pbutils-enumtypes.h"

/* enumerations from "install-plugins.h" */
GType
gst_install_plugins_return_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_INSTALL_PLUGINS_SUCCESS, "GST_INSTALL_PLUGINS_SUCCESS", "success" },
      { GST_INSTALL_PLUGINS_NOT_FOUND, "GST_INSTALL_PLUGINS_NOT_FOUND", "not-found" },
      { GST_INSTALL_PLUGINS_ERROR, "GST_INSTALL_PLUGINS_ERROR", "error" },
      { GST_INSTALL_PLUGINS_PARTIAL_SUCCESS, "GST_INSTALL_PLUGINS_PARTIAL_SUCCESS", "partial-success" },
      { GST_INSTALL_PLUGINS_USER_ABORT, "GST_INSTALL_PLUGINS_USER_ABORT", "user-abort" },
      { GST_INSTALL_PLUGINS_CRASHED, "GST_INSTALL_PLUGINS_CRASHED", "crashed" },
      { GST_INSTALL_PLUGINS_INVALID, "GST_INSTALL_PLUGINS_INVALID", "invalid" },
      { GST_INSTALL_PLUGINS_STARTED_OK, "GST_INSTALL_PLUGINS_STARTED_OK", "started-ok" },
      { GST_INSTALL_PLUGINS_INTERNAL_FAILURE, "GST_INSTALL_PLUGINS_INTERNAL_FAILURE", "internal-failure" },
      { GST_INSTALL_PLUGINS_HELPER_MISSING, "GST_INSTALL_PLUGINS_HELPER_MISSING", "helper-missing" },
      { GST_INSTALL_PLUGINS_INSTALL_IN_PROGRESS, "GST_INSTALL_PLUGINS_INSTALL_IN_PROGRESS", "install-in-progress" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstInstallPluginsReturn", values);
  }
  return etype;
}



