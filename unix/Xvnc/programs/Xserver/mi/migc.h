/* $XConsortium: migc.h,v 1.3 94/04/17 20:27:37 dpw Exp $ */
/*

Copyright (c) 1993  X Consortium

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


extern void miRegisterGCPrivateIndex(
#if NeedFunctionPrototypes
    int /*gcindex*/
#endif
);

extern void miChangeGC(
#if NeedFunctionPrototypes
    GCPtr  /*pGC*/,
    unsigned long /*mask*/
#endif
);

extern void miDestroyGC(
#if NeedFunctionPrototypes
    GCPtr  /*pGC*/
#endif
);

extern GCOpsPtr miCreateGCOps(
#if NeedFunctionPrototypes
    GCOpsPtr /*prototype*/
#endif
);

extern void miDestroyGCOps(
#if NeedFunctionPrototypes
    GCOpsPtr /*ops*/
#endif
);

extern void miDestroyClip(
#if NeedFunctionPrototypes
    GCPtr /*pGC*/
#endif
);

extern void miChangeClip(
#if NeedFunctionPrototypes
    GCPtr   /*pGC*/,
    int     /*type*/,
    pointer /*pvalue*/,
    int     /*nrects*/
#endif
);

extern void miCopyClip(
#if NeedFunctionPrototypes
    GCPtr /*pgcDst*/,
    GCPtr /*pgcSrc*/
#endif
);

extern void miCopyGC(
#if NeedFunctionPrototypes
    GCPtr /*pGCSrc*/,
    unsigned long /*changes*/,
    GCPtr /*pGCDst*/
#endif
);

extern void miComputeCompositeClip(
#if NeedFunctionPrototypes
    GCPtr       /*pGC*/,
    DrawablePtr /*pDrawable*/
#endif
);
