/* $XConsortium: fontxlfd.h,v 1.6 94/04/17 20:17:30 gildea Exp $ */

/*

Copyright (c) 1990, 1994  X Consortium

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

/*
 * Author:  Keith Packard, MIT X Consortium
 */

#ifndef _FONTXLFD_H_
#define _FONTXLFD_H_

#include "FSproto.h"

/* Constants for values_supplied bitmap */

#define SIZE_SPECIFY_MASK		0xf

#define PIXELSIZE_MASK			0x3
#define PIXELSIZE_UNDEFINED		0
#define PIXELSIZE_SCALAR		0x1
#define PIXELSIZE_ARRAY			0x2
#define PIXELSIZE_SCALAR_NORMALIZED	0x3	/* Adjusted for resolution */

#define POINTSIZE_MASK			0xc
#define POINTSIZE_UNDEFINED		0
#define POINTSIZE_SCALAR		0x4
#define POINTSIZE_ARRAY			0x8

#define PIXELSIZE_WILDCARD		0x10
#define POINTSIZE_WILDCARD		0x20

#define ENHANCEMENT_SPECIFY_MASK	0x40

#define CHARSUBSET_SPECIFIED		0x40

#define EPS		1.0e-20
#define XLFD_NDIGITS	3		/* Round numbers in pixel and
					   point arrays to this many
					   digits for repeatability */
double xlfd_round_double();

typedef struct _FontScalable {
    int		values_supplied;	/* Bitmap identifying what advanced
					   capabilities or enhancements
					   were specified in the font name */
    double	pixel_matrix[4];
    double	point_matrix[4];

    /* Pixel and point fields are deprecated in favor of the
       transformation matrices.  They are provided and filled in for the
       benefit of rasterizers that do not handle the matrices.  */

    int		pixel,
		point;

    int         x,
                y,
                width;
    char	*xlfdName;
    int		nranges;
    fsRange	*ranges;
}           FontScalableRec, *FontScalablePtr;

extern Bool FontParseXLFDName();
extern fsRange *FontParseRanges();

#define FONT_XLFD_REPLACE_NONE	0
#define FONT_XLFD_REPLACE_STAR	1
#define FONT_XLFD_REPLACE_ZERO	2
#define FONT_XLFD_REPLACE_VALUE	3

#endif				/* _FONTXLFD_H_ */
