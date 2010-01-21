#!/bin/sh

#$0 is really pointing to the script calling this one (e.g. Libraries-x86.sh)

common_startup() {
	#Setup default configuration
	export COMPUTERNAME=OSSBuild
	export OperatingSystemName=Linux
	export PlatformName=x86
	export ConfigurationName=Release
	export MSLibMachine=x86
	export BuildTriplet=
	export BuildTripletDash=
	export HostTriplet=
	export HostTripletDash=
	
	export DEFAULT_PREFIX=-ossbuild-
	export DefaultPrefix=$DEFAULT_PREFIX
	
	export DEFAULT_SUFFIX=
	export DefaultSuffix=$DEFAULT_SUFFIX
	
	if [ "$TOP" = "" ]; then
		export TOP=$(dirname $0)
	fi

	#Gather some important variables
	export CURR_DIR=`pwd`
	export ROOT=$( (cd -P "$TOP" && pwd) )
	
	export MAIN_DIR=$ROOT/Main
	export BUILD_DIR=$ROOT/Build
	export TOOLS_DIR=$ROOT/Tools
	export SHARED_DIR=$ROOT/Shared
	export LIBRARIES_DIR=$ROOT/Libraries
	export PACKAGING_DIR=$ROOT/Packaging
	export DEPLOYMENT_DIR=$ROOT/Deployment
	
	export LIBRARIES_PATCH_DIR=$LIBRARIES_DIR/Patches
	export LIBRARIES_UNPACK_DIR=$LIBRARIES_DIR/Source
	export LIBRARIES_PACKAGE_DIR=$LIBRARIES_DIR/Packages
	
	export SHARED_SDK_DIR=$SHARED_DIR/SDKs
	
	export SHARED_BUILD_DIR=$SHARED_DIR/Build
	export SHARED_SCRIPTS_DIR=$SHARED_DIR/Scripts
	export PLATFORM_CONFIG_SCRIPT=$SHARED_SCRIPTS_DIR/Arch-$PlatformName.sh
	
	export SHARED_GCC_DIR=$SHARED_DIR/GCC
	export SHARED_GCC_SPECS_DIR=$SHARED_GCC_DIR/Specs
	
	export SHARED_MSVC_DIR=$SHARED_DIR/MSVC
	export SHARED_MSVC_BIN_DIR=$SHARED_MSVC_DIR/Bin
	export SHARED_MSVC_INCLUDE_DIR=$SHARED_MSVC_DIR/Include
	export SHARED_MSVC_MANIFESTS_DIR=$SHARED_MSVC_DIR/Manifests
	export SHARED_MSVC_PROPERTIES_DIR=$SHARED_MSVC_DIR/Properties
	
	export PATH=$PATH:$TOOLS_DIR

	#If we called this function with arguments, then be sure to use those
	if [ "$1" != "" ]; then
		export OperatingSystemName=$1
	fi
	if [ "$2" != "" ]; then
		export PlatformName=$2
	fi
	if [ "$3" != "" ]; then
		export ConfigurationName=$3
	fi
	if [ "$4" != "" ]; then
		export MSLibMachine=$4
	fi
	if [ "$5" != "" ]; then
		export BuildTriplet=$5
	fi
	if [ "$6" != "" ]; then
		export HostTriplet=$6
	else
		export HostTriplet=$BuildTriplet
	fi
	
	if [ "$BuildTriplet" != "" ]; then
		export BuildTripletDash=${BuildTriplet}-
	fi
	
	if [ "$HostTriplet" != "" ]; then
		export HostTripletDash=${HostTriplet}-
	fi
	
	export Host=$HostTriplet
	export Build=$BuildTriplet
	
	export configureHost=--host=$Host
	export configureBuild=--build=$Build

	#Make sure we're in the root directory
	cd "$ROOT"

	#Call Arch-x86.sh or Arch-x86_64.sh if it exists
	if [ ! -f "$PLATFORM_CONFIG_SCRIPT" ]; then 
		. "$PLATFORM_CONFIG_SCRIPT"
		platform_startup
	fi

	#Setup output variables
	. "$SHARED_SCRIPTS_DIR/Output.sh"
	output_startup
}

common_shutdown() {
	#Make sure we're in the root directory
	cd "$ROOT"

	#Shutdown in reverse order of startup
	output_shutdown
	if [ ! -f "$PLATFORM_CONFIG_SCRIPT" ]; then 
		platform_shutdown
	fi

	#Move back to the original directory we called this from
	cd "$CURR_DIR"

	echo "Done!"
}

mkdir_and_move() {
	if [ "$1" = "" ]; then
		echo "Missing argument in mkdir_and_move"
		return
	fi

	mydir=$1
	
	#Not sure why, but sometimes a carriage return gets into the dir name
	mydir=$( echo $mydir | tr -d "\r" )

	mkdir -p $mydir
	cd $mydir
}

copy_files_to_dir() {
	myfiles=$1
	mydir=$2
	for f in `find $myfiles`; do cp "$f" "$mydir"; done
}

move_files_to_dir() {
	myfiles=$1
	mydir=$2
	for f in `find $myfiles`; do mv "$f" "$mydir"; done
}

remove_files_from_dir() {
	myfiles=$1
	for f in `find $myfiles`; do rm -f "$f"; done
}

change_libtool_file_magic_windows_x86() {
	change_cmd "file_magic_cmd" "export win32_libid_type='x86 DLL' \&\& echo \$win32_libid_type"
}

change_cmd() {
	mycurrdir=`pwd`
	myquote='\"'
	mycmd="export win32_libid_type='x86 DLL' \&\& echo \$win32_libid_type"
	mydir=.
	myfile=libtool
	mycmdname=file_magic_cmd
	if [ "$1" != "" ]; then
		mycmdname=$1
	fi
	if [ "$2" != "" ]; then
		mycmd=$2
	fi
	if [ "$3" != "" ]; then
		mydir=$3
	fi
	if [ "$4" != "" ]; then
		myfile=$4
	fi
	
	mycmd=${myquote}${mycmd}${myquote}
	
	cd "${mydir}"
	cat "${myfile}" | sed "s/^${mycmdname}.*=.*$/${mycmdname}=${mycmd}/g" > "${myfile}.tmp"
	mv -f "${myfile}.tmp" "${myfile}"
	cd "${mycurrdir}"
}

change_suffix() {
	if [ "$1" = "" ]; then
		echo "Missing suffix argument in change_suffix"
		return
	fi
	
	mycurrdir=`pwd`
	mysuffix=$1
	mydir=.
	myfile=config.mak
	mysuffixname=BUILDSUF
	if [ "$2" != "" ]; then
		mydir=$2
	fi
	if [ "$3" != "" ]; then
		myfile=$3
	fi
	if [ "$4" != "" ]; then
		mysuffixname=$4
	fi
	
	cd "${mydir}"
	cat "${myfile}" | sed "s/^${mysuffixname}.*=.*$/${mysuffixname}=${mysuffix}/g" > "${myfile}.tmp"
	mv -f "${myfile}.tmp" "${myfile}"
	cd "${mycurrdir}"
}

change_package() {
	mycurrdir=`pwd`
	mypackage=$DEFAULT_PREFIX
	mydir=.
	myfile=Makefile
	mypackagename=PACKAGE
	if [ "$1" != "" ]; then
		mypackage=$1
	fi
	if [ "$2" != "" ]; then
		mydir=$2
	fi
	if [ "$3" != "" ]; then
		myfile=$3
	fi
	if [ "$4" != "" ]; then
		mypackagename=$4
	fi
	
	cd "${mydir}"
	cat "${myfile}" | sed "s/^${mypackagename}.*=.*$/${mypackagename}=${mypackage}/g" > "${myfile}.tmp"
	mv -f "${myfile}.tmp" "${myfile}"
	cd "${mycurrdir}"
}

change_key() {
	mycurrdir=`pwd`
	mydir=.
	myfile=Makefile
	mykey=
	myvalue=
	if [ "$1" != "" ]; then
		mydir=$1
	fi
	if [ "$2" != "" ]; then
		myfile=$2
	fi
	if [ "$3" != "" ]; then
		mykey=$3
	fi
	if [ "$4" != "" ]; then
		myvalue=$4
	fi
	
	cd "${mydir}"
	cat "${myfile}" | sed "s:^${mykey}.*=.*$:${mykey}=${myvalue}:g" > "${myfile}.tmp"
	mv -f "${myfile}.tmp" "${myfile}"
	cd "${mycurrdir}"
}

change_libname_spec() {
	mycurrdir=`pwd`
	myprefix=$DEFAULT_PREFIX
	mysuffix=$DEFAULT_SUFFIX
	mydir=.
	myfile=libtool
	#libname_spec="lib\$name" --> libname_spec="lib-ossbuild-\$name"
	mylibname_spec=libname_spec
	myspec_1='\"lib'
	myspec_2='\\$name'
	myspec_3='\"'
	if [ "$1" != "" ]; then
		myprefix=$1
	fi
	if [ "$2" != "" ]; then
		mysuffix=$2
	fi
	if [ "$3" != "" ]; then
		mydir=$3
	fi
	if [ "$4" != "" ]; then
		myfile=$4
	fi
	if [ "$5" != "" ]; then
		mylibname_spec=$5
	fi
	if [ "$6" != "" ]; then
		myprefix_ex=$6
	fi
	
	myprefix=${myspec_1}${myprefix}${myspec_2}${mysuffix}${myspec_3}
	
	cd "${mydir}"
	cat "${myfile}" | sed "s/^${mylibname_spec}.*=.*$/${mylibname_spec}=${myprefix}/g" > "${myfile}.tmp"
	mv -f "${myfile}.tmp" "${myfile}"
	cd "${mycurrdir}"
}

translate_path_to_windows() {
	mypath=$1
	mypath=`cd "$mypath" && pwd`
	
	tmppath=${mypath:3}
	retpath=${mypath:1:1}:\\${tmppath//\//\\}
	
	echo $retpath
}

init_unpack_and_move() {
	myfile=$1
	mymovedir=$2
	mycreatesubdir=$3
	mycreatedir=$LIBRARIES_UNPACK_DIR
	mydir=$LIBRARIES_UNPACK_DIR/$mymovedir
	
	#Not sure why, but sometimes a carriage return gets into the dir name
	mydir=$( echo $mydir | tr -d "\r" )

	if [ "$mycreatesubdir" != "" ]; then
		mycreatedir=$mycreatedir/$mycreatesubdir
	fi
	
	mycreatedir=$( echo $mycreatedir | tr -d "\r" )
	
	export PKG_DIR=$mydir
	if [ -d "$mydir" ]; then
		cd "$mycreatedir"
		return 1
	fi
	mkdir -p "$mydir"
	mkdir -p "$mycreatedir"
	cd "$mycreatedir"
	return 0
}

unpack_bzip2_and_move() {
	init_unpack_and_move "$1" "$2" "$3"
	if [ "$?" -eq "1" ]; then
		cd "$PKG_DIR"
		return
	fi
	echo Extracting $1...
	tar xjvf "$LIBRARIES_PACKAGE_DIR/$1" > NUL
	cd "$PKG_DIR"
}

unpack_gzip_and_move() {
	init_unpack_and_move "$1" "$2" "$3"
	if [ "$?" -eq "1" ]; then
		cd "$PKG_DIR"
		return
	fi
	echo Extracting $1...
	tar xzvf "$LIBRARIES_PACKAGE_DIR/$1" > NUL
	cd "$PKG_DIR"
}

unpack_zip_and_move_windows() {
	init_unpack_and_move "$1" "$2" "$3"
	if [ "$?" -eq "1" ]; then
		cd "$PKG_DIR"
		return
	fi
	echo Extracting $1...
	7z x "$LIBRARIES_PACKAGE_DIR/$1" > NUL
	cd "$PKG_DIR"
}

unpack_zip_and_move_linux() {
	init_unpack_and_move "$1" "$2" "$3"
	if [ "$?" -eq "1" ]; then
		cd "$PKG_DIR"
		return
	fi
	echo Extracting $1...
	unzip "$LIBRARIES_PACKAGE_DIR/$1" > NUL
	cd "$PKG_DIR"
}
