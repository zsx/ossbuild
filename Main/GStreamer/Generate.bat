@echo off

REM **********************************************************************
REM * Some of the source code that gstreamer uses is auto-generated 
REM * through the use of bison/flex and the glib tools.
REM * 
REM * Unfortunately, some of these tools and their dependencies only 
REM * work in certain versions of Windows. As a consequence, we must 
REM * generate them ahead of time and include them here.
REM * 
REM * This script is just here to make generating those files a bit 
REM * easier for us.
REM **********************************************************************

set ORIGPATH=%PATH%
set CURR=%CD%
set DIR=%~dp0.
set TOPDIR=%DIR%\..\..
set GSTDIR=%DIR%
set SOURCEDIR=%GSTDIR%\Source
set GENERATEDDIR=%GSTDIR%\Generated

set TOOLSDIR=%TOPDIR%\Tools
set MAINDIR=%TOPDIR%\Main
set SHAREDDIR=%TOPDIR%\Shared

set FLEX_BAT=call "%GENERATEDDIR%\flex.bat"
set BISON_BAT=call "%GENERATEDDIR%\bison.bat"
set MK_ENUMS_BAT=call "%GENERATEDDIR%\mkenum.bat"
set GEN_MARSHAL_BAT=call "%GENERATEDDIR%\genmarshal.bat"
set MK_ENUMS_CUSTOM_BAT=call "%GENERATEDDIR%\mkenum-custom.bat"

set SRC_GSTREAMER_DIR=%SOURCEDIR%\gstreamer
set SRC_GST_PLUGINS_BASE_DIR=%SOURCEDIR%\gst-plugins-base
set SRC_GST_PLUGINS_FARSIGHT_DIR=%SOURCEDIR%\gst-plugins-farsight

set GEN_GSTREAMER_DIR=%GENERATEDDIR%\gstreamer
set GEN_GST_PLUGINS_BASE_DIR=%GENERATEDDIR%\gst-plugins-base
set GEN_GST_PLUGINS_FARSIGHT_DIR=%GENERATEDDIR%\gst-plugins-farsight


mkdir "%DESTDIR%" 2> NUL

:start
cd /d "%DIR%"


rem %MK_ENUMS% COMMON "%SRC_GSTREAMER_DIR%\gst" "%GSTDIR%\Common\common.mkenum.lst.txt" "%GEN_GSTREAMER_DIR%\gst\gstenumtypes.h" "%GEN_GSTREAMER_DIR%\gst\gstenumtypes.c"

rem gstreamer/gst
%GEN_MARSHAL_BAT% gst_marshal "%SRC_GSTREAMER_DIR%\gst\gstmarshal.list" "%GEN_GSTREAMER_DIR%\gstmarshal.h" "%GEN_GSTREAMER_DIR%\gstmarshal.c"
%MK_ENUMS_CUSTOM_BAT% "%SRC_GSTREAMER_DIR%\gst" "%GSTDIR%\Common\common.mkenum.lst.txt" "%GEN_GSTREAMER_DIR%\gst\gstenumtypes.h" "%GEN_GSTREAMER_DIR%\gst\gstenumtypes.c" "#ifndef __GST_ENUM_TYPES_H__\n#define __GST_ENUM_TYPES_H__\n\n#include <glib-object.h>\n\nG_BEGIN_DECLS\n" "\n/* enumerations from \"@filename@\" */\n" "G_END_DECLS\n\n#endif /* __GST_ENUM_TYPES_H__ */" "" "GType @enum_name@_get_type (void);\n#define GST_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" "" "" "" "#include \"gst_private.h\"\n#include <gst/gst.h>\n#define C_ENUM(v) ((gint) v)\n#define C_FLAGS(v) ((guint) v)\n " "\n/* enumerations from \"@filename@\" */" "" "" "static void\nregister_@enum_name@ (GType* id)\n{\n  static const G@Type@Value values[] = {" "    { C_@TYPE@(@VALUENAME@), \"@VALUENAME@\", \"@valuenick@\" }," "    { 0, NULL, NULL }\n  };\n  *id = g_@type@_register_static (\"@EnumName@\", values);\n}\nGType\n@enum_name@_get_type (void)\n{\n  static GType id;\n  static GOnce once = G_ONCE_INIT;\n\n  g_once (&once, (GThreadFunc)register_@enum_name@, &id);\n  return id;\n}\n" ""

rem gstreamer/gst/parse
%BISON_BAT% _gst_parse_yy "%SRC_GSTREAMER_DIR%\gst\parse\grammar.y" "%GEN_GSTREAMER_DIR%\gst\parse\grammar.tab.c"
%FLEX_BAT% _gst_parse_yy "%SRC_GSTREAMER_DIR%\gst\parse\parse.l" "%GEN_GSTREAMER_DIR%\gst\parse\parse.c"


rem gst-plugins-base/gst/tcp
%GEN_MARSHAL_BAT% gst_tcp_marshal "%SRC_GST_PLUGINS_BASE_DIR%\gst\tcp\gsttcp-marshal.list" "%GEN_GST_PLUGINS_BASE_DIR%\gst\tcp\gsttcp-marshal.h" "%GEN_GST_PLUGINS_BASE_DIR%\gst\tcp\gsttcp-marshal.c"

rem gst-plugins-base/gst/playback
%GEN_MARSHAL_BAT% gst_play_marshal "%SRC_GST_PLUGINS_BASE_DIR%\gst\playback\gstplay-marshal.list" "%GEN_GST_PLUGINS_BASE_DIR%\gst\playback\gstplay-marshal.h" "%GEN_GST_PLUGINS_BASE_DIR%\gst\playback\gstplay-marshal.c"


rem gst-plugins-farsight/gst/rtpdemux
%GEN_MARSHAL_BAT% gst_rtp_demux_marshal "%SRC_GST_PLUGINS_FARSIGHT_DIR%\gst\rtpdemux\gstrtpdemux-marshal.list" "%GEN_GST_PLUGINS_FARSIGHT_DIR%\gst\rtpdemux\gstrtpdemux-marshal.h" "%GEN_GST_PLUGINS_FARSIGHT_DIR%\gst\rtpdemux\gstrtpdemux-marshal.c"

rem gst-plugins-farsight/gst/rtpjitterbuffer
%GEN_MARSHAL_BAT% gstrtpjitterbuffer_marshal "%SRC_GST_PLUGINS_FARSIGHT_DIR%\gst\rtpjitterbuffer\gstrtpjitterbuffer-marshal.list" "%GEN_GST_PLUGINS_FARSIGHT_DIR%\gst\rtpjitterbuffer\gstrtpjitterbuffer-marshal.h" "%GEN_GST_PLUGINS_FARSIGHT_DIR%\gst\rtpjitterbuffer\gstrtpjitterbuffer-marshal.c"



goto exit

:exit
set PATH=%ORIGPATH%
cd /d "%CURR%"