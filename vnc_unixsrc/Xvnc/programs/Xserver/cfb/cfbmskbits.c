/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and X Consortium make no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/* $XConsortium: cfbmskbits.c,v 4.13 94/07/28 12:30:41 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/cfb/cfbmskbits.c,v 3.0 1996/06/29 09:05:41 dawes Exp $ */

/*
 * ==========================================================================
 * Converted to Color Frame Buffer by smarks@sun, April-May 1987.  The "bit 
 * numbering" in the doc below really means "byte numbering" now.
 * ==========================================================================
 */

/*
   these tables are used by several macros in the cfb code.

   the vax numbers everything left to right, so bit indices on the
screen match bit indices in longwords.  the pc-rt and Sun number
bits on the screen the way they would be written on paper,
(i.e. msb to the left), and so a bit index n on the screen is
bit index 32-n in a longword

   see also cfbmskbits.h
*/
#include	<X.h>
#include	<Xmd.h>
#include	<servermd.h>
#include	"cfb.h"
#include	"cfbmskbits.h"

#define _cfbBits(a) (PixelGroup)(a)

#if	(BITMAP_BIT_ORDER == MSBFirst)
#define cfbBits(v)	_cfbBits(v)
#else /* BITMAP_BIT_ORDER == LSBFirst */
#define cfbFlip2(a)	((((a) & 0x1) << 1) | (((a) & 0x2) >> 1))
#define cfbFlip4(a)	((cfbFlip2(a) << 2) | cfbFlip2(a >> 2))
#define cfbFlip8(a)	((cfbFlip4(a) << 4) | cfbFlip4(a >> 4))
#define cfbFlip16(a)	((cfbFlip8(a) << 8) | cfbFlip8(a >> 8))
#define cfbFlip32(a)	((cfbFlip16(a) << 16) | cfbFlip16(a >> 16))
#if PGSZ == 32
#define cfbBits(a)	cfbFlip32(_cfbBits(a))
#else /* PGSZ == 64 */
#define cfbFlip64(a)	((cfbFlip32(a) << 32) | cfbFlip32(a >> 32))
#define cfbBits(a)	cfbFlip64(_cfbBits(a))
#endif /* PGSZ */
#endif /* BITMAP_BIT_ORDER */

/* NOTE:
the first element in starttab could be 0xffffffff.  making it 0
lets us deal with a full first word in the middle loop, rather
than having to do the multiple reads and masks that we'd
have to do if we thought it was partial.
*/
#if PSZ == 4
#if PGSZ == 32
PixelGroup cfbstarttab[] =
    {
	cfbBits(0x00000000),
	cfbBits(0x0FFFFFFF),
	cfbBits(0x00FFFFFF),
	cfbBits(0x000FFFFF),
	cfbBits(0x0000FFFF),
	cfbBits(0x00000FFF),
	cfbBits(0x000000FF),
	cfbBits(0x0000000F)
    };
PixelGroup cfbendtab[] =
    {
	cfbBits(0x00000000),
	cfbBits(0xF0000000),
	cfbBits(0xFF000000),
	cfbBits(0xFFF00000),
	cfbBits(0xFFFF0000),
	cfbBits(0xFFFFF000),
	cfbBits(0xFFFFFF00),
	cfbBits(0xFFFFFFF0)
    };
#else /* PGSZ == 64 */
PixelGroup cfbstarttab[] =
    {
	cfbBits(0x0000000000000000),
	cfbBits(0x0FFFFFFFFFFFFFFF),
	cfbBits(0x00FFFFFFFFFFFFFF),
	cfbBits(0x000FFFFFFFFFFFFF),
	cfbBits(0x0000FFFFFFFFFFFF),
	cfbBits(0x00000FFFFFFFFFFF),
	cfbBits(0x000000FFFFFFFFFF),
	cfbBits(0x0000000FFFFFFFFF),
	cfbBits(0x00000000FFFFFFFF),
	cfbBits(0x000000000FFFFFFF),
	cfbBits(0x0000000000FFFFFF),
	cfbBits(0x00000000000FFFFF),
	cfbBits(0x000000000000FFFF),
	cfbBits(0x0000000000000FFF),
	cfbBits(0x00000000000000FF),
	cfbBits(0x000000000000000F),
    };
PixelGroup cfbendtab[] =
    {
	cfbBits(0x0000000000000000),
	cfbBits(0xF000000000000000),
	cfbBits(0xFF00000000000000),
	cfbBits(0xFFF0000000000000),
	cfbBits(0xFFFF000000000000),
	cfbBits(0xFFFFF00000000000),
	cfbBits(0xFFFFFF0000000000),
	cfbBits(0xFFFFFFF000000000),
	cfbBits(0xFFFFFFFF00000000),
	cfbBits(0xFFFFFFFFF0000000),
	cfbBits(0xFFFFFFFFFF000000),
	cfbBits(0xFFFFFFFFFFF00000),
	cfbBits(0xFFFFFFFFFFFF0000),
	cfbBits(0xFFFFFFFFFFFFF000),
	cfbBits(0xFFFFFFFFFFFFFF00),
	cfbBits(0xFFFFFFFFFFFFFFF0),
    };
#endif /* PGSZ */
#endif /* PSZ == 4 */

#if PSZ == 8
#if PGSZ == 32
PixelGroup cfbstarttab[] =
    {
	cfbBits(0x00000000),
	cfbBits(0x00FFFFFF),
	cfbBits(0x0000FFFF),
	cfbBits(0x000000FF)
    };
PixelGroup cfbendtab[] =
    {
	cfbBits(0x00000000),
	cfbBits(0xFF000000),
	cfbBits(0xFFFF0000),
	cfbBits(0xFFFFFF00)
    };
#else /* PGSZ == 64 */
PixelGroup cfbstarttab[] =
    {
	cfbBits(0x0000000000000000),
	cfbBits(0x00FFFFFFFFFFFFFF),
	cfbBits(0x0000FFFFFFFFFFFF),
	cfbBits(0x000000FFFFFFFFFF),
	cfbBits(0x00000000FFFFFFFF),
	cfbBits(0x0000000000FFFFFF),
	cfbBits(0x000000000000FFFF),
	cfbBits(0x00000000000000FF)
    };
PixelGroup cfbendtab[] =
    {
	cfbBits(0x0000000000000000),
	cfbBits(0xFF00000000000000),
	cfbBits(0xFFFF000000000000),
	cfbBits(0xFFFFFF0000000000),
	cfbBits(0xFFFFFFFF00000000),
	cfbBits(0xFFFFFFFFFF000000),
	cfbBits(0xFFFFFFFFFFFF0000),
	cfbBits(0xFFFFFFFFFFFFFF00)
    };
#endif /* PGSZ */
#endif /* PSZ == 8 */

#if PSZ == 16
#if PGSZ == 32
PixelGroup cfbstarttab[] =
    {
	cfbBits(0x00000000),
	cfbBits(0x0000FFFF),
    };
PixelGroup cfbendtab[] =
    {
	cfbBits(0x00000000),
	cfbBits(0xFFFF0000),
    };
#else /* PGSZ == 64 */
PixelGroup cfbstarttab[] =
    {
	cfbBits(0x0000000000000000),
	cfbBits(0x0000FFFFFFFFFFFF),
	cfbBits(0x00000000FFFFFFFF),
	cfbBits(0x000000000000FFFF),
    };
PixelGroup cfbendtab[] =
    {
	cfbBits(0x0000000000000000),
	cfbBits(0xFFFF000000000000),
	cfbBits(0xFFFFFFFF00000000),
	cfbBits(0xFFFFFFFFFFFF0000),
    };
#endif /* PGSZ */
#endif

#if PSZ == 24
#if PGSZ == 32
PixelGroup cfbstarttab[] =
    {
	cfbBits(0x00000000),
	cfbBits(0x000000FF),
	cfbBits(0x0000FFFF),
	cfbBits(0x00FFFFFF),
    };
PixelGroup cfbendtab[] = 
    {
	cfbBits(0x00000000),
	cfbBits(0xFFFFFF00),
	cfbBits(0xFFFF0000),
	cfbBits(0xFF000000),
    };
#else /* PGSZ == 64 */
PixelGroup cfbstarttab[] =
    {
	cfbBits(0x0000000000000000),
	cfbBits(0x000000FFFFFFFFFF),
	cfbBits(0x000000000000FFFF),
    };
PixelGroup cfbendtab[] = 
    {
	cfbBits(0x0000000000000000),
	cfbBits(0xFFFFFFFFFF000000),
	cfbBits(0xFFFF000000000000),
    };
#endif /* PGSZ */
#endif /* PSZ == 24 */

#if PSZ == 32
#if PGSZ == 32
PixelGroup cfbstarttab[] =
    {
	cfbBits(0x00000000),
    };
PixelGroup cfbendtab[] = 
    {
	cfbBits(0x00000000),
    };
#else /* PGSZ == 64 */
PixelGroup cfbstarttab[] =
    {
	cfbBits(0x0000000000000000),
	cfbBits(0x00000000FFFFFFFF),
    };
PixelGroup cfbendtab[] = 
    {
	cfbBits(0x0000000000000000),
	cfbBits(0xFFFFFFFF00000000),
    };
#endif /* PGSZ */
#endif /* PSZ == 32 */

/* a hack, for now, since the entries for 0 need to be all
   1 bits, not all zeros.
   this means the code DOES NOT WORK for segments of length
   0 (which is only a problem in the horizontal line code.)
*/
#if PSZ == 4
#if PGSZ == 32
PixelGroup cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0x0FFFFFFF),
	cfbBits(0x00FFFFFF),
	cfbBits(0x000FFFFF),
	cfbBits(0x0000FFFF),
	cfbBits(0x00000FFF),
	cfbBits(0x000000FF),
	cfbBits(0x0000000F)
    };

PixelGroup cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0xF0000000),
	cfbBits(0xFF000000),
	cfbBits(0xFFF00000),
	cfbBits(0xFFFF0000),
	cfbBits(0xFFFFF000),
	cfbBits(0xFFFFFF00),
	cfbBits(0xFFFFFFF0)
    };
#else /* PGSZ == 64 */
PixelGroup cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFFFFFFFFFF),
	cfbBits(0x0FFFFFFFFFFFFFFF),
	cfbBits(0x00FFFFFFFFFFFFFF),
	cfbBits(0x000FFFFFFFFFFFFF),
	cfbBits(0x0000FFFFFFFFFFFF),
	cfbBits(0x00000FFFFFFFFFFF),
	cfbBits(0x000000FFFFFFFFFF),
	cfbBits(0x0000000FFFFFFFFF),
	cfbBits(0x00000000FFFFFFFF),
	cfbBits(0x000000000FFFFFFF),
	cfbBits(0x0000000000FFFFFF),
	cfbBits(0x00000000000FFFFF),
	cfbBits(0x000000000000FFFF),
	cfbBits(0x0000000000000FFF),
	cfbBits(0x00000000000000FF),
	cfbBits(0x000000000000000F),
    };

PixelGroup cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFFFFFFFFFF),
	cfbBits(0xF000000000000000),
	cfbBits(0xFF00000000000000),
	cfbBits(0xFFF0000000000000),
	cfbBits(0xFFFF000000000000),
	cfbBits(0xFFFFF00000000000),
	cfbBits(0xFFFFFF0000000000),
	cfbBits(0xFFFFFFF000000000),
	cfbBits(0xFFFFFFFF00000000),
	cfbBits(0xFFFFFFFFF0000000),
	cfbBits(0xFFFFFFFFFF000000),
	cfbBits(0xFFFFFFFFFFF00000),
	cfbBits(0xFFFFFFFFFFFF0000),
	cfbBits(0xFFFFFFFFFFFFF000),
	cfbBits(0xFFFFFFFFFFFFFF00),
	cfbBits(0xFFFFFFFFFFFFFFF0),
    };
#endif /* PGSZ */
#endif /* PSZ == 4 */

#if PSZ == 8
#if PGSZ == 32
PixelGroup cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0x00FFFFFF),
	cfbBits(0x0000FFFF),
	cfbBits(0x000000FF)
    };

PixelGroup cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0xFF000000),
	cfbBits(0xFFFF0000),
	cfbBits(0xFFFFFF00)
    };
#else /* PGSZ == 64 */
PixelGroup cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFFFFFFFFFF),
	cfbBits(0x00FFFFFFFFFFFFFF),
	cfbBits(0x0000FFFFFFFFFFFF),
	cfbBits(0x000000FFFFFFFFFF),
	cfbBits(0x00000000FFFFFFFF),
	cfbBits(0x0000000000FFFFFF),
	cfbBits(0x000000000000FFFF),
	cfbBits(0x00000000000000FF),
    };

PixelGroup cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFFFFFFFFFF),
	cfbBits(0xFF00000000000000),
	cfbBits(0xFFFF000000000000),
	cfbBits(0xFFFFFF0000000000),
	cfbBits(0xFFFFFFFF00000000),
	cfbBits(0xFFFFFFFFFF000000),
	cfbBits(0xFFFFFFFFFFFF0000),
	cfbBits(0xFFFFFFFFFFFFFF00),
    };
#endif /* PGSZ */
#endif /* PSZ == 8 */

#if PSZ == 16
#if PGSZ == 32
PixelGroup cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0x0000FFFF),
    };

PixelGroup cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0xFFFF0000),
    };
#else /* PGSZ == 64 */
PixelGroup cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFFFFFFFFFF),
	cfbBits(0x0000FFFFFFFFFFFF),
	cfbBits(0x00000000FFFFFFFF),
	cfbBits(0x000000000000FFFF),
    };

PixelGroup cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFFFFFFFFFF),
	cfbBits(0xFFFF000000000000),
	cfbBits(0xFFFFFFFF00000000),
	cfbBits(0xFFFFFFFFFFFF0000),
    };
#endif /* PGSZ */
#endif /* PSZ == 16 */

#if PSZ == 24
#if PGSZ == 32
PixelGroup cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0x000000FF),
	cfbBits(0x0000FFFF),
	cfbBits(0x00FFFFFF),
    };

PixelGroup cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0xFFFFFF00),
	cfbBits(0xFFFF0000),
	cfbBits(0xFF000000),
    };
#else /* PGSZ == 64 */
PixelGroup cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFFFFFFFFFF),
	cfbBits(0x0000FFFFFFFFFFFF),
	cfbBits(0x000000FFFFFFFFFF),
	cfbBits(0x00000000FFFFFFFF),
	cfbBits(0x0000000000FFFFFF),
	cfbBits(0x000000000000FFFF),
	cfbBits(0x00000000000000FF),
    };

PixelGroup cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFFFFFFFFFF),
	cfbBits(0xFFFFFFFFFFFF0000),
	cfbBits(0xFFFFFFFFFF000000),
	cfbBits(0xFFFFFFFF00000000),
	cfbBits(0xFFFFFF0000000000),
	cfbBits(0xFFFF000000000000),
	cfbBits(0xFF00000000000000),
    };
#endif /* PGSZ */
#endif /* PSZ == 24 */

#if PSZ == 32
#if PGSZ == 32
PixelGroup cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFF),
    };

PixelGroup cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFF),
    };
#else /* PGSZ == 64 */
PixelGroup cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFFFFFFFFFF),
	cfbBits(0x00000000FFFFFFFF),
    };

PixelGroup cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFFFFFFFFFF),
	cfbBits(0xFFFFFFFF00000000),
    };
#endif /* PGSZ */
#endif /* PSZ == 32 */

/* used for masking bits in bresenham lines
   mask[n] is used to mask out all but bit n in a longword (n is a
screen position).
   rmask[n] is used to mask out the single bit at position n (n
is a screen posiotion.)
*/

#if PSZ == 4
#if PGSZ == 32
PixelGroup cfbmask[] =
    {
	cfbBits(0xF0000000),
	cfbBits(0x0F000000),
 	cfbBits(0x00F00000),
 	cfbBits(0x000F0000),
 	cfbBits(0x0000F000),
 	cfbBits(0x00000F00),
 	cfbBits(0x000000F0),
 	cfbBits(0x0000000F)
    }; 
PixelGroup cfbrmask[] = 
    {
	cfbBits(0x0FFFFFFF),
	cfbBits(0xF0FFFFFF),
 	cfbBits(0xFF0FFFFF),
 	cfbBits(0xFFF0FFFF),
 	cfbBits(0xFFFF0FFF),
 	cfbBits(0xFFFFF0FF),
 	cfbBits(0xFFFFFF0F),
 	cfbBits(0xFFFFFFF0)
    };
#else /* PGSZ == 64 */
PixelGroup cfbmask[] =
    {
	cfbBits(0xF000000000000000),
	cfbBits(0x0F00000000000000),
 	cfbBits(0x00F0000000000000),
 	cfbBits(0x000F000000000000),
 	cfbBits(0x0000F00000000000),
 	cfbBits(0x00000F0000000000),
 	cfbBits(0x000000F000000000),
 	cfbBits(0x0000000F00000000),
	cfbBits(0x00000000F0000000),
	cfbBits(0x000000000F000000),
 	cfbBits(0x0000000000F00000),
 	cfbBits(0x00000000000F0000),
 	cfbBits(0x000000000000F000),
 	cfbBits(0x0000000000000F00),
 	cfbBits(0x00000000000000F0),
 	cfbBits(0x000000000000000F),
    }; 
PixelGroup cfbrmask[] = 
    {
	cfbBits(0x0FFFFFFFFFFFFFFF),
	cfbBits(0xF0FFFFFFFFFFFFFF),
 	cfbBits(0xFF0FFFFFFFFFFFFF),
 	cfbBits(0xFFF0FFFFFFFFFFFF),
 	cfbBits(0xFFFF0FFFFFFFFFFF),
 	cfbBits(0xFFFFF0FFFFFFFFFF),
 	cfbBits(0xFFFFFF0FFFFFFFFF),
 	cfbBits(0xFFFFFFF0FFFFFFFF),
	cfbBits(0xFFFFFFFF0FFFFFFF),
	cfbBits(0xFFFFFFFFF0FFFFFF),
 	cfbBits(0xFFFFFFFFFF0FFFFF),
 	cfbBits(0xFFFFFFFFFFF0FFFF),
 	cfbBits(0xFFFFFFFFFFFF0FFF),
 	cfbBits(0xFFFFFFFFFFFFF0FF),
 	cfbBits(0xFFFFFFFFFFFFFF0F),
 	cfbBits(0xFFFFFFFFFFFFFFF0),
    };
#endif /* PGSZ */
#endif /* PSZ == 4 */

#if PSZ == 8
#if PGSZ == 32
PixelGroup cfbmask[] =
    {
	cfbBits(0xFF000000),
 	cfbBits(0x00FF0000),
 	cfbBits(0x0000FF00),
 	cfbBits(0x000000FF)
    }; 
PixelGroup cfbrmask[] = 
    {
	cfbBits(0x00FFFFFF),
 	cfbBits(0xFF00FFFF),
 	cfbBits(0xFFFF00FF),
 	cfbBits(0xFFFFFF00)
    };
#else /* PGSZ == 64 */
PixelGroup cfbmask[] =
    {
	cfbBits(0xFF00000000000000),
 	cfbBits(0x00FF000000000000),
 	cfbBits(0x0000FF0000000000),
 	cfbBits(0x000000FF00000000),
	cfbBits(0x00000000FF000000),
 	cfbBits(0x0000000000FF0000),
 	cfbBits(0x000000000000FF00),
 	cfbBits(0x00000000000000FF),
    }; 
PixelGroup cfbrmask[] = 
    {
	cfbBits(0x00FFFFFFFFFFFFFF),
 	cfbBits(0xFF00FFFFFFFFFFFF),
 	cfbBits(0xFFFF00FFFFFFFFFF),
 	cfbBits(0xFFFFFF00FFFFFFFF),
	cfbBits(0xFFFFFFFF00FFFFFF),
 	cfbBits(0xFFFFFFFFFF00FFFF),
 	cfbBits(0xFFFFFFFFFFFF00FF),
 	cfbBits(0xFFFFFFFFFFFFFF00),
    };
#endif /* PGSZ */
#endif /* PSZ == 8 */

#if PSZ == 16
#if PGSZ == 32
PixelGroup cfbmask[] =
    {
	cfbBits(0xFFFF0000),
 	cfbBits(0x0000FFFF),
    }; 
PixelGroup cfbrmask[] = 
    {
	cfbBits(0x0000FFFF),
 	cfbBits(0xFFFF0000),
    };
#else /* PGSZ == 64 */
PixelGroup cfbmask[] =
    {
	cfbBits(0xFFFF000000000000),
 	cfbBits(0x0000FFFF00000000),
	cfbBits(0x00000000FFFF0000),
 	cfbBits(0x000000000000FFFF),
    }; 
PixelGroup cfbrmask[] = 
    {
	cfbBits(0x0000FFFFFFFFFFFF),
 	cfbBits(0xFFFF0000FFFFFFFF),
	cfbBits(0xFFFFFFFF0000FFFF),
 	cfbBits(0xFFFFFFFFFFFF0000),
    };
#endif /* PGSZ */
#endif /* PSZ == 16 */

#if PSZ == 24
#if PGSZ == 32
PixelGroup cfbmask[] =
    {
 	cfbBits(0xFFFFFF00),
	cfbBits(0x00000000),
	cfbBits(0x000000FF),
 	cfbBits(0xFFFF0000),
	cfbBits(0x0000FFFF),
	cfbBits(0xFF000000),
	cfbBits(0x00FFFFFF),
	cfbBits(0x00000000),
    }; 
PixelGroup cfbrmask[] = 
    {
	cfbBits(0x000000FF),
 	cfbBits(0xFFFFFFFF),
 	cfbBits(0xFFFFFF00),
	cfbBits(0x0000FFFF),
	cfbBits(0xFFFF0000),
	cfbBits(0x00FFFFFF),
	cfbBits(0xFF000000),
 	cfbBits(0xFFFFFFFF),
    };
#else /* PGSZ == 64 */
PixelGroup cfbmask[] =
    {
	cfbBits(0xFFFFFF0000000000),
 	cfbBits(0x000000FFFFFF0000),
 	cfbBits(0x000000000000FFFF),
    }; 
PixelGroup cfbmask2[] =
    {
 	cfbBits(0x0000000000000000),
 	cfbBits(0x0000000000000000),
	cfbBits(0xFF00000000000000),
    }; 
PixelGroup cfbrmask[] = 
    {
 	cfbBits(0x000000FFFFFFFFFF),
 	cfbBits(0xFFFFFF000000FFFF),
 	cfbBits(0xFFFFFFFFFFFF0000),
    };
PixelGroup cfbrmask2[] = 
    {
 	cfbBits(0x0000000000000000),
 	cfbBits(0x0000000000000000),
 	cfbBits(0x00FFFFFFFFFFFFFF),
    };
#endif /* PGSZ */
#endif /* PSZ == 24 */

#if PSZ == 32
#if PGSZ == 32
PixelGroup cfbmask[] =
    {
	cfbBits(0xFFFFFFFF),
    }; 
PixelGroup cfbrmask[] = 
    {
	cfbBits(0xFFFFFFFF),
    };
#else /* PGSZ == 64 */
PixelGroup cfbmask[] =
    {
	cfbBits(0xFFFFFFFF00000000),
	cfbBits(0x00000000FFFFFFFF),
    }; 
PixelGroup cfbrmask[] = 
    {
	cfbBits(0x00000000FFFFFFFF),
	cfbBits(0xFFFFFFFF00000000),
    };
#endif /* PGSZ */
#endif /* PSZ == 32 */

/*
 * QuartetBitsTable contains PPW+1 masks whose binary values are masks in the
 * low order quartet that contain the number of bits specified in the
 * index.  This table is used by getstipplepixels.
 */
#if PSZ == 4
PixelGroup QuartetBitsTable[] = {
#if PGSZ == 32
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 00000000 */
    0x00000080,				/* 1 - 10000000 */
    0x000000C0,                         /* 2 - 11000000 */
    0x000000E0,                         /* 3 - 11100000 */
    0x000000F0,                         /* 4 - 11110000 */
    0x000000F8,                         /* 5 - 11111000 */
    0x000000FC,                         /* 6 - 11111100 */
    0x000000FE,                         /* 7 - 11111110 */
    0x000000FF                          /* 8 - 11111111 */
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 00000000 */
    0x00000001,                         /* 1 - 00000001 */
    0x00000003,                         /* 2 - 00000011 */
    0x00000007,                         /* 3 - 00000111 */
    0x0000000F,                         /* 4 - 00001111 */
    0x0000001F,                         /* 5 - 00011111 */
    0x0000003F,                         /* 6 - 00111111 */
    0x0000007F,                         /* 7 - 01111111 */
    0x000000FF                          /* 8 - 11111111 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
#else /* PGSZ == 64 */
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 0000000000000000 */
    0x00008000,				/* 1 - 1000000000000000 */
    0x0000C000,                         /* 2 - 1100000000000000 */
    0x0000E000,                         /* 3 - 1110000000000000 */
    0x0000F000,                         /* 4 - 1111000000000000 */
    0x0000F800,                         /* 5 - 1111100000000000 */
    0x0000FC00,                         /* 6 - 1111110000000000 */
    0x0000FE00,                         /* 7 - 1111111000000000 */
    0x0000FF00,                         /* 8 - 1111111100000000 */
    0x0000FF80,				/* 9 - 1111111110000000 */
    0x0000FFC0,                         /* 10- 1111111111000000 */
    0x0000FFE0,                         /* 11- 1111111111100000 */
    0x0000FFF0,                         /* 12- 1111111111110000 */
    0x0000FFF8,                         /* 13- 1111111111111000 */
    0x0000FFFC,                         /* 14- 1111111111111100 */
    0x0000FFFE,                         /* 15- 1111111111111110 */
    0x0000FFFF,                         /* 16- 1111111111111111 */
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 0000000000000000 */
    0x00000001,                         /* 1 - 0000000000000001 */
    0x00000003,                         /* 2 - 0000000000000011 */
    0x00000007,                         /* 3 - 0000000000000111 */
    0x0000000F,                         /* 4 - 0000000000001111 */
    0x0000001F,                         /* 5 - 0000000000011111 */
    0x0000003F,                         /* 6 - 0000000000111111 */
    0x0000007F,                         /* 7 - 0000000001111111 */
    0x000000FF,                         /* 8 - 0000000011111111 */
    0x000001FF,                         /* 9 - 0000000111111111 */
    0x000003FF,                         /* 10- 0000001111111111 */
    0x000007FF,                         /* 11- 0000011111111111 */
    0x00000FFF,                         /* 12- 0000111111111111 */
    0x00001FFF,                         /* 13- 0001111111111111 */
    0x00003FFF,                         /* 14- 0011111111111111 */
    0x00007FFF,                         /* 15- 0111111111111111 */
    0x0000FFFF,                         /* 16- 1111111111111111 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
#endif /* PGSZ */
};
#endif /* PSZ == 4 */

#if PSZ == 8
PixelGroup QuartetBitsTable[] = {
#if PGSZ == 32
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 0000 */
    0x00000008,                         /* 1 - 1000 */
    0x0000000C,                         /* 2 - 1100 */
    0x0000000E,                         /* 3 - 1110 */
    0x0000000F                          /* 4 - 1111 */
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 0000 */
    0x00000001,                         /* 1 - 0001 */
    0x00000003,                         /* 2 - 0011 */
    0x00000007,                         /* 3 - 0111 */
    0x0000000F                          /* 4 - 1111 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
#else /* PGSZ == 64 */
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 00000000 */
    0x00000080,                         /* 1 - 10000000 */
    0x000000C0,                         /* 2 - 11000000 */
    0x000000E0,                         /* 3 - 11100000 */
    0x000000F0,                         /* 4 - 11110000 */
    0x000000F8,                         /* 5 - 11111000 */
    0x000000FC,                         /* 6 - 11111100 */
    0x000000FE,                         /* 7 - 11111110 */
    0x000000FF                          /* 8 - 11111111 */
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 00000000 */
    0x00000001,                         /* 1 - 00000001 */
    0x00000003,                         /* 2 - 00000011 */
    0x00000007,                         /* 3 - 00000111 */
    0x0000000F,                         /* 4 - 10000111 */
    0x0000001F,                         /* 5 - 00011111 */
    0x0000003F,                         /* 6 - 00111111 */
    0x0000007F,                         /* 7 - 01111111 */
    0x000000FF                          /* 8 - 11111111 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
#endif /* PGSZ */
};
#endif /* PSZ == 8 */

#if PSZ == 16
PixelGroup QuartetBitsTable[] = {
#if PGSZ == 32
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 00 */
    0x00000002,                         /* 1 - 10 */
    0x00000003,                         /* 2 - 11 */
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 00 */
    0x00000001,                         /* 1 - 01 */
    0x00000003,                         /* 2 - 11 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
#else /* PGSZ == 64 */
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 0000 */
    0x00000008,                         /* 1 - 1000 */
    0x0000000C,                         /* 2 - 1100 */
    0x0000000E,                         /* 3 - 1110 */
    0x0000000F,                         /* 4 - 1111 */
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 0000 */
    0x00000001,                         /* 1 - 0001 */
    0x00000003,                         /* 2 - 0011 */
    0x00000007,                         /* 3 - 0111 */
    0x0000000F,                         /* 4 - 1111 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
#endif /* PGSZ */
};
#endif /* PSZ == 16 */

#if PSZ == 24
PixelGroup QuartetBitsTable[] = {
#if PGSZ == 32
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 0 */
    0x00000001,                         /* 1 - 1 */
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 0 */
    0x00000001,                         /* 1 - 1 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
#else /* PGSZ == 64 */
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 00 */
    0x00000002,                         /* 1 - 10 */
    0x00000003,                         /* 2 - 11*/
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 00 */
    0x00000001,                         /* 1 - 01 */
    0x00000003,                         /* 2 - 11 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
#endif /* PGSZ */
};
#endif /* PSZ == 24 */

#if PSZ == 32
PixelGroup QuartetBitsTable[] = {
#if PGSZ == 32
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 0 */
    0x00000001,                         /* 1 - 1 */
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 0 */
    0x00000001,                         /* 1 - 1 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
#else /* PGSZ == 64 */
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 00 */
    0x00000002,                         /* 1 - 10 */
    0x00000003,                         /* 2 - 11*/
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 00 */
    0x00000001,                         /* 1 - 01 */
    0x00000003,                         /* 2 - 11 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
#endif /* PGSZ */
};
#endif /* PSZ == 32 */

/*
 * QuartetPixelMaskTable is used by getstipplepixels to get a pixel mask
 * corresponding to a quartet of bits.  Note: the bit/byte order dependency
 * is handled by QuartetBitsTable above.
 */
#if PSZ == 4
#if PGSZ == 32
PixelGroup QuartetPixelMaskTable[] = {
    0x00000000,
    0x0000000F,
    0x000000F0,
    0x000000FF,
    0x00000F00,
    0x00000F0F,
    0x00000FF0,
    0x00000FFF,
    0x0000F000,
    0x0000F00F,
    0x0000F0F0,
    0x0000F0FF,
    0x0000FF00,
    0x0000FF0F,
    0x0000FFF0,
    0x0000FFFF,
    0x000F0000,
    0x000F000F,
    0x000F00F0,
    0x000F00FF,
    0x000F0F00,
    0x000F0F0F,
    0x000F0FF0,
    0x000F0FFF,
    0x000FF000,
    0x000FF00F,
    0x000FF0F0,
    0x000FF0FF,
    0x000FFF00,
    0x000FFF0F,
    0x000FFFF0,
    0x000FFFFF,
    0x00F00000,
    0x00F0000F,
    0x00F000F0,
    0x00F000FF,
    0x00F00F00,
    0x00F00F0F,
    0x00F00FF0,
    0x00F00FFF,
    0x00F0F000,
    0x00F0F00F,
    0x00F0F0F0,
    0x00F0F0FF,
    0x00F0FF00,
    0x00F0FF0F,
    0x00F0FFF0,
    0x00F0FFFF,
    0x00FF0000,
    0x00FF000F,
    0x00FF00F0,
    0x00FF00FF,
    0x00FF0F00,
    0x00FF0F0F,
    0x00FF0FF0,
    0x00FF0FFF,
    0x00FFF000,
    0x00FFF00F,
    0x00FFF0F0,
    0x00FFF0FF,
    0x00FFFF00,
    0x00FFFF0F,
    0x00FFFFF0,
    0x00FFFFFF,
    0x0F000000,
    0x0F00000F,
    0x0F0000F0,
    0x0F0000FF,
    0x0F000F00,
    0x0F000F0F,
    0x0F000FF0,
    0x0F000FFF,
    0x0F00F000,
    0x0F00F00F,
    0x0F00F0F0,
    0x0F00F0FF,
    0x0F00FF00,
    0x0F00FF0F,
    0x0F00FFF0,
    0x0F00FFFF,
    0x0F0F0000,
    0x0F0F000F,
    0x0F0F00F0,
    0x0F0F00FF,
    0x0F0F0F00,
    0x0F0F0F0F,
    0x0F0F0FF0,
    0x0F0F0FFF,
    0x0F0FF000,
    0x0F0FF00F,
    0x0F0FF0F0,
    0x0F0FF0FF,
    0x0F0FFF00,
    0x0F0FFF0F,
    0x0F0FFFF0,
    0x0F0FFFFF,
    0x0FF00000,
    0x0FF0000F,
    0x0FF000F0,
    0x0FF000FF,
    0x0FF00F00,
    0x0FF00F0F,
    0x0FF00FF0,
    0x0FF00FFF,
    0x0FF0F000,
    0x0FF0F00F,
    0x0FF0F0F0,
    0x0FF0F0FF,
    0x0FF0FF00,
    0x0FF0FF0F,
    0x0FF0FFF0,
    0x0FF0FFFF,
    0x0FFF0000,
    0x0FFF000F,
    0x0FFF00F0,
    0x0FFF00FF,
    0x0FFF0F00,
    0x0FFF0F0F,
    0x0FFF0FF0,
    0x0FFF0FFF,
    0x0FFFF000,
    0x0FFFF00F,
    0x0FFFF0F0,
    0x0FFFF0FF,
    0x0FFFFF00,
    0x0FFFFF0F,
    0x0FFFFFF0,
    0x0FFFFFFF,
    0xF0000000,
    0xF000000F,
    0xF00000F0,
    0xF00000FF,
    0xF0000F00,
    0xF0000F0F,
    0xF0000FF0,
    0xF0000FFF,
    0xF000F000,
    0xF000F00F,
    0xF000F0F0,
    0xF000F0FF,
    0xF000FF00,
    0xF000FF0F,
    0xF000FFF0,
    0xF000FFFF,
    0xF00F0000,
    0xF00F000F,
    0xF00F00F0,
    0xF00F00FF,
    0xF00F0F00,
    0xF00F0F0F,
    0xF00F0FF0,
    0xF00F0FFF,
    0xF00FF000,
    0xF00FF00F,
    0xF00FF0F0,
    0xF00FF0FF,
    0xF00FFF00,
    0xF00FFF0F,
    0xF00FFFF0,
    0xF00FFFFF,
    0xF0F00000,
    0xF0F0000F,
    0xF0F000F0,
    0xF0F000FF,
    0xF0F00F00,
    0xF0F00F0F,
    0xF0F00FF0,
    0xF0F00FFF,
    0xF0F0F000,
    0xF0F0F00F,
    0xF0F0F0F0,
    0xF0F0F0FF,
    0xF0F0FF00,
    0xF0F0FF0F,
    0xF0F0FFF0,
    0xF0F0FFFF,
    0xF0FF0000,
    0xF0FF000F,
    0xF0FF00F0,
    0xF0FF00FF,
    0xF0FF0F00,
    0xF0FF0F0F,
    0xF0FF0FF0,
    0xF0FF0FFF,
    0xF0FFF000,
    0xF0FFF00F,
    0xF0FFF0F0,
    0xF0FFF0FF,
    0xF0FFFF00,
    0xF0FFFF0F,
    0xF0FFFFF0,
    0xF0FFFFFF,
    0xFF000000,
    0xFF00000F,
    0xFF0000F0,
    0xFF0000FF,
    0xFF000F00,
    0xFF000F0F,
    0xFF000FF0,
    0xFF000FFF,
    0xFF00F000,
    0xFF00F00F,
    0xFF00F0F0,
    0xFF00F0FF,
    0xFF00FF00,
    0xFF00FF0F,
    0xFF00FFF0,
    0xFF00FFFF,
    0xFF0F0000,
    0xFF0F000F,
    0xFF0F00F0,
    0xFF0F00FF,
    0xFF0F0F00,
    0xFF0F0F0F,
    0xFF0F0FF0,
    0xFF0F0FFF,
    0xFF0FF000,
    0xFF0FF00F,
    0xFF0FF0F0,
    0xFF0FF0FF,
    0xFF0FFF00,
    0xFF0FFF0F,
    0xFF0FFFF0,
    0xFF0FFFFF,
    0xFFF00000,
    0xFFF0000F,
    0xFFF000F0,
    0xFFF000FF,
    0xFFF00F00,
    0xFFF00F0F,
    0xFFF00FF0,
    0xFFF00FFF,
    0xFFF0F000,
    0xFFF0F00F,
    0xFFF0F0F0,
    0xFFF0F0FF,
    0xFFF0FF00,
    0xFFF0FF0F,
    0xFFF0FFF0,
    0xFFF0FFFF,
    0xFFFF0000,
    0xFFFF000F,
    0xFFFF00F0,
    0xFFFF00FF,
    0xFFFF0F00,
    0xFFFF0F0F,
    0xFFFF0FF0,
    0xFFFF0FFF,
    0xFFFFF000,
    0xFFFFF00F,
    0xFFFFF0F0,
    0xFFFFF0FF,
    0xFFFFFF00,
    0xFFFFFF0F,
    0xFFFFFFF0,
    0xFFFFFFFF,
};
#else /* PGSZ == 64 */
No QuartetPixelMaskTable for psz=PSZ
this would be a 64K entry table, a bit much I think.
Try breaking things in two:
mask = table[index&0xff00]<<32 | table[index&0xff]
#endif /* PGSZ */
#endif /* PSZ == 4 */

#if PSZ == 8
PixelGroup QuartetPixelMaskTable[] = {
#if PGSZ == 32
    0x00000000,
    0x000000FF,
    0x0000FF00,
    0x0000FFFF,
    0x00FF0000,
    0x00FF00FF,
    0x00FFFF00,
    0x00FFFFFF,
    0xFF000000,
    0xFF0000FF,
    0xFF00FF00,
    0xFF00FFFF,
    0xFFFF0000,
    0xFFFF00FF,
    0xFFFFFF00,
    0xFFFFFFFF
#else /* PGSZ == 64 */
    0x0000000000000000,    0x00000000000000FF,
    0x000000000000FF00,    0x000000000000FFFF,
    0x0000000000FF0000,    0x0000000000FF00FF,
    0x0000000000FFFF00,    0x0000000000FFFFFF,
    0x00000000FF000000,    0x00000000FF0000FF,
    0x00000000FF00FF00,    0x00000000FF00FFFF,
    0x00000000FFFF0000,    0x00000000FFFF00FF,
    0x00000000FFFFFF00,    0x00000000FFFFFFFF,
    0x000000FF00000000,    0x000000FF000000FF,
    0x000000FF0000FF00,    0x000000FF0000FFFF,
    0x000000FF00FF0000,    0x000000FF00FF00FF,
    0x000000FF00FFFF00,    0x000000FF00FFFFFF,
    0x000000FFFF000000,    0x000000FFFF0000FF,
    0x000000FFFF00FF00,    0x000000FFFF00FFFF,
    0x000000FFFFFF0000,    0x000000FFFFFF00FF,
    0x000000FFFFFFFF00,    0x000000FFFFFFFFFF,
    0x0000FF0000000000,    0x0000FF00000000FF,
    0x0000FF000000FF00,    0x0000FF000000FFFF,
    0x0000FF0000FF0000,    0x0000FF0000FF00FF,
    0x0000FF0000FFFF00,    0x0000FF0000FFFFFF,
    0x0000FF00FF000000,    0x0000FF00FF0000FF,
    0x0000FF00FF00FF00,    0x0000FF00FF00FFFF,
    0x0000FF00FFFF0000,    0x0000FF00FFFF00FF,
    0x0000FF00FFFFFF00,    0x0000FF00FFFFFFFF,
    0x0000FFFF00000000,    0x0000FFFF000000FF,
    0x0000FFFF0000FF00,    0x0000FFFF0000FFFF,
    0x0000FFFF00FF0000,    0x0000FFFF00FF00FF,
    0x0000FFFF00FFFF00,    0x0000FFFF00FFFFFF,
    0x0000FFFFFF000000,    0x0000FFFFFF0000FF,
    0x0000FFFFFF00FF00,    0x0000FFFFFF00FFFF,
    0x0000FFFFFFFF0000,    0x0000FFFFFFFF00FF,
    0x0000FFFFFFFFFF00,    0x0000FFFFFFFFFFFF,
    0x00FF000000000000,    0x00FF0000000000FF,
    0x00FF00000000FF00,    0x00FF00000000FFFF,
    0x00FF000000FF0000,    0x00FF000000FF00FF,
    0x00FF000000FFFF00,    0x00FF000000FFFFFF,
    0x00FF0000FF000000,    0x00FF0000FF0000FF,
    0x00FF0000FF00FF00,    0x00FF0000FF00FFFF,
    0x00FF0000FFFF0000,    0x00FF0000FFFF00FF,
    0x00FF0000FFFFFF00,    0x00FF0000FFFFFFFF,
    0x00FF00FF00000000,    0x00FF00FF000000FF,
    0x00FF00FF0000FF00,    0x00FF00FF0000FFFF,
    0x00FF00FF00FF0000,    0x00FF00FF00FF00FF,
    0x00FF00FF00FFFF00,    0x00FF00FF00FFFFFF,
    0x00FF00FFFF000000,    0x00FF00FFFF0000FF,
    0x00FF00FFFF00FF00,    0x00FF00FFFF00FFFF,
    0x00FF00FFFFFF0000,    0x00FF00FFFFFF00FF,
    0x00FF00FFFFFFFF00,    0x00FF00FFFFFFFFFF,
    0x00FFFF0000000000,    0x00FFFF00000000FF,
    0x00FFFF000000FF00,    0x00FFFF000000FFFF,
    0x00FFFF0000FF0000,    0x00FFFF0000FF00FF,
    0x00FFFF0000FFFF00,    0x00FFFF0000FFFFFF,
    0x00FFFF00FF000000,    0x00FFFF00FF0000FF,
    0x00FFFF00FF00FF00,    0x00FFFF00FF00FFFF,
    0x00FFFF00FFFF0000,    0x00FFFF00FFFF00FF,
    0x00FFFF00FFFFFF00,    0x00FFFF00FFFFFFFF,
    0x00FFFFFF00000000,    0x00FFFFFF000000FF,
    0x00FFFFFF0000FF00,    0x00FFFFFF0000FFFF,
    0x00FFFFFF00FF0000,    0x00FFFFFF00FF00FF,
    0x00FFFFFF00FFFF00,    0x00FFFFFF00FFFFFF,
    0x00FFFFFFFF000000,    0x00FFFFFFFF0000FF,
    0x00FFFFFFFF00FF00,    0x00FFFFFFFF00FFFF,
    0x00FFFFFFFFFF0000,    0x00FFFFFFFFFF00FF,
    0x00FFFFFFFFFFFF00,    0x00FFFFFFFFFFFFFF,
    0xFF00000000000000,    0xFF000000000000FF,
    0xFF0000000000FF00,    0xFF0000000000FFFF,
    0xFF00000000FF0000,    0xFF00000000FF00FF,
    0xFF00000000FFFF00,    0xFF00000000FFFFFF,
    0xFF000000FF000000,    0xFF000000FF0000FF,
    0xFF000000FF00FF00,    0xFF000000FF00FFFF,
    0xFF000000FFFF0000,    0xFF000000FFFF00FF,
    0xFF000000FFFFFF00,    0xFF000000FFFFFFFF,
    0xFF0000FF00000000,    0xFF0000FF000000FF,
    0xFF0000FF0000FF00,    0xFF0000FF0000FFFF,
    0xFF0000FF00FF0000,    0xFF0000FF00FF00FF,
    0xFF0000FF00FFFF00,    0xFF0000FF00FFFFFF,
    0xFF0000FFFF000000,    0xFF0000FFFF0000FF,
    0xFF0000FFFF00FF00,    0xFF0000FFFF00FFFF,
    0xFF0000FFFFFF0000,    0xFF0000FFFFFF00FF,
    0xFF0000FFFFFFFF00,    0xFF0000FFFFFFFFFF,
    0xFF00FF0000000000,    0xFF00FF00000000FF,
    0xFF00FF000000FF00,    0xFF00FF000000FFFF,
    0xFF00FF0000FF0000,    0xFF00FF0000FF00FF,
    0xFF00FF0000FFFF00,    0xFF00FF0000FFFFFF,
    0xFF00FF00FF000000,    0xFF00FF00FF0000FF,
    0xFF00FF00FF00FF00,    0xFF00FF00FF00FFFF,
    0xFF00FF00FFFF0000,    0xFF00FF00FFFF00FF,
    0xFF00FF00FFFFFF00,    0xFF00FF00FFFFFFFF,
    0xFF00FFFF00000000,    0xFF00FFFF000000FF,
    0xFF00FFFF0000FF00,    0xFF00FFFF0000FFFF,
    0xFF00FFFF00FF0000,    0xFF00FFFF00FF00FF,
    0xFF00FFFF00FFFF00,    0xFF00FFFF00FFFFFF,
    0xFF00FFFFFF000000,    0xFF00FFFFFF0000FF,
    0xFF00FFFFFF00FF00,    0xFF00FFFFFF00FFFF,
    0xFF00FFFFFFFF0000,    0xFF00FFFFFFFF00FF,
    0xFF00FFFFFFFFFF00,    0xFF00FFFFFFFFFFFF,
    0xFFFF000000000000,    0xFFFF0000000000FF,
    0xFFFF00000000FF00,    0xFFFF00000000FFFF,
    0xFFFF000000FF0000,    0xFFFF000000FF00FF,
    0xFFFF000000FFFF00,    0xFFFF000000FFFFFF,
    0xFFFF0000FF000000,    0xFFFF0000FF0000FF,
    0xFFFF0000FF00FF00,    0xFFFF0000FF00FFFF,
    0xFFFF0000FFFF0000,    0xFFFF0000FFFF00FF,
    0xFFFF0000FFFFFF00,    0xFFFF0000FFFFFFFF,
    0xFFFF00FF00000000,    0xFFFF00FF000000FF,
    0xFFFF00FF0000FF00,    0xFFFF00FF0000FFFF,
    0xFFFF00FF00FF0000,    0xFFFF00FF00FF00FF,
    0xFFFF00FF00FFFF00,    0xFFFF00FF00FFFFFF,
    0xFFFF00FFFF000000,    0xFFFF00FFFF0000FF,
    0xFFFF00FFFF00FF00,    0xFFFF00FFFF00FFFF,
    0xFFFF00FFFFFF0000,    0xFFFF00FFFFFF00FF,
    0xFFFF00FFFFFFFF00,    0xFFFF00FFFFFFFFFF,
    0xFFFFFF0000000000,    0xFFFFFF00000000FF,
    0xFFFFFF000000FF00,    0xFFFFFF000000FFFF,
    0xFFFFFF0000FF0000,    0xFFFFFF0000FF00FF,
    0xFFFFFF0000FFFF00,    0xFFFFFF0000FFFFFF,
    0xFFFFFF00FF000000,    0xFFFFFF00FF0000FF,
    0xFFFFFF00FF00FF00,    0xFFFFFF00FF00FFFF,
    0xFFFFFF00FFFF0000,    0xFFFFFF00FFFF00FF,
    0xFFFFFF00FFFFFF00,    0xFFFFFF00FFFFFFFF,
    0xFFFFFFFF00000000,    0xFFFFFFFF000000FF,
    0xFFFFFFFF0000FF00,    0xFFFFFFFF0000FFFF,
    0xFFFFFFFF00FF0000,    0xFFFFFFFF00FF00FF,
    0xFFFFFFFF00FFFF00,    0xFFFFFFFF00FFFFFF,
    0xFFFFFFFFFF000000,    0xFFFFFFFFFF0000FF,
    0xFFFFFFFFFF00FF00,    0xFFFFFFFFFF00FFFF,
    0xFFFFFFFFFFFF0000,    0xFFFFFFFFFFFF00FF,
    0xFFFFFFFFFFFFFF00,    0xFFFFFFFFFFFFFFFF,
#endif /* PGSZ */
};
#endif /* PSZ == 8 */

#if PSZ == 16
PixelGroup QuartetPixelMaskTable[] = {
#if PGSZ == 32
    0x00000000,
    0x0000FFFF,
    0xFFFF0000,
    0xFFFFFFFF,
#else /* PGSZ == 64 */
    0x0000000000000000,    0x000000000000FFFF,
    0x00000000FFFF0000,    0x00000000FFFFFFFF,
    0x0000FFFF00000000,    0x0000FFFF0000FFFF,
    0x0000FFFFFFFF0000,    0x0000FFFFFFFFFFFF,
    0xFFFF000000000000,    0xFFFF00000000FFFF,
    0xFFFF0000FFFF0000,    0xFFFF0000FFFFFFFF,
    0xFFFFFFFF00000000,    0xFFFFFFFF0000FFFF,
    0xFFFFFFFFFFFF0000,    0xFFFFFFFFFFFFFFFF,
#endif /* PGSZ */
};
#endif /* PSZ == 16 */

#if PSZ == 24
PixelGroup QuartetPixelMaskTable[] = {
#if PGSZ == 32
/* Four pixels consist three pixel groups....*/
    0x00000000, 0x00FFFFFF, /*0x00000000, *//*0*/
/*    0x00000000, 0x00000000, 0x00000000,*/ /*0*/
/*    0x00FFFFFF, 0x00000000, 0x00000000,*/ /*1*/
/*    0xFF000000, 0x0000FFFF, 0x00000000,*/ /*2*/
/*    0xFFFFFFFF, 0x0000FFFF, 0x00000000,*/ /*3*/
/*    0x00000000, 0xFFFF0000, 0x000000FF,*/ /*4*/
/*    0x00FFFFFF, 0xFFFF0000, 0x000000FF,*/ /*5*/
/*    0xFF000000, 0xFFFFFFFF, 0x000000FF,*/ /*6*/
/*    0xFFFFFFFF, 0xFFFFFFFF, 0x000000FF,*/ /*7*/
/*    0x00000000, 0x00000000, 0xFFFFFF00,*/ /*8*/
/*    0x00FFFFFF, 0x00000000, 0xFFFFFF00,*/ /*9*/
/*    0xFF000000, 0x0000FFFF, 0xFFFFFF00,*/ /*10*/
/*    0xFFFFFFFF, 0x0000FFFF, 0xFFFFFF00,*/ /*11*/
/*    0x00000000, 0xFFFF0000, 0xFFFFFFFF,*/ /*12*/
/*    0x00FFFFFF, 0xFFFF0000, 0xFFFFFFFF,*/ /*13*/
/*    0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF,*/ /*14*/
/*    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,*/ /*15*/
#else /* PGSZ == 64 */
    0x0000000000000000,    0x0000000000FFFFFF,
    0x0000FFFFFF000000,    0xFFFFFFFFFFFFFFFF
#endif /* PGSZ */
};
#endif /* PSZ == 24 */

#if PSZ == 32
PixelGroup QuartetPixelMaskTable[] = {
#if PGSZ == 32
    0x00000000,
    0xFFFFFFFF,
#else /* PGSZ == 64 */
    0x0000000000000000,
    0x00000000FFFFFFFF,
    0xFFFFFFFF00000000,
    0xFFFFFFFFFFFFFFFF
#endif /* PGSZ */
};
#endif /* PSZ == 32 */

#if PSZ == 24
int cfb24Shift[] = 
#if	(BITMAP_BIT_ORDER == MSBFirst)
{8,0,16,16,8,24,0,0};
#else	/* (BITMAP_BIT_ORDER == LSBFirst) */
{0,0,24,8,16,16,8,0};
#endif	/* (BITMAP_BIT_ORDER == MSBFirst) */
#endif
