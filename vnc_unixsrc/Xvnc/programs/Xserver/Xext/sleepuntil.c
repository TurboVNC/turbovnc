/*
 * $XConsortium: sleepuntil.c,v 1.5 94/04/17 20:32:57 dpw Exp $
 * $XFree86: xc/programs/Xserver/Xext/sleepuntil.c,v 3.0 1996/05/06 05:55:34 dawes Exp $
 *
Copyright (c) 1992  X Consortium

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
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/* dixsleep.c - implement millisecond timeouts for X clients */

#include "X.h"
#include "Xmd.h"
#include "misc.h"
#include "windowstr.h"
#include "dixstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

typedef struct _Sertafied {
    struct _Sertafied	*next;
    TimeStamp		revive;
    ClientPtr		pClient;
    XID			id;
    void		(*notifyFunc)(
#if NeedNestedPrototypes
			ClientPtr /* client */,
			pointer /* closure */
#endif
			);

    pointer		closure;
} SertafiedRec, *SertafiedPtr;

static SertafiedPtr pPending;
static RESTYPE	    SertafiedResType;
static Bool	    BlockHandlerRegistered;
static int	    SertafiedGeneration;

static void	    ClientAwaken(
#if NeedFunctionPrototypes
    ClientPtr /* client */,
    pointer /* closure */
#endif
);
static int	    SertafiedDelete(
#if NeedFunctionPrototypes
    pointer /* value */,
    XID /* id */
#endif
);
static void	    SertafiedBlockHandler(
#if NeedFunctionPrototypes
    pointer /* data */,
    OSTimePtr /* wt */,
    pointer /* LastSelectMask */
#endif
);
static void	    SertafiedWakeupHandler(
#if NeedFunctionPrototypes
    pointer /* data */,
    int /* i */,
    pointer /* LastSelectMask */
#endif
);

int
ClientSleepUntil (client, revive, notifyFunc, closure)
    ClientPtr	client;
    TimeStamp	*revive;
    void	(*notifyFunc)();
    pointer	closure;
{
    SertafiedPtr	pRequest, pReq, pPrev;

    if (SertafiedGeneration != serverGeneration)
    {
	SertafiedResType = CreateNewResourceType (SertafiedDelete);
	if (!SertafiedResType)
	    return FALSE;
	SertafiedGeneration = serverGeneration;
	BlockHandlerRegistered = FALSE;
    }
    pRequest = (SertafiedPtr) xalloc (sizeof (SertafiedRec));
    if (!pRequest)
	return FALSE;
    pRequest->pClient = client;
    pRequest->revive = *revive;
    pRequest->id = FakeClientID (client->index);
    pRequest->closure = closure;
    if (!BlockHandlerRegistered)
    {
	if (!RegisterBlockAndWakeupHandlers (SertafiedBlockHandler,
					     SertafiedWakeupHandler,
					     (pointer) 0))
	{
	    xfree (pRequest);
	    return FALSE;
	}
	BlockHandlerRegistered = TRUE;
    }
    pRequest->notifyFunc = 0;
    if (!AddResource (pRequest->id, SertafiedResType, (pointer) pRequest))
	return FALSE;
    if (!notifyFunc)
	notifyFunc = ClientAwaken;
    pRequest->notifyFunc = notifyFunc;
    /* Insert into time-ordered queue, with earliest activation time coming first. */
    pPrev = 0;
    for (pReq = pPending; pReq; pReq = pReq->next)
    {
	if (CompareTimeStamps (pReq->revive, *revive) == LATER)
	    break;
	pPrev = pReq;
    }
    if (pPrev)
	pPrev->next = pRequest;
    else
	pPending = pRequest;
    pRequest->next = pReq;
    IgnoreClient (client);
    return TRUE;
}

static void
ClientAwaken (client, closure)
    ClientPtr	client;
    pointer	closure;
{
    if (!client->clientGone)
	AttendClient (client);
}


static int
SertafiedDelete (value, id)
    pointer value;
    XID id;
{
    SertafiedPtr	pRequest = (SertafiedPtr)value;
    SertafiedPtr	pReq, pPrev;

    pPrev = 0;
    for (pReq = pPending; pReq; pPrev = pReq, pReq = pReq->next)
	if (pReq == pRequest)
	{
	    if (pPrev)
		pPrev->next = pReq->next;
	    else
		pPending = pReq->next;
	    break;
	}
    if (pRequest->notifyFunc)
	(*pRequest->notifyFunc) (pRequest->pClient, pRequest->closure);
    xfree (pRequest);
    return TRUE;
}

static void
SertafiedBlockHandler (data, wt, LastSelectMask)
    pointer	    data;		/* unused */
    OSTimePtr	    wt;			/* wait time */
    pointer	    LastSelectMask;
{
    SertafiedPtr	    pReq, pNext;
    unsigned long	    newdelay, olddelay;
    TimeStamp		    now;

    if (!pPending)
	return;
    now.milliseconds = GetTimeInMillis ();
    now.months = currentTime.months;
    if ((int) (now.milliseconds - currentTime.milliseconds) < 0)
	now.months++;
    for (pReq = pPending; pReq; pReq = pNext)
    {
	pNext = pReq->next;
	if (CompareTimeStamps (pReq->revive, now) == LATER)
	    break;
	FreeResource (pReq->id, RT_NONE);

 	/* AttendClient() may have been called via the resource delete
 	 * function so a client may have input to be processed and so
 	 *  set delay to 0 to prevent blocking in WaitForSomething().
 	 */
 	AdjustWaitForDelay (wt, 0);
    }
    pReq = pPending;
    if (!pReq)
	return;
    newdelay = pReq->revive.milliseconds - now.milliseconds;
    AdjustWaitForDelay (wt, newdelay);
}

static void
SertafiedWakeupHandler (data, i, LastSelectMask)
    pointer	    data;
    int		    i;
    pointer	    LastSelectMask;
{
    SertafiedPtr	pReq, pNext;
    TimeStamp		now;

    now.milliseconds = GetTimeInMillis ();
    now.months = currentTime.months;
    if ((int) (now.milliseconds - currentTime.milliseconds) < 0)
	now.months++;
    for (pReq = pPending; pReq; pReq = pNext)
    {
	pNext = pReq->next;
	if (CompareTimeStamps (pReq->revive, now) == LATER)
	    break;
	FreeResource (pReq->id, RT_NONE);
    }
    if (!pPending)
    {
	RemoveBlockAndWakeupHandlers (SertafiedBlockHandler,
				      SertafiedWakeupHandler,
				      (pointer) 0);
	BlockHandlerRegistered = FALSE;
    }
}
