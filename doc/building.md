# Building nyan

This project requires a C++17 compiler (e.g gcc >= 7 or clang >= 4) and
uses `flex` and `cmake`.


## Dependencies

### Ubuntu 16.04 or later

```
sudo apt-get update
sudo apt-get install cmake flex make
```

For gcc: `sudo apt-get install gcc g++`

For clang: `sudo apt-get install clang`


### Archlinux

```
sudo pacman -Syu --as-needed cmake flex make clang gcc
```

### Gentoo

```
sudo emerge -avt cmake flex make
```

### Windows

##### MSYS2
  - Download and follow the install instructions for [MSYS2](https://www.msys2.org/)
    - if you have [Chocolatey](chocolatey.org/) installed you can install MSYS2 as follows

    `choco install msys2`

  - After installing start a MSYS2-shell from `<MSYS2-directory>/msys2_shell.cmd`
  - Update the preinstalled packages

`pacman -Syuu`

  - Install one of these compilers:

For gcc: `pacman -Syu --needed mingw-w64-x86_64-gcc`

For clang: `pacman -Syu --needed mingw-w64-x86_64-clang`

And these dependencies:
`pacman -Syu --needed git flex mingw-w64-x86_64-cmake mingw-w64-x86_64-make mingw-w64-x86_64-dlfcn`


##### Visual Studio
  - [CMake](https://cmake.org/download/)
  - [Visual Studio 2017 Community edition](https://www.visualstudio.com/downloads/)
  - [flex](https://sourceforge.net/projects/winflexbison/)
    - The path to win_flex.exe needs to be added to the PATH environment variable
  
## Clone this repository

```
git clone https://github.com/SFTtech/nyan.git
```

## Build

This project is built like every standard [cmake project](http://lmgtfy.com/?q=building+a+cmake+project). It boils down to:

```
cd nyan
mkdir build
cd build
cmake ..
make -j$(nproc)
```

#### Building on MSYS2 (Windows)

Add the `bin`-directory to your `PATH` (in MSYS2):
```
echo "export PATH=/mingw64/bin:mingw64\x86_64-w64-mingw32\bin:$PATH" >> ~/.bashrc
source ~/.bashrc
```

Then configure the build directory:
```
cd nyan
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=mingw32-make -DFLEX_EXECUTABLE=/usr/bin/flex.exe -G "CodeBlocks - MinGW Makefiles" ..
```

__**IMPORTANT:**__ [`MinGW Makefiles`](https://cmake.org/cmake/help/v3.12/generator/MinGW%20Makefiles.html#generator:MinGW%20Makefiles) flag for the generator, means that CMake generates makefiles for use with `mingw32-make`
under a Windows command prompt (CMD). So you need to add the path to the `<msys2-install-dir>/mingw64/bin` as well to your [Windows `PATH` environment variable](https://lmgtfy.com/?q=windows+add+folder+to+path+environment&s=d).


Navigate to the `build`-directory within your systems file explorer and put `cmd` into the adress bar. This should start up a cmd shell inside that folder.
Now start the build process in the cmd shell with this command:

`mingw32-make -j2 VERBOSE=1`

You should find the `libnyan.dll` and `libnyan.dll.a` inside the `<build-directory>/nyan`-folder.

## About the Buildsystem

`cmake` registers the project in the [user package registry](https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#user-package-registry).
Other projects can therefore easily find `nyan` **without** installing it
(no `sudo make install` needed).

`find_package(nyan CONFIG REQUIRED)` will directly provide `nyan::nyan` as a
target to link to (with its include directories etc).
