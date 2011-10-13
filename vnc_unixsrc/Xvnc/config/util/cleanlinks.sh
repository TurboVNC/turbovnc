#!/bin/sh
#
# Copyright © 2000, 2003 by The XFree86 Project, Inc
# 
# Remove dangling symlinks and empty directories from a shadow link tree
# (created with lndir).
#
# Author: David Dawes <dawes@xfree86.org>
#
# $XFree86: xc/config/util/cleanlinks.sh,v 1.2 2003/04/15 03:05:16 dawes Exp $

find . -type l -print |
(
	read i
	while [ X"$i" != X ]; do
		if [ ! -f "$i" ]; then
			echo $i is a dangling symlink, removing
			rm -f "$i"
		fi
		read i
	done
)

echo Removing empty directories ...
#find . -type d -depth -print | xargs rmdir > /dev/null 2>&1
find . -type d -depth -empty -print -exec rmdir {} \;
exit 0
