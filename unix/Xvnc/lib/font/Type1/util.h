/* $Xorg: util.h,v 1.3 2000/08/17 19:46:34 cpqbld Exp $ */
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
/* $XFree86: xc/lib/font/Type1/util.h,v 1.5 2001/01/17 19:43:24 dawes Exp $ */

#ifndef UTIL_H
#define UTIL_H
 
 
#ifndef boolean
typedef int boolean;
#endif
 
#ifndef TRUE
#define TRUE (1)
#endif
 
#ifndef FALSE
#define FALSE (0)
#endif
 
/***================================================================***/
/* Portable definitions for 2's complement machines.
 * NOTE: These really should be based on PostScript types,
 * for example, sizeof(ps_integer), or sizeof(ps_unsigned)
 */
#define MAX_ULONG             (~(unsigned long)(0))
/* This code is portable, assuming K&R C and 2's complement arithmetic */
#define MAX_INTEGER      \
     ((long)((((unsigned long) 1)<<(sizeof(unsigned long)*8-1))-1))
#define MIN_INTEGER           ((-MAX_INTEGER)-1)
 
#define MAX_ARRAY_CNT         (65535)
#define MAX_DICT_CNT          (65535)
#define MAX_STRING_LEN        (65535)
#define MAX_NAME_LEN          (128)
 
/* this is the size of memory allocated for reading fonts */
 
#ifdef BUILDCID
#define VM_SIZE               (100*1024)
#else
#define VM_SIZE               (50*1024)
#endif
/***================================================================***/
 
#ifndef MIN
#define   MIN(a,b)   (((a)<(b)) ? a : b )
#endif
 
/***================================================================***/
/*  Routines for managing virtual memory                              */
/***================================================================***/

extern boolean  vm_init ( int cnt );
extern long     vm_free;
extern long     vm_size;
extern char    *vm_next;
extern char    *vm_alloc ( int bytes );

/***================================================================***/
/*  Macros for managing virtual memory                                */
/***================================================================***/
#define vm_next_byte()  (vm_next)
#define vm_free_bytes()  (vm_free)
#define vm_avail(B)     (B <= vm_free)
 
 
 
/***================================================================***/
/* Types of PostScript objects */
/***================================================================***/
#define OBJ_INTEGER    (0)
#define OBJ_REAL       (1)
#define OBJ_BOOLEAN    (2)
#define OBJ_ARRAY      (3)
#define OBJ_STRING     (4)
#define OBJ_NAME       (5)
#define OBJ_FILE       (6)
#define OBJ_ENCODING   (7)
 
/***================================================================***/
/* Value of PostScript objects */
/***================================================================***/
typedef union ps_value {
  char            *valueP;     /* value pointer for unspecified type */
  int              value;      /* value for unspecified type         */
  int              integer;    /* when type is OBJ_INTEGER           */
  float            real;       /* when type is OBJ_REAL              */
  int              boolean;    /* when type is OBJ_BOOLEAN           */
  struct ps_obj   *arrayP;     /* when type is OBJ_ARRAY             */
  unsigned char   *stringP;    /* when type is OBJ_STRING            */
  char            *nameP;      /* when type is OBJ_NAME              */
  FILE            *fileP;      /* when type is OBJ_FILE              */
} psvalue;
 
/***================================================================***/
/* Definition of a PostScript object */
/***================================================================***/
typedef struct ps_obj {
  char type;
  char unused;
  unsigned short len;
  union ps_value data;
} psobj;
 
/***================================================================***/
/*     Definition of a PostScript Dictionary Entry */
/***================================================================***/
typedef struct ps_dict {
  psobj   key;
  psobj   value;
} psdict;
 
/***================================================================***/
/* Macros for testing type of PostScript objects */
/***================================================================***/
#define objIsInteger(o)          ((o).type == OBJ_INTEGER)
#define objIsReal(o)             ((o).type == OBJ_REAL)
#define objIsBoolean(o)          ((o).type == OBJ_BOOLEAN)
#define objIsArray(o)            ((o).type == OBJ_ARRAY)
#define objIsString(o)           ((o).type == OBJ_STRING)
#define objIsName(o)             ((o).type == OBJ_NAME)
#define objIsFile(o)             ((o).type == OBJ_FILE)
 
/***================================================================***/
/* Macros for setting type of PostScript objects */
/***================================================================***/
#define objSetInteger(o)         ((o).type = OBJ_INTEGER)
#define objSetReal(o)            ((o).type = OBJ_REAL)
#define objSetBoolean(o)         ((o).type = OBJ_BOOLEAN)
#define objSetArray(o)           ((o).type = OBJ_ARRAY)
#define objSetString(o)          ((o).type = OBJ_STRING)
#define objSetName(o)            ((o).type = OBJ_NAME)
#define objSetFile(o)            ((o).type = OBJ_FILE)
 
/***================================================================***/
/* Macros for testing type of PostScript objects (pointer access) */
/***================================================================***/
#define objPIsInteger(o)         ((o)->type == OBJ_INTEGER)
#define objPIsReal(o)            ((o)->type == OBJ_REAL)
#define objPIsBoolean(o)         ((o)->type == OBJ_BOOLEAN)
#define objPIsArray(o)           ((o)->type == OBJ_ARRAY)
#define objPIsString(o)          ((o)->type == OBJ_STRING)
#define objPIsName(o)            ((o)->type == OBJ_NAME)
#define objPIsFile(o)            ((o)->type == OBJ_FILE)
 
/***================================================================***/
/* Macros for setting type of PostScript objects (pointer access) */
/***================================================================***/
#define objPSetInteger(o)        ((o)->type = OBJ_INTEGER)
#define objPSetReal(o)           ((o)->type = OBJ_REAL)
#define objPSetBoolean(o)        ((o)->type = OBJ_BOOLEAN)
#define objPSetArray(o)          ((o)->type = OBJ_ARRAY)
#define objPSetString(o)         ((o)->type = OBJ_STRING)
#define objPSetName(o)           ((o)->type = OBJ_NAME)
#define objPSetFile(o)           ((o)->type = OBJ_FILE)

/***================================================================***/
/* Prototypes of object formatting functions */
/***================================================================***/
extern void objFormatInteger ( psobj *objP, int value );
extern void objFormatReal ( psobj *objP, float value );
extern void objFormatBoolean ( psobj *objP, boolean value );
extern void objFormatEncoding ( psobj *objP, int length, psobj *valueP );
extern void objFormatArray ( psobj *objP, int length, psobj *valueP );
extern void objFormatString ( psobj *objP, int length, char *valueP );
extern void objFormatName ( psobj *objP, int length, char *valueP );
extern void objFormatFile ( psobj *objP, FILE *valueP );

#endif
