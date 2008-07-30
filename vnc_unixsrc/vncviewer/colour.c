/*
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

/*
 * colour.c - functions to deal with colour - i.e. RFB pixel formats, X visuals
 * and colormaps.  Thanks to Grant McDorman for some of the ideas used here.
 */

#include "vncviewer.h"
#include <limits.h>


#define INVALID_PIXEL 0xffffffff
#define MAX_CMAP_SIZE 256
#define BGR233_SIZE 256
unsigned long BGR233ToPixel[BGR233_SIZE];

Colormap cmap;
Visual *vis;
unsigned int visdepth, visbpp;
Bool allocColorFailed = False;

static int nBGR233ColoursAllocated;

static Bool GetPseudoColorVisualAndCmap(int depth);
static Bool GetTrueColorVisualAndCmap(int depth);
static int GetBPPForDepth(int depth);
static void SetupBGR233Map();
static void AllocateExactBGR233Colours();
static Bool AllocateBGR233Colour(int r, int g, int b);


/*
 * SetVisualAndCmap() deals with the wonderful world of X "visuals" (which are
 * equivalent to the RFB protocol's "pixel format").  Having decided on the
 * best visual, it also creates a colormap if necessary, sets the appropriate
 * resources on the toplevel widget, and sets up the myFormat structure to
 * describe the pixel format in terms that the RFB server will be able to
 * understand.
 *
 * The algorithm for deciding which visual to use is as follows:
 *
 * If forceOwnCmap is true then we try to use a PseudoColor visual - we first
 * see if there's one of the same depth as the RFB server, followed by an 8-bit
 * deep one.
 *
 * If forceTrueColour is true then we try to use a TrueColor visual - if
 * requestedDepth is set then it must be of that depth, otherwise any depth
 * will be used.
 *
 * Otherwise, we use the X server's default visual and colormap.  If this is
 * TrueColor then we just ask the RFB server for this format.  If the default
 * isn't TrueColor, or if useBGR233 is true, then we ask the RFB server for
 * BGR233 pixel format and use a lookup table to translate to the nearest
 * colours provided by the X server.
 */

void
SetVisualAndCmap()
{
  if (appData.forceOwnCmap) {
    if (!si.format.trueColour) {
      if (GetPseudoColorVisualAndCmap(si.format.depth))
	return;
    }
    if (GetPseudoColorVisualAndCmap(8))
      return;
    fprintf(stderr,"Couldn't find a matching PseudoColor visual.\n");
  }

  if (appData.forceTrueColour) {
    if (GetTrueColorVisualAndCmap(appData.requestedDepth))
      return;
    fprintf(stderr,"Couldn't find a matching TrueColor visual.\n");
  }

  /* just use default visual and colormap */

  vis = DefaultVisual(dpy,DefaultScreen(dpy));
  visdepth = DefaultDepth(dpy,DefaultScreen(dpy));
  visbpp = GetBPPForDepth(visdepth);
  cmap = DefaultColormap(dpy,DefaultScreen(dpy));

  if (!appData.useBGR233 && (vis->class == TrueColor)) {

    myFormat.bitsPerPixel = visbpp;
    myFormat.depth = visdepth;
    myFormat.trueColour = 1;
    myFormat.bigEndian = (ImageByteOrder(dpy) == MSBFirst);
    myFormat.redShift = ffs(vis->red_mask) - 1;
    myFormat.greenShift = ffs(vis->green_mask) - 1;
    myFormat.blueShift = ffs(vis->blue_mask) - 1;
    myFormat.redMax = vis->red_mask >> myFormat.redShift;
    myFormat.greenMax = vis->green_mask >> myFormat.greenShift;
    myFormat.blueMax = vis->blue_mask >> myFormat.blueShift;

    fprintf(stderr,
	    "Using default colormap which is TrueColor.  Pixel format:\n");
    PrintPixelFormat(&myFormat);
    return;
  }

  appData.useBGR233 = True;

  myFormat.bitsPerPixel = 8;
  myFormat.depth = 8;
  myFormat.trueColour = 1;
  myFormat.bigEndian = 0;
  myFormat.redMax = 7;
  myFormat.greenMax = 7;
  myFormat.blueMax = 3;
  myFormat.redShift = 0;
  myFormat.greenShift = 3;
  myFormat.blueShift = 6;

  fprintf(stderr,
       "Using default colormap and translating from BGR233.  Pixel format:\n");
  PrintPixelFormat(&myFormat);

  SetupBGR233Map();
}


/*
 * GetPseudoColorVisualAndCmap tries to find a PseudoColor visual of the given
 * depth.  If successful it sets vis, visdepth, cmap and myFormat, and also
 * sets the appropriate resources on the toplevel widget.
 */

static Bool
GetPseudoColorVisualAndCmap(int depth)
{
  XVisualInfo tmpl;
  XVisualInfo *vinfo;
  int nvis;

  tmpl.screen = DefaultScreen(dpy);
  tmpl.depth = depth;
  tmpl.class = PseudoColor;
  tmpl.colormap_size = (1 << depth);

  vinfo = XGetVisualInfo(dpy,
			 VisualScreenMask|VisualDepthMask|
			 VisualClassMask|VisualColormapSizeMask,
			 &tmpl, &nvis);

  if (vinfo) {
    vis = vinfo[0].visual;
    visdepth = vinfo[0].depth;
    XFree(vinfo);
    visbpp = GetBPPForDepth(visdepth);
    myFormat.bitsPerPixel = visbpp;
    myFormat.depth = visdepth;
    myFormat.trueColour = 0;
    myFormat.bigEndian = (ImageByteOrder(dpy) == MSBFirst);
    myFormat.redMax = myFormat.greenMax = myFormat.blueMax = 0;
    myFormat.redShift = myFormat.greenShift = myFormat.blueShift = 0;

    cmap = XCreateColormap(dpy, DefaultRootWindow(dpy), vis, AllocAll);

    XtVaSetValues(toplevel, XtNcolormap, cmap, XtNdepth, visdepth,
		  XtNvisual, vis, NULL);

    if (appData.fullScreen) {
      XInstallColormap(dpy, cmap);
    }

    fprintf(stderr,"Using PseudoColor visual, depth %d.  Pixel format:\n",
	    visdepth);
    PrintPixelFormat(&myFormat);

    return True;
  }

  return False;
}


/*
 * GetTrueColorVisualAndCmap tries to find a TrueColor visual of the given
 * depth.  If successful it sets vis, visdepth, cmap and myFormat, and also
 * sets the appropriate resources on the toplevel widget.
 */

static Bool
GetTrueColorVisualAndCmap(int depth)
{
  XVisualInfo tmpl;
  XVisualInfo *vinfo;
  int nvis;
  int mask = VisualScreenMask|VisualClassMask;

  tmpl.screen = DefaultScreen(dpy);
  tmpl.class = TrueColor;

  if (depth != 0) {
    tmpl.depth = depth;
    mask |= VisualDepthMask;
  }

  vinfo = XGetVisualInfo(dpy, mask, &tmpl, &nvis);

  if (vinfo) {
    vis = vinfo[0].visual;
    visdepth = vinfo[0].depth;
    XFree(vinfo);
    visbpp = GetBPPForDepth(visdepth);
    myFormat.bitsPerPixel = visbpp;
    myFormat.depth = visdepth;
    myFormat.trueColour = 1;
    myFormat.bigEndian = (ImageByteOrder(dpy) == MSBFirst);
    myFormat.redShift = ffs(vis->red_mask) - 1;
    myFormat.greenShift = ffs(vis->green_mask) - 1;
    myFormat.blueShift = ffs(vis->blue_mask) - 1;
    myFormat.redMax = vis->red_mask >> myFormat.redShift;
    myFormat.greenMax = vis->green_mask >> myFormat.greenShift;
    myFormat.blueMax = vis->blue_mask >> myFormat.blueShift;

    cmap = XCreateColormap(dpy, DefaultRootWindow(dpy), vis, AllocNone);

    XtVaSetValues(toplevel, XtNcolormap, cmap, XtNdepth, visdepth,
		  XtNvisual, vis, NULL);

    if (appData.fullScreen) {
      XInstallColormap(dpy, cmap);
    }

    fprintf(stderr,"Using TrueColor visual, depth %d.  Pixel format:\n",
	    visdepth);
    PrintPixelFormat(&myFormat);

    return True;
  }

  return False;
}


/*
 * GetBPPForDepth looks through the "pixmap formats" to find the bits-per-pixel
 * for the given depth.
 */

static int
GetBPPForDepth(int depth)
{
  XPixmapFormatValues *format;
  int nformats;
  int i;
  int bpp;

  format = XListPixmapFormats(dpy, &nformats);

  for (i = 0; i < nformats; i++) {
    if (format[i].depth == depth)
      break;
  }

  if (i == nformats) {
    fprintf(stderr,"no pixmap format for depth %d???\n", depth);
    exit(1);
  }

  bpp = format[i].bits_per_pixel;

  XFree(format);

  if (bpp != 1 && bpp != 8 && bpp != 16 && bpp != 32) {
    fprintf(stderr,"Can't cope with %d bits-per-pixel.  Sorry.\n", bpp);
    exit(1);
  }

  return bpp;
}



/*
 * SetupBGR233Map() sets up the BGR233ToPixel array.
 *
 * It calls AllocateExactBGR233Colours to allocate some exact BGR233 colours
 * (limited by space in the colormap and/or by the value of the nColours
 * resource).  If the number allocated is less than BGR233_SIZE then it fills
 * the rest in using the "nearest" colours available.  How this is done depends
 * on the value of the useSharedColours resource.  If it's false, we use only
 * colours from the exact BGR233 colours we've just allocated.  If it's true,
 * then we also use other clients' "shared" colours available in the colormap.
 */

static void
SetupBGR233Map()
{
  int r, g, b;
  long i;
  unsigned long nearestPixel = 0;
  int cmapSize;
  XColor cmapEntry[MAX_CMAP_SIZE];
  Bool exactBGR233[MAX_CMAP_SIZE];
  Bool shared[MAX_CMAP_SIZE];
  Bool usedAsNearest[MAX_CMAP_SIZE];
  int nSharedUsed = 0;

  if (visdepth > 8) {
    appData.nColours = 256; /* ignore nColours setting for > 8-bit deep */
  }

  for (i = 0; i < BGR233_SIZE; i++) {
    BGR233ToPixel[i] = INVALID_PIXEL;
  }

  AllocateExactBGR233Colours();

  fprintf(stderr,"Got %d exact BGR233 colours out of %d\n",
	  nBGR233ColoursAllocated, appData.nColours);

  if (nBGR233ColoursAllocated < BGR233_SIZE) {

    if (visdepth > 8) { /* shouldn't get here */
      fprintf(stderr,"Error: couldn't allocate BGR233 colours even though "
	      "depth is %d\n", visdepth);
      exit(1);
    }

    cmapSize = (1 << visdepth);

    for (i = 0; i < cmapSize; i++) {
      cmapEntry[i].pixel = i;
      exactBGR233[i] = False;
      shared[i] = False;
      usedAsNearest[i] = False;
    }

    XQueryColors(dpy, cmap, cmapEntry, cmapSize);

    /* mark all our exact BGR233 pixels */

    for (i = 0; i < BGR233_SIZE; i++) {
      if (BGR233ToPixel[i] != INVALID_PIXEL)
	exactBGR233[BGR233ToPixel[i]] = True;
    }

    if (appData.useSharedColours) {

      /* Try to find existing shared colours.  This is harder than it sounds
	 because XQueryColors doesn't tell us whether colours are shared,
	 private or unallocated.  What we do is go through the colormap and for
	 each pixel try to allocate exactly its RGB values.  If this returns a
	 different pixel then it's definitely either a private or unallocated
	 pixel, so no use to us.  If it returns us the same pixel again, then
	 it's likely that it's a shared colour - however, it is possible that
	 it was actually an unallocated pixel, which we've now allocated.  We
	 minimise this possibility by going through the pixels in reverse order
	 - this helps becuse the X server allocates new pixels from the lowest
	 number up, so it should only be a problem for the lowest unallocated
	 pixel.  Got that? */

      for (i = cmapSize-1; i >= 0; i--) {
	if (!exactBGR233[i] &&
	    XAllocColor(dpy, cmap, &cmapEntry[i])) {

	  if (cmapEntry[i].pixel == i) {

	    shared[i] = True; /* probably shared */

	  } else {

	    /* "i" is either unallocated or private.  We have now unnecessarily
	       allocated cmapEntry[i].pixel.  Free it. */

	    XFreeColors(dpy, cmap, &cmapEntry[i].pixel, 1, 0);
	  }
	}
      }
    }

    /* Now fill in the nearest colours */

    for (r = 0; r < 8; r++) {
      for (g = 0; g < 8; g++) {
	for (b = 0; b < 4; b++) {
	  if (BGR233ToPixel[(b<<6) | (g<<3) | r] == INVALID_PIXEL) {

	    unsigned long minDistance = ULONG_MAX;

	    for (i = 0; i < cmapSize; i++) {
	      if (exactBGR233[i] || shared[i]) {
		unsigned long distance
		  = (abs(cmapEntry[i].red - r * 65535 / 7)
		     + abs(cmapEntry[i].green - g * 65535 / 7)
		     + abs(cmapEntry[i].blue - b * 65535 / 3));

		if (distance < minDistance) {
		  minDistance = distance;
		  nearestPixel = i;
		}
	      }
	    }

	    BGR233ToPixel[(b<<6) | (g<<3) | r] = nearestPixel;
	    if (shared[nearestPixel] && !usedAsNearest[nearestPixel])
	      nSharedUsed++;
	    usedAsNearest[nearestPixel] = True;
	  }
	}
      }
    }

    /* Tidy up shared colours which we allocated but aren't going to use */

    for (i = 0; i < cmapSize; i++) {
      if (shared[i] && !usedAsNearest[i]) {
	  XFreeColors(dpy, cmap, (unsigned long *)&i, 1, 0);
      }
    }

    fprintf(stderr,"Using %d existing shared colours\n", nSharedUsed);
  }
}


/*
 * AllocateExactBGR233Colours() attempts to allocate each of the colours in the
 * BGR233 colour cube, stopping when an allocation fails.  The order it does
 * this in is such that we should get a fairly well spread subset of the cube,
 * however many allocations are made.  There's probably a neater algorithm for
 * doing this, but it's not obvious to me anyway.  The way this algorithm works
 * is:
 *
 * At each stage, we introduce a new value for one of the primaries, and
 * allocate all the colours with the new value of that primary and all previous
 * values of the other two primaries.  We start with r=0 as the "new" value
 * for r, and g=0, b=0 as the "previous" values of g and b.  So we get:
 *
 * New primary value   Previous values of other primaries   Colours allocated
 * -----------------   ----------------------------------   -----------------
 * r=0                 g=0       b=0                        r0 g0 b0
 * g=7                 r=0       b=0                        r0 g7 b0
 * b=3                 r=0       g=0,7                      r0 g0 b3
 *                                                          r0 g7 b3
 * r=7                 g=0,7     b=0,3                      r7 g0 b0
 * 		       		 			    r7 g0 b3
 * 							    r7 g7 b0
 *							    r7 g7 b3
 * g=3                 r=0,7     b=0,3                      r0 g3 b0
 *                                                          r0 g3 b3
 *                                                          r7 g3 b0
 *                                                          r7 g3 b3
 * ....etc.
 * */

static void
AllocateExactBGR233Colours()
{
  int rv[] = {0,7,3,5,1,6,2,4};
  int gv[] = {0,7,3,5,1,6,2,4};
  int bv[] = {0,3,1,2};
  int rn = 0;
  int gn = 1;
  int bn = 1;
  int ri, gi, bi;

  nBGR233ColoursAllocated = 0;

  while (1) {
    if (rn == 8)
      break;

    ri = rn;
    for (gi = 0; gi < gn; gi++) {
      for (bi = 0; bi < bn; bi++) {
	if (!AllocateBGR233Colour(rv[ri], gv[gi], bv[bi]))
	  return;
      }
    }
    rn++;

    if (gn == 8)
      break;

    gi = gn;
    for (ri = 0; ri < rn; ri++) {
      for (bi = 0; bi < bn; bi++) {
	if (!AllocateBGR233Colour(rv[ri], gv[gi], bv[bi]))
	  return;
      }
    }
    gn++;

    if (bn < 4) {

      bi = bn;
      for (ri = 0; ri < rn; ri++) {
	for (gi = 0; gi < gn; gi++) {
	  if (!AllocateBGR233Colour(rv[ri], gv[gi], bv[bi]))
	    return;
	}
      }
      bn++;
    }
  }
}


/*
 * AllocateBGR233Colour() attempts to allocate the given BGR233 colour as a
 * shared colormap entry, storing its pixel value in the BGR233ToPixel array.
 * r is from 0 to 7, g from 0 to 7 and b from 0 to 3.  It fails either when the
 * allocation fails or when we would exceed the number of colours specified in
 * the nColours resource.
 */

static Bool
AllocateBGR233Colour(int r, int g, int b)
{
  XColor c;

  if (nBGR233ColoursAllocated >= appData.nColours)
    return False;

  c.red = r * 65535 / 7;
  c.green = g * 65535 / 7;
  c.blue = b * 65535 / 3;

  if (!XAllocColor(dpy, cmap, &c))
    return False;

  BGR233ToPixel[(b<<6) | (g<<3) | r] = c.pixel;

  nBGR233ColoursAllocated++;

  return True;
}
