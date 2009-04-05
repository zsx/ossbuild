#!/bin/sh

#Specify which MSVCRT to use (VS6, VS2003, VS2005, VS2008, etc.)

crt_startup() {
	setup_crt
	select_crt
}

crt_shutdown() {
	restore_crt
	deselect_crt
}

setup_crt() {
	nothing=0
	
	#Compile manifest
	#mypwd=`pwd`
	#cd "$SHARED_MSVC_MANIFESTS_DIR"
	#
	#echo \#include \"winuser.h\" > msvcr90.dll.rc
	#echo 2 RT_MANIFEST Microsoft.VC90.CRT.manifest >> msvcr90.dll.rc
	#windres --input msvcr90.dll.rc --output msvcr90.dll.res --output-format=coff
	#
	#echo \#include \"winuser.h\" > msvcr90.exe.rc
	#echo 1 RT_MANIFEST Microsoft.VC90.CRT.manifest >> msvcr90.exe.rc
	#windres --input msvcr90.exe.rc --output msvcr90.exe.res --output-format=coff
	#
	#remove_files_from_dir "*.exe.rc *.dll.rc"
	#
	#cd "$mypwd"
	#
	##http://www.mingw.org/node/25#comment-106
	#
	#export TMP_PATH="$IntDir/temp"
	#export TMP_SPEC_FILE="$TMP_PATH/specs"
	#export GCC_SPEC_PATH=/c/msys/lib/gcc/mingw32/4.3.3
	#export GCC_SPEC_FILE="$GCC_SPEC_PATH/specs"
	#export GCC_SPEC_MSVCR_FILE="$GCC_SPEC_PATH/msvcr71"
	#
	#Save off the gcc spec file if it exists
	#mkdir -p "$TMP_PATH"
	#if [ -f "$GCC_SPEC_FILE" ]; then 
	#	cp -p "$GCC_SPEC_FILE" "$TMP_SPEC_FILE"
	#fi
	#
	#Use our own default spec file(s)
	#cp -p "$SHARED_GCC_SPECS_DIR/specs" "$GCC_SPEC_FILE"
	#cp -p "$SHARED_GCC_SPECS_DIR/msvcr71" "$GCC_SPEC_MSVCR_FILE"
}

restore_crt() {
	nothing=0

	#If gcc had a spec file specified, then copy it back
	#if [ -f "$TMP_SPEC_FILE" ]; then 
	#	cp -p "$TMP_SPEC_FILE" "$GCC_SPEC_FILE"
	#fi
	#if [ -f "$GCC_SPEC_MSVCR_FILE" ]; then 
	#	rm -f "$GCC_SPEC_MSVCR_FILE"
	#fi
}

select_crt() {
	nothing=0
	
	##export ORIG_LDFLAGS="-lmoldname71 -lmsvcr71 $ORIG_LDFLAGS"
	### $SHARED_GCC_MANIFESTS_DIR/msvcr90.exe.res"
	##export ORIG_CFLAGS="-D__MSVCRT_VERSION__=0x0710 $ORIG_CFLAGS"
	##export ORIG_CPPFLAGS="-D__MSVCRT_VERSION__=0x0710 $ORIG_CPPFLAGS"
	#
	#export ORIG_CFLAGS="-specs=msvcr71 $ORIG_CFLAGS"
	#export ORIG_LDFLAGS="-specs=msvcr71 $ORIG_LDFLAGS"
	#export PATH=$PATH:$SHARED_MSVC_BIN_DIR
	#reset_flags
}

deselect_crt() {
	nothing=0
	
	#reset_flags
}