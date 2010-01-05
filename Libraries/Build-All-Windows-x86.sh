#!/bin/sh

TOP=$(dirname $0)/..

CFLAGS="$CFLAGS -fno-strict-aliasing -fomit-frame-pointer -mms-bitfields -pipe -D_WIN32_WINNT=0x0501 -DUSE_GETADDRINFO -DHAVE_GETNAMEINFO -DHAVE_GETSOCKOPT -DHAVE_INET_NTOP -DHAVE_INET_PTON"
CPPFLAGS="$CPPFLAGS -DMINGW32 -D__MINGW32__"
LDFLAGS="-Wl,--enable-auto-image-base -Wl,--enable-auto-import -Wl,--enable-runtime-pseudo-reloc"
CXXFLAGS="${CFLAGS}"

##GCC 4.4+ doesn't set the stack alignment how we'd like which causes problems when 
##loading shared libraries such as the ffmpeg ones through Windows' LoadLibrary() function.
##CPPFLAGS="$CPPFLAGS -mincoming-stack-boundary=2"

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

#Generate libtool .la files
generate_all_wapi_libtool_la_x86

#clear_flags

#Not using dwarf2 yet
###gcc_dw2
##if [ ! -f "$BinDir/libgcc_s_dw2-1.dll" ]; then
##	#Needed for new dwarf2 exception handling
##	copy_files_to_dir "$LIBRARIES_PATCH_DIR/gcc_dw2/libgcc_s_dw2-1.dll" "$BinDir"
##fi

#proxy-libintl
if [ ! -f "$LibDir/intl.lib" ]; then 
	unpack_zip_and_move_windows "proxy-libintl.zip" "proxy-libintl" "proxy-libintl"
	mkdir_and_move "$IntDir/proxy-libintl"
	copy_files_to_dir "$PKG_DIR/*" .
	gcc -Wall -I"$IncludeDir" -c libintl.c
	ar rc "$LibDir/libintl.a" libintl.o
	cp "$LibDir/libintl.a" "$LibDir/intl.lib"
	strip --strip-unneeded "$LibDir/intl.lib"
	cp libintl.h "$IncludeDir"
	generate_libtool_la_windows "libintl.la" "" "libintl.a"
fi


#Update flags to make sure we don't try and export intl (gettext) functions more than once
#Setting it to ORIG_LDFLAGS ensures that any calls to reset_flags will include these changes.
ORIG_LDFLAGS="$ORIG_LDFLAGS -Wl,--exclude-libs=libintl.a -Wl,--add-stdcall-alias"
reset_flags


#liboil
if [ ! -f "$BinDir/lib${DefaultPrefix}oil-0.3-0.dll" ]; then
	unpack_gzip_and_move "liboil.tar.gz" "$PKG_DIR_LIBOIL"
	mkdir_and_move "$IntDir/liboil"
	
	CFLAGS="$CFLAGS -DHAVE_SYMBOL_UNDERSCORE=1"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	update_library_names_windows "lib${DefaultPrefix}oil-0.3.dll.a" "liboil-0.3.la"
	
	$MSLIB /name:lib${DefaultPrefix}oil-0.3-0.dll /out:oil-0.3.lib /machine:$MSLibMachine /def:liboil/.libs/lib${DefaultPrefix}oil-0.3-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	reset_flags
fi

#pthreads
if [ ! -f "$BinDir/lib${DefaultPrefix}pthreadGC2.dll" ]; then 
	unpack_gzip_and_move "pthreads-w32.tar.gz" "$PKG_DIR_PTHREADS"
	patch -u -N -i "$LIBRARIES_PATCH_DIR/pthreads-w32/sched.h.patch"
	mkdir_and_move "$IntDir/pthreads"
	
	cd "$PKG_DIR"
	change_package "lib${DefaultPrefix}pthreadGC\$(DLL_VER).dll" "." "GNUmakefile" "GC_DLL"
	make GC-inlined
	$MSLIB /name:lib${DefaultPrefix}pthreadGC2.dll /out:pthreadGC2.lib /machine:$MSLibMachine /def:pthread.def
	copy_files_to_dir "*.exp *.lib *.a" "$LibDir"
	copy_files_to_dir "*.dll" "$BinDir"
	copy_files_to_dir "pthread.h sched.h" "$IncludeDir"
	make clean
	
	generate_libtool_la_windows "libpthreadGC2.la" "lib${DefaultPrefix}pthreadGC2.dll" "libpthreadGC2.dll.a"
fi

#win-iconv
if [ ! -f "$LibDir/iconv.lib" ]; then 
	unpack_bzip2_and_move "win-iconv.tar.bz2" "$PKG_DIR_WIN_ICONV"
	mkdir_and_move "$IntDir/win-iconv"
	copy_files_to_dir "$LIBRARIES_DIR/Source/Win-Iconv/*.c $LIBRARIES_DIR/Source/Win-Iconv/*.h" .
	
	gcc -I"$IncludeDir" -O2 -DUSE_LIBICONV_DLL -c win_iconv.c 
	ar crv libiconv.a win_iconv.o 
	#gcc $(CFLAGS) -O2 -shared -o lib${DefaultPrefix}iconv.dll
	dlltool --export-all-symbols -D lib${DefaultPrefix}iconv.dll -l libiconv.dll.a -z in.def libiconv.a
	ranlib libiconv.dll.a
	gcc -shared -s -mwindows -def in.def -o lib${DefaultPrefix}iconv.dll libiconv.a
	cp iconv.h "$IncludeDir"
	
	$MSLIB /name:lib${DefaultPrefix}iconv.dll /out:iconv.lib /machine:$MSLibMachine /def:in.def
	move_files_to_dir "*.dll" "$BinDir"
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	copy_files_to_dir "*.dll.a" "$LibDir"
	
	generate_libtool_la_windows "libiconv.la" "lib${DefaultPrefix}iconv.dll" "libiconv.dll.a"
fi

#zlib
#Can't use separate build dir
if [ ! -f "$BinDir/lib${DefaultPrefix}z.dll" ]; then 
	unpack_zip_and_move_windows "zlib.zip" "zlib" "zlib"
	mkdir_and_move "$IntDir/zlib"
	cd "$PKG_DIR"
	
	#cp contrib/asm686/match.S ./match.S
	#make LOC=-DASMV OBJA=match.o -fwin32/Makefile.gcc
	change_package "lib${DefaultPrefix}z.dll" "win32" "Makefile.gcc" "SHAREDLIB"
	make -fwin32/Makefile.gcc lib${DefaultPrefix}z.dll
	INCLUDE_PATH=$IncludeDir LIBRARY_PATH=$BinDir make install -fwin32/Makefile.gcc
	
	cp -p lib${DefaultPrefix}z.dll "$BinDir"
	make clean -fwin32/Makefile.gcc
	
	mv "$BinDir/libzdll.a" "$LibDir/libz.dll.a"
	rm -f "$BinDir/libz.a"
	
	$MSLIB /name:lib${DefaultPrefix}z.dll /out:z.lib /machine:$MSLibMachine /def:win32/zlib.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	generate_libtool_la_windows "libz.la" "lib${DefaultPrefix}z.dll" "libz.dll.a"
fi

#bzip2
if [ ! -f "$BinDir/lib${DefaultPrefix}bz2.dll" ]; then 
	unpack_gzip_and_move "bzip2.tar.gz" "$PKG_DIR_BZIP2"
	mkdir_and_move "$IntDir/bzip2"
	
	cd "$PKG_DIR"
	
	gcc -DDLL_EXPORT -Wall -Winline -O2 -D_FILE_OFFSET_BITS=64 -c blocksort.c
	gcc -DDLL_EXPORT -Wall -Winline -O2 -D_FILE_OFFSET_BITS=64 -c huffman.c
	gcc -DDLL_EXPORT -Wall -Winline -O2 -D_FILE_OFFSET_BITS=64 -c crctable.c
	gcc -DDLL_EXPORT -Wall -Winline -O2 -D_FILE_OFFSET_BITS=64 -c randtable.c
	gcc -DDLL_EXPORT -Wall -Winline -O2 -D_FILE_OFFSET_BITS=64 -c compress.c
	gcc -DDLL_EXPORT -Wall -Winline -O2 -D_FILE_OFFSET_BITS=64 -c decompress.c
	gcc -DDLL_EXPORT -Wall -Winline -O2 -D_FILE_OFFSET_BITS=64 -c bzlib.c
	rm -f libbz2.a
	ar cq libbz2.a blocksort.o huffman.o crctable.o randtable.o compress.o decompress.o bzlib.o
	gcc -shared -o lib${DefaultPrefix}bz2.dll blocksort.o huffman.o crctable.o randtable.o compress.o decompress.o bzlib.o
	
	
	cp -p libbz2.a "$LibDir/libbz2.a"
	cp -p lib${DefaultPrefix}bz2.dll "$BinDir/"
	
	pexports lib${DefaultPrefix}bz2.dll > "in.def"
	sed -e 's/DATA//g' in.def > in-mod.def
	dlltool --dllname lib${DefaultPrefix}bz2.dll -d in-mod.def -l libbz2.dll.a
	cp -p libbz2.dll.a "$LibDir/"
	
	make
	make install PREFIX=$InstallDir
	remove_files_from_dir "*.exe"
	remove_files_from_dir "*.so.*"
	
	$MSLIB /name:lib${DefaultPrefix}bz2.dll /out:bz2.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	copy_files_to_dir "bzlib.h" "$IncludeDir"
	
	generate_libtool_la_windows "libbz2.la" "lib${DefaultPrefix}bz2.dll" "libbz2.dll.a"
	
	reset_flags
fi

#glew
if [ ! -f "$BinDir/lib${DefaultPrefix}glew32.dll" ]; then
	unpack_gzip_and_move "glew.tar.gz" "$PKG_DIR_GLEW"
	mkdir_and_move "$IntDir/glew"
	
	cd "$PKG_DIR"
	change_package "lib${DefaultPrefix}\$(NAME).dll" "config" "Makefile.mingw" "LIB.SONAME"
	change_package "lib${DefaultPrefix}\$(NAME).dll" "config" "Makefile.mingw" "LIB.SHARED"
	make
	
	cd "lib"
	strip "libglew32.dll.a"
	
	pexports "lib${DefaultPrefix}glew32.dll" > in.def
	sed -e '/LIBRARY glew32/d' -e 's/DATA//g' in.def > in-mod.def
	$MSLIB /name:lib${DefaultPrefix}glew32.dll /out:glew32.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	cp -f "lib${DefaultPrefix}glew32.dll" "$BinDir"
	cp -f "libglew32.dll.a" "$LibDir"
	
	cd "../include/GL/"
	mkdir -p "$IncludeDir/GL/"
	copy_files_to_dir "glew.h wglew.h" "$IncludeDir/GL/"
	
	generate_libtool_la_windows "libglew32.la" "lib${DefaultPrefix}glew32.dll" "libglew32.dll.a"
fi

#expat
if [ ! -f "$BinDir/lib${DefaultPrefix}expat-1.dll" ]; then
	unpack_gzip_and_move "expat.tar.gz" "$PKG_DIR_EXPAT"
	mkdir_and_move "$IntDir/expat"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	update_library_names_windows "lib${DefaultPrefix}expat.dll.a" "libexpat.la"
	
	copy_files_to_dir "$PKG_DIR/lib/libexpat.def" "$IntDir/expat"
	$MSLIB /name:lib${DefaultPrefix}expat-1.dll /out:expat.lib /machine:$MSLibMachine /def:libexpat.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
fi

#libxml2
if [ ! -f "$BinDir/lib${DefaultPrefix}xml2-2.dll" ]; then 
	unpack_gzip_and_move "libxml2.tar.gz" "$PKG_DIR_LIBXML2"
	mkdir_and_move "$IntDir/libxml2"
	
	#cd "$LIBRARIES_DIR/LibXML2/Source/win32"
	#cscript configure.js iconv=no xml_debug=no static=yes debug=no threads=native zlib=yes vcmanifest=no compiler=mingw "prefix=$InstallDir" "incdir=$IncludeDir" "libdir=$LibDir" "sodir=$BinDir"
	
	#Compiling with iconv causes an odd dependency on "in.dll" which I think is an intermediary for iconv.a
	$PKG_DIR/configure --with-zlib --with-iconv --with-threads=native --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	
	#We want to ignore pthreads and use native threads. Configure does not 
	#respect our desire so we manipulate the config.h file.
	sed -r s/HAVE_PTHREAD_H/HAVE_WIN32_THREADS/g config.h > tmp1.txt
	sed -r s/HAVE_LIBPTHREAD/HAVE_WIN32/g tmp1.txt > tmp.txt
	rm -f tmp1.txt
	mv tmp.txt config.h
	
	#Build forgets to reference testapi.c correctly
	cp $PKG_DIR/testapi.c .
	
	make && make install
	
	update_library_names_windows "lib${DefaultPrefix}xml2.dll.a" "libxml2.la"
	
	#Preprocess-only the .def.src file
	#The preprocessor generates some odd "# 1" statements so we want to eliminate those
	CFLAGS="$CFLAGS -I$IncludeDir/libxml2 -I$SharedIncludeDir/libxml2"
	gcc $CFLAGS -x c -E -D _REENTRANT $PKG_DIR/win32/libxml2.def.src > tmp1.txt
	sed '/# /d' tmp1.txt > tmp2.txt
	sed '/LIBRARY libxml2/d' tmp2.txt > libxml2.def
	reset_flags
	
	#Use the output .def file to generate an MS-compatible lib file
	$MSLIB /name:lib${DefaultPrefix}xml2-2.dll /out:xml2.lib /machine:$MSLibMachine /def:libxml2.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	strip "$LibDir\libxml2.dll.a"
fi

#libjpeg
if [ ! -f "$BinDir/lib${DefaultPrefix}jpeg-7.dll" ]; then 
	unpack_gzip_and_move "jpegsrc.tar.gz" "$PKG_DIR_LIBJPEG"
	mkdir_and_move "$IntDir/libjpeg"
	
	CFLAGS="$CFLAGS -O2"
	
	#Configure, compile, and install
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make
	make install
	
	pexports "$BinDir/libjpeg-7.dll" > in.def
	sed -e '/LIBRARY libjpeg/d' -e 's/DATA//g' in.def > in-mod.def
	$MSLIB /name:lib${DefaultPrefix}jpeg-7.dll /out:jpeg.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	update_library_names_windows "lib${DefaultPrefix}jpeg.dll.a" "libjpeg.la"
	
	reset_flags
fi

#openjpeg
if [ ! -f "$BinDir/lib${DefaultPrefix}openjpeg-2.dll" ]; then 
	unpack_gzip_and_move "openjpeg.tar.gz" "$PKG_DIR_OPENJPEG"
	mkdir_and_move "$IntDir/openjpeg"
	
	cd "$PKG_DIR"
	cp "$LIBRARIES_PATCH_DIR/openjpeg/Makefile" .
	
	change_package "${DefaultPrefix}openjpeg" "." "Makefile" "TARGET"
	make install LDFLAGS="-lm" PREFIX=$InstallDir
	make clean
	
	cd "$IntDir/openjpeg"
	pexports "$BinDir/lib${DefaultPrefix}openjpeg-2.dll" | sed "s/^_//" > in.def
	$MSLIB /name:lib${DefaultPrefix}openjpeg-2.dll /out:openjpeg.lib /machine:$MSLibMachine /def:in.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	update_library_names_windows "lib${DefaultPrefix}openjpeg.dll.a"
fi

#libpng
if [ ! -f "$BinDir/lib${DefaultPrefix}png12-0.dll" ]; then 
	unpack_gzip_and_move "libpng.tar.gz" "$PKG_DIR_LIBPNG"
	mkdir_and_move "$IntDir/libpng"	
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	remove_files_from_dir "$BinDir/lib${DefaultPrefix}png-3.dll"
	remove_files_from_dir "$LibDir/lib${DefaultPrefix}png.dll.a"
	
	pexports "$BinDir/lib${DefaultPrefix}png12-0.dll" > in.def
	sed -e '/LIBRARY libpng/d' -e '/DATA/d' in.def > in-mod.def
	$MSLIB /name:lib${DefaultPrefix}png12-0.dll /out:png12.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	update_library_names_windows "lib${DefaultPrefix}png12.dll.a" "libpng12.la"
	cp -f -p "$LibDir\libpng12.la" "$LibDir\libpng.la" 
fi

#glib
if [ ! -f "$BinDir/lib${DefaultPrefix}glib-2.0-0.dll" ]; then 
	unpack_bzip2_and_move "glib.tar.bz2" "$PKG_DIR_GLIB"
	mkdir_and_move "$IntDir/glib"
	
	#Need to get rid of MS build tools b/c the makefile call is incorrectly passing it msys-style paths.
	reset_path
	
	$PKG_DIR/configure --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	#Add in MS build tools again
	setup_ms_build_env_path
	
	cd "$IntDir/glib/gio/.libs"
	$MSLIB /name:lib${DefaultPrefix}gio-2.0-0.dll /out:gio-2.0.lib /machine:$MSLibMachine /def:lib${DefaultPrefix}gio-2.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	cd "../../glib/.libs"
	$MSLIB /name:lib${DefaultPrefix}glib-2.0-0.dll /out:glib-2.0.lib /machine:$MSLibMachine /def:lib${DefaultPrefix}glib-2.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	cd "../../gmodule/.libs"
	$MSLIB /name:lib${DefaultPrefix}gmodule-2.0-0.dll /out:gmodule-2.0.lib /machine:$MSLibMachine /def:lib${DefaultPrefix}gmodule-2.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	cd "../../gobject/.libs"
	$MSLIB /name:lib${DefaultPrefix}gobject-2.0-0.dll /out:gobject-2.0.lib /machine:$MSLibMachine /def:lib${DefaultPrefix}gobject-2.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	cd "../../gthread/.libs"
	$MSLIB /name:lib${DefaultPrefix}gthread-2.0-0.dll /out:gthread-2.0.lib /machine:$MSLibMachine /def:lib${DefaultPrefix}gthread-2.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	cd "$LibDir"
	remove_files_from_dir "g*-2.0.def"
	
	#This is silly - but glib 2.21.4 (at least) doesn't copy this config file even when it's needed.
	#See bug 592773 for more information: http://bugzilla.gnome.org/show_bug.cgi?id=592773
	cp -f "$PKG_DIR/glibconfig.h.win32" "$IncludeDir/glib-2.0/glibconfig.h"
	
	update_library_names_windows "lib${DefaultPrefix}glib-2.0.dll.a" "libglib-2.0.la"
	update_library_names_windows "lib${DefaultPrefix}gio-2.0.dll.a" "libgio-2.0.la"
	update_library_names_windows "lib${DefaultPrefix}gmodule-2.0.dll.a" "libgmodule-2.0.la"
	update_library_names_windows "lib${DefaultPrefix}gobject-2.0.dll.a" "libgobject-2.0.la"
	update_library_names_windows "lib${DefaultPrefix}gthread-2.0.dll.a" "libgthread-2.0.la"
fi

#openssl
#if [ ! -f "$LibDir/libcrypto.so" ]; then 
#	unpack_gzip_and_move "openssl.tar.gz" "$PKG_DIR_OPENSSL"
#	mkdir_and_move "$IntDir/openssl"
#	cd "$PKG_DIR/"
#	
#	echo Building OpenSSL...
#	cp ../mingw.bat ms/
#	cmd.exe /c "ms\mingw.bat"
#	rm -f ms/mingw.bat
#fi

#libgpg-error
if [ ! -f "$BinDir/lib${DefaultPrefix}gpg-error-0.dll" ]; then 
	unpack_bzip2_and_move "libgpg-error.tar.bz2" "$PKG_DIR_LIBGPG_ERROR"
	mkdir_and_move "$IntDir/libgpg-error"
	
	CFLAGS=$ORIG_CFLAGS
	CPPFLAGS=$ORIG_CPPFLAGS
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	
	#This file was being incorrectly linked. The script points to src/versioninfo.o when it should be 
	#src/.libs/versioninfo.o. This attempts to correct this simply by copying the .o file to the src/ dir.
	cd src
	make versioninfo.o
	cp .libs/versioninfo.o .
	cd ..
	
	#For whatever reason, CPPFLAGS is still being set. We don't want that so ensure it's blank...
	make CPPFLAGS=
	make install
	
	$MSLIB /name:lib${DefaultPrefix}gpg-error-0.dll /out:gpg-error.lib /machine:$MSLibMachine /def:src/.libs/lib${DefaultPrefix}gpg-error-0.dll.def
	
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	update_library_names_windows "lib${DefaultPrefix}gpg-error.dll.a" "libgpg-error.la"
	
	reset_flags
fi

#libgcrypt
if [ ! -f "$BinDir/lib${DefaultPrefix}gcrypt-11.dll" ]; then 
	unpack_bzip2_and_move "libgcrypt.tar.bz2" "$PKG_DIR_LIBGCRYPT"
	mkdir_and_move "$IntDir/libgcrypt"
	
	CFLAGS=$ORIG_CFLAGS
	CPPFLAGS=$ORIG_CPPFLAGS
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	
	reset_flags
	
	make && make install
	
	$MSLIB /name:lib${DefaultPrefix}gcrypt-11.dll /out:gcrypt.lib /machine:$MSLibMachine /def:src/.libs/lib${DefaultPrefix}gcrypt-11.dll.def
	
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	cd "$LibDir"
	rm -f "libgcrypt.def"
	
	update_library_names_windows "lib${DefaultPrefix}gcrypt.dll.a" "libgcrypt.la"
fi

#libtasn1
if [ ! -f "$BinDir/lib${DefaultPrefix}tasn1-3.dll" ]; then 
	unpack_gzip_and_move "libtasn1.tar.gz" "$PKG_DIR_LIBTASN1"
	mkdir_and_move "$IntDir/libtasn1"
	
	CFLAGS=$ORIG_CFLAGS
	CPPFLAGS=$ORIG_CPPFLAGS
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	
	reset_flags
	
	make && make install

	pexports "$BinDir/lib${DefaultPrefix}tasn1-3.dll" > in.def
	sed -e "/LIBRARY lib${DefaultPrefix}tasn1-3.dll/d" -e '/DATA/d' in.def > in-mod.def

	$MSLIB /name:lib${DefaultPrefix}tasn1-3.dll /out:tasn1.lib /machine:$MSLibMachine /def:in-mod.def
	
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	cd "$LibDir"
	rm -f "libtasn1.def"
	
	update_library_names_windows "lib${DefaultPrefix}tasn1.dll.a" "libtasn1.la"
fi

#gnutls
if [ ! -f "$BinDir/lib${DefaultPrefix}gnutls-26.dll" ]; then 
	unpack_bzip2_and_move "gnutls.tar.bz2" "$PKG_DIR_GNUTLS"
	mkdir_and_move "$IntDir/gnutls"
	
	CFLAGS="-I$PKG_DIR/lib/includes -I$IntDir/gnutls/lib/includes"
	CPPFLAGS="-I$PKG_DIR/lib/includes -I$IntDir/gnutls/lib/includes"
	#LDFLAGS="-Wl,-static-libstdc++"
	
	$PKG_DIR/configure --disable-cxx --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	change_libname_spec "${DefaultPrefix}" "${DefaultSuffix}" "lib"
	change_libname_spec "${DefaultPrefix}" "${DefaultSuffix}" "libextra"
	make && make install
	
	$MSLIB /name:lib${DefaultPrefix}gnutls-26.dll /out:gnutls.lib /machine:$MSLibMachine /def:lib/libgnutls-26.def
	$MSLIB /name:lib${DefaultPrefix}gnutls-extra-26.dll /out:gnutls-extra.lib /machine:$MSLibMachine /def:libextra/libgnutls-extra-26.def
	$MSLIB /name:lib${DefaultPrefix}gnutls-openssl-26.dll /out:gnutls-openssl.lib /machine:$MSLibMachine /def:libextra/libgnutls-openssl-26.def
	
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	cd "$BinDir" && remove_files_from_dir "libgnutls-*.def"
	
	reset_flags
	
	update_library_names_windows "lib${DefaultPrefix}gnutls.dll.a" "libgnutls.la"
	update_library_names_windows "lib${DefaultPrefix}gnutls-extra.dll.a" "libgnutls-extra.la"
	update_library_names_windows "lib${DefaultPrefix}gnutls-openssl.dll.a" "libgnutls-openssl.la"
fi

#soup
if [ ! -f "$BinDir/lib${DefaultPrefix}soup-2.4-1.dll" ]; then 
	unpack_bzip2_and_move "libsoup.tar.bz2" "$PKG_DIR_LIBSOUP"
	mkdir_and_move "$IntDir/libsoup"
	
	$PKG_DIR/configure --disable-silent-rules --disable-glibtest --enable-ssl --enable-debug=no --disable-more-warnings --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	change_package '"dlltool -U"' "." "libtool" "DLLTOOL"
	make && make install
	
	exit 0

	cd "libsoup/.libs"
	pexports "$BinDir/libsoup-2.4-1.dll" > in.def
	sed -e '/LIBRARY libsoup/d' -e 's/DATA//g' in.def > in-mod.def
	
	$MSLIB /name:lib${DefaultPrefix}soup-2.4-1.dll /out:soup-2.4.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi
exit 0
#neon
if [ ! -f "$BinDir/libneon-27.dll" ]; then 
	unpack_gzip_and_move "neon.tar.gz" "$PKG_DIR_NEON"
	mkdir_and_move "$IntDir/neon"
	
	CPPFLAGS="$CPPFLAGS -D_WIN32_WINNT=0x0501 -DUSE_GETADDRINFO -DHAVE_GETNAMEINFO -DHAVE_GETSOCKOPT -DHAVE_INET_NTOP -DHAVE_INET_PTON -UHAVE_SETLOCALE"
	
	$PKG_DIR/configure --with-libxml2 --with-ssl=gnutls --disable-debug --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	cd "src/.libs"
	pexports "$BinDir/libneon-27.dll" | sed "s/^_//" > in.def
	sed -e '/LIBRARY libneon-27.dll/d' in.def > in-mod.def
	$MSLIB /name:libneon-27.dll /out:neon.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
fi

#freetype
if [ ! -f "$BinDir/libfreetype-6.dll" ]; then 
	unpack_bzip2_and_move "freetype.tar.bz2" "$PKG_DIR_FREETYPE"
	mkdir_and_move "$IntDir/freetype"
	
	#cp -f "$LIBRARIES_PATCH_DIR/freetype/ftoption_original.h" "$PKG_DIR/include/freetype/config/ftoption.h"
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	#cp -f "$LIBRARIES_PATCH_DIR/freetype/ftoption_windows.h" "$PKG_DIR/include/freetype/config/ftoption.h"	
	
	cp -p .libs/libfreetype.dll.a "$LibDir/freetype.lib"
fi

#fontconfig
if [ ! -f "$BinDir/libfontconfig-1.dll" ]; then 
	unpack_gzip_and_move "fontconfig.tar.gz" "$PKG_DIR_FONTCONFIG"
	mkdir_and_move "$IntDir/fontconfig"
	
	CFLAGS="-D_WIN32_WINNT=0x0501"
	$PKG_DIR/configure --enable-libxml2 --disable-debug --disable-static --disable-docs --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install RUN_FC_CACHE_TEST=false
	
	#Automagically creates .lib file
	
	cd "$LibDir" && remove_files_from_dir "fontconfig.def"
	
	reset_flags
fi

#pixman
if [ ! -f "$BinDir/libpixman-1-0.dll" ]; then 
	unpack_gzip_and_move "pixman.tar.gz" "$PKG_DIR_PIXMAN"
	patch -u -N -i "$LIBRARIES_PATCH_DIR/pixman/blitters-test-win32.patch"
	
	mkdir_and_move "$IntDir/pixman"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	cd "$LIBRARIES_PATCH_DIR/pixman/"
	$MSLIB /name:libpixman-1-0.dll /out:pixman.lib /machine:$MSLibMachine /def:pixman.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#cairo
if [ ! -f "$BinDir/libcairo-2.dll" ]; then 
	unpack_gzip_and_move "cairo.tar.gz" "$PKG_DIR_CAIRO"
	mkdir_and_move "$IntDir/cairo"
	
	CFLAGS="$CFLAGS -D CAIRO_HAS_WIN32_SURFACE -D CAIRO_HAS_WIN32_FONT -Wl,-lpthreadGC2"
	$PKG_DIR/configure --enable-xlib=auto --enable-xlib-xrender=auto --enable-png=yes --enable-ft=yes --enable-pdf=yes --enable-svg=yes --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	make && make install
	
	cd src/.libs/
	$MSLIB /name:libcairo-2.dll /out:cairo.lib /machine:$MSLibMachine /def:libcairo-2.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
fi

#pango
if [ ! -f "$BinDir/libpango-1.0-0.dll" ]; then 
	unpack_bzip2_and_move "pango.tar.bz2" "$PKG_DIR_PANGO"
	mkdir_and_move "$IntDir/pango"
	
	#Need to get rid of MS build tools b/c the makefile call is incorrectly passing it msys-style paths.
	reset_path
	
	$PKG_DIR/configure --with-included-modules --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	#Pango incorrectly creates pango-enum-types.h by running glib-mkenums and it's somehow inserting a path in the fprods like: C:/msys
	#Since it already has pre-made ones, we just copy those over and use them
	copy_files_to_dir "$PKG_DIR/pango/pango-enum-types.h" "$IntDir/pango/pango"
	copy_files_to_dir "$PKG_DIR/pango/pango-enum-types.c" "$IntDir/pango/pango"
	
	make && make install
	
	#Add in MS build tools again
	setup_ms_build_env_path
	
	cd "$IntDir/pango/pango/.libs/"
	$MSLIB /name:libpango-1.0-0.dll /out:pango-1.0.lib /machine:$MSLibMachine /def:libpango-1.0-0.dll.def
	$MSLIB /name:libpangoft2-1.0-0.dll /out:pangoft2-1.0.lib /machine:$MSLibMachine /def:libpangoft2-1.0-0.dll.def
	$MSLIB /name:libpangowin32-1.0-0.dll /out:pangowin32-1.0.lib /machine:$MSLibMachine /def:libpangowin32-1.0-0.dll.def
	$MSLIB /name:libpangocairo-1.0-0.dll /out:pangocairo-1.0.lib /machine:$MSLibMachine /def:libpangocairo-1.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#sdl
if [ ! -f "$BinDir/SDL.dll" ]; then 
	unpack_gzip_and_move "sdl.tar.gz" "$PKG_DIR_SDL"
	mkdir_and_move "$IntDir/sdl"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir 
	make && make install
	
	cp $PKG_DIR/include/SDL_config.h.default $IncludeDir/SDL/SDL_config.h
	cp $PKG_DIR/include/SDL_config_win32.h $IncludeDir/SDL
	
	cd build/.libs
	
	pexports "$BinDir/SDL.dll" | sed "s/^_//" > in.def
	sed -e '/LIBRARY SDL.dll/d' in.def > in-mod.def
	$MSLIB /name:SDL.dll /out:sdl.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	reset_flags
fi

#libogg
if [ ! -f "$BinDir/libogg-0.dll" ]; then 
	unpack_gzip_and_move "libogg.tar.gz" "$PKG_DIR_LIBOGG"
	mkdir_and_move "$IntDir/libogg"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	copy_files_to_dir "$PKG_DIR/win32/ogg.def" .
	$MSLIB /name:libogg-0.dll /out:ogg.lib /machine:$MSLibMachine /def:ogg.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#libvorbis
if [ ! -f "$BinDir/libvorbis-0.dll" ]; then 
	unpack_bzip2_and_move "libvorbis.tar.bz2" "$PKG_DIR_LIBVORBIS"
	mkdir_and_move "$IntDir/libvorbis"
	
	LDFLAGS="$LDFLAGS -logg"
	$PKG_DIR/configure --disable-oggtest --with-ogg-libraries=$LibDir --with-ogg-includes=$IncludeDir --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	copy_files_to_dir "$PKG_DIR/win32/*.def" .
	sed '/vorbis_encode_*/d' vorbis.def > vorbis-mod.def
	$MSLIB /name:libvorbis-0.dll /out:vorbis.lib /machine:$MSLibMachine /def:vorbis-mod.def
	$MSLIB /name:libvorbisenc-2.dll /out:vorbisenc.lib /machine:$MSLibMachine /def:vorbisenc.def
	$MSLIB /name:libvorbisfile-3.dll /out:vorbisfile.lib /machine:$MSLibMachine /def:vorbisfile.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
fi

#libcelt
if [ ! -f "$BinDir/libcelt-0.dll" ]; then 
	unpack_gzip_and_move "libcelt.tar.gz" "$PKG_DIR_LIBCELT"
	mkdir_and_move "$IntDir/libcelt"
	
	LDFLAGS="$LDFLAGS -logg"
	$PKG_DIR/configure --disable-oggtest --with-ogg-libraries=$LibDir --with-ogg-includes=$IncludeDir --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	copy_files_to_dir "$LIBRARIES_PATCH_DIR/celt/*.def" .
	$MSLIB /name:libcelt-0.dll /out:celt.lib /machine:$MSLibMachine /def:libcelt.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
fi

#libtheora
if [ ! -f "$BinDir/libtheora-0.dll" ]; then 
	unpack_bzip2_and_move "libtheora.tar.bz2" "$PKG_DIR_LIBTHEORA"
	mkdir_and_move "$IntDir/libtheora"
	
	$PKG_DIR/configure --disable-examples --disable-oggtest --disable-vorbistest --disable-sdltest --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	copy_files_to_dir "$LIBRARIES_PATCH_DIR/theora/win32/*def" .
	copy_files_to_dir "lib/.libs/*.def" .
	flip -d libtheora.def
	sed -e '/LIBRARY	libtheora/d' libtheora.def > libtheora-mod.def
	$MSLIB /name:libtheora-0.dll /out:theora.lib /machine:$MSLibMachine /def:libtheora-mod.def
	$MSLIB /name:libtheoradec-1.dll /out:theoradec.lib /machine:$MSLibMachine /def:libtheoradec-1.dll.def
	$MSLIB /name:libtheoraenc-1.dll /out:theoraenc.lib /machine:$MSLibMachine /def:libtheoraenc-1.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#libmms
if [ ! -f "$BinDir/libmms-0.dll" ]; then 
	unpack_bzip2_and_move "libmms.tar.bz2" "$PKG_DIR_LIBMMS"
	mkdir_and_move "$IntDir/libmms"
	
	CFLAGS="$CFLAGS -D LIBMMS_HAVE_64BIT_OFF_T"
	LDFLAGS="$LDFLAGS -lwsock32 -lglib-2.0 -lgobject-2.0"
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	make && make install
	
	copy_files_to_dir "$LIBRARIES_PATCH_DIR/libmms/*.def" .
	$MSLIB /name:libmms-0.dll /out:mms.lib /machine:$MSLibMachine /def:libmms.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
fi

#x264
if [ ! -f "$BinDir/libx264-67.dll" ]; then 
	unpack_bzip2_and_move "x264.tar.bz2" "$PKG_DIR_X264"
	mkdir_and_move "$IntDir/x264"
	
	PATH=$PATH:$TOOLS_DIR
	
	cd "$PKG_DIR/"
	CFLAGS="$CFLAGS -D X264_CPU_SHUFFLE_IS_FAST=0x000800"
	./configure --disable-mp4-output --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	make clean && make distclean
	reset_flags
	
	reset_path
	setup_ms_build_env_path
	
	cd "$IntDir/x264"
	rm -rf "$LibDir/libx264.a"
	copy_files_to_dir "$LIBRARIES_PATCH_DIR/x264/libx264.def" .
	$MSLIB /name:libx264-67.dll /out:x264.lib /machine:$MSLibMachine /def:libx264.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#libspeex
if [ ! -f "$BinDir/libspeex-1.dll" ]; then 
	unpack_gzip_and_move "speex.tar.gz" "$PKG_DIR_LIBSPEEX"
	mkdir_and_move "$IntDir/libspeex"
	
	$PKG_DIR/configure --disable-oggtest --enable-sse --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	copy_files_to_dir "$PKG_DIR/win32/*.def" .
	
	#Remove the explicit library declaration since it overrides our commandline one
	sed '/LIBRARY libspeex/d' libspeex.def > libspeex-mod.def
	sed '/LIBRARY libspeexdsp/d' libspeexdsp.def > libspeexdsp-mod.def
	
	#Add some missing functions from our lib to the def file
	echo speex_nb_mode >> libspeex-mod.def
	echo speex_uwb_mode >> libspeex-mod.def
	echo speex_wb_mode >> libspeex-mod.def
	echo speex_header_free >> libspeex-mod.def
	echo speex_mode_list >> libspeex-mod.def
	
	$MSLIB /name:libspeex-1.dll /out:speex.lib /machine:$MSLibMachine /def:libspeex-mod.def
	$MSLIB /name:libspeexdsp-1.dll /out:speexdsp.lib /machine:$MSLibMachine /def:libspeexdsp-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#libschroedinger (dirac support)
if [ ! -f "$BinDir/libschroedinger-1.0-0.dll" ]; then 
	unpack_gzip_and_move "schroedinger.tar.gz" "$PKG_DIR_LIBSCHROEDINGER"
	
	mkdir_and_move "$IntDir/libschroedinger"
	
	LDFLAGS="-lstdc++_s"
	$PKG_DIR/configure --with-thread=auto --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	cd "schroedinger/.libs"
	$MSLIB /name:libschroedinger-1.0-0.dll /out:schroedinger-1.0.lib /machine:$MSLibMachine /def:libschroedinger-1.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
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

#mp3lame
if [ ! -f "$BinDir/libmp3lame-0.dll" ]; then 
	unpack_gzip_and_move "lame.tar.gz" "$PKG_DIR_MP3LAME"
	mkdir_and_move "$IntDir/mp3lame"
	
	$PKG_DIR/configure --enable-expopt=no --enable-debug=no --disable-brhist -disable-frontend --enable-nasm --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	pexports "$BinDir/libmp3lame-0.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY libmp3lame-0.dll/d' in.def > in-mod.def
	$MSLIB /name:libmp3lame-0.dll /out:mp3lame.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#ffmpeg
if [ ! -f "$BinDir/avcodec-52.dll" ]; then 
	#We want to use the GStreamer-supplied version of ffmpeg in case there are differences
	PKG_DIR="$MAIN_DIR/GStreamer/Source/gst-ffmpeg/gst-libs/ext/ffmpeg"
	
	#unpack_bzip2_and_move "ffmpeg.tar.bz2" "$PKG_DIR_FFMPEG"
	mkdir_and_move "$IntDir/ffmpeg"
	
	#LGPL-compatible version
	#On Windows, you must disable the roq decoder in order to build shared libraries. This is a known bug.
	$PKG_DIR/configure --extra-ldflags="$LibFlags -Wl,--exclude-libs=libintl.a -Wl,--add-stdcall-alias" --extra-cflags="$IncludeFlags -mno-cygwin -mms-bitfields -fno-common -fno-strict-aliasing -D_WIN32_WINNT=0x0501 -DUSE_GETADDRINFO -DHAVE_GETNAMEINFO -DHAVE_GETSOCKOPT -DHAVE_INET_NTOP -DHAVE_INET_PTON" --disable-encoder=roq_dpcm --enable-avfilter-lavf --enable-avfilter --disable-vhook --enable-avisynth --target-os=mingw32 --arch=i686 --cpu=i686 --enable-memalign-hack --enable-zlib --enable-bzlib --enable-w32threads --enable-libmp3lame --enable-libvorbis --enable-libopenjpeg --enable-libtheora --enable-libspeex --enable-libschroedinger --enable-ffmpeg --disable-ffplay --disable-ffserver --disable-debug --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$LibDir --shlibdir=$BinDir --incdir=$IncludeDir
	
	make && make install
	
	##If it built successfully, then the .lib and .dll files are all in the lib/ folder with 
	##sym links. We want to take out the sym links and keep just the .lib and .dll files we need 
	##for development and execution.
	cd "$BinDir" && move_files_to_dir "av*.lib" "$LibDir"
	cd "$BinDir" && remove_files_from_dir "avcodec-52.*.dll avcodec.dll avdevice-52.*.dll avdevice.dll avfilter-0.*.dll avfilter.dll avformat-52.*.dll avformat.dll avutil-49.*.dll avutil.dll"
	cd "$LibDir" && remove_files_from_dir "avcodec-52.lib avdevice-52.lib avfilter-0.lib avformat-52.lib avutil-49.lib"

	#Create .dll.a versions of the libs
	cd "$IntDir/ffmpeg"
	pexports "$BinDir/avutil-49.dll" | sed "s/^_//" > avutil.def
	pexports "$BinDir/avcodec-52.dll" | sed "s/^_//" > avcodec.def
	pexports "$BinDir/avdevice-52.dll" | sed "s/^_//" > avdevice.def
	pexports "$BinDir/avfilter-0.dll" | sed "s/^_//" > avfilter.def
	pexports "$BinDir/avformat-52.dll" | sed "s/^_//" > avformat.def
	dlltool -U -d avutil.def -l libavutil.dll.a
	dlltool -U -d avcodec.def -l libavcodec.dll.a
	dlltool -U -d avdevice.def -l libavdevice.dll.a
	dlltool -U -d avfilter.def -l libavfilter.dll.a
	dlltool -U -d avformat.def -l libavformat.dll.a
	
	move_files_to_dir "*.dll.a" "$LibDir/"
	
	reset_flags
fi


#################
# GPL Libraries #
#################



#libnice
if [ ! -f "$BinDir/libnice-0.dll" ]; then 
	unpack_gzip_and_move "libnice.tar.gz" "$PKG_DIR_LIBNICE"
	patch -u -N -i "$LIBRARIES_PATCH_DIR/libnice/bind.c-win32.patch"
	patch -u -N -i "$LIBRARIES_PATCH_DIR/libnice/rand.c-win32.patch"
	patch -u -N -i "$LIBRARIES_PATCH_DIR/libnice/address.h-win32.patch"
	patch -u -N -i "$LIBRARIES_PATCH_DIR/libnice/interfaces.c-win32.patch"
	
	mkdir_and_move "$IntDir/libnice"
	
	CFLAGS="-D_SSIZE_T_ -I$PKG_DIR -I$PKG_DIR/stun -D_WIN32_WINNT=0x0501 -DUSE_GETADDRINFO -DHAVE_GETNAMEINFO -DHAVE_GETSOCKOPT -DHAVE_INET_NTOP -DHAVE_INET_PTON"
	LDFLAGS="$LDFLAGS -lwsock32 -lws2_32 -liphlpapi -no-undefined -mno-cygwin -fno-common -fno-strict-aliasing -Wl,--exclude-libs=libintl.a -Wl,--add-stdcall-alias"
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	cd "nice/.libs/"
	
	$MSLIB /name:libnice-0.dll /out:nice.lib /machine:$MSLibMachine /def:libnice-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
fi

if [ ! -f "$BinDir/xvidcore.dll" ]; then
	echo "$PKG_DIR_XVIDCORE"
	unpack_gzip_and_move "xvidcore.tar.gz" "$PKG_DIR_XVIDCORE"
	mkdir_and_move "$IntDir/xvidcore"

	cd $PKG_DIR/build/generic/
	./configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	make 
	make install

	mv $LibDir/xvidcore.dll $BinDir
	mv $PKG_DIR/build/generic/=build/xvidcore.dll.a $LibDir

	$MSLIB /name:xvidcore.dll /out:xvidcore.lib /machine:$MSLibMachine /def:libxvidcore.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"

	make clean

	
fi

if [ ! -f "$BinDir/libwavpack-1.dll" ]; then 
	unpack_bzip2_and_move "wavpack.tar.bz2" "$PKG_DIR_WAVPACK"
	mkdir_and_move "$IntDir/wavpack"
	
	cp -f "$LIBRARIES_PATCH_DIR/wavpack/Makefile.in" "$PKG_DIR"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	
	make 
	make install

	cd src/.libs
	
	$MSLIB /name:libwavpack-1.dll /out:wavpack.lib /machine:$MSLibMachine /def:libwavpack-1.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"	

	
fi

#a52dec
if [ ! -f "$BinDir/liba52-0.dll" ]; then 
	unpack_gzip_and_move "a52.tar.gz" "$PKG_DIR_A52DEC"
	
	./bootstrap
	
	mkdir_and_move "$IntDir/a52dec"
	 
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	pexports "$BinDir/liba52-0.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY liba52-0.dll/d' in.def > in-mod.def
	$MSLIB /name:liba52-0.dll /out:a52.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"

fi

#mpeg2
if [ ! -f "$BinDir/libmpeg2-0.dll" ]; then 
	unpack_gzip_and_move "libmpeg2.tar.gz" "$PKG_DIR_LIBMPEG2"
	mkdir_and_move "$IntDir/libmpeg2"
	
	$PKG_DIR/configure --disable-sdl --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	pexports "$BinDir/libmpeg2-0.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY libmpeg2-0.dll/d' in.def > in-mod.def
	$MSLIB /name:libmpeg2-0.dll /out:mpeg2.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"

	reset_flags
fi

#libdca
if [ ! -f "$BinDir/libdca-0.dll" ]; then 
	unpack_bzip2_and_move "libdca.tar.bz2" "$PKG_DIR_LIBDCA"
	mkdir_and_move "$IntDir/libdca"
	
	$PKG_DIR/configure --enable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	pexports "$BinDir/libdca-0.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY libdca-0.dll/d' in.def > in-mod.def
	$MSLIB /name:libdca-0.dll /out:dca.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
fi

#faac
if [ ! -f "$BinDir/libfaac-0.dll" ]; then 

	unpack_bzip2_and_move "faac.tar.bz2" "$PKG_DIR_FAAC"
	
	mkdir_and_move "$IntDir/faac"
	 
	$PKG_DIR/configure --without-mp4v2 --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir LDFLAGS="$LDFLAGS -no-undefined" 
	make && make install
		
	cd $PKG_DIR/libfaac
	pexports "$BinDir/libfaac-0.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY libfaac-0.dll/d' in.def > in-mod.def
	$MSLIB /name:libfaac-0.dll /out:faac.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"

	reset_flags	

fi

#faad
if [ ! -f "$BinDir/libfaad-2.dll" ]; then 

	unpack_bzip2_and_move "faad2.tar.bz2" "$PKG_DIR_FAAD2"
	
	mkdir_and_move "$IntDir/faad2"
	 
	cp "$LIBRARIES_PATCH_DIR/faad2/Makefile.in" .
	
	$PKG_DIR/configure --without-mp4v2 --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir LDFLAGS="$LDFLAGS -no-undefined" 
	make && make install
	
	cd $PKG_DIR/libfaad
	pexports "$BinDir/libfaad-2.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY libfaad-2.dll/d' in.def > in-mod.def
	$MSLIB /name:libfaad-2.dll /out:faad2.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"

	reset_flags	

fi

#libdl
#if [ ! -f "$BinDir/libdl.dll" ]; then 
#	unpack_bzip2_and_move "dlfcn.tar.bz2" "$PKG_DIR_DLFCN"
#	mkdir_and_move "$IntDir/libdl" 
#	 
#	cd "$PKG_DIR"
#	./configure --disable-static --enable-shared --prefix=$InstallDir --libdir=$LibDir --incdir=$IncludeDir
#
#	make && make install
#	mv "$LibDir/libdl.lib" "$LibDir/dl.lib"
#fi

#dvdread
if [ ! -f "$BinDir/libdvdread-4.dll" ]; then 

	unpack_bzip2_and_move "libdvdread.tar.bz2" "$PKG_DIR_LIBDVDREAD"
		
	mkdir_and_move "$IntDir/libdvdread"
	
	 
	sh $PKG_DIR/autogen.sh --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir LDFLAGS="$LDFLAGS"
	make && make install

	cp -f "$LIBRARIES_PATCH_DIR/dvdread/dvd_reader.h" "$IncludeDir/dvdread"/ 

	cd src/.libs
	$MSLIB /name:libdvdread-4.dll /out:dvdread.lib /machine:$MSLibMachine /def:libdvdread-4.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	reset_flags

fi

#dvdnav
if [ ! -f "$BinDir/libdvdnav-4.dll" ]; then 

	unpack_bzip2_and_move "libdvdnav.tar.bz2" "$PKG_DIR_LIBDVDNAV"
		
	mkdir_and_move "$IntDir/libdvdnav"
	
	 
	sh $PKG_DIR/autogen.sh --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir LDFLAGS="$LDFLAGS -ldvdread"

	make && make install

	cd src/.libs
	
	$MSLIB /name:libdvdnav-4.dll /out:dvdnav.lib /machine:$MSLibMachine /def:libdvdnav-4.dll.def
	$MSLIB /name:libdvdnavmini-4.dll /out:dvdnavmini.lib /machine:$MSLibMachine /def:libdvdnavmini-4.dll.def

	move_files_to_dir "*.exp *.lib" "$LibDir/"
	reset_flags

fi

#dvdcss
if [ ! -f "$BinDir/libdvdcss-2.dll" ]; then 

	unpack_bzip2_and_move "libdvdcss.tar.bz2" "$PKG_DIR_LIBDVDCSS"
		
	mkdir_and_move "$IntDir/libdvdcss"
	
	 
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir 

	make && make install

	cd src/.libs
	pexports "$BinDir/libdvdcss-2.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY libdvdcss-2.dll/d' in.def > in-mod.def
	$MSLIB /name:libdvdcss-2.dll /out:dvdcss.lib /machine:$MSLibMachine /def:in-mod.def


	move_files_to_dir "*.exp *.lib" "$LibDir/"
	reset_flags
fi

#ffmpeg GPL version
if [ ! -f "$BinDir/avcodec-gpl-52.dll" ]; then 
	#We want to use the GStreamer-supplied version of ffmpeg in case there are differences
	PKG_DIR="$MAIN_DIR/GStreamer/Source/gst-ffmpeg/gst-libs/ext/ffmpeg"
	
	#unpack_bzip2_and_move "ffmpeg.tar.bz2" "$PKG_DIR_FFMPEG"
	mkdir_and_move "$IntDir/ffmpeg-gpl"
	
	$PKG_DIR/configure --extra-ldflags="$LibFlags -Wl,--exclude-libs=libintl.a -Wl,--add-stdcall-alias" --extra-cflags="$IncludeFlags -mno-cygwin -mms-bitfields -fno-common -fno-strict-aliasing -D_WIN32_WINNT=0x0501 -DUSE_GETADDRINFO -DHAVE_GETNAMEINFO -DHAVE_GETSOCKOPT -DHAVE_INET_NTOP -DHAVE_INET_PTON" --enable-gpl --enable-swscale --enable-libfaad --disable-encoder=roq_dpcm --enable-avfilter-lavf --enable-avfilter --disable-vhook --enable-avisynth --target-os=mingw32 --arch=i686 --cpu=i686 --enable-memalign-hack --extra-cflags=-fno-common --enable-zlib --enable-bzlib --enable-w32threads --enable-libmp3lame --enable-libvorbis --enable-libxvid --enable-libopenjpeg --enable-libtheora --enable-libspeex --enable-libschroedinger --enable-ffmpeg --disable-ffplay --disable-ffserver --disable-debug --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$LibDir --shlibdir=$BinDir --incdir=$IncludeDir
	
	#Add a "gpl-" suffix to the shared libraries
	cat "config.mak" | sed "s/^BUILDSUF=/BUILDSUF=-gpl/g" > config.mak.tmp
	mv -f "config.mak.tmp" "config.mak"
	
	make 
	
	reset_flags

	#Create .dll.a versions of the libs
	cd "$IntDir/ffmpeg-gpl"
	dlltool -U --dllname avutil-gpl-49.dll -d "libavutil/avutil-gpl-49.def" -l libavutil-gpl.dll.a
	dlltool -U --dllname avcodec-gpl-52.dll -d "libavcodec/avcodec-gpl-52.def" -l libavcodec-gpl.dll.a
	dlltool -U --dllname avdevice-gpl-52.dll -d "libavdevice/avdevice-gpl-52.def" -l libavdevice-gpl.dll.a
	dlltool -U --dllname avfilter-gpl-0.dll -d "libavfilter/avfilter-gpl-0.def" -l libavfilter-gpl.dll.a
	dlltool -U --dllname avformat-gpl-52.dll -d "libavformat/avformat-gpl-52.def" -l libavformat-gpl.dll.a
	dlltool -U --dllname swscale-gpl-0.dll -d "libswscale/swscale-gpl-0.def" -l libswscale-gpl.dll.a
	
	move_files_to_dir "*.dll.a" "$LibDir/"
	
	cp -p "ffmpeg.exe" "$BinDir/ffmpeg-gpl.exe"
	
	cp -p "libavutil/avutil-gpl-49.dll" "."
	cp -p "libavutil/avutil-gpl-49.dll" "$BinDir/"
	cp -p "libavutil/avutil-gpl-49.lib" "$LibDir/avutil-gpl.lib"
	
	cp -p "libavcodec/avcodec-gpl-52.dll" "."
	cp -p "libavcodec/avcodec-gpl-52.dll" "$BinDir/"
	cp -p "libavcodec/avcodec-gpl-52.lib" "$LibDir/avcodec-gpl.lib"
	
	cp -p "libavdevice/avdevice-gpl-52.dll" "."
	cp -p "libavdevice/avdevice-gpl-52.dll" "$BinDir/"
	cp -p "libavdevice/avdevice-gpl-52.lib" "$LibDir/avdevice-gpl.lib"
	
	cp -p "libavfilter/avfilter-gpl-0.dll" "."
	cp -p "libavfilter/avfilter-gpl-0.dll" "$BinDir/"
	cp -p "libavfilter/avfilter-gpl-0.lib" "$LibDir/avfilter-gpl.lib"
	
	cp -p "libavformat/avformat-gpl-52.dll" "."
	cp -p "libavformat/avformat-gpl-52.dll" "$BinDir/"
	cp -p "libavformat/avformat-gpl-52.lib" "$LibDir/avformat-gpl.lib"
	
	cp -p "libswscale/swscale-gpl-0.dll" "."
	cp -p "libswscale/swscale-gpl-0.dll" "$BinDir/"
	cp -p "libswscale/swscale-gpl-0.lib" "$LibDir/swscale-gpl.lib"
	
	mkdir -p "$IncludeDir/libswscale"
	cp -p "$PKG_DIR/libswscale/swscale.h" "$IncludeDir/libswscale"
	
	#Copy some other dlls for testing
	cp -p "$BinDir/liboil-0.3-0.dll" "."
	cp -p "$BinDir/libfaad-2.dll" "."
	cp -p "$BinDir/libmp3lame-0.dll" "."
	cp -p "$BinDir/libopenjpeg-2.dll" "."
	cp -p "$BinDir/libschroedinger-1.0-0.dll" "."
	cp -p "$BinDir/libspeex-1.dll" "."
	cp -p "$BinDir/libogg-0.dll" "."
	cp -p "$BinDir/libtheora-0.dll" "."
	cp -p "$BinDir/libvorbis-0.dll" "."
	cp -p "$BinDir/libvorbisenc-2.dll" "."
	cp -p "$BinDir/zlib1.dll" "."
	cp -p "$BinDir/xvidcore.dll" "."
	
	reset_flags
fi
#
#PKG_DIR="$MAIN_DIR/GStreamer/Source/gst-ffmpeg/gst-libs/ext/ffmpeg"
#	
##unpack_bzip2_and_move "ffmpeg.tar.bz2" "$PKG_DIR_FFMPEG"
#mkdir_and_move "$IntDir/ffmpeg-gpl"
#
##removed --enable-win32-threads
##added --enable-pthreads --disable-devices --disable-libamr-nb --disable-libamr-wb
##added -march=nocona -mmmx -msse -msse2 -msse3 -mfpmath=sse -falign-functions=64 -fforce-addr
##made static, no shared
#$PKG_DIR/configure --extra-ldflags="$LibFlags -Wl,--exclude-libs=libintl.a -Wl,--add-stdcall-alias" --extra-cflags="$IncludeFlags -march=nocona -mmmx -msse -msse2 -msse3 -mfpmath=sse -falign-functions=64 -fforce-addr -mno-cygwin -mms-bitfields -fno-common -fno-strict-aliasing -DHAVE_CONFIG_H -DHAVE_SIGNAL_H -DPTW32_LEVEL=2 -D_WIN32_WINNT=0x0501 -DUSE_GETADDRINFO -DHAVE_GETNAMEINFO -DHAVE_GETSOCKOPT -DHAVE_INET_NTOP -DHAVE_INET_PTON" --enable-gpl --enable-swscale --enable-libx264 --enable-libfaad --disable-devices --disable-libamr-nb --disable-libamr-wb --disable-encoder=roq_dpcm --enable-avfilter-lavf --enable-avfilter --disable-vhook --enable-avisynth --target-os=mingw32 --arch=i686 --cpu=i686 --enable-memalign-hack --enable-zlib --enable-bzlib --enable-libmp3lame --enable-libvorbis --enable-libxvid --enable-libopenjpeg --enable-libtheora --enable-libspeex --enable-libschroedinger --enable-pthreads --enable-ffmpeg --disable-ffplay --disable-ffserver --disable-debug --enable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$LibDir --shlibdir=$BinDir --incdir=$IncludeDir
#
##Add a "gpl-" suffix to the shared libraries
#cat "config.mak" | sed "s/^BUILDSUF=/BUILDSUF=-gpl/g" > config.mak.tmp
#mv -f "config.mak.tmp" "config.mak"
#
#make 
#
##exit 0
#
#cp -p "libavutil/libavutil-gpl.a" "$LibDir/libavutil-gpl.a"
#cp -p "libavcodec/libavcodec-gpl.a" "$LibDir/libavcodec-gpl.a"
#cp -p "libavdevice/libavdevice-gpl.a" "$LibDir/libavdevice-gpl.a"
#cp -p "libavfilter/libavfilter-gpl.a" "$LibDir/libavfilter-gpl.a"
#cp -p "libavformat/libavformat-gpl.a" "$LibDir/libavformat-gpl.a"
#cp -p "libswscale/libswscale-gpl.a" "$LibDir/libswscale-gpl.a"
#
#exit 0

reset_flags

#Make sure the shared directory has all our updates
#create_shared

#Cleanup CRT
crt_shutdown

#Call common shutdown routines
common_shutdown

