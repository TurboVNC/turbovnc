/* $XFree86: xc/programs/Xserver/hw/xfree86/common/compiler.h,v 3.24.2.2 1998/02/07 00:44:37 dawes Exp $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XConsortium: compiler.h /main/16 1996/10/25 15:38:34 kaleb $ */

#ifndef _COMPILER_H
#define _COMPILER_H

#ifndef __STDC__
# ifdef signed
#  undef signed
# endif
# ifdef volatile
#  undef volatile
# endif
# ifdef const
#  undef const
# endif
# define signed /**/
# ifdef __GNUC__
#  define volatile __volatile__
#  define const __const__
#  ifdef PC98
#   undef NO_INLINE
#  endif
# else
#  define const /**/
#  ifdef PC98
#   define __inline__ /**/
#  endif
# endif /* __GNUC__ */
#endif /* !__STDC__ */

#if defined(IODEBUG) && defined(__GNUC__)
#define outb RealOutb
#define outw RealOutw
#define outl RealOutl
#define inb RealInb
#define inw RealInw
#define inl RealInl
#endif

#ifdef NO_INLINE

extern void outb();
extern void outw();
extern void outl();
extern unsigned int inb();
extern unsigned int inw();
extern unsigned int inl();
#if NeedFunctionPrototypes
extern unsigned char rdinx(unsigned short int, unsigned char);
extern void wrinx(unsigned short int, unsigned char, unsigned char);
extern void modinx(unsigned short int, unsigned char, unsigned char, unsigned char);
extern int testrg(unsigned short int, unsigned char);
extern int testinx2(unsigned short int, unsigned char, unsigned char);
extern int testinx(unsigned short int, unsigned char);
#else /* NeedFunctionProtoypes */
extern unsigned char rdinx();
extern void wrinx();
extern void modinx();
extern int testrg();
extern int testinx2();
extern int testinx();
#endif /* NeedFunctionProtoypes */

#else /* NO_INLINE */

#ifdef __GNUC__

#if defined(linux) && defined(__alpha__)
/* for Linux on Alpha, we use the LIBC _inx/_outx routines */
/* note that the appropriate setup via "ioperm" needs to be done */
/*  *before* any inx/outx is done. */

static __inline__ void
outb(port, val)
     unsigned short port;
     char val;
{
    extern void _outb(char val, unsigned short port);
    _outb(val, port);
}

static __inline__ void
outw(port, val)
     unsigned short port;
     short val;
{
    extern void _outw(short val, unsigned short port);
    _outw(val, port);
}

static __inline__ void
outl(port, val)
     unsigned short port;
     int val;
{
    extern void _outl(int val, unsigned short port);
    _outl(val, port);
}

static __inline__ unsigned int
inb(port)
     unsigned short port;
{
  extern unsigned int _inb(unsigned short port);
  return _inb(port);
}

static __inline__ unsigned int
inw(port)
     unsigned short port;
{
  extern unsigned int _inw(unsigned short port);
  return _inw(port);
}

static __inline__ unsigned int
inl(port)
     unsigned short port;
{
  extern unsigned int _inl(unsigned short port);
  return _inl(port);
}


/*
 * inline functions to do unaligned accesses
 * from linux/include/asm-alpha/unaligned.h
 */

static __inline__ unsigned long ldq_u(unsigned long * r11)
{
	unsigned long r1,r2;
	__asm__("ldq_u %0,%3\n\t"
		"ldq_u %1,%4\n\t"
		"extql %0,%2,%0\n\t"
		"extqh %1,%2,%1\n\t"
		"bis %1,%0,%0"
		:"=&r" (r1), "=&r" (r2)
		:"r" (r11),
		 "m" (*r11),
		 "m" (*(unsigned long *)(7+(char *) r11)));
	return r1;
}

static __inline__ unsigned long ldl_u(unsigned int * r11)
{
	unsigned long r1,r2;
	__asm__("ldq_u %0,%3\n\t"
		"ldq_u %1,%4\n\t"
		"extll %0,%2,%0\n\t"
		"extlh %1,%2,%1\n\t"
		"bis %1,%0,%0"
		:"=&r" (r1), "=&r" (r2)
		:"r" (r11),
		 "m" (*r11),
		 "m" (*(unsigned long *)(3+(char *) r11)));
	return r1;
}

static __inline__ unsigned long ldw_u(unsigned short * r11)
{
	unsigned long r1,r2;
	__asm__("ldq_u %0,%3\n\t"
		"ldq_u %1,%4\n\t"
		"extwl %0,%2,%0\n\t"
		"extwh %1,%2,%1\n\t"
		"bis %1,%0,%0"
		:"=&r" (r1), "=&r" (r2)
		:"r" (r11),
		 "m" (*r11),
		 "m" (*(unsigned long *)(1+(char *) r11)));
	return r1;
}

static __inline__ void stq_u(unsigned long r5, unsigned long * r11)
{
	unsigned long r1,r2,r3,r4;

	__asm__("ldq_u %3,%1\n\t"
		"ldq_u %2,%0\n\t"
		"insqh %6,%7,%5\n\t"
		"insql %6,%7,%4\n\t"
		"mskqh %3,%7,%3\n\t"
		"mskql %2,%7,%2\n\t"
		"bis %3,%5,%3\n\t"
		"bis %2,%4,%2\n\t"
		"stq_u %3,%1\n\t"
		"stq_u %2,%0"
		:"=m" (*r11),
		 "=m" (*(unsigned long *)(7+(char *) r11)),
		 "=&r" (r1), "=&r" (r2), "=&r" (r3), "=&r" (r4)
		:"r" (r5), "r" (r11));
}

static __inline__ void stl_u(unsigned long r5, unsigned int * r11)
{
	unsigned long r1,r2,r3,r4;

	__asm__("ldq_u %3,%1\n\t"
		"ldq_u %2,%0\n\t"
		"inslh %6,%7,%5\n\t"
		"insll %6,%7,%4\n\t"
		"msklh %3,%7,%3\n\t"
		"mskll %2,%7,%2\n\t"
		"bis %3,%5,%3\n\t"
		"bis %2,%4,%2\n\t"
		"stq_u %3,%1\n\t"
		"stq_u %2,%0"
		:"=m" (*r11),
		 "=m" (*(unsigned long *)(3+(char *) r11)),
		 "=&r" (r1), "=&r" (r2), "=&r" (r3), "=&r" (r4)
		:"r" (r5), "r" (r11));
}

static __inline__ void stw_u(unsigned long r5, unsigned short * r11)
{
	unsigned long r1,r2,r3,r4;

	__asm__("ldq_u %3,%1\n\t"
		"ldq_u %2,%0\n\t"
		"inswh %6,%7,%5\n\t"
		"inswl %6,%7,%4\n\t"
		"mskwh %3,%7,%3\n\t"
		"mskwl %2,%7,%2\n\t"
		"bis %3,%5,%3\n\t"
		"bis %2,%4,%2\n\t"
		"stq_u %3,%1\n\t"
		"stq_u %2,%0"
		:"=m" (*r11),
		 "=m" (*(unsigned long *)(1+(char *) r11)),
		 "=&r" (r1), "=&r" (r2), "=&r" (r3), "=&r" (r4)
		:"r" (r5), "r" (r11));
}

#define mem_barrier()        __asm__ __volatile__("mb"  : : : "memory")
#ifdef __ELF__
#define write_mem_barrier()  __asm__ __volatile__("wmb" : : : "memory")
#else  /*  ECOFF gas 2.6 doesn't know "wmb" :-(  */
#define write_mem_barrier()  mem_barrier()
#endif

#else /* defined(linux) && defined(__alpha__) */
#if defined(__mips__)

unsigned int IOPortBase;  /* Memory mapped I/O port area */

static __inline__ void
outb(port, val)
     short port;
     char val;
{
	*(volatile unsigned char*)(((unsigned short)(port))+IOPortBase) = val;
}

static __inline__ void
outw(port, val)
     short port;
     short val;
{
	*(volatile unsigned short*)(((unsigned short)(port))+IOPortBase) = val;
}

static __inline__ void
outl(port, val)
     short port;
     int val;
{
	*(volatile unsigned long*)(((unsigned short)(port))+IOPortBase) = val;
}

static __inline__ unsigned int
inb(port)
     short port;
{
	return(*(volatile unsigned char*)(((unsigned short)(port))+IOPortBase));
}

static __inline__ unsigned int
inw(port)
     short port;
{
	return(*(volatile unsigned short*)(((unsigned short)(port))+IOPortBase));
}

static __inline__ unsigned int
inl(port)
     short port;
{
	return(*(volatile unsigned long*)(((unsigned short)(port))+IOPortBase));
}


static __inline__ unsigned long ldq_u(unsigned long * r11)
{
	unsigned long r1;
	__asm__("lwr %0,%2\n\t"
		"lwl %0,%3\n\t"
		:"=&r" (r1)
		:"r" (r11),
		 "m" (*r11),
		 "m" (*(unsigned long *)(3+(char *) r11)));
	return r1;
}

static __inline__ unsigned long ldl_u(unsigned int * r11)
{
	unsigned long r1;
	__asm__("lwr %0,%2\n\t"
		"lwl %0,%3\n\t"
		:"=&r" (r1)
		:"r" (r11),
		 "m" (*r11),
		 "m" (*(unsigned long *)(3+(char *) r11)));
	return r1;
}

static __inline__ unsigned long ldw_u(unsigned short * r11)
{
	unsigned long r1;
	__asm__("lwr %0,%2\n\t"
		"lwl %0,%3\n\t"
		:"=&r" (r1)
		:"r" (r11),
		 "m" (*r11),
		 "m" (*(unsigned long *)(1+(char *) r11)));
	return r1;
}

#define stq_u(v,p)	stl_u(v,p)
#define stl_u(v,p)	((unsigned char *)(p)) = (v); \
			((unsigned char *)(p)+1) = ((v) >> 8);  \
			((unsigned char *)(p)+2) = ((v) >> 16); \
			((unsigned char *)(p)+3) = ((v) >> 24)

#define stw_u(v,p)	((unsigned char *)(p)) = (v); \
			((unsigned char *)(p)+1) = ((v) >> 8)

#define mem_barrier()   /* NOP */

#else /* defined(mips) */

#define ldq_u(p)	(*((unsigned long  *)(p)))
#define ldl_u(p)	(*((unsigned int   *)(p)))
#define ldw_u(p)	(*((unsigned short *)(p)))
#define stq_u(v,p)	((unsigned long  *)(p)) = (v)
#define stl_u(v,p)	((unsigned int   *)(p)) = (v)
#define stw_u(v,p)	((unsigned short *)(p)) = (v)
#define mem_barrier()   /* NOP */
#define write_mem_barrier()   /* NOP */

#if !defined(FAKEIT) && !defined(__mc68000__)
#ifdef GCCUSESGAS

/*
 * If gcc uses gas rather than the native assembler, the syntax of these
 * inlines has to be different.		DHD
 */
#ifndef PC98

static __inline__ void
#if NeedFunctionPrototypes
outb(
unsigned short int port,
unsigned char val)
#else
outb(port, val)
unsigned short int port;
unsigned char val;
#endif /* NeedFunctionPrototypes */
{
   __asm__ __volatile__("outb %0,%1" : :"a" (val), "d" (port));
}


static __inline__ void
#if NeedFunctionPrototypes
outw(
unsigned short int port,
unsigned short int val)
#else
outw(port, val)
unsigned short int port;
unsigned short int val;
#endif /* NeedFunctionPrototypes */
{
   __asm__ __volatile__("outw %0,%1" : :"a" (val), "d" (port));
}

static __inline__ void
#if NeedFunctionPrototypes
outl(
unsigned short int port,
unsigned int val)
#else
outl(port, val)
unsigned short int port;
unsigned int val;
#endif /* NeedFunctionPrototypes */
{
   __asm__ __volatile__("outl %0,%1" : :"a" (val), "d" (port));
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
inb(
unsigned short int port)
#else
inb(port)
unsigned short int port;
#endif /* NeedFunctionPrototypes */
{
   unsigned char ret;
   __asm__ __volatile__("inb %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
inw(
unsigned short int port)
#else
inw(port)
unsigned short int port;
#endif /* NeedFunctionPrototypes */
{
   unsigned short int ret;
   __asm__ __volatile__("inw %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
inl(
unsigned short int port)
#else
inl(port)
unsigned short int port;
#endif /* NeedFunctionPrototypes */
{
   unsigned int ret;
   __asm__ __volatile__("inl %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}

#else /* PC98 */

static __inline__ void
#if NeedFunctionPrototypes
_outb(
unsigned short int port,
unsigned char val)
#else
_outb(port, val)
unsigned short int port;
unsigned char val;
#endif /* NeedFunctionPrototypes */
{
     __asm__ __volatile__("outb %0,%1" ::"a" (val), "d" (port));
}

static __inline__ void
#if NeedFunctionPrototypes
_outw(
unsigned short int port,
unsigned short int val)
#else
_outw(port, val)
unsigned short int port;
unsigned short int val;
#endif /* NeedFunctionPrototypes */
{
     __asm__ __volatile__("outw %0,%1" ::"a" (val), "d" (port));
}
 
static __inline__ void
#if NeedFunctionPrototypes
_outl(
unsigned short int port,
unsigned int val)
#else
_outl(port, val)
unsigned short int port;
unsigned int val;
#endif /* NeedFunctionPrototypes */
{
   __asm__ __volatile__("outl %0,%1" : :"a" (val), "d" (port));
}


static __inline__ unsigned int
#if NeedFunctionPrototypes
_inb(
unsigned short int port)
#else
_inb(port)
unsigned short int port;
#endif /* NeedFunctionPrototypes */
{
     unsigned char ret;
     __asm__ __volatile__("inb %1,%0" :
                          "=a" (ret) :
                          "d" (port));
     return ret;
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
_inw(
unsigned short int port)
#else
_inw(port)
unsigned short int port;
#endif /* NeedFunctionPrototypes */
{
     unsigned short ret;
     __asm__ __volatile__("inw %1,%0" :
                          "=a" (ret) :
                          "d" (port));
     return ret;
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
_inl(
unsigned short int port)
#else
_inl(port)
unsigned short int port;
#endif /* NeedFunctionPrototypes */
{
   unsigned int ret;
   __asm__ __volatile__("inl %1,%0" :
       "=a" (ret) :
       "d" (port));
   return ret;
}


#if defined(PC98_PW) || defined(PC98_XKB) || defined(PC98_NEC) || defined(PC98_PWLB) || defined(PC98_GA968)
#define PW_PORT 0x600
extern short chipID;
#if NeedFunctionPrototypes
extern void *mmioBase;
#else
extern unsigned char *mmioBase;
#endif
extern unsigned short _port_tbl[];
#define	port_convert(x)	_port_tbl[(unsigned short)x]
#endif 

#if defined(PC98_WAB) ||  defined(PC98_GANB_WAP)
static __inline__ unsigned short
port_convert(unsigned short port)
{
     port <<= 8;
     port &= 0x7f00; /* Mask 0111 1111 0000 0000 */
     port |= 0xE0;
     return port;
}
#endif /* PC98_WAB || PC98_GANB_WAP */
 
#if defined(PC98_WABEP)
static __inline__ unsigned short
port_convert(unsigned short port)
{
     port &= 0x7f; /* Mask 0000 0000 0111 1111 */
     port |= 0x0f00;
     return port;
}
#endif /* PC98_WABEP */

#ifdef PC98_WSNA
static __inline__ unsigned short
port_convert(unsigned short port)
{
     port <<= 8;
     port &= 0x7f00; /* Mask 0111 1111 0000 0000 */
     port |= 0xE2;
     return port;
}
#endif /* PC98_WSNA */

#ifdef PC98_NKVNEC
#ifdef	PC98_NEC_CIRRUS2
static __inline__ unsigned short
port_convert(unsigned short port)
{
     port = (port & 0xf) + ((port & 0xf0) << 4) + 0x0050;
     return port;
}
#else
static __inline__ unsigned short
port_convert(unsigned short port)
{
     port = (port & 0xf) + ((port & 0xf0) << 4) + 0x00a0;
     return port;
}
#endif /* PC98_NEC_CIRRUS2 */
#endif /* PC98_NKVNEC */

#if defined(PC98_TGUI) || defined(PC98_MGA)
#if NeedFunctionPrototypes
extern void *mmioBase;
#else
extern unsigned char *mmioBase;
#endif
#endif

static __inline__ void
#if NeedFunctionPrototypes
outb(
unsigned short port,
unsigned char val)
#else
outb(port, val)
unsigned short port;
unsigned char val;
#endif /* NeedFunctionPrototypes */
{
#if defined(PC98_GANB_WAP) || defined(PC98_NKVNEC) || defined(PC98_WAB) || \
    defined(PC98_WABEP) || defined(PC98_WSNA) || defined(PC98_PW) || \
    defined(PC98_XKB) || defined(PC98_NEC)
   unsigned short tmp;
   tmp=port_convert(port);
   port=tmp;
#endif

#if defined(PC98_NEC)||defined(PC98_PWLB)||defined(PC98_TGUI)||defined(PC98_MGA)
   *(volatile unsigned char *)((char *)mmioBase+(port)) = (unsigned char)(val);
#else
   __asm__ __volatile__("outb %0,%1" : :"a" (val), "d" (port));
#endif
}

static __inline__ void
#if NeedFunctionPrototypes
outw(
unsigned short port,
unsigned short val)
#else
outw(port, val)
unsigned short port;
unsigned short val;
#endif /* NeedFunctionPrototypes */
{
#if defined(PC98_GANB_WAP) || defined(PC98_NKVNEC) || defined(PC98_WAB) || \
    defined(PC98_WABEP) || defined(PC98_WSNA) || defined(PC98_PW) || \
    defined(PC98_XKB) || defined(PC98_NEC)
   unsigned short tmp;
   tmp=port_convert(port);
   port=tmp;
#endif

#if defined(PC98_NEC)||defined(PC98_PWLB)||defined(PC98_TGUI)||defined(PC98_MGA)
   *(volatile unsigned short *)((char *)mmioBase+(port)) = (unsigned short)(val);
#else
   __asm__ __volatile__("outw %0,%1" : :"a" (val), "d" (port));
#endif
}

static __inline__ void
#if NeedFunctionPrototypes
outl(
unsigned short port,
unsigned int val)
#else
outl(port, val)
unsigned short port;
unsigned int val;
#endif /* NeedFunctionPrototypes */
{
#if defined(PC98_GANB_WAP) || defined(PC98_NKVNEC) || defined(PC98_WAB) || \
    defined(PC98_WABEP) || defined(PC98_WSNA) || defined(PC98_PW) || \
    defined(PC98_XKB) || defined(PC98_NEC)
   unsigned short tmp;
   tmp=port_convert(port);
   port=tmp;
#endif

#if defined(PC98_NEC)||defined(PC98_PWLB)||defined(PC98_TGUI)||defined(PC98_MGA)
   *(volatile unsigned int *)((char *)mmioBase+(port)) = (unsigned int)(val);
#else
   __asm__ __volatile__("outl %0,%1" : :"a" (val), "d" (port));
#endif
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
inb(
unsigned short port)
#else
inb(port)
unsigned short port;
#endif /* NeedFunctionPrototypes */
{
   unsigned char ret;

#if defined(PC98_GANB_WAP) || defined(PC98_NKVNEC) || defined(PC98_WAB) || \
    defined(PC98_WABEP) || defined(PC98_WSNA) || defined(PC98_PW) || \
    defined(PC98_XKB) || defined(PC98_NEC)
   unsigned short tmp;
   tmp=port_convert(port);
   port=tmp;
#endif

#if defined(PC98_NEC)||defined(PC98_PWLB)||defined(PC98_TGUI)||defined(PC98_MGA)
   ret =*(volatile unsigned char *)((char *)mmioBase+(port));
#else
   __asm__ __volatile__("inb %1,%0" :
       "=a" (ret) :
       "d" (port));
#endif
   return ret;
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
inw(
unsigned short port)
#else
inw(port)
unsigned short port;
#endif /* NeedFunctionPrototypes */
{
   unsigned short ret;

#if defined(PC98_GANB_WAP) || defined(PC98_NKVNEC) || defined(PC98_WAB) || \
    defined(PC98_WABEP) || defined(PC98_WSNA) || defined(PC98_PW) || \
    defined(PC98_XKB) || defined(PC98_NEC)
   unsigned short tmp;
   tmp=port_convert(port);
   port=tmp;
#endif

#if defined(PC98_NEC)||defined(PC98_PWLB)||defined(PC98_TGUI)||defined(PC98_MGA)
   ret =*(volatile unsigned short *)((char *)mmioBase+(port));
#else
   __asm__ __volatile__("inw %1,%0" :
       "=a" (ret) :
       "d" (port));
#endif
   return ret;
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
inl(
unsigned short port)
#else
inl(port)
unsigned short port;
#endif /* NeedFunctionPrototypes */
{
   unsigned int ret;

#if defined(PC98_GANB_WAP) || defined(PC98_NKVNEC) || defined(PC98_WAB) || \
    defined(PC98_WABEP) || defined(PC98_WSNA) || defined(PC98_PW) || \
    defined(PC98_XKB) || defined(PC98_NEC)
   unsigned short tmp;
   tmp=port_convert(port);
   port=tmp;
#endif

#if defined(PC98_NEC)||defined(PC98_PWLB)||defined(PC98_TGUI)||defined(PC98_MGA)
   ret =*(volatile unsigned int *)((char *)mmioBase+(port));
#else
   __asm__ __volatile__("inl %1,%0" :
       "=a" (ret) :
       "d" (port));
#endif
   return ret;
}

#endif /* PC98 */

#else	/* GCCUSESGAS */

static __inline__ void
#if NeedFunctionPrototypes
outb(
unsigned short int port,
unsigned char val)
#else
outb(port, val)
unsigned short int port;
unsigned char val;
#endif /* NeedFunctionPrototypes */
{
  __asm__ __volatile__("out%B0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ void
#if NeedFunctionPrototypes
outw(
unsigned short int port,
unsigned short int val)
#else
outw(port, val)
unsigned short int port;
unsigned short int val;
#endif /* NeedFunctionPrototypes */
{
  __asm__ __volatile__("out%W0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ void
#if NeedFunctionPrototypes
outl(
unsigned short int port,
unsigned int val)
#else
outl(port, val)
unsigned short int port;
unsigned int val;
#endif /* NeedFunctionPrototypes */
{
  __asm__ __volatile__("out%L0 (%1)" : :"a" (val), "d" (port));
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
inb(
unsigned short int port)
#else
inb(port)
unsigned short int port;
#endif /* NeedFunctionPrototypes */
{
  unsigned char ret;
  __asm__ __volatile__("in%B0 (%1)" :
		   "=a" (ret) :
		   "d" (port));
  return ret;
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
inw(
unsigned short int port)
#else
inw(port)
unsigned short int port;
#endif /* NeedFunctionPrototypes */
{
  unsigned short int ret;
  __asm__ __volatile__("in%W0 (%1)" :
		   "=a" (ret) :
		   "d" (port));
  return ret;
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
inl(
unsigned short int port)
#else
inl(port)
unsigned short int port;
#endif /* NeedFunctionPrototypes */
{
  unsigned int ret;
  __asm__ __volatile__("in%L0 (%1)" :
                   "=a" (ret) :
                   "d" (port));
  return ret;
}

#endif /* GCCUSESGAS */

#else /* !defined(FAKEIT) && !defined(__mc68000__) */

static __inline__ void
#if NeedFunctionPrototypes
outb(
unsigned short int port,
unsigned char val)
#else
outb(port, val)
unsigned short int port;
unsigned char val;
#endif /* NeedFunctionPrototypes */
{
}

static __inline__ void
#if NeedFunctionPrototypes
outw(
unsigned short int port,
unsigned short int val)
#else
outw(port, val)
unsigned short int port;
unsigned short int val;
#endif /* NeedFunctionPrototypes */
{
}

static __inline__ void
#if NeedFunctionPrototypes
outl(
unsigned short int port,
unsigned int val)
#else
outl(port, val)
unsigned short int port;
unsigned int val;
#endif /* NeedFunctionPrototypes */
{
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
inb(
unsigned short int port)
#else
inb(port)
unsigned short int port;
#endif /* NeedFunctionPrototypes */
{
  return 0;
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
inw(
unsigned short int port)
#else
inw(port)
unsigned short int port;
#endif /* NeedFunctionPrototypes */
{
  return 0;
}

static __inline__ unsigned int
#if NeedFunctionPrototypes
inl(
unsigned short int port)
#else
inl(port)
unsigned short int port;
#endif /* NeedFunctionPrototypes */
{
  return 0;
}

#endif /* FAKEIT */

#endif /* defined(mips) */
#endif /* defined(AlphaArchitecture) && defined(LinuxArchitecture) */

#else /* __GNUC__ */
#if !defined(AMOEBA) && !defined(MINIX)
# if defined(__STDC__) && (__STDC__ == 1)
#  ifndef asm
#   define asm __asm
#  endif
# endif
# ifdef SVR4
#  include <sys/types.h>
#  ifndef __HIGHC__
#   ifndef __USLC__
#    define __USLC__
#   endif
#  endif
# endif
# ifndef PC98
#  ifndef SCO325
#   include <sys/inline.h>
#  else
#   include "scoasm.h"
#  endif
# else
#if defined(PC98_PW) || defined(PC98_XKB) || defined(PC98_NEC) || defined(PC98_PWLB) || defined(PC98_GA968)
#define PW_PORT 0x600
extern short chipID;
#if NeedFunctionPrototypes
extern void *mmioBase;
#else
extern unsigned char *mmioBase;
#endif
extern unsigned short _port_tbl[];
#define	port_convert(x)	_port_tbl[(unsigned short)x]
#endif 

#if defined(PC98_TGUI) || defined(PC98_MGA)
#if NeedFunctionPrototypes
extern void *mmioBase;
#else
extern unsigned char *mmioBase;
#endif
#endif

asm     void _outl(port,val)
{
%reg	port,val;
	movl	port, %edx
	movl	val, %eax
	outl	(%dx)
%reg	port; mem	val;
	movl	port, %edx
	movl    val, %eax
	outl	(%dx)
%mem	port; reg	val;
	movw	port, %dx
	movl	val, %eax
	outl	(%dx)
%mem	port,val;
	movw	port, %dx
	movl    val, %eax
	outl	(%dx)
}

asm	void _outw(port,val)
{
%reg	port,val;
	movl	port, %edx
	movl	val, %eax
	data16
	outl	(%dx)
%reg	port; mem	val;
	movl	port, %edx
	movw	val, %ax
	data16
	outl	(%dx)
%mem	port; reg	val;
	movw	port, %dx
	movl	val, %eax
	data16
	outl	(%dx)
%mem	port,val;
	movw	port, %dx
	movw	val, %ax
	data16
	outl	(%dx)
}

asm	void _outb(port,val)
{
%reg	port,val;
	movl	port, %edx
	movl	val, %eax
	outb	(%dx)
%reg	port; mem	val;
	movl	port, %edx
	movb	val, %al
	outb	(%dx)
%mem	port; reg	val;
	movw	port, %dx
	movl	val, %eax
	outb	(%dx)
%mem	port,val;
	movw	port, %dx
	movb	val, %al
	outb	(%dx)
}

asm     int _inl(port)
{
%reg	port;
	movl	port, %edx
	inl	(%dx)
%mem	port;
	movw	port, %dx
	inl	(%dx)
}

asm	int _inw(port)
{
%reg	port;
	subl    %eax, %eax
	movl	port, %edx
	data16
	inl	(%dx)
%mem	port;
	subl    %eax, %eax
	movw	port, %dx
	data16
	inl	(%dx)
}

asm	int _inb(port)
{
%reg	port;
	subl    %eax, %eax
	movl	port, %edx
	inb	(%dx)
%mem	port;
	subl    %eax, %eax
	movw	port, %dx
	inb	(%dx)
}

#if defined(PC98_WAB) ||  defined(PC98_GANB_WAP)
static unsigned short
port_convert(unsigned short port)
{
     port <<= 8;
     port &= 0x7f00; /* Mask 0111 1111 0000 0000 */
     port |= 0xE0;
     return port;
}
#endif /* PC98_WAB || PC98_GANB_WAP */

#if defined(PC98_WABEP)
static unsigned short
port_convert(unsigned short port)
{
     port &= 0x7f; /* Mask 0000 0000 0111 1111 */
     port |= 0x0f00;
     return port;
}
#endif /* PC98_WABEP */

#ifdef PC98_WSNA
static unsigned short
port_convert(unsigned short port)
{
     port <<= 8;
     port &= 0x7f00; /* Mask 0111 1111 0000 0000 */
     port |= 0xE2;
     return port;
}
#endif /* PC98_WSNA */

#ifdef PC98_NKVNEC
#ifdef	PC98_NEC_CIRRUS2
static unsigned short
port_convert(unsigned short port)
{
     port = (port & 0xf) + ((port & 0xf0) << 4) + 0x0050;
     return port;
}
#else
static unsigned short
port_convert(unsigned short port)
{
     port = (port & 0xf) + ((port & 0xf0) << 4) + 0x00a0;
     return port;
}
#endif /* PC98_NEC_CIRRUS2 */
#endif /* PC98_NKVNEC */

static void outl(port,val)
{
#if defined(PC98_GANB_WAP) || defined(PC98_NKVNEC) || defined(PC98_WAB) || \
    defined(PC98_WABEP) || defined(PC98_WSNA) || defined(PC98_PW) || \
    defined(PC98_XKB) || defined(PC98_NEC)
   unsigned short tmp;
   tmp=port_convert(port);
   port=tmp;
#endif

#if defined(PC98_NEC)||defined(PC98_PWLB)||defined(PC98_TGUI)||defined(PC98_MGA)
   *(volatile unsigned int *)((char *)mmioBase+(port)) = (unsigned int)(val);
#else
   _outl(port,val);
#endif
}

static void outw(port,val)
{
#if defined(PC98_GANB_WAP) || defined(PC98_NKVNEC) || defined(PC98_WAB) || \
    defined(PC98_WABEP) || defined(PC98_WSNA) || defined(PC98_PW) || \
    defined(PC98_XKB) || defined(PC98_NEC)
   unsigned short tmp;
   tmp=port_convert(port);
   port=tmp;
#endif

#if defined(PC98_NEC)||defined(PC98_PWLB)||defined(PC98_TGUI)||defined(PC98_MGA)
   *(volatile unsigned short *)((char *)mmioBase+(port)) = (unsigned short)(val);
#else
   _outw(port,val);
#endif
}

static void outb(port,val)
{
#if defined(PC98_GANB_WAP) || defined(PC98_NKVNEC) || defined(PC98_WAB) || \
    defined(PC98_WABEP) || defined(PC98_WSNA) || defined(PC98_PW) || \
    defined(PC98_XKB) || defined(PC98_NEC)
   unsigned short tmp;
   tmp=port_convert(port);
   port=tmp;
#endif

#if defined(PC98_NEC)||defined(PC98_PWLB)||defined(PC98_TGUI)||defined(PC98_MGA)
   *(volatile unsigned char *)((char *)mmioBase+(port)) = (unsigned char)(val);
#else
   _outb(port,val);
#endif
}

static int inl(port)
{
   unsigned int ret;

#if defined(PC98_GANB_WAP) || defined(PC98_NKVNEC) || defined(PC98_WAB) || \
    defined(PC98_WABEP) || defined(PC98_WSNA) || defined(PC98_PW) || \
    defined(PC98_XKB) || defined(PC98_NEC)
   unsigned short tmp;
   tmp=port_convert(port);
   port=tmp;
#endif

#if defined(PC98_NEC)||defined(PC98_PWLB)||defined(PC98_TGUI)||defined(PC98_MGA)
   ret =*(volatile unsigned int *)((char *)mmioBase+(port));
#else
   ret = _inl(port);
#endif
   return ret;
}

static int inw(port)
{
   unsigned short ret;

#if defined(PC98_GANB_WAP) || defined(PC98_NKVNEC) || defined(PC98_WAB) || \
    defined(PC98_WABEP) || defined(PC98_WSNA) || defined(PC98_PW) || \
    defined(PC98_XKB) || defined(PC98_NEC)
   unsigned short tmp;
   tmp=port_convert(port);
   port=tmp;
#endif

#if defined(PC98_NEC)||defined(PC98_PWLB)||defined(PC98_TGUI)||defined(PC98_MGA)
   ret =*(volatile unsigned short *)((char *)mmioBase+(port));
#else
   ret = _inw(port);
#endif
   return ret;
}

static int inb(port)
{
   unsigned char ret;

#if defined(PC98_GANB_WAP) || defined(PC98_NKVNEC) || defined(PC98_WAB) || \
    defined(PC98_WABEP) || defined(PC98_WSNA) || defined(PC98_PW) || \
    defined(PC98_XKB) || defined(PC98_NEC)
   unsigned short tmp;
   tmp=port_convert(port);
   port=tmp;
#endif

#if defined(PC98_NEC)||defined(PC98_PWLB)||defined(PC98_TGUI)||defined(PC98_MGA)
   ret =*(volatile unsigned char *)((char *)mmioBase+(port));
#else
   ret = _inb(port);
#endif
   return ret;
}


# endif /* PC98 */
# if !defined(__HIGHC__) && !defined(SCO325)
#  pragma asm partial_optimization outl
#  pragma asm partial_optimization outw
#  pragma asm partial_optimization outb
#  pragma asm partial_optimization inl
#  pragma asm partial_optimization inw
#  pragma asm partial_optimization inb
# endif
#endif
#define ldq_u(p)	(*((unsigned long  *)(p)))
#define ldl_u(p)	(*((unsigned int   *)(p)))
#define ldw_u(p)	(*((unsigned short *)(p)))
#define stq_u(v,p)	((unsigned long  *)(p)) = (v)
#define stl_u(v,p)	((unsigned int   *)(p)) = (v)
#define stw_u(v,p)	((unsigned short *)(p)) = (v)
#define mem_barrier()   /* NOP */
#define write_mem_barrier()   /* NOP */
#endif /* __GNUC__ */

#if defined(IODEBUG) && defined(__GNUC__)
#undef inb
#undef inw
#undef inl
#undef outb
#undef outw
#undef outl
#define inb(a) __extension__ ({unsigned char __c=RealInb(a); ErrorF("inb(0x%03x) = 0x%02x\t@ line %4d, file %s\n", a, __c, __LINE__, __FILE__);__c;})
#define inw(a) __extension__ ({unsigned short __c=RealInw(a); ErrorF("inw(0x%03x) = 0x%04x\t@ line %4d, file %s\n", a, __c, __LINE__, __FILE__);__c;})
#define inl(a) __extension__ ({unsigned long __c=RealInl(a); ErrorF("inl(0x%03x) = 0x%08x\t@ line %4d, file %s\n", a, __c, __LINE__, __FILE__);__c;})

#define outb(a,b) (ErrorF("outb(0x%03x, 0x%02x)\t@ line %4d, file %s\n", a, b, __LINE__, __FILE__),RealOutb(a,b))
#define outw(a,b) (ErrorF("outw(0x%03x, 0x%04x)\t@ line %4d, file %s\n", a, b, __LINE__, __FILE__),RealOutw(a,b))
#define outl(a,b) (ErrorF("outl(0x%03x, 0x%08x)\t@ line %4d, file %s\n", a, b, __LINE__, __FILE__),RealOutl(a,b))
#endif

/*
 * This header sometimes gets included where is isn't needed, and on some
 * OSs this causes problems because the following functions generate
 * references to inb() and outb() which can't be resolved.  Defining
 * NO_COMPILER_H_EXTRAS avoids this problem.
 */

#ifndef NO_COMPILER_H_EXTRAS
/*
 *-----------------------------------------------------------------------
 * Port manipulation convenience functions
 *-----------------------------------------------------------------------
 */

#ifndef __GNUC__
#define __inline__ /**/
#endif

/*
 * rdinx - read the indexed byte port 'port', index 'ind', and return its value
 */
static __inline__ unsigned char 
#ifdef __STDC__
rdinx(unsigned short int port, unsigned char ind)
#else
rdinx(port, ind)
unsigned short int port;
unsigned char ind;
#endif
{
	if (port == 0x3C0)		/* reset attribute flip-flop */
		(void) inb(0x3DA);
	outb(port, ind);
	return(inb(port+1));
}

/*
 * wrinx - write 'val' to port 'port', index 'ind'
 */
static __inline__ void 
#ifdef __STDC__
wrinx(unsigned short int port, unsigned char ind, unsigned char val)
#else
wrinx(port, ind, val)
unsigned short int port;
unsigned char ind, val;
#endif
{
	outb(port, ind);
	outb(port+1, val);
}

/*
 * modinx - in register 'port', index 'ind', set the bits in 'mask' as in 'new';
 *	    the other bits are unchanged.
 */
static __inline__ void
#ifdef __STDC__
modinx(unsigned short int port, unsigned char ind, 
       unsigned char mask, unsigned char new)
#else
modinx(port, ind, mask, new)
unsigned short int port;
unsigned char ind, mask, new;
#endif
{
	unsigned char tmp;

	tmp = (rdinx(port, ind) & ~mask) | (new & mask);
	wrinx(port, ind, tmp);
}

/*
 * tstrg - returns true iff the bits in 'mask' of register 'port' are
 *	   readable & writable.
 */

static __inline__ int
#ifdef __STDC__
testrg(unsigned short int port, unsigned char mask)
#else
tstrg(port, mask)
unsigned short int port;
unsigned char mask;
#endif
{
	unsigned char old, new1, new2;

	old = inb(port);
	outb(port, old & ~mask);
	new1 = inb(port) & mask;
	outb(port, old | mask);
	new2 = inb(port) & mask;
	outb(port, old);
	return((new1 == 0) && (new2 == mask));
}

/*
 * testinx2 - returns true iff the bits in 'mask' of register 'port', index
 *	      'ind' are readable & writable.
 */
static __inline__ int
#ifdef __STDC__
testinx2(unsigned short int port, unsigned char ind, unsigned char mask)
#else
testinx2(port, ind, mask)
unsigned short int port;
unsigned char ind, mask;
#endif
{
	unsigned char old, new1, new2;

	old = rdinx(port, ind);
	wrinx(port, ind, old & ~mask);
	new1 = rdinx(port, ind) & mask;
	wrinx(port, ind, old | mask);
	new2 = rdinx(port, ind) & mask;
	wrinx(port, ind, old);
	return((new1 == 0) && (new2 == mask));
}

/*
 * testinx - returns true iff all bits of register 'port', index 'ind' are 
 *     	     readable & writable.
 */
static __inline__ int
#ifdef __STDC__
testinx(unsigned short int port, unsigned char ind)
#else
testinx(port, ind, mask)
unsigned short int port;
unsigned char ind;
#endif
{
	return(testinx2(port, ind, 0xFF));
}
#endif /* NO_COMPILER_H_EXTRAS */

#endif /* NO_INLINE */
#endif /* _COMPILER_H */
