# Building nyan

This project requires a C++23 compiler (e.g gcc >= 11 or clang >= 13) and
uses `flex` and `cmake`.


## Dependencies

###### Ubuntu 22.04 or later

```
sudo apt-get update
sudo apt-get install cmake flex make
```

For gcc: `sudo apt-get install gcc g++`

For clang: `sudo apt-get install clang`


###### Archlinux

```
sudo pacman -Syu --needed cmake flex make clang gcc
```

###### Gentoo

```
sudo emerge -avt cmake flex make
```

###### Windows

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

_Note:_ on windows the last command is
```powershell
cmake --build . --config RelWithDebInfo
```

_Note:_ if nyan can't find flex add `-DFLEX_EXECUTABLE=path/to/win_flex.exe` to the cmake configure command.


`cmake` registers the project in the [user package registry](https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#user-package-registry).
Other projects can therefore easily find `nyan` **without** installing it
(no `sudo make install` needed).

`find_package(nyan CONFIG REQUIRED)` will directly provide `nyan::nyan` as a
target to link to (with its include directories etc).
