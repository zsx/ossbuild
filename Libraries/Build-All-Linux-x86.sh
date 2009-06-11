#!/bin/sh

TOP=$(dirname $0)/..

#Call common startup routines to load a bunch of variables we'll need
. $TOP/Shared/Scripts/Common.sh
common_startup "Linux" "x86" "Release"

#Setup library versions
. $ROOT/Shared/Scripts/Version.sh

#Move to intermediate directory
#cd "$IntDir"

#clear_flags

#Causes us to now always include the bin dir to look for libraries, even after calling reset_flags
export ORIG_LDFLAGS="$ORIG_LDFLAGS -L$BinDir -L$SharedBinDir"
reset_flags

#Here's where the magic really happens


#liboil
if [ ! -f "$BinDir/liboil-0.3.so" ]; then 
	unpack_gzip_and_move "liboil.tar.gz" "$PKG_DIR_LIBOIL"
	mkdir_and_move "$IntDir/liboil"

	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "liboil-0.3.so" "0" "0.3.0" "liboil-0.3.la" "liboil-0.3.pc" "$LibDir"
fi

#zlib
#Can't use separate build dir
if [ ! -f "$BinDir/libz.so" ]; then 
	unpack_zip_and_move_linux "zlib.zip" "zlib" "zlib"
	mkdir_and_move "$IntDir/zlib"
	cd "$PKG_DIR"

	chmod u+x ./configure
	dos2unix ./configure
	"$PKG_DIR/configure" -s --shared --prefix=$InstallDir --exec_prefix=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	
	cp contrib/asm686/match.S ./match.S
	make LOC=-DASMV OBJA=match.o
	make libz.a
	make install
	cp libz.a "$LibDir/libz.a"
	make clean
	make distclean
	rm -f match.S
	
	arrange_shared "$BinDir" "libz.so" "1" "1.2.3"
fi

#bzip2
if [ ! -f "$BinDir/libbz2.so" ]; then 
	unpack_gzip_and_move "bzip2.tar.gz" "$PKG_DIR_BZIP2"
	cd "$PKG_DIR"

	make
	make -f Makefile-libbz2_so
	make install PREFIX=$InstallDir
	
	cp -p libbz2.a "$LibDir/libbz2.a"
	copy_files_to_dir "libbz2.so.1.0" "$BinDir"
	arrange_shared "$BinDir" "libbz2.so" "1" "1.0"
	
	make clean
	remove_files_from_dir "*.so.* bzip2-shared"
fi

#Skip pthreads

#libxml2
if [ ! -f "$BinDir/libxml2.so" ]; then 
	unpack_gzip_and_move "libxml2.tar.gz" "$PKG_DIR_LIBXML2"
	mkdir_and_move "$IntDir/libxml2"
	
	CFLAGS="$CFLAGS -O2"
	$PKG_DIR/configure --with-zlib --with-threads=native --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	reset_flags

	arrange_shared "$BinDir" "libxml2.so" "2" "2.7.3" "libxml2.la" "libxml-2.0.pc" "$LibDir"
fi

#libjpeg
if [ ! -f "$BinDir/libjpeg.so" ]; then 
	unpack_gzip_and_move "jpegsrc.tar.gz" "$PKG_DIR_LIBJPEG"
	mkdir_and_move "$IntDir/libjpeg"
	
	$PKG_DIR/configure --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	move_files_to_dir "$LibDir/libjpeg*" "$BinDir"
	arrange_shared "$BinDir" "libjpeg.so" "62" "62.0.0" "libjpeg.la" "" "$LibDir"
fi

#libpng
if [ ! -f "$BinDir/libpng12.so" ]; then 
	unpack_gzip_and_move "libpng.tar.gz" "$PKG_DIR_LIBPNG"
	mkdir_and_move "$IntDir/libpng"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libpng12.so" "0" "0.35.0" "libpng12.la" "libpng12.pc" "$LibDir"

	cd "$BinDir"
	move_files_to_dir "libpng12.a" "$LibDir"
	move_files_to_dir "pkgconfig/libpng.pc" "$LibDir/pkgconfig/"
	remove_files_from_dir "libpng.a libpng.la libpng.so*"
fi

#glib
if [ ! -f "$BinDir/libglib-2.0.so" ]; then 
	unpack_gzip_and_move "glib.tar.gz" "$PKG_DIR_GLIB"
	mkdir_and_move "$IntDir/glib"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_shared "$BinDir" "libgio-2.0.so" "0" "0.2000.1" "libgio-2.0.la" "gio-2.0.pc gio-unix-2.0.pc" "$LibDir"
	arrange_shared "$BinDir" "libglib-2.0.so" "0" "0.2000.1" "libglib-2.0.la" "glib-2.0.pc" "$LibDir"
	arrange_shared "$BinDir" "libgmodule-2.0.so" "0" "0.2000.1" "libgmodule-2.0.la" "gmodule-2.0.pc gmodule-export-2.0.pc gmodule-no-export-2.0.pc" "$LibDir"
	arrange_shared "$BinDir" "libgobject-2.0.so" "0" "0.2000.1" "libgobject-2.0.la" "gobject-2.0.pc" "$LibDir"
	arrange_shared "$BinDir" "libgthread-2.0.so" "0" "0.2000.1" "libgthread-2.0.la" "gthread-2.0.pc" "$LibDir"
	test -d "$LibDir/gio" && rm -rf "$LibDir/gio"
	test -d "$LibDir/glib-2.0" && rm -rf "$LibDir/glib-2.0"
	mv "$BinDir/gio" "$LibDir"
	mv "$BinDir/glib-2.0" "$LibDir"
	cp -p "$LibDir/glib-2.0/include/glibconfig.h" "$IncludeDir/glib-2.0/"
fi
exit 0
#openssl
#if [ ! -f "$BinDir/libcrypto.so" ]; then 
#	mkdir_and_move "$IntDir/openssl"
#	cd "$LIBRARIES_DIR/OpenSSL/Source/"
#	./config threads shared zlib-dynamic --prefix=$InstallDir
#	make build_libs
#	cd apps && make && cd ..
#	make install_sw
#	make clean
#	#Remove generated symlinks that are corrupting subversion commits
#	cd test/
#	find . -maxdepth 1 -lname '*' -exec rm {} \;
#	cd ../
#	cd apps/
#	find . -maxdepth 1 -lname '*' -exec rm {} \;
#	cd ../
#	cd include/openssl/
#	find . -maxdepth 1 -lname '*' -exec rm {} \;
#	cd ../../
#	#move_files_to_dir "$LibDir/libcrypto.so*" "$BinDir"
#	#move_files_to_dir "$LibDir/libssl.so*" "$BinDir"
#fi

#libgpg-error
if [ ! -f "$BinDir/libgpg-error.so" ]; then 
	mkdir_and_move "$IntDir/libgpg-error"
	
	$LIBRARIES_DIR/LibGPG-Error/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install

	move_files_to_dir "$LibDir/libgpg-error*" "$BinDir"
	arrange_shared "$BinDir" "libgpg-error.so" "0" "0.5.0" "libgpg-error.la" "" "$LibDir"
fi

#libgcrypt
if [ ! -f "$BinDir/libgcrypt.so" ]; then 
	mkdir_and_move "$IntDir/libgcrypt"
	
	$LIBRARIES_DIR/LibGCrypt/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libgcrypt.so" "11" "11.5.2" "libgcrypt.la" "" "$LibDir"
fi

#gnutls
if [ ! -f "$BinDir/libgnutls.so" ]; then 
	mkdir_and_move "$IntDir/gnutls"
	
	$LIBRARIES_DIR/GnuTLS/Source/configure --with-included-libtasn1 --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libgnutls.so" "26" "26.11.5" "libgnutls.la" "gnutls.pc" "$LibDir"
	arrange_shared "$BinDir" "libgnutls-extra.so" "26" "26.11.5" "libgnutls-extra.la" "gnutls-extra.pc" "$LibDir"
	arrange_shared "$BinDir" "libgnutlsxx.so" "26" "26.11.5" "libgnutlsxx.la" "" "$LibDir"
	arrange_shared "$BinDir" "libgnutls-openssl.so" "26" "26.11.5" "libgnutls-openssl.la" "" "$LibDir"
fi

#soup
if [ ! -f "$BinDir/libsoup-2.4.so" ]; then 
	mkdir_and_move "$IntDir/libsoup"
	
	$LIBRARIES_DIR/LibSoup/Source/configure --without-gnome --disable-glibtest --enable-ssl --enable-debug=no --disable-more-warnings --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	arrange_shared "$BinDir" "libsoup-2.4.so" "1" "1.2.0" "libsoup-2.4.la" "libsoup-2.4.pc" "$LibDir"
fi

#neon
if [ ! -f "$BinDir/libneon.so" ]; then 
	mkdir_and_move "$IntDir/neon"
	$LIBRARIES_DIR/Neon/Source/configure --with-libxml2 --with-ssl=gnutls --disable-webdav --disable-debug --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	arrange_shared "$BinDir" "libneon.so" "27" "27.1.4" "libneon.la" "neon.pc" "$LibDir"
fi

#freetype
if [ ! -f "$BinDir/libfreetype.so" ]; then 
	mkdir_and_move "$IntDir/freetype"
	cp -f "$LIBRARIES_DIR/FreeType/ftoption_original.h" "$LIBRARIES_DIR/FreeType/Source/include/freetype/config/ftoption.h"
	$LIBRARIES_DIR/FreeType/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	cp -f "$LIBRARIES_DIR/FreeType/ftoption_windows.h" "$LIBRARIES_DIR/FreeType/Source/include/freetype/config/ftoption.h"	
	arrange_shared "$BinDir" "libfreetype.so" "6" "6.3.19" "libfreetype.la" "freetype2.pc" "$LibDir"
fi

#fontconfig
if [ ! -f "$BinDir/libfontconfig.so" ]; then 
	mkdir_and_move "$IntDir/fontconfig"
	$LIBRARIES_DIR/FontConfig/Source/configure --enable-libxml2 --disable-debug --disable-static --disable-docs --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	arrange_shared "$BinDir" "libfontconfig.so" "1" "1.3.0" "libfontconfig.la" "fontconfig.pc" "$LibDir"
fi

#pixman
if [ ! -f "$BinDir/libpixman-1.so" ]; then 
	mkdir_and_move "$IntDir/pixman"
	$LIBRARIES_DIR/Pixman/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	arrange_shared "$BinDir" "libpixman-1.so" "0" "0.14.0" "libpixman-1.la" "pixman-1.pc" "$LibDir"
fi

#cairo
if [ ! -f "$BinDir/libcairo.so" ]; then 
	mkdir_and_move "$IntDir/cairo"
	$LIBRARIES_DIR/Cairo/Source/configure --enable-xlib=auto --enable-xlib-xrender=auto --enable-png=yes --enable-ft=yes --enable-pdf=yes --enable-svg=yes --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	arrange_shared "$BinDir" "libcairo.so" "2" "2.10800.6" "libcairo.la" "cairo*.pc" "$LibDir"
fi

#pango
if [ ! -f "$BinDir/libpango-1.0.so" ]; then 
	mkdir_and_move "$IntDir/pango"
	LDFLAGS="$LDFLAGS -lxml2"
	$LIBRARIES_DIR/Pango/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	reset_flags
	arrange_shared "$BinDir" "libpango-1.0.so" "0" "0.2300.0" "libpango-1.0.la" "pango.pc" "$LibDir"
	arrange_shared "$BinDir" "libpangoft2-1.0.so" "0" "0.2300.0" "libpangoft2-1.0.la" "pangoft2.pc" "$LibDir"
	arrange_shared "$BinDir" "libpangocairo-1.0.so" "0" "0.2300.0" "libpangocairo-1.0.la" "pangocairo.pc" "$LibDir"
	mkdir -p "$LibDir/pango/1.6.0/modules"
	move_files_to_dir "$BinDir/pango/1.6.0/modules/*.la" "$LibDir/pango/1.6.0/modules"
fi

#libogg
if [ ! -f "$BinDir/libogg.so" ]; then 
	mkdir_and_move "$IntDir/libogg"
	$LIBRARIES_DIR/LibOgg/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	arrange_shared "$BinDir" "libogg.so" "0" "0.5.3" "libogg.la" "ogg.pc" "$LibDir"
fi

#libvorbis
if [ ! -f "$BinDir/libvorbis.so" ]; then 
	mkdir_and_move "$IntDir/libvorbis"
	$LIBRARIES_DIR/LibVorbis/Source/configure --disable-oggtest --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	arrange_shared "$BinDir" "libvorbis.so" "0" "0.4.0" "libvorbis.la" "vorbis.pc" "$LibDir"
	arrange_shared "$BinDir" "libvorbisenc.so" "2" "2.0.3" "libvorbisenc.la" "vorbisenc.pc" "$LibDir"
	arrange_shared "$BinDir" "libvorbisfile.so" "3" "3.2.0" "libvorbisfile.la" "vorbisfile.pc" "$LibDir"
fi

#libtheora
if [ ! -f "$BinDir/libtheora.so" ]; then 
	mkdir_and_move "$IntDir/libtheora"
	$LIBRARIES_DIR/LibTheora/Source/configure --disable-examples --disable-oggtest --disable-vorbistest --disable-sdltest --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	arrange_shared "$BinDir" "libtheora.so" "0" "0.3.4" "libtheora.la" "theora.pc" "$LibDir"
	arrange_shared "$BinDir" "libtheoradec.so" "1" "1.0.1" "libtheoradec.la" "theoradec.pc" "$LibDir"
	arrange_shared "$BinDir" "libtheoraenc.so" "1" "1.0.1" "libtheoraenc.la" "theoraenc.pc" "$LibDir"
fi

#libmms
if [ ! -f "$BinDir/libmms.so" ]; then 
	cd "$LIBRARIES_DIR/LibMMS/Source/"
	./autogen.sh
	mkdir_and_move "$IntDir/libmms"
	$LIBRARIES_DIR/LibMMS/Source/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	arrange_shared "$BinDir" "libmms.so" "0" "0.0.2" "libmms.la" "libmms.pc" "$LibDir"
fi

#x264
#For now, it can only decode H.264 (--disable-mp4-output)
#To encode, you'll need gpac and a proper license
if [ ! -f "$BinDir/libx264.so" ]; then 
	mkdir_and_move "$IntDir/x264"
	cd "$LIBRARIES_DIR/X264/Source/"
	./configure --disable-mp4-output --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	make clean && make distclean
	arrange_shared "$BinDir" "libx264.so" "67" "67.0.0" "" "x264.pc" "$LibDir"
	move_files_to_dir "$BinDir/libx264.a" "$LibDir"
fi

#libspeex
if [ ! -f "$BinDir/libspeex.so" ]; then 
	mkdir_and_move "$IntDir/libspeex"
	$LIBRARIES_DIR/LibSpeex/Source/configure --disable-oggtest --enable-sse --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	arrange_shared "$BinDir" "libspeex.so" "1" "1.5.0" "libspeex.la" "speex.pc" "$LibDir"
	arrange_shared "$BinDir" "libspeexdsp.so" "1" "1.5.0" "libspeexdsp.la" "speexdsp.pc" "$LibDir"
fi

#libschrodinger (dirac support)
if [ ! -f "$BinDir/libschroedinger-1.0.so" ]; then 
	mkdir_and_move "$IntDir/libschrodinger"
	$LIBRARIES_DIR/LibSchrodinger/Source/configure --with-thread=auto --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	arrange_shared "$BinDir" "libschroedinger-1.0.so" "0" "0.2.0" "libschroedinger-1.0.la" "schroedinger-1.0.pc" "$LibDir"
fi

#ffmpeg
if [ ! -f "$BinDir/libavcodec.so" ]; then 
	mkdir_and_move "$IntDir/ffmpeg"
	#LGPL-compatible version
	$LIBRARIES_DIR/FFmpeg/Source/configure --extra-ldflags="-no-undefined" --extra-cflags="-fno-common" --disable-avisynth --enable-avfilter-lavf --enable-avfilter --disable-vhook --enable-zlib --enable-pthreads --enable-ipv6 --disable-ffmpeg --disable-ffplay --disable-ffserver --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$BinDir --shlibdir=$BinDir --incdir=$IncludeDir
	make && make install
	arrange_shared "$BinDir" "libavutil.so" "49" "49.15.0" "" "libavutil.pc" "$LibDir"
	arrange_shared "$BinDir" "libavfilter.so" "0" "0.4.0" "" "libavfilter.pc" "$LibDir"
	arrange_shared "$BinDir" "libavcodec.so" "52" "52.20.0" "" "libavcodec.pc" "$LibDir"
	arrange_shared "$BinDir" "libavdevice.so" "52" "52.1.0" "" "libavdevice.pc" "$LibDir"
	arrange_shared "$BinDir" "libavformat.so" "52" "52.31.0" "" "libavformat.pc" "$LibDir"
fi

reset_flags

#Call common shutdown routines
common_shutdown
