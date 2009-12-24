#!/bin/sh

TOP=$(dirname $0)/..

CFLAGS="$CFLAGS -fno-strict-aliasing -D_WIN32_WINNT=0x0501 -DUSE_GETADDRINFO -DHAVE_GETNAMEINFO -DHAVE_GETSOCKOPT -DHAVE_INET_NTOP -DHAVE_INET_PTON"

#Call common startup routines to load a bunch of variables we'll need
. $TOP/Shared/Scripts/Common.sh
common_startup "Windows" "Win32" "Release" "x86"

#Select which MS CRT we want to build against (msvcr90.dll)
. $ROOT/Shared/Scripts/CRT-x86.sh
crt_startup

#Setup library versions
. $ROOT/Shared/Scripts/Version.sh

#Move to intermediate directory
cd "$IntDir"

#Add MS build tools
setup_ms_build_env_path

#clear_flags

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
##pthreads
#unpack_gzip_and_move "pthreads-w32.tar.gz" "$PKG_DIR_PTHREADS"
#cp -p -r "$PKG_QUEUE_USER_APC_EX" "$PKG_DIR"
#cp -p "$BinDir\lib${DefaultPrefix}pthreadGC2.dll" "$PKG_DIR\pthreadGC2.dll"
#cd tests
#make GC
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
###libgpg-error
##mkdir_and_move "$IntDir/libgpg-error"
##make check
##libgcrypt
#mkdir_and_move "$IntDir/libgcrypt"
#make check






reset_flags


#Cleanup CRT
crt_shutdown

#Call common shutdown routines
common_shutdown

