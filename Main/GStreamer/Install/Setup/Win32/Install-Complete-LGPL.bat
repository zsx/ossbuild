@echo off
set OLDDIR=%CD%
set MYDIR=%~dp0
cd /d "%MYDIR%"


cd /d "bin/Release/en-us"
msiexec /i "x86-OSSBuild-GStreamer-Complete-LGPL.msi" /l*v ../../../Install.log


cd /d "%OLDDIR%"
@echo on