# nyan project utility commands

builddir := "build"

help:
	@just --list

# configure the build dir with cmake
configure:
	cmake -S . -B {{builddir}} -G Ninja

# build the nyan library and tools; configures the build dir on first run
build:
	@test -f {{builddir}}/CMakeCache.txt || just configure
	cmake --build {{builddir}}

# parse the test files with nyancat
test: build
	{{builddir}}/nyan/nyancat --test-parser -f test/test.nyan

# create the documentation
doc: build
	cmake --build {{builddir}} --target doc

# remove object files and binaries
clean:
	cmake --build {{builddir}} --target clean

# remove the build dir
cleanbuild:
	rm -rf {{builddir}}
