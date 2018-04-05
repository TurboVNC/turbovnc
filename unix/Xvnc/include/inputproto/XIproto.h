/************************************************************

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Copyright 1989 by Hewlett-Packard Company, Palo Alto, California.

			All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Hewlett-Packard not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HEWLETT-PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

#ifndef _XIPROTO_H
#define _XIPROTO_H

#include <X11/Xproto.h>
#include <X11/X.h>

/* make sure types have right sizes for protocol structures. */
#define Window CARD32
#define Time CARD32
#define KeyCode CARD8
#define Mask CARD32
#define Atom CARD32
#define Cursor CARD32

/*********************************************************
 *
 * number of events, errors, and extension name.
 *
 */

#define MORE_EVENTS	0x80
#define DEVICE_BITS	0x7F

#define InputClassBits	0x3F	/* bits in mode field for input classes */
#define ModeBitsShift	6	/* amount to shift the remaining bits   */

#define numInputClasses 7

#define IEVENTS         17       /* does NOT include generic events */
#define IERRORS         5
#define IREQUESTS       39

#define CLIENT_REQ      1

typedef struct  _XExtEventInfo
    {
    Mask	mask;
    BYTE	type;
    BYTE	word;
    } XExtEventInfo;

#ifndef _XITYPEDEF_POINTER
typedef void *Pointer;
#endif

struct tmask
    {
    Mask	mask;
    void        *dev;
    };

/*********************************************************
 *
 * Event constants used by library.
 *
 */

#define XI_DeviceValuator		0
#define XI_DeviceKeyPress		1
#define XI_DeviceKeyRelease		2
#define XI_DeviceButtonPress		3
#define XI_DeviceButtonRelease		4
#define XI_DeviceMotionNotify		5
#define XI_DeviceFocusIn		6
#define XI_DeviceFocusOut		7
#define XI_ProximityIn			8
#define XI_ProximityOut			9
#define XI_DeviceStateNotify		10
#define XI_DeviceMappingNotify		11
#define XI_ChangeDeviceNotify		12
#define XI_DeviceKeystateNotify		13
#define XI_DeviceButtonstateNotify	14
#define XI_DevicePresenceNotify		15
#define XI_DevicePropertyNotify         16

/*********************************************************
 *
 * Protocol request constants
 *
 */

#define X_GetExtensionVersion		1
#define X_ListInputDevices		2
#define X_OpenDevice			3
#define X_CloseDevice			4
#define X_SetDeviceMode			5
#define X_SelectExtensionEvent		6
#define X_GetSelectedExtensionEvents	7
#define X_ChangeDeviceDontPropagateList 8
#define X_GetDeviceDontPropagateList	9
#define X_GetDeviceMotionEvents		10
#define X_ChangeKeyboardDevice		11
#define X_ChangePointerDevice		12
#define X_GrabDevice			13
#define X_UngrabDevice			14
#define X_GrabDeviceKey			15
#define X_UngrabDeviceKey		16
#define X_GrabDeviceButton		17
#define X_UngrabDeviceButton		18
#define X_AllowDeviceEvents		19
#define X_GetDeviceFocus		20
#define X_SetDeviceFocus		21
#define X_GetFeedbackControl		22
#define X_ChangeFeedbackControl		23
#define X_GetDeviceKeyMapping		24
#define X_ChangeDeviceKeyMapping	25
#define X_GetDeviceModifierMapping	26
#define X_SetDeviceModifierMapping	27
#define X_GetDeviceButtonMapping	28
#define X_SetDeviceButtonMapping	29
#define X_QueryDeviceState		30
#define X_SendExtensionEvent		31
#define X_DeviceBell			32
#define X_SetDeviceValuators		33
#define X_GetDeviceControl		34
#define X_ChangeDeviceControl		35
/* XI 1.5 */
#define X_ListDeviceProperties          36
#define X_ChangeDeviceProperty          37
#define X_DeleteDeviceProperty          38
#define X_GetDeviceProperty             39

/*********************************************************
 *
 * Protocol request and reply structures.
 *
 * GetExtensionVersion.
 *
 */

typedef struct {
    CARD8	reqType;       /* input extension major code   */
    CARD8	ReqType;       /* always X_GetExtensionVersion */
    CARD16	length B16;
    CARD16	nbytes B16;
    CARD8	pad1, pad2;
} xGetExtensionVersionReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;	/* always X_GetExtensionVersion */
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	major_version B16;
    CARD16	minor_version B16;
    BOOL	present;
    CARD8	pad1, pad2, pad3;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
} xGetExtensionVersionReply;

/*********************************************************
 *
 * ListInputDevices.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;	/* always X_ListInputDevices	*/
    CARD16	length B16;
} xListInputDevicesReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;        /* always X_ListInputDevices	*/
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	ndevices;
    CARD8	pad1, pad2, pad3;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    CARD32	pad05 B32;
} xListInputDevicesReply;

typedef struct _xDeviceInfo *xDeviceInfoPtr;

typedef struct _xAnyClassinfo *xAnyClassPtr;

typedef struct _xAnyClassinfo {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;
#else
    CARD8	class;
#endif
    CARD8	length;
    } xAnyClassInfo;

typedef struct _xDeviceInfo {
    CARD32	type B32;
    CARD8	id;
    CARD8	num_classes;
    CARD8	use;      /* IsXPointer | IsXKeyboard | IsXExtension... */
    CARD8	attached; /* id of master dev (if IsXExtension..) */
    } xDeviceInfo;

typedef struct _xKeyInfo *xKeyInfoPtr;

typedef struct _xKeyInfo {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;
#else
    CARD8	class;
#endif
    CARD8	length;
    KeyCode	min_keycode;
    KeyCode	max_keycode;
    CARD16	num_keys B16;
    CARD8	pad1,pad2;
    } xKeyInfo;

typedef struct _xButtonInfo *xButtonInfoPtr;

typedef struct _xButtonInfo {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;
#else
    CARD8	class;
#endif
    CARD8	length;
    CARD16	num_buttons B16;
    } xButtonInfo;

typedef struct _xValuatorInfo *xValuatorInfoPtr;

typedef struct _xValuatorInfo {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;
#else
    CARD8	class;
#endif
    CARD8	length;
    CARD8	num_axes;
    CARD8	mode;
    CARD32	motion_buffer_size B32;
    } xValuatorInfo;

typedef struct _xAxisInfo *xAxisInfoPtr;

typedef struct _xAxisInfo {
    CARD32	resolution B32;
    CARD32	min_value B32;
    CARD32	max_value B32;
    } xAxisInfo;

/*********************************************************
 *
 * OpenDevice.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* always X_OpenDevice		*/
    CARD16	length B16;
    CARD8       deviceid;
    BYTE	pad1, pad2, pad3;
} xOpenDeviceReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;	/* always X_OpenDevice		*/
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	num_classes;
    BYTE	pad1, pad2, pad3;
    CARD32	pad00 B32;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    } xOpenDeviceReply;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;
#else
    CARD8	class;
#endif
    CARD8	event_type_base;
    } xInputClassInfo;

/*********************************************************
 *
 * CloseDevice.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* always X_CloseDevice	*/
    CARD16	length B16;
    CARD8       deviceid;
    BYTE	pad1, pad2, pad3;
} xCloseDeviceReq;

/*********************************************************
 *
 * SetDeviceMode.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;	/* always X_SetDeviceMode	*/
    CARD16	length B16;
    CARD8       deviceid;
    CARD8       mode;
    BYTE	pad1, pad2;
} xSetDeviceModeReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;	/* always X_SetDeviceMode	*/
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	status;
    BYTE	pad1, pad2, pad3;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    CARD32	pad05 B32;
} xSetDeviceModeReply;

/*********************************************************
 *
 * SelectExtensionEvent.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* always X_SelectExtensionEvent */
    CARD16	length B16;
    Window	window B32;
    CARD16	count B16;
    CARD16	pad00 B16;
} xSelectExtensionEventReq;

/*********************************************************
 *
 * GetSelectedExtensionEvent.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* X_GetSelectedExtensionEvents */
    CARD16	length B16;
    Window	window B32;
} xGetSelectedExtensionEventsReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;	/* GetSelectedExtensionEvents	*/
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	this_client_count B16;
    CARD16	all_clients_count B16;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    CARD32	pad05 B32;
} xGetSelectedExtensionEventsReply;

/*********************************************************
 *
 * ChangeDeviceDontPropagateList.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* X_ChangeDeviceDontPropagateList */
    CARD16	length B16;
    Window	window B32;
    CARD16	count B16;
    CARD8	mode;
    BYTE	pad;
} xChangeDeviceDontPropagateListReq;

/*********************************************************
 *
 * GetDeviceDontPropagateList.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* X_GetDeviceDontPropagateList */
    CARD16	length B16;
    Window	window B32;
} xGetDeviceDontPropagateListReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;        /* GetDeviceDontPropagateList   */
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	count B16;
    CARD16	pad00 B16;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    CARD32	pad05 B32;
    } xGetDeviceDontPropagateListReply;

/*********************************************************
 *
 * GetDeviceMotionEvents.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* always X_GetDeviceMotionEvents*/
    CARD16	length B16;
    Time	start B32;
    Time	stop B32;
    CARD8	deviceid;
    BYTE	pad1, pad2, pad3;
} xGetDeviceMotionEventsReq;

typedef struct {
    CARD8	repType;	/* X_Reply */
    CARD8	RepType;        /* always X_GetDeviceMotionEvents  */
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	nEvents B32;
    CARD8	axes;
    CARD8	mode;
    BYTE	pad1, pad2;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
} xGetDeviceMotionEventsReply;

/*********************************************************
 *
 * ChangeKeyboardDevice.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* X_ChangeKeyboardDevice	*/
    CARD16	length B16;
    CARD8	deviceid;
    BYTE	pad1, pad2, pad3;
} xChangeKeyboardDeviceReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;        /* always X_ChangeKeyboardDevice*/
    CARD16	sequenceNumber B16;
    CARD32	length B32;  /* 0 */
    CARD8	status;
    BYTE	pad1, pad2, pad3;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    CARD32	pad05 B32;
    } xChangeKeyboardDeviceReply;

/*********************************************************
 *
 * ChangePointerDevice.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* X_ChangePointerDevice	*/
    CARD16	length B16;
    CARD8	xaxis;
    CARD8	yaxis;
    CARD8	deviceid;
    BYTE	pad1;
} xChangePointerDeviceReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;        /* always X_ChangePointerDevice */
    CARD16	sequenceNumber B16;
    CARD32	length B32;  /* 0 */
    CARD8	status;
    BYTE	pad1, pad2, pad3;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    CARD32	pad05 B32;
    } xChangePointerDeviceReply;

/*********************************************************
 *
 * GrabDevice.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* always X_GrabDevice */
    CARD16	length B16;
    Window	grabWindow B32;
    Time	time B32;
    CARD16	event_count B16;
    CARD8	this_device_mode;
    CARD8	other_devices_mode;
    BOOL	ownerEvents;
    CARD8	deviceid;
    CARD16	pad01 B16;
} xGrabDeviceReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;        /* always X_GrabDevice	*/
    CARD16	sequenceNumber B16;
    CARD32	length B32;  /* 0 */
    CARD8	status;
    BYTE	pad1, pad2, pad3;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    CARD32	pad05 B32;
    } xGrabDeviceReply;

/*********************************************************
 *
 * UngrabDevice.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* always X_UnGrabDevice	*/
    CARD16	length B16;
    Time	time B32;
    CARD8	deviceid;
    BYTE	pad1, pad2, pad3;
} xUngrabDeviceReq;

/*********************************************************
 *
 * GrabDeviceKey.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* always X_GrabDeviceKey	*/
    CARD16	length B16;
    Window	grabWindow B32;
    CARD16	event_count B16;
    CARD16	modifiers B16;
    CARD8	modifier_device;
    CARD8	grabbed_device;
    CARD8	key;
    BYTE	this_device_mode;
    BYTE	other_devices_mode;
    BOOL	ownerEvents;
    BYTE	pad1, pad2;
} xGrabDeviceKeyReq;

/*********************************************************
 *
 * UngrabDeviceKey.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* always X_UngrabDeviceKey	*/
    CARD16	length B16;
    Window	grabWindow B32;
    CARD16	modifiers B16;
    CARD8	modifier_device;
    CARD8	key;
    CARD8	grabbed_device;
    BYTE	pad1, pad2, pad3;
} xUngrabDeviceKeyReq;

/*********************************************************
 *
 * GrabDeviceButton.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* always X_GrabDeviceButton	*/
    CARD16	length B16;
    Window	grabWindow B32;
    CARD8	grabbed_device;
    CARD8	modifier_device;
    CARD16	event_count B16;
    CARD16	modifiers B16;
    BYTE	this_device_mode;
    BYTE	other_devices_mode;
    CARD8	button;
    BOOL	ownerEvents;
    BYTE	pad1, pad2;
} xGrabDeviceButtonReq;

/*********************************************************
 *
 * UngrabDeviceButton.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* always X_UngrabDeviceButton	*/
    CARD16	length B16;
    Window	grabWindow B32;
    CARD16	modifiers B16;
    CARD8	modifier_device;
    CARD8	button;
    CARD8	grabbed_device;
    BYTE	pad1, pad2, pad3;
} xUngrabDeviceButtonReq;

/*********************************************************
 *
 * AllowDeviceEvents.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* always X_AllowDeviceEvents	*/
    CARD16	length B16;
    Time	time B32;
    CARD8	mode;
    CARD8	deviceid;
    BYTE	pad1, pad2;
} xAllowDeviceEventsReq;

/*********************************************************
 *
 * GetDeviceFocus.
 *
 */

typedef struct {
    CARD8	reqType;        /* input extension major code   */
    CARD8	ReqType;        /* always X_GetDeviceFocus	*/
    CARD16	length B16;
    CARD8	deviceid;
    BYTE	pad1, pad2, pad3;
} xGetDeviceFocusReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;        /* always X_GetDeviceFocus	*/
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	focus B32;
    Time	time B32;
    CARD8	revertTo;
    BYTE	pad1, pad2, pad3;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    } xGetDeviceFocusReply;

/*********************************************************
 *
 * SetDeviceFocus.
 *
 */

typedef struct {
    CARD8	reqType;        /* input extension major code   */
    CARD8	ReqType;        /* always X_SetDeviceFocus	*/
    CARD16	length B16;
    Window	focus B32;
    Time	time B32;
    CARD8	revertTo;
    CARD8	device;
    CARD16	pad01 B16;
} xSetDeviceFocusReq;

/*********************************************************
 *
 * GetFeedbackControl.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* X_GetFeedbackControl	*/
    CARD16	length B16;
    CARD8	deviceid;
    BYTE	pad1, pad2, pad3;
} xGetFeedbackControlReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;        /* always X_GetFeedbackControl	*/
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	num_feedbacks B16;
    CARD16	pad01 B16;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    CARD32	pad05 B32;
    CARD32	pad06 B32;
} xGetFeedbackControlReply;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;	/* feedback class		*/
#else
    CARD8	class;		/* feedback class		*/
#endif
    CARD8	id;		/* feedback id		*/
    CARD16	length B16;	/* feedback length		*/
} xFeedbackState;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8   c_class;
#else
    CARD8   class;
#endif
    CARD8   id;
    CARD16  length B16;
    CARD16  pitch B16;
    CARD16  duration B16;
    CARD32  led_mask B32;
    CARD32  led_values B32;
    BOOL    global_auto_repeat;
    CARD8   click;
    CARD8   percent;
    BYTE    pad;
    BYTE    auto_repeats[32];
} xKbdFeedbackState;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8   c_class;
#else
    CARD8   class;
#endif
    CARD8   id;
    CARD16  length B16;
    CARD8   pad1,pad2;
    CARD16  accelNum B16;
    CARD16  accelDenom B16;
    CARD16  threshold B16;
} xPtrFeedbackState;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;	/* feedback class id		*/
#else
    CARD8	class;		/* feedback class id		*/
#endif
    CARD8	id;
    CARD16	length B16;	/* feedback length		*/
    CARD32	resolution B32;
    INT32	min_value B32;
    INT32	max_value B32;
} xIntegerFeedbackState;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;	/* feedback class id		*/
#else
    CARD8	class;		/* feedback class id		*/
#endif
    CARD8	id;
    CARD16	length B16;	/* feedback length		*/
    CARD16	max_symbols B16;
    CARD16	num_syms_supported B16;
} xStringFeedbackState;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;	/* feedback class id		*/
#else
    CARD8	class;		/* feedback class id		*/
#endif
    CARD8	id;
    CARD16	length B16;	/* feedback length		*/
    CARD8	percent;
    BYTE	pad1, pad2, pad3;
    CARD16	pitch B16;
    CARD16	duration B16;
} xBellFeedbackState;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;	/* feedback class id		*/
#else
    CARD8	class;		/* feedback class id		*/
#endif
    CARD8	id;
    CARD16	length B16;	/* feedback length		*/
    CARD32	led_mask B32;
    CARD32	led_values B32;
} xLedFeedbackState;

/*********************************************************
 *
 * ChangeFeedbackControl.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* X_ChangeFeedbackControl	*/
    CARD16	length B16;
    CARD32	mask B32;
    CARD8	deviceid;
    CARD8	feedbackid;
    BYTE	pad1, pad2;
} xChangeFeedbackControlReq;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;	/* feedback class id		*/
#else
    CARD8	class;		/* feedback class id		*/
#endif
    CARD8	id;		/* feedback id		*/
    CARD16	length B16;	/* feedback length		*/
} xFeedbackCtl;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;	/* feedback class id		*/
#else
    CARD8	class;		/* feedback class id		*/
#endif
    CARD8	id;		/* feedback length		*/
    CARD16	length B16;	/* feedback length		*/
    KeyCode	key;
    CARD8	auto_repeat_mode;
    INT8	click;
    INT8	percent;
    INT16	pitch B16;
    INT16	duration B16;
    CARD32	led_mask B32;
    CARD32	led_values B32;
} xKbdFeedbackCtl;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;	/* feedback class id		*/
#else
    CARD8	class;		/* feedback class id		*/
#endif
    CARD8	id;		/* feedback id		*/
    CARD16	length B16;	/* feedback length		*/
    CARD8	pad1,pad2;
    INT16	num B16;
    INT16	denom B16;
    INT16	thresh B16;
} xPtrFeedbackCtl;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;	/* feedback class id		*/
#else
    CARD8	class;		/* feedback class id		*/
#endif
    CARD8	id;		/* feedback id		*/
    CARD16	length B16;	/* feedback length		*/
    INT32	int_to_display B32;
} xIntegerFeedbackCtl;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;	/* feedback class id		*/
#else
    CARD8	class;		/* feedback class id		*/
#endif
    CARD8	id;		/* feedback id		*/
    CARD16	length B16;	/* feedback length		*/
    CARD8	pad1,pad2;
    CARD16	num_keysyms B16;
} xStringFeedbackCtl;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;	/* feedback class id		*/
#else
    CARD8	class;		/* feedback class id		*/
#endif
    CARD8	id;		/* feedback id		*/
    CARD16	length B16;	/* feedback length		*/
    INT8	percent;
    BYTE	pad1, pad2, pad3;
    INT16	pitch B16;
    INT16	duration B16;
} xBellFeedbackCtl;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;	/* feedback class id		*/
#else
    CARD8	class;		/* feedback class id		*/
#endif
    CARD8	id;		/* feedback id		*/
    CARD16	length B16;	/* feedback length		*/
    CARD32	led_mask B32;
    CARD32	led_values B32;
} xLedFeedbackCtl;

/*********************************************************
 *
 * GetDeviceKeyMapping.
 *
 */

typedef struct {
    CARD8	reqType;        /* input extension major code   */
    CARD8	ReqType;	/* always X_GetDeviceKeyMapping */
    CARD16	length B16;
    CARD8	deviceid;
    KeyCode	firstKeyCode;
    CARD8	count;
    BYTE	pad1;
} xGetDeviceKeyMappingReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;	/* always X_GetDeviceKeyMapping */
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	keySymsPerKeyCode;
    CARD8	pad0;
    CARD16	pad1 B16;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xGetDeviceKeyMappingReply;

/*********************************************************
 *
 * ChangeDeviceKeyMapping.
 *
 */

typedef struct {
    CARD8	reqType;        /* input extension major code   */
    CARD8	ReqType;        /* always X_ChangeDeviceKeyMapping */
    CARD16	length B16;
    CARD8	deviceid;
    KeyCode	firstKeyCode;
    CARD8	keySymsPerKeyCode;
    CARD8	keyCodes;
} xChangeDeviceKeyMappingReq;

/*********************************************************
 *
 * GetDeviceModifierMapping.
 *
 */

typedef struct {
    CARD8	reqType;        /* input extension major code   */
    CARD8	ReqType;        /* always X_GetDeviceModifierMapping */
    CARD16	length B16;
    CARD8	deviceid;
    BYTE	pad1, pad2, pad3;
} xGetDeviceModifierMappingReq;

typedef struct {
    CARD8	repType;	/* X_Reply */
    CARD8	RepType;        /* always X_GetDeviceModifierMapping */
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	numKeyPerModifier;
    CARD8	pad0;
    CARD16	pad1 B16;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xGetDeviceModifierMappingReply;

/*********************************************************
 *
 * SetDeviceModifierMapping.
 *
 */

typedef struct {
    CARD8	reqType;        /* input extension major code   */
    CARD8	ReqType;        /* always X_SetDeviceModifierMapping */
    CARD16	length B16;
    CARD8	deviceid;
    CARD8	numKeyPerModifier;
    CARD16	pad1 B16;
} xSetDeviceModifierMappingReq;

typedef struct {
    CARD8	repType;	/* X_Reply */
    CARD8	RepType;        /* always X_SetDeviceModifierMapping */
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	success;
    CARD8	pad0;
    CARD16	pad1 B16;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xSetDeviceModifierMappingReply;

/*********************************************************
 *
 * GetDeviceButtonMapping.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* X_GetDeviceButtonMapping     */
    CARD16	length B16;
    CARD8	deviceid;
    BYTE	pad1, pad2, pad3;
} xGetDeviceButtonMappingReq;

typedef struct {
    CARD8	repType;	/* X_Reply */
    CARD8	RepType;        /* always X_GetDeviceButtonMapping */
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	nElts;
    BYTE	pad1, pad2, pad3;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    CARD32	pad05 B32;
} xGetDeviceButtonMappingReply;

/*********************************************************
 *
 * SetDeviceButtonMapping.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;        /* X_SetDeviceButtonMapping     */
    CARD16	length B16;
    CARD8	deviceid;
    CARD8	map_length;
    BYTE	pad1, pad2;
} xSetDeviceButtonMappingReq;

typedef struct {
    CARD8	repType;		/* X_Reply */
    CARD8	RepType;	/* always X_SetDeviceButtonMapping */
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	status;
    BYTE	pad0;
    CARD16	pad1 B16;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xSetDeviceButtonMappingReply;

/*********************************************************
 *
 * QueryDeviceState.
 *
 */

typedef struct {
    CARD8	reqType;
    CARD8	ReqType;        /* always X_QueryDeviceState */
    CARD16	length B16;
    CARD8	deviceid;
    BYTE	pad1, pad2, pad3;
} xQueryDeviceStateReq;

typedef struct {
    CARD8	repType;		/* X_Reply */
    CARD8	RepType;	/* always X_QueryDeviceState	*/
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	num_classes;
    BYTE	pad0;
    CARD16	pad1 B16;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xQueryDeviceStateReply;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;
#else
    CARD8	class;
#endif
    CARD8	length;
    CARD8	num_keys;
    BYTE	pad1;
    CARD8	keys[32];
} xKeyState;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;
#else
    CARD8	class;
#endif
    CARD8	length;
    CARD8	num_buttons;
    BYTE	pad1;
    CARD8	buttons[32];
} xButtonState;

typedef struct {
#if defined(__cplusplus) || defined(c_plusplus)
    CARD8	c_class;
#else
    CARD8	class;
#endif
    CARD8	length;
    CARD8	num_valuators;
    CARD8	mode;
} xValuatorState;

/*********************************************************
 *
 * SendExtensionEvent.
 * THIS REQUEST MUST BE KEPT A MULTIPLE OF 8 BYTES IN LENGTH!
 * MORE EVENTS MAY FOLLOW AND THEY MUST BE QUAD-ALIGNED!
 *
 */

typedef struct {
    CARD8	reqType;
    CARD8	ReqType;        /* always X_SendExtensionEvent */
    CARD16	length B16;
    Window	destination B32;
    CARD8	deviceid;
    BOOL	propagate;
    CARD16	count B16;
    CARD8	num_events;
    BYTE	pad1,pad2,pad3;
} xSendExtensionEventReq;

/*********************************************************
 *
 * DeviceBell.
 *
 */

typedef struct {
    CARD8	reqType;
    CARD8	ReqType;        /* always X_DeviceBell */
    CARD16	length B16;
    CARD8	deviceid;
    CARD8	feedbackid;
    CARD8	feedbackclass;
    INT8	percent;
} xDeviceBellReq;

/*********************************************************
 *
 * SetDeviceValuators.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;	/* always X_SetDeviceValuators	*/
    CARD16	length B16;
    CARD8       deviceid;
    CARD8       first_valuator;
    CARD8       num_valuators;
    BYTE	pad1;
} xSetDeviceValuatorsReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;	/* always X_SetDeviceValuators	*/
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	status;
    BYTE	pad1, pad2, pad3;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    CARD32	pad05 B32;
} xSetDeviceValuatorsReply;

/*********************************************************
 *
 * GetDeviceControl.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;	/* always X_GetDeviceControl	*/
    CARD16	length B16;
    CARD16      control B16;
    CARD8       deviceid;
    BYTE	pad2;
} xGetDeviceControlReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;	/* always X_GetDeviceControl	*/
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	status;
    BYTE	pad1, pad2, pad3;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    CARD32	pad05 B32;
} xGetDeviceControlReply;

typedef struct {
    CARD16	control B16;	/* control type		*/
    CARD16	length B16;	/* control length		*/
} xDeviceState;

typedef struct {
    CARD16	control B16;		/* control type		*/
    CARD16	length B16;		/* control length		*/
    CARD32	num_valuators B32;	/* number of valuators		*/
} xDeviceResolutionState;

typedef struct {
     CARD16         control B16;
     CARD16         length B16;
     INT32          min_x B32;
     INT32          max_x B32;
     INT32          min_y B32;
     INT32          max_y B32;
     CARD32         flip_x B32;
     CARD32         flip_y B32;
     CARD32         rotation B32;
     CARD32         button_threshold B32;
} xDeviceAbsCalibState;

typedef struct {
     CARD16         control B16;
     CARD16         length B16;
     CARD32         offset_x B32;
     CARD32         offset_y B32;
     CARD32         width B32;
     CARD32         height B32;
     CARD32         screen B32;
     CARD32         following B32;
} xDeviceAbsAreaState;

typedef struct {
    CARD16      control B16;            /* control type                 */
    CARD16      length  B16;            /* control length               */
    CARD8       status;
    CARD8       iscore;
    CARD16      pad1 B16;
} xDeviceCoreState;

typedef struct {
    CARD16      control B16;            /* control type                 */
    CARD16      length  B16;            /* control length               */
    CARD8       enable;
    CARD8       pad0;
    CARD16      pad1 B16;
} xDeviceEnableState;

/*********************************************************
 *
 * ChangeDeviceControl.
 *
 */

typedef struct {
    CARD8	reqType;	/* input extension major code	*/
    CARD8	ReqType;	/* always X_ChangeDeviceControl */
    CARD16	length B16;
    CARD16      control B16;
    CARD8       deviceid;
    BYTE        pad0;
} xChangeDeviceControlReq;

typedef struct {
    CARD8	repType;	/* X_Reply			*/
    CARD8	RepType;	/* always X_ChangeDeviceControl	*/
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	status;
    BYTE	pad1, pad2, pad3;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    CARD32	pad05 B32;
} xChangeDeviceControlReply;

typedef struct {
    CARD16	control B16;	/* control type		*/
    CARD16	length B16;	/* control length		*/
} xDeviceCtl;

typedef struct {
    CARD16	control B16;		/* control type		*/
    CARD16	length B16;		/* control length		*/
    CARD8	first_valuator;		/* first valuator to change     */
    CARD8	num_valuators;		/* number of valuators to change*/
    CARD8	pad1,pad2;
} xDeviceResolutionCtl;

typedef struct {
     CARD16         control B16;
     CARD16         length B16;
     INT32          min_x;
     INT32          max_x;
     INT32          min_y;
     INT32          max_y;
     CARD32         flip_x;
     CARD32         flip_y;
     CARD32         rotation;
     CARD32         button_threshold;
} xDeviceAbsCalibCtl;

typedef struct {
     CARD16         control B16;
     CARD16         length B16;
     CARD32         offset_x;
     CARD32         offset_y;
     INT32          width;
     INT32          height;
     INT32          screen;
     CARD32         following;
} xDeviceAbsAreaCtl;

typedef struct {
    CARD16          control B16;
    CARD16          length  B16;
    CARD8           status;
    CARD8           pad0;
    CARD16          pad1 B16;
} xDeviceCoreCtl;

typedef struct {
    CARD16          control B16;
    CARD16          length  B16;
    CARD8           enable;
    CARD8           pad0;
    CARD16          pad1 B16;
} xDeviceEnableCtl;

/* XI 1.5 */

/*********************************************************
 *
 * ListDeviceProperties.
 *
 */

typedef struct {
    CARD8       reqType;        /* input extension major opcode */
    CARD8       ReqType;        /* always X_ListDeviceProperties */
    CARD16      length B16;
    CARD8       deviceid;
    CARD8       pad0;
    CARD16      pad1 B16;
} xListDevicePropertiesReq;

typedef struct {
    CARD8       repType;        /* X_Reply                       */
    CARD8       RepType;        /* always X_ListDeviceProperties */
    CARD16      sequenceNumber B16;
    CARD32      length B32;
    CARD16      nAtoms B16;
    CARD16      pad1 B16;
    CARD32      pad2 B32;
    CARD32      pad3 B32;
    CARD32      pad4 B32;
    CARD32      pad5 B32;
    CARD32      pad6 B32;
} xListDevicePropertiesReply;

/*********************************************************
 *
 * ChangeDeviceProperty.
 *
 */

typedef struct {
    CARD8       reqType;        /* input extension major opcode */
    CARD8       ReqType;        /* always X_ChangeDeviceProperty */
    CARD16      length B16;
    Atom        property B32;
    Atom        type B32;
    CARD8       deviceid;
    CARD8       format;
    CARD8       mode;
    CARD8       pad;
    CARD32      nUnits B32;
} xChangeDevicePropertyReq;

/*********************************************************
 *
 * DeleteDeviceProperty.
 *
 */

typedef struct {
    CARD8       reqType;        /* input extension major opcode */
    CARD8       ReqType;        /* always X_DeleteDeviceProperty */
    CARD16      length B16;
    Atom        property B32;
    CARD8       deviceid;
    CARD8       pad0;
    CARD16      pad1 B16;
} xDeleteDevicePropertyReq;

/*********************************************************
 *
 * GetDeviceProperty.
 *
 */

typedef struct {
    CARD8       reqType;        /* input extension major opcode */
    CARD8       ReqType;        /* always X_GetDeviceProperty */
    CARD16      length B16;
    Atom        property B32;
    Atom        type B32;
    CARD32      longOffset B32;
    CARD32      longLength B32;
    CARD8       deviceid;
#if defined(__cplusplus) || defined(c_plusplus)
    BOOL        c_delete;
#else
    BOOL        delete;
#endif
    CARD16      pad;
} xGetDevicePropertyReq;

typedef struct {
    CARD8       repType;        /* X_Reply                        */
    CARD8       RepType;        /* always X_GetDeviceProperty   */
    CARD16      sequenceNumber B16;
    CARD32      length B32;
    Atom        propertyType B32;
    CARD32      bytesAfter B32;
    CARD32      nItems B32;
    CARD8       format;
    CARD8       deviceid;
    CARD16      pad1 B16;
    CARD32      pad2 B32;
    CARD32      pad3 B32;
} xGetDevicePropertyReply;


/**********************************************************
 *
 * Input extension events.
 *
 * DeviceValuator
 *
 */

typedef struct
    {
    BYTE	type;
    CARD8       deviceid;
    CARD16	sequenceNumber B16;
    KeyButMask  device_state B16;
    CARD8	num_valuators;
    CARD8       first_valuator;
    INT32	valuator0 B32;
    INT32	valuator1 B32;
    INT32	valuator2 B32;
    INT32	valuator3 B32;
    INT32	valuator4 B32;
    INT32	valuator5 B32;
    }  deviceValuator;

/**********************************************************
 *
 * DeviceKeyButtonPointer.
 *
 * Used for: DeviceKeyPress, DeviceKeyRelease,
 *	     DeviceButtonPress, DeviceButtonRelease,
 *	     ProximityIn, ProximityOut
 *	     DeviceMotionNotify,
 *
 */

typedef struct
    {
    BYTE	type;
    BYTE        detail;
    CARD16	sequenceNumber B16;
    Time        time B32;
    Window      root B32;
    Window      event B32;
    Window      child B32;
    INT16       root_x B16;
    INT16       root_y B16;
    INT16       event_x B16;
    INT16       event_y B16;
    KeyButMask  state B16;
    BOOL        same_screen;
    CARD8       deviceid;
    }  deviceKeyButtonPointer;

/**********************************************************
 *
 * DeviceFocus.
 *
 */

typedef struct
    {
    BYTE	type;
    BYTE        detail;
    CARD16	sequenceNumber B16;
    Time        time B32;
    Window      window B32;
    BYTE	mode;
    CARD8       deviceid;
    BYTE	pad1, pad2;
    CARD32	pad00 B32;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    }  deviceFocus;

/**********************************************************
 *
 * DeviceStateNotify.
 *
 * Note that the two high-order bits in the classes_reported
 * field are the proximity state (InProximity or OutOfProximity),
 * and the device mode (Absolute or Relative), respectively.
 *
 */

typedef struct
    {
    BYTE	type;
    BYTE        deviceid;
    CARD16	sequenceNumber B16;
    Time        time B32;
    CARD8	num_keys;
    CARD8	num_buttons;
    CARD8	num_valuators;
    CARD8       classes_reported;
    CARD8       buttons[4];
    CARD8       keys[4];
    INT32	valuator0 B32;
    INT32	valuator1 B32;
    INT32	valuator2 B32;
    }  deviceStateNotify;

/**********************************************************
 *
 * DeviceKeyStateNotify.
 *
 */

typedef struct
    {
    BYTE	type;
    BYTE        deviceid;
    CARD16	sequenceNumber B16;
    CARD8       keys[28];
    }  deviceKeyStateNotify;

/**********************************************************
 *
 * DeviceButtonStateNotify.
 *
 */

typedef struct
    {
    BYTE	type;
    BYTE        deviceid;
    CARD16	sequenceNumber B16;
    CARD8       buttons[28];
    }  deviceButtonStateNotify;

/**********************************************************
 *
 * DeviceMappingNotify.
 * Fields must be kept in sync with core mappingnotify event.
 *
 */

typedef struct
    {
    BYTE	type;
    BYTE        deviceid;
    CARD16	sequenceNumber B16;
    CARD8       request;
    KeyCode     firstKeyCode;
    CARD8       count;
    BYTE        pad1;
    Time        time B32;
    CARD32	pad00 B32;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    }  deviceMappingNotify;

/**********************************************************
 *
 * ChangeDeviceNotify.
 *
 */

typedef struct
    {
    BYTE	type;
    BYTE        deviceid;
    CARD16	sequenceNumber B16;
    Time        time B32;
    CARD8       request;
    BYTE        pad1, pad2, pad3;
    CARD32	pad00 B32;
    CARD32	pad01 B32;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    }  changeDeviceNotify;

/**********************************************************
 *
 * devicePresenceNotify.
 *
 */

typedef struct
    {
    BYTE	type;
    BYTE        pad00;
    CARD16	sequenceNumber B16;
    Time        time B32;
    BYTE        devchange; /* Device{Added|Removed|Enabled|Disabled|ControlChanged} */
    BYTE        deviceid;
    CARD16      control B16;
    CARD32	pad02 B32;
    CARD32	pad03 B32;
    CARD32	pad04 B32;
    CARD32	pad05 B32;
    CARD32	pad06 B32;
    }  devicePresenceNotify;


/*********************************************************
 * DevicePropertyNotifyEvent
 *
 * Sent whenever a device's property changes.
 *
 */

typedef struct
    {
    BYTE        type;
    BYTE        state;               /* NewValue or Deleted */
    CARD16      sequenceNumber B16;
    CARD32      time B32;
    Atom        atom B32;            /* affected property */
    CARD32      pad0 B32;
    CARD32      pad1 B32;
    CARD32      pad2 B32;
    CARD32      pad3 B32;
    CARD16      pad5 B16;
    CARD8       pad4;
    CARD8       deviceid;            /* id of device */
    } devicePropertyNotify;

#undef Window
#undef Time
#undef KeyCode
#undef Mask
#undef Atom
#undef Cursor

#endif
