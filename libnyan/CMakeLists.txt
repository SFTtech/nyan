# source file configuration
# for the resulting nyan library


find_package(FLEX REQUIRED)

set(nyanl_cpp "${CMAKE_CURRENT_SOURCE_DIR}/nyan_flex_lexer.cpp")
set(nyanl_h "${CMAKE_CURRENT_SOURCE_DIR}/nyan_flex_lexer.h")

add_custom_command(OUTPUT ${nyanl_cpp} ${nyanl_h}
	COMMAND ${FLEX_EXECUTABLE} -Ca --header-file=${nyanl_h} -o ${nyanl_cpp} nyan_flex_lexer.lpp
	VERBATIM
	DEPENDS nyan_flex_lexer.lpp
	COMMENT "[flex] Generating scanner with flex ${FLEX_VERSION}"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

include_directories(
	${FLEX_INCLUDE_DIRS}
)

add_library(nyan SHARED
	nyan_flex_lexer.cpp
	nyan_ast.cpp
	nyan_container.cpp
	nyan_database.cpp
	nyan_error.cpp
	nyan_file.cpp
	nyan_lexer.cpp
	nyan_location.cpp
	nyan_member.cpp
	nyan_namespace.cpp
	nyan_object.cpp
	nyan_ops.cpp
	nyan_orderedset.cpp
	nyan_parser.cpp
	nyan_ptr_container.cpp
	nyan_token.cpp
	nyan_type.cpp
	nyan_type_container.cpp
	nyan_util.cpp
	nyan_value.cpp
	nyan_value_container.cpp
	nyan_value_number.cpp
	nyan_value_orderedset.cpp
	nyan_value_set.cpp
	nyan_value_set_base.cpp
	nyan_value_text.cpp
)

target_link_libraries(nyan
	dl
)


set_target_properties(nyan PROPERTIES LINK_FLAGS "-Wl,--no-undefined")
set_target_properties(nyan PROPERTIES VERSION 0)

install(TARGETS nyan DESTINATION ${CMAKE_INSTALL_LIBDIR} LIBRARY)
