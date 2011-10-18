/* $XConsortium: fntfil.h,v 1.5 94/04/17 20:17:28 gildea Exp $ */

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

#ifndef _FONTFILE_H_
#define _FONTFILE_H_
typedef struct _FontEntry	    *FontEntryPtr;
typedef struct _FontTable	    *FontTablePtr;
typedef struct _FontName	    *FontNamePtr;
typedef struct _FontScaled	    *FontScaledPtr;
typedef struct _FontScalableExtra   *FontScalableExtraPtr;
typedef struct _FontScalableEntry   *FontScalableEntryPtr;
typedef struct _FontScaleAliasEntry *FontScaleAliasEntryPtr;
typedef struct _FontBitmapEntry	    *FontBitmapEntryPtr;
typedef struct _FontAliasEntry	    *FontAliasEntryPtr;
typedef struct _FontBCEntry	    *FontBCEntryPtr;
typedef struct _FontDirectory	    *FontDirectoryPtr;
typedef struct _FontRenderer	    *FontRendererPtr;

#define NullFontEntry		    ((FontEntryPtr) 0)
#define NullFontTable		    ((FontTablePtr) 0)
#define NullFontName		    ((FontNamePtr) 0)
#define NullFontScaled		    ((FontScaled) 0)
#define NullFontScalableExtra	    ((FontScalableExtra) 0)
#define NullFontscalableEntry	    ((FontScalableEntry) 0)
#define NullFontScaleAliasEntry	    ((FontScaleAliasEntry) 0)
#define NullFontBitmapEntry	    ((FontBitmapEntry) 0)
#define NullFontAliasEntry	    ((FontAliasEntry) 0)
#define NullFontBCEntry		    ((FontBCEntry) 0)
#define NullFontDirectory	    ((FontDirectoryPtr) 0)
#define NullFontRenderer	    ((FontRendererPtr) 0)

#define FONT_ENTRY_SCALABLE	0
#define FONT_ENTRY_SCALE_ALIAS	1
#define FONT_ENTRY_BITMAP	2
#define FONT_ENTRY_ALIAS	3
#define FONT_ENTRY_BC		4

#define MAXFONTNAMELEN	    1024
#define MAXFONTFILENAMELEN  1024

#define FontDirFile	    "fonts.dir"
#define FontAliasFile	    "fonts.alias"
#define FontScalableFile    "fonts.scale"

extern FontEntryPtr	FontFileFindNameInDir ();
extern FontEntryPtr	FontFileFindNameInScalableDir ();
extern FontDirectoryPtr	FontFileMakeDir ();
extern FontRendererPtr	FontFileMatchRenderer ();
extern char		*FontFileSaveString ();
extern FontScaledPtr	FontFileFindScaledInstance ();
#endif /* _FONTFILE_H_ */
