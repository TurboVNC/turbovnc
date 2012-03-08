/* $Xorg: scanfont.c,v 1.3 2000/08/17 19:46:32 cpqbld Exp $ */
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
/* $XFree86: xc/lib/font/Type1/scanfont.c,v 1.17 2003/11/29 04:55:28 dawes Exp $ */

#ifndef FONTMODULE
#include <string.h>
#else
#include "Xdefs.h"	/* Bool declaration */
#include "Xmd.h"	/* INT32 declaration */
#include "xf86_ansic.h"
#endif
#include "t1stdio.h"
#include "util.h"
#include "token.h"
#ifdef BUILDCID
#include "range.h"
#endif
#include "objects.h"
#include "spaces.h"
#include "fontfcn.h"
#include "blues.h"
 
#ifdef BUILDCID
#define CID_BUFSIZE 80

extern psfont *FDArrayP;
static spacerange *spacerangeP;
static cidrange *notdefrangeP;
static cidrange *cidrangeP;
extern int FDArrayIndex;
static boolean CIDWantFontInfo;
static psobj inputFile1;
#endif
 
static int rc;
static boolean InPrivateDict;
static boolean WantFontInfo;
static boolean TwoSubrs;
static psobj inputFile;
static psobj filterFile;
static psobj *inputP;
 
 
/**********************************************************************/
/*   Init_BuiltInEncoding()                                           */
/*                                                                    */
/*     Initializes the StandardEncoding and ISOLatin1Encoding vector. */
/*                                                                    */
/**********************************************************************/
typedef struct				/* Builtin Standard Encoding */
{
   int  index;
   char *name;
} EncodingTable;

static EncodingTable StdEnc[] = {
  {  040 , "space" },
  {  041 , "exclam" },
  {  042 , "quotedbl" },
  {  043 , "numbersign" },
  {  044 , "dollar" },
  {  045 , "percent" },
  {  046 , "ampersand" },
  {  047 , "quoteright" },
  {  050 , "parenleft" },
  {  051 , "parenright" },
  {  052 , "asterisk" },
  {  053 , "plus" },
  {  054 , "comma" },
  {  055 , "hyphen" },
  {  056 , "period" },
  {  057 , "slash" },
  {  060 , "zero" },
  {  061 , "one" },
  {  062 , "two" },
  {  063 , "three" },
  {  064 , "four" },
  {  065 , "five" },
  {  066 , "six" },
  {  067 , "seven" },
  {  070 , "eight" },
  {  071 , "nine" },
  {  072 , "colon" },
  {  073 , "semicolon" },
  {  074 , "less" },
  {  075 , "equal" },
  {  076 , "greater" },
  {  077 , "question" },
  { 0100 , "at" },
  { 0101 , "A" },
  { 0102 , "B" },
  { 0103 , "C" },
  { 0104 , "D" },
  { 0105 , "E" },
  { 0106 , "F" },
  { 0107 , "G" },
  { 0110 , "H" },
  { 0111 , "I" },
  { 0112 , "J" },
  { 0113 , "K" },
  { 0114 , "L" },
  { 0115 , "M" },
  { 0116 , "N" },
  { 0117 , "O" },
  { 0120 , "P" },
  { 0121 , "Q" },
  { 0122 , "R" },
  { 0123 , "S" },
  { 0124 , "T" },
  { 0125 , "U" },
  { 0126 , "V" },
  { 0127 , "W" },
  { 0130 , "X" },
  { 0131 , "Y" },
  { 0132 , "Z" },
  { 0133 , "bracketleft" },
  { 0134 , "backslash" },
  { 0135 , "bracketright" },
  { 0136 , "asciicircum" },
  { 0137 , "underscore" },
  { 0140 , "quoteleft" },
  { 0141 , "a" },
  { 0142 , "b" },
  { 0143 , "c" },
  { 0144 , "d" },
  { 0145 , "e" },
  { 0146 , "f" },
  { 0147 , "g" },
  { 0150 , "h" },
  { 0151 , "i" },
  { 0152 , "j" },
  { 0153 , "k" },
  { 0154 , "l" },
  { 0155 , "m" },
  { 0156 , "n" },
  { 0157 , "o" },
  { 0160 , "p" },
  { 0161 , "q" },
  { 0162 , "r" },
  { 0163 , "s" },
  { 0164 , "t" },
  { 0165 , "u" },
  { 0166 , "v" },
  { 0167 , "w" },
  { 0170 , "x" },
  { 0171 , "y" },
  { 0172 , "z" },
  { 0173 , "braceleft" },
  { 0174 , "bar" },
  { 0175 , "braceright" },
  { 0176 , "asciitilde" },
  { 0241 , "exclamdown" },
  { 0242 , "cent" },
  { 0243 , "sterling" },
  { 0244 , "fraction" },
  { 0245 , "yen" },
  { 0246 , "florin" },
  { 0247 , "section" },
  { 0250 , "currency" },
  { 0251 , "quotesingle" },
  { 0252 , "quotedblleft" },
  { 0253 , "guillemotleft" },
  { 0254 , "guilsinglleft" },
  { 0255 , "guilsinglright" },
  { 0256 , "fi" },
  { 0257 , "fl" },
  { 0261 , "endash" },
  { 0262 , "dagger" },
  { 0263 , "daggerdbl" },
  { 0264 , "periodcentered" },
  { 0266 , "paragraph" },
  { 0267 , "bullet" },
  { 0270 , "quotesinglbase" },
  { 0271 , "quotedblbase" },
  { 0272 , "quotedblright" },
  { 0273 , "guillemotright" },
  { 0274 , "ellipsis" },
  { 0275 , "perthousand" },
  { 0277 , "questiondown" },
  { 0301 , "grave" },
  { 0302 , "acute" },
  { 0303 , "circumflex" },
  { 0304 , "tilde" },
  { 0305 , "macron" },
  { 0306 , "breve" },
  { 0307 , "dotaccent" },
  { 0310 , "dieresis" },
  { 0312 , "ring" },
  { 0313 , "cedilla" },
  { 0315 , "hungarumlaut" },
  { 0316 , "ogonek" },
  { 0317 , "caron" },
  { 0320 , "emdash" },
  { 0341 , "AE" },
  { 0343 , "ordfeminine" },
  { 0350 , "Lslash" },
  { 0351 , "Oslash" },
  { 0352 , "OE" },
  { 0353 , "ordmasculine" },
  { 0361 , "ae" },
  { 0365 , "dotlessi" },
  { 0370 , "lslash" },
  { 0371 , "oslash" },
  { 0372 , "oe" },
  { 0373 , "germandbls" },
  {    0,      0 }
};

static EncodingTable ISO8859Enc[] = {
 {  32, "space" },
 {  33, "exclam" },
 {  34, "quotedbl" },
 {  35, "numbersign" },
 {  36, "dollar" },
 {  37, "percent" },
 {  38, "ampersand" },
 {  39, "quoteright" },
 {  40, "parenleft" },
 {  41, "parenright" },
 {  42, "asterisk" },
 {  43, "plus" },
 {  44, "comma" },
 {  45, "minus" },
 {  46, "period" },
 {  47, "slash" },
 {  48, "zero" },
 {  49, "one" },
 {  50, "two" },
 {  51, "three" },
 {  52, "four" },
 {  53, "five" },
 {  54, "six" },
 {  55, "seven" },
 {  56, "eight" },
 {  57, "nine" },
 {  58, "colon" },
 {  59, "semicolon" },
 {  60, "less" },
 {  61, "equal" },
 {  62, "greater" },
 {  63, "question" },
 {  64, "at" },
 {  65, "A" },
 {  66, "B" },
 {  67, "C" },
 {  68, "D" },
 {  69, "E" },
 {  70, "F" },
 {  71, "G" },
 {  72, "H" },
 {  73, "I" },
 {  74, "J" },
 {  75, "K" },
 {  76, "L" },
 {  77, "M" },
 {  78, "N" },
 {  79, "O" },
 {  80, "P" },
 {  81, "Q" },
 {  82, "R" },
 {  83, "S" },
 {  84, "T" },
 {  85, "U" },
 {  86, "V" },
 {  87, "W" },
 {  88, "X" },
 {  89, "Y" },
 {  90, "Z" },
 {  91, "bracketleft" },
 {  92, "backslash" },
 {  93, "bracketright" },
 {  94, "asciicircum" },
 {  95, "underscore" },
 {  96, "quoteleft" },
 {  97, "a" },
 {  98, "b" },
 {  99, "c" },
 { 100, "d" },
 { 101, "e" },
 { 102, "f" },
 { 103, "g" },
 { 104, "h" },
 { 105, "i" },
 { 106, "j" },
 { 107, "k" },
 { 108, "l" },
 { 109, "m" },
 { 110, "n" },
 { 111, "o" },
 { 112, "p" },
 { 113, "q" },
 { 114, "r" },
 { 115, "s" },
 { 116, "t" },
 { 117, "u" },
 { 118, "v" },
 { 119, "w" },
 { 120, "x" },
 { 121, "y" },
 { 122, "z" },
 { 123, "braceleft" },
 { 124, "bar" },
 { 125, "braceright" },
 { 126, "asciitilde" },
 { 160, "space" },
 { 161, "exclamdown" },
 { 162, "cent" },
 { 163, "sterling" },
 { 164, "currency" },
 { 165, "yen" },
 { 166, "brokenbar" },
 { 167, "section" },
 { 168, "dieresis" },
 { 169, "copyright" },
 { 170, "ordfeminine" },
 { 171, "guillemotleft" },
 { 172, "logicalnot" },
 { 173, "hyphen" },
 { 174, "registered" },
 { 175, "macron" },
 { 176, "degree" },
 { 177, "plusminus" },
 { 178, "twosuperior" },
 { 179, "threesuperior" },
 { 180, "acute" },
 { 181, "mu" },
 { 182, "paragraph" },
 { 183, "periodcentered" },
 { 184, "cedilla" },
 { 185, "onesuperior" },
 { 186, "ordmasculine" },
 { 187, "guillemotright" },
 { 188, "onequarter" },
 { 189, "onehalf" },
 { 190, "threequarters" },
 { 191, "questiondown" },
 { 192, "Agrave" },
 { 193, "Aacute" },
 { 194, "Acircumflex" },
 { 195, "Atilde" },
 { 196, "Adieresis" },
 { 197, "Aring" },
 { 198, "AE" },
 { 199, "Ccedilla" },
 { 200, "Egrave" },
 { 201, "Eacute" },
 { 202, "Ecircumflex" },
 { 203, "Edieresis" },
 { 204, "Igrave" },
 { 205, "Iacute" },
 { 206, "Icircumflex" },
 { 207, "Idieresis" },
 { 208, "Eth" },
 { 209, "Ntilde" },
 { 210, "Ograve" },
 { 211, "Oacute" },
 { 212, "Ocircumflex" },
 { 213, "Otilde" },
 { 214, "Odieresis" },
 { 215, "multiply" },
 { 216, "Oslash" },
 { 217, "Ugrave" },
 { 218, "Uacute" },
 { 219, "Ucircumflex" },
 { 220, "Udieresis" },
 { 221, "Yacute" },
 { 222, "Thorn" },
 { 223, "germandbls" },
 { 224, "agrave" },
 { 225, "aacute" },
 { 226, "acircumflex" },
 { 227, "atilde" },
 { 228, "adieresis" },
 { 229, "aring" },
 { 230, "ae" },
 { 231, "ccedilla" },
 { 232, "egrave" },
 { 233, "eacute" },
 { 234, "ecircumflex" },
 { 235, "edieresis" },
 { 236, "igrave" },
 { 237, "iacute" },
 { 238, "icircumflex" },
 { 239, "idieresis" },
 { 240, "eth" },
 { 241, "ntilde" },
 { 242, "ograve" },
 { 243, "oacute" },
 { 244, "ocircumflex" },
 { 245, "otilde" },
 { 246, "odieresis" },
 { 247, "divide" },
 { 248, "oslash" },
 { 249, "ugrave" },
 { 250, "uacute" },
 { 251, "ucircumflex" },
 { 252, "udieresis" },
 { 253, "yacute" },
 { 254, "thorn" },
 { 255, "ydieresis" },
 {   0,      0 }
};

static psobj *StdEncArrayP = NULL;
psobj *ISOLatin1EncArrayP = NULL; 
 
static psobj *
MakeEncodingArrayP(EncodingTable *encodingTable)
{
  int i;
  psobj *encodingArrayP;
 
  encodingArrayP = (psobj *)vm_alloc(256*(sizeof(psobj)));
  if (!encodingArrayP)
      return NULL;

  /* initialize everything to .notdef */
  for (i=0; i<256;i++)
      objFormatName(&(encodingArrayP[i]),7, ".notdef");

  for (i=0; encodingTable[i].name; i++)
  {
      objFormatName(&(encodingArrayP[encodingTable[i].index]),
		    strlen(encodingTable[i].name),
		    encodingTable[i].name);
  }

  return(encodingArrayP);
}
 
boolean 
Init_BuiltInEncoding(void)
{
    StdEncArrayP = MakeEncodingArrayP(StdEnc);
    ISOLatin1EncArrayP = MakeEncodingArrayP(ISO8859Enc);
    return (StdEncArrayP && ISOLatin1EncArrayP);
}
 
/********************************************************************/
/***================================================================***/
static int 
getNextValue(int valueType)
{
  scan_token(inputP);
  if (tokenType != valueType) {
    return(SCAN_ERROR);
  }
  return(SCAN_OK);
 
}
/***================================================================***/
/*  This routine will set the global rc if there is an error          */
/***================================================================***/
static int 
getInt(void)
{
  scan_token(inputP);
  if (tokenType != TOKEN_INTEGER) {
    rc = SCAN_ERROR;
    return(0);
  }
  else {
    return( tokenValue.integer);
  }
 
}
/***================================================================***/
/*
 * See Sec 10.3 of ``Adobe Type 1 Font Format'' v1.1,
 * for parsing Encoding.
 */
static int 
getEncoding(psobj *arrayP)
{
  scan_token(inputP);
  if ((tokenType == TOKEN_NAME && (tokenLength==16 || tokenLength==17)))
  {
    if((tokenLength==16) && (!strncmp(tokenStartP,"StandardEncoding",16)))
          arrayP->data.valueP = (char *) StdEncArrayP;
    else
          arrayP->data.valueP = (char *) ISOLatin1EncArrayP;
      arrayP->len = 256;
      return(SCAN_OK);
  }
  else if ( (tokenType == TOKEN_LEFT_BRACE) ||
       (tokenType == TOKEN_LEFT_BRACKET) )
  {
      /* Array of literal names */

      psobj *objP;
      int i;

      objP = (psobj *)vm_alloc(256*(sizeof(psobj)));
      if (!(objP)) return(SCAN_OUT_OF_MEMORY);

      arrayP->data.valueP = (char *) objP;
      arrayP->len = 256;

      for (i=0; i<256; i++, objP++)
      {
	  scan_token(inputP);
	  
	  if (tokenType != TOKEN_LITERAL_NAME)
	      return(SCAN_ERROR);

	  if (!(vm_alloc(tokenLength)) ) return(SCAN_OUT_OF_MEMORY);
	  objFormatName(objP,tokenLength,tokenStartP);
      }

      scan_token(inputP);
      if ( (tokenType == TOKEN_RIGHT_BRACE) ||
	  (tokenType == TOKEN_RIGHT_BRACKET) )
	  return(SCAN_OK);
  }
  else
  {
      /* Must be sequences of ``dup <index> <charactername> put" */

      psobj *objP;
      int i;

      objP = (psobj *)vm_alloc(256*(sizeof(psobj)));
      if (!(objP)) return(SCAN_OUT_OF_MEMORY);

      arrayP->data.valueP = (char *) objP;
      arrayP->len = 256;

      for (i=0; i<256; i++)
	  objFormatName(objP + i, 7, ".notdef");

      while (TRUE)
      {
	  scan_token(inputP);

	  switch (tokenType)
	  {
	  case TOKEN_NAME:
	      if (tokenLength == 3)
	      {
		  if (strncmp(tokenStartP,"dup",3) == 0)
		  {
		      /* get <index> */
		      scan_token(inputP);
		      if (tokenType != TOKEN_INTEGER ||
			  tokenValue.integer < 0 ||
			  tokenValue.integer > 255)
			  return (SCAN_ERROR);
		      i = tokenValue.integer;

		      /* get <characer_name> */
		      scan_token(inputP);
		      if (tokenType != TOKEN_LITERAL_NAME)
			  return(SCAN_ERROR);

		      if (!(vm_alloc(tokenLength)) )
			  return(SCAN_OUT_OF_MEMORY);
		      objFormatName(objP + i,tokenLength,tokenStartP);

		      /* get "put" */
		      scan_token(inputP);
		      if (tokenType != TOKEN_NAME)
			  return(SCAN_ERROR);
		  }
		  else if (strncmp(tokenStartP,"def",3) == 0)
		      return (SCAN_OK);
	      }
	      break;
	  case TOKEN_EOF:
	  case TOKEN_NONE:
	  case TOKEN_INVALID:
	      return (SCAN_ERROR);
	  }
      }
  }

  return (SCAN_ERROR);
}
/***================================================================***/
#ifdef BUILDCID
static int 
getFDArray(psobj *arrayP)
{
  int rc;

  /* get the number of items in the FDArray */
  scan_token(inputP);
  if (tokenType == TOKEN_INTEGER) {
      /* an FD array must contain at least one element */
      if (tokenValue.integer <= 0)
          return(SCAN_ERROR);
      arrayP->len = tokenValue.integer;
  } else
      return(SCAN_ERROR);

  /* get the token "array" */
  scan_token(inputP);
  if (tokenType != TOKEN_NAME || strncmp(tokenStartP, "array", 5) != 0)
      return(SCAN_ERROR);
 
  /* format the array in memory, save pointer to the beginning */
  arrayP->data.valueP = tokenStartP;

  /* allocate FDArray */
  FDArrayP = (psfont *)vm_alloc(arrayP->len*(sizeof(psfont)));
  if (!(FDArrayP)) return(SCAN_OUT_OF_MEMORY);

  /* get a specified number of font dictionaries */
  for (FDArrayIndex = 0; FDArrayIndex < arrayP->len; FDArrayIndex++) {
      /* get "dup" */
      scan_token(inputP);
      if (tokenType != TOKEN_NAME || strncmp(tokenStartP, "dup", 3) != 0)
          return(SCAN_ERROR);
      /* get an integer digit */
      scan_token(inputP);
      if (tokenType != TOKEN_INTEGER)
          return(SCAN_ERROR);

      /* read a CID version of a Type 1 font */
      if (!CIDType1fontfcnA(&rc))
          return(rc);

      /* get "put" */
      scan_token(inputP);
      if (tokenType != TOKEN_NAME || strncmp(tokenStartP, "put", 3) != 0)
          return(SCAN_ERROR);
  }
  return(SCAN_OK);
}
#endif

static int 
getArray(psobj *arrayP)
{
  int N;   /* count the items in the array */
  psobj *objP;

  /* That is totally a kludge. If some stupid font file has
   *	/foo/foo	# ftp://ftp.cdrom.com/pub/os2/fonts/future.zip
   * we will treat it as /foo.
   *  H.J. */
  char tmp [1024];

  strncpy (tmp, tokenStartP, sizeof (tmp));
  tmp [sizeof (tmp) - 1] = '\0';

restart: 
  scan_token(inputP);
  switch (tokenType)
  {
  case TOKEN_LEFT_BRACE:
  case TOKEN_LEFT_BRACKET:
    break;

  case TOKEN_LITERAL_NAME:
    tokenStartP[tokenLength] = '\0';
    if (strcmp (tokenStartP, tmp) == 0)
    {
      /* Ok, We see /foo/foo. Let's restart. */
      goto restart;
    }

  default:
    return(SCAN_ERROR);
  }
  /* format the array in memory, save pointer to the beginning */
  arrayP->data.valueP = tokenStartP;
  /* loop, picking up next object, until right BRACE or BRACKET */
  N = 0;
  do {
    scan_token(inputP);
    if ( (tokenType == TOKEN_RIGHT_BRACE) ||
         (tokenType == TOKEN_RIGHT_BRACKET) ) {
      /* save then number of items in the array */
      arrayP->len = N;
      return(SCAN_OK);
    }
     /* allocate the space for the object */
    objP = (psobj *)vm_alloc(sizeof(psobj));
    if (!(objP)) return(SCAN_OUT_OF_MEMORY);
 
    /* array is an array of numbers, (real or integer)  */
    if (tokenType == TOKEN_REAL) {
      objFormatReal(objP, tokenValue.real);
    }
    else
      if (tokenType == TOKEN_INTEGER) {
        objFormatInteger(objP, tokenValue.integer);
      }
      else return(SCAN_ERROR);
    N++;
  }  while ( 1>0 );
  /* NOTREACHED*/
}
/***================================================================***/
static int 
getName(char *nameP)
{
  do {
    scan_token(inputP);
    if (tokenType <= TOKEN_NONE) {
      if (tokenTooLong) return(SCAN_OUT_OF_MEMORY);
      return(SCAN_ERROR);
    }
  } while ((tokenType != TOKEN_NAME) ||
    (0 != strncmp(tokenStartP,nameP,strlen(nameP))) );
  /* found */
  return(SCAN_OK);
}
/***================================================================***/
static int 
getNbytes(int N)
{
  int I;
 
 
  tokenStartP = vm_next_byte();
  tokenMaxP = tokenStartP + MIN(vm_free_bytes(), MAX_STRING_LEN);
  if (N > vm_free_bytes()) {
    return(SCAN_OUT_OF_MEMORY);
  }
  I = T1Read(tokenStartP,1,N,inputP->data.fileP);
  if ( I != N )     return(SCAN_FILE_EOF);
  return(SCAN_OK);
}
 
/***================================================================***/
/*  getLiteralName(nameObjP)                                          */
/*     scan for next literal.                                         */
/*  if we encounter the name 'end' then terminate and say ok.         */
/*    It means that the CharStrings does not have as many characters  */
/*    as the dictionary said it would and that is ok.                 */
/***================================================================***/
static int 
getLiteralName(psobj *nameObjP)
{
  do {
    scan_token(inputP);
    if (tokenType <= TOKEN_NONE) {
      if (tokenTooLong) return(SCAN_OUT_OF_MEMORY);
      return(SCAN_ERROR);
    }
    if (tokenType == TOKEN_NAME) {
      if (0 == strncmp(tokenStartP,"end",3) ) {
        return(SCAN_END);
      }
    }
  } while  (tokenType != TOKEN_LITERAL_NAME) ;
  nameObjP->len = tokenLength;
  /* allocate all the names in the CharStrings Structure */
  if (!(vm_alloc(tokenLength)) ) return(SCAN_OUT_OF_MEMORY);
  nameObjP->data.valueP =  tokenStartP;
  /* found */
  return(SCAN_OK);
}
 
/***================================================================***/
/*
 *   BuildSubrs routine
 */
/***================================================================***/
 
static int 
BuildSubrs(psfont *FontP)
{
   int N;   /* number of values in Subrs */
   int I;   /* index into Subrs */
   int i;   /* loop thru  Subrs */
   int J;   /* length of Subrs entry */
   psobj *arrayP;
 
   /* next token should be a positive int */
   /* note: rc is set by getInt. */
   N = getInt();
   if (rc) return(rc);
   if (N < 0 ) return(SCAN_ERROR);
   /* if we already have a Subrs, then skip the second one */
   /* The second one is for hiresolution devices.          */
   if (FontP->Subrs.data.arrayP != NULL) {
     TwoSubrs = TRUE;
     /* process all the Subrs, but do not update anything */
     /* can not just skip them because of the binary data */
     for (i=0;i<N;i++) {
       /* look for dup */
       rc = getName("dup");
       if (rc) return(rc);
       /* get 2 integers */
       I = getInt();
       if (rc) return(rc);
       J = getInt();
       if (rc) return(rc);
       if ( (I < 0) || (J < 0 ) ) return (SCAN_ERROR);
       /* get the next token, it should be RD or -|, either is ok */
       rc = getNextValue(TOKEN_NAME);
       if ( rc != SCAN_OK ) return(rc);
       rc = getNbytes(J);
       if (rc) return(rc);
     }
     return(SCAN_OK);
   }
 
   arrayP = (psobj *)vm_alloc(N*sizeof(psobj));
   if (!(arrayP) ) return(SCAN_OUT_OF_MEMORY);
   FontP->Subrs.len = N;
   FontP->Subrs.data.arrayP =  arrayP;
   /* get N values for Subrs */
   for (i=0;i<N;i++) {
     /* look for dup */
     rc = getName("dup");
     if (rc) return(rc);
     /* get 2 integers */
     I = getInt();
     if (rc) return(rc);
     J = getInt();
     if (rc) return(rc);
     if ( (I < 0) || (J < 0 ) ) return (SCAN_ERROR);
     arrayP[I].len = J;
     /* get the next token, it should be RD or -|, either is ok */
     rc = getNextValue(TOKEN_NAME);
     if ( rc != SCAN_OK ) return(rc);
     rc = getNbytes(J);
     if (rc == SCAN_OK) {
       arrayP[I].data.valueP = tokenStartP;
       if ( !(vm_alloc(J)) ) return(SCAN_OUT_OF_MEMORY);
     }
     else return(rc);
   }
   return(SCAN_OK);
 
}
/***================================================================***/
/***================================================================***/
/*
 *   BuildCharStrings routine
 */
/***================================================================***/
 
static int 
BuildCharStrings(psfont *FontP)
{
   int N;   /* number of values in CharStrings */
   int i;   /* loop thru  Subrs */
   int J;   /* length of Subrs entry */
   psdict  *dictP;
 
   /* next token should be a positive int */
   N = getInt();
   if (rc) {
     /* check if file had TwoSubrs, hi resolution stuff is in file*/
     if (TwoSubrs) {
       do {
         scan_token(inputP);
         if (tokenType <= TOKEN_NONE) {
           if (tokenTooLong) return(SCAN_OUT_OF_MEMORY);
           return(SCAN_ERROR);
         }
       } while (tokenType != TOKEN_INTEGER);
       N = tokenValue.integer;
     }
     else return(rc);  /* if next token was not an Int */
   }
   if (N<=0) return(SCAN_ERROR);
   /* save number of entries in the dictionary */
 
   dictP = (psdict *)vm_alloc((N+1)*sizeof(psdict));
   if (!(dictP)) return(SCAN_OUT_OF_MEMORY);
   FontP->CharStringsP = dictP;
   dictP[0].key.len = N;
   /* get N values for CharStrings */
   for (i=1;i<=N;i++) {
     /* look for next literal name  */
     rc = getLiteralName(&(dictP[i].key));
     if (rc) return(rc);
     /* get 1 integer */
     J = getInt();
     if (rc) return(rc);  /* if next token was not an Int */
     if (J<0) return (SCAN_ERROR);
     dictP[i].value.len = J;
     /* get the next token, it should be RD or -|, either is ok */
     rc = getNextValue(TOKEN_NAME);
     if ( rc != SCAN_OK ) return(rc);
     rc = getNbytes(J);
     if (rc == SCAN_OK) {
       dictP[i].value.data.valueP = tokenStartP;
       if ( !(vm_alloc(J)) ) return(SCAN_OUT_OF_MEMORY);
     }
     else return(rc);
   }
   return(SCAN_OK);
 
}
/***================================================================***/
#ifdef BUILDCID
/***================================================================***/
/*
 *   BuildCIDFontInfo Dictionary
 */
/***================================================================***/
static int 
BuildCIDFontInfo(cidfont *CIDfontP)
{
  psdict *dictP;

  /* allocate the private dictionary (max number of entries + 1) */
  dictP = (psdict *)vm_alloc(20*sizeof(psdict));
  if (!(dictP)) return(SCAN_OUT_OF_MEMORY);

  CIDfontP->CIDfontInfoP = dictP;
  CIDfontP->CIDfontInfoP[0].key.len = 18;  /* number of actual entries */
  objFormatName(&(dictP[CIDCOUNT].key),8,"CIDCount");
  objFormatInteger(&(dictP[CIDCOUNT].value),-1);
  objFormatName(&(dictP[CIDFONTNAME].key),11,"CIDFontName");
  objFormatName(&(dictP[CIDFONTNAME].value),0,NULL);
  objFormatName(&(dictP[CIDFONTTYPE].key),11,"CIDFontType");
  objFormatInteger(&(dictP[CIDFONTTYPE].value),-1);
  objFormatName(&(dictP[CIDVERSION].key),14,"CIDFontVersion");
  objFormatInteger(&(dictP[CIDVERSION].value),-1);
  objFormatName(&(dictP[CIDREGISTRY].key),8,"Registry");
  objFormatString(&(dictP[CIDREGISTRY].value),0,NULL);
  objFormatName(&(dictP[CIDORDERING].key),8,"Ordering");
  objFormatString(&(dictP[CIDORDERING].value),0,NULL);
  objFormatName(&(dictP[CIDSUPPLEMENT].key),10,"Supplement");
  objFormatInteger(&(dictP[CIDSUPPLEMENT].value),-1);
  objFormatName(&(dictP[CIDMAPOFFSET].key),12,"CIDMapOffset");
  objFormatInteger(&(dictP[CIDMAPOFFSET].value),-1);
  objFormatName(&(dictP[CIDFDARRAY].key),7,"FDArray");
  objFormatArray(&(dictP[CIDFDARRAY].value),0,NULL);
  objFormatName(&(dictP[CIDFDBYTES].key),7,"FDBytes");
  objFormatInteger(&(dictP[CIDFDBYTES].value),-1);
  objFormatName(&(dictP[CIDFONTBBOX].key),8,"FontBBox");
  objFormatArray(&(dictP[CIDFONTBBOX].value),0,NULL);
  objFormatName(&(dictP[CIDFULLNAME].key),8,"FullName");
  objFormatString(&(dictP[CIDFULLNAME].value),0,NULL);
  objFormatName(&(dictP[CIDFAMILYNAME].key),10,"FamilyName");
  objFormatString(&(dictP[CIDFAMILYNAME].value),0,NULL);
  objFormatName(&(dictP[CIDWEIGHT].key),6,"Weight");
  objFormatString(&(dictP[CIDWEIGHT].value),0,NULL);
  objFormatName(&(dictP[CIDNOTICE].key),6,"Notice");
  objFormatString(&(dictP[CIDNOTICE].value),0,NULL);
  objFormatName(&(dictP[CIDGDBYTES].key),7,"GDBytes");
  objFormatInteger(&(dictP[CIDGDBYTES].value),-1);
  objFormatName(&(dictP[CIDUIDBASE].key),7,"UIDBase");
  objFormatInteger(&(dictP[CIDUIDBASE].value),0);
  objFormatName(&(dictP[CIDXUID].key),4,"XUID");
  objFormatInteger(&(dictP[CIDXUID].value),0);
  return(SCAN_OK);
}
/***================================================================***/
/*
 *   BuildCMapInfo Dictionary
 */
/***================================================================***/
static int 
BuildCMapInfo(cmapres *CMapP)
{
  psdict *dictP;

  /* allocate the private dictionary (max number of entries + 1) */
  dictP = (psdict *)vm_alloc(20*sizeof(psdict));
  if (!(dictP)) return(SCAN_OUT_OF_MEMORY);

  CMapP->CMapInfoP = dictP;
  CMapP->CMapInfoP[0].key.len = 8;  /* number of actual entries */
  objFormatName(&(dictP[CMAPREGISTRY].key),8,"Registry");
  objFormatString(&(dictP[CMAPREGISTRY].value),0,NULL);
  objFormatName(&(dictP[CMAPORDERING].key),8,"Ordering");
  objFormatString(&(dictP[CMAPORDERING].value),0,NULL);
  objFormatName(&(dictP[CMAPSUPPLEMENT].key),10,"Supplement");
  objFormatInteger(&(dictP[CMAPSUPPLEMENT].value),-1);
  objFormatName(&(dictP[CMAPNAME].key),8,"CMapName");
  objFormatString(&(dictP[CMAPNAME].value),0,NULL);
  objFormatName(&(dictP[CMAPVERSION].key),11,"CMapVersion");
  objFormatInteger(&(dictP[CMAPVERSION].value),-1);
  objFormatName(&(dictP[CMAPTYPE].key),8,"CMapType");
  objFormatInteger(&(dictP[CMAPTYPE].value),-1);
  objFormatName(&(dictP[CMAPWMODE].key),5,"WMode");
  objFormatInteger(&(dictP[CMAPWMODE].value),-1);
  objFormatName(&(dictP[CMAPCIDCOUNT].key),8,"CIDCount");
  objFormatInteger(&(dictP[CMAPCIDCOUNT].value),-1);
  return(SCAN_OK);
}
#endif

/***================================================================***/
/*
 *   BuildFontInfo Dictionary
 */
/***================================================================***/
static int 
BuildFontInfo(psfont *fontP)
{
  psdict *dictP;
 
  /* allocate the private dictionary */
  dictP = (psdict *)vm_alloc(20*sizeof(psdict));
  if (!(dictP)) return(SCAN_OUT_OF_MEMORY);
 
  fontP->fontInfoP = dictP;
  fontP->fontInfoP[0].key.len = 17;  /* number of actual entries */
  objFormatName(&(dictP[FONTNAME].key),8,"FontName");
  objFormatName(&(dictP[FONTNAME].value),0,NULL);
  objFormatName(&(dictP[PAINTTYPE].key),9,"PaintType");
  objFormatInteger(&(dictP[PAINTTYPE].value),0);
  objFormatName(&(dictP[FONTTYPENUM].key),8,"FontType");
  objFormatInteger(&(dictP[FONTTYPENUM].value),0);
  objFormatName(&(dictP[FONTMATRIX].key),10,"FontMatrix");
  objFormatArray(&(dictP[FONTMATRIX].value),0,NULL);
  objFormatName(&(dictP[FONTBBOX].key),8,"FontBBox");
  objFormatArray(&(dictP[FONTBBOX].value),0,NULL);
  objFormatName(&(dictP[ENCODING].key),8,"Encoding");
  objFormatEncoding(&(dictP[ENCODING].value),0,NULL);
  objFormatName(&(dictP[UNIQUEID].key),8,"UniqueID");
  objFormatInteger(&(dictP[UNIQUEID].value),0);
  objFormatName(&(dictP[STROKEWIDTH].key),11,"StrokeWidth");
  objFormatReal(&(dictP[STROKEWIDTH].value),0.0);
  objFormatName(&(dictP[VERSION].key),7,"version");
  objFormatString(&(dictP[VERSION].value),0,NULL);
  objFormatName(&(dictP[NOTICE].key),6,"Notice");
  objFormatString(&(dictP[NOTICE].value),0,NULL);
  objFormatName(&(dictP[FULLNAME].key),8,"FullName");
  objFormatString(&(dictP[FULLNAME].value),0,NULL);
  objFormatName(&(dictP[FAMILYNAME].key),10,"FamilyName");
  objFormatString(&(dictP[FAMILYNAME].value),0,NULL);
  objFormatName(&(dictP[WEIGHT].key),6,"Weight");
  objFormatString(&(dictP[WEIGHT].value),0,NULL);
  objFormatName(&(dictP[ITALICANGLE].key),11,"ItalicAngle");
  objFormatReal(&(dictP[ITALICANGLE].value),0.0);
  objFormatName(&(dictP[ISFIXEDPITCH].key),12,"isFixedPitch");
  objFormatBoolean(&(dictP[ISFIXEDPITCH].value),FALSE);
  objFormatName(&(dictP[UNDERLINEPOSITION].key),17,"UnderlinePosition");
  objFormatReal(&(dictP[UNDERLINEPOSITION].value),0.0);
  objFormatName(&(dictP[UNDERLINETHICKNESS].key),18,"UnderlineThickness");
  objFormatReal(&(dictP[UNDERLINETHICKNESS].value),0.0);
  return(SCAN_OK);
}
#ifdef BUILDCID
/***================================================================***/
/*
 *   BuildCIDType1Private Dictionary
 */
/***================================================================***/
static int 
BuildCIDType1Private(psfont *fontP)
{
  psdict *Private;

  /* allocate the private dictionary */
  Private = (psdict *)vm_alloc(21*sizeof(psdict));

  if (!(Private)) return(SCAN_OUT_OF_MEMORY);

  fontP->Private = Private;
  fontP->Private[0].key.len = 20;  /* number of actual entries */

  objFormatName(&(Private[CIDT1MINFEATURE].key),10,"MinFeature");
  objFormatArray(&(Private[CIDT1MINFEATURE].value),0,NULL);
  objFormatName(&(Private[CIDT1LENIV].key),5,"lenIV");
  objFormatInteger(&(Private[CIDT1LENIV].value),DEFAULTLENIV);
  objFormatName(&(Private[CIDT1LANGGROUP].key),13,"LanguageGroup");
  objFormatInteger(&(Private[CIDT1LANGGROUP].value),DEFAULTLANGUAGEGROUP);
  objFormatName(&(Private[CIDT1BLUEVALUES].key),10,"BlueValues");
  objFormatArray(&(Private[CIDT1BLUEVALUES].value),0,NULL);
  objFormatName(&(Private[CIDT1OTHERBLUES].key),10,"OtherBlues");
  objFormatArray(&(Private[CIDT1OTHERBLUES].value),0,NULL);
  objFormatName(&(Private[CIDT1BLUESCALE].key),9,"BlueScale");
  objFormatReal(&(Private[CIDT1BLUESCALE].value),DEFAULTBLUESCALE);
  objFormatName(&(Private[CIDT1BLUEFUZZ].key),8,"BlueFuzz");
  objFormatInteger(&(Private[CIDT1BLUEFUZZ].value),DEFAULTBLUEFUZZ);
  objFormatName(&(Private[CIDT1BLUESHIFT].key),9,"BlueShift");
  objFormatInteger(&(Private[CIDT1BLUESHIFT].value),DEFAULTBLUESHIFT);
  objFormatName(&(Private[CIDT1FAMBLUES].key),11,"FamilyBlues");
  objFormatArray(&(Private[CIDT1FAMBLUES].value),0,NULL);
  objFormatName(&(Private[CIDT1FAMOTHERBLUES].key),16,"FamilyOtherBlues");
  objFormatArray(&(Private[CIDT1FAMOTHERBLUES].value),0,NULL);
  objFormatName(&(Private[CIDT1STDHW].key),5,"StdHW");
  objFormatArray(&(Private[CIDT1STDHW].value),0,NULL);
  objFormatName(&(Private[CIDT1STDVW].key),5,"StdVW");
  objFormatArray(&(Private[CIDT1STDVW].value),0,NULL);
  objFormatName(&(Private[CIDT1STEMSNAPH].key),9,"StemSnapH");
  objFormatArray(&(Private[CIDT1STEMSNAPH].value),0,NULL);
  objFormatName(&(Private[CIDT1STEMSNAPV].key),9,"StemSnapV");
  objFormatArray(&(Private[CIDT1STEMSNAPV].value),0,NULL);
  /* skip password */
  objFormatName(&(Private[CIDT1SUBMAPOFF].key),13,"SubrMapOffset");
  objFormatInteger(&(Private[CIDT1SUBMAPOFF].value),0);
  objFormatName(&(Private[CIDT1SDBYTES].key),7,"SDBytes");
  objFormatInteger(&(Private[CIDT1SDBYTES].value),0);
  objFormatName(&(Private[CIDT1SUBRCNT].key),9,"SubrCount");
  objFormatInteger(&(Private[CIDT1SUBRCNT].value),0);
  objFormatName(&(Private[CIDT1FORCEBOLD].key),9,"ForceBold");
  objFormatBoolean(&(Private[CIDT1FORCEBOLD].value),DEFAULTFORCEBOLD);
  objFormatName(&(Private[CIDT1RNDSTEMUP].key),9,"RndStemUp");
  objFormatBoolean(&(Private[CIDT1RNDSTEMUP].value),DEFAULTRNDSTEMUP);
  objFormatName(&(Private[CIDT1EXPFACTOR].key),15,"ExpansionFactor");
  objFormatReal(&(Private[CIDT1EXPFACTOR].value),
                          DEFAULTEXPANSIONFACTOR);
  return(SCAN_OK);
}
#endif
/***================================================================***/
/*
 *   BuildPrivate Dictionary
 */
/***================================================================***/
static int 
BuildPrivate(psfont *fontP)
{
  psdict *Private;
 
  /* allocate the private dictionary */
  Private = (psdict *)vm_alloc(20*sizeof(psdict));
 
  if (!(Private)) return(SCAN_OUT_OF_MEMORY);
 
  fontP->Private = Private;
  fontP->Private[0].key.len = 16;  /* number of actual entries */
 
  objFormatName(&(Private[BLUEVALUES].key),10,"BlueValues");
  objFormatArray(&(Private[BLUEVALUES].value),0,NULL);
  objFormatName(&(Private[OTHERBLUES].key),10,"OtherBlues");
  objFormatArray(&(Private[OTHERBLUES].value),0,NULL);
  objFormatName(&(Private[FAMILYBLUES].key),11,"FamilyBlues");
  objFormatArray(&(Private[FAMILYBLUES].value),0,NULL);
  objFormatName(&(Private[FAMILYOTHERBLUES].key),16,"FamilyOtherBlues");
  objFormatArray(&(Private[FAMILYOTHERBLUES].value),0,NULL);
  objFormatName(&(Private[BLUESCALE].key),9,"BlueScale");
  objFormatReal(&(Private[BLUESCALE].value),DEFAULTBLUESCALE);
  objFormatName(&(Private[BLUESHIFT].key),9,"BlueShift");
  objFormatInteger(&(Private[BLUESHIFT].value),DEFAULTBLUESHIFT);
  objFormatName(&(Private[BLUEFUZZ].key),8,"BlueFuzz");
  objFormatInteger(&(Private[BLUEFUZZ].value),DEFAULTBLUEFUZZ);
  objFormatName(&(Private[STDHW].key),5,"StdHW");
  objFormatArray(&(Private[STDHW].value),0,NULL);
  objFormatName(&(Private[STDVW].key),5,"StdVW");
  objFormatArray(&(Private[STDVW].value),0,NULL);
  objFormatName(&(Private[STEMSNAPH].key),9,"StemSnapH");
  objFormatArray(&(Private[STEMSNAPH].value),0,NULL);
  objFormatName(&(Private[STEMSNAPV].key),9,"StemSnapV");
  objFormatArray(&(Private[STEMSNAPV].value),0,NULL);
  objFormatName(&(Private[FORCEBOLD].key),9,"ForceBold");
  objFormatBoolean(&(Private[FORCEBOLD].value),DEFAULTFORCEBOLD);
  objFormatName(&(Private[LANGUAGEGROUP].key),13,"LanguageGroup");
  objFormatInteger(&(Private[LANGUAGEGROUP].value),DEFAULTLANGUAGEGROUP);
  objFormatName(&(Private[LENIV].key),5,"lenIV");
  objFormatInteger(&(Private[LENIV].value),DEFAULTLENIV);
  objFormatName(&(Private[RNDSTEMUP].key),9,"RndStemUp");
  objFormatBoolean(&(Private[RNDSTEMUP].value),DEFAULTRNDSTEMUP);
  objFormatName(&(Private[EXPANSIONFACTOR].key),9,"ExpansionFactor");
  objFormatReal(&(Private[EXPANSIONFACTOR].value),
                          DEFAULTEXPANSIONFACTOR);
  return(SCAN_OK);
}
/***================================================================***/
/**********************************************************************/
/*     GetType1Blues(fontP)                                           */
/*                                                                    */
/*   Routine to support font-level hints.                             */
/*                                                                    */
/*         Gets all the Blues information from the Private dictionary */
/*         for the font.                                              */
/*                                                                    */
/*                                                                    */
/**********************************************************************/
static int 
GetType1Blues(psfont *fontP)
{
  psdict *PrivateDictP;   /* the Private dict relating to hints */
  struct blues_struct *blues;  /* ptr for the blues struct we will allocate */
  int i;
  psobj *HintEntryP;
 
 
 
  /* get the Private dictionary pointer */
  PrivateDictP = fontP->Private;
 
  /* allocate the memory for the blues structure */
  blues = (struct blues_struct *) vm_alloc(sizeof(struct blues_struct));
 
  if (!blues)  return(SCAN_OUT_OF_MEMORY);
 
  /* Make fontP's blues ptr point to this newly allocated structure. */
  fontP->BluesP = blues;
 
  /* fill in the BlueValues array */
  HintEntryP = &(PrivateDictP[BLUEVALUES].value);
  /* check to see if the entry exists and if it's an array */
  if ( !objPIsArray(HintEntryP) || (HintEntryP->len == 0 ))
      blues->numBlueValues = 0;
  else {
      /* get the number of values in the array */
      if (HintEntryP->len > NUMBLUEVALUES) {
          blues->numBlueValues = NUMBLUEVALUES;
      } else
          blues->numBlueValues = HintEntryP->len;
      for (i = 0; i<= blues->numBlueValues-1; ++i) {
          if (objPIsInteger(&HintEntryP->data.arrayP[i]))
              blues->BlueValues[i] =
                  HintEntryP->data.arrayP[i].data.integer;
          else if (objPIsReal(&HintEntryP->data.arrayP[i]))
              blues->BlueValues[i] =
                  HintEntryP->data.arrayP[i].data.real;
          else
              blues->BlueValues[i] = 0;
      }
  }
 
  /* fill in the OtherBlues array */
  HintEntryP =  &(PrivateDictP[OTHERBLUES].value);
  /* check to see if the entry exists and if it's an array */
  if ( !objPIsArray(HintEntryP) || (HintEntryP->len == 0 ))
      blues->numOtherBlues = 0;
  else {
      /* get the number of values in the array */
      if (HintEntryP->len > NUMOTHERBLUES) {
          blues->numOtherBlues = NUMOTHERBLUES;
      } else
          blues->numOtherBlues = HintEntryP->len;
      for (i = 0; i<= blues->numOtherBlues-1; ++i) {
          if (objPIsInteger(&HintEntryP->data.arrayP[i]))
              blues->OtherBlues[i] =
                  HintEntryP->data.arrayP[i].data.integer;
          else if (objPIsReal(&HintEntryP->data.arrayP[i]))
              blues->OtherBlues[i] =
                  HintEntryP->data.arrayP[i].data.real;
          else
              blues->OtherBlues[i] = 0;
      }
  }
 
  /* fill in the FamilyBlues array */
  HintEntryP =  &(PrivateDictP[FAMILYBLUES].value);
  /* check to see if the entry exists and if it's an array */
  if ( !objPIsArray(HintEntryP) || (HintEntryP->len == 0 ))
      blues->numFamilyBlues = 0;
  else {
      /* get the number of values in the array */
      if (HintEntryP->len > NUMFAMILYBLUES) {
          blues->numFamilyBlues = NUMFAMILYBLUES;
      } else
          blues->numFamilyBlues = HintEntryP->len;
      for (i = 0; i<= blues->numFamilyBlues-1; ++i) {
          if (objPIsInteger(&HintEntryP->data.arrayP[i]))
              blues->FamilyBlues[i] =
                  HintEntryP->data.arrayP[i].data.integer;
          else if (objPIsReal(&HintEntryP->data.arrayP[i]))
              blues->FamilyBlues[i] =
                  HintEntryP->data.arrayP[i].data.real;
          else
              blues->FamilyBlues[i] = 0;
      }
  }
 
  /* fill in the FamilyOtherBlues array */
  HintEntryP =  &(PrivateDictP[FAMILYOTHERBLUES].value);
  /* check to see if the entry exists and if it's an array */
  if ( !objPIsArray(HintEntryP) || (HintEntryP->len == 0 ))
      blues->numFamilyOtherBlues = 0;
  else {
      /* get the number of values in the array */
      if (HintEntryP->len > NUMFAMILYOTHERBLUES) {
          blues->numFamilyOtherBlues = NUMFAMILYOTHERBLUES;
      } else
          blues->numFamilyOtherBlues = HintEntryP->len;
      for (i = 0; i<= blues->numFamilyOtherBlues-1; ++i) {
          if (objPIsInteger(&HintEntryP->data.arrayP[i]))
              blues->FamilyOtherBlues[i] =
                  HintEntryP->data.arrayP[i].data.integer;
          else if (objPIsReal(&HintEntryP->data.arrayP[i]))
              blues->FamilyOtherBlues[i] =
                  HintEntryP->data.arrayP[i].data.real;
          else
              blues->FamilyOtherBlues[i] = 0;
      }
  }
 
  /* fill in the StemSnapH array */
  HintEntryP =  &(PrivateDictP[STEMSNAPH].value);
  /* check to see if the entry exists and if it's an array */
  if ( !objPIsArray(HintEntryP) || (HintEntryP->len == 0 ))
      blues->numStemSnapH = 0;
  else {
      /* get the number of values in the array */
      if (HintEntryP->len > NUMSTEMSNAPH) {
          blues->numStemSnapH = NUMSTEMSNAPH;
      } else
          blues->numStemSnapH = HintEntryP->len;
      for (i = 0; i<= blues->numStemSnapH-1; ++i) {
          if (objPIsInteger(&HintEntryP->data.arrayP[i]))
              blues->StemSnapH[i] =
                  HintEntryP->data.arrayP[i].data.integer;
          else if (objPIsReal(&HintEntryP->data.arrayP[i]))
              blues->StemSnapH[i] =
                  HintEntryP->data.arrayP[i].data.real;
          else
              blues->StemSnapH[i] = 0;
      }
  }
 
  /* fill in the StemSnapV array */
  HintEntryP =  &(PrivateDictP[STEMSNAPV].value);
  /* check to see if the entry exists and if it's an array */
  if ( !objPIsArray(HintEntryP) || (HintEntryP->len == 0 ))
      blues->numStemSnapV = 0;
  else {
      /* get the number of values in the array */
      if (HintEntryP->len > NUMSTEMSNAPV) {
          blues->numStemSnapV = NUMSTEMSNAPV;
      } else
          blues->numStemSnapV = HintEntryP->len;
      for (i = 0; i<= blues->numStemSnapV-1; ++i) {
          if (objPIsInteger(&HintEntryP->data.arrayP[i]))
              blues->StemSnapV[i] =
                  HintEntryP->data.arrayP[i].data.integer;
          else if (objPIsReal(&HintEntryP->data.arrayP[i]))
              blues->StemSnapV[i] =
                  HintEntryP->data.arrayP[i].data.real;
          else
              blues->StemSnapV[i] = 0;
      }
  }
 
  /* fill in the StdVW array */
  HintEntryP =  &(PrivateDictP[STDVW].value);
  /* check to see if the entry exists and if it's an array */
  if ( !objPIsArray(HintEntryP) || (HintEntryP->len == 0 ))
      /* a value of zero signifies no entry */
      blues->StdVW = 0;
  else {
      if (HintEntryP->len > NUMSTDVW) {
      }
      if (objPIsInteger(&HintEntryP->data.arrayP[0]))
          blues->StdVW = HintEntryP->data.arrayP[0].data.integer;
      else if (objPIsReal(&HintEntryP->data.arrayP[0]))
          blues->StdVW = HintEntryP->data.arrayP[0].data.real;
      else
          blues->StdVW = 0;
  }
 
  /* fill in the StdHW array */
  HintEntryP =  &(PrivateDictP[STDHW].value);
  /* check to see if the entry exists and if it's an array */
  if ( !objPIsArray(HintEntryP) || (HintEntryP->len == 0 ))
      /* a value of zero signifies no entry */
      blues->StdHW = 0;
  else {
      if (HintEntryP->len > NUMSTDHW) {
      }
          if (objPIsInteger(&HintEntryP->data.arrayP[0]))
             blues->StdHW = HintEntryP->data.arrayP[0].data.integer;
          else if (objPIsReal(&HintEntryP->data.arrayP[0]))
             blues->StdHW = HintEntryP->data.arrayP[0].data.real;
          else
             blues->StdHW = 0;
  }
 
 
  /* get the ptr to the BlueScale entry */
  HintEntryP =  &(PrivateDictP[BLUESCALE].value);
  /* put the BlueScale in the blues structure */
  if (objPIsInteger(HintEntryP)) /* Must be integer! */
      blues->BlueScale = HintEntryP->data.integer;
  else if (objPIsReal(HintEntryP)) /* Error? */
      blues->BlueScale = HintEntryP->data.real;
  else
      blues->BlueScale = DEFAULTBLUESCALE;
 
  /* get the ptr to the BlueShift entry */
  HintEntryP =  &(PrivateDictP[BLUESHIFT].value);
  if (objPIsInteger(HintEntryP)) /* Must be integer! */
      blues->BlueShift = HintEntryP->data.integer;
  else if (objPIsReal(HintEntryP)) /* Error? */
      blues->BlueShift = HintEntryP->data.real;
  else
      blues->BlueShift = DEFAULTBLUESHIFT;
 
  /* get the ptr to the BlueFuzz entry */
  HintEntryP =  &(PrivateDictP[BLUEFUZZ].value);
  if (objPIsInteger(HintEntryP)) /* Must be integer! */
      blues->BlueFuzz = HintEntryP->data.integer;
  else if (objPIsReal(HintEntryP)) /* Error? */
      blues->BlueFuzz = HintEntryP->data.real;
  else
      blues->BlueFuzz = DEFAULTBLUEFUZZ;
 
  /* get the ptr to the ForceBold entry */
  HintEntryP =  &(PrivateDictP[FORCEBOLD].value);
  if (objPIsBoolean(HintEntryP))  /* Must be integer! */
      blues->ForceBold = HintEntryP->data.boolean;
  else
      blues->ForceBold = DEFAULTFORCEBOLD;
 
  /* get the ptr to the LanguageGroup entry */
  HintEntryP =  &(PrivateDictP[LANGUAGEGROUP].value);
  if (objPIsInteger(HintEntryP)) /* Must be integer! */
      blues->LanguageGroup = HintEntryP->data.integer;
  else
      blues->LanguageGroup = DEFAULTLANGUAGEGROUP;
 
  /* get the ptr to the RndStemUp entry */
  HintEntryP =  &(PrivateDictP[RNDSTEMUP].value);
  if (objPIsBoolean(HintEntryP)) /* Must be integer! */
      blues->RndStemUp = HintEntryP->data.boolean;
  else
      blues->RndStemUp = DEFAULTRNDSTEMUP;
 
  /* get the ptr to the lenIV entry */
  HintEntryP =  &(PrivateDictP[LENIV].value);
  if (objPIsInteger(HintEntryP)) /* Must be integer! */
      blues->lenIV = HintEntryP->data.integer;
  else
      blues->lenIV = DEFAULTLENIV;
 
  /* get the ptr to the ExpansionFactor entry */
  HintEntryP =  &(PrivateDictP[EXPANSIONFACTOR].value);
  if (objPIsInteger(HintEntryP))
      blues->ExpansionFactor = HintEntryP->data.integer;
  else if (objPIsReal(HintEntryP))
      blues->ExpansionFactor = HintEntryP->data.real;
  else
      blues->ExpansionFactor = DEFAULTEXPANSIONFACTOR;
  return(SCAN_OK);
}
/**********************************************************************/
/*   GetType1CharString(fontP,code)                                   */
/*                                                                    */
/*          Look up code in the standard encoding vector and return   */
/*          the charstring associated with the character name.        */
/*                                                                    */
/*   fontP  is the psfont structure.                                  */
/*                                                                    */
/*   Returns a psobj (string)                                         */
/**********************************************************************/
psobj *
GetType1CharString(psfont *fontP, unsigned char code)
{
  int  N;           /* the 'Nth' entry in the CharStrings       */
  psobj *charnameP; /* points to psobj that is name of character*/
 
  psdict *CharStringsDictP; /* dictionary with char strings     */
  psobj  *theStringP;  /* the definition for the code */
 
 
 
  if (StdEncArrayP == NULL) {
    return(NULL);
  }
  /* use the code to index into the standard encoding vector  */
  charnameP = &(StdEncArrayP[code]);
 
  /* test if the encoding array points to a name */
  if (!(objPIsName(charnameP)) ) {
    return(NULL);
  }
 
  /* Now that we have the character name out of the standardencoding */
  /* get the character definition out of the current font */
  CharStringsDictP =  fontP->CharStringsP;
 
  /* search the chars string for this charname as key */
  N = SearchDictName(CharStringsDictP,charnameP);
  if (N<=0) {
    return(NULL);
  }
  /* OK, the nth item is the psobj that is the string for this char */
  theStringP = &(CharStringsDictP[N].value);
 
  return(theStringP);
}
 
/***================================================================***/
/*
 *   FindDictValue
 */
/***================================================================***/
 
static int 
FindDictValue(psdict *dictP)
{
   psobj LitName;
   int   N;
   int   V;
 
   /* we have just scanned a token and it is a literal name */
   /* need to check if that name is in Private dictionary */
   objFormatName(&LitName,tokenLength,tokenStartP);
   /* is it in the dictP */
   N = SearchDictName(dictP,&LitName);
   /* if found */
   if ( N > 0 ) {
     /* what type */
     switch (dictP[N].value.type) {
       case OBJ_ENCODING:
         V = getEncoding(&(dictP[N].value));
         if ( V != SCAN_OK ) return(V);
         break;
       case OBJ_ARRAY:
#ifdef BUILDCID
         if (0 == strncmp(tokenStartP,"FDArray",7))
             V = getFDArray(&(dictP[N].value));
         else
             V = getArray(&(dictP[N].value));
#else
         V = getArray(&(dictP[N].value));
#endif
         if ( V != SCAN_OK ) return(V);
         break;
       case OBJ_INTEGER:
         /* next value in integer */
         dictP[N].value.data.integer = getInt();
         if (rc) return(rc);  /* if next token was not an Int */
         break;
       case OBJ_REAL:
         /* next value must be real or int, store as a real */
         scan_token(inputP);
         if (tokenType == TOKEN_REAL) {
           dictP[N].value.data.real = tokenValue.real;
         }
         else
           if (tokenType == TOKEN_INTEGER) {
             dictP[N].value.data.real = tokenValue.integer;
           }
         else return(SCAN_ERROR);
         break;
       case OBJ_NAME:
         V = getNextValue(TOKEN_LITERAL_NAME);
         if ( V != SCAN_OK ) return(V);
         if (!(vm_alloc(tokenLength)) ) return(SCAN_OUT_OF_MEMORY);
         objFormatName(&(dictP[N].value),tokenLength,tokenStartP);
         break;
       case OBJ_STRING:
         V = getNextValue(TOKEN_STRING);
         if ( V != SCAN_OK ) return(V);
         if (!(vm_alloc(tokenLength)) ) return(SCAN_OUT_OF_MEMORY);
         objFormatString(&(dictP[N].value),tokenLength,tokenStartP);
         break;
       case OBJ_BOOLEAN:
         scan_token(inputP);
         if (tokenType != TOKEN_NAME) {
           return(SCAN_ERROR);
         }
         if (0 == strncmp(tokenStartP,"true",4) ) {
           dictP[N].value.data.boolean =TRUE;
         }
         else
           if (0 == strncmp(tokenStartP,"false",5) ) {
             dictP[N].value.data.boolean =FALSE;
           }
           else return(SCAN_ERROR);
         break;
 
       default:
         return(SCAN_ERROR);
     }
   }
   /* Name is not in dictionary.  That is ok. */
   return(SCAN_OK);
 
}
/***================================================================***/

#ifdef BUILDCID
/*
 * -------------------------------------------------------------------
 *  Scan the next token and convert it into an object
 *  Result is placed on the Operand Stack as next object
 * -------------------------------------------------------------------
 */
int 
scan_cidfont(cidfont *CIDFontP, cmapres *CMapP)
{
  char   filename[CID_PATH_MAX];
  char   cmapfile[CID_PATH_MAX];
  char   buf[CID_BUFSIZE];
  char   filetype[3];
  FILE   *fileP;
  FILE   *fileP1;
  char   *nameP;
  int    namelen;
  int    i, j;
  int    cread, rangecnt;
  unsigned int char_row, char_col;

    filetype[0] = 'r';
    filetype[1] = 'b';
    filetype[2] = '\0';

    /* copy the filename and remove leading or trailing blanks */
    /* point to name and search for leading blanks */
    nameP= CIDFontP->CIDFontFileName.data.nameP;
    namelen  = CIDFontP->CIDFontFileName.len;
    while (nameP[0] == ' ') {
        nameP++;
        namelen--;
    }
    /* now remove any trailing blanks */
    while ((namelen>0) && ( nameP[namelen-1] == ' ')) {
      namelen--;
    }
    strncpy(filename,nameP,namelen);
    filename[namelen] = '\0';
    /* file name is now constructed */
    inputFile.data.fileP = NULL;
    filterFile.data.fileP = NULL;

    /* check whether a CIDFont file */
    if ((fileP = fopen(filename,filetype))) {
        cread = fread(buf, 1, CID_BUFSIZE, fileP);
        fclose(fileP);
        if (cread > 17) {
            if (strncmp(buf, "%!", 2) ||
                strstr(buf, "Resource-CIDFont") == NULL)
                return(SCAN_FILE_OPEN_ERROR);
        } else
            return(SCAN_FILE_OPEN_ERROR);
    } else
        return(SCAN_FILE_OPEN_ERROR);

    /* copy the CMap file name and remove leading or trailing blanks */
    /* point to name and search for leading blanks                   */
    nameP = CMapP->CMapFileName.data.nameP;
    namelen  = CMapP->CMapFileName.len;
    while (nameP[0] == ' ') {
        nameP++;
        namelen--;
    }
    /* now remove any trailing blanks */
    while ((namelen>0) && ( nameP[namelen-1] == ' ')) {
      namelen--;
    }
    strncpy(cmapfile,nameP,namelen);
    cmapfile[namelen] = '\0';
    /* CMap file name is now constructed */
    inputFile1.data.fileP = NULL;

    /* check whether a CMap file */
    if ((fileP1 = fopen(cmapfile,filetype))) {
        cread = fread(buf, 1, CID_BUFSIZE, fileP1);
        fclose(fileP1);
        if (cread > 17) {
            if (strncmp(buf, "%!", 2) ||
                strstr(buf, "Resource-CMap") == NULL)
                return(SCAN_FILE_OPEN_ERROR);
        } else
            return(SCAN_FILE_OPEN_ERROR);
    } else
        return(SCAN_FILE_OPEN_ERROR);

  /* read the specified CMap file */
  inputP = &inputFile1;

  if (!(fileP1 = fopen(cmapfile,filetype)))
    return(SCAN_FILE_OPEN_ERROR);

  objFormatFile(inputP,fileP1);

  if ((rc = BuildCMapInfo(CMapP)) != 0)
    return(rc);

  /* Assume everything will be OK */
  rc = 0;
  rangecnt = 0;

  do {
    /* Scan the next token */
    scan_token(inputP);
    if (tokenType == TOKEN_INTEGER)
      rangecnt = tokenValue.integer;

    /* ==> tokenLength, tokenTooLong, tokenType, and */
    /* tokenValue are now set                        */

    switch (tokenType) {
      case TOKEN_EOF:
      case TOKEN_NONE:
      case TOKEN_INVALID:
        /* in this case we are done */
        if (tokenTooLong) return(SCAN_OUT_OF_MEMORY);
        rc = SCAN_ERROR;
        break;
      case TOKEN_LITERAL_NAME:
        /* Look up the name */
        tokenStartP[tokenLength] = '\0';

        rc = FindDictValue(CMapP->CMapInfoP);
        /* we are not going to report errors except out of memory */
        if (rc != SCAN_OUT_OF_MEMORY)
          rc = SCAN_OK;
        break;
      case TOKEN_NAME:
        if (0 == strncmp(tokenStartP,"begincodespacerange",19)) {
          CIDFontP->spacerangecnt++;
          spacerangeP = (spacerange *)vm_alloc(sizeof(spacerange));
          if (!spacerangeP) {
            rc = SCAN_OUT_OF_MEMORY;
            break;
          }
          spacerangeP->next = NULL;
          spacerangeP->rangecnt = rangecnt;
          spacerangeP->spacecode =
            (spacerangecode *)vm_alloc(rangecnt*sizeof(spacerangecode));
          if (!spacerangeP->spacecode) {
            rc = SCAN_OUT_OF_MEMORY;
            break;
          }
          for (i = 0; i < rangecnt; i++) {
            scan_token(inputP);
            if (tokenType != TOKEN_HEX_STRING) {
              rc = SCAN_ERROR;
              break;
            }
            spacerangeP->spacecode[i].srcCodeLo = 0;
            for (j = 0; j < tokenLength; j++)
              spacerangeP->spacecode[i].srcCodeLo +=
                (unsigned char)tokenStartP[j] << (8 * (tokenLength - 1 - j));

            scan_token(inputP);
            if (tokenType != TOKEN_HEX_STRING) {
              rc = SCAN_ERROR;
              break;
            }
            spacerangeP->spacecode[i].srcCodeHi = 0;
            for (j = 0; j < tokenLength; j++)
              spacerangeP->spacecode[i].srcCodeHi +=
                (unsigned char)tokenStartP[j] << (8 * (tokenLength - 1 - j));
          }

          if (CIDFontP->spacerangeP) {
            if (CIDFontP->spacerangeP->next == NULL)
              CIDFontP->spacerangeP->next = spacerangeP;
            else {
              spacerangeP->next = CIDFontP->spacerangeP->next;
              CIDFontP->spacerangeP->next = spacerangeP;
            }
          } else
            CIDFontP->spacerangeP = spacerangeP;
 
          /* read "endcodespacerange" */
          scan_token(inputP);
          if (tokenType != TOKEN_NAME || (tokenType == TOKEN_NAME &&
            (strncmp(tokenStartP,"endcodespacerange",17) != 0))) {
            rc = SCAN_ERROR;
            break;
          }
        }
        if (0 == strncmp(tokenStartP,"begincidrange",13)) {
          CIDFontP->cidrangecnt++;
          cidrangeP = (cidrange *)vm_alloc(sizeof(cidrange));
          if (!cidrangeP) {
            rc = SCAN_OUT_OF_MEMORY;
            break;
          }
          cidrangeP->next = NULL;
          cidrangeP->rangecnt = rangecnt;
          cidrangeP->range =
            (cidrangecode *)vm_alloc(rangecnt*sizeof(cidrangecode));
          if (!cidrangeP->range) {
            rc = SCAN_OUT_OF_MEMORY;
            break;
          }
          for (i = 0; i < rangecnt; i++) {
            scan_token(inputP);
            if (tokenType != TOKEN_HEX_STRING) {
              rc = SCAN_ERROR;
              break;
            }
            cidrangeP->range[i].srcCodeLo = 0;
            for (j = 0; j < tokenLength; j++)
              cidrangeP->range[i].srcCodeLo +=
                (unsigned char)tokenStartP[j] << (8 * (tokenLength - 1 - j));
            char_row = (cidrangeP->range[i].srcCodeLo >> 8) & 0xff;
            char_col = cidrangeP->range[i].srcCodeLo & 0xff;
            if (char_row < CMapP->firstRow)
              CMapP->firstRow = char_row;
            if (char_row > CMapP->lastRow)
              CMapP->lastRow = char_row;
            if (char_col < CMapP->firstCol)
              CMapP->firstCol = char_col;
            if (char_col > CMapP->lastCol)
              CMapP->lastCol = char_col;
            scan_token(inputP);
            if (tokenType != TOKEN_HEX_STRING) {
              rc = SCAN_ERROR;
              break;
            }
            cidrangeP->range[i].srcCodeHi = 0;
            for (j = 0; j < tokenLength; j++)
              cidrangeP->range[i].srcCodeHi +=
                (unsigned char)tokenStartP[j] << (8 * (tokenLength - 1 - j));
            char_row = (cidrangeP->range[i].srcCodeHi >> 8) & 0xff;
            char_col = cidrangeP->range[i].srcCodeHi & 0xff;
            if (char_row < CMapP->firstRow)
              CMapP->firstRow = char_row;
            if (char_row > CMapP->lastRow)
              CMapP->lastRow = char_row;
            if (char_col < CMapP->firstCol)
              CMapP->firstCol = char_col;
            if (char_col > CMapP->lastCol)
              CMapP->lastCol = char_col;
            scan_token(inputP);
            if (tokenType != TOKEN_INTEGER) {
              rc = SCAN_ERROR;
              break;
            }
            cidrangeP->range[i].dstCIDLo = tokenValue.integer;
          }

          if (CIDFontP->cidrangeP) {
            if (CIDFontP->cidrangeP->next == NULL)
              CIDFontP->cidrangeP->next = cidrangeP;
            else {
              cidrangeP->next = CIDFontP->cidrangeP->next;
              CIDFontP->cidrangeP->next = cidrangeP;
            }
          } else
            CIDFontP->cidrangeP = cidrangeP;

          /* read "endcidrange" */
          scan_token(inputP);
          if (tokenType != TOKEN_NAME || (tokenType == TOKEN_NAME &&
            (strncmp(tokenStartP,"endcidrange",11) != 0))) {
            rc = SCAN_ERROR;
            break;
          }
        }

        if (0 == strncmp(tokenStartP,"beginnotdefrange",16)) {
          CIDFontP->notdefrangecnt++;
          notdefrangeP = (cidrange *)vm_alloc(sizeof(cidrange));
          if (!notdefrangeP) {
            rc = SCAN_OUT_OF_MEMORY;
            break;
          }
          notdefrangeP->next = 0;
          notdefrangeP->rangecnt = rangecnt;
          notdefrangeP->range =
            (cidrangecode *)vm_alloc(rangecnt*sizeof(cidrangecode));
          if (!notdefrangeP->range) {
            rc = SCAN_OUT_OF_MEMORY;
            break;
          }
          for (i = 0; i < rangecnt; i++) {
            scan_token(inputP);
            if (tokenType != TOKEN_HEX_STRING) {
              rc = SCAN_ERROR;
              break;
            }
            notdefrangeP->range[i].srcCodeLo = 0;
            for (j = 0; j < tokenLength; j++)
              notdefrangeP->range[i].srcCodeLo = (int)(tokenStartP[j] <<
                (8 * (tokenLength - 1 - j)));
            scan_token(inputP);
            if (tokenType != TOKEN_HEX_STRING) {
              rc = SCAN_ERROR;
              break;
            }
            notdefrangeP->range[i].srcCodeHi = 0;
            for (j = 0; j < tokenLength; j++)
              notdefrangeP->range[i].srcCodeHi = (int)(tokenStartP[j] <<
                (8 * (tokenLength - 1 - j)));
            scan_token(inputP);
            if (tokenType != TOKEN_INTEGER) {
              rc = SCAN_ERROR;
              break;
            }
            notdefrangeP->range[i].dstCIDLo = tokenValue.integer;
          }
          if (CIDFontP->notdefrangeP) {
            if (CIDFontP->notdefrangeP->next == NULL)
              CIDFontP->notdefrangeP->next = notdefrangeP;
            else {
              notdefrangeP->next = CIDFontP->notdefrangeP->next;
              CIDFontP->notdefrangeP->next = notdefrangeP;
            }
          } else
            CIDFontP->notdefrangeP = notdefrangeP;

          /* read "endnotdefrange" */
          scan_token(inputP);
          if (tokenType != TOKEN_NAME || (tokenType == TOKEN_NAME &&
            (strncmp(tokenStartP,"endnotdefrange",14) != 0))) {
            rc = SCAN_ERROR;
            break;
          }
        }

        if (0 == strncmp(tokenStartP,"endcmap",7)) {
          if (CMapP->CMapInfoP[CMAPREGISTRY].value.data.valueP == NULL ||
            CMapP->CMapInfoP[CMAPORDERING].value.data.valueP == NULL ||
            CMapP->CMapInfoP[CMAPSUPPLEMENT].value.data.integer == -1) {
            rc = SCAN_ERROR;
            break;
          } else {
            rc = SCAN_FILE_EOF;
            break;
          }
        }
        break;
    }
  }
  while (rc == 0);
  fclose(inputP->data.fileP);
  if (tokenTooLong)
      rc = SCAN_OUT_OF_MEMORY;
  if (rc == SCAN_OUT_OF_MEMORY) return(rc);

  /* open the specified CIDFont file */
  if (!(fileP = fopen(filename,filetype)))
      return(SCAN_FILE_OPEN_ERROR);

  inputP = &inputFile;
  objFormatFile(inputP,fileP);
  CIDWantFontInfo  = TRUE;
  TwoSubrs      = FALSE;
  rc = BuildCIDFontInfo(CIDFontP);
  if (rc != 0) return(rc);

  /* Assume everything will be OK */
  rc = 0;

  /* Loop until complete font is read  */
  do {
    /* Scan the next token */
    scan_token(inputP);

    /* ==> tokenLength, tokenTooLong, tokenType, and tokenValue are */
    /* now set */

    switch (tokenType) {
      case TOKEN_EOF:
      case TOKEN_NONE:
      case TOKEN_INVALID:
        /* in this case we are done */
        if (tokenTooLong) return(SCAN_OUT_OF_MEMORY);
        rc = SCAN_ERROR;
        break;
      case TOKEN_LITERAL_NAME:
        /* Look up the name */
        tokenStartP[tokenLength] = '\0';

         if (CIDWantFontInfo) {
             rc = FindDictValue(CIDFontP->CIDfontInfoP);
             /* we are not going to report errors except out of memory */
             if (rc != SCAN_OUT_OF_MEMORY)
               rc = SCAN_OK;
             break;
         }
        break;
      case TOKEN_STRING:
        tokenStartP[tokenLength] = '\0';
        if (0 == strncmp(tokenStartP,"Binary",6)) {
          CIDFontP->binarydata = 1;
          scan_token(inputP);
          if (tokenType == TOKEN_INTEGER)
            CIDFontP->bytecnt = tokenValue.integer;
          else {
            rc = SCAN_ERROR;
            break;
          }
        } else if (0 == strncmp(tokenStartP,"Hex",3)) {
          /* not yet supported */
          rc = SCAN_ERROR;
          break;
#if 0
          /* uncomment when the hex format is supported */
          CIDFontP->binarydata = 0;
          scan_token(inputP);
          if (tokenType == TOKEN_INTEGER)
            CIDFontP->bytecnt = tokenValue.integer;
          else {
            rc = SCAN_ERROR;
            break;
          }
#endif
        }
        break;
      case TOKEN_NAME:
        /* end of PostScript and beginning of data */
        if (0 == strncmp(tokenStartP,"StartData",9)) {
          /* every CIDFont must have an FDArray */
          /* check whether other required dictionary entries were found */
          if (CIDFontP->CIDfontInfoP[CIDFDARRAY].value.data.arrayP == NULL ||
            CIDFontP->CIDfontInfoP[CIDFONTNAME].value.data.nameP == NULL ||
            CIDFontP->CIDfontInfoP[CIDFONTTYPE].value.data.integer == -1 ||
            CIDFontP->CIDfontInfoP[CIDVERSION].value.data.integer == -1 ||
            CIDFontP->CIDfontInfoP[CIDREGISTRY].value.data.valueP == NULL ||
            CIDFontP->CIDfontInfoP[CIDORDERING].value.data.valueP == NULL ||
            CIDFontP->CIDfontInfoP[CIDSUPPLEMENT].value.data.integer == -1 ||
            CIDFontP->CIDfontInfoP[CIDFONTBBOX].value.data.arrayP == NULL ||
            CIDFontP->CIDfontInfoP[CIDMAPOFFSET].value.data.integer == -1 ||
            CIDFontP->CIDfontInfoP[CIDFDBYTES].value.data.integer == -1 ||
            CIDFontP->CIDfontInfoP[CIDGDBYTES].value.data.integer == -1 ||
            CIDFontP->CIDfontInfoP[CIDCOUNT].value.data.integer == -1) {
            rc = SCAN_ERROR;
            break;
          } else {
            /* do Registry and Ordering entries match? */
            if (strcmp(CIDFontP->CIDfontInfoP[CIDREGISTRY].value.data.valueP,
              CMapP->CMapInfoP[CMAPREGISTRY].value.data.valueP) != 0 ||
              strcmp(CIDFontP->CIDfontInfoP[CIDORDERING].value.data.valueP,
              CMapP->CMapInfoP[CMAPORDERING].value.data.valueP) != 0) {
              rc = SCAN_ERROR;
              break;
            } else {
              fclose(inputP->data.fileP);
              return(SCAN_OK);
            }
          }
        }
        break;
    }

  }
  while (rc ==0);
  fclose(inputP->data.fileP);
  if (tokenTooLong) return(SCAN_OUT_OF_MEMORY);
  return(rc);
}

/*
 * -------------------------------------------------------------------
 *  Scan the next token and convert it into an object
 *  Result is placed on the Operand Stack as next object
 * -------------------------------------------------------------------
 */
int 
scan_cidtype1font(psfont *FontP)
{
  int    i;
  int    begincnt = 0; /* counter for the number of unpaired begin operators */
  int    currentfilefound = 0;

  WantFontInfo  = TRUE;
  InPrivateDict = FALSE;
  TwoSubrs      = FALSE;
  rc = BuildFontInfo(FontP);
  if (rc != 0) return(rc);

  /* Assume everything will be OK */
  rc       = 0;
  filterFile.data.fileP = NULL;

  /* Loop until complete font is read  */
  do {
    /* Scan the next token */
    scan_token(inputP);

    /* ==> tokenLength, tokenTooLong, tokenType, and tokenValue are */
    /* now set */

    switch (tokenType) {
      case TOKEN_EOF:
      case TOKEN_NONE:
      case TOKEN_INVALID:
        /* in this case we are done */
        if (tokenTooLong) return(SCAN_OUT_OF_MEMORY);
        rc = SCAN_ERROR;
        break;
      case TOKEN_LITERAL_NAME:
            /* Look up the name */
            tokenStartP[tokenLength] = '\0';
            if (InPrivateDict ) {
              rc = FindDictValue(FontP->Private);
              /* we are not going to report errors */
              /* Sometimes the font file may test a value such as */
              /* testing to see if the font is alreadly loaded with */
              /* same UniqueID.  We would faile on /UniqueID get  */
              /* because we are expecting a int to follow UniqueID*/
              /* If the correct object type does not follow a Name*/
              /* then we will skip over it without reporting error except */
              /* out of memory */
              if (rc != SCAN_OUT_OF_MEMORY)
                rc = SCAN_OK;
              break;
            }   /* end of reading Private dictionary */
            else
              if (0 == strncmp(tokenStartP,"Private",7) ) {
                InPrivateDict = TRUE;
                rc = BuildCIDType1Private(FontP);
                break;
              }
              else
                if (WantFontInfo) {
                  rc = FindDictValue(FontP->fontInfoP);
                  /* we are not going to report errors except out of memory */
                  if (rc != SCAN_OUT_OF_MEMORY)
                    rc = SCAN_OK;
                  break;
                }
        break;
      case TOKEN_NAME:
            if (0 == strncmp(tokenStartP,"currentfile",11)) {
                currentfilefound = 1;
                break;
            } else if (0 == strncmp(tokenStartP,"eexec",5)) {
                if (currentfilefound == 1) {
                    currentfilefound = 0;
                    filterFile.data.fileP = CIDeexec(inputP->data.fileP);
                    if (filterFile.data.fileP == NULL) {
                      fclose(inputFile.data.fileP);
                      return(SCAN_FILE_OPEN_ERROR);
                    }
                    inputP = &filterFile;
                } else {
                    rc = SCAN_ERROR;
                    break;
                }
            } else if (0 == strncmp(tokenStartP,"begin",5)) {
                begincnt++;
                currentfilefound = 0;
            } else if (0 == strncmp(tokenStartP,"end",3)) {
                currentfilefound = 0;
                begincnt--;
                if (begincnt == 0) {
                    if (filterFile.data.fileP != NULL) {
                        scan_token(inputP); /* get 'currentfile' */
                        scan_token(inputP); /* get 'closefile' */
                        inputP = &inputFile;
                        resetDecrypt();
                        inputP->data.fileP->b_cnt =
                            F_BUFSIZ - (inputP->data.fileP->b_ptr -
                            inputP->data.fileP->b_base);
                        if (inputP->data.fileP->b_cnt > 0) {
                            for (i = 0; i < inputP->data.fileP->b_cnt; i++)
                                if (*(inputP->data.fileP->b_ptr + i) == '%')
                                    break;
                            if (i < inputP->data.fileP->b_cnt) {
                                inputP->data.fileP->b_cnt -= i;
                                inputP->data.fileP->b_ptr += i;
                            } else
                                inputP->data.fileP->b_cnt = 0;
                        }
                    }
                    rc = SCAN_OK;
                    return(rc);
                }
                if (begincnt < 0) {
                    rc = SCAN_ERROR;
                    break;
                }
            }
        break;
    }

  }
  while (rc == 0);
  if (tokenTooLong) return(SCAN_OUT_OF_MEMORY);
  return(rc);
}
#endif

/*
 * -------------------------------------------------------------------
 *  Scan the next token and convert it into an object
 *  Result is placed on the Operand Stack as next object
 * -------------------------------------------------------------------
 */
int 
scan_font(psfont *FontP)
{
 
 
  char   filename[128];
  char   filetype[3];
  FILE   *fileP;
  char   *nameP;
  int    namelen;
  int    V;
  int    i;
  boolean starthex80;
 
    starthex80 = FALSE;
    filetype[0] = 'r';
    filetype[1] = 'b';
    filetype[2] = '\0';
    /* copy the filename and remove leading or trailing blanks */
    /* point to name and search for leading blanks */
    nameP= FontP->FontFileName.data.nameP;
    namelen  = FontP->FontFileName.len;
    while (nameP[0] == ' ') {
        nameP++;
        namelen--;
    }
    /* now remove any trailing blanks */
    while ((namelen>0) && ( nameP[namelen-1] == ' ')) {
      namelen--;
    }
    strncpy(filename,nameP,namelen);
    filename[namelen] = '\0';
    /* file name is now constructed */
    inputFile.data.fileP = NULL;
    filterFile.data.fileP = NULL;
 
    inputP = &inputFile;
    if ((fileP = T1Open(filename,filetype))) {
      /* get the first byte of file */
      V = _XT1getc(fileP);
      /* if file starts with x'80' then skip next 5 bytes */
      if ( V == 0X80 ) {
        for (i=0;i<5;i++) V = _XT1getc(fileP);
        starthex80 = TRUE;
      }
      else T1Ungetc(V,fileP);
      objFormatFile(inputP,fileP);
    }
    else {
      return(SCAN_FILE_OPEN_ERROR);
    };
 
  WantFontInfo  = TRUE;
  InPrivateDict = FALSE;
  TwoSubrs      = FALSE;
  rc = BuildFontInfo(FontP);
  if (rc != 0) return(rc);
 
  /* Assume everything will be OK */
  rc       = 0;
 
  /* Loop until complete font is read  */
  do {
    /* Scan the next token */
    scan_token(inputP);
 
    /* ==> tokenLength, tokenTooLong, tokenType, and tokenValue are */
    /* now set */
 
    switch (tokenType) {
      case TOKEN_EOF:
      case TOKEN_NONE:
      case TOKEN_INVALID:
        /* in this case we are done */
        if (tokenTooLong) return(SCAN_OUT_OF_MEMORY);
        rc = SCAN_ERROR;
        break;
      case TOKEN_LITERAL_NAME:
            /* Look up the name */
            tokenStartP[tokenLength] = '\0';
            if (InPrivateDict ) {
              if (0== strncmp(tokenStartP,"Subrs",5) ) {
                rc = BuildSubrs(FontP);
                break;
              }
              if (0== strncmp(tokenStartP,"CharStrings",11) ) {
                rc = BuildCharStrings(FontP);
                if ( (rc == SCAN_OK) ||(rc == SCAN_END) ) {
                  T1Close(inputP->data.fileP);
                  /* Build the Blues Structure */
                  rc = GetType1Blues(FontP);
                  /* whatever the return code, return it */
                  /* all the work is done. This is the normal exit.*/
                  return(rc);
                }
                break;
              }
              rc = FindDictValue(FontP->Private);
              /* we are not going to report errors */
              /* Sometimes the font file may test a value such as */
              /* testing to see if the font is alreadly loaded with */
              /* same UniqueID.  We would faile on /UniqueID get  */
              /* because we are expecting a int to follow UniqueID*/
              /* If the correct object type does not follow a Name*/
              /* then we will skip over it without reporting error except */
              /* when out of memory */
              if (rc != SCAN_OUT_OF_MEMORY)
                rc = SCAN_OK;
              break;
            }   /* end of reading Private dictionary */
            else
              if (0== strncmp(tokenStartP,"Private",7) ) {
                InPrivateDict = TRUE;
                rc = BuildPrivate(FontP);
                break;
              }
              else
                if (WantFontInfo) {
                  rc = FindDictValue(FontP->fontInfoP);
                  /* we are not going to report errors except out of memory */
                  if (rc != SCAN_OUT_OF_MEMORY)
                    rc = SCAN_OK;
                  break;
                }
        break;
      case TOKEN_NAME:
            if (0 == strncmp(tokenStartP,"eexec",5) ) {
               /* if file started with x'80', check next 5 bytes */
               if (starthex80) {
                 V = _XT1getc(fileP);
                 if ( V == 0X80 ) {
                   for (i=0;i<5;i++) V = _XT1getc(fileP);
                 }
                 else T1Ungetc(V,fileP);
               }
               filterFile.data.fileP = T1eexec(inputP->data.fileP);
               if (filterFile.data.fileP == NULL) {
                 T1Close(inputFile.data.fileP);
                 return(SCAN_FILE_OPEN_ERROR);
               }
               inputP = &filterFile;
 
               WantFontInfo = FALSE;
            }
        break;
    }
 
  }
  while (rc ==0);
  T1Close(inputP->data.fileP);
  if (tokenTooLong) return(SCAN_OUT_OF_MEMORY);
  return(rc);
}
 
