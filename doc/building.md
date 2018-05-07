# Building nyan

This project requires a C++17 compiler (e.g gcc >= 7 or clang >= 4) and
uses `flex` and `cmake`.


## Dependencies

###### Ubuntu 16.04 or later

```
sudo apt-get update
sudo apt-get install cmake flex make
```

For gcc: `sudo apt-get install gcc g++`

For clang: `sudo apt-get install clang`


###### Archlinux

```
sudo pacman -Syu --as-needed cmake flex make clang gcc
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

`cmake` registers the project in the [user package registry](https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#user-package-registry).
Other projects can therefore easily find `nyan` **without** installing it
(no `sudo make install` needed).

`find_package(nyan CONFIG REQUIRED)` will directly provide `nyan::nyan` as a
target to link to (with its include directories etc).
