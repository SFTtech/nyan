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
	config.cpp
	container.cpp
	database.cpp
	datastructure/orderedset.cpp
	error.cpp
	file.cpp
	lexer.cpp
	location.cpp
	member.cpp
	namespace.cpp
	object.cpp
	ops.cpp
	parser.cpp
	ptr_container.cpp
	token.cpp
	type.cpp
	type_container.cpp
	util.cpp
	value/value.cpp
	value/container.cpp
	value/number.cpp
	value/orderedset.cpp
	value/set.cpp
	value/set_base.cpp
	value/text.cpp
)

target_link_libraries(nyan
	dl
)


set_target_properties(nyan PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
set_target_properties(nyan PROPERTIES VERSION 0)

install(TARGETS nyan DESTINATION ${CMAKE_INSTALL_LIBDIR} LIBRARY)
