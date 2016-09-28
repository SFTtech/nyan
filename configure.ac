dnl nyan configuration

AC_INIT([nyan], [0.1], [jj@stusta.net])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AM_PROG_AR
AC_PROG_CXX
AM_PROG_LEX

AC_CONFIG_MACRO_DIRS([m4])

# use c++14
AC_SUBST([AM_CXXFLAGS], [-std=c++14])

LT_INIT

# find libdl for dladdr
AC_CHECK_LIB([c], [dladdr], LIBDL_LDFLAGS="",
             [AC_CHECK_LIB([dl], [dladdr], LIBDL_LDFLAGS="-ldl")])
AC_SUBST(LIBDL_LDFLAGS)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
                 nyan/Makefile])
AC_OUTPUT
