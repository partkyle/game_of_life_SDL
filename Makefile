.PHONY: run

COMPILER_FLAGS = -Wno-c++11-compat-deprecated-writable-strings

default: build

clean:
	rm -rf build

run: build
	open build/sdl_platform.app

build: build/sdl_platform.app/Contents/MacOS/sdl_platform \
	     build/sdl_platform.app/Contents/MacOS/game.so

build/sdl_platform.app:
	mkdir -p build/sdl_platform.app/Contents/MacOS

build/sdl_platform.app/Contents/MacOS/sdl_platform: build/sdl_platform.app sdl*.h sdl*.cpp
	clang++ sdl_platform.cpp $(COMPILER_FLAGS) -I/usr/local/include/SDL -D_GNU_SOURCE=1 -D_THREAD_SAFE -L/usr/local/lib  -Wl,-framework,Cocoa -lSDL2 -o build/sdl_platform.app/Contents/MacOS/sdl_platform

build/sdl_platform.app/Contents/MacOS/game.so: build/sdl_platform.app game*.cpp game*.h
	clang++ game.cpp $(COMPILER_FLAGS) -shared -undefined dynamic_lookup -I/usr/local/include/SDL -D_GNU_SOURCE=1 -D_THREAD_SAFE -L/usr/local/lib  -Wl,-framework,Cocoa -lSDL2 -o build/sdl_platform.app/Contents/MacOS/game.so
