# nyan library

# also build parser header
#AM_YFLAGS = -d
#BUILT_SOURCES = nyan_parser.h

lib_LTLIBRARIES = libnyan.la
libnyan_la_SOURCES = libnyan.cpp nyan_lexer.lpp nyan_parser.ypp
include_HEADERS = nyan.h

# nyan executable
bin_PROGRAMS = nyan
nyan_SOURCES = nyan.cpp nyan.h
nyan_LDADD = libnyan.la
