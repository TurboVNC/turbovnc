#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.56])
AC_INIT([turbovnc], [0.6.82])
BUILD=`date +%Y%m%d`

AM_INIT_AUTOMAKE([-Wall foreign dist-bzip2])
AC_PREFIX_DEFAULT(/opt/TurboVNC)

# Checks for programs.
SAVED_CFLAGS=${CFLAGS}
SAVED_CXXFLAGS=${CXXFLAGS}
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

host_cpu=`echo $host_alias | cut -d- -f1`
host_os=`echo $host_alias | cut -d- -f3`

if test "x${GCC}" = "xyes"; then
  if test "x${SAVED_CFLAGS}" = "x"; then
    CFLAGS=-O3
  fi
  if test "x${SAVED_CXXFLAGS}" = "x"; then
    CXXFLAGS=-O3
  fi
fi

AC_CHECK_DECL([__SUNPRO_C], [SUNCC="yes"], [SUNCC="no"])
if test "x${SUNCC}" = "xyes"; then
  if test "x${SAVED_CFLAGS}" = "x"; then
    CFLAGS=-xO5
  fi
  if test "x${SAVED_CXXFLAGS}" = "x"; then
    CXXFLAGS=-xO5
  fi
fi

case $host in
  sparc*-*solaris*)
    USE_MEDIALIB=yes
    ;;
  *)
    USE_MEDIALIB=no
    ;;
esac

AC_ARG_WITH([medialib],
  AC_HELP_STRING([--with-medialib],
    [Use Sun mediaLib to perform JPEG compression/decompression]),
  [USE_MEDIALIB=$with_medialib], [])

if test "x$USE_MEDIALIB" = "xyes"; then

  AC_CHECK_HEADERS([mlib.h], [],
    [AC_MSG_ERROR([Could not find mediaLib header files])])
  AC_CHECK_LIB(mlib, mlib_version, [],
    [AC_MSG_ERROR([Could not link with mediaLib library])])

else

  case $host_os in
    darwin*)
      LJTLIB=/usr/lib/libturbojpeg.a
      ;;
    solaris*)
      LJTINC=-I/opt/libjpeg-turbo/include
      if test "$host_cpu" = "x86_64"; then
        LJTLIB='-L/opt/libjpeg-turbo/lib/amd64 -Wl,-Bstatic -lturbojpeg -Wl,-Bdynamic'
      else
        LJTLIB='-L/opt/libjpeg-turbo/lib -Wl,-Bstatic -lturbojpeg -Wl,-Bdynamic'
      fi
      ;;
    *)
      LJTLIB='-Wl,-Bstatic -lturbojpeg -Wl,-Bdynamic'
      ;;
  esac

  AC_ARG_WITH([libjpeg-turbo],
    AC_HELP_STRING([--with-libjpeg-turbo=DIR],
      [Specify directory in which libjpeg-turbo is located]),
    [LJTLIB='-L$withval -lturbojpeg'; LJTINC='-I$withval'], [])

  SAVE_CPPFLAGS=${CPPFLAGS}
  CPPFLAGS="${CPPFLAGS} ${LJTINC}"
  AC_CHECK_HEADERS([turbojpeg.h], [DUMMY=1],
    [AC_MSG_ERROR([Could not find libjpeg-turbo header files])])
  CPPFLAGS=${SAVE_CPPFLAGS}
  AC_CHECK_LIB(turbojpeg, tjInitCompress, [DUMMY=1],
    [AC_MSG_ERROR([Could not link with libjpeg-turbo libraries])],
    [$LJTLIB])

fi

AM_CONDITIONAL([WITH_MEDIALIB], [test "x$USE_MEDIALIB" != "xno"])
AC_SUBST(LJTINC)
AC_SUBST(LJTLIB)

AC_PATH_XTRA

USE_MITSHM=yes
AC_ARG_WITH([shm],
  AC_HELP_STRING([--without-shm],
    [Don't use MIT-SHM extension, even if it is available]))
if test "x$with_shm" != "xno"; then
  AC_CHECK_HEADERS([X11/extensions/XShm.h], [DUMMY=1],
    [AC_MSG_ERROR([Could not find MIT-SHM header files])],
    [#include <X11/Xlib.h>])
  AC_CHECK_LIB(Xext, XShmQueryExtension, [DUMMY=1],
    [AC_MSG_ERROR([MIT-SHM not available in Xext library])],
    [$X_LIBS -lX11])
fi
if test "x$USE_MITSHM" = "xyes"; then
  AC_DEFINE(MITSHM)
  X_EXTRA_LIBS="$X_EXTRA_LIBS -lXext"
fi

AM_CONDITIONAL([X86_64], [test "x$host_cpu" = "xx86_64"])

case "$host_cpu" in
  x86_64)
    RPMARCH=x86_64
    DEBARCH=amd64
    ;;
  i*86 | x86 | ia32)
    RPMARCH=i386
    DEBARCH=i386
    ;;
esac

AC_SUBST(RPMARCH)
AC_SUBST(DEBARCH)
AC_SUBST(BUILD)

AC_OUTPUT([
Makefile
libvncauth/Makefile
turbojpeg/Makefile
vncconnect/Makefile
vncpasswd/Makefile
vncviewer/Makefile
])
