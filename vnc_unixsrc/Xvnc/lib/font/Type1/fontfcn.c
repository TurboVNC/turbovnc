/* $XConsortium: fontfcn.c,v 1.10 94/02/06 16:07:23 gildea Exp $ */
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
 
#include <stdio.h>
#include <string.h>
#include "t1imager.h"
#include "util.h"
#include "fontfcn.h"
#include "fontmisc.h"
 
extern xobject Type1Char();
/***================================================================***/
/*   GLOBALS                                                          */
/***================================================================***/
char CurFontName[120];
char *CurFontEnv;
char *vm_base = NULL;
psfont *FontP = NULL;
psfont TheCurrentFont;
 
 
/***================================================================***/
/*   SearchDict - look for  name                                      */
/*              - compare for match on len and string                 */
/*                return 0 - not found.                               */
/*                return n - nth element in dictionary.               */
/***================================================================***/
int SearchDictName(dictP,keyP)
 psdict *dictP;
 psobj  *keyP;
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
/***================================================================***/
boolean initFont(cnt)
int cnt;
{

  if (!(vm_init(cnt))) return(FALSE);
  vm_base = vm_next_byte();
  if (!(Init_BuiltInEncoding())) return(FALSE);
  strcpy(CurFontName, "");    /* iniitialize to none */
  FontP = &TheCurrentFont;
  FontP->vm_start = vm_next_byte();
  FontP->FontFileName.len = 0;
  FontP->FontFileName.data.valueP = CurFontName;
  return(TRUE);
}
/***================================================================***/
static void resetFont(env)
    char *env;
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
/***================================================================***/
int readFont(env)
char *env;
{
  int rcode;
 
  /* restore the virtual memory and eliminate old font */
  resetFont(env);
  /* This will load the font into the FontP */
  rcode = scan_font(FontP);
  if (rcode == SCAN_OUT_OF_MEMORY) {
    /* free the memory and start again */
    xfree(vm_base);
    if (!(initFont(vm_size * 2))) {
      /* we are really out of memory */
      return(SCAN_OUT_OF_MEMORY);
      }
    resetFont(env);
    rcode = scan_font(FontP);
    /* only double the memory once, then report error */
  }
  return(rcode);
}
/***================================================================***/
xobject fontfcnB(S,code,lenP,mode)
XYspace S;
unsigned char *code;
int  *lenP;
int  *mode;
{
  path updateWidth();
 
  psobj *charnameP; /* points to psobj that is name of character*/
  int   N;
  unsigned char *s;          /* used to search the name for '|' */
  psdict *CharStringsDictP; /* dictionary with char strings     */
  psobj   CodeName;   /* used to store the translation of the name*/
  psobj  *SubrsArrayP;
  psobj  *theStringP;
 
  path  charpath;   /* the path for this character              */
 
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
  charpath = Type1Char(FontP,S,theStringP,SubrsArrayP,NULL,
               FontP->BluesP , mode);
  /* if Type1Char reported an error, then return */
  if ( *mode == FF_PARSE_ERROR)  return(NULL);
  /* fill with winding rule unless path was requested */
  if (*mode != FF_PATH) {
    charpath =  Interior(charpath,WINDINGRULE+CONTINUITY);
  }
  return(charpath);
}
/***================================================================***/
/*   fontfcnA(env, mode)                                              */
/*                                                                    */
/*          env is a pointer to a string that contains the fontname.  */
/*                                                                    */
/*     1) initialize the font     - global indicates it has been done */
/*     2) load the font                                               */
/***================================================================***/
Bool fontfcnA(env,mode)
char *env;
int  *mode;
{
  int rc;
 
  /* Has the FontP initialized?  If not, then   */
  /* Initialize  */
  if (FontP == NULL) {
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
/***================================================================***/
/*   QueryFontLib(env, infoName,infoValue,rcodeP)                     */
/*                                                                    */
/*          env is a pointer to a string that contains the fontname.  */
/*                                                                    */
/*     1) initialize the font     - global indicates it has been done */
/*     2) load the font                                               */
/*     3) use the font to call getInfo for that value.                */
/***================================================================***/

void QueryFontLib(env,infoName,infoValue,rcodeP)
char *env;
char *infoName;
pointer infoValue;    /* parameter returned here    */
int  *rcodeP;
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
