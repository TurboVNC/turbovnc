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

/* THIS IS NOT AN X CONSORTIUM STANDARD */

/* $XConsortium: shm.c,v 1.25 95/04/06 16:00:55 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/Xext/shm.c,v 3.8 1997/01/18 06:52:59 dawes Exp $ */

#include <sys/types.h>
#ifndef Lynx
#include <sys/ipc.h>
#include <sys/shm.h>
#else
#include <ipc.h>
#include <shm.h>
#endif
#define NEED_REPLIES
#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "resource.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "gcstruct.h"
#include "extnsionst.h"
#include "servermd.h"
#define _XSHM_SERVER_
#include "shmstr.h"
#include "Xfuncproto.h"

typedef struct _ShmDesc {
    struct _ShmDesc *next;
    int shmid;
    int refcnt;
    char *addr;
    Bool writable;
    unsigned long size;
} ShmDescRec, *ShmDescPtr;

static void miShmPutImage(XSHM_PUT_IMAGE_ARGS);
static void fbShmPutImage(XSHM_PUT_IMAGE_ARGS);
static PixmapPtr fbShmCreatePixmap(XSHM_CREATE_PIXMAP_ARGS);
static int ShmDetachSegment(
#if NeedFunctionPrototypes
    pointer		/* value */,
    XID			/* shmseg */
#endif
    );
static void ShmResetProc(
#if NeedFunctionPrototypes
    ExtensionEntry *	/* extEntry */
#endif
    );
static void SShmCompletionEvent(
#if NeedFunctionPrototypes
    xShmCompletionEvent * /* from */,
    xShmCompletionEvent * /* to */
#endif
    );

static DISPATCH_PROC(ProcShmAttach);
static DISPATCH_PROC(ProcShmCreatePixmap);
static DISPATCH_PROC(ProcShmDetach);
static DISPATCH_PROC(ProcShmDispatch);
static DISPATCH_PROC(ProcShmGetImage);
static DISPATCH_PROC(ProcShmGetImage);
static DISPATCH_PROC(ProcShmGetImage);
static DISPATCH_PROC(ProcShmPutImage);
static DISPATCH_PROC(ProcShmQueryVersion);
static DISPATCH_PROC(SProcShmAttach);
static DISPATCH_PROC(SProcShmCreatePixmap);
static DISPATCH_PROC(SProcShmDetach);
static DISPATCH_PROC(SProcShmDispatch);
static DISPATCH_PROC(SProcShmGetImage);
static DISPATCH_PROC(SProcShmPutImage);
static DISPATCH_PROC(SProcShmQueryVersion);

static unsigned char ShmReqCode;
static int ShmCompletionCode;
static int BadShmSegCode;
static RESTYPE ShmSegType, ShmPixType;
static ShmDescPtr Shmsegs;
static Bool sharedPixmaps;
static int pixmapFormat;
static int shmPixFormat[MAXSCREENS];
static ShmFuncsPtr shmFuncs[MAXSCREENS];
static ShmFuncs miFuncs = {NULL, miShmPutImage};
static ShmFuncs fbFuncs = {fbShmCreatePixmap, fbShmPutImage};

#define VERIFY_SHMSEG(shmseg,shmdesc,client) \
{ \
    shmdesc = (ShmDescPtr)LookupIDByType(shmseg, ShmSegType); \
    if (!shmdesc) \
    { \
	client->errorValue = shmseg; \
	return BadShmSegCode; \
    } \
}

#define VERIFY_SHMPTR(shmseg,offset,needwrite,shmdesc,client) \
{ \
    VERIFY_SHMSEG(shmseg, shmdesc, client); \
    if ((offset & 3) || (offset > shmdesc->size)) \
    { \
	client->errorValue = offset; \
	return BadValue; \
    } \
    if (needwrite && !shmdesc->writable) \
	return BadAccess; \
}

#define VERIFY_SHMSIZE(shmdesc,offset,len,client) \
{ \
    if ((offset + len) > shmdesc->size) \
    { \
	return BadAccess; \
    } \
}


#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/signal.h>

static Bool badSysCall = FALSE;

static void
SigSysHandler(signo)
int signo;
{
    badSysCall = TRUE;
}

static Bool CheckForShmSyscall()
{
    void (*oldHandler)();
    int shmid = -1;

    /* If no SHM support in the kernel, the bad syscall will generate SIGSYS */
    oldHandler = signal(SIGSYS, SigSysHandler);

    badSysCall = FALSE;
    shmid = shmget(IPC_PRIVATE, 4096, IPC_CREAT);
    /* Clean up */
    if (shmid != -1)
    {
	shmctl(shmid, IPC_RMID, (struct shmid_ds *)NULL);
    }
    signal(SIGSYS, oldHandler);
    return(!badSysCall);
}
#endif
    
void
ShmExtensionInit()
{
    ExtensionEntry *extEntry;
    int i;

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    if (!CheckForShmSyscall())
    {
	ErrorF("MIT-SHM extension disabled due to lack of kernel support\n");
	return;
    }
#endif

#ifdef INTERNAL_VS_EXTERNAL_PADDING
    sharedPixmaps = xFalse;
    pixmapFormat = 0;
#else
    sharedPixmaps = xTrue;
    pixmapFormat = shmPixFormat[0];
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	if (!shmFuncs[i])
	    shmFuncs[i] = &miFuncs;
	if (!shmFuncs[i]->CreatePixmap)
	    sharedPixmaps = xFalse;
	if (shmPixFormat[i] && (shmPixFormat[i] != pixmapFormat))
	{
	    sharedPixmaps = xFalse;
	    pixmapFormat = 0;
	}
    }
    if (!pixmapFormat)
	pixmapFormat = ZPixmap;
#endif
    ShmSegType = CreateNewResourceType(ShmDetachSegment);
    ShmPixType = CreateNewResourceType(ShmDetachSegment);
    if (ShmSegType && ShmPixType &&
	(extEntry = AddExtension(SHMNAME, ShmNumberEvents, ShmNumberErrors,
				 ProcShmDispatch, SProcShmDispatch,
				 ShmResetProc, StandardMinorOpcode)))
    {
	ShmReqCode = (unsigned char)extEntry->base;
	ShmCompletionCode = extEntry->eventBase;
	BadShmSegCode = extEntry->errorBase;
	EventSwapVector[ShmCompletionCode] = (EventSwapPtr) SShmCompletionEvent;
    }
}

/*ARGSUSED*/
static void
ShmResetProc (extEntry)
ExtensionEntry	*extEntry;
{
    int i;

    for (i = 0; i < MAXSCREENS; i++)
    {
	shmFuncs[i] = (ShmFuncsPtr)NULL;
	shmPixFormat[i] = 0;
    }
}

void
ShmRegisterFuncs(pScreen, funcs)
    ScreenPtr pScreen;
    ShmFuncsPtr funcs;
{
    shmFuncs[pScreen->myNum] = funcs;
}

void
ShmSetPixmapFormat(pScreen, format)
    ScreenPtr pScreen;
    int format;
{
    shmPixFormat[pScreen->myNum] = format;
}

void
ShmRegisterFbFuncs(pScreen)
    ScreenPtr pScreen;
{
    shmFuncs[pScreen->myNum] = &fbFuncs;
}

static int
ProcShmQueryVersion(client)
    register ClientPtr client;
{
    xShmQueryVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xShmQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.sharedPixmaps = sharedPixmaps;
    rep.pixmapFormat = pixmapFormat;
    rep.majorVersion = SHM_MAJOR_VERSION;
    rep.minorVersion = SHM_MINOR_VERSION;
    rep.uid = geteuid();
    rep.gid = getegid();
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.majorVersion, n);
	swaps(&rep.minorVersion, n);
	swaps(&rep.uid, n);
	swaps(&rep.gid, n);
    }
    WriteToClient(client, sizeof(xShmQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcShmAttach(client)
    register ClientPtr client;
{
    struct shmid_ds buf;
    ShmDescPtr shmdesc;
    REQUEST(xShmAttachReq);

    REQUEST_SIZE_MATCH(xShmAttachReq);
    LEGAL_NEW_RESOURCE(stuff->shmseg, client);
    if ((stuff->readOnly != xTrue) && (stuff->readOnly != xFalse))
    {
	client->errorValue = stuff->readOnly;
        return(BadValue);
    }
    for (shmdesc = Shmsegs;
	 shmdesc && (shmdesc->shmid != stuff->shmid);
	 shmdesc = shmdesc->next)
	;
    if (shmdesc)
    {
	if (!stuff->readOnly && !shmdesc->writable)
	    return BadAccess;
	shmdesc->refcnt++;
    }
    else
    {
	shmdesc = (ShmDescPtr) xalloc(sizeof(ShmDescRec));
	if (!shmdesc)
	    return BadAlloc;
	shmdesc->addr = shmat(stuff->shmid, 0,
			      stuff->readOnly ? SHM_RDONLY : 0);
	if ((shmdesc->addr == ((char *)-1)) ||
	    shmctl(stuff->shmid, IPC_STAT, &buf))
	{
	    xfree(shmdesc);
	    return BadAccess;
	}
	shmdesc->shmid = stuff->shmid;
	shmdesc->refcnt = 1;
	shmdesc->writable = !stuff->readOnly;
	shmdesc->size = buf.shm_segsz;
	shmdesc->next = Shmsegs;
	Shmsegs = shmdesc;
    }
    if (!AddResource(stuff->shmseg, ShmSegType, (pointer)shmdesc))
	return BadAlloc;
    return(client->noClientException);
}

/*ARGSUSED*/
static int
ShmDetachSegment(value, shmseg)
    pointer value; /* must conform to DeleteType */
    XID shmseg;
{
    ShmDescPtr shmdesc = (ShmDescPtr)value;
    ShmDescPtr *prev;

    if (--shmdesc->refcnt)
	return TRUE;
    shmdt(shmdesc->addr);
    for (prev = &Shmsegs; *prev != shmdesc; prev = &(*prev)->next)
	;
    *prev = shmdesc->next;
    xfree(shmdesc);
    return Success;
}

static int
ProcShmDetach(client)
    register ClientPtr client;
{
    ShmDescPtr shmdesc;
    REQUEST(xShmDetachReq);

    REQUEST_SIZE_MATCH(xShmDetachReq);
    VERIFY_SHMSEG(stuff->shmseg, shmdesc, client);
    FreeResource(stuff->shmseg, RT_NONE);
    return(client->noClientException);
}

static void
miShmPutImage(dst, pGC, depth, format, w, h, sx, sy, sw, sh, dx, dy, data)
    DrawablePtr dst;
    GCPtr	pGC;
    int		depth, w, h, sx, sy, sw, sh, dx, dy;
    unsigned int format;
    char 	*data;
{
    PixmapPtr pmap;
    GCPtr putGC;

    putGC = GetScratchGC(depth, dst->pScreen);
    if (!putGC)
	return;
    pmap = (*dst->pScreen->CreatePixmap)(dst->pScreen, sw, sh, depth);
    if (!pmap)
    {
	FreeScratchGC(putGC);
	return;
    }
    ValidateGC((DrawablePtr)pmap, putGC);
    (*putGC->ops->PutImage)((DrawablePtr)pmap, putGC, depth, -sx, -sy, w, h, 0,
			    (format == XYPixmap) ? XYPixmap : ZPixmap, data);
    FreeScratchGC(putGC);
    if (format == XYBitmap)
	(void)(*pGC->ops->CopyPlane)((DrawablePtr)pmap, dst, pGC, 0, 0, sw, sh,
				     dx, dy, 1L);
    else
	(void)(*pGC->ops->CopyArea)((DrawablePtr)pmap, dst, pGC, 0, 0, sw, sh,
				    dx, dy);
    (*pmap->drawable.pScreen->DestroyPixmap)(pmap);
}

static void
fbShmPutImage(dst, pGC, depth, format, w, h, sx, sy, sw, sh, dx, dy, data)
    DrawablePtr dst;
    GCPtr	pGC;
    int		depth, w, h, sx, sy, sw, sh, dx, dy;
    unsigned int format;
    char 	*data;
{
    if ((format == ZPixmap) || (depth == 1))
    {
	PixmapPtr pPixmap;

	pPixmap = GetScratchPixmapHeader(dst->pScreen, w, h, depth,
			/*XXX*/depth, PixmapBytePad(w, depth), (pointer)data);
	if (!pPixmap)
	    return;
	if (format == XYBitmap)
	    (void)(*pGC->ops->CopyPlane)((DrawablePtr)pPixmap, dst, pGC,
					 sx, sy, sw, sh, dx, dy, 1L);
	else
	    (void)(*pGC->ops->CopyArea)((DrawablePtr)pPixmap, dst, pGC,
					sx, sy, sw, sh, dx, dy);
	FreeScratchPixmapHeader(pPixmap);
    }
    else
	miShmPutImage(dst, pGC, depth, format, w, h, sx, sy, sw, sh, dx, dy,
		      data);
}

static int
ProcShmPutImage(client)
    register ClientPtr client;
{
    register GCPtr pGC;
    register DrawablePtr pDraw;
    long length;
#ifdef INTERNAL_VS_EXTERNAL_PADDING
    long lengthProto;
    char *tmpImage;
    int  tmpAlloced = 0;
#endif
    ShmDescPtr shmdesc;
    REQUEST(xShmPutImageReq);

    REQUEST_SIZE_MATCH(xShmPutImageReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);
    VERIFY_SHMPTR(stuff->shmseg, stuff->offset, FALSE, shmdesc, client);
    if ((stuff->sendEvent != xTrue) && (stuff->sendEvent != xFalse))
	return BadValue;
    if (stuff->format == XYBitmap)
    {
        if (stuff->depth != 1)
            return BadMatch;
        length = PixmapBytePad(stuff->totalWidth, 1);
#ifdef INTERNAL_VS_EXTERNAL_PADDING
        lengthProto = PixmapBytePadProto(stuff->totalWidth, 1);
#endif
    }
    else if (stuff->format == XYPixmap)
    {
        if (pDraw->depth != stuff->depth)
            return BadMatch;
        length = PixmapBytePad(stuff->totalWidth, 1);
	length *= stuff->depth;
#ifdef INTERNAL_VS_EXTERNAL_PADDING
        lengthProto = PixmapBytePadProto(stuff->totalWidth, 1);
	lengthProto *= stuff->depth;
#endif
    }
    else if (stuff->format == ZPixmap)
    {
        if (pDraw->depth != stuff->depth)
            return BadMatch;
        length = PixmapBytePad(stuff->totalWidth, stuff->depth);
#ifdef INTERNAL_VS_EXTERNAL_PADDING
        lengthProto = PixmapBytePadProto(stuff->totalWidth, stuff->depth);
#endif
    }
    else
    {
	client->errorValue = stuff->format;
        return BadValue;
    }

#ifdef INTERNAL_VS_EXTERNAL_PADDING
    VERIFY_SHMSIZE(shmdesc, stuff->offset, lengthProto * stuff->totalHeight,
		   client);
#else
    VERIFY_SHMSIZE(shmdesc, stuff->offset, length * stuff->totalHeight,
		   client);
#endif
    if (stuff->srcX > stuff->totalWidth)
    {
	client->errorValue = stuff->srcX;
	return BadValue;
    }
    if (stuff->srcY > stuff->totalHeight)
    {
	client->errorValue = stuff->srcY;
	return BadValue;
    }
    if ((stuff->srcX + stuff->srcWidth) > stuff->totalWidth)
    {
	client->errorValue = stuff->srcWidth;
	return BadValue;
    }
    if ((stuff->srcY + stuff->srcHeight) > stuff->totalHeight)
    {
	client->errorValue = stuff->srcHeight;
	return BadValue;
    }

#ifdef INTERNAL_VS_EXTERNAL_PADDING
    /* handle 64 bit case where protocol may pad to 32 and we want 64 
     * In this case, length is what the server wants and lengthProto is
     * what the protocol thinks it is.  If the the two are different,
     * copy the protocol version (i.e. the memory shared between the 
     * server and the client) to a version with a scanline pad of 64.
     */
    if (length != lengthProto) 
    {
	register int 	i;
	char 		* stuffptr, /* pointer into protocol data */
			* tmpptr;   /* new location to copy to */

        if(!(tmpImage = (char *) ALLOCATE_LOCAL(length*stuff->totalHeight)))
            return (BadAlloc);
	tmpAlloced = 1;
    
	bzero(tmpImage,length*stuff->totalHeight);
    
	if (stuff->format == XYPixmap) 
	{
	    int lineBytes =  PixmapBytePad(stuff->totalWidth, 1);
	    int lineBytesProto = PixmapBytePadProto(stuff->totalWidth, 1);
	    int depth = stuff->depth;

	    stuffptr = shmdesc->addr + stuff->offset ;
	    tmpptr = tmpImage;
	    for (i = 0; i < stuff->totalHeight*stuff->depth;
		 stuffptr += lineBytesProto,tmpptr += lineBytes, i++) 
	        bcopy(stuffptr,tmpptr,lineBytesProto);
	}
	else 
	{
	    for (i = 0,
		 stuffptr = shmdesc->addr + stuff->offset,
		 tmpptr=tmpImage;
		 i < stuff->totalHeight;
		 stuffptr += lengthProto,tmpptr += length, i++) 
	        bcopy(stuffptr,tmpptr,lengthProto);
	}
    }
    /* handle 64-bit case where stuff is not 64-bit aligned 
     */
    else if ((unsigned long)(shmdesc->addr+stuff->offset) & 
	     (sizeof(long)-1)) 
    {
        if(!(tmpImage = (char *) ALLOCATE_LOCAL(length*stuff->totalHeight)))
            return (BadAlloc);
	tmpAlloced = 1;
	bcopy((char *)(shmdesc->addr+stuff->offset),
	      tmpImage,
	      length*stuff->totalHeight);
    }
    else
	tmpImage = (char *)(shmdesc->addr+stuff->offset);
#endif

    if ((((stuff->format == ZPixmap) && (stuff->srcX == 0)) ||
	 ((stuff->format != ZPixmap) &&
	  (stuff->srcX < screenInfo.bitmapScanlinePad) &&
	  ((stuff->format == XYBitmap) ||
	   ((stuff->srcY == 0) &&
	    (stuff->srcHeight == stuff->totalHeight))))) &&
	((stuff->srcX + stuff->srcWidth) == stuff->totalWidth))
	(*pGC->ops->PutImage) (pDraw, pGC, stuff->depth,
			       stuff->dstX, stuff->dstY,
			       stuff->totalWidth, stuff->srcHeight, 
			       stuff->srcX, stuff->format, 
#ifdef INTERNAL_VS_EXTERNAL_PADDING
			       tmpImage +
#else
			       shmdesc->addr + stuff->offset +
#endif
			       (stuff->srcY * length));
    else
	(*shmFuncs[pDraw->pScreen->myNum]->PutImage)(
			       pDraw, pGC, stuff->depth, stuff->format,
			       stuff->totalWidth, stuff->totalHeight,
			       stuff->srcX, stuff->srcY,
			       stuff->srcWidth, stuff->srcHeight,
			       stuff->dstX, stuff->dstY,
#ifdef INTERNAL_VS_EXTERNAL_PADDING
			       tmpImage);
    
#else
                               shmdesc->addr + stuff->offset);
#endif

    if (stuff->sendEvent)
    {
	xShmCompletionEvent ev;

	ev.type = ShmCompletionCode;
	ev.drawable = stuff->drawable;
	ev.sequenceNumber = client->sequence;
	ev.minorEvent = X_ShmPutImage;
	ev.majorEvent = ShmReqCode;
	ev.shmseg = stuff->shmseg;
	ev.offset = stuff->offset;
	WriteEventsToClient(client, 1, (xEvent *) &ev);
    }

#ifdef INTERNAL_VS_EXTERNAL_PADDING
    if (tmpAlloced)
        DEALLOCATE_LOCAL(tmpImage);
#endif

     return (client->noClientException);
}



static int
ProcShmGetImage(client)
    register ClientPtr client;
{
    register DrawablePtr pDraw;
    long		lenPer, length;
    Mask		plane;
    xShmGetImageReply	xgi;
    ShmDescPtr		shmdesc;
    int			n;
#ifdef INTERNAL_VS_EXTERNAL_PADDING
    long		widthBytesLine,widthBytesLineProto;
    long 		lenPerProto,lengthProto;
    char 		*tmpImage;
    int  		tmpAlloced = 0;
#endif

    REQUEST(xShmGetImageReq);

    REQUEST_SIZE_MATCH(xShmGetImageReq);
    if ((stuff->format != XYPixmap) && (stuff->format != ZPixmap))
    {
	client->errorValue = stuff->format;
        return(BadValue);
    }
    VERIFY_DRAWABLE(pDraw, stuff->drawable, client);
    VERIFY_SHMPTR(stuff->shmseg, stuff->offset, TRUE, shmdesc, client);
    if (pDraw->type == DRAWABLE_WINDOW)
    {
      if( /* check for being viewable */
	 !((WindowPtr) pDraw)->realized ||
	  /* check for being on screen */
         pDraw->x + stuff->x < 0 ||
 	 pDraw->x + stuff->x + (int)stuff->width > pDraw->pScreen->width ||
         pDraw->y + stuff->y < 0 ||
         pDraw->y + stuff->y + (int)stuff->height > pDraw->pScreen->height ||
          /* check for being inside of border */
         stuff->x < - wBorderWidth((WindowPtr)pDraw) ||
         stuff->x + (int)stuff->width >
		wBorderWidth((WindowPtr)pDraw) + (int)pDraw->width ||
         stuff->y < -wBorderWidth((WindowPtr)pDraw) ||
         stuff->y + (int)stuff->height >
		wBorderWidth((WindowPtr)pDraw) + (int)pDraw->height
        )
	    return(BadMatch);
	xgi.visual = wVisual(((WindowPtr)pDraw));
    }
    else
    {
	if (stuff->x < 0 ||
	    stuff->x+(int)stuff->width > pDraw->width ||
	    stuff->y < 0 ||
	    stuff->y+(int)stuff->height > pDraw->height
	    )
	    return(BadMatch);
	xgi.visual = None;
    }
    xgi.type = X_Reply;
    xgi.length = 0;
    xgi.sequenceNumber = client->sequence;
    xgi.depth = pDraw->depth;
    if(stuff->format == ZPixmap)
    {
#ifdef INTERNAL_VS_EXTERNAL_PADDING
	widthBytesLine = PixmapBytePad(stuff->width, pDraw->depth);
	length = widthBytesLine * stuff->height;
	widthBytesLineProto =  PixmapBytePadProto(stuff->width, pDraw->depth);
	lengthProto = widthBytesLineProto * stuff->height;
#else
	length = PixmapBytePad(stuff->width, pDraw->depth) * stuff->height;
#endif
    }
    else 
    {
#ifdef INTERNAL_VS_EXTERNAL_PADDING
	widthBytesLine = PixmapBytePad(stuff->width, 1);
	lenPer = widthBytesLine * stuff->height;
	plane = ((Mask)1) << (pDraw->depth - 1);
	/* only planes asked for */
	length = lenPer * Ones(stuff->planeMask & (plane | (plane - 1)));

	widthBytesLineProto = PixmapBytePadProto(stuff->width, 1);
	lenPerProto = widthBytesLineProto * stuff->height;
	lengthProto = lenPerProto * Ones(stuff->planeMask & 
					 (plane | (plane - 1)));
#else
	lenPer = PixmapBytePad(stuff->width, 1) * stuff->height;
	plane = ((Mask)1) << (pDraw->depth - 1);
	/* only planes asked for */
	length = lenPer * Ones(stuff->planeMask & (plane | (plane - 1)));
#endif
    }

#ifdef INTERNAL_VS_EXTERNAL_PADDING
    VERIFY_SHMSIZE(shmdesc, stuff->offset, lengthProto, client);
    xgi.size = lengthProto;
#else
    VERIFY_SHMSIZE(shmdesc, stuff->offset, length, client);
    xgi.size = length;
#endif

    if (length == 0)
    {
	/* nothing to do */
    }
    else if (stuff->format == ZPixmap)
    {
#ifdef INTERNAL_VS_EXTERNAL_PADDING
        /* check for protocol/server padding differences.
	 */
        if ((widthBytesLine != widthBytesLineProto) ||
	    ((unsigned long)shmdesc->addr + stuff->offset & (sizeof(long)-1))) 
	{
	    /* temp stuff for 64 bit alignment stuff */
	    register char * bufPtr, * protoPtr;
	    register int i;

	    if(!(tmpImage = (char *) ALLOCATE_LOCAL(length))) 
	      return (BadAlloc);
	    tmpAlloced = 1;
	    
	    (*pDraw->pScreen->GetImage)(pDraw, stuff->x, stuff->y,
					stuff->width, stuff->height,
					stuff->format, stuff->planeMask,
					tmpImage);
	    
	    /* for 64-bit server, convert image to pad to 32 bits 
	     */
	    bzero(shmdesc->addr + stuff->offset,lengthProto);
	    
	    for (i=0,bufPtr=tmpImage,protoPtr=shmdesc->addr + stuff->offset; 
		 i < stuff->height;
		 bufPtr += widthBytesLine,protoPtr += widthBytesLineProto, 
		 i++)
		bcopy(bufPtr,protoPtr,widthBytesLineProto);
	}
	else 
	{
	    (*pDraw->pScreen->GetImage)(pDraw, stuff->x, stuff->y,
					stuff->width, stuff->height,
					stuff->format, stuff->planeMask,
					shmdesc->addr + stuff->offset);
	}
#else
	(*pDraw->pScreen->GetImage)(pDraw, stuff->x, stuff->y,
				    stuff->width, stuff->height,
				    stuff->format, stuff->planeMask,
				    shmdesc->addr + stuff->offset);
#endif
    }
    else
    {
#ifdef INTERNAL_VS_EXTERNAL_PADDING
	/* check for protocol/server padding differences.
	 */
	if ((widthBytesLine != widthBytesLineProto) ||
	    ((unsigned long)shmdesc->addr + stuff->offset & 
	     (sizeof(long)-1))) 
	{
	    if(!(tmpImage = (char *) ALLOCATE_LOCAL(length)))
	      return (BadAlloc);
	    tmpAlloced = 1;
	}
#endif

	length = stuff->offset;
        for (; plane; plane >>= 1)
	{
	    if (stuff->planeMask & plane)
	    {
#ifdef INTERNAL_VS_EXTERNAL_PADDING
		if ((widthBytesLine != widthBytesLineProto) ||
		    ((unsigned long)shmdesc->addr + stuff->offset & 
		     (sizeof(long)-1))) 
		{
		    /* get image for each plane. 
		     */
		    (*pDraw->pScreen->GetImage)(pDraw,
						stuff->x, stuff->y,
						stuff->width, stuff->height,
						stuff->format, plane,
						tmpImage);
		    
		    /* for 64-bit server, convert image to pad to 32 bits */
		    bzero(shmdesc->addr+length, widthBytesLine);
		    bcopy(tmpImage, shmdesc->addr+length, widthBytesLineProto);
		    /* increment length */
		    length += lenPerProto;
		}
		else /* no diff between protocol and server */
		{
		    (*pDraw->pScreen->GetImage)(pDraw,
						stuff->x, stuff->y,
						stuff->width, stuff->height,
						stuff->format, plane,
						shmdesc->addr + length);
		    length += lenPer;
		}
#else
		(*pDraw->pScreen->GetImage)(pDraw,
					    stuff->x, stuff->y,
					    stuff->width, stuff->height,
					    stuff->format, plane,
					    shmdesc->addr + length);
		length += lenPer;
#endif
	    }
	}
    }
    
    if (client->swapped) {
    	swaps(&xgi.sequenceNumber, n);
    	swapl(&xgi.length, n);
	swapl(&xgi.visual, n);
	swapl(&xgi.size, n);
    }
    WriteToClient(client, sizeof(xShmGetImageReply), (char *)&xgi);

#ifdef INTERNAL_VS_EXTERNAL_PADDING
    if (tmpAlloced)
	DEALLOCATE_LOCAL(tmpImage);
#endif

    return(client->noClientException);
}

static PixmapPtr
fbShmCreatePixmap (pScreen, width, height, depth, addr)
    ScreenPtr	pScreen;
    int		width;
    int		height;
    int		depth;
    char	*addr;
{
    register PixmapPtr pPixmap;

    pPixmap = (*pScreen->CreatePixmap)(pScreen, 0, 0, pScreen->rootDepth);
    if (!pPixmap)
	return NullPixmap;

    if (!(*pScreen->ModifyPixmapHeader)(pPixmap, width, height, depth,
		  /*XXX*/depth, PixmapBytePad(width, depth), (pointer)addr))
	return NullPixmap;
    return pPixmap;
}

static int
ProcShmCreatePixmap(client)
    register ClientPtr client;
{
    PixmapPtr pMap;
    register DrawablePtr pDraw;
    DepthPtr pDepth;
    register int i;
    ShmDescPtr shmdesc;
    REQUEST(xShmCreatePixmapReq);

    REQUEST_SIZE_MATCH(xShmCreatePixmapReq);
    client->errorValue = stuff->pid;
    if (!sharedPixmaps)
	return BadImplementation;
    LEGAL_NEW_RESOURCE(stuff->pid, client);
    VERIFY_GEOMETRABLE(pDraw, stuff->drawable, client);
    VERIFY_SHMPTR(stuff->shmseg, stuff->offset, TRUE, shmdesc, client);
    if (!stuff->width || !stuff->height)
    {
	client->errorValue = 0;
        return BadValue;
    }
    if (stuff->depth != 1)
    {
        pDepth = pDraw->pScreen->allowedDepths;
        for (i=0; i<pDraw->pScreen->numDepths; i++, pDepth++)
	   if (pDepth->depth == stuff->depth)
               goto CreatePmap;
	client->errorValue = stuff->depth;
        return BadValue;
    }
CreatePmap:
    VERIFY_SHMSIZE(shmdesc, stuff->offset,
		   PixmapBytePad(stuff->width, stuff->depth) * stuff->height,
		   client);
    pMap = (*shmFuncs[pDraw->pScreen->myNum]->CreatePixmap)(
			    pDraw->pScreen, stuff->width,
			    stuff->height, stuff->depth,
			    shmdesc->addr + stuff->offset);
    if (pMap)
    {
	pMap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	pMap->drawable.id = stuff->pid;
	if (AddResource(stuff->pid, RT_PIXMAP, (pointer)pMap))
	{
	    shmdesc->refcnt++;
	    if (AddResource(stuff->pid, ShmPixType, (pointer)shmdesc))
		return(client->noClientException);
	    FreeResource(stuff->pid, RT_NONE);
	}
    }
    return (BadAlloc);
}

static int
ProcShmDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_ShmQueryVersion:
	return ProcShmQueryVersion(client);
    case X_ShmAttach:
	return ProcShmAttach(client);
    case X_ShmDetach:
	return ProcShmDetach(client);
    case X_ShmPutImage:
	return ProcShmPutImage(client);
    case X_ShmGetImage:
	return ProcShmGetImage(client);
    case X_ShmCreatePixmap:
	return ProcShmCreatePixmap(client);
    default:
	return BadRequest;
    }
}

static void
SShmCompletionEvent(from, to)
    xShmCompletionEvent *from, *to;
{
    to->type = from->type;
    cpswaps(from->sequenceNumber, to->sequenceNumber);
    cpswapl(from->drawable, to->drawable);
    cpswaps(from->minorEvent, to->minorEvent);
    to->majorEvent = from->majorEvent;
    cpswapl(from->shmseg, to->shmseg);
    cpswapl(from->offset, to->offset);
}

static int
SProcShmQueryVersion(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xShmQueryVersionReq);

    swaps(&stuff->length, n);
    return ProcShmQueryVersion(client);
}

static int
SProcShmAttach(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xShmAttachReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xShmAttachReq);
    swapl(&stuff->shmseg, n);
    swapl(&stuff->shmid, n);
    return ProcShmAttach(client);
}

static int
SProcShmDetach(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xShmDetachReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xShmDetachReq);
    swapl(&stuff->shmseg, n);
    return ProcShmDetach(client);
}

static int
SProcShmPutImage(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xShmPutImageReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xShmPutImageReq);
    swapl(&stuff->drawable, n);
    swapl(&stuff->gc, n);
    swaps(&stuff->totalWidth, n);
    swaps(&stuff->totalHeight, n);
    swaps(&stuff->srcX, n);
    swaps(&stuff->srcY, n);
    swaps(&stuff->srcWidth, n);
    swaps(&stuff->srcHeight, n);
    swaps(&stuff->dstX, n);
    swaps(&stuff->dstY, n);
    swapl(&stuff->shmseg, n);
    swapl(&stuff->offset, n);
    return ProcShmPutImage(client);
}

static int
SProcShmGetImage(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xShmGetImageReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xShmGetImageReq);
    swapl(&stuff->drawable, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swapl(&stuff->planeMask, n);
    swapl(&stuff->shmseg, n);
    swapl(&stuff->offset, n);
    return ProcShmGetImage(client);
}

static int
SProcShmCreatePixmap(client)
    ClientPtr client;
{
    register int n;
    REQUEST(xShmCreatePixmapReq);
    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xShmCreatePixmapReq);
    swapl(&stuff->pid, n);
    swapl(&stuff->drawable, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swapl(&stuff->shmseg, n);
    swapl(&stuff->offset, n);
    return ProcShmCreatePixmap(client);
}

static int
SProcShmDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_ShmQueryVersion:
	return SProcShmQueryVersion(client);
    case X_ShmAttach:
	return SProcShmAttach(client);
    case X_ShmDetach:
	return SProcShmDetach(client);
    case X_ShmPutImage:
	return SProcShmPutImage(client);
    case X_ShmGetImage:
	return SProcShmGetImage(client);
    case X_ShmCreatePixmap:
	return SProcShmCreatePixmap(client);
    default:
	return BadRequest;
    }
}
