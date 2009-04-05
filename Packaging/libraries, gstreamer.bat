@echo off

rem **************************************************************************************************************************************
rem *	Package up all related services, take out 
rem *	any subversion directories and zip it up
rem **************************************************************************************************************************************

call lib\env.bat

set ZIPFILE=%DEPLOYMENTDIR%\[%TODAYSDATE%] Libraries, GStreamer.zip
set GSTDIR=%MAINDIR%\gstreamer

REM Determine build type

REM if not exist "%SERVICESDIR%\Scheduler\bin\Release\" goto DEBUG
REM if exist "%SERVICESDIR%\Scheduler\bin\Release\" goto RELEASE

:DEBUG
REM set BUILD=Debug
REM goto CONTINUE

:RELEASE
set BUILD=Release
goto CONTINUE

:CONTINUE

del /F /Q "%ZIPFILE%"

call lib\excludes.bat .svn .csproj.user .suo .pdb .mdb .sql .cs \obj\ 

REM xcopy /E /R /K /Y /EXCLUDE:%EXCLUDESFILE% "%SERVICESDIR%\Scheduler\bin\%BUILD%\*" "%PKGDIR%\" 

call lib\zip.bat


call lib\cleanup.bat

@echo on
