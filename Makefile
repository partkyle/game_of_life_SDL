.PHONY: run all

all:
	./build-osx.sh

run: all
	./build/sdl_platform

clean:
	rm -rf build
