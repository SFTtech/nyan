# Copyright 2017-2017 the nyan authors. See copying.md for legal info.

# main nyan build configuration file

cmake_minimum_required(VERSION 3.1.0)
# required for CMAKE_CXX_STANDARD

message("

       ▄▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▄
       █░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░█
       █░▒▒▒▒▒▒▒▒▒▒▄▀▀▄▒▒▒░░█▄▀▀▄
  ▄▄   █░▒▒▒▒▒▒▒▒▒▒█▓▓▓▀▄▄▄▄▀▓▓▓█      ▄ ▄▄   ▄   ▄   ▄▄▄   ▄ ▄▄
█▓▓█▄▄█░▒▒▒▒▒▒▒▒▒▄▀▓▓▓▓▓▓▓▓▓▓▓▓▀▄      █▀  █  ▀▄ ▄▀  ▀   █  █▀  █
 ▀▄▄▓▓█░▒▒▒▒▒▒▒▒▒█▓▓▓▄█▓▓▓▄▓▄█▓▓█      █   █   █▄█   ▄▀▀▀█  █   █
     ▀▀█░▒▒▒▒▒▒▒▒▒█▓▒▒▓▄▓▓▄▓▓▄▓▒▒█     █   █   ▀█    ▀▄▄▀█  █   █
      ▄█░░▒▒▒▒▒▒▒▒▒▀▄▓▓▀▀▀▀▀▀▀▓▄▀              ▄▀
    ▄▀▓▀█▄▄▄▄▄▄▄▄▄▄▄▄██████▀█▀▀               ▀▀
    █▄▄▀ █▄▄▀       █▄▄▀ ▀▄▄█


For information about building like dependencies, look at [doc/building.md].

If you encounter problems you can't fix yourself,
report problems at https://github.com/SFTtech/nyan

Contact: #sfttech:matrix.org
")


if("$ENV{CXX}" STREQUAL "")
	set(CMAKE_CXX_COMPILER "clang++")
else()
	set(CMAKE_CXX_COMPILER "$ENV{CXX}" CACHE STRING "compiler" FORCE)
endif()


# cmake < 3.4.0 has problems with projects that use C++ but not C,
# especially in the FindThreads module (when header files are checked).
if(CMAKE_VERSION VERSION_LESS "3.4.0")
	message(WARNING "using cmake ${CMAKE_VERSION} < 3.4.0, falling back and setting CC=CXX.")
	project(nyan C CXX)

	# simple hack to set CC=CXX because
	# older cmake requires CC for some header tests
	set(CMAKE_C_COMPILER "${CMAKE_CXX_COMPILER}")
	set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")
else()
	project(nyan CXX)
endif()

# Ensure CMAKE_BUILD_TYPE is set correctly.
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()
string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" BUILD_TYPE_CXX_FLAGS)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb3")

# add search paths to helper modules
set(BUILDSYSTEM_DIR "${CMAKE_SOURCE_DIR}/buildsystem")
set(CMAKE_MODULE_PATH "${BUILDSYSTEM_DIR}" "${BUILDSYSTEM_DIR}/modules/")

# load helper modules
include(CTest)
include(doxygen)
include(GNUInstallDirs)

# documentation
doxygen_configure(libnyan/ nyan/ doc/ README.md)

# register sources
add_subdirectory(nyan/)
add_subdirectory(libnyan/)

# show build configuration overview
message("
          project | ${PROJECT_NAME}
         compiler | ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}
       build type | ${CMAKE_BUILD_TYPE}
         cxxflags | ${CMAKE_CXX_FLAGS}
 build type flags | ${${BUILD_TYPE_CXX_FLAGS}}
        build dir | ${CMAKE_BINARY_DIR}
   install prefix | ${CMAKE_INSTALL_PREFIX}
")

set_property(GLOBAL PROPERTY TARGET_MESSAGES OFF)
