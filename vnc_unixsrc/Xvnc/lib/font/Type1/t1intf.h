/* $Xorg: t1intf.h,v 1.3 2000/08/17 19:46:33 cpqbld Exp $ */
/* Copyright International Business Machines,Corp. 1991
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is
 * hereby granted, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the name of IBM not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.
 *
 * IBM PROVIDES THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES
 * OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE QUALITY AND
 * PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF
 * THE SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM) ASSUMES
 * THE ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN
 * NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/* Copyright (c) 1994-1999 Silicon Graphics, Inc. All Rights Reserved.
 *
 * The contents of this file are subject to the CID Font Code Public Licence
 * Version 1.0 (the "License"). You may not use this file except in compliance
 * with the Licence. You may obtain a copy of the License at Silicon Graphics,
 * Inc., attn: Legal Services, 2011 N. Shoreline Blvd., Mountain View, CA
 * 94043 or at http://www.sgi.com/software/opensource/cid/license.html.
 *
 * Software distributed under the License is distributed on an "AS IS" basis.
 * ALL WARRANTIES ARE DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED
 * WARRANTIES OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR PURPOSE OR OF
 * NON-INFRINGEMENT. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Software is CID font code that was developed by Silicon
 * Graphics, Inc.
 */
/* $XFree86: xc/lib/font/Type1/t1intf.h,v 1.7 2001/07/25 15:04:55 dawes Exp $ */

#ifdef BUILDCID
#include "AFM.h"
#endif
 
struct type1font {
       CharInfoPtr  pDefault;
       CharInfoRec  glyphs[256];
};

#ifdef BUILDCID
typedef struct cid_glyphs {
       char           *CIDFontName;
       char           *CMapName;
       long            dataoffset;
       double          pixel_matrix[4];
       CharInfoPtr     pDefault;
       CharInfoRec   **glyphs;
       FontInfo       *AFMinfo;
#ifdef USE_MMAP
       unsigned char  *CIDdata;
       long            CIDsize;
#endif
} cidglyphs;
#endif

/*
 * Function prototypes
 */
/* t1funcs.c */
#ifdef BUILDCID
extern int CIDOpenScalable ( FontPathElementPtr fpe, FontPtr *ppFont,
			     int flags, FontEntryPtr entry, char *fileName, 
			     FontScalablePtr vals, fsBitmapFormat format, 
			     fsBitmapFormatMask fmask, 
			     FontPtr non_cachable_font );
#endif
extern int Type1OpenScalable ( FontPathElementPtr fpe, FontPtr *ppFont, 
			       int flags, FontEntryPtr entry, char *fileName,
			       FontScalablePtr vals, fsBitmapFormat format, 
			       fsBitmapFormatMask fmask,
			       FontPtr non_cachable_font );
#ifdef BUILDCID
extern unsigned int getCID ( FontPtr pFont, unsigned int charcode );
extern int CIDGetGlyphs ( FontPtr pFont, unsigned long count, 
			  unsigned char *chars, FontEncoding charEncoding, 
			  unsigned long *glyphCount, CharInfoPtr *glyphs );
extern int CIDGetMetrics ( FontPtr pFont, unsigned long count, 
			   unsigned char *chars, FontEncoding charEncoding, 
			   unsigned long *glyphCount, xCharInfo **glyphs );
extern void CIDCloseFont ( FontPtr pFont );
#endif
extern void Type1CloseFont ( FontPtr pFont );
extern int Type1ReturnCodeToXReturnCode ( int rc );
#ifdef BUILDCID
extern CharInfoPtr CIDRenderGlyph ( FontPtr pFont, psobj *charstringP, 
				    psobj *subarrayP, 
				    struct blues_struct *bluesP, 
				    CharInfoPtr pci, int *mode );
#endif

/* t1info.c */
#ifdef CID_ALL_CHARS
extern void ComputeBoundsAllChars ( FontPtr pFont, char *cfmfilename, double sxmult );
#endif
#ifdef BUILDCID
extern int CIDGetInfoScalable ( FontPathElementPtr fpe, FontInfoPtr pInfo, 
				FontEntryPtr entry, FontNamePtr fontName, 
				char *fileName, FontScalablePtr Vals );
#endif
extern int Type1GetInfoScalable ( FontPathElementPtr fpe, FontInfoPtr pInfo, 
				  FontEntryPtr entry, FontNamePtr fontName, 
				  char *fileName, FontScalablePtr Vals );
#ifdef BUILDCID
extern void CIDFillFontInfo ( FontPtr pFont, FontScalablePtr Vals, 
			      char *Filename, char *Fontname, char *Cmapname, 
#ifdef HAVE_CFM
			      char *cfmfilename,
#endif
			      long sAscent, long sDescent, double sxmult );
#endif
extern void T1FillFontInfo ( FontPtr pFont, FontScalablePtr Vals, 
			     char *Filename, char *Fontname, long sWidth );
extern void Type1InitStdProps ( void );

/* cidchar.c */
extern CharInfoPtr CIDGetGlyphInfo ( FontPtr pFont, unsigned int cidcode, 
				     CharInfoPtr pci, int *rc );
extern int CIDGetAFM ( FontPtr pFont, unsigned long count, 
		       unsigned char *chars, FontEncoding charEncoding, 
		       unsigned long *glyphCount, CharInfoPtr *glyphs, 
		       char *cidafmfile );
