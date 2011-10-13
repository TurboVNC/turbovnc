/* $Xorg: sperr.c,v 1.4 2001/02/09 02:04:00 xorgcvs Exp $ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of M.I.T., Network Computing Devices,
 * or Digital not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. Network Computing Devices, or Digital
 * make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*

Copyright 1994, 1998  The Open Group

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
/* $XFree86: xc/lib/font/Speedo/sperr.c,v 1.6 2001/12/14 19:56:41 dawes Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include	"spint.h"

#ifndef FONTMODULE
#include <stdarg.h>
#endif

extern void ErrorF(const char* f, ...);

void
SpeedoErr(char *str, ...)
{
    va_list v;
    int a1;

    va_start(v, str);
    ErrorF("Speedo: ");
    a1 = va_arg(v, int);
    ErrorF(str, a1);
    va_end(v);
}

/*
 * Called by Speedo character generator to report an error.
 *
 *  Since character data not available is one of those errors
 *  that happens many times, don't report it to user
 */
void
sp_report_error(fix15 n)
{
    switch (n) {
    case 1:
	SpeedoErr("Insufficient font data loaded\n");
	break;
    case 3:
	SpeedoErr("Transformation matrix out of range\n");
	break;
    case 4:
	SpeedoErr("Font format error\n");
	break;
    case 5:
	SpeedoErr("Requested specs not compatible with output module\n");
	break;
    case 7:
	SpeedoErr("Intelligent transformation requested but not supported\n");
	break;
    case 8:
	SpeedoErr("Unsupported output mode requested\n");
	break;
    case 9:
	SpeedoErr("Extended font loaded but only compact fonts supported\n");
	break;
    case 10:
	SpeedoErr("Font specs not set prior to use of font\n");
	break;
    case 12:
	break;
    case 13:
	SpeedoErr("Track kerning data not available()\n");
	break;
    case 14:
	SpeedoErr("Pair kerning data not available()\n");
	break;
    default:
	SpeedoErr("report_error(%d)\n", n);
	break;
    }
}
