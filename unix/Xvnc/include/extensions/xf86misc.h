/* $XFree86: xc/include/extensions/xf86misc.h,v 3.6.2.4 1998/02/24 19:05:44 hohndel Exp $ */

/*
 * Copyright (c) 1995, 1996  The XFree86 Project, Inc
 */

/* THIS IS NOT AN X CONSORTIUM STANDARD */

#ifndef _XF86MISC_H_
#define _XF86MISC_H_

#include <X11/Xfuncproto.h>

#define X_XF86MiscQueryVersion		0
#ifdef _XF86MISC_SAVER_COMPAT_
#define X_XF86MiscGetSaver		1
#define X_XF86MiscSetSaver		2
#endif
#define X_XF86MiscGetMouseSettings	3
#define X_XF86MiscGetKbdSettings	4
#define X_XF86MiscSetMouseSettings	5
#define X_XF86MiscSetKbdSettings	6

#define XF86MiscNumberEvents		0

#define XF86MiscBadMouseProtocol	0
#define XF86MiscBadMouseBaudRate	1
#define XF86MiscBadMouseFlags		2
#define XF86MiscBadMouseCombo		3
#define XF86MiscBadKbdType		4
#define XF86MiscModInDevDisabled	5
#define XF86MiscModInDevClientNotLocal	6
#define XF86MiscNumberErrors		(XF86MiscModInDevClientNotLocal + 1)

/* Never renumber these */
#define MTYPE_MICROSOFT		0
#define MTYPE_MOUSESYS		1
#define MTYPE_MMSERIES		2
#define MTYPE_LOGITECH		3
#define MTYPE_BUSMOUSE		4
#define MTYPE_LOGIMAN		5
#define MTYPE_PS_2		6
#define MTYPE_MMHIT		7
#define MTYPE_GLIDEPOINT	8
#define MTYPE_IMSERIAL		9
#define MTYPE_THINKING		10
#define MTYPE_IMPS2		11
#define MTYPE_THINKINGPS2	12
#define MTYPE_MMANPLUSPS2	13
#define MTYPE_GLIDEPOINTPS2	14
#define MTYPE_NETPS2		15
#define MTYPE_NETSCROLLPS2	16
#define MTYPE_SYSMOUSE		17
#define MTYPE_AUTOMOUSE		18

#define MTYPE_XQUEUE		127
#define MTYPE_OSMOUSE		126

#define KTYPE_UNKNOWN		0
#define KTYPE_84KEY		1
#define KTYPE_101KEY		2
#define KTYPE_OTHER		3
#define KTYPE_XQUEUE		4

#define MF_CLEAR_DTR		1
#define MF_CLEAR_RTS		2
#define MF_REOPEN		128

#ifndef _XF86MISC_SERVER_

_XFUNCPROTOBEGIN

typedef struct {
    char*	device;
    int		type;
    int		baudrate;
    int		samplerate;
    int		resolution;
    int		buttons;
    Bool	emulate3buttons;
    int		emulate3timeout;
    Bool	chordmiddle;
    int		flags;
} XF86MiscMouseSettings;

typedef struct {
    int		type;
    int		rate;
    int		delay;
    Bool	servnumlock;
} XF86MiscKbdSettings;

Bool XF86MiscQueryVersion(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int*		/* majorVersion */,
    int*		/* minorVersion */
#endif
);

Bool XF86MiscQueryExtension(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int*		/* event_base */,
    int*		/* error_base */
#endif
);

Status XF86MiscGetMouseSettings(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    XF86MiscMouseSettings*	/* mouse info */
#endif
);

Status XF86MiscGetKbdSettings(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    XF86MiscKbdSettings*	/* keyboard info */
#endif
);

Status XF86MiscSetMouseSettings(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    XF86MiscMouseSettings*	/* mouse info */
#endif
);

Status XF86MiscSetKbdSettings(
#if NeedFunctionPrototypes
    Display*			/* dpy */,
    XF86MiscKbdSettings*	/* keyboard info */
#endif
);

_XFUNCPROTOEND

#endif

#endif
