#!/bin/sh

TOP=$(dirname $0)/..

CFLAGS="$CFLAGS -fno-strict-aliasing -fomit-frame-pointer -mms-bitfields -pipe -D_WIN32_WINNT=0x0501 -DUSE_GETADDRINFO -DHAVE_GETNAMEINFO -DHAVE_GETSOCKOPT -DHAVE_INET_NTOP -DHAVE_INET_PTON"
CPPFLAGS="$CPPFLAGS -DMINGW32 -D__MINGW32__"
LDFLAGS="-Wl,--enable-auto-image-base -Wl,--enable-auto-import -Wl,--enable-runtime-pseudo-reloc -Wl,--kill-at "
CXXFLAGS="${CFLAGS}"

#Call common startup routines to load a bunch of variables we'll need
. $TOP/Shared/Scripts/Common.sh
common_startup "Windows" "Win32" "Release" "x86"

#Select which MS CRT we want to build against (msvcr90.dll)
. $ROOT/Shared/Scripts/CRT-x86.sh
crt_startup

#Setup library versions
. $ROOT/Shared/Scripts/Version.sh

save_prefix ${DefaultPrefix}

#Move to intermediate directory
cd "$IntDir"

#Add MS build tools
setup_ms_build_env_path

#clear_flags

#No prefix from here out
clear_prefix

#Update flags to make sure we don't try and export intl (gettext) functions more than once
#Setting it to ORIG_LDFLAGS ensures that any calls to reset_flags will include these changes.
ORIG_LDFLAGS="$ORIG_LDFLAGS -Wl,--exclude-libs=libintl.a -Wl,--add-stdcall-alias"
reset_flags

#QueueUserAPCEx
#Make it available for use by other libs that might need it (e.g. pthreads)
unpack_zip_and_move_windows "queueuserapcex.zip" "QueueUserAPCEx" "."
PKG_QUEUE_USER_APC_EX=`pwd`










##liboil
#cd "$IntDir/liboil"
#make check
#
#pthreads
PthreadsPrefix=lib${Prefix}
if [ "${Prefix}" = "" ]; then
	PthreadsPrefix=""
fi
unpack_gzip_and_move "pthreads-w32.tar.gz" "$PKG_DIR_PTHREADS"
cp -p -r "$PKG_QUEUE_USER_APC_EX" "$PKG_DIR"
cp -p "$BinDir/${PthreadsPrefix}pthreadGC2.dll" "$PKG_DIR\pthreadGC2.dll"
cd tests
make GC
#
##zlib
#unpack_zip_and_move_windows "zlib.zip" "zlib" "zlib"
#make -fwin32/Makefile.gcc test
#
##bzip2
#unpack_gzip_and_move "bzip2.tar.gz" "$PKG_DIR_BZIP2"
#make test
#
##expat
#cd "$IntDir/expat"
#make check
#
##glew
#
###libxml2
##cd "$IntDir/libxml2"
###wget http://www.w3.org/XML/Test/xmlts20080827.tar.gz
##make tests
#
##libjpeg
#cd "$IntDir/libjpeg"
#make check
#
##openjpeg
#unpack_gzip_and_move "openjpeg.tar.gz" "$PKG_DIR_OPENJPEG"
#
##libpng
#mkdir_and_move "$IntDir/libpng"	
#make check
#
##glib
#mkdir_and_move "$IntDir/glib"
#make check
#
##libgpg-error, libgcrypt were meant to be tested only from 
##linux. The windows builds are typically cross-compiled from 
##linux and as such, won't run under Windows.
###libgpg-error
##mkdir_and_move "$IntDir/libgpg-error"
##make check
##libgcrypt
##mkdir_and_move "$IntDir/libgcrypt"
##make check
#
##libtasn1
#mkdir_and_move "$IntDir/libtasn1"
#make check
#
##libgnutls
#mkdir_and_move "$IntDir/gnutls"
#make check
#
##curl
##TODO: Fixme
#mkdir_and_move "$IntDir/curl"
#make check
#
##soup
##TODO: Fixme
#mkdir_and_move "$IntDir/libsoup"
#make check
#
###neon
###TODO: Doesn't work
##mkdir_and_move "$IntDir/neon"
##make check CFLAGS="$CFLAGS -DLC_MESSAGES=6"
#
###freetype
###No testing pkgs
##mkdir_and_move "$IntDir/freetype"
##make check
#
###fontconfig
###TODO: Doesn't work
##mkdir_and_move "$IntDir/fontconfig"
##cp -p "$BinDir/lib${DefaultPrefix}fontconfig-1.dll" "test/libfontconfig-1.dll"
##make check
#
##pixman
#mkdir_and_move "$IntDir/pixman"
#make check
#
###cairo
###TODO: Doesn't work
##mkdir_and_move "$IntDir/cairo"
##echo "int main () { return 0; }" > main.c
##gcc -o main.o -c main.c
##make check CFLAGS="$CFLAGS -Wl,main.o"
#
##pango
#mkdir_and_move "$IntDir/pango"
#make check
#
###sdl
###TODO: Find how to test this, if possible
##mkdir_and_move "$IntDir/sdl"
##make check
#
##libogg
#mkdir_and_move "$IntDir/libogg"
#make check
#
##libvorbis
#mkdir_and_move "$IntDir/libvorbis"
#make check
#
##libcelt
#mkdir_and_move "$IntDir/libcelt"
#make check
#
##libtheora
#mkdir_and_move "$IntDir/libtheora"
#make check
#
##libmms
#mkdir_and_move "$IntDir/libmms"
#make check
#
##x264
#unpack_bzip2_and_move "x264.tar.bz2" "$PKG_DIR_X264"
#cd "$PKG_DIR/"
#wget "http://samples.mplayerhq.hu/V-codecs/h264/x264.avi"
#make test VIDS="x264.avi"
#
##libspeex
#mkdir_and_move "$IntDir/libspeex"
#make check
#
##libschroedinger
##TODO: Some tests are failing
#mkdir_and_move "$IntDir/libschroedinger"
#make check
#
##mp3lame
#mkdir_and_move "$IntDir/mp3lame"
#make check
#
#ffmpeg
#mkdir_and_move "$IntDir/ffmpeg"
#make check



reset_flags


#Cleanup CRT
crt_shutdown

#Call common shutdown routines
common_shutdown

