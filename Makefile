.PHONY: run

default: run

run: build/sdl_platform.app
	open build/sdl_platform.app

build: build/sdl_platform.app

build/sdl_platform.app: sdl_platform.cpp
	mkdir -p build/sdl_platform.app/Contents/MacOS
	clang++ sdl_platform.cpp -I/usr/local/include/SDL -D_GNU_SOURCE=1 -D_THREAD_SAFE -L/usr/local/lib  -Wl,-framework,Cocoa  -lSDL2 -o build/sdl_platform.app/Contents/MacOS/sdl_platform
