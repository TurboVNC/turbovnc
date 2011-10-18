/************************************************************

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

********************************************************/

/* RANDOM CRUFT! THIS HAS NO OFFICIAL X CONSORTIUM BLESSING */

/* $XConsortium: mitmiscstr.h,v 1.4 94/04/17 20:11:20 keith Exp $ */

#include "MITMisc.h"

#define MITMISCNAME "MIT-SUNDRY-NONSTANDARD"

typedef struct _SetBugMode {
    CARD8	reqType;	/* always MITReqCode */
    CARD8	mitReqType;	/* always X_MITSetBugMode */
    CARD16	length B16;
    BOOL	onOff;
    BYTE	pad0;
    CARD16	pad1;
} xMITSetBugModeReq;
#define sz_xMITSetBugModeReq	8

typedef struct _GetBugMode {
    CARD8	reqType;	/* always MITReqCode */
    CARD8	mitReqType;	/* always X_MITGetBugMode */
    CARD16	length B16;
} xMITGetBugModeReq;
#define sz_xMITGetBugModeReq	4

typedef struct {
    BYTE	type;			/* X_Reply */
    BOOL	onOff;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xMITGetBugModeReply;
#define sz_xMITGetBugModeReply	32

