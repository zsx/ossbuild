Requirements
-------------------------------------

  Windows Build

    1. Visual Studio 2008 or later
    2. Windows Installer XML (WiX) 3.5 or later (http://wix.sourceforge.net/releases/3.5.1322.0/Wix35.msi)
    3. Perl 5.10+
       a. Recommended ActiveState Perl (http://www.activestate.com/downloads/) on Windows
          1. Download the LibXML package 
             a. Load the perl package manager ("ppm" command)
             b. Select the Edit > Preferences menu
             c. Under "Add Repository" select "University of Winnipeg" from the "suggested" list and select "Add"
             d. Select "OK"
             e. Scroll down the list and right click on "XML-LibXML" and "XML-LibXML-Common" both and select install
             f. Select File > Run Marked Actions
    4. DirectX SDK (November 2008 or later)
       a. Download from http://www.microsoft.com/downloads/info.aspx?na=90&p=&SrcDisplayLang=en&SrcCategoryId=&SrcFamilyId=5493f76a-6d37-478d-ba17-28b1cca4865a&u=http%3a%2f%2fdownload.microsoft.com%2fdownload%2f5%2f8%2f2%2f58223f79-689d-47ae-bdd0-056116ee8d16%2fDXSDK_Nov08.exe
    5. Python 2.5 (for backward compatibility)
       a. Download from http://www.python.org/download/releases/2.5/
       b. Install to C:\Python25
    6. PyGobject (2.14)
       a. Download from http://ftp.gnome.org/pub/GNOME/binaries/win32/pygobject/2.14/

  Linux Build

    1. (sudo) apt-get install perl sed pkg-config build-essential bison flex libtool tofrodos vim gettext yasm nasm


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

For legal purposes, you must inspect the individual licenses of all packages to determine if 
they fit your legal constraints. OSSBuild or any of its developers are NOT legally responsible 
for any failure to ensure compliance or any legal action resulting from the use of this 
software. It is solely the responsibility of the recipient and user to determine legal 
eligibility for use.
