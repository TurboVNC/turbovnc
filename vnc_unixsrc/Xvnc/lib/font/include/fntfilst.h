/* $XConsortium: fntfilst.h,v 1.8 94/04/17 20:17:29 gildea Exp $ */
/* $XFree86: xc/lib/font/include/fntfilst.h,v 3.0 1995/11/16 11:03:45 dawes Exp $ */

/*

Copyright (c) 1991  X Consortium

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

*/

/*
 * Author:  Keith Packard, MIT X Consortium
 */

#ifndef _FONTFILEST_H_
#define _FONTFILEST_H_

#include <X11/Xos.h>
#include "fontmisc.h"
#include "fontstruct.h"
#include "fntfil.h"
#include "fontxlfd.h"

typedef struct _FontName {
    char	*name;
    short	length;
    short	ndashes;
} FontNameRec;

typedef struct _FontScaled {
    FontScalableRec	vals;
    FontEntryPtr	bitmap;
    FontPtr		pFont;
} FontScaledRec;

typedef struct _FontScalableExtra {
    FontScalableRec	defaults;
    int			numScaled;
    int			sizeScaled;
    FontScaledPtr	scaled;
    pointer		private;
} FontScalableExtraRec;

typedef struct _FontScalableEntry {
    FontRendererPtr	    renderer;
    char		    *fileName;
    FontScalableExtraPtr   extra;
} FontScalableEntryRec;

/*
 * This "can't" work yet - the returned alias string must be permanent,
 * but this layer would need to generate the appropriate name from the
 * resolved scalable + the XLFD values passed in.  XXX
 */

typedef struct _FontScaleAliasEntry {
    char		*resolved;
} FontScaleAliasEntryRec;

typedef struct _FontBitmapEntry {
    FontRendererPtr	renderer;
    char		*fileName;
    FontPtr		pFont;
} FontBitmapEntryRec;

typedef struct _FontAliasEntry {
    char	*resolved;
} FontAliasEntryRec;

typedef struct _FontBCEntry {
    FontScalableRec	    vals;
    FontEntryPtr	    entry;
} FontBCEntryRec;

typedef struct _FontEntry {
    FontNameRec	name;
    int		type;
    union _FontEntryParts {
	FontScalableEntryRec	scalable;
	FontBitmapEntryRec	bitmap;
	FontAliasEntryRec	alias;
	FontBCEntryRec		bc;
    } u;
} FontEntryRec;

typedef struct _FontTable {
    int		    used;
    int		    size;
    FontEntryPtr    entries;
    Bool	    sorted;
} FontTableRec;

typedef struct _FontDirectory {
    char	    *directory;
    unsigned long   dir_mtime;
    unsigned long   alias_mtime;
    FontTableRec    scalable;
    FontTableRec    nonScalable;
    char	    *attributes;
} FontDirectoryRec;

/* Capability bits: for definition of capabilities bitmap in the
   FontRendererRec to indicate support of XLFD enhancements */

#define CAP_MATRIX		0x1
#define CAP_CHARSUBSETTING	0x2

typedef struct _FontRenderer {
    char    *fileSuffix;
    int	    fileSuffixLen;
    int	    (*OpenBitmap)(/* fpe, pFont, flags, entry, fileName, format, fmask */);
    int	    (*OpenScalable)(/* fpe, pFont, flags, entry, fileName, vals, format, fmask */);
    int	    (*GetInfoBitmap)(/* fpe, pFontInfo, entry, fileName */);
    int	    (*GetInfoScalable)(/* fpe, pFontInfo, entry, fileName, vals */);
    int	    number;
    int     capabilities;	/* Bitmap components defined above */
} FontRendererRec;

typedef struct _FontRenders {
    int		    number;
    FontRendererPtr *renderers;
} FontRenderersRec, *FontRenderersPtr;

typedef struct _BitmapInstance {
    FontScalableRec	vals;
    FontBitmapEntryPtr	bitmap;
} BitmapInstanceRec, *BitmapInstancePtr;

typedef struct _BitmapScalablePrivate {
    int			numInstances;
    BitmapInstancePtr	instances;
} BitmapScalablePrivateRec, *BitmapScalablePrivatePtr;

typedef struct _BitmapSources {
    FontPathElementPtr	*fpe;
    int			size;
    int			count;
} BitmapSourcesRec, *BitmapSourcesPtr;

extern BitmapSourcesRec	FontFileBitmapSources;

/* Defines for FontFileFindNamesInScalableDir() behavior */
#define NORMAL_ALIAS_BEHAVIOR		0
#define LIST_ALIASES_AND_TARGET_NAMES   (1<<0)
#define IGNORE_SCALABLE_ALIASES		(1<<1)

#endif /* _FONTFILEST_H_ */
