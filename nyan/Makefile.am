# build libnyan and the nyan tool

#all-am: print_build_config

# all warnings n stuff
AM_CXXFLAGS = -Wall -Wextra -pedantic -ftemplate-backtrace-limit=0

# flex flags
AM_LFLAGS = -Ca

# nyan library
lib_LTLIBRARIES = libnyan.la
libnyan_la_LDFLAGS = -no-undefined
libnyan_la_LIBADD = @LIBDL_LDFLAGS@
libnyan_la_SOURCES = \
                     nyan_ast.cpp \
                     nyan_container.cpp \
                     nyan_error.cpp \
                     nyan_file.cpp \
                     nyan_flex_lexer.lpp \
                     nyan_lexer.cpp \
                     nyan_location.cpp \
                     nyan_member.cpp \
                     nyan_namespace.cpp \
                     nyan_object.cpp \
                     nyan_ops.cpp \
                     nyan_orderedset.cpp \
                     nyan_parser.cpp \
                     nyan_ptr_container.cpp \
                     nyan_store.cpp \
                     nyan_token.cpp \
                     nyan_type.cpp \
                     nyan_type_container.cpp \
                     nyan_util.cpp \
                     nyan_value.cpp \
                     nyan_value_container.cpp \
                     nyan_value_number.cpp \
                     nyan_value_orderedset.cpp \
                     nyan_value_set.cpp \
                     nyan_value_set_base.cpp \
                     nyan_value_text.cpp


nyanincludedir = $(includedir)/nyan
nyaninclude_HEADERS = \
                      nyan.h \
                      nyan_ast.h \
                      nyan_container.h \
                      nyan_error.h \
                      nyan_file.h \
                      nyan_flex_lexer.h \
                      nyan_lexer.h \
                      nyan_location.h \
                      nyan_member.h \
                      nyan_namespace.h \
                      nyan_object.h \
                      nyan_ops.h \
                      nyan_orderedset.h \
                      nyan_parser.h \
                      nyan_ptr_container.h \
                      nyan_store.h \
                      nyan_token.h \
                      nyan_type.h \
                      nyan_type_container.h \
                      nyan_util.h \
                      nyan_value.h \
                      nyan_value_container.h \
                      nyan_value_number.h \
                      nyan_value_orderedset.h \
                      nyan_value_set.h \
                      nyan_value_set_base.h \
                      nyan_value_text.h


# nyan executable
bin_PROGRAMS = nyan
nyan_SOURCES = nyan_tool.cpp
nyan_LDFLAGS = -rdynamic
nyan_LDADD = libnyan.la
