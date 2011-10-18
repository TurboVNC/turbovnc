/*
Copyright (c) 1996  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and sell copies of the Software, and to
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
/* $XConsortium: Xagsrv.h /main/7 1996/11/22 14:48:37 kaleb $ */

#ifndef _XAGSRV_H_
#define _XAGSRV_H_

extern void XagExtensionInit(
#if NeedFunctionPrototypes
    void
#endif
);

extern void XagConnectionInfo(
#if NeedFunctionPrototypes
    ClientPtr			/* client */,
    xConnSetupPrefix**		/* conn_prefix */,
    char**			/* conn_info */,
    int*			/* num_screens */
#endif
);

extern VisualID XagRootVisual(
#if NeedFunctionPrototypes
    ClientPtr			/* client */
#endif
);

extern Colormap XagDefaultColormap(
#if NeedFunctionPrototypes
    ClientPtr			/* client */
#endif
);

extern ClientPtr XagLeader(
#if NeedFunctionPrototypes
    ClientPtr			/* client */
#endif
);

extern void XagCallClientStateChange(
#if NeedFunctionPrototypes
    ClientPtr			/* client */
#endif
);

extern Bool XagIsControlledRoot (
#if NeedFunctionPrototypes
    ClientPtr			/* client */,
    WindowPtr			/* pParent */
#endif
);

extern XID XagId (
#if NeedFunctionPrototypes
    ClientPtr			/* client */
#endif
);

extern void XagGetDeltaInfo (
#if NeedFunctionPrototypes
    ClientPtr			/* client */,
    CARD32*			/* buf */
#endif
);

#endif /* _XAGSRV_H_ */

