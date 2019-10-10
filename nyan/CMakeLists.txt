# source file configuration
# for the resulting nyan library

if (APPLE)
	execute_process(COMMAND brew --prefix flex
	                OUTPUT_VARIABLE BREW_FLEX_HOME
	                OUTPUT_STRIP_TRAILING_WHITESPACE)
	if (EXISTS "${BREW_FLEX_HOME}/bin/flex")
		message(STATUS "Found homebrew flex, using it instead.")
		set(FLEX_EXECUTABLE "${BREW_FLEX_HOME}/bin/flex")
	endif ()
endif ()

find_package(FLEX 2.6 REQUIRED)

set(nyanl_cpp "${CMAKE_CURRENT_BINARY_DIR}/flex.gen.cpp")
set(nyanl_h "${CMAKE_CURRENT_BINARY_DIR}/flex.gen.h")
set(nyanl_lpp "${CMAKE_CURRENT_SOURCE_DIR}/lexer/flex.lpp")

add_custom_command(OUTPUT ${nyanl_cpp} ${nyanl_h}
	COMMAND ${FLEX_EXECUTABLE} -Ca --header-file=${nyanl_h} -o ${nyanl_cpp} ${nyanl_lpp}
	VERBATIM
	DEPENDS ${nyanl_lpp}
	COMMENT "[flex] Generating scanner with flex ${FLEX_VERSION}"
)

add_library(nyan SHARED
	api_error.cpp
	ast.cpp
	basic_type.cpp
	c3.cpp
	change_tracker.cpp
	config.cpp
	curve.cpp
	database.cpp
	datastructure/orderedset.cpp
	error.cpp
	file.cpp
	id_token.cpp
	inheritance_change.cpp
	${nyanl_cpp}
	lang_error.cpp
	lexer/bracket.cpp
	lexer/impl.cpp
	lexer/lexer.cpp
	location.cpp
	member.cpp
	member_info.cpp
	meta_info.cpp
	namespace.cpp
	namespace_finder.cpp
	object.cpp
	object_history.cpp
	object_info.cpp
	object_notifier.cpp
	object_notifier_types.cpp
	object_state.cpp
	ops.cpp
	parser.cpp
	patch_info.cpp
	state.cpp
	state_history.cpp
	token.cpp
	token_stream.cpp
	transaction.cpp
	type.cpp
	util.cpp
	value/boolean.cpp
	value/container.cpp
	value/file.cpp
	value/number.cpp
	value/object.cpp
	value/orderedset.cpp
	value/set.cpp
	value/set_base.cpp
	value/set_types.cpp
	value/text.cpp
	value/value.cpp
	value/value_holder.cpp
	view.cpp
)
add_library(nyan::nyan ALIAS nyan)

if(UNIX)
	if("${CMAKE_SYSTEM_NAME}" MATCHES "^(Free|Net|Open)BSD|DragonFly")
		find_library(EXECINFO_LIBRARY execinfo)
		target_link_libraries(nyan ${CMAKE_DL_LIBS} ${EXECINFO_LIBRARY})
	else()
		target_link_libraries(nyan ${CMAKE_DL_LIBS})
	endif()

	if(NOT APPLE)
		# for ld on macOS: "The default is to treat undefined symbols as errors."
		set_target_properties(nyan PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
	endif()
endif()

if(WIN32)
	set_target_properties(nyan PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
	target_link_libraries(nyan PRIVATE DbgHelp)
endif()

set_target_properties(nyan PROPERTIES
	VERSION ${nyan_VERSION}
	SOVERSION 1
	INTERFACE_nyan_MAJOR_VERSION 1
	COMPATIBLE_INTERFACE_STRING nyan_MAJOR_VERSION
)

# C++ standard requirement
target_compile_features(nyan
	PUBLIC
		cxx_std_17
)

# binaries
install(
	TARGETS nyan
	EXPORT ${nyan_exports_name}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# install headers
install(DIRECTORY
	./
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/nyan"
	COMPONENT Devel
	FILES_MATCHING PATTERN "*.h"
)


# include directory specification
# depending if nyan was built or installed,
# projects using libnyan will automatically include the correct dir.
target_include_directories(nyan
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/lexer
		${CMAKE_CURRENT_BINARY_DIR}
)


# the nyan tool
add_executable(nyancat
	nyan_tool.cpp
)
target_link_libraries(nyancat nyan)
install(
	TARGETS nyancat
	EXPORT ${nyan_exports_name}
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
