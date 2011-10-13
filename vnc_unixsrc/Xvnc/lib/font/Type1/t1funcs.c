/* $Xorg: t1funcs.c,v 1.5 2001/02/09 02:04:01 xorgcvs Exp $ */
/* Copyright International Business Machines,Corp. 1991
 * All Rights Reserved
 *
 * License, subject to the license given below, to use,
 * copy, modify, and distribute this software * and its
 * documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear
 * in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the name of IBM not be used in advertising or
 * publicity pertaining to distribution of the software
 * without specific, written prior permission.
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
 *
 * Author: Jeffrey B. Lotspiech, IBM Almaden Research Center
 *   Modeled on spfuncs.c by Dave Lemke, Network Computing Devices, Inc
 *   which contains the following copyright and permission notices:
 *
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices
 * or Digital not be used in advertising or publicity pertaining to 
 * distribution of the software without specific, written prior permission.  
 * Network Computing Devices or Digital make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES OR DIGITAL BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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
/* $XFree86: xc/lib/font/Type1/t1funcs.c,v 3.33 2003/07/19 13:16:40 tsi Exp $ */

/*

Copyright 1987, 1994, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef BUILDCID
#define XFONT_CID 1
#endif

#ifndef FONTMODULE
#include <string.h>
#if XFONT_CID
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#endif
#ifdef _XOPEN_SOURCE
#include <math.h>
#else
#define _XOPEN_SOURCE	/* to get prototype for hypot on some systems */
#include <math.h>
#undef _XOPEN_SOURCE
#endif
#include "X11/Xfuncs.h"
#ifdef USE_MMAP
#include <sys/types.h>
#include <sys/mman.h>
#endif
#else
#include "Xmd.h"
#include "Xdefs.h"
#endif

#ifdef FONTMODULE
#include "os.h"
#include "xf86_ansic.h"
#endif

#include <X11/fonts/fntfilst.h>
#include <X11/fonts/fontutil.h>
#include <X11/fonts/FSproto.h>
#include <X11/fonts/fontenc.h>
#include "t1unicode.h"
 
#if XFONT_CID
#include "range.h"
#endif

#include "objects.h"
#include "spaces.h"
#include "paths.h"
#include "regions.h"
#include "t1stdio.h"
#include "util.h"
#include "fontfcn.h"
#include "t1intf.h"


static int Type1GetGlyphs ( FontPtr pFont, unsigned long count,
			    unsigned char *chars, FontEncoding charEncoding, 
			    unsigned long *glyphCount, CharInfoPtr *glyphs );

#if XFONT_CID
#define CMapDir "/CMap/"
#define CFMDir "/CFM/"
#define CIDFontDir "/CIDFont/"
#endif

static int Type1GetMetrics ( FontPtr pFont, unsigned long count, 
				    unsigned char *chars, 
				    FontEncoding charEncoding, 
				    unsigned long *glyphCount, 
				    xCharInfo **glyphs );

 
#define minchar(p) ((p).min_char_low + ((p).min_char_high << 8))
#define maxchar(p) ((p).max_char_low + ((p).max_char_high << 8))

static void fillrun ( char *p, pel x0, pel x1, int bit );
 
extern psfont *FontP;
extern psobj *ISOLatin1EncArrayP;

#if XFONT_CID
extern char CurCIDFontName[];
extern char CurCMapName[];

static CharInfoPtr CIDGetGlyph ( FontPtr pFont, unsigned int charcode, 
				 CharInfoPtr pci );

extern cidfont *CIDFontP;
extern cmapres *CMapP;
#endif

static void fill ( char *dest, int h, int w, struct region *area, int byte, 
		   int bit, int wordsize );

#if XFONT_CID
int 
CIDOpenScalable (FontPathElementPtr fpe, 
		 FontPtr *ppFont, 
		 int flags, 
		 FontEntryPtr entry, 
		 char *fileName, 
		 FontScalablePtr vals, 
		 fsBitmapFormat format,
		 fsBitmapFormatMask fmask, 
		 FontPtr non_cachable_font) /* We don't do licensing */
{
    FontPtr     pFont;
    int         bit,
                byte,
                glyph,
                scan,
                image;
    long *pool;           /* memory pool for ximager objects              */
    int size;             /* for memory size calculations                 */
    struct XYspace *S;    /* coordinate space for character               */
    register int i;
    int nchars, len, rc;
    cidglyphs *cid;
    char *p;
    double t1 = .001, t2 = 0.0, t3 = 0.0, t4 = .001;
    double sxmult;
    char CIDFontName[CID_NAME_MAX];
    char CMapName[CID_NAME_MAX];
    char cidfontname[CID_PATH_MAX];
    char cmapname[CID_PATH_MAX];
    char *path;
    char cidfontpath[CID_PATH_MAX];
    char cmappath[CID_PATH_MAX];
#if defined(HAVE_CFM) || defined(CID_ALL_CHARS)
    char cfmdir[CID_PATH_MAX];
    char cfmfilename[CID_NAME_MAX];
#endif
#if defined(CID_ALL_CHARS)
    char *cf;
#else
    long sAscent, sDescent;
#endif

    /* check the font name */
    len = strlen(fileName);
    if (len <= 0 || len > CID_NAME_MAX - 1)
        return BadFontName;

#if defined(HAVE_CFM) || defined(CID_ALL_CHARS)
    strcpy(cfmdir, fileName);
    p = strrchr(cfmdir, '/');
    if (p) *p = '\0';
#endif

    path = fileName;
    if (!(fileName = strrchr(fileName, '/')))
        return BadFontName;

    len = fileName - path;
    strncpy(cidfontpath, path, len);
    cidfontpath[len] = '\0';
    strcpy(cmappath, cidfontpath);
    strcat(cmappath, CMapDir);
#ifdef HAVE_CFM
    strcpy(cfmdir, cidfontpath);
    strcat(cfmdir, CFMDir);
#endif
    strcat(cidfontpath, CIDFontDir);

    fileName++;

    /* extract the CIDFontName and CMapName from the font name */
    /* check for <CIDFontName>--<CMapName> */
    if ((p = strstr(fileName, "--"))) {
        if (p == fileName)
            return BadFontName;
        else {
            strcpy(CIDFontName, fileName);
            CIDFontName[p - fileName] = '\0';
            p += 2;
            i = 0;
            while (*p && *p != '.')
                CMapName[i++] = *p++;
            CMapName[i] = '\0';
            if ((len = strlen(CMapName)) <= 0)
                return BadFontName;
        }
    } else
        return BadFontName;

    /* The CMap files whose names end with -V are not yet supported */
    len = strlen(CMapName);
    if ((len >= 2 && CMapName[len - 2] == '-' && CMapName[len - 1] == 'V') ||
        (len == 1 && CMapName[len - 1] == 'V'))
        return BadFontName;

    /* Reject ridiculously small font sizes that will blow up the math */
    if (hypot(vals->pixel_matrix[0], vals->pixel_matrix[1]) < 1.0 ||
           hypot(vals->pixel_matrix[2], vals->pixel_matrix[3]) < 1.0)
           return BadFontName;

#ifdef CID_ALL_CHARS
    if ((cf = getenv("CFMDIR")) == NULL)
        strcat(cfmdir, CFMDir);
    else {
        strcpy(cfmdir, cf);
        strcat(cfmdir, "/");
    }
#endif

#if defined(HAVE_CFM) || defined(CID_ALL_CHARS)
    strcpy(cfmfilename, cfmdir);
    strcat(cfmfilename, CIDFontName);
    strcat(cfmfilename, "--");
    strcat(cfmfilename, CMapName);
    strcat(cfmfilename, ".cfm");
#endif

    /* create a full-path name for a CIDFont file */
    if (strlen(cidfontpath) + strlen(CIDFontName) + 2 >
        CID_PATH_MAX)
        return BadFontName;
    strcpy(cidfontname, cidfontpath);
    strcat(cidfontname, CIDFontName);

    /* create a full-path name for a CMap file */
    if (strlen(cmappath) + strlen(CMapName) + 2 > CID_PATH_MAX)
        return BadFontName;
    strcpy(cmapname, cmappath);
    strcat(cmapname, CMapName);

    /* set up default values */
    FontDefaultFormat(&bit, &byte, &glyph, &scan);
    /* get any changes made from above */
    rc = CheckFSFormat(format, fmask, &bit, &byte, &scan, &glyph, &image);
    if (rc != Successful)
        return rc;

#define  PAD(bits, pad)  (((bits)+(pad)-1)&-(pad))

    if (!(pFont = CreateFontRec()))
        return AllocError;

    cid = (cidglyphs *)xalloc(sizeof(cidglyphs));
    if (cid == NULL) {
        DestroyFontRec(pFont);
        return AllocError;
    }
    bzero(cid, sizeof(cidglyphs));

    /* heuristic for "maximum" size of pool we'll need: */
    size = 200000 + 600 *
              (int)hypot(vals->pixel_matrix[2], vals->pixel_matrix[3])
              * sizeof(short);
    if (size < 0 || NULL == (pool = (long *) xalloc(size))) {
            xfree(cid);
            DestroyFontRec(pFont);
            return AllocError;
    }

    addmemory(pool, size);

    /* load font if not already loaded */
    if (!CIDfontfcnA(cidfontname, cmapname, &rc)) {
      FontP = NULL;
      delmemory();
      xfree(pool);
      xfree(cid);
      DestroyFontRec(pFont);
      return Type1ReturnCodeToXReturnCode(rc);
    }

    FontP = NULL;

    S = (struct XYspace *) t1_Transform((struct xobject *)IDENTITY, 
					t1, t2, t3, t4);

    S = (struct XYspace *) Permanent(t1_Transform((struct xobject *)S, 
						   vals->pixel_matrix[0],
						   -vals->pixel_matrix[1],
						   vals->pixel_matrix[2],
						   -vals->pixel_matrix[3]));

    /* multiplier for computation of raw values */
    sxmult = hypot(vals->pixel_matrix[0], vals->pixel_matrix[1]);
    if (sxmult > EPS) sxmult = 1000.0 / sxmult;

    pFont->info.firstRow = CMapP->firstRow;
    pFont->info.firstCol = CMapP->firstCol;
    pFont->info.lastRow = CMapP->lastRow;
    pFont->info.lastCol = CMapP->lastCol;

    nchars = (pFont->info.lastRow - pFont->info.firstRow + 1) *
        (pFont->info.lastCol - pFont->info.firstCol + 1);

    delmemory();
    xfree(pool);

    if (pFont->info.firstCol > pFont->info.lastCol)
    {
      xfree(cid);
      DestroyFontRec(pFont);
      return BadFontName;
    }

    cid->glyphs = (CharInfoRec **)xalloc(nchars*sizeof(CharInfoRec *));
    if (cid->glyphs == NULL) {
      xfree(cid);
      DestroyFontRec(pFont);
      return AllocError;
    }
    bzero(cid->glyphs, nchars*sizeof(CharInfoRec *));

    pFont->info.defaultCh = 0;
    pFont->format      = format;

    pFont->bit         = bit;
    pFont->byte        = byte;
    pFont->glyph       = glyph;
    pFont->scan        = scan;

    pFont->get_metrics = CIDGetMetrics;
    pFont->get_glyphs  = CIDGetGlyphs;
    pFont->unload_font = CIDCloseFont;
    pFont->unload_glyphs = NULL;
    pFont->refcnt = 0;

    len = strlen(cidfontname);
    cid->CIDFontName = (char *)xalloc(len + 1);
    if (cid->CIDFontName == NULL) {
      xfree(cid->glyphs);
      xfree(cid);
      DestroyFontRec(pFont);
      return AllocError;
    }
    strcpy(cid->CIDFontName, cidfontname);

    len = strlen(cmapname);
    cid->CMapName = (char *)xalloc(len + 1);
    if (cid->CMapName == NULL) {
      xfree(cid->CIDFontName);
      xfree(cid->glyphs);
      xfree(cid);
      DestroyFontRec(pFont);
      return AllocError;
    }
    strcpy(cid->CMapName, cmapname);

    cid->pixel_matrix[0] = vals->pixel_matrix[0];
    cid->pixel_matrix[1] = vals->pixel_matrix[1];
    cid->pixel_matrix[2] = vals->pixel_matrix[2];
    cid->pixel_matrix[3] = vals->pixel_matrix[3];

    pFont->fontPrivate = (unsigned char *)cid;

    pFont->info.fontAscent =
      (CIDFontP->CIDfontInfoP[CIDFONTBBOX].value.data.arrayP[3].data.integer *
      vals->pixel_matrix[3] +
      (CIDFontP->CIDfontInfoP[CIDFONTBBOX].value.data.arrayP[3].data.integer >
      0 ? 500 : -500)) / 1000;

    pFont->info.fontDescent =
      -(int)((double)CIDFontP->CIDfontInfoP[CIDFONTBBOX].value.data.arrayP[1].data.integer
      * vals->pixel_matrix[3] +
      (CIDFontP->CIDfontInfoP[CIDFONTBBOX].value.data.arrayP[1].data.integer >
      0 ? 500 : -500)) / 1000;

    /* Adobe does not put isFixedPitch entries in CID-keyed fonts.  */
    /* CID-keyed are not constant-width fonts.                      */
    pFont->info.constantWidth = 0;

#ifndef CID_ALL_CHARS
    sAscent = CIDFontP->CIDfontInfoP[CIDFONTBBOX].value.data.arrayP[3].data.integer;
    sDescent = -CIDFontP->CIDfontInfoP[CIDFONTBBOX].value.data.arrayP[1].data.integer;
#endif

    if (strncmp(entry->name.name, "-bogus", 6)) {
#ifdef CID_ALL_CHARS
      ComputeBoundsAllChars(pFont, cfmfilename, sxmult);
#else
#ifdef HAVE_CFM
      CIDFillFontInfo(pFont, vals, cidfontname, entry->name.name, cmapname,
                    cfmfilename, sAscent, sDescent, sxmult);
#else
      CIDFillFontInfo(pFont, vals, cidfontname, entry->name.name, cmapname,
                    sAscent, sDescent, sxmult);
#endif /* HAVE_CFM */
#endif /* CID_ALL_CHARS */
    }

    *ppFont = pFont;

    return Successful;
}
#endif
 
/*ARGSUSED*/
int 
Type1OpenScalable (FontPathElementPtr fpe, 
		   FontPtr *ppFont, 
		   int flags, 
		   FontEntryPtr entry, 
		   char *fileName, 
		   FontScalablePtr vals, 
		   fsBitmapFormat format,
		   fsBitmapFormatMask fmask, 
		   FontPtr non_cachable_font) 	/* We don't do licensing */
{
       FontPtr     pFont;
       int         bit,
                   byte,
                   glyph,
                   scan,
                   image;
       int pad,wordsize;     /* scan & image in bits                         */
       long *pool;           /* memory pool for ximager objects              */
       int size;             /* for memory size calculations                 */
       struct XYspace *S;    /* coordinate space for character               */
       struct region *area;
       CharInfoRec *glyphs;
       int len, rc, count = 0, i = 0;
       struct type1font *type1;
       char *p;
       FontMapPtr mapping = NULL;
       int no_mapping;
       psobj *fontmatrix;
       long x0, total_width = 0, total_raw_width = 0;
       double x1, y1, t1 = .001, t2 = 0.0, t3 = 0.0, t4 = .001;
       double sxmult;

       /* Reject ridiculously small font sizes that will blow up the math */
       if (hypot(vals->pixel_matrix[0], vals->pixel_matrix[1]) < 1.0 ||
	   hypot(vals->pixel_matrix[2], vals->pixel_matrix[3]) < 1.0)
	   return BadFontName;

       /* set up default values */
       FontDefaultFormat(&bit, &byte, &glyph, &scan);
       /* get any changes made from above */
       rc = CheckFSFormat(format, fmask, &bit, &byte, &scan, &glyph, &image);
       if (rc != Successful)
               return rc;
 
       pad                = glyph * 8;
       wordsize           = scan * 8;
 
#define  PAD(bits, pad)  (((bits)+(pad)-1)&-(pad))
 
       pFont = CreateFontRec();
       if (pFont == NULL)
           return AllocError;

       type1 = (struct type1font *)xalloc(sizeof(struct type1font));
       if (type1 == NULL) {
               DestroyFontRec(pFont);
               return AllocError;
       }
       bzero(type1, sizeof(struct type1font));
 
       /* heuristic for "maximum" size of pool we'll need: */
#if XFONT_CID
       size = 400000 + 600 *
#else
       size = 200000 + 600 *
#endif
	      (int)hypot(vals->pixel_matrix[2], vals->pixel_matrix[3])
	      * sizeof(short);
       if (size < 0 || NULL == (pool = (long *) xalloc(size))) {
               xfree(type1);
               DestroyFontRec(pFont);
               return AllocError;
       }
 
       addmemory(pool, size);
 
 
       glyphs = type1->glyphs;
 
       /* load font if not already loaded */
       if (!fontfcnA(fileName, &rc)) {
         delmemory();
	 xfree(type1);
	 DestroyFontRec(pFont);
         xfree(pool);
         return Type1ReturnCodeToXReturnCode(rc);
       }

       fontmatrix = &FontP->fontInfoP[FONTMATRIX].value;
       if (objPIsArray(fontmatrix) && fontmatrix->len == 6)
       {
#define assign(n,d,f) if (objPIsInteger(fontmatrix->data.arrayP + n)) \
			  d = fontmatrix->data.arrayP[n].data.integer; \
		      else if (objPIsReal(fontmatrix->data.arrayP + n)) \
			  d = fontmatrix->data.arrayP[n].data.real; \
		      else d = f;

	   assign(0, t1, .001);
	   assign(1, t2, 0.0);
	   assign(2, t3, 0.0);
	   assign(3, t4, .001);
       }

       S = (struct XYspace *) t1_Transform((struct xobject *)IDENTITY,
					   t1, t2, t3, t4);

       S = (struct XYspace *) Permanent(t1_Transform((struct xobject *)S, 
						     vals->pixel_matrix[0],
						     -vals->pixel_matrix[1],
						     vals->pixel_matrix[2],
						     -vals->pixel_matrix[3]));


       /* multiplier for computation of raw values */
       sxmult = hypot(vals->pixel_matrix[0], vals->pixel_matrix[1]);
       if (sxmult > EPS) sxmult = 1000.0 / sxmult;

       no_mapping=0;
       p = FontEncFromXLFD(entry->name.name, entry->name.length);

       if(p==0) {               /* XLFD does not specify an encoding */
           mapping=0;
           no_mapping=2;        /* ISO 8859-1 */
       }

       if(!strcmp(p, "adobe-fontspecific")) {
           mapping=0;
           no_mapping=1;        /* font's native encoding vector */
       }

       pFont->info.firstCol = 255;
       pFont->info.lastCol  = 0;

       if(!no_mapping) {
           mapping = FontEncMapFind(p, 
                                    FONT_ENCODING_POSTSCRIPT, -1, -1,
                                    fileName);
           if(!mapping)
               mapping = FontEncMapFind(p, 
                                        FONT_ENCODING_UNICODE, -1, -1,
                                        fileName);
           if(!mapping)
	       goto NoEncoding;
           else
               no_mapping=0;
       }

       for (i=0; i < 256; i++) {
               long h,w;
               long paddedW;
	       int j;
	       char *codename;

               if(no_mapping == 1) {
                   codename = FontP->fontInfoP[ENCODING].
                       value.data.arrayP[i].data.valueP;
                   len = FontP->fontInfoP[ENCODING].
                       value.data.arrayP[i].len;
               } else if(no_mapping) {
                   codename = unicodetoPSname(i);
                 len = codename ? strlen(codename) : 0;
               } else {
                 if(mapping->type == FONT_ENCODING_UNICODE) {
                     codename = unicodetoPSname(FontEncRecode(i, mapping));
                 } else
                     codename = FontEncName(i, mapping);
                 len=codename?strlen(codename):0;
               }

               /* Avoid multiply rasterising the undefined glyph */
               if(len==7 && !strncmp(codename, ".notdef", 7)) {
                   len=0;
                   codename=0;
               }

               /* But do rasterise it at least once */
               if(len==0) {
                   if(i==0) {
                       codename=".notdef";
                       len=7;
                   } else
                       continue;
               }

	       /* See if this character is in the list of ranges specified
		  in the XLFD name */
               if(i!=0) {
                   for (j = 0; j < vals->nranges; j++)
                       if (i >= minchar(vals->ranges[j]) &&
                           i <= maxchar(vals->ranges[j]))
                           break;

                   /* If not, don't realize it. */
                   if (vals->nranges && j == vals->nranges)
                       continue;
               }

               rc = 0;
               area = (struct region *)fontfcnB(S, (unsigned char *)codename,
                                                &len, &rc);
               if (rc < 0) {
                       rc = Type1ReturnCodeToXReturnCode(rc);
                       break;
               }
               else if (rc > 0)
                       continue;
 
               if (area == NULL)
                       continue;
 
	       if (pFont->info.firstCol > i)
		   pFont->info.firstCol = i;
	       if (pFont->info.lastCol < i)
		   pFont->info.lastCol = i;

               h       = area->ymax - area->ymin;
               w       = area->xmax - area->xmin;
               paddedW = PAD(w, pad);
 
               if (h > 0 && w > 0) {
                       size = h * paddedW / 8;
                       glyphs[i].bits = (char *)xalloc(size);
                       if (glyphs[i].bits == NULL) {
                               rc = AllocError;
                               break;
                       }
               }
               else {
		       size = 0;
                       h = w = 0;
                       area->xmin = area->xmax = 0;
                       area->ymax = area->ymax = 0;
               }
 
               glyphs[i].metrics.leftSideBearing  = area->xmin;
	       x1 = (double)(x0 = area->ending.x - area->origin.x);
	       y1 = (double)(area->ending.y - area->origin.y);
               glyphs[i].metrics.characterWidth   =
		   (x0 + (x0 > 0 ? FPHALF : -FPHALF)) / (1 << FRACTBITS);
               if (!glyphs[i].metrics.characterWidth && size == 0)
	       {
		   /* Zero size and zero extents: presumably caused by
		      the choice of transformation.  Let's create a
		      small bitmap so we're not mistaken for an undefined
		      character. */
		   h = w = 1;
		   size = paddedW = PAD(w, pad);
		   glyphs[i].bits = (char *)xalloc(size);
                   if (glyphs[i].bits == NULL) {
                       rc = AllocError;
                       break;
                   }
	       }
               glyphs[i].metrics.attributes =
		   NEARESTPEL((long)(hypot(x1, y1) * sxmult));
	       total_width += glyphs[i].metrics.attributes;
	       total_raw_width += abs((int)(INT16)glyphs[i].metrics.attributes);
	       count++;
               glyphs[i].metrics.rightSideBearing = w + area->xmin;
               glyphs[i].metrics.descent          = area->ymax - NEARESTPEL(area->origin.y);
               glyphs[i].metrics.ascent           = h - glyphs[i].metrics.descent;
 
 
               bzero(glyphs[i].bits, size);
               if (h > 0 && w > 0) {
                   fill(glyphs[i].bits, h, paddedW, area, byte, bit, wordsize );
               }
 
               Destroy(area);
       }
 NoEncoding:
       
       delmemory();
       xfree(pool);
 
       if (pFont->info.firstCol > pFont->info.lastCol)
       {
               xfree(type1);
               DestroyFontRec(pFont);
               return BadFontName;
       }
 
       if (i != 256) {
               for (i--; i >= 0; i--)
                       if (glyphs[i].bits != NULL)
                               xfree(glyphs[i].bits);
               xfree(type1);
               DestroyFontRec(pFont);
               return rc;
       }
       type1->pDefault    = NULL;
 
       pFont->format      = format;
 
       pFont->bit         = bit;
       pFont->byte        = byte;
       pFont->glyph       = glyph;
       pFont->scan        = scan;
 
       pFont->info.firstRow = 0;
       pFont->info.lastRow  = 0;
 
       pFont->get_metrics = Type1GetMetrics;
       pFont->get_glyphs  = Type1GetGlyphs;
       pFont->unload_font = Type1CloseFont;
       pFont->unload_glyphs = NULL;
       pFont->refcnt = 0;
 
       pFont->fontPrivate = (unsigned char *) type1;

       if (count)
       {
	   total_raw_width = (total_raw_width * 10 + count / 2) / count;
	   if (total_width < 0)
	   {
	       /* Predominant direction is R->L */
	       total_raw_width = -total_raw_width;
	   }
	   vals->width = (int)((double)total_raw_width *
			       vals->pixel_matrix[0] / 1000.0 +
			       (vals->pixel_matrix[0] > 0 ? .5 : -.5));
       }

       T1FillFontInfo(pFont, vals, fileName, entry->name.name, total_raw_width);
 
       *ppFont = pFont;
       return Successful;
}

#if XFONT_CID
unsigned int 
getCID(FontPtr pFont, unsigned int charcode)
{
    unsigned int cidcode = 0;
    Bool charvalid = FALSE;
    cidglyphs *cid;
    int i, j;
    unsigned int char_row, char_col, rangelo_row, rangelo_col, k;
    unsigned int rangehi_row, rangehi_col;
    spacerange *spacerangeP;
    cidrange *notdefrangeP, *cidrangeP;

    cid = (cidglyphs *)pFont->fontPrivate;

    if (cid == NULL)
        return cidcode;

    char_row = (charcode >> 8) & 0xff;
    char_col = charcode & 0xff;

    spacerangeP = CIDFontP->spacerangeP;
    for (i = 0; i < CIDFontP->spacerangecnt; i++) {
      for (j = 0; j < spacerangeP->rangecnt; j++) {
        rangelo_row =
          (spacerangeP->spacecode[j].srcCodeLo >> 8) & 0xff;
        rangelo_col = spacerangeP->spacecode[j].srcCodeLo & 0xff;
        rangehi_row =
          (spacerangeP->spacecode[j].srcCodeHi >> 8) & 0xff;
        rangehi_col = spacerangeP->spacecode[j].srcCodeHi & 0xff;
        if (char_row >= rangelo_row && char_row <= rangehi_row &&
            char_col >= rangelo_col && char_col <= rangehi_col) {
            charvalid = TRUE;
            break;
        }
      }
      if (charvalid) break;
      spacerangeP = spacerangeP->next;
    }

    if (charvalid) {
      charvalid = FALSE;
      cidrangeP = CIDFontP->cidrangeP;
      for (i = 0; i < CIDFontP->cidrangecnt; i++) {
        for (j = 0; j < cidrangeP->rangecnt; j++) {
          rangelo_row =
            (cidrangeP->range[j].srcCodeLo >> 8) & 0xff;
          rangelo_col = cidrangeP->range[j].srcCodeLo & 0xff;
          rangehi_row =
            (cidrangeP->range[j].srcCodeHi >> 8) & 0xff;
          rangehi_col = cidrangeP->range[j].srcCodeHi & 0xff;
          if (char_row >= rangelo_row && char_row <= rangehi_row &&
            char_col >= rangelo_col && char_col <= rangehi_col) {
            charvalid = TRUE;
            for (k = cidrangeP->range[j].srcCodeLo;
              k <= cidrangeP->range[j].srcCodeHi; k++) {
              if (k == charcode)
                cidcode = cidrangeP->range[j].dstCIDLo + k -
                  cidrangeP->range[j].srcCodeLo;
            }
            break;
          }
        }
        if (charvalid) break;
        cidrangeP = cidrangeP->next;
      }
    }

    if (charvalid) {
      charvalid = FALSE;
      notdefrangeP = CIDFontP->notdefrangeP;
      for (i = 0; i < CIDFontP->notdefrangecnt; i++) {
        for (j = 0; j < notdefrangeP->rangecnt; j++) {
          rangelo_row =
            (notdefrangeP->range[j].srcCodeLo >> 8) & 0xff;
          rangelo_col = notdefrangeP->range[j].srcCodeLo & 0xff;
          rangehi_row =
            (notdefrangeP->range[j].srcCodeHi >> 8) & 0xff;
          rangehi_col = notdefrangeP->range[j].srcCodeHi & 0xff;
          if (char_row >= rangelo_row && char_row <= rangehi_row &&
            char_col >= rangelo_col && char_col <= rangehi_col) {
            charvalid = TRUE;
            for (k = notdefrangeP->range[j].srcCodeLo;
              k <= notdefrangeP->range[j].srcCodeHi; k++) {
              if (k == charcode)
                /* the whole range is mapped to a single CID code */
                cidcode = notdefrangeP->range[j].dstCIDLo;
            }
            break;
          }
        }
        if (charvalid) break;
        notdefrangeP = notdefrangeP->next;
      }
    }

    /* If you specify a CMap that has more CIDs than a specified CIDFont, */
    /* the program could go beyond the number of entries in CIDMap. Make  */
    /* sure that that does not happen.                                    */
    if (cidcode < CIDFontP->CIDfontInfoP[CIDCOUNT].value.data.integer)
        return cidcode;
    else
        return 0;
}

static CharInfoPtr
CIDGetGlyph(FontPtr pFont, unsigned int charcode, CharInfoPtr pci)
{
    int  rc;
    CharInfoPtr cp = NULL;
    unsigned int cidcode;

    /* character code -> CID */
    cidcode = getCID(pFont, charcode);

    cp = CIDGetGlyphInfo(pFont, cidcode, pci, &rc);

    if (rc != Successful && cidcode) {
        cidcode = 0;
        cp = CIDGetGlyphInfo(pFont, cidcode, pci, &rc);
    }

    return cp;
}

int 
CIDGetGlyphs(FontPtr pFont, 
	     unsigned long count, 
	     unsigned char *chars, 
	     FontEncoding charEncoding, 
	     unsigned long *glyphCount, /* RETURN */
	     CharInfoPtr *glyphs)	/* RETURN */
{
    unsigned int code, char_row, char_col;
    CharInfoPtr *glyphsBase;
    register unsigned int c;
    CharInfoPtr pci;
    CharInfoPtr pDefault;
    cidglyphs *cid;
    register int firstCol;
    int rc = 0;
    int cid_valid = 0;

    cid = (cidglyphs *)pFont->fontPrivate;

    FontP = NULL;

    firstCol   = pFont->info.firstCol;
    pDefault   = cid->pDefault;
    glyphsBase = glyphs;

    switch (charEncoding) {

#define EXIST(pci) \
    ((pci)->metrics.attributes || \
     (pci)->metrics.ascent != -(pci)->metrics.descent || \
     (pci)->metrics.leftSideBearing != (pci)->metrics.rightSideBearing)

    case Linear8Bit:
    case TwoD8Bit:
        if (pFont->info.firstRow > 0)
            break;
        while (count--) {
            c = (*chars++);
            if (c >= firstCol && c <= pFont->info.lastCol) {
                code = c - firstCol;
                if (!(pci = (CharInfoRec *)cid->glyphs[code]) ||
                    ((long)pci->bits == CID_BITMAP_UNDEFINED)) {
                    /* load font if not already loaded */
                    if(!cid_valid) {
                        if(!CIDfontfcnA(cid->CIDFontName, cid->CMapName, &rc)) {                            FontP = NULL;
                            return Type1ReturnCodeToXReturnCode(rc);
                        }
                        cid_valid = 1;
                    }
                    pci = CIDGetGlyph(pFont, c, pci);
                }
                if (pci && EXIST(pci)) {
                    *glyphs++ = pci;
                    cid->glyphs[code] = pci;
                } else if (pDefault) {
                    *glyphs++ = pDefault;
                    cid->glyphs[code] = pDefault;
                }
            } else if (pDefault)
                *glyphs++ = pDefault;
        }
        break;
    case Linear16Bit:
        while (count--) {
            char_row = *chars++;
            char_col = *chars++;
            c = char_row << 8;
            c = (c | char_col);
            if (pFont->info.firstRow <= char_row && char_row <=
                pFont->info.lastRow && pFont->info.firstCol <= char_col &&
                char_col <= pFont->info.lastCol) {
                code = pFont->info.lastCol - pFont->info.firstCol + 1;
                char_row = char_row - pFont->info.firstRow;
                char_col = char_col - pFont->info.firstCol;
                code = char_row * code + char_col;
                if (!(pci = (CharInfoRec *)cid->glyphs[code]) ||
                    ((long)pci->bits == CID_BITMAP_UNDEFINED)) {
                    /* load font if not already loaded */
                    if(!cid_valid) {
                        if(!CIDfontfcnA(cid->CIDFontName, cid->CMapName, &rc)) {                            FontP = NULL;
                            return Type1ReturnCodeToXReturnCode(rc);
                        }
                        cid_valid = 1;
                    }
                    pci = CIDGetGlyph(pFont, c, pci);
                }
                if (pci && EXIST(pci)) {
                    *glyphs++ = pci;
                    cid->glyphs[code] = pci;
                } else if (pDefault) {
                    *glyphs++ = pDefault;
                    cid->glyphs[code] = pDefault;
                }
            } else if (pDefault)
                *glyphs++ = pDefault;
        }
        break;

    case TwoD16Bit:
        while (count--) {
            char_row = (*chars++);
            char_col = (*chars++);
            c = char_row << 8;
            c = (c | char_col);
            if (pFont->info.firstRow <= char_row && char_row <=
                pFont->info.lastRow && pFont->info.firstCol <= char_col &&
                char_col <= pFont->info.lastCol) {
                code = pFont->info.lastCol - pFont->info.firstCol + 1;
                char_row = char_row - pFont->info.firstRow;
                char_col = char_col - pFont->info.firstCol;
                code = char_row * code + char_col;
                if (!(pci = (CharInfoRec *)cid->glyphs[code]) ||
                    ((long)pci->bits == CID_BITMAP_UNDEFINED)) {
                    /* load font if not already loaded */
                    if(!cid_valid) {
                        if(!CIDfontfcnA(cid->CIDFontName, cid->CMapName, &rc)) {                            FontP = NULL;
                            return Type1ReturnCodeToXReturnCode(rc);
                        }
                        cid_valid = 1;
                    }
                    pci = CIDGetGlyph(pFont, c, pci);
                }
                if (pci && EXIST(pci)) {
                    *glyphs++ = pci;
                    cid->glyphs[code] = pci;
                } else if (pDefault) {
                    *glyphs++ = pDefault;
                    cid->glyphs[code] = pDefault;
                }
            } else if (pDefault)
                *glyphs++ = pDefault;
        }
        break;
    }
    *glyphCount = glyphs - glyphsBase;
    return Successful;

#undef EXIST
}
#endif
 
static int
Type1GetGlyphs(FontPtr pFont, 
	       unsigned long count, 
	       unsigned char *chars, 
	       FontEncoding charEncoding, 
	       unsigned long *glyphCount,  /* RETURN */
	       CharInfoPtr *glyphs)	   /* RETURN */
{
    unsigned int firstRow;
    unsigned int numRows;
    CharInfoPtr *glyphsBase;
    register unsigned int c;
    register CharInfoPtr pci;
    unsigned int r;
    CharInfoPtr pDefault;
    register struct type1font *type1Font;
    register int firstCol;
 
    type1Font  = (struct type1font *) pFont->fontPrivate;
    firstCol   = pFont->info.firstCol;
    pDefault   = type1Font->pDefault;
    glyphsBase = glyphs;
 
    switch (charEncoding) {

#define EXIST(pci) \
    ((pci)->metrics.attributes || \
     (pci)->metrics.ascent != -(pci)->metrics.descent || \
     (pci)->metrics.leftSideBearing != (pci)->metrics.rightSideBearing)
 
    case Linear8Bit:
    case TwoD8Bit:
        if (pFont->info.firstRow > 0)
            break;
        while (count--) {
                c = (*chars++);
                if (c >= firstCol &&
                       (pci = &type1Font->glyphs[c]) &&
		       EXIST(pci))
                    *glyphs++ = pci;
                else if (pDefault)
                    *glyphs++ = pDefault;
        }
        break;
    case Linear16Bit:
        while (count--) {
                c = *chars++ << 8;
                c = (c | *chars++);
                if (c < 256 && c >= firstCol &&
                        (pci = &type1Font->glyphs[c]) &&
			EXIST(pci))
                    *glyphs++ = pci;
                else if (pDefault)
                    *glyphs++ = pDefault;
        }
        break;
 
    case TwoD16Bit:
        firstRow = pFont->info.firstRow;
        numRows = pFont->info.lastRow - firstRow + 1;
        while (count--) {
            r = (*chars++) - firstRow;
            c = (*chars++);
            if (r < numRows && c < 256 && c >= firstCol &&
                    (pci = &type1Font->glyphs[(r << 8) + c]) &&
		    EXIST(pci))
                *glyphs++ = pci;
            else if (pDefault)
                *glyphs++ = pDefault;
        }
        break;
    }
    *glyphCount = glyphs - glyphsBase;
    return Successful;

#undef EXIST
}

#if XFONT_CID
static CharInfoRec nonExistantChar;

int
CIDGetMetrics(FontPtr pFont, 
	      unsigned long count, 
	      unsigned char *chars, 
	      FontEncoding charEncoding, 
	      unsigned long *glyphCount, /* RETURN */
	      xCharInfo **glyphs)	 /* RETURN */
{
    int         ret;
    cidglyphs *cid;
    CharInfoPtr oldDefault;
    char cidafmname[CID_PATH_MAX];
    char CIDFontName[CID_NAME_MAX];
    char *ptr;

    cid = (cidglyphs *)pFont->fontPrivate;

    strcpy(cidafmname, cid->CIDFontName);
    if (!(ptr = strrchr(cidafmname, '/')))
        return BadFontName;

    *ptr = '\0';

    strcpy(CIDFontName, ptr + 1);

    if (!(ptr = strrchr(cidafmname, '/')))
        return BadFontName;

    *ptr = '\0';

    strcat(cidafmname, "/AFM/");
    strcat(cidafmname, CIDFontName);

    strcat(cidafmname, ".afm");

    oldDefault = cid->pDefault;
    cid->pDefault = &nonExistantChar;

    ret = CIDGetAFM(pFont, count, chars, charEncoding, glyphCount, (CharInfoPtr
*)glyphs, cidafmname);
    if (ret != Successful)
        ret = CIDGetGlyphs(pFont, count, chars, charEncoding, glyphCount, 
			   (CharInfoPtr *)glyphs);

    *ptr = 0;
    cid->pDefault = oldDefault;
    return ret;
}
#endif

static int
Type1GetMetrics(FontPtr pFont, 
		unsigned long count, 
		unsigned char *chars, 
		FontEncoding charEncoding, 
		unsigned long *glyphCount, /* RETURN */
		xCharInfo **glyphs) 	   /* RETURN */
{
    static CharInfoRec nonExistantChar;
 
    int         ret;
    struct type1font *type1Font;
    CharInfoPtr oldDefault;
 
    type1Font = (struct type1font *) pFont->fontPrivate;
    oldDefault = type1Font->pDefault;
    type1Font->pDefault = &nonExistantChar;
    ret = Type1GetGlyphs(pFont, count, chars, charEncoding, glyphCount, (CharInfoPtr *) glyphs);
    type1Font->pDefault = oldDefault;
    return ret;
}

#if XFONT_CID
void 
CIDCloseFont(FontPtr pFont)
{
    register int i;
    cidglyphs *cid;
    int nchars;

    if (pFont) {

        cid = (cidglyphs *)pFont->fontPrivate;

        if (cid) {

            if (cid->CIDFontName && !strcmp(cid->CIDFontName, CurCIDFontName)
                 && cid->CMapName && !strcmp(cid->CMapName, CurCMapName)){
                 strcpy(CurCIDFontName, ""); /* initialize to none */
                 strcpy(CurCMapName, "");    /* initialize to none */
            }

            if (cid->CIDFontName)
                xfree(cid->CIDFontName);

            if (cid->CMapName)
                xfree(cid->CMapName);

            nchars = (pFont->info.lastRow - pFont->info.firstRow + 1) *
                (pFont->info.lastCol - pFont->info.firstCol + 1);

            for (i = 0; i < nchars; i++) {
                if (cid->glyphs[i] && (cid->glyphs[i] != &nonExistantChar)) {
                    if (cid->glyphs[i]->bits)
                        xfree(cid->glyphs[i]->bits);
                    xfree(cid->glyphs[i]);
                }
            }

            if (cid->glyphs)
                xfree(cid->glyphs);

            if (cid->AFMinfo)
                xfree(cid->AFMinfo);
#ifdef USE_MMAP
            if (cid->CIDdata)
               munmap(cid->CIDdata, cid->CIDsize);
#endif
            xfree(cid);
        }

        if (pFont->info.props)
            xfree(pFont->info.props);

        if (pFont->info.isStringProp)
            xfree(pFont->info.isStringProp);

        DestroyFontRec(pFont);
    }
}
#endif

void 
Type1CloseFont(FontPtr pFont)
{
       register int i;
       struct type1font *type1;
 
       type1 = (struct type1font *) pFont->fontPrivate;
       for (i=0; i < 256; i++)
               if (type1->glyphs[i].bits != NULL)
                        xfree(type1->glyphs[i].bits);
       xfree(type1);

       if (pFont->info.props)
	   xfree(pFont->info.props);

       if (pFont->info.isStringProp)
	   xfree(pFont->info.isStringProp);

       DestroyFontRec(pFont);
}

static void 
fill(char *dest,             /* destination bitmap                           */
     int h, int w,           /* dimensions of 'dest', w padded               */
     struct region *area,    /* region to write to 'dest'                    */
     int byte, int bit,      /* flags; LSBFirst or MSBFirst                  */
     int wordsize)           /* number of bits per word for LSB/MSB purposes */
{
       register struct edgelist *edge;  /* for looping through edges         */
       register char *p;     /* current scan line in 'dest'                  */
       register int y;       /* for looping through scans                    */
       register int wbytes = w / 8;  /* number of bytes in width             */
       register pel *leftP,*rightP;  /* pointers to X values, left and right */
       int xmin = area->xmin;  /* upper left X                               */
       int ymin = area->ymin;  /* upper left Y                               */
 
       for (edge = area->anchor; VALIDEDGE(edge); edge = edge->link->link) {
 
               p = dest + (edge->ymin - ymin) * wbytes;
               leftP = edge->xvalues;
               rightP = edge->link->xvalues;
 
               for (y = edge->ymin; y < edge->ymax; y++) {
                       fillrun(p, *leftP++ - xmin, *rightP++ - xmin, bit);
                       p += wbytes;
               }
       }
/*
Now, as an afterthought, we'll go reorganize if odd byte order requires
it:
*/
       if (byte == LSBFirst && wordsize != 8) {
               register int i;
 
               switch (wordsize) {
                   case 16:
                   {
                       register unsigned short data,*p;
 
                       p = (unsigned short *) dest;
 
                       for (i = h * w /16; --i >= 0;) {
                               data = *p;
                               *p++ = (data << 8) + (data >> 8);
                       }
                       break;
                   }
                   case 64:
                   case 32:
                   {
                       register unsigned long data,*p;
 
                       p = (unsigned long *) dest;
 
                       for (i = h * w / 32; --i >= 0;) {
                               data = *p;
                               *p++ = (data << 24) + (data >> 24)
                                      + (0xFF00 & (data >> 8))
                                      + (0xFF0000 & (data << 8));
                       }
                       if (wordsize == 64) {
 
                               p = (unsigned long *) dest;
 
                               for (i = h * w / 64; --i >= 0;) {
                                       data = *p++;
                                       p[-1] = p[0];
                                       *p++ = data;
                               }
                       }
                       break;
                   }
                   default:
                       Abort("xiFill: unknown format");
               }
       }
 
}
 
#define  ALLONES  0xFF
 
static void 
fillrun(char *p,             /* address of this scan line                    */
	pel x0, pel x1,      /* left and right X                             */
	int bit)             /* format:  LSBFirst or MSBFirst                */
{
       register int startmask,endmask;  /* bits to set in first and last char*/
       register int middle;  /* number of chars between start and end + 1    */
 
       if (x1 <= x0)
               return;
       middle = x1/8 - x0/8;
       p += x0/8;
       x0 &= 7;  x1 &= 7;
       if (bit == LSBFirst) {
               startmask = ALLONES << x0;
               endmask = ~(ALLONES << x1);
       }
       else {
               startmask = ALLONES >> x0;
               endmask = ~(ALLONES >> x1);
       }
       if (middle == 0)
               *p++ |= startmask & endmask;
       else {
               *p++ |= startmask;
               while (--middle > 0)
                       *p++ = (char)ALLONES;
               *p |= endmask;
       }
}
 
#define CAPABILITIES (CAP_MATRIX | CAP_CHARSUBSETTING)

#if XFONT_CID
FontRendererRec CIDRendererInfo[] = {
  { ".cid", 4, NULL, CIDOpenScalable,
        NULL, CIDGetInfoScalable, 0, CAPABILITIES }
};
#endif
 
#if XFONT_CID
FontRendererRec Type1RendererInfo[] = {
#else
static FontRendererRec renderers[] = {
#endif
  { ".pfa", 4, NULL, Type1OpenScalable,
        NULL, Type1GetInfoScalable, 0, CAPABILITIES },
  { ".pfb", 4, NULL, Type1OpenScalable,
        NULL, Type1GetInfoScalable, 0, CAPABILITIES }
};

#if XFONT_CID
void 
CIDRegisterFontFileFunctions(void)
{
    int i;

    Type1InitStdProps();
    for (i=0; i < sizeof(CIDRendererInfo) / sizeof(FontRendererRec); i++)
            FontFileRegisterRenderer(&CIDRendererInfo[i]);
}
#endif
 
void
Type1RegisterFontFileFunctions(void)
{
    int i;
 
#if XFONT_CID
    Type1InitStdProps();
    for (i=0; i < sizeof(Type1RendererInfo) / sizeof(FontRendererRec); i++)
            FontFilePriorityRegisterRenderer(&Type1RendererInfo[i], -10);
#else
    T1InitStdProps();
    for (i=0; i < sizeof(renderers) / sizeof(FontRendererRec); i++)
            FontFilePriorityRegisterRenderer(&renderers[i], -10);
#endif
}

int 
Type1ReturnCodeToXReturnCode(int rc)
{
    switch(rc) {
    case SCAN_OK:
	return Successful;
    case SCAN_FILE_EOF:
	/* fall through to BadFontFormat */
    case SCAN_ERROR:
	return BadFontFormat;
    case SCAN_OUT_OF_MEMORY:
	return AllocError;
    case SCAN_FILE_OPEN_ERROR:
	return BadFontName;
    case SCAN_TRUE:
    case SCAN_FALSE:
    case SCAN_END:
	/* fall through */
    default:
	/* this should not happen */
#if XFONT_CID
        ErrorF("Font return code cannot be converted to X return code: %d\n", rc);
#else
	ErrorF("Type1 return code not convertable to X return code: %d\n", rc);
#endif
	return rc;
    }
}

#if XFONT_CID
CharInfoPtr 
CIDRenderGlyph(FontPtr pFont, psobj *charstringP, psobj *subarrayP,
	       struct blues_struct *bluesP, CharInfoPtr pci, int *mode)
{
       int         bit,
                   byte,
                   glyph,
                   scan,
                   image;
       int pad,wordsize;     /* scan & image in bits                         */
       long *pool;           /* memory pool for ximager objects              */
       int size;             /* for memory size calculations                 */
       struct XYspace *S;    /* coordinate space for character               */
       struct region *area;
       CharInfoRec *glyphs;
       int len, rc;
       long x0;
       double x1, y1, t1 = .001, t2 = 0.0, t3 = 0.0, t4 = .001;
       double sxmult;
       long h,w;
       long paddedW;
       cidglyphs *cid;
       fsBitmapFormat      format = 0;
       fsBitmapFormatMask  fmask = 0;

       cid = (cidglyphs *)pFont->fontPrivate;

       /* set up default values */
       FontDefaultFormat(&bit, &byte, &glyph, &scan);
       /* get any changes made from above */
       rc = CheckFSFormat(format, fmask, &bit, &byte, &scan, &glyph, &image);
       if (rc != Successful) {
               *mode = rc;
               return(NULL);
       }

       pad                = glyph * 8;
       wordsize           = scan * 8;

#define  PAD(bits, pad)  (((bits)+(pad)-1)&-(pad))

       /* heuristic for "maximum" size of pool we'll need: */
       size = 200000 + 600 *
              (int)hypot(cid->pixel_matrix[2], cid->pixel_matrix[3])
              * sizeof(short);
       if (size < 0 || NULL == (pool = (long *) xalloc(size))) {
              *mode = AllocError;
              return(NULL);
       }

       addmemory(pool, size);

       if (pci && (long)pci->bits == CID_BITMAP_UNDEFINED)
           glyphs = pci;
       else {
           if (!(glyphs = (CharInfoRec *)xalloc(sizeof(CharInfoRec)))) {
             delmemory();
             xfree(pool);
             *mode = AllocError;
             return(NULL);
           }
           bzero(glyphs, sizeof(CharInfoRec));
       }

       S = (struct XYspace *) t1_Transform((struct xobject *)IDENTITY, 
					   t1, t2, t3, t4);

       S = (struct XYspace *) Permanent(t1_Transform((struct xobject *)S,
						     cid->pixel_matrix[0],
						     -cid->pixel_matrix[1],
						     cid->pixel_matrix[2],
						     -cid->pixel_matrix[3]));

       /* multiplier for computation of raw values */
       sxmult = hypot(cid->pixel_matrix[0], cid->pixel_matrix[1]);
       if (sxmult > EPS) sxmult = 1000.0 / sxmult;

       rc = 0;
       area = (struct region *)CIDfontfcnC(S, charstringP, subarrayP, bluesP, 
					   &len, &rc);
       if (rc < 0 || area == NULL) {
         delmemory();
         xfree(pool);
         if (pci != glyphs) xfree(glyphs);
         *mode = Type1ReturnCodeToXReturnCode(rc);
         return(NULL);
       }

       h       = area->ymax - area->ymin;
       w       = area->xmax - area->xmin;
       paddedW = PAD(w, pad);

       if (h > 0 && w > 0) {
               size = h * paddedW / 8;
               glyphs[0].bits = (char *)xalloc(size);
               if (glyphs[0].bits == NULL) {
                       Destroy(area);
                       delmemory();
                       xfree(pool);
                       if (pci != glyphs) xfree(glyphs);
                       *mode = AllocError;
                       return(NULL);
               }
               bzero(glyphs[0].bits, size);
       }
       else {
               size = 0;
               h = w = 0;
               area->xmin = area->xmax = 0;
               area->ymax = area->ymax = 0;
               glyphs[0].bits = NULL;
       }

       glyphs[0].metrics.leftSideBearing  = area->xmin;
       x1 = (double)(x0 = area->ending.x - area->origin.x);
       y1 = (double)(area->ending.y - area->origin.y);
       glyphs[0].metrics.characterWidth   =
           (x0 + (x0 > 0 ? FPHALF : -FPHALF)) / (1 << FRACTBITS);
       if (!glyphs[0].metrics.characterWidth && size == 0)
       {
           /* Zero size and zero extents: presumably caused by
              the choice of transformation.  Let's create a
              small bitmap so we're not mistaken for an undefined
              character. */
           h = w = 1;
           size = paddedW = PAD(w, pad);
           glyphs[0].bits = (char *)xalloc(size);
           if (glyphs[0].bits == NULL) {
               Destroy(area);
               delmemory();
               xfree(pool);
               if (pci != glyphs) xfree(glyphs);
               *mode = AllocError;
               return(NULL);
           }
           bzero(glyphs[0].bits, size);
       }
       glyphs[0].metrics.attributes =
           NEARESTPEL((long)(hypot(x1, y1) * sxmult));
       glyphs[0].metrics.rightSideBearing = w + area->xmin;
       glyphs[0].metrics.descent          = area->ymax - NEARESTPEL(area->origin.y);
       glyphs[0].metrics.ascent           = h - glyphs[0].metrics.descent;

       if (h > 0 && w > 0)
           fill(glyphs[0].bits, h, paddedW, area, byte, bit, wordsize);
       Destroy(area);
       delmemory();
       xfree(pool);
       *mode = Successful;
       return(glyphs);
}
#endif
