@echo off

set OLDDIR=%CD%
set MYDIR=%~dp0

if "x%VS90COMNTOOLS%" == "x" (
	echo Missing VS90COMNTOOLS environment variable. Cannot continue.
	goto Error
)


call "%VS90COMNTOOLS%\vsvars32.bat"

cd /d "%MYDIR%"


lib /nologo /machine:x64 /name:msvcrt.dll /def:msvcrt-compat.def /out:lib\x64\msvcrt_compat.lib
lib /nologo /machine:x86 /name:msvcrt.dll /def:msvcrt-compat.def /out:lib\Win32\msvcrt_compat.lib

echo Removing unnecessary files...
del lib\x64\msvcrt_compat.exp
del lib\Win32\msvcrt_compat.exp

echo Completed successfully!

pause
goto Done


:Error
cd /d "%OLDDIR%"
exit 1

:Done
cd /d "%OLDDIR%"
exit 0