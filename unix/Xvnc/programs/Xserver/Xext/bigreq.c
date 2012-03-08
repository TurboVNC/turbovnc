/* $XConsortium: bigreq.c /main/5 1996/08/01 19:22:48 dpw $ */
/* $XFree86: xc/programs/Xserver/Xext/bigreq.c,v 3.2 1996/12/23 06:28:58 dawes Exp $ */
/*

Copyright (c) 1992  X Consortium

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

#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "bigreqstr.h"

static unsigned char XBigReqCode;

static void BigReqResetProc(
#if NeedFunctionPrototypes
    ExtensionEntry * /* extEntry */
#endif
);

static DISPATCH_PROC(ProcBigReqDispatch);

void
BigReqExtensionInit()
{
    ExtensionEntry *extEntry;

    if ((extEntry = AddExtension(XBigReqExtensionName, 0, 0,
				 ProcBigReqDispatch, ProcBigReqDispatch,
				 BigReqResetProc, StandardMinorOpcode)) != 0)
	XBigReqCode = (unsigned char)extEntry->base;
    DeclareExtensionSecurity(XBigReqExtensionName, TRUE);
}

/*ARGSUSED*/
static void
BigReqResetProc (extEntry)
    ExtensionEntry	*extEntry;
{
}

static int
ProcBigReqDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xBigReqEnableReq);
    xBigReqEnableReply rep;
    register int n;

    if (client->swapped) {
	swaps(&stuff->length, n);
    }
    if (stuff->brReqType != X_BigReqEnable)
	return BadRequest;
    REQUEST_SIZE_MATCH(xBigReqEnableReq);
    client->big_requests = TRUE;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.max_request_size = MAX_BIG_REQUEST_SIZE;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swapl(&rep.max_request_size, n);
    }
    WriteToClient(client, sizeof(xBigReqEnableReply), (char *)&rep);
    return(client->noClientException);
}
