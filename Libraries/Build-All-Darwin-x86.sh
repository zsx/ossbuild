#!/bin/sh

TOP=$(dirname $0)/..

#Call common startup routines to load a bunch of variables we'll need
. $TOP/Shared/Scripts/Common.sh
common_startup "Darwin" "x86" "Release"

#Setup library versions
. $ROOT/Shared/Scripts/Version.sh

# Install pkg-config and gettext are required to build glib.
echo "**************************** Installing pkg-config *****************************"                         
#tar xvfz $ROOT/Tools/pkg-config-0.23.tar.gz
#cd pkg-config-0.23
#./configure
#make
#sudo make install
#cd ..
#rm -Rf pkg-config-0.23


echo "****************************** Installing gettext ******************************" 
#tar xvfz $ROOT/Tools/gettext-0.17.tar.gz
#cd gettext-0.17
#./configure
#make
#sudo make install
#cd ..
#rm -Rf gettext-0.17


echo "******************************** Installing yasm *******************************" 
#tar xvfz $ROOT/Tools/yasm-0.8.0.tar.gz
#cd yasm-0.8.0
#./configure
#make
#sudo make install
#cd ..

#Move to intermediate directory
#cd "$IntDir"

#clear_flags

#Causes us to now always include the bin dir to look for libraries, even after calling reset_flags
export ORIG_LDFLAGS="$ORIG_LDFLAGS -L$BinDir -L$SharedBinDir"
reset_flags

# To ensure that tools like bzip2 find the required dynamic libraries.
#DYLD_LIBRARY_PATH="$BinDir"
#export DYLD_LIBRARY_PATH

#Here's where the magic really happens

#liboil
if [ ! -f "$BinDir/liboil.dylib" ]; then 
    unpack_gzip_and_move "liboil.tar.gz" "$PKG_DIR_LIBOIL"
	mkdir_and_move "$IntDir/liboil"

	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_dylib "$BinDir" "liboil" "-" "0" "0.3.0" "liboil-0.3.la" "liboil-0.3.pc" "$LibDir"
fi

#zlib
#Can't use separate build dir
if [ ! -f "$BinDir/libz.dylib" ]; then                      
	unpack_zip_and_move_linux "zlib.zip" "zlib" "zlib"
	mkdir_and_move "$IntDir/zlib"
	cd "$PKG_DIR"

	chmod u+x ./configure
	$ROOT/Tools/flip.osx -u ./configure
	echo "$PKG_DIR/configure -s --shared --prefix=$InstallDir --exec_prefix=$BinDir --libdir=$BinDir --includedir=$IncludeDir"
	"$PKG_DIR/configure" -s --shared --prefix=$InstallDir --exec_prefix=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	
	cp contrib/asm686/match.S ./match.S
	make LOC=-DASMV OBJA=match.o
	make libz.a
	make install
	cp libz.a "$LibDir/libz.a"
	make clean
	make distclean
	rm -f match.S
	
	arrange_dylib "$BinDir" "libz" "." "1" "1.2.3"
fi

#bzip2
if [ ! -f "$BinDir/libbz2.dylib" ]; then                      
	unpack_gzip_and_move "bzip2.tar.gz" "$PKG_DIR_BZIP2"
	cd "$PKG_DIR"

    cp -f "$LIBRARIES_PATCH_DIR/bzip2/Makefile.darwin" ./Makefile
	make PREFIX=$BinDir
	make install PREFIX=$InstallDir
	
	cp -p libbz2.a "$LibDir/libbz2.a"
	copy_files_to_dir "libbz2.1.0.5.dylib" "$BinDir"

	
	arrange_dylib "$BinDir" "libbz2" "." "1" "1.0.5"
	
	make clean
	remove_files_from_dir "*.dylib"
	rm "$LibDir/libbz2.1.0.5.dylib"
fi

#Skip pthreads

#libxml2
if [ ! -f "$BinDir/libxml2.dylib" ]; then 
	unpack_gzip_and_move "libxml2.tar.gz" "$PKG_DIR_LIBXML2"
	mkdir_and_move "$IntDir/libxml2"
	
	CFLAGS="$CFLAGS -O2"
	$PKG_DIR/configure --with-zlib --with-threads=native --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	reset_flags

	arrange_dylib "$BinDir" "libxml2" "." "2" "2.7.3" "libxml2.la" "libxml-2.0.pc" "$LibDir"
fi

#libjpeg
if [ ! -f "$BinDir/libjpeg.dylib" ]; then                             
	unpack_gzip_and_move "jpegsrc.tar.gz" "$PKG_DIR_LIBJPEG"
	mkdir_and_move "$IntDir/libjpeg"
	
	cp -f "$LIBRARIES_PATCH_DIR/libjpeg/darwin/config.sub" "$PKG_DIR/config.sub"
	cp -f "$LIBRARIES_PATCH_DIR/libjpeg/darwin/jpeglib.h" "$PKG_DIR/jpeglib.h"
	cp -f "$LIBRARIES_PATCH_DIR/libjpeg/darwin/ltconfig" "$PKG_DIR/ltconfig"
	cp -f "$LIBRARIES_PATCH_DIR/libjpeg/darwin/ltmain.sh" "$PKG_DIR/ltmain.sh"
	cp -f "$LIBRARIES_PATCH_DIR/libjpeg/darwin/makefile.cfg" "$PKG_DIR/makefile.cfg"
	
	$PKG_DIR/configure --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	move_files_to_dir "$LibDir/libjpeg*" "$BinDir"
	arrange_dylib "$BinDir" "libjpeg" "." "62" "62.0.0" "libjpeg.la" "" "$LibDir"
	move_files_to_dir "$BinDir/libjpeg.a" "$LibDir"
fi

#libpng
if [ ! -f "$BinDir/libpng12.dlib" ]; then                         
	unpack_gzip_and_move "libpng.tar.gz" "$PKG_DIR_LIBPNG"
	mkdir_and_move "$IntDir/libpng"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_dylib "$BinDir" "libpng12" "." "0" "0.35.0" "libpng12.la" "libpng12.pc" "$LibDir"

	cd "$BinDir"
	move_files_to_dir "libpng12.a" "$LibDir"
	move_files_to_dir "pkgconfig/libpng.pc" "$LibDir/pkgconfig/"
	remove_files_from_dir "libpng.a libpng.la libpng.so*"
fi

#glib
if [ ! -f "$BinDir/libglib-2.0.dylib" ]; then                              
	unpack_gzip_and_move "glib.tar.gz" "$PKG_DIR_GLIB"
	mkdir_and_move "$IntDir/glib"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_dylib "$BinDir" "libgio-2.0" "." "0" "0.2000.1" "libgio-2.0.la" "gio-2.0.pc gio-unix-2.0.pc" "$LibDir"
	arrange_dylib "$BinDir" "libglib-2.0" "." "0" "0.2000.1" "libglib-2.0.la" "glib-2.0.pc" "$LibDir"
	arrange_dylib "$BinDir" "libgmodule-2.0" "." "0" "0.2000.1" "libgmodule-2.0.la" "gmodule-2.0.pc gmodule-export-2.0.pc gmodule-no-export-2.0.pc" "$LibDir"
	arrange_dylib "$BinDir" "libgobject-2.0" "." "0" "0.2000.1" "libgobject-2.0.la" "gobject-2.0.pc" "$LibDir"
	arrange_dylib "$BinDir" "libgthread-2.0" "." "0" "0.2000.1" "libgthread-2.0.la" "gthread-2.0.pc" "$LibDir"
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
if [ ! -f "$BinDir/libgpg-error.dlib" ]; then                          
	unpack_gzip_and_move "libgpg-error.tar.gz" "$PKG_DIR_LIBGPG_ERROR"
	mkdir_and_move "$IntDir/libgpg-error"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install

	move_files_to_dir "$LibDir/libgpg-error*" "$BinDir"
	arrange_dylib "$BinDir" "libgpg-error" "." "0" "0.5.0" "libgpg-error.la" "" "$LibDir"
fi

#libgcrypt
if [ ! -f "$BinDir/libgcrypt.dylib" ]; then 
	unpack_gzip_and_move "libgcrypt.tar.gz" "$PKG_DIR_LIBGCRYPT"
	mkdir_and_move "$IntDir/libgcrypt"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_dylib "$BinDir" "libgcrypt" "." "11" "11.5.2" "libgcrypt.la" "" "$LibDir"
fi

#gnutls
if [ ! -f "$BinDir/libgnutls.dylib" ]; then                             
	unpack_bzip2_and_move "gnutls.tar.bz2" "$PKG_DIR_GNUTLS"
	mkdir_and_move "$IntDir/gnutls"
	
	$PKG_DIR/configure --with-included-libtasn1 --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_dylib "$BinDir" "libgnutls" "." "26" "26.11.6" "libgnutls.la" "gnutls.pc" "$LibDir"
	arrange_dylib "$BinDir" "libgnutls-extra" "." "26" "26.11.6" "libgnutls-extra.la" "gnutls-extra.pc" "$LibDir"
	arrange_dylib "$BinDir" "libgnutlsxx" "." "26" "26.11.6" "libgnutlsxx.la" "" "$LibDir"
	arrange_dylib "$BinDir" "libgnutls-openssl" "." "26" "26.11.6" "libgnutls-openssl.la" "" "$LibDir"
fi

#soup
if [ ! -f "$BinDir/libsoup-2.4.dylib" ]; then 
	unpack_bzip2_and_move "libsoup.tar.bz2" "$PKG_DIR_LIBSOUP"
	mkdir_and_move "$IntDir/libsoup"
	
	$PKG_DIR/configure --without-gnome --disable-glibtest --enable-ssl --enable-debug=no --disable-more-warnings --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	arrange_dylib "$BinDir" "libsoup-2.4" "." "1" "1.2.0" "libsoup-2.4.la" "libsoup-2.4.pc" "$LibDir"
fi

#neon
if [ ! -f "$BinDir/libneon.dylib" ]; then                               
	unpack_gzip_and_move "neon.tar.gz" "$PKG_DIR_NEON"
	mkdir_and_move "$IntDir/neon"
	
	$PKG_DIR/configure --with-libxml2 --with-ssl --disable-webdav --disable-debug --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_dylib "$BinDir" "libneon" "." "27" "27.1.4" "libneon.la" "neon.pc" "$LibDir"
fi

#freetype
if [ ! -f "$BinDir/libfreetype.dylib" ]; then                         
	unpack_bzip2_and_move "freetype.tar.bz2" "$PKG_DIR_FREETYPE"
	mkdir_and_move "$IntDir/freetype"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_dylib "$BinDir" "libfreetype" "." "6" "6.3.20" "libfreetype.la" "freetype2.pc" "$LibDir"
fi

#fontconfig
if [ ! -f "$BinDir/libfontconfig.dylib" ]; then                              
	unpack_gzip_and_move "fontconfig.tar.gz" "$PKG_DIR_FONTCONFIG"
	mkdir_and_move "$IntDir/fontconfig"
	
	$PKG_DIR/configure --enable-libxml2 --disable-debug --disable-static --disable-docs --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir

	make && make install

	arrange_dylib "$BinDir" "libfontconfig" "." "1" "1.3.0" "libfontconfig.la" "fontconfig.pc" "$LibDir"
fi

#pixman
if [ ! -f "$BinDir/libpixman-1.dylib" ]; then                           
	unpack_gzip_and_move "pixman.tar.gz" "$PKG_DIR_PIXMAN"
	mkdir_and_move "$IntDir/pixman"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_dylib "$BinDir" "libpixman-1" "." "0" "0.15.2" "libpixman-1.la" "pixman-1.pc" "$LibDir"
fi

#cairo
if [ ! -f "$BinDir/libcairo.dylib" ]; then
	unpack_gzip_and_move "cairo.tar.gz" "$PKG_DIR_CAIRO"
	mkdir_and_move "$IntDir/cairo"
	
	$PKG_DIR/configure --disable-glitz --disable-quartz --disable-quartz-font --disable-quartz-image --disable-xcb --enable-xlib=auto --enable-xlib-xrender=auto --enable-png=yes --enable-ft=yes --enable-pdf=yes --enable-svg=yes --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_dylib "$BinDir" "libcairo" "." "2" "2.10800.6" "libcairo.la" "cairo*.pc" "$LibDir"
fi

#pango
if [ ! -f "$BinDir/libpango-1.0.dylib" ]; then                                 
	unpack_gzip_and_move "pango.tar.gz" "$PKG_DIR_PANGO"
	mkdir_and_move "$IntDir/pango"
	
	LDFLAGS="$LDFLAGS -lxml2"
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	reset_flags

	arrange_dylib "$BinDir" "libpango-1.0" "." "0" "0.2400.1" "libpango-1.0.la" "pango.pc" "$LibDir"
	arrange_dylib "$BinDir" "libpangoft2-1.0" "." "0" "0.2400.1" "libpangoft2-1.0.la" "pangoft2.pc" "$LibDir"
	arrange_dylib "$BinDir" "libpangocairo-1.0" "." "0" "0.2400.1" "libpangocairo-1.0.la" "pangocairo.pc" "$LibDir"
	mkdir -p "$LibDir/pango/1.6.0/modules"
	move_files_to_dir "$BinDir/pango/1.6.0/modules/*.la" "$LibDir/pango/1.6.0/modules"
fi

#libogg
if [ ! -f "$BinDir/libogg.dylib" ]; then                      
	unpack_gzip_and_move "libogg.tar.gz" "$PKG_DIR_LIBOGG"
	mkdir_and_move "$IntDir/libogg"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_dylib "$BinDir" "libogg" "." "0" "0.5.3" "libogg.la" "ogg.pc" "$LibDir"
fi

#libvorbis
if [ ! -f "$BinDir/libvorbis.dylib" ]; then 
	unpack_bzip2_and_move "libvorbis.tar.bz2" "$PKG_DIR_LIBVORBIS"
	mkdir_and_move "$IntDir/libvorbis"
	
	$PKG_DIR/configure --disable-oggtest --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_dylib "$BinDir" "libvorbis" "." "0" "0.4.0" "libvorbis.la" "vorbis.pc" "$LibDir"
	arrange_dylib "$BinDir" "libvorbisenc" "." "2" "2.0.3" "libvorbisenc.la" "vorbisenc.pc" "$LibDir"
	arrange_dylib "$BinDir" "libvorbisfile" "." "3" "3.2.0" "libvorbisfile.la" "vorbisfile.pc" "$LibDir"
fi

#libtheora
if [ ! -f "$BinDir/libtheora.dylib" ]; then 
	unpack_bzip2_and_move "libtheora.tar.bz2" "$PKG_DIR_LIBTHEORA"
	mkdir_and_move "$IntDir/libtheora"
	
	$PKG_DIR/configure --disable-examples --disable-oggtest --disable-vorbistest --disable-sdltest --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_dylib "$BinDir" "libtheora" "." "0" "0.3.4" "libtheora.la" "theora.pc" "$LibDir"
	arrange_dylib "$BinDir" "libtheoradec" "." "1" "1.0.1" "libtheoradec.la" "theoradec.pc" "$LibDir"
	arrange_dylib "$BinDir" "libtheoraenc" "." "1" "1.0.1" "libtheoraenc.la" "theoraenc.pc" "$LibDir"
fi

#libmms
if [ ! -f "$BinDir/libmms.dylib" ]; then 
	unpack_gzip_and_move "libmms-0.4.tar.gz" "$PKG_DIR_LIBMMS"

    GLIB_CFLAGS="-I$ROOT/Build/Darwin/x86/Release/include/glib-2.0/ -I$ROOT/Build/Darwin/x86/Release/lib/glib-2.0/include/"
    export GLIB_CFLAGS

    GLIB_LIBS="$ROOT/Build/Darwin/x86/Release/lib/libglib-2.0.la"
    export GLIB_CFLAGS

    PKG_CONFIG_PATH="$ROOT/Build/Darwin/x86/Release/lib/pkgconfig"
    export PKG_CONFIG_PATH

	mkdir_and_move "$IntDir/libmms"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_dylib "$BinDir" "libmms" "." "0" "0.0.2" "libmms.la" "libmms.pc" "$LibDir"
fi

#x264
#For now, it can only decode H.264 (--disable-mp4-output)
#To encode, you'll need gpac and a proper license
if [ ! -f "$BinDir/libx264.dylib" ]; then 
	unpack_bzip2_and_move "x264.tar.bz2" "$PKG_DIR_X264"
	mkdir_and_move "$IntDir/x264"
	
	cd "$PKG_DIR/"
	CFLAGS="$CFLAGS -D X264_CPU_SHUFFLE_IS_FAST=0x000800"
	./configure --disable-mp4-output --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	make clean && make distclean
	reset_flags
	
	arrange_dylib "$BinDir" "libx264" "." "67" "67.0.0" "" "x264.pc" "$LibDir"
	move_files_to_dir "$BinDir/libx264.a" "$LibDir"
fi

#libspeex
if [ ! -f "$BinDir/libspeex.dylib" ]; then 
	unpack_gzip_and_move "speex.tar.gz" "$PKG_DIR_LIBSPEEX"
	mkdir_and_move "$IntDir/libspeex"
	
	$PKG_DIR/configure --disable-oggtest --enable-sse --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_dylib "$BinDir" "libspeex" "." "1" "1.5.0" "libspeex.la" "speex.pc" "$LibDir"
	arrange_dylib "$BinDir" "libspeexdsp" "." "1" "1.5.0" "libspeexdsp.la" "speexdsp.pc" "$LibDir"
fi

#libschrodinger (dirac support)
if [ ! -f "$BinDir/libschroedinger-1.0.dylib" ]; then 
	unpack_gzip_and_move "schroedinger.tar.gz" "$PKG_DIR_LIBSCHROEDINGER"
	mkdir_and_move "$IntDir/libschroedinger"
	
	$PKG_DIR/configure --with-thread=auto --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_dylib "$BinDir" "libschroedinger-1.0" "." "0" "0.2.0" "libschroedinger-1.0.la" "schroedinger-1.0.pc" "$LibDir"
fi

#openjpeg
if [ ! -f "$BinDir/libopenjpeg.dylib" ]; then 
	unpack_gzip_and_move "openjpeg.tar.gz" "$PKG_DIR_OPENJPEG"
	mkdir_and_move "$IntDir/openjpeg"
	
	cd "$PKG_DIR"
	cp -f "$LIBRARIES_PATCH_DIR/openjpeg/Makefile.osx" ./Makefile.osx
	cp -f "$LIBRARIES_PATCH_DIR/openjpeg/opj_malloc.h" ./libopenjpeg/opj_malloc.h
	
	make osx INSTALL_NAME_DIR=$BinDir
	make osxinstall PREFIX=$InstallDir
	make clean
	
	cd "$BinDir"
	mv "$LibDir/libopenjpeg-2.1.2.0.dylib" "./libopenjpeg-2.1.2.0.dylib"
	rm "$LibDir/libopenjpeg.dylib"
    ln -s "libopenjpeg-2.1.2.0.dylib" "$BinDir/libopenjpeg-2.dylib"
	ln -s "libopenjpeg-2.dylib" "$BinDir/libopenjpeg.dylib"
fi

#mp3lame
if [ ! -f "$BinDir/libmp3lame.dylib" ]; then 
	unpack_gzip_and_move "lame.tar.gz" "$PKG_DIR_MP3LAME"
	mkdir_and_move "$IntDir/mp3lame"
	
	$PKG_DIR/configure --enable-expopt=no --enable-debug=no --disable-brhist -disable-frontend --enable-nasm --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_dylib "$BinDir" "libmp3lame" "." "0" "0.0.0" "libmp3lame.la" "" "$LibDir"
fi

#ffmpeg
if [ ! -f "$BinDir/libavcodec.dylib" ]; then 
	unpack_bzip2_and_move "ffmpeg.tar.bz2" "$PKG_DIR_FFMPEG"
	mkdir_and_move "$IntDir/ffmpeg"
	
	cp -f "$LIBRARIES_PATCH_DIR/ffmpeg/darwin/internal.h" "$PKG_DIR/libavutil"
	cp -f "$LIBRARIES_PATCH_DIR/ffmpeg/darwin/Makefile" "$PKG_DIR/libswscale"
	cp -f "$LIBRARIES_PATCH_DIR/ffmpeg/darwin/swscale.h" "$PKG_DIR/libswscale"
	cp -f "$LIBRARIES_PATCH_DIR/ffmpeg/darwin/avfilter.h" "$PKG_DIR/libavfilter"
	
	#LGPL-compatible version
    $PKG_DIR/configure --extra-ldflags="-no-undefined" --extra-cflags="-fno-common" --disable-avisynth --enable-avfilter-lavf --enable-avfilter --arch=i686 --cpu=i686 --disable-vhook --enable-zlib --enable-bzlib --enable-pthreads --enable-ipv6 --enable-libmp3lame --enable-libvorbis --enable-libopenjpeg --enable-libtheora --enable-libspeex --enable-libschroedinger --disable-ffmpeg --disable-ffplay --disable-ffserver --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$BinDir --shlibdir=$BinDir --incdir=$IncludeDir

	make && make install
	
	arrange_dylib "$BinDir" "libavutil" "." "49" "49.15.0" "" "libavutil.pc" "$LibDir"
	arrange_dylib "$BinDir" "libavfilter" "." "0" "0.4.0" "" "libavfilter.pc" "$LibDir"
	arrange_dylib "$BinDir" "libavcodec" "." "52" "52.20.0" "" "libavcodec.pc" "$LibDir"
	arrange_dylib "$BinDir" "libavdevice" "." "52" "52.1.0" "" "libavdevice.pc" "$LibDir"
	arrange_dylib "$BinDir" "libavformat" "." "52" "52.31.0" "" "libavformat.pc" "$LibDir"
fi

#sdl
if [ ! -f "$BinDir/libSDL.dylib" ]; then 
	unpack_gzip_and_move "sdl.tar.gz" "$PKG_DIR_SDL"
	mkdir_and_move "$IntDir/sdl"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$BinDir --libexecdir=$BinDir --includedir=$IncludeDir 
	make && make install

	arrange_dylib "$BinDir" "libSDL-1.2" "." "0" "0.11.2" "libSDL.la" "sdl.pc" "$LibDir"
	move_files_to_dir "$BinDir/libSDLmain.a" "$LibDir"
fi