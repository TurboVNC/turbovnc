XCOMM!/bin/sh

XCOMM
XCOMM makedepend which uses 'gcc -M'
XCOMM
XCOMM $XFree86: xc/config/util/gccmdep.cpp,v 3.3 1996/02/25 01:16:15 dawes Exp $
XCOMM
XCOMM Based on mdepend.cpp and code supplied by Hongjiu Lu <hjl@nynexst.com>
XCOMM

TMP=/tmp/mdep$$
CC=CCCMD
RM=RMCMD
LN=LNCMD
MV=MVCMD

trap "$RM ${TMP}*; exit 1" 1 2 15
trap "$RM ${TMP}*; exit 0" 1 2 13

files=
makefile=
endmarker=
magic_string='# DO NOT DELETE'
append=n
args=
asmfiles=

while [ $# != 0 ]; do
    if [ "$endmarker"x != x -a "$endmarker" = "$1" ]; then
	endmarker=
    else
	case "$1" in
	    -D*|-I*)
		args="$args '$1'"
		;;
	    -g|-o)
		;;
	    *)
		if [ "$endmarker"x = x ]; then
		    case $1 in
XCOMM ignore these flags
			-w|-o|-cc)
			    shift
			    ;;
			-v)
			    ;;
			-s)
			    magic_string="$2"
			    shift
			    ;;
			-f-)
			    makefile="-"
			    ;;
			-f)
			    makefile="$2"
			    shift
			    ;;
			--*)
			    endmarker=`echo $1 | sed 's/^\-\-//'`
			    if [ "$endmarker"x = x ]; then
				endmarker="--"
			    fi
			    ;;
			-a)
			    append=y
			    ;;
			-*)
			    echo "Unknown option '$1' ignored" 1>&2
			    ;;
			*)
			    files="$files $1"
			    ;;
		    esac
		fi
		;;
	esac
    fi
    shift
done

if [ x"$files" = x ]; then
XCOMM Nothing to do
    exit 0
fi

case "$makefile" in
    '')
	if [ -r makefile ]; then
	    makefile=makefile
	elif [ -r Makefile ]; then
	    makefile=Makefile
	else
	    echo 'no makefile or Makefile found' 1>&2
	    exit 1
	fi
	;;
esac

if [ X"$makefile" != X- ]; then
    if [ x"$append" = xn ]; then
        sed -e "/^$magic_string/,\$d" < $makefile > $TMP
        echo "$magic_string" >> $TMP
    else
        cp $makefile $TMP
    fi
fi

XCOMM need to link .s files to .S
for i in $files; do
    case $i in
	*.s)
	    dir=`dirname $i`
	    base=`basename $i .s`
	    (cd $dir; $RM ${base}.S; $LN ${base}.s ${base}.S)
	    asmfiles="$asmfiles ${base}.S"
	    ;;
    esac
done

CMD="$CC -M $args `echo $files | sed -e 's,\.s$,\.S,g' -e 's,\.s ,\.S ,g'` | \
	sed -e 's,\.S$,\.s,g' -e 's,\.S ,\.s ,g'"
if [ X"$makefile" != X- ]; then
    CMD="$CMD >> $TMP"
fi
eval $CMD
if [ X"$makefile" != X- ]; then
    $RM ${makefile}.bak
    $MV $makefile ${makefile}.bak
    $MV $TMP $makefile
fi

if [ x"$asmfiles" != x ]; then
    $RM $asmfiles
fi
$RM ${TMP}*
exit 0
