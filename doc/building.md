# Building nyan

This project requires a C++14 compiler (e.g gcc >= 5.1 or clang >= 3.4) and uses Flex, make, automake, autoconf and libtool (any "recent" version should do).

## Dependencies install

###### Ubuntu 15.04 and 15.10 

```
sudo apt-get update
sudo apt-get install make libtool autoconf automake flex bison git
```

For gcc* : `sudo apt-get install gcc g++`

For clang : `sudo apt-get install clang` 

Note: **Ubuntu 15.04** does not propose GCC5+. On 15.04 you should use clang instead.
Ubuntu 15.04 is obsolete and you should consider upgrading your system.

###### Ubuntu 14.04

```
sudo apt-get install -y software-properties-common
sudo add-apt-repository ppa:ubuntu-toolchain-r/test 
sudo apt-get install make libtool autoconf automake flex bison git
```

For gcc : `sudo apt-get install gcc-5 g++-5`

For clang : `sudo apt-get install clang`

###### Ubuntu 14.10, 13.10 and previous (really?)

Those versions of Ubuntu are **OBSOLETE**. Build dependencies are not available for those versions. 
You are on your own and should consisder upgrading your system.

###### Archlinux

```
sudo pacman -Sy
sudo pacman -S --noconfirm gcc clang make autoconf automake libtool flex git
```

## Clone this repository

```
git clone https://github.com/SFTtech/nyan.git
cd nyan
```

## Build

###### Ubuntu

For gcc on Ubuntu 15.10 : `./autogen.sh && ./configure CC=gcc CXX=g++ && make`

For gcc on Ubuntu 14.04 : `./autogen.sh && ./configure CC=gcc-5 CXX=g++-5 && make`

For clang on Ubuntu 14.04, 15.04 or 15.10 : `./autogen.sh && ./configure CC=clang CXX=clang++ && make`

###### Archlinux

For gcc : `./autogen.sh && ./configure CC=gcc CXX=g++ && make`

For clang : `./autogen.sh && ./configure CC=clang CXX=clang++ && make`

