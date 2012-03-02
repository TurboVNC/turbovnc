/* $Xorg: t1malloc.c,v 1.3 2000/08/17 19:46:34 cpqbld Exp $ */
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
/* $XFree86: xc/lib/font/Type1/t1malloc.c,v 1.11 2002/02/18 20:51:57 herrb Exp $ */
 /* MALLOC   CWEB         V0004 LOTS                                 */
/*
:h1.MALLOC - Fast Memory Allocation
 
This module is meant to provide portable C-style memory allocation
routines (malloc/free).
 
&author. Jeffrey B. Lotspiech (lotspiech@almaden.ibm.com)
 
*/

#ifdef FONTMODULE
#include "Xdefs.h"	/* Bool declaration */
#include "Xmd.h"	/* INT32 declaration */
#include "os.h"
#include "xf86_ansic.h"
#else
#include "os.h"
#endif
#include "objects.h"	/* get #define for Abort() */


/*
:h3.Define NULL
 
In the beginning, C compilers made no assumptions about NULL.  It was
even theoretically possible that NULL would not be 0.  ANSI has tied
this down a bit.  The following definition seems to be the most
popular (in terms of reducing compiler complaints), however, if your
compiler is unhappy about it, you can redefine it on the command line:
*/
#ifndef   NULL
#include <stddef.h>
#endif
/*
Of course, NULL is important because xiMalloc() is defined to return
NULL when out of memory.
 
:h2.Data Structures Used to Manage Free Memory
 
:h3.The "freeblock" Structure
 
The list of available memory blocks is a doubly-linked list.  Each
block begins with the following structure:
*/
 
struct freeblock {
        long size;                      /* number of 'longs' in block,
                                           including this header */
        struct freeblock *fore;        /* forward in doubly-linked list */
        struct freeblock *back;        /* backward in doubly-linked list */
} ;
/*
In addition, each free block has a TRAILER that is simply the 'size'
repeated.  Thus 'size' is found at the beginning of the block and at the
end of the block (size-1 longs away).  'size' includes both the header
and the trailer.
 
When a block is allocated, its 'size' is turned negative (both at the
beginning and at the end).  Thus, checking whether two blocks may be
combined is very simple.  We merely examine both neighboring blocks'
size to see if they are positive (and hence available for combination).
 
The memory address returned to the user is therefore one "long" below the
size, and one extra "long" is added to the end of the block (beyond what
the user requested) to store the trailing size.
 
:h3."firstfree" and "lastfree", the Anchors to the Free List
 
"firstfree" points to the first available free block; "lastfree" points
to the end of the chain of available blocks.  These are linked together
by initialization code; see :hdref refid=addmem..
*/
 
static struct freeblock firstfree = { 0L, NULL, NULL };
static struct freeblock lastfree = { 0L, NULL, NULL };
 
/*
:h3."firstcombined" and "uncombined", Keeping Track of Uncombined Blocks
 
This module is designed to make the combining of adjacent free memory
blocks be very fast.  Nonetheless, combining blocks is naturally the
most expensive part of any memory system.  In an X system,
it is worthwhile to defer the combination for a while, because
frequently we will end up asking for a block of exactly the same
size that we recently returned and we can save ourselves some work.
 
"MAXUNCOMBINED" is the maximum number of uncombined blocks that we will
allow at any time:
*/
 
#define   MAXUNCOMBINED  3
 
/*
"firstcombined" is a pointer into the free list.  The uncombined blocks
are always at the front of the list.  "firstcombined" points to the
first block that has been combined.
*/
static struct freeblock *firstcombined = &lastfree;
static short uncombined = 0; /* current number of uncombined blocks          */
 
/*
Uncombined blocks have a negative 'size'; in this they are like
allocated blocks.
 
We store a distinctive hex pattern in 'size' when we combine a block
to help us debug:
*/
#define   COMBINED   0xBADBAD
 
/*
:h3.DEBUGWORDS - Extra Memory Saved With Each Block for Debug
 
We add 'DEBUGWORDS' words to each allocated block to put interesting
debug information:
*/
#ifndef   DEBUGWORDS
#define   DEBUGWORDS   0
#endif
 
/*
:h3.MINEXCESS - Amount of "Excess" We Would be Willing to Ignore
 
When we search the free list to find memory for a user request, we
frequently find an area that is bigger than what the user has asked for.
Normally we put the remaining words (the excess) back on the free list.
However, if the area is just slightly bigger than what the user needs,
it is counter-productive to do this, as the small amount recovered tends
to hurt by increasing memory fragmentation rather than help by providing
more available memory.  "MINEXCESS" is the number of words that must be
recovered before we would bother to put the excess back on the free
list.  If there is not enough excess, we just give the user more than he
asked for.
*/
 
#define   MINEXCESS      (7 + DEBUGWORDS)
 
/*
:h3.Some Flags for Debug
*/
 
long AvailableWords = 0;     /* number of words available in memory          */
char mallocdebug = 0;        /* a flag that enables some chatty printf's     */

/*
:h3.Prototypes of static functions
*/

static void combine ( void );
static void freeuncombinable ( long *addr, long size );
static void unhook ( struct freeblock *p );
static void dumpchain ( void );
#ifdef notused
static void reportarea ( long *area );
#endif
 
/*
:h3.whocalledme() - Debug for Memory Leaks
 
This routine is 68000-specific; it copies the value of the application's
curOper variable (which is often a pointer to a character string), and
the first part of the stack at the time malloc was called into the
DEBUGWORDS area reserved with each block.
We use it to see who is malloc-ing memory without free-ing it.
*/
 
#if DEBUGWORDS
 
static void 
whocalledme(long *addr,      /* address of memory block                      */
	    long *stack)     /* address of malloc's parameter on stack       */
{
       register long size;   /* size of memory block                         */
       register int i;       /* loop index                                   */
       extern char *curOper; /* ptr to last operator (kept by appl.) */
 
       stack--;
       size = - *addr;
 
       addr += size - 1 - DEBUGWORDS;
       *addr++ = (long) curOper;
       for (i=0; i < DEBUGWORDS-1; i++)
               *addr++ = *stack++;
}
#else
 
#define whocalledme(addr, stack)
 
#endif
/*
:h2.xiFree() - User-Callable "Return Memory" Routine
 
The actual beginning of the block is one 'long' before the address we
gave to the user.  The block begins and ends with '-size' in words.
*/
 
void 
xiFree(long *addr)                     /* user's memory to be returned   */
{
        register long size;            /* amount of memory in this block */
        register struct freeblock *p;  /* identical to 'addr'            */
 
        if (addr == NULL) {  /* common "mistake", so allow it (CHT) */
            printf("\nxiFree(NULL)?\n");
            return;
        }
 
        size = *--addr;
/*
Make sure this address looks OK; 'size' must be less than zero (meaning
the block is allocated) and should be repeated at the end of the block.
*/
        if (size >= 0)
                Abort("free: bad size");
        if (addr[-1 - size] != size)
                Abort("free: mismatched size");
/*
Now make this a 'freeblock' structure and tack it on the FRONT of the
free list (where uncombined blocks go):
*/
        AvailableWords -= size;  /* actually INCREASES AvailableWords */
        p = (struct freeblock *) addr;
        p->back = &firstfree;
        (p->fore = firstfree.fore)->back = p;
        firstfree.fore = p;
/*
If we have too many uncombined blocks, call combine() to combine one.
*/
        if (++uncombined > MAXUNCOMBINED) {
                combine();
                if (mallocdebug) {
                        printf("xiFree(%p) with combine, ", addr);
                        dumpchain();
                }
        }
        else {
                if (mallocdebug) {
                        printf("xiFree(%p), ", addr);
                        dumpchain();
                }
        }
 
        return;
}
 
/*
:h3.combine() - Subroutine of xiFree() to Combine Blocks
 
This routine tries to combine the block just before 'firstcombined'.
In any event, that block will be moved to the end of the list (after
'firstcombined').
*/
 
static void
combine(void)
{
        register struct freeblock *p;   /* block we will try to combine */
        register long *addr;            /* identical to 'p' for 'long' access */
        register long size;             /* size of this block  */
        register long size2;            /* size of potential combinee */
 
        p = firstcombined->back;
        if (p == &firstfree)
               Abort("why are we combining?");
 
        addr = (long *) p;
        size = - p->size;
        if (--uncombined < 0)
                Abort("too many combine()s");
 
        if (addr[-1] < 0 && addr[size] < 0) {
/*
We special case the situation where no combining can be done.  Then, we
just mark the chain "combined" (i.e., positive size), move the
'firstcombined' pointer back in the chain, and return.
*/
                addr[0] = addr[size - 1] = size;
                firstcombined = (struct freeblock *) addr;
                return;
        }
/*
Otherwise, we unhook this pointer from the chain:
*/
        unhook(p);
/*
First we attempt to combine this with the block immediately above:
*/
        size2 = addr[-1];
        if (size2 > 0) {     /* i.e., block above is free */
                *addr = COMBINED;  /* might help debug */
                addr -= size2;
                if (addr[0] != size2)
                        Abort("bad block above");
                unhook((struct freeblock *)addr);
                size += size2;
        }
/*
At this point 'addr' and 'size' may be the original block, or it may be
the newly combined block.  Now we attempt to combine it with the block
below:
*/
        p = (struct freeblock *) (addr + size);
        size2 = p->size;
 
        if (size2 > 0) {     /* i.e., block below is free                    */
                p->size = COMBINED;
                if (size2 != ((long *) p)[size2 - 1])
                        Abort("bad block below");
                unhook(p);
                size += size2;
        }
/*
Finally we take the newly combined block and put it on the end of the
chain by calling the "freeuncombinable" subroutine:
*/
        freeuncombinable(addr, size);
}
 
/*
:h3.freeuncombinable() - Free a Block That Need Not be Combined
 
This block is "uncombinable" either because we have already combined
it with its eligible neighbors, or perhaps because we know it has
no neighbors.
*/
 
static void
freeuncombinable(long *addr,  /* address of the block to be freed            */
		 long size)   /* size of block in words                      */
{
        register struct freeblock *p;  /* a convenient synonym for 'addr'    */
 
/*
Mark block allocated and combined by setting its 'size' positive:
*/
        addr[size - 1] = addr[0] = size;
/*
Now tack the block on the end of the doubly-linked free list:
*/
        p = (struct freeblock *) addr;
        p->fore = &lastfree;
        (p->back = lastfree.back)->fore = p;
        lastfree.back = p;
/*
If we have previously had no combined blocks, we must update
'firstcombined' to point to this block:
*/
        if (firstcombined->fore == NULL)
                firstcombined = p;
}
 
/*
:h3.unhook() - Unhook a Block from the Doubly-linked List
 
The only tricky thing here is to make sure that 'firstcombined' is
updated if this block happened to be the old 'firstcombined'.  (We
would never be unhooking 'firstfree' or 'lastfree', so we do not
have to worry about the end cases.)
*/
 
static void
unhook(struct freeblock *p)            /* block to unhook                    */
{
        p->back->fore = p->fore;
        p->fore->back = p->back;
 
        if (firstcombined == p)
                firstcombined = p->fore;
}
/*
:h2.xiMalloc() - Main User Entry Point for Getting Memory
 
We have two slightly different versions of xiMalloc().  In the case
where we have TYPE1IMAGER and a font cache, we are prepared, when nominally
out of memory, to loop calling TYPE1IMAGER's GimeSpace() to release font
cache.
*/
 
/* The following code put in by MDC on 11/10/90 */
 
#ifdef TYPE1IMAGER
 
static char *malloc_local(unsigned size);
 
char *
xiMalloc(unsigned size)
{
  char *memaddr;
 
  while ( (memaddr = malloc_local(size)) == NULL ) {
    /* Ask TYPE1IMAGER to give us some of its cache back */
    if ( I_GimeSpace() == 0 ) break; /* We are really, really, out of memory */
  }
 
  return(memaddr);
}
#endif
 
/*
Now begins the real workhorse xiMalloc() (called 'malloc_local' if
we are taking advantage of TYPE1IMAGER).  Its argument is an unsigned;
at least that lets users with 16-bit integers get a 64K chunk of
memory, and it is also compatible with the definition of a "size_t"
in most systems.
*/
#ifdef TYPE1IMAGER
static char *
malloc_local(unsigned Size)  /* number of bytes the user requested           */
#else
char *
xiMalloc(unsigned Size)
#endif
{
        register long size = (long)Size;  /* a working register for size     */
        register struct freeblock *p;  /* tentative block to be returned     */
        register long excess; /* words in excess of user request             */
        register long *area; /* a convenient synonym for 'p'                 */
 
/*
First, we increase 'size' to allow for the two size fields we will
save with the block, plus any information for debug purposes.
Then we ensure that the block will be large enough to hold our
'freeblock' information.  Finally we convert it to be in words
(longs), not bytes, increased to span an integral number of double
words, so that all memory blocks dispensed with be properly aligned.
*/
        size += 2*sizeof(long) + DEBUGWORDS*sizeof(long);
        if (size < sizeof(struct freeblock) + sizeof(long))
               size = sizeof(struct freeblock) + sizeof(long);
        size = ((unsigned) (size + sizeof(double) - 1) / sizeof(double)) * (sizeof(double)/sizeof(long));
 
/*
For speed, we will try first to give the user back a very recently
returned block--one that is on the front of the chain before
'firstcombined'.  These blocks still have negative sizes, and need
only to be "unhook"ed:
*/
        size = -size;
        for (p=firstfree.fore; p != firstcombined; p=p->fore) {
                if (p->size == size) {
                        unhook(p);
                        uncombined--;
                        if (mallocdebug) {
                               printf("fast xiMalloc(%ld) = %p, ", size, p);
                               dumpchain();
                        }
                        AvailableWords += size;  /* decreases AvailableWords */
                        whocalledme(p, &Size);
                        return((char *)&p->fore);
                }
        }
/*
Well, if we get here, there are no uncombined blocks matching the user's
request.  So, we search the rest of the chain for a block that is big
enough.  ('size' becomes positive again):
*/
        size = -size;
        for (;; p = p->fore) {
/*
If we hit the end of the chain (p->size == 0), we are probably out of
memory.  However, we should first try to combine any memory that has
not yet been combined before we give that pessimistic answer.  If
we succeed in combining, we can call ourselves recursively to try to
allocate the requested amount:
*/
               if (p->size == 0) {
                       if (uncombined <= 0)
                              return(NULL);
                       while (firstfree.fore != firstcombined)
                              combine();
                       return(xiMalloc(sizeof(long) * (size - 2 - DEBUGWORDS)));
               }
/*
Otherwise, we keep searching until we find a big enough block:
*/
               if (p->size >= size)
                       break;
        }
/*
At this point, 'p' contains a block at least as big as what the user
requested, so we take it off the free chain.  If it is excessively big,
we return the excess to the free chain:
*/
        unhook(p);
        excess = p->size - size;
        area = (long *) p;
 
        if (excess > MINEXCESS)
                freeuncombinable(area + size, excess);
        else
                size = p->size;
 
        AvailableWords -= size;
/*
Mark first and last word of block with the negative of the size, to
flag that this block is allocated:
*/
        area[size - 1] = area[0] = - size;
 
        if (mallocdebug) {
                printf("slow xiMalloc(%ld) @ %p, ", size, area);
                dumpchain();
        }
        whocalledme(area, &Size);
/*
The address we return to the user is one 'long' BELOW the address of
the block.  This protects our 'size' field, so we can tell the size
of the block when he returns it to us with xiFree().  Also, he better not
touch the 'size' field at the end of the block either.  (That would be
nasty of him, as he would be touching memory outside of the bytes he
requested).
*/
        return((char *) (area + 1));
}
 
/*
:h2 id=addmem.addmemory() - Initialize Free Memory
 
This routine should be called at initialization to initialize the
free chain.  There is no standard way to do this in C.
We want the memory dispensed by malloc to be aligned on a double word
boundary (because some machines either require alignment, or are
more efficient if accesses are aligned).  Since the total size of
any block created by malloc is an integral number of double words,
all we have to do to ensure alignment is to adjust each large block
added to the free chain to start on an odd long-word boundary.
(Malloc's size field will occupy the odd long and the user's memory
will then begin on an even boundary.)  Since we fill in additional
size fields at the beginning and end of each of the large freeblocks,
we need only adjust the address passed to addmemory to a double word
boundary.
*/
 
#define   MAXAREAS   10      /* there can be this many calls to addmemory()  */
 
static long *freearea[MAXAREAS] = { NULL };  /* so we can report later       */
 
void 
addmemory(long *addr,        /* beginning of free area                       */
	  long size)         /* number of bytes of free area                 */
{
        register int i;      /* loop index variable                          */
        register long *aaddr;  /* aligned beginning of free area             */
 
#if DEBUGWORDS
        printf("malloc has DEBUGWORDS=%d\n", DEBUGWORDS);
#endif
/*
First link together firstfree and lastfree if necessary:
*/
        if (firstfree.fore == NULL) {
                firstfree.fore = &lastfree;
                lastfree.back = &firstfree;
        }
/*
We'll record where the area was that was given to us for later reports:
*/
        for (i=0; i < MAXAREAS; i++)
                if (freearea[i] == NULL) break;
        if (i >= MAXAREAS)
                Abort("too many addmemory()s");
        aaddr = (long *) ( ((long) addr + sizeof(double) - 1) & - (long)sizeof(double) );
        size -= (char *) aaddr - (char *) addr;
        freearea[i] = aaddr;
/*
Convert 'size' to number of longs, and store '-size' guards at the
beginning and end of this area so we will not accidentally recombine the
first or last block:
*/
        size /= sizeof(long);
 
        AvailableWords += size - 2;
 
        aaddr[size - 1] = aaddr[0] = -size;
/*
Finally, call 'freeuncombinable' to put the remaining memory on the
free list:
*/
        freeuncombinable(aaddr + 1, size - 2);
}
 
/*
:h3.delmemory() - Delete Memory Pool
*/
void 
delmemory(void)
{
       register int i;
 
       AvailableWords = 0;
       firstfree.fore = &lastfree;
       lastfree.back  = &firstfree;
       firstcombined  = &lastfree;
       uncombined     = 0;
       for (i=0; i<MAXAREAS; i++)
               freearea[i] = NULL;
}
 
/*
:h2.Debug Routines
 
:h3.dumpchain() - Print the Chain of Free Blocks
*/
 
static void
dumpchain(void)
{
        register struct freeblock *p;  /* current free block                 */
        register long size;  /* size of block                                */
        register struct freeblock *back;  /* block before 'p'                */
        register int i;      /* temp variable for counting                   */
 
        printf("DUMPING FAST FREE LIST:\n");
        back = &firstfree;
        for (p = firstfree.fore, i=uncombined; p != firstcombined;
                                 p = p->fore) {
                if (--i < 0)
                        Abort("too many uncombined areas");
                size = p->size;
                printf(". . . area @ %p, size = %ld\n", p, -size);
                if (size >= 0 || size != ((int *) p)[-1 - size])
                        Abort("dumpchain: bad size");
                if (p->back != back)
                        Abort("dumpchain: bad back");
                back = p;
        }
        printf("DUMPING COMBINED FREE LIST:\n");
        for (; p != &lastfree; p = p->fore)  {
                size = p->size;
                printf(". . . area @ %p, size = %ld\n", p, size);
                if (size <= 0 || size != ((int *) p)[size - 1])
                        Abort("dumpchain: bad size");
                if (p->back != back)
                        Abort("dumpchain: bad back");
                back = p;
        }
        if (back != lastfree.back)
                Abort("dumpchain: bad lastfree");
}
 
#ifdef notused
/*
:h3.reportarea() - Display a Contiguous Set of Memory Blocks
*/
 
static void
reportarea(long *area)        /* start of blocks (from addmemory)            */
{
       register long size;    /* size of current block                       */
       register long wholesize;  /* size of original area                    */
       register struct freeblock *p;  /* pointer to block                    */
 
       if (area == NULL)
               return;
       wholesize = - *area++;
       wholesize -= 2;
 
       while (wholesize > 0) {
               size = *area;
               if (size < 0) {
                       register int i,j;
 
                       size = -size;
                       printf("Allocated %5ld bytes at %p, first words=%08lx %08lx\n",
                               size * sizeof(long), area + 1, area[1], area[2]);
#if DEBUGWORDS
                       printf("  ...Last operator: %s\n",
                               (char *)area[size-DEBUGWORDS-1]);
#endif
                       for (i = size - DEBUGWORDS; i < size - 2; i += 8) {
                               printf("  ...");
                               for (j=0; j<8; j++)
                                       printf(" %08lx", area[i+j]);
                               printf("\n");
                       }
 
               }
               else {
                       printf("Free %ld bytes at %p\n", size * sizeof(long),
                               area);
                       if (size == 0)
                               Abort("zero sized memory block");
 
                       for (p = firstfree.fore; p != NULL; p = p->fore)
                               if ((long *) p == area) break;
                       if ((long *) p != area)
                               Abort("not found on forward chain");
 
                       for (p = lastfree.back; p != NULL; p = p->back)
                               if ((long *) p == area) break;
                       if ((long *) p != area)
                               Abort("not found on backward chain");
               }
               if (area[0] != area[size - 1])
                       Abort("unmatched check sizes");
               area += size;
               wholesize -= size;
       }
}
 
/*
:h3.MemReport() - Display All of Memory
*/
 
void 
MemReport(void)
{
       register int i;
 
       dumpchain();
 
       for (i=0; i<MAXAREAS; i++)
               reportarea(freearea[i]);
}
 
/*
:h3.MemBytesAvail - Display Number of Bytes Now Available
*/
 
void 
MemBytesAvail(void)
{
       printf("There are now %ld bytes available\n", AvailableWords *
                                                    sizeof(long) );
}
#endif
