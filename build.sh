#!/bin/sh

# '/bin/sh' is the most portable way to write a shellscript but doesn't
# support some features that bash does (e.g. using '[[' is not supported
# you can you '[').
#
# To be sure that the script is portable and fully POSIX # compliant use a
# tool like shellcheck to validate it.
# 
# https://github.com/koalaman/shellcheck

# shellcheck disable=SC2048,SC2086 

set -e

# Make sure we are at the root of the project
cd "$(realpath "$(dirname "$0")")"

SRC_ROOT="$PWD/code"
CUSTOM_ROOT="$SRC_ROOT/custom"
BUILD_ROOT="$PWD/build"

# c++ is the default toolchain's compiler installed on the machine
if [ -z "$CC" ]; then
    CC=c++
fi

CFLAGS="-D_GNU_SOURCE -fPIC -fpermissive -Wno-write-strings -Wno-comment"
CFLAGS="$CFLAGS -Wno-null-dereference -Wno-logical-op-parentheses -Wno-switch"
CFLAGS="$CFLAGS -I$SRC_ROOT -I$CUSTOM_ROOT"

if [ ! -d "$BUILD_ROOT" ]; then
    mkdir -p "$BUILD_ROOT"
fi

eval "$CC $CFLAGS $SRC_ROOT/4ed_build.cpp -g -o $BUILD_ROOT/build"

"$BUILD_ROOT/build" $*
