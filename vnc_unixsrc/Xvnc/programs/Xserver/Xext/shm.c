/* $XFree86: xc/programs/Xserver/Xext/shm.c,v 3.41 2003/12/17 23:28:56 alanh Exp $ */
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

********************************************************/

/* THIS IS NOT AN X CONSORTIUM STANDARD OR AN X PROJECT TEAM SPECIFICATION */

/* $Xorg: shm.c,v 1.4 2001/02/09 02:04:33 xorgcvs Exp $ */

#define SHM

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <sys/types.h>
#ifndef Lynx
#include <sys/ipc.h>
#include <sys/shm.h>
#else
#include <ipc.h>
#include <shm.h>
#endif
#include <unistd.h>
#include <sys/stat.h>
#define NEED_REPLIES
#define NEED_EVENTS
#include <X11/X.h>
#include <X11/Xproto.h>
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
#include <X11/extensions/shmstr.h>
#include <X11/Xfuncproto.h>
#ifdef EXTMODULE
#include "xf86_ansic.h"
#endif

#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#endif

#include "modinit.h"

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
    pointer		/* value */,
    XID			/* shmseg */
    );
static void ShmResetProc(
    ExtensionEntry *	/* extEntry */
    );
static void SShmCompletionEvent(
    xShmCompletionEvent * /* from */,
    xShmCompletionEvent * /* to */
    );

static Bool ShmDestroyPixmap (PixmapPtr pPixmap);

static DISPATCH_PROC(ProcShmAttach);
static DISPATCH_PROC(ProcShmCreatePixmap);
static DISPATCH_PROC(ProcShmDetach);
static DISPATCH_PROC(ProcShmDispatch);
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
int ShmCompletionCode;
int BadShmSegCode;
RESTYPE ShmSegType;
static ShmDescPtr Shmsegs;
static Bool sharedPixmaps;
static int pixmapFormat;
static int shmPixFormat[MAXSCREENS];
static ShmFuncsPtr shmFuncs[MAXSCREENS];
static DestroyPixmapProcPtr destroyPixmap[MAXSCREENS];
#ifdef PIXPRIV
static int  shmPixmapPrivate;
#endif
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


#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__CYGWIN__)
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

    if (shmid != -1)
    {
        /* Successful allocation - clean up */
	shmctl(shmid, IPC_RMID, (struct shmid_ds *)NULL);
    }
    else
    {
        /* Allocation failed */
        badSysCall = TRUE;
    }
    signal(SIGSYS, oldHandler);
    return(!badSysCall);
}

#define MUST_CHECK_FOR_SHM_SYSCALL

#endif

void
ShmExtensionInit(INITARGS)
{
    ExtensionEntry *extEntry;
    int i;

#ifdef MUST_CHECK_FOR_SHM_SYSCALL
    if (!CheckForShmSyscall())
    {
	ErrorF("MIT-SHM extension disabled due to lack of kernel support\n");
	return;
    }
#endif

    sharedPixmaps = xFalse;
    pixmapFormat = 0;
    {
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
      if (sharedPixmaps)
      {
	for (i = 0; i < screenInfo.numScreens; i++)
	{
	    destroyPixmap[i] = screenInfo.screens[i]->DestroyPixmap;
	    screenInfo.screens[i]->DestroyPixmap = ShmDestroyPixmap;
	}
#ifdef PIXPRIV
	shmPixmapPrivate = AllocatePixmapPrivateIndex();
	for (i = 0; i < screenInfo.numScreens; i++)
	{
	    if (!AllocatePixmapPrivate(screenInfo.screens[i],
				       shmPixmapPrivate, 0))
		return;
	}
#endif
      }
    }
    ShmSegType = CreateNewResourceType(ShmDetachSegment);
    if (ShmSegType &&
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
ShmRegisterFuncs(
    ScreenPtr pScreen,
    ShmFuncsPtr funcs)
{
    shmFuncs[pScreen->myNum] = funcs;
}

void
ShmSetPixmapFormat(
    ScreenPtr pScreen,
    int format)
{
    shmPixFormat[pScreen->myNum] = format;
}

static Bool
ShmDestroyPixmap (PixmapPtr pPixmap)
{
    ScreenPtr	    pScreen = pPixmap->drawable.pScreen;
    Bool	    ret;
    if (pPixmap->refcnt == 1)
    {
	ShmDescPtr  shmdesc;
#ifdef PIXPRIV
	shmdesc = (ShmDescPtr) pPixmap->devPrivates[shmPixmapPrivate].ptr;
#else
	char	*base = (char *) pPixmap->devPrivate.ptr;
	
	if (base != (pointer) (pPixmap + 1))
	{
	    for (shmdesc = Shmsegs; shmdesc; shmdesc = shmdesc->next)
	    {
		if (shmdesc->addr <= base && base <= shmdesc->addr + shmdesc->size)
		    break;
	    }
	}
	else
	    shmdesc = 0;
#endif
	if (shmdesc)
	    ShmDetachSegment ((pointer) shmdesc, pPixmap->drawable.id);
    }
    
    pScreen->DestroyPixmap = destroyPixmap[pScreen->myNum];
    ret = (*pScreen->DestroyPixmap) (pPixmap);
    destroyPixmap[pScreen->myNum] = pScreen->DestroyPixmap;
    pScreen->DestroyPixmap = ShmDestroyPixmap;
    return ret;
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

/*
 * Simulate the access() system call for a shared memory segement,
 * using the credentials from the client if available
 */
static int
shm_access(ClientPtr client, struct ipc_perm *perm, int readonly)
{
    int uid, gid;
    mode_t mask;

    if (LocalClientCred(client, &uid, &gid) != -1) {
	
	/* User id 0 always gets access */
	if (uid == 0) {
	    return 0;
	}
	/* Check the owner */
	if (perm->uid == uid || perm->cuid == uid) {
	    mask = S_IRUSR;
	    if (!readonly) {
		mask |= S_IWUSR;
	    }
	    return (perm->mode & mask) == mask ? 0 : -1;
	}
	/* Check the group */
	if (perm->gid == gid || perm->cgid == gid) {
	    mask = S_IRGRP;
	    if (!readonly) {
		mask |= S_IWGRP;
	    }
	    return (perm->mode & mask) == mask ? 0 : -1;
	}
    }
    /* Otherwise, check everyone else */
    mask = S_IROTH;
    if (!readonly) {
	mask |= S_IWOTH;
    }
    return (perm->mode & mask) == mask ? 0 : -1;
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

	/* The attach was performed with root privs. We must
	 * do manual checking of access rights for the credentials 
	 * of the client */

	if (shm_access(client, &(buf.shm_perm), stuff->readOnly) == -1) {
	    shmdt(shmdesc->addr);
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
		BitsPerPixel(depth), PixmapBytePad(w, depth), (pointer)data);
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


#ifdef PANORAMIX
static int 
ProcPanoramiXShmPutImage(register ClientPtr client)
{
    int			 j, result = 0, orig_x, orig_y;
    PanoramiXRes	*draw, *gc;
    Bool		 sendEvent, isRoot;

    REQUEST(xShmPutImageReq);
    REQUEST_SIZE_MATCH(xShmPutImageReq);

    if(!(draw = (PanoramiXRes *)SecurityLookupIDByClass(
                client, stuff->drawable, XRC_DRAWABLE, SecurityWriteAccess)))
        return BadDrawable;

    if(!(gc = (PanoramiXRes *)SecurityLookupIDByType(
                client, stuff->gc, XRT_GC, SecurityReadAccess)))
        return BadGC;

    isRoot = (draw->type == XRT_WINDOW) && draw->u.win.root;

    orig_x = stuff->dstX;
    orig_y = stuff->dstY;
    sendEvent = stuff->sendEvent;
    stuff->sendEvent = 0;
    FOR_NSCREENS(j) {
	if(!j) stuff->sendEvent = sendEvent;
	stuff->drawable = draw->info[j].id;
	stuff->gc = gc->info[j].id;
	if (isRoot) {
	    stuff->dstX = orig_x - panoramiXdataPtr[j].x;
	    stuff->dstY = orig_y - panoramiXdataPtr[j].y;
	}
	result = ProcShmPutImage(client);
	if(result != client->noClientException) break;
    }
    return(result);
}

static int 
ProcPanoramiXShmGetImage(ClientPtr client)
{
    PanoramiXRes	*draw;
    DrawablePtr 	drawables[MAXSCREENS];
    DrawablePtr 	pDraw;
    xShmGetImageReply	xgi;
    ShmDescPtr		shmdesc;
    int         	i, x, y, w, h, format;
    Mask		plane = 0, planemask;
    long		lenPer = 0, length, widthBytesLine;
    Bool		isRoot;

    REQUEST(xShmGetImageReq);

    REQUEST_SIZE_MATCH(xShmGetImageReq);

    if ((stuff->format != XYPixmap) && (stuff->format != ZPixmap)) {
	client->errorValue = stuff->format;
        return(BadValue);
    }

    if(!(draw = (PanoramiXRes *)SecurityLookupIDByClass(
		client, stuff->drawable, XRC_DRAWABLE, SecurityWriteAccess)))
	return BadDrawable;

    if (draw->type == XRT_PIXMAP)
	return ProcShmGetImage(client);

    VERIFY_DRAWABLE(pDraw, stuff->drawable, client);

    VERIFY_SHMPTR(stuff->shmseg, stuff->offset, TRUE, shmdesc, client);

    x = stuff->x;
    y = stuff->y;
    w = stuff->width;
    h = stuff->height;
    format = stuff->format;
    planemask = stuff->planeMask;

    isRoot = (draw->type == XRT_WINDOW) && draw->u.win.root;

    if(isRoot) {
      if( /* check for being onscreen */
	x < 0 || x + w > PanoramiXPixWidth ||
	y < 0 || y + h > PanoramiXPixHeight )
	    return(BadMatch);
    } else {
      if( /* check for being onscreen */
	panoramiXdataPtr[0].x + pDraw->x + x < 0 ||
	panoramiXdataPtr[0].x + pDraw->x + x + w > PanoramiXPixWidth ||
        panoramiXdataPtr[0].y + pDraw->y + y < 0 ||
	panoramiXdataPtr[0].y + pDraw->y + y + h > PanoramiXPixHeight ||
	 /* check for being inside of border */
       	x < - wBorderWidth((WindowPtr)pDraw) ||
	x + w > wBorderWidth((WindowPtr)pDraw) + (int)pDraw->width ||
	y < -wBorderWidth((WindowPtr)pDraw) ||
	y + h > wBorderWidth ((WindowPtr)pDraw) + (int)pDraw->height)
	    return(BadMatch);
    }

    drawables[0] = pDraw;
    for(i = 1; i < PanoramiXNumScreens; i++)
	VERIFY_DRAWABLE(drawables[i], draw->info[i].id, client);

    xgi.visual = wVisual(((WindowPtr)pDraw));
    xgi.type = X_Reply;
    xgi.length = 0;
    xgi.sequenceNumber = client->sequence;
    xgi.depth = pDraw->depth;

    if(format == ZPixmap) {
	widthBytesLine = PixmapBytePad(w, pDraw->depth);
	length = widthBytesLine * h;
    } else {
	widthBytesLine = PixmapBytePad(w, 1);
	lenPer = widthBytesLine * h;
	plane = ((Mask)1) << (pDraw->depth - 1);
	length = lenPer * Ones(planemask & (plane | (plane - 1)));
    }

    VERIFY_SHMSIZE(shmdesc, stuff->offset, length, client);
    xgi.size = length;

    if (length == 0) {/* nothing to do */ }
    else if (format == ZPixmap) {
	    XineramaGetImageData(drawables, x, y, w, h, format, planemask,
					shmdesc->addr + stuff->offset,
					widthBytesLine, isRoot);
    } else {

	length = stuff->offset;
        for (; plane; plane >>= 1) {
	    if (planemask & plane) {
		XineramaGetImageData(drawables, x, y, w, h, 
				     format, plane, shmdesc->addr + length,
				     widthBytesLine, isRoot);
		length += lenPer;
	    }
	}
    }
    
    if (client->swapped) {
	register int n;
    	swaps(&xgi.sequenceNumber, n);
    	swapl(&xgi.length, n);
	swapl(&xgi.visual, n);
	swapl(&xgi.size, n);
    }
    WriteToClient(client, sizeof(xShmGetImageReply), (char *)&xgi);

    return(client->noClientException);
}

static int
ProcPanoramiXShmCreatePixmap(
    register ClientPtr client)
{
    ScreenPtr pScreen = NULL;
    PixmapPtr pMap = NULL;
    DrawablePtr pDraw;
    DepthPtr pDepth;
    int i, j, result;
    ShmDescPtr shmdesc;
    REQUEST(xShmCreatePixmapReq);
    PanoramiXRes *newPix;

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

    if(!(newPix = (PanoramiXRes *) xalloc(sizeof(PanoramiXRes))))
	return BadAlloc;

    newPix->type = XRT_PIXMAP;
    newPix->u.pix.shared = TRUE;
    newPix->info[0].id = stuff->pid;
    for(j = 1; j < PanoramiXNumScreens; j++)
	newPix->info[j].id = FakeClientID(client->index);

    result = (client->noClientException);

    FOR_NSCREENS(j) {
	pScreen = screenInfo.screens[j];

	pMap = (*shmFuncs[j]->CreatePixmap)(pScreen, 
				stuff->width, stuff->height, stuff->depth,
				shmdesc->addr + stuff->offset);

	if (pMap) {
#ifdef PIXPRIV
            pMap->devPrivates[shmPixmapPrivate].ptr = (pointer) shmdesc;
#endif
            shmdesc->refcnt++;
	    pMap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	    pMap->drawable.id = newPix->info[j].id;
	    if (!AddResource(newPix->info[j].id, RT_PIXMAP, (pointer)pMap)) {
		(*pScreen->DestroyPixmap)(pMap);
		result = BadAlloc;
		break;
	    }
	} else {
	   result = BadAlloc;
	   break;
	}
    }

    if(result == BadAlloc) {
	while(j--) {
	    (*pScreen->DestroyPixmap)(pMap);
	    FreeResource(newPix->info[j].id, RT_NONE);
	}
	xfree(newPix);
    } else 
	AddResource(stuff->pid, XRT_PIXMAP, newPix);

    return result;
}

#endif

static int
ProcShmPutImage(client)
    register ClientPtr client;
{
    register GCPtr pGC;
    register DrawablePtr pDraw;
    long length;
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
    }
    else if (stuff->format == XYPixmap)
    {
        if (pDraw->depth != stuff->depth)
            return BadMatch;
        length = PixmapBytePad(stuff->totalWidth, 1);
	length *= stuff->depth;
    }
    else if (stuff->format == ZPixmap)
    {
        if (pDraw->depth != stuff->depth)
            return BadMatch;
        length = PixmapBytePad(stuff->totalWidth, stuff->depth);
    }
    else
    {
	client->errorValue = stuff->format;
        return BadValue;
    }

    VERIFY_SHMSIZE(shmdesc, stuff->offset, length * stuff->totalHeight,
		   client);
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
			       shmdesc->addr + stuff->offset +
			       (stuff->srcY * length));
    else
	(*shmFuncs[pDraw->pScreen->myNum]->PutImage)(
			       pDraw, pGC, stuff->depth, stuff->format,
			       stuff->totalWidth, stuff->totalHeight,
			       stuff->srcX, stuff->srcY,
			       stuff->srcWidth, stuff->srcHeight,
			       stuff->dstX, stuff->dstY,
                               shmdesc->addr + stuff->offset);

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

    return (client->noClientException);
}



static int
ProcShmGetImage(client)
    register ClientPtr client;
{
    register DrawablePtr pDraw;
    long		lenPer = 0, length;
    Mask		plane = 0;
    xShmGetImageReply	xgi;
    ShmDescPtr		shmdesc;
    int			n;

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
	length = PixmapBytePad(stuff->width, pDraw->depth) * stuff->height;
    }
    else 
    {
	lenPer = PixmapBytePad(stuff->width, 1) * stuff->height;
	plane = ((Mask)1) << (pDraw->depth - 1);
	/* only planes asked for */
	length = lenPer * Ones(stuff->planeMask & (plane | (plane - 1)));
    }

    VERIFY_SHMSIZE(shmdesc, stuff->offset, length, client);
    xgi.size = length;

    if (length == 0)
    {
	/* nothing to do */
    }
    else if (stuff->format == ZPixmap)
    {
	(*pDraw->pScreen->GetImage)(pDraw, stuff->x, stuff->y,
				    stuff->width, stuff->height,
				    stuff->format, stuff->planeMask,
				    shmdesc->addr + stuff->offset);
    }
    else
    {

	length = stuff->offset;
        for (; plane; plane >>= 1)
	{
	    if (stuff->planeMask & plane)
	    {
		(*pDraw->pScreen->GetImage)(pDraw,
					    stuff->x, stuff->y,
					    stuff->width, stuff->height,
					    stuff->format, plane,
					    shmdesc->addr + length);
		length += lenPer;
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
	    BitsPerPixel(depth), PixmapBytePad(width, depth), (pointer)addr)) {
	(*pScreen->DestroyPixmap)(pPixmap);
	return NullPixmap;
    }
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
#ifdef PIXPRIV
	pMap->devPrivates[shmPixmapPrivate].ptr = (pointer) shmdesc;
#endif
	shmdesc->refcnt++;
	pMap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	pMap->drawable.id = stuff->pid;
	if (AddResource(stuff->pid, RT_PIXMAP, (pointer)pMap))
	{
	    return(client->noClientException);
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
#ifdef PANORAMIX
        if ( !noPanoramiXExtension )
	   return ProcPanoramiXShmPutImage(client);
#endif
	return ProcShmPutImage(client);
    case X_ShmGetImage:
#ifdef PANORAMIX
        if ( !noPanoramiXExtension )
	   return ProcPanoramiXShmGetImage(client);
#endif
	return ProcShmGetImage(client);
    case X_ShmCreatePixmap:
#ifdef PANORAMIX
        if ( !noPanoramiXExtension )
	   return ProcPanoramiXShmCreatePixmap(client);
#endif
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
