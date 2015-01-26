#!/bin/sh -e

CC="clang -std=c89"

CFLAGS="-Wno-c++11-compat-deprecated-writable-strings -ISDL2-2.0.3/include"

LDFLAGS="-LSDL2-2.0.3/lib -lSDL2"


lockfile=build/build.lock

mkdir -p build

touch $lockfile

CC game.cpp $CFLAGS $LDFLAGS -shared -undefined dynamic_lookup -o build/game.so
CC sdl_platform.cpp $CFLAGS $LDFLAGS -o build/sdl_platform

rm $lockfile
