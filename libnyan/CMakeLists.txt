# source file configuration
# for the resulting nyan library


find_package(FLEX REQUIRED)

set(nyanl_cpp "${CMAKE_CURRENT_SOURCE_DIR}/flex_lexer.cpp")
set(nyanl_h "${CMAKE_CURRENT_SOURCE_DIR}/flex_lexer.h")
set(nyanl_lpp "flex_lexer.lpp")

add_custom_command(OUTPUT ${nyanl_cpp} ${nyanl_h}
	COMMAND ${FLEX_EXECUTABLE} -Ca --header-file=${nyanl_h} -o ${nyanl_cpp} ${nyanl_lpp}
	VERBATIM
	DEPENDS ${nyanl_lpp}
	COMMENT "[flex] Generating scanner with flex ${FLEX_VERSION}"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

include_directories(
	${FLEX_INCLUDE_DIRS}
)

add_library(nyan SHARED
	flex_lexer.cpp
	ast.cpp
	basic_type.cpp
	c3.cpp
	config.cpp
	curve.cpp
	database.cpp
	datastructure/orderedset.cpp
	error.cpp
	file.cpp
	id_token.cpp
	lexer.cpp
	location.cpp
	member.cpp
	member_info.cpp
	meta_info.cpp
	namespace.cpp
	namespace_finder.cpp
	object.cpp
	object_info.cpp
	object_state.cpp
	ops.cpp
	parser.cpp
	patch_info.cpp
	state.cpp
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
	value/value_wrapper.cpp
	view.cpp
)

target_link_libraries(nyan
	dl
)


set_target_properties(nyan PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
set_target_properties(nyan PROPERTIES VERSION 0)

install(TARGETS nyan DESTINATION ${CMAKE_INSTALL_LIBDIR} LIBRARY)
