/*
Copyright (C) 1995 Pascal Haible.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
PASCAL HAIBLE BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Except as contained in this notice, the name of Pascal Haible shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from
Pascal Haible.
*/

/* $XFree86: xc/programs/Xserver/os/xalloc.c,v 3.12.2.1 1997/05/10 07:03:02 hohndel Exp $ */

/* Only used if INTERNAL_MALLOC is defined
 * - otherwise xalloc() in utils.c is used
 */
#ifdef INTERNAL_MALLOC

#if defined(__STDC__) || defined(AMOEBA)
#ifndef NOSTDHDRS
#include <stdlib.h>	/* for malloc() etc. */
#endif
#else
extern char *malloc();
extern char *calloc();
extern char *realloc();
#endif

#include "Xos.h"
#include "misc.h"
#include "X.h"

#ifdef XALLOC_LOG
#include <stdio.h>
#endif

extern Bool Must_have_memory;

/*
 ***** New malloc approach for the X server *****
 * Pascal Haible 1995
 *
 * Some statistics about memory allocation of the X server
 * The test session included several clients of different size, including
 * xv, emacs and xpaint with a new canvas of 3000x2000, zoom 5.
 * All clients were running together.
 * A protocolling version of Xalloc recorded 318917 allocating actions
 * (191573 Xalloc, 85942 XNFalloc, 41438 Xrealloc, 279727 Xfree).
 * Results grouped by size, excluding the next lower size
 * (i.e. size=32 means 16<size<=32):
 *
 *    size   nr of alloc   max nr of blocks allocated together
 *       8	1114		287
 *      16	17341		4104
 *      32	147352		2068
 *      64	59053		2518
 *     128	46882		1230
 *     256	20544		1217
 *     512	6808		117
 *    1024	8254		171
 *    2048	4841		287
 *    4096	2429		84
 *    8192	3364		85
 *   16384	573		22
 *   32768	49		7
 *   65536	45		5
 *  131072	48		2
 *  262144	209		2
 *  524288	7		4
 * 1048576	2		1
 * 8388608	2		2
 *
 * The most used sizes:
 * count size
 * 24	136267
 * 40	37055
 * 72	17278
 * 56	13504
 * 80	9372
 * 16	8966
 * 32	8411
 * 136	8399
 * 104	7690
 * 12	7630
 * 120	5512
 * 88	4634
 * 152	3062
 * 52	2881
 * 48	2736
 * 156	1569
 * 168	1487
 * 160	1483
 * 28	1446
 * 1608	1379
 * 184	1305
 * 552	1270
 * 64	934
 * 320	891
 * 8	754
 *
 * Conclusions: more than the half of all allocations are <= 32 bytes.
 * But of these about 150,000 blocks, only a maximum of about 6,000 are
 * allocated together (including memory leaks..).
 * On the other side, only 935 of the 191573 or 0.5% were larger than 8kB
 * (362 or 0.2% larger than 16k).
 *
 * What makes the server really grow is the fragmentation of the heap,
 * and the fact that it can't shrink.
 * To cure this, we do the following:
 * - large blocks (>=11k) are mmapped on xalloc, and unmapped on xfree,
 *   so we don't need any free lists etc.
 *   As this needs 2 system calls, we only do this for the quite
 *   infrequent large (>=11k) blocks.
 * - instead of reinventing the wheel, we use system malloc for medium
 *   sized blocks (>256, <11k).
 * - for small blocks (<=256) we use an other approach:
 *   As we need many small blocks, and most ones for a short time,
 *   we don't go through the system malloc:
 *   for each fixed sizes a seperate list of free blocks is kept.
 *   to KISS (Keep it Small and Simple), we don't free them
 *   (not freeing a block of 32 bytes won't be worse than having fragmented
 *   a larger area on allocation).
 *   This way, we (almost) allways have a fitting free block right at hand,
 *   and don't have to walk any lists.
 */

/*
 * structure layout of a allocated block
 * unsigned long	size:
 *				rounded up netto size for small and medium blocks
 *				brutto size == mmap'ed area for large blocks
 * unsigned long	DEBUG ? MAGIC : unused
 * ....			data
 * ( unsigned long	MAGIC2 ) only if SIZE_TAIL defined
 *
 */
 
/* use otherwise unused long in the header to store a magic */
/* shouldn't this be removed for production release ? */
#define XALLOC_DEBUG

#ifdef XALLOC_DEBUG
/* Xfree fills the memory with a certain pattern (currently 0xF0) */
/* this should really be removed for production release! */
#define XFREE_ERASES
#endif

/* this must be a multiple of SIZE_STEPS below */
#define MAX_SMALL 264		/* quite many blocks of 264 */

#define MIN_LARGE (11*1024)
/* worst case is 25% loss with a page size of 4k */

/* SIZE_STEPS defines the granularity of size of small blocks -
 * this makes blocks align to that, too! */
#define SIZE_STEPS		(sizeof(double))
#define SIZE_HEADER		(2*sizeof(long)) /* = sizeof(double) for 32bit */
#ifdef XALLOC_DEBUG
#if defined(__sparc__) || defined(__hppa__)
#define SIZE_TAIL		(2*sizeof(long)) /* = sizeof(double) for 32bit */
#else
#define SIZE_TAIL		(sizeof(long))
#endif
#endif

#undef TAIL_SIZE
#ifdef SIZE_TAIL
#define TAIL_SIZE		SIZE_TAIL
#else
#define TAIL_SIZE		0
#endif

#ifdef __alpha__
#define MAGIC			0x1404196414071968
#define MAGIC2			0x2515207525182079
#else
#define MAGIC			0x14071968
#define MAGIC2			0x25182079
#endif

/* To get some statistics about memory allocation */

#ifdef XALLOC_LOG
#define XALLOC_LOG_FILE "/tmp/Xalloc.log"	/* unsecure... */
#define LOG_BODY(_body)					\
		{ FILE *f;				\
		  f = fopen(XALLOC_LOG_FILE, "a");	\
		  if (NULL!=f) {			\
			_body;				\
			fclose(f);			\
		  }					\
		}
#if defined(linux) && defined(i386)
#define LOG_ALLOC(_fun, _size, _ret)						\
	{	unsigned long *from;						\
		__asm__("movl %%ebp,%0" : /*OUT*/ "=r" (from) : /*IN*/ );	\
		LOG_BODY(fprintf(f, "%s\t%i\t%p\t[%lu]\n", _fun, _size, _ret, *(from+1))) \
	}
#else
#define LOG_ALLOC(_fun, _size, _ret)				\
	LOG_BODY(fprintf(f, "%s\t%i\t%p\n", _fun, _size, _ret))
#endif
#define LOG_REALLOC(_fun, _ptr, _size, _ret)			\
	LOG_BODY(fprintf(f, "%s\t%p\t%i\t%p\n", _fun, _ptr, _size, _ret))
#define LOG_FREE(_fun, _ptr)					\
	LOG_BODY(fprintf(f, "%s\t%p\n", _fun, _ptr))
#else
#define LOG_ALLOC(_fun, _size, _ret)
#define LOG_REALLOC(_fun, _ptr, _size, _ret)
#define LOG_FREE(_fun, _ptr)
#endif /* XALLOC_LOG */

static unsigned long *free_lists[MAX_SMALL/SIZE_STEPS];

/*
 * systems that support it should define HAS_MMAP_ANON or MMAP_DEV_ZERO
 * and include the appropriate header files for
 * mmap(), munmap(), PROT_READ, PROT_WRITE, MAP_PRIVATE,
 * PAGE_SIZE or _SC_PAGESIZE (and MAP_ANON for HAS_MMAP_ANON).
 *
 * systems that don't support MAP_ANON fall through to the 2 fold behaviour
 */

#if defined(linux)
#define HAS_MMAP_ANON
#include <sys/types.h>
#include <sys/mman.h>
#include <asm/page.h>	/* PAGE_SIZE */
#endif /* linux */

#if defined(CSRG_BASED)
#define HAS_MMAP_ANON
#define HAS_GETPAGESIZE
#include <sys/types.h>
#include <sys/mman.h>
#endif /* CSRG_BASED */

#if defined(SVR4)
#define MMAP_DEV_ZERO
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#endif /* SVR4 */

#if defined(sun) && !defined(SVR4) /* SunOS */
#define MMAP_DEV_ZERO	/* doesn't SunOS have MAP_ANON ?? */
#define HAS_GETPAGESIZE
#include <sys/types.h>
#include <sys/mman.h>
#endif /* sun && !SVR4 */

#ifdef XNO_SYSCONF
#undef _SC_PAGESIZE
#endif

#if defined(HAS_MMAP_ANON) || defined (MMAP_DEV_ZERO)
static int pagesize;
#endif

#ifdef MMAP_DEV_ZERO
static int devzerofd = -1;
#include <errno.h>
#ifdef X_NOT_STDC_ENV
extern int errno;
#endif
#endif


unsigned long *
Xalloc (amount)
    unsigned long amount;
{
    register unsigned long *ptr;
    int indx;

    /* sanity checks */

    /* zero size requested */
    if (amount == 0) {
	LOG_ALLOC("Xalloc=0", amount, 0);
	return (unsigned long *)NULL;
    }
    /* negative size (or size > 2GB) - what do we do? */
    if ((long)amount < 0) {
	/* Diagnostic */
#ifdef FATALERRORS
 	FatalError("Xalloc: Xalloc(<0)\n");
#else
 	ErrorF("Xalloc warning: Xalloc(<0) ignored..\n");
#endif
 	LOG_ALLOC("Xalloc<0", amount, 0);
	return (unsigned long *)NULL;
    }

    /* alignment check */
#if defined(__alpha__) || defined(__sparc__) || defined(__mips__) || defined(__hppa__)
    amount = (amount + (sizeof(long)-1)) & ~(sizeof(long)-1);
#endif

    if (amount <= MAX_SMALL) {
	/*
	 * small block
	 */
	/* pick a ready to use small chunk */
	indx = (amount-1) / SIZE_STEPS;
	ptr = free_lists[indx];
	if (NULL == ptr) {
		/* list empty - get 20 or 40 more */
		/* amount = size rounded up */
		amount = (indx+1) * SIZE_STEPS;
		ptr = (unsigned long *)calloc(1,(amount+SIZE_HEADER+TAIL_SIZE)
						* (amount<100 ? 40 : 20));
		if (NULL!=ptr) {
			int i;
			unsigned long *p1, *p2;
			p2 = (unsigned long *)((char *)ptr + SIZE_HEADER);
			for (i=0; i<(amount<100 ? 40 : 20); i++) {
				p1 = p2;
				p1[-2] = amount;
#ifdef XALLOC_DEBUG
				p1[-1] = MAGIC;
#endif /* XALLOC_DEBUG */
#ifdef SIZE_TAIL
				*(unsigned long *)((unsigned char *)p1 + amount) = MAGIC2;
#endif /* SIZE_TAIL */
				p2 = (unsigned long *)((char *)p1 + SIZE_HEADER + amount + TAIL_SIZE);
				*(unsigned long **)p1 = p2;
			}
			/* last one has no next one */
			*(unsigned long **)p1 = NULL;
			/* put the second in the list */
			free_lists[indx] = (unsigned long *)((char *)ptr + SIZE_HEADER + amount + TAIL_SIZE + SIZE_HEADER);
			/* take the fist one */
			ptr = (unsigned long *)((char *)ptr + SIZE_HEADER);
			LOG_ALLOC("Xalloc-S", amount, ptr);
			return ptr;
		} /* else fall through to 'Out of memory' */
	} else {
		/* take that piece of mem out of the list */
		free_lists[indx] = *((unsigned long **)ptr);
		/* already has size (and evtl. magic) filled in */
		LOG_ALLOC("Xalloc-S", amount, ptr);
		return ptr;
	}

#if defined(HAS_MMAP_ANON) || defined(MMAP_DEV_ZERO)
    } else if (amount >= MIN_LARGE) {
	/*
	 * large block
	 */
	/* mmapped malloc */
	/* round up amount */
	amount += SIZE_HEADER + TAIL_SIZE;
	/* round up brutto amount to a multiple of the page size */
	amount = (amount + pagesize-1) & ~(pagesize-1);
#ifdef MMAP_DEV_ZERO
	ptr = (unsigned long *)mmap((caddr_t)0,
					(size_t)amount,
					PROT_READ | PROT_WRITE,
					MAP_PRIVATE,
					devzerofd,
					(off_t)0);
#else
	ptr = (unsigned long *)mmap((caddr_t)0,
					(size_t)amount,
					PROT_READ | PROT_WRITE,
					MAP_ANON | MAP_PRIVATE,
					-1,
					(off_t)0);
#endif
	if (-1!=(long)ptr) {
		ptr[0] = amount - SIZE_HEADER - TAIL_SIZE;
#ifdef XALLOC_DEBUG
		ptr[1] = MAGIC;
#endif /* XALLOC_DEBUG */
#ifdef SIZE_TAIL
# ifdef __hppa__
		/* reserved space for 2 * sizeof(long), so use correct one */
		/* see SIZE_TAIL macro */
		((unsigned long *)((char *)ptr + amount))[-2] = MAGIC2;
# else
		((unsigned long *)((char *)ptr + amount))[-1] = MAGIC2;
# endif /* __hppa__ */
#endif /* SIZE_TAIL */
		ptr = (unsigned long *)((char *)ptr + SIZE_HEADER);
		LOG_ALLOC("Xalloc-L", amount, ptr);
		return ptr;
	} /* else fall through to 'Out of memory' */
#endif /* HAS_MMAP_ANON || MMAP_DEV_ZERO */
    } else {
	/*
	 * medium sized block
	 */
	/* 'normal' malloc() */
	ptr=(unsigned long *)calloc(1,amount+SIZE_HEADER+TAIL_SIZE);
	if (ptr != (unsigned long *)NULL) {
		ptr[0] = amount;
#ifdef XALLOC_DEBUG
		ptr[1] = MAGIC;
#endif /* XALLOC_DEBUG */
#ifdef SIZE_TAIL
		*(unsigned long *)((char *)ptr + amount + SIZE_HEADER) = MAGIC2;
#endif /* SIZE_TAIL */
		ptr = (unsigned long *)((char *)ptr + SIZE_HEADER);
		LOG_ALLOC("Xalloc-M", amount, ptr);
		return ptr;
	}
    }
    if (Must_have_memory)
	FatalError("Out of memory");
    LOG_ALLOC("Xalloc-oom", amount, 0);
    return (unsigned long *)NULL;
}

/*****************
 * XNFalloc 
 * "no failure" realloc, alternate interface to Xalloc w/o Must_have_memory
 *****************/

unsigned long *
XNFalloc (amount)
    unsigned long amount;
{
    register unsigned long *ptr;

    /* zero size requested */
    if (amount == 0) {
	LOG_ALLOC("XNFalloc=0", amount, 0);
	return (unsigned long *)NULL;
    }
    /* negative size (or size > 2GB) - what do we do? */
    if ((long)amount < 0) {
	/* Diagnostic */
#ifdef FATALERRORS
	FatalError("Xalloc: XNFalloc(<0)\n");
#else
	ErrorF("Xalloc warning: XNFalloc(<0) ignored..\n");
#endif
 	LOG_ALLOC("XNFalloc<0", amount, 0);
	return (unsigned long *)NULL;
    }
    ptr = Xalloc(amount);
    if (!ptr)
    {
        FatalError("Out of memory");
    }
    return ptr;
}

/*****************
 * Xcalloc
 *****************/

unsigned long *
Xcalloc (amount)
    unsigned long   amount;
{
    unsigned long   *ret;

    ret = Xalloc (amount);
    if (ret
#if defined(HAS_MMAP_ANON) || defined(MMAP_DEV_ZERO)
	    && (amount < MIN_LARGE)	/* mmaped anonymous mem is already cleared */
#endif
       )
	bzero ((char *) ret, (int) amount);
    return ret;
}

/*****************
 * Xrealloc
 *****************/

unsigned long *
Xrealloc (ptr, amount)
    register pointer ptr;
    unsigned long amount;
{
    register unsigned long *new_ptr;

    /* zero size requested */
    if (amount == 0) {
	if (ptr)
		Xfree(ptr);
	LOG_REALLOC("Xrealloc=0", ptr, amount, 0);
	return (unsigned long *)NULL;
    }
    /* negative size (or size > 2GB) - what do we do? */
    if ((long)amount < 0) {
	/* Diagnostic */
#ifdef FATALERRORS
	FatalError("Xalloc: Xrealloc(<0)\n");
#else
	ErrorF("Xalloc warning: Xrealloc(<0) ignored..\n");
#endif
	if (ptr)
		Xfree(ptr);	/* ?? */
	LOG_REALLOC("Xrealloc<0", ptr, amount, 0);
	return (unsigned long *)NULL;
    }

    new_ptr = Xalloc(amount);
    if ( (new_ptr) && (ptr) ) {
	unsigned long old_size;
	old_size = ((unsigned long *)ptr)[-2];
#ifdef XALLOC_DEBUG
	if (MAGIC != ((unsigned long *)ptr)[-1]) {
#ifdef FATALERRORS
		FatalError("Xalloc error: header corrupt in Xrealloc() :-(\n");
#else
		ErrorF("Xalloc error: header corrupt in Xrealloc() :-(\n");
#endif
		LOG_REALLOC("Xalloc error: header corrupt in Xrealloc() :-(",
			ptr, amount, 0);
		return (unsigned long *)NULL;
	}
#endif /* XALLOC_DEBUG */
	/* copy min(old size, new size) */
	memcpy((char *)new_ptr, (char *)ptr, (amount < old_size ? amount : old_size));
    }
    if (ptr)
	Xfree(ptr);
    if (new_ptr) {
	LOG_REALLOC("Xrealloc", ptr, amount, new_ptr);
	return new_ptr;
    }
    if (Must_have_memory)
	FatalError("Out of memory");
    LOG_REALLOC("Xrealloc", ptr, amount, 0);
    return (unsigned long *)NULL;
}
                    
/*****************
 * XNFrealloc 
 * "no failure" realloc, alternate interface to Xrealloc w/o Must_have_memory
 *****************/

unsigned long *
XNFrealloc (ptr, amount)
    register pointer ptr;
    unsigned long amount;
{
    if (( ptr = (pointer)Xrealloc( ptr, amount ) ) == NULL)
    {
        FatalError( "Out of memory" );
    }
    return ((unsigned long *)ptr);
}

/*****************
 *  Xfree
 *    calls free 
 *****************/    

void
Xfree(ptr)
    register pointer ptr;
{
    unsigned long size;
    unsigned long *pheader;

    /* free(NULL) IS valid :-(  - and widely used throughout the server.. */
    if (!ptr)
	return;

    pheader = (unsigned long *)((char *)ptr - SIZE_HEADER);
#ifdef XALLOC_DEBUG
    if (MAGIC != pheader[1]) {
	/* Diagnostic */
#ifdef FATALERRORS
	FatalError("Xalloc error: Header corrupt in Xfree() :-(\n");
#else
	ErrorF("Xalloc error: Header corrupt in Xfree() :-(\n");
#endif
	LOG_FREE("Xalloc error:  Header corrupt in Xfree() :-(", ptr);
	return;
    }
#endif /* XALLOC_DEBUG */

    size = pheader[0];
    if (size <= MAX_SMALL) {
	int indx;
	/*
	 * small block
	 */
#ifdef SIZE_TAIL
	if (MAGIC2 != *(unsigned long *)((char *)ptr + size)) {
		/* Diagnostic */
#ifdef FATALERRORS
		FatalError("Xalloc error: Tail corrupt in Xfree() for small block (adr=0x%x, val=0x%x)\n",(char *)ptr + size,*(unsigned long *)((char *)ptr + size));
#else
		ErrorF("Xalloc error: Tail corrupt in Xfree() for small block (adr=0x%x, val=0x%x)\n",(char *)ptr + size,*(unsigned long *)((char *)ptr + size));
#endif
		LOG_FREE("Xalloc error: Tail corrupt in Xfree() for small block", ptr);
		return;
	}
#endif /* SIZE_TAIL */

#ifdef XFREE_ERASES
	memset(ptr,0xF0,size);
#endif /* XFREE_ERASES */

	/* put this small block at the head of the list */
	indx = (size-1) / SIZE_STEPS;
	*(unsigned long **)(ptr) = free_lists[indx];
	free_lists[indx] = (unsigned long *)ptr;
	LOG_FREE("Xfree", ptr);
	return;

#if defined(HAS_MMAP_ANON) || defined(MMAP_DEV_ZERO)
    } else if (size >= MIN_LARGE) {
	/*
	 * large block
	 */
#ifdef SIZE_TAIL
	if (MAGIC2 != ((unsigned long *)((char *)ptr + size))[0]) {
		/* Diagnostic */
#ifdef FATALERRORS
		FatalError("Xalloc error: Tail corrupt in Xfree() for big block (adr=0x%x, val=0x%x)\n",(char *)ptr+size,((unsigned long *)((char *)ptr + size))[0]);
#else
		ErrorF("Xalloc error: Tail corrupt in Xfree() for big block (adr=0x%x, val=0x%x)\n",(char *)ptr+size,((unsigned long *)((char *)ptr + size))[0]);
#endif
		LOG_FREE("Xalloc error: Tail corrupt in Xfree() for big block", ptr);
		return;
	}
	size += SIZE_TAIL;
#endif /* SIZE_TAIL */

	LOG_FREE("Xfree", ptr);
	size += SIZE_HEADER;
	munmap((caddr_t)pheader, (size_t)size);
	/* no need to clear - mem is inaccessible after munmap.. */
#endif /* HAS_MMAP_ANON */

    } else {
	/*
	 * medium sized block
	 */
#ifdef SIZE_TAIL
	if (MAGIC2 != *(unsigned long *)((char *)ptr + size)) {
		/* Diagnostic */
#ifdef FATALERRORS
		FatalError("Xalloc error: Tail corrupt in Xfree() for medium block (adr=0x%x, val=0x%x)\n",(char *)ptr + size,*(unsigned long *)((char *)ptr + size));
#else
		ErrorF("Xalloc error: Tail corrupt in Xfree() for medium block (adr=0x%x, val=0x%x)\n",(char *)ptr + size,*(unsigned long *)((char *)ptr + size));
#endif
		LOG_FREE("Xalloc error: Tail corrupt in Xfree() for medium block", ptr);
		return;
	}
#endif /* SIZE_TAIL */

#ifdef XFREE_ERASES
	memset(pheader,0xF0,size+SIZE_HEADER);
#endif /* XFREE_ERASES */

	LOG_FREE("Xfree", ptr);
	free((char *)pheader);
    }
}

void
OsInitAllocator ()
{
    static Bool beenhere = FALSE;

    if (beenhere)
	return;
    beenhere = TRUE;

#if defined(HAS_MMAP_ANON) || defined (MMAP_DEV_ZERO)
#if defined(_SC_PAGESIZE) /* || defined(linux) */
    pagesize = sysconf(_SC_PAGESIZE);
#else
#ifdef HAS_GETPAGESIZE
    pagesize = getpagesize();
#else
    pagesize = PAGE_SIZE;
#endif
#endif
#endif

    /* set up linked lists of free blocks */
    bzero ((char *) free_lists, MAX_SMALL/SIZE_STEPS*sizeof(unsigned long *));

#ifdef MMAP_DEV_ZERO
    /* open /dev/zero on systems that have mmap, but not MAP_ANON */
    if (devzerofd < 0) {
	if ((devzerofd = open("/dev/zero", O_RDWR, 0)) < 0)
	    FatalError("OsInitAllocator: Cannot open /dev/zero (errno=%d)\n",
			errno);
    }
#endif

#ifdef XALLOC_LOG
    /* reset the log file to zero length */
    {
	FILE *f;
	f = fopen(XALLOC_LOG_FILE, "w");
	if (NULL!=f)
		fclose(f);
    }
#endif
}

#else /* !INTERNAL_MALLOC */
/* This is to avoid an empty .o */
static int no_internal_xalloc;
#endif /* INTERNAL_MALLOC */
