/* $XFree86: xc/programs/Xserver/cfb24/cfbmodule.c,v 1.8 1999/01/26 05:53:49 dawes Exp $ */
/*
 * Copyright (C) 1998 The XFree86 Project, Inc.  All Rights Reserved.
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

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#define PSZ 24

#ifdef XFree86LOADER

#include "xf86Module.h"
#include "cfb.h"

static MODULESETUPPROTO(cfb24Setup);

static XF86ModuleVersionInfo VersRec =
{
	"cfb24",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	1, 0, 0,
	ABI_CLASS_ANSIC,		/* Only need the ansic layer */
	ABI_ANSIC_VERSION,
	MOD_CLASS_NONE,
	{0,0,0,0}       /* signature, to be patched into the file by a tool */
};

XF86ModuleData cfb24ModuleData = { &VersRec, cfb24Setup, NULL };

static pointer
cfb24Setup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    /* This modules requires cfb, so load it */
    return LoadSubModule(module, "cfb", NULL, NULL, NULL, NULL,
			 errmaj, errmin);
}

#endif
