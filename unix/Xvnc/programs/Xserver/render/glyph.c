/*
 * $XFree86: xc/programs/Xserver/render/glyph.c,v 1.5 2001/01/30 07:01:22 keithp Exp $
 *
 * Copyright © 2000 SuSE, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, SuSE, Inc.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "misc.h"
#include "scrnintstr.h"
#include "os.h"
#include "regionstr.h"
#include "validate.h"
#include "windowstr.h"
#include "input.h"
#include "resource.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "dixstruct.h"
#include "gcstruct.h"
#include "servermd.h"
#include "picturestr.h"
#include "glyphstr.h"

/*
 * From Knuth -- a good choice for hash/rehash values is p, p-2 where
 * p and p-2 are both prime.  These tables are sized to have an extra 10%
 * free to avoid exponential performance degradation as the hash table fills
 */
static GlyphHashSetRec glyphHashSets[] = {
    { 32,		43,		41        },
    { 64,		73,		71        },
    { 128,		151,		149       },
    { 256,		283,		281       },
    { 512,		571,		569       },
    { 1024,		1153,		1151      },
    { 2048,		2269,		2267      },
    { 4096,		4519,		4517      },
    { 8192,		9013,		9011      },
    { 16384,		18043,		18041     },
    { 32768,		36109,		36107     },
    { 65536,		72091,		72089     },
    { 131072,		144409,		144407    },
    { 262144,		288361,		288359    },
    { 524288,		576883,		576881    },
    { 1048576,		1153459,	1153457   },
    { 2097152,		2307163,	2307161   },
    { 4194304,		4613893,	4613891   },
    { 8388608,		9227641,	9227639   },
    { 16777216,		18455029,	18455027  },
    { 33554432,		36911011,	36911009  },
    { 67108864,		73819861,	73819859  },
    { 134217728,	147639589,	147639587 },
    { 268435456,	295279081,	295279079 },
    { 536870912,	590559793,	590559791 }
};

#define NGLYPHHASHSETS	(sizeof(glyphHashSets)/sizeof(glyphHashSets[0]))

const CARD8	glyphDepths[GlyphFormatNum] = { 1, 4, 8, 16, 32 };

GlyphHashRec	globalGlyphs[GlyphFormatNum];

int		globalTotalGlyphPrivateSize = 0;

static int	glyphPrivateCount = 0;

void
ResetGlyphPrivates ()
{
    glyphPrivateCount = 0;
}

int
AllocateGlyphPrivateIndex ()
{
    return glyphPrivateCount++;
}

Bool
AllocateGlyphPrivate (ScreenPtr pScreen,
		      int	index2,
		      unsigned	amount)
{
    PictureScreenPtr ps;
    unsigned	     oldamount;

    ps = GetPictureScreenIfSet (pScreen);
    if (!ps)
	return FALSE;

    /* Round up sizes for proper alignment */
    amount = ((amount + (sizeof (DevUnion) - 1)) / sizeof (DevUnion)) *
	sizeof (DevUnion);

    if (index2 >= ps->glyphPrivateLen)
    {
	unsigned *nsizes;
	nsizes = (unsigned *) xrealloc (ps->glyphPrivateSizes,
					(index2 + 1) * sizeof (unsigned));
	if (!nsizes)
	    return FALSE;
	
	while (ps->glyphPrivateLen <= index2)
	{
	    nsizes[ps->glyphPrivateLen++] = 0;
	    ps->totalGlyphPrivateSize += sizeof (DevUnion);
	}
	ps->glyphPrivateSizes = nsizes;
    }
    oldamount = ps->glyphPrivateSizes[index2];
    if (amount > oldamount)
    {
	ps->glyphPrivateSizes[index2] = amount;
	ps->totalGlyphPrivateSize += (amount - oldamount);
    }
    ps->totalGlyphPrivateSize = BitmapBytePad (ps->totalGlyphPrivateSize * 8);
    
    return TRUE;
}

static void
SetGlyphScreenPrivateOffsets (void)
{
    PictureScreenPtr ps;
    int		     offset = 0;
    int		     i;

    for (i = 0; i < screenInfo.numScreens; i++)
    {
	ps = GetPictureScreenIfSet (screenInfo.screens[i]);
	if (ps && ps->totalGlyphPrivateSize)
	{
	    ps->glyphPrivateOffset = offset;
	    offset += ps->totalGlyphPrivateSize / sizeof (DevUnion);
	}
    }
}

static void
SetGlyphPrivatePointers (GlyphPtr glyph)
{
    PictureScreenPtr ps;
    int		     i;
    char	     *ptr;
    DevUnion         *ppriv;
    unsigned         *sizes;
    unsigned         size;
    int		     len;

    for (i = 0; i < screenInfo.numScreens; i++)
    {
	ps = GetPictureScreenIfSet (screenInfo.screens[i]);
	if (ps && ps->totalGlyphPrivateSize)
	{
	    ppriv = glyph->devPrivates + ps->glyphPrivateOffset;
	    sizes = ps->glyphPrivateSizes;
	    ptr = (char *) (ppriv + ps->glyphPrivateLen);
	    for (len = ps->glyphPrivateLen; --len >= 0; ppriv++, sizes++)
	    {
		if ((size = *sizes) != 0)
		{
		    ppriv->ptr = (pointer) ptr;
		    ptr += size;
		}
		else
		    ppriv->ptr = (pointer) 0;
	    }
	}
    }
}

static Bool
ReallocGlobalGlyphPrivate (GlyphPtr glyph)
{
    PictureScreenPtr ps;
    DevUnion         *devPrivates;
    char	     *ptr;
    int		     i;

    devPrivates = xalloc (globalTotalGlyphPrivateSize);
    if (!devPrivates)
	return FALSE;

    ptr = (char *) devPrivates;
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	ps = GetPictureScreenIfSet (screenInfo.screens[i]);
	if (ps && ps->totalGlyphPrivateSize)
	{
	    if (ps->glyphPrivateOffset != -1)
	    {
		memcpy (ptr, glyph->devPrivates + ps->glyphPrivateOffset,
			ps->totalGlyphPrivateSize);
	    }
	    else if (ps->totalGlyphPrivateSize)
	    {
		memset (ptr, 0, ps->totalGlyphPrivateSize);
	    }
	    
	    ptr += ps->totalGlyphPrivateSize;
	}
    }

    if (glyph->devPrivates)
	xfree (glyph->devPrivates);
    
    glyph->devPrivates = devPrivates;

    return TRUE;
}

Bool
GlyphInit (ScreenPtr pScreen)
{
    PictureScreenPtr ps = GetPictureScreen (pScreen);
    
    ps->totalGlyphPrivateSize = 0;
    ps->glyphPrivateSizes = 0;
    ps->glyphPrivateLen = 0;
    ps->glyphPrivateOffset = -1;
    
    return TRUE;
}

Bool
GlyphFinishInit (ScreenPtr pScreen)
{
    PictureScreenPtr ps = GetPictureScreen (pScreen);

    if (ps->totalGlyphPrivateSize)
    {
	GlyphPtr glyph;
	int	 fdepth, i;
	
	globalTotalGlyphPrivateSize += ps->totalGlyphPrivateSize;
	
	for (fdepth = 0; fdepth < GlyphFormatNum; fdepth++)
	{
	    if (!globalGlyphs[fdepth].hashSet)
		continue;
		
	    for (i = 0; i < globalGlyphs[fdepth].hashSet->size; i++)
	    {
		glyph = globalGlyphs[fdepth].table[i].glyph;
		if (glyph && glyph != DeletedGlyph)
		{
		    if (!ReallocGlobalGlyphPrivate (glyph))
			return FALSE;
		}
	    }
	}

	SetGlyphScreenPrivateOffsets ();

	for (fdepth = 0; fdepth < GlyphFormatNum; fdepth++)
	{
	    if (!globalGlyphs[fdepth].hashSet)
		continue;
		
	    for (i = 0; i < globalGlyphs[fdepth].hashSet->size; i++)
	    {
		glyph = globalGlyphs[fdepth].table[i].glyph;
		if (glyph && glyph != DeletedGlyph)
		{
		    SetGlyphPrivatePointers (glyph);
			
		    if (!(*ps->RealizeGlyph) (pScreen, glyph))
			return FALSE;
		}
	    }
	}
    }
    else
	ps->glyphPrivateOffset = 0;
    
    return TRUE;
}

void
GlyphUninit (ScreenPtr pScreen)
{
    PictureScreenPtr ps = GetPictureScreen (pScreen);
    GlyphPtr	     glyph;
    int		     fdepth, i;

    globalTotalGlyphPrivateSize -= ps->totalGlyphPrivateSize;

    for (fdepth = 0; fdepth < GlyphFormatNum; fdepth++)
    {
	if (!globalGlyphs[fdepth].hashSet)
	    continue;
	
	for (i = 0; i < globalGlyphs[fdepth].hashSet->size; i++)
	{
	    glyph = globalGlyphs[fdepth].table[i].glyph;
	    if (glyph && glyph != DeletedGlyph)
	    {
		(*ps->UnrealizeGlyph) (pScreen, glyph);
		
		if (globalTotalGlyphPrivateSize)
		{
		    if (!ReallocGlobalGlyphPrivate (glyph))
			return;
		}
		else
		{
		    if (glyph->devPrivates)
			xfree (glyph->devPrivates);
		    glyph->devPrivates = NULL;
		}
	    }
	}
    }

    if (globalTotalGlyphPrivateSize)
	SetGlyphScreenPrivateOffsets ();

    for (fdepth = 0; fdepth < GlyphFormatNum; fdepth++)
    {
	if (!globalGlyphs[fdepth].hashSet)
	    continue;
	
	for (i = 0; i < globalGlyphs[fdepth].hashSet->size; i++)
	{
	    glyph = globalGlyphs[fdepth].table[i].glyph;    
	    if (glyph && glyph != DeletedGlyph)
	    {
		if (globalTotalGlyphPrivateSize)
		    SetGlyphPrivatePointers (glyph);
	    }
	}
    }

    if (ps->glyphPrivateSizes)
	xfree (ps->glyphPrivateSizes);
}

GlyphHashSetPtr
FindGlyphHashSet (CARD32 filled)
{
    int	i;

    for (i = 0; i < NGLYPHHASHSETS; i++)
	if (glyphHashSets[i].entries >= filled)
	    return &glyphHashSets[i];
    return 0;
}

static int _GlyphSetPrivateAllocateIndex = 0;

int
AllocateGlyphSetPrivateIndex (void)
{
    return _GlyphSetPrivateAllocateIndex++;
}

void
ResetGlyphSetPrivateIndex (void)
{
    _GlyphSetPrivateAllocateIndex = 0;
}

Bool
_GlyphSetSetNewPrivate (GlyphSetPtr glyphSet, int n, pointer ptr)
{
    pointer *new;

    if (n > glyphSet->maxPrivate) {
	if (glyphSet->devPrivates &&
	    glyphSet->devPrivates != (pointer)(&glyphSet[1])) {
	    new = (pointer *) xrealloc (glyphSet->devPrivates,
					(n + 1) * sizeof (pointer));
	    if (!new)
		return FALSE;
	} else {
	    new = (pointer *) xalloc ((n + 1) * sizeof (pointer));
	    if (!new)
		return FALSE;
	    if (glyphSet->devPrivates)
		memcpy (new,
			glyphSet->devPrivates,
			(glyphSet->maxPrivate + 1) * sizeof (pointer));
	}
	glyphSet->devPrivates = new;
	/* Zero out new, uninitialize privates */
	while (++glyphSet->maxPrivate < n)
	    glyphSet->devPrivates[glyphSet->maxPrivate] = (pointer)0;
    }
    glyphSet->devPrivates[n] = ptr;
    return TRUE;
}

GlyphRefPtr
FindGlyphRef (GlyphHashPtr hash, CARD32 signature, Bool match, GlyphPtr compare)
{
    CARD32	elt, step, s;
    GlyphPtr	glyph;
    GlyphRefPtr	table, gr, del;
    CARD32	tableSize = hash->hashSet->size;

    table = hash->table;
    elt = signature % tableSize;
    step = 0;
    del = 0;
    for (;;)
    {
	gr = &table[elt];
	s = gr->signature;
	glyph = gr->glyph;
	if (!glyph)
	{
	    if (del)
		gr = del;
	    break;
	}
	if (glyph == DeletedGlyph)
	{
	    if (!del)
		del = gr;
	    else if (gr == del)
		break;
	}
	else if (s == signature &&
		 (!match || 
		  memcmp (&compare->info, &glyph->info, compare->size) == 0))
	{
	    break;
	}
	if (!step)
	{
	    step = signature % hash->hashSet->rehash;
	    if (!step)
		step = 1;
	}
	elt += step;
	if (elt >= tableSize)
	    elt -= tableSize;
    }
    return gr;
}

CARD32
HashGlyph (GlyphPtr glyph)
{
    CARD32  *bits = (CARD32 *) &(glyph->info);
    CARD32  hash;
    int	    n = glyph->size / sizeof (CARD32);

    hash = 0;
    while (n--)
	hash ^= *bits++;
    return hash;
}

#ifdef CHECK_DUPLICATES
void
DuplicateRef (GlyphPtr glyph, char *where)
{
    ErrorF ("Duplicate Glyph 0x%x from %s\n", glyph, where);
}

void
CheckDuplicates (GlyphHashPtr hash, char *where)
{
    GlyphPtr	g;
    int		i, j;

    for (i = 0; i < hash->hashSet->size; i++)
    {
	g = hash->table[i].glyph;
	if (!g || g == DeletedGlyph)
	    continue;
	for (j = i + 1; j < hash->hashSet->size; j++)
	    if (hash->table[j].glyph == g)
		DuplicateRef (g, where);
    }
}
#else
#define CheckDuplicates(a,b)
#define DuplicateRef(a,b)
#endif

void
FreeGlyph (GlyphPtr glyph, int format)
{
    CheckDuplicates (&globalGlyphs[format], "FreeGlyph");
    if (--glyph->refcnt == 0)
    {
	PictureScreenPtr ps;
	GlyphRefPtr      gr;
	int	         i;
	int	         first;

	first = -1;
	for (i = 0; i < globalGlyphs[format].hashSet->size; i++)
	    if (globalGlyphs[format].table[i].glyph == glyph)
	    {
		if (first != -1)
		    DuplicateRef (glyph, "FreeGlyph check");
		first = i;
	    }

	gr = FindGlyphRef (&globalGlyphs[format],
			   HashGlyph (glyph), TRUE, glyph);
	if (gr - globalGlyphs[format].table != first)
	    DuplicateRef (glyph, "Found wrong one");
	if (gr->glyph && gr->glyph != DeletedGlyph)
	{
	    gr->glyph = DeletedGlyph;
	    gr->signature = 0;
	    globalGlyphs[format].tableEntries--;
	}

	for (i = 0; i < screenInfo.numScreens; i++)
	{
	    ps = GetPictureScreenIfSet (screenInfo.screens[i]);
	    if (ps)
		(*ps->UnrealizeGlyph) (screenInfo.screens[i], glyph);
	}
	
	if (glyph->devPrivates)
	    xfree (glyph->devPrivates);
	xfree (glyph);
    }
}

void
AddGlyph (GlyphSetPtr glyphSet, GlyphPtr glyph, Glyph id)
{
    GlyphRefPtr	    gr;
    CARD32	    hash;

    CheckDuplicates (&globalGlyphs[glyphSet->fdepth], "AddGlyph top global");
    /* Locate existing matching glyph */
    hash = HashGlyph (glyph);
    gr = FindGlyphRef (&globalGlyphs[glyphSet->fdepth], hash, TRUE, glyph);
    if (gr->glyph && gr->glyph != DeletedGlyph)
    {
	PictureScreenPtr ps;
	int              i;
	
	for (i = 0; i < screenInfo.numScreens; i++)
	{
	    ps = GetPictureScreenIfSet (screenInfo.screens[i]);
	    if (ps)
		(*ps->UnrealizeGlyph) (screenInfo.screens[i], glyph);
	}
	if (glyph->devPrivates)
	    xfree (glyph->devPrivates);
	xfree (glyph);
	glyph = gr->glyph;
    }
    else
    {
	gr->glyph = glyph;
	gr->signature = hash;
	globalGlyphs[glyphSet->fdepth].tableEntries++;
    }
    
    /* Insert/replace glyphset value */
    gr = FindGlyphRef (&glyphSet->hash, id, FALSE, 0);
    ++glyph->refcnt;
    if (gr->glyph && gr->glyph != DeletedGlyph)
	FreeGlyph (gr->glyph, glyphSet->fdepth);
    else
	glyphSet->hash.tableEntries++;
    gr->glyph = glyph;
    gr->signature = id;
    CheckDuplicates (&globalGlyphs[glyphSet->fdepth], "AddGlyph bottom");
}

Bool
DeleteGlyph (GlyphSetPtr glyphSet, Glyph id)
{
    GlyphRefPtr     gr;
    GlyphPtr	    glyph;

    gr = FindGlyphRef (&glyphSet->hash, id, FALSE, 0);
    glyph = gr->glyph;
    if (glyph && glyph != DeletedGlyph)
    {
	gr->glyph = DeletedGlyph;
	glyphSet->hash.tableEntries--;
	FreeGlyph (glyph, glyphSet->fdepth);
	return TRUE;
    }
    return FALSE;
}

GlyphPtr
FindGlyph (GlyphSetPtr glyphSet, Glyph id)
{
    GlyphPtr        glyph;

    glyph = FindGlyphRef (&glyphSet->hash, id, FALSE, 0)->glyph;
    if (glyph == DeletedGlyph)
	glyph = 0;
    return glyph;
}

GlyphPtr
AllocateGlyph (xGlyphInfo *gi, int fdepth)
{
    PictureScreenPtr ps;
    int		     size;
    GlyphPtr	     glyph;
    int		     i;

    size = gi->height * PixmapBytePad (gi->width, glyphDepths[fdepth]);
    glyph = (GlyphPtr) xalloc (size + sizeof (GlyphRec));
    if (!glyph)
	return 0;
    glyph->refcnt = 0;
    glyph->size = size + sizeof (xGlyphInfo);
    glyph->info = *gi;

    if (globalTotalGlyphPrivateSize)
    {
	glyph->devPrivates = xalloc (globalTotalGlyphPrivateSize);
	if (!glyph->devPrivates)
	    return 0;

	SetGlyphPrivatePointers (glyph);
    } else
	glyph->devPrivates = NULL;

    for (i = 0; i < screenInfo.numScreens; i++)
    {
	ps = GetPictureScreenIfSet (screenInfo.screens[i]);
	if (ps)
	{
	    if (!(*ps->RealizeGlyph) (screenInfo.screens[i], glyph))
	    {
		while (i--)
		{
		    ps = GetPictureScreenIfSet (screenInfo.screens[i]);
		    if (ps)
			(*ps->UnrealizeGlyph) (screenInfo.screens[i], glyph);
		}
		
		if (glyph->devPrivates)
		    xfree (glyph->devPrivates);
		xfree (glyph);
		return 0;
	    }
	}
    }
    
    return glyph;
}
    
Bool
AllocateGlyphHash (GlyphHashPtr hash, GlyphHashSetPtr hashSet)
{
    hash->table = (GlyphRefPtr) xalloc (hashSet->size * sizeof (GlyphRefRec));
    if (!hash->table)
	return FALSE;
    memset (hash->table, 0, hashSet->size * sizeof (GlyphRefRec));
    hash->hashSet = hashSet;
    hash->tableEntries = 0;
    return TRUE;
}

Bool
ResizeGlyphHash (GlyphHashPtr hash, CARD32 change, Bool global)
{
    CARD32	    tableEntries;
    GlyphHashSetPtr hashSet;
    GlyphHashRec    newHash;
    GlyphRefPtr	    gr;
    GlyphPtr	    glyph;
    int		    i;
    int		    oldSize;
    CARD32	    s;

    tableEntries = hash->tableEntries + change;
    hashSet = FindGlyphHashSet (tableEntries);
    if (hashSet == hash->hashSet)
	return TRUE;
    if (global)
	CheckDuplicates (hash, "ResizeGlyphHash top");
    if (!AllocateGlyphHash (&newHash, hashSet))
	return FALSE;
    if (hash->table)
    {
	oldSize = hash->hashSet->size;
	for (i = 0; i < oldSize; i++)
	{
	    glyph = hash->table[i].glyph;
	    if (glyph && glyph != DeletedGlyph)
	    {
		s = hash->table[i].signature;
		gr = FindGlyphRef (&newHash, s, global, glyph);
		gr->signature = s;
		gr->glyph = glyph;
		++newHash.tableEntries;
	    }
	}
	xfree (hash->table);
    }
    *hash = newHash;
    if (global)
	CheckDuplicates (hash, "ResizeGlyphHash bottom");
    return TRUE;
}

Bool
ResizeGlyphSet (GlyphSetPtr glyphSet, CARD32 change)
{
    return (ResizeGlyphHash (&glyphSet->hash, change, FALSE) &&
	    ResizeGlyphHash (&globalGlyphs[glyphSet->fdepth], change, TRUE));
}
			    
GlyphSetPtr
AllocateGlyphSet (int fdepth, PictFormatPtr format)
{
    GlyphSetPtr	glyphSet;
    int size;
    
    if (!globalGlyphs[fdepth].hashSet)
    {
	if (!AllocateGlyphHash (&globalGlyphs[fdepth], &glyphHashSets[0]))
	    return FALSE;
    }

    size = (sizeof (GlyphSetRec) +
	    (sizeof (pointer) * _GlyphSetPrivateAllocateIndex));
    glyphSet = xalloc (size);
    if (!glyphSet)
	return FALSE;
    bzero((char *)glyphSet, size);
    glyphSet->maxPrivate = _GlyphSetPrivateAllocateIndex - 1;
    if (_GlyphSetPrivateAllocateIndex)
	glyphSet->devPrivates = (pointer)(&glyphSet[1]);

    if (!AllocateGlyphHash (&glyphSet->hash, &glyphHashSets[0]))
    {
	xfree (glyphSet);
	return FALSE;
    }
    glyphSet->refcnt = 1;
    glyphSet->fdepth = fdepth;
    glyphSet->format = format;
    return glyphSet;	
}

int
FreeGlyphSet (pointer	value,
	      XID       gid)
{
    GlyphSetPtr	glyphSet = (GlyphSetPtr) value;
    
    if (--glyphSet->refcnt == 0)
    {
	CARD32	    i, tableSize = glyphSet->hash.hashSet->size;
	GlyphRefPtr table = glyphSet->hash.table;
	GlyphPtr    glyph;
    
	for (i = 0; i < tableSize; i++)
	{
	    glyph = table[i].glyph;
	    if (glyph && glyph != DeletedGlyph)
		FreeGlyph (glyph, glyphSet->fdepth);
	}
	if (!globalGlyphs[glyphSet->fdepth].tableEntries)
	{
	    xfree (globalGlyphs[glyphSet->fdepth].table);
	    globalGlyphs[glyphSet->fdepth].table = 0;
	    globalGlyphs[glyphSet->fdepth].hashSet = 0;
	}
	else
	    ResizeGlyphHash (&globalGlyphs[glyphSet->fdepth], 0, TRUE);
	xfree (table);

	if (glyphSet->devPrivates &&
	    glyphSet->devPrivates != (pointer)(&glyphSet[1]))
	    xfree(glyphSet->devPrivates);

	xfree (glyphSet);
    }
    return Success;
}
