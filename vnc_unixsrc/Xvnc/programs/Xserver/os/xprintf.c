/* 
 * printf routines which xalloc their buffer
 */ 
/*
 * Copyright (c) 2004 Alexander Gottwald
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/Xos.h>
#include "os.h"
#include <stdarg.h>
#include <stdio.h>

#ifndef va_copy
# ifdef __va_copy
#  define va_copy __va_copy
# else
#  error "no working va_copy was found"
# endif
#endif
    
char *
Xvprintf(const char *format, va_list va)
{
    char *ret;
    int size;
    va_list va2;

    va_copy(va2, va);
    size = vsnprintf(NULL, 0, format, va2);
    va_end(va2);

    ret = (char *)Xalloc(size + 1);
    if (ret == NULL)
        return NULL;

    vsnprintf(ret, size + 1, format, va);
    ret[size] = 0;
    return ret;
}

char *Xprintf(const char *format, ...)
{
    char *ret;
    va_list va;
    va_start(va, format);
    ret = Xvprintf(format, va);
    va_end(va);
    return ret;
}

char *
XNFvprintf(const char *format, va_list va)
{
    char *ret;
    int size;
    va_list va2;

    va_copy(va2, va);
    size = vsnprintf(NULL, 0, format, va2);
    va_end(va2);

    ret = (char *)XNFalloc(size + 1);
    if (ret == NULL)
        return NULL;

    vsnprintf(ret, size + 1, format, va);
    ret[size] = 0;
    return ret;
}

char *XNFprintf(const char *format, ...)
{
    char *ret;
    va_list va;
    va_start(va, format);
    ret = XNFvprintf(format, va);
    va_end(va);
    return ret;
}
