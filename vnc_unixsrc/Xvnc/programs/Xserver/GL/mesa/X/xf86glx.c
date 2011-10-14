/* $XFree86: xc/programs/Xserver/GL/mesa/src/X/xf86glx.c,v 1.19 2003/07/16 01:38:27 dawes Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *   Brian E. Paul <brian@precisioninsight.com>
 *
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <regionstr.h>
#include <resource.h>
#include <GL/gl.h>
#include <GL/glxint.h>
#include <GL/glxtokens.h>
#include <scrnintstr.h>
#include <config.h>
#include <glxserver.h>
#include <glxscreens.h>
#include <glxdrawable.h>
#include <glxcontext.h>
#include <glxext.h>
#include <glxutil.h>
#include "xf86glxint.h"
#include "context.h"
#include "xmesaP.h"
#include <GL/xf86glx.h>
#include "context.h"

/*
 * This define is for the glcore.h header file.
 * If you add it here, then make sure you also add it in
 * ../../../glx/Imakefile.
 */
#if 0
#define DEBUG
#include <GL/internal/glcore.h>
#undef DEBUG
#else
#include <GL/internal/glcore.h>
#endif

#include "glcontextmodes.h"

/*
 * This structure is statically allocated in the __glXScreens[]
 * structure.  This struct is not used anywhere other than in
 * __glXScreenInit to initialize each of the active screens
 * (__glXActiveScreens[]).  Several of the fields must be initialized by
 * the screenProbe routine before they are copied to the active screens
 * struct.  In particular, the contextCreate, pGlxVisual, numVisuals,
 * and numUsableVisuals fields must be initialized.
 */
static __GLXscreenInfo __glDDXScreenInfo = {
    __MESA_screenProbe,   /* Must be generic and handle all screens */
    __MESA_createContext, /* Substitute screen's createContext routine */
    __MESA_createBuffer,  /* Substitute screen's createBuffer routine */
    NULL,                 /* Set up modes in probe */
    NULL,                 /* Set up pVisualPriv in probe */
    0,                    /* Set up numVisuals in probe */
    0,                    /* Set up numUsableVisuals in probe */
    NULL,                 /* GLextensions is overwritten by __glXScreenInit */
    "Vendor String",      /* GLXvendor is overwritten by __glXScreenInit */
    "Version String",     /* GLXversion is overwritten by __glXScreenInit */
    "Extensions String",  /* GLXextensions is overwritten by __glXScreenInit */
    NULL                  /* WrappedPositionWindow is overwritten */
};

void *__glXglDDXScreenInfo(void) {
    return &__glDDXScreenInfo;
}

static __GLXextensionInfo __glDDXExtensionInfo = {
    GL_CORE_MESA,
    __MESA_resetExtension,
    __MESA_initVisuals,
    __MESA_setVisualConfigs
};

void *__glXglDDXExtensionInfo(void) {
    return &__glDDXExtensionInfo;
}

static __MESA_screen  MESAScreens[MAXSCREENS];
static __GLcontext   *MESA_CC        = NULL;

static int                 numConfigs     = 0;
static __GLXvisualConfig  *visualConfigs  = NULL;
static void              **visualPrivates = NULL;


static int count_bits(unsigned int n)
{
   int bits = 0;

   while (n > 0) {
      if (n & 1) bits++;
      n >>= 1;
   }
   return bits;
}


static XMesaVisual find_mesa_visual(int screen, VisualID vid)
{
    __MESA_screen * const pMScr = &MESAScreens[screen];
    const __GLcontextModes *modes;
    unsigned i = 0;

    for ( modes = pMScr->modes ; modes != NULL ; modes = modes->next ) {
	if ( modes->visualID == vid ) {
	    break;
	}

	i++;
    }

    return (modes != NULL) ? pMScr->xm_vis[i] : NULL;
}


/*
 * In the case the driver defines no GLX visuals we'll use these.
 * Note that for TrueColor and DirectColor visuals, bufferSize is the 
 * sum of redSize, greenSize, blueSize and alphaSize, which may be larger 
 * than the nplanes/rootDepth of the server's X11 visuals
 */
#define NUM_FALLBACK_CONFIGS 5
static __GLXvisualConfig FallbackConfigs[NUM_FALLBACK_CONFIGS] = {
  /* [0] = RGB, double buffered, Z */
  {
    -1,                 /* vid */
    -1,                 /* class */
    True,               /* rgba */
    -1, -1, -1, 0,      /* rgba sizes */
    -1, -1, -1, 0,      /* rgba masks */
     0,  0,  0, 0,      /* rgba accum sizes */
    True,               /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    0,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
  /* [1] = RGB, double buffered, Z, stencil, accum */
  {
    -1,                 /* vid */
    -1,                 /* class */
    True,               /* rgba */
    -1, -1, -1, 0,      /* rgba sizes */
    -1, -1, -1, 0,      /* rgba masks */
    16, 16, 16, 0,      /* rgba accum sizes */
    True,               /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    8,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
  /* [2] = RGB+Alpha, double buffered, Z, stencil, accum */
  {
    -1,                 /* vid */
    -1,                 /* class */
    True,               /* rgba */
    -1, -1, -1, 8,      /* rgba sizes */
    -1, -1, -1, -1,     /* rgba masks */
    16, 16, 16, 16,     /* rgba accum sizes */
    True,               /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    8,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
  /* [3] = RGB+Alpha, single buffered, Z, stencil, accum */
  {
    -1,                 /* vid */
    -1,                 /* class */
    True,               /* rgba */
    -1, -1, -1, 8,      /* rgba sizes */
    -1, -1, -1, -1,     /* rgba masks */
    16, 16, 16, 16,     /* rgba accum sizes */
    False,              /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    8,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
  /* [4] = CI, double buffered, Z */
  {
    -1,                 /* vid */
    -1,                 /* class */
    False,              /* rgba? (false = color index) */
    -1, -1, -1, 0,      /* rgba sizes */
    -1, -1, -1, 0,      /* rgba masks */
     0,  0,  0, 0,      /* rgba accum sizes */
    True,               /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    0,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
};


static Bool init_visuals(int *nvisualp, VisualPtr *visualp,
			 VisualID *defaultVisp,
			 int ndepth, DepthPtr pdepth,
			 int rootDepth)
{
    int numRGBconfigs;
    int numCIconfigs;
    int numVisuals = *nvisualp;
    int numNewVisuals;
    int numNewConfigs;
    VisualPtr pVisual = *visualp;
    VisualPtr pVisualNew = NULL;
    VisualID *orig_vid = NULL;
    __GLcontextModes *modes;
    __GLXvisualConfig *pNewVisualConfigs = NULL;
    void **glXVisualPriv;
    void **pNewVisualPriv;
    int found_default;
    int i, j, k;

    if (numConfigs > 0)
        numNewConfigs = numConfigs;
    else
        numNewConfigs = NUM_FALLBACK_CONFIGS;

    /* Alloc space for the list of new GLX visuals */
    pNewVisualConfigs = (__GLXvisualConfig *)
                     __glXMalloc(numNewConfigs * sizeof(__GLXvisualConfig));
    if (!pNewVisualConfigs) {
	return FALSE;
    }

    /* Alloc space for the list of new GLX visual privates */
    pNewVisualPriv = (void **) __glXMalloc(numNewConfigs * sizeof(void *));
    if (!pNewVisualPriv) {
	__glXFree(pNewVisualConfigs);
	return FALSE;
    }

    /*
    ** If SetVisualConfigs was not called, then use default GLX
    ** visual configs.
    */
    if (numConfigs == 0) {
	memcpy(pNewVisualConfigs, FallbackConfigs,
               NUM_FALLBACK_CONFIGS * sizeof(__GLXvisualConfig));
	memset(pNewVisualPriv, 0, NUM_FALLBACK_CONFIGS * sizeof(void *));
    }
    else {
        /* copy driver's visual config info */
        for (i = 0; i < numConfigs; i++) {
            pNewVisualConfigs[i] = visualConfigs[i];
            pNewVisualPriv[i] = visualPrivates[i];
        }
    }

    /* Count the number of RGB and CI visual configs */
    numRGBconfigs = 0;
    numCIconfigs = 0;
    for (i = 0; i < numNewConfigs; i++) {
	if (pNewVisualConfigs[i].rgba)
	    numRGBconfigs++;
	else
	    numCIconfigs++;
    }

    /* Count the total number of visuals to compute */
    numNewVisuals = 0;
    for (i = 0; i < numVisuals; i++) {
        numNewVisuals +=
	    (pVisual[i].class == TrueColor || pVisual[i].class == DirectColor)
	    ? numRGBconfigs : numCIconfigs;
    }

    /* Reset variables for use with the next screen/driver's visual configs */
    visualConfigs = NULL;
    numConfigs = 0;

    /* Alloc temp space for the list of orig VisualIDs for each new visual */
    orig_vid = (VisualID *)__glXMalloc(numNewVisuals * sizeof(VisualID));
    if (!orig_vid) {
	__glXFree(pNewVisualPriv);
	__glXFree(pNewVisualConfigs);
	return FALSE;
    }

    /* Alloc space for the list of glXVisuals */
    modes = _gl_context_modes_create(numNewVisuals, sizeof(__GLcontextModes));
    if (modes == NULL) {
	__glXFree(orig_vid);
	__glXFree(pNewVisualPriv);
	__glXFree(pNewVisualConfigs);
	return FALSE;
    }

    /* Alloc space for the list of glXVisualPrivates */
    glXVisualPriv = (void **)__glXMalloc(numNewVisuals * sizeof(void *));
    if (!glXVisualPriv) {
	_gl_context_modes_destroy( modes );
	__glXFree(orig_vid);
	__glXFree(pNewVisualPriv);
	__glXFree(pNewVisualConfigs);
	return FALSE;
    }

    /* Alloc space for the new list of the X server's visuals */
    pVisualNew = (VisualPtr)__glXMalloc(numNewVisuals * sizeof(VisualRec));
    if (!pVisualNew) {
	__glXFree(glXVisualPriv);
	_gl_context_modes_destroy( modes );
	__glXFree(orig_vid);
	__glXFree(pNewVisualPriv);
	__glXFree(pNewVisualConfigs);
	return FALSE;
    }

    /* Initialize the new visuals */
    found_default = FALSE;
    MESAScreens[screenInfo.numScreens-1].modes = modes;
    for (i = j = 0; i < numVisuals; i++) {
        int is_rgb = (pVisual[i].class == TrueColor ||
		      pVisual[i].class == DirectColor);

	for (k = 0; k < numNewConfigs; k++) {
	    if (pNewVisualConfigs[k].rgba != is_rgb)
		continue;

	    assert( modes != NULL );

	    /* Initialize the new visual */
	    pVisualNew[j] = pVisual[i];
	    pVisualNew[j].vid = FakeClientID(0);

	    /* Check for the default visual */
	    if (!found_default && pVisual[i].vid == *defaultVisp) {
		*defaultVisp = pVisualNew[j].vid;
		found_default = TRUE;
	    }

	    /* Save the old VisualID */
	    orig_vid[j] = pVisual[i].vid;

	    /* Initialize the glXVisual */
	    _gl_copy_visual_to_context_mode( modes, & pNewVisualConfigs[k] );
	    modes->visualID = pVisualNew[j].vid;

	    /*
	     * If the class is -1, then assume the X visual information
	     * is identical to what GLX needs, and take them from the X
	     * visual.  NOTE: if class != -1, then all other fields MUST
	     * be initialized.
	     */
	    if (modes->visualType == GLX_NONE) {
		modes->visualType = _gl_convert_from_x_visual_type( pVisual[i].class );
		modes->redBits    = count_bits(pVisual[i].redMask);
		modes->greenBits  = count_bits(pVisual[i].greenMask);
		modes->blueBits   = count_bits(pVisual[i].blueMask);
		modes->alphaBits  = modes->alphaBits;
		modes->redMask    = pVisual[i].redMask;
		modes->greenMask  = pVisual[i].greenMask;
		modes->blueMask   = pVisual[i].blueMask;
		modes->alphaMask  = modes->alphaMask;
		modes->rgbBits = (is_rgb)
		    ? (modes->redBits + modes->greenBits +
		       modes->blueBits + modes->alphaBits)
		    : rootDepth;
	    }

	    /* Save the device-dependent private for this visual */
	    glXVisualPriv[j] = pNewVisualPriv[k];

	    j++;
	    modes = modes->next;
	}
    }

    assert(j <= numNewVisuals);

    /* Save the GLX visuals in the screen structure */
    MESAScreens[screenInfo.numScreens-1].num_vis = numNewVisuals;
    MESAScreens[screenInfo.numScreens-1].private = glXVisualPriv;

    /* Set up depth's VisualIDs */
    for (i = 0; i < ndepth; i++) {
	int numVids = 0;
	VisualID *pVids = NULL;
	int k, n = 0;

	/* Count the new number of VisualIDs at this depth */
	for (j = 0; j < pdepth[i].numVids; j++)
	    for (k = 0; k < numNewVisuals; k++)
		if (pdepth[i].vids[j] == orig_vid[k])
		    numVids++;

	/* Allocate a new list of VisualIDs for this depth */
	pVids = (VisualID *)__glXMalloc(numVids * sizeof(VisualID));

	/* Initialize the new list of VisualIDs for this depth */
	for (j = 0; j < pdepth[i].numVids; j++)
	    for (k = 0; k < numNewVisuals; k++)
		if (pdepth[i].vids[j] == orig_vid[k])
		    pVids[n++] = pVisualNew[k].vid;

	/* Update this depth's list of VisualIDs */
	__glXFree(pdepth[i].vids);
	pdepth[i].vids = pVids;
	pdepth[i].numVids = numVids;
    }

    /* Update the X server's visuals */
    *nvisualp = numNewVisuals;
    *visualp = pVisualNew;

    /* Free the old list of the X server's visuals */
    __glXFree(pVisual);

    /* Clean up temporary allocations */
    __glXFree(orig_vid);
    __glXFree(pNewVisualPriv);
    __glXFree(pNewVisualConfigs);

    /* Free the private list created by DDX HW driver */
    if (visualPrivates)
        xfree(visualPrivates);
    visualPrivates = NULL;

    return TRUE;
}

void __MESA_setVisualConfigs(int nconfigs, __GLXvisualConfig *configs,
			     void **privates)
{
    numConfigs = nconfigs;
    visualConfigs = configs;
    visualPrivates = privates;
}

Bool __MESA_initVisuals(VisualPtr *visualp, DepthPtr *depthp,
			int *nvisualp, int *ndepthp, int *rootDepthp,
			VisualID *defaultVisp, unsigned long sizes,
			int bitsPerRGB)
{
    /*
     * Setup the visuals supported by this particular screen.
     */
    return init_visuals(nvisualp, visualp, defaultVisp,
			*ndepthp, *depthp, *rootDepthp);
}

static void fixup_visuals(int screen)
{
    ScreenPtr pScreen = screenInfo.screens[screen];
    __MESA_screen *pMScr = &MESAScreens[screen];
    int j;
    __GLcontextModes *modes;

    for ( modes = pMScr->modes ; modes != NULL ; modes = modes->next ) {
	const int vis_class = _gl_convert_to_x_visual_type( modes->visualType );
	const int nplanes = (modes->rgbBits - modes->alphaBits);
	const VisualPtr pVis = pScreen->visuals;

	/* Find a visual that matches the GLX visual's class and size */
	for (j = 0; j < pScreen->numVisuals; j++) {
	    if (pVis[j].class == vis_class &&
		pVis[j].nplanes == nplanes) {

		/* Fixup the masks */
		modes->redMask   = pVis[j].redMask;
		modes->greenMask = pVis[j].greenMask;
		modes->blueMask  = pVis[j].blueMask;

		/* Recalc the sizes */
		modes->redBits   = count_bits(modes->redMask);
		modes->greenBits = count_bits(modes->greenMask);
		modes->blueBits  = count_bits(modes->blueMask);
	    }
	}
    }
}

static void init_screen_visuals(int screen)
{
    ScreenPtr pScreen = screenInfo.screens[screen];
    __GLcontextModes *modes;
    XMesaVisual *pXMesaVisual;
    int *used;
    int i, j;

    /* Alloc space for the list of XMesa visuals */
    pXMesaVisual = (XMesaVisual *)__glXMalloc(MESAScreens[screen].num_vis *
					      sizeof(XMesaVisual));
    __glXMemset(pXMesaVisual, 0,
		MESAScreens[screen].num_vis * sizeof(XMesaVisual));

    /* FIXME: Change 'used' to be a array of bits (rather than of ints),
     * FIXME: create a stack array of 8 or 16 bytes.  If 'numVisuals' is less
     * FIXME: than 64 or 128 the stack array can be used instead of calling
     * FIXME: __glXMalloc / __glXFree.  If nothing else, convert 'used' to
     * FIXME: array of bytes instead of ints!
     */
    used = (int *)__glXMalloc(pScreen->numVisuals * sizeof(int));
    __glXMemset(used, 0, pScreen->numVisuals * sizeof(int));

    i = 0;
    for ( modes = MESAScreens[screen].modes 
	  ; modes != NULL
	  ; modes = modes->next ) {
	const int vis_class = _gl_convert_to_x_visual_type( modes->visualType );
	const int nplanes = (modes->rgbBits - modes->alphaBits);
	const VisualPtr pVis = pScreen->visuals;

	for (j = 0; j < pScreen->numVisuals; j++) {
	    if (pVis[j].class     == vis_class &&
		pVis[j].nplanes   == nplanes &&
		pVis[j].redMask   == modes->redMask &&
		pVis[j].greenMask == modes->greenMask &&
		pVis[j].blueMask  == modes->blueMask &&
		!used[j]) {

		/* Create the XMesa visual */
		pXMesaVisual[i] =
		    XMesaCreateVisual(pScreen,
				      pVis,
				      modes->rgbMode,
				      (modes->alphaBits > 0),
				      modes->doubleBufferMode,
				      modes->stereoMode,
				      GL_TRUE, /* ximage_flag */
				      modes->depthBits,
				      modes->stencilBits,
				      modes->accumRedBits,
				      modes->accumGreenBits,
				      modes->accumBlueBits,
				      modes->accumAlphaBits,
				      modes->samples,
				      modes->level,
				      modes->visualRating);
		/* Set the VisualID */
		modes->visualID = pVis[j].vid;

		/* Mark this visual used */
		used[j] = 1;
		break;
	    }
	}

	if ( j == pScreen->numVisuals ) {
	    ErrorF("No matching visual for __GLcontextMode with "
		   "visual class = %d (%d), nplanes = %u\n",
		   vis_class, 
		   modes->visualType,
		   (modes->rgbBits - modes->alphaBits) );
	}
	else if ( modes->visualID == -1 ) {
	    FatalError( "Matching visual found, but visualID still -1!\n" );
	}

	i++;
    }

    __glXFree(used);

    MESAScreens[screen].xm_vis = pXMesaVisual;
}

Bool __MESA_screenProbe(int screen)
{
    /*
     * Set up the current screen's visuals.
     */
    __glDDXScreenInfo.modes = MESAScreens[screen].modes;
    __glDDXScreenInfo.pVisualPriv = MESAScreens[screen].private;
    __glDDXScreenInfo.numVisuals =
	__glDDXScreenInfo.numUsableVisuals = MESAScreens[screen].num_vis;

    /*
     * Set the current screen's createContext routine.  This could be
     * wrapped by a DDX GLX context creation routine.
     */
    __glDDXScreenInfo.createContext = __MESA_createContext;

    /*
     * The ordering of the rgb compenents might have been changed by the
     * driver after mi initialized them.
     */
    fixup_visuals(screen);

    /*
     * Find the GLX visuals that are supported by this screen and create
     * XMesa's visuals.
     */
    init_screen_visuals(screen);

    return TRUE;
}

extern void __MESA_resetExtension(void)
{
    int i, j;

    XMesaReset();

    for (i = 0; i < screenInfo.numScreens; i++) {
	for (j = 0; j < MESAScreens[i].num_vis; j++) {
	  if (MESAScreens[i].xm_vis[j]) {
	        XMesaDestroyVisual(MESAScreens[i].xm_vis[j]);
		MESAScreens[i].xm_vis[j] = NULL;
	  }
	}
	_gl_context_modes_destroy( MESAScreens[i].modes );
	MESAScreens[i].modes = NULL;
	__glXFree(MESAScreens[i].private);
	MESAScreens[i].private = NULL;
	__glXFree(MESAScreens[i].xm_vis);
	MESAScreens[i].xm_vis = NULL;
	MESAScreens[i].num_vis = 0;
    }
    __glDDXScreenInfo.modes = NULL;
    MESA_CC = NULL;
}

void __MESA_createBuffer(__GLXdrawablePrivate *glxPriv)
{
    DrawablePtr pDraw = glxPriv->pDraw;
    XMesaVisual xm_vis = find_mesa_visual(pDraw->pScreen->myNum,
					  glxPriv->modes->visualID);
    __GLdrawablePrivate *glPriv = &glxPriv->glPriv;
    __MESA_buffer buf;

    if (xm_vis == NULL) {
	ErrorF("find_mesa_visual returned NULL for visualID = 0x%04x\n",
	       glxPriv->modes->visualID);
    }
    buf = (__MESA_buffer)__glXMalloc(sizeof(struct __MESA_bufferRec));

    /* Create Mesa's buffers */
    if (glxPriv->type == DRAWABLE_WINDOW) {
	buf->xm_buf = (void *)XMesaCreateWindowBuffer(xm_vis,
						      (WindowPtr)pDraw);
    } else {
	buf->xm_buf = (void *)XMesaCreatePixmapBuffer(xm_vis,
						      (PixmapPtr)pDraw, 0);
    }

    /* Wrap the front buffer's resize routine */
    buf->fbresize = glPriv->frontBuffer.resize;
    glPriv->frontBuffer.resize = __MESA_resizeBuffers;

    /* Wrap the swap buffers routine */
    buf->fbswap = glxPriv->swapBuffers;
    glxPriv->swapBuffers = __MESA_swapBuffers;

    /* Save Mesa's private buffer structure */
    glPriv->private = (void *)buf;
    glPriv->freePrivate = __MESA_destroyBuffer;
}

GLboolean __MESA_resizeBuffers(__GLdrawableBuffer *buffer,
			       GLint x, GLint y,
			       GLuint width, GLuint height, 
			       __GLdrawablePrivate *glPriv,
			       GLuint bufferMask)
{
    __MESA_buffer buf = (__MESA_buffer)glPriv->private;

    if (buf->xm_buf)
	XMesaResizeBuffers(buf->xm_buf);

    return (*buf->fbresize)(buffer, x, y, width, height, glPriv, bufferMask);
}

GLboolean __MESA_swapBuffers(__GLXdrawablePrivate *glxPriv)
{
    __MESA_buffer buf = (__MESA_buffer)glxPriv->glPriv.private;

    /*
    ** Do not call the wrapped swap buffers routine since Mesa has
    ** already done the swap.
    */
    XMesaSwapBuffers(buf->xm_buf);

    return GL_TRUE;
}

void __MESA_destroyBuffer(__GLdrawablePrivate *glPriv)
{
    __MESA_buffer buf = (__MESA_buffer)glPriv->private;
    __GLXdrawablePrivate *glxPriv = (__GLXdrawablePrivate *)glPriv->other;

    /* Destroy Mesa's buffers */
    if (buf->xm_buf)
	XMesaDestroyBuffer(buf->xm_buf);

    /* Unwrap these routines */
    glxPriv->swapBuffers = buf->fbswap;
    glPriv->frontBuffer.resize = buf->fbresize;

    __glXFree(glPriv->private);
    glPriv->private = NULL;
}

__GLinterface *__MESA_createContext(__GLimports *imports,
				    __GLcontextModes *modes,
				    __GLinterface *shareGC)
{
    __GLcontext *gl_ctx = NULL;
    __GLcontext *m_share = NULL;
    __GLXcontext *glxc = (__GLXcontext *)imports->other;
    XMesaVisual xm_vis;

    if (shareGC) 
       m_share = (__GLcontext *)shareGC;

    xm_vis = find_mesa_visual(glxc->pScreen->myNum, glxc->modes->visualID);
    if (xm_vis) {
       XMesaContext xmshare = m_share ? m_share->DriverCtx : 0;
       XMesaContext xmctx = XMesaCreateContext(xm_vis, xmshare);
       gl_ctx = xmctx ? &xmctx->mesa : 0;
    }
    else {
	ErrorF("find_mesa_visual returned NULL for visualID = 0x%04x\n",
	       glxc->modes->visualID);
    }


    if (!gl_ctx)
       return NULL;
	
    gl_ctx->imports = *imports;
    gl_ctx->exports.destroyContext = __MESA_destroyContext;
    gl_ctx->exports.loseCurrent = __MESA_loseCurrent;
    gl_ctx->exports.makeCurrent = __MESA_makeCurrent;
    gl_ctx->exports.shareContext = __MESA_shareContext;
    gl_ctx->exports.copyContext = __MESA_copyContext;
    gl_ctx->exports.forceCurrent = __MESA_forceCurrent;
    gl_ctx->exports.notifyResize = __MESA_notifyResize;
    gl_ctx->exports.notifyDestroy = __MESA_notifyDestroy;
    gl_ctx->exports.notifySwapBuffers = __MESA_notifySwapBuffers;
    gl_ctx->exports.dispatchExec = __MESA_dispatchExec;
    gl_ctx->exports.beginDispatchOverride = __MESA_beginDispatchOverride;
    gl_ctx->exports.endDispatchOverride = __MESA_endDispatchOverride;

    return (__GLinterface *)gl_ctx;
}

GLboolean __MESA_destroyContext(__GLcontext *gc)
{
    XMesaContext xmesa = (XMesaContext) gc->DriverCtx;
    XMesaDestroyContext( xmesa );
    return GL_TRUE;
}

GLboolean __MESA_loseCurrent(__GLcontext *gc)
{
    XMesaContext xmesa = (XMesaContext) gc->DriverCtx;
    MESA_CC = NULL;
    __glXLastContext = NULL;
    return XMesaLoseCurrent(xmesa);
}

GLboolean __MESA_makeCurrent(__GLcontext *gc)
{
    __GLdrawablePrivate *drawPriv = gc->imports.getDrawablePrivate( gc );
    __MESA_buffer drawBuf = (__MESA_buffer)drawPriv->private;
    __GLdrawablePrivate *readPriv = gc->imports.getReadablePrivate( gc );
    __MESA_buffer readBuf = (__MESA_buffer)readPriv->private;
    XMesaContext xmesa = (XMesaContext) gc->DriverCtx;

    MESA_CC = gc;
    return XMesaMakeCurrent2(xmesa, drawBuf->xm_buf, readBuf->xm_buf);
}

GLboolean __MESA_shareContext(__GLcontext *gc, __GLcontext *gcShare)
{
    /* NOT_DONE */
    /* XXX I don't see where/how this could ever be called */
    ErrorF("__MESA_shareContext\n");
    return GL_FALSE;
}

GLboolean __MESA_copyContext(__GLcontext *dst, const __GLcontext *src,
			     GLuint mask)
{
    XMesaContext xm_dst = (XMesaContext) dst->DriverCtx;
    const XMesaContext xm_src = (const XMesaContext) src->DriverCtx;
    _mesa_copy_context(&xm_src->mesa, &xm_dst->mesa, mask);
    return GL_TRUE;
}

GLboolean __MESA_forceCurrent(__GLcontext *gc)
{
    XMesaContext xmesa = (XMesaContext) gc->DriverCtx;
    MESA_CC = gc;
    return XMesaForceCurrent(xmesa);
}

GLboolean __MESA_notifyResize(__GLcontext *gc)
{
    /* NOT_DONE */
    ErrorF("__MESA_notifyResize\n");
    return GL_FALSE;
}

void __MESA_notifyDestroy(__GLcontext *gc)
{
    /* NOT_DONE */
    ErrorF("__MESA_notifyDestroy\n");
    return;
}

void __MESA_notifySwapBuffers(__GLcontext *gc)
{
    _mesa_notifySwapBuffers(gc);
}

struct __GLdispatchStateRec *__MESA_dispatchExec(__GLcontext *gc)
{
    /* NOT_DONE */
    ErrorF("__MESA_dispatchExec\n");
    return NULL;
}

void __MESA_beginDispatchOverride(__GLcontext *gc)
{
    /* NOT_DONE */
    ErrorF("__MESA_beginDispatchOverride\n");
    return;
}

void __MESA_endDispatchOverride(__GLcontext *gc)
{
    /* NOT_DONE */
    ErrorF("__MESA_endDispatchOverride\n");
    return;
}


/*
 * Server-side GLX uses these functions which are normally defined
 * in the OpenGL SI.
 */

GLuint __glFloorLog2(GLuint val)
{
    int c = 0;

    while (val > 1) {
	c++;
	val >>= 1;
    }
    return c;
}

