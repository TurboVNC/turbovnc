/* $XConsortium: fontfcn.h,v 1.3 92/03/26 16:42:23 eswu Exp $ */
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
 
extern boolean Init_StdEnc();
extern int scan_font();
extern int GetFontInfo();
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
