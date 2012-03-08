/* $Xorg: objects.h,v 1.3 2000/08/17 19:46:31 cpqbld Exp $ */
/* Copyright International Business Machines, Corp. 1991
 * All Rights Reserved
 * Copyright Lexmark International, Inc. 1991
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM or Lexmark not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM AND LEXMARK PROVIDE THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES OF
 * ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE
 * QUALITY AND PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF THE
 * SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM OR LEXMARK) ASSUMES THE
 * ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN NO EVENT SHALL
 * IBM OR LEXMARK BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
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
/* $XFree86: xc/lib/font/Type1/objects.h,v 1.15 2003/05/27 22:26:45 tsi Exp $ */
/*SHARED*/
 
/*END SHARED*/
#include <Xdefs.h>
#include <Xfuncproto.h>
#ifndef FONTMODULE
#include <stdlib.h>
#endif
/*SHARED*/

#define   Permanent(obj)    t1_Permanent(obj)
#ifdef notused
#define   Temporary(obj)    t1_Temporary(obj)
#endif
#define   Destroy(obj)      t1_Destroy(obj)
#define   Dup(obj)          t1_Dup(obj)
#define   InitImager        t1_InitImager
#define   TermImager        t1_TermImager
#define   Pragmatics(f,v)   t1_Pragmatics(f,v)
#define   ErrorMsg          t1_ErrorMsg
 
/* make an object permanent                 */
extern struct xobject *t1_Permanent ( pointer obj );

#ifdef notused 
/* make an object temporary                 */
extern struct xobject *t1_Temporary( pointer obj );
#endif

/* destroy an object                        */
extern struct xobject *t1_Destroy ( pointer obj );

/* duplicate an object                      */
extern struct xobject *t1_Dup ( pointer obj );


extern void t1_InitImager ( void ); /* initialize TYPE1IMAGER                */
extern void t1_TermImager ( void ); /* terminate TYPE1IMAGER                 */
/* set debug flags, etc.                       */
extern void t1_Pragmatics ( char *username, int value ); 

/* return last TYPE1IMAGER error message       */
extern char *t1_ErrorMsg ( void ); 
 
/*END SHARED*/
/*SHARED*/
extern void xiFree ( long *addr );
extern char *xiMalloc ( unsigned Size );
extern void addmemory ( long *addr, long size );
extern void delmemory ( void );

#ifndef OS_H
extern void FatalError(const char *f, ...)
#if defined(__GNUC__) && \
    ((__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ > 4)))
__attribute((noreturn))
#endif
;

extern void ErrorF(const char *f, ...);
#endif

#define   Abort(line)       FatalError(line)
#define   Allocate(n,t,s)   t1_Allocate(n,t,s)
#define   Free(obj)         t1_Free(obj)
#define   NonObjectFree(a)  xiFree((long *)(a))
#define   Consume           t1_Consume
#define   ArgErr(s,o,r)     t1_ArgErr(s,o,r)
#define   TypeErr(n,o,e,r)  t1_TypeErr(n,o,e,r)
#define   Copy(obj)         t1_Copy(obj)
#define   Unique(obj)       t1_Unique(obj)
 
/* allocate memory                         */
extern struct xobject *t1_Allocate( int size, pointer template, 
				    int extra );

/* free memory                             */
extern void t1_Free ( pointer obj );

/* make a unique temporary copy of an object   */
extern struct xobject *t1_Unique ( pointer obj );

/* handle argument errors                      */
extern struct xobject *t1_ArgErr ( char *string, pointer obj, pointer ret );

/* handle 'bad type' argument errors           */
extern struct xobject *t1_TypeErr ( char *name,  pointer obj, 
				    int expect, pointer ret );

/* consume a variable number of arguments      */
extern void t1_Consume ( int n, ... );

/* make a new copy, not reference bump PNM     */
extern struct xobject *t1_Copy ( pointer obj );

 
/*END SHARED*/
/*SHARED*/
 
#define   ON          (~0)   /* all bits on                                  */
#ifndef FALSE
#define   FALSE       0      /* handy zero value                             */
#endif
#ifndef TRUE
#define   TRUE        1      /* handy non-zero value                         */
#endif
 
#ifndef   NULL
#include <stddef.h>
/*
The NULL pointer is system specific.  (Most systems, however, use 0.)
TYPE1IMAGER could have its own NULL, independent of the rest of the system,
were it not for malloc().  The system call malloc() returns NULL when
out of memory.
:i1/portibility assumptions/
*/
#endif
 
#ifndef MIN
#define   MIN(a,b)    (((a)<(b)) ? a : b)
#endif
#ifndef MAX
#define   MAX(a,b)    (((a)>(b)) ? a : b)
#endif
#ifndef ABS
#define   ABS(a)      (((a)>=0)?(a):-(a))
#endif

/*END SHARED*/
/*SHARED*/
 
struct xobject {
       char type;           /* encoded type of object                        */
       unsigned char flag;  /* flag byte for temporary object characteristics*/
       short references;    /* count of pointers to this object
                               (plus 1 for permanent objects) PNM            */
} ;
 
/*END SHARED*/
/*SHARED*/
 
#define XOBJ_COMMON      char type; unsigned char flag; short references;
 
/*END SHARED*/
/*SHARED*/
 
 
#define   INVALIDTYPE    0
#define   FONTTYPE       1
#define   REGIONTYPE     3
#define   PICTURETYPE    4
#define   SPACETYPE      5
#define   LINESTYLETYPE  6
#define   EDGETYPE       7
#define   STROKEPATHTYPE 8
#define   CLUTTYPE       9
 
#define   ISPATHTYPE(type)    ((type)&0x10)  /* all path segments have this bit on */
#define   LINETYPE    (0+ISPATHTYPE(ON))
#define   CONICTYPE   (1+ISPATHTYPE(ON))
#define   BEZIERTYPE  (2+ISPATHTYPE(ON))
#define   HINTTYPE    (3+ISPATHTYPE(ON))
 
#define   MOVETYPE    (5+ISPATHTYPE(ON))
#define   TEXTTYPE    (6+ISPATHTYPE(ON))
 
/*END SHARED*/
/*SHARED*/
 
#define   ISPERMANENT(flag)   ((flag)&0x01)
#define   ISIMMORTAL(flag)    ((flag)&0x02)
 
/*END SHARED*/
/*SHARED*/
 
#define   PRESERVE(obj)   if (!ISPERMANENT((obj)->flag)) \
   (obj)->references++;
 
/*END SHARED*/
/*SHARED*/
 
#define  LONGCOPY(dest,source,bytes) { \
    register long *p1 = (long *)dest;  register long *p2 = (long *)source; \
    register int count = (bytes) / sizeof(long); \
    while (--count >= 0) *p1++ = *p2++; }
 
 
/*END SHARED*/
/*SHARED*/
 
#define   FOLLOWING(p)  ((p)+1)
 
/*END SHARED*/
/*SHARED*/
 
#define  TYPECHECK(name, obj, expect, whenBAD, consumables, rettype) { \
    if (obj->type != expect) { \
         (Consume)consumables; \
         return((rettype)TypeErr(name, obj, expect, whenBAD)); \
    } \
}
 
/*END SHARED*/
/*SHARED*/
 
#define  ARGCHECK(test,msg,obj,whenBAD,consumables,rettype) { \
    if (test) { \
        (Consume)consumables; \
        return((rettype)ArgErr(msg, obj, whenBAD)); \
    } \
}
 
/*END SHARED*/
/*SHARED*/
 
/* Changed use of Dup() below to Temporary(Copy()) because Dup() does not
   necessarily return a Unique Copy anymore! 3-26-91 */
#define  TYPENULLCHECK(name, obj, expect, whenBAD, consumables,rettype) \
    if (obj == NULL) { \
        (Consume)consumables; \
        if (whenBAD != NULL && ISPERMANENT(whenBAD->flag)) \
              return((rettype)Temporary(Copy(whenBAD))); \
        else  return((rettype)whenBAD); \
    } else { \
        if (obj->type != expect) { \
             (Consume)consumables; \
             return((rettype)TypeErr(name, obj, expect, whenBAD)); \
        } \
    }
/*END SHARED*/
/*SHARED*/
 
#define  MAKECONSUME(obj,stmt)  { if (!ISPERMANENT(obj->flag)) stmt; }
 
/*END SHARED*/
/*SHARED*/
 
#define MAKEUNIQUE(obj,stmt) ( ( (obj)->references > 1 ) ? stmt : obj )
 
/*END SHARED*/
/*SHARED*/
 
#ifdef GLOBALS
 
#define   extern
#define   INITIALIZED(value)      = value
 
#else
 
#define   INITIALIZED(value)
 
#endif
 
extern char MustCheckArgs  INITIALIZED(TRUE);
extern char MustTraceCalls INITIALIZED(FALSE);
#ifdef BUILDCID
extern char MustCrash      INITIALIZED(FALSE);
#else
extern char MustCrash      INITIALIZED(TRUE);
#endif
extern char InternalTrace  INITIALIZED(TRUE);
extern char LineIOTrace    INITIALIZED(TRUE);
 
extern char ProcessHints   INITIALIZED(TRUE);
 
extern char SaveFontPaths  INITIALIZED(TRUE);
 
extern short CRASTERCompressionType   INITIALIZED(1);
 
extern char ConicDebug     INITIALIZED(0);
extern char LineDebug      INITIALIZED(0);
extern char RegionDebug    INITIALIZED(0);
extern char PathDebug      INITIALIZED(0);
extern char FontDebug      INITIALIZED(0);
extern char SpaceDebug     INITIALIZED(0);
extern char StrokeDebug    INITIALIZED(0);
extern char MemoryDebug    INITIALIZED(0);
extern char HintDebug      INITIALIZED(0);
extern char ImageDebug     INITIALIZED(0);
extern char OffPageDebug   INITIALIZED(0);
 
extern short CachedChars   INITIALIZED(0x7FFF);
extern short CachedFonts   INITIALIZED(0x7FFF);
extern int   CacheBLimit   INITIALIZED(12500);
extern char  Continuity    INITIALIZED(2);
 
#ifdef extern
#undef extern
#endif
 
/*
We define other routines formatting parameters
*/
#define    DumpArea(area)    t1_DumpArea(area)
#define    DumpText(text)    t1_DumpText(text)
#define    DumpPath(path)    t1_DumpPath(path)
#define    DumpSpace(space)  t1_DumpSpace(space)
#define    DumpEdges(e)      t1_DumpEdges(e)
#define    FormatFP(s,p)     t1_FormatFP(s,p)
 
/* dump a textpath structure                   */
extern void t1_DumpText(void);
 
/*END SHARED*/
