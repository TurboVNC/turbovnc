/* $XFree86: xc/programs/Xserver/GL/glx/glximports.c,v 1.5 2001/03/21 16:29:36 dawes Exp $ */
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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"
#include "glxcontext.h"
#include "glximports.h"
#include "GL/glx_ansic.h"

void *__glXImpMalloc(__GLcontext *gc, size_t size)
{
    void *addr;

    if (size == 0) {
	return NULL;
    }
    addr = xalloc(size);
    if (addr == NULL) {
	/* XXX: handle out of memory error */
	return NULL;
    }
    return addr;
}

void *__glXImpCalloc(__GLcontext *gc, size_t numElements, size_t elementSize)
{
    void *addr;
    size_t size;

    if ((numElements == 0) || (elementSize == 0)) {
	return NULL;
    }
    size = numElements * elementSize;
    addr = xalloc(size);
    if (addr == NULL) {
	/* XXX: handle out of memory error */
	return NULL;
    }
    /* zero out memory */
    __glXMemset(addr, 0, size);

    return addr;
}

void __glXImpFree(__GLcontext *gc, void *addr)
{
    if (addr) {
	xfree(addr);
    }
}

void *__glXImpRealloc(__GLcontext *gc, void *addr, size_t newSize)
{
    void *newAddr;

    if (addr) {
	if (newSize == 0) {
	    xfree(addr);
	    return NULL;
	}
	newAddr = xrealloc(addr, newSize);
    } else {
	if (newSize == 0) {
	    return NULL;
	}
	newAddr = xalloc(newSize);
    }
    if (newAddr == NULL) {
	return NULL;	/* XXX: out of memory error */
    }

    return newAddr;
}

void __glXImpWarning(__GLcontext *gc, char *msg)
{
    ErrorF((char *)msg);
}

void __glXImpFatal(__GLcontext *gc, char *msg)
{
    ErrorF((char *)msg);
    __glXAbort();
}

char *__glXImpGetenv(__GLcontext *gc, const char *var)
{
    return __glXGetenv(var);
}

int __glXImpAtoi(__GLcontext *gc, const char *str)
{
    return __glXAtoi(str);
}

int __glXImpSprintf(__GLcontext *gc, char *str, const char *fmt, ...)
{
    va_list ap;
    int ret;

    /* have to deal with var args */
    va_start(ap, fmt);
    ret = __glXVsprintf(str, fmt, ap);
    va_end(ap);

    return ret;
}

void *__glXImpFopen(__GLcontext *gc, const char *path, const char *mode)
{
    return (void *) __glXFopen(path, mode);
}

int __glXImpFclose(__GLcontext *gc, void *stream)
{
    return __glXFclose((FILE *)stream);
}

int __glXImpFprintf(__GLcontext *gc, void *stream, const char *fmt, ...)
{
    va_list ap;
    int ret;

    /* have to deal with var args */
    va_start(ap, fmt);
    ret = __glXVfprintf((FILE *)stream, fmt, ap);
    va_end(ap);

    return ret;
}


__GLdrawablePrivate *__glXImpGetDrawablePrivate(__GLcontext *gc)
{
    __GLinterface *glci = (__GLinterface *) gc;
    __GLXcontext *glrc = (__GLXcontext *) glci->imports.other;

    return &glrc->drawPriv->glPriv;
}


__GLdrawablePrivate *__glXImpGetReadablePrivate(__GLcontext *gc)
{
    __GLinterface *glci = (__GLinterface *) gc;
    __GLXcontext *glrc = (__GLXcontext *) glci->imports.other;

    return &glrc->readPriv->glPriv;
}
