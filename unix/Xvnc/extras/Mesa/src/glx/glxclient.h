/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice including the dates of first publication and
 * either this permission notice or a reference to
 * http://oss.sgi.com/projects/FreeB/
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * SILICON GRAPHICS, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Silicon Graphics, Inc.
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization from
 * Silicon Graphics, Inc.
 */

/**
 * \file glxclient.h
 * Direct rendering support added by Precision Insight, Inc.
 *
 * \author Kevin E. Martin <kevin@precisioninsight.com>
 */

#ifndef _GLX_client_h_
#define _GLX_client_h_
#include <X11/Xproto.h>
#include <X11/Xlibint.h>
#include <X11/Xfuncproto.h>
#include <X11/extensions/extutil.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "GL/glxproto.h"
#include "glxconfig.h"
#include "glxhash.h"
#include "util/macros.h"

#include "glxextensions.h"

#if defined(USE_LIBGLVND)
#define _GLX_PUBLIC _X_HIDDEN
#else
#define _GLX_PUBLIC _X_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define GLX_MAJOR_VERSION 1       /* current version numbers */
#define GLX_MINOR_VERSION 4

#define __GLX_MAX_TEXTURE_UNITS 32

struct glx_display;
struct glx_context;

/************************************************************************/

#ifdef GLX_DIRECT_RENDERING

extern void DRI_glXUseXFont(struct glx_context *ctx,
			    Font font, int first, int count, int listbase);

#endif

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)

/**
 * Display dependent methods.  This structure is initialized during the
 * \c driCreateDisplay call.
 */
typedef struct __GLXDRIdisplayRec __GLXDRIdisplay;
typedef struct __GLXDRIscreenRec __GLXDRIscreen;
typedef struct __GLXDRIdrawableRec __GLXDRIdrawable;

struct __GLXDRIdisplayRec
{
    /**
     * Method to destroy the private DRI display data.
     */
   void (*destroyDisplay) (__GLXDRIdisplay * display);

   struct glx_screen *(*createScreen)(int screen, struct glx_display * priv);
};

struct __GLXDRIscreenRec {

   void (*destroyScreen)(struct glx_screen *psc);

   struct glx_context *(*createContext)(struct glx_screen *psc,
					struct glx_config *config,
					struct glx_context *shareList,
					int renderType);

   __GLXDRIdrawable *(*createDrawable)(struct glx_screen *psc,
				       XID drawable,
				       GLXDrawable glxDrawable,
				       struct glx_config *config);

   int64_t (*swapBuffers)(__GLXDRIdrawable *pdraw, int64_t target_msc,
			  int64_t divisor, int64_t remainder, Bool flush);
   void (*copySubBuffer)(__GLXDRIdrawable *pdraw,
			 int x, int y, int width, int height, Bool flush);
   int (*getDrawableMSC)(struct glx_screen *psc, __GLXDRIdrawable *pdraw,
			 int64_t *ust, int64_t *msc, int64_t *sbc);
   int (*waitForMSC)(__GLXDRIdrawable *pdraw, int64_t target_msc,
		     int64_t divisor, int64_t remainder, int64_t *ust,
		     int64_t *msc, int64_t *sbc);
   int (*waitForSBC)(__GLXDRIdrawable *pdraw, int64_t target_sbc, int64_t *ust,
		     int64_t *msc, int64_t *sbc);
   int (*setSwapInterval)(__GLXDRIdrawable *pdraw, int interval);
   int (*getSwapInterval)(__GLXDRIdrawable *pdraw);
   int (*getBufferAge)(__GLXDRIdrawable *pdraw);
};

struct __GLXDRIdrawableRec
{
   void (*destroyDrawable) (__GLXDRIdrawable * drawable);

   XID xDrawable;
   XID drawable;
   struct glx_screen *psc;
   GLenum textureTarget;
   GLenum textureFormat;        /* EXT_texture_from_pixmap support */
   unsigned long eventMask;
   int refcount;
};

/*
** Function to create and DRI display data and initialize the display
** dependent methods.
*/
extern __GLXDRIdisplay *driswCreateDisplay(Display * dpy);
extern __GLXDRIdisplay *driCreateDisplay(Display * dpy);
extern __GLXDRIdisplay *dri2CreateDisplay(Display * dpy);
extern __GLXDRIdisplay *dri3_create_display(Display * dpy);
extern __GLXDRIdisplay *driwindowsCreateDisplay(Display * dpy);

/*
**
*/
extern void dri2InvalidateBuffers(Display *dpy, XID drawable);
extern unsigned dri2GetSwapEventType(Display *dpy, XID drawable);

/*
** Functions to obtain driver configuration information from a direct
** rendering client application
*/
extern const char *glXGetScreenDriver(Display * dpy, int scrNum);

extern const char *glXGetDriverConfig(const char *driverName);

#endif

/************************************************************************/

#define __GL_CLIENT_ATTRIB_STACK_DEPTH 16

typedef struct __GLXpixelStoreModeRec
{
   GLboolean swapEndian;
   GLboolean lsbFirst;
   GLuint rowLength;
   GLuint imageHeight;
   GLuint imageDepth;
   GLuint skipRows;
   GLuint skipPixels;
   GLuint skipImages;
   GLuint alignment;
} __GLXpixelStoreMode;


typedef struct __GLXattributeRec
{
   GLuint mask;

    /**
     * Pixel storage state.  Most of the pixel store mode state is kept
     * here and used by the client code to manage the packing and
     * unpacking of data sent to/received from the server.
     */
   __GLXpixelStoreMode storePack, storeUnpack;

    /**
     * Is EXT_vertex_array / GL 1.1 DrawArrays protocol specifically
     * disabled?
     */
   GLboolean NoDrawArraysProtocol;

    /**
     * Vertex Array storage state.  The vertex array component
     * state is stored here and is used to manage the packing of
     * DrawArrays data sent to the server.
     */
   struct array_state_vector *array_state;
} __GLXattribute;

typedef struct __GLXattributeMachineRec
{
   __GLXattribute *stack[__GL_CLIENT_ATTRIB_STACK_DEPTH];
   __GLXattribute **stackPointer;
} __GLXattributeMachine;

struct mesa_glinterop_device_info;
struct mesa_glinterop_export_in;
struct mesa_glinterop_export_out;

struct glx_context_vtable {
   void (*destroy)(struct glx_context *ctx);
   int (*bind)(struct glx_context *context, struct glx_context *old,
	       GLXDrawable draw, GLXDrawable read);
   void (*unbind)(struct glx_context *context, struct glx_context *new_ctx);
   void (*wait_gl)(struct glx_context *ctx);
   void (*wait_x)(struct glx_context *ctx);
   void (*use_x_font)(struct glx_context *ctx,
		      Font font, int first, int count, int listBase);
   void (*bind_tex_image)(Display * dpy,
			  GLXDrawable drawable,
			  int buffer, const int *attrib_list);
   void (*release_tex_image)(Display * dpy, GLXDrawable drawable, int buffer);
   void * (*get_proc_address)(const char *symbol);
   int (*interop_query_device_info)(struct glx_context *ctx,
                                    struct mesa_glinterop_device_info *out);
   int (*interop_export_object)(struct glx_context *ctx,
                                struct mesa_glinterop_export_in *in,
                                struct mesa_glinterop_export_out *out);
};

/**
 * GLX state that needs to be kept on the client.  One of these records
 * exist for each context that has been made current by this client.
 */
struct glx_context
{
    /**
     * \name Drawing command buffer.
     *
     * Drawing commands are packed into this buffer before being sent as a
     * single GLX protocol request.  The buffer is sent when it overflows or
     * is flushed by \c __glXFlushRenderBuffer.  \c pc is the next location
     * in the buffer to be filled.  \c limit is described above in the buffer
     * slop discussion.
     *
     * Commands that require large amounts of data to be transfered will
     * also use this buffer to hold a header that describes the large
     * command.
     *
     * These must be the first 6 fields since they are static initialized
     * in the dummy context in glxext.c
     */
   /*@{ */
   GLubyte *buf;
   GLubyte *pc;
   GLubyte *limit;
   GLubyte *bufEnd;
   GLint bufSize;
   /*@} */

   const struct glx_context_vtable *vtable;

    /**
     * The XID of this rendering context.  When the context is created a
     * new XID is allocated.  This is set to None when the context is
     * destroyed but is still current to some thread. In this case the
     * context will be freed on next MakeCurrent.
     */
   XID xid;

    /**
     * The XID of the \c shareList context.
     */
   XID share_xid;

    /**
     * Screen number.
     */
   GLint screen;
   struct glx_screen *psc;

    /**
     * \c GL_TRUE if the context was created with ImportContext, which
     * means the server-side context was created by another X client.
     */
   GLboolean imported;

    /**
     * The context tag returned by MakeCurrent when this context is made
     * current. This tag is used to identify the context that a thread has
     * current so that proper server context management can be done.  It is
     * used for all context specific commands (i.e., \c Render, \c RenderLarge,
     * \c WaitX, \c WaitGL, \c UseXFont, and \c MakeCurrent (for the old
     * context)).
     */
   GLXContextTag currentContextTag;

    /**
     * \name Rendering mode
     *
     * The rendering mode is kept on the client as well as the server.
     * When \c glRenderMode is called, the buffer associated with the
     * previous rendering mode (feedback or select) is filled.
     */
   /*@{ */
   GLenum renderMode;
   GLfloat *feedbackBuf;
   GLuint *selectBuf;
   /*@} */

    /**
     * Fill newImage with the unpacked form of \c oldImage getting it
     * ready for transport to the server.
     */
   void (*fillImage) (struct glx_context *, GLint, GLint, GLint, GLint, GLenum,
                      GLenum, const GLvoid *, GLubyte *, GLubyte *);

    /**
     * Client side attribs.
     */
   __GLXattributeMachine attributes;

    /**
     * Client side error code.  This is set when client side gl API
     * routines need to set an error because of a bad enumerant or
     * running out of memory, etc.
     */
   GLenum error;

    /**
     * Whether this context does direct rendering.
     */
   Bool isDirect;

#if defined(GLX_DIRECT_RENDERING) && defined(GLX_USE_APPLEGL)
   void *driContext;
#endif

    /**
     * \c dpy of current display for this context.  Will be \c NULL if not
     * current to any display, or if this is the "dummy context".
     */
   Display *currentDpy;

    /**
     * The current drawable for this context.  Will be None if this
     * context is not current to any drawable.  currentReadable is below.
     */
   GLXDrawable currentDrawable;

    /**
     * \name GL Constant Strings
     *
     * Constant strings that describe the server implementation
     * These pertain to GL attributes, not to be confused with
     * GLX versioning attributes.
     */
   /*@{ */
   GLubyte *vendor;
   GLubyte *renderer;
   GLubyte *version;
   GLubyte *extensions;
   /*@} */

    /**
     * Maximum small render command size.  This is the smaller of 64k and
     * the size of the above buffer.
     */
   GLint maxSmallRenderCommandSize;

    /**
     * Major opcode for the extension.  Copied here so a lookup isn't
     * needed.
     */
   GLint majorOpcode;

    /**
     * Pointer to the config used to create this context.
     */
   struct glx_config *config;

    /**
     * The current read-drawable for this context.  Will be None if this
     * context is not current to any drawable.
     *
     * \since Internal API version 20030606.
     */
   GLXDrawable currentReadable;

   /**
    * Pointer to client-state data that is private to libGL.  This is only
    * used for indirect rendering contexts.
    *
    * No internal API version change was made for this change.  Client-side
    * drivers should NEVER use this data or even care that it exists.
    */
   void *client_state_private;

   /**
    * Stored value for \c glXQueryContext attribute \c GLX_RENDER_TYPE.
    */
   int renderType;

   /**
    * \name Raw server GL version
    *
    * True core GL version supported by the server.  This is the raw value
    * returned by the server, and it may not reflect what is actually
    * supported (or reported) by the client-side library.
    */
   /*@{ */
   int server_major;        /**< Major version number. */
   int server_minor;        /**< Minor version number. */
   /*@} */

   /**
    * Number of threads we're currently current in.
    */
   unsigned long thread_refcount;

   char gl_extension_bits[__GL_EXT_BYTES];
};

extern Bool
glx_context_init(struct glx_context *gc,
		 struct glx_screen *psc, struct glx_config *fbconfig);

#define __glXSetError(gc,code)  \
   if (!(gc)->error) {          \
      (gc)->error = code;       \
   }

extern void __glFreeAttributeState(struct glx_context *);

/************************************************************************/

/**
 * The size of the largest drawing command known to the implementation
 * that will use the GLXRender GLX command.  In this case it is
 * \c glPolygonStipple.
 */
#define __GLX_MAX_SMALL_RENDER_CMD_SIZE 156

/**
 * To keep the implementation fast, the code uses a "limit" pointer
 * to determine when the drawing command buffer is too full to hold
 * another fixed size command.  This constant defines the amount of
 * space that must always be available in the drawing command buffer
 * at all times for the implementation to work.  It is important that
 * the number be just large enough, but not so large as to reduce the
 * efficacy of the buffer.  The "+32" is just to keep the code working
 * in case somebody counts wrong.
 */
#define __GLX_BUFFER_LIMIT_SIZE (__GLX_MAX_SMALL_RENDER_CMD_SIZE + 32)

/**
 * This implementation uses a smaller threshold for switching
 * to the RenderLarge protocol than the protcol requires so that
 * large copies don't occur.
 */
#define __GLX_RENDER_CMD_SIZE_LIMIT 4096

/**
 * One of these records exists per screen of the display.  It contains
 * a pointer to the config data for that screen (if the screen supports GL).
 */
struct glx_screen_vtable {
   struct glx_context *(*create_context)(struct glx_screen *psc,
					 struct glx_config *config,
					 struct glx_context *shareList,
					 int renderType);

   struct glx_context *(*create_context_attribs)(struct glx_screen *psc,
						 struct glx_config *config,
						 struct glx_context *shareList,
						 unsigned num_attrib,
						 const uint32_t *attribs,
						 unsigned *error);
   int (*query_renderer_integer)(struct glx_screen *psc,
                                 int attribute,
                                 unsigned int *value);
   int (*query_renderer_string)(struct glx_screen *psc,
                                int attribute,
                                const char **value);
};

struct glx_screen
{
   const struct glx_screen_vtable *vtable;

    /**
     * GLX extension string reported by the X-server.
     */
   const char *serverGLXexts;

    /**
     * GLX extension string to be reported to applications.  This is the
     * set of extensions that the application can actually use.
     */
   char *effectiveGLXexts;

   struct glx_display *display;

   Display *dpy;
   int scr;

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
    /**
     * Per screen direct rendering interface functions and data.
     */
   __GLXDRIscreen *driScreen;
#endif

    /**
     * Linked list of glx visuals and  fbconfigs for this screen.
     */
   struct glx_config *visuals, *configs;

    /**
     * Per-screen dynamic GLX extension tracking.  The \c direct_support
     * field only contains enough bits for 64 extensions.  Should libGL
     * ever need to track more than 64 GLX extensions, we can safely grow
     * this field.  The \c struct glx_screen structure is not used outside
     * libGL.
     */
   /*@{ */
   unsigned char direct_support[8];
   GLboolean ext_list_first_time;
   /*@} */

};

/**
 * Per display private data.  One of these records exists for each display
 * that is using the OpenGL (GLX) extension.
 */
struct glx_display
{
   /* The extension protocol codes */
   XExtCodes *codes;
   struct glx_display *next;

    /**
     * Back pointer to the display
     */
   Display *dpy;

    /**
     * The \c majorOpcode is common to all connections to the same server.
     * It is also copied into the context structure.
     */
   int majorOpcode;

    /**
     * \name Server Version
     *
     * Major and minor version returned by the server during initialization.
     */
   /*@{ */
   int majorVersion, minorVersion;
   /*@} */

    /**
     * \name Storage for the servers GLX vendor and versions strings.
     *
     * These are the same for all screens on this display. These fields will
     * be filled in on demand.
     */
   /*@{ */
   const char *serverGLXvendor;
   const char *serverGLXversion;
   /*@} */

    /**
     * Configurations of visuals for all screens on this display.
     * Also, per screen data which now includes the server \c GLX_EXTENSION
     * string.
     */
   struct glx_screen **screens;

   __glxHashTable *glXDrawHash;

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
   __glxHashTable *drawHash;

    /**
     * Per display direct rendering interface functions and data.
     */
   __GLXDRIdisplay *driswDisplay;
   __GLXDRIdisplay *driDisplay;
   __GLXDRIdisplay *dri2Display;
   __GLXDRIdisplay *dri3Display;
#endif
#ifdef GLX_USE_WINDOWSGL
   __GLXDRIdisplay *windowsdriDisplay;
#endif
};

struct glx_drawable {
   XID xDrawable;
   XID drawable;

   uint32_t lastEventSbc;
   int64_t eventSbcWrap;
};

extern int
glx_screen_init(struct glx_screen *psc,
		int screen, struct glx_display * priv);
extern void
glx_screen_cleanup(struct glx_screen *psc);

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
extern __GLXDRIdrawable *
dri2GetGlxDrawableFromXDrawableId(Display *dpy, XID id);
#endif

extern GLubyte *__glXFlushRenderBuffer(struct glx_context *, GLubyte *);

extern void __glXSendLargeChunk(struct glx_context * gc, GLint requestNumber,
                                GLint totalRequests,
                                const GLvoid * data, GLint dataLen);

extern void __glXSendLargeCommand(struct glx_context *, const GLvoid *, GLint,
                                  const GLvoid *, GLint);

/* Initialize the GLX extension for dpy */
extern struct glx_display *__glXInitialize(Display *);

extern void __glXPreferEGL(int state);

/************************************************************************/

extern int __glXDebug;

/* This is per-thread storage in an MT environment */

extern void __glXSetCurrentContext(struct glx_context * c);

# if defined( GLX_USE_TLS )

extern __thread void *__glX_tls_Context
   __attribute__ ((tls_model("initial-exec")));

#  define __glXGetCurrentContext() __glX_tls_Context

# else

extern struct glx_context *__glXGetCurrentContext(void);

# endif /* defined( GLX_USE_TLS ) */

extern void __glXSetCurrentContextNull(void);


/*
** Global lock for all threads in this address space using the GLX
** extension
*/
extern pthread_mutex_t __glXmutex;
#define __glXLock()    pthread_mutex_lock(&__glXmutex)
#define __glXUnlock()  pthread_mutex_unlock(&__glXmutex)

/*
** Setup for a command.  Initialize the extension for dpy if necessary.
*/
extern CARD8 __glXSetupForCommand(Display * dpy);

/************************************************************************/

/*
** Data conversion and packing support.
*/

extern const GLuint __glXDefaultPixelStore[9];

/* Send an image to the server using RenderLarge. */
extern void __glXSendLargeImage(struct glx_context * gc, GLint compsize, GLint dim,
                                GLint width, GLint height, GLint depth,
                                GLenum format, GLenum type,
                                const GLvoid * src, GLubyte * pc,
                                GLubyte * modes);

/* Return the size, in bytes, of some pixel data */
extern GLint __glImageSize(GLint, GLint, GLint, GLenum, GLenum, GLenum);

/* Return the number of elements per group of a specified format*/
extern GLint __glElementsPerGroup(GLenum format, GLenum type);

/* Return the number of bytes per element, based on the element type (other
** than GL_BITMAP).
*/
extern GLint __glBytesPerElement(GLenum type);

/*
** Fill the transport buffer with the data from the users buffer,
** applying some of the pixel store modes (unpack modes) to the data
** first.  As a side effect of this call, the "modes" field is
** updated to contain the modes needed by the server to decode the
** sent data.
*/
extern void __glFillImage(struct glx_context *, GLint, GLint, GLint, GLint, GLenum,
                          GLenum, const GLvoid *, GLubyte *, GLubyte *);

/* Copy map data with a stride into a packed buffer */
extern void __glFillMap1f(GLint, GLint, GLint, const GLfloat *, GLubyte *);
extern void __glFillMap1d(GLint, GLint, GLint, const GLdouble *, GLubyte *);
extern void __glFillMap2f(GLint, GLint, GLint, GLint, GLint,
                          const GLfloat *, GLfloat *);
extern void __glFillMap2d(GLint, GLint, GLint, GLint, GLint,
                          const GLdouble *, GLdouble *);

/*
** Empty an image out of the reply buffer into the clients memory applying
** the pack modes to pack back into the clients requested format.
*/
extern void __glEmptyImage(struct glx_context *, GLint, GLint, GLint, GLint, GLenum,
                           GLenum, const GLubyte *, GLvoid *);


/*
** Allocate and Initialize Vertex Array client state, and free.
*/
extern void __glXInitVertexArrayState(struct glx_context *);
extern void __glXFreeVertexArrayState(struct glx_context *);

/*
** Inform the Server of the major and minor numbers and of the client
** libraries extension string.
*/
extern void __glXClientInfo(Display * dpy, int opcode);

_X_HIDDEN void
__glX_send_client_info(struct glx_display *glx_dpy);

/************************************************************************/

/*
** Declarations that should be in Xlib
*/
#ifdef __GL_USE_OUR_PROTOTYPES
extern void _XFlush(Display *);
extern Status _XReply(Display *, xReply *, int, Bool);
extern void _XRead(Display *, void *, long);
extern void _XSend(Display *, const void *, long);
#endif


extern void __glXInitializeVisualConfigFromTags(struct glx_config * config,
                                                int count, const INT32 * bp,
                                                Bool tagged_only,
                                                Bool fbconfig_style_tags);

extern char *__glXQueryServerString(Display * dpy, int opcode,
                                    CARD32 screen, CARD32 name);
extern char *__glXGetString(Display * dpy, int opcode,
                            CARD32 screen, CARD32 name);

extern const char __glXGLClientVersion[];
extern const char __glXGLClientExtensions[];

/* Get the unadjusted system time */
extern int __glXGetUST(int64_t * ust);

extern GLboolean __glXGetMscRateOML(Display * dpy, GLXDrawable drawable,
                                    int32_t * numerator,
                                    int32_t * denominator);

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)
extern GLboolean
__glxGetMscRate(struct glx_screen *psc,
		int32_t * numerator, int32_t * denominator);

/* So that dri2.c:DRI2WireToEvent() can access
 * glx_info->codes->first_event */
XExtDisplayInfo *__glXFindDisplay (Display *dpy);

extern void
GarbageCollectDRIDrawables(struct glx_screen *psc);

extern __GLXDRIdrawable *
GetGLXDRIDrawable(Display *dpy, GLXDrawable drawable);
#endif

extern struct glx_screen *GetGLXScreenConfigs(Display * dpy, int scrn);

#ifdef GLX_USE_APPLEGL
extern struct glx_screen *
applegl_create_screen(int screen, struct glx_display * priv);

extern struct glx_context *
applegl_create_context(struct glx_screen *psc,
			struct glx_config *mode,
			struct glx_context *shareList, int renderType);

extern int
applegl_create_display(struct glx_display *display);
#endif

extern Bool validate_renderType_against_config(const struct glx_config *config,
                                               int renderType);


extern struct glx_drawable *GetGLXDrawable(Display *dpy, GLXDrawable drawable);
extern int InitGLXDrawable(Display *dpy, struct glx_drawable *glxDraw,
			   XID xDrawable, GLXDrawable drawable);
extern void DestroyGLXDrawable(Display *dpy, GLXDrawable drawable);

extern struct glx_context dummyContext;

extern struct glx_screen *
indirect_create_screen(int screen, struct glx_display * priv);
extern struct glx_context *
indirect_create_context(struct glx_screen *psc,
			struct glx_config *mode,
			struct glx_context *shareList, int renderType);
extern struct glx_context *
indirect_create_context_attribs(struct glx_screen *base,
                                struct glx_config *config_base,
                                struct glx_context *shareList,
                                unsigned num_attribs,
                                const uint32_t *attribs,
                                unsigned *error);

#ifdef __cplusplus
}
#endif

#endif /* !__GLX_client_h__ */
