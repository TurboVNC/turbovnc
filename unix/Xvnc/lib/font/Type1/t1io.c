/* $Xorg: t1io.c,v 1.3 2000/08/17 19:46:33 cpqbld Exp $ */
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
 * Author: Carol H. Thompson  IBM Almaden Research Center
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
/* $XFree86: xc/lib/font/Type1/t1io.c,v 3.9 2001/08/27 19:49:54 dawes Exp $ */
/*******************************************************************
*  I/O package for Type 1 font reading
********************************************************************/

#ifndef STATIC
#define STATIC static
#endif
 
#ifndef FONTMODULE
#include <fcntl.h>
#include <unistd.h>
#else
#include "Xdefs.h"	/* Bool declaration */
#include "Xmd.h"	/* INT32 declaration */
#include "xf86_ansic.h"
#endif
#include "t1stdio.h"
#include "t1hdigit.h"
#ifdef WIN32
#include <X11/Xw32defs.h>
#endif
#include "Xdefs.h"

/* Constants and variables used in the decryption */
#define c1 ((unsigned short)52845)
#define c2 ((unsigned short)22719)
static unsigned short r;
static int asc, Decrypt;
static int extrach;
static int haveextrach;
 
/* Our single FILE structure and buffer for this package */
STATIC F_FILE TheFile;
STATIC unsigned char TheBuffer[F_BUFSIZ];
 
/* Our routines */
static int T1Decrypt ( unsigned char *p, int len );
static int T1Fill ( F_FILE *f );

#ifdef BUILDCID
void 
resetDecrypt(void)
{
    Decrypt = 0;
}
#endif
 
/* -------------------------------------------------------------- */
/*ARGSUSED*/
F_FILE *
T1Open(char *fn,   /* Pointer to filename */
       char *mode) /* Pointer to open mode string */
{
  F_FILE *of = &TheFile;
  int oflags = O_RDONLY;	/* We know we are only reading */

  Decrypt = 0;
 
#ifdef O_BINARY			/* VMS or DOS */
  oflags |= O_BINARY;
#endif
  of->fd = open(fn, oflags, 0);
  if (of->fd < 0)
      return NULL;
 
  /* Initialize the buffer information of our file descriptor */
  of->b_base = TheBuffer;
  of->b_size = F_BUFSIZ;
  of->b_ptr = NULL;
  of->b_cnt = 0;
  of->flags = 0;
  of->error = 0;
  haveextrach = 0;
  return &TheFile;
} /* end Open */
 
/* -------------------------------------------------------------- */
int                  /* Read one character */
T1Getc(F_FILE *f)    /* Stream descriptor */
{
  if (f->b_base == NULL) return EOF;  /* already closed */
 
  if (f->flags & UNGOTTENC) { /* there is an ungotten c */
    f->flags &= ~UNGOTTENC;
    return (int) f->ungotc;
  }
 
  if (f->b_cnt == 0)  /* Buffer needs to be (re)filled */
    f->b_cnt = T1Fill(f);
  if (f->b_cnt > 0) return (f->b_cnt--, (int) *(f->b_ptr++));
  else {
    f->flags |= FIOEOF;
    return EOF;
  }
} /* end Getc */
 
/* -------------------------------------------------------------- */
int                  /* Put back one character */
T1Ungetc(int c, 
	 F_FILE *f)  /* Stream descriptor */ 
{
  if (c != EOF) {
    f->ungotc = c;
    f->flags |= UNGOTTENC;  /* set flag */
    f->flags &= ~FIOEOF;    /* reset EOF */
  }
  return c;
} /* end Ungetc */
 
/* -------------------------------------------------------------- */
int                  /* Read n items into caller's buffer */
T1Read(char *buffP,  /* Buffer to be filled */
       int size,     /* Size of each item */
       int n,        /* Number of items to read */
       F_FILE *f)    /* Stream descriptor */
{
  int bytelen, cnt, i;
  F_char *p = (F_char *)buffP;
  int  icnt;         /* Number of characters to read */
 
  if (f->b_base == NULL) return 0;  /* closed */
  icnt = (size!=1)?n*size:n;  /* Number of bytes we want */
 
  if (f->flags & UNGOTTENC) { /* there is an ungotten c */
    f->flags &= ~UNGOTTENC;
    *(p++) = f->ungotc;
    icnt--; bytelen = 1;
  }
  else bytelen = 0;
 
  while (icnt > 0) {
    /* First use any bytes we have buffered in the stream buffer */
    if ((cnt=f->b_cnt) > 0) {
      if (cnt > icnt) cnt = icnt;
      for (i=0; i<cnt; i++) *(p++) = *(f->b_ptr++);
      f->b_cnt -= cnt;
      icnt -= cnt;
      bytelen += cnt;
    }
 
    if ((icnt == 0) || (f->flags & FIOEOF)) break;
 
    f->b_cnt = T1Fill(f);
  }
  return ((size!=1)?bytelen/size:bytelen);
} /* end Read */
 
/* -------------------------------------------------------------- */
int                  /* Close the file */
T1Close(F_FILE *f)   /* Stream descriptor */    
{
  if (f->b_base == NULL) return 0;  /* already closed */
  f->b_base = NULL;  /* no valid stream */
  return close(f->fd);
} /* end Close */

 
/* -------------------------------------------------------------- */
F_FILE *             /* Initialization */
T1eexec(F_FILE *f)   /* Stream descriptor */
{
  int i, c;
  int H;
  unsigned char *p;
  unsigned char randomP[8];

  r = 55665;  /* initial key */
  asc = 1;    /* indicate ASCII form */

  /* Consume the 4 random bytes, determining if we are also to
     ASCIIDecodeHex as we process our input.  (See pages 63-64
     of the Adobe Type 1 Font Format book.)  */

  /* Skip over any initial white space chars */
  while (HighHexP[c=_XT1getc(f)] == HWHITE_SPACE) ;

  /* If ASCII, the next 7 chars are guaranteed consecutive */
  randomP[0] = c;  /* store first non white space char */
  T1Read((pointer)(randomP+1), 1, 3, f);  /* read 3 more, for a total of 4 */
  /* store first four chars */
  for (i=0,p=randomP; i<4; i++) {  /* Check 4 valid ASCIIEncode chars */
    if (HighHexP[*p++] > LAST_HDIGIT) {  /* non-ASCII byte */
      asc = 0;
      break;
    }
  }
  if (asc) {  /* ASCII form, convert first eight bytes to binary */
    T1Read((pointer)(randomP+4), 1, 4, f);  /* Need four more */
    for (i=0,p=randomP; i<4; i++) {  /* Convert */
      H = HighHexP[*p++];
      randomP[i] = H | LowHexP[*p++];
    }
  }

  /* Adjust our key */
  for (i=0,p=randomP; i<4; i++) {
    r = (*p++ + r) * c1 + c2;
  }

  /* Decrypt the remaining buffered bytes */
  f->b_cnt = T1Decrypt(f->b_ptr, f->b_cnt);
  Decrypt = 1;
  return (T1Feof(f))?NULL:f;
} /* end eexec */

#ifdef BUILDCID
F_FILE *             /* Initialization */
CIDeexec(F_FILE *f)  /* Stream descriptor */ 
{
  int i, c;
  int H;
  unsigned char *p;
  unsigned char randomP[8];
 
  r = 55665;  /* initial key */
  asc = 1;    /* indicate ASCII form */
 
  /* Consume the 4 random bytes, determining if we are also to
     ASCIIDecodeHex as we process our input.  (See pages 63-64
     of the Adobe Type 1 Font Format book.)  */
 
  /* Skip over any initial white space chars */
  while (HighHexP[c=_XT1getc(f)] == HWHITE_SPACE) ;
 
  /* If ASCII, the next 7 chars are guaranteed consecutive */
  randomP[0] = c;  /* store first non white space char */
  T1Read((pointer)(randomP+1), 1, 3, f);  /* read 3 more, for a total of 4 */
  /* store first four chars */
  for (i=0,p=randomP; i<4; i++) {  /* Check 4 valid ASCIIEncode chars */
    if (HighHexP[*p++] > LAST_HDIGIT) {  /* non-ASCII byte */
      asc = 0;
      break;
    }
  }
  if (asc) {  /* ASCII form, convert first eight bytes to binary */
    T1Read((pointer)(randomP+4), 1, 4, f);  /* Need four more */
    for (i=0,p=randomP; i<4; i++) {  /* Convert */
      H = HighHexP[*p++];
      randomP[i] = H | LowHexP[*p++];
    }
  }
 
  /* Adjust our key */
  for (i=0,p=randomP; i<4; i++) {
    r = (*p++ + r) * c1 + c2;
  }
 
  /* Decrypt up to, but not including, the first '%' sign */
  if (f->b_cnt > 0) {
      for (i = 0; i < f->b_cnt; i++)
         if (*(f->b_ptr + i) == '%')
             break;

      if (i < f->b_cnt) {
          if (i == 0)
              f->b_cnt = 0;
          else
              f->b_cnt = T1Decrypt(f->b_ptr, i);
      } else
          f->b_cnt = T1Decrypt(f->b_ptr, f->b_cnt);
  }
  Decrypt = 1;
  return (T1Feof(f))?NULL:f;
} /* end eexec */
#endif

/* -------------------------------------------------------------- */
STATIC int 
T1Decrypt(unsigned char *p, int len)
{
  int n;
  int H = 0, L;
  unsigned char *inp = p;
  unsigned char *tblP;
 
  if (asc) {
    if (haveextrach) {
      H = extrach;
      tblP = LowHexP;
    }
    else tblP = HighHexP;
    for (n=0; len>0; len--) {
      L = tblP[*inp++];
      if (L == HWHITE_SPACE) continue;
      if (L > LAST_HDIGIT) break;
      if (tblP == HighHexP) { /* Got first hexit value */
        H = L;
        tblP = LowHexP;
      } else { /* Got second hexit value; compute value and store it */
        n++;
        tblP = HighHexP;
        H |= L;
        /* H is an int, 0 <= H <= 255, so all of this will work */
        *p++ = H ^ (r >> 8);
        r = (H + r) * c1 + c2;
      }
    }
    if (tblP != HighHexP) {  /* We had an odd number of hexits */
      extrach = H;
      haveextrach = 1;
    } else haveextrach = 0;
    return n;
  } else {
    for (n = len; n>0; n--) {
      H = *inp++;
      *p++ = H ^ (r >> 8);
      r = (H + r) * c1 + c2;
    }
    return len;
  }
} /* end Decrypt */
 
/* -------------------------------------------------------------- */
STATIC int           /* Refill stream buffer */ 
T1Fill(F_FILE *f)    /* Stream descriptor */
{
  int rc;

  rc = read(f->fd, f->b_base, F_BUFSIZ);
  /* propagate any error or eof to current file */
  if (rc <= 0) {
    if (rc == 0)    /* means EOF */
      f->flags |= FIOEOF;
    else {
      f->error = (short)-rc;
      f->flags |= FIOERROR;
      rc = 0;
    }
  }
  f->b_ptr = f->b_base;
  if (Decrypt) rc = T1Decrypt(f->b_base, rc);
  return rc;
} /* end Fill */
