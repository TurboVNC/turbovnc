/* $XConsortium: format.c,v 1.5 94/04/17 20:17:34 dpw Exp $ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices or Digital 
 * not be used in advertising or publicity pertaining to distribution of the 
 * software without specific, written prior permission. Network Computing 
 * Devices and Digital make no representations about the suitability of 
 * this software for any purpose.  It is provided "as is" without express 
 * or implied warranty.
 *
 * NETWORK COMPUTING DEVICES AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES OR DIGITAL BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

#include	"FSproto.h"
#include	"font.h"

int
CheckFSFormat(format, fmask, bit_order, byte_order, scan, glyph, image)
    fsBitmapFormat format;
    fsBitmapFormatMask fmask;
    int        *bit_order,
               *byte_order,
               *scan,
               *glyph,
               *image;
{
    /* convert format to what the low levels want */
    if (fmask & BitmapFormatMaskBit) {
	*bit_order = format & BitmapFormatBitOrderMask;
	*bit_order = (*bit_order == BitmapFormatBitOrderMSB)
	    	     ? MSBFirst : LSBFirst;
    }
    if (fmask & BitmapFormatMaskByte) {
	*byte_order = format & BitmapFormatByteOrderMask;
	*byte_order = (*byte_order == BitmapFormatByteOrderMSB)
	    	      ? MSBFirst : LSBFirst;
    }
    if (fmask & BitmapFormatMaskScanLineUnit) {
	*scan = format & BitmapFormatScanlineUnitMask;
	/* convert byte paddings into byte counts */
	switch (*scan) {
	case BitmapFormatScanlineUnit8:
	    *scan = 1;
	    break;
	case BitmapFormatScanlineUnit16:
	    *scan = 2;
	    break;
	case BitmapFormatScanlineUnit32:
	    *scan = 4;
	    break;
	default:
	    return BadFontFormat;
	}
    }
    if (fmask & BitmapFormatMaskScanLinePad) {
	*glyph = format & BitmapFormatScanlinePadMask;
	/* convert byte paddings into byte counts */
	switch (*glyph) {
	case BitmapFormatScanlinePad8:
	    *glyph = 1;
	    break;
	case BitmapFormatScanlinePad16:
	    *glyph = 2;
	    break;
	case BitmapFormatScanlinePad32:
	    *glyph = 4;
	    break;
	default:
	    return BadFontFormat;
	}
    }
    if (fmask & BitmapFormatMaskImageRectangle) {
	*image = format & BitmapFormatImageRectMask;

	if (*image != BitmapFormatImageRectMin &&
		*image != BitmapFormatImageRectMaxWidth &&
		*image != BitmapFormatImageRectMax)
	    return BadFontFormat;
    }
    return Successful;
}
