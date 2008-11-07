/* $TOG: patcache.c /main/8 1997/06/12 11:51:59 barstow $ */

/*

Copyright (c) 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/
/* $XFree86: xc/lib/font/util/patcache.c,v 3.0.4.1 1997/07/05 15:55:37 dawes Exp $ */

/*
 * Author:  Keith Packard, MIT X Consortium
 */

#include    <fontmisc.h>
#include    <fontstruct.h>

/*
 * Static sized hash table for looking up font name patterns
 *
 * LRU entries, reusing old entries
 */

#define NBUCKETS	16
#define NENTRIES	64

#define UNSET		(NENTRIES+1)

typedef unsigned char	EntryPtr;

typedef struct _FontPatternCacheEntry {
    struct _FontPatternCacheEntry   *next, **prev;
    short			    patlen;
    char			    *pattern;
    int				    hash;
    FontPtr			    pFont;	/* associated font */
} FontPatternCacheEntryRec, *FontPatternCacheEntryPtr;

typedef struct _FontPatternCache {
    FontPatternCacheEntryPtr	buckets[NBUCKETS];
    FontPatternCacheEntryRec	entries[NENTRIES];
    FontPatternCacheEntryPtr	free;
} FontPatternCacheRec;

/* Create and initialize cache */
FontPatternCachePtr
MakeFontPatternCache ()
{
    FontPatternCachePtr	cache;
    int			i;
    cache = (FontPatternCachePtr) xalloc (sizeof *cache);
    if (!cache)
	return 0;
    for (i = 0; i < NENTRIES; i++) {
	cache->entries[i].patlen = 0;
	cache->entries[i].pattern = 0;
	cache->entries[i].pFont = 0;
    }
    EmptyFontPatternCache (cache);
    return cache;
}

/* toss cache */
void
FreeFontPatternCache (cache)
    FontPatternCachePtr	cache;
{
    int	    i;

    for (i = 0; i < NENTRIES; i++)
	xfree (cache->entries[i].pattern);
    xfree (cache);
}

/* compute id for string */
static
Hash (string, len)
    char    *string;
    int	    len;
{
    int	hash;

    hash = 0;
    while (len--)
	hash = (hash << 1) ^ *string++;
    if (hash < 0)
	hash = -hash;
    return hash;
}

/* Empty cache (for rehash) */
void
EmptyFontPatternCache (cache)
    FontPatternCachePtr	cache;
{
    int	    i;
    
    for (i = 0; i < NBUCKETS; i++)
	cache->buckets[i] = 0;
    for (i = 0; i < NENTRIES; i++)
    {
	cache->entries[i].next = &cache->entries[i+1];
	cache->entries[i].prev = 0;
	cache->entries[i].pFont = 0;
	xfree (cache->entries[i].pattern);
	cache->entries[i].pattern = 0;
	cache->entries[i].patlen = 0;
    }
    cache->free = &cache->entries[0];
    cache->entries[NENTRIES - 1].next = 0;
}

/* add entry */
void
CacheFontPattern (cache, pattern, patlen, pFont)
    FontPatternCachePtr	cache;
    char		*pattern;
    int			patlen;
    FontPtr		pFont;
{
    FontPatternCacheEntryPtr	e;
    char			*newpat;
    int				i;

    newpat = (char *) xalloc (patlen);
    if (!newpat)
	return;
    if (cache->free)
    {
	e = cache->free;
	cache->free = e->next;
    }
    else
    {
    	i = rand ();
    	if (i < 0)
	    i = -i;
    	i %= NENTRIES;
	e = &cache->entries[i];
	if (e->next)
	    e->next->prev = e->prev;
	*e->prev = e->next;
	xfree (e->pattern);
    }
    /* set pattern */
    memcpy (newpat, pattern, patlen);
    e->pattern = newpat;
    e->patlen = patlen;
    /* link to new hash chain */
    e->hash = Hash (pattern, patlen);
    i = e->hash % NBUCKETS;
    e->next = cache->buckets[i];
    if (e->next)
	e->next->prev = &(e->next);
    cache->buckets[i] = e;
    e->prev = &(cache->buckets[i]);
    e->pFont = pFont;
}

/* find matching entry */
FontPtr
FindCachedFontPattern (cache, pattern, patlen)
    FontPatternCachePtr	cache;
    char		*pattern;
    int			patlen;
{
    int				hash;
    int				i;
    FontPatternCacheEntryPtr	e;

    hash = Hash (pattern, patlen);
    i = hash % NBUCKETS;
    for (e = cache->buckets[i]; e; e = e->next)
    {
	if (e->patlen == patlen && e->hash == hash &&
	    !memcmp (e->pattern, pattern, patlen))
	{
	    return e->pFont;
	}
    }
    return 0;
}

void
RemoveCachedFontPattern (cache, pFont)
    FontPatternCachePtr	cache;
    FontPtr		pFont;
{
    FontPatternCacheEntryPtr	e;
    int				i;

    for (i = 0; i < NENTRIES; i++)
    {
	if ((e = &cache->entries[i])->pFont == pFont)
	{
	    e->pFont = 0;
	    if (e->next)
		e->next->prev = e->prev;
	    *e->prev = e->next;
	    e->next = cache->free;
	    cache->free = e;
	    xfree (e->pattern);
	    e->pattern = 0;
	}
    }
}
