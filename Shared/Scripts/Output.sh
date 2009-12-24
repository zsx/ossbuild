#!/bin/sh

output_startup() {
	echo "Setting up output and shared directories..."

	#$BUILD_DIR, $SHARED_DIR, etc. are found in Common.sh
	export OutDir=$BUILD_DIR/$OperatingSystemName/$PlatformName/$ConfigurationName
	export IntDir=$OutDir/obj
	export LibDir=$OutDir/lib
	export BinDir=$OutDir/bin
	export EtcDir=$OutDir/etc
	export IncludeDir=$OutDir/include
	export TemplateDir=$OutDir/templates
	export PkgConfigDir=$LibDir/pkgconfig
	export InstallDir=$OutDir

	export SharedOutDir=$SHARED_BUILD_DIR/$OperatingSystemName/$PlatformName
	export SharedLibDir=$SharedOutDir/lib
	export SharedBinDir=$SharedOutDir/bin
	export SharedEtcDir=$SharedOutDir/etc
	export SharedIncludeDir=$SharedOutDir/include
	export SharedTemplateDir=$SharedOutDir/templates
	export SharedPkgConfigDir=$SharedLibDir/pkgconfig
	export SharedInstallDir=$SharedOutDir
	
	export TemplateLibDir=$TemplateDir/lib
	export TemplatePkgConfigDir=$TemplateLibDir/pkgconfig
	export SharedTemplateLibDir=$SharedTemplateDir/lib
	export SharedTemplatePkgConfigDir=$SharedTemplateLibDir/pkgconfig

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
	
	#Create templates if we must
	if [ ! -f "$SharedLibDir/libpng.la" ]; then
		expand_templates
	fi
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

create_templates() {
	echo "Creating templates..."
	
	mkdir -p "$TemplateDir"
	mkdir -p "$TemplateLibDir"
	mkdir -p "$TemplatePkgConfigDir"
	
	cd "$LibDir"
	for f in `find *.la`; do create_template_libtool_la "$f"; done
	
	cd "$PkgConfigDir"
	for f in `find *.pc`; do create_template_pkgconfig_pc "$f"; done
}

create_template_libtool_la() {
	myla=$1
	echo "Creating libtool template for $myla"
	sedInstallDir=${InstallDir//\//\\\/}
	sedSharedLibDir=${SharedLibDir//\//\\\/}
	
	sed "s/$sedInstallDir/@SHARED_BUILD_DIR@/g" "$myla" > "$TemplateLibDir/tmp.la"
	sed "s/ -L$sedSharedLibDir//g" "$TemplateLibDir/tmp.la" > "$TemplateLibDir/$myla.in"
	rm -f "$TemplateLibDir/tmp.la"
	cp -p "$TemplateLibDir/$myla.in" "$SharedTemplateLibDir"
}

create_template_pkgconfig_pc() {
	mypc=$1
	echo "Creating pkg-config template for $mypc"
	sedInstallDir=${InstallDir//\//\\\/}
	
	sed "s/$sedInstallDir/@SHARED_BUILD_DIR@/g" "$mypc" > "$TemplatePkgConfigDir/$mypc.in"
	cp -p "$TemplatePkgConfigDir/$mypc.in" "$SharedTemplatePkgConfigDir"
}

expand_templates() {
	if [ -d "$SharedTemplateDir" ]; then
		echo "Expanding templates..."
		
		mkdir -p "$SharedPkgConfigDir"
		
		cd "$SharedTemplateLibDir"
		for f in `find *.la.in`; do expand_template_libtool_la "$f"; done
		
		cd "$SharedTemplatePkgConfigDir"
		for f in `find *.pc.in`; do expand_template_pkgconfig_pc "$f"; done
	fi
}

expand_template_libtool_la() {
	myla=$1
	mydestla=${myla%.*}
	sedSharedInstallDir=${SharedInstallDir//\//\\\/}
	
	sed "s/@SHARED_BUILD_DIR@/$sedSharedInstallDir/g" "$myla" > "$SharedLibDir/$mydestla"
}

expand_template_pkgconfig_pc() {
	mypc=$1
	mydestpc=${mypc%.*}
	sedSharedInstallDir=${SharedInstallDir//\//\\\/}
	
	sed "s/@SHARED_BUILD_DIR@/$sedSharedInstallDir/g" "$mypc" > "$SharedPkgConfigDir/$mydestpc"
}

create_shared() {
	if [ "$DISABLE_SHARED_COPY" = "1" ]; then 
		echo "Skipping copy to shared directory..."
		return
	fi
	
	echo "Copying to shared directory..."
	
	#Bin
	cd "$BinDir" && copy_files_to_dir "*" "$SharedBinDir"
	cd "$SharedBinDir" && remove_files_from_dir "*.def"
	
	#Lib
	cd "$LibDir" && copy_files_to_dir "*.a *.lib *.sh" "$SharedLibDir"
	test -d "glib-2.0" && cp -ru "glib-2.0" "$SharedLibDir"
	
	#Include
	cd "$IncludeDir" && cp -ru * "$SharedIncludeDir"
	
	#Create pkgconfig/libtool templates
	create_templates
	
	#Etc
	cd "$EtcDir/fonts" && cp -ru * "$SharedEtcDir/fonts"
}

update_library_names_windows() {
	if [ "$1" = "" ]; then
		echo "Missing archive name argument in update_library_names_windows"
		return
	fi
	
	mypwd=`pwd`
	mylib_a=$1
	mylib_la=$2
	myprefix=$DEFAULT_PREFIX
	mysuffix=$DEFAULT_SUFFIX
	if [ "$3" != "" ]; then
		myprefix=$3
	fi
	if [ "$4" != "" ]; then
		mysuffix=$4
	fi
	
	#Get the name of the lib w/o the prefix. e.g.: lib-ossbuild-abc.dll.a --> libabc.dll.a
	mydestlib_a=`echo "${mylib_a}" | sed -e "s/^\(.*\)${myprefix}\(.*\)${mysuffix}\(.*\)$/\1\2\3/g"`
	
	cd "$LibDir"
	if [ -e "$mylib_la" ]; then 
		sed -e "s/^library_names\(.*\)=\(.*\)${myprefix}\(.*\)${mysuffix}\(.*\)$/library_names\1=\2\3\4/g" "${mylib_la}" > "${mylib_la}.tmp"
		mv -f "${mylib_la}.tmp" "${mylib_la}"
	fi
	mv "$mylib_a" "$mydestlib_a"
	cd "$mypwd"
}

generate_libtool_la_windows() {
	if [ "$1" = "" ]; then
		echo "Missing file name argument in generate_la_windows_x86"
		return
	fi
	
	mycurrdir=`pwd`
	
	myla=$1
	if [ "$2" != "" ]; then
		mylib="../bin/$2"
	fi
	mylibname=''
	myoldlib=''
	myver_current=0
	myver_age=0
	myver_revision=0
	if [ "$3" != "" ]; then
		mylibname=$3
	fi
	if [ "$4" != "" ]; then
		myoldlib=$4
	fi
	if [ "$5" != "" ]; then
		myver_current=$5
	fi
	if [ "$6" != "" ]; then
		myver_age=$6
	fi
	if [ "$7" != "" ]; then
		myver_revision=$7
	fi
	
	cd "$LibDir"
	
	echo -en "# Generated by ossbuild - GNU libtool 1.5.22 (1.1220.2.365 2005/12/18 22:14:06)\n" > $myla
	echo -en "dlname='${mylib}'\n" >> $myla
	echo -en "library_names='${mylibname}'\n" >> $myla
	echo -en "old_library='${myoldlib}'\n" >> $myla
	echo -en "inherited_linker_flags=''\n" >> $myla
	echo -en "dependency_libs=' -L${LibDir} -L${SharedLibDir}'\n" >> $myla
	echo -en "weak_library_names=''\n" >> $myla
	echo -en "current=${myver_current}\n" >> $myla
	echo -en "age=${myver_age}\n" >> $myla
	echo -en "revision=${myver_revision}\n" >> $myla
	echo -en "installed=yes\n" >> $myla
	echo -en "shouldnotlink=no\n" >> $myla
	echo -en "dlopen=''\n" >> $myla
	echo -en "dlpreopen=''\n" >> $myla
	echo -en "libdir='${LibDir}'\n" >> $myla
	
	cd "$mycurrdir"
}

generate_libtool_la_wapi_x86() {
	if [ "$1" = "" ]; then
		echo "Missing file name argument in generate_libtool_la_wapi_x86"
		return
	fi
	if [ "$2" = "" ]; then
		echo "Missing library name argument in generate_libtool_la_wapi_x86"
		return
	fi
	
	mycurrdir=`pwd`
	
	myla=$1
	mylib=''
	mylibname=$2
	myoldlib=''
	myver_current=0
	myver_age=0
	myver_revision=0
	if [ "$3" != "" ]; then
		myoldlib=$3
	fi
	if [ "$4" != "" ]; then
		myver_current=$4
	fi
	if [ "$5" != "" ]; then
		myver_age=$5
	fi
	if [ "$6" != "" ]; then
		myver_revision=$6
	fi
	
	cd "/mingw/lib"
	
	if [ ! -e "$myla" ]; then 
		echo -en "# Generated by ossbuild - GNU libtool 1.5.22 (1.1220.2.365 2005/12/18 22:14:06)\n" > $myla
		echo -en "dlname=''\n" >> $myla
		echo -en "library_names='${mylibname}'\n" >> $myla
		echo -en "old_library='${myoldlib}'\n" >> $myla
		echo -en "inherited_linker_flags=''\n" >> $myla
		echo -en "dependency_libs=''\n" >> $myla
		echo -en "weak_library_names=''\n" >> $myla
		echo -en "current=${myver_current}\n" >> $myla
		echo -en "age=${myver_age}\n" >> $myla
		echo -en "revision=${myver_revision}\n" >> $myla
		echo -en "installed=yes\n" >> $myla
		echo -en "shouldnotlink=no\n" >> $myla
		echo -en "dlopen=''\n" >> $myla
		echo -en "dlpreopen=''\n" >> $myla
		echo -en "libdir='/mingw/lib/.'\n" >> $myla
	fi 
	
	cd "$mycurrdir"
}

generate_all_wapi_libtool_la_x86() {
	generate_libtool_la_wapi_x86 "libadvapi32.la" "libadvapi32.a"
	generate_libtool_la_wapi_x86 "libcrypt32.la" "libcrypt32.a"
	generate_libtool_la_wapi_x86 "libcomctl32.la" "libcomctl32.a"
	generate_libtool_la_wapi_x86 "libcomdlg32.la" "libcomdlg32.a"
	generate_libtool_la_wapi_x86 "libdnsapi.la" "libdnsapi.a"
	generate_libtool_la_wapi_x86 "libddraw.la" "libddraw.a"
	generate_libtool_la_wapi_x86 "libdsound.la" "libdsound.a"
	generate_libtool_la_wapi_x86 "libd3d9.la" "libd3d9.a"
	generate_libtool_la_wapi_x86 "libgdi32.la" "libgdi32.a"
	generate_libtool_la_wapi_x86 "libglu32.la" "libglu32.a"
	generate_libtool_la_wapi_x86 "libkernel32.la" "libkernel32.a"
	generate_libtool_la_wapi_x86 "libm.la" "libm.a"
	generate_libtool_la_wapi_x86 "libmpr.la" "libmpr.a"
	generate_libtool_la_wapi_x86 "libmsvcrt.la" "libmsvcrt.a"
	generate_libtool_la_wapi_x86 "libmsvfw32.la" "libmsvfw32.a"
	generate_libtool_la_wapi_x86 "libmswsock.la" "libmswsock.a"
	generate_libtool_la_wapi_x86 "libnetapi32.la" "libnetapi32.a"
	generate_libtool_la_wapi_x86 "libntdll.la" "libntdll.a"
	generate_libtool_la_wapi_x86 "libodbc32.la" "libodbc32.a"
	generate_libtool_la_wapi_x86 "libole32.la" "libole32.a"
	generate_libtool_la_wapi_x86 "libopengl32.la" "libopengl32.a"
	generate_libtool_la_wapi_x86 "libpsapi.la" "libpsapi.a"
	generate_libtool_la_wapi_x86 "libquartz.la" "libquartz.a"
	generate_libtool_la_wapi_x86 "libshell32.la" "libshell32.a"
	generate_libtool_la_wapi_x86 "libshlwapi.la" "libshlwapi.a"
	generate_libtool_la_wapi_x86 "libuser32.la" "libuser32.a"
	generate_libtool_la_wapi_x86 "libvfw32.la" "libvfw32.a"
	generate_libtool_la_wapi_x86 "libwininet.la" "libwininet.a"
	generate_libtool_la_wapi_x86 "libwinmm.la" "libwinmm.a"
	generate_libtool_la_wapi_x86 "libwow32.la" "libwow32.a"
	generate_libtool_la_wapi_x86 "libws2_32.la" "libws2_32.a"
	generate_libtool_la_wapi_x86 "libwsock32.la" "libwsock32.a"
}
