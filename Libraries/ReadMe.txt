
Version Information

zlib		- 		1.2.3 		- 	http://www.zlib.net/zlib123.zip
openssl		- 		0.9.8j 		- 	http://www.openssl.org/source/openssl-0.9.8j.tar.gz
libxml2		- 		2.7.3 		- 	ftp://xmlsoft.org/libxml2/libxml2-2.7.3.tar.gz
libneon		- 		0.28.4 		- 	http://www.webdav.org/neon/neon-0.28.4.tar.gz
pthreads 	- 		2.8.0 		- 	ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-8-0-release.tar.gz
glib 		- 		2.20.0 		- 	http://ftp.gnome.org/pub/gnome/sources/glib/2.20/glib-2.20.0.tar.bz2
proxy-libintl 	- 		20080918 	- 	http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/proxy-libintl-dev_20080918_win32.zip
libjpeg 	- 		6.2.0 (6b)	- 	http://www.ijg.org/files/jpegsrc.v6b.tar.gz
libpng 		- 		1.2.35 		- 	ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng-1.2.35.tar.gz
pango 		-		1.23.0 		- 	http://ftp.gnome.org/pub/GNOME/sources/pango/1.23/pango-1.23.0.tar.gz
freetype 	- 		2.3.8		- 	http://mirror.its.uidaho.edu/pub/savannah/freetype/freetype-2.3.8.tar.gz
fontconfig 	- 		2.6.0 		- 	http://fontconfig.org/release/fontconfig-2.6.0.tar.gz
libogg 		- 		1.1.3 		- 	http://downloads.xiph.org/releases/ogg/libogg-1.1.3.tar.gz
libvorbis 	- 		1.2.0 		- 	http://downloads.xiph.org/releases/vorbis/libvorbis-1.2.0.tar.gz
libtheora 	- 		1.0.0 		- 	http://downloads.xiph.org/releases/theora/libtheora-1.0.tar.bz2
libmms 		- 		0.4.0 		- 	https://code.launchpad.net/~libmms-devel/libmms/win32
pixman 		- 		0.14.0 		- 	http://cairographics.org/releases/pixman-0.14.0.tar.gz
cairo 		- 		1.8.6 		- 	http://cairographics.org/releases/cairo-1.8.6.tar.gz
x264 		- 	   r2245 (build 67)	- 	http://download.videolan.org/pub/videolan/x264/snapshots/x264-snapshot-20090310-2245.tar.bz2
ffmpeg 		- 		0.5.0 		- 	http://www.ffmpeg.org/releases/ffmpeg-0.5.tar.bz2
liboil 		- 		0.3.16 		- 	http://liboil.freedesktop.org/download/liboil-0.3.16.tar.gz
libsoup 	- 		2.26.0.9	- 	http://ftp.gnome.org/pub/GNOME/sources/libsoup/2.26/libsoup-2.26.0.9.tar.gz
sqlite 		- 		3.6.11 		- 	http://www.sqlite.org/sqlite-amalgamation-3.6.11.tar.gz
libgpg-error 	- 		1.7.0 		- 	ftp://ftp.gnupg.org/gcrypt/libgpg-error/libgpg-error-1.7.tar.gz
libgcrypt 	- 		1.4.4 		- 	ftp://ftp.gnupg.org/gcrypt/libgcrypt/libgcrypt-1.4.4.tar.gz
gnutls 		- 		2.6.5 		- 	http://ftp.gnu.org/pub/gnu/gnutls/gnutls-2.6.5.tar.bz2
libspeex 	- 		1.2rc1 		- 	http://downloads.xiph.org/releases/speex/speex-1.2rc1.tar.gz
libschrodinger 	- 		1.0.6 		- 	http://diracvideo.org/download/schroedinger/schroedinger-1.0.6.tar.gz
mp3lame 	- 		3.98.2 		- 	http://sourceforge.net/project/downloading.php?group_id=290&use_mirror=superb-west&filename=lame-398-2.tar.gz&a=60930405

libjpeg.def was added to libjpeg to define exported functions. I preferred not to redefine the "GLOBAL" macro as per the instructions in the documentation.

There are inter-dependencies among the projects. If you don't need something, you'll have to adjust the preprocessor symbols and link lib includes.

FreeType - made 1 change to include/config/ftoption.h (line 229): made it: 
#define  FT_EXPORT(x)       __declspec( dllexport ) x
That's so it will generate exportable funcs for use in a dll.

Fontconfig is not currently being used anywhere. It was originally intended for use by pangoft2 which also requires FreeType. But neither is required for text rendering in Windows. So we use the pangowin32 lib instead.

Libmms required a checkout and download of a win32-compatible version using glib. See the URL given for more details.


MinGW-64:
http://www.drangon.org/mingw/



A correct build order (not the only one):

zlib
pthreads
libxml2
libjpeg
libpng
proxy-libintl
glib
openssl
libneon
freetype
fontconfig
pixman
cairo
pango
libogg
libvorbis
libtheora
libmms
x264


A correct load order (linux):

libz.so
libxml2.so
libjpeg.so
libpng12.so
libglib-2.0.so
libgobject-2.0.so
libgmodule-2.0.so
libgthread-2.0.so
libgio-2.0.so
libcrypto.so
libssl.so
libneon.so
libfreetype.so
libfontconfig.so
libpixman-1.so
libcairo.so
libpango-1.0.so
libpangocairo-1.0.so
libpangoft2-1.0.so
libogg.so
libvorbis.so
libvorbisenc.so
libvorbisfile.so
libtheora.so
libtheoradec.so
libtheoraenc.so
libmms.so
libx264.so

Libraries requiring a posix environment for compilation:

liboil
ffmpeg
