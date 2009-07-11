set MY_ORIGPATH=%PATH%
set MY_CURR=%CD%
set MY_DIR=%~dp0.
set MY_TOPDIR=%MY_DIR%\..\..\..
set MY_TOOLSDIR=%MY_TOPDIR%\Tools

set MY_FLEX=flex.exe
set PATH=%MY_TOOLSDIR%;%PATH%

set M4=%MY_TOOLSDIR%\m4.exe
set BISON_PKGDATADIR=%MY_TOOLSDIR%\share\bison
set M4PATH=%BISON_PKGDATADIR%

set MY_PREFIX=%1
set MY_INPUT=%2
set MY_OUTPUT=%3

echo Generating %MY_OUTPUT%
%MY_FLEX% -L -P%MY_PREFIX% -o%MY_OUTPUT% %MY_INPUT%

goto end

:error
goto end

:end
set PATH=%MY_ORIGPATH%
cd /d "%MY_CURR%"