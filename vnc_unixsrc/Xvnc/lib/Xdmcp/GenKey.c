/*
 * $Xorg: GenKey.c,v 1.4 2001/02/09 02:03:48 xorgcvs Exp $
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

/* $XFree86: xc/lib/Xdmcp/GenKey.c,v 3.7 2001/07/25 15:04:50 dawes Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

static void
getbits (long data, unsigned char *dst)
{
    dst[0] = (data      ) & 0xff;
    dst[1] = (data >>  8) & 0xff;
    dst[2] = (data >> 16) & 0xff;
    dst[3] = (data >> 24) & 0xff;
}

#define Time_t time_t

#include <stdlib.h>

#if defined(SYSV) || defined(SVR4)
#define srandom srand48
#define random lrand48
#endif
#ifdef WIN32
#include <process.h>
#define srandom srand
#define random rand
#define getpid(x) _getpid(x)
#endif

void
XdmcpGenerateKey (XdmAuthKeyPtr key)
{
    long    lowbits, highbits;

    srandom ((int)getpid() ^ time((Time_t *)0));
    lowbits = random ();
    highbits = random ();
    getbits (lowbits, key->data);
    getbits (highbits, key->data + 4);
}
