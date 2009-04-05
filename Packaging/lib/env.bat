set OLDDIR=%CD%..
set MYDIR=%~dp0..
set ROOTDIR=%MYDIR%\..
set SDKDIR=%ROOTDIR%\SDKs
set MAINDIR=%ROOTDIR%\Main
set TOOLSDIR=%ROOTDIR%\Tools
set BUILDDIR=%ROOTDIR%\Build
set SHAREDDIR=%ROOTDIR%\Shared
set PACKAGINGDIR=%ROOTDIR%\Packaging
set LIBRARIESDIR=%ROOTDIR%\Libraries
set DEPLOYMENTDIR=%ROOTDIR%\Deployment
set PROPERTIESDIR=%ROOTDIR%\Properties

set PKGDIR=%DEPLOYMENTDIR%\pkg
set EXCLUDESFILE=pkg-excludes.txt

set SHORTCUTFILE=%PKGDIR%\Run.bat

REM MM-DD-YYYY
set TODAYSDATE=%DATE:~4,2%-%DATE:~7,2%-%DATE:~10,4%

REM call "%VS90COMNTOOLS%\vsvars32.bat"

cd /d "%MYDIR%"

rmdir /S /Q "%PKGDIR%"
mkdir "%PKGDIR%"

call lib\excludes.bat