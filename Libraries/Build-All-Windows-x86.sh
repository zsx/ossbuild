#!/bin/sh

TOP=$(dirname $0)/..

CFLAGS="$CFLAGS -mms-bitfields -pipe -D_WIN32_WINNT=0x0501 -Dsocklen_t=int "
CPPFLAGS="$CPPFLAGS -DMINGW32 -D__MINGW32__"
LDFLAGS="-Wl,--enable-auto-image-base -Wl,--enable-auto-import -Wl,--enable-runtime-pseudo-reloc -Wl,--kill-at "
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

Prefix=${DefaultPrefix}

#Move to intermediate directory
cd "$IntDir"

#Add MS build tools
setup_ms_build_env_path

#Generate libtool .la files
generate_all_wapi_libtool_la_x86

#Create symbolic link for gcc etc.
create_cross_compiler_path_windows


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
	$gcc -Wall -I"$IncludeDir" -c libintl.c
	$ar rc "$LibDir/libintl.a" libintl.o
	cp "$LibDir/libintl.a" "$LibDir/intl.lib"
	$strip --strip-unneeded "$LibDir/intl.lib"
	cp libintl.h "$IncludeDir"
	generate_libtool_la_windows "libintl.la" "" "libintl.a"
fi


#Update flags to make sure we don't try and export intl (gettext) functions more than once
#Setting it to ORIG_LDFLAGS ensures that any calls to reset_flags will include these changes.
ORIG_LDFLAGS="$ORIG_LDFLAGS -Wl,--exclude-libs=libintl.a -Wl,--add-stdcall-alias"
reset_flags


#liboil
if [ ! -f "$BinDir/lib${Prefix}oil-0.3-0.dll" ]; then
	unpack_gzip_and_move "liboil.tar.gz" "$PKG_DIR_LIBOIL"
	mkdir_and_move "$IntDir/liboil"
	
	CFLAGS="$CFLAGS -DHAVE_SYMBOL_UNDERSCORE=1"
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
	
	#Do it this way b/c changing libname_spec causes it to not build correctly
	mv "$BinDir/liboil-0.3-0.dll" "$BinDir/lib${Prefix}oil-0.3-0.dll"
	pexports "$BinDir/lib${Prefix}oil-0.3-0.dll" > "in.def"
	sed -e '/^LIBRARY/d' -e 's/DATA//g' in.def > in-mod.def
	$dlltool --dllname lib${Prefix}oil-0.3-0.dll -d "in-mod.def" -l lib${Prefix}oil-0.3.dll.a
	cp -p "lib${Prefix}oil-0.3.dll.a" "$LibDir/liboil-0.3.dll.a"
	$MSLIB /name:lib${Prefix}oil-0.3-0.dll /out:oil-0.3.lib /machine:$MSLibMachine /def:liboil/.libs/liboil-0.3-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	change_key "$LibDir/" "liboil-0.3.la" "dlname" "\'\.\.\/bin\/lib${Prefix}oil-0\.3-0\.dll\'"
	
	#change_libname_spec
	#make && make install
	#update_library_names_windows "lib${Prefix}oil-0.3.dll.a" "liboil-0.3.la"
	#
	#$MSLIB /name:lib${Prefix}oil-0.3-0.dll /out:oil-0.3.lib /machine:$MSLibMachine /def:liboil/.libs/lib${Prefix}oil-0.3-0.dll.def
	#move_files_to_dir "*.exp *.lib" "$LibDir"
	
	reset_flags
fi

#pthreads
if [ ! -f "$BinDir/lib${Prefix}pthreadGC2.dll" ]; then 
	unpack_gzip_and_move "pthreads-w32.tar.gz" "$PKG_DIR_PTHREADS"
	patch -u -N -i "$LIBRARIES_PATCH_DIR/pthreads-w32/sched.h.patch"
	mkdir_and_move "$IntDir/pthreads"
	
	cd "$PKG_DIR"
	change_package "lib${Prefix}pthreadGC\$(DLL_VER).dll" "." "GNUmakefile" "GC_DLL"
	make GC-inlined
	$MSLIB /name:lib${Prefix}pthreadGC2.dll /out:pthreadGC2.lib /machine:$MSLibMachine /def:pthread.def
	copy_files_to_dir "*.exp *.lib *.a" "$LibDir"
	copy_files_to_dir "*.dll" "$BinDir"
	copy_files_to_dir "pthread.h sched.h" "$IncludeDir"
	make clean
	
	mv "$LibDir/libpthreadGC2.a" "$LibDir/libpthreadGC2.dll.a"
	cp -p "$LibDir/libpthreadGC2.dll.a" "$LibDir/libpthread.dll.a"
	
	generate_libtool_la_windows "libpthreadGC2.la" "lib${Prefix}pthreadGC2.dll" "libpthreadGC2.dll.a"
fi

#win-iconv
if [ ! -f "$LibDir/iconv.lib" ]; then 
	unpack_bzip2_and_move "win-iconv.tar.bz2" "$PKG_DIR_WIN_ICONV"
	mkdir_and_move "$IntDir/win-iconv"
	copy_files_to_dir "$LIBRARIES_DIR/Source/Win-Iconv/*.c $LIBRARIES_DIR/Source/Win-Iconv/*.h" .
	
	gcc -I"$IncludeDir" -O2 -DUSE_LIBICONV_DLL -c win_iconv.c 
	ar crv libiconv.a win_iconv.o 
	#gcc $(CFLAGS) -O2 -shared -o lib${Prefix}iconv.dll
	dlltool --export-all-symbols -D lib${Prefix}iconv.dll -l libiconv.dll.a -z in.def libiconv.a
	ranlib libiconv.dll.a
	gcc -shared -s -mwindows -def in.def -o lib${Prefix}iconv.dll libiconv.a
	cp iconv.h "$IncludeDir"
	
	$MSLIB /name:lib${Prefix}iconv.dll /out:iconv.lib /machine:$MSLibMachine /def:in.def
	move_files_to_dir "*.dll" "$BinDir"
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	copy_files_to_dir "*.dll.a" "$LibDir"
	
	generate_libtool_la_windows "libiconv.la" "lib${Prefix}iconv.dll" "libiconv.dll.a"
fi

#zlib
#Can't use separate build dir
if [ ! -f "$BinDir/lib${Prefix}z.dll" ]; then 
	unpack_zip_and_move_windows "zlib.zip" "zlib" "zlib"
	mkdir_and_move "$IntDir/zlib"
	cd "$PKG_DIR"
	
	#cp contrib/asm686/match.S ./match.S
	#make LOC=-DASMV OBJA=match.o -fwin32/Makefile.gcc
	change_package "lib${Prefix}z.dll" "win32" "Makefile.gcc" "SHAREDLIB"
	make -fwin32/Makefile.gcc lib${Prefix}z.dll
	INCLUDE_PATH=$IncludeDir LIBRARY_PATH=$BinDir make install -fwin32/Makefile.gcc
	
	cp -p lib${Prefix}z.dll "$BinDir"
	make clean -fwin32/Makefile.gcc
	
	mv "$BinDir/libzdll.a" "$LibDir/libz.dll.a"
	rm -f "$BinDir/libz.a"
	
	$MSLIB /name:lib${Prefix}z.dll /out:z.lib /machine:$MSLibMachine /def:win32/zlib.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	generate_libtool_la_windows "libz.la" "lib${Prefix}z.dll" "libz.dll.a"
fi

#bzip2
if [ ! -f "$BinDir/lib${Prefix}bz2.dll" ]; then 
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
	gcc -shared -o lib${Prefix}bz2.dll blocksort.o huffman.o crctable.o randtable.o compress.o decompress.o bzlib.o
	
	
	cp -p libbz2.a "$LibDir/libbz2.a"
	cp -p lib${Prefix}bz2.dll "$BinDir/"
	
	pexports lib${Prefix}bz2.dll > "in.def"
	sed -e 's/DATA//g' in.def > in-mod.def
	dlltool --dllname lib${Prefix}bz2.dll -d in-mod.def -l libbz2.dll.a
	cp -p libbz2.dll.a "$LibDir/"
	
	make
	make install PREFIX=$InstallDir
	remove_files_from_dir "*.exe"
	remove_files_from_dir "*.so.*"
	
	$MSLIB /name:lib${Prefix}bz2.dll /out:bz2.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	copy_files_to_dir "bzlib.h" "$IncludeDir"
	
	generate_libtool_la_windows "libbz2.la" "lib${Prefix}bz2.dll" "libbz2.dll.a"
	
	reset_flags
fi

#glew
if [ ! -f "$BinDir/lib${Prefix}glew32.dll" ]; then
	unpack_gzip_and_move "glew.tar.gz" "$PKG_DIR_GLEW"
	mkdir_and_move "$IntDir/glew"
	
	cd "$PKG_DIR"
	change_package "lib${Prefix}\$(NAME).dll" "config" "Makefile.mingw" "LIB.SONAME"
	change_package "lib${Prefix}\$(NAME).dll" "config" "Makefile.mingw" "LIB.SHARED"
	make
	
	cd "lib"
	strip "libglew32.dll.a"
	
	pexports "lib${Prefix}glew32.dll" > in.def
	sed -e '/LIBRARY glew32/d' -e 's/DATA//g' in.def > in-mod.def
	$MSLIB /name:lib${Prefix}glew32.dll /out:glew32.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	cp -f "lib${Prefix}glew32.dll" "$BinDir"
	cp -f "libglew32.dll.a" "$LibDir"
	
	cd "../include/GL/"
	mkdir -p "$IncludeDir/GL/"
	copy_files_to_dir "glew.h wglew.h" "$IncludeDir/GL/"
	
	generate_libtool_la_windows "libglew32.la" "lib${Prefix}glew32.dll" "libglew32.dll.a"
fi

#expat
if [ ! -f "$BinDir/lib${Prefix}expat-1.dll" ]; then
	unpack_gzip_and_move "expat.tar.gz" "$PKG_DIR_EXPAT"
	mkdir_and_move "$IntDir/expat"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	update_library_names_windows "lib${Prefix}expat.dll.a" "libexpat.la"
	
	copy_files_to_dir "$PKG_DIR/lib/libexpat.def" "$IntDir/expat"
	$MSLIB /name:lib${Prefix}expat-1.dll /out:expat.lib /machine:$MSLibMachine /def:libexpat.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
fi

#libxml2
if [ ! -f "$BinDir/lib${Prefix}xml2-2.dll" ]; then 
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
	
	update_library_names_windows "lib${Prefix}xml2.dll.a" "libxml2.la"
	
	#Preprocess-only the .def.src file
	#The preprocessor generates some odd "# 1" statements so we want to eliminate those
	CFLAGS="$CFLAGS -I$IncludeDir/libxml2 -I$SharedIncludeDir/libxml2"
	gcc $CFLAGS -x c -E -D _REENTRANT $PKG_DIR/win32/libxml2.def.src > tmp1.txt
	sed '/# /d' tmp1.txt > tmp2.txt
	sed '/LIBRARY libxml2/d' tmp2.txt > libxml2.def
	reset_flags
	
	#Use the output .def file to generate an MS-compatible lib file
	$MSLIB /name:lib${Prefix}xml2-2.dll /out:xml2.lib /machine:$MSLibMachine /def:libxml2.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	strip "$LibDir\libxml2.dll.a"
	
	#Unfortunate necessity for linking to the dll later. 
	#Thankfully the linker is compatible w/ the ms .lib format
	cp -p "$LibDir\xml2.lib" "$LibDir\libxml2.dll.a"
fi

#libjpeg
if [ ! -f "$BinDir/lib${Prefix}jpeg-8.dll" ]; then 
	unpack_gzip_and_move "jpegsrc.tar.gz" "$PKG_DIR_LIBJPEG"
	mkdir_and_move "$IntDir/libjpeg"
	
	CFLAGS="$CFLAGS -O2"
	
	#Configure, compile, and install
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make
	make install
	
	pexports "$BinDir/lib${Prefix}jpeg-8.dll" > in.def
	sed -e '/LIBRARY lib${Prefix}jpeg/d' -e 's/DATA//g' in.def > in-mod.def
	$MSLIB /name:lib${Prefix}jpeg-8.dll /out:jpeg.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	update_library_names_windows "lib${Prefix}jpeg.dll.a" "libjpeg.la"
	
	reset_flags
fi

#openjpeg
if [ ! -f "$BinDir/lib${Prefix}openjpeg-2.dll" ]; then 
	unpack_gzip_and_move "openjpeg.tar.gz" "$PKG_DIR_OPENJPEG"
	mkdir_and_move "$IntDir/openjpeg"
	
	cd "$PKG_DIR"
	cp "$LIBRARIES_PATCH_DIR/openjpeg/Makefile" .
	
	change_package "${Prefix}openjpeg" "." "Makefile" "TARGET"
	make install LDFLAGS="-lm" PREFIX=$InstallDir
	make clean
	
	cd "$IntDir/openjpeg"
	pexports "$BinDir/lib${Prefix}openjpeg-2.dll" | sed "s/^_//" > in.def
	$MSLIB /name:lib${Prefix}openjpeg-2.dll /out:openjpeg.lib /machine:$MSLibMachine /def:in.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	update_library_names_windows "lib${Prefix}openjpeg.dll.a"
fi

#libpng
if [ ! -f "$BinDir/lib${Prefix}png14-14.dll" ]; then 
	unpack_gzip_and_move "libpng.tar.gz" "$PKG_DIR_LIBPNG"
	mkdir_and_move "$IntDir/libpng"	
	
	#png functions are not being properly exported
	cd "$PKG_DIR"
	change_key "." "Makefile.am" "libpng14_la_LDFLAGS" "-no-undefined\ -export-symbols-regex\ \'\^\(png\|_png\|png_\)\.\*\'\ \\\\"
	automake
	
	cd "$IntDir/libpng"
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	pexports "$BinDir/lib${Prefix}png14-14.dll" > in.def
	sed -e '/LIBRARY lib${Prefix}png14-14.dll/d' -e '/DATA/d' in.def > in-mod.def
	$MSLIB /name:lib${Prefix}png14-14.dll /out:png14.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	update_library_names_windows "lib${Prefix}png14.dll.a" "libpng14.la"
	cp -f -p "$LibDir/libpng14.la" "$LibDir/libpng.la" 
	cp -f -p "$PkgConfigDir/libpng14.pc" "$PkgConfigDir/libpng12.pc"
fi

#glib
if [ ! -f "$BinDir/lib${Prefix}glib-2.0-0.dll" ]; then 
	unpack_bzip2_and_move "glib.tar.bz2" "$PKG_DIR_GLIB"
	patch -u -N -i "$LIBRARIES_PATCH_DIR/glib/run-markup-tests.sh.patch"
	
	mkdir_and_move "$IntDir/glib"
	
	#Need to get rid of MS build tools b/c the makefile call is incorrectly passing it msys-style paths.
	reset_path
	
	$PKG_DIR/configure --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	#Add in MS build tools again
	setup_ms_build_env_path
	
	cd "$IntDir/glib/gio/.libs"
	$MSLIB /name:lib${Prefix}gio-2.0-0.dll /out:gio-2.0.lib /machine:$MSLibMachine /def:lib${Prefix}gio-2.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	cd "../../glib/.libs"
	$MSLIB /name:lib${Prefix}glib-2.0-0.dll /out:glib-2.0.lib /machine:$MSLibMachine /def:lib${Prefix}glib-2.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	cd "../../gmodule/.libs"
	$MSLIB /name:lib${Prefix}gmodule-2.0-0.dll /out:gmodule-2.0.lib /machine:$MSLibMachine /def:lib${Prefix}gmodule-2.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	cd "../../gobject/.libs"
	$MSLIB /name:lib${Prefix}gobject-2.0-0.dll /out:gobject-2.0.lib /machine:$MSLibMachine /def:lib${Prefix}gobject-2.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	cd "../../gthread/.libs"
	$MSLIB /name:lib${Prefix}gthread-2.0-0.dll /out:gthread-2.0.lib /machine:$MSLibMachine /def:lib${Prefix}gthread-2.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir"
	
	cd "$LibDir"
	remove_files_from_dir "g*-2.0.def"
	
	#This is silly - but glib 2.21.4 (at least) doesn't copy this config file even when it's needed.
	#See bug 592773 for more information: http://bugzilla.gnome.org/show_bug.cgi?id=592773
	cp -f "$PKG_DIR/glibconfig.h.win32" "$IncludeDir/glib-2.0/glibconfig.h"
	
	update_library_names_windows "lib${Prefix}glib-2.0.dll.a" "libglib-2.0.la"
	update_library_names_windows "lib${Prefix}gio-2.0.dll.a" "libgio-2.0.la"
	update_library_names_windows "lib${Prefix}gmodule-2.0.dll.a" "libgmodule-2.0.la"
	update_library_names_windows "lib${Prefix}gobject-2.0.dll.a" "libgobject-2.0.la"
	update_library_names_windows "lib${Prefix}gthread-2.0.dll.a" "libgthread-2.0.la"
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
if [ ! -f "$BinDir/lib${Prefix}gpg-error-0.dll" ]; then 
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
	
	$MSLIB /name:lib${Prefix}gpg-error-0.dll /out:gpg-error.lib /machine:$MSLibMachine /def:src/.libs/lib${Prefix}gpg-error-0.dll.def
	
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	update_library_names_windows "lib${Prefix}gpg-error.dll.a" "libgpg-error.la"
	
	reset_flags
fi

#libgcrypt
if [ ! -f "$BinDir/lib${Prefix}gcrypt-11.dll" ]; then 
	unpack_bzip2_and_move "libgcrypt.tar.bz2" "$PKG_DIR_LIBGCRYPT"
	mkdir_and_move "$IntDir/libgcrypt"
	
	CFLAGS=$ORIG_CFLAGS
	CPPFLAGS=$ORIG_CPPFLAGS
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	
	reset_flags
	
	make && make install
	
	$MSLIB /name:lib${Prefix}gcrypt-11.dll /out:gcrypt.lib /machine:$MSLibMachine /def:src/.libs/lib${Prefix}gcrypt-11.dll.def
	
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	cd "$LibDir"
	rm -f "libgcrypt.def"
	
	update_library_names_windows "lib${Prefix}gcrypt.dll.a" "libgcrypt.la"
fi

#libtasn1
if [ ! -f "$BinDir/lib${Prefix}tasn1-3.dll" ]; then 
	unpack_gzip_and_move "libtasn1.tar.gz" "$PKG_DIR_LIBTASN1"
	mkdir_and_move "$IntDir/libtasn1"
	
	CFLAGS=$ORIG_CFLAGS
	CPPFLAGS=$ORIG_CPPFLAGS
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	
	reset_flags
	
	make && make install

	pexports "$BinDir/lib${Prefix}tasn1-3.dll" > in.def
	sed -e "/LIBRARY lib${Prefix}tasn1-3.dll/d" -e '/DATA/d' in.def > in-mod.def

	$MSLIB /name:lib${Prefix}tasn1-3.dll /out:tasn1.lib /machine:$MSLibMachine /def:in-mod.def
	
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	cd "$LibDir"
	rm -f "libtasn1.def"
	
	update_library_names_windows "lib${Prefix}tasn1.dll.a" "libtasn1.la"
fi

#gnutls
if [ ! -f "$BinDir/lib${Prefix}gnutls-26.dll" ]; then 
	unpack_bzip2_and_move "gnutls.tar.bz2" "$PKG_DIR_GNUTLS"
	mkdir_and_move "$IntDir/gnutls"
	
	CFLAGS="-I$PKG_DIR/lib/includes -I$IntDir/gnutls/lib/includes"
	CPPFLAGS="-I$PKG_DIR/lib/includes -I$IntDir/gnutls/lib/includes"
	#LDFLAGS="-Wl,-static-libstdc++"
	
	$PKG_DIR/configure --disable-cxx --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	change_libname_spec "${Prefix}" "${DefaultSuffix}" "lib"
	change_libname_spec "${Prefix}" "${DefaultSuffix}" "libextra"
	make && make install
	
	$MSLIB /name:lib${Prefix}gnutls-26.dll /out:gnutls.lib /machine:$MSLibMachine /def:lib/libgnutls-26.def
	$MSLIB /name:lib${Prefix}gnutls-extra-26.dll /out:gnutls-extra.lib /machine:$MSLibMachine /def:libextra/libgnutls-extra-26.def
	$MSLIB /name:lib${Prefix}gnutls-openssl-26.dll /out:gnutls-openssl.lib /machine:$MSLibMachine /def:libextra/libgnutls-openssl-26.def
	
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	cd "$BinDir" && remove_files_from_dir "libgnutls-*.def"
	
	reset_flags
	
	update_library_names_windows "lib${Prefix}gnutls.dll.a" "libgnutls.la"
	update_library_names_windows "lib${Prefix}gnutls-extra.dll.a" "libgnutls-extra.la"
	update_library_names_windows "lib${Prefix}gnutls-openssl.dll.a" "libgnutls-openssl.la"
fi

#curl
#This is used for testing purposes only
if [ ! -f "$BinDir/lib${Prefix}curl-4.dll" ]; then 
	unpack_bzip2_and_move "curl.tar.bz2" "$PKG_DIR_CURL"
	mkdir_and_move "$IntDir/curl"
	
	$PKG_DIR/configure --with-gnutls --enable-optimize --disable-curldebug --disable-debug --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	reset_flags
	
	update_library_names_windows "lib${Prefix}curl.dll.a" "libcurl.la"
fi

#soup
if [ ! -f "$BinDir/lib${Prefix}soup-2.4-1.dll" ]; then 
	unpack_bzip2_and_move "libsoup.tar.bz2" "$PKG_DIR_LIBSOUP"
	mkdir_and_move "$IntDir/libsoup"
	
	#TODO: Check if this is no longer the case. Bug reported: https://bugzilla.gnome.org/show_bug.cgi?id=606455
	#libsoup isn't outputting the correct exported symbols, so update Makefile.am so libtool will pick it up
	#What we want, essentially, is this: 
	#	libsoup_2_4_la_LDFLAGS = \
	#		-export-symbols-regex '^(soup|_soup|soup_|_SOUP_METHOD_|SOUP_METHOD_).*' \
	#		-version-info $(SOUP_CURRENT):$(SOUP_REVISION):$(SOUP_AGE) -no-undefined
	cd "$PKG_DIR/"
	change_key "libsoup" "Makefile.am" "libsoup_2_4_la_LDFLAGS" "-export-symbols-regex\ \'\^\(soup\|_soup\|soup_\|_SOUP_METHOD\|SOUP_METHOD_\)\.\*\'\ \\\\"
	automake

	#Proceed normally
	
	cd "$IntDir/libsoup"
	$PKG_DIR/configure --disable-silent-rules --disable-glibtest --enable-ssl --enable-debug=no --disable-more-warnings --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install

	cd "libsoup/.libs"
	pexports "$BinDir/lib${Prefix}soup-2.4-1.dll" > in.def
	sed -e '/LIBRARY libsoup/d' -e 's/DATA//g' in.def > in-mod.def
	
	$MSLIB /name:lib${Prefix}soup-2.4-1.dll /out:soup-2.4.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
	
	update_library_names_windows "lib${Prefix}soup-2.4.dll.a" "libsoup-2.4.la"
fi

#neon
if [ ! -f "$BinDir/lib${Prefix}neon-27.dll" ]; then 
	unpack_gzip_and_move "neon.tar.gz" "$PKG_DIR_NEON"
	mkdir_and_move "$IntDir/neon"
	
	$PKG_DIR/configure --with-ssl=gnutls --disable-debug --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	cd "src/.libs"
	pexports "$BinDir/lib${Prefix}neon-27.dll" | sed "s/^_//" > in.def
	sed -e '/LIBRARY lib${Prefix}neon-27.dll/d' in.def > in-mod.def
	$MSLIB /name:lib${Prefix}neon-27.dll /out:neon.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
	
	update_library_names_windows "lib${Prefix}neon.dll.a" "libneon.la"
fi

#freetype
if [ ! -f "$BinDir/lib${Prefix}freetype-6.dll" ]; then 
	unpack_bzip2_and_move "freetype.tar.bz2" "$PKG_DIR_FREETYPE"
	mkdir_and_move "$IntDir/freetype"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	cp -p .libs/lib${Prefix}freetype.dll.a "$LibDir/freetype.lib"
	
	update_library_names_windows "lib${Prefix}freetype.dll.a" "libfreetype.la"
fi

#fontconfig
if [ ! -f "$BinDir/lib${Prefix}fontconfig-1.dll" ]; then 
	unpack_gzip_and_move "fontconfig.tar.gz" "$PKG_DIR_FONTCONFIG"
	mkdir_and_move "$IntDir/fontconfig"
	
	$PKG_DIR/configure --disable-debug --disable-static --disable-docs --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install RUN_FC_CACHE_TEST=false

	cp -p "fontconfig.pc" "$LibDir/pkgconfig/"
	
	cd "src/.libs"
	
	sed -e '/LIBRARY/d' lib${Prefix}fontconfig-1.dll.def > in-mod.def
	dlltool --dllname lib${Prefix}fontconfig-1.dll -d "in-mod.def" -l lib${Prefix}fontconfig.dll.a
	cp -p "lib${Prefix}fontconfig.dll.a" "$LibDir/"
	$MSLIB /name:lib${Prefix}fontconfig-1.dll /out:fontconfig.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
	
	update_library_names_windows "lib${Prefix}fontconfig.dll.a" "libfontconfig.la"
	
	echo "<OSSBuild>: Please ignore install errors"
fi

#pixman
if [ ! -f "$BinDir/lib${Prefix}pixman-1-0.dll" ]; then 
	unpack_gzip_and_move "pixman.tar.gz" "$PKG_DIR_PIXMAN"
	mkdir_and_move "$IntDir/pixman"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	cd "$LIBRARIES_PATCH_DIR/pixman/"
	$MSLIB /name:lib${Prefix}pixman-1-0.dll /out:pixman.lib /machine:$MSLibMachine /def:pixman.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	update_library_names_windows "lib${Prefix}pixman-1.dll.a" "libpixman-1.la"
fi

#cairo
if [ ! -f "$BinDir/lib${Prefix}cairo-2.dll" ]; then 
	unpack_gzip_and_move "cairo.tar.gz" "$PKG_DIR_CAIRO"
	mkdir_and_move "$IntDir/cairo"
	
	CFLAGS="$CFLAGS -D CAIRO_HAS_WIN32_SURFACE -D CAIRO_HAS_WIN32_FONT -Wl,-lpthreadGC2"
	$PKG_DIR/configure --enable-xlib=auto --enable-xlib-xrender=auto --enable-png=yes --enable-ft=yes --enable-pdf=yes --enable-svg=yes --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	cd src/.libs/
	
	sed -e '/LIBRARY/d' lib${Prefix}cairo-2.dll.def > in-mod.def
	dlltool --dllname lib${Prefix}cairo-2.dll -d "in-mod.def" -l lib${Prefix}cairo.dll.a
	cp -p "lib${Prefix}cairo.dll.a" "$LibDir/"
	$MSLIB /name:lib${Prefix}cairo-2.dll /out:cairo.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
	
	update_library_names_windows "lib${Prefix}cairo.dll.a" "libcairo.la"
fi

#pango
if [ ! -f "$BinDir/lib${Prefix}pango-1.0-0.dll" ]; then 
	unpack_bzip2_and_move "pango.tar.bz2" "$PKG_DIR_PANGO"
	mkdir_and_move "$IntDir/pango"
	
	#Need to get rid of MS build tools b/c the makefile call is incorrectly passing it msys-style paths.
	reset_path
	
	$PKG_DIR/configure --with-included-modules --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	#Add in MS build tools again
	setup_ms_build_env_path
	
	cd "$IntDir/pango/pango/.libs/"
	$MSLIB /name:lib${Prefix}pango-1.0-0.dll /out:pango-1.0.lib /machine:$MSLibMachine /def:lib${Prefix}pango-1.0-0.dll.def
	$MSLIB /name:lib${Prefix}pangoft2-1.0-0.dll /out:pangoft2-1.0.lib /machine:$MSLibMachine /def:lib${Prefix}pangoft2-1.0-0.dll.def
	$MSLIB /name:lib${Prefix}pangowin32-1.0-0.dll /out:pangowin32-1.0.lib /machine:$MSLibMachine /def:lib${Prefix}pangowin32-1.0-0.dll.def
	$MSLIB /name:lib${Prefix}pangocairo-1.0-0.dll /out:pangocairo-1.0.lib /machine:$MSLibMachine /def:lib${Prefix}pangocairo-1.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	update_library_names_windows "lib${Prefix}pango-1.0.dll.a" "libpango-1.0.la"
	update_library_names_windows "lib${Prefix}pangoft2-1.0.dll.a" "libpangoft2-1.0.la"
	update_library_names_windows "lib${Prefix}pangowin32-1.0.dll.a" "libpangowin32-1.0.la"
	update_library_names_windows "lib${Prefix}pangocairo-1.0.dll.a" "libpangocairo-1.0.la"
	
	cd "$LibDir" && remove_files_from_dir "pango*.def"
fi

#sdl
if [ ! -f "$BinDir/lib${Prefix}SDL.dll" ]; then 
	unpack_gzip_and_move "sdl.tar.gz" "$PKG_DIR_SDL"
	mkdir_and_move "$IntDir/sdl"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir 
	#Required b/c SDL removes the "lib" portion from the name. This reinserts it.
	change_libname_spec "lib${Prefix}"
	make && make install
	
	cp $PKG_DIR/include/SDL_config.h.default $IncludeDir/SDL/SDL_config.h
	cp $PKG_DIR/include/SDL_config_win32.h $IncludeDir/SDL
	
	cd build/.libs
	
	pexports "$BinDir/lib${Prefix}SDL.dll" | sed "s/^_//" > in.def
	sed -e '/LIBRARY lib${Prefix}SDL.dll/d' in.def > in-mod.def
	$MSLIB /name:lib${Prefix}SDL.dll /out:sdl.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	reset_flags
	
	cd "$LibDir/"
	mv "liblib${Prefix}SDL.dll.a" "lib${Prefix}SDL.dll.a"
	update_library_names_windows "lib${Prefix}SDL.dll.a" "libSDL.la"
	change_key "." "libSDL.la" "library_names" "\'libSDL.dll.a\'"
fi

#libogg
if [ ! -f "$BinDir/lib${Prefix}ogg-0.dll" ]; then 
	unpack_gzip_and_move "libogg.tar.gz" "$PKG_DIR_LIBOGG"
	mkdir_and_move "$IntDir/libogg"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	copy_files_to_dir "$PKG_DIR/win32/ogg.def" .
	$MSLIB /name:lib${Prefix}ogg-0.dll /out:ogg.lib /machine:$MSLibMachine /def:ogg.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	update_library_names_windows "lib${Prefix}ogg.dll.a" "libogg.la"
fi

#libvorbis
if [ ! -f "$BinDir/lib${Prefix}vorbis-0.dll" ]; then 
	unpack_bzip2_and_move "libvorbis.tar.bz2" "$PKG_DIR_LIBVORBIS"
	mkdir_and_move "$IntDir/libvorbis"
	
	LDFLAGS="$LDFLAGS -logg"
	$PKG_DIR/configure --with-ogg-libraries=$LibDir --with-ogg-includes=$IncludeDir --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	update_library_names_windows "lib${Prefix}vorbis.dll.a" "libvorbis.la"
	
	#Yeah, we're calling this twice b/c all the object files are compiled for all the libs, 
	#but they're not all being linked and installed b/c it can't find -lvorbis. Calling 
	#make/make install twice seems to solve it. But it's a hack.
	make && make install
	
	copy_files_to_dir "$PKG_DIR/win32/*.def" .
	sed '/vorbis_encode_*/d' vorbis.def > vorbis-mod.def
	$MSLIB /name:lib${Prefix}vorbis-0.dll /out:vorbis.lib /machine:$MSLibMachine /def:vorbis-mod.def
	$MSLIB /name:lib${Prefix}vorbisenc-2.dll /out:vorbisenc.lib /machine:$MSLibMachine /def:vorbisenc.def
	$MSLIB /name:lib${Prefix}vorbisfile-3.dll /out:vorbisfile.lib /machine:$MSLibMachine /def:vorbisfile.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
	
	update_library_names_windows "lib${Prefix}vorbis.dll.a" "libvorbis.la"
	update_library_names_windows "lib${Prefix}vorbisenc.dll.a" "libvorbisenc.la"
	update_library_names_windows "lib${Prefix}vorbisfile.dll.a" "libvorbisfile.la"
fi

#libcelt
#TODO: Fix this!
if [ ! -f "$BinDir/lib${Prefix}celt-0.dll" ]; then 
	unpack_gzip_and_move "libcelt.tar.gz" "$PKG_DIR_LIBCELT"
	mkdir_and_move "$IntDir/libcelt"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	
	cd "libcelt"
	
	echo "int main () { return 0; }" > main.c
	gcc -o main.o -c main.c

	#This will fail to produce the dll b/c of some odd dependency on a main() function (for some reason it's linking libmingw32.a)
	#We could try linking even if there are missing dependencies...
	make libcelt.la
	gcc --link -shared -o .libs/lib${Prefix}celt-0.dll -Wl,--output-def=libcelt.def -Wl,--out-implib=.libs/lib${Prefix}celt.dll.a -std=gnu99 $LDFLAGS \
		.libs/bands.o \
		.libs/celt.o \
		.libs/cwrs.o \
		.libs/entcode.o \
		.libs/entdec.o \
		.libs/entenc.o \
		.libs/header.o \
		.libs/kfft_single.o \
		.libs/kiss_fft.o \
		.libs/kiss_fftr.o \
		.libs/laplace.o \
		.libs/mdct.o \
		.libs/modes.o \
		.libs/pitch.o \
		.libs/psy.o \
		.libs/quant_bands.o \
		.libs/rangedec.o \
		.libs/rangeenc.o \
		.libs/rate.o \
		.libs/vq.o \
		main.o
	rm libcelt.la
	rm .libs/libcelt.la
	rm .libs/libcelt.lai
	rm .libs/lib${Prefix}celt.a
	echo -en "# Generated by ossbuild - GNU libtool 1.5.22 (1.1220.2.365 2005/12/18 22:14:06)\n" > libcelt.la
	echo -en "dlname='lib${Prefix}celt-0.dll'\n" >> libcelt.la
	echo -en "library_names='lib${Prefix}celt.dll.a'\n" >> libcelt.la
	echo -en "old_library=''\n" >> libcelt.la
	echo -en "inherited_linker_flags=''\n" >> libcelt.la
	echo -en "dependency_libs=''\n" >> libcelt.la
	echo -en "weak_library_names=''\n" >> libcelt.la
	echo -en "current=0\n" >> libcelt.la
	echo -en "age=0\n" >> libcelt.la
	echo -en "revision=0\n" >> libcelt.la
	echo -en "installed=no\n" >> libcelt.la
	echo -en "shouldnotlink=no\n" >> libcelt.la
	echo -en "dlopen=''\n" >> libcelt.la
	echo -en "dlpreopen=''\n" >> libcelt.la
	echo -en "libdir='$LibDir'\n" >> libcelt.la
	cp -p libcelt.la .libs/
	cd .libs/
	echo -en "# Generated by ossbuild - GNU libtool 1.5.22 (1.1220.2.365 2005/12/18 22:14:06)\n" > libcelt.lai
	echo -en "dlname='../bin/lib${Prefix}celt-0.dll'\n" >> libcelt.lai
	echo -en "library_names='lib${Prefix}celt.dll.a'\n" >> libcelt.lai
	echo -en "old_library=''\n" >> libcelt.lai
	echo -en "inherited_linker_flags=''\n" >> libcelt.lai
	echo -en "dependency_libs='-L$LibDir -lm'\n" >> libcelt.lai
	echo -en "weak_library_names=''\n" >> libcelt.lai
	echo -en "current=0\n" >> libcelt.lai
	echo -en "age=0\n" >> libcelt.lai
	echo -en "revision=0\n" >> libcelt.lai
	echo -en "installed=yes\n" >> libcelt.lai
	echo -en "shouldnotlink=no\n" >> libcelt.lai
	echo -en "dlopen=''\n" >> libcelt.lai
	echo -en "dlpreopen=''\n" >> libcelt.lai
	echo -en "libdir='$LibDir'\n" >> libcelt.lai
	cd ..
	make
	make install
	
	copy_files_to_dir "$LIBRARIES_PATCH_DIR/celt/*.def" .
	$MSLIB /name:lib${Prefix}celt-0.dll /out:celt.lib /machine:$MSLibMachine /def:libcelt.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
	
	update_library_names_windows "lib${Prefix}celt.dll.a" "libcelt.la"
fi

#libtheora
if [ ! -f "$BinDir/lib${Prefix}theora-0.dll" ]; then 
	unpack_bzip2_and_move "libtheora.tar.bz2" "$PKG_DIR_LIBTHEORA"
	mkdir_and_move "$IntDir/libtheora"
	
	$PKG_DIR/configure --with-vorbis=$BinDir --with-vorbis-libraries=$LibDir --with-vorbis-includes=$IncludeDir --with-ogg=$BinDir --with-ogg-libraries=$LibDir --with-ogg-includes=$IncludeDir --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	copy_files_to_dir "$LIBRARIES_PATCH_DIR/theora/win32/*def" .
	copy_files_to_dir "lib/.libs/*.def" .
	flip -d lib${Prefix}theora.def
	sed -e '/LIBRARY	lib${Prefix}theora/d' lib${Prefix}theora.def > libtheora-mod.def
	$MSLIB /name:lib${Prefix}theora-0.dll /out:theora.lib /machine:$MSLibMachine /def:libtheora-mod.def
	$MSLIB /name:lib${Prefix}theoradec-1.dll /out:theoradec.lib /machine:$MSLibMachine /def:lib${Prefix}theoradec-1.dll.def
	$MSLIB /name:lib${Prefix}theoraenc-1.dll /out:theoraenc.lib /machine:$MSLibMachine /def:lib${Prefix}theoraenc-1.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	update_library_names_windows "lib${Prefix}theora.dll.a" "libtheora.la"
	update_library_names_windows "lib${Prefix}theoradec.dll.a" "libtheoradec.la"
	update_library_names_windows "lib${Prefix}theoraenc.dll.a" "libtheoraenc.la"
fi

#libmms
if [ ! -f "$BinDir/lib${Prefix}mms-0.dll" ]; then 
	unpack_bzip2_and_move "libmms.tar.bz2" "$PKG_DIR_LIBMMS"
	mkdir_and_move "$IntDir/libmms"
	
	CFLAGS="$CFLAGS -D LIBMMS_HAVE_64BIT_OFF_T"
	LDFLAGS="$LDFLAGS -lwsock32 -lglib-2.0 -lgobject-2.0"
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	copy_files_to_dir "$LIBRARIES_PATCH_DIR/libmms/*.def" .
	$MSLIB /name:lib${Prefix}mms-0.dll /out:mms.lib /machine:$MSLibMachine /def:libmms.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
	
	update_library_names_windows "lib${Prefix}mms.dll.a" "libmms.la"
fi

#x264
if [ ! -f "$BinDir/lib${Prefix}x264-67.dll" ]; then 
	unpack_bzip2_and_move "x264.tar.bz2" "$PKG_DIR_X264"
	mkdir_and_move "$IntDir/x264"
	
	PATH=$PATH:$TOOLS_DIR
	
	cd "$PKG_DIR/"
	./configure --disable-mp4-output --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_key "." "config.mak" "SONAME" "lib${Prefix}x264-67.dll"
	change_key "." "config.mak" "IMPLIBNAME" "lib${Prefix}x264.dll.a"
	make && make install
	reset_flags
	
	reset_path
	setup_ms_build_env_path

	cd "$IntDir/x264"
	rm -rf "$LibDir/libx264.a"
	pexports "$BinDir/lib${Prefix}x264-67.dll" | sed "s/^_//" > in.def
	sed -e 's/DATA//g' in.def > in-mod.def
	$MSLIB /name:lib${Prefix}x264-67.dll /out:x264.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	update_library_names_windows "lib${Prefix}x264.dll.a" "libx264.la"
fi

#libspeex
if [ ! -f "$BinDir/lib${Prefix}speex-1.dll" ]; then 
	unpack_gzip_and_move "speex.tar.gz" "$PKG_DIR_LIBSPEEX"
	mkdir_and_move "$IntDir/libspeex"
	
	$PKG_DIR/configure --enable-sse --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
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
	
	$MSLIB /name:lib${Prefix}speex-1.dll /out:speex.lib /machine:$MSLibMachine /def:libspeex-mod.def
	$MSLIB /name:lib${Prefix}speexdsp-1.dll /out:speexdsp.lib /machine:$MSLibMachine /def:libspeexdsp-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	cp -p "libspeex/.libs/lib${Prefix}speexdsp.dll.a" "$LibDir"
	
	update_library_names_windows "lib${Prefix}speex.dll.a" "libspeex.la"
	update_library_names_windows "lib${Prefix}speexdsp.dll.a" "libspeexdsp.la"
fi

#libschroedinger (dirac support)
if [ ! -f "$BinDir/lib${Prefix}schroedinger-1.0-0.dll" ]; then 
	unpack_gzip_and_move "schroedinger.tar.gz" "$PKG_DIR_LIBSCHROEDINGER"
	mkdir_and_move "$IntDir/libschroedinger"
	
	#LDFLAGS="-lstdc++_s"
	$PKG_DIR/configure --with-thread=auto --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	cd "schroedinger/.libs"
	$MSLIB /name:lib${Prefix}schroedinger-1.0-0.dll /out:schroedinger-1.0.lib /machine:$MSLibMachine /def:lib${Prefix}schroedinger-1.0-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
	
	update_library_names_windows "lib${Prefix}schroedinger-1.0.dll.a" "libschroedinger-1.0.la"
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
if [ ! -f "$BinDir/lib${Prefix}mp3lame-0.dll" ]; then 
	unpack_gzip_and_move "lame.tar.gz" "$PKG_DIR_MP3LAME"
	mkdir_and_move "$IntDir/mp3lame"
	
	$PKG_DIR/configure --enable-expopt=no --enable-debug=no --disable-brhist -disable-frontend --enable-nasm --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	pexports "$BinDir/lib${Prefix}mp3lame-0.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY libmp3lame-0.dll/d' in.def > in-mod.def
	$MSLIB /name:lib${Prefix}mp3lame-0.dll /out:mp3lame.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	update_library_names_windows "lib${Prefix}mp3lame.dll.a" "libmp3lame.la"
fi

#ffmpeg
if [ ! -f "$BinDir/lib${Prefix}avcodec-52.dll" ]; then 
	unpack_bzip2_and_move "ffmpeg.tar.bz2" "$PKG_DIR_FFMPEG"
	mkdir_and_move "$IntDir/ffmpeg"
	
	#LGPL-compatible version
	#Please see http://www.mail-archive.com/ffmpeg-issues@lscube.org/msg04083.html 
	#for an explanation on why -Dav_cold=' ' works
	#-mincoming-stack-boundary=4 -fno-caller-saves
	#--disable-encoder=dnxhd --disable-decoder=dnxhd --disable-muxer=dnxhd --disable-demuxer=dnxhd --disable-parser=dnxhd
	#These tests fail:
	#dnxhd_1080i
        #dnxhd_720p
        #dnxhd_720p_rd
	#adpcm_ms
	#wma
	CFLAGS=""
	CPPFLAGS=""
	LDFLAGS=""
	
	$PKG_DIR/configure --cc=$gcc --ld=$gcc --extra-ldflags="$LibFlags -Wl,--enable-auto-image-base -Wl,--enable-auto-import -Wl,--enable-runtime-pseudo-reloc -Wl,--kill-at -Wl,--exclude-libs=libintl.a -Wl,--add-stdcall-alias" --extra-cflags="$IncludeFlags -mno-cygwin -mms-bitfields -D_WIN32_WINNT=0x0501 -D__MINGW32_MAJOR_VERSION=3 -D__MINGW32_MINOR_VERSION=15" --enable-runtime-cpudetect --enable-avfilter-lavf --enable-avfilter --enable-avisynth --target-os=mingw32 --arch=i686 --cpu=i686 --enable-memalign-hack --enable-zlib --enable-bzlib --enable-libmp3lame --enable-libvorbis --enable-libopenjpeg --enable-libtheora --enable-libspeex --enable-libschroedinger --enable-ffmpeg --disable-ffplay --disable-ffserver --disable-debug --disable-static --enable-shared --prefix=$InstallDir --bindir=$BinDir --libdir=$LibDir --shlibdir=$BinDir --incdir=$IncludeDir 
	change_key "." "config.mak" "LIBPREF" "lib${Prefix}"
	change_key "." "config.mak" "SLIBPREF" "lib${Prefix}"
	#Adds $(SLIBPREF) to lib names when linking
	change_key "." "common.mak" "FFEXTRALIBS\ \\:" "\$\(addprefix\ -l\$\(SLIBPREF\),\$\(addsuffix\ \$\(BUILDSUF\),\$\(FFLIBS\)\)\)\ \$\(EXTRALIBS\)"
	make && make install
	
	#If it built successfully, then the .lib and .dll files are all in the lib/ folder with 
	#sym links. We want to take out the sym links and keep just the .lib and .dll files we need 
	#for development and execution.
	cd "$BinDir" && move_files_to_dir "lib${Prefix}av*.lib" "$LibDir"
	cd "$BinDir" && move_files_to_dir "lib${Prefix}swscale*.lib" "$LibDir"
	cd "$BinDir" && remove_files_from_dir "lib${Prefix}avcodec-*.*.*.dll lib${Prefix}avcodec.dll lib${Prefix}avdevice-*.*.*.dll lib${Prefix}avdevice.dll lib${Prefix}avfilter-*.*.*.dll lib${Prefix}avfilter.dll lib${Prefix}avformat-*.*.*.dll lib${Prefix}avformat.dll lib${Prefix}avutil-*.*.*.dll lib${Prefix}avutil.dll   lib${Prefix}swscale-*.*.*.dll lib${Prefix}swscale.dll"
	cd "$LibDir" && remove_files_from_dir "lib${Prefix}avcodec-*.lib lib${Prefix}avdevice-*.lib lib${Prefix}avfilter-*.lib lib${Prefix}avformat-*.lib lib${Prefix}avutil-*.lib  lib${Prefix}swscale-*.lib"
	
	reset_flags
	
	cd "$BinDir"
	strip "lib${Prefix}avcodec-52.dll"
	
	cd "$LibDir"
	mv "liblib${Prefix}avutil.dll.a" "libavutil.dll.a"
	mv "liblib${Prefix}avcodec.dll.a" "libavcodec.dll.a"
	mv "liblib${Prefix}avdevice.dll.a" "libavdevice.dll.a"
	mv "liblib${Prefix}avfilter.dll.a" "libavfilter.dll.a"
	mv "liblib${Prefix}avformat.dll.a" "libavformat.dll.a"
	mv "liblib${Prefix}swscale.dll.a" "libswscale.dll.a"
	mv "lib${Prefix}avutil.lib" "avutil.lib"
	mv "lib${Prefix}avcodec.lib" "avcodec.lib"
	mv "lib${Prefix}avdevice.lib" "avdevice.lib"
	mv "lib${Prefix}avfilter.lib" "avfilter.lib"
	mv "lib${Prefix}avformat.lib" "avformat.lib"
	mv "lib${Prefix}swscale.lib" "swscale.lib"
	
	cd "$IntDir/ffmpeg"
	copy_files_to_dir "$BinDir/*.dll" "."
fi



#################
# GPL Libraries #
#################



#libnice
if [ ! -f "$BinDir/lib${Prefix}nice-0.dll" ]; then 
	unpack_gzip_and_move "libnice.tar.gz" "$PKG_DIR_LIBNICE"
	patch -u -N -i "$LIBRARIES_PATCH_DIR/libnice/bind.c-win32.patch"
	patch -u -N -i "$LIBRARIES_PATCH_DIR/libnice/rand.c-win32.patch"
	patch -u -N -i "$LIBRARIES_PATCH_DIR/libnice/address.h-win32.patch"
	patch -u -N -i "$LIBRARIES_PATCH_DIR/libnice/interfaces.c-win32.patch"
	
	mkdir_and_move "$IntDir/libnice"
	
	CFLAGS="-D_SSIZE_T_ -I$PKG_DIR -I$PKG_DIR/stun -D_WIN32_WINNT=0x0501 -DUSE_GETADDRINFO -DHAVE_GETNAMEINFO -DHAVE_GETSOCKOPT -DHAVE_INET_NTOP -DHAVE_INET_PTON"
	LDFLAGS="$LDFLAGS -lwsock32 -lws2_32 -liphlpapi -no-undefined -mno-cygwin -fno-common -fno-strict-aliasing -Wl,--exclude-libs=libintl.a -Wl,--add-stdcall-alias"
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	cd "nice/.libs/"
	
	$MSLIB /name:lib${Prefix}nice-0.dll /out:nice.lib /machine:$MSLibMachine /def:lib${Prefix}nice-0.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags
	
	update_library_names_windows "lib${Prefix}nice.dll.a" "libnice.la"
fi

if [ ! -f "$BinDir/lib${Prefix}xvidcore.dll" ]; then
	echo "$PKG_DIR_XVIDCORE"
	unpack_gzip_and_move "xvidcore.tar.gz" "$PKG_DIR_XVIDCORE"
	mkdir_and_move "$IntDir/xvidcore"

	cd $PKG_DIR/build/generic/
	./configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_key "." "platform.inc" "STATIC_LIB" "lib${Prefix}xvidcore\.\$\(STATIC_EXTENSION\)"
	change_key "." "platform.inc" "SHARED_LIB" "lib${Prefix}xvidcore\.\$\(SHARED_EXTENSION\)"
	change_key "." "platform.inc" "PRE_SHARED_LIB" "lib${Prefix}xvidcore\.\$\(SHARED_EXTENSION\)"
	make && make install

	mv "$LibDir/lib${Prefix}xvidcore.dll" "$BinDir"
	mv "$PKG_DIR/build/generic/=build/lib${Prefix}xvidcore.dll.a" "$LibDir/libxvidcore.dll.a"

	$MSLIB /name:lib${Prefix}xvidcore.dll /out:xvidcore.lib /machine:$MSLibMachine /def:libxvidcore.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	rm -f "$LibDir/lib${Prefix}xvidcore.a"
fi

if [ ! -f "$BinDir/lib${Prefix}wavpack-1.dll" ]; then 
	unpack_bzip2_and_move "wavpack.tar.bz2" "$PKG_DIR_WAVPACK"
	mkdir_and_move "$IntDir/wavpack"
	
	cp -p -f "$LIBRARIES_PATCH_DIR/wavpack/Makefile.in" "$PKG_DIR"
	
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	cd src/.libs	
	$MSLIB /name:lib${Prefix}wavpack-1.dll /out:wavpack.lib /machine:$MSLibMachine /def:lib${Prefix}wavpack-1.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags

	update_library_names_windows "lib${Prefix}wavpack.dll.a" "libwavpack.la"
fi

#a52dec
if [ ! -f "$BinDir/lib${Prefix}a52-0.dll" ]; then 
	unpack_gzip_and_move "a52.tar.gz" "$PKG_DIR_A52DEC"
	
	./bootstrap
	
	mkdir_and_move "$IntDir/a52dec"
	 
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	pexports "$BinDir/lib${Prefix}a52-0.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY lib${Prefix}a52-0.dll/d' in.def > in-mod.def
	$MSLIB /name:lib${Prefix}a52-0.dll /out:a52.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"

	update_library_names_windows "lib${Prefix}a52.dll.a" "liba52.la"
fi

#mpeg2
if [ ! -f "$BinDir/lib${Prefix}mpeg2-0.dll" ]; then 
	unpack_gzip_and_move "libmpeg2.tar.gz" "$PKG_DIR_LIBMPEG2"
	mkdir_and_move "$IntDir/libmpeg2"
	
	$PKG_DIR/configure --disable-sdl --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	pexports "$BinDir/lib${Prefix}mpeg2-0.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY lib${Prefix}mpeg2-0.dll/d' in.def > in-mod.def
	$MSLIB /name:lib${Prefix}mpeg2-0.dll /out:mpeg2.lib /machine:$MSLibMachine /def:in-mod.def
	
	pexports "$BinDir/lib${Prefix}mpeg2convert-0.dll" | sed "s/^_//" > in-convert.def
	sed '/LIBRARY lib${Prefix}mpeg2convert-0.dll/d' in.def > in-convert-mod.def
	$MSLIB /name:lib${Prefix}mpeg2convert-0.dll /out:mpeg2convert.lib /machine:$MSLibMachine /def:in-convert-mod.def
	
	move_files_to_dir "*.exp *.lib" "$LibDir/"

	reset_flags
	
	update_library_names_windows "lib${Prefix}mpeg2.dll.a" "libmpeg2.la"
	update_library_names_windows "lib${Prefix}mpeg2convert.dll.a" "libmpeg2convert.la"
fi

#libdca
if [ ! -f "$BinDir/lib${Prefix}dca-0.dll" ]; then 
	unpack_bzip2_and_move "libdca.tar.bz2" "$PKG_DIR_LIBDCA"
	mkdir_and_move "$IntDir/libdca"
	
	$PKG_DIR/configure --enable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	change_libname_spec
	make && make install
	
	#Install dies b/c it can't find "libdca.a" (it instead finds lib${Prefix}dca.a)
	#So copy it and run install again
	cp -p "$LibDir/lib${Prefix}dca.a" "$LibDir/libdca.a"
	make install
	
	rm -f "$LibDir/libdca.a"
	rm -f "$LibDir/lib${Prefix}dca.a"
	
	pexports "$BinDir/lib${Prefix}dca-0.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY lib${Prefix}dca-0.dll/d' in.def > in-mod.def
	$MSLIB /name:lib${Prefix}dca-0.dll /out:dca.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	update_library_names_windows "lib${Prefix}dca.dll.a" "libdca.la"
fi

#faac
if [ ! -f "$BinDir/lib${Prefix}faac-0.dll" ]; then 
	unpack_bzip2_and_move "faac.tar.bz2" "$PKG_DIR_FAAC"
	mkdir_and_move "$IntDir/faac"
	 
	$PKG_DIR/configure --without-mp4v2 --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir LDFLAGS="$LDFLAGS -no-undefined" 
	change_libname_spec
	make && make install
		
	cd $PKG_DIR/libfaac
	pexports "$BinDir/lib${Prefix}faac-0.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY lib${Prefix}faac-0.dll/d' in.def > in-mod.def
	$MSLIB /name:lib${Prefix}faac-0.dll /out:faac.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"

	reset_flags	

	update_library_names_windows "lib${Prefix}faac.dll.a" "libfaac.la"
fi

#faad
if [ ! -f "$BinDir/lib${Prefix}faad-2.dll" ]; then 
	unpack_bzip2_and_move "faad2.tar.bz2" "$PKG_DIR_FAAD2"
	mkdir_and_move "$IntDir/faad2"
	 
	cp "$LIBRARIES_PATCH_DIR/faad2/Makefile.in" .
	
	$PKG_DIR/configure --without-mp4v2 --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir LDFLAGS="$LDFLAGS -no-undefined" 
	change_libname_spec
	make && make install
	
	cd $PKG_DIR/libfaad
	pexports "$BinDir/lib${Prefix}faad-2.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY lib${Prefix}faad-2.dll/d' in.def > in-mod.def
	$MSLIB /name:lib${Prefix}faad-2.dll /out:faad2.lib /machine:$MSLibMachine /def:in-mod.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"

	reset_flags	

	update_library_names_windows "lib${Prefix}faad.dll.a" "libfaad.la"
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
if [ ! -f "$BinDir/lib${Prefix}dvdread-4.dll" ]; then 
	unpack_bzip2_and_move "libdvdread.tar.bz2" "$PKG_DIR_LIBDVDREAD"
	mkdir_and_move "$IntDir/libdvdread"
	 
	sh $PKG_DIR/autogen.sh --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir LDFLAGS="$LDFLAGS"
	change_libname_spec
	make && make install

	cp -f "$LIBRARIES_PATCH_DIR/dvdread/dvd_reader.h" "$IncludeDir/dvdread"/ 

	cd src/.libs
	$MSLIB /name:lib${Prefix}dvdread-4.dll /out:dvdread.lib /machine:$MSLibMachine /def:lib${Prefix}dvdread-4.dll.def
	move_files_to_dir "*.exp *.lib" "$LibDir/"
	
	reset_flags

	update_library_names_windows "lib${Prefix}dvdread.dll.a" "libdvdread.la"
fi

#dvdnav
if [ ! -f "$BinDir/lib${Prefix}dvdnav-4.dll" ]; then 
	unpack_bzip2_and_move "libdvdnav.tar.bz2" "$PKG_DIR_LIBDVDNAV"
	mkdir_and_move "$IntDir/libdvdnav"
	 
	sh $PKG_DIR/autogen.sh --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir LDFLAGS="$LDFLAGS -ldvdread"
	change_libname_spec
	make && make install

	cd src/.libs
	
	$MSLIB /name:lib${Prefix}dvdnav-4.dll /out:dvdnav.lib /machine:$MSLibMachine /def:lib${Prefix}dvdnav-4.dll.def
	$MSLIB /name:lib${Prefix}dvdnavmini-4.dll /out:dvdnavmini.lib /machine:$MSLibMachine /def:lib${Prefix}dvdnavmini-4.dll.def

	move_files_to_dir "*.exp *.lib" "$LibDir/"
	reset_flags

	update_library_names_windows "lib${Prefix}dvdnav.dll.a" "libdvdnav.la"
	update_library_names_windows "lib${Prefix}dvdnavmini.dll.a" "libdvdnavmini.la"
fi

#dvdcss
if [ ! -f "$BinDir/lib${Prefix}dvdcss-2.dll" ]; then 
	unpack_bzip2_and_move "libdvdcss.tar.bz2" "$PKG_DIR_LIBDVDCSS"
	mkdir_and_move "$IntDir/libdvdcss"
	 
	$PKG_DIR/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir 
	change_libname_spec
	make && make install

	cd src/.libs
	pexports "$BinDir/lib${Prefix}dvdcss-2.dll" | sed "s/^_//" > in.def
	sed '/LIBRARY lib${Prefix}dvdcss-2.dll/d' in.def > in-mod.def
	$MSLIB /name:lib${Prefix}dvdcss-2.dll /out:dvdcss.lib /machine:$MSLibMachine /def:in-mod.def


	move_files_to_dir "*.exp *.lib" "$LibDir/"
	reset_flags
	
	update_library_names_windows "lib${Prefix}dvdcss.dll.a" "libdvdcss.la"
fi

#ffmpeg GPL
if [ ! -f "$BinDir/lib${Prefix}avcodec-gpl-52.dll" ]; then 
	unpack_bzip2_and_move "ffmpeg.tar.bz2" "$PKG_DIR_FFMPEG"
	mkdir_and_move "$IntDir/ffmpeg-gpl"

	#GPL-compatible version
	#Please see http://www.mail-archive.com/ffmpeg-issues@lscube.org/msg04083.html 
	#for an explanation on why -Dav_cold=' ' works	
	CFLAGS=""
	CPPFLAGS=""
	LDFLAGS=""
	$PKG_DIR/configure --extra-ldflags="$LibFlags -Wl,--exclude-libs=libintl.a -Wl,--add-stdcall-alias" --extra-cflags="$IncludeFlags -mno-cygwin -mms-bitfields -D_WIN32_WINNT=0x0501" --enable-runtime-cpudetect --enable-avfilter-lavf --enable-avfilter --enable-avisynth --target-os=mingw32 --arch=i686 --cpu=i686 --enable-memalign-hack --enable-zlib --enable-bzlib --enable-libmp3lame --enable-libvorbis --enable-libopenjpeg --enable-libtheora --enable-libspeex --enable-libschroedinger --enable-ffmpeg --disable-ffplay --disable-ffserver --disable-debug --disable-static --enable-shared --enable-gpl --enable-libfaad --enable-libxvid --enable-libx264 --prefix=$InstallDir --bindir=$BinDir --libdir=$LibDir --shlibdir=$BinDir --incdir=$IncludeDir
	change_key "." "config.mak" "BUILDSUF" "-gpl"
	change_key "." "config.mak" "LIBPREF" "lib${Prefix}"
	change_key "." "config.mak" "SLIBPREF" "lib${Prefix}"
	#Adds $(SLIBPREF) to lib names when linking
	change_key "." "common.mak" "FFEXTRALIBS\ \\:" "\$\(addprefix\ -l\$\(SLIBPREF\),\$\(addsuffix\ \$\(BUILDSUF\),\$\(FFLIBS\)\)\)\ \$\(EXTRALIBS\)"
	make

	reset_flags 
	
	#Create .dll.a versions of the libs
	dlltool -U --dllname lib${Prefix}avutil-gpl-50.dll -d "libavutil/lib${Prefix}avutil-gpl-50.def" -l libavutil-gpl.dll.a
	dlltool -U --dllname lib${Prefix}avcodec-gpl-52.dll -d "libavcodec/lib${Prefix}avcodec-gpl-52.def" -l libavcodec-gpl.dll.a
	dlltool -U --dllname lib${Prefix}avdevice-gpl-52.dll -d "libavdevice/lib${Prefix}avdevice-gpl-52.def" -l libavdevice-gpl.dll.a
	dlltool -U --dllname lib${Prefix}avfilter-gpl-1.dll -d "libavfilter/lib${Prefix}avfilter-gpl-1.def" -l libavfilter-gpl.dll.a
	dlltool -U --dllname lib${Prefix}avformat-gpl-52.dll -d "libavformat/lib${Prefix}avformat-gpl-52.def" -l libavformat-gpl.dll.a
	dlltool -U --dllname lib${Prefix}swscale-gpl-0.dll -d "libswscale/lib${Prefix}swscale-gpl-0.def" -l libswscale-gpl.dll.a
	
	move_files_to_dir "*.dll.a" "$LibDir/"
	
	cp -p "ffmpeg.exe" "$BinDir/ffmpeg-gpl.exe"
	
	cp -p "libavutil/lib${Prefix}avutil-gpl-50.dll" "."
	cp -p "libavutil/lib${Prefix}avutil-gpl-50.dll" "$BinDir/"
	cp -p "libavutil/lib${Prefix}avutil-gpl-50.lib" "$LibDir/avutil-gpl.lib"
	
	cp -p "libavcodec/lib${Prefix}avcodec-gpl-52.dll" "."
	cp -p "libavcodec/lib${Prefix}avcodec-gpl-52.dll" "$BinDir/"
	cp -p "libavcodec/lib${Prefix}avcodec-gpl-52.lib" "$LibDir/avcodec-gpl.lib"
	
	cp -p "libavdevice/lib${Prefix}avdevice-gpl-52.dll" "."
	cp -p "libavdevice/lib${Prefix}avdevice-gpl-52.dll" "$BinDir/"
	cp -p "libavdevice/lib${Prefix}avdevice-gpl-52.lib" "$LibDir/avdevice-gpl.lib"
	
	cp -p "libavfilter/lib${Prefix}avfilter-gpl-1.dll" "."
	cp -p "libavfilter/lib${Prefix}avfilter-gpl-1.dll" "$BinDir/"
	cp -p "libavfilter/lib${Prefix}avfilter-gpl-1.lib" "$LibDir/avfilter-gpl.lib"
	
	cp -p "libavformat/lib${Prefix}avformat-gpl-52.dll" "."
	cp -p "libavformat/lib${Prefix}avformat-gpl-52.dll" "$BinDir/"
	cp -p "libavformat/lib${Prefix}avformat-gpl-52.lib" "$LibDir/avformat-gpl.lib"
	
	cp -p "libswscale/lib${Prefix}swscale-gpl-0.dll" "."
	cp -p "libswscale/lib${Prefix}swscale-gpl-0.dll" "$BinDir/"
	cp -p "libswscale/lib${Prefix}swscale-gpl-0.lib" "$LibDir/swscale-gpl.lib"
	
	#Copy some other dlls for testing
	copy_files_to_dir "$BinDir/*.dll" "."
fi

reset_flags

#Make sure the shared directory has all our updates
#create_shared

#Cleanup CRT
crt_shutdown

#Call common shutdown routines
common_shutdown

