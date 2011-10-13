/* $XFree86: xc/programs/Xserver/Xext/xvmod.c,v 1.1 1998/08/13 14:45:36 dawes Exp $ */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "misc.h"
#include "scrnintstr.h"
#include "gc.h"
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvproto.h>
#include "xvdix.h"
#include "xvmodproc.h"

void
XvRegister()
{
    XvScreenInitProc = XvScreenInit;
    XvGetScreenIndexProc = XvGetScreenIndex;
    XvGetRTPortProc = XvGetRTPort;
    XvMCScreenInitProc = XvMCScreenInit;
}

