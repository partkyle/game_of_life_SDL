#!/bin/sh -e

CC="clang -std=c89"

CFLAGS="-Wno-c++11-compat-deprecated-writable-strings -Ivendor/SDL2-2.0.3/include"

LDFLAGS="-Lvendor/SDL2-2.0.3/lib -lSDL2"


lockfile=build/game.so.lock

mkdir -p build

touch $lockfile
  CC game.cpp $CFLAGS $LDFLAGS -g -shared -undefined dynamic_lookup -o build/game.so
rm $lockfile

CC sdl_platform.cpp $CFLAGS $LDFLAGS -g -o build/sdl_platform
