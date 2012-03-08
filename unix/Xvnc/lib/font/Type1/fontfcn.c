/* $Xorg: fontfcn.c,v 1.4 2000/08/17 19:46:30 cpqbld Exp $ */
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
/* Author: Katherine A. Hitchcock    IBM Almaden Research Laboratory */
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
/* $XFree86: xc/lib/font/Type1/fontfcn.c,v 1.11 2001/11/23 19:21:31 dawes Exp $ */
 
#ifndef FONTMODULE
#include <stdio.h>
#include <string.h>
#else
#include "Xmd.h"	/* For INT32 declaration */
#include "Xdefs.h"	/* For Bool */
#include "xf86_ansic.h"
#endif
#include "t1imager.h"
#include "util.h"
#ifdef BUILDCID
#include "range.h"
#include "Xdefs.h"
#endif
#include "fntfilst.h"
#include "fontfcn.h"

extern struct segment *Type1Char ( char *env, XYspace S, 
				   psobj *charstrP, psobj *subrsP, 
				   psobj *osubrsP, 
				   struct blues_struct *bluesP, int *modeP );

#ifdef BUILDCID
extern struct xobject *CIDChar ( char *env, XYspace S, 
				 psobj *charstrP, psobj *subrsP, 
				 psobj *osubrsP, 
				 struct blues_struct *bluesP, int *modeP );
static boolean initCIDFont( int cnt );
#endif

/***================================================================***/
/*   GLOBALS                                                          */
/***================================================================***/
char CurFontName[120];
char *CurFontEnv;
char *vm_base = NULL;
psfont *FontP = NULL;
psfont TheCurrentFont;
#ifdef BUILDCID
char CurCIDFontName[CID_PATH_MAX];
char CurCMapName[CID_PATH_MAX];
cidfont *CIDFontP = NULL;
cmapres *CMapP = NULL;
cidfont TheCurrentCIDFont;
cmapres TheCurrentCMap;
psfont *FDArrayP = NULL;
int FDArrayIndex = 0;
#endif
 
/***================================================================***/
/*   SearchDict - look for  name                                      */
/*              - compare for match on len and string                 */
/*                return 0 - not found.                               */
/*                return n - nth element in dictionary.               */
/***================================================================***/
int 
SearchDictName(psdict *dictP, psobj *keyP)
{
  int i,n;
 
 
  n =  dictP[0].key.len;
  for (i=1;i<=n;i++) {          /* scan the intire dictionary */
    if (
        (dictP[i].key.len  == keyP->len )
        &&
        (strncmp(dictP[i].key.data.valueP,
                 keyP->data.valueP,
                 keyP->len) == 0
        )
       ) return(i);
  }
  return(0);
}

#ifdef BUILDCID
static boolean 
initCIDFont(int cnt)
{
  if (!(vm_init(cnt))) return(FALSE);
  vm_base = vm_next_byte();
  strcpy(CurCIDFontName, "");    /* initialize to none */
  strcpy(CurCMapName, "");    /* initialize to none */
  /* cause a font data reset on the next Type 1 font */
  strcpy(CurFontName, "");    /* initialize to none */
  CIDFontP = &TheCurrentCIDFont;
  CMapP = &TheCurrentCMap;
  CIDFontP->vm_start = vm_next_byte();
  CIDFontP->spacerangecnt = 0;
  CIDFontP->notdefrangecnt = 0;
  CIDFontP->cidrangecnt = 0;
  CIDFontP->spacerangeP = NULL;
  CIDFontP->notdefrangeP = NULL;
  CIDFontP->cidrangeP = NULL;
  CIDFontP->CIDFontFileName.len = 0;
  CIDFontP->CIDFontFileName.data.valueP = CurCIDFontName;
  CMapP->CMapFileName.len = 0;
  CMapP->CMapFileName.data.valueP = CurCMapName;
  CMapP->firstRow = 0xFFFF;
  CMapP->firstCol = 0xFFFF;
  CMapP->lastRow = 0;
  CMapP->lastCol = 0;
  return(TRUE);
}

/***================================================================***/
boolean 
initCIDType1Font(void)
{
  strcpy(CurFontName, "");    /* initialize to none */
  FontP = &FDArrayP[FDArrayIndex];
  FontP->vm_start = vm_next_byte();
  FontP->FontFileName.len = 0;
  FontP->FontFileName.data.valueP = CurFontName;
  FontP->Subrs.len = 0;
  FontP->Subrs.data.stringP = NULL;
  FontP->CharStringsP = NULL;
  FontP->Private = NULL;
  FontP->fontInfoP = NULL;
  FontP->BluesP = NULL;
  return(TRUE);
}
#endif

boolean 
initFont(int cnt)
{

  if (!(vm_init(cnt))) return(FALSE);
  vm_base = vm_next_byte();
  if (!(Init_BuiltInEncoding())) return(FALSE);
  strcpy(CurFontName, "");    /* iniitialize to none */
#ifdef BUILDCID
  /* cause a font data reset on the next CID-keyed font */
  strcpy(CurCIDFontName, "");    /* initialize to none */
#endif
  FontP = &TheCurrentFont;
  FontP->vm_start = vm_next_byte();
  FontP->FontFileName.len = 0;
  FontP->FontFileName.data.valueP = CurFontName;
  return(TRUE);
}
/***================================================================***/
#ifdef BUILDCID
static void 
resetCIDFont(char *cidfontname, char *cmapfile)
{

  vm_next =  CIDFontP->vm_start;
  vm_free = vm_size - ( vm_next - vm_base);
  CIDFontP->spacerangecnt = 0;
  CIDFontP->notdefrangecnt = 0;
  CIDFontP->cidrangecnt = 0;
  CIDFontP->spacerangeP = NULL;
  CIDFontP->notdefrangeP = NULL;
  CIDFontP->cidrangeP = NULL;
  CIDFontP->CIDfontInfoP = NULL;
  /* This will load the font into the FontP */
  strcpy(CurCIDFontName,cidfontname);
  strcpy(CurCMapName,cmapfile);
  CIDFontP->CIDFontFileName.len = strlen(CurCIDFontName);
  CIDFontP->CIDFontFileName.data.valueP = CurCIDFontName;
  CMapP->CMapFileName.len = strlen(CurCMapName);
  CMapP->CMapFileName.data.valueP = CurCMapName;
  CMapP->firstRow = 0xFFFF;
  CMapP->firstCol = 0xFFFF;
  CMapP->lastRow = 0;
  CMapP->lastCol = 0;
}

static void 
resetCIDType1Font(void)
{

  vm_next =  FontP->vm_start;
  vm_free = vm_size - ( vm_next - vm_base);
  FontP->Subrs.len = 0;
  FontP->Subrs.data.stringP = NULL;
  FontP->CharStringsP = NULL;
  FontP->Private = NULL;
  FontP->fontInfoP = NULL;
  FontP->BluesP = NULL;
  /* This will load the font into the FontP */
  FontP->FontFileName.len = strlen(CurFontName);
  FontP->FontFileName.data.valueP = CurFontName;
}
#endif

static void 
resetFont(char *env)
{
 
  vm_next =  FontP->vm_start;
  vm_free = vm_size - ( vm_next - vm_base);
  FontP->Subrs.len = 0;
  FontP->Subrs.data.stringP = NULL;
  FontP->CharStringsP = NULL;
  FontP->Private = NULL;
  FontP->fontInfoP = NULL;
  FontP->BluesP = NULL;
  /* This will load the font into the FontP */
  strcpy(CurFontName,env);
  FontP->FontFileName.len = strlen(CurFontName);
  FontP->FontFileName.data.valueP = CurFontName;
 
}

#ifdef BUILDCID
/***================================================================***/
int 
readCIDFont(char *cidfontname, char *cmapfile)
{
  int rcode;

  /* restore the virtual memory and eliminate old font */
  resetCIDFont(cidfontname, cmapfile);
  /* This will load the font into the FontP */
  rcode = scan_cidfont(CIDFontP, CMapP);
  if (rcode == SCAN_OUT_OF_MEMORY) {
    /* free the memory and start again */
    if (!(initCIDFont(vm_size * 2))) {
      /* we are really out of memory */
      return(SCAN_OUT_OF_MEMORY);
    }
    resetCIDFont(cidfontname, cmapfile);
    rcode = scan_cidfont(CIDFontP, CMapP);
    /* only double the memory twice, then report error */
    if (rcode == SCAN_OUT_OF_MEMORY) {
      /* free the memory and start again */
      if (!(initCIDFont(vm_size * 2))) {
        /* we are really out of memory */
        return(SCAN_OUT_OF_MEMORY);
      }
      resetCIDFont(cidfontname, cmapfile);
      rcode = scan_cidfont(CIDFontP, CMapP);
    }
  }
  return(rcode);
}

int 
readCIDType1Font(void)
{
  int rcode;

  resetCIDType1Font();

  /* This will load the font into the FontP */
  rcode = scan_cidtype1font(FontP);
  return(rcode);
}
#endif

int 
readFont(char *env)
{
  int rcode;
 
  /* restore the virtual memory and eliminate old font */
  resetFont(env);
  /* This will load the font into the FontP */
  rcode = scan_font(FontP);
  if (rcode == SCAN_OUT_OF_MEMORY) {
    /* free the memory and start again */
#ifdef BUILDCID
    /* xfree(vm_base); */
#else
    xfree(vm_base);
#endif
    if (!(initFont(vm_size * 2))) {
      /* we are really out of memory */
      return(SCAN_OUT_OF_MEMORY);
      }
    resetFont(env);
    rcode = scan_font(FontP);
#ifdef BUILDCID
    /* only double the memory twice, then report error */
    if (rcode == SCAN_OUT_OF_MEMORY) {
      /* free the memory and start again */
      /* xfree(vm_base) */
      if (!(initFont(vm_size * 2))) {
        /* we are really out of memory */
        return(SCAN_OUT_OF_MEMORY);
      }
      resetFont(env);
      rcode = scan_font(FontP);
    }
#else
    /* only double the memory once, then report error */
#endif
  }
  return(rcode);
}
/***================================================================***/
struct xobject *
fontfcnB(struct XYspace *S, unsigned char *code, int *lenP, int *mode)
{
  psobj *charnameP; /* points to psobj that is name of character*/
  int   N;
  psdict *CharStringsDictP; /* dictionary with char strings     */
  psobj   CodeName;   /* used to store the translation of the name*/
  psobj  *SubrsArrayP;
  psobj  *theStringP;
 
  struct xobject *charpath;   /* the path for this character              */
 
  charnameP = &CodeName;
  charnameP->len = *lenP;
  charnameP->data.stringP = code;

  CharStringsDictP =  FontP->CharStringsP;
 
  /* search the chars string for this charname as key */
  N = SearchDictName(CharStringsDictP,charnameP);
  if (N<=0) {
    *mode = FF_PARSE_ERROR;
    return(NULL);
  }
  /* ok, the nth item is the psobj that is the string for this char */
  theStringP = &(CharStringsDictP[N].value);
 
  /* get the dictionary pointers to the Subrs  */
 
  SubrsArrayP = &(FontP->Subrs);
  /* scale the Adobe fonts to 1 unit high */
  /* call the type 1 routine to rasterize the character     */
  charpath = (struct xobject *)Type1Char((char *)FontP,S,theStringP,
					 SubrsArrayP,NULL,
               FontP->BluesP , mode);
  /* if Type1Char reported an error, then return */
  if ( *mode == FF_PARSE_ERROR)  return(NULL);
  /* fill with winding rule unless path was requested */
  if (*mode != FF_PATH) {
    charpath =  (struct xobject *)Interior((struct segment *)charpath,
					   WINDINGRULE+CONTINUITY);
  }
  return(charpath);
}

#ifdef BUILDCID
/***================================================================***/
/*   CIDfontfcnA(cidfontname, cmapfile, mode)                         */
/*                                                                    */
/*     1) initialize the font     - global indicates it has been done */
/*     2) load the font                                               */
/***================================================================***/
Bool 
CIDfontfcnA(char *cidfontname, char *cmapfile, int *mode)
{
  int rcode, cidinit;

  cidinit = 0;
  if (CIDFontP == NULL || strcmp(CurCIDFontName, "") == 0) {
    InitImager();
    if (!(initCIDFont(VM_SIZE))) {
      /* we are really out of memory */
      *mode = SCAN_OUT_OF_MEMORY;
      return(FALSE);
    }
    cidinit = 1;
  }

  /* if the cidfontname is null, then use font already loaded */

  /* if not the same font name */
  if (cidinit || (cidfontname && strcmp(cidfontname,CurCIDFontName) != 0) ||
      (cmapfile && strcmp(cmapfile,CurCMapName) != 0)) {
    /* restore the virtual memory and eliminate old font, read new one */
    rcode = readCIDFont(cidfontname, cmapfile);
    if (rcode != 0 ) {
      strcpy(CurCIDFontName, "");    /* no CIDFont loaded */
      strcpy(CurCMapName, "");    /* no CMap loaded */
      *mode = rcode;
      return(FALSE);
    }
  }
  return(TRUE);

}

/***================================================================***/
/*   CIDType1fontfcnA(mode)                                           */
/*                                                                    */
/*     1) initialize the font     - global indicates it has been done */
/*     2) load the font                                               */
/***================================================================***/
Bool 
CIDType1fontfcnA(int *mode)
{
  int rcode;

  if (!(initCIDType1Font())) {
    /* we are really out of memory */
    *mode = SCAN_OUT_OF_MEMORY;
    return(FALSE);
  }

  if ((rcode = readCIDType1Font()) != 0) {
    strcpy(CurFontName, "");    /* no font loaded */
    *mode = rcode;
    return(FALSE);
  }
  return(TRUE);

}
#endif

/***================================================================***/
/*   fontfcnA(env, mode)                                              */
/*                                                                    */
/*          env is a pointer to a string that contains the fontname.  */
/*                                                                    */
/*     1) initialize the font     - global indicates it has been done */
/*     2) load the font                                               */
/***================================================================***/
Bool 
fontfcnA(char *env, int *mode)
{
  int rc;
 
  /* Has the FontP initialized?  If not, then   */
  /* Initialize  */
#ifdef BUILDCID
  if (FontP == NULL || strcmp(CurFontName, "") == 0) {
#else
  if (FontP == NULL) {
#endif
    InitImager();
    if (!(initFont(VM_SIZE))) {
      /* we are really out of memory */
      *mode = SCAN_OUT_OF_MEMORY;
      return(FALSE);
    }
  }
 
  /* if the env is null, then use font already loaded */
 
  /* if the not same font name */
  if ( (env) && (strcmp(env,CurFontName) != 0 ) ) {
    /* restore the virtual memory and eliminate old font, read new one */
    rc = readFont(env);
    if (rc != 0 ) {
      strcpy(CurFontName, "");    /* no font loaded */
      *mode = rc;
      return(FALSE);
    }
  }
  return(TRUE);
 
}

#ifdef BUILDCID
/***================================================================***/
/*   CIDQueryFontLib(cidfontname,cmapfile,infoName,infoValue,rcodeP)  */
/*                                                                    */
/*   cidfontname is a pointer to a string that contains the fontname. */
/*                                                                    */
/*     1) initialize the font     - global indicates it has been done */
/*     2) load the font                                               */
/*     3) use the font to call getInfo for that value.                */
/***================================================================***/

void 
CIDQueryFontLib(char *cidfontname, char *cmapfile, char *infoName,
		pointer infoValue, /* parameter returned here    */
		int *rcodeP)
{
  int rc,N,i,cidinit;
  psdict *dictP;
  psobj  nameObj;
  psobj  *valueP;

  /* Has the CIDFontP initialized?  If not, then   */
  /* Initialize  */
  cidinit = 0;
  if (CIDFontP == NULL || strcmp(CurCIDFontName, "") == 0) {
    InitImager();
    if (!(initCIDFont(VM_SIZE))) {
      *rcodeP = 1;
      return;
    }
    cidinit = 1;
  }
  /* if the file name is null, then use font already loaded */
  /* if the not same font name, reset and load next font */
  if (cidinit || (cidfontname && strcmp(cidfontname,CurCIDFontName) != 0) ||
    (cmapfile && strcmp(cmapfile,CurCMapName) != 0)) {
    /* restore the virtual memory and eliminate old font */
    rc = readCIDFont(cidfontname, cmapfile);
    if (rc != 0 ) {
      strcpy(CurCIDFontName, "");    /* no font loaded */
      strcpy(CurCMapName, "");    /* no font loaded */
      *rcodeP = 1;
      return;
    }
  }
  dictP = CIDFontP->CIDfontInfoP;
  objFormatName(&nameObj,strlen(infoName),infoName);
  N = SearchDictName(dictP,&nameObj);
  /* if found */
  if ( N > 0 ) {
    *rcodeP = 0;
    switch (dictP[N].value.type) {
       case OBJ_ARRAY:
         valueP = dictP[N].value.data.arrayP;
         /* Just double check valueP. H.J. */
         if (valueP == NULL) break;
         if (strcmp(infoName,"FontMatrix") == 0) {
           /* 6 elments, return them as floats      */
           for (i=0;i<6;i++) {
             if (valueP->type == OBJ_INTEGER )
               ((float *)infoValue)[i] = valueP->data.integer;
             else
               ((float *)infoValue)[i] = valueP->data.real;
            valueP++;
           }
         }
         if (strcmp(infoName,"FontBBox") == 0) {
           /* 4 elments for Bounding Box.  all integers   */
           for (i=0;i<4;i++) {
             ((int *)infoValue)[i] = valueP->data.integer;
             valueP++;
           }
         break;
       case OBJ_INTEGER:
       case OBJ_BOOLEAN:
         *((int *)infoValue) = dictP[N].value.data.integer;
         break;
       case OBJ_REAL:
         *((float *)infoValue) = dictP[N].value.data.real;
         break;
       case OBJ_NAME:
       case OBJ_STRING:
         *((char **)infoValue) =  dictP[N].value.data.valueP;
         break;
       default:
         *rcodeP = 1;
         break;
     }
   }
  }
  else *rcodeP = 1;
}
#endif

/***================================================================***/
/*   QueryFontLib(env, infoName,infoValue,rcodeP)                     */
/*                                                                    */
/*          env is a pointer to a string that contains the fontname.  */
/*                                                                    */
/*     1) initialize the font     - global indicates it has been done */
/*     2) load the font                                               */
/*     3) use the font to call getInfo for that value.                */
/***================================================================***/

void 
QueryFontLib(char *env, char *infoName,
	     pointer infoValue, /* parameter returned here    */
	     int *rcodeP)
{
  int rc,N,i;
  psdict *dictP;
  psobj  nameObj;
  psobj  *valueP;
 
  /* Has the FontP initialized?  If not, then   */
  /* Initialize  */
  if (FontP == NULL) {
    InitImager();
    if (!(initFont(VM_SIZE))) {
      *rcodeP = 1;
      return;
    }
  }
  /* if the env is null, then use font already loaded */
  /* if the not same font name, reset and load next font */
  if ( (env) && (strcmp(env,CurFontName) != 0 ) ) {
    /* restore the virtual memory and eliminate old font */
    rc = readFont(env);
    if (rc != 0 ) {
      strcpy(CurFontName, "");    /* no font loaded */
      *rcodeP = 1;
      return;
    }
  }
  dictP = FontP->fontInfoP;
  objFormatName(&nameObj,strlen(infoName),infoName);
  N = SearchDictName(dictP,&nameObj);
  /* if found */
  if ( N > 0 ) {
    *rcodeP = 0;
    switch (dictP[N].value.type) {
       case OBJ_ARRAY:
         valueP = dictP[N].value.data.arrayP;
	 /* Just double check valueP. H.J. */
	 if (valueP == NULL) break;
         if (strcmp(infoName,"FontMatrix") == 0) {
           /* 6 elments, return them as floats      */
           for (i=0;i<6;i++) {
             if (valueP->type == OBJ_INTEGER )
               ((float *)infoValue)[i] = valueP->data.integer;
             else
               ((float *)infoValue)[i] = valueP->data.real;
            valueP++;
           }
         }
         if (strcmp(infoName,"FontBBox") == 0) {
           /* 4 elments for Bounding Box.  all integers   */
           for (i=0;i<4;i++) {
             ((int *)infoValue)[i] = valueP->data.integer;
             valueP++;
           }
         break;
       case OBJ_INTEGER:
       case OBJ_BOOLEAN:
         *((int *)infoValue) = dictP[N].value.data.integer;
         break;
       case OBJ_REAL:
         *((float *)infoValue) = dictP[N].value.data.real;
         break;
       case OBJ_NAME:
       case OBJ_STRING:
         *((char **)infoValue) =  dictP[N].value.data.valueP;
         break;
       default:
         *rcodeP = 1;
         break;
     }
   }
  }
  else *rcodeP = 1;
}

#ifdef BUILDCID
struct xobject *
CIDfontfcnC(struct XYspace *S, psobj *theStringP, 
	    psobj *SubrsArrayP, struct blues_struct *BluesP,
	    int *lenP, int *mode)
{
  struct xobject *charpath;   /* the path for this character              */

  charpath = (struct xobject *)CIDChar((char *)FontP,S,theStringP,
				       SubrsArrayP,NULL,BluesP,mode);
  /* if Type1Char reported an error, then return */
  if ( *mode == FF_PARSE_ERROR)  return(NULL);
  /* fill with winding rule unless path was requested */
  if (*mode != FF_PATH) {
    charpath = (struct xobject *)Interior((struct segment *)charpath,
					  WINDINGRULE+CONTINUITY);
  }
  return(charpath);
}
#endif
