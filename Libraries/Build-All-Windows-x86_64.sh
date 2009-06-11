#!/bin/sh

TOP=$(dirname $0)/..

#Call common startup routines to load a bunch of variables we'll need
. $TOP/Shared/Scripts/Common.sh
common_startup "Windows" "x64" "Release" "x64"

#Select which MS CRT we want to build against (msvcr90.dll)
. $ROOT/Shared/Scripts/CRT-x86_64.sh
crt_startup

#Move to intermediate directory
cd "$IntDir"

#Add MS build tools
setup_ms_build_env_path

#Setup compiler options
HOST=x86_64-pc-mingw32
GCC=$HOST-gcc
STRIP=$HOST-strip
CPP=$GCC
CC=$GCC



#proxy-libintl
if [ ! -f "$LibDir/intl.lib" ]; then 
	mkdir_and_move "$IntDir/proxy-libintl"
	copy_files_to_dir "$LIBRARIES_DIR/Proxy-LibIntl/Source/*" .
	$GCC -Wall -I"$IncludeDir" -c libintl.c
	ar rc "$LibDir/libintl.a" libintl.o
	cp "$LibDir/libintl.a" "$LibDir/intl.lib"
	$STRIP --strip-unneeded "$LibDir/intl.lib"
	cp libintl.h "$IncludeDir"
fi

#Update flags to make sure we don't try and export intl (gettext) functions more than once
#Setting it to ORIG_LDFLAGS ensures that any calls to reset_flags will include these changes.
ORIG_LDFLAGS="$ORIG_LDFLAGS -Wl,--exclude-libs=libintl.a"
reset_flags

#liboil
if [ ! -f "$BinDir/liboil-0.3-0.dll" ]; then 
	mkdir_and_move "$IntDir/liboil"
	
	as_cv_unaligned_access="yes"
	$LIBRARIES_DIR/LibOil/Source/configure --host=$HOST --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	#make && make install
	as_cv_unaligned_access=""
	
	#$MSLIB /name:liboil-0.3-0.dll /out:oil-0.3.lib /machine:$MSLibMachine /def:liboil/.libs/liboil-0.3-0.dll.def
	#move_files_to_dir "*.exp *.lib" "$LibDir"
fi



reset_flags

#Cleanup CRT
crt_shutdown

#Call common shutdown routines
common_shutdown
