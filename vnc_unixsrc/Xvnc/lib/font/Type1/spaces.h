/* $Xorg: spaces.h,v 1.3 2000/08/17 19:46:32 cpqbld Exp $ */
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
/* $XFree86: xc/lib/font/Type1/spaces.h,v 3.4 2001/01/17 19:43:23 dawes Exp $ */

/*SHARED*/
 
#define   USER                       t1_User
#define   IDENTITY                   t1_Identity
 
#define   Context(d,u)               t1_Context(d,u)
#define   Transform(o,f1,f2,f3,f4)   t1_Transform(o,f1,f2,f3,f4)
#define   Rotate(o,d)                t1_Rotate(o,d)
#define   Scale(o,sx,sy)             t1_Scale(o,sx,sy)
#define   QuerySpace(S,f1,f2,f3,f4)  t1_QuerySpace(S,f1,f2,f3,f4)
#define   Warp(s1,o,s2)              t1_Warp(s1,o,s2)

/* IDENTITY space */
extern struct XYspace *IDENTITY;

/* creates a coordinate space for a device      */
extern struct XYspace *Context(pointer device, double units);
/* transform an object                       */
extern struct xobject *t1_Transform ( struct xobject *obj, double cxx, 
				      double cyx, double cxy, double cyy );
#if 0
struct xobject *t1_Rotate();  /* rotate an object                             */
#endif
/* scale an object                              */
extern struct xobject *t1_Scale ( struct xobject *obj, double sx, double sy );
#if 0
struct xobject *t1_Warp();    /* transform like delta of two spaces           */
#endif
/* returns coordinate space matrix              */
extern void t1_QuerySpace ( struct XYspace *S, double *cxxP, double *cyxP, 
			    double *cxyP, double *cyyP );
 
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
 
struct fractpoint {
       fractpel x,y;
} ;
 
/*SHARED*/

typedef fractpel (*convertFunc)(double, double, double, double);
typedef fractpel (*iconvertFunc)(fractpel, fractpel, long, long);

struct XYspace {
       XOBJ_COMMON           /* xobject common data define 3-26-91 PNM       */
			     /* type = SPACETYPE			     */
       void (*convert)(struct fractpoint *, struct XYspace *, double, double);     /* calculate "fractpoint" X,Y from float X,Y   */
       void (*iconvert)(struct fractpoint *, struct XYspace *, long, long);    /* calculate "fractpoint" X,Y from int X,Y     */
       convertFunc xconvert;  /* subroutine of convert                       */
       convertFunc yconvert;  /* subroutine of convert                       */
       iconvertFunc ixconvert;  /* subroutine of iconvert                    */
       iconvertFunc iyconvert;  /* subroutine of iconvert                    */
       int ID;               /* unique identifier (used in font caching)     */
       unsigned char context;  /* device context of coordinate space         */
       struct doublematrix tofract;  /* xform to get to fractional pels      */
       fractpel itofract[2][2];  /* integer version of "tofract.normal"      */
} ;
 
#define    INVALIDID  0      /* no valid space will have this ID             */
 
/*END SHARED*/
/*END SHARED*/
/*SHARED*/
 
#define   DeviceResolution   t1_DeviceResolution
#define   InitSpaces         t1_InitSpaces
#define   CopySpace(s)       t1_CopySpace(s)
#define   Xform(o,M)         t1_Xform(o,M)
#define   UnConvert(S,pt,xp,yp)    t1_UnConvert(S,pt,xp,yp)
#define   MatrixMultiply(A,B,C)    t1_MMultiply(A,B,C)
#define   MatrixInvert(A,B)        t1_MInvert(A,B)
#define   PseudoSpace(S,M)   t1_PseudoSpace(S,M)
#define   FindContext(M)     t1_FindContext(M)
 
/* initialize pre-defined coordinate spaces     */
extern void t1_InitSpaces ( void );
/* duplicate a coordinate space               */
extern struct XYspace *t1_CopySpace ( struct XYspace *S );
/* transform object by matrix                   */
extern struct xobject *t1_Xform ( struct xobject *obj, double M[2][2] );
/* return user coordinates from device coordinates */
extern void t1_UnConvert ( struct XYspace *S, struct fractpoint *pt, 
			   double *xp, double *yp );
/* multiply two matrices                        */
extern void t1_MMultiply ( double A[2][2], double B[2][2], double C[2][2] );
/* invert a matrix                              */
extern void t1_MInvert ( double M[2][2], double Mprime[2][2] );
/* force a coordinate space from a matrix       */
extern void t1_PseudoSpace ( struct XYspace *S, double M[2][2] );
/* return the "context" represented by a matrix */
int t1_FindContext(double M[2][2]);          

/*END SHARED*/
/*SHARED*/
 
#define  NULLCONTEXT   0
 
/*END SHARED*/
 
/* dump a coordinate space structure           */
extern void t1_DumpSpace ( struct XYspace *S );
/* dump a format a "fractpel" coordinate       */
extern void t1_FormatFP ( char *string, fractpel fpel );
