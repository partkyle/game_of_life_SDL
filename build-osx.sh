CFLAGS="-Wno-c++11-compat-deprecated-writable-strings -ISDL2-2.0.3/include"

LDFLAGS="-LSDL2-2.0.3/lib -lSDL2"

mkdir -p build
clang++ game.cpp $CFLAGS $LDFLAGS -shared -undefined dynamic_lookup -o build/game.so
clang++ sdl_platform.cpp $CFLAGS $LDFLAGS -o build/sdl_platform
