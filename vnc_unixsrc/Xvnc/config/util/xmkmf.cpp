XCOMM!/bin/sh

XCOMM
XCOMM make a Makefile from an Imakefile from inside or outside the sources
XCOMM 
XCOMM $XConsortium: xmkmf.cpp /main/22 1996/09/28 16:17:05 rws $

usage="usage:  $0 [-a] [top_of_sources_pathname [current_directory]]"

configdirspec=CONFIGDIRSPEC
topdir=
curdir=.
do_all=

case "$1" in
-a)
    do_all="yes"
    shift
    ;;
esac

case $# in 
    0) ;;
    1) topdir=$1 ;;
    2) topdir=$1  curdir=$2 ;;
    *) echo "$usage" 1>&2; exit 1 ;;
esac

case "$topdir" in
    -*) echo "$usage" 1>&2; exit 1 ;;
esac

if [ -f Makefile ]; then 
    echo mv -f Makefile Makefile.bak
    mv -f Makefile Makefile.bak
fi

if [ "$topdir" = "" ]; then
    args="-DUseInstalled "$configdirspec
else
    args="-I$topdir/config/cf -DTOPDIR=$topdir -DCURDIR=$curdir"
fi

echo imake $args
case "$do_all" in
yes)
    imake $args && 
    echo "make Makefiles" &&
    make Makefiles &&
    echo "make includes" &&
    make includes &&
    echo "make depend" &&
    make depend
    ;;
*)
    imake $args
    ;;
esac
