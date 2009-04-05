@echo off

rem **************************************************************************************************************************************
rem *	Package up MSys development environment 
rem **************************************************************************************************************************************

call lib\env.bat

set PATH=%TOOLSDIR%;%PATH%
set BZIPFILE=%DEPLOYMENTDIR%\msys.tar.bz2
if exist "%BZIPFILE%" (
    echo Removing previous file
    echo Please wait...
    del /F /Q "%BZIPFILE%"
)


echo Removing home directories
echo Please wait...

rem Get rid of all home directories which aren't really used for anything anyway in msys
cd /d "C:\msys\"
rmdir /S /Q home
mkdir home

echo Archiving and compressing %BZIPPATH%
echo Please wait (this can take a long time)...

cd /d "C:\"
bsdtar.exe cjf "%BZIPFILE%" "msys"


cd /d "%PACKAGINGDIR%"
call lib\cleanup.bat

@echo on
