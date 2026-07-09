#!/usr/bin/env bash
pushd "$(dirname "$0")" > /dev/null
ROOT="$(pwd)"

FILES=""
FILES="$FILES $ROOT/src/base/ftsystem.c"
FILES="$FILES $ROOT/src/base/ftinit.c"
FILES="$FILES $ROOT/src/base/ftdebug.c"
FILES="$FILES $ROOT/src/base/ftbase.c"
FILES="$FILES $ROOT/src/base/ftbitmap.c"
FILES="$FILES $ROOT/src/sfnt/sfnt.c"
FILES="$FILES $ROOT/src/truetype/truetype.c"
FILES="$FILES $ROOT/src/cff/cff.c"
FILES="$FILES $ROOT/src/smooth/smooth.c"
FILES="$FILES $ROOT/src/psaux/psaux.c"
FILES="$FILES $ROOT/src/psnames/psnames.c"
FILES="$FILES $ROOT/src/pshinter/pshinter.c"

BUILD_DIR="$ROOT/../x64"
if [ "$Platform" = "X86" ] || [ "$Platform" = "x86" ]; then
    BUILD_DIR="$ROOT/../x86"
fi
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

rm -f *.o 2> /dev/null
cc -c -O2 -g -DFT2_BUILD_LIBRARY -D_CRT_SECURE_NO_WARNINGS -I"$ROOT" $FILES
ar rcs libfreetype.a *.o
rm -f *.o 2> /dev/null
popd > /dev/null
