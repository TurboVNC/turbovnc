/* $XConsortium: xtest1dd.c,v 1.14 94/04/17 20:33:00 gildea Exp $ */
/* $XFree86: xc/programs/Xserver/Xext/xtest1dd.c,v 3.0 1996/05/06 05:55:42 dawes Exp $ */
/*
 *	File: xtest1dd.c
 *
 *	This file contains the device dependent parts of the input
 *	synthesis extension.
 */

/*


Copyright (c) 1986, 1987, 1988   X Consortium

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


Copyright 1986, 1987, 1988 by Hewlett-Packard Corporation

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Hewlett-Packard not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

Hewlett-Packard makes no representations about the 
suitability of this software for any purpose.  It is provided 
"as is" without express or implied warranty.

This software is not subject to any license of the American
Telephone and Telegraph Company or of the Regents of the
University of California.

*/

/***************************************************************
 * include files
 ***************************************************************/

#define	NEED_EVENTS
#define	NEED_REPLIES

#include <stdio.h>
#include "Xos.h"
#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "misc.h"
#include "dixstruct.h"
#define  XTestSERVER_SIDE
#include "xtestext1.h"	

#include "xtest1dd.h"

/***************************************************************
 * defines
 ***************************************************************/

/*
 * the size of the fake input action array
 */
#define ACTION_ARRAY_SIZE	100

/***************************************************************
 * externals
 ***************************************************************/

/*
 * Holds the xTestInputAction event type code.
 * This is defined in xtestext1di.c.
 */
extern int			XTestInputActionType;
/*
 * Holds the xTestFakeAck event type code.
 * This is defined in xtestext1di.c.
 */
extern int			XTestFakeAckType;
/*
 * used in the WriteReplyToClient macro
 */
extern int			exclusive_steal;

/***************************************************************
 * variables
 ***************************************************************/

/*
 * array to hold fake input actions
 */
struct {
	/*
	 * holds the action type, one of: XTestDELAY_ACTION,
	 * XTestKEY_ACTION, XTestMOTION_ACTION, XTestJUMP_ACTION
	 */
	CARD8	type;	
	/*
	 * holds the device type, in the range 0 to 15
	 */
	CARD8	device;
	/*
	 * for XTestKEY_ACTION type, holds the keycode
	 */
	CARD8	keycode;
	/*
	 * for XTestKEY_ACTION type, holds the key up/down state
	 */
	CARD8	keystate;
	/*
	 * for XTestMOTION_ACTION and XTestJUMP_ACTION types,
	 * holds the x and y coordinates to move the mouse to
	 */
	int	x;
	int	y;
	/*
	 * holds the time to delay (in milliseconds) before performing
	 * the action
	 */
	CARD32	delay_time;
}action_array[ACTION_ARRAY_SIZE];

/*
 * write index for input action array
 */
static int			write_index = 0;
/*
 * read index for input action array
 */
static int			read_index = 0;
/*
 * this is where the input actions are accumulated until they are sent
 * to a client (in a wire event)
 */
static xTestInputActionEvent	input_action_packet;
/*
 * holds the index (in bytes) into the input actions buffer in the
 * current input action event
 */
static int 			packet_index;
/*
 * set to 1 when the input action event is full and needs to be sent to the 
 * client
 */
static int			input_action_event_full = 0;
/*
 * logical x position of the mouse during input action gathering
 */
short				xtest_mousex;
/*
 * logical y position of the mouse during input action gathering
 */
short				xtest_mousey;
/*
 * logical x position of the mouse during input action playback
 */
static short			mx;
/*
 * logical y position of the mouse during input action playback
 */
static short			my;
/*
 * logical x position of the mouse while we are reading fake input actions
 * from the client and putting them into the fake input action array
 */
static short			pmousex;
/*
 * logical y position of the mouse while we are reading fake input actions
 * from the client and putting them into the fake input action array
 */
static short			pmousey;
/*
 * The playback_on flag is set to 1 while there are input actions in the 
 * input action array.  It is set to 0 when the server has received all of
 * the user actions.
 */
int			playback_on = 0;
/*
 * identity of the client using XTestGetInput to get user input actions
 */
ClientPtr 		current_xtest_client;
/*
 * if 1 send multiple input actions per XTestInputAction event;
 * if 0 send one input action per XTestInputAction event
 */
static char			packed_mode;
/*
 * identity of the client using the XTestFakeInput function to send some
 * fake input actions to the server
 */
ClientPtr		playback_client = NULL;
/*
 * Set to 1 when the XTestFAKE_ACK_REQUEST flag is set in a XTestFakeInput
 * request.  Set back to 0 when all of the input actions have been sent
 * to the server.
 */
static int			acknowledge = 0;
/*
 * The server's idea of the current time is saved in these variables when
 * a XTestFakeInput request is received.  It is restored when all fake input
 * actions are sent to the server or when the playback client disconnects.
 */
static int			saved_sec;
static int			saved_usec;
/*
 * Set to 1 when there is a valid time in saved_sec and saved_usec.
 */
static int			time_saved = 0;
/*
 * holds the extension's notion of what the current time is while it is 
 * sending input actions to a client
 */
static struct timeval		current_time;
/*
 * holds the time when the extension should place the next fake input action
 * into the server's normal events queue
 */
static struct timeval		play_time;
/*
 * set to 1 when play_time is first set, cleared to 0 when the
 * client using the extension disconnects, or when XTestReset is called
 */
static char			play_clock = 0;
/*
 * holds the amount of time left until the next input action from the
 * input action array can be sent to the server
 */
static struct timeval		rtime;
/*
 * Set to 1 after the extension is done waiting for the correct time delay
 * for an input action to be sent to the server.  Remains a 1 until the time
 * delay for the next input action is computed.  Then set to 0 if the
 * extension has to wait for the correct time delay.
 */
static int			go_for_next = 1;
/*
 * needed to restore waitime if playback is to be aborted
 */
static struct timeval		*restorewait;
/*
 * tmon special command key
 *
 * To use the test monitor program (called tmon) efficiently, it is
 * desirable to have the extension be able to recognize a special "trigger"
 * key.  If the extension did not do this, tmon would have to have the
 * extension send all keyboard user input actions exclusively to tmon,
 * only to have tmon send them right back if they were not the command key.
 *
 * If the extension can recognize the command key, then tmon can let the
 * extension handle keyboard user input actions normally until the command
 * key is pressed (and released), and only then have the extension start
 * sending keyboard user input actions exclusively to tmon.
 *
 * Any key on the keyboard can be used for this command key.  It is most
 * convenient if it is a low-frequency key.  If you want to generate a
 * normal occurrance of this key to a client, just hit it twice.  Tmon
 * will recognize the first occurrance of the key, take control of the input
 * actions, and wait for certain keys.  If it sees another occurrance of the
 * command key, it will send one occurrance of the command key to the
 * extension, and go back to waiting.
 *
 * set and also referenced in device layer
 * XXX there should be a way to set this through the protocol
 */
KeyCode			xtest_command_key = 0;

/***************************************************************
 * function declarations
 ***************************************************************/

static void	parse_key_fake(
#if NeedFunctionPrototypes
			XTestKeyInfo	* /* fkey */
#endif
			);
static void	parse_motion_fake(
#if NeedFunctionPrototypes
			XTestMotionInfo	* /* fmotion */
#endif
			);
static void	parse_jump_fake(
#if NeedFunctionPrototypes
			XTestJumpInfo	* /* fjump */
#endif
			);
static void	parse_delay_fake(
#if NeedFunctionPrototypes
			XTestDelayInfo	* /* tevent */
#endif
			);
static void	send_ack(
#if NeedFunctionPrototypes
			ClientPtr	 /* client */
#endif
			);
static void	start_play_clock(
#if NeedFunctionPrototypes
			void
#endif
			);
static void	compute_action_time(
#if NeedFunctionPrototypes
			struct timeval	* /* rtime */
#endif
			);
static int	find_residual_time(
#if NeedFunctionPrototypes
			struct timeval	* /* rtime */
#endif
			);

static CARD16	check_time_event(
#if NeedFunctionPrototypes
			void
#endif
			);
static CARD32	current_ms(
#if NeedFunctionPrototypes
			struct timeval	* /* otime */
#endif
			);
static int	there_is_room(
#if NeedFunctionPrototypes
			int	/* actsize */
#endif
			);

/******************************************************************************
 *
 * 	stop_stealing_input
 *
 *	Stop stealing input actions.
 */
void
stop_stealing_input()
{
/*
 * put any code that you might need to stop stealing input actions here
 */
	if (packet_index != 0) 
	{
		/*
		 * if there is a partially full input action event waiting
		 * when this function is called, send it to the client
		 */
		flush_input_actions();
	}
}

/******************************************************************************
 *
 * 	steal_input
 *
 *	Start stealing input actions and sending them to the passed-in client.
 */
void
steal_input(client, mode)
/*
 * which client is to receive the input action events
 */
ClientPtr	client;
/*
 * what input action packing mode to use.  one of 0, XTestPACKED_MOTION,
 * or XTestPACKED_ACTIONS; optionally 'or'ed with XTestEXCLUSIVE,
 */
CARD32		mode;
{
	if (packet_index != 0) 
	{
		/*
		 * if there is a partially full input action event waiting
		 * when this function is called, send it to the client
		 */
		flush_input_actions();
	}
	else
	{	
		/*
		 * otherwise, set up a new input action event
		 */
		input_action_packet.type = XTestInputActionType;
		packet_index = 0;
	}
	/*
	 * set up the new input action packing mode
	 */
	packed_mode = mode & ~(XTestEXCLUSIVE);
	/*
	 * keep track of where the mouse is
	 */
	XTestGetPointerPos(&xtest_mousex, &xtest_mousey);
	/*
	 * keep track of which client is getting input actions
	 */
	current_xtest_client = client;
	/*
	 * find out what time it is
	 */
	X_GETTIMEOFDAY(&current_time);
	/*
	 * jump to the initial position of the mouse, using a device type of 0.
	 */
	XTestStealJumpData(xtest_mousex, xtest_mousey, 0);
}
	
/******************************************************************************
 *
 *	flush_input_actions
 *
 *	Write the input actions event to the current requesting client
 *	and re-initialize the input action event.
 */
void
flush_input_actions()
{
	/*
	 * pointer to the input action event
	 */
	char			*rep;
	/*
	 * loop index
	 */
	int			i;

	if (packet_index == 0)
	{
		/*
		 * empty input actions event 
		 */
		return;
	}
	else if (packet_index < XTestACTIONS_SIZE)
	{
		/*
		 * fill to the end of the input actions event with 0's
		 */
		for (i = packet_index; i <XTestACTIONS_SIZE; i++)
		{
			input_action_packet.actions[i] = 0;
		}
	}
	rep = (char *) (&input_action_packet);

	/*
	 * set the serial number of the input action event
	 */
	input_action_packet.sequenceNumber = current_xtest_client->sequence;
	/*
	 * send the input action event to the client
	 */
	WriteEventsToClient(current_xtest_client, 1, (xEvent *) rep);
	/*
	 * re-initialize the input action event
	 */
	input_action_event_full = 0;
	input_action_packet.type = XTestInputActionType;
 	packet_index = 0;
}	

/******************************************************************************
 *
 *	XTestStealJumpData
 *
 *	Create one or more input actions and put them in the input action
 *	event.  The input actions will be an (maybe) XTestDELAY_ACTION
 *	and an XTestJUMP_ACTION.
 */
void
XTestStealJumpData(jx, jy, dev_type)
/*
 * the x and y coordinates to jump to
 */
short	jx;
short	jy;
/*
 * which device caused the jump
 */
int	dev_type;
{	
	XTestJumpInfo 	*jmp_ptr;
	/*
	 * time delta (in ms) from previous event
	 */
	CARD16			tchar;

	/*
	 * Get the time delta from the previous event.  If needed,
	 * the check_time_event routine will put an XTestDELAY_ACTION
	 * type action in the input action event.
	 */
	tchar = check_time_event();
	if (!there_is_room(sizeof(XTestJumpInfo)))
	{
		/*
		 * If there isn't room in the input action event for
		 * an XTestJUMP_ACTION, then send that event to the
		 * client and start filling an empty one.
		 */
		flush_input_actions();
	}
	/*
	 * update the logical mouse position
	 */
	xtest_mousex = jx;
	xtest_mousey = jy;
	/*
	 * point jmp_ptr to the correct place in the input action event
	 */
	jmp_ptr = (XTestJumpInfo *)
		  &(input_action_packet.actions[packet_index]);
	/*
	 * compute the input action header
	 */
	jmp_ptr->header = (XTestPackDeviceID(dev_type) | XTestJUMP_ACTION);	
	/*
	 * set the x and y coordinates to jump to in the input action
	 */
	jmp_ptr->jumpx = jx;
	jmp_ptr->jumpy = jy;
	/*
	 * set the delay time in the input action
	 */
	jmp_ptr->delay_time = tchar;
	/*
	 * increment the packet index by the size of the input action
	 */
	packet_index = packet_index + sizeof(XTestJumpInfo);
	if (packed_mode == 0)
	{
		/*
		 * if input actions are not packed, send the input
		 * action event to the client
		 */
		flush_input_actions();
	}
}	

/******************************************************************************
 *
 *	current_ms
 *
 *	Returns the number of milliseconds from the passed-in time to the
 *	current time, and then updates the passed-in time to the current time.
 */
static CARD32
current_ms(otime)
struct timeval	*otime;
{	
	struct timeval	tval;
	unsigned long	the_ms;
	unsigned long	sec;
	unsigned long	usec;

	/*
	 * get the current time
	 */
	X_GETTIMEOFDAY(&tval);
	if (tval.tv_usec < otime->tv_usec)
	{
		/*
		 * borrow a second's worth of microseconds if needed
		 */
		usec = tval.tv_usec - otime->tv_usec + 1000000;
		sec = tval.tv_sec - 1 - otime->tv_sec;
	}
	else
	{
		usec = tval.tv_usec - otime->tv_usec;
		sec = tval.tv_sec - otime->tv_sec;
	}
	/*
	 * update the passed-in time to the new time
	 */
	*otime = tval;
	/*
	 * compute the number of milliseconds contained in
	 * 'sec' seconds and 'usec' microseconds
	 */
	the_ms = (sec * 1000000L + usec) / 1000L;
	return (the_ms);
}

/******************************************************************************
 *
 *	check_time_event
 *
 *	If time delta is > XTestSHORT_DELAY_TIME then insert a time event
 *	and return 0; else return the delay time.
 */
static CARD16
check_time_event()
{
	CARD32		tstamp;
	CARD16		tchar;
	XTestDelayInfo	*tptr;

	/*
	 * get the number of milliseconds between input actions
	 */
	tstamp = current_ms(&current_time);
	/*
	 * if the number of milliseconds is too large to fit in a CARD16,
	 * then add a XTestDELAY_ACTION to the input action event.
	 */
	if (tstamp > XTestSHORT_DELAY_TIME)
	{
		/*
		 * If there isn't room in the input action event for
		 * an XTestDELAY_ACTION, then send that event to the
		 * client and start filling an empty one.
		 */
		if (!there_is_room(sizeof(XTestDelayInfo)))
		{
			flush_input_actions();
		}
		/*
		 * point tptr to the correct place in the input action event
		 */
		tptr = (XTestDelayInfo *)
		       (&(input_action_packet.actions[packet_index]));
		/*
		 * compute the input action header
		 */
		tptr->header = XTestPackDeviceID(XTestDELAY_DEVICE_ID) |
			       XTestDELAY_ACTION;
		/*
		 * set the delay time in the input action
		 */
		tptr->delay_time = tstamp;
		/*
		 * increment the packet index by the size of the input action
		 */
		packet_index = packet_index + (sizeof(XTestDelayInfo));
		if (packed_mode != XTestPACKED_ACTIONS) 
		{
			/*
			 * if input actions are not packed, send the input
			 * action event to the client
			 */
			flush_input_actions();
		}
		/*
		 * set the returned delay time to 0
		 */
		tchar = 0;
	}
	else
	{
		/*
		 * set the returned delay time to the computed delay time
		 */
		tchar = tstamp;
	}
	return(tchar);
}

/******************************************************************************
 *
 *	there_is_room
 *
 *	Checks if there is room in the input_action_packet for an input action
 *	of the size actsize bytes.  Returns 1 if there is space, 0 otherwise.
 *
 */
static int
there_is_room(actsize)
/*
 * the number of bytes of space needed
 */
int	actsize;
{
	if ((packet_index + actsize) > XTestACTIONS_SIZE)
	{ 
		input_action_event_full = 1;
		return(0);
	}
	else
	{
		return(1);
	}
}

/******************************************************************************
 *
 *	XTestStealMotionData
 *
 *	Put motion information from the locator into an input action.
 *
 *	called from x_hil.c
 */
void
XTestStealMotionData(dx, dy, dev_type, mx, my)
/*
 * the x and y delta motion of the locator
 */
short	dx;
short	dy;
/*
 * which locator did the moving
 */
int	dev_type;
/*
 * the x and y position of the locator before the delta motion
 */
short	mx;
short	my;
{
	/*
	 * pointer to a XTestMOTION_ACTION input action
	 */
	XTestMotionInfo	*fm;
	/*
	 * time delta from previous event
	 */
	CARD16			tchar;

	/*
	 * if the current position of the locator is not the same as
	 * the logical position, then update the logical position
	 */
	if ((mx != xtest_mousex) || (my != xtest_mousey))
	{
		XTestStealJumpData(mx, my, dev_type);
	}
	/*
	 * if the delta motion is outside the range that can
	 * be held in a motion input action, use a jump input action
	 */
	if ((dx > XTestMOTION_MAX) || (dx < XTestMOTION_MIN) ||
	    (dy > XTestMOTION_MAX) || (dy < XTestMOTION_MIN))
	{
		XTestStealJumpData((xtest_mousex + dx),
				   (xtest_mousey + dy), dev_type);
	}
	else
	{ 
		/*
		 * compute the new logical position of the mouse
		 */
		xtest_mousex += dx;
		xtest_mousey += dy;
		/*
		 * Get the time delta from the previous event.  If needed,
		 * the check_time_event routine will put an XTestDELAY_ACTION
		 * type action in the input action event.
		 */
		tchar = check_time_event();
		/*
		 * If there isn't room in the input action event for
		 * an XTestDELAY_ACTION, then send that event to the
		 * client and start filling an empty one.
		 */
		if (!there_is_room(sizeof(XTestMotionInfo)))
		{
			flush_input_actions();
		/*
		 * point fm to the correct place in the input action event
		 */
		}
		fm = (XTestMotionInfo *)
		     &(input_action_packet.actions[packet_index]);
		/*
		 * compute the input action header
		 */
		fm->header = XTestMOTION_ACTION;
		if (dx < 0)	
		{  
			fm->header |= XTestX_NEGATIVE;
			dx = abs(dx);
		}
		if (dy < 0)   
		{  
			fm->header |= XTestY_NEGATIVE;
			dy = abs(dy);
		}
		fm->header |= XTestPackDeviceID(dev_type);
		/*
		 * compute the motion data byte
		 */
		fm->motion_data = XTestPackYMotionValue(dy);
		fm->motion_data |= XTestPackXMotionValue(dx);
		/*
		 * set the delay time in the input action
		 */
		fm->delay_time = tchar;
		/*
		 * increment the packet index by the size of the input action
		 */
		packet_index = packet_index + sizeof(XTestMotionInfo);
		if (packed_mode == 0)
		{
			/*
			 * if input actions are not packed, send the input
			 * action event to the client
			 */
			flush_input_actions();
		}

	}   
}

/******************************************************************************
 *
 *	XTestStealKeyData
 *
 * 	Place this key data in the input_action_packet.
 *
 */
Bool
XTestStealKeyData(keycode, keystate, dev_type, locx, locy)
/*
 * which key/button moved
 */
CARD8	keycode;
/*
 * whether the key/button was pressed or released
 */
char	keystate;
/*
 * which device caused the input action
 */
int	dev_type;
/*
 * the x and y coordinates of the locator when the action happenned
 */
short	locx;
short	locy;
{
	/*
	 * pointer to key/button motion input action
	 */
	XTestKeyInfo	*kp;
	/*
	 * time delta from previous event
	 */
	CARD16			tchar;
	char		keytrans;

	/*
	 * update the logical position of the locator if the physical position
	 * of the locator is not the same as the logical position.
	 */
	if ((locx != xtest_mousex) || (locy != xtest_mousey))
	{
		XTestStealJumpData(locx, locy, dev_type);
	}
	/*
	 * Get the time delta from the previous event.  If needed,
	 * the check_time_event routine will put an XTestDELAY_ACTION
	 * type action in the input action event.
	 */
	tchar = check_time_event();
	if (!there_is_room(sizeof(XTestKeyInfo)))
	{
		/*
		 * If there isn't room in the input action event for
		 * an XTestDELAY_ACTION, then send that event to the
		 * client and start filling an empty one.
		 */
		flush_input_actions();
	}
	/*
	 * point kp to the correct place in the input action event
	 */
	kp = (XTestKeyInfo *)
	     (&(input_action_packet.actions[packet_index]));
	/*
	 * compute the input action header
	 */
	kp->header = XTestPackDeviceID(dev_type);
	if ((keystate == KeyRelease) || (keystate == ButtonRelease))
	{
		keytrans = XTestKEY_UP;
	}
	else if ((keystate == KeyPress) || (keystate == ButtonPress))
	{
		keytrans = XTestKEY_DOWN;
	}
	else
	{
		printf("%s: invalid key/button state %d.\n",
		       XTestEXTENSION_NAME,
		       keystate);
	}
	kp->header = kp->header | keytrans | XTestKEY_ACTION;
	/*
	 * set the keycode in the input action
	 */
	kp->keycode = keycode;
	/*
	 * set the delay time in the input action
	 */
	kp->delay_time = tchar;
	/*
	 * increment the packet index by the size of the input action
	 */
	packet_index = packet_index + sizeof(XTestKeyInfo);
	/*
	 * if the command key has been released or input actions are not
	 * packed, send the input action event to the client
	 */
 	if(((keycode == xtest_command_key) && (keystate == KeyRelease)) ||
	   (packed_mode != XTestPACKED_ACTIONS))
	{	
		flush_input_actions();
	}
	/* return TRUE if the event should be passed on to DIX */
	if (exclusive_steal)
		return ((keystate == KeyRelease) &&
			(keycode == xtest_command_key));
	else
		return ((keystate != KeyRelease) ||
			(keycode != xtest_command_key));
}

/******************************************************************************
 *
 *	parse_fake_input
 *
 *	Parsing routine for a XTestFakeInput request.  It will take a request
 *	and parse its contents into the input action array.  Eventually the
 *	XTestProcessInputAction routine will be called to take input actions
 *	from the input action array and send them to the server to be handled.
 */
void
parse_fake_input(client, req)
/*
 * which client did the XTestFakeInput request
 */
ClientPtr	client;
/*
 * a pointer to the xTestFakeInputReq structure sent by the client
 */
char		*req;
{	
	/*
	 * if set to 1, done processing input actions from the request
	 */
	int	        	done = 0;
	/*
	 * type of input action
	 */
	CARD8			action_type;
	/*
	 * device type
	 */
	CARD8			dev_type;
	/*
	 * pointer to an xTestFakeInputReq structure
	 */
	xTestFakeInputReq	*request;
	/*
	 * holds the index into the action list in the request
	 */
	int			parse_index;	

	/*
	 * get a correct-type pointer to the client-supplied request data
	 */
	request = (xTestFakeInputReq *) req;
	/*
	 * save the acknowledge requested state for use in
	 * XTestProcessInputAction
	 */
	acknowledge = request->ack;
	/*
	 * set up an index into the action list in the request
	 */
	parse_index = 0;
	if (write_index >= ACTION_ARRAY_SIZE)
	{
		/*
		 * if the input action array is full, don't add any more
		 */
		done = 1;
	}
	while (!done)
	{ 
		/*
		 * get the type of input action in the list
		 */
		action_type = (request->action_list[parse_index])
			      & XTestACTION_TYPE_MASK;
		/*
		 * get the type of device in the list
		 */
		dev_type = XTestUnpackDeviceID(request->action_list[parse_index]);
		/*
		 * process the input action appropriately
		 */
		switch (action_type)
		{ 
		case XTestKEY_ACTION:
			parse_key_fake((XTestKeyInfo *)
				       &(request->action_list[parse_index]));
			parse_index = parse_index + sizeof(XTestKeyInfo);
			break;
		case XTestMOTION_ACTION:
			parse_motion_fake((XTestMotionInfo *)
					  &(request->action_list[parse_index]));
			parse_index = parse_index + sizeof(XTestMotionInfo);
			break;
		case XTestJUMP_ACTION:
			parse_jump_fake((XTestJumpInfo *)
					&(request->action_list[parse_index]));
			parse_index = parse_index + sizeof(XTestJumpInfo);
			break;
		case XTestDELAY_ACTION:
			if (dev_type == XTestDELAY_DEVICE_ID)
			{ 
				parse_delay_fake((XTestDelayInfo *)
						 &(request->action_list[parse_index]));
				parse_index = parse_index +
					      sizeof(XTestDelayInfo);
			}
			else
			{ 
				/*
				 * An invalid input action header byte has
				 * been detected, so there are no more
				 * input actions in this request.
				 * The intended invalid action header byte
				 * for this case should have a value of 0.
				 */
				done = 1;
			}
			break;
		}
		if (parse_index >= XTestMAX_ACTION_LIST_SIZE)
		{
			/*
			 * entire XTestFakeInput request has been processed
			 */
			done = 1;
		}
		if (write_index >= ACTION_ARRAY_SIZE) 
		{
			/*
			 * no room in the input actions array
			 */
			done = 1;
		}
	}
	if (write_index > read_index)
	{ 
		/*
		 * there are fake input actions in the input action array
		 * to be given to the server
		 */
		playback_on = 1;
		playback_client = client;
	} 
}

/******************************************************************************
 *
 *	parse_key_fake
 *
 *	Called from parse_fake_input.
 *
 *	Copy the fake key input action from its packed form into the array of
 *	pending input events.
 */
static void
parse_key_fake(fkey)
XTestKeyInfo	*fkey;
{	
	action_array[write_index].type = XTestKEY_ACTION;
	action_array[write_index].device = XTestUnpackDeviceID(fkey->header);
	action_array[write_index].keycode = fkey->keycode;
	action_array[write_index].keystate = fkey->header & XTestKEY_STATE_MASK;
	action_array[write_index].delay_time = fkey->delay_time;
	write_index++;
}

/******************************************************************************
 *
 *	parse_motion_fake
 *
 *	Called from parse_fake_input.
 *
 *	Copy the fake motion input action from its packed form into the array of
 *	pending input events.
 */
static void
parse_motion_fake(fmotion)
XTestMotionInfo	*fmotion;
{	
	int	dx;
	int	dy;

	dx = (XTestUnpackXMotionValue(fmotion->motion_data));
	dy = (XTestUnpackYMotionValue(fmotion->motion_data));
	if (((fmotion->header) & XTestX_SIGN_BIT_MASK) == XTestX_NEGATIVE)
	{
		pmousex -= dx;
	}
	else
	{
		pmousex += dx;
	}
	if (((fmotion->header) & XTestY_SIGN_BIT_MASK) == XTestY_NEGATIVE)
	{
		pmousey -= dy;
	}
	else 
	{
		pmousey += dy;
	}
	action_array[write_index].type = XTestJUMP_ACTION;
	action_array[write_index].device = XTestUnpackDeviceID(fmotion->header);
	action_array[write_index].x = pmousex;
	action_array[write_index].y = pmousey;
	action_array[write_index].delay_time = fmotion->delay_time;
	write_index++;
}

/******************************************************************************
 *
 *	parse_jump_fake
 *
 *	Called from parse_fake_input.
 *
 *	Copy the fake jump input action from its packed form into the array of
 *	pending input events.
 */
static void
parse_jump_fake(fjump)
XTestJumpInfo	*fjump;
{
	pmousex = fjump->jumpx;
	pmousey = fjump->jumpy;
	action_array[write_index].type = XTestJUMP_ACTION;
	action_array[write_index].device = XTestUnpackDeviceID(fjump->header);
	action_array[write_index].x = pmousex;
	action_array[write_index].y = pmousey;
	action_array[write_index].delay_time = fjump->delay_time;
	write_index++;
}

/******************************************************************************
 *
 *	parse_delay_fake
 *
 *	Called from parse_fake_input.
 *
 *	Copy the fake delay input action from its packed form into the array of
 *	pending input events.
 */
static void
parse_delay_fake(tevent)
XTestDelayInfo	*tevent;
{
	action_array[write_index].type = XTestDELAY_ACTION;
	action_array[write_index].delay_time = tevent->delay_time;
	write_index++;
}

/******************************************************************************
 *
 *	XTestComputeWaitTime
 *
 *	Compute the amount of time the server should wait before sending the
 *	next monitor event in playback mode.
 */
void
XTestComputeWaitTime(waittime)
struct timeval	*waittime;
{	
	/*
	 * The playback_on flag is set to 1 in parse_fake_input.  It is set to
	 * 0 in XTestProcessInputAction if the server has replayed all input
	 * actions.
	 */
	if (playback_on)
	{  
		if (!play_clock)
		{
			/*
			 * if the playback clock has never been set,
			 * then do it now
			 */
			start_play_clock();
		}
		/*
		 * We need to save the waittime the first time through.  This
		 * is a value the server uses, and we have to restore it when
		 * all of the input actions are processed by the server.
		 */
		if (!time_saved)
		{
			saved_sec = waittime->tv_sec;
			saved_usec = waittime->tv_usec; 
			time_saved = 1;
		}	
		if (go_for_next) 
		{
			/*
			 * if we just processed an input action, figure out
			 * how long to wait for the next input action
			 */
			compute_action_time(&rtime);
		}
		else  
		{
			/*
			 * else just find out how much more time to wait
			 * on the current input action
			 */
			(void)find_residual_time(&rtime);
		}
		waittime->tv_sec = rtime.tv_sec;
		waittime->tv_usec = rtime.tv_usec;
	}
}

/******************************************************************************
 *
 *	XTestProcessInputAction
 *
 *	If there are any input actions in the input action array,
 *	then take one out and process it.
 *
 */
int
XTestProcessInputAction(readable, waittime)
/*
 * This is the value that a 'select' function returned just before this
 * routine was called.  If the select timed out, this value will be 0.
 *
 * This extension modifies the select call's timeout value to cause the
 * select to time out when the next input action is ready to given to
 * the server.  This routine is called immediately after the select, to 
 * give it a chance to process an input action.  If we have an input action
 * to process and the only reason that the select returned was because it
 * timed out, then we change the select value to 1 and return 1 instead of 0.
 */
int		readable;
/*
 * this is the timeout value that the select was called with
 */
struct timeval	*waittime;
{	
int mousex, mousey;
	/*
	 * if playback_on is 0, then the input action array is empty
	 */
	if (playback_on)
	{ 
		restorewait = waittime;
		/*
		 * figure out if we need to wait for the next input action
		 */
		if (find_residual_time(&rtime) > 0) 
		{
			/*
			 * still have to wait before processing the current
			 * input action
			 */
			go_for_next = 0;
		}
		else 
		{
			/*
			 * don't have to wait any longer before processing
			 * the current input action
			 */
			go_for_next = 1;
		}
		/*
		 * if we have an input action to process and the only reason
		 * that the select returned was because it timed out, then we
		 * change the select value to 1 and return 1 instead of 0
		 */
		if (readable == 0) 
		{
			readable++;			
		}
		/*
		 * if we don't need to wait, then get an input action from
		 * the input action array and process it
		 */
		if (go_for_next)
		{  
			/*
			 * There are three possible types of input actions in
			 * the input action array (motion input actions are
			 * converted to jump input actions before being put
			 * into the input action array).  Delay input actions 
			 * are processed by the compute_action_time function
			 * which is called from XTestComputeWaitTime.  The
			 * other two types of input actions are processed here.
			 */
			if (action_array[read_index].type == XTestJUMP_ACTION)
			{	
				XTestJumpPointer(
					action_array[read_index].x, 
					action_array[read_index].y, 
					action_array[read_index].device);
				mx = action_array[read_index].x;
				my = action_array[read_index].y;
			}
			if (action_array[read_index].type == XTestKEY_ACTION)
			    {
			    GetSpritePosition(&mousex, &mousey);
			    XTestGenerateEvent(
				     action_array[read_index].device, 
				     action_array[read_index].keycode, 
				     action_array[read_index].keystate,
				     mousex,
				     mousey);
			    }
			read_index++;
			/*
			 * if all input actions are processed, then restore 
			 * the server state 
			 */
			if (read_index >= write_index)
			{ 
				waittime->tv_sec = saved_sec;
				waittime->tv_usec = saved_usec;
				time_saved = 0;
				playback_on = 0;
				if (acknowledge) 
				{ 
					/*
					 * if the playback client is waiting
					 * for an xTestFakeAck event, send
					 * it to him
					 */
					send_ack(playback_client);		
					acknowledge = 0;
				}
				write_index = 0;
				read_index = 0;
				playback_client = (ClientPtr) NULL;
				play_clock = 0;
			}
		}
	}
	return(readable);
}

/******************************************************************************
 *
 *	send_ack
 *
 *	send an xTestFakeAck event to the client
 */
static void
send_ack(client)
ClientPtr	client;
{
	xTestFakeAckEvent  rep;

	/*
	 * set the serial number of the xTestFakeAck event
	 */
	rep.sequenceNumber = client->sequence;
	rep.type = XTestFakeAckType;
	WriteEventsToClient(client, 1, (xEvent *) &rep);		
}		

/******************************************************************************
 *
 *	start_play_clock
 *
 *	start the clock for play back.
 */
static void
start_play_clock()
{
	X_GETTIMEOFDAY(&play_time);
	/*
	 * flag that play_time is valid
	 */
	play_clock = 1;
}

/******************************************************************************
 *
 *	compute_action_time
 *
 *	Set the play clock to the time when the next input action should be put
 *	into the server's input queue.  Fill the rtime structure with values
 *	for the delta until the time for the next input action.
 */
static void
compute_action_time(rtime)
struct timeval	*rtime;
{
	/*
	 * holds the delay time in milliseconds
	 */
	unsigned long	dtime;
	/*
	 * holds the number of microseconds in the sum of the dtime value
	 * and the play_time value
	 */
	unsigned long	tot_usec;
	/*
	 * holds the number of seconds and microseconds in the
	 * dtime value
	 */
	unsigned long 	sec;
	unsigned long 	usec;
	/*
	 * holds the current time
	 */
	struct timeval	btime;

	/*
	 * Put the time from the current input action in dtime
	 */
	dtime = action_array[read_index].delay_time;
	/*
	 * If the current input action is a delay input action,
	 * add in the time from the following input action.
	 */
	if ((action_array[read_index].type == XTestDELAY_ACTION) &&
	    ((read_index + 1) < write_index))
	{  
		read_index++;
		dtime = dtime + action_array[read_index].delay_time;
	}
	/*
	 * compute the number of seconds and microseconds in the
	 * dtime value
	 */
  	sec = dtime / 1000;
  	usec = (dtime % 1000) * 1000;
	/*
	 * get the current time in btime
	 */
	X_GETTIMEOFDAY(&btime);
	/*
	 * compute the number of microseconds in the sum of the dtime value
	 * and the current usec value
	 */
	tot_usec = btime.tv_usec + usec;
	/*
	 * if it is greater than one second's worth, adjust the seconds
	 */
	if (tot_usec >= 1000000)
	{ 
		tot_usec -= 1000000;
		sec++;
	}
	play_time.tv_usec = tot_usec;
	play_time.tv_sec = btime.tv_sec + sec;
	/*
	 * put the time until the next input action in rtime
	 */
	rtime->tv_sec = sec;
	rtime->tv_usec = usec;
}

/******************************************************************************
 *
 *	find_residual_time
 *
 *	Find the time interval from the current time to the value in play_time.
 *	This is the time to wait till putting the next input action into the
 *	server's input queue.  If the time is already up, reset play_time to
 *	the current time.
 */
static int
find_residual_time(the_residual)
struct timeval	*the_residual;
{
	/*
	 * if > 0, there is time to wait.  If < 0, then don't wait
	 */
	int		wait = 1;
	/*
	 * holds the current time
	 */
	struct timeval	btime;
	/*
	 * holds the current time in seconds and microseconds
	 */
	unsigned long	bsec;
	unsigned long	busec;
	/*
	 * holds the playback time in seconds and microseconds
	 */
	unsigned long	psec;
	unsigned long	pusec;

	/*
	 * get the current time in btime
	 */
	X_GETTIMEOFDAY(&btime);
	/*
	 * get the current time in seconds and microseconds
	 */
	bsec = btime.tv_sec;
	busec = btime.tv_usec;
	/*
	 * get the playback time in seconds and microseconds
	 */
	psec = play_time.tv_sec;
	pusec = play_time.tv_usec;
	/*
	 * if the current time is already later than the playback time,
	 * we don't need to wait
	 */
	if (bsec > psec)	
	{
	    wait = -1;
	}
	else
	{ 
		if (bsec == psec)
		{ 
			/*
			 * if the current and playback times have the same
			 * second value, then compare the microsecond values
			 */
			if ( busec >= pusec) 
			{ 
				/*
				 * if the current time is already later than
				 * the playback time, we don't need to wait
				 */
				wait = -1;
			}
			else
			{ 
				the_residual->tv_usec = pusec - busec;
				the_residual->tv_sec = 0;
			}
		}
		else	
		{ 
			if (busec > pusec)
			{ 
				/*
				 * 'borrow' a second's worth of microseconds
				 * from the seconds left to wait
				 */
				the_residual->tv_usec = 1000000 - busec + pusec;
				psec--;
				the_residual->tv_sec = psec - bsec;
			}
			else
			{ 
				the_residual->tv_sec = psec - bsec;
				the_residual->tv_usec = pusec - busec;
			}
		}
	}
	if (wait < 0)
	{ 
		/*
		 * if don't need to wait, set the playback time
		 * to the current time
		 */
		X_GETTIMEOFDAY(&play_time);
		/*
		 * set the time to wait to 0
		 */
		the_residual->tv_sec = 0;
		the_residual->tv_usec = 0;
	}
	return(wait);
}
	
/******************************************************************************
 *
 *	abort_play_back
 */
void
abort_play_back()
{
	/*
	 * If we were playing back input actions at the time of the abort,
	 * restore the original wait time for the select in the main wait
	 * loop of the server
	 */
	if (playback_on)
	{
		restorewait->tv_sec = saved_sec;
		restorewait->tv_usec = saved_usec;
	}
	/*
	 * make the input action array empty
	 */
	read_index = 0;
	write_index = 0;
	/*
	 * we are no longer playing back anything
	 */
	playback_on = 0;
	play_clock = 0;
	go_for_next = 1;
	/*
	 * there is no valid wait time saved any more
	 */
	time_saved = 0;
	/*
	 * there are no valid clients using this extension
	 */
	playback_client = (ClientPtr) NULL;
	current_xtest_client = (ClientPtr) NULL;
}

/******************************************************************************
 *
 *	return_input_array_size
 *
 *	Return the number of input actions in the input action array.
 */
void
return_input_array_size(client)
/*
 * which client to send the reply to
 */
ClientPtr	client;
{
	xTestQueryInputSizeReply  rep;

	rep.type = X_Reply;
	/*
	 * set the serial number of the reply
	 */
	rep.sequenceNumber = client->sequence;
	rep.length = 0;
	rep.size_return = ACTION_ARRAY_SIZE;
	WriteReplyToClient(client,
			   sizeof(xTestQueryInputSizeReply),
			   (pointer) &rep);		
}		
