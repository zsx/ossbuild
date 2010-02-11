@echo off
set OLDDIR=%CD%
set MYDIR=%~dp0
cd /d "%MYDIR%"


cd /d "../../../../../Deployment/GStreamer/Windows/x86/Install/"
msiexec /i "x86-OSSBuild-GStreamer-SDK-LGPL.msi" /l*v "%MYDIR%/Install.log"


cd /d "%OLDDIR%"
@echo on