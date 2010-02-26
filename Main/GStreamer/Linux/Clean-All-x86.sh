#!/bin/sh

###############################################################################
#                                                                             #
#                             Linux x86 Clean                                 #
#                                                                             #
# Cleans out any gstreamer elements, plugins, etc.                            #
#                                                                             #
###############################################################################

TOP=$(dirname $0)/../../..

#Call common startup routines to load a bunch of variables we'll need
. $TOP/Shared/Scripts/Common.sh
common_startup "Linux" "x86" "Release"


echo "Removing intermediate directory: $BUILD_DIR"
rm -rf "$BUILD_DIR"


#Call common shutdown routines
common_shutdown

