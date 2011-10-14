/* $XFree86$ */
/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
** 
** http://oss.sgi.com/projects/FreeB
** 
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
** 
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
** 
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
**
*/

#define NEED_REPLIES
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"

/*
** The last context used by the server.  It is the context that is current
** from the server's perspective.
*/
__GLXcontext *__glXLastContext;

/*
** X resources.
*/
RESTYPE __glXContextRes;
RESTYPE __glXClientRes;
RESTYPE __glXPixmapRes;
RESTYPE __glXSwapBarrierRes;

/*
** Error codes with the extension error base already added in.
*/
int __glXBadContext, __glXBadContextState, __glXBadDrawable, __glXBadPixmap;
int __glXBadContextTag, __glXBadCurrentWindow;
int __glXBadRenderRequest, __glXBadLargeRequest;
int __glXUnsupportedPrivateRequest;

/*
** Reply for most singles.
*/
xGLXSingleReply __glXReply;

/*
** A set of state for each client.  The 0th one is unused because client
** indices start at 1, not 0.
*/
__GLXclientState *__glXClients[MAXCLIENTS+1];

