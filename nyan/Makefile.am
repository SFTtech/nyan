# build libnyan and the nyan tool

# c++14 plus all warnings
AM_CXXFLAGS = -std=c++14 -Wall -Wextra -pedantic

# flex flags
AM_LFLAGS = -Ca

# nyan library
lib_LTLIBRARIES = libnyan.la
libnyan_la_LDFLAGS = -no-undefined
libnyan_la_SOURCES = \
                     nyan_ast.cpp \
                     nyan_error.cpp \
                     nyan_flex_lexer.lpp \
                     nyan_lexer.cpp \
                     nyan_namespace.cpp \
                     nyan_object.cpp \
                     nyan_ops.cpp \
                     nyan_parser.cpp \
                     nyan_store.cpp \
                     nyan_token.cpp \
                     nyan_util.cpp

nyanincludedir = $(includedir)/nyan
nyaninclude_HEADERS = \
                      nyan.h \
                      nyan_object.h \
                      nyan_namespace.h \
                      nyan_store.h \
                      nyan_ops.h \
                      nyan_ast.h \
                      nyan_token.h \
                      nyan_util.h

# nyan executable
bin_PROGRAMS = nyan
nyan_SOURCES = nyan_tool.cpp
nyan_LDADD = libnyan.la
