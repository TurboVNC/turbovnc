/* $XConsortium: bdfint.h,v 1.3 94/04/17 20:17:09 rws Exp $ */

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

#ifndef BDFINT_H
#define BDFINT_H

#define bdfIsPrefix(buf,str)	(!strncmp((char *)buf,str,strlen(str)))
#define	bdfStrEqual(s1,s2)	(!strcmp(s1,s2))

#define	BDF_GENPROPS	6
#define NullProperty	((FontPropPtr)0)

/*
 * This structure holds some properties we need to generate if they aren't
 * specified in the BDF file and some other values read from the file
 * that we'll need to calculate them.  We need to keep track of whether
 * or not we've read them.
 */
typedef struct BDFSTAT {
    int         linenum;
    char       *fileName;
    char        fontName[MAXFONTNAMELEN];
    float       pointSize;
    int         resolution_x;
    int         resolution_y;
    int         digitCount;
    int         digitWidths;
    int         exHeight;

    FontPropPtr fontProp;
    FontPropPtr pointSizeProp;
    FontPropPtr resolutionXProp;
    FontPropPtr resolutionYProp;
    FontPropPtr resolutionProp;
    FontPropPtr xHeightProp;
    FontPropPtr weightProp;
    FontPropPtr quadWidthProp;
    BOOL        haveFontAscent;
    BOOL        haveFontDescent;
    BOOL        haveDefaultCh;
}           bdfFileState;

extern unsigned char *bdfGetLine();

extern void bdfError();
extern void bdfWarning();
extern Atom bdfForceMakeAtom();
extern Atom bdfGetPropertyValue();
extern unsigned char bdfHexByte();

#endif				/* BDFINT_H */
