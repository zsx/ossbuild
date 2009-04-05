#!/bin/sh

TOP=$(dirname $0)/..

#Call common startup routines to load a bunch of variables we'll need
. $TOP/Shared/Scripts/Common.sh
common_startup "Windows" "x64" "Release" "x64"

#Select which MS CRT we want to build against (msvcr90.dll)
. $ROOT/Shared/Scripts/CRT-x86_64.sh
crt_startup

#Move to intermediate directory
cd "$IntDir"

#Add MS build tools
setup_ms_build_env_path




echo "Fill this in..."




reset_flags

#Cleanup CRT
crt_shutdown

#Call common shutdown routines
common_shutdown
