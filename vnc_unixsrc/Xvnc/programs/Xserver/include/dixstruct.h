/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: dixstruct.h /main/43 1996/12/15 21:25:06 rws $ */
/* $XFree86: xc/programs/Xserver/include/dixstruct.h,v 3.8 1996/12/24 02:27:28 dawes Exp $ */

#ifndef DIXSTRUCT_H
#define DIXSTRUCT_H

#include "dix.h"
#include "resource.h"
#include "cursor.h"
#include "gc.h"
#include "pixmap.h"
#include <X11/Xmd.h>

/*
 * 	direct-mapped hash table, used by resource manager to store
 *      translation from client ids to server addresses.
 */

#ifdef DEBUG
#define MAX_REQUEST_LOG 100
#endif

extern CallbackListPtr ClientStateCallback;

typedef struct {
    ClientPtr 		client;
    xConnSetupPrefix 	*prefix; 
    xConnSetup  	*setup;
} NewClientInfoRec;

typedef void (*ReplySwapPtr) (
#if NeedNestedPrototypes
		ClientPtr	/* pClient */,
		int		/* size */,
		void *		/* pbuf */
#endif
);

extern void ReplyNotSwappd (
#if NeedNestedPrototypes
		ClientPtr	/* pClient */,
		int		/* size */,
		void *		/* pbuf */
#endif
);

typedef enum {ClientStateInitial,
	      ClientStateAuthenticating,
	      ClientStateRunning,
	      ClientStateRetained,
	      ClientStateGone,
	      ClientStateCheckingSecurity,
	      ClientStateCheckedSecurity} ClientState;

typedef struct _Client {
    int         index;
    Mask        clientAsMask;
    pointer     requestBuffer;
    pointer     osPrivate;	/* for OS layer, including scheduler */
    Bool        swapped;
    ReplySwapPtr pSwapReplyFunc;
    XID         errorValue;
    int         sequence;
    int         closeDownMode;
    int         clientGone;
    int         noClientException;	/* this client died or needs to be
					 * killed */
    DrawablePtr lastDrawable;
    Drawable    lastDrawableID;
    GCPtr       lastGC;
    GContext    lastGCID;
    pointer    *saveSet;
    int         numSaved;
    pointer     screenPrivate[MAXSCREENS];
    int         (**requestVector) (
#if NeedNestedPrototypes
		ClientPtr /* pClient */
#endif
);
    CARD32	req_len;		/* length of current request */
    Bool	big_requests;		/* supports large requests */
    int		priority;
    ClientState clientState;
    DevUnion	*devPrivates;
#ifdef XKB
    unsigned short	xkbClientFlags;
    unsigned short	mapNotifyMask;
    unsigned short	newKeyboardNotifyMask;
    unsigned short	vMajor,vMinor;
    KeyCode		minKC,maxKC;
#endif

#ifdef DEBUG
    unsigned char requestLog[MAX_REQUEST_LOG];
    int         requestLogIndex;
#endif
#ifdef LBX
    int		(*readRequest)(
#if NeedNestedPrototypes
	ClientPtr /*client*/
#endif
);
#endif
    unsigned long replyBytesRemaining;
#ifdef XCSECURITY
    XID		authId;
    unsigned int trustLevel;
    pointer (* CheckAccess)(
#if NeedNestedPrototypes
	    ClientPtr /*pClient*/,
	    XID /*id*/,
	    RESTYPE /*classes*/,
	    Mask /*access_mode*/,
	    pointer /*resourceval*/
#endif
);
#endif
#ifdef XAPPGROUP
    struct _AppGroupRec*	appgroup;
#endif
    struct _FontResolution * (*fontResFunc) (    /* no need for font.h */
#if NeedNestedPrototypes
		ClientPtr	/* pClient */,
		int *		/* num */
#endif
);
}           ClientRec;

/* This prototype is used pervasively in Xext, dix */
#if NeedFunctionPrototypes
#define DISPATCH_PROC(func) int func(ClientPtr /* client */)
#else
#define DISPATCH_PROC(func) int func(/* ClientPtr client */)
#endif

typedef struct _WorkQueue {
    struct _WorkQueue *next;
    Bool        (*function) (
#if NeedNestedPrototypes
		ClientPtr	/* pClient */,
		pointer		/* closure */
#endif
);
    ClientPtr   client;
    pointer     closure;
}           WorkQueueRec;

extern TimeStamp currentTime;
extern TimeStamp lastDeviceEventTime;

extern int CompareTimeStamps(
#if NeedFunctionPrototypes
    TimeStamp /*a*/,
    TimeStamp /*b*/
#endif
);

extern TimeStamp ClientTimeToServerTime(
#if NeedFunctionPrototypes
    CARD32 /*c*/
#endif
);

typedef struct _CallbackRec {
  CallbackProcPtr proc;
  pointer data;
  Bool deleted;
  struct _CallbackRec *next;
} CallbackRec, *CallbackPtr;

typedef struct _CallbackList {
  CallbackFuncsRec funcs;
  int inCallback;
  Bool deleted;
  int numDeleted;
  CallbackPtr list;
} CallbackListRec;

/* proc vectors */

extern int (* InitialVector[3]) (
#if NeedNestedPrototypes
    ClientPtr /*client*/
#endif
);

extern int (* ProcVector[256]) (
#if NeedNestedPrototypes
    ClientPtr /*client*/
#endif
);

extern int (* SwappedProcVector[256]) (
#if NeedNestedPrototypes
    ClientPtr /*client*/
#endif
);

#ifdef K5AUTH
extern int (*k5_Vector[256])() =
#if NeedNestedPrototypes
    ClientPtr /*client*/
#endif
);
#endif

extern void (* ReplySwapVector[256]) ();

extern int ProcBadRequest(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

#endif				/* DIXSTRUCT_H */
