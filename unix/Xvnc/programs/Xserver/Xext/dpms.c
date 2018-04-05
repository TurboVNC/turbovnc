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
#include <X11/extensions/dpmsproto.h>
#include "dpmsproc.h"
#include "extinit.h"

static int
ProcDPMSGetVersion(ClientPtr client)
{
    /* REQUEST(xDPMSGetVersionReq); */
    xDPMSGetVersionReply rep = {
        .type = X_Reply,
        .sequenceNumber = client->sequence,
        .length = 0,
        .majorVersion = DPMSMajorVersion,
        .minorVersion = DPMSMinorVersion
    };

    REQUEST_SIZE_MATCH(xDPMSGetVersionReq);

    if (client->swapped) {
        swaps(&rep.sequenceNumber);
        swaps(&rep.majorVersion);
        swaps(&rep.minorVersion);
    }
    WriteToClient(client, sizeof(xDPMSGetVersionReply), &rep);
    return Success;
}

static int
ProcDPMSCapable(ClientPtr client)
{
    /* REQUEST(xDPMSCapableReq); */
    xDPMSCapableReply rep = {
        .type = X_Reply,
        .sequenceNumber = client->sequence,
        .length = 0,
        .capable = DPMSCapableFlag
    };

    REQUEST_SIZE_MATCH(xDPMSCapableReq);

    if (client->swapped) {
        swaps(&rep.sequenceNumber);
    }
    WriteToClient(client, sizeof(xDPMSCapableReply), &rep);
    return Success;
}

static int
ProcDPMSGetTimeouts(ClientPtr client)
{
    /* REQUEST(xDPMSGetTimeoutsReq); */
    xDPMSGetTimeoutsReply rep = {
        .type = X_Reply,
        .sequenceNumber = client->sequence,
        .length = 0,
        .standby = DPMSStandbyTime / MILLI_PER_SECOND,
        .suspend = DPMSSuspendTime / MILLI_PER_SECOND,
        .off = DPMSOffTime / MILLI_PER_SECOND
    };

    REQUEST_SIZE_MATCH(xDPMSGetTimeoutsReq);

    if (client->swapped) {
        swaps(&rep.sequenceNumber);
        swaps(&rep.standby);
        swaps(&rep.suspend);
        swaps(&rep.off);
    }
    WriteToClient(client, sizeof(xDPMSGetTimeoutsReply), &rep);
    return Success;
}

static int
ProcDPMSSetTimeouts(ClientPtr client)
{
    REQUEST(xDPMSSetTimeoutsReq);

    REQUEST_SIZE_MATCH(xDPMSSetTimeoutsReq);

    if ((stuff->off != 0) && (stuff->off < stuff->suspend)) {
        client->errorValue = stuff->off;
        return BadValue;
    }
    if ((stuff->suspend != 0) && (stuff->suspend < stuff->standby)) {
        client->errorValue = stuff->suspend;
        return BadValue;
    }

    DPMSStandbyTime = stuff->standby * MILLI_PER_SECOND;
    DPMSSuspendTime = stuff->suspend * MILLI_PER_SECOND;
    DPMSOffTime = stuff->off * MILLI_PER_SECOND;
    SetScreenSaverTimer();

    return Success;
}

static int
ProcDPMSEnable(ClientPtr client)
{
    Bool was_enabled = DPMSEnabled;

    REQUEST_SIZE_MATCH(xDPMSEnableReq);

    if (DPMSCapableFlag) {
        DPMSEnabled = TRUE;
        if (!was_enabled)
            SetScreenSaverTimer();
    }

    return Success;
}

static int
ProcDPMSDisable(ClientPtr client)
{
    /* REQUEST(xDPMSDisableReq); */

    REQUEST_SIZE_MATCH(xDPMSDisableReq);

    DPMSSet(client, DPMSModeOn);

    DPMSEnabled = FALSE;

    return Success;
}

static int
ProcDPMSForceLevel(ClientPtr client)
{
    REQUEST(xDPMSForceLevelReq);

    REQUEST_SIZE_MATCH(xDPMSForceLevelReq);

    if (!DPMSEnabled)
        return BadMatch;

    if (stuff->level != DPMSModeOn &&
        stuff->level != DPMSModeStandby &&
        stuff->level != DPMSModeSuspend && stuff->level != DPMSModeOff) {
        client->errorValue = stuff->level;
        return BadValue;
    }

    DPMSSet(client, stuff->level);

    return Success;
}

static int
ProcDPMSInfo(ClientPtr client)
{
    /* REQUEST(xDPMSInfoReq); */
    xDPMSInfoReply rep = {
        .type = X_Reply,
        .sequenceNumber = client->sequence,
        .length = 0,
        .power_level = DPMSPowerLevel,
        .state = DPMSEnabled
    };

    REQUEST_SIZE_MATCH(xDPMSInfoReq);

    if (client->swapped) {
        swaps(&rep.sequenceNumber);
        swaps(&rep.power_level);
    }
    WriteToClient(client, sizeof(xDPMSInfoReply), &rep);
    return Success;
}

static int
ProcDPMSDispatch(ClientPtr client)
{
    REQUEST(xReq);

    switch (stuff->data) {
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
SProcDPMSGetVersion(ClientPtr client)
{
    REQUEST(xDPMSGetVersionReq);

    swaps(&stuff->length);
    REQUEST_SIZE_MATCH(xDPMSGetVersionReq);
    swaps(&stuff->majorVersion);
    swaps(&stuff->minorVersion);
    return ProcDPMSGetVersion(client);
}

static int
SProcDPMSCapable(ClientPtr client)
{
    REQUEST(xDPMSCapableReq);

    swaps(&stuff->length);
    REQUEST_SIZE_MATCH(xDPMSCapableReq);

    return ProcDPMSCapable(client);
}

static int
SProcDPMSGetTimeouts(ClientPtr client)
{
    REQUEST(xDPMSGetTimeoutsReq);

    swaps(&stuff->length);
    REQUEST_SIZE_MATCH(xDPMSGetTimeoutsReq);

    return ProcDPMSGetTimeouts(client);
}

static int
SProcDPMSSetTimeouts(ClientPtr client)
{
    REQUEST(xDPMSSetTimeoutsReq);

    swaps(&stuff->length);
    REQUEST_SIZE_MATCH(xDPMSSetTimeoutsReq);

    swaps(&stuff->standby);
    swaps(&stuff->suspend);
    swaps(&stuff->off);
    return ProcDPMSSetTimeouts(client);
}

static int
SProcDPMSEnable(ClientPtr client)
{
    REQUEST(xDPMSEnableReq);

    swaps(&stuff->length);
    REQUEST_SIZE_MATCH(xDPMSEnableReq);

    return ProcDPMSEnable(client);
}

static int
SProcDPMSDisable(ClientPtr client)
{
    REQUEST(xDPMSDisableReq);

    swaps(&stuff->length);
    REQUEST_SIZE_MATCH(xDPMSDisableReq);

    return ProcDPMSDisable(client);
}

static int
SProcDPMSForceLevel(ClientPtr client)
{
    REQUEST(xDPMSForceLevelReq);

    swaps(&stuff->length);
    REQUEST_SIZE_MATCH(xDPMSForceLevelReq);

    swaps(&stuff->level);

    return ProcDPMSForceLevel(client);
}

static int
SProcDPMSInfo(ClientPtr client)
{
    REQUEST(xDPMSInfoReq);

    swaps(&stuff->length);
    REQUEST_SIZE_MATCH(xDPMSInfoReq);

    return ProcDPMSInfo(client);
}

static int
SProcDPMSDispatch(ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data) {
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

void
DPMSExtensionInit(void)
{
    AddExtension(DPMSExtensionName, 0, 0,
                 ProcDPMSDispatch, SProcDPMSDispatch,
                 NULL, StandardMinorOpcode);
}
