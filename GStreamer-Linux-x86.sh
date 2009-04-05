#!/bin/sh

#Call common startup routines to load a bunch of variables we'll need
. $(dirname $0)/Properties/Common.sh
common_startup "Linux" "x86" "Release"

#Move to intermediate directory
cd "$IntDir"

#Compile in the gstreamer subpath (e.g. x86/Release/obj/gstreamer/)
output_append_subpath "gstreamer"

#gstreamer
if [ ! -f "$LibDir/libgstreamer-0.10.so" ]; then 
	mkdir_and_move "$IntDir/gstreamer"
	$MAIN_DIR/GStreamer/Source/gstreamer/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	make && make install
fi

#gst-plugins-base
#if [ ! -f "$LibDir/libgstreamer-0.10.so" ]; then 
	mkdir_and_move "$IntDir/gst-plugins-base"
	$MAIN_DIR/GStreamer/Source/gst-plugins-base/configure --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$LibDir --includedir=$IncludeDir
	#make && make install
#fi

#Call common shutdown routines
common_shutdown
