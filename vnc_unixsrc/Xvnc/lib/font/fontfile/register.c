/* $Xorg: register.c,v 1.4 2001/02/09 02:04:03 xorgcvs Exp $ */

/*

Copyright 1994, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
/* $XFree86: xc/lib/font/fontfile/register.c,v 1.15 2001/12/14 19:56:52 dawes Exp $ */

/*
 * This is in a separate source file so that small programs
 * such as mkfontdir that want to use the fontfile utilities don't
 * end up dragging in code from all the renderers, which is not small.
 */

#include "fontmisc.h"
#include "fntfilst.h"
#include "bitmap.h"

#ifdef LOADABLEFONTS
#include "fontmod.h"
#endif

void
FontFileRegisterFpeFunctions(void)
{
#ifndef LOADABLEFONTS
    BitmapRegisterFontFileFunctions ();

#ifndef LOWMEMFTPT

#ifndef CRAY
#ifdef BUILD_SPEEDO
    SpeedoRegisterFontFileFunctions ();
#endif
#ifdef BUILD_TYPE1
    Type1RegisterFontFileFunctions();
#endif
#endif
#ifdef BUILD_CID
    CIDRegisterFontFileFunctions();
#endif
#ifdef BUILD_FREETYPE
    FreeTypeRegisterFontFileFunctions();
#endif
#ifdef BUILD_XTRUETYPE
    XTrueTypeRegisterFontFileFunctions();
#endif

#endif /* ifndef LOWMEMFTPT */

#else
    {
	int i;

	if (FontModuleList) {
	    for (i = 0; FontModuleList[i].name; i++) {
		if (FontModuleList[i].initFunc)
		    FontModuleList[i].initFunc();
	    }
	}
    }
#endif

    FontFileRegisterLocalFpeFunctions ();
}

