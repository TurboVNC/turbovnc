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

AC_CANONICAL_HOST

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

AC_ARG_VAR(JPEG_CFLAGS, [Custom C compiler flags for libjpeg-turbo, e.g. -I{libjpeg-turbo_directory}/include])
AC_ARG_VAR(JPEG_LDFLAGS, [Custom Linker flags for libjpeg-turbo, e.g. -L{libjpeg-turbo_directory}/lib -lturbojpeg])

if test "x$USE_MEDIALIB" = "xyes"; then

  AC_CHECK_HEADERS([mlib.h], [],
    [AC_MSG_ERROR([Could not find mediaLib header files])])
  AC_CHECK_LIB(mlib, mlib_version, [],
    [AC_MSG_ERROR([Could not link with mediaLib library])])

else

  if test "x$JPEG_CFLAGS" = "x"; then
    JPEG_CFLAGS=-I/opt/libjpeg-turbo/include
  fi
  if test "x$JPEG_LDFLAGS" = "x"; then
    if test "$host_cpu" = "x86_64"; then
      JPEG_LDFLAGS='-L/opt/libjpeg-turbo/lib64 -L/opt/libjpeg-turbo/lib -Wl,-Bstatic -lturbojpeg -Wl,-Bdynamic'
    else
      JPEG_LDFLAGS='-L/opt/libjpeg-turbo/lib32 -L/opt/libjpeg-turbo/lib -Wl,-Bstatic -lturbojpeg -Wl,-Bdynamic'
    fi
    case $host_os in
      darwin*)
        JPEG_LDFLAGS=/opt/libjpeg-turbo/lib/libturbojpeg.a
        ;;
    esac
  fi

  SAVE_CPPFLAGS=${CPPFLAGS}
  CPPFLAGS="${CPPFLAGS} ${JPEG_CFLAGS}"
  AC_CHECK_HEADERS([turbojpeg.h], [DUMMY=1],
    [AC_MSG_ERROR([Could not find libjpeg-turbo header files])])
  AC_MSG_CHECKING([for libjpeg-turbo library])
  LJT_CHECK=no
  CPPFLAGS=${SAVE_CPPFLAGS}
  SAVE_LIBS=${LIBS}
  LIBS="${LIBS} ${JPEG_LDFLAGS}"
  AC_LINK_IFELSE([AC_LANG_CALL([], [tjInitCompress])], [LJT_CHECK=yes],
    [AC_MSG_ERROR([Could not link with libjpeg-turbo libraries])])
  LIBS=${SAVE_LIBS}
	AC_MSG_RESULT([$LJT_CHECK])

fi

AM_CONDITIONAL([WITH_MEDIALIB], [test "x$USE_MEDIALIB" != "xno"])

USE_DEV_URANDOM=no;
AC_MSG_CHECKING([for /dev/urandom])
if test -r /dev/urandom; then
  USE_DEV_URANDOM=yes
  AC_DEFINE(UseDevUrandom)
fi
AC_MSG_RESULT([$USE_DEV_URANDOM])

AC_PATH_XTRA

USE_MITSHM=yes
AC_ARG_WITH([shm],
  AC_HELP_STRING([--without-shm],
    [Don't use MIT-SHM extension, even if it is available]))
if test "x$with_shm" != "xno"; then
  SAVE_CPPFLAGS=${CPPFLAGS}
  CPPFLAGS="${CPPFLAGS} ${X_CFLAGS}"
  AC_CHECK_HEADERS([X11/extensions/XShm.h], [DUMMY=1],
    [AC_MSG_ERROR([Could not find MIT-SHM header files])],
    [#include <X11/Xlib.h>])
  CPPFLAGS=${SAVE_CPPFLAGS}
  AC_CHECK_LIB(Xext, XShmQueryExtension, [DUMMY=1],
    [AC_MSG_ERROR([MIT-SHM not available in Xext library])],
    [$X_LIBS -lX11])
else
  USE_MITSHM=no
fi
if test "x$USE_MITSHM" = "xyes"; then
  AC_DEFINE(MITSHM)
  X_EXTRA_LIBS="$X_EXTRA_LIBS -lXext"
fi

AM_CONDITIONAL([X86_64], [test "x$host_cpu" = "xx86_64"])

case "$host_cpu" in
  sparc*)
    PKGARCH=sparc
    ;;
  x86_64)
    RPMARCH=x86_64
    PKGARCH=amd64
    ;;
  i*86 | x86 | ia32)
    RPMARCH=i386
    PKGARCH=i386
    ;;
esac

AM_CONDITIONAL([X86], [test "x$RPMARCH" = "xi386"])
AM_CONDITIONAL([GCC], [test "x$GCC" = "xyes"])

AC_SUBST(RPMARCH)
AC_SUBST(PKGARCH)
AC_SUBST(BUILD)
AC_SUBST(abs_top_srcdir)
AC_DEFINE_UNQUOTED(__BUILD, "${BUILD}")

AC_CONFIG_FILES([vncserver], [chmod +x vncserver])
AC_CONFIG_FILES([Xvnc/config/cf/tvnc_version.h])

AC_OUTPUT([
Makefile
libvncauth/Makefile
turbojpeg/Makefile
vncconnect/Makefile
vncpasswd/Makefile
vncviewer/Makefile
])
