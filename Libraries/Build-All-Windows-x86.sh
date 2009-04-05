#!/bin/sh

TOP=$(dirname $0)/..

#Call common startup routines to load a bunch of variables we'll need
. $TOP/Shared/Scripts/Common.sh
common_startup "Windows" "Win32" "Release" "x86"

#Select which MS CRT we want to build against (msvcr90.dll)
. $ROOT/Shared/Scripts/CRT-x86.sh
crt_startup

#Move to intermediate directory
cd "$IntDir"

#Add MS build tools
setup_ms_build_env_path

#clear_flags


#proxy-libintl
if [ ! -f "$LibDir/intl.lib" ]; then 
	mkdir_and_move "$IntDir/proxy-libintl"
	copy_files_to_dir "$LIBRARIES_DIR/Proxy-LibIntl/Source/*" .
	gcc -Wall -I"$IncludeDir" -c libintl.c
	ar rc "$LibDir/libintl.a" libintl.o
	cp "$LibDir/libintl.a" "$LibDir/intl.lib"
	strip --strip-unneeded "$LibDir/intl.lib"
	cp libintl.h "$IncludeDir"
fi

#Update flags to make sure we don't try and export intl (gettext) functions more than once
#Setting it to ORIG_LDFLAGS ensures that any calls to reset_flags will include these changes.
ORIG_LDFLAGS="$ORIG_LDFLAGS -Wl,--exclude-libs=libintl.a"
reset_flags

#liboil
if [ ! -f "$BinDir/liboil-0.3-0.dll" ]; then 
	mkdir_and_move "$IntDir/liboil"
	
	$LIBRARIES_DIR/LibOil/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	$MSLIB /name:liboil-0.3-0.dll /out:oil-0.3.lib /machine:$MSLibMachine /def:liboil/.libs/liboil-0.3-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
fi

#pthreads
if [ ! -f "$BinDir/pthreadGC2.dll" ]; then 
	mkdir_and_move "$IntDir/pthreads"
	cd "$LIBRARIES_DIR/PThreads/Source"
	make GC-inlined
	$MSLIB /name:pthreadGC2.dll /out:pthreadGC2.lib /machine:$MSLibMachine /def:pthread.def
	move_files_to_dir "*.exp *.lib *.a" "$LibDir"
	move_files_to_dir "*.dll" "$BinDir"
	copy_files_to_dir "pthread.h" "$IncludeDir"
	make clean
fi

#win-iconv
if [ ! -f "$LibDir/iconv.lib" ]; then 
	mkdir_and_move "$IntDir/win-iconv"
	copy_files_to_dir "$LIBRARIES_DIR/Win-Iconv/Source/*.c $LIBRARIES_DIR/Win-Iconv/Source/*.h" .
	
	gcc -I"$IncludeDir" -O2 -c win_iconv.c
	ar crv libiconv.a win_iconv.o
	#gcc $(CFLAGS) -O2 -shared -o iconv.dll win_iconv_dll.c
	dlltool --export-all-symbols -D iconv.dll -l libiconv.dll.a -z in.def libiconv.a
	ranlib libiconv.dll.a
	gcc -shared -s -mwindows -def in.def -o iconv.dll libiconv.a
	cp iconv.h "$IncludeDir"
	
	$MSLIB /name:iconv.dll /out:iconv.lib /machine:$MSLibMachine /def:in.def
	move_files_to_dir "*.dll" "$BinDir"
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	copy_files_to_dir "*.dll.a" "$LibDir"
fi

#zlib
#Can't use separate build dir
if [ ! -f "$BinDir/zlib1.dll" ]; then 
	mkdir_and_move "$IntDir/zlib"
	cd "$LIBRARIES_DIR/ZLib/Source"
	
	#cp contrib/asm686/match.S ./match.S
	#make LOC=-DASMV OBJA=match.o -fwin32/Makefile.gcc
	make -fwin32/Makefile.gcc zlib1.dll
	INCLUDE_PATH=$IncludeDir LIBRARY_PATH=$BinDir make install -fwin32/Makefile.gcc
	cp -p zlib1.dll "$BinDir"
	make clean -fwin32/Makefile.gcc
	
	mv "$BinDir/libzdll.a" "$LibDir/libz.dll.a"
	rm -f "$BinDir/libz.a"
	
	$MSLIB /name:zlib1.dll /out:z.lib /machine:$MSLibMachine /def:win32/zlib.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
fi

#bzip2
if [ ! -f "$BinDir/libbz2.dll" ]; then 
	mkdir_and_move "$IntDir/bzip2"
	
	cd "$LIBRARIES_DIR/BZip2/Source"
	
	make
	make -f Makefile-libbz2_so
	make install PREFIX=$InstallDir
	
	cp -p libbz2.a "$LibDir/libbz2.a"
	cp -p libbz2.so.1.0 "$BinDir/libbz2.dll"
	
	make clean
	remove_files_from_dir "*.exe"
	remove_files_from_dir "*.so.*"
	
	$MSLIB /name:libbz2.dll /out:bz2.lib /machine:$MSLibMachine /def:libbz2.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	copy_files_to_dir "bzlib.h" "$IncludeDir"
fi

#libxml2
if [ ! -f "$BinDir/libxml2-2.dll" ]; then 
	mkdir_and_move "$IntDir/libxml2"
	
	#cd "$LIBRARIES_DIR/LibXML2/Source/win32"
	#cscript configure.js iconv=no xml_debug=no static=yes debug=no threads=native zlib=yes vcmanifest=no compiler=mingw "prefix=$InstallDir" "incdir=$IncludeDir" "libdir=$LibDir" "sodir=$BinDir"
	
	#Compiling with iconv causes an odd dependency on "in.dll" which I think is an intermediary for iconv.a
	$LIBRARIES_DIR/LibXML2/Source/configure --with-zlib --with-iconv --with-threads=native --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	#We want to ignore pthreads and use native threads. Configure does not 
	#respect our desire so we manipulate the config.h file.
	sed -r s/HAVE_PTHREAD_H/HAVE_WIN32_THREADS/g config.h > tmp1.txt
	sed -r s/HAVE_LIBPTHREAD/HAVE_WIN32/g tmp1.txt > tmp.txt
	rm -f tmp1.txt
	mv tmp.txt config.h
	
	#Build forgets to reference testapi.c correctly
	cp $LIBRARIES_DIR/LibXML2/Source/testapi.c .
	
	make && make install
	
	#Preprocess-only the .def.src file
	#The preprocessor generates some odd "# 1" statements so we want to eliminate those
	CFLAGS="$CFLAGS -I$IncludeDir/libxml2 -I$SharedIncludeDir/libxml2"
	gcc $CFLAGS -x c -E -D _REENTRANT $LIBRARIES_DIR/LibXML2/Source/win32/libxml2.def.src > tmp1.txt
	sed '/# /d' tmp1.txt > tmp2.txt
	sed '/LIBRARY libxml2/d' tmp2.txt > libxml2.def
	reset_flags
	
	#Use the output .def file to generate an MS-compatible lib file
	$MSLIB /name:libxml2-2.dll /out:xml2.lib /machine:$MSLibMachine /def:libxml2.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
fi

#libjpeg
if [ ! -f "$BinDir/libjpeg.dll" ]; then 
	mkdir_and_move "$IntDir/libjpeg"
	
	CFLAGS="$CFLAGS -02"
	
	#Configure, compile, and install
	$LIBRARIES_DIR/LibJPEG/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make
	dlltool --export-all-symbols -D jpeg.dll -l libjpeg.dll.a -z in.def .libs/libjpeg.a
	ranlib libjpeg.dll.a
	gcc -shared -s -mwindows -def in.def -o libjpeg.dll .libs/libjpeg.a
	make install-headers
	$MSLIB /name:libjpeg.dll /out:jpeg.lib /machine:$MSLibMachine /def:in.def
	move_files_to_dir "*.dll" "$BinDir"
	move_files_to_dir "*.exp *.lib *.a" "$LibDir"
	
	reset_flags
fi

#libpng
if [ ! -f "$BinDir/libpng12-0.dll" ]; then 
	mkdir_and_move "$IntDir/libpng"
	
	$LIBRARIES_DIR/LibPNG/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	remove_files_from_dir "$BinDir/libpng-3.dll"
	remove_files_from_dir "$LibDir/libpng.dll.a"
	
	cd "$LIBRARIES_DIR/LibPNG/Source/scripts/"
	$MSLIB /name:libpng12-0.dll /out:png12.lib /machine:$MSLibMachine /def:pngw32.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
fi

#glib
if [ ! -f "$BinDir/libglib-2.0-0.dll" ]; then 
	mkdir_and_move "$IntDir/glib"
	
	#Need to get rid of MS build tools b/c the makefile call is incorrectly passing it msys-style paths.
	reset_path
	
	$LIBRARIES_DIR/GLib/Source/configure --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	#Add in MS build tools again
	setup_ms_build_env_path
	
	cd "$LibDir"
	$MSLIB /name:libgio-2.0-0.dll /out:gio-2.0.lib /machine:$MSLibMachine /def:gio-2.0.def
	$MSLIB /name:libglib-2.0-0.dll /out:glib-2.0.lib /machine:$MSLibMachine /def:glib-2.0.def
	$MSLIB /name:libgmodule-2.0-0.dll /out:gmodule-2.0.lib /machine:$MSLibMachine /def:gmodule-2.0.def
	$MSLIB /name:libgobject-2.0-0.dll /out:gobject-2.0.lib /machine:$MSLibMachine /def:gobject-2.0.def
	$MSLIB /name:libgthread-2.0-0.dll /out:gthread-2.0.lib /machine:$MSLibMachine /def:gthread-2.0.def
fi

#openssl
#if [ ! -f "$LibDir/libcrypto.so" ]; then 
#	mkdir_and_move "$IntDir/openssl"
#	cd "$LIBRARIES_DIR/OpenSSL/Source/"
#	
#	echo Building OpenSSL...
#	cp ../mingw.bat ms/
#	cmd.exe /c "ms\mingw.bat"
#	rm -f ms/mingw.bat
#fi

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
	
	$MSLIB /name:libgpg-error-0.dll /out:gpg-error.lib /machine:$MSLibMachine /def:src/.libs/libgpg-error-0.dll.def
	
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
fi

#libgcrypt
if [ ! -f "$BinDir/libgcrypt-11.dll" ]; then 
	mkdir_and_move "$IntDir/libgcrypt"
	
	CFLAGS=$ORIG_CFLAGS
	CPPFLAGS=$ORIG_CPPFLAGS
	
	$LIBRARIES_DIR/LibGCrypt/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	reset_flags
	
	make && make install
	
	$MSLIB /name:libgcrypt-11.dll /out:gcrypt.lib /machine:$MSLibMachine /def:src/.libs/libgcrypt-11.dll.def
	
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#gnutls
if [ ! -f "$BinDir/libgnutls-26.dll" ]; then 
	mkdir_and_move "$IntDir/gnutls"
	
	CFLAGS=
	CPPFLAGS=
	LDFLAGS=
	
	$LIBRARIES_DIR/GnuTLS/Source/configure --with-included-libtasn1 --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	make && make install
	
	$MSLIB /name:libgnutls-26.dll /out:gnutls.lib /machine:$MSLibMachine /def:lib/libgnutls-26.def
	$MSLIB /name:libgnutls-extra-26.dll /out:gnutls-extra.lib /machine:$MSLibMachine /def:libextra/libgnutls-extra-26.def
	$MSLIB /name:libgnutls-openssl-26.dll /out:gnutls-openssl.lib /machine:$MSLibMachine /def:libextra/libgnutls-openssl-26.def
	
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
fi

#soup
if [ ! -f "$BinDir/libsoup-2.4-1.dll" ]; then 
	mkdir_and_move "$IntDir/libsoup"
	
	$LIBRARIES_DIR/LibSoup/Source/configure --disable-glibtest --enable-ssl --enable-debug=no --disable-more-warnings --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install

	cd "$LIBRARIES_DIR/LibSoup"
	$MSLIB /name:libsoup-2.4-1.dll /out:soup-2.4.lib /machine:$MSLibMachine /def:libsoup.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#neon
if [ ! -f "$BinDir/libneon-27.dll" ]; then 
	mkdir_and_move "$IntDir/neon"
	
	$LIBRARIES_DIR/Neon/Source/configure --with-libxml2 --with-ssl=gnutls --disable-webdav --disable-debug --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	$MSLIB /name:libneon-27.dll /out:neon.lib /machine:$MSLibMachine /def:src/.libs/libneon-27.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
fi

#freetype
if [ ! -f "$BinDir/libfreetype-6.dll" ]; then 
	mkdir_and_move "$IntDir/freetype"
	
	cp -f "$LIBRARIES_DIR/FreeType/ftoption_original.h" "$LIBRARIES_DIR/FreeType/Source/include/freetype/config/ftoption.h"
	$LIBRARIES_DIR/FreeType/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	cp -f "$LIBRARIES_DIR/FreeType/ftoption_windows.h" "$LIBRARIES_DIR/FreeType/Source/include/freetype/config/ftoption.h"	
	
	cp -p .libs/libfreetype.dll.a "$LibDir/freetype.lib"
fi

#fontconfig
if [ ! -f "$BinDir/libfontconfig-1.dll" ]; then 
	mkdir_and_move "$IntDir/fontconfig"
	
	$LIBRARIES_DIR/FontConfig/Source/configure --enable-libxml2 --disable-debug --disable-static --disable-docs --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install RUN_FC_CACHE_TEST=false
	copy_files_to_dir "fc-cache/fc-cache.exe fc-arch/fc-arch.exe fc-case/fc-case.exe fc-cat/fc-cat.exe fc-glyphname/fc-glyphname.exe fc-lang/fc-lang.exe fc-list/fc-list.exe fc-match/fc-match.exe" "$BinDir"	
	
	#Automagically creates .lib file
fi

#pixman
if [ ! -f "$BinDir/libpixman-1-0.dll" ]; then 
	mkdir_and_move "$IntDir/pixman"
	
	$LIBRARIES_DIR/Pixman/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	cd "$LIBRARIES_DIR/Pixman/"
	$MSLIB /name:libpixman-1-0.dll /out:pixman.lib /machine:$MSLibMachine /def:pixman.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#cairo
if [ ! -f "$BinDir/libcairo-2.dll" ]; then 
	mkdir_and_move "$IntDir/cairo"
	
	CFLAGS="$CFLAGS -D CAIRO_HAS_WIN32_SURFACE -D CAIRO_HAS_WIN32_FONT"
	$LIBRARIES_DIR/Cairo/Source/configure --enable-xlib=auto --enable-xlib-xrender=auto --enable-png=yes --enable-ft=yes --enable-pdf=yes --enable-svg=yes --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	make && make install
	
	cd src/.libs/
	$MSLIB /name:libcairo-2.dll /out:cairo.lib /machine:$MSLibMachine /def:libcairo-2.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
fi

#pango
if [ ! -f "$BinDir/libpango-1.0-0.dll" ]; then 
	mkdir_and_move "$IntDir/pango"
	
	#Need to get rid of MS build tools b/c the makefile call is incorrectly passing it msys-style paths.
	reset_path
	
	$LIBRARIES_DIR/Pango/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	#Add in MS build tools again
	setup_ms_build_env_path
	
	cd "$IntDir/pango/pango/.libs/"
	$MSLIB /name:libpango-1.0-0.dll /out:pango-1.0.lib /machine:$MSLibMachine /def:libpango-1.0-0.dll.def
	$MSLIB /name:libpangoft2-1.0-0.dll /out:pangoft2-1.0.lib /machine:$MSLibMachine /def:libpangoft2-1.0-0.dll.def
	$MSLIB /name:libpangowin32-1.0-0.dll /out:pangowin32-1.0.lib /machine:$MSLibMachine /def:libpangowin32-1.0-0.dll.def
	$MSLIB /name:libpangocairo-1.0-0.dll /out:pangocairo-1.0.lib /machine:$MSLibMachine /def:libpangocairo-1.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	mkdir -p "$BinDir/pango/1.6.0/modules"
	move_files_to_dir "$LibDir/pango/1.6.0/modules/*.dll" "$BinDir/pango/1.6.0/modules"
fi

#libogg
if [ ! -f "$BinDir/libogg-0.dll" ]; then 
	mkdir_and_move "$IntDir/libogg"
	
	$LIBRARIES_DIR/LibOgg/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	copy_files_to_dir "$LIBRARIES_DIR/LibOgg/Source/win32/ogg.def" .
	$MSLIB /name:libogg-0.dll /out:ogg.lib /machine:$MSLibMachine /def:ogg.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#libvorbis
if [ ! -f "$BinDir/libvorbis-0.dll" ]; then 
	mkdir_and_move "$IntDir/libvorbis"
	
	LDFLAGS="$LDFLAGS -logg"
	$LIBRARIES_DIR/LibVorbis/Source/configure --disable-oggtest --with-ogg-libraries=$LibDir --with-ogg-includes=$IncludeDir --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	copy_files_to_dir "$LIBRARIES_DIR/LibVorbis/Source/win32/*.def" .
	sed '/vorbis_encode_*/d' vorbis.def > vorbis-mod.def
	$MSLIB /name:libvorbis-0.dll /out:vorbis.lib /machine:$MSLibMachine /def:vorbis-mod.def
	$MSLIB /name:libvorbisenc-2.dll /out:vorbisenc.lib /machine:$MSLibMachine /def:vorbisenc.def
	$MSLIB /name:libvorbisfile-3.dll /out:vorbisfile.lib /machine:$MSLibMachine /def:vorbisfile.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
fi

#libtheora
if [ ! -f "$BinDir/libtheora-0.dll" ]; then 
	mkdir_and_move "$IntDir/libtheora"
	
	$LIBRARIES_DIR/LibTheora/Source/configure --disable-examples --disable-oggtest --disable-vorbistest --disable-sdltest --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	copy_files_to_dir "$LIBRARIES_DIR/LibTheora/Source/win32/*.def" .
	copy_files_to_dir "lib/.libs/*.def" .
	sed '/LIBRARY	libtheora/d' libtheora.def > libtheora-mod.def
	$MSLIB /name:libtheora-0.dll /out:theora.lib /machine:$MSLibMachine /def:libtheora-mod.def
	$MSLIB /name:libtheoradec-1.dll /out:theoradec.lib /machine:$MSLibMachine /def:libtheoradec-1.dll.def
	$MSLIB /name:libtheoraenc-1.dll /out:theoraenc.lib /machine:$MSLibMachine /def:libtheoraenc-1.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#libmms
if [ ! -f "$BinDir/libmms-0.dll" ]; then 
	mkdir_and_move "$IntDir/libmms"
	
	CFLAGS="$CFLAGS -D LIBMMS_HAVE_64BIT_OFF_T"
	LDFLAGS="$LDFLAGS -lwsock32 -lglib-2.0 -lgobject-2.0"
	$LIBRARIES_DIR/LibMMS/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	make && make install
	
	copy_files_to_dir "$LIBRARIES_DIR/LibMMS/*.def" .
	$MSLIB /name:libmms-0.dll /out:mms.lib /machine:$MSLibMachine /def:libmms.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
fi

#x264
if [ ! -f "$BinDir/libx264-67.dll" ]; then 
	mkdir_and_move "$IntDir/x264"
	
	PATH=$PATH:$TOOLS_DIR
	
	cd "$LIBRARIES_DIR/X264/Source/"
	./configure --disable-mp4-output --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	make clean && make distclean
	
	reset_path
	setup_ms_build_env_path
	
	cd "$IntDir/x264"
	rm -rf "$LibDir/libx264.a"
	copy_files_to_dir "$LIBRARIES_DIR/X264/libx264.def" .
	$MSLIB /name:libx264-67.dll /out:x264.lib /machine:$MSLibMachine /def:libx264.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#libspeex
if [ ! -f "$BinDir/libspeex-1.dll" ]; then 
	mkdir_and_move "$IntDir/libspeex"
	
	$LIBRARIES_DIR/LibSpeex/Source/configure --disable-oggtest --enable-sse --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	copy_files_to_dir "$LIBRARIES_DIR/LibSpeex/Source/win32/*.def" .
	$MSLIB /name:libspeex-1.dll /out:speex.lib /machine:$MSLibMachine /def:libspeex.def
	$MSLIB /name:libspeexdsp-1.dll /out:speexdsp.lib /machine:$MSLibMachine /def:libspeexdsp.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#libschroedinger (dirac support)
if [ ! -f "$BinDir/libschroedinger-1.0-0.dll" ]; then 
	mkdir_and_move "$IntDir/libschrodinger"
	
	$LIBRARIES_DIR/LibSchrodinger/Source/configure --with-thread=auto --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	cd "schroedinger/.libs"
	$MSLIB /name:libschroedinger-1.0-0.dll /out:schroedinger-1.0.lib /machine:$MSLibMachine /def:libschroedinger-1.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#Not supported at this time!
#taglib
#if [ ! -f "$BinDir/taglib.dll" ]; then 
#	mkdir_and_move "$IntDir/taglib"
#	
#	cd $LIBRARIES_DIR/TagLib/Source/
#	autoreconf -i -f
#	
#	cd "$IntDir/taglib"
#	$LIBRARIES_DIR/TagLib/Source/configure --disable-nmcheck --disable-final --disable-coverage --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
#	
#	#Overwrite the auto-generated libtool (it's old) w/ a newer one that properly handles dll.a files
#	cp -p "$LIBRARIES_DIR/TagLib/libtool-windows" ./libtool
#	make && make install
#	
#	#copy_files_to_dir "$LIBRARIES_DIR/TagLib/Source/win32/*.def" .
#	#$MSLIB /out:libspeex.lib /machine:$MSLibMachine /def:libspeex.def
#	#move_files_to_dir "*.exp *.lib" "$LibDir/"
#fi

#ffmpeg
if [ ! -f "$BinDir/avcodec-52.dll" ]; then 
	mkdir_and_move "$IntDir/ffmpeg"
	
	CFLAGS=$ORIG_CFLAGS
	CPPFLAGS=$ORIG_CPPFLAGS
	
	#LGPL-compatible version
	#$LIBRARIES_DIR/FFmpeg/Source/configure --extra-ldflags="-no-undefined" --extra-cflags="-mno-cygwin -mms-bitfields -fno-common" --enable-avisynth --enable-memalign-hack --target-os=mingw32 --enable-avfilter-lavf --enable-avfilter --disable-vhook --enable-zlib --enable-w32threads --enable-ipv6 --disable-ffmpeg --disable-ffplay --disable-ffserver --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$LibDir --shlibdir=$LibDir --incdir=$IncludeDir
	$LIBRARIES_DIR/FFmpeg/Source/configure --enable-memalign-hack --disable-vhook --enable-zlib --enable-w32threads --disable-ffmpeg --disable-ffplay --disable-ffserver --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$LibDir --shlibdir=$LibDir --incdir=$IncludeDir
	
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
	
	#Create .dll.a versions of the libs
	cd "$IntDir/ffmpeg"
	pexports "$BinDir/avutil-49.dll" | sed "s/^_//" > avutil.def
	pexports "$BinDir/avcodec-52.dll" | sed "s/^_//" > avcodec.def
	pexports "$BinDir/avdevice-52.dll" | sed "s/^_//" > avdevice.def
	#pexports "$BinDir/avfilter-0.dll" | sed "s/^_//" > avfilter.def
	pexports "$BinDir/avformat-52.dll" | sed "s/^_//" > avformat.def
	dlltool -U -d avutil.def -l libavutil.dll.a
	dlltool -U -d avcodec.def -l libavcodec.dll.a
	dlltool -U -d avdevice.def -l libavdevice.dll.a
	#dlltool -U -d avfilter.def -l libavfilter.dll.a
	dlltool -U -d avformat.def -l libavformat.dll.a
	move_files_to_dir "*.dll.a" "$LibDir/"
	
	cd "$LibDir"
	mv avcodec-52.lib avcodec.lib
	mv avdevice-52.lib avdevice.lib
	#mv avfilter-0.lib avfilter.lib
	mv avformat-52.lib avformat.lib
	mv avutil-49.lib avutil.lib
fi

reset_flags

#Cleanup CRT
crt_shutdown

#Call common shutdown routines
common_shutdown
