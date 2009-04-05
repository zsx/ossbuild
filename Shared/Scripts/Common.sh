#!/bin/sh

#$0 is really pointing to the script calling this one (e.g. Libraries-x86.sh)

common_startup() {
	#Setup default configuration
	export OperatingSystemName=Linux
	export PlatformName=x86
	export ConfigurationName=Release
	export MSLibMachine=x86
	
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

translate_path_to_windows() {
	mypath=$1
	mypath=`cd "$mypath" && pwd`
	
	tmppath=${mypath:3}
	retpath=${mypath:1:1}:\\${tmppath//\//\\}
	
	echo $retpath
}
