/*
 * $XFree86: xc/programs/Xserver/render/glyph.c,v 1.6 2001/10/28 03:34:19 tsi Exp $
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

GlyphHashSetPtr
FindGlyphHashSet (CARD32 filled)
{
    int	i;

    for (i = 0; i < NGLYPHHASHSETS; i++)
	if (glyphHashSets[i].entries >= filled)
	    return &glyphHashSets[i];
    return 0;
}

Bool
GlyphInit (ScreenPtr pScreen)
{
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
	GlyphRefPtr gr;
	int	    i;
	int	    first;

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
    int		size;
    GlyphPtr	glyph;

    size = gi->height * PixmapBytePad (gi->width, glyphDepths[fdepth]);
    glyph = (GlyphPtr) xalloc (size + sizeof (GlyphRec));
    if (!glyph)
	return 0;
    glyph->refcnt = 0;
    glyph->size = size + sizeof (xGlyphInfo);
    glyph->info = *gi;
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
    
    if (!globalGlyphs[fdepth].hashSet)
    {
	if (!AllocateGlyphHash (&globalGlyphs[fdepth], &glyphHashSets[0]))
	    return FALSE;
    }
    glyphSet = xalloc (sizeof (GlyphSetRec));
    if (!glyphSet)
	return FALSE;
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
	xfree (glyphSet);
    }
    return Success;
}
