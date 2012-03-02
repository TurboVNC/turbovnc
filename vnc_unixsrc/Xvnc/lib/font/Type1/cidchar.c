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
/* $XFree86: xc/lib/font/Type1/cidchar.c,v 1.10 2003/05/27 22:26:45 tsi Exp $ */

#ifdef BUILDCID
#ifndef FONTMODULE
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifdef USE_MMAP
#include <sys/mman.h>
#ifndef MAP_FAILED
#define MAP_FAILED ((caddr_t)(-1))
#endif
#endif
#else
#include "Xmd.h"        /* For INT32 declaration */
#include "Xdefs.h"      /* For Bool */
#include "xf86_ansic.h"
#endif
#ifndef FONTMODULE
#ifdef _XOPEN_SOURCE
#include <math.h>
#else
#define _XOPEN_SOURCE
#include <math.h>
#undef _XOPEN_SOURCE
#endif
#endif
#include "fntfilst.h"
#include "objects.h"
#include "spaces.h"
#include "range.h"
#include "util.h"
#include "fontfcn.h"
#include "blues.h"
#include "AFM.h"
#include "t1intf.h"

#define BSIZE 4096

extern cidfont *CIDFontP;
extern psfont *FDArrayP;
extern psfont *FontP;

static unsigned char sd[] = "StartData";

CharInfoPtr
CIDGetGlyphInfo(FontPtr pFont, unsigned int cidcode, CharInfoPtr pci, int *rc)
{
    CharInfoPtr cp = NULL;
#ifdef USE_MMAP
    int   fd;
    unsigned char *buf;
    long total_len = 0;
#else
    FILE *fp;
    unsigned char buf[BSIZE];
    unsigned int count = 0;
#endif
    cidglyphs *cid;
    unsigned char *p1 = NULL;
#ifndef USE_MMAP
    unsigned char *p2;
#endif
    register int i = 0, j;
    long byteoffset;
    int FDindex, FDBytes, GDBytes, SDBytes, SubrCount, CIDMapOffset, len;
    psobj *arrayP;
    psobj charstring;
    long *subroffsets = NULL, cstringoffset, nextcstringoffset;
    struct blues_struct *blues;

    cid = (cidglyphs *)pFont->fontPrivate;

#ifdef USE_MMAP
    if (!cid->CIDdata) {
       if (!(fd = open(cid->CIDFontName, O_RDONLY, 0))) {
           *rc = BadFontName;
           return(cp);
       }
       cid->CIDsize = lseek(fd, 0, SEEK_END);
       cid->CIDdata = (unsigned char *)
           mmap(0, (size_t)cid->CIDsize, PROT_READ, MAP_SHARED, fd, 0);
       close(fd);
       if (cid->CIDdata == (unsigned char *)MAP_FAILED) {
           *rc = AllocError;
           cid->CIDdata = NULL;
           return (cp);
       }
    }
#else
    if (!(fp = fopen(cid->CIDFontName,"rb"))) {
        *rc = BadFontName;
        return(cp);
    }
#endif

#ifdef USE_MMAP
    if (cid->dataoffset == 0) {
       if ((p1 = (unsigned char *)strstr((char *)cid->CIDdata, (char *)sd)) 
           != NULL) {
           cid->dataoffset = (p1 - cid->CIDdata) + strlen((char *)sd);
       }
       else {
           *rc = BadFontFormat;
           return(cp);
       }
    }
#else /* USE_MMAP */
    if (cid->dataoffset == 0) {
        p2 = sd;

        /* find "StartData" */
        while (*p2) {
            cid->dataoffset += count;
            if ((count = fread(buf, 1, BSIZE, fp)) == 0)
                break;
            p1 = buf;
            for (i=0; i < count && *p2; i++) {
                if (*p1 == *p2)
                    p2++;
                else {
                    p2 = sd;
                    if (*p1 == *p2)
                        p2++;
                }
                p1++;
            }
        }

        /* if "StartData" not found, or end of file */
        if (*p2 || count == 0) {
            *rc = BadFontFormat;
            fclose(fp);
            return(cp);
        }

        if (i >= count) {
            cid->dataoffset += count;
            count = fread(buf, 1, BSIZE, fp);
            p1 = buf;
        } else {
            cid->dataoffset += p1 - buf;
            count = count - (p1 - buf);
        }
    } else {
        if (fseek(fp, cid->dataoffset, SEEK_SET)) {
            *rc = BadFontFormat;
            fclose(fp);
            return(cp);
        }
        if ((count = fread(buf, 1, BSIZE, fp)) == 0) {
            *rc = BadFontFormat;
            fclose(fp);
            return(cp);
        }
        p1 = buf;
    }

    /* if "StartData" not found, or "Binary" data and the next character */
    /* is not the space character (0x20)                                 */

    if (count == 0 || (CIDFontP->binarydata && (*p1 != ' '))) {
        *rc = BadFontFormat;
        fclose(fp);
        return(cp);
    }
#endif /* USE_MMAP */

    FDBytes = CIDFontP->CIDfontInfoP[CIDFDBYTES].value.data.integer;
    GDBytes = CIDFontP->CIDfontInfoP[CIDGDBYTES].value.data.integer;
    CIDMapOffset = CIDFontP->CIDfontInfoP[CIDMAPOFFSET].value.data.integer;
    byteoffset = cid->dataoffset + 1 + CIDMapOffset +
        cidcode * (FDBytes + GDBytes);
#ifdef USE_MMAP
    buf = &cid->CIDdata[byteoffset];
#else
    if (fseek(fp, byteoffset, SEEK_SET)) {
        *rc = BadFontFormat;
        fclose(fp);
        return(cp);
    }
    if ((count = fread(buf, 1, BSIZE, fp)) < 2*(FDBytes + GDBytes)) {
        *rc = BadFontFormat;
        fclose(fp);
        return(cp);
    }
#endif

    /* if FDBytes is equal to 0, the CIDMap contains no FD indices, and the */
    /* FD index of 0 is assumed.                                            */
    if (FDBytes == 0)
        FDindex = 0;
    else {
        FDindex = 0;
        for (i = 0; i < FDBytes; i++)
            FDindex += (unsigned char)buf[i] << (8 * (FDBytes - 1 - i));
    }

    if (FDindex >= CIDFontP->CIDfontInfoP[CIDFDARRAY].value.len) {
        *rc = BadFontFormat;
#ifndef USE_MMAP
        fclose(fp);
#endif
        return(cp);
    }

    cstringoffset = 0;
    for (i = 0; i < GDBytes; i++)
        cstringoffset += (unsigned char)buf[FDBytes + i] <<
            (8 * (GDBytes - 1 - i));

    nextcstringoffset = 0;
    for (i = 0; i < GDBytes; i++)
        nextcstringoffset += (unsigned char)buf[2*FDBytes + GDBytes + i] <<
            (8 * (GDBytes - 1 - i));

    len = nextcstringoffset - cstringoffset;

    if (len <= 0) { /* empty interval, missing glyph */
        *rc = BadFontFormat;
#ifndef USE_MMAP
        fclose(fp);
#endif
        return(cp);
    }

    FontP = &FDArrayP[FDindex];

    charstring.type = OBJ_INTEGER;
    charstring.len = len;

#ifndef USE_MMAP
    if (!(charstring.data.stringP = (unsigned char *)xalloc(len))) {
        *rc = AllocError;
        fclose(fp);
        return(cp);
    }
#endif

    byteoffset = cid->dataoffset + 1 + cstringoffset;

#ifdef USE_MMAP
    charstring.data.stringP =  &cid->CIDdata[byteoffset];
#else
    if (fseek(fp, byteoffset, SEEK_SET)) {
        *rc = BadFontFormat;
        xfree(charstring.data.stringP);
        fclose(fp);
        return(cp);
    }

    if ((count = fread(charstring.data.stringP, 1, len, fp)) != len) {
        *rc = BadFontFormat;
        xfree(charstring.data.stringP);
        fclose(fp);
        return(cp);
    }
#endif

    if (FontP->Subrs.data.arrayP == NULL) {
        /* get subroutine data */
        byteoffset = cid->dataoffset + 1 +
            FDArrayP[FDindex].Private[CIDT1SUBMAPOFF].value.data.integer;

        SDBytes = FDArrayP[FDindex].Private[CIDT1SDBYTES].value.data.integer;

        SubrCount = FDArrayP[FDindex].Private[CIDT1SUBRCNT].value.data.integer;
#ifdef USE_MMAP
        buf = &cid->CIDdata[byteoffset];
#else
        if (fseek(fp, byteoffset, SEEK_SET)) {
            *rc = BadFontFormat;
            fclose(fp);
            return(cp);
        }

        if ((count = fread(buf, 1, BSIZE, fp)) < SDBytes * (SubrCount + 1)) {
            *rc = BadFontFormat;
            fclose(fp);
            return(cp);
        }
#endif

        arrayP = (psobj *)vm_alloc(SubrCount*sizeof(psobj));
        if (!arrayP) {
            *rc = AllocError;
#ifndef USE_MMAP
            fclose(fp);
#endif
            return(cp);
        }  

        if (!(subroffsets = (long *)xalloc((SubrCount + 1)*sizeof(long)))) {
            *rc = AllocError;
#ifndef USE_MMAP
            fclose(fp);
#endif
            return(cp);
        }

        for (i = 0; i <= SubrCount; i++) {
            subroffsets[i] = 0;
            for (j = 0; j < SDBytes; j++)
                subroffsets[i] += (unsigned char)buf[i * SDBytes + j] <<
                    (8 * (SDBytes - 1 - j));
        }

        byteoffset = cid->dataoffset + 1 + subroffsets[0];

        /* get subroutine info */
#ifndef USE_MMAP
        if (fseek(fp, byteoffset, SEEK_SET)) {
            *rc = BadFontFormat;
            xfree(subroffsets);
            fclose(fp);
            return(cp);
        }
#else
        total_len = byteoffset;
#endif
        for (i = 0; i < SubrCount; i++) {
            len = subroffsets[i + 1] - subroffsets[i];
#ifndef USE_MMAP
            arrayP[i].data.valueP = vm_alloc(len);
            if (!arrayP[i].data.valueP) {
                *rc = AllocError;
                xfree(subroffsets);
                fclose(fp);
                return(cp);
            }
#endif
            arrayP[i].len = len;
#ifdef USE_MMAP
            arrayP[i].data.valueP = (char *)&cid->CIDdata[total_len];
            total_len += len;
#else
            if ((count = fread(arrayP[i].data.valueP, 1, len, fp)) != len) {
                *rc = BadFontFormat;
                xfree(subroffsets);
                fclose(fp);
                return(cp);
            }
#endif
        }

        FontP->Subrs.len = SubrCount;
        FontP->Subrs.data.arrayP =  arrayP;
        xfree(subroffsets);
    }

    if (FontP->BluesP == NULL) {
        blues = (struct blues_struct *) vm_alloc(sizeof(struct blues_struct));
        if (!blues) {
            *rc = AllocError;
#ifndef USE_MMAP
            xfree(subroffsets);
            fclose(fp);
#endif
            return(cp);
        }
        bzero(blues, sizeof(struct blues_struct));
        blues->numBlueValues =
            FDArrayP[FDindex].Private[CIDT1BLUEVALUES].value.len;
        for (i = 0; i < blues->numBlueValues; i++)
            blues->BlueValues[i] =
                FDArrayP[FDindex].Private[CIDT1BLUEVALUES].value.data.arrayP[i].data.integer;
        blues->numOtherBlues =
            FDArrayP[FDindex].Private[CIDT1OTHERBLUES].value.len;
        for (i = 0; i < blues->numOtherBlues; i++)
            blues->OtherBlues[i] =
                FDArrayP[FDindex].Private[CIDT1OTHERBLUES].value.data.arrayP[i].data.integer;
        blues->numFamilyBlues =
            FDArrayP[FDindex].Private[CIDT1FAMBLUES].value.len;
        for (i = 0; i < blues->numFamilyBlues; i++)
            blues->FamilyBlues[i] =
                FDArrayP[FDindex].Private[CIDT1FAMBLUES].value.data.arrayP[i].data.integer;
        blues->numFamilyOtherBlues =
            FDArrayP[FDindex].Private[CIDT1FAMOTHERBLUES].value.len;
        for (i = 0; i < blues->numFamilyOtherBlues; i++)
            blues->FamilyOtherBlues[i] =
                FDArrayP[FDindex].Private[CIDT1FAMOTHERBLUES].value.data.arrayP[i].data.integer;
        blues->BlueScale = FDArrayP[FDindex].Private[CIDT1BLUESCALE].value.data.real;
        blues->BlueShift = FDArrayP[FDindex].Private[CIDT1BLUESHIFT].value.data.integer;
        blues->BlueFuzz = FDArrayP[FDindex].Private[CIDT1BLUEFUZZ].value.data.integer;
        blues->StdHW = (double)FDArrayP[FDindex].Private[CIDT1STDHW].value.data.arrayP[0].data.integer;
        blues->StdVW = (double)FDArrayP[FDindex].Private[CIDT1STDVW].value.data.arrayP[0].data.integer;

        blues->numStemSnapH =
            FDArrayP[FDindex].Private[CIDT1STEMSNAPH].value.len;
        for (i = 0; i < blues->numStemSnapH; i++)
            blues->StemSnapH[i] =
                FDArrayP[FDindex].Private[CIDT1STEMSNAPH].value.data.arrayP[i].data.integer;
        blues->numStemSnapV =
            FDArrayP[FDindex].Private[CIDT1STEMSNAPV].value.len;
        for (i = 0; i < blues->numStemSnapV; i++)
            blues->StemSnapV[i] =
                FDArrayP[FDindex].Private[CIDT1STEMSNAPV].value.data.arrayP[i].data.integer;
        blues->ForceBold =
            FDArrayP[FDindex].Private[CIDT1FORCEBOLD].value.data.boolean;

        blues->LanguageGroup =
            FDArrayP[FDindex].Private[CIDT1LANGGROUP].value.data.integer;

        blues->RndStemUp =
            FDArrayP[FDindex].Private[CIDT1RNDSTEMUP].value.data.boolean;

        blues->lenIV =
            FDArrayP[FDindex].Private[CIDT1LENIV].value.data.integer;

        blues->ExpansionFactor =
            FDArrayP[FDindex].Private[CIDT1EXPFACTOR].value.data.real;

        FontP->BluesP = blues;
    }

    cp = CIDRenderGlyph(pFont, &charstring, &FontP->Subrs, FontP->BluesP, pci, rc);

#ifndef USE_MMAP
    xfree(charstring.data.stringP);

    fclose(fp);
#endif
    return(cp);
}

static int 
node_compare(const void *node1, const void *node2)
{
   return (((Metrics *)node1)->code - ((Metrics *)node2)->code);
}

static CharInfoRec *
CIDGetCharMetrics(FontPtr pFont, FontInfo *fi, unsigned int charcode, double sxmult)
{
    CharInfoPtr cp;
    Metrics *p, node;
    unsigned int cidcode;

    cidcode = node.code = getCID(pFont, charcode);
    if ((cidcode < fi->nChars) && (cidcode == fi->metrics[cidcode].code))
        p = &fi->metrics[cidcode];
    else
        p = (Metrics *)bsearch(&node, fi->metrics, fi->nChars, sizeof(Metrics), node_compare);

    if (!p)
        p = &fi->metrics[0];

    if (!(cp = (CharInfoRec *)Xalloc(sizeof(CharInfoRec))))
        return NULL;
    bzero(cp, sizeof(CharInfoRec));

    /* indicate that character bitmap is not defined */
    cp->bits = (char *)CID_BITMAP_UNDEFINED;


    /* get metric data for this CID code from the CID AFM file */
    cp->metrics.leftSideBearing =
        floor(p->charBBox.llx / sxmult + 0.5);
    cp->metrics.rightSideBearing =
        floor(p->charBBox.urx / sxmult + 0.5);
    cp->metrics.characterWidth = floor(p->wx / sxmult + 0.5);
    cp->metrics.ascent = floor(p->charBBox.ury / sxmult + 0.5);
    cp->metrics.descent = -floor(p->charBBox.lly / sxmult + 0.5);

    cp->metrics.attributes = p->wx;

    return cp;
}

int 
CIDGetAFM(FontPtr pFont, unsigned long count, unsigned char *chars, FontEncoding charEncoding, unsigned long *glyphCount, CharInfoPtr *glyphs, char *cidafmfile)
{
    FILE *fp;
    FontInfo *fi = NULL;
    cidglyphs *cid;
    CharInfoPtr *glyphsBase;
    register unsigned int c;

    register CharInfoPtr pci;
    CharInfoPtr pDefault;
    unsigned int firstCol, code, char_row, char_col;
    double sxmult;

    cid = (cidglyphs *)pFont->fontPrivate;

    if (cid->AFMinfo == NULL) {
        if (!(fp = fopen(cidafmfile, "rb")))
            return(BadFontName);

        if (CIDAFM(fp, &fi) != 0) {
            fprintf(stderr,
                "There is something wrong with Adobe Font Metric file %s.\n",
                cidafmfile);
            fclose(fp);
            return(BadFontName);
        }
        fclose(fp);
        cid->AFMinfo = fi;
    }
    fi = cid->AFMinfo;

    firstCol   = pFont->info.firstCol;
    pDefault   = cid->pDefault;
    glyphsBase = glyphs;

    /* multiplier for computation of raw values */
    sxmult = hypot(cid->pixel_matrix[0], cid->pixel_matrix[1]);
    if (sxmult > EPS) sxmult = 1000.0 / sxmult;
    if (sxmult == 0.0) return(0);

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
                if (!(pci = (CharInfoRec *)cid->glyphs[code]))
                    pci = CIDGetCharMetrics(pFont, fi, c, sxmult);
                if (pci && EXIST(pci)) {
                    *glyphs++ = pci;
                    cid->glyphs[code] = pci;
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
                if (!(pci = (CharInfoRec *)cid->glyphs[code]))
                    pci = CIDGetCharMetrics(pFont, fi, c, sxmult);
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
                if (!(pci = (CharInfoRec *)cid->glyphs[code]))
                    pci = CIDGetCharMetrics(pFont, fi, c, sxmult);
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

#undef EXIST

    return Successful;

}
#endif
