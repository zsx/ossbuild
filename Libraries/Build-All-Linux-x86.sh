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
	move_files_to_dir "$BinDir/libjpeg.a" "$LibDir"
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
	unpack_gzip_and_move "libgpg-error.tar.gz" "$PKG_DIR_LIBGPG_ERROR"
	mkdir_and_move "$IntDir/libgpg-error"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install

	move_files_to_dir "$LibDir/libgpg-error*" "$BinDir"
	arrange_shared "$BinDir" "libgpg-error.so" "0" "0.5.0" "libgpg-error.la" "" "$LibDir"
fi

#libgcrypt
if [ ! -f "$BinDir/libgcrypt.so" ]; then 
	unpack_gzip_and_move "libgcrypt.tar.gz" "$PKG_DIR_LIBGCRYPT"
	mkdir_and_move "$IntDir/libgcrypt"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libgcrypt.so" "11" "11.5.2" "libgcrypt.la" "" "$LibDir"
fi

#gnutls
if [ ! -f "$BinDir/libgnutls.so" ]; then 
	unpack_bzip2_and_move "gnutls.tar.bz2" "$PKG_DIR_GNUTLS"
	mkdir_and_move "$IntDir/gnutls"
	
	$PKG_DIR/configure --with-included-libtasn1 --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libgnutls.so" "26" "26.11.6" "libgnutls.la" "gnutls.pc" "$LibDir"
	arrange_shared "$BinDir" "libgnutls-extra.so" "26" "26.11.6" "libgnutls-extra.la" "gnutls-extra.pc" "$LibDir"
	arrange_shared "$BinDir" "libgnutlsxx.so" "26" "26.11.6" "libgnutlsxx.la" "" "$LibDir"
	arrange_shared "$BinDir" "libgnutls-openssl.so" "26" "26.11.6" "libgnutls-openssl.la" "" "$LibDir"
fi

#soup
if [ ! -f "$BinDir/libsoup-2.4.so" ]; then 
	unpack_bzip2_and_move "libsoup.tar.bz2" "$PKG_DIR_LIBSOUP"
	mkdir_and_move "$IntDir/libsoup"
	
	$PKG_DIR/configure --without-gnome --disable-glibtest --enable-ssl --enable-debug=no --disable-more-warnings --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	arrange_shared "$BinDir" "libsoup-2.4.so" "1" "1.2.0" "libsoup-2.4.la" "libsoup-2.4.pc" "$LibDir"
fi

#neon
if [ ! -f "$BinDir/libneon.so" ]; then 
	unpack_gzip_and_move "neon.tar.gz" "$PKG_DIR_NEON"
	mkdir_and_move "$IntDir/neon"
	
	$PKG_DIR/configure --with-libxml2 --with-ssl=gnutls --disable-webdav --disable-debug --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_shared "$BinDir" "libneon.so" "27" "27.1.4" "libneon.la" "neon.pc" "$LibDir"
fi

#freetype
if [ ! -f "$BinDir/libfreetype.so" ]; then 
	unpack_bzip2_and_move "freetype.tar.bz2" "$PKG_DIR_FREETYPE"
	mkdir_and_move "$IntDir/freetype"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libfreetype.so" "6" "6.3.20" "libfreetype.la" "freetype2.pc" "$LibDir"
fi

#fontconfig
if [ ! -f "$BinDir/libfontconfig.so" ]; then 
	unpack_gzip_and_move "fontconfig.tar.gz" "$PKG_DIR_FONTCONFIG"
	mkdir_and_move "$IntDir/fontconfig"
	
	$PKG_DIR/configure --enable-libxml2 --disable-debug --disable-static --disable-docs --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir

	make && make install

	arrange_shared "$BinDir" "libfontconfig.so" "1" "1.3.0" "libfontconfig.la" "fontconfig.pc" "$LibDir"
fi

#pixman
if [ ! -f "$BinDir/libpixman-1.so" ]; then 
	unpack_gzip_and_move "pixman.tar.gz" "$PKG_DIR_PIXMAN"
	mkdir_and_move "$IntDir/pixman"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libpixman-1.so" "0" "0.15.2" "libpixman-1.la" "pixman-1.pc" "$LibDir"
fi

#cairo
if [ ! -f "$BinDir/libcairo.so" ]; then 
	unpack_gzip_and_move "cairo.tar.gz" "$PKG_DIR_CAIRO"
	mkdir_and_move "$IntDir/cairo"
	
	$PKG_DIR/configure --enable-xlib=auto --enable-xlib-xrender=auto --enable-png=yes --enable-ft=yes --enable-pdf=yes --enable-svg=yes --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_shared "$BinDir" "libcairo.so" "2" "2.10800.6" "libcairo.la" "cairo*.pc" "$LibDir"
fi

#pango
if [ ! -f "$BinDir/libpango-1.0.so" ]; then 
	unpack_gzip_and_move "pango.tar.gz" "$PKG_DIR_PANGO"
	mkdir_and_move "$IntDir/pango"
	
	LDFLAGS="$LDFLAGS -lxml2"
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	reset_flags

	arrange_shared "$BinDir" "libpango-1.0.so" "0" "0.2400.1" "libpango-1.0.la" "pango.pc" "$LibDir"
	arrange_shared "$BinDir" "libpangoft2-1.0.so" "0" "0.2400.1" "libpangoft2-1.0.la" "pangoft2.pc" "$LibDir"
	arrange_shared "$BinDir" "libpangocairo-1.0.so" "0" "0.2400.1" "libpangocairo-1.0.la" "pangocairo.pc" "$LibDir"
	mkdir -p "$LibDir/pango/1.6.0/modules"
	move_files_to_dir "$BinDir/pango/1.6.0/modules/*.la" "$LibDir/pango/1.6.0/modules"
fi

#libogg
if [ ! -f "$BinDir/libogg.so" ]; then 
	unpack_gzip_and_move "libogg.tar.gz" "$PKG_DIR_LIBOGG"
	mkdir_and_move "$IntDir/libogg"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libogg.so" "0" "0.5.3" "libogg.la" "ogg.pc" "$LibDir"
fi

#libvorbis
if [ ! -f "$BinDir/libvorbis.so" ]; then 
	unpack_bzip2_and_move "libvorbis.tar.bz2" "$PKG_DIR_LIBVORBIS"
	mkdir_and_move "$IntDir/libvorbis"
	
	$PKG_DIR/configure --disable-oggtest --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_shared "$BinDir" "libvorbis.so" "0" "0.4.0" "libvorbis.la" "vorbis.pc" "$LibDir"
	arrange_shared "$BinDir" "libvorbisenc.so" "2" "2.0.3" "libvorbisenc.la" "vorbisenc.pc" "$LibDir"
	arrange_shared "$BinDir" "libvorbisfile.so" "3" "3.2.0" "libvorbisfile.la" "vorbisfile.pc" "$LibDir"
fi

#libtheora
if [ ! -f "$BinDir/libtheora.so" ]; then 
	unpack_bzip2_and_move "libtheora.tar.bz2" "$PKG_DIR_LIBTHEORA"
	mkdir_and_move "$IntDir/libtheora"
	
	$PKG_DIR/configure --disable-examples --disable-oggtest --disable-vorbistest --disable-sdltest --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_shared "$BinDir" "libtheora.so" "0" "0.3.4" "libtheora.la" "theora.pc" "$LibDir"
	arrange_shared "$BinDir" "libtheoradec.so" "1" "1.0.1" "libtheoradec.la" "theoradec.pc" "$LibDir"
	arrange_shared "$BinDir" "libtheoraenc.so" "1" "1.0.1" "libtheoraenc.la" "theoraenc.pc" "$LibDir"
fi

#libmms
if [ ! -f "$BinDir/libmms.so" ]; then 
	unpack_bzip2_and_move "libmms.tar.bz2" "$PKG_DIR_LIBMMS"
	./autogen.sh
	mkdir_and_move "$IntDir/libmms"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_shared "$BinDir" "libmms.so" "0" "0.0.2" "libmms.la" "libmms.pc" "$LibDir"
fi

#x264
#For now, it can only decode H.264 (--disable-mp4-output)
#To encode, you'll need gpac and a proper license
if [ ! -f "$BinDir/libx264.so" ]; then 
	unpack_bzip2_and_move "x264.tar.bz2" "$PKG_DIR_X264"
	mkdir_and_move "$IntDir/x264"
	
	cd "$PKG_DIR/"
	CFLAGS="$CFLAGS -D X264_CPU_SHUFFLE_IS_FAST=0x000800"
	./configure --disable-mp4-output --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	make clean && make distclean
	reset_flags
	
	arrange_shared "$BinDir" "libx264.so" "67" "67.0.0" "" "x264.pc" "$LibDir"
	move_files_to_dir "$BinDir/libx264.a" "$LibDir"
fi

#libspeex
if [ ! -f "$BinDir/libspeex.so" ]; then 
	unpack_gzip_and_move "speex.tar.gz" "$PKG_DIR_LIBSPEEX"
	mkdir_and_move "$IntDir/libspeex"
	
	$PKG_DIR/configure --disable-oggtest --enable-sse --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_shared "$BinDir" "libspeex.so" "1" "1.5.0" "libspeex.la" "speex.pc" "$LibDir"
	arrange_shared "$BinDir" "libspeexdsp.so" "1" "1.5.0" "libspeexdsp.la" "speexdsp.pc" "$LibDir"
fi

#libschrodinger (dirac support)
if [ ! -f "$BinDir/libschroedinger-1.0.so" ]; then 
	unpack_gzip_and_move "schroedinger.tar.gz" "$PKG_DIR_LIBSCHROEDINGER"
	mkdir_and_move "$IntDir/libschroedinger"
	
	$PKG_DIR/configure --with-thread=auto --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_shared "$BinDir" "libschroedinger-1.0.so" "0" "0.2.0" "libschroedinger-1.0.la" "schroedinger-1.0.pc" "$LibDir"
fi

#openjpeg
if [ ! -f "$BinDir/libopenjpeg.so" ]; then 
	unpack_gzip_and_move "openjpeg.tar.gz" "$PKG_DIR_OPENJPEG"
	mkdir_and_move "$IntDir/openjpeg"
	
	cd "$PKG_DIR"
	cp -f "$LIBRARIES_PATCH_DIR/openjpeg/Makefile.linux" ./Makefile
	make install PREFIX=$InstallDir
	make clean
	
	cd "$BinDir"
	mv "$LibDir/libopenjpeg-2.1.3.0.so" "./libopenjpeg.so.2"
	rm -f "$LibDir/libopenjpeg.so.2"
	ln -s "libopenjpeg.so.2" "libopenjpeg.so"
	move_files_to_dir "$BinDir/libopenjpeg.a" "$LibDir"
fi

#mp3lame
if [ ! -f "$BinDir/libmp3lame.so" ]; then 
	unpack_gzip_and_move "lame.tar.gz" "$PKG_DIR_MP3LAME"
	mkdir_and_move "$IntDir/mp3lame"
	
	$PKG_DIR/configure --enable-expopt=no --enable-debug=no --disable-brhist -disable-frontend --enable-nasm --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_shared "$BinDir" "libmp3lame.so" "0" "0.0.0" "libmp3lame.la" "" "$LibDir"
fi

#ffmpeg
if [ ! -f "$BinDir/libavcodec.so" ]; then 
	unpack_bzip2_and_move "ffmpeg.tar.bz2" "$PKG_DIR_FFMPEG"
	mkdir_and_move "$IntDir/ffmpeg"
	
	#LGPL-compatible version
	$PKG_DIR/configure --extra-ldflags="-no-undefined" --extra-cflags="-fno-common" --disable-avisynth --enable-avfilter-lavf --enable-avfilter --arch=i686 --cpu=i686 --disable-vhook --enable-zlib --enable-bzlib --enable-pthreads --enable-ipv6 --enable-libmp3lame --enable-libvorbis --enable-libopenjpeg --enable-libtheora --enable-libspeex --enable-libschroedinger --disable-ffmpeg --disable-ffplay --disable-ffserver --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$BinDir --shlibdir=$BinDir --incdir=$IncludeDir
	make && make install
	
	arrange_shared "$BinDir" "libavutil.so" "49" "49.15.0" "" "libavutil.pc" "$LibDir"
	arrange_shared "$BinDir" "libavfilter.so" "0" "0.4.0" "" "libavfilter.pc" "$LibDir"
	arrange_shared "$BinDir" "libavcodec.so" "52" "52.20.0" "" "libavcodec.pc" "$LibDir"
	arrange_shared "$BinDir" "libavdevice.so" "52" "52.1.0" "" "libavdevice.pc" "$LibDir"
	arrange_shared "$BinDir" "libavformat.so" "52" "52.31.0" "" "libavformat.pc" "$LibDir"
fi

#sdl
if [ ! -f "$BinDir/libSDL-1.2.so" ]; then 
	unpack_gzip_and_move "sdl.tar.gz" "$PKG_DIR_SDL"
	mkdir_and_move "$IntDir/sdl"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$BinDir --libexecdir=$BinDir --includedir=$IncludeDir 
	make && make install

	arrange_shared "$BinDir" "libSDL-1.2.so" "0" "0.11.2" "libSDL.la" "sdl.pc" "$LibDir"
	move_files_to_dir "$BinDir/libSDLmain.a" "$LibDir"
	rm -f "$BinDir/libSDL.so"
fi




#################
# GPL Libraries #
#################




#libnice
if [ ! -f "$BinDir/libnice.so" ]; then 
	unpack_gzip_and_move "libnice.tar.gz" "$PKG_DIR_LIBNICE"
	mkdir_and_move "$IntDir/libnice"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make
	make install
	
	arrange_shared "$BinDir" "libnice.so" "0" "0.4.0" "libnice.la" "nice.pc" "$LibDir"
fi

#xvid
if [ ! -f "$BinDir/libxvidcore.so" ]; then
	unpack_gzip_and_move "xvidcore-1.2.2.tar.gz" "$PKG_DIR_XVIDCORE"
	mkdir_and_move "$IntDir/xvidcore"

	cd $PKG_DIR/build/generic/
	./configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	
	make 
	make install
	make clean
	
	arrange_shared "$BinDir" "libxvidcore.so" "4" "4.2" "" "" "$LibDir"
	move_files_to_dir "$BinDir/libxvidcore.a" "$LibDir"
	chmod a+x "$BinDir/libxvidcore.so.4"
fi

#ffmpeg GPL version
if [ ! -f "$BinDir/libavcodec-gpl.so" ]; then 
	unpack_bzip2_and_move "ffmpeg.tar.bz2" "$PKG_DIR_FFMPEG"
	mkdir_and_move "$IntDir/ffmpeg-gpl"
	
	#LGPL-compatible version
	#$PKG_DIR/configure --enable-gpl --extra-ldflags="-no-undefined" --extra-cflags="-fno-common" --disable-avisynth --arch=i686 --cpu=i686 --disable-vhook --enable-libxvid --enable-zlib --enable-bzlib --enable-pthreads --enable-ipv6 --disable-ffmpeg --disable-ffplay --disable-ffserver --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir/gpl --libdir=$BinDir/gpl --shlibdir=$BinDir/gpl --incdir=$IncludeDir/gpl
	make && make install
	
	arrange_shared "$BinDir/gpl" "libavutil.so" "49" "49.15.0" "" "libavutil.pc" "$LibDir/gpl"
	arrange_shared "$BinDir/gpl" "libavformat.so" "52" "52.31.0" "" "libavformat.pc" "$LibDir/gpl"
	arrange_shared "$BinDir/gpl" "libavcodec.so" "52" "52.20.0" "" "libavcodec.pc" "$LibDir/gpl"
	
	cd "$BinDir"
	
	cd "gpl"
	rm -f libavdevice*
	mv "libavutil.so.49" "../libavutil-gpl.so.49"
	mv "libavcodec.so.52" "../libavcodec-gpl.so.52"
	mv "libavformat.so.52" "../libavformat-gpl.so.52"
	rm -f "libavutil.so"
	rm -f "libavcodec.so"
	rm -f "libavformat.so"
	
	cd ".."
	ln -s "libavutil-gpl.so.49" "libavutil-gpl.so"
	ln -s "libavcodec-gpl.so.52" "libavcodec-gpl.so"
	ln -s "libavformat-gpl.so.52" "libavformat-gpl.so"
	
	rm -rf "gpl"
	
	#We have to update the pkg-config file to point to this GPL version
	cd "$LibDir/gpl/pkgconfig"
	
	#Need to escape the forward slashes so sed can pick them up later
	sedBinDir=$( echo $BinDir | sed 's/\//\\\//g' )
	sedGplBinDir=$( echo $BinDir/gpl | sed 's/\//\\\//g' )
	sedIncludeDir=$( echo $IncludeDir | sed 's/\//\\\//g' )
	sedGplIncludeDir=$( echo $IncludeDir/gpl | sed 's/\//\\\//g' )
	
	sed "s/$sedGplBinDir/$sedBinDir/g" 										libavutil.pc > tmp.1.txt
	sed "s/$sedGplIncludeDir/$sedIncludeDir/g" 								tmp.1.txt > tmp.2.txt
	sed "s/Name: libavutil/Name: libavutil-gpl/g" 							tmp.2.txt > tmp.3.txt
	sed "s/-lavutil/-lavutil-gpl/g" 										tmp.3.txt > tmp.4.txt
	mv tmp.4.txt libavutil.pc
	rm -f tmp.*.txt
	
	sed "s/$sedGplBinDir/$sedBinDir/g" 										libavcodec.pc > tmp.1.txt
	sed "s/$sedGplIncludeDir/$sedIncludeDir/g" 								tmp.1.txt > tmp.2.txt
	sed "s/Name: libavcodec/Name: libavcodec-gpl/g" 						tmp.2.txt > tmp.3.txt
	sed "s/Requires.private: libavutil/Requires.private: libavutil-gpl/g" 	tmp.3.txt > tmp.4.txt
	sed "s/-lavcodec/-lavcodec-gpl/g" 										tmp.4.txt > tmp.5.txt
	mv tmp.5.txt libavcodec.pc
	rm -f tmp.*.txt
	
	sed "s/$sedGplBinDir/$sedBinDir/g" 										libavformat.pc > tmp.1.txt
	sed "s/$sedGplIncludeDir/$sedIncludeDir/g" 								tmp.1.txt > tmp.2.txt
	sed "s/Name: libavformat/Name: libavformat-gpl/g" 						tmp.2.txt > tmp.3.txt
	sed "s/Requires.private: libavcodec/Requires.private: libavcodec-gpl/g" tmp.3.txt > tmp.4.txt
	sed "s/-lavformat/-lavformat-gpl/g" 									tmp.4.txt > tmp.5.txt
	mv tmp.5.txt libavformat.pc
	rm -f tmp.*.txt
	
	mv "libavutil.pc" "../../pkgconfig/libavutil-gpl.pc"
	mv "libavcodec.pc" "../../pkgconfig/libavcodec-gpl.pc"
	mv "libavformat.pc" "../../pkgconfig/libavformat-gpl.pc"
	
	cd "../../"
	rm -rf "gpl"
	
	#Get rid of the include gpl directory as well
	rm -rf "$IncludeDir/gpl"
fi

reset_flags

#Call common shutdown routines
common_shutdown
