/* Copyright (C)2004 Landmark Graphics Corporation
 * Copyright (C)2005, 2006 Sun Microsystems, Inc.
 * Copyright (C)2011 D. R. Commander
 *
 * This library is free software and may be redistributed and/or modified under
 * the terms of the wxWindows Library License, Version 3.1 or (at your option)
 * any later version.  The full license is in the LICENSE.txt file included
 * with this distribution.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * wxWindows Library License for more details.
 */

/*
  FBX -- the fast Frame Buffer eXchange library
  This library is designed to facilitate transferring pixels to/from the
  framebuffer using fast 2D O/S-native methods that do not rely on OpenGL
  acceleration
*/

#ifndef __FBX_H__
#define __FBX_H__

#define USESHM

#include <stdio.h>

#ifdef _WIN32

#include <windows.h>
typedef HDC fbx_gc;
typedef HWND fbx_wh;

#else

#include <X11/Xlib.h>

#ifdef USESHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

#include <X11/Xutil.h>

typedef GC fbx_gc;
typedef struct
{
  Display *dpy;  Drawable d;  Visual *v;
} fbx_wh;

#endif /* _WIN32 */


/* Pixel formats */
#define FBX_FORMATS 7
enum { FBX_RGB, FBX_RGBA, FBX_BGR, FBX_BGRA, FBX_ABGR, FBX_ARGB, FBX_INDEX };

static const int fbx_ps[FBX_FORMATS]=
	{ 3, 4, 3, 4, 4, 4, 1 };
static const int fbx_bgr[FBX_FORMATS]=
	{ 0, 0, 1, 1, 1, 0, 0 };
static const int fbx_alphafirst[FBX_FORMATS]=
	{ 0, 0, 0, 0, 1, 1, 0 };
static const int fbx_roffset[FBX_FORMATS]=
	{ 0, 0, 2, 2, 3, 1, 0 };
static const int fbx_goffset[FBX_FORMATS]=
	{ 1, 1, 1, 1, 2, 2, 0 };
static const int fbx_boffset[FBX_FORMATS]=
	{ 2, 2, 0, 0, 1, 3, 0 };


typedef struct _fbx_struct
{
	int width, height, pitch;
	char *bits;
	int format;
	fbx_wh wh;
	int shm;

	#ifdef _WIN32
	HDC hmdc;  HBITMAP hdib;
	#else
	#ifdef USESHM
	XShmSegmentInfo shminfo;  int xattach;
	#endif
	GC xgc;
	XImage *xi;
	Pixmap pm;
	int pixmap;
	#endif
} fbx_struct;


#ifdef __cplusplus
extern "C" {
#endif

/*
  All of these methods return -1 on failure or 0 on success.
*/


/*
  fbx_init
  (fbx_struct *fb, fbx_wh wh, int width, int height, int useShm)

  fb = Address of fbx_struct (must be pre-allocated by user)
  wh = Handle to the window that you wish to read from or write to.  On
       Windows, this is the same as the HWND.  On Unix, this is a struct
       (see above) that describes the X11 display and drawable you wish to use.
       If wh.v is non-NULL, then FBX assumes that the drawable is a Pixmap.
  width = Width of buffer (in pixels) that you wish to create.  0 = use width
          of window
  height = Height of buffer (in pixels) that you wish to create.  0 = use
           height of window
  useShm = Use MIT-SHM extension, if available (Unix only.)

  NOTES:
  -- fbx_init() is idempotent.  If you call it multiple times, it will
     re-initialize the buffer only when it is necessary to do so (such as when
     the window size has changed.)
  -- On Windows, fbx_init() will return a buffer configured with the same pixel
     format as the screen, unless the screen depth is < 24 bits, in which case
     it will always return a 32-bit BGRA buffer.

  On return, fbx_init() fills in the following relevant information in the
  fbx_struct that you passed to it:

  fb->format = pixel format of the buffer (one of FBX_RGB, FBX_RGBA, FBX_BGR,
               FBX_BGRA, FBX_ABGR, or FBX_ARGB)
  fb->width, fb->height = dimensions of the buffer
  fb->pitch = bytes in each scanline of the buffer
  fb->bits = address of the start of the buffer
*/
int fbx_init(fbx_struct *fb, fbx_wh wh, int width, int height, int useShm);


/*
  fbx_read
  (fbx_struct *fb, int x, int y)

  This routine copies pixels from the framebuffer into the memory buffer
  specified by fb.

  fb = Address of fbx_struct previously initialized by a call to fbx_init()
  x = Horizontal offset (from left of drawable) of rectangle to read
  y = Vertical offset (from top of drawable) of rectangle to read

  NOTE: width and height of rectangle are not adjustable without re-calling
  fbx_init()

  On return, fb->bits contains a facsimile of the window's pixels
*/
int fbx_read(fbx_struct *fb, int x, int y);


/*
  fbx_write
  (fbx_struct *fb, int srcX, int srcY, int dstX, int dstY, int width,
   int height)

  This routine copies pixels from the memory buffer specified by fb to the
  framebuffer.

  fb = Address of fbx_struct previously initialized by a call to fbx_init()
       fb->bits should contain the pixels you wish to blit
  srcX = left offset of the region you wish to blit (relative to the memory
         buffer)
  srcY = top offset of the region you wish to blit (relative to the memory
         buffer)
  dstX = left offset of where you want the pixels to end up (relative to
         drawable area)
  dstY = top offset of where you want the pixels to end up (relative to
         drawable area)
  width = width of region you wish to blit (0 = whole bitmap)
  height = height of region you wish to blit (0 = whole bitmap)
*/
int fbx_write (fbx_struct *fb, int srcX, int srcY, int dstX, int dstY,
	int width, int height);


/*
  fbx_awrite
  (fbx_struct *fb, int srcX, int srcY, int dstX, int dstY, int width,
   int height)

  Same as fbx_write, but asynchronous.  The write isn't guaranteed to complete
  until fbx_sync() is called.  On Windows, fbx_awrite is the same as fbx_write.
*/
#ifdef _WIN32
#define fbx_awrite fbx_write
#else
int fbx_awrite (fbx_struct *fb, int srcX, int srcY, int dstX, int dstY,
	int width, int height);
#endif


/*
  fbx_flip
  (fbx_struct *fb, int srcX, int srcY, int width, int height)

  This routine performs an in-place vertical flip of the region of interest
  specified by srcX, srcY, width, and height in the memory buffer specified by
  fb.

  fb = Address of fbx_struct previously initialized by a call to fbx_init()
       fb->bits should contain the pixels you wish to flip
  srcX = left offset of the region you wish to flip (relative to the memory
         buffer)
  srcY = top offset of the region you wish to flip (relative to the memory
         buffer)
  width = width of region you wish to flip (0 = whole bitmap)
  height = height of region you wish to flip (0 = whole bitmap)
*/
int fbx_flip(fbx_struct *fb, int srcX, int srcY, int width, int height);


/*
  fbx_sync
  (fbx_struct *fb)

  Complete a previous asynchronous write.  On Windows, this does nothing.
*/
int fbx_sync (fbx_struct *fb);


/*
  fbx_term
  (fbx_struct *fb)

  Free the memory buffers pointed to by structure fb.

  NOTE: this routine is idempotent.  It only frees stuff that needs freeing.
*/
int fbx_term(fbx_struct *fb);


/*
  fbx_geterrmsg

  This returns a string containing the reason why the last command failed.
*/
char *fbx_geterrmsg(void);


/*
  fbx_geterrline

  This returns the line (within fbx.c) of the last failure.
*/
int fbx_geterrline(void);


/*
  fbx_formatname

  Returns a character string describing the pixel format specified in the
  format parameter.
*/
const char *fbx_formatname(int format);


/*
  fbx_printwarnings
  (FILE *output_stream)

  By default, FBX will not print warning messages (such as messages related to
  its automatic selection of a particular drawing method.)  These messages are
  sometimes useful when diagnosing performance issues.  Passing a stream
  pointer (such as stdout, stderr, or a pointer returned from a previous call
  to fopen()) to this function will enable warning messages and will cause them
  to be printed to the specified stream.  Passing an argument of NULL to this
  function will disable warnings.
*/
void fbx_printwarnings(FILE *output_stream);


#ifdef __cplusplus
}
#endif

#endif /* __FBX_H__ */
