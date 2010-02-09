set MY_ORIGPATH=%PATH%
set MY_CURR=%CD%
set MY_DIR=%~dp0.
set MY_TOPDIR=%MY_DIR%\..\..\..\..
set MY_TOOLSDIR=%MY_TOPDIR%\Tools
set MY_SHAREDDIR=%MY_TOPDIR%\Shared
set MY_SHAREDBINDIR=%MY_SHAREDDIR%\Build\Windows\Win32\bin

set MY_PYTHON_VERSION=Python25
set MY_PYTHON_INSTALL_DIR=C:\%MY_PYTHON_VERSION%
set MY_PYTHON=%MY_PYTHON_INSTALL_DIR%\python.exe

set MY_SED=sed.exe

set PATH=%MY_SHAREDBINDIR%;%MY_TOOLSDIR%;%MY_PYTHON_INSTALL_DIR%;%PATH%

set GST_PYTHON_SOURCES_DIR=%1
set GENERATED_OUTPUT_DIR=%2
set MY_PREFIX=%3

set MY_GST_PYTHON_SOURCES_DIR=..\..\..\Source\gst-python

rem Attempt to see if python exists on the system
%MY_PYTHON% -V > NUL

if %ERRORLEVEL% neq 0 (
	echo ** WARNING **
	echo Unable to produce %MY_PREFIX%.c
	echo You are missing python. The script was unable to generate python-related files.
	echo If you do not need to generate the python files from scratch, then you're okay.
	goto end
)


cd %GENERATED_OUTPUT_DIR%
echo %GENERATED_OUTPUT_DIR%

echo Generating %OUTPUT_DIR%\%MY_PREFIX%.c
%MY_PYTHON% %MY_GST_PYTHON_SOURCES_DIR%\codegen\codegen.py --load-types %MY_GST_PYTHON_SOURCES_DIR%\gst\arg-types.py --register %MY_GST_PYTHON_SOURCES_DIR%\gst\gst-types.defs --extendpath %MY_GST_PYTHON_SOURCES_DIR%\gst --override %MY_GST_PYTHON_SOURCES_DIR%\gst\%MY_PREFIX%.override  --prefix py%MY_PREFIX% %MY_GST_PYTHON_SOURCES_DIR%\gst\%MY_PREFIX%.defs > gen-%MY_PREFIX%.c

%MY_SED% -e "s/..\\..\\..\\Source\\gst-python\\gst\\/..\\\\..\\\\..\\\\Source\\\\gst-python\\\\gst\\\\/g" gen-%MY_PREFIX%.c > %MY_PREFIX%.c
del "gen-%MY_PREFIX%.c"

goto end

:error
goto end

:end
set PATH=%MY_ORIGPATH%
cd /d "%MY_CURR%"