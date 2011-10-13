/* $Xorg: cbrt.c,v 1.4 2001/02/09 02:05:19 xorgcvs Exp $ */
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
/* $XFree86: xc/programs/Xserver/mi/cbrt.c,v 3.3 2001/05/29 22:24:06 dawes Exp $ */

/* simple cbrt, in case your math library doesn't have a good one */

/*
 * Would normally include <math.h> for this, but for the sake of compiler
 * warnings, we don't want to get duplicate declarations for cbrt().
 */

double pow(double, double);
double cbrt(double);

double
cbrt(double x)
{
    if (x > 0.0)
	return pow(x, 1.0/3.0);
    else
	return -pow(-x, 1.0/3.0);
}
