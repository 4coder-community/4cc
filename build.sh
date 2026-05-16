#!/usr/bin/env bash

set -e

# Make sure we are at the root of the project
cd "$(realpath "$(dirname "$0")")"

SRC_ROOT="$PWD/code"
CUSTOM_ROOT="$SRC_ROOT/custom"
BUILD_ROOT="$PWD/build"

if [[ -z "$CC" ]]; then
    CC=c++
fi

CFLAGS="-D_GNU_SOURCE -fPIC -fpermissive -Wno-write-strings -Wno-comment"
CFLAGS="$CFLAGS -Wno-null-dereference -Wno-logical-op-parentheses -Wno-switch"
CFLAGS="$CFLAGS -I$SRC_ROOT -I$CUSTOM_ROOT"

if [[ ! -d "$BUILD_ROOT" ]]; then
    mkdir -p "$BUILD_ROOT"
fi

eval "$CC $CFLAGS $SRC_ROOT/4ed_build.cpp -g -o $BUILD_ROOT/build"

pushd "$SRC_ROOT"
"$BUILD_ROOT/build" $*
popd
