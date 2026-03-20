AC_INIT([vntools], [0.1])
AM_INIT_AUTOMAKE([subdir-objects foreign])
AM_PROG_AR
AC_PROG_CC
AC_PROG_RANLIB
AC_OPENMP

AC_CONFIG_HEADERS([config.h])

AC_CHECK_FUNCS([strcasecmp], [], [], [
#include <strings.h>
])

AM_CONDITIONAL([NEED_STRCASECMP], [test "x$ac_cv_func_strcasecmp" != xyes])

AC_CONFIG_FILES([Makefile lib/Makefile])
AC_OUTPUT
