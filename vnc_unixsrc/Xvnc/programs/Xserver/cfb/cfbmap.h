/*
 * $XConsortium: cfbmap.h,v 1.11 95/06/08 23:20:39 gildea Exp $
 * $XFree86: xc/programs/Xserver/cfb/cfbmap.h,v 3.3 1996/06/29 09:05:40 dawes Exp $
 *
Copyright (c) 1991  X Consortium

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
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * Map names around so that multiple depths can be supported simultaneously
 */

/* a losing vendor cpp dumps core if we define NAME in terms of CATNAME */

#if PSZ != 8
#if PSZ == 32
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define NAME(subname) cfb32##subname
#else
#define NAME(subname) cfb32/**/subname
#endif
#endif

#if PSZ == 24
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define NAME(subname) cfb24##subname
#else
#define NAME(subname) cfb24/**/subname
#endif
#endif

#if PSZ == 16
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define NAME(subname) cfb16##subname
#else
#define NAME(subname) cfb16/**/subname
#endif
#endif

#if PSZ == 4
#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define NAME(subname) cfb4##subname
#else
#define NAME(subname) cfb4/**/subname
#endif
#endif

#ifndef NAME
cfb can not hack PSZ yet
#endif

#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define CATNAME(prefix,subname) prefix##subname
#else
#define CATNAME(prefix,subname) prefix/**/subname
#endif

#define cfbScreenPrivateIndex NAME(ScreenPrivateIndex)
#define QuartetBitsTable NAME(QuartetBitsTable)
#define QuartetPixelMaskTable NAME(QuartetPixelMaskTable)
#define cfbAllocatePrivates NAME(AllocatePrivates)
#define cfbBSFuncRec NAME(BSFuncRec)
#define cfbBitBlt NAME(BitBlt)
#define cfbBresD NAME(BresD)
#define cfbBresS NAME(BresS)
#define cfbChangeWindowAttributes NAME(ChangeWindowAttributes)
#define cfbCloseScreen NAME(CloseScreen)
#define cfbCopyArea NAME(CopyArea)
#define cfbCopyImagePlane NAME(CopyImagePlane)
#define cfbCopyPixmap NAME(CopyPixmap)
#define cfbCopyPlane NAME(CopyPlane)
#define cfbCopyRotatePixmap NAME(CopyRotatePixmap)
#define cfbCopyWindow NAME(CopyWindow)
#define cfbCreateGC NAME(CreateGC)
#define cfbCreatePixmap NAME(CreatePixmap)
#define cfbCreateWindow NAME(CreateWindow)
#define cfbCreateScreenResources NAME(CreateScreenResources)
#define cfbDestroyPixmap NAME(DestroyPixmap)
#define cfbDestroyWindow NAME(DestroyWindow)
#define cfbDoBitblt NAME(DoBitblt)
#define cfbDoBitbltCopy NAME(DoBitbltCopy)
#define cfbDoBitbltGeneral NAME(DoBitbltGeneral)
#define cfbDoBitbltOr NAME(DoBitbltOr)
#define cfbDoBitbltXor NAME(DoBitbltXor)
#define cfbFillBoxSolid NAME(FillBoxSolid)
#define cfbFillBoxTile32 NAME(FillBoxTile32)
#define cfbFillBoxTile32sCopy NAME(FillBoxTile32sCopy)
#define cfbFillBoxTile32sGeneral NAME(FillBoxTile32sGeneral)
#define cfbFillBoxTileOdd NAME(FillBoxTileOdd)
#define cfbFillBoxTileOddCopy NAME(FillBoxTileOddCopy)
#define cfbFillBoxTileOddGeneral NAME(FillBoxTileOddGeneral)
#define cfbFillPoly1RectCopy NAME(FillPoly1RectCopy)
#define cfbFillPoly1RectGeneral NAME(FillPoly1RectGeneral)
#define cfbFillRectSolidCopy NAME(FillRectSolidCopy)
#define cfbFillRectSolidGeneral NAME(FillRectSolidGeneral)
#define cfbFillRectSolidXor NAME(FillRectSolidXor)
#define cfbFillRectTile32Copy NAME(FillRectTile32Copy)
#define cfbFillRectTile32General NAME(FillRectTile32General)
#define cfbFillRectTileOdd NAME(FillRectTileOdd)
#define cfbFillSpanTile32sCopy NAME(FillSpanTile32sCopy)
#define cfbFillSpanTile32sGeneral NAME(FillSpanTile32sGeneral)
#define cfbFillSpanTileOddCopy NAME(FillSpanTileOddCopy)
#define cfbFillSpanTileOddGeneral NAME(FillSpanTileOddGeneral)
#define cfbFinishScreenInit NAME(FinishScreenInit)
#define cfbGCFuncs NAME(GCFuncs)
#define cfbGetImage NAME(GetImage)
#define cfbGetSpans NAME(GetSpans)
#define cfbHorzS NAME(HorzS)
#define cfbImageGlyphBlt8 NAME(ImageGlyphBlt8)
#define cfbLineSD NAME(LineSD)
#define cfbLineSS NAME(LineSS)
#define cfbMapWindow NAME(MapWindow)
#define cfbMatchCommon NAME(MatchCommon)
#define cfbNonTEOps NAME(NonTEOps)
#define cfbNonTEOps1Rect NAME(NonTEOps1Rect)
#define cfbPadPixmap NAME(PadPixmap)
#define cfbPaintWindow NAME(PaintWindow)
#define cfbPolyGlyphBlt8 NAME(PolyGlyphBlt8)
#define cfbPolyGlyphRop8 NAME(PolyGlyphRop8)
#define cfbPolyFillArcSolidCopy NAME(PolyFillArcSolidCopy)
#define cfbPolyFillArcSolidGeneral NAME(PolyFillArcSolidGeneral)
#define cfbPolyFillRect NAME(PolyFillRect)
#define cfbPolyPoint NAME(PolyPoint)
#define cfbPositionWindow NAME(PositionWindow)
#define cfbPutImage NAME(PutImage)
#define cfbReduceRasterOp NAME(ReduceRasterOp)
#define cfbRestoreAreas NAME(RestoreAreas)
#define cfbSaveAreas NAME(SaveAreas)
#define cfbScreenInit NAME(ScreenInit)
#define cfbSegmentSD NAME(SegmentSD)
#define cfbSegmentSS NAME(SegmentSS)
#define cfbSetScanline NAME(SetScanline)
#define cfbSetSpans NAME(SetSpans)
#define cfbSetupScreen NAME(SetupScreen)
#define cfbSolidSpansCopy NAME(SolidSpansCopy)
#define cfbSolidSpansGeneral NAME(SolidSpansGeneral)
#define cfbSolidSpansXor NAME(SolidSpansXor)
#define cfbStippleStack NAME(StippleStack)
#define cfbStippleStackTE NAME(StippleStackTE)
#define cfbTEGlyphBlt NAME(TEGlyphBlt)
#define cfbTEOps NAME(TEOps)
#define cfbTEOps1Rect NAME(TEOps1Rect)
#define cfbTile32FSCopy NAME(Tile32FSCopy)
#define cfbTile32FSGeneral NAME(Tile32FSGeneral)
#define cfbUnmapWindow NAME(UnmapWindow)
#define cfbUnnaturalStippleFS NAME(UnnaturalStippleFS)
#define cfbUnnaturalTileFS NAME(UnnaturalTileFS)
#define cfbValidateGC NAME(ValidateGC)
#define cfbVertS NAME(VertS)
#define cfbXRotatePixmap NAME(XRotatePixmap)
#define cfbYRotatePixmap NAME(YRotatePixmap)
#define cfbendpartial NAME(endpartial)
#define cfbendtab NAME(endtab)
#define cfbmask NAME(mask)
#define cfbrmask NAME(rmask)
#define cfbstartpartial NAME(startpartial)
#define cfbstarttab NAME(starttab)
#define cfb8LineSS1Rect NAME(LineSS1Rect)
#define cfb8SegmentSS1Rect NAME(SegmentSS1Rect)
#define cfb8ClippedLineCopy NAME(ClippedLineCopy)
#define cfb8ClippedLineXor NAME(ClippedLineXor)
#define cfb8ClippedLineGeneral  NAME(ClippedLineGeneral )
#define cfb8SegmentSS1RectCopy NAME(SegmentSS1RectCopy)
#define cfb8SegmentSS1RectXor NAME(SegmentSS1RectXor)
#define cfb8SegmentSS1RectGeneral  NAME(SegmentSS1RectGeneral )
#define cfb8SegmentSS1RectShiftCopy NAME(SegmentSS1RectShiftCopy)
#define cfb8LineSS1RectCopy NAME(LineSS1RectCopy)
#define cfb8LineSS1RectXor NAME(LineSS1RectXor)
#define cfb8LineSS1RectGeneral  NAME(LineSS1RectGeneral )
#define cfb8LineSS1RectPreviousCopy NAME(LineSS1RectPreviousCopy)
#define cfbZeroPolyArcSS8Copy NAME(ZeroPolyArcSSCopy)
#define cfbZeroPolyArcSS8Xor NAME(ZeroPolyArcSSXor)
#define cfbZeroPolyArcSS8General NAME(ZeroPolyArcSSGeneral)

#endif /* PSZ != 8 */
