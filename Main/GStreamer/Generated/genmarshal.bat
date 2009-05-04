set MY_ORIGPATH=%PATH%
set MY_CURR=%CD%
set MY_DIR=%~dp0.
set MY_TOPDIR=%MY_DIR%\..\..\..
set MY_TOOLSDIR=%MY_TOPDIR%\Tools
set MY_SHAREDDIR=%MY_TOPDIR%\Shared
set MY_SHAREDBINDIR=%MY_SHAREDDIR%\Build\Windows\Win32\bin

set MY_GEN_MARSHAL=glib-genmarshal.exe
set PATH=%MY_SHAREDBINDIR%;%MY_TOOLSDIR%;%PATH%

set MY_PREFIX=%1
set MY_INPUT=%2
set MY_H_OUTPUT=%3
set MY_S_OUTPUT=%4

echo Generating %MY_H_OUTPUT%
%MY_GEN_MARSHAL% --header --skip-source --prefix=%MY_PREFIX% %MY_INPUT% > %MY_H_OUTPUT%

echo Generating %MY_S_OUTPUT%
%MY_GEN_MARSHAL% --body --skip-source --prefix=%MY_PREFIX%  %MY_INPUT% > %MY_S_OUTPUT%

goto end

:error
goto end

:end
set PATH=%MY_ORIGPATH%
cd /d "%MY_CURR%"