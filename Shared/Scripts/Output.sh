#!/bin/sh

output_startup() {
	echo "Setting up output and shared directories..."

	#$BUILD_DIR, $SHARED_DIR, etc. are found in Common.sh
	export OutDir=$BUILD_DIR/$OperatingSystemName/$PlatformName/$ConfigurationName
	export IntDir=$OutDir/obj
	export LibDir=$OutDir/lib
	export BinDir=$OutDir/bin
	export IncludeDir=$OutDir/include
	export PkgConfigDir=$LibDir/pkgconfig
	export InstallDir=$OutDir

	export SharedOutDir=$SHARED_BUILD_DIR/$OperatingSystemName/$PlatformName
	export SharedLibDir=$SharedOutDir/lib
	export SharedBinDir=$SharedOutDir/bin
	export SharedIncludeDir=$SharedOutDir/include
	export SharedPkgConfigDir=$SharedLibDir/pkgconfig
	export SharedInstallDir=$SharedOutDir

	#Setup compile/link flags (includes, search directories, etc.)
	export LibFlags="-L$LibDir -L$SharedLibDir"
	export IncludeFlags="-I$IncludeDir -I$SharedIncludeDir"

	#Save off originals so we can reset to these values if needed
	export ORIG_PATH=$PATH
	export ORIG_CFLAGS=$CFLAGS
	export ORIG_LDFLAGS=$LDFLAGS
	export ORIG_CPPFLAGS=$CPPFLAGS
	export ORIG_PKG_CONFIG_PATH=$PKG_CONFIG_PATH

	#Ensure variables are in a known state
	reset_path
	reset_flags
	reset_pkgconfig_path

	#Make sure directories exist and ignore whether or not shared directories exist (we don't care)
	mkdir -p "$BUILD_DIR"
	mkdir -p "$OutDir"
	mkdir -p "$IntDir"
	mkdir -p "$LibDir"
	mkdir -p "$BinDir"
	mkdir -p "$IncludeDir"
	mkdir -p "$InstallDir"
}

output_shutdown() {
	nothing=0
}

clear_cflags() {
	export CFLAGS=
	export CPPFLAGS=
}

clear_ldflags() {
	export LDFLAGS=
}

clear_flags() {
	clear_cflags
	clear_ldflags
}

reset_path() {
	export PATH="$BinDir:$SharedBinDir:$ORIG_PATH"
}

reset_flags() {
	export CFLAGS="$ORIG_CFLAGS $IncludeFlags"
	export LDFLAGS="$ORIG_LDFLAGS $LibFlags"
	export CPPFLAGS="$ORIG_CPPFLAGS $IncludeFlags"
}

reset_pkgconfig_path() {
	export PKG_CONFIG_PATH="$PkgConfigDir:$SharedPkgConfigDir:$ORIG_PKG_CONFIG_PATH"
}

output_append_subpath() {
	mysuffix=$1

	export IntDir=$OutDir/obj/$mysuffix
	export LibDir=$OutDir/lib/$mysuffix
	export BinDir=$OutDir/bin/$mysuffix
	export IncludeDir=$OutDir/include/$mysuffix
	export PkgConfigDir=$LibDir/pkgconfig
	#export InstallDir=$OutDir

	export SharedLibDir=$SharedOutDir/lib/$mysuffix
	export SharedBinDir=$SharedOutDir/bin/$mysuffix
	export SharedIncludeDir=$SharedOutDir/include/$mysuffix
	export SharedPkgConfigDir=$SharedLibDir/pkgconfig
	#export SharedInstallDir=$SharedOutDir

	export LibFlags="-L$LibDir -L$SharedLibDir $LibFlags"
	export IncludeFlags="-I$IncludeDir -I$SharedIncludeDir $IncludeFlags"

	export PATH="$BinDir:$SharedBinDir:$PATH"
	export PKG_CONFIG_PATH="$PkgConfigDir:$SharedPkgConfigDir:$PKG_CONFIG_PATH"

	reset_flags
}

arrange_shared() {
	mydir=$1
	mylib=$2
	mymajor=$3
	myfullversion=$4
	mylibtoola=$5
	mypkgcfg=$6
	mylibdir=$7
	mycurrdir=$(pwd)

	if [ -f "$mydir/$mylib" ]; then
		rm -rf "$mydir/$mylib"
	fi
	if [ -f "$mydir/$mylib.$myfullversion" ]; then
		mv "$mydir/$mylib.$myfullversion" "$mydir/$mylib.$mymajor"
	fi
	if [ -f "$mydir/$mylib.$mymajor" ]; then
		cd "$mydir"
		ln -s "$mylib.$mymajor" "$mylib"
	fi

	cd "$mycurrdir"

	#Move the libtool file if it exists
	if [ "$mylibdir" != "" ]; then
		if [ -f "$mydir/$mylibtoola" ]; then
			mv "$mydir/$mylibtoola" "$mylibdir"
		fi
		if [ "$mypkgcfg" != "" ]; then
			mkdir -p "$mylibdir/pkgconfig"
			cd "$mydir/pkgconfig/"
			move_files_to_dir "$mypkgcfg" "$mylibdir/pkgconfig"
		fi
	fi

	cd "$mycurrdir"
}

ms_lib_from_dll() {
	mydll=$1
	mylib=$2
	#pexports "$mydll" | sed "s/^_//" > tmp.txt
	pexports "$mydll" > tmp.txt
	sed "/LIBRARY/d" tmp.txt > in.def
	#dlltool -U -d in.def -l "$mylib"
	$MSLIB /out:"$mylib" /machine:$MSLibMachine /def:in.def
}

setup_ms_build_env_path() {
	#Make sure that the tools are in the path
	#For libraries such as ffmpeg to correctly build .lib files, you must make sure that 
	#lib.exe is in the path. The following should do the trick (if you have VS 2008 installed).
	cd "$TOOLS_DIR"
	VSCOMNTOOLS=`cmd.exe /c "msysPath.bat \"$VS90COMNTOOLS\""`
	MSVCIDE=`cd "$VSCOMNTOOLS" && cd "../IDE/" && pwd`
	MSVCTOOLS=`cd "$VSCOMNTOOLS" && cd "../../VC/bin/" && pwd`
	PATH=$PATH:$MSVCIDE:$MSVCTOOLS
	MSLIB=lib.exe
}
