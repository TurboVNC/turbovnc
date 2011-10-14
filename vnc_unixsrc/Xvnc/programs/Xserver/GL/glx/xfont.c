/* $XFree86$ */
/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
** 
** http://oss.sgi.com/projects/FreeB
** 
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
** 
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
** 
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
**
*/

#define NEED_REPLIES
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"
#include "glxutil.h"
#include "g_disptab.h"
#include <unpack.h>
#include <GL/gl.h>
#include <pixmapstr.h>
#include <windowstr.h>
#include <dixfontstr.h>

extern XID clientErrorValue;	/* imported kludge from dix layer */

/*
** Make a single GL bitmap from a single X glyph
*/
static int __glXMakeBitmapFromGlyph(FontPtr font, CharInfoPtr pci)
{
    int i, j;
    int widthPadded;	/* width of glyph in bytes, as padded by X */
    int allocBytes;	/* bytes to allocate to store bitmap */
    int w;		/* width of glyph in bits */
    int h;		/* height of glyph */
    register unsigned char *pglyph;
    register unsigned char *p;
    unsigned char *allocbuf;
#define __GL_CHAR_BUF_SIZE 2048
    unsigned char buf[__GL_CHAR_BUF_SIZE];

    w = GLYPHWIDTHPIXELS(pci);
    h = GLYPHHEIGHTPIXELS(pci);
    widthPadded = GLYPHWIDTHBYTESPADDED(pci);

    /*
    ** Use the local buf if possible, otherwise malloc.
    */
    allocBytes = widthPadded * h;
    if (allocBytes <= __GL_CHAR_BUF_SIZE) {
	p = buf;
	allocbuf = 0;
    } else {
	p = (unsigned char *) __glXMalloc(allocBytes);
	if (!p)
	    return BadAlloc;
	allocbuf = p;
    }

    /*
    ** We have to reverse the picture, top to bottom
    */

    pglyph = FONTGLYPHBITS(FONTGLYPHS(font), pci) + (h-1)*widthPadded;
    for (j=0; j < h; j++) {
	for (i=0; i < widthPadded; i++) {
	    p[i] = pglyph[i];
	}
	pglyph -= widthPadded;
	p += widthPadded;
    }
    glBitmap(w, h, -pci->metrics.leftSideBearing, pci->metrics.descent,
	     pci->metrics.characterWidth, 0, allocbuf ? allocbuf : buf);

    if (allocbuf) {
	__glXFree(allocbuf);
    }
    return Success;
#undef __GL_CHAR_BUF_SIZE
}

/*
** Create a GL bitmap for each character in the X font.  The bitmap is stored
** in a display list.
*/

static int 
MakeBitmapsFromFont(FontPtr pFont, int first, int count, int list_base)
{
    unsigned long   i, nglyphs;
    CARD8	    chs[2];		/* the font index we are going after */
    CharInfoPtr	    pci;
    int rv;				/* return value */
    int encoding = (FONTLASTROW(pFont) == 0) ? Linear16Bit : TwoD16Bit;
    
    glPixelStorei(GL_UNPACK_SWAP_BYTES, FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST, BITMAP_BIT_ORDER == LSBFirst);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, GLYPHPADBYTES);
    for (i=0; i < count; i++) {
	chs[0] = (first + i) >> 8;	/* high byte is first byte */
	chs[1] = first + i;

	(*pFont->get_glyphs)(pFont, 1, chs, (FontEncoding)encoding, 
		&nglyphs, &pci);

	/*
	** Define a display list containing just a glBitmap() call.
	*/
	glNewList(list_base + i, GL_COMPILE);
	if (nglyphs ) {
	    rv = __glXMakeBitmapFromGlyph(pFont, pci);
	    if (rv) {
		return rv;
	    }
	}
	glEndList();
    }
    return Success;
}

/************************************************************************/

int __glXUseXFont(__GLXclientState *cl, GLbyte *pc)
{
    ClientPtr client = cl->client;
    xGLXUseXFontReq *req;
    FontPtr pFont;
    GC *pGC;
    GLuint currentListIndex;
    __GLXcontext *cx;
    int error;

    req = (xGLXUseXFontReq *) pc;
    cx = __glXForceCurrent(cl, req->contextTag, &error);
    if (!cx) {
	return error;
    }

    glGetIntegerv(GL_LIST_INDEX, (GLint*) &currentListIndex);
    if (currentListIndex != 0) {
	/*
	** A display list is currently being made.  It is an error
	** to try to make a font during another lists construction.
	*/
	client->errorValue = cx->id;
	return __glXBadContextState;
    }

    /*
    ** Font can actually be either the ID of a font or the ID of a GC
    ** containing a font.
    */
    pFont = (FontPtr)LookupIDByType(req->font, RT_FONT);
    if (!pFont) {
        pGC = (GC *)LookupIDByType(req->font, RT_GC);
        if (!pGC) {
	    client->errorValue = req->font;
            return BadFont;
	}
	pFont = pGC->font;
    }

    return MakeBitmapsFromFont(pFont, req->first, req->count,
				    req->listBase);
}
