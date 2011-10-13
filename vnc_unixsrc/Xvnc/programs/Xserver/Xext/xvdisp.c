/* $XdotOrg: xc/programs/Xserver/Xext/xvdisp.c,v 1.6 2005/07/03 08:53:36 daniels Exp $ */
/***********************************************************
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
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
/* $XFree86: xc/programs/Xserver/Xext/xvdisp.c,v 1.27 2003/07/16 01:38:31 dawes Exp $ */

/*
** File: 
**
**   xvdisp.c --- Xv server extension dispatch module.
**
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   11.06.91 Carver
**     - changed SetPortControl to SetPortAttribute
**     - changed GetPortControl to GetPortAttribute
**     - changed QueryBestSize
**
**   15.05.91 Carver
**     - version 2.0 upgrade
**
**   24.01.91 Carver
**     - version 1.4 upgrade
**
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "gcstruct.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"

#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvproto.h>
#include "xvdix.h"
#ifdef MITSHM
#define _XSHM_SERVER_
#include <X11/extensions/shmstr.h>
#endif

#ifdef EXTMODULE
#include "xf86_ansic.h"
#endif

#include "xvdisp.h"

#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"

unsigned long XvXRTPort;

#ifdef MITSHM
static int XineramaXvShmPutImage(ClientPtr);
#endif
static int XineramaXvPutImage(ClientPtr);
static int XineramaXvPutVideo(ClientPtr);
static int XineramaXvPutStill(ClientPtr);
static int XineramaXvSetPortAttribute(ClientPtr);
static int XineramaXvStopVideo(ClientPtr);
#endif

/* INTERNAL */

static int ProcXvQueryExtension(ClientPtr);
static int ProcXvQueryAdaptors(ClientPtr);
static int ProcXvQueryEncodings(ClientPtr);
static int ProcXvPutVideo(ClientPtr);
static int ProcXvPutStill(ClientPtr);
static int ProcXvGetVideo(ClientPtr);
static int ProcXvGetStill(ClientPtr);
static int ProcXvGrabPort(ClientPtr);
static int ProcXvUngrabPort(ClientPtr);
static int ProcXvSelectVideoNotify(ClientPtr);
static int ProcXvSelectPortNotify(ClientPtr);
static int ProcXvStopVideo(ClientPtr);
static int ProcXvSetPortAttribute(ClientPtr);
static int ProcXvGetPortAttribute(ClientPtr);
static int ProcXvQueryBestSize(ClientPtr);
static int ProcXvQueryPortAttributes(ClientPtr);
static int ProcXvPutImage(ClientPtr);
#ifdef MITSHM
static int ProcXvShmPutImage(ClientPtr);
#endif
static int ProcXvQueryImageAttributes(ClientPtr);
static int ProcXvListImageFormats(ClientPtr);

static int SProcXvQueryExtension(ClientPtr);
static int SProcXvQueryAdaptors(ClientPtr);
static int SProcXvQueryEncodings(ClientPtr);
static int SProcXvPutVideo(ClientPtr);
static int SProcXvPutStill(ClientPtr);
static int SProcXvGetVideo(ClientPtr);
static int SProcXvGetStill(ClientPtr);
static int SProcXvGrabPort(ClientPtr);
static int SProcXvUngrabPort(ClientPtr);
static int SProcXvSelectVideoNotify(ClientPtr);
static int SProcXvSelectPortNotify(ClientPtr);
static int SProcXvStopVideo(ClientPtr);
static int SProcXvSetPortAttribute(ClientPtr);
static int SProcXvGetPortAttribute(ClientPtr);
static int SProcXvQueryBestSize(ClientPtr);
static int SProcXvQueryPortAttributes(ClientPtr);
static int SProcXvPutImage(ClientPtr);
#ifdef MITSHM
static int SProcXvShmPutImage(ClientPtr);
#endif
static int SProcXvQueryImageAttributes(ClientPtr);
static int SProcXvListImageFormats(ClientPtr);

static int SWriteQueryAdaptorsReply(ClientPtr, xvQueryAdaptorsReply *);
static int SWriteQueryExtensionReply(ClientPtr, xvQueryExtensionReply *);
static int SWriteQueryEncodingsReply(ClientPtr, xvQueryEncodingsReply *);
static int SWriteAdaptorInfo(ClientPtr, xvAdaptorInfo *);
static int SWriteEncodingInfo(ClientPtr, xvEncodingInfo *);
static int SWriteFormat(ClientPtr, xvFormat *);
static int SWriteAttributeInfo(ClientPtr, xvAttributeInfo *);
static int SWriteGrabPortReply(ClientPtr, xvGrabPortReply *);
static int SWriteGetPortAttributeReply(ClientPtr, xvGetPortAttributeReply *);
static int SWriteQueryBestSizeReply(ClientPtr, xvQueryBestSizeReply *);
static int SWriteQueryPortAttributesReply(
		ClientPtr, xvQueryPortAttributesReply *);
static int SWriteQueryImageAttributesReply(
		ClientPtr, xvQueryImageAttributesReply*);
static int SWriteListImageFormatsReply(ClientPtr, xvListImageFormatsReply*);
static int SWriteImageFormatInfo(ClientPtr, xvImageFormatInfo*);

#define _WriteQueryAdaptorsReply(_c,_d) \
  if ((_c)->swapped) SWriteQueryAdaptorsReply(_c, _d); \
  else WriteToClient(_c, sz_xvQueryAdaptorsReply, (char*)_d)

#define _WriteQueryExtensionReply(_c,_d) \
  if ((_c)->swapped) SWriteQueryExtensionReply(_c, _d); \
  else WriteToClient(_c, sz_xvQueryExtensionReply, (char*)_d)

#define _WriteQueryEncodingsReply(_c,_d) \
  if ((_c)->swapped) SWriteQueryEncodingsReply(_c, _d); \
  else WriteToClient(_c, sz_xvQueryEncodingsReply, (char*)_d)

#define _WriteAdaptorInfo(_c,_d) \
  if ((_c)->swapped) SWriteAdaptorInfo(_c, _d); \
  else WriteToClient(_c, sz_xvAdaptorInfo, (char*)_d)

#define _WriteAttributeInfo(_c,_d) \
  if ((_c)->swapped) SWriteAttributeInfo(_c, _d); \
  else WriteToClient(_c, sz_xvAttributeInfo, (char*)_d)

#define _WriteEncodingInfo(_c,_d) \
  if ((_c)->swapped) SWriteEncodingInfo(_c, _d); \
  else WriteToClient(_c, sz_xvEncodingInfo, (char*)_d)

#define _WriteFormat(_c,_d) \
  if ((_c)->swapped) SWriteFormat(_c, _d); \
  else WriteToClient(_c, sz_xvFormat, (char*)_d)

#define _WriteGrabPortReply(_c,_d) \
  if ((_c)->swapped) SWriteGrabPortReply(_c, _d); \
  else WriteToClient(_c, sz_xvGrabPortReply, (char*)_d)

#define _WriteGetPortAttributeReply(_c,_d) \
  if ((_c)->swapped) SWriteGetPortAttributeReply(_c, _d); \
  else WriteToClient(_c, sz_xvGetPortAttributeReply, (char*)_d)

#define _WriteQueryBestSizeReply(_c,_d) \
  if ((_c)->swapped) SWriteQueryBestSizeReply(_c, _d); \
  else WriteToClient(_c, sz_xvQueryBestSizeReply,(char*) _d)

#define _WriteQueryPortAttributesReply(_c,_d) \
  if ((_c)->swapped) SWriteQueryPortAttributesReply(_c, _d); \
  else WriteToClient(_c, sz_xvQueryPortAttributesReply,(char*) _d)

#define _WriteQueryImageAttributesReply(_c,_d) \
  if ((_c)->swapped) SWriteQueryImageAttributesReply(_c, _d); \
  else WriteToClient(_c, sz_xvQueryImageAttributesReply,(char*) _d)

#define _WriteListImageFormatsReply(_c,_d) \
  if ((_c)->swapped) SWriteListImageFormatsReply(_c, _d); \
  else WriteToClient(_c, sz_xvListImageFormatsReply,(char*) _d)

#define _WriteImageFormatInfo(_c,_d) \
  if ((_c)->swapped) SWriteImageFormatInfo(_c, _d); \
  else WriteToClient(_c, sz_xvImageFormatInfo, (char*)_d)

#define _AllocatePort(_i,_p) \
  ((_p)->id != _i) ? (* (_p)->pAdaptor->ddAllocatePort)(_i,_p,&_p) : Success

/*
** ProcXvDispatch
**
**
**
*/

int
ProcXvDispatch(ClientPtr client)
{
  REQUEST(xReq);

  UpdateCurrentTime();

  switch (stuff->data) 
    {
    case xv_QueryExtension: return(ProcXvQueryExtension(client));
    case xv_QueryAdaptors: return(ProcXvQueryAdaptors(client));
    case xv_QueryEncodings: return(ProcXvQueryEncodings(client));
    case xv_PutVideo:
#ifdef PANORAMIX
        if(!noPanoramiXExtension)
            return(XineramaXvPutVideo(client));
        else
#endif
            return(ProcXvPutVideo(client));
    case xv_PutStill:
#ifdef PANORAMIX
        if(!noPanoramiXExtension)
            return(XineramaXvPutStill(client));
        else
#endif
    	    return(ProcXvPutStill(client));
    case xv_GetVideo: return(ProcXvGetVideo(client));
    case xv_GetStill: return(ProcXvGetStill(client));
    case xv_GrabPort: return(ProcXvGrabPort(client));
    case xv_UngrabPort: return(ProcXvUngrabPort(client));
    case xv_SelectVideoNotify: return(ProcXvSelectVideoNotify(client));
    case xv_SelectPortNotify: return(ProcXvSelectPortNotify(client));
    case xv_StopVideo: 
#ifdef PANORAMIX
        if(!noPanoramiXExtension)
	    return(XineramaXvStopVideo(client));
	else
#endif
	    return(ProcXvStopVideo(client));
    case xv_SetPortAttribute: 
#ifdef PANORAMIX
        if(!noPanoramiXExtension)
	    return(XineramaXvSetPortAttribute(client));
	else
#endif
	    return(ProcXvSetPortAttribute(client));
    case xv_GetPortAttribute: return(ProcXvGetPortAttribute(client));
    case xv_QueryBestSize: return(ProcXvQueryBestSize(client));
    case xv_QueryPortAttributes: return(ProcXvQueryPortAttributes(client));
    case xv_PutImage:
#ifdef PANORAMIX
        if(!noPanoramiXExtension)
	    return(XineramaXvPutImage(client));
	else
#endif
	    return(ProcXvPutImage(client));
#ifdef MITSHM
    case xv_ShmPutImage: 
#ifdef PANORAMIX
        if(!noPanoramiXExtension)
	    return(XineramaXvShmPutImage(client));
	else
#endif
	    return(ProcXvShmPutImage(client));
#endif
    case xv_QueryImageAttributes: return(ProcXvQueryImageAttributes(client));
    case xv_ListImageFormats: return(ProcXvListImageFormats(client));
    default:
      if (stuff->data < xvNumRequests)
	{
	  SendErrorToClient(client, XvReqCode, stuff->data, 0, 
			    BadImplementation);
	  return(BadImplementation);
	}
      else
	{
	  SendErrorToClient(client, XvReqCode, stuff->data, 0, BadRequest);
	  return(BadRequest);
	}
    }
}

int
SProcXvDispatch(ClientPtr client)
{
  REQUEST(xReq);

  UpdateCurrentTime();

  switch (stuff->data) 
    {
    case xv_QueryExtension: return(SProcXvQueryExtension(client));
    case xv_QueryAdaptors: return(SProcXvQueryAdaptors(client));
    case xv_QueryEncodings: return(SProcXvQueryEncodings(client));
    case xv_PutVideo: return(SProcXvPutVideo(client));
    case xv_PutStill: return(SProcXvPutStill(client));
    case xv_GetVideo: return(SProcXvGetVideo(client));
    case xv_GetStill: return(SProcXvGetStill(client));
    case xv_GrabPort: return(SProcXvGrabPort(client));
    case xv_UngrabPort: return(SProcXvUngrabPort(client));
    case xv_SelectVideoNotify: return(SProcXvSelectVideoNotify(client));
    case xv_SelectPortNotify: return(SProcXvSelectPortNotify(client));
    case xv_StopVideo: return(SProcXvStopVideo(client));
    case xv_SetPortAttribute: return(SProcXvSetPortAttribute(client));
    case xv_GetPortAttribute: return(SProcXvGetPortAttribute(client));
    case xv_QueryBestSize: return(SProcXvQueryBestSize(client));
    case xv_QueryPortAttributes: return(SProcXvQueryPortAttributes(client));
    case xv_PutImage: return(SProcXvPutImage(client));
#ifdef MITSHM
    case xv_ShmPutImage: return(SProcXvShmPutImage(client));
#endif
    case xv_QueryImageAttributes: return(SProcXvQueryImageAttributes(client));
    case xv_ListImageFormats: return(SProcXvListImageFormats(client));
    default:
      if (stuff->data < xvNumRequests)
	{
	  SendErrorToClient(client, XvReqCode, stuff->data, 0, 
			    BadImplementation);
	  return(BadImplementation);
	}
      else
	{
	  SendErrorToClient(client, XvReqCode, stuff->data, 0, BadRequest);
	  return(BadRequest);
	}
    }
}

static int
ProcXvQueryExtension(ClientPtr client)
{
  xvQueryExtensionReply rep;
  /* REQUEST(xvQueryExtensionReq); */
  REQUEST_SIZE_MATCH(xvQueryExtensionReq);

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.length = 0;
  rep.version = XvVersion;
  rep.revision = XvRevision;

  _WriteQueryExtensionReply(client, &rep);

  return Success;

}

static int
ProcXvQueryAdaptors(ClientPtr client)
{
  xvFormat format;
  xvAdaptorInfo ainfo;
  xvQueryAdaptorsReply rep;
  int totalSize;
  int na;
  XvAdaptorPtr pa;
  int nf;
  XvFormatPtr pf;
  WindowPtr pWin;
  ScreenPtr pScreen;
  XvScreenPtr pxvs;

  REQUEST(xvQueryAdaptorsReq);
  REQUEST_SIZE_MATCH(xvQueryAdaptorsReq);

  if(!(pWin = (WindowPtr)LookupWindow(stuff->window, client) ))
    {
      client->errorValue = stuff->window;
      return (BadWindow);
    }

  pScreen = pWin->drawable.pScreen;
  pxvs = (XvScreenPtr)pScreen->devPrivates[XvScreenIndex].ptr;

  if (!pxvs)
    {
      rep.type = X_Reply;
      rep.sequenceNumber = client->sequence;
      rep.num_adaptors = 0;
      rep.length = 0;

      _WriteQueryAdaptorsReply(client, &rep);

      return Success;
    }

  (* pxvs->ddQueryAdaptors)(pScreen, &pxvs->pAdaptors, &pxvs->nAdaptors);

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.num_adaptors = pxvs->nAdaptors;

  /* CALCULATE THE TOTAL SIZE OF THE REPLY IN BYTES */

  totalSize = pxvs->nAdaptors * sz_xvAdaptorInfo;

  /* FOR EACH ADPATOR ADD UP THE BYTES FOR ENCODINGS AND FORMATS */

  na = pxvs->nAdaptors;
  pa = pxvs->pAdaptors;
  while (na--)
    {
      totalSize += (strlen(pa->name) + 3) & ~3;
      totalSize += pa->nFormats * sz_xvFormat;
      pa++;
    }

  rep.length = totalSize >> 2;

  _WriteQueryAdaptorsReply(client, &rep);

  na = pxvs->nAdaptors;
  pa = pxvs->pAdaptors;
  while (na--)
    {

      ainfo.base_id = pa->base_id;
      ainfo.num_ports = pa->nPorts;
      ainfo.type = pa->type;
      ainfo.name_size = strlen(pa->name);
      ainfo.num_formats = pa->nFormats;

      _WriteAdaptorInfo(client, &ainfo);

      WriteToClient(client, ainfo.name_size, pa->name);

      nf = pa->nFormats;
      pf = pa->pFormats;
      while (nf--)
	{
	  format.depth = pf->depth;
	  format.visual = pf->visual;
	  _WriteFormat(client, &format);
	  pf++;
	}

      pa++;

    }

  return (client->noClientException);

}

static int
ProcXvQueryEncodings(ClientPtr client)
{
  xvEncodingInfo einfo;
  xvQueryEncodingsReply rep;
  int totalSize;
  XvPortPtr pPort;
  int ne;
  XvEncodingPtr pe;
  int status;

  REQUEST(xvQueryEncodingsReq);
  REQUEST_SIZE_MATCH(xvQueryEncodingsReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.num_encodings = pPort->pAdaptor->nEncodings;

  /* FOR EACH ENCODING ADD UP THE BYTES FOR ENCODING NAMES */

  ne = pPort->pAdaptor->nEncodings;
  pe = pPort->pAdaptor->pEncodings;
  totalSize = ne * sz_xvEncodingInfo;
  while (ne--)
    {
      totalSize += (strlen(pe->name) + 3) & ~3;
      pe++;
    }

  rep.length = totalSize >> 2;

  _WriteQueryEncodingsReply(client, &rep);

  ne = pPort->pAdaptor->nEncodings;
  pe = pPort->pAdaptor->pEncodings;
  while (ne--) 
    {
      einfo.encoding = pe->id;
      einfo.name_size = strlen(pe->name);
      einfo.width = pe->width;
      einfo.height = pe->height;
      einfo.rate.numerator = pe->rate.numerator;
      einfo.rate.denominator = pe->rate.denominator;
      _WriteEncodingInfo(client, &einfo);
      WriteToClient(client, einfo.name_size, pe->name);
      pe++;
    }

  return (client->noClientException);

}

static int
ProcXvPutVideo(ClientPtr client)
{
  register DrawablePtr pDraw;
  XvPortPtr pPort;
  register GCPtr pGC;
  int status;

  REQUEST(xvPutVideoReq);
  REQUEST_SIZE_MATCH(xvPutVideoReq);

  VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!(pPort->pAdaptor->type & XvInputMask) ||
	!(pPort->pAdaptor->type & XvVideoMask))
    {
      client->errorValue = stuff->port;
      return (BadMatch);
    }

  status = XVCALL(diMatchPort)(pPort, pDraw);
  if (status != Success)
    {
      return status;
    }

  return XVCALL(diPutVideo)(client, pDraw, pPort, pGC,
			    stuff->vid_x, stuff->vid_y,
			    stuff->vid_w, stuff->vid_h,
			    stuff->drw_x, stuff->drw_y,
			    stuff->drw_w, stuff->drw_h);

}

static int
ProcXvPutStill(ClientPtr client)
{
  register DrawablePtr pDraw;
  XvPortPtr pPort;
  register GCPtr pGC;
  int status;

  REQUEST(xvPutStillReq);
  REQUEST_SIZE_MATCH(xvPutStillReq);

  VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!(pPort->pAdaptor->type & XvInputMask) ||
	!(pPort->pAdaptor->type & XvStillMask))
    {
      client->errorValue = stuff->port;
      return (BadMatch);
    }

  status = XVCALL(diMatchPort)(pPort, pDraw);
  if (status != Success)
    {
      return status;
    }

  return XVCALL(diPutStill)(client, pDraw, pPort, pGC,
			    stuff->vid_x, stuff->vid_y,
			    stuff->vid_w, stuff->vid_h,
			    stuff->drw_x, stuff->drw_y,
			    stuff->drw_w, stuff->drw_h);

}


static int
ProcXvGetVideo(ClientPtr client)
{
  register DrawablePtr pDraw;
  XvPortPtr pPort;
  register GCPtr pGC;
  int status;

  REQUEST(xvGetVideoReq);
  REQUEST_SIZE_MATCH(xvGetVideoReq);

  VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!(pPort->pAdaptor->type & XvOutputMask) ||
	!(pPort->pAdaptor->type & XvVideoMask))
    {
      client->errorValue = stuff->port;
      return (BadMatch);
    }

  status = XVCALL(diMatchPort)(pPort, pDraw);
  if (status != Success)
    {
      return status;
    }

  return XVCALL(diGetVideo)(client, pDraw, pPort, pGC,
			    stuff->vid_x, stuff->vid_y,
			    stuff->vid_w, stuff->vid_h,
			    stuff->drw_x, stuff->drw_y,
			    stuff->drw_w, stuff->drw_h);

}


static int
ProcXvGetStill(ClientPtr client)
{
  register DrawablePtr pDraw;
  XvPortPtr pPort;
  register GCPtr pGC;
  int status;

  REQUEST(xvGetStillReq);
  REQUEST_SIZE_MATCH(xvGetStillReq);

  VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!(pPort->pAdaptor->type & XvOutputMask) ||
	!(pPort->pAdaptor->type & XvStillMask))
    {
      client->errorValue = stuff->port;
      return (BadMatch);
    }

  status = XVCALL(diMatchPort)(pPort, pDraw);
  if (status != Success)
    {
      return status;
    }

  return XVCALL(diGetStill)(client, pDraw, pPort, pGC,
			    stuff->vid_x, stuff->vid_y,
			    stuff->vid_w, stuff->vid_h,
			    stuff->drw_x, stuff->drw_y,
			    stuff->drw_w, stuff->drw_h);

}

static int
ProcXvSelectVideoNotify(ClientPtr client)
{
  register DrawablePtr pDraw;
  REQUEST(xvSelectVideoNotifyReq);
  REQUEST_SIZE_MATCH(xvSelectVideoNotifyReq);

  if(!(pDraw = (DrawablePtr)LOOKUP_DRAWABLE(stuff->drawable, client) ))
    {
      client->errorValue = stuff->drawable;
      return (BadWindow);
    }

  return XVCALL(diSelectVideoNotify)(client, pDraw, stuff->onoff);

}

static int
ProcXvSelectPortNotify(ClientPtr client)
{
  int status;
  XvPortPtr pPort;
  REQUEST(xvSelectPortNotifyReq);
  REQUEST_SIZE_MATCH(xvSelectPortNotifyReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  return XVCALL(diSelectPortNotify)(client, pPort, stuff->onoff);

}

static int
ProcXvGrabPort(ClientPtr client)
{
  int result, status;
  XvPortPtr pPort;
  xvGrabPortReply rep;
  REQUEST(xvGrabPortReq);
  REQUEST_SIZE_MATCH(xvGrabPortReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  status = XVCALL(diGrabPort)(client, pPort, stuff->time, &result);

  if (status != Success)
    {
      return status;
    }

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.length = 0;
  rep.result = result;

  _WriteGrabPortReply(client, &rep);

  return Success;

}

static int
ProcXvUngrabPort(ClientPtr client)
{
  int status;
  XvPortPtr pPort;
  REQUEST(xvGrabPortReq);
  REQUEST_SIZE_MATCH(xvGrabPortReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  return XVCALL(diUngrabPort)(client, pPort, stuff->time);

}


static int
ProcXvStopVideo(ClientPtr client)
{
  int status;
  register DrawablePtr pDraw;
  XvPortPtr pPort;
  REQUEST(xvStopVideoReq);
  REQUEST_SIZE_MATCH(xvStopVideoReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if(!(pDraw = LOOKUP_DRAWABLE(stuff->drawable, client) ))
    {
      client->errorValue = stuff->drawable;
      return (BadDrawable);
    }

  return XVCALL(diStopVideo)(client, pPort, pDraw);

}

static int
ProcXvSetPortAttribute(ClientPtr client)
{
  int status;
  XvPortPtr pPort;
  REQUEST(xvSetPortAttributeReq);
  REQUEST_SIZE_MATCH(xvSetPortAttributeReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!ValidAtom(stuff->attribute))
    {
      client->errorValue = stuff->attribute;
      return(BadAtom);
    }

  status = XVCALL(diSetPortAttribute)(client, pPort, 
				    stuff->attribute, stuff->value);

  if (status == BadMatch) 
      client->errorValue = stuff->attribute;
  else
      client->errorValue = stuff->value;

  return status;
}

static int
ProcXvGetPortAttribute(ClientPtr client)
{
  INT32 value;
  int status;
  XvPortPtr pPort;
  xvGetPortAttributeReply rep;
  REQUEST(xvGetPortAttributeReq);
  REQUEST_SIZE_MATCH(xvGetPortAttributeReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!ValidAtom(stuff->attribute))
    {
      client->errorValue = stuff->attribute;
      return(BadAtom);
    }

  status = XVCALL(diGetPortAttribute)(client, pPort, stuff->attribute, &value);
  if (status != Success)
    {
      client->errorValue = stuff->attribute;
      return status;
    }

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.length = 0;
  rep.value = value;
 
  _WriteGetPortAttributeReply(client, &rep);

  return Success;
}

static int
ProcXvQueryBestSize(ClientPtr client)
{
  int status;
  unsigned int actual_width, actual_height;
  XvPortPtr pPort;
  xvQueryBestSizeReply rep;
  REQUEST(xvQueryBestSizeReq);
  REQUEST_SIZE_MATCH(xvQueryBestSizeReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.length = 0;

  (* pPort->pAdaptor->ddQueryBestSize)(client, pPort, stuff->motion,
				       stuff->vid_w, stuff->vid_h, 
				       stuff->drw_w, stuff->drw_h, 
				       &actual_width, &actual_height);

  rep.actual_width = actual_width;
  rep.actual_height = actual_height;
 
  _WriteQueryBestSizeReply(client, &rep);

  return Success;
}


static int
ProcXvQueryPortAttributes(ClientPtr client)
{
  int status, size, i;
  XvPortPtr pPort;
  XvAttributePtr pAtt;
  xvQueryPortAttributesReply rep;
  xvAttributeInfo Info;
  REQUEST(xvQueryPortAttributesReq);
  REQUEST_SIZE_MATCH(xvQueryPortAttributesReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.num_attributes = pPort->pAdaptor->nAttributes;
  rep.text_size = 0;

  for(i = 0, pAtt = pPort->pAdaptor->pAttributes; 
      i < rep.num_attributes; i++, pAtt++) 
  {    
      rep.text_size += (strlen(pAtt->name) + 1 + 3) & ~3L;
  }

  rep.length = (rep.num_attributes * sz_xvAttributeInfo) + rep.text_size;
  rep.length >>= 2;

  _WriteQueryPortAttributesReply(client, &rep);

  for(i = 0, pAtt = pPort->pAdaptor->pAttributes; 
      i < rep.num_attributes; i++, pAtt++) 
  {
      size = strlen(pAtt->name) + 1;  /* pass the NULL */
      Info.flags = pAtt->flags;
      Info.min = pAtt->min_value;
      Info.max = pAtt->max_value;
      Info.size = (size + 3) & ~3L;

      _WriteAttributeInfo(client, &Info);

      WriteToClient(client, size, pAtt->name);
  }

  return Success;
}



static int 
ProcXvPutImage(ClientPtr client)
{
  DrawablePtr pDraw;
  XvPortPtr pPort;
  XvImagePtr pImage = NULL;
  GCPtr pGC;
  int status, i, size;
  CARD16 width, height;

  REQUEST(xvPutImageReq);
  REQUEST_AT_LEAST_SIZE(xvPutImageReq);

  VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!(pPort->pAdaptor->type & XvImageMask) ||
	!(pPort->pAdaptor->type & XvInputMask))
    {
      client->errorValue = stuff->port;
      return (BadMatch);
    }

  status = XVCALL(diMatchPort)(pPort, pDraw);
  if (status != Success)
    {
      return status;
    }

  for(i = 0; i < pPort->pAdaptor->nImages; i++) {
      if(pPort->pAdaptor->pImages[i].id == stuff->id) {
	  pImage = &(pPort->pAdaptor->pImages[i]);
	  break;
      }
  }

  if(!pImage)
     return BadMatch;

  width = stuff->width;
  height = stuff->height;
  size = (*pPort->pAdaptor->ddQueryImageAttributes)(client, 
			pPort, pImage, &width, &height, NULL, NULL);
  size += sizeof(xvPutImageReq);
  size = (size + 3) >> 2;
  
  if((width < stuff->width) || (height < stuff->height))
     return BadValue;

  if(client->req_len < size)
     return BadLength;

  return XVCALL(diPutImage)(client, pDraw, pPort, pGC, 
			    stuff->src_x, stuff->src_y,
			    stuff->src_w, stuff->src_h,
			    stuff->drw_x, stuff->drw_y,
			    stuff->drw_w, stuff->drw_h,
			    pImage, (unsigned char*)(&stuff[1]), FALSE,
			    stuff->width, stuff->height);
}

#ifdef MITSHM
/* redefined here since it's not in any header file */
typedef struct _ShmDesc {
    struct _ShmDesc *next;
    int shmid;
    int refcnt;
    char *addr;
    Bool writable;
    unsigned long size;
} ShmDescRec, *ShmDescPtr;

extern RESTYPE ShmSegType;
extern int BadShmSegCode;
extern int ShmCompletionCode;

static int 
ProcXvShmPutImage(ClientPtr client)
{
  ShmDescPtr shmdesc;
  DrawablePtr pDraw;
  XvPortPtr pPort;
  XvImagePtr pImage = NULL;
  GCPtr pGC;
  int status, size_needed, i;
  CARD16 width, height;

  REQUEST(xvShmPutImageReq);
  REQUEST_SIZE_MATCH(xvShmPutImageReq);

  VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!(pPort->pAdaptor->type & XvImageMask) ||
	!(pPort->pAdaptor->type & XvInputMask))
    {
      client->errorValue = stuff->port;
      return (BadMatch);
    }

  status = XVCALL(diMatchPort)(pPort, pDraw);
  if (status != Success)
    {
      return status;
    }

  for(i = 0; i < pPort->pAdaptor->nImages; i++) {
      if(pPort->pAdaptor->pImages[i].id == stuff->id) {
	  pImage = &(pPort->pAdaptor->pImages[i]);
	  break;
      }
  }

  if(!pImage)
     return BadMatch;

  if(!(shmdesc = (ShmDescPtr)LookupIDByType(stuff->shmseg, ShmSegType))) 
    {
      client->errorValue = stuff->shmseg;
      return BadShmSegCode;  
    }	
 
  width = stuff->width;
  height = stuff->height;
  size_needed = (*pPort->pAdaptor->ddQueryImageAttributes)(client, 
			pPort, pImage, &width, &height, NULL, NULL);
  if((size_needed + stuff->offset) > shmdesc->size)
      return BadAccess;

  if((width < stuff->width) || (height < stuff->height))
     return BadValue;
     
  status = XVCALL(diPutImage)(client, pDraw, pPort, pGC, 
			    stuff->src_x, stuff->src_y,
			    stuff->src_w, stuff->src_h,
			    stuff->drw_x, stuff->drw_y,
			    stuff->drw_w, stuff->drw_h, pImage,
			    (unsigned char *)shmdesc->addr + stuff->offset, 
			    stuff->send_event, stuff->width, stuff->height);

  if((status == Success) && stuff->send_event) {
        xShmCompletionEvent ev;

        ev.type = ShmCompletionCode;
        ev.drawable = stuff->drawable;
        ev.sequenceNumber = client->sequence;
        ev.minorEvent = xv_ShmPutImage;
        ev.majorEvent = XvReqCode;
        ev.shmseg = stuff->shmseg;
        ev.offset = stuff->offset;
        WriteEventsToClient(client, 1, (xEvent *) &ev);
  }

  return status;
}
#endif

#ifdef XvMCExtension
#include "xvmcext.h"
#endif

static int 
ProcXvQueryImageAttributes(ClientPtr client)
{
  xvQueryImageAttributesReply rep;
  int size, num_planes, i;
  CARD16 width, height;
  XvImagePtr pImage = NULL;
  XvPortPtr pPort;
  int *offsets;
  int *pitches;
  REQUEST(xvQueryImageAttributesReq);

  REQUEST_SIZE_MATCH(xvQueryImageAttributesReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }
  
  for(i = 0; i < pPort->pAdaptor->nImages; i++) {
      if(pPort->pAdaptor->pImages[i].id == stuff->id) {
	  pImage = &(pPort->pAdaptor->pImages[i]);
	  break;
      }
  }

#ifdef XvMCExtension
  if(!pImage)
     pImage = XvMCFindXvImage(pPort, stuff->id);
#endif

  if(!pImage)
     return BadMatch;

  num_planes = pImage->num_planes;

  if(!(offsets = xalloc(num_planes << 3)))
	return BadAlloc;
  pitches = offsets + num_planes;

  width = stuff->width;
  height = stuff->height;

  size = (*pPort->pAdaptor->ddQueryImageAttributes)(client, pPort, pImage,
					&width, &height, offsets, pitches);

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.length = num_planes << 1;
  rep.num_planes = num_planes;
  rep.width = width;
  rep.height = height;
  rep.data_size = size;
 
  _WriteQueryImageAttributesReply(client, &rep);
  if(client->swapped)
    SwapLongs((CARD32*)offsets, rep.length);
  WriteToClient(client, rep.length << 2, (char*)offsets);

  xfree(offsets);

  return Success;
}

static int 
ProcXvListImageFormats(ClientPtr client)
{
  XvPortPtr pPort;
  XvImagePtr pImage;
  int i;
  xvListImageFormatsReply rep;
  xvImageFormatInfo info;
  REQUEST(xvListImageFormatsReq);

  REQUEST_SIZE_MATCH(xvListImageFormatsReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.num_formats = pPort->pAdaptor->nImages;
  rep.length = rep.num_formats * sz_xvImageFormatInfo >> 2;

  _WriteListImageFormatsReply(client, &rep);

  pImage = pPort->pAdaptor->pImages;
  
  for(i = 0; i < rep.num_formats; i++, pImage++) {
     info.id = pImage->id; 	
     info.type = pImage->type; 	
     info.byte_order = pImage->byte_order; 
     memcpy(&info.guid, pImage->guid, 16);	
     info.bpp = pImage->bits_per_pixel; 	
     info.num_planes = pImage->num_planes; 	
     info.depth = pImage->depth; 	
     info.red_mask = pImage->red_mask; 	
     info.green_mask = pImage->green_mask; 	
     info.blue_mask = pImage->blue_mask; 	
     info.format = pImage->format; 	
     info.y_sample_bits = pImage->y_sample_bits; 	
     info.u_sample_bits = pImage->u_sample_bits; 	
     info.v_sample_bits = pImage->v_sample_bits; 	
     info.horz_y_period = pImage->horz_y_period; 	
     info.horz_u_period = pImage->horz_u_period; 	
     info.horz_v_period = pImage->horz_v_period; 	
     info.vert_y_period = pImage->vert_y_period; 	
     info.vert_u_period = pImage->vert_u_period; 	
     info.vert_v_period = pImage->vert_v_period; 	
     memcpy(&info.comp_order, pImage->component_order, 32);	
     info.scanline_order = pImage->scanline_order;
     _WriteImageFormatInfo(client, &info);
  }  

  return Success;
}



/* Swapped Procs */

static int
SProcXvQueryExtension(ClientPtr client)
{
  register char n;
  REQUEST(xvQueryExtensionReq);
  swaps(&stuff->length, n);
  return ProcXvQueryExtension(client);
}

static int
SProcXvQueryAdaptors(ClientPtr client)
{
  register char n;
  REQUEST(xvQueryAdaptorsReq);
  swaps(&stuff->length, n);
  swapl(&stuff->window, n);
  return ProcXvQueryAdaptors(client);
}

static int
SProcXvQueryEncodings(ClientPtr client)
{
  register char n;
  REQUEST(xvQueryEncodingsReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  return ProcXvQueryEncodings(client);
}

static int
SProcXvGrabPort(ClientPtr client)
{
  register char n;
  REQUEST(xvGrabPortReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->time, n);
  return ProcXvGrabPort(client);
}

static int
SProcXvUngrabPort(ClientPtr client)
{
  register char n;
  REQUEST(xvUngrabPortReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->time, n);
  return ProcXvUngrabPort(client);
}

static int
SProcXvPutVideo(ClientPtr client)
{
  register char n;
  REQUEST(xvPutVideoReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->drawable, n);
  swapl(&stuff->gc, n);
  swaps(&stuff->vid_x, n);
  swaps(&stuff->vid_y, n);
  swaps(&stuff->vid_w, n);
  swaps(&stuff->vid_h, n);
  swaps(&stuff->drw_x, n);
  swaps(&stuff->drw_y, n);
  swaps(&stuff->drw_w, n);
  swaps(&stuff->drw_h, n);
  return ProcXvPutVideo(client);
}

static int
SProcXvPutStill(ClientPtr client)
{
  register char n;
  REQUEST(xvPutStillReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->drawable, n);
  swapl(&stuff->gc, n);
  swaps(&stuff->vid_x, n);
  swaps(&stuff->vid_y, n);
  swaps(&stuff->vid_w, n);
  swaps(&stuff->vid_h, n);
  swaps(&stuff->drw_x, n);
  swaps(&stuff->drw_y, n);
  swaps(&stuff->drw_w, n);
  swaps(&stuff->drw_h, n);
  return ProcXvPutStill(client);
}

static int
SProcXvGetVideo(ClientPtr client)
{
  register char n;
  REQUEST(xvGetVideoReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->drawable, n);
  swapl(&stuff->gc, n);
  swaps(&stuff->vid_x, n);
  swaps(&stuff->vid_y, n);
  swaps(&stuff->vid_w, n);
  swaps(&stuff->vid_h, n);
  swaps(&stuff->drw_x, n);
  swaps(&stuff->drw_y, n);
  swaps(&stuff->drw_w, n);
  swaps(&stuff->drw_h, n);
  return ProcXvGetVideo(client);
}

static int
SProcXvGetStill(ClientPtr client)
{
  register char n;
  REQUEST(xvGetStillReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->drawable, n);
  swapl(&stuff->gc, n);
  swaps(&stuff->vid_x, n);
  swaps(&stuff->vid_y, n);
  swaps(&stuff->vid_w, n);
  swaps(&stuff->vid_h, n);
  swaps(&stuff->drw_x, n);
  swaps(&stuff->drw_y, n);
  swaps(&stuff->drw_w, n);
  swaps(&stuff->drw_h, n);
  return ProcXvGetStill(client);
}

static int
SProcXvPutImage(ClientPtr client)
{
  register char n;
  REQUEST(xvPutImageReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->drawable, n);
  swapl(&stuff->gc, n);
  swapl(&stuff->id, n);
  swaps(&stuff->src_x, n);
  swaps(&stuff->src_y, n);
  swaps(&stuff->src_w, n);
  swaps(&stuff->src_h, n);
  swaps(&stuff->drw_x, n);
  swaps(&stuff->drw_y, n);
  swaps(&stuff->drw_w, n);
  swaps(&stuff->drw_h, n);
  swaps(&stuff->width, n);
  swaps(&stuff->height, n);
  return ProcXvPutImage(client);
}

#ifdef MITSHM
static int
SProcXvShmPutImage(ClientPtr client)
{
  register char n;
  REQUEST(xvShmPutImageReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->drawable, n);
  swapl(&stuff->gc, n);
  swapl(&stuff->shmseg, n);
  swapl(&stuff->id, n);
  swaps(&stuff->src_x, n);
  swaps(&stuff->src_y, n);
  swaps(&stuff->src_w, n);
  swaps(&stuff->src_h, n);
  swaps(&stuff->drw_x, n);
  swaps(&stuff->drw_y, n);
  swaps(&stuff->drw_w, n);
  swaps(&stuff->drw_h, n);
  swaps(&stuff->offset, n);
  swaps(&stuff->width, n);
  swaps(&stuff->height, n);
  return ProcXvShmPutImage(client);
}
#endif


static int
SProcXvSelectVideoNotify(ClientPtr client)
{
  register char n;
  REQUEST(xvSelectVideoNotifyReq);
  swaps(&stuff->length, n);
  swapl(&stuff->drawable, n);
  return ProcXvSelectVideoNotify(client);
}

static int
SProcXvSelectPortNotify(ClientPtr client)
{
  register char n;
  REQUEST(xvSelectPortNotifyReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  return ProcXvSelectPortNotify(client);
}

static int
SProcXvStopVideo(ClientPtr client)
{
  register char n;
  REQUEST(xvStopVideoReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->drawable, n);
  return ProcXvStopVideo(client);
}

static int
SProcXvSetPortAttribute(ClientPtr client)
{
  register char n;
  REQUEST(xvSetPortAttributeReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->attribute, n);
  return ProcXvSetPortAttribute(client);
}

static int
SProcXvGetPortAttribute(ClientPtr client)
{
  register char n;
  REQUEST(xvGetPortAttributeReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->attribute, n);
  return ProcXvGetPortAttribute(client);
}

static int
SProcXvQueryBestSize(ClientPtr client)
{
  register char n;
  REQUEST(xvQueryBestSizeReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swaps(&stuff->vid_w, n);
  swaps(&stuff->vid_h, n);
  swaps(&stuff->drw_w, n);
  swaps(&stuff->drw_h, n);
  return ProcXvQueryBestSize(client);
}

static int
SProcXvQueryPortAttributes(ClientPtr client)
{
  register char n;
  REQUEST(xvQueryPortAttributesReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  return ProcXvQueryPortAttributes(client);
}

static int
SProcXvQueryImageAttributes(ClientPtr client)
{
  register char n;
  REQUEST(xvQueryImageAttributesReq);
  swaps(&stuff->length, n);
  swapl(&stuff->id, n);
  swaps(&stuff->width, n);
  swaps(&stuff->width, n);
  return ProcXvQueryImageAttributes(client);
}

static int
SProcXvListImageFormats(ClientPtr client)
{
  register char n;
  REQUEST(xvListImageFormatsReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  return ProcXvListImageFormats(client);
}


static int
SWriteQueryExtensionReply(
   ClientPtr client,
   xvQueryExtensionReply *rep
){
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);
  swaps(&rep->version, n);
  swaps(&rep->revision, n);
  
  (void)WriteToClient(client, sz_xvQueryExtensionReply, (char *)&rep);

  return Success;
}

static int
SWriteQueryAdaptorsReply(
   ClientPtr client,
   xvQueryAdaptorsReply *rep
){
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);
  swaps(&rep->num_adaptors, n);
  
  (void)WriteToClient(client, sz_xvQueryAdaptorsReply, (char *)&rep);

  return Success;
}

static int
SWriteQueryEncodingsReply(
   ClientPtr client,
   xvQueryEncodingsReply *rep
){
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);
  swaps(&rep->num_encodings, n);
  
  (void)WriteToClient(client, sz_xvQueryEncodingsReply, (char *)&rep);

  return Success;
}

static int
SWriteAdaptorInfo(
   ClientPtr client,
   xvAdaptorInfo *pAdaptor
){
  register char n;

  swapl(&pAdaptor->base_id, n);
  swaps(&pAdaptor->name_size, n);
  swaps(&pAdaptor->num_ports, n);
  swaps(&pAdaptor->num_formats, n);

  (void)WriteToClient(client, sz_xvAdaptorInfo, (char *)pAdaptor);

  return Success;
}

static int
SWriteEncodingInfo(
   ClientPtr client,
   xvEncodingInfo *pEncoding
){
  register char n;
  
  swapl(&pEncoding->encoding, n);
  swaps(&pEncoding->name_size, n);
  swaps(&pEncoding->width, n);
  swaps(&pEncoding->height, n);
  swapl(&pEncoding->rate.numerator, n);
  swapl(&pEncoding->rate.denominator, n);
  (void)WriteToClient(client, sz_xvEncodingInfo, (char *)pEncoding);

  return Success;
}

static int
SWriteFormat(
   ClientPtr client,
   xvFormat *pFormat
){
  register char n;

  swapl(&pFormat->visual, n);
  (void)WriteToClient(client, sz_xvFormat, (char *)pFormat);

  return Success;
}

static int
SWriteAttributeInfo(
   ClientPtr client,
   xvAttributeInfo *pAtt
){
  register char n;

  swapl(&pAtt->flags, n);
  swapl(&pAtt->size, n);
  swapl(&pAtt->min, n);
  swapl(&pAtt->max, n);
  (void)WriteToClient(client, sz_xvAttributeInfo, (char *)pAtt);

  return Success;
}

static int
SWriteImageFormatInfo(
   ClientPtr client,
   xvImageFormatInfo *pImage
){
  register char n;

  swapl(&pImage->id, n);
  swapl(&pImage->red_mask, n);
  swapl(&pImage->green_mask, n);
  swapl(&pImage->blue_mask, n);
  swapl(&pImage->y_sample_bits, n);
  swapl(&pImage->u_sample_bits, n);
  swapl(&pImage->v_sample_bits, n);
  swapl(&pImage->horz_y_period, n);
  swapl(&pImage->horz_u_period, n);
  swapl(&pImage->horz_v_period, n);
  swapl(&pImage->vert_y_period, n);
  swapl(&pImage->vert_u_period, n);
  swapl(&pImage->vert_v_period, n);

  (void)WriteToClient(client, sz_xvImageFormatInfo, (char *)pImage);

  return Success;
}



static int
SWriteGrabPortReply(
   ClientPtr client,
   xvGrabPortReply *rep
){
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);

  (void)WriteToClient(client, sz_xvGrabPortReply, (char *)&rep);

  return Success;
}

static int
SWriteGetPortAttributeReply(
   ClientPtr client,
   xvGetPortAttributeReply *rep
){
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);
  swapl(&rep->value, n);

  (void)WriteToClient(client, sz_xvGetPortAttributeReply, (char *)&rep);

  return Success;
}

static int
SWriteQueryBestSizeReply(
   ClientPtr client,
   xvQueryBestSizeReply *rep
){
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);
  swaps(&rep->actual_width, n);
  swaps(&rep->actual_height, n);

  (void)WriteToClient(client, sz_xvQueryBestSizeReply, (char *)&rep);

  return Success;
}

static int
SWriteQueryPortAttributesReply(
   ClientPtr client,
   xvQueryPortAttributesReply *rep
){
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);
  swapl(&rep->num_attributes, n);
  swapl(&rep->text_size, n);

  (void)WriteToClient(client, sz_xvQueryPortAttributesReply, (char *)&rep);

  return Success;
}

static int
SWriteQueryImageAttributesReply(
   ClientPtr client,
   xvQueryImageAttributesReply *rep
){
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);
  swapl(&rep->num_planes, n);
  swapl(&rep->data_size, n);
  swaps(&rep->width, n);
  swaps(&rep->height, n);

  (void)WriteToClient(client, sz_xvQueryImageAttributesReply, (char *)&rep);

  return Success;
}


static int
SWriteListImageFormatsReply(
   ClientPtr client,
   xvListImageFormatsReply *rep
){
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);
  swapl(&rep->num_formats, n);

  (void)WriteToClient(client, sz_xvListImageFormatsReply, (char *)&rep);

  return Success;
}


#ifdef PANORAMIX




static int
XineramaXvStopVideo(ClientPtr client)
{
   int result = Success, i;
   PanoramiXRes *draw, *port;
   REQUEST(xvStopVideoReq);
   REQUEST_SIZE_MATCH(xvStopVideoReq);

   if(!(draw = (PanoramiXRes *)SecurityLookupIDByClass(
                client, stuff->drawable, XRC_DRAWABLE, SecurityWriteAccess)))
        return BadDrawable;

   if(!(port = (PanoramiXRes *)SecurityLookupIDByType(
                client, stuff->port, XvXRTPort, SecurityReadAccess)))
        return _XvBadPort;

   FOR_NSCREENS_BACKWARD(i) {
	if(port->info[i].id) {
	   stuff->drawable = draw->info[i].id;
	   stuff->port = port->info[i].id;
	   result = ProcXvStopVideo(client);
     	}
   }

   return result;
}

static int
XineramaXvSetPortAttribute(ClientPtr client)
{
    REQUEST(xvSetPortAttributeReq);
    PanoramiXRes *port;
    int result = Success, i;

    REQUEST_SIZE_MATCH(xvSetPortAttributeReq);

    if(!(port = (PanoramiXRes *)SecurityLookupIDByType(
                client, stuff->port, XvXRTPort, SecurityReadAccess)))
        return _XvBadPort;

    FOR_NSCREENS_BACKWARD(i) {
	if(port->info[i].id) {
	   stuff->port = port->info[i].id;
	   result = ProcXvSetPortAttribute(client);
	}
    }
    return result;
}


#ifdef MITSHM
static int 
XineramaXvShmPutImage(ClientPtr client)
{
    REQUEST(xvShmPutImageReq);
    PanoramiXRes *draw, *gc, *port;
    Bool send_event = stuff->send_event;
    Bool isRoot;
    int result = Success, i, x, y;

    REQUEST_SIZE_MATCH(xvShmPutImageReq);

    if(!(draw = (PanoramiXRes *)SecurityLookupIDByClass(
                client, stuff->drawable, XRC_DRAWABLE, SecurityWriteAccess)))
        return BadDrawable;

    if(!(gc = (PanoramiXRes *)SecurityLookupIDByType(
                client, stuff->gc, XRT_GC, SecurityReadAccess)))
        return BadGC;    

    if(!(port = (PanoramiXRes *)SecurityLookupIDByType(
                client, stuff->port, XvXRTPort, SecurityReadAccess)))
        return _XvBadPort;
 
    isRoot = (draw->type == XRT_WINDOW) && draw->u.win.root;

    x = stuff->drw_x;
    y = stuff->drw_y;

    FOR_NSCREENS_BACKWARD(i) {
	if(port->info[i].id) {
	   stuff->drawable = draw->info[i].id;
	   stuff->port = port->info[i].id;
	   stuff->gc = gc->info[i].id;
	   stuff->drw_x = x;
	   stuff->drw_y = y;
	   if(isRoot) {
		stuff->drw_x -= panoramiXdataPtr[i].x;
		stuff->drw_y -= panoramiXdataPtr[i].y;
	   }
	   stuff->send_event = (send_event && !i) ? 1 : 0;

	   result = ProcXvShmPutImage(client);
	}
    }
    return result;
}
#endif

static int 
XineramaXvPutImage(ClientPtr client)
{
    REQUEST(xvPutImageReq);
    PanoramiXRes *draw, *gc, *port;
    Bool isRoot;
    int result = Success, i, x, y;

    REQUEST_AT_LEAST_SIZE(xvPutImageReq);

    if(!(draw = (PanoramiXRes *)SecurityLookupIDByClass(
                client, stuff->drawable, XRC_DRAWABLE, SecurityWriteAccess)))
        return BadDrawable;

    if(!(gc = (PanoramiXRes *)SecurityLookupIDByType(
                client, stuff->gc, XRT_GC, SecurityReadAccess)))
        return BadGC;    

    if(!(port = (PanoramiXRes *)SecurityLookupIDByType(
		client, stuff->port, XvXRTPort, SecurityReadAccess)))
	return _XvBadPort;
 
    isRoot = (draw->type == XRT_WINDOW) && draw->u.win.root;

    x = stuff->drw_x;
    y = stuff->drw_y;

    FOR_NSCREENS_BACKWARD(i) {
	if(port->info[i].id) {
	   stuff->drawable = draw->info[i].id;
	   stuff->port = port->info[i].id;
	   stuff->gc = gc->info[i].id;
	   stuff->drw_x = x;
	   stuff->drw_y = y;
	   if(isRoot) {
		stuff->drw_x -= panoramiXdataPtr[i].x;
		stuff->drw_y -= panoramiXdataPtr[i].y;
	   }

	   result = ProcXvPutImage(client);
	}
    }
    return result;
}

static int
XineramaXvPutVideo(ClientPtr client)
{
    REQUEST(xvPutImageReq);
    PanoramiXRes *draw, *gc, *port;
    Bool isRoot;
    int result = Success, i, x, y;

    REQUEST_AT_LEAST_SIZE(xvPutVideoReq);

    if(!(draw = (PanoramiXRes *)SecurityLookupIDByClass(
                client, stuff->drawable, XRC_DRAWABLE, SecurityWriteAccess)))
        return BadDrawable;

    if(!(gc = (PanoramiXRes *)SecurityLookupIDByType(
                client, stuff->gc, XRT_GC, SecurityReadAccess)))
        return BadGC;

    if(!(port = (PanoramiXRes *)SecurityLookupIDByType(
                client, stuff->port, XvXRTPort, SecurityReadAccess)))
        return _XvBadPort;

    isRoot = (draw->type == XRT_WINDOW) && draw->u.win.root;

    x = stuff->drw_x;
    y = stuff->drw_y;

    FOR_NSCREENS_BACKWARD(i) {
        if(port->info[i].id) {
           stuff->drawable = draw->info[i].id;
           stuff->port = port->info[i].id;
           stuff->gc = gc->info[i].id;
           stuff->drw_x = x;
           stuff->drw_y = y;
           if(isRoot) {
                stuff->drw_x -= panoramiXdataPtr[i].x;
                stuff->drw_y -= panoramiXdataPtr[i].y;
           }

           result = ProcXvPutVideo(client);
        }
    }
    return result;
}

static int
XineramaXvPutStill(ClientPtr client)
{
    REQUEST(xvPutImageReq);
    PanoramiXRes *draw, *gc, *port;
    Bool isRoot;
    int result = Success, i, x, y;

    REQUEST_AT_LEAST_SIZE(xvPutImageReq);

    if(!(draw = (PanoramiXRes *)SecurityLookupIDByClass(
                client, stuff->drawable, XRC_DRAWABLE, SecurityWriteAccess)))
        return BadDrawable;

    if(!(gc = (PanoramiXRes *)SecurityLookupIDByType(
                client, stuff->gc, XRT_GC, SecurityReadAccess)))
        return BadGC;

    if(!(port = (PanoramiXRes *)SecurityLookupIDByType(
                client, stuff->port, XvXRTPort, SecurityReadAccess)))
        return _XvBadPort;

    isRoot = (draw->type == XRT_WINDOW) && draw->u.win.root;

    x = stuff->drw_x;
    y = stuff->drw_y;

    FOR_NSCREENS_BACKWARD(i) {
        if(port->info[i].id) {
           stuff->drawable = draw->info[i].id;
           stuff->port = port->info[i].id;
           stuff->gc = gc->info[i].id;
           stuff->drw_x = x;
           stuff->drw_y = y;
           if(isRoot) {
                stuff->drw_x -= panoramiXdataPtr[i].x;
                stuff->drw_y -= panoramiXdataPtr[i].y;
           }

           result = ProcXvPutStill(client);
        }
    }
    return result;
}


void XineramifyXv(void)
{
   ScreenPtr pScreen, screen0 = screenInfo.screens[0];
   XvScreenPtr xvsp0 = (XvScreenPtr)screen0->devPrivates[XvScreenIndex].ptr;
   XvAdaptorPtr refAdapt, pAdapt;
   XvAttributePtr pAttr;
   XvScreenPtr xvsp;
   Bool isOverlay, hasOverlay;
   PanoramiXRes *port;
   XvAdaptorPtr MatchingAdaptors[MAXSCREENS];
   int i, j, k, l;

   XvXRTPort = CreateNewResourceType(XineramaDeleteResource);

   if(!xvsp0) return;
   
   for(i = 0; i < xvsp0->nAdaptors; i++) {
      refAdapt = xvsp0->pAdaptors + i;

      bzero(MatchingAdaptors, sizeof(XvAdaptorPtr) * MAXSCREENS);
      
      MatchingAdaptors[0] = refAdapt;
   
      if(!(refAdapt->type & XvInputMask)) continue;
      
      isOverlay = FALSE;
      for(j = 0; j < refAdapt->nAttributes; j++) {
         pAttr = refAdapt->pAttributes + j;
         if(!strcmp(pAttr->name, "XV_COLORKEY")) {
	    isOverlay = TRUE;
	    break;
	 }
      }
   
      for(j = 1; j < PanoramiXNumScreens; j++) {
         pScreen = screenInfo.screens[j];
	 xvsp = (XvScreenPtr)pScreen->devPrivates[XvScreenIndex].ptr;

         /* Do not try to go on if xv is not supported on this screen */
         if (xvsp==NULL) continue ;
	 
         /* if the adaptor has the same name it's a perfect match */
	 for(k = 0; k < xvsp->nAdaptors; k++) {
	   pAdapt = xvsp->pAdaptors + k;
           if(!strcmp(refAdapt->name, pAdapt->name)) {
	       MatchingAdaptors[j] = pAdapt;
	       break;
	   }
         }
	 if(MatchingAdaptors[j]) continue; /* found it */
	 
	 /* otherwise we only look for XvImage adaptors */
	 if(!(refAdapt->type & XvImageMask)) continue;
	 if(refAdapt->nImages <= 0) continue;
	 
	 /* prefer overlay/overlay non-overlay/non-overlay pairing */
	 for(k = 0; k < xvsp->nAdaptors; k++) {
	    pAdapt = xvsp->pAdaptors + k;
	    if((pAdapt->type & XvImageMask) && (pAdapt->nImages > 0)) {
	      hasOverlay = FALSE;
              for(l = 0; l < pAdapt->nAttributes; l++) {
	         if(!strcmp(pAdapt->name, "XV_COLORKEY")) {
		   hasOverlay = TRUE;
		   break;
		 }
	      }
	      if(isOverlay && hasOverlay) {
	      	 MatchingAdaptors[j] = pAdapt;
		 break;
	      }
              else if(!isOverlay && !hasOverlay) {
	      	 MatchingAdaptors[j] = pAdapt;
		 break;
	      }
	    }
         }
	 
	 if(MatchingAdaptors[j]) continue; /* found it */
	 
	 /* but we'll take any XvImage pairing if we can get it */
	 	 
	 for(k = 0; k < xvsp->nAdaptors; k++) {
	    pAdapt = xvsp->pAdaptors + k;
	    if((pAdapt->type & XvImageMask) && (pAdapt->nImages > 0)) {
	      	 MatchingAdaptors[j] = pAdapt;
		 break;
	    }
         }
      }

      /* now create a resource for each port */
      for(j = 0; j < refAdapt->nPorts; j++) {
         if(!(port = xalloc(sizeof(PanoramiXRes))))
	    break;
	 port->info[0].id = MatchingAdaptors[0]->base_id + j;
	 AddResource(port->info[0].id, XvXRTPort, port);

	 for(k = 1; k < PanoramiXNumScreens; k++) {
	    if(MatchingAdaptors[k] && (MatchingAdaptors[k]->nPorts > j)) 
		port->info[k].id = MatchingAdaptors[k]->base_id + j;
	    else
		port->info[k].id = 0;
	 } 
      }
   }
}

#endif
