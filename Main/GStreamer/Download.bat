@echo off

REM **********************************************************************
REM * If you would like to upgrade, then go to 
REM * 		http://gstreamer.freedesktop.org/src/
REM * 
REM * Find the latest version of each of the components (plugins) 
REM * and update the VER_* variables below. It's advisable that you don't 
REM * mix-and-match old and new versions of the various components. Either 
REM * upgrade everything at once or none at all.
REM **********************************************************************

set DOWNLOAD=1
set UNTAR=1

set DIR=%~dp0.
set DOWNLOADDIR=%DIR%\Downloads
set TOOLSDIR=%DIR%\..\..\Tools

set PATH=%TOOLSDIR%;%PATH%

mkdir "%DOWNLOADDIR%"

cd /d "%DOWNLOADDIR%"

set VER_GST=0.10.22
set VER_GST_FFMPEG=0.10.7
set VER_GST_PLUGINS_BASE=0.10.22
set VER_GST_PLUGINS_GOOD=0.10.14
set VER_GST_PLUGINS_UGLY=0.10.10
set VER_GST_PLUGINS_BAD=0.10.10

set DIR_GST=gstreamer-%VER_GST%
set DIR_GST_FFMPEG=gst-ffmpeg-%VER_GST_FFMPEG%
set DIR_GST_PLUGINS_BASE=gst-plugins-base-%VER_GST_PLUGINS_BASE%
set DIR_GST_PLUGINS_GOOD=gst-plugins-good-%VER_GST_PLUGINS_GOOD%
set DIR_GST_PLUGINS_UGLY=gst-plugins-ugly-%VER_GST_PLUGINS_UGLY%
set DIR_GST_PLUGINS_BAD=gst-plugins-bad-%VER_GST_PLUGINS_BAD%

set DEST_DIR_GST=gstreamer
set DEST_DIR_GST_FFMPEG=gst-ffmpeg
set DEST_DIR_GST_PLUGINS_BASE=gst-plugins-base
set DEST_DIR_GST_PLUGINS_GOOD=gst-plugins-good
set DEST_DIR_GST_PLUGINS_UGLY=gst-plugins-ugly
set DEST_DIR_GST_PLUGINS_BAD=gst-plugins-bad

if "%DOWNLOAD%" == "1" (
	wget --no-check-certificate -O gstreamer.tar.gz http://gstreamer.freedesktop.org/src/gstreamer/gstreamer-%VER_GST%.tar.gz
	wget --no-check-certificate -O gst-ffmpeg.tar.gz http://gstreamer.freedesktop.org/src/gst-ffmpeg/gst-ffmpeg-%VER_GST_FFMPEG%.tar.gz
	wget --no-check-certificate -O gst-plugins-base.tar.gz http://gstreamer.freedesktop.org/src/gst-plugins-base/gst-plugins-base-%VER_GST_PLUGINS_BASE%.tar.gz
	wget --no-check-certificate -O gst-plugins-good.tar.gz http://gstreamer.freedesktop.org/src/gst-plugins-good/gst-plugins-good-%VER_GST_PLUGINS_GOOD%.tar.gz
	wget --no-check-certificate -O gst-plugins-ugly.tar.gz http://gstreamer.freedesktop.org/src/gst-plugins-ugly/gst-plugins-ugly-%VER_GST_PLUGINS_UGLY%.tar.gz
	wget --no-check-certificate -O gst-plugins-bad.tar.gz http://gstreamer.freedesktop.org/src/gst-plugins-bad/gst-plugins-bad-%VER_GST_PLUGINS_BAD%.tar.gz
)

if "%UNTAR%" == "1" (
	if exist "%DIR_GST%" rmdir /S /Q "%DIR_GST%"
	if exist "%DEST_DIR_GST%" rmdir /S /Q "%DEST_DIR_GST%"
	7z x gstreamer.tar.gz
	7z x gstreamer.tar
	del gstreamer.tar
	move "%DIR_GST%" "%DEST_DIR_GST%"

	if exist "%DIR_GST_FFMPEG%" rmdir /S /Q "%DIR_GST_FFMPEG%"
	if exist "%DEST_DIR_GST_FFMPEG%" rmdir /S /Q "%DEST_DIR_GST_FFMPEG%"
	7z x gst-ffmpeg.tar.gz
	7z x gst-ffmpeg.tar
	del gst-ffmpeg.tar
	move "%DIR_GST_FFMPEG%" "%DEST_DIR_GST_FFMPEG%"

	if exist "%DIR_GST_PLUGINS_BASE%" rmdir /S /Q "%DIR_GST_PLUGINS_BASE%"
	if exist "%DEST_DIR_GST_PLUGINS_BASE%" rmdir /S /Q "%DEST_DIR_GST_PLUGINS_BASE%"
	7z x gst-plugins-base.tar.gz
	7z x gst-plugins-base.tar
	del gst-plugins-base.tar
	move "%DIR_GST_PLUGINS_BASE%" "%DEST_DIR_GST_PLUGINS_BASE%"

	if exist "%DIR_GST_PLUGINS_GOOD%" rmdir /S /Q "%DIR_GST_PLUGINS_GOOD%"
	if exist "%DEST_DIR_GST_PLUGINS_GOOD%" rmdir /S /Q "%DEST_DIR_GST_PLUGINS_GOOD%"
	7z x gst-plugins-good.tar.gz
	7z x gst-plugins-good.tar
	del gst-plugins-good.tar
	move "%DIR_GST_PLUGINS_GOOD%" "%DEST_DIR_GST_PLUGINS_GOOD%"

	if exist "%DIR_GST_PLUGINS_UGLY%" rmdir /S /Q "%DIR_GST_PLUGINS_UGLY%"
	if exist "%DEST_DIR_GST_PLUGINS_UGLY%" rmdir /S /Q "%DEST_DIR_GST_PLUGINS_UGLY%"
	7z x gst-plugins-ugly.tar.gz
	7z x gst-plugins-ugly.tar
	del gst-plugins-ugly.tar
	move "%DIR_GST_PLUGINS_UGLY%" "%DEST_DIR_GST_PLUGINS_UGLY%"

	if exist "%DIR_GST_PLUGINS_BAD%" rmdir /S /Q "%DIR_GST_PLUGINS_BAD%"
	if exist "%DEST_DIR_GST_PLUGINS_BAD%" rmdir /S /Q "%DEST_DIR_GST_PLUGINS_BAD%"
	7z x gst-plugins-bad.tar.gz
	7z x gst-plugins-bad.tar
	del gst-plugins-bad.tar
	move "%DIR_GST_PLUGINS_BAD%" "%DEST_DIR_GST_PLUGINS_BAD%"
)


REM rmdir /S /Q %DIR_GST%
REM rmdir /S /Q %DIR_GST_FFMPEG%
REM rmdir /S /Q %DIR_GST_PLUGINS_BASE%
REM rmdir /S /Q %DIR_GST_PLUGINS_GOOD%
REM rmdir /S /Q %DIR_GST_PLUGINS_UGLY%
REM rmdir /S /Q %DIR_GST_PLUGINS_BAD%

:exit
cd /d "%DIR%"