/*
 * Copyright (C) 1998-2002 The XFree86 Project, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from the
 * XFree86 Project.
 */
/* $XFree86: xc/lib/font/FreeType/module/ftmodule.c,v 1.18 2003/11/02 04:30:57 dawes Exp $ */

#include "misc.h"

#include <X11/fonts/fontmod.h>
#include "xf86Module.h"

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

static MODULESETUPPROTO(freetypeSetup);

    /*
     * This is the module data function that is accessed when loading
     * libfreetype as a module.
     */

static XF86ModuleVersionInfo VersRec =
{
	"freetype",
	MODULEVENDORSTRING " & the After X-TT Project",
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
        2, 1, 0,
	ABI_CLASS_FONT,			/* Font module */
	ABI_FONT_VERSION,
	MOD_CLASS_FONT,
	{0,0,0,0}       /* signature, to be patched into the file by a tool */
};

XF86ModuleData freetypeModuleData = { &VersRec, freetypeSetup, NULL };

extern void FreeTypeRegisterFontFileFunctions(void);

FontModule freetypeModule = {
    FreeTypeRegisterFontFileFunctions,
    "FreeType",
    NULL
};

static pointer
freetypeSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    freetypeModule.module = module;
    LoadFont(&freetypeModule);

    /* Need a non-NULL return */
    return (pointer)1;
}
