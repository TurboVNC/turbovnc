#!/bin/sh
# $Xorg: aminstall.sh,v 1.3 2000/08/17 19:41:52 cpqbld Exp $
# aminstall - transfer cross-compiled files to system running Amoeba.
# Usage: aminstall binary-directory [-s stksz] unix-source amoeba-dest

#
# Default soap mask for files
#
SPMASK=0xFF:2:2
export SPMASK
PROG=$0
USAGE="Usage: $PROG binary-directory unix-source amoeba-dest"

#
# Argument check
#
case $# in
3|5)	;;
*)	echo $USAGE >&2
	exit 1
	;;
esac

bindir=$1
stksz=
case $2 in
-s)	if [ $# -ne 5 ]
	then
	    echo $USAGE >&2
	    exit 1
	fi
	stksz="-s $3"
	shift
	shift
	;;
esac

unxfile=$2
dest=$3

#
# Change /public .... into /super (just for installation)
#
stripped=`echo $dest | sed 's:^/public::'`
if [ X$dest != X$stripped ]; then
    dest=/super$stripped
fi

#
# If the file already exists, then delete it
#
INFO=`$bindir/std_info $dest 2>&1`
case $INFO in
*"not found"*)	;;
*failed*)	;;
*bytes*)	$bindir/del -f $dest
		;;
/??????)	echo $PROG: cannot install over directory 1>&2
		exit
		;;
*)		$bindir/del -d $dest
		;;
esac

#
# Transfer the file to Amoeba
#
$bindir/../bin.scripts/ainstall $stksz $unxfile $dest > /dev/null 2>&1
if [ $? -ne 0 ]
then
	echo "This is not binary - using tob"
	$bindir/tob $unxfile $dest
fi

