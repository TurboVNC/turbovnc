dnl Process this file with autoconf to produce a configure script.
AC_INIT(autocutsel.c)
AM_INIT_AUTOMAKE(autocutsel,0.9.0)

AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PATH_XTRA
if test "$no_x" = "yes"
then
  AC_MSG_ERROR([Unable to find X libraries])
fi
AC_CHECK_LIB(Xext, main, LIBS="$LIBS -lXext", [], $X_PRE_LIB $X_LIBS $LIBS -lX11 $X_EXTRA_LIBS)

LIBS="$X_PRE_LIBS $X_LIBS $LIBS -lX11 $X_EXTRA_LIBS"

AC_CHECK_LIB(Xaw, main, 
  LIBS="$LIBS -lXaw",
  AC_MSG_ERROR(Unable to find Xaw library),
  $LIBS)

AC_CHECK_LIB(Xt, XtDisplay,
  LIBS="$LIBS -lXt",
  AC_MSG_ERROR(Unable to find Xt library),
  $LIBS)

AC_CHECK_LIB(Xmu, XmuGetHostname,
  LIBS="$LIBS -lXmu",
  AC_MSG_ERROR(Unable to find Xmu library),
  $LIBS)

dnl Checks for libraries.

dnl Checks for header files.

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIGNAL

dnl Checks for library functions.
AC_FUNC_SETPGRP

dnl Warnings
AC_MSG_CHECKING([whether to enable -Wall])
if test -n "$GCC"; then 
  AC_MSG_RESULT(adding -Wall to CFLAGS.)
  CFLAGS="$CFLAGS -Wall"
else
  AC_MSG_RESULT(no)
fi

CFLAGS="$CFLAGS $X_CFLAGS"

AC_OUTPUT(Makefile)
