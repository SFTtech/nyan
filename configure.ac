dnl nyan configuration

AC_INIT([nyan], [0.1], [jj@stusta.net])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AM_CONDITIONAL([GCC], [$($CXX -v 2>&1 | grep 'gcc version' >/dev/null 2>&1)])
AM_CONDITIONAL([CLANG34], [$($CXX -v 2>&1 | grep 'clang version 3.4' >/dev/null 2>&1)])

AM_PROG_AR
AC_PROG_CXX
AM_PROG_LEX

AC_CONFIG_MACRO_DIRS([m4])

LT_INIT

AC_CHECK_LIB([c], [dladdr], LIBDL_LDFLAGS="",
             [AC_CHECK_LIB([dl], [dladdr], LIBDL_LDFLAGS="-ldl")])
AC_SUBST(LIBDL_LDFLAGS)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile
                 nyan/Makefile])
AC_OUTPUT
