#!/bin/sh

###############################################################################
#                                                                             #
#                             Linux x86 Build                                 #
#                                                                             #
# Builds all dependencies for this platform.                                  #
#                                                                             #
###############################################################################

TOP=$(dirname $0)/..

#Global directories
PERL_BIN_DIR=/usr/bin

#Global flags
CFLAGS="$CFLAGS"
CPPFLAGS="$CPPFLAGS"
LDFLAGS=""
CXXFLAGS="$CXXFLAGS"

#Call common startup routines to load a bunch of variables we'll need
. $TOP/Shared/Scripts/Common.sh
common_startup "Linux" "x86" "Release"

#Setup library versions
. $ROOT/Shared/Scripts/Version.sh

save_prefix ${DefaultPrefix}

#Move to intermediate directory
#cd "$IntDir"

#clear_flags

#No prefix from here out
clear_prefix

#Causes us to now always include the bin dir to look for libraries, even after calling reset_flags
export ORIG_LDFLAGS="$ORIG_LDFLAGS -L$BinDir -L$SharedBinDir"
reset_flags

#Here's where the magic really happens

export PATH="$BinDir:$PATH"
export LD_LIBRARY_PATH="$BinDir:$LD_LIBRARY_PATH"

#liboil
if [ ! -f "$BinDir/liboil-0.3.so.0" ]; then 
	unpack_gzip_and_move "liboil.tar.gz" "$PKG_DIR_LIBOIL"
	mkdir_and_move "$IntDir/liboil"

	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "liboil-0.3.so" "0" "0.3.0" "liboil-0.3.la" "liboil-0.3.pc" "$LibDir"
fi

#zlib
#Don't actually use this one - use the sys-supplied one.
#Can't use separate build dir
if [ ! -f "$BinDir/libz.so.1" ]; then 
	unpack_zip_and_move_linux "zlib.zip" "zlib" "zlib"
	mkdir_and_move "$IntDir/zlib"
	cd "$PKG_DIR"

	chmod u+x ./configure
	dos2unix ./configure
	cp -p "$LIBRARIES_PATCH_DIR/zlib/zlib.map" .
	"$PKG_DIR/configure" -s --shared --prefix=$InstallDir --exec_prefix=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	
	cp contrib/asm686/match.S ./match.S
	make LOC=-DASMV OBJA=match.o LDSHARED="gcc -shared -Wl,-soname,libz.so.1 -Wl,--version-script zlib.map"
	make libz.a
	make install
	cp libz.a "$LibDir/libz.a"
	make clean
	make distclean
	rm -f match.S
	
	arrange_shared "$BinDir" "libz.so" "1" "1.2.3"
	
	rm -rf "$LibDir/libz.a"
fi

#bzip2
if [ ! -f "$BinDir/libbz2.so.1" ]; then 
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
	
	rm -rf "$LibDir/libbz2.a"
fi

#Skip pthreads

#glew
if [ ! -f "$BinDir/libGLEW.so.1.5" ]; then
	unpack_gzip_and_move "glew.tar.gz" "$PKG_DIR_GLEW"
	mkdir_and_move "$IntDir/glew"

	cd "$PKG_DIR"
	make
	make install GLEW_DEST=$InstallDir
	
	arrange_shared "$LibDir" "libGLEW.so" "1.5" "1.5.2"
	move_files_to_dir "$LibDir/lib*GLEW*.so*" "$BinDir"
	chmod uag+x "$BinDir/libGLEW.so.1.5"
	
	rm -rf "$LibDir/libGLEW.a"
fi

#expat
if [ ! -f "$BinDir/libexpat.so.1" ]; then
	unpack_gzip_and_move "expat.tar.gz" "$PKG_DIR_EXPAT"
	mkdir_and_move "$IntDir/expat"

	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libexpat.so" "1" "1.5.2" "libexpat.la" "" "$LibDir"
fi

#libxml2
if [ ! -f "$BinDir/libxml2.so.2" ]; then 
	unpack_gzip_and_move "libxml2.tar.gz" "$PKG_DIR_LIBXML2"
	mkdir_and_move "$IntDir/libxml2"
	
	CFLAGS="$CFLAGS -O2"
	$PKG_DIR/configure --with-zlib --with-threads=native --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	reset_flags

	arrange_shared "$BinDir" "libxml2.so" "2" "2.7.6" "libxml2.la" "libxml-2.0.pc" "$LibDir"
	
	chmod aug+x "$BinDir/xml2Conf.sh"
fi

#libjpeg
if [ ! -f "$BinDir/libjpeg.so.8" ]; then 
	unpack_gzip_and_move "jpegsrc.tar.gz" "$PKG_DIR_LIBJPEG"
	mkdir_and_move "$IntDir/libjpeg"
	
	$PKG_DIR/configure --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	move_files_to_dir "$LibDir/libjpeg*" "$BinDir"
	arrange_shared "$BinDir" "libjpeg.so" "8" "8.0.0" "libjpeg.la" "" "$LibDir"
	move_files_to_dir "$BinDir/libjpeg.a" "$LibDir"
	
	rm -rf "$LibDir/libjpeg.a"
fi

#openjpeg
if [ ! -f "$BinDir/libopenjpeg.so.2" ]; then 
	unpack_gzip_and_move "openjpeg.tar.gz" "$PKG_DIR_OPENJPEG"
	mkdir_and_move "$IntDir/openjpeg"

	cd "$PKG_DIR"
	cp -f "$LIBRARIES_PATCH_DIR/openjpeg/Makefile.linux" ./Makefile

	make install PREFIX=$InstallDir
	mv "$LibDir/libopenjpeg-2.1.3.0.so" "$BinDir/libopenjpeg.so.2"
	rm -f "$LibDir/libopenjpeg.so.2"
	cd "$BinDir/"
	ln -s libopenjpeg.so.2 libopenjpeg.so
	
	rm -rf "$LibDir/libopenjpeg.a"
fi

#libpng
if [ ! -f "$BinDir/libpng14.so.14" ]; then 
	unpack_gzip_and_move "libpng.tar.gz" "$PKG_DIR_LIBPNG"
	mkdir_and_move "$IntDir/libpng"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libpng14.so" "14" "14.0.0" "libpng14.la" "libpng14.pc" "$LibDir"

	cd "$BinDir"
	move_files_to_dir "pkgconfig/libpng.pc" "$LibDir/pkgconfig/"
	mv libpng.la "$LibDir"
	ln -s libpng14.so libpng.so
	ln -s libpng14.so libpng12.so
	cd "$LibDir"
	ln -s libpng14.la libpng12.la
	cd "$LibDir/pkgconfig/"
	ln -s libpng14.pc libpng12.pc
fi

#libtiff
if [ ! -f "$BinDir/libtiff.so.3" ]; then 
	unpack_gzip_and_move "tiff.tar.gz" "$PKG_DIR_LIBTIFF"
	mkdir_and_move "$IntDir/tiff"
	
	#Configure, compile, and install
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libtiff.so" "3" "3.9.2" "libtiff.la" "" "$LibDir"
	arrange_shared "$BinDir" "libtiffxx.so" "3" "3.9.2" "libtiffxx.la" "" "$LibDir"
fi

#glib
if [ ! -f "$BinDir/libglib-2.0.so.0" ]; then 
	unpack_bzip2_and_move "glib.tar.bz2" "$PKG_DIR_GLIB"
	#patch -u -N -i "$LIBRARIES_PATCH_DIR/glib/run-markup-tests.sh.patch"

	mkdir_and_move "$IntDir/glib"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libgio-2.0.so" "0" "0.2200.4" "libgio-2.0.la" "gio-2.0.pc gio-unix-2.0.pc" "$LibDir"
	arrange_shared "$BinDir" "libglib-2.0.so" "0" "0.2200.4" "libglib-2.0.la" "glib-2.0.pc" "$LibDir"
	arrange_shared "$BinDir" "libgmodule-2.0.so" "0" "0.2200.4" "libgmodule-2.0.la" "gmodule-2.0.pc gmodule-export-2.0.pc gmodule-no-export-2.0.pc" "$LibDir"
	arrange_shared "$BinDir" "libgobject-2.0.so" "0" "0.2200.4" "libgobject-2.0.la" "gobject-2.0.pc" "$LibDir"
	arrange_shared "$BinDir" "libgthread-2.0.so" "0" "0.2200.4" "libgthread-2.0.la" "gthread-2.0.pc" "$LibDir"
	test -d "$LibDir/gio" && rm -rf "$LibDir/gio"
	test -d "$LibDir/glib-2.0" && rm -rf "$LibDir/glib-2.0"
	mv "$BinDir/gio" "$LibDir"
	mv "$BinDir/glib-2.0" "$LibDir"
	cp -p "$LibDir/glib-2.0/include/glibconfig.h" "$IncludeDir/glib-2.0/"
	rm -rf "$LibDir/glib-2.0/"
	rm -rf "$LibDir/gio/"
	chmod aug+x "$BinDir/gtester-report"
fi

#atk
if [ ! -f "$BinDir/libatk-1.0.so.0" ]; then 
	unpack_bzip2_and_move "atk.tar.bz2" "$PKG_DIR_ATK"
	mkdir_and_move "$IntDir/atk"
	
	#Configure, compile, and install
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libatk-1.0.so" "0" "0.2913.1" "libatk-1.0.la" "atk.pc" "$LibDir"
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
if [ ! -f "$BinDir/libgpg-error.so.0" ]; then 
	unpack_bzip2_and_move "libgpg-error.tar.bz2" "$PKG_DIR_LIBGPG_ERROR"
	mkdir_and_move "$IntDir/libgpg-error"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install

	move_files_to_dir "$LibDir/libgpg-error*" "$BinDir"
	arrange_shared "$BinDir" "libgpg-error.so" "0" "0.5.0" "libgpg-error.la" "" "$LibDir"
fi

#libgcrypt
if [ ! -f "$BinDir/libgcrypt.so.11" ]; then 
	unpack_bzip2_and_move "libgcrypt.tar.bz2" "$PKG_DIR_LIBGCRYPT"
	mkdir_and_move "$IntDir/libgcrypt"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libgcrypt.so" "11" "11.5.3" "libgcrypt.la" "" "$LibDir"
fi

#libtasn1
if [ ! -f "$BinDir/libtasn1.so.3" ]; then 
	unpack_gzip_and_move "libtasn1.tar.gz" "$PKG_DIR_LIBTASN1"
	mkdir_and_move "$IntDir/libtasn1"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libtasn1.so" "3" "3.1.7" "libtasn1.la" "libtasn1.pc" "$LibDir"
fi

#gnutls
if [ ! -f "$BinDir/libgnutls.so.26" ]; then 
	unpack_bzip2_and_move "gnutls.tar.bz2" "$PKG_DIR_GNUTLS"
	mkdir_and_move "$IntDir/gnutls"
	
	$PKG_DIR/configure --disable-cxx --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libgnutls.so" "26" "26.14.12" "libgnutls.la" "gnutls.pc" "$LibDir"
	arrange_shared "$BinDir" "libgnutls-extra.so" "26" "26.14.12" "libgnutls-extra.la" "gnutls-extra.pc" "$LibDir"
	#arrange_shared "$BinDir" "libgnutlsxx.so" "26" "26.14.12" "libgnutlsxx.la" "" "$LibDir"
	arrange_shared "$BinDir" "libgnutls-openssl.so" "26" "26.14.12" "libgnutls-openssl.la" "" "$LibDir"
fi

#soup
if [ ! -f "$BinDir/libsoup-2.4.so.1" ]; then 
	unpack_bzip2_and_move "libsoup.tar.bz2" "$PKG_DIR_LIBSOUP"
	mkdir_and_move "$IntDir/libsoup"
	
	$PKG_DIR/configure --without-gnome --disable-glibtest --enable-ssl --enable-debug=no --disable-more-warnings --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	
	arrange_shared "$BinDir" "libsoup-2.4.so" "1" "1.3.0" "libsoup-2.4.la" "libsoup-2.4.pc" "$LibDir"
fi

#neon
if [ ! -f "$BinDir/libneon.so.27" ]; then 
	unpack_gzip_and_move "neon.tar.gz" "$PKG_DIR_NEON"
	mkdir_and_move "$IntDir/neon"
	
	$PKG_DIR/configure --with-ssl=gnutls --disable-debug --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libneon.so" "27" "27.2.3" "libneon.la" "neon.pc" "$LibDir"
fi

#freetype
if [ ! -f "$BinDir/libfreetype.so.6" ]; then 
	unpack_bzip2_and_move "freetype.tar.bz2" "$PKG_DIR_FREETYPE"
	mkdir_and_move "$IntDir/freetype"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libfreetype.so" "6" "6.4.0" "libfreetype.la" "freetype2.pc" "$LibDir"
fi

#fontconfig
if [ ! -f "$BinDir/libfontconfig.so.1" ]; then 
	unpack_gzip_and_move "fontconfig.tar.gz" "$PKG_DIR_FONTCONFIG"
	mkdir_and_move "$IntDir/fontconfig"
	
	$PKG_DIR/configure --disable-debug --disable-static --disable-docs --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libfontconfig.so" "1" "1.4.4" "libfontconfig.la" "fontconfig.pc" "$LibDir"
fi

#pixman
if [ ! -f "$BinDir/libpixman-1.so.0" ]; then 
	unpack_gzip_and_move "pixman.tar.gz" "$PKG_DIR_PIXMAN"
	mkdir_and_move "$IntDir/pixman"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libpixman-1.so" "0" "0.17.6" "libpixman-1.la" "pixman-1.pc" "$LibDir"
fi

#cairo
if [ ! -f "$BinDir/libcairo.so.2" ]; then 
	unpack_gzip_and_move "cairo.tar.gz" "$PKG_DIR_CAIRO"
	mkdir_and_move "$IntDir/cairo"
	
	$PKG_DIR/configure --enable-xlib=yes --enable-xlib-xrender=no --enable-png=yes --enable-ft=yes --enable-pdf=yes --enable-svg=yes --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	cp -p "$IntDir/cairo/src/cairo-features.h" "$PKG_DIR/src/"
	make && make install

	arrange_shared "$BinDir" "libcairo.so" "2" "2.10800.10" "libcairo.la" "cairo*.pc" "$LibDir"
fi

#pango
if [ ! -f "$BinDir/libpango-1.0.so.0" ]; then 
	unpack_bzip2_and_move "pango.tar.bz2" "$PKG_DIR_PANGO"
	mkdir_and_move "$IntDir/pango"
	
	$PKG_DIR/configure --with-included-modules --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
	reset_flags

	arrange_shared "$BinDir" "libpango-1.0.so" "0" "0.2701.0" "libpango-1.0.la" "pango.pc" "$LibDir"
	arrange_shared "$BinDir" "libpangox-1.0.so" "0" "0.2701.0" "libpangox-1.0.la" "pangox.pc" "$LibDir"
	arrange_shared "$BinDir" "libpangoft2-1.0.so" "0" "0.2701.0" "libpangoft2-1.0.la" "pangoft2.pc" "$LibDir"
	arrange_shared "$BinDir" "libpangocairo-1.0.so" "0" "0.2701.0" "libpangocairo-1.0.la" "pangocairo.pc" "$LibDir"
	rm -rf "$BinDir/pango/"
	chmod uag+x "$LibDir/libpangox-1.0.la"
	chmod uag+x "$LibDir/libpangoft2-1.0.la"
fi

#gtk+
if [ ! -f "$BinDir/libgdk_pixbuf-2.0.so.0" ]; then 
	unpack_bzip2_and_move "gtk+.tar.bz2" "$PKG_DIR_GTKPLUS"
	mkdir_and_move "$IntDir/gtkplus"
	
	$PKG_DIR/configure --with-included-loaders --with-included-immodules --without-libjasper --disable-debug --enable-explicit-deps=no --disable-gtk-doc --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libgailutil.so" "18" "18.0.1" "libgailutil.la" "gailutil.pc" "$LibDir"
	arrange_shared "$BinDir" "libgdk_pixbuf-2.0.so" "0" "0.1800.7" "libgdk_pixbuf-2.0.la" "gdk-pixbuf-2.0.pc" "$LibDir"
	arrange_shared "$BinDir" "libgdk_pixbuf_xlib-2.0.so" "0" "0.1800.7" "libgdk_pixbuf_xlib-2.0.la" "gdk-pixbuf-xlib-2.0.pc" "$LibDir"
	arrange_shared "$BinDir" "libgdk-x11-2.0.so" "0" "0.1800.7" "libgdk-x11-2.0.la" "gdk-x11-2.0.pc" "$LibDir"
	arrange_shared "$BinDir" "libgtk-x11-2.0.so" "0" "0.1800.7" "libgtk-x11-2.0.la" "gtk+-x11-2.0.pc" "$LibDir"
	move_files_to_dir "$BinDir/pkgconfig/*gail*.pc" "$LibDir/pkgconfig/"
	move_files_to_dir "$BinDir/pkgconfig/*gdk*.pc" "$LibDir/pkgconfig/"
	move_files_to_dir "$BinDir/pkgconfig/*gtk*.pc" "$LibDir/pkgconfig/"
	chmod uag+x "$LibDir/libgdk-x11-2.0.la"
	chmod uag+x "$LibDir/libgtk-x11-2.0.la"
	mv "$BinDir/gtk-2.0" "$LibDir/"
	mv "$LibDir/gtk-2.0/include/gdkconfig.h" "$IncludeDir/gtk-2.0/"
	rm -rf "$LibDir/gtk-2.0/include/"
	rm -rf "$BinDir/gtk-2.0"
fi

#gtkglarea
if [ ! -f "$BinDir/libgtkgl-2.0.so.1" ]; then 
	unpack_bzip2_and_move "gtkglarea.tar.bz2" "$PKG_DIR_GTKGLAREA"
	mkdir_and_move "$IntDir/gtkglarea"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libgtkgl-2.0.so" "1" "1.0.1" "libgtkgl-2.0.la" "gtkgl-2.0.pc" "$LibDir"
fi

#libcroco
if [ ! -f "$BinDir/libcroco-0.6.so.3" ]; then 
	unpack_bzip2_and_move "libcroco.tar.bz2" "$PKG_DIR_LIBCROCO"
	mkdir_and_move "$IntDir/libcroco"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libcroco-0.6.so" "3" "3.0.1" "libcroco-0.6.la" "libcroco-0.6.pc" "$LibDir"
fi

#intltool
if [ ! -f "$BinDir/intltool-merge" ]; then 
	unpack_bzip2_and_move "intltool.tar.bz2" "$PKG_DIR_INTLTOOL"
	mkdir_and_move "$IntDir/intltool"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install
fi

#libgsf
if [ ! -f "$BinDir/libgsf-1.so.114" ]; then 
	unpack_bzip2_and_move "libgsf.tar.bz2" "$PKG_DIR_LIBGSF"
	mkdir_and_move "$IntDir/libgsf"
	
	$PKG_DIR/configure --without-python --disable-gtk-doc --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libgsf-1.so" "114" "114.0.17" "libgsf-1.la" "libgsf-1.pc" "$LibDir"
fi

#librsvg
if [ ! -f "$BinDir/librsvg-2.so.2" ]; then 
	unpack_bzip2_and_move "librsvg.tar.bz2" "$PKG_DIR_LIBRSVG"
	mkdir_and_move "$IntDir/librsvg"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "librsvg-2.so" "2" "2.26.0" "librsvg-2.la" "librsvg-2.0.pc" "$LibDir"
	
	move_files_to_dir "$BinDir/gtk-2.0/2.10.0/engines/*" "$LibDir/gtk-2.0/2.10.0/engines/"
	move_files_to_dir "$BinDir/gtk-2.0/2.10.0/loaders/*" "$LibDir/gtk-2.0/2.10.0/loaders/"
	rm -rf "$BinDir/gtk-2.0/"
fi

#sdl
if [ ! -f "$BinDir/libSDL-1.2.so.0" ]; then 
	unpack_gzip_and_move "sdl.tar.gz" "$PKG_DIR_SDL"
	mkdir_and_move "$IntDir/sdl"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$BinDir --libexecdir=$BinDir --includedir=$IncludeDir 
	make && make install

	arrange_shared "$BinDir" "libSDL-1.2.so" "0" "0.11.3" "libSDL.la" "sdl.pc" "$LibDir"
	move_files_to_dir "$BinDir/libSDLmain.a" "$LibDir"
	rm -f "$BinDir/libSDL.so"
	
	cd "$BinDir"
	ln -s "libSDL-1.2.so.0" "libSDL.so"
fi

#libogg
if [ ! -f "$BinDir/libogg.so.0" ]; then 
	unpack_gzip_and_move "libogg.tar.gz" "$PKG_DIR_LIBOGG"
	mkdir_and_move "$IntDir/libogg"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libogg.so" "0" "0.6.0" "libogg.la" "ogg.pc" "$LibDir"
fi

#libvorbis
if [ ! -f "$BinDir/libvorbis.so.0" ]; then 
	unpack_bzip2_and_move "libvorbis.tar.bz2" "$PKG_DIR_LIBVORBIS"
	mkdir_and_move "$IntDir/libvorbis"
	
	$PKG_DIR/configure --disable-oggtest --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libvorbis.so" "0" "0.4.3" "libvorbis.la" "vorbis.pc" "$LibDir"
	arrange_shared "$BinDir" "libvorbisenc.so" "2" "2.0.6" "libvorbisenc.la" "vorbisenc.pc" "$LibDir"
	arrange_shared "$BinDir" "libvorbisfile.so" "3" "3.3.2" "libvorbisfile.la" "vorbisfile.pc" "$LibDir"
fi

#libcelt
if [ ! -f "$BinDir/libcelt0.so.0" ]; then 
	unpack_gzip_and_move "libcelt.tar.gz" "$PKG_DIR_LIBCELT"
	mkdir_and_move "$IntDir/libcelt"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libcelt0.so" "0" "0.0.0" "libcelt0.la" "celt.pc" "$LibDir"
fi

#libtheora
if [ ! -f "$BinDir/libtheora.so.0" ]; then 
	unpack_bzip2_and_move "libtheora.tar.bz2" "$PKG_DIR_LIBTHEORA"
	mkdir_and_move "$IntDir/libtheora"
	
	$PKG_DIR/configure --disable-examples --disable-oggtest --disable-vorbistest --disable-sdltest --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libtheora.so" "0" "0.3.10" "libtheora.la" "theora.pc" "$LibDir"
	arrange_shared "$BinDir" "libtheoradec.so" "1" "1.1.4" "libtheoradec.la" "theoradec.pc" "$LibDir"
	arrange_shared "$BinDir" "libtheoraenc.so" "1" "1.1.2" "libtheoraenc.la" "theoraenc.pc" "$LibDir"
fi

#libmms
if [ ! -f "$BinDir/libmms.so.0" ]; then 
	unpack_bzip2_and_move "libmms.tar.bz2" "$PKG_DIR_LIBMMS"
	./autogen.sh
	mkdir_and_move "$IntDir/libmms"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libmms.so" "0" "0.0.2" "libmms.la" "libmms.pc" "$LibDir"
fi

#x264
if [ ! -f "$BinDir/libx264.so.88" ]; then 
	unpack_bzip2_and_move "x264.tar.bz2" "$PKG_DIR_X264"
	mkdir_and_move "$IntDir/x264"
	
	cd "$PKG_DIR/"
	
	./configure --disable-mp4-output --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	
	#Download example y4m for use in profiling and selecting the best CPU instruction set
	wget http://samples.mplayerhq.hu/yuv4mpeg2/example.y4m.bz2
	bzip2 -d -f "example.y4m.bz2"
	
	make fprofiled VIDS="example.y4m"
	make install
	
	reset_flags

	mv "$BinDir/pkgconfig/x264.pc" "$LibDir/pkgconfig/"
	rm -f "$BinDir/libx264.a"
fi

#libspeex
if [ ! -f "$BinDir/libspeex.so.1" ]; then 
	unpack_gzip_and_move "speex.tar.gz" "$PKG_DIR_LIBSPEEX"
	mkdir_and_move "$IntDir/libspeex"
	
	$PKG_DIR/configure --enable-sse --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libspeex.so" "1" "1.5.0" "libspeex.la" "speex.pc" "$LibDir"
	arrange_shared "$BinDir" "libspeexdsp.so" "1" "1.5.0" "libspeexdsp.la" "speexdsp.pc" "$LibDir"
fi

#libschroedinger (dirac support)
if [ ! -f "$BinDir/libschroedinger-1.0.so.0" ]; then 
	unpack_gzip_and_move "schroedinger.tar.gz" "$PKG_DIR_LIBSCHROEDINGER"
	mkdir_and_move "$IntDir/libschroedinger"
	
	$PKG_DIR/configure --with-thread=auto --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libschroedinger-1.0.so" "0" "0.2.0" "libschroedinger-1.0.la" "schroedinger-1.0.pc" "$LibDir"
fi

#mp3lame
if [ ! -f "$BinDir/libmp3lame.so.0" ]; then 
	unpack_gzip_and_move "lame.tar.gz" "$PKG_DIR_MP3LAME"
	mkdir_and_move "$IntDir/mp3lame"
	
	$PKG_DIR/configure --enable-expopt=no --enable-debug=no --disable-brhist -disable-frontend --enable-nasm --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libmp3lame.so" "0" "0.0.0" "libmp3lame.la" "" "$LibDir"
fi

#ffmpeg
FFmpegSuffix=-lgpl
if [ ! -f "$BinDir/libavcodec${FFmpegSuffix}.so.52" ]; then 
	unpack_bzip2_and_move "ffmpeg.tar.bz2" "$PKG_DIR_FFMPEG"
	mkdir_and_move "$IntDir/ffmpeg"
	
	#LGPL-compatible version
	CFLAGS=""
	CPPFLAGS=""
	LDFLAGS=""
	$PKG_DIR/configure --cc=$gcc --ld=$gcc --enable-runtime-cpudetect --enable-avfilter-lavf --enable-avfilter --enable-ffmpeg --enable-ffplay --disable-ffserver --disable-debug --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$LibDir --shlibdir=$BinDir --incdir=$IncludeDir 

	change_key "." "config.mak" "BUILDSUF" "${FFmpegSuffix}"
	#Adds $(SLIBPREF) to lib names when linking
	change_key "." "common.mak" "FFEXTRALIBS\ \\:" "\$\(addprefix\ -l,\$\(addsuffix\ \$\(BUILDSUF\),\$\(FFLIBS\)\)\)\ \$\(EXTRALIBS\)"

	make && make install
	
	reset_flags
	
	arrange_shared "$BinDir" "libavutil${FFmpegSuffix}.so" "50" "50.9.0" "" "libavutil.pc" "$LibDir"
	arrange_shared "$BinDir" "libavfilter${FFmpegSuffix}.so" "1" "1.17.0" "" "libavfilter.pc" "$LibDir"
	arrange_shared "$BinDir" "libavcodec${FFmpegSuffix}.so" "52" "52.55.0" "" "libavcodec.pc" "$LibDir"
	arrange_shared "$BinDir" "libavdevice${FFmpegSuffix}.so" "52" "52.2.0" "" "libavdevice.pc" "$LibDir"
	arrange_shared "$BinDir" "libavformat${FFmpegSuffix}.so" "52" "52.54.0" "" "libavformat.pc" "$LibDir"
	arrange_shared "$BinDir" "libswscale${FFmpegSuffix}.so" "0" "0.10.0" "" "libswscale.pc" "$LibDir"
	
	cd "$BinDir/"
	mv ffmpeg ffmpeg${FFmpegSuffix}
	mv ffplay ffplay${FFmpegSuffix}
	ln -fs ffmpeg${FFmpegSuffix} ffmpeg
	ln -fs ffplay${FFmpegSuffix} ffplay
	
	cd "$LibDir/pkgconfig/"
	sed -e "s/-lavcodec/-lavcodec${FFmpegSuffix}/g" libavcodec.pc > libavcodec${FFmpegSuffix}.pc
	sed -e "s/-lavdevice/-lavdevice${FFmpegSuffix}/g" libavdevice.pc > libavdevice${FFmpegSuffix}.pc
	sed -e "s/-lavfilter/-lavfilter${FFmpegSuffix}/g" libavfilter.pc > libavfilter${FFmpegSuffix}.pc
	sed -e "s/-lavformat/-lavformat${FFmpegSuffix}/g" libavformat.pc > libavformat${FFmpegSuffix}.pc
	sed -e "s/-lavutil/-lavutil${FFmpegSuffix}/g" libavutil.pc > libavutil${FFmpegSuffix}.pc
	sed -e "s/-lswscale/-lswscale${FFmpegSuffix}/g" libswscale.pc > libswscale${FFmpegSuffix}.pc
	
	ln -fs libavcodec${FFmpegSuffix}.pc libavcodec.pc
	ln -fs libavdevice${FFmpegSuffix}.pc libavdevice.pc
	ln -fs libavfilter${FFmpegSuffix}.pc libavfilter.pc
	ln -fs libavformat${FFmpegSuffix}.pc libavformat.pc
	ln -fs libavutil${FFmpegSuffix}.pc libavutil.pc
	ln -fs libswscale${FFmpegSuffix}.pc libswscale.pc
fi




#################
# GPL Libraries #
#################




#libnice
if [ ! -f "$BinDir/libnice.so.0" ]; then 
	unpack_gzip_and_move "libnice.tar.gz" "$PKG_DIR_LIBNICE"
	mkdir_and_move "$IntDir/libnice"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libnice.so" "0" "0.6.0" "libnice.la" "nice.pc" "$LibDir"
fi

#xvid
if [ ! -f "$BinDir/libxvidcore.so.4" ]; then
	unpack_gzip_and_move "xvidcore.tar.gz" "$PKG_DIR_XVIDCORE"
	mkdir_and_move "$IntDir/xvidcore"

	cd $PKG_DIR/build/generic/
	./configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libxvidcore.so" "4" "4.2" "" "" "$LibDir"
	rm -f "$BinDir/libxvidcore.a"
	chmod uag+x "$BinDir/libxvidcore.so.4"
fi

#wavpack
if [ ! -f "$BinDir/libwavpack.so.1" ]; then
	unpack_bzip2_and_move "wavpack.tar.bz2" "$PKG_DIR_WAVPACK"
	mkdir_and_move "$IntDir/wavpack"
	
	cp -p -f "$LIBRARIES_PATCH_DIR/wavpack/Makefile.in" "$PKG_DIR"
	dos2unix "$PKG_DIR/Makefile.in"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libwavpack.so" "1" "1.1.4" "libwavpack.la" "wavpack.pc" "$LibDir"
fi

#a52
if [ ! -f "$BinDir/liba52.so.0" ]; then
	unpack_gzip_and_move "a52.tar.gz" "$PKG_DIR_A52DEC"
	./bootstrap
	mkdir_and_move "$IntDir/a52dec"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "liba52.so" "0" "0.0.0" "liba52.la" "" "$LibDir"
fi

#mpeg2
if [ ! -f "$BinDir/libmpeg2.so.0" ]; then
	unpack_gzip_and_move "libmpeg2.tar.gz" "$PKG_DIR_LIBMPEG2"
	mkdir_and_move "$IntDir/libmpeg2"
	
	$PKG_DIR/configure --disable-sdl --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libmpeg2.so" "0" "0.1.0" "libmpeg2.la" "libmpeg2.pc" "$LibDir"
	arrange_shared "$BinDir" "libmpeg2convert.so" "0" "0.0.0" "libmpeg2convert.la" "libmpeg2convert.pc" "$LibDir"
fi

#libdca
if [ ! -f "$BinDir/libdca.so.0" ]; then
	unpack_bzip2_and_move "libdca.tar.bz2" "$PKG_DIR_LIBDCA"
	mkdir_and_move "$IntDir/libdca"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libdca.so" "0" "0.0.0" "libdca.la" "libdca.pc" "$LibDir"
	rm -f "$BinDir/libdts.a"
	rm -f "$BinDir/pkgconfig/libdts.pc"
	cd "$BinDir/"
	ln -s libdca.so libdts.so
	ln -s libdca.so.0 libdts.so.0
	cd "$LibDir/"
	ln -s libdca.la libdts.la
	cd "$LibDir/pkgconfig/"
	ln -s libdca.pc libdts.pc
fi

#faac
if [ ! -f "$BinDir/libfaac.so.0" ]; then
	unpack_bzip2_and_move "faac.tar.bz2" "$PKG_DIR_FAAC"
	mkdir_and_move "$IntDir/faac"

	$PKG_DIR/configure --without-mp4v2 --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libfaac.so" "0" "0.0.0" "libfaac.la" "" "$LibDir"
fi

#faad
if [ ! -f "$BinDir/libfaad.so.2" ]; then
	unpack_bzip2_and_move "faad2.tar.bz2" "$PKG_DIR_FAAD2"
	mkdir_and_move "$IntDir/faad2"
	
	cp "$LIBRARIES_PATCH_DIR/faad2/Makefile.in" .
	
	$PKG_DIR/configure --without-mp4v2 --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir
	make && make install

	arrange_shared "$BinDir" "libfaad.so" "2" "2.0.0" "libfaad.la" "" "$LibDir"
fi

#dvdread
if [ ! -f "$BinDir/libdvdread.so.4" ]; then
	unpack_bzip2_and_move "libdvdread.tar.bz2" "$PKG_DIR_LIBDVDREAD"
	mkdir_and_move "$IntDir/libdvdread"
	
	sh $PKG_DIR/autogen.sh --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir LDFLAGS="$LDFLAGS"
	make && make install
	
	cp -f "$LIBRARIES_PATCH_DIR/dvdread/dvd_reader.h" "$IncludeDir/dvdread/"	

	arrange_shared "$BinDir" "libdvdread.so" "4" "4.1.2" "libdvdread.la" "dvdread.pc" "$LibDir"
fi

#dvdnav
if [ ! -f "$BinDir/libdvdnav.so.4" ]; then
	unpack_bzip2_and_move "libdvdnav.tar.bz2" "$PKG_DIR_LIBDVDNAV"
	mkdir_and_move "$IntDir/libdvdnav"
	
	sh $PKG_DIR/autogen.sh --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir LDFLAGS="$LDFLAGS -ldvdread"
	make && make install

	arrange_shared "$BinDir" "libdvdnav.so" "4" "4.1.2" "libdvdnav.la" "dvdnav.pc" "$LibDir"
	arrange_shared "$BinDir" "libdvdnavmini.so" "4" "4.1.2" "libdvdnavmini.la" "dvdnavmini.pc" "$LibDir"
fi

#dvdcss
if [ ! -f "$BinDir/libdvdcss.so.2" ]; then
	unpack_bzip2_and_move "libdvdcss.tar.bz2" "$PKG_DIR_LIBDVDCSS"
	mkdir_and_move "$IntDir/libdvdcss"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir 
	make && make install

	arrange_shared "$BinDir" "libdvdcss.so" "2" "2.0.8" "libdvdcss.la" "" "$LibDir"
fi

FFmpegSuffix=-gpl
if [ ! -f "$BinDir/libavcodec${FFmpegSuffix}.so.52" ]; then 
	unpack_bzip2_and_move "ffmpeg.tar.bz2" "$PKG_DIR_FFMPEG"
	mkdir_and_move "$IntDir/ffmpeg${FFmpegSuffix}"
	
	#GPL-compatible version
	CFLAGS=""
	CPPFLAGS=""
	LDFLAGS=""
	$PKG_DIR/configure --cc=$gcc --ld=$gcc --enable-runtime-cpudetect --enable-avfilter-lavf --enable-avfilter --enable-ffmpeg --enable-ffplay --disable-ffserver --disable-debug --disable-static --enable-shared --enable-gpl --prefix=$InstallDir --bindir=$BinDir --libdir=$LibDir --shlibdir=$BinDir --incdir=$IncludeDir 

	change_key "." "config.mak" "BUILDSUF" "${FFmpegSuffix}"
	#Adds $(SLIBPREF) to lib names when linking
	change_key "." "common.mak" "FFEXTRALIBS\ \\:" "\$\(addprefix\ -l,\$\(addsuffix\ \$\(BUILDSUF\),\$\(FFLIBS\)\)\)\ \$\(EXTRALIBS\)"

	make && make install
	
	reset_flags
	
	arrange_shared "$BinDir" "libavutil${FFmpegSuffix}.so" "50" "50.9.0" "" "libavutil.pc" "$LibDir"
	arrange_shared "$BinDir" "libavfilter${FFmpegSuffix}.so" "1" "1.17.0" "" "libavfilter.pc" "$LibDir"
	arrange_shared "$BinDir" "libavcodec${FFmpegSuffix}.so" "52" "52.55.0" "" "libavcodec.pc" "$LibDir"
	arrange_shared "$BinDir" "libavdevice${FFmpegSuffix}.so" "52" "52.2.0" "" "libavdevice.pc" "$LibDir"
	arrange_shared "$BinDir" "libavformat${FFmpegSuffix}.so" "52" "52.54.0" "" "libavformat.pc" "$LibDir"
	arrange_shared "$BinDir" "libswscale${FFmpegSuffix}.so" "0" "0.10.0" "" "libswscale.pc" "$LibDir"
	
	cd "$BinDir/"
	mv ffmpeg ffmpeg${FFmpegSuffix}
	mv ffplay ffplay${FFmpegSuffix}
	ln -fs ffmpeg${FFmpegSuffix} ffmpeg
	ln -fs ffplay${FFmpegSuffix} ffplay
	
	cd "$LibDir/pkgconfig/"
	sed -e "s/-lavcodec/-lavcodec${FFmpegSuffix}/g" libavcodec.pc > libavcodec${FFmpegSuffix}.pc
	sed -e "s/-lavdevice/-lavdevice${FFmpegSuffix}/g" libavdevice.pc > libavdevice${FFmpegSuffix}.pc
	sed -e "s/-lavfilter/-lavfilter${FFmpegSuffix}/g" libavfilter.pc > libavfilter${FFmpegSuffix}.pc
	sed -e "s/-lavformat/-lavformat${FFmpegSuffix}/g" libavformat.pc > libavformat${FFmpegSuffix}.pc
	sed -e "s/-lavutil/-lavutil${FFmpegSuffix}/g" libavutil.pc > libavutil${FFmpegSuffix}.pc
	sed -e "s/-lswscale/-lswscale${FFmpegSuffix}/g" libswscale.pc > libswscale${FFmpegSuffix}.pc
	
	#Reset to LGPL version
	cd "$BinDir/"
	ln -fs ffmpeg-lgpl ffmpeg
	ln -fs ffplay-lgpl ffplay
fi

#Cleanup
rm -f "$BinDir/libmp4ff.a"
rm -rf "$BinDir/pkgconfig"
rm -rf "$BinDir/mozilla"

#Fix GTK paths
cd "$EtcDir/gtk-2.0/"
sed -e "s:$BinDir:../../lib:g" gdk-pixbuf.loaders > gdk-pixbuf.loaders.temp
mv gdk-pixbuf.loaders.temp gdk-pixbuf.loaders
grep -v -E 'Automatically generated|Created by|ModulesPath =' < gtk.immodules > gtk.immodules.temp 
mv gtk.immodules.temp gtk.immodules

#Fix Pango paths
cd "$EtcDir/pango/"
grep -v -E 'Automatically generated|Created by|ModulesPath =' < pango.modules > pango.modules.temp 
mv pango.modules.temp pango.modules

reset_flags

#Make sure the shared directory has all our updates
create_shared
rm -rf "$SharedShareDir/gdb/"

#Call common shutdown routines
common_shutdown

