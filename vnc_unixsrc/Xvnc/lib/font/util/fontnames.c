/* $XConsortium: fontnames.c,v 1.2 94/04/17 20:17:32 keith Exp $ */

/*

Copyright (c) 1991  X Consortium

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
 *
 *	@(#)fontnames.c	3.1	91/04/10
 */

#include	"fontmisc.h"
#include	"fontstruct.h"

void
FreeFontNames(pFN)
    FontNamesPtr pFN;
{
    int         i;

    if (!pFN)
	return;
    for (i = 0; i < pFN->nnames; i++) {
	xfree(pFN->names[i]);
    }
    xfree(pFN->names);
    xfree(pFN->length);
    xfree(pFN);
}

FontNamesPtr
MakeFontNamesRecord(size)
    unsigned    size;
{
    FontNamesPtr pFN;

    pFN = (FontNamesPtr) xalloc(sizeof(FontNamesRec));
    if (pFN) {
	pFN->nnames = 0;
	pFN->size = size;
	if (size)
	{
	    pFN->length = (int *) xalloc(size * sizeof(int));
	    pFN->names = (char **) xalloc(size * sizeof(char *));
	    if (!pFN->length || !pFN->names) {
	    	xfree(pFN->length);
	    	xfree(pFN->names);
	    	xfree(pFN);
	    	pFN = (FontNamesPtr) 0;
	    }
	}
	else
	{
	    pFN->length = 0;
	    pFN->names = 0;
	}
    }
    return pFN;
}

int
AddFontNamesName(names, name, length)
    FontNamesPtr names;
    char       *name;
    int         length;
{
    int         index = names->nnames;
    char       *nelt;

    nelt = (char *) xalloc(length + 1);
    if (!nelt)
	return AllocError;
    if (index >= names->size) {
	int         size = names->size << 1;
	int        *nlength;
	char      **nnames;

	if (size == 0)
	    size = 8;
	nlength = (int *) xrealloc(names->length, size * sizeof(int));
	nnames = (char **) xrealloc(names->names, size * sizeof(char *));
	if (nlength && nnames) {
	    names->size = size;
	    names->length = nlength;
	    names->names = nnames;
	} else {
	    xfree(nelt);
	    xfree(nlength);
	    xfree(nnames);
	    return AllocError;
	}
    }
    names->length[index] = length;
    names->names[index] = nelt;
    strncpy(nelt, name, length);
    nelt[length] = '\0';
    names->nnames++;
    return Successful;
}
