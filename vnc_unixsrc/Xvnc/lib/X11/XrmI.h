/* $Xorg: XrmI.h,v 1.4 2001/02/09 02:03:39 xorgcvs Exp $ */
/*

Copyright 1990, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/


/*
 * Macros to abstract out reading the file, and getting its size.
 *
 * You may need to redefine these for various other operating systems. 
 */

#include	<X11/Xos.h>
#include        <sys/stat.h>                        

#define GetSizeOfFile(fd,size)                      \
{                                                   \
    struct stat status_buffer;                      \
    if ( (fstat((fd), &status_buffer)) == -1 )      \
	size = -1;                                  \
    else                                            \
	size = status_buffer.st_size;               \
}
