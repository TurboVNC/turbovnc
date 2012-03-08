/* $XConsortium: xcmiscstr.h,v 1.4 94/04/17 20:11:28 dpw Exp $ */
/*

Copyright (c) 1993, 1994  X Consortium

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

#define X_XCMiscGetVersion	0
#define X_XCMiscGetXIDRange	1
#define X_XCMiscGetXIDList	2

#define XCMiscNumberEvents	0

#define XCMiscNumberErrors	0

#define XCMiscMajorVersion	1
#define XCMiscMinorVersion	1

#define XCMiscExtensionName	"XC-MISC"

typedef struct {
    CARD8	reqType;	/* always XCMiscCode */
    CARD8	miscReqType;	/* always X_XCMiscGetVersion */
    CARD16	length B16;
    CARD16	majorVersion B16;
    CARD16	minorVersion B16;
} xXCMiscGetVersionReq;
#define sz_xXCMiscGetVersionReq 8

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	pad0;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	majorVersion B16;
    CARD16	minorVersion B16;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xXCMiscGetVersionReply;
#define sz_xXCMiscGetVersionReply 32

typedef struct {
    CARD8	reqType;	/* always XCMiscCode */
    CARD8	miscReqType;	/* always X_XCMiscGetXIDRange */
    CARD16	length B16;
} xXCMiscGetXIDRangeReq;
#define sz_xXCMiscGetXIDRangeReq 4

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	pad0;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	start_id B32;
    CARD32	count B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
} xXCMiscGetXIDRangeReply;
#define sz_xXCMiscGetXIDRangeReply 32

typedef struct {
    CARD8	reqType;	/* always XCMiscCode */
    CARD8	miscReqType;	/* always X_XCMiscGetXIDList */
    CARD16	length B16;
    CARD32	count B32;	/* number of IDs requested */
} xXCMiscGetXIDListReq;
#define sz_xXCMiscGetXIDListReq 8

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	pad0;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	count B32;	/* number of IDs requested */
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xXCMiscGetXIDListReply;
#define sz_xXCMiscGetXIDListReply 32

