#! /bin/sh
# makeg - run "make" with options necessary to make a debuggable executable
# $XConsortium: makeg.sh /main/4 1996/06/13 11:45:08 ray $

# set GDB=1 in your environment if using gdb on Solaris 2.

make="${MAKE-make}"
flags="CDEBUGFLAGS=-g CXXDEBUGFLAGS=-g"

# gdb on Solaris needs the stabs included in the executable
test "${GDB+yes}" = yes && flags="$flags -xs"

exec "$make" $flags LDSTRIPFLAGS= ${1+"$@"}
