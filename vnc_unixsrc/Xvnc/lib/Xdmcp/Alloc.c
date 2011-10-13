/*
 * $Xorg: Alloc.c,v 1.4 2001/02/09 02:03:48 xorgcvs Exp $
 *
 * 
Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 * *
 * Author:  Keith Packard, MIT X Consortium
 */

/* $XFree86: xc/lib/Xdmcp/Alloc.c,v 3.4 2001/07/25 15:04:50 dawes Exp $ */

/* stubs for use when Xalloc, Xrealloc and Xfree are not defined */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>
#include <stdlib.h>

void *
Xalloc (unsigned long amount)
{
    if (amount == 0)
	amount = 1;
    return malloc (amount);
}

void *
Xrealloc (void *old, unsigned long amount)
{
    if (amount == 0)
	amount = 1;
    if (!old)
	return malloc (amount);
    return realloc ((char *) old, amount);
}

void
Xfree (void *old)
{
    if (old)
	free ((char *) old);
}
