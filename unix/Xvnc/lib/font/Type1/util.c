/* $Xorg: util.c,v 1.3 2000/08/17 19:46:34 cpqbld Exp $ */
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
/* $XFree86: xc/lib/font/Type1/util.c,v 1.6 2001/01/17 19:43:24 dawes Exp $ */
/* Author: Katherine A. Hitchcock    IBM Almaden Research Laboratory */
 
#ifndef FONTMODULE
#include <stdio.h>
#else
#include "Xdefs.h"
#include "Xmd.h"
#include "xf86_ansic.h"
#endif
#include "util.h"
#include "fontmisc.h"			/* for xalloc/xfree */
 
static char *vm_base = NULL;  /* Start of virtual memory area */
       char *vm_next = NULL;  /* Pointer to first free byte */
       long  vm_free = 0;     /* Count of free bytes */
       long  vm_size = 0;     /* Total size of memory */
 
/*
 * Initialize memory.
 */
boolean 
vm_init(int cnt)
{
#ifdef BUILDCID
  if (vm_base == NULL || (vm_base != NULL && vm_size != cnt)) {
      if (vm_base != NULL) xfree(vm_base);
      vm_next = vm_base = (char *)xalloc (cnt);
  } else
      vm_next = vm_base;
#else
  vm_next = vm_base = (char *)xalloc (cnt);
#endif
 
  if (vm_base != NULL) {
    vm_free = cnt;
    vm_size = cnt;
    return(TRUE);
  }
  else
    return(FALSE);
 
}
 
char *
vm_alloc(int bytes)
{
  char *answer;
 
  /* Round to next word multiple */
  bytes = (bytes + 7) & ~7;
 
  /* Allocate the space, if it is available */
  if (bytes <= vm_free) {
    answer = vm_next;
    vm_free -= bytes;
    vm_next += bytes;
  }
  else
    answer = NULL;
 
  return(answer);
}
 
/*
 * Format an Integer object
 */
void 
objFormatInteger(psobj *objP, int value)
{
  if (objP != NULL) {
    objP->type         = OBJ_INTEGER;
    objP->len          = 0;
    objP->data.integer = value;
  }
}
 
/*
 * Format a Real object
 */
void 
objFormatReal(psobj *objP, float value)
{
  if (objP != NULL) {
    objP->type       = OBJ_REAL;
    objP->len        = 0;
    objP->data.real  = value;
  }
}
 
/*
 * Format a Boolean object
 */
void 
objFormatBoolean(psobj *objP, boolean value)
{
  if (objP != NULL) {
    objP->type         = OBJ_BOOLEAN;
    objP->len          = 0;
    objP->data.boolean = value;
  }
}
 
/*
 * Format an Encoding object
 */
void 
objFormatEncoding(psobj *objP, int length, psobj *valueP)
{
  if (objP != NULL) {
    objP->type        = OBJ_ENCODING;
    objP->len         = length;
    objP->data.arrayP = valueP;
  }
}
 
/*
 * Format an Array object
 */
void 
objFormatArray(psobj *objP, int length, psobj *valueP)
{
  if (objP != NULL) {
    objP->type        = OBJ_ARRAY;
    objP->len         = length;
    objP->data.arrayP = valueP;
  }
}
 
 
/*
 * Format a String object
 */
void 
objFormatString(psobj *objP, int length, char *valueP)
{
  if (objP != NULL) {
    objP->type         = OBJ_STRING;
    objP->len          = length;
    objP->data.valueP  = valueP;
  }
}
 
/*
 * Format a Name object
 */
void 
objFormatName(psobj *objP, int length, char *valueP)
{
  if (objP != NULL) {
    objP->type         = OBJ_NAME;
    objP->len          = length;
    objP->data.nameP   = valueP;
  }
}
 
/*
 * Format a File object
 */
void 
objFormatFile(psobj *objP, FILE *valueP)
{
  if (objP != NULL) {
    objP->type         = OBJ_FILE;
    objP->len          = 0;
    objP->data.fileP   = valueP;
  }
}
 
