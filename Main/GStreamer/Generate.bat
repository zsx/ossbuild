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
set PLUGINSDIR=%GSTDIR%\Plugins

set TOOLSDIR=%TOPDIR%\Tools
set MAINDIR=%TOPDIR%\Main
set SHAREDDIR=%TOPDIR%\Shared

set FLEX_BAT=call "%GENERATEDDIR%\flex.bat"
set BISON_BAT=call "%GENERATEDDIR%\bison.bat"
set MK_ENUMS_BAT=call "%GENERATEDDIR%\mkenum.bat"
set GEN_MARSHAL_BAT=call "%GENERATEDDIR%\genmarshal.bat"
set MK_ENUMS_FS_PREFIX_BAT=call "%GENERATEDDIR%\mkenum-fs-prefix.bat"
set MK_ENUMS_CUSTOM_BAT=call "%GENERATEDDIR%\mkenum-custom.bat"
set GEN_PYTHON_BAT=call "%GENERATEDDIR%\genpython.bat"

set SRC_GSTREAMER_DIR=%SOURCEDIR%\gstreamer
set SRC_GST_PLUGINS_BAD_DIR=%SOURCEDIR%\gst-plugins-bad
set SRC_GST_PLUGINS_BASE_DIR=%SOURCEDIR%\gst-plugins-base
set SRC_GST_PLUGINS_GOOD_DIR=%SOURCEDIR%\gst-plugins-good
set SRC_GST_PLUGINS_FARSIGHT_DIR=%SOURCEDIR%\gst-plugins-farsight
set SRC_FARSIGHT2_DIR=%SOURCEDIR%\farsight2
set SRC_GST_PYTHON_DIR=%SOURCEDIR%\gst-python

set PLUGINS_FARSIGHT2_DIR=%PLUGINSDIR%\Farsight2

set GEN_GSTREAMER_DIR=%GENERATEDDIR%\gstreamer
set GEN_GST_PLUGINS_BAD_DIR=%GENERATEDDIR%\gst-plugins-bad
set GEN_GST_PLUGINS_BASE_DIR=%GENERATEDDIR%\gst-plugins-base
set GEN_GST_PLUGINS_GOOD_DIR=%GENERATEDDIR%\gst-plugins-good
set GEN_GST_PLUGINS_FARSIGHT_DIR=%GENERATEDDIR%\gst-plugins-farsight
set GEN_FARSIGHT2_DIR=%GENERATEDDIR%\farsight2
set GEN_GST_PYTHON_DIR=%GENERATEDDIR%\gst-python


mkdir "%DESTDIR%" 2> NUL

:start
cd /d "%DIR%"


rem gstreamer/win32/common/gstconfig.h
echo "Copying gstconfig.h"
copy "%SRC_GSTREAMER_DIR%\win32\common\gstconfig.h" "%SRC_GSTREAMER_DIR%\gst\gstconfig.h"

rem gstreamer/gst
%GEN_MARSHAL_BAT% gst_marshal "%SRC_GSTREAMER_DIR%\gst\gstmarshal.list" "%GEN_GSTREAMER_DIR%\gst\gstmarshal.h" "%GEN_GSTREAMER_DIR%\gst\gstmarshal.c"
%MK_ENUMS_CUSTOM_BAT% "%SRC_GSTREAMER_DIR%\gst" "%GSTDIR%\Libraries\enumtypes.mkenum.lst.txt" "%GEN_GSTREAMER_DIR%\gst\gstenumtypes.h" "%GEN_GSTREAMER_DIR%\gst\gstenumtypes.c" "#ifndef __GST_ENUM_TYPES_H__\n#define __GST_ENUM_TYPES_H__\n\n#include <glib-object.h>\n\nG_BEGIN_DECLS\n" "\n/* enumerations from \"@filename@\" */\n" "G_END_DECLS\n\n#endif /* __GST_ENUM_TYPES_H__ */" "" "GType @enum_name@_get_type (void);\n#define GST_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" "" "" "" "#include \"gst_private.h\"\n#include <gst/gst.h>\n#define C_ENUM(v) ((gint) v)\n#define C_FLAGS(v) ((guint) v)\n " "\n/* enumerations from \"@filename@\" */" "" "" "static void\nregister_@enum_name@ (GType* id)\n{\n  static const G@Type@Value values[] = {" "    { C_@TYPE@(@VALUENAME@), \"@VALUENAME@\", \"@valuenick@\" }," "    { 0, NULL, NULL }\n  };\n  *id = g_@type@_register_static (\"@EnumName@\", values);\n}\nGType\n@enum_name@_get_type (void)\n{\n  static GType id;\n  static GOnce once = G_ONCE_INIT;\n\n  g_once (&once, (GThreadFunc)register_@enum_name@, &id);\n  return id;\n}\n" ""

rem gstreamer/gst/parse
%BISON_BAT% _gst_parse_yy "%SRC_GSTREAMER_DIR%\gst\parse\grammar.y" "%GEN_GSTREAMER_DIR%\gst\parse\grammar.tab.c"
%FLEX_BAT% _gst_parse_yy "%SRC_GSTREAMER_DIR%\gst\parse\parse.l" "%GEN_GSTREAMER_DIR%\gst\parse\parse.c"


rem gst-plugins-farsight/gst/rtpdemux
%GEN_MARSHAL_BAT% gst_rtp_demux_marshal "%SRC_GST_PLUGINS_FARSIGHT_DIR%\gst\rtpdemux\gstrtpdemux-marshal.list" "%GEN_GST_PLUGINS_FARSIGHT_DIR%\gst\rtpdemux\gstrtpdemux-marshal.h" "%GEN_GST_PLUGINS_FARSIGHT_DIR%\gst\rtpdemux\gstrtpdemux-marshal.c"

rem gst-plugins-farsight/gst/rtpjitterbuffer
%GEN_MARSHAL_BAT% gstrtpjitterbuffer_marshal "%SRC_GST_PLUGINS_FARSIGHT_DIR%\gst\rtpjitterbuffer\gstrtpjitterbuffer-marshal.list" "%GEN_GST_PLUGINS_FARSIGHT_DIR%\gst\rtpjitterbuffer\gstrtpjitterbuffer-marshal.h" "%GEN_GST_PLUGINS_FARSIGHT_DIR%\gst\rtpjitterbuffer\gstrtpjitterbuffer-marshal.c"


rem gst-plugins-base/gst/tcp
%GEN_MARSHAL_BAT% gst_tcp_marshal "%SRC_GST_PLUGINS_BASE_DIR%\gst\tcp\gsttcp-marshal.list" "%GEN_GST_PLUGINS_BASE_DIR%\gst\tcp\gsttcp-marshal.h" "%GEN_GST_PLUGINS_BASE_DIR%\gst\tcp\gsttcp-marshal.c"

rem gst-plugins-base/gst/playback
%GEN_MARSHAL_BAT% gst_play_marshal "%SRC_GST_PLUGINS_BASE_DIR%\gst\playback\gstplay-marshal.list" "%GEN_GST_PLUGINS_BASE_DIR%\gst\playback\gstplay-marshal.h" "%GEN_GST_PLUGINS_BASE_DIR%\gst\playback\gstplay-marshal.c"

rem gst-plugins-base/gst-libs/gst/app
%GEN_MARSHAL_BAT% __gst_app_marshal "%SRC_GST_PLUGINS_BASE_DIR%\gst-libs\gst\app\gstapp-marshal.list" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\app\gstapp-marshal.h" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\app\gstapp-marshal.c"

rem gst-plugins-base/gst-libs/gst/audio
%MK_ENUMS_BAT% AUDIO "%SRC_GST_PLUGINS_BASE_DIR%\gst-libs\gst\audio" "%GSTDIR%\Plugins\Base\gst-libs\audio.mkenum.lst.txt" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\audio\audio-enumtypes.h" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\audio\audio-enumtypes.c"

rem gst-plugins-base/gst-libs/gst/interfaces
%MK_ENUMS_BAT% INTERFACES "%SRC_GST_PLUGINS_BASE_DIR%\gst-libs\gst\interfaces" "%GSTDIR%\Plugins\Base\gst-libs\interfaces.mkenum.lst.txt" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\interfaces\interfaces-enumtypes.h" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\interfaces\interfaces-enumtypes.c"
%GEN_MARSHAL_BAT% gst_interfaces_marshal "%SRC_GST_PLUGINS_BASE_DIR%\gst-libs\gst\interfaces\interfaces-marshal.list" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\interfaces\interfaces-marshal.h" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\interfaces\interfaces-marshal.c"

rem gst-plugins-base/gst-libs/gst/pbutils
%MK_ENUMS_BAT% INSTALL "%SRC_GST_PLUGINS_BASE_DIR%\gst-libs\gst\pbutils" "%GSTDIR%\Plugins\Base\gst-libs\pbutils.mkenum.lst.txt" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\pbutils\pbutils-enumtypes.h" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\pbutils\pbutils-enumtypes.c"

rem gst-plugins-base/gst-libs/gst/rtsp
%MK_ENUMS_BAT% RTSP "%SRC_GST_PLUGINS_BASE_DIR%\gst-libs\gst\rtsp" "%GSTDIR%\Plugins\Base\gst-libs\rtsp.mkenum.lst.txt" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\rtsp\rtsp-enumtypes.h" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\rtsp\rtsp-enumtypes.c"
%GEN_MARSHAL_BAT% gst_rtsp_marshal "%SRC_GST_PLUGINS_BASE_DIR%\gst-libs\gst\rtsp\rtsp-marshal.list" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\rtsp\rtsp-marshal.h" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\rtsp\rtsp-marshal.c"

rem gst-plugins-base/gst-libs/gst/video
%MK_ENUMS_BAT% VIDEO "%SRC_GST_PLUGINS_BASE_DIR%\gst-libs\gst\video" "%GSTDIR%\Plugins\Base\gst-libs\video.mkenum.lst.txt" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\video\video-enumtypes.h" "%GEN_GST_PLUGINS_BASE_DIR%\gst-libs\gst\video\video-enumtypes.c"


rem gst-plugins-good/gst/udp
%MK_ENUMS_BAT% UDP "%SRC_GST_PLUGINS_GOOD_DIR%\gst\udp" "%GSTDIR%\Plugins\Good\gst\udp.mkenum.lst.txt" "%GEN_GST_PLUGINS_GOOD_DIR%\gst\udp\gstudp-enumtypes.h" "%GEN_GST_PLUGINS_GOOD_DIR%\gst\udp\gstudp-enumtypes.c"
%GEN_MARSHAL_BAT% gst_udp_marshal "%SRC_GST_PLUGINS_GOOD_DIR%\gst\udp\gstudp-marshal.list" "%GEN_GST_PLUGINS_GOOD_DIR%\gst\udp\gstudp-marshal.h" "%GEN_GST_PLUGINS_GOOD_DIR%\gst\udp\gstudp-marshal.c"


rem gst-plugins-bad/gst-libs/gst/interfaces
%MK_ENUMS_BAT% PHOTOGRAPHY "%SRC_GST_PLUGINS_BAD_DIR%\gst-libs\gst\interfaces" "%GSTDIR%\Plugins\Bad\gst-libs\photography.mkenum.lst.txt" "%GEN_GST_PLUGINS_BAD_DIR%\gst-libs\gst\interfaces\photography-enumtypes.h" "%GEN_GST_PLUGINS_BAD_DIR%\gst-libs\gst\interfaces\photography-enumtypes.c"

rem gst-plugins-bad/gst/camerabin
%GEN_MARSHAL_BAT% __gst_camerabin_marshal "%SRC_GST_PLUGINS_BAD_DIR%\gst\camerabin\gstcamerabin-marshal.list" "%GEN_GST_PLUGINS_BAD_DIR%\gst\camerabin\gstcamerabin-marshal.h" "%GEN_GST_PLUGINS_BAD_DIR%\gst\camerabin\gstcamerabin-marshal.c"

rem gst-plugins-bad/gst/rtpmanager
%GEN_MARSHAL_BAT% gst_rtp_bin_marshal "%SRC_GST_PLUGINS_BAD_DIR%\gst\rtpmanager\gstrtpbin-marshal.list" "%GEN_GST_PLUGINS_BAD_DIR%\gst\rtpmanager\gstrtpbin-marshal.h" "%GEN_GST_PLUGINS_BAD_DIR%\gst\rtpmanager\gstrtpbin-marshal.c"

rem gst-plugins-bad/gst/selector
%GEN_MARSHAL_BAT% gst_selector_marshal "%SRC_GST_PLUGINS_BAD_DIR%\gst\selector\gstselector-marshal.list" "%GEN_GST_PLUGINS_BAD_DIR%\gst\selector\gstselector-marshal.h" "%GEN_GST_PLUGINS_BAD_DIR%\gst\selector\gstselector-marshal.c"


rem farsight2/gst/fsrtpconference
%GEN_MARSHAL_BAT% _fs_rtp_marshal "%PLUGINS_FARSIGHT2_DIR%\gst\fs-rtp-marshal.list" "%GEN_FARSIGHT2_DIR%\gst\fsrtpconference\fs-rtp-marshal.h" "%GEN_FARSIGHT2_DIR%\gst\fsrtpconference\fs-rtp-marshal.c"


rem farsight2/gst-libs/farsight
echo "%PLUGINS_FARSIGHT2_DIR%\gst-libs\farsight-mkenum.list.txt" 
%MK_ENUMS_FS_PREFIX_BAT% FS "%SRC_FARSIGHT2_DIR%\gst-libs\gst\farsight" "%PLUGINS_FARSIGHT2_DIR%\gst-libs\farsight-mkenum.list.txt" "%GEN_FARSIGHT2_DIR%\gst-libs\farsight\fs-enum-types.h" "%GEN_FARSIGHT2_DIR%\gst-libs\farsight\fs-enum-types.c"
%GEN_MARSHAL_BAT% _fs_marshal "%PLUGINS_FARSIGHT2_DIR%\gst-libs\fs-marshal.list" "%GEN_FARSIGHT2_DIR%\gst-libs\farsight\fs-marshal.h" "%GEN_FARSIGHT2_DIR%\gst-libs\farsight\fs-marshal.c"


rem farsight2/transmitters/rawup
%GEN_MARSHAL_BAT% _fs_rawudp_marshal "%PLUGINS_FARSIGHT2_DIR%\transmitters\fs-rawudp-marshal.list" "%GEN_FARSIGHT2_DIR%\transmitters\rawudp\fs-rawudp-marshal.h" "%GEN_FARSIGHT2_DIR%\transmitters\rawudp\fs-rawudp-marshal.c"

rem farsight2/config.h.in
sed.exe -e "s/@GETTEXT_PACKAGE@/farsight2/g" -e "s/@PACKAGE@/farsight2/g" -e "s/@PACKAGE_NAME@/Farsight2/g" -e "s/@VERSION@/0.0.9/g" -e "s/@GST_MAJORMINOR@/0.10/g" -e "s/@HOST_CPU@/x86_32/g"  "%PLUGINS_FARSIGHT2_DIR%\config.h.in" > "%GEN_FARSIGHT2_DIR%\config.h"

rem gst-python/pygst.py.in
echo "Generating gst-python/pygst.py.in
sed.exe -e "s/'@GST_MAJORMINOR@'/\"0.10\"/g" -e "s/_pygst_dir = '@PYGSTDIR@'//g"  "%SRC_GST_PYTHON_DIR%\pygst.py.in" > "%GEN_GST_PYTHON_DIR%\pygst.py"

rem Copying python generated files
echo "Copying %GSTDIR%\Bindings\Python\gstversion.override %SRC_GST_PYTHON_DIR%\gst"
copy "%GSTDIR%\Bindings\Python\gstversion.override" "%SRC_GST_PYTHON_DIR%\gst"
 
rem gst-python/gst/gst
%GEN_PYTHON_BAT% "%SRC_GST_PYTHON_DIR%" "%GEN_GST_PYTHON_DIR%" gst

rem gst-python/gst/audio
%GEN_PYTHON_BAT% "%SRC_GST_PYTHON_DIR%" "%GEN_GST_PYTHON_DIR%" audio

rem gst-python/gst/video
%GEN_PYTHON_BAT% "%SRC_GST_PYTHON_DIR%" "%GEN_GST_PYTHON_DIR%" video

rem gst-python/gst/tag
%GEN_PYTHON_BAT% "%SRC_GST_PYTHON_DIR%" "%GEN_GST_PYTHON_DIR%" tag

rem gst-python/gst/interfaces
%GEN_PYTHON_BAT% "%SRC_GST_PYTHON_DIR%" "%GEN_GST_PYTHON_DIR%" interfaces

rem gst-python/gst/gst
%GEN_PYTHON_BAT% "%SRC_GST_PYTHON_DIR%" "%GEN_GST_PYTHON_DIR%" pbutils





goto exit

:exit
set PATH=%ORIGPATH%
cd /d "%CURR%"