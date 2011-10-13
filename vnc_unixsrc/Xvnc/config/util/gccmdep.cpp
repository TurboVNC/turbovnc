XCOMM!/bin/sh

XCOMM
XCOMM makedepend which uses 'gcc -M'
XCOMM
XCOMM $XFree86: xc/config/util/gccmdep.cpp,v 3.10tsi Exp $
XCOMM
XCOMM Based on mdepend.cpp and code supplied by Hongjiu Lu <hjl@nynexst.com>
XCOMM

TMP=mdep$$.tmp
CC=CCCMD
RM=RMCMD
LN=LNCMD
MV=MVCMD

${RM} ${TMP}

trap "${RM} ${TMP}*; exit 1" 1 2 15
trap "${RM} ${TMP}*; exit 0" 1 2 13

files=
makefile=
endmarker=
magic_string='# DO NOT DELETE'
append=n
args=

while [ $# != 0 ]; do
    if [ "$endmarker"x != x -a "$endmarker" = "$1" ]; then
	endmarker=
    else
	case "$1" in
	    -D*|-I*|-U*)
		args="$args '$1'"
		;;
	    -g*|-O*)
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
			-f*)
			    if [ "$1" = "-f-" ]; then
				makefile="-"
			    elif [ "$1" = "-f" ]; then
				makefile="$2"
				shift
			    else
				echo "$1" | sed 's/^\-f//' >${TMP}arg
				makefile="`cat ${TMP}arg`"
				rm -f ${TMP}arg
			    fi
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

CMD="$CC -M $args $files"
if [ X"$makefile" != X- ]; then
    CMD="$CMD >> $TMP"
fi
eval $CMD
if [ X"$makefile" != X- ]; then
    $RM ${makefile}.bak
    $MV $makefile ${makefile}.bak
    $MV $TMP $makefile
fi

$RM ${TMP}*
exit 0
