XCOMM!/bin/sh
XCOMM
XCOMM $XConsortium: mergelib.cpp,v 1.4 94/04/17 20:10:43 rws Exp $
XCOMM 
XCOMM Copyright (c) 1989  X Consortium
XCOMM 
XCOMM Permission is hereby granted, free of charge, to any person obtaining a copy
XCOMM of this software and associated documentation files (the "Software"), to deal
XCOMM in the Software without restriction, including without limitation the rights
XCOMM to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
XCOMM copies of the Software, and to permit persons to whom the Software is
XCOMM furnished to do so, subject to the following conditions:
XCOMM 
XCOMM The above copyright notice and this permission notice shall be included in
XCOMM all copies or substantial portions of the Software.
XCOMM 
XCOMM THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
XCOMM IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
XCOMM FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
XCOMM X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
XCOMM AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
XCOMM CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
XCOMM 
XCOMM Except as contained in this notice, the name of the X Consortium shall not be
XCOMM used in advertising or otherwise to promote the sale, use or other dealings
XCOMM in this Software without prior written authorization from the X Consortium.
XCOMM 
XCOMM Author:  Jim Fulton, MIT X Consortium
XCOMM 
XCOMM mergelib - merge one library into another; this is commonly used by X
XCOMM     to add the extension library into the base Xlib.
XCOMM

usage="usage:  $0  to-library from-library [object-filename-prefix]"
objprefix=_

case $# in
    2) ;;
    3) objprefix=$3 ;;
    *) echo "$usage" 1>&2; exit 1 ;;
esac

tolib=$1
fromlib=$2

if [ ! -f $fromlib ]; then
    echo "$0:  no such from-library $fromlib" 1>&2
    exit 1
fi

if [ ! -f $tolib ]; then
    echo "$0:  no such to-library $tolib" 1>&2
    exit 1
fi


XCOMM
XCOMM Create a temp directory, and figure out how to reference the 
XCOMM object files from it (i.e. relative vs. absolute path names).
XCOMM

tmpdir=tmp.$$
origdir=..

mkdir $tmpdir

if [ ! -d $tmpdir ]; then
    echo "$0:  unable to create temporary directory $tmpdir" 1>&2
    exit 1
fi

case "$fromlib" in
    /?*) upfrom= ;;
    *)  upfrom=../ ;;
esac

case "$tolib" in
    /?*) upto= ;;
    *)  upto=../ ;;
esac


XCOMM
XCOMM In the temp directory, extract all of the object files and prefix
XCOMM them with some symbol to avoid name clashes with the base library.
XCOMM
cd $tmpdir
ar x ${upfrom}$fromlib
for i in *.o; do
    mv $i ${objprefix}$i
done


XCOMM
XCOMM Merge in the object modules, ranlib (if appropriate) and cleanup
XCOMM
ARCMD ${upto}$tolib *.o
RANLIB ${upto}$tolib
cd $origdir
rm -rf $tmpdir



