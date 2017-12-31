# source file configuration
# for the resulting nyan library


find_package(FLEX REQUIRED)

set(nyanl_cpp "${CMAKE_CURRENT_SOURCE_DIR}/lexer/flex.gen.cpp")
set(nyanl_h "${CMAKE_CURRENT_SOURCE_DIR}/lexer/flex.gen.h")
set(nyanl_lpp "${CMAKE_CURRENT_SOURCE_DIR}/lexer/flex.lpp")

add_custom_command(OUTPUT ${nyanl_cpp} ${nyanl_h}
	COMMAND ${FLEX_EXECUTABLE} -Ca --header-file=${nyanl_h} -o ${nyanl_cpp} ${nyanl_lpp}
	VERBATIM
	DEPENDS ${nyanl_lpp}
	COMMENT "[flex] Generating scanner with flex ${FLEX_VERSION}"
)

add_library(nyan SHARED
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
	value/container.cpp
	value/file.cpp
	value/number.cpp
	value/object.cpp
	value/orderedset.cpp
	value/set.cpp
	value/set_base.cpp
	value/text.cpp
	value/value.cpp
	value/value_holder.cpp
	view.cpp
)
add_library(nyan::nyan ALIAS nyan)

if(UNIX)
	target_link_libraries(nyan
		dl
	)

	set_target_properties(nyan PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
endif()
if(WIN32)
	set_target_properties(nyan PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set_target_properties(nyan PROPERTIES
	VERSION ${nyan_VERSION}
	SOVERSION 1
	INTERFACE_nyan_MAJOR_VERSION 1
	COMPATIBLE_INTERFACE_STRING nyan_MAJOR_VERSION
)

# binaries
install(
	TARGETS nyan
	EXPORT ${nyan_exports_name}
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
		${FLEX_INCLUDE_DIRS}
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
