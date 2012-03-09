XCOMM!/bin/sh
XCOMM
XCOMM $Xorg: mergelib.cpp,v 1.4 2001/02/09 02:03:17 xorgcvs Exp $
XCOMM 
XCOMM Copyright (c) 1989, 1998 The Open Group
XCOMM 
XCOMM Permission to use, copy, modify, distribute, and sell this software and 
XCOMM its documentation for any purpose is hereby granted without fee, provided
XCOMM that the above copyright notice appear in all copies and that both that
XCOMM copyright notice and this permission notice appear in supporting
XCOMM documentation.
XCOMM 
XCOMM The above copyright notice and this permission notice shall be included in
XCOMM all copies or substantial portions of the Software.
XCOMM 
XCOMM THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
XCOMM IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
XCOMM FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
XCOMM OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
XCOMM AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
XCOMM CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
XCOMM 
XCOMM Except as contained in this notice, the name of The Open Group shall not be
XCOMM used in advertising or otherwise to promote the sale, use or other dealings
XCOMM in this Software without prior written authorization from The Open Group.
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

XCOMM Remove directory if we fail
trap "rm -rf $tmpdir; exit 1" 1 2 15
trap "rm -rf $tmpdir; exit 0" 1 2 13

mkdir $tmpdir

XCOMM Security: if $tmpdir exists before mkdir exit immediately
if [ $? -gt 0 -o ! -d $tmpdir ]; then
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
cd $tmpdir || exit 1
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

