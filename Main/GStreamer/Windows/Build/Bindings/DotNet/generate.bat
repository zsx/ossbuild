@echo off

REM **********************************************************************
REM * ...
REM **********************************************************************

echo Generating gstreamer-sharp files...


set GSTREAMERSHARP_API_VERSION=0.9.2


set ORIGPATH=%PATH%
set CURR=%CD%
set DIR=%~dp0.
set TOPDIR=%DIR%\..\..\..\..\..\..
set GSTDIR=%DIR%\..\..\..\..\
set SOURCEDIR=%GSTDIR%\Source
set GENERATEDDIR=%GSTDIR%\Windows\Generated
set PLUGINSDIR=%GSTDIR%\Windows\Build\Plugins
set WINDOWSBUILDDIR=%GSTDIR%\Windows\Build

set BUILDDIR=%TOPDIR%\Build
set TOOLSDIR=%TOPDIR%\Tools
set MAINDIR=%TOPDIR%\Main
set SHAREDDIR=%TOPDIR%\Shared

set PATH=%TOOLSDIR%;%PATH%

set BINDIR=%BUILDDIR%\Windows\Win32\Release\bin
set OBJDIR=%BUILDDIR%\Windows\Win32\Release\obj

set SHAREDBINDIR=%SHAREDDIR%\Build\Windows\Win32\bin

set DOTNET_BIN_DIR=%BINDIR%\gstreamer\bindings\dotnet
set DOTNET_OBJ_DIR=%OBJDIR%\gstreamer\bindings\dotnet
set DOTNET_OBJ_SRC_DIR=%DOTNET_OBJ_DIR%\source
set DOTNET_OBJ_GSTREAMER_SHARP_DIR=%DOTNET_OBJ_DIR%\gstreamer-sharp

set GSTREAMER_SHARP_GENERATED_DIR=%GENERATEDDIR%\gstreamer-sharp

set GST_USER_CACHE_DIR=%USERPROFILE%\.gstreamer-0.10

set ELEMENT_GEN="%DOTNET_BIN_DIR%\element-gen.exe"
set GST_GAPI_FIXUP="%DOTNET_BIN_DIR%\gst-gapi-fixup.exe"
set GST_GAPI_PARSER="%DOTNET_BIN_DIR%\gst-gapi-parser.exe"
set GST_GAPI_CODEGEN="%DOTNET_BIN_DIR%\gst-gapi_codegen.exe"
set GST_GENERATE_TAGS="%DOTNET_BIN_DIR%\gst-generate-tags.exe"

set GST_XMLINSPECT="%BINDIR%\gst-xmlinspect.exe" "--gst-plugin-path=%BINDIR%\plugins"

set GSTREAMERSHARP_API=gstreamer-api.xml
set GSTREAMERSHARP_API_RAW=gstreamer-api.raw
set GSTREAMERSHARP_METADATA=Gstreamer.metadata
set GSTREAMERSHARP_SYMBOLS=gstreamer-symbols.xml

set GSTREAMERSHARP_SRC_DIR=%SOURCEDIR%\gstreamer-sharp
set GSTREAMERSHARP_API_PATH=%DOTNET_OBJ_DIR%\%GSTREAMERSHARP_API%
set GSTREAMERSHARP_SYMBOLS_PATH=%GSTREAMERSHARP_SRC_DIR%\gstreamer-sharp\%GSTREAMERSHARP_SYMBOLS%
set GSTREAMERSHARP_API_RAW_PATH=%GSTREAMERSHARP_SRC_DIR%\gstreamer-sharp\%GSTREAMERSHARP_API_RAW%
set GSTREAMERSHARP_METADATA_PATH=%GSTREAMERSHARP_SRC_DIR%\gstreamer-sharp\%GSTREAMERSHARP_METADATA%

set GSTREAMERSHARP_INTERFACES_DIR=%GSTREAMERSHARP_SRC_DIR%\elementgen\interfaces

set GSTREAMERSHARP_API_VERSION_REGEX=%GSTREAMERSHARP_API_VERSION:.=\.%


mkdir "%DOTNETBINDIR%" > NUL 2>&1
mkdir "%DOTNETOBJDIR%" > NUL 2>&1
mkdir "%DOTNETOBJSRCDIR%" > NUL 2>&1
mkdir "%DOTNET_OBJ_GSTREAMER_SHARP_DIR%\coreplugins\inspect" > NUL 2>&1
mkdir "%DOTNET_OBJ_GSTREAMER_SHARP_DIR%\baseplugins\inspect" > NUL 2>&1

REM Copy over all the shared libraries that we might need to run gst-inspect
if not exist "%BINDIR%\iconv.dll" (
	copy /Y "%SHAREDBINDIR%\*.dll" "%BINDIR%\"
)

REM Take out the cached gst registry if it exists
if exist "%GST_USER_CACHE_DIR%\registry.i686.bin" (
	del /F /Q "%GST_USER_CACHE_DIR%\registry.i686.bin"
)

cd "%GSTREAMERSHARP_SRC_DIR%\source"




REM There's a bug in gstreamer-sharp/parser/gapi2xml.pl where it doesn't recognize a method (record_toggled) in gst-plugins-base\gst-libs\gst\interfaces\mixer.h
REM Because of that, we can't auto-generate gstreamer-api.raw as we should. And our auto-generated versions are missing some vital interfaces as a result of the bug.
REM So we have to use the provided one.
REM
REM echo Generating gstreamer-api.raw...
REM %GST_GAPI_PARSER% "gstreamer-sharp-source.xml" > NUL 2>&1

echo Generating gstreamer-api.xml...
copy "%GSTREAMERSHARP_API_RAW_PATH%" "%GSTREAMERSHARP_API_PATH%" > NUL 2>&1
%GST_GAPI_FIXUP% "--api=%GSTREAMERSHARP_API_PATH%" "--metadata=%GSTREAMERSHARP_METADATA_PATH%" "--symbols=%GSTREAMERSHARP_SYMBOLS_PATH%" > NUL 2>&1


echo Generating API code...
%GST_GAPI_CODEGEN% --generate "%GSTREAMERSHARP_API_PATH%" "--outdir=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp" "--customdir=%GSTREAMERSHARP_SRC_DIR%\gstreamer-sharp" --assembly-name=gstreamer-sharp --gluelib-name=gstreamersharpglue-0.10.dll "--glue-filename=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\glue\generated.c" "--glue-includes=gst/gst.h gst/base/gstcollectpads.h gst/interfaces/colorbalance.h gst/interfaces/colorbalancechannel.h gst/interfaces/tuner.h gst/interfaces/tunerchannel.h gst/interfaces/tunernorm.h gst/cdda/gstcddabasesrc.h"
copy "%GSTREAMERSHARP_SRC_DIR%\gstreamer-sharp\override\*" "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp"
REM TO-DO: Run sed command to change "public" to "internal" for ObjectManager class

REM Base plugins
REM ximagesink xvimagesink decodebin2 playbin2
REM ximagesink, xvimagesink are noticeably absent -- they don't work in the Windows version

echo Generating base plugins...
cd "%DOTNET_OBJ_GSTREAMER_SHARP_DIR%\baseplugins\inspect"
%GST_XMLINSPECT% playbin2   > playbin2.raw
%GST_XMLINSPECT% decodebin2 > decodebin2.raw

copy "playbin2.raw"   "playbin2.xml"   > NUL 2>&1
copy "decodebin2.raw" "decodebin2.xml" > NUL 2>&1

%GST_GAPI_FIXUP% "--api=playbin2.xml"   "--metadata=%GSTREAMERSHARP_SRC_DIR%\gstreamer-sharp\baseplugins\playbin2.metadata"
%GST_GAPI_FIXUP% "--api=decodebin2.xml" "--metadata=%GSTREAMERSHARP_SRC_DIR%\gstreamer-sharp\baseplugins\decodebin2.metadata"

%ELEMENT_GEN% --input=playbin2.xml   "--customfile=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\baseplugins\playbin2.cs"   --namespace=Gst.BasePlugins "--api=%GSTREAMERSHARP_API_PATH%" "--interfacesdir=%GSTREAMERSHARP_INTERFACES_DIR%" > "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\baseplugins\playbin2.cs"
%ELEMENT_GEN% --input=decodebin2.xml "--customfile=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\baseplugins\decodebin2.cs" --namespace=Gst.BasePlugins "--api=%GSTREAMERSHARP_API_PATH%" "--interfacesdir=%GSTREAMERSHARP_INTERFACES_DIR%" > "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\baseplugins\decodebin2.cs"


REM Core plugins
REM capsfilter fakesrc fakesink fdsrc fdsink filesrc filesink identity queue tee typefind multiqueue
REM fdsrc and fdsink are noticeably absent -- they're not existent in the Windows version

echo Generating core plugins...
cd "%DOTNET_OBJ_GSTREAMER_SHARP_DIR%\coreplugins\inspect"
%GST_XMLINSPECT% capsfilter > capsfilter.raw
%GST_XMLINSPECT% fakesrc    > fakesrc.raw
%GST_XMLINSPECT% fakesink   > fakesink.raw
%GST_XMLINSPECT% filesrc    > filesrc.raw
%GST_XMLINSPECT% filesink   > filesink.raw
%GST_XMLINSPECT% identity   > identity.raw
%GST_XMLINSPECT% queue      > queue.raw
%GST_XMLINSPECT% tee        > tee.raw
%GST_XMLINSPECT% typefind   > typefind.raw
%GST_XMLINSPECT% multiqueue > multiqueue.raw

copy "capsfilter.raw" "capsfilter.xml" > NUL 2>&1
copy "fakesrc.raw"    "fakesrc.xml"    > NUL 2>&1
copy "fakesink.raw"   "fakesink.xml"   > NUL 2>&1
copy "filesrc.raw"    "filesrc.xml"    > NUL 2>&1
copy "filesink.raw"   "filesink.xml"   > NUL 2>&1
copy "identity.raw"   "identity.xml"   > NUL 2>&1
copy "queue.raw"      "queue.xml"      > NUL 2>&1
copy "tee.raw"        "tee.xml"        > NUL 2>&1
copy "typefind.raw"   "typefind.xml"   > NUL 2>&1
copy "multiqueue.raw" "multiqueue.xml" > NUL 2>&1

%GST_GAPI_FIXUP% "--api=capsfilter.xml" "--metadata=capsfilter.xml"
%GST_GAPI_FIXUP% "--api=fakesrc.xml"    "--metadata=%GSTREAMERSHARP_SRC_DIR%\gstreamer-sharp\coreplugins\fakesrc.metadata"
%GST_GAPI_FIXUP% "--api=fakesink.xml"   "--metadata=%GSTREAMERSHARP_SRC_DIR%\gstreamer-sharp\coreplugins\fakesink.metadata"
%GST_GAPI_FIXUP% "--api=filesrc.xml"    "--metadata=filesrc.xml"
%GST_GAPI_FIXUP% "--api=filesink.xml"   "--metadata=filesink.xml"
%GST_GAPI_FIXUP% "--api=identity.xml"   "--metadata=%GSTREAMERSHARP_SRC_DIR%\gstreamer-sharp\coreplugins\identity.metadata"
%GST_GAPI_FIXUP% "--api=queue.xml"      "--metadata=queue.xml"
%GST_GAPI_FIXUP% "--api=tee.xml"        "--metadata=tee.xml"
%GST_GAPI_FIXUP% "--api=typefind.xml"   "--metadata=%GSTREAMERSHARP_SRC_DIR%\gstreamer-sharp\coreplugins\typefind.metadata"
%GST_GAPI_FIXUP% "--api=multiqueue.xml" "--metadata=multiqueue.xml"

%ELEMENT_GEN% --input=capsfilter.xml "--customfile=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\capsfilter.cs"  --namespace=Gst.CorePlugins "--api=%GSTREAMERSHARP_API_PATH%" "--interfacesdir=%GSTREAMERSHARP_INTERFACES_DIR%" > "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\capsfilter.cs"
%ELEMENT_GEN% --input=fakesrc.xml    "--customfile=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\fakesrc.cs"     --namespace=Gst.CorePlugins "--api=%GSTREAMERSHARP_API_PATH%" "--interfacesdir=%GSTREAMERSHARP_INTERFACES_DIR%" > "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\fakesrc.cs"
%ELEMENT_GEN% --input=fakesink.xml   "--customfile=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\fakesink.cs"    --namespace=Gst.CorePlugins "--api=%GSTREAMERSHARP_API_PATH%" "--interfacesdir=%GSTREAMERSHARP_INTERFACES_DIR%" > "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\fakesink.cs"
%ELEMENT_GEN% --input=filesrc.xml    "--customfile=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\filesrc.cs"     --namespace=Gst.CorePlugins "--api=%GSTREAMERSHARP_API_PATH%" "--interfacesdir=%GSTREAMERSHARP_INTERFACES_DIR%" > "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\filesrc.cs"
%ELEMENT_GEN% --input=filesink.xml   "--customfile=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\filesink.cs"    --namespace=Gst.CorePlugins "--api=%GSTREAMERSHARP_API_PATH%" "--interfacesdir=%GSTREAMERSHARP_INTERFACES_DIR%" > "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\filesink.cs"
%ELEMENT_GEN% --input=identity.xml   "--customfile=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\identity.cs"    --namespace=Gst.CorePlugins "--api=%GSTREAMERSHARP_API_PATH%" "--interfacesdir=%GSTREAMERSHARP_INTERFACES_DIR%" > "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\identity.cs"
%ELEMENT_GEN% --input=queue.xml      "--customfile=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\queue.cs"       --namespace=Gst.CorePlugins "--api=%GSTREAMERSHARP_API_PATH%" "--interfacesdir=%GSTREAMERSHARP_INTERFACES_DIR%" > "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\queue.cs"
%ELEMENT_GEN% --input=tee.xml        "--customfile=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\tee.cs"         --namespace=Gst.CorePlugins "--api=%GSTREAMERSHARP_API_PATH%" "--interfacesdir=%GSTREAMERSHARP_INTERFACES_DIR%" > "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\tee.cs"
%ELEMENT_GEN% --input=typefind.xml   "--customfile=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\typefind.cs"    --namespace=Gst.CorePlugins "--api=%GSTREAMERSHARP_API_PATH%" "--interfacesdir=%GSTREAMERSHARP_INTERFACES_DIR%" > "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\typefind.cs"
%ELEMENT_GEN% --input=multiqueue.xml "--customfile=%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\multiqueue.cs"  --namespace=Gst.CorePlugins "--api=%GSTREAMERSHARP_API_PATH%" "--interfacesdir=%GSTREAMERSHARP_INTERFACES_DIR%" > "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\coreplugins\multiqueue.cs"


REM Configure AssemblyInfo.cs
cd "%GSTREAMERSHARP_SRC_DIR%\gstreamer-sharp\"
sed -e "s/@API_VERSION@/%GSTREAMERSHARP_API_VERSION_REGEX%/g" "AssemblyInfo.cs.in" > "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\AssemblyInfo.cs"

cd "%GSTREAMER_SHARP_GENERATED_DIR%\gstreamer-sharp\"
sed -e "s/public class ObjectManager/internal class ObjectManager/g" ObjectManager.cs > ObjectManager.cs.tmp
move ObjectManager.cs.tmp ObjectManager.cs

REM gst-generate-tags.exe "--header=%SOURCEDIR%\gstreamer\gst\gsttaglist.h" --namespace=Gst --class=Tags






goto exit

:exit
set PATH=%ORIGPATH%
cd /d "%CURR%"