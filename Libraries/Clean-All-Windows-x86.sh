#!/bin/sh

TOP=$(dirname $0)/..

#Call common startup routines to load a bunch of variables we'll need
. $TOP/Shared/Scripts/Common.sh
common_startup "Windows" "Win32" "Release" "x86"

echo "Removing unpacked source directory: $LIBRARIES_UNPACK_DIR"
rm -rf "$LIBRARIES_UNPACK_DIR"

echo "Cleaning precompiled shared binaries directory (preserving VCS folders): $SharedOutDir"
cd "$SharedOutDir"
find . -type f -writable -exec rm -rf {} \;

echo "Removing intermediate directory: $BUILD_DIR"
rm -rf "$BUILD_DIR"


#Call common shutdown routines
common_shutdown

