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


For information about building, look at [doc/building.md].

If you encounter problems you can't fix yourself,
report problems at https://github.com/SFTtech/nyan

Contact: #sfttech:matrix.org
")


# nyan library version
set(nyan_VERSION 1.0)

set(nyan_exports_name "nyanTargets")


if("$ENV{CXX}" STREQUAL "")
	set(CMAKE_CXX_COMPILER "clang++")
else()
	set(CMAKE_CXX_COMPILER "$ENV{CXX}" CACHE STRING "compiler" FORCE)
endif()


# cmake < 3.4.0 has problems with projects that use C++ but not C,
# especially in the FindThreads module (when header files are checked).
if(CMAKE_VERSION VERSION_LESS "3.4.0")
	if(WIN32)
		# Need cmake >= 3.4.0 to enable WINDOWS_EXPORT_ALL_SYMBOLS for nyan
		message(FATAL_ERROR "Cannot build with cmake ${CMAKE_VERSION} < 3.4.0 for MSVC.")
	endif()
	message(WARNING "using cmake ${CMAKE_VERSION} < 3.4.0, falling back and setting CC=CXX.")
	project(nyan VERSION ${nyan_VERSION} LANGUAGES C CXX)

	# simple hack to set CC=CXX because
	# older cmake requires CC for some header tests
	set(CMAKE_C_COMPILER "${CMAKE_CXX_COMPILER}")
	set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")
else()
	project(nyan VERSION ${nyan_VERSION} LANGUAGES CXX)
endif()

# Ensure CMAKE_BUILD_TYPE is set correctly.
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()
string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" BUILD_TYPE_CXX_FLAGS)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb3")
endif()

# add search paths to helper modules
set(BUILDSYSTEM_DIR "${nyan_SOURCE_DIR}/buildsystem")
set(CMAKE_MODULE_PATH "${BUILDSYSTEM_DIR}")

# load helper modules
include(CMakePackageConfigHelpers)
include(CTest)
include(GenerateExportHeader)
include(doxygen)
include(GNUInstallDirs)

# documentation
doxygen_configure(nyan/ doc/ README.md)

# register sources
add_subdirectory(nyan/)

###############################################################################
# cmake package generation

# the cmake-package information files will be stored there.
set(config_package_location "${CMAKE_INSTALL_LIBDIR}/cmake/nyan")


# this file will not be installed, it's just for linking to nyan in its build tree!
export(EXPORT "${nyan_exports_name}"
	FILE "${CMAKE_CURRENT_BINARY_DIR}/${nyan_exports_name}.cmake"
	NAMESPACE nyan::
)

# this is like the export(EXPORT) above but it will generated and installed.
# generation happens at install time!
install(
	EXPORT "${nyan_exports_name}"
	FILE "${nyan_exports_name}.cmake"
	NAMESPACE nyan::
	DESTINATION "${config_package_location}"
)


# create the main find_package entry file.
configure_package_config_file("cmake/nyanConfig.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/nyanConfig.cmake"
	INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}"
	INSTALL_DESTINATION "${config_package_location}"
	PATH_VARS
	CMAKE_INSTALL_PREFIX
	CMAKE_INSTALL_INCLUDEDIR
	CMAKE_INSTALL_LIBDIR
)

# generate and install the package version config
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/nyanConfigVersion.cmake"
	VERSION "${nyan_VERSION}"
	COMPATIBILITY AnyNewerVersion
)

install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/nyanConfig.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/nyanConfigVersion.cmake"
	DESTINATION ${config_package_location}
	COMPONENT Devel
)

# register the package in cmake's registry
# this may for example be ~/.cmake/packages
message(STATUS
	"registering nyan in the cmake user package registry "
	"(~/.cmake/package/nyan)..."
)
export(PACKAGE nyan)

# that's it, the package setup is done.
###############################################################################


# show build configuration overview
message("
          project | ${PROJECT_NAME}
          version | ${nyan_VERSION}
         compiler | ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}
       build type | ${CMAKE_BUILD_TYPE}
         cxxflags | ${CMAKE_CXX_FLAGS}
 build type flags | ${${BUILD_TYPE_CXX_FLAGS}}
        build dir | ${CMAKE_BINARY_DIR}
   install prefix | ${CMAKE_INSTALL_PREFIX}
")

set_property(GLOBAL PROPERTY TARGET_MESSAGES OFF)
