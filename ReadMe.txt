Requirements
-------------------------------------

  Windows Build

    1. Visual Studio 2008 or later
    2. Perl 5.10+
       a. Recommended ActiveState Perl (http://www.activestate.com/downloads/) on Windows
    3. DirectX SDK (November 2008 or later)
       a. Download from http://www.microsoft.com/downloads/info.aspx?na=90&p=&SrcDisplayLang=en&SrcCategoryId=&SrcFamilyId=5493f76a-6d37-478d-ba17-28b1cca4865a&u=http%3a%2f%2fdownload.microsoft.com%2fdownload%2f5%2f8%2f2%2f58223f79-689d-47ae-bdd0-056116ee8d16%2fDXSDK_Nov08.exe
    4. Python 2.5 (to maintainn backward compatibility)
       a. Download from http://www.python.org/download/releases/2.5/
       b. Install at c:\Python25
    5. PyGobject (2.14)
       a. Download from http://ftp.gnome.org/pub/GNOME/binaries/win32/pygobject/2.14/

  Linux Build

    1. (sudo) apt-get install perl sed pkg-config build-essential bison flex


Setup Visual Studio 2008+
-------------------------------------

  Add Custom Rules Path

    1. Select the Tools > Options... menu.
    2. Expand "Projects and Solutions" and select "VC++ Project Settings"
    3. In "Rule File Search Paths", add the top-level directory for this code (e.g. C:\Work\Dependencies)
       a. If needed, separate other search paths with a semicolon (;).

Setup Linux
-------------------------------------

  Ubuntu

    1. sudo apt-get install build-essential perl sed pkg-config subversion rapidsvn autoconf libtool bison flex gettext yasm xorg-dev?


Acknowledgments
-------------------------------------

OAH Build
https://launchpad.net/oah

GStreamer WinBuilds
http://www.gstreamer-winbuild.ylatuya.es/doku.php?id=start

GStreamer
http://gstreamer.freedesktop.org/

libxml2
http://www.xmlsoft.org/index.html

zlib
http://www.zlib.net/

openssl
http://www.openssl.org/

libneon
http://www.webdav.org/neon/

pthreads-win32
http://sourceware.org/pthreads-win32/

glib (+proxy-libintl)
http://www.gtk.org/

libjpeg
http://www.ijg.org/

libpng
http://www.libpng.org/pub/png/libpng.html

ffmpeg
http://ffmpeg.org/

7-Zip
http://www.7-zip.org/

Msys+Mingw
http://www.mingw.org/

Yasm
http://www.tortall.net/projects/yasm/


Licenses
-------------------------------------

Please see the licenses in the Licenses/ folder for details on each library/application used.

For copyright information, please see COPYING.