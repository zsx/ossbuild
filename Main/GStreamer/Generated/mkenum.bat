set MY_ORIGPATH=%PATH%
set MY_CURR=%CD%
set MY_DIR=%~dp0.
set MY_TOPDIR=%MY_DIR%\..\..\..
set MY_TOOLSDIR=%MY_TOPDIR%\Tools
set MY_SHAREDDIR=%MY_TOPDIR%\Shared
set MY_SHAREDBIN=%MY_SHAREDDIR%\Build\Windows\Win32\bin
set PATH=%MY_SHAREDBINDIR%;%MY_TOOLSDIR%;%PATH%

set MY_NAME=%1
set MY_WORKINGDIR=%2
set MY_INPUT=%3
set MY_H_OUTPUT=%4
set MY_S_OUTPUT=%5
set MY_H_OUTPUT_FILE=%~nx4

set MY_H_FHEAD=--fhead "#ifndef __GST_%NAME%_ENUM_TYPES_H__\n#define __GST_%NAME%_ENUM_TYPES_H__\n\n#include <glib-object.h>\n\nG_BEGIN_DECLS\n"
set MY_H_FPROD=--fprod "\n/* enumerations from \"@filename@\" */\n"
set MY_H_FTAIL=--ftail "G_END_DECLS\n\n#endif /* __GST_%NAME%_ENUM_TYPES_H__ */"
set MY_H_EPROD=--eprod ""
set MY_H_VHEAD=--vhead "GType @enum_name@_get_type (void);\n#define GST_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n"
set MY_H_VPROD=--vprod ""
set MY_H_VTAIL=--vtail ""
set MY_H_COMMENTS=--comments ""

rem Processes each line in the file
set MY_HEADERS=
setlocal enabledelayedexpansion
for /F %%A in ('type %MY_INPUT%') do set MY_HEADERS=!MY_HEADERS!\n#include \"%%A\"
setlocal disabledelayedexpansion

set MY_S_FHEAD=--fhead "#include \"%H_OUTPUT_FILE%\"\n%HEADERS%"
set MY_S_FPROD=--fprod "\n/* enumerations from \"@filename@\" */"
set MY_S_FTAIL=--ftail ""
set MY_S_EPROD=--eprod ""
set MY_S_VHEAD=--vhead "GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {"
set MY_S_VPROD=--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" },"
set MY_S_VTAIL=--vtail "      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n  return etype;\n}\n"
set MY_S_COMMENTS=--comments ""

set MY_H_ARGS=%MY_H_FHEAD% %MY_H_FPROD% %MY_H_FTAIL% %MY_H_EPROD% %MY_H_VHEAD% %MY_H_VPROD% %MY_H_VTAIL% %MY_H_COMMENTS%
set MY_S_ARGS=%MY_S_FHEAD% %MY_S_FPROD% %MY_S_FTAIL% %MY_S_EPROD% %MY_S_VHEAD% %MY_S_VPROD% %MY_S_VTAIL% %MY_S_COMMENTS%


set MY_MK_ENUMS=perl "%MY_SHAREDBIN%\glib-mkenums"

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
