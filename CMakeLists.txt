# Copyright 2017-2021 the nyan authors. See copying.md for legal info.

# main nyan build configuration file

cmake_minimum_required(VERSION 3.8.0)
# required for CMAKE_CXX_STANDARD 17

message("

       ▄▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▄
      █░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░░░█
      █░▒▒▒▒▒▒▒▒▒▒▄▀▀▄▒▒▒░░█▄▀▀▄
 ▄▄   █░▒▒▒▒▒▒▒▒▒▒█▓▓▓▀▄▄▄▄▀▓▓▓█      ▄ ▄▄   ▄   ▄   ▄▄▄   ▄ ▄▄
█▓▓█▄▄█░▒▒▒▒▒▒▒▒▒▄▀▓▓▓▓▓▓▓▓▓▓▓▓▀▄     █▀  █  ▀▄ ▄▀  ▀   █  █▀  █
 ▀▄▄▓▓█░▒▒▒▒▒▒▒▒▒█▓▓▓▄█▓▓▓▄▓▄█▓▓█     █   █   █▄█   ▄▀▀▀█  █   █
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
file(STRINGS "nyan_version" version)
list(GET version 0 ver)
set(nyan_VERSION ${ver})

project(nyan VERSION ${nyan_VERSION} LANGUAGES CXX)

set(nyan_exports_name "nyanTargets")

# C++ standard requirement
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMake policies
foreach(pol
        CMP0067  # honor language standard in try_compile()
       )
	if (POLICY ${pol})
		cmake_policy(SET ${pol} NEW)
	endif()
endforeach()

# Ensure CMAKE_BUILD_TYPE is set correctly.
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()
string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" BUILD_TYPE_CXX_FLAGS)

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb3")
endif()

# enhanced ide support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add search paths to helper modules
set(BUILDSYSTEM_DIR "${nyan_SOURCE_DIR}/buildsystem")
set(CMAKE_MODULE_PATH "${BUILDSYSTEM_DIR}")

# load helper modules
include(CheckCompilerFeatures)
include(CMakePackageConfigHelpers)
include(CTest)
include(GenerateExportHeader)
include(doxygen)
include(GNUInstallDirs)

# allow to disable the documentation generation.
# set the variable to off if you encounter a target name conflict.
if(NOT DEFINED DOXYGEN_ENABLE)
	option(
		DOXYGEN_ENABLE
		"whether to create a `doc` target that invokes doxygen"
		ON
	)
endif()
if(DOXYGEN_ENABLE)
	doxygen_configure(nyan/ doc/ README.md)
endif()

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
configure_package_config_file("buildsystem/templates/nyanConfig.cmake.in"
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

# Copy nyan version file to build dir
file(COPY
    "nyan_version"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/nyan/"
)


# allow to disable the userpackage registration
# set the variable to off if you include nyan as a submodule!
if(NOT DEFINED REGISTER_USERPACKAGE)
	option(
		REGISTER_USERPACKAGE
		"whether to register nyan in the cmake user package registry"
		ON
	)
endif()
if(REGISTER_USERPACKAGE)
	# register the package in cmake's registry
	# this may for example be ~/.cmake/packages
	message(STATUS
		"registering nyan in the cmake user package registry "
		"(~/.cmake/package/nyan)..."
	)
	export(PACKAGE nyan)
endif()

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
