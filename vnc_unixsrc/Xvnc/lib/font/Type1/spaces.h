/* $XConsortium: spaces.h,v 1.4 94/02/06 16:27:06 gildea Exp $ */
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
/*SHARED*/
 
#define   USER                       t1_User
#define   IDENTITY                   t1_Identity
 
#define   Context(d,u)               t1_Context(d,u)
#define   Transform(o,f1,f2,f3,f4)   t1_Transform(o,f1,f2,f3,f4)
#define   Rotate(o,d)                t1_Rotate(o,d)
#define   Scale(o,sx,sy)             t1_Scale(o,sx,sy)
#define   QuerySpace(S,f1,f2,f3,f4)  t1_QuerySpace(S,f1,f2,f3,f4)
#define   Warp(s1,o,s2)              t1_Warp(s1,o,s2)
 
struct XYspace *t1_Context(); /* creates a coordinate space for a device      */
struct xobject *t1_Transform();  /* transform an object                       */
struct xobject *t1_Rotate();  /* rotate an object                             */
struct xobject *t1_Scale();   /* scale an object                              */
struct xobject *t1_Warp();    /* transform like delta of two spaces           */
void t1_QuerySpace();         /* returns coordinate space matrix              */
 
/*END SHARED*/
/*SHARED*/
 
#define   DeviceResolution   t1_DeviceResolution
#define   InitSpaces()       t1_InitSpaces()
#define   CopySpace(s)       t1_CopySpace(s)
#define   Xform(o,M)         t1_Xform(o,M)
#define   UnConvert(S,pt,xp,yp)    t1_UnConvert(S,pt,xp,yp)
#define   MatrixMultiply(A,B,C)    t1_MMultiply(A,B,C)
#define   MatrixInvert(A,B)        t1_MInvert(A,B)
#define   PseudoSpace(S,M)   t1_PseudoSpace(S,M)
#define   FindContext(M)     t1_FindContext(M)
 
void t1_InitSpaces();         /* initialize pre-defined coordinate spaces     */
struct XYspace *t1_CopySpace(); /* duplicate a coordinate space               */
struct xobject *t1_Xform();   /* transform object by matrix                   */
void t1_UnConvert();          /* return user coordinates from device coordinates */
void t1_MMultiply();          /* multiply two matrices                        */
void t1_MInvert();            /* invert a matrix                              */
void t1_PseudoSpace();        /* force a coordinate space from a matrix       */
int t1_FindContext();         /* return the "context" represented by a matrix */
 
/*END SHARED*/
/*SHARED*/
 
/* #define    KillSpace(s)     Free(s)
Note - redefined KillSpace() to check references !
3-26-91 PNM */
 
#define KillSpace(s)      if ( (--(s->references) == 0) ||\
                      ( (s->references == 1) && ISPERMANENT(s->flag) ) )\
                        Free(s)
 
#define    ConsumeSpace(s)  MAKECONSUME(s,KillSpace(s))
#define    UniqueSpace(s)   MAKEUNIQUE(s,CopySpace(s))
 
/*END SHARED*/
/*SHARED*/
 
typedef short pel;           /* integer pel locations                        */
typedef long fractpel;       /* fractional pel locations                     */
 
#define   FRACTBITS     16   /* number of fractional bits in 'fractpel'      */
/*
We define the following macros to convert from 'fractpel' to 'pel' and
vice versa:
*/
#define   TOFRACTPEL(p)   (((fractpel)p)<<FRACTBITS)
#define   FPHALF          (1<<(FRACTBITS-1))
#define   NEARESTPEL(fp)  (((fp)+FPHALF)>>FRACTBITS)
#define   FRACTFLOAT   (double)(1L<<FRACTBITS)
 
/*END SHARED*/
/*SHARED*/
 
struct doublematrix {
       double normal[2][2];
       double inverse[2][2];
} ;
 
/*END SHARED*/
/*SHARED*/
 
struct XYspace {
       XOBJ_COMMON           /* xobject common data define 3-26-91 PNM       */
			     /* type = SPACETYPE			     */
       void (*convert)();     /* calculate "fractpoint" X,Y from float X,Y   */
       void (*iconvert)();    /* calculate "fractpoint" X,Y from int X,Y     */
       fractpel (*xconvert)();  /* subroutine of convert                     */
       fractpel (*yconvert)();  /* subroutine of convert                     */
       fractpel (*ixconvert)();  /* subroutine of iconvert                   */
       fractpel (*iyconvert)();  /* subroutine of iconvert                   */
       int ID;               /* unique identifier (used in font caching)     */
       unsigned char context;  /* device context of coordinate space         */
       struct doublematrix tofract;  /* xform to get to fractional pels      */
       fractpel itofract[2][2];  /* integer version of "tofract.normal"      */
} ;
 
#define    INVALIDID  0      /* no valid space will have this ID             */
 
/*END SHARED*/
/*SHARED*/
 
struct fractpoint {
       fractpel x,y;
} ;
 
/*END SHARED*/
/*SHARED*/
 
#define  NULLCONTEXT   0
 
/*END SHARED*/
