/*
 * $XConsortium: Unwrap.c,v 1.9 94/04/17 20:16:43 keith Exp $
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

#ifdef HASXDMAUTH

/*
 * The following function exists only to demonstrate the
 * desired functional interface for this routine.  You will
 * need to add the appropriate algorithm if you wish to
 * use XDM-AUTHENTICATION-1/XDM-AUTHORIZATION-1.
 *
 * The interface for this routine is quite simple.  All three
 * arguments are arrays of 8 unsigned characters, the first two
 * are 64 bits of useful data, the last is 56 bits of useful
 * data packed into 8 bytes, using the low 7 bits of each
 * byte, filling the high bit with odd parity.
 *
 * Examine the XDMCP specification for the correct algorithm
 */

#include "Wrap.h"

void
XdmcpUnwrap (input, wrapper, output, bytes)
    unsigned char	*input, *output;
    unsigned char	*wrapper;
    int			bytes;
{
    int			i, j, k;
    unsigned char	tmp[8];
    unsigned char	blocks[2][8];
    unsigned char	expand_wrapper[8];
    auth_wrapper_schedule	schedule;

    _XdmcpWrapperToOddParity (wrapper, expand_wrapper);
    _XdmcpAuthSetup (expand_wrapper, schedule);

    k = 0;
    for (j = 0; j < bytes; j += 8)
    {
	if (bytes - j < 8)
	    return; /* bad input length */
	for (i = 0; i < 8; i++)
	    blocks[k][i] = input[j + i];
	_XdmcpAuthDoIt ((unsigned char *) (input + j), (unsigned char *) tmp, schedule, 0);
	/* block chaining */
	k = (k == 0) ? 1 : 0;
	for (i = 0; i < 8; i++)
	{
	    if (j == 0)
		output[j + i] = tmp[i];
	    else
		output[j + i] = tmp[i] ^ blocks[k][i];
	}
    }
}

#endif /* HASXDMAUTH */
