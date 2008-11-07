/* $XConsortium: fontaccel.c,v 1.4 94/04/17 20:17:31 gildea Exp $ */

/*

Copyright (c) 1990  X Consortium

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

#include    "fontmisc.h"
#include    "fontstruct.h"

FontComputeInfoAccelerators(pFontInfo)
    FontInfoPtr pFontInfo;
{
    pFontInfo->noOverlap = FALSE;
    if (pFontInfo->maxOverlap <= pFontInfo->minbounds.leftSideBearing)
	pFontInfo->noOverlap = TRUE;

    if ((pFontInfo->minbounds.ascent == pFontInfo->maxbounds.ascent) &&
	    (pFontInfo->minbounds.descent == pFontInfo->maxbounds.descent) &&
	    (pFontInfo->minbounds.leftSideBearing ==
	     pFontInfo->maxbounds.leftSideBearing) &&
	    (pFontInfo->minbounds.rightSideBearing ==
	     pFontInfo->maxbounds.rightSideBearing) &&
	    (pFontInfo->minbounds.characterWidth ==
	     pFontInfo->maxbounds.characterWidth) &&
      (pFontInfo->minbounds.attributes == pFontInfo->maxbounds.attributes)) {
	pFontInfo->constantMetrics = TRUE;
	if ((pFontInfo->maxbounds.leftSideBearing == 0) &&
		(pFontInfo->maxbounds.rightSideBearing ==
		 pFontInfo->maxbounds.characterWidth) &&
		(pFontInfo->maxbounds.ascent == pFontInfo->fontAscent) &&
		(pFontInfo->maxbounds.descent == pFontInfo->fontDescent))
	    pFontInfo->terminalFont = TRUE;
	else
	    pFontInfo->terminalFont = FALSE;
    } else {
	pFontInfo->constantMetrics = FALSE;
	pFontInfo->terminalFont = FALSE;
    }
    if (pFontInfo->minbounds.characterWidth == pFontInfo->maxbounds.characterWidth)
	pFontInfo->constantWidth = TRUE;
    else
	pFontInfo->constantWidth = FALSE;

    if ((pFontInfo->minbounds.leftSideBearing >= 0) &&
	    (pFontInfo->maxOverlap <= 0) &&
	    (pFontInfo->minbounds.ascent >= -pFontInfo->fontDescent) &&
	    (pFontInfo->maxbounds.ascent <= pFontInfo->fontAscent) &&
	    (-pFontInfo->minbounds.descent <= pFontInfo->fontAscent) &&
	    (pFontInfo->maxbounds.descent <= pFontInfo->fontDescent))
	pFontInfo->inkInside = TRUE;
    else
	pFontInfo->inkInside = FALSE;
}

FontCouldBeTerminal(pFontInfo)
    FontInfoPtr pFontInfo;
{
    if ((pFontInfo->minbounds.leftSideBearing >= 0) &&
	    (pFontInfo->maxbounds.rightSideBearing <= pFontInfo->maxbounds.characterWidth) &&
	    (pFontInfo->minbounds.characterWidth == pFontInfo->maxbounds.characterWidth) &&
	    (pFontInfo->maxbounds.ascent <= pFontInfo->fontAscent) &&
	    (pFontInfo->maxbounds.descent <= pFontInfo->fontDescent) &&
	    (pFontInfo->maxbounds.leftSideBearing != 0 ||
	     pFontInfo->minbounds.rightSideBearing != pFontInfo->minbounds.characterWidth ||
	     pFontInfo->minbounds.ascent != pFontInfo->fontAscent ||
	     pFontInfo->minbounds.descent != pFontInfo->fontDescent)) {
	/* blow off font with nothing but a SPACE */
	if (pFontInfo->maxbounds.ascent == 0 && 
	    pFontInfo->maxbounds.descent == 0)
		return FALSE;
	return TRUE;
    }
    return FALSE;
}
