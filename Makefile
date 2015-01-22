.PHONY: run

CC = clang++

CFLAGS = -Wno-c++11-compat-deprecated-writable-strings \
	$(shell sdl2-config --cflags)

LDFLAGS = $(shell sdl2-config --libs)

all: build/sdl_platform build/game.so

run: all
	./build/sdl_platform

build/sdl_platform: sdl*.cpp sdl*.h
	mkdir -p build
	clang++ sdl_platform.cpp $(CFLAGS) $(LDFLAGS) -o $@

build/game.so: game*.cpp game*.h
	mkdir -p build
	clang++ game.cpp -shared -undefined dynamic_lookup $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm -rf build
