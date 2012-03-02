/* $Xorg: fontfcn.h,v 1.3 2000/08/17 19:46:30 cpqbld Exp $ */
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
/* $XFree86: xc/lib/font/Type1/fontfcn.h,v 1.5 2001/01/17 19:43:22 dawes Exp $ */


#ifdef BUILDCID
/* Definition of a PostScript CIDFont resource */
typedef struct cid_font {
            char               *vm_start;
            int                spacerangecnt;
            int                notdefrangecnt;
            int                cidrangecnt;
            spacerange         *spacerangeP;
            cidrange           *notdefrangeP;
            cidrange           *cidrangeP;
            int                binarydata; /* 1=binary data, 0=hex data */
            long               bytecnt;
            psobj              CIDFontFileName;
            psdict             *CIDfontInfoP;
} cidfont;

/* Definition of a PostScript CMap resource */
typedef struct cmap_res {
            unsigned short firstCol;
            unsigned short lastCol;
            unsigned short firstRow;
            unsigned short lastRow;
            psobj   CMapFileName;
            psdict  *CMapInfoP;
} cmapres;
#endif

/*     Definition of a PostScript FONT             */
typedef struct ps_font {
            char    *vm_start;
            psobj   FontFileName;
            psobj   Subrs;
            psdict  *CharStringsP;
            psdict  *Private;
            psdict  *fontInfoP;
struct blues_struct *BluesP;
} psfont;
/***================================================================***/
/*  Routines in scan_font                                             */
/***================================================================***/

extern boolean Init_BuiltInEncoding ( void );
#ifdef BUILDCID
extern int scan_cidfont ( cidfont *CIDFontP, cmapres *CMapP );
extern int scan_cidtype1font ( psfont *FontP );
#endif
extern int scan_font ( psfont *FontP );
/***================================================================***/
/*  Return codes from scan_font                                       */
/***================================================================***/
#define SCAN_OK               0
#define SCAN_FILE_EOF        -1
#define SCAN_ERROR           -2
#define SCAN_OUT_OF_MEMORY   -3
#define SCAN_FILE_OPEN_ERROR -4
#define SCAN_TRUE            -5
#define SCAN_FALSE           -6
#define SCAN_END             -7

#ifdef BUILDCID
/***================================================================***/
/*  Name of CID FontInfo fields                                       */
/***================================================================***/
#define CIDCOUNT 1
#define CIDFONTNAME 2
#define CIDFONTTYPE 3
#define CIDVERSION 4
#define CIDREGISTRY 5
#define CIDORDERING 6
#define CIDSUPPLEMENT 7
#define CIDMAPOFFSET 8
#define CIDFDARRAY 9
#define CIDFDBYTES 10
#define CIDFONTBBOX 11
#define CIDFULLNAME 12
#define CIDFAMILYNAME 13
#define CIDWEIGHT 14
#define CIDNOTICE 15
#define CIDGDBYTES 16
#define CIDUIDBASE 17
#define CIDXUID 18

/***================================================================***/
/*  Name of CMapInfo fields                                           */
/***================================================================***/
#define CMAPREGISTRY 1
#define CMAPORDERING 2
#define CMAPSUPPLEMENT 3
#define CMAPNAME 4
#define CMAPVERSION 5
#define CMAPTYPE 6
#define CMAPWMODE 7
#define CMAPCIDCOUNT 8
#endif

/***================================================================***/
/*  Name of FontInfo fields                                           */
/***================================================================***/
 
#define FONTNAME 1
#define PAINTTYPE 2
#define FONTTYPENUM 3
#define FONTMATRIX 4
#define FONTBBOX   5
#define UNIQUEID  6
#define STROKEWIDTH  7
#define VERSION     8
#define NOTICE     9
#define FULLNAME 10
#define FAMILYNAME 11
#define WEIGHT 12
#define ITALICANGLE 13
#define ISFIXEDPITCH  14
#define UNDERLINEPOSITION 15
#define UNDERLINETHICKNESS 16
#define ENCODING 17
/***================================================================***/
/*  Name of Private values                                            */
/***================================================================***/
#define BLUEVALUES 1
#define OTHERBLUES 2
#define FAMILYBLUES 3
#define FAMILYOTHERBLUES 4
#define BLUESCALE 5
#define BLUESHIFT 6
#define BLUEFUZZ  7
#define STDHW     8
#define STDVW     9
#define STEMSNAPH 10
#define STEMSNAPV 11
#define FORCEBOLD 12
#define LANGUAGEGROUP 13
#define LENIV     14
#define RNDSTEMUP 15
#define EXPANSIONFACTOR 16

#ifdef BUILDCID
/***================================================================***/
/*  Name of CID Type 1 Private values                                 */
/***================================================================***/
#define CIDT1MINFEATURE     1
#define CIDT1LENIV          2
#define CIDT1LANGGROUP      3
#define CIDT1BLUEVALUES     4
#define CIDT1OTHERBLUES     5
#define CIDT1BLUESCALE      6
#define CIDT1BLUEFUZZ       7
#define CIDT1BLUESHIFT      8
#define CIDT1FAMBLUES       9
#define CIDT1FAMOTHERBLUES 10
#define CIDT1STDHW         11
#define CIDT1STDVW         12
#define CIDT1STEMSNAPH     13
#define CIDT1STEMSNAPV     14
#define CIDT1SUBMAPOFF     15
#define CIDT1SDBYTES       16
#define CIDT1SUBRCNT       17
#define CIDT1FORCEBOLD     18
#define CIDT1RNDSTEMUP     19
#define CIDT1EXPFACTOR     20

#define CID_BITMAP_UNDEFINED       0
extern int SearchDictName ( psdict *dictP, psobj *keyP );
#ifdef BUILDCID
extern boolean initCIDType1Font ( void );
#endif
extern boolean initFont ( int cnt );
#ifdef BUILDCID
extern int readCIDFont ( char *cidfontname, char *cmapfile );
extern int readCIDType1Font ( void );
#endif
extern int readFont ( char *env );
extern struct xobject *fontfcnB ( struct XYspace *S, unsigned char *code, 
				  int *lenP, int *mode );
#ifdef BUILDCID
extern Bool CIDfontfcnA ( char *cidfontname, char *cmapfile, int *mode );
extern Bool CIDType1fontfcnA ( int *mode );
#endif
extern Bool fontfcnA ( char *env, int *mode );
#ifdef BUILDCID
extern void CIDQueryFontLib ( char *cidfontname, char *cmapfile, 
			      char *infoName, pointer infoValue, int *rcodeP );
#endif
extern void QueryFontLib ( char *env, char *infoName, pointer infoValue, 
			   int *rcodeP );
#ifdef BUILDCID
extern struct xobject *CIDfontfcnC ( struct XYspace *S, psobj *theStringP, 
				     psobj *SubrsArrayP,
				     struct blues_struct *BluesP, int *lenP, 
				     int *mode );
#endif
#endif
