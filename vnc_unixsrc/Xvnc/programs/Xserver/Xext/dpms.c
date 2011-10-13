/* $Xorg: dpms.c,v 1.3 2000/08/17 19:47:56 cpqbld Exp $ */
/*****************************************************************

Copyright (c) 1996 Digital Equipment Corporation, Maynard, Massachusetts.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
DIGITAL EQUIPMENT CORPORATION BE LIABLE FOR ANY CLAIM, DAMAGES, INCLUDING, 
BUT NOT LIMITED TO CONSEQUENTIAL OR INCIDENTAL DAMAGES, OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Digital Equipment Corporation 
shall not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from Digital 
Equipment Corporation.

******************************************************************/

/*
 * HISTORY
 *
 * @(#)RCSfile: dpms.c,v Revision: 1.1.4.5  (DEC) Date: 1996/03/04 15:27:00
 */

/* $XFree86: xc/programs/Xserver/Xext/dpms.c,v 3.10tsi Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "opaque.h"
#define DPMS_SERVER
#include <X11/extensions/dpms.h>
#include <X11/extensions/dpmsstr.h>
#include "dpmsproc.h"
#include "modinit.h"

#if 0
static unsigned char DPMSCode;
#endif
static DISPATCH_PROC(ProcDPMSDispatch);
static DISPATCH_PROC(SProcDPMSDispatch);
static DISPATCH_PROC(ProcDPMSGetVersion);
static DISPATCH_PROC(SProcDPMSGetVersion);
static DISPATCH_PROC(ProcDPMSGetTimeouts);
static DISPATCH_PROC(SProcDPMSGetTimeouts);
static DISPATCH_PROC(ProcDPMSSetTimeouts);
static DISPATCH_PROC(SProcDPMSSetTimeouts);
static DISPATCH_PROC(ProcDPMSEnable);
static DISPATCH_PROC(SProcDPMSEnable);
static DISPATCH_PROC(ProcDPMSDisable);
static DISPATCH_PROC(SProcDPMSDisable);
static DISPATCH_PROC(ProcDPMSForceLevel);
static DISPATCH_PROC(SProcDPMSForceLevel);
static DISPATCH_PROC(ProcDPMSInfo);
static DISPATCH_PROC(SProcDPMSInfo);
static DISPATCH_PROC(ProcDPMSCapable);
static DISPATCH_PROC(SProcDPMSCapable);
static void DPMSResetProc(ExtensionEntry* extEntry);

void
DPMSExtensionInit(INITARGS)
{
#if 0
    ExtensionEntry *extEntry;
    
    if ((extEntry = AddExtension(DPMSExtensionName, 0, 0,
				ProcDPMSDispatch, SProcDPMSDispatch,
				DPMSResetProc, StandardMinorOpcode)))
	DPMSCode = (unsigned char)extEntry->base;
#else
    (void) AddExtension(DPMSExtensionName, 0, 0,
			ProcDPMSDispatch, SProcDPMSDispatch,
			DPMSResetProc, StandardMinorOpcode);
#endif
}

/*ARGSUSED*/
static void
DPMSResetProc (extEntry)
    ExtensionEntry	*extEntry;
{
}

static int
ProcDPMSGetVersion(client)
    register ClientPtr client;
{
    /* REQUEST(xDPMSGetVersionReq); */
    xDPMSGetVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xDPMSGetVersionReq);

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = DPMSMajorVersion;
    rep.minorVersion = DPMSMinorVersion;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swaps(&rep.majorVersion, n);
	swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof(xDPMSGetVersionReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcDPMSCapable(register ClientPtr client)
{
    /* REQUEST(xDPMSCapableReq); */
    xDPMSCapableReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xDPMSCapableReq);

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.capable = DPMSCapableFlag;

    if (client->swapped) {
	swaps(&rep.sequenceNumber, n);
    }
    WriteToClient(client, sizeof(xDPMSCapableReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcDPMSGetTimeouts(client)
    register ClientPtr client;
{
    /* REQUEST(xDPMSGetTimeoutsReq); */
    xDPMSGetTimeoutsReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xDPMSGetTimeoutsReq);

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.standby = DPMSStandbyTime / MILLI_PER_SECOND;
    rep.suspend = DPMSSuspendTime / MILLI_PER_SECOND;
    rep.off = DPMSOffTime / MILLI_PER_SECOND;

    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swaps(&rep.standby, n);
	swaps(&rep.suspend, n);
	swaps(&rep.off, n);
    }
    WriteToClient(client, sizeof(xDPMSGetTimeoutsReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcDPMSSetTimeouts(client)
    register ClientPtr client;
{
    REQUEST(xDPMSSetTimeoutsReq);

    REQUEST_SIZE_MATCH(xDPMSSetTimeoutsReq);

    if ((stuff->off != 0)&&(stuff->off < stuff->suspend)) 
    {
	client->errorValue = stuff->off;
	return BadValue;
    }
    if ((stuff->suspend != 0)&&(stuff->suspend < stuff->standby))
    {
	client->errorValue = stuff->suspend;
	return BadValue;
    }  
	
    DPMSStandbyTime = stuff->standby * MILLI_PER_SECOND;
    DPMSSuspendTime = stuff->suspend * MILLI_PER_SECOND;
    DPMSOffTime = stuff->off * MILLI_PER_SECOND;
    SetDPMSTimers();
    
    return(client->noClientException);
}

static int
ProcDPMSEnable(client)
    register ClientPtr client;
{
    /* REQUEST(xDPMSEnableReq); */

    REQUEST_SIZE_MATCH(xDPMSEnableReq);

    if (DPMSCapableFlag)
	DPMSEnabled = TRUE;

    return(client->noClientException);
}

static int
ProcDPMSDisable(client)
    register ClientPtr client;
{
    /* REQUEST(xDPMSDisableReq); */

    REQUEST_SIZE_MATCH(xDPMSDisableReq);

    DPMSSet(DPMSModeOn);

    DPMSEnabled = FALSE;

    return(client->noClientException);
}

static int
ProcDPMSForceLevel(client)
    register ClientPtr client;
{
    REQUEST(xDPMSForceLevelReq);

    REQUEST_SIZE_MATCH(xDPMSForceLevelReq);

    if (!DPMSEnabled)
	return BadMatch;

    if (stuff->level == DPMSModeOn) {
      lastDeviceEventTime.milliseconds =
          GetTimeInMillis();
    } else if (stuff->level == DPMSModeStandby) {
      lastDeviceEventTime.milliseconds =
          GetTimeInMillis() -  DPMSStandbyTime;
    } else if (stuff->level == DPMSModeSuspend) {
      lastDeviceEventTime.milliseconds =
          GetTimeInMillis() -  DPMSSuspendTime;
    } else if (stuff->level == DPMSModeOff) {
      lastDeviceEventTime.milliseconds =
          GetTimeInMillis() -  DPMSOffTime;
    } else {
	client->errorValue = stuff->level;
	return BadValue;
    }

    DPMSSet(stuff->level);

    return(client->noClientException);
}

static int
ProcDPMSInfo(register ClientPtr client)
{
    /* REQUEST(xDPMSInfoReq); */
    xDPMSInfoReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xDPMSInfoReq);

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.power_level = DPMSPowerLevel;
    rep.state = DPMSEnabled;

    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
	swaps(&rep.power_level, n);
    }
    WriteToClient(client, sizeof(xDPMSInfoReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcDPMSDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);

    switch (stuff->data)
    {
    case X_DPMSGetVersion:
	return ProcDPMSGetVersion(client);
    case X_DPMSCapable:
	return ProcDPMSCapable(client);
    case X_DPMSGetTimeouts:
	return ProcDPMSGetTimeouts(client);
    case X_DPMSSetTimeouts:
	return ProcDPMSSetTimeouts(client);
    case X_DPMSEnable:
	return ProcDPMSEnable(client);
    case X_DPMSDisable:
	return ProcDPMSDisable(client);
    case X_DPMSForceLevel:
	return ProcDPMSForceLevel(client);
    case X_DPMSInfo:
	return ProcDPMSInfo(client);
    default:
	return BadRequest;
    }
}

static int
SProcDPMSGetVersion(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xDPMSGetVersionReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDPMSGetVersionReq);
    swaps(&stuff->majorVersion, n);
    swaps(&stuff->minorVersion, n);
    return ProcDPMSGetVersion(client);
}

static int
SProcDPMSCapable(register ClientPtr client)
{
    REQUEST(xDPMSCapableReq);
    register int n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDPMSCapableReq);

    return ProcDPMSCapable(client);
}

static int
SProcDPMSGetTimeouts(client)
    register ClientPtr client;
{
    REQUEST(xDPMSGetTimeoutsReq);
    register int n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDPMSGetTimeoutsReq);

    return ProcDPMSGetTimeouts(client);
}

static int
SProcDPMSSetTimeouts(client)
    register ClientPtr client;
{
    REQUEST(xDPMSSetTimeoutsReq);
    register int n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDPMSSetTimeoutsReq);

    swaps(&stuff->standby, n);
    swaps(&stuff->suspend, n);
    swaps(&stuff->off, n);
    return ProcDPMSSetTimeouts(client);
}

static int
SProcDPMSEnable(client)
    register ClientPtr client;
{
    REQUEST(xDPMSEnableReq);
    register int n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDPMSEnableReq);

    return ProcDPMSEnable(client);
}

static int
SProcDPMSDisable(client)
    register ClientPtr client;
{
    REQUEST(xDPMSDisableReq);
    register int n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDPMSDisableReq);

    return ProcDPMSDisable(client);
}

static int
SProcDPMSForceLevel(client)
    register ClientPtr client;
{
    REQUEST(xDPMSForceLevelReq);
    register int n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDPMSForceLevelReq);

    swaps(&stuff->level, n);

    return ProcDPMSForceLevel(client);
}

static int
SProcDPMSInfo(client)
    register ClientPtr client;
{
    REQUEST(xDPMSInfoReq);
    register int n;

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xDPMSInfoReq);

    return ProcDPMSInfo(client);
}

static int
SProcDPMSDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_DPMSGetVersion:
	return SProcDPMSGetVersion(client);
    case X_DPMSCapable:
	return SProcDPMSCapable(client);
    case X_DPMSGetTimeouts:
	return SProcDPMSGetTimeouts(client);
    case X_DPMSSetTimeouts:
	return SProcDPMSSetTimeouts(client);
    case X_DPMSEnable:
	return SProcDPMSEnable(client);
    case X_DPMSDisable:
	return SProcDPMSDisable(client);
    case X_DPMSForceLevel:
	return SProcDPMSForceLevel(client);
    case X_DPMSInfo:
	return SProcDPMSInfo(client);
    default:
	return BadRequest;
    }
}
