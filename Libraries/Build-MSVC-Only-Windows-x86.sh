#!/bin/sh

TOP=$(dirname $0)/..

#Call common startup routines to load a bunch of variables we'll need
. $TOP/Shared/Scripts/Common.sh
common_startup "Windows" "Win32" "Release"

#Move to intermediate directory
cd "$IntDir"

#Make sure that the tools are in the path
#For libraries such as ffmpeg to correctly build .lib files, you must make sure that 
#lib.exe is in the path. The following should do the trick (if you have VS 2008 installed).
cd "$TOOLS_DIR"
VSCOMNTOOLS=`cmd.exe /c "msysPath.bat \"$VS90COMNTOOLS\""`
MSVCIDE=`cd "$VSCOMNTOOLS" && cd "../IDE/" && pwd`
MSVCTOOLS=`cd "$VSCOMNTOOLS" && cd "../../VC/bin/" && pwd`
PATH=$PATH:$MSVCIDE:$MSVCTOOLS
MSLIB=lib.exe


#liboil
if [ ! -f "$BinDir/liboil-0.3-0.dll" ]; then 
	mkdir_and_move "$IntDir/liboil"
	
	CFLAGS=$ORIG_CFLAGS
	CPPFLAGS=$ORIG_CPPFLAGS
	
	$LIBRARIES_DIR/LibOil/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	reset_flags
	
	make && make install
	
	$MSLIB /out:liboil-0.3-0.lib /machine:x86 /def:liboil/.libs/liboil-0.3-0.dll.def
	
	move_files_to_dir "*.exp" "$LibDir/"
	move_files_to_dir "*.lib" "$LibDir/"
fi

#libgpg-error
if [ ! -f "$BinDir/libgpg-error-0.dll" ]; then 
	mkdir_and_move "$IntDir/libgpg-error"
	
	CFLAGS=$ORIG_CFLAGS
	CPPFLAGS=$ORIG_CPPFLAGS
	
	$LIBRARIES_DIR/LibGPG-Error/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	#This file was being incorrectly linked. The script points to src/versioninfo.o when it should be 
	#src/.libs/versioninfo.o. This attempts to correct this simply by copying the .o file to the src/ dir.
	cd src
	make versioninfo.o
	cp .libs/versioninfo.o .
	cd ..
	
	#For whatever reason, CPPFLAGS is still being set. We don't want that so ensure it's blank...
	make CPPFLAGS=
	make install
	
	$MSLIB /out:libgpg-error-0.lib /machine:x86 /def:src/.libs/libgpg-error-0.dll.def
	
	move_files_to_dir "*.exp" "$LibDir/"
	move_files_to_dir "*.lib" "$LibDir/"
	
	reset_flags
fi

#libgcrypt
if [ ! -f "$BinDir/libgcrypt-11.dll" ]; then 
	mkdir_and_move "$IntDir/libgcrypt"
	
	CFLAGS=$ORIG_CFLAGS
	CPPFLAGS=$ORIG_CPPFLAGS
	
	#$LIBRARIES_DIR/LibGCrypt/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	reset_flags
	
	make && make install
	
	$MSLIB /out:libgcrypt-11.lib /machine:x86 /def:src/.libs/libgcrypt-11.dll.def
	
	move_files_to_dir "*.exp" "$LibDir/"
	move_files_to_dir "*.lib" "$LibDir/"
fi

#gnutls
if [ ! -f "$BinDir/libgnutls-26.dll" ]; then 
	mkdir_and_move "$IntDir/gnutls"
	
	CFLAGS=
	CPPFLAGS=
	LDFLAGS=
	
	$LIBRARIES_DIR/GnuTLS/Source/configure --with-included-libtasn1 --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	make && make install
	
	$MSLIB /out:libgnutls-26.lib /machine:x86 /def:lib/libgnutls-26.def
	$MSLIB /out:libgnutls-extra-26.lib /machine:x86 /def:libextra/libgnutls-extra-26.def
	$MSLIB /out:libgnutls-openssl-26.lib /machine:x86 /def:libextra/libgnutls-openssl-26.def
	
	move_files_to_dir "*.exp" "$LibDir/"
	move_files_to_dir "*.lib" "$LibDir/"
	
	reset_flags
fi


#ffmpeg
if [ ! -f "$BinDir/avcodec-52.dll" ]; then 
	mkdir_and_move "$IntDir/ffmpeg"
	
	CFLAGS=$ORIG_CFLAGS
	CPPFLAGS=$ORIG_CPPFLAGS
	
	#LGPL-compatible version
	$LIBRARIES_DIR/FFmpeg/Source/configure --extra-ldflags="-no-undefined" --extra-cflags="-mno-cygwin -mms-bitfields -fno-common" --enable-avisynth --enable-memalign-hack --target-os=mingw32 --enable-avfilter-lavf --enable-avfilter --disable-vhook --enable-zlib --enable-w32threads --enable-ipv6 --disable-ffmpeg --disable-ffplay --disable-ffserver --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$LibDir --shlibdir=$LibDir --incdir=$IncludeDir
	
	reset_flags
	
	make && make install
	
	#If it built successfully, then the .lib and .dll files are all in the lib/ folder with 
	#sym links. We want to take out the sym links and keep just the .lib and .dll files we need 
	#for development and execution.
	
	mkdir -p "$LibDir/tmp/"
	
	remove_files_from_dir "$LibDir/av*-*.*.dll"
	move_files_to_dir "$LibDir/av*-*.dll" "$LibDir/tmp/"
	remove_files_from_dir "$LibDir/av*.dll"
	move_files_to_dir "$LibDir/tmp/av*-*.dll" "$BinDir/"
	
	remove_files_from_dir "$LibDir/av*-*.*.lib"
	move_files_to_dir "$LibDir/av*-*.lib" "$LibDir/tmp/"
	remove_files_from_dir "$LibDir/av*.lib"
	move_files_to_dir "$LibDir/tmp/av*-*.lib" "$LibDir/"
	
	rm -rf "$LibDir/tmp/"
fi



#Call common shutdown routines
common_shutdown
