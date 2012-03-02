/* $Xorg: t1info.c,v 1.4 2001/02/09 02:04:01 xorgcvs Exp $ */
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
 * Author: Carol H. Thompson  IBM Almaden Research Center
 *   Modeled on spinfo.c by Dave Lemke, Network Computing Devices, Inc
 *   which contains the following copyright and permission notices:
 *
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices or Digital 
 * not be used in advertising or publicity pertaining to distribution of the 
 * software without specific, written prior permission. Network Computing 
 * Devices and Digital make no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * NETWORK COMPUTING DEVICES AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES OR DIGITAL BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*

Copyright 1987, 1998  The Open Group

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
/* $XFree86: xc/lib/font/Type1/t1info.c,v 1.19 2003/05/27 22:26:47 tsi Exp $ */

#include "fntfilst.h"
#include "fontutil.h"
#ifndef FONTMODULE
#include <stdio.h> 
#ifndef BUILDCID
#include <math.h>
#endif
#else
#include "xf86_ansic.h"
#endif
#include "FSproto.h"

#ifdef BUILDCID
#ifndef FONTMODULE
#ifdef _XOPEN_SOURCE
#include <math.h>
#else
#define _XOPEN_SOURCE
#include <math.h>
#undef _XOPEN_SOURCE
#endif
#endif
#include "objects.h"
#include "spaces.h"
#include "range.h"
#endif

#ifdef BUILDCID
#include "util.h"
#include "fontfcn.h"

#if defined(HAVE_CFM) || defined(CID_ALL_CHARS)
#ifndef DEFAULT_CFM_DIR
#define DEFAULT_CFM_DIR "./"
#endif
char cfmDefaultDir[] = DEFAULT_CFM_DIR;
#define CFMMAGIC 0x91239123
#endif
#endif
#include "t1intf.h"

#define DECIPOINTSPERINCH 722.7
#define DEFAULTRES 75
#define DEFAULTPOINTSIZE 120
 
enum scaleType {
    atom, truncate_atom, pixel_size, point_size, resolution_x,
    resolution_y, average_width
};

#ifdef BUILDCID
extern cidfont *CIDFontP;
static int stdpropsinit = 0;

typedef struct cfm_rec {
    xCharInfo   maxbounds;
    xCharInfo   minbounds;
    xCharInfo   ink_maxbounds;
    xCharInfo   ink_minbounds;
    INT32       totalrw;
    INT16       maxo;
    INT16       alle;
} cfmrec;
#endif

typedef struct _fontProp {
    char       *name;
    long        atom;
    enum scaleType type;
}           fontProp;
 
static fontProp fontNamePropTable[] = {  /* Example: */
    { "FOUNDRY", 0, atom },			/* adobe */
    { "FAMILY_NAME", 0, atom },			/* times roman */
    { "WEIGHT_NAME", 0, atom },			/* bold */
    { "SLANT", 0, atom },			/* i */
    { "SETWIDTH_NAME", 0, atom },		/* normal */
    { "ADD_STYLE_NAME", 0, atom },		/* */
    { "PIXEL_SIZE", 0, pixel_size },		/* 18 */
    { "POINT_SIZE", 0, point_size },		/* 180 */
    { "RESOLUTION_X", 0, resolution_x },	/* 72 */
    { "RESOLUTION_Y", 0, resolution_y },	/* 72 */
    { "SPACING", 0, atom },			/* p */
    { "AVERAGE_WIDTH", 0, average_width },	/* 0 */
    { "CHARSET_REGISTRY", 0, atom },		/* ISO8859 */
    { "CHARSET_ENCODING", 0, truncate_atom }	/* 1 */
};
 
/* NOTICE: Following array is closely related to the sequence of defines
   following it. */
static fontProp extraProps[] = {
    { "FONT", 0, },
    { "COPYRIGHT", 0, },
    { "RAW_PIXEL_SIZE", 0, },
    { "RAW_POINT_SIZE", 0, },
    { "RAW_ASCENT", 0, },
    { "RAW_DESCENT", 0, },
    { "RAW_AVERAGE_WIDTH", 0, },
    { "FACE_NAME", 0, },
    { "FONT_TYPE", 0, },
    { "RASTERIZER_NAME", 0, }
};
 
/* this is a bit kludgy */
#define FONTPROP        0
#define COPYRIGHTPROP   1
#define RAWPIXELPROP	2
#define RAWPOINTPROP	3
#define RAWASCENTPROP	4
#define RAWDESCENTPROP	5
#define RAWWIDTHPROP	6
#define FACE_NAMEPROP	7
#define FONT_TYPEPROP   8
#define RASTERIZER_NAMEPROP 9

#define NNAMEPROPS (sizeof(fontNamePropTable) / sizeof(fontProp))
#define NEXTRAPROPS (sizeof(extraProps) / sizeof(fontProp))
 
#define NPROPS  (NNAMEPROPS + NEXTRAPROPS)
 
/*ARGSUSED*/
static void
FillHeader(FontInfoPtr pInfo, FontScalablePtr Vals)
{
    /* OpenScalable in T1FUNCS sets the following:
    pInfo->firstCol,
    pInfo->firstRow,
    pInfo->lastCol, and
    pInfo->lastRow. */
    /* the following are ununsed
    pInfo->pad. */
 
    /* Items we should handle better someday +++ */
    pInfo->defaultCh = 0;
    pInfo->drawDirection = LeftToRight;
    if (Vals->point_matrix[0] == Vals->point_matrix[3])
	pInfo->anamorphic = 0;
    else
	pInfo->anamorphic = 1;
    pInfo->inkMetrics = 0;  /* no ink metrics here */
    pInfo->cachable = 1;    /* no licensing (yet) */
}
 
static void
adjust_min_max(xCharInfo *minc, xCharInfo *maxc, xCharInfo *tmp)
{
#define MINMAX(field,ci) \
        if (minc->field > (ci)->field) \
             minc->field = (ci)->field; \
        if (maxc->field < (ci)->field) \
            maxc->field = (ci)->field;
 
    MINMAX(ascent, tmp);
    MINMAX(descent, tmp);
    MINMAX(leftSideBearing, tmp);
    MINMAX(rightSideBearing, tmp);
    MINMAX(characterWidth, tmp);

    /* Do MINMAX for attributes field.  Since that field is CARD16,
       we'll cast to a signed integer */
    if ((INT16)minc->attributes > (INT16)tmp->attributes)
         minc->attributes = tmp->attributes;
    if ((INT16)maxc->attributes < (INT16)tmp->attributes)
        maxc->attributes = tmp->attributes;
 
#undef  MINMAX
}
 
static void
ComputeBounds(FontInfoPtr pInfo, CharInfoPtr pChars, FontScalablePtr Vals)
{
    int i;
    xCharInfo minchar, maxchar;
    int numchars = 0;
    int totchars;
    int overlap;
    int maxlap;
 
    minchar.ascent = minchar.descent =
        minchar.leftSideBearing = minchar.rightSideBearing =
        minchar.characterWidth = minchar.attributes = 32767;
    maxchar.ascent = maxchar.descent =
        maxchar.leftSideBearing = maxchar.rightSideBearing =
        maxchar.characterWidth = maxchar.attributes = -32767;
 
    maxlap = -32767;
    totchars = pInfo->lastCol - pInfo->firstCol + 1;
    pChars += pInfo->firstCol;
    pInfo->allExist = 1;
    for (i = 0; i < totchars; i++,pChars++) {
        xCharInfo *pmetrics = &pChars->metrics;
 
        if (pmetrics->attributes ||
	    pmetrics->ascent != -pmetrics->descent ||
	    pmetrics->leftSideBearing != pmetrics->rightSideBearing) {
            numchars++;
            adjust_min_max(&minchar, &maxchar, pmetrics);
            overlap = pmetrics->rightSideBearing - pmetrics->characterWidth;
            if (overlap > maxlap) maxlap = overlap;
        }
        else pInfo->allExist = 0;
    }

    /* If we're monospaced, round the average width field to the
       nearest pixel */
    if (minchar.characterWidth == maxchar.characterWidth)
	Vals->width = minchar.characterWidth * 10;
 
    pInfo->maxbounds = maxchar;
    pInfo->minbounds = minchar;
    pInfo->ink_maxbounds = maxchar;
    pInfo->ink_minbounds = minchar;
    pInfo->maxOverlap = maxlap + -(minchar.leftSideBearing);
 
    /* Set the pInfo flags */
    /* Properties set by FontComputeInfoAccelerators:
        pInfo->noOverlap;
        pInfo->terminalFont;
        pInfo->constantMetrics;
        pInfo->constantWidth;
        pInfo->inkInside;
 
    */
    FontComputeInfoAccelerators (pInfo);
}

#ifdef BUILDCID
#ifdef CID_ALL_CHARS
void
ComputeBoundsAllChars(FontPtr pFont, char *cfmfilename, double sxmult)
{
    FILE *cfm;
    CARD32 magic;
    int count = 0;
    int maxlap, overlap, i, j, k, ret;
    xCharInfo minchar, maxchar;
    cidrange *cidrangeP;
    unsigned char ccode[2];
    unsigned long ccount;
    xCharInfo *pmetrics;
    long total_raw_width = 0, total_width = 0;
    char cfmd[CID_PATH_MAX];
    cfmrec *cfmp;
    char *p;

    if (!(cfm = fopen(cfmfilename, "w"))) {
        fprintf(stderr,
    "Unable to open the file %s. You are probably not logged in as root.\n",
            cfmfilename);
        p = strrchr(cfmfilename, '/');
        if (p == NULL) exit(1);
        strcpy(cfmd, cfmDefaultDir);
        strcat(cfmd, p + 1);
        if (!(cfm = fopen(cfmd, "w"))) {
            fprintf(stderr,
            "Switching to current directory. Unable to open the file %s.\n",
            cfmd);
            exit(1);
        }
    }

    if ((cfmp = (cfmrec *)xalloc(sizeof(cfmrec))) == NULL) {
        fprintf(stderr, "Unable to allocate memory.");
        exit(1);
    }
    bzero(cfmp, sizeof(cfmrec));

    minchar.ascent = minchar.descent =
        minchar.leftSideBearing = minchar.rightSideBearing =
        minchar.characterWidth = minchar.attributes = 32767;
    maxchar.ascent = maxchar.descent =
        maxchar.leftSideBearing = maxchar.rightSideBearing =
        maxchar.characterWidth = maxchar.attributes = -32767;

    maxlap = -32767;
    cfmp->alle = 1;
    cidrangeP = CIDFontP->cidrangeP;

    /* go through all character codes specified in a given CMap */
    for (i = 0; i < CIDFontP->cidrangecnt; i++) {
      for (j = 0; j < cidrangeP->rangecnt; j++) {
        for (k = cidrangeP->range[j].srcCodeLo;
          k <= cidrangeP->range[j].srcCodeHi; k++) {
          ccode[0] = (k >> 8) & 0xff;
          ccode[1] = k & 0xff;
          ret = CIDGetMetrics(pFont, 1, ccode, Linear16Bit, &ccount, &pmetrics);
          if (ret != Successful || (ret == Successful && pmetrics == NULL))
              continue;
          total_width += pmetrics->attributes;
          total_raw_width += abs((int)(INT16)pmetrics->attributes);
          if (pmetrics->attributes ||
              pmetrics->ascent != -pmetrics->descent ||
              pmetrics->leftSideBearing != pmetrics->rightSideBearing) {
              count++;
              adjust_min_max(&minchar, &maxchar, pmetrics);
              overlap = pmetrics->rightSideBearing - pmetrics->characterWidth;
             if (overlap > maxlap) maxlap = overlap;
           }
           else cfmp->alle = 0;
        }
      }
    }

    if (count)
    {
        total_raw_width = (total_raw_width * 10 + count / 2) / count;
        if (total_width < 0)
        {
            /* Predominant direction is R->L */
            total_raw_width = -total_raw_width;
        }
    }

    cfmp->totalrw = (INT32)total_raw_width;

    cfmp->maxbounds.leftSideBearing =
        floor((double)maxchar.leftSideBearing * sxmult + 0.5);
    cfmp->maxbounds.rightSideBearing =
        floor((double)maxchar.rightSideBearing * sxmult + 0.5);
    cfmp->maxbounds.characterWidth =
        floor((double)maxchar.characterWidth * sxmult + 0.5);
    cfmp->maxbounds.ascent =
        floor((double)maxchar.ascent * sxmult + 0.5);
    cfmp->maxbounds.descent =
        floor((double)maxchar.descent * sxmult);
    cfmp->maxbounds.attributes = maxchar.attributes;

    cfmp->minbounds.leftSideBearing =
        floor((double)minchar.leftSideBearing * sxmult + 0.5);
    cfmp->minbounds.rightSideBearing =
        floor((double)minchar.rightSideBearing * sxmult + 0.5);
    cfmp->minbounds.characterWidth =
        floor((double)minchar.characterWidth * sxmult + 0.5);
    cfmp->minbounds.ascent =
        floor((double)minchar.ascent * sxmult + 0.5);
    cfmp->minbounds.descent =
        floor((double)minchar.descent * sxmult + 0.5);
    cfmp->minbounds.attributes = minchar.attributes;

    cfmp->ink_maxbounds.leftSideBearing =
        floor((double)maxchar.leftSideBearing * sxmult + 0.5);
    cfmp->ink_maxbounds.rightSideBearing =
        floor((double)maxchar.rightSideBearing * sxmult + 0.5);
    cfmp->ink_maxbounds.characterWidth =
        floor((double)maxchar.characterWidth * sxmult + 0.5);
    cfmp->ink_maxbounds.ascent =
        floor((double)maxchar.ascent * sxmult + 0.5);
    cfmp->ink_maxbounds.descent =
        floor((double)maxchar.descent * sxmult + 0.5);
    cfmp->ink_maxbounds.attributes = maxchar.attributes;

    cfmp->ink_minbounds.leftSideBearing =
        floor((double)minchar.leftSideBearing * sxmult + 0.5);
    cfmp->ink_minbounds.rightSideBearing =
        floor((double)minchar.rightSideBearing * sxmult + 0.5);
    cfmp->ink_minbounds.characterWidth =
        floor((double)minchar.characterWidth * sxmult + 0.5);
    cfmp->ink_minbounds.ascent =
        floor((double)minchar.ascent * sxmult + 0.5);
    cfmp->ink_minbounds.descent =
        floor((double)minchar.descent * sxmult + 0.5);
    cfmp->ink_minbounds.attributes = minchar.attributes;

    cfmp->maxo = (INT32)(maxlap + -(minchar.leftSideBearing));

    magic = CFMMAGIC;
    fwrite(&magic, sizeof(CARD32), 1, cfm);
    fwrite(cfmp, sizeof(cfmrec), 1, cfm);
    xfree(cfmp);
    fclose(cfm);
}
#else
static long
ComputeBoundsAll(FontPtr pFont)
{
    int count = 0;
    int maxlap, overlap, i, j, k, ret;
    xCharInfo minchar, maxchar;
    cidrange *cidrangeP;
    unsigned char ccode[2];
    unsigned long ccount;
    xCharInfo *pmetrics;
    CharInfoRec *cinfo[1];
    long total_raw_width = 0, total_width = 0;
    FontInfoPtr pInfo = &(pFont->info);

    minchar.ascent = minchar.descent =
        minchar.leftSideBearing = minchar.rightSideBearing =
        minchar.characterWidth = minchar.attributes = 32767;
    maxchar.ascent = maxchar.descent =
        maxchar.leftSideBearing = maxchar.rightSideBearing =
        maxchar.characterWidth = maxchar.attributes = -32767;

    maxlap = -32767;
    pInfo->allExist = 1;
    cidrangeP = CIDFontP->cidrangeP;

    /* go through all character codes specified in a given CMap */
    for (i = 0; i < CIDFontP->cidrangecnt; i++) {
      for (j = 0; j < cidrangeP->rangecnt; j++) {
        for (k = cidrangeP->range[j].srcCodeLo;
          k <= cidrangeP->range[j].srcCodeHi; k++) {
          ccode[0] = (k >> 8) & 0xff;
          ccode[1] = k & 0xff;
          ret = CIDGetMetrics(pFont, 1, ccode, Linear16Bit, &ccount, (xCharInfo **)cinfo);
          if (ret != Successful || cinfo == NULL)
              continue;
          pmetrics = &cinfo[0]->metrics;
          total_width += pmetrics->attributes;
          total_raw_width += abs((int)(INT16)pmetrics->attributes);
          if (pmetrics->attributes ||
              pmetrics->ascent != -pmetrics->descent ||
              pmetrics->leftSideBearing != pmetrics->rightSideBearing) {
              count++;
              adjust_min_max(&minchar, &maxchar, pmetrics);
              overlap = pmetrics->rightSideBearing - pmetrics->characterWidth;
             if (overlap > maxlap) maxlap = overlap;
           }
           else pInfo->allExist = 0;
        }
      }
    }

    if (count)
    {
        total_raw_width = (total_raw_width * 10 + count / 2) / count;
        if (total_width < 0)
        {
            /* Predominant direction is R->L */
            total_raw_width = -total_raw_width;
        }
    }

    pInfo->maxbounds.leftSideBearing = maxchar.leftSideBearing;
    pInfo->maxbounds.rightSideBearing = maxchar.rightSideBearing;
    pInfo->maxbounds.characterWidth = maxchar.characterWidth;
    pInfo->maxbounds.ascent = maxchar.ascent;
    pInfo->maxbounds.descent = maxchar.descent;
    pInfo->maxbounds.attributes = maxchar.attributes;

    pInfo->minbounds.leftSideBearing = minchar.leftSideBearing;
    pInfo->minbounds.rightSideBearing = minchar.rightSideBearing;
    pInfo->minbounds.characterWidth = minchar.characterWidth;
    pInfo->minbounds.ascent = minchar.ascent;
    pInfo->minbounds.descent = minchar.descent;
    pInfo->minbounds.attributes = minchar.attributes;

    pInfo->ink_maxbounds.leftSideBearing = maxchar.leftSideBearing;
    pInfo->ink_maxbounds.rightSideBearing = maxchar.rightSideBearing;
    pInfo->ink_maxbounds.characterWidth = maxchar.characterWidth;
    pInfo->ink_maxbounds.ascent = maxchar.ascent;
    pInfo->ink_maxbounds.descent = maxchar.descent;
    pInfo->ink_maxbounds.attributes = maxchar.attributes;

    pInfo->ink_minbounds.leftSideBearing = minchar.leftSideBearing;
    pInfo->ink_minbounds.rightSideBearing = minchar.rightSideBearing;
    pInfo->ink_minbounds.characterWidth = minchar.characterWidth;
    pInfo->ink_minbounds.ascent = minchar.ascent;
    pInfo->ink_minbounds.descent = minchar.descent;
    pInfo->ink_minbounds.attributes = minchar.attributes;

    pInfo->maxOverlap = maxlap + -(minchar.leftSideBearing);

    return total_raw_width;
}
#endif
#endif

static void
ComputeProps(FontInfoPtr pInfo, FontScalablePtr Vals, char *Filename, 
	     long *sAscent, long *sDescent)
{
    int infoint;
    int infoBBox[4];
    int rc;
 
    QueryFontLib(Filename, "isFixedPitch", &infoint, &rc);
    if (!rc) {
        pInfo->constantWidth = infoint;
    }
    QueryFontLib((char *)0, "FontBBox", infoBBox, &rc);
    if (!rc) {
	pInfo->fontAscent =
	    (int)((double)infoBBox[3] * Vals->pixel_matrix[3] +
		  (infoBBox[3] > 0 ? 500 : -500)) / 1000;
	pInfo->fontDescent =
	    -(int)((double)infoBBox[1] * Vals->pixel_matrix[3] +
		   (infoBBox[1] > 0 ? 500 : -500)) / 1000;
	*sAscent = infoBBox[3];
	*sDescent = -infoBBox[1];
    }
}

#ifdef BUILDCID
#ifndef CID_ALL_CHARS
static void
CIDComputeStdProps(FontInfoPtr pInfo, FontScalablePtr Vals, 
		   char *Filename, char *Cmapname, char *Fontname, 
		   long sAscent, long sDescent, long sWidth)
{
    FontPropPtr pp;
    int         i,
                nprops;
    fontProp   *fpt;
    char       *is_str;
    char       *ptr1 = NULL,
               *ptr2;
    char       *ptr3;
    char *infostrP;
    int rc;
    char      scaledName[CID_PATH_MAX];

    strcpy (scaledName, Fontname);
    /* Fill in our copy of the fontname from the Vals structure */
    FontParseXLFDName (scaledName, Vals, FONT_XLFD_REPLACE_VALUE);

    /* This form of the properties is used by the X-client; the X-server
       doesn't care what they are. */
    nprops = pInfo->nprops = NPROPS;
    pInfo->isStringProp = (char *) xalloc(sizeof(char) * nprops);
    pInfo->props = (FontPropPtr) xalloc(sizeof(FontPropRec) * nprops);
    if (!pInfo->isStringProp || !pInfo->props) {
        xfree(pInfo->isStringProp);
        pInfo->isStringProp = (char *) 0;
        xfree(pInfo->props);
        pInfo->props = (FontPropPtr) 0;
        pInfo->nprops = 0;
        return;
    }
    bzero(pInfo->isStringProp, (sizeof(char) * nprops));

    ptr2 = scaledName;
    for (i = NNAMEPROPS, pp = pInfo->props, fpt = fontNamePropTable, is_str = pInfo->isStringProp;
            i;
            i--, pp++, fpt++, is_str++) {

        if (*ptr2)
        {
            ptr1 = ptr2 + 1;
            if (!(ptr2 = strchr(ptr1, '-'))) ptr2 = strchr(ptr1, '\0');
        }

        pp->name = fpt->atom;
        switch (fpt->type) {
         case atom:  /* Just copy info from scaledName */
            *is_str = TRUE;
            pp->value = MakeAtom(ptr1, ptr2 - ptr1, TRUE);
            break;
        case truncate_atom:
            *is_str = TRUE;
            for (ptr3 = ptr1; *ptr3; ptr3++)
                if (*ptr3 == '[')
                    break;
            pp->value = MakeAtom(ptr1, ptr3 - ptr1, TRUE);
            break;
         case pixel_size:
            pp->value = (int)(fabs(Vals->pixel_matrix[3]) + .5);
            break;
         case point_size:
            pp->value = (int)(fabs(Vals->point_matrix[3]) * 10.0 + .5);
            break;
         case resolution_x:
            pp->value = Vals->x;
            break;
         case resolution_y:
            pp->value = Vals->y;
            break;
         case average_width:
            pp->value = Vals->width;
            break;
        }
    }

    for (i = 0, fpt = extraProps;
          i < NEXTRAPROPS;
          i++, is_str++, pp++, fpt++) {
        pp->name = fpt->atom;
        switch (i) {
         case FONTPROP:
            *is_str = TRUE;
            pp->value = MakeAtom(scaledName, strlen(scaledName), TRUE);
            break;
         case COPYRIGHTPROP:
            *is_str = TRUE;
            CIDQueryFontLib(Filename, Cmapname, "Notice", &infostrP, &rc);
            if (rc || !infostrP) {
                infostrP = "Copyright Notice not available";
            }
            pp->value = MakeAtom(infostrP, strlen(infostrP), TRUE);
            break;
         case FACE_NAMEPROP:
            *is_str = TRUE;
            CIDQueryFontLib(Filename, Cmapname, "CIDFontName", &infostrP, &rc);
            if (rc || !infostrP) {
                infostrP = "(unknown)";
            }
            pp->value = MakeAtom(infostrP, strlen(infostrP), TRUE);
            break;
         case FONT_TYPEPROP:
            *is_str = TRUE;
            infostrP = "CIDFont";
            pp->value = MakeAtom(infostrP, strlen(infostrP), TRUE);
            break;
         case RASTERIZER_NAMEPROP:
            *is_str = TRUE;
            infostrP = "X Consortium Type 1 Rasterizer";
            pp->value = MakeAtom(infostrP, strlen(infostrP), TRUE);
            break;
         case RAWPIXELPROP:
            *is_str = FALSE;
            pp->value = 1000;
            break;
         case RAWPOINTPROP:
            *is_str = FALSE;
            pp->value = (long)(72270.0 / (double)Vals->y + .5);
            break;
         case RAWASCENTPROP:
            *is_str = FALSE;
            pp->value = sAscent;
            break;
         case RAWDESCENTPROP:
            *is_str = FALSE;
            pp->value = sDescent;
            break;
         case RAWWIDTHPROP:
            *is_str = FALSE;
            pp->value = sWidth;
            break;
        }
    }
}
#endif
#endif
 
static void
ComputeStdProps(FontInfoPtr pInfo, FontScalablePtr Vals, 
		char *Filename, char *Fontname, 
		long sAscent, long sDescent, long sWidth)
{
    FontPropPtr pp;
    int         i,
                nprops;
    fontProp   *fpt;
    char       *is_str;
    char       *ptr1 = NULL,
               *ptr2;
    char       *ptr3;
    char *infostrP;
    int rc;
    char      scaledName[MAXFONTNAMELEN];
 
    strcpy (scaledName, Fontname);
    /* Fill in our copy of the fontname from the Vals structure */
    FontParseXLFDName (scaledName, Vals, FONT_XLFD_REPLACE_VALUE);
 
    /* This form of the properties is used by the X-client; the X-server
       doesn't care what they are. */
    nprops = pInfo->nprops = NPROPS;
    pInfo->isStringProp = (char *) xalloc(sizeof(char) * nprops);
    pInfo->props = (FontPropPtr) xalloc(sizeof(FontPropRec) * nprops);
    if (!pInfo->isStringProp || !pInfo->props) {
        xfree(pInfo->isStringProp);
        pInfo->isStringProp = (char *) 0;
        xfree(pInfo->props);
        pInfo->props = (FontPropPtr) 0;
        return;
    }
    bzero(pInfo->isStringProp, (sizeof(char) * nprops));
 
    ptr2 = scaledName;
    for (i = NNAMEPROPS, pp = pInfo->props, fpt = fontNamePropTable, is_str = pInfo->isStringProp;
            i;
            i--, pp++, fpt++, is_str++) {

	if (*ptr2)
	{
	    ptr1 = ptr2 + 1;
	    if (!(ptr2 = strchr(ptr1, '-'))) ptr2 = strchr(ptr1, '\0');
	}

        pp->name = fpt->atom;
        switch (fpt->type) {
         case atom:  /* Just copy info from scaledName */
            *is_str = TRUE;
            pp->value = MakeAtom(ptr1, ptr2 - ptr1, TRUE);
            break;
	case truncate_atom:
            *is_str = TRUE;
	    for (ptr3 = ptr1; *ptr3; ptr3++)
		if (*ptr3 == '[')
		    break;
	    pp->value = MakeAtom(ptr1, ptr3 - ptr1, TRUE);
	    break;
         case pixel_size:
            pp->value = (int)(fabs(Vals->pixel_matrix[3]) + .5);
            break;
         case point_size:
            pp->value = (int)(fabs(Vals->point_matrix[3]) * 10.0 + .5);
            break;
         case resolution_x:
            pp->value = Vals->x;
            break;
         case resolution_y:
            pp->value = Vals->y;
            break;
         case average_width:
            pp->value = Vals->width;
            break;
        }
    }
 
    for (i = 0, fpt = extraProps;
          i < NEXTRAPROPS;
          i++, is_str++, pp++, fpt++) {
        pp->name = fpt->atom;
        switch (i) {
         case FONTPROP:
            *is_str = TRUE;
            pp->value = MakeAtom(scaledName, strlen(scaledName), TRUE);
            break;
         case COPYRIGHTPROP:
            *is_str = TRUE;
            QueryFontLib(Filename, "Notice", &infostrP, &rc);
            if (rc || !infostrP) {
                infostrP = "Copyright Notice not available";
            }
            pp->value = MakeAtom(infostrP, strlen(infostrP), TRUE);
            break;
         case FACE_NAMEPROP:
            *is_str = TRUE;
            QueryFontLib(Filename, "FontName", &infostrP, &rc);
            if (rc || !infostrP) {
                infostrP = "(unknown)"; 
            }
            pp->value = MakeAtom(infostrP, strlen(infostrP), TRUE);
            break;
         case FONT_TYPEPROP:
            *is_str = TRUE;
            infostrP = "Type 1";
            pp->value = MakeAtom(infostrP, strlen(infostrP), TRUE);
            break;
         case RASTERIZER_NAMEPROP:
            *is_str = TRUE;
            infostrP = "X Consortium Type 1 Rasterizer";
            pp->value = MakeAtom(infostrP, strlen(infostrP), TRUE);
            break;
         case RAWPIXELPROP:
            *is_str = FALSE;
            pp->value = 1000;
	    break;
         case RAWPOINTPROP:
            *is_str = FALSE;
            pp->value = (long)(72270.0 / (double)Vals->y + .5);
	    break;
         case RAWASCENTPROP:
            *is_str = FALSE;
            pp->value = sAscent;
	    break;
         case RAWDESCENTPROP:
            *is_str = FALSE;
            pp->value = sDescent;
	    break;
         case RAWWIDTHPROP:
            *is_str = FALSE;
            pp->value = sWidth;
	    break;
        }
    }
}

#ifdef BUILDCID
/*ARGSUSED*/
int
CIDGetInfoScalable(FontPathElementPtr fpe, 
		   FontInfoPtr pInfo, 
		   FontEntryPtr entry, 
		   FontNamePtr fontName, 
		   char *fileName, 
		   FontScalablePtr Vals)
{
    FontPtr pfont;
    int flags = 0;
    long format = 0;  /* It doesn't matter what format for just info */
    long fmask = 0;
    int ret;

    ret = CIDOpenScalable(fpe, &pfont, flags, entry, fileName, Vals, 
			  format, fmask, NULL);
    if (ret != Successful)
        return ret;
    *pInfo = pfont->info;

    /* XXX - Set pointers in pfont->info to NULL so they are not freed. */
    pfont->info.props = NULL;
    pfont->info.isStringProp = NULL;

    CIDCloseFont(pfont);
    return Successful;
}
#endif

/*ARGSUSED*/
int
Type1GetInfoScalable(FontPathElementPtr fpe, 
		     FontInfoPtr pInfo, 
		     FontEntryPtr entry, 
		     FontNamePtr fontName, 
		     char *fileName, 
		     FontScalablePtr Vals)
{
    FontPtr pfont;
    int flags = 0;
    long format = 0;  /* It doesn't matter what format for just info */
    long fmask = 0;
    int ret;
 
    ret = Type1OpenScalable(fpe, &pfont, flags, entry, fileName, Vals, 
			    format, fmask , NULL);
    if (ret != Successful)
	return ret;
    *pInfo = pfont->info;

    /* XXX - Set pointers in pfont->info to NULL so they are not freed. */
    pfont->info.props = NULL;
    pfont->info.isStringProp = NULL;

    Type1CloseFont(pfont);
    return Successful;
}

#ifdef BUILDCID
#ifndef CID_ALL_CHARS
void
CIDFillFontInfo(FontPtr pFont, FontScalablePtr Vals, 
		char *Filename, char *Fontname, char *Cmapname, 
#ifdef HAVE_CFM
		char *cfmfilename, 
#endif
		long sAscent, long sDescent, double sxmult)
{
#ifdef HAVE_CFM
    FILE *cfm;
    cfmrec *cfmp;
    int gotcfm = 0;
    CARD32 magic;
#endif
    long sWidth = 0;
    FontInfoPtr         pInfo = &pFont->info;

    FillHeader(pInfo, Vals);

#ifdef HAVE_CFM
    if ((cfm = fopen(cfmfilename,"r"))) {
        fread(&magic,sizeof(CARD32),1,cfm);
        if(magic == CFMMAGIC) {
            if ((cfmp = (cfmrec *)xalloc(sizeof(cfmrec))) != NULL) {
                fread(cfmp,sizeof(cfmrec),1,cfm);
                sWidth = (long)cfmp->totalrw;
                pInfo->allExist = cfmp->alle;
                if (sxmult != 0) {
                    pInfo->maxbounds.leftSideBearing =
                        floor((double)cfmp->maxbounds.leftSideBearing /
                            sxmult + 0.5);
                    pInfo->maxbounds.rightSideBearing =
                        floor((double)cfmp->maxbounds.rightSideBearing /
                            sxmult + 0.5);
                    pInfo->maxbounds.characterWidth =
                        floor((double)cfmp->maxbounds.characterWidth /
                            sxmult + 0.5);
                    pInfo->maxbounds.ascent =
                        floor((double)cfmp->maxbounds.ascent /
                            sxmult + 0.5);
                    pInfo->maxbounds.descent =
                        floor((double)cfmp->maxbounds.descent /
                            sxmult + 0.5);
                    pInfo->maxbounds.attributes =
                        cfmp->maxbounds.attributes;

                    pInfo->minbounds.leftSideBearing =
                        cfmp->minbounds.leftSideBearing / sxmult;
                    pInfo->minbounds.rightSideBearing =
                        cfmp->minbounds.rightSideBearing / sxmult;
                    pInfo->minbounds.characterWidth =
                        cfmp->minbounds.characterWidth / sxmult;
                    pInfo->minbounds.ascent =
                        cfmp->minbounds.ascent / sxmult;
                    pInfo->minbounds.descent =
                        cfmp->minbounds.descent / sxmult;
                    pInfo->minbounds.attributes = cfmp->minbounds.attributes;

                    pInfo->ink_maxbounds.leftSideBearing =
                        cfmp->ink_maxbounds.leftSideBearing / sxmult;
                    pInfo->ink_maxbounds.rightSideBearing =
                        cfmp->ink_maxbounds.rightSideBearing / sxmult;
                    pInfo->ink_maxbounds.characterWidth =
                        cfmp->ink_maxbounds.characterWidth / sxmult;
                    pInfo->ink_maxbounds.ascent =
                        cfmp->ink_maxbounds.ascent / sxmult;
                    pInfo->ink_maxbounds.descent =
                        cfmp->ink_maxbounds.descent / sxmult;
                    pInfo->ink_maxbounds.attributes =
                        cfmp->ink_maxbounds.attributes;

                    pInfo->ink_minbounds.leftSideBearing =
                        cfmp->ink_minbounds.leftSideBearing / sxmult;
                    pInfo->ink_minbounds.rightSideBearing =
                        cfmp->ink_minbounds.rightSideBearing / sxmult;
                    pInfo->ink_minbounds.characterWidth =
                        cfmp->ink_minbounds.characterWidth / sxmult;
                    pInfo->ink_minbounds.ascent =
                        cfmp->ink_minbounds.ascent / sxmult;
                    pInfo->ink_minbounds.descent =
                        cfmp->ink_minbounds.descent / sxmult;
                    pInfo->ink_minbounds.attributes =
                        cfmp->ink_minbounds.attributes;
                    pInfo->ink_minbounds.attributes =
                        cfmp->ink_minbounds.attributes;

                    pInfo->maxOverlap = (short)cfmp->maxo;

                    gotcfm = 1;
                }
                xfree(cfmp);
            }
        }
        fclose(cfm);
    }

    if (!gotcfm)
#endif
        sWidth = ComputeBoundsAll(pFont);

    FontComputeInfoAccelerators(pInfo);

    CIDComputeStdProps(pInfo, Vals, Filename, Cmapname, Fontname, sAscent,
        sDescent, sWidth);
}
#endif /* CID_ALL_CHARS */
#endif /* BUILDCID */

void
T1FillFontInfo(FontPtr pFont, FontScalablePtr Vals, 
	       char *Filename, char *Fontname, long sWidth)
{
    FontInfoPtr         pInfo = &pFont->info;
    struct type1font *p = (struct type1font *)pFont->fontPrivate;
    long sAscent, sDescent;	/* Scalable 1000-pixel values */
 
    FillHeader(pInfo, Vals);
 
    ComputeBounds(pInfo, p->glyphs, Vals);
 
    ComputeProps(pInfo, Vals, Filename, &sAscent, &sDescent);
    ComputeStdProps(pInfo, Vals, Filename, Fontname, sAscent, sDescent, sWidth);
}
 
/* Called once, at renderer registration time */
void
#ifdef BUILDCID
Type1InitStdProps(void)
#else
T1InitStdProps(void)
#endif
{
    int         i;
    fontProp   *t;
 
#ifdef BUILDCID
    if (!stdpropsinit) {
        stdpropsinit = 1;
        i = sizeof(fontNamePropTable) / sizeof(fontProp);
        for (t = fontNamePropTable; i; i--, t++)
            t->atom = MakeAtom(t->name, (unsigned) strlen(t->name), TRUE);
        i = sizeof(extraProps) / sizeof(fontProp);
        for (t = extraProps; i; i--, t++)
            t->atom = MakeAtom(t->name, (unsigned) strlen(t->name), TRUE);
    }
#else
    i = sizeof(fontNamePropTable) / sizeof(fontProp);
    for (t = fontNamePropTable; i; i--, t++)
        t->atom = MakeAtom(t->name, (unsigned) strlen(t->name), TRUE);
    i = sizeof(extraProps) / sizeof(fontProp);
    for (t = extraProps; i; i--, t++)
        t->atom = MakeAtom(t->name, (unsigned) strlen(t->name), TRUE);
#endif
}
