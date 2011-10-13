/* $XFree86$ */
/*
Copyright 1996, 1998  The Open Group

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

/* $Xorg: xpstubs.c,v 1.5 2001/03/08 17:52:08 pookie Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "misc.h"
#include <X11/fonts/font.h>
#ifdef XPRINT
#include "DiPrint.h"
#endif

Bool
XpClientIsBitmapClient(
    ClientPtr client)
{
    return TRUE;
}

Bool
XpClientIsPrintClient(
    ClientPtr client,
    FontPathElementPtr fpe)
{
    return FALSE;
}
#ifdef XPRINT
int
PrinterOptions(
    int argc,
    char **argv,
    int i)
{
    return i;
}
void
PrinterInitOutput(
     ScreenInfo *pScreenInfo,
     int argc,
     char **argv)
{
}
void PrinterUseMsg(void)
{
}
void PrinterInitGlobals(void)
{
}
#endif /* XPRINT */

