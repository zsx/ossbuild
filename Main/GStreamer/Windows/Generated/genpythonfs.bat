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

set MY_FS_PYTHON_SOURCES_DIR=..\..\..\..\Source\farsight2\python

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

echo Generating %OUTPUT_DIR%\pyfarsight.c
%MY_PYTHON% %GST_PYTHON_SOURCES_DIR%\codegen\codegen.py  --register %GST_PYTHON_SOURCES_DIR%\gst\gst-types.defs --override %MY_FS_PYTHON_SOURCES_DIR%\pyfarsight.override  --prefix fs %MY_FS_PYTHON_SOURCES_DIR%\pyfarsight.defs > gen-pyfarsight.c

%MY_SED% -e "s/..\\..\\..\\..\\Source\\farsight2\\python\\/..\\\\..\\\\..\\\\..\\\\Source\\\\farsight2\\\\python\\\\/g" gen-pyfarsight.c > pyfarsight.c
del "gen-pyfarsight.c"

goto end

:error
goto end

:end
set PATH=%MY_ORIGPATH%
cd /d "%MY_CURR%"