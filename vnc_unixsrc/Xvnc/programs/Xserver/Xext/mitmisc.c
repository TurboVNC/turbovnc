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

/* $XConsortium: mitmisc.c,v 1.5 94/04/17 20:32:54 rws Exp $ */
/* $XFree86: xc/programs/Xserver/Xext/mitmisc.c,v 3.1 1996/05/06 05:55:29 dawes Exp $ */

#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#define _MITMISC_SERVER_
#include "mitmiscstr.h"

extern Bool permitOldBugs;

static unsigned char MITReqCode;

static void MITResetProc(
#if NeedFunctionPrototypes
    ExtensionEntry * /* extEntry */
#endif
);

static DISPATCH_PROC(ProcMITDispatch);
static DISPATCH_PROC(ProcMITGetBugMode);
static DISPATCH_PROC(ProcMITSetBugMode);
static DISPATCH_PROC(SProcMITDispatch);
static DISPATCH_PROC(SProcMITGetBugMode);
static DISPATCH_PROC(SProcMITSetBugMode);

void
MITMiscExtensionInit()
{
    ExtensionEntry *extEntry;

    if ((extEntry = AddExtension(MITMISCNAME, 0, 0,
				 ProcMITDispatch, SProcMITDispatch,
				 MITResetProc, StandardMinorOpcode)) != 0)
	MITReqCode = (unsigned char)extEntry->base;
}

/*ARGSUSED*/
static void
MITResetProc (extEntry)
ExtensionEntry	*extEntry;
{
}

static int
ProcMITSetBugMode(client)
    register ClientPtr client;
{
    REQUEST(xMITSetBugModeReq);

    REQUEST_SIZE_MATCH(xMITSetBugModeReq);
    if ((stuff->onOff != xTrue) && (stuff->onOff != xFalse))
    {
	client->errorValue = stuff->onOff;
	return BadValue;
    }
    permitOldBugs = stuff->onOff;
    return(client->noClientException);
}

static int
ProcMITGetBugMode(client)
    register ClientPtr client;
{
    xMITGetBugModeReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xMITGetBugModeReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.onOff = permitOldBugs;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof(xMITGetBugModeReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcMITDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_MITSetBugMode:
	return ProcMITSetBugMode(client);
    case X_MITGetBugMode:
	return ProcMITGetBugMode(client);
    default:
	return BadRequest;
    }
}

static int
SProcMITSetBugMode(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xMITSetBugModeReq);

    swaps(&stuff->length, n);
    return ProcMITSetBugMode(client);
}

static int
SProcMITGetBugMode(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xMITGetBugModeReq);

    swaps(&stuff->length, n);
    return ProcMITGetBugMode(client);
}

static int
SProcMITDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_MITSetBugMode:
	return SProcMITSetBugMode(client);
    case X_MITGetBugMode:
	return SProcMITGetBugMode(client);
    default:
	return BadRequest;
    }
}
