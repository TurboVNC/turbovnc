#!/bin/sh
#
# $XFree86: xc/config/util/mkhtmlindex.sh,v 1.3 2000/08/26 04:30:49 dawes Exp $
#
# Copyright © 2000 by Precision Insight, Inc.
#
# Generate index files for the HTML man pages
#
# Author:	David Dawes <dawes@xfree86.org>
#

VOLLIST="1 2 3 4 5 6 7 8 9 o l n p"
INDEX="manindex"

if [ $# != 1 ]; then
	echo Usage: $0 htmlmandir
	exit 1
fi

if [ ! -d $1 ]; then
	echo $1 is not a directory
	exit 1
fi

cd $1

for s in $VOLLIST; do
	list="`ls *.$s.html 2> /dev/null`" || : # ignore failed glob expansion
	if [ X"$list" != X ]; then
		file=$INDEX$s.html
		rm -f $file
		cat <<EOF > $file
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">
<HTML>
<HEAD>
<META HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=iso-8859-1">
<TITLE>X.Org Manual pages: Section $s</TITLE>
</HEAD>
<BODY BGCOLOR="#efefef" TEXT="black" LINK="blue" VLINK="#551A8B" ALINK="red">

<H1>X.Org Manual pages: Section $s</H1>
<P>
<UL>
EOF
		for i in $list; do
			title="`sed -e '/^[^0-9A-Za-z]/d' -e '/^$/' -e '/^Name/d' -e q $i`"
			name="`echo \"$title\" | sed -e 's/ - .*//'`"
			desc="`echo \"$title\" | sed -e 's/[^-]* - //' -e 's/<P>//'`"
			echo "<LI><A href=\"$i\">$name</A> - $desc</LI>" >> $file
		done
		cat <<EOF >> $file
</UL>
<P>
</BODY>
</HTML>
EOF
	fi
done

exit 0
