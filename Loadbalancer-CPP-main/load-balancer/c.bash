#!/bin/bash
set -e

CC=g++
SRC="main.cpp"
OUT="a"

if [[ "$1" == "debug" ]]; then
    CFLAGS="-g -Wall"
    OUT="a"
else
    CFLAGS="-O2 -Wall"
fi

echo "Compiling $SRC -> $OUT with flags: $CFLAGS"
$CC $CFLAGS "$SRC" -o "$OUT"

echo "Build complete: ./$OUT"
./"$OUT"