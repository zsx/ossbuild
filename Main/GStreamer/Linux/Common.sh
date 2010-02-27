#!/bin/sh

###############################################################################
#                                                                             #
#                          Linux Common Settings                              #
#                                                                             #
# Provides common variables/settings used across these scripts.               #
#                                                                             #
###############################################################################

export GstApiVersion=0.10
export Farsight2ApiVersion=0.0
export PKG_URI="http://code.google.com/p/ossbuild/"




#Causes us to now always include the bin dir to look for libraries, even after calling reset_flags
export ORIG_LDFLAGS="$ORIG_LDFLAGS -L$BinDir -L$SharedBinDir"
reset_flags

#Setup possible license suffixes
export LicenseSuffixes='-lgpl -gpl'

#Setup GStreamer paths
export GstDir=$MAIN_DIR/GStreamer
export GstSrcDir=$GstDir/Source
export GstPatchDir=$GstSrcDir/patches
export GstIntDir=$IntDir

export GstPluginBinDir=$BinDir/gstreamer-$GstApiVersion
export GstPluginLibDir=$LibDir/gstreamer-$GstApiVersion
export GST_PLUGIN_PATH=$GstPluginBinDir

mkdir -p $GstPluginBinDir
mkdir -p $GstPluginLibDir

#Setup Farsight2 paths
export Farsight2BinDir=$BinDir/farsight2-$Farsight2ApiVersion
export Farsight2LibDir=$LibDir/farsight2-$Farsight2ApiVersion

mkdir -p $Farsight2BinDir
mkdir -p $Farsight2LibDir

#Augment paths to find our uninstalled binaries
export GST_PATH="$BinDir:$SharedBinDir:$GstPluginBinDir:$PATH"
export GST_LD_LIBRARY_PATH="$BinDir:$SharedBinDir:$GstPluginBinDir:$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH=$GST_LD_LIBRARY_PATH
export PATH=$GST_PATH

#Default configure options
export GstConfigureDefault=" --with-package-origin=$PKG_URI --disable-debug --disable-static --enable-shared --prefix=$InstallDir --libexecdir=$BinDir --bindir=$BinDir --libdir=$BinDir --includedir=$IncludeDir "

