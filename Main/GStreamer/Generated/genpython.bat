set MY_ORIGPATH=%PATH%
set MY_CURR=%CD%
set MY_DIR=%~dp0.
set MY_TOPDIR=%MY_DIR%\..\..\..
set MY_TOOLSDIR=%MY_TOPDIR%\Tools
set MY_SHAREDDIR=%MY_TOPDIR%\Shared
set MY_SHAREDBINDIR=%MY_SHAREDDIR%\Build\Windows\Win32\bin

set MY_PYTHON_VERSION=Python25
set MY_PYTHON_INSTALL_DIR=c:\%MY_PYTHON_VERSION%
set MY_PYTHON=%MY_PYTHON_INSTALL_DIR%\python.exe

set MY_SED=sed.exe

set PATH=%MY_SHAREDBINDIR%;%MY_TOOLSDIR%;%PATH%

set GST_PYTHON_SOURCES_DIR=%1
set OUTPUT_DIR=%2
set MY_PREFIX=%3


echo %GST_PYTHON_SOURCES_DIR%
cd %GST_PYTHON_SOURCES_DIR%

echo Generating %OUTPUT_DIR%\%MY_PREFIX%.c
%MY_PYTHON% codegen/codegen.py --load-types gst\arg-types.py --register gst\gst-types.defs --extendpath gst\ --override gst\%MY_PREFIX%.override  --prefix py%MY_PREFIX% gst\%MY_PREFIX%.defs > %OUTPUT_DIR%\gen-%MY_PREFIX%.c
rem %MY_SED% -e "s/..\\..\\gst\\/..\\\\..\\\\gst\\\\/g" %OUTPUT_DIR%\%MY_PREFIX%.c
move %OUTPUT_DIR%\gen-%MY_PREFIX%.c %OUTPUT_DIR%\%MY_PREFIX%.c 

goto end

:error
goto end

:end
set PATH=%MY_ORIGPATH%
cd /d "%MY_CURR%"