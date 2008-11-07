/*
 * $XConsortium: GenKey.c,v 1.6 94/04/17 20:16:36 rws Exp $
 * $XFree86: xc/lib/Xdmcp/GenKey.c,v 3.1 1996/10/23 13:08:39 dawes Exp $
 *
 * 
Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 * *
 * Author:  Keith Packard, MIT X Consortium
 */

#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

static getbits (data, dst)
    long	    data;
    unsigned char   *dst;
{
    dst[0] = (data      ) & 0xff;
    dst[1] = (data >>  8) & 0xff;
    dst[2] = (data >> 16) & 0xff;
    dst[3] = (data >> 24) & 0xff;
}

/* EMX is not STDC, but sometimes it is */
#if defined(X_NOT_STDC_ENV) && !defined(__EMX__)
#define Time_t long
extern Time_t time ();
#else
#define Time_t time_t
#endif

#if defined(SYSV) || defined(SVR4)
#define srandom srand48
#define random lrand48
#endif

#ifdef linux
#include <stdlib.h>
#else
long random();
#endif

void
XdmcpGenerateKey (key)
    XdmAuthKeyPtr   key;
{
    long    lowbits, highbits;

    srandom ((int)getpid() ^ time((Time_t *)0));
    lowbits = random ();
    highbits = random ();
    getbits (lowbits, key->data);
    getbits (highbits, key->data + 4);
}
