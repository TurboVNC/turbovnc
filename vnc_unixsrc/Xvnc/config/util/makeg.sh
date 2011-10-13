#! /bin/sh
# makeg - run "make" with options necessary to make a debuggable executable
# $Xorg: makeg.sh,v 1.3 2000/08/17 19:41:52 cpqbld Exp $

# set GDB=1 in your environment if using gdb on Solaris 2.

make="${MAKE-make}"
flags="CDEBUGFLAGS=-g CXXDEBUGFLAGS=-g"

# gdb on Solaris needs the stabs included in the executable
test "${GDB+yes}" = yes && flags="$flags -xs"

exec "$make" $flags LDSTRIPFLAGS= ${1+"$@"}
