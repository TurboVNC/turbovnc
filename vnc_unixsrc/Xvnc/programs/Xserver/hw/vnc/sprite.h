/*
 * sprite.h
 *
 * software-sprite/sprite drawing - based on misprite
 */

/*
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

/*

Copyright (c) 1989  X Consortium

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

typedef struct {
    Bool	(*RealizeCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		CursorPtr /*pCursor*/
#endif
);
    Bool	(*UnrealizeCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		CursorPtr /*pCursor*/
#endif
);
    Bool	(*PutUpCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		CursorPtr /*pCursor*/,
		int /*x*/,
		int /*y*/,
		unsigned long /*source*/,
		unsigned long /*mask*/
#endif
);
    Bool	(*SaveUnderCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		int /*x*/,
		int /*y*/,
		int /*w*/,
		int /*h*/
#endif
);
    Bool	(*RestoreUnderCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		int /*x*/,
		int /*y*/,
		int /*w*/,
		int /*h*/
#endif
);

} rfbSpriteCursorFuncRec, *rfbSpriteCursorFuncPtr;

extern Bool rfbSpriteInitialize(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    rfbSpriteCursorFuncPtr /*cursorFuncs*/,
    miPointerScreenFuncPtr /*screenFuncs*/
#endif
);

extern void rfbSpriteRestoreCursor(
#if NeedFunctionPrototypes
    ScreenPtr	/*pScreen*/
#endif
);

extern void rfbSpriteRemoveCursor(
#if NeedFunctionPrototypes
    ScreenPtr	/*pScreen*/
#endif
);

extern CursorPtr rfbSpriteGetCursorPtr(
#if NeedFunctionPrototypes
    ScreenPtr	/*pScreen*/
#endif
);

extern void rfbSpriteGetCursorPos(
#if NeedFunctionPrototypes
    ScreenPtr	/*pScreen*/,
    int *       /*px*/,
    int *       /*py*/
#endif
);
