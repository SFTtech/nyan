# Building nyan

This project requires a C++26 compiler (g++ >= 16, clang++ >= 21 or
Visual Studio 2022 >= 17.4) and uses `flex`, `cmake` and `ninja`.

You can use the `justfile` to launch common build steps.


## Dependencies

###### Ubuntu 26.04 or later

```
sudo apt-get update
sudo apt-get install cmake flex ninja-build
```

For gcc: `sudo apt-get install gcc g++`

For clang: `sudo apt-get install clang`


###### Archlinux

```
sudo pacman -Syu --needed cmake flex ninja clang gcc
```

###### Gentoo

```
sudo emerge -avt cmake flex ninja
```

###### Windows

  - [CMake](https://cmake.org/download/)
  - [Visual Studio 2022 Community edition](https://www.visualstudio.com/downloads/)
  - [flex](https://sourceforge.net/projects/winflexbison/)
    - The path to win_flex.exe needs to be added to the PATH environment variable
  
## Clone this repository

```
git clone https://github.com/SFTtech/nyan.git
```

## Build

The common build steps are driven by the `justfile`:

```
cd nyan
just build   # configure with ninja and build
just test    # run tests
```

which boils down to the standard [cmake project](https://cmake.org/cmake/help/latest/guide/tutorial/index.html) dance:

```
cd nyan
cmake -B build -G Ninja
cmake --build build
```

_Note:_ if nyan can't find flex add `-DFLEX_EXECUTABLE=path/to/win_flex.exe` to the cmake configure command.


`cmake` registers the project in the [user package registry](https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#user-package-registry).
Other projects can therefore easily find `nyan` **without** installing it
(no `sudo make install` needed).

`find_package(nyan CONFIG REQUIRED)` will directly provide `nyan::nyan` as a
target to link to (with its include directories etc).
