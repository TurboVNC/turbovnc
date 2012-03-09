XCOMM!/bin/sh

XCOMM $XFree86: xc/config/util/xmkmf.cpp,v 1.4 2001/01/17 16:39:02 dawes Exp $
XCOMM
XCOMM make a Makefile from an Imakefile from inside or outside the sources
XCOMM 
XCOMM $Xorg: xmkmf.cpp,v 1.3 2000/08/17 19:41:53 cpqbld Exp $

usage="usage:  $0 [-a] [top_of_sources_pathname [current_directory]]"

configdirspec=CONFIGDIRSPEC
topdir=
curdir=.
do_all=
imake_defines=

while [ $# -gt 0 ]
do
    case "$1" in
    -D*)
       	imake_defines="$imake_defines $1"
	shift
	;;
    -a)
	do_all="yes"
	shift
	;;
    *)
    	break
	;;
    esac
done

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

echo imake $imake_defines $args
case "$do_all" in
yes)
    imake $imake_defines $args && 
    echo "make Makefiles" &&
    make Makefiles &&
    echo "make includes" &&
    make includes &&
    echo "make depend" &&
    make depend
    ;;
*)
    imake $imake_defines $args
    ;;
esac
