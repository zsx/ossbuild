set MY_ORIGPATH=%PATH%
set MY_CURR=%CD%
set MY_DIR=%~dp0.
set MY_TOPDIR=%MY_DIR%\..\..\..\..
set MY_TOOLSDIR=%MY_TOPDIR%\Tools
set MY_SHAREDDIR=%MY_TOPDIR%\Shared
set MY_SHAREDBIN=%MY_SHAREDDIR%\Build\Windows\Win32\bin
set MY_BUILDBINDIR=%MY_TOPDIR%\Build\Windows\Win32\Release\bin
set PATH=%MY_BUILDBINDIR%;%MY_SHAREDBINDIR%;%MY_TOOLSDIR%;%PATH%

set MY_WORKINGDIR=%1
set MY_INPUT=%2
set MY_H_OUTPUT=%3
set MY_S_OUTPUT=%4
set MY_H_OUTPUT_FILE=%~nx3

shift
shift & set MY_H_FHEAD=--fhead %4
shift & set MY_H_FPROD=--fprod %4
shift & set MY_H_FTAIL=--ftail %4
shift & set MY_H_EPROD=--eprod %4
shift & set MY_H_VHEAD=--vhead %4
shift & set MY_H_VPROD=--vprod %4
shift & set MY_H_VTAIL=--vtail %4
shift & set MY_H_COMMENTS=--comments %4

shift & set MY_S_FHEAD=--fhead %4
shift & set MY_S_FPROD=--fprod %4
shift & set MY_S_FTAIL=--ftail %4
shift & set MY_S_EPROD=--eprod %4
shift & set MY_S_VHEAD=--vhead %4
shift & set MY_S_VPROD=--vprod %4
shift & set MY_S_VTAIL=--vtail %4
shift & set MY_S_COMMENTS=--comments %4

set MY_H_ARGS=%MY_H_FHEAD% %MY_H_FPROD% %MY_H_FTAIL% %MY_H_EPROD% %MY_H_VHEAD% %MY_H_VPROD% %MY_H_VTAIL% %MY_H_COMMENTS%
set MY_S_ARGS=%MY_S_FHEAD% %MY_S_FPROD% %MY_S_FTAIL% %MY_S_EPROD% %MY_S_VHEAD% %MY_S_VPROD% %MY_S_VTAIL% %MY_S_COMMENTS%


set MY_MK_ENUMS=perl "%MY_BUILDBINDIR%\glib-mkenums"
if not exist "%MY_BUILDBINDIR%\glib-mkenums" (
	set MY_MK_ENUMS=perl "%MY_SHAREDBIN%\glib-mkenums"
	if not exist "%MY_SHAREDBIN%\glib-mkenums" (
		echo Missing glib-mkenums
		goto error
	)
)

if "%MY_WORKINGDIR%" == "" (
    echo Missing working directory
    goto error
)

cd /d "%MY_WORKINGDIR%"

rem Create a space-delimited list of every line in the input file
for /F "tokens=*" %%A in ('sed -n "1h;2,$H;${g;s/\n/ /g;p}" %MY_INPUT%') do set MY_FILES=%%A



:header
if "%MY_H_OUTPUT%" == "" (
    echo Missing valid header output path
    goto error
)

echo Generating %MY_H_OUTPUT%
%MY_MK_ENUMS% %MY_H_ARGS% %MY_FILES% > %MY_H_OUTPUT%



:source
if "%MY_S_OUTPUT%" == "" (
    echo Missing valid source output path
    goto error
)

echo Generating %MY_S_OUTPUT%
%MY_MK_ENUMS% %MY_S_ARGS% %MY_FILES% > %MY_S_OUTPUT%



goto end

:error
set PATH=%MY_ORIGPATH%
cd /d "%MY_CURR%"
goto end

:end
set PATH=%MY_ORIGPATH%
cd /d "%MY_CURR%"
