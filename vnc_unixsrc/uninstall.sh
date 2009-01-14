# Copyright (C)2009 Sun Microsystems, Inc.
#
# This library is free software and may be redistributed and/or modified under
# the terms of the wxWindows Library License, Version 3.1 or (at your option)
# any later version.  The full license is in the LICENSE.txt file included
# with this distribution.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# wxWindows Library License for more details.

#!/bin/sh

error()
{
	echo $1
	exit -1
}

if [ ! "`id -u`" = "0" ]; then
	error "This command must be run as root"
fi

RCPT=/Library/Receipts/TurboVNC.pkg

if [ ! -d $RCPT ]; then error "Could not find package receipt"; fi

echo Removing files ...
lsbom -s -f -l $RCPT/Contents/Archive.bom >/dev/null || error "Could not list package contents"
RETCODE=0
PWD=`pwd`
cd /
lsbom -s -f -l $RCPT/Contents/Archive.bom | while read line; do
	rm "$line" 2>&1 || RETCODE=-1
done
cd $PWD

echo Removing directories ...
rmdir /opt/TurboVNC/bin 2>&1 || RETCODE=-1
rmdir /opt/TurboVNC/lib 2>&1 || RETCODE=-1
rmdir /opt/TurboVNC/man/man1 2>&1 || RETCODE=-1
rmdir /opt/TurboVNC/man 2>&1 || RETCODE=-1
rmdir /opt/TurboVNC 2>&1 || RETCODE=-1
rmdir /Library/Documentation/TurboVNC 2>&1 || RETCODE=-1
rmdir /Applications/TurboVNC 2>&1 || RETCODE=-1

echo Removing package receipt $RCPT ...
rm -r $RCPT 2>&1 || error "Could not remove package receipt"

for RCPT in /Library/Receipts/TurboVNC-*.pkg; do
	if [ -d $RCPT ]; then
		echo Attempting to remove old package `basename $RCPT` ...
		lsbom -s -f -l $RCPT/Contents/Archive.bom 2>/dev/null 1>/dev/null || continue
		PWD=`pwd`
		cd /
		lsbom -s -f -l $RCPT/Contents/Archive.bom | while read line; do
			rm "$line" 2>/dev/null 1>/dev/null
		done
		rmdir /Library/Documentation/`basename $RCPT .pkg` 2>/dev/null 1>/dev/null
		cd $PWD
		rm -r $RCPT 2>/dev/null 1>/dev/null
	fi
done

exit $RETCODE
