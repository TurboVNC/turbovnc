/*-
 * Copyright (c) 1998-1999 Shunsuke Akiyama <akiyama@jp.FreeBSD.org>.
 * All rights reserved.
 * Copyright (c) 1998-1999 X-TrueType Server Project, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	Id: fontcache.c,v 1.19 1999/01/31 13:06:00 akiyama Exp $
 */
/* $XFree86: xc/lib/font/fontcache/fontcache.c,v 1.4 2001/04/05 17:42:28 dawes Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fontcache.h"

#define LOW_MARK	0
#define HI_MARK		1

#define PURGE_ENTRY	1
#define PURGE_BITMAP	2

typedef struct {
    long hiMark;	/* Cache hi water mark */
    long lowMark;	/* Cache low water mark */
    long allocated;	/* Cache allocated size */
    long used;		/* Cache used size */
} FontCacheSize_t;

static int CacheInitialized = 0;

static TAILQ_HEAD(FcInUseQueue, cache_entry) InUseQueueHead, *InUseQueue;
static TAILQ_HEAD(FcFreeQueue, cache_entry) FreeQueueHead, *FreeQueue;
static FCBCB FreeBitmapHead, *FreeBitmap;

static long CacheHiMark;
static long CacheLowMark;
static int CacheBalance;
static FontCacheSize_t HashSize;
static FontCacheSize_t AllocSize;
static int NeedPurgeCache;
static FontCacheStatistics CacheStatistics;

static void fc_assign_cache(void);
static int fc_assign_entry(void);
static void fc_flush_cache(void);
static int fc_get_bitmap_area(FontCacheEntryPtr, int);
static void fc_free_bitmap_area(FontCacheBitmapPtr);
static int fc_check_size(int);
static void fc_purge_cache(void);
static void fc_purge_bitmap(void);
static void fc_flush_cache_bitmap(void);
static void fc_flush_cache_inuse(void);
static void fc_flush_cache_free(void);
static void fc_purge_cache_entry(void);
static void fc_purge_cache_entry_pool(void);
static void fc_purge_bitmap_pool(void);


/*
 *  FontCacheInitialize()
 *
 *  Initialize cache work area.
 */

int
FontCacheInitialize()
{
#ifdef FONTCACHE
    int i;

    if (!CacheInitialized) {
	/*
	 *  first time initialization
	 */
#if defined(HASH_DEBUG) || defined(DEBUG)
	fprintf(stderr, "FontCacheInitialize: initializing cache\n");
#endif
	InUseQueue = &InUseQueueHead;
	TAILQ_INIT(InUseQueue);

	FreeQueue = &FreeQueueHead;
	TAILQ_INIT(FreeQueue);

	FreeBitmap = &FreeBitmapHead;
	FreeBitmap->index = 0;
	for (i = 0; i < FC_MEM_HASH_SIZE; i++) {
	    TAILQ_INIT(&FreeBitmap->head[i]);
	}

	CacheHiMark = FC_DEFAULT_CACHE_SIZE * 1024;	/* temporary */
	CacheLowMark = (CacheHiMark / 4) * 3;
	CacheBalance = FC_CACHE_BALANCE;

	NeedPurgeCache = 0;

	HashSize.allocated = HashSize.used = 0;
	AllocSize.allocated = AllocSize.used = 0;
	fc_assign_cache();
	fc_assign_entry();
#if defined(DEBUG)
	fprintf(stderr, "FontCacheInitialize: hi=%ld, lo=%ld, bal=%d\n",
        	CacheHiMark, CacheLowMark, CacheBalance);
#endif

	CacheInitialized = 1;
    } else {
	/*
	 *  second time or later case.
	 *  flush and reassign cache.
	 */
#if defined(HASH_DEBUG) || defined(DEBUG)
	fprintf(stderr, "FontCacheInitialize: initializing cache, again\n");
#endif
    }

    memset(&CacheStatistics, 0, sizeof (CacheStatistics));
#endif /* FONTCACHE */

    return 0;		/* make lint happy */
}

/*
 *  FontCacheChangeSettings()
 *
 *  Change cache size and reinitialize work areas.
 *
 *  Returns 0, if memory allocation failed.  Otherwise 1.
 */

int
FontCacheChangeSettings(FontCacheSettingsPtr cs)
{
    int result;

    if (!CacheInitialized) {
        FontCacheInitialize();
        if (!CacheInitialized)
            return 0;
    }

#if defined(HASH_DEBUG) || defined(DEBUG)
fprintf(stderr,
	"FontCahceChangeSettings: hi-mark=%ld, low-mark=%ld, balance=%ld\n",
	cs->himark, cs->lowmark, cs->balance);
#endif

    fc_flush_cache();

    CacheHiMark = cs->himark;
    CacheLowMark = cs->lowmark;
    CacheBalance = cs->balance;

    fc_assign_cache();
    result = fc_assign_entry();

    return result;
}

/*
 *  FontCacheGetSettings()
 *
 *  Get current cache control parameters.
 */

void
FontCacheGetSettings(FontCacheSettingsPtr cs)
{
    if (!CacheInitialized) {
        FontCacheInitialize();
        if (!CacheInitialized)
            return;
    }

    cs->himark = CacheHiMark;
    cs->lowmark = CacheLowMark;
    cs->balance = CacheBalance;
}

/*
 *  FontCacheGetStatistics()
 *
 *  Get current cache statistics.
 */

void
FontCacheGetStatistics(FontCacheStatisticsPtr cs)
{
    if (!CacheInitialized) {
        FontCacheInitialize();
        if (!CacheInitialized)
            return;
    }

    CacheStatistics.purge_stat = NeedPurgeCache;
    CacheStatistics.balance = CacheBalance;
    CacheStatistics.f.usage = HashSize.used;
    CacheStatistics.v.usage = AllocSize.used;

    memcpy(cs, &CacheStatistics, sizeof (CacheStatistics));
}

/*
 *  FontCacheOpenCache()
 *
 *  Allocate font cache control block and initialize it.
 *
 *  Returns pointer to font cache control block.  Or returns NULL when
 *  detected illegal parameter or memory allocation failed.
 */

FCCBPtr
FontCacheOpenCache(void *arg)
{
    int linesize;
    FCCBPtr this;
    int size = 0, mask = 0;
    int i;

    static int sizes[] = { 16, 32, 64, 128, 0 };

    if (!CacheInitialized) {
        FontCacheInitialize();
        if (!CacheInitialized)
            return NULL;
    }

    linesize = (long)arg;
#if defined(HASH_DEBUG) || defined(DEBUG)
fprintf(stderr, "FontCacheOpenCache: line size=%d\n", linesize);
#endif

    for (i = 0; sizes[i] != 0; i++) {
	if (sizes[i] == linesize) {
	    size = linesize;
	    mask = linesize - 1;
	    break;
	}
    }
    if (sizes[i] == 0) {
	return NULL;
    }

    this = (FCCBPtr) malloc(sizeof (FCCB));
    if (this != NULL) {
	memset(this, 0, sizeof (FCCB));
	this->head = (FontCacheHeadPtr) malloc(sizeof (FontCacheHead) * size);
	if (this->head == NULL) {
	    free(this);
	    this = NULL;
	} else {
	    this->size = size;
	    this->mask = mask;
	    for (i = 0; i < size; i++) {
		TAILQ_INIT(&this->head[i]);
	    }
	}
    }

    return this;
}

/*
 *  FontCacheCloseCache()
 *
 *  Release font cache control block and all it's related entries.
 */

void
FontCacheCloseCache(FCCBPtr this)
{
    FontCacheEntryPtr entry, next;
    int i;
    int size;

    if (!CacheInitialized) {
	return;
    }

    size = this->size;
    for (i = 0; i < size; i++) {
	entry = TAILQ_FIRST(&this->head[i]);
	while (entry != NULL) {
	    /* remove entry from in-use queue, here */
	    TAILQ_REMOVE(InUseQueue, entry, c_lru);
	    
	    /* remove entry from the hash */
	    if (entry->bitmapsize > FC_SMALL_BITMAP_SIZE
		&& entry->charInfo.bits != NULL) {
		fc_free_bitmap_area(entry->bmp);
	    }
	    entry->charInfo.bits = NULL;
	    entry->bitmapsize = 0;

	    next = TAILQ_NEXT(entry, c_hash);
	    TAILQ_INSERT_HEAD(FreeQueue, entry, c_lru);
	    HashSize.used -= sizeof (FontCacheEntry);
	    entry = next;
	}
    }

    free(this->head);
    free(this);
}

/*
 *  FontCacheGetEntry()
 *
 *  Allocate font cache entry and initialize it.
 */

FontCacheEntryPtr
FontCacheGetEntry()
{
    FontCacheEntryPtr entry;
    FontCacheEntryPtr p;
    long size;

    /* scan in-use queue and purge if required */
    fc_purge_cache();

    /* allocate hash entry */
    if (TAILQ_EMPTY(FreeQueue)) {
	size = sizeof (FontCacheEntry);
	p = (FontCacheEntryPtr) malloc(size);
	if (p != NULL) {
	    TAILQ_INSERT_HEAD(FreeQueue, p, c_lru);
	    HashSize.allocated += size;
#if defined(HASH_DEBUG) || defined(DEBUG)
fprintf(stderr, "FontCachegetEntry: allocated new entry\n");
#endif
	}
    }

    if (!TAILQ_EMPTY(FreeQueue)) {
	entry = TAILQ_FIRST(FreeQueue);
	TAILQ_REMOVE(FreeQueue, entry, c_lru);
	memset(entry, 0, sizeof (FontCacheEntry));
    } else {
	entry = NULL;
    }

    return entry;
}

/*
 *  FontCacheGetBitmap()
 *
 *  Allocate font glyph bitmap area.
 *
 *  Note:
 *    Allocated area should be cleared.
 */

int
FontCacheGetBitmap(FontCacheEntryPtr entry, int size)
{
    int oldsize;
    int result;

    /* XXX */
    if ((AllocSize.used > AllocSize.hiMark - size) &&
       (size > FC_SMALL_BITMAP_SIZE)) {
      fc_purge_bitmap();
    }

    if (size < 0) /* wrong size */
      return 0;

    result = 0;
    oldsize = entry->bitmapsize;
    if (size <= FC_SMALL_BITMAP_SIZE) {
	/* use coresponding bitmap area */
	if (oldsize > FC_SMALL_BITMAP_SIZE) {
	    /* We don't need allocated area anymore */
	    fc_free_bitmap_area(entry->bmp);
	}
	entry->bitmapsize = size;
	if (size > 0) {
	entry->charInfo.bits = entry->bitmap;
	memset(entry->charInfo.bits, 0, size);
	} else
	  entry->charInfo.bits = NULL;
	  
	result = 1;
    } else {
	/* need extra bitmap area */
	if (entry->charInfo.bits == NULL) {
	    /* no any extra bitmap area */
	    if (fc_get_bitmap_area(entry, size)) {
		entry->bitmapsize = size;
		memset(entry->charInfo.bits, 0, size);
		if (fc_check_size(HI_MARK)) {
		    fc_purge_cache();
		}
		result = 1;
	    }
	} else {
	    /* we already have extra bitmap area */
	    if (oldsize == size) {
		/* same size, reuse it */
		memset(entry->charInfo.bits, 0, size);
		result = 1;
	    } else {
		/* different size */
		fc_free_bitmap_area(entry->bmp);
		if (fc_get_bitmap_area(entry, size)) {
		    entry->bitmapsize = size;
		    memset(entry->charInfo.bits, 0, size);
		    if (fc_check_size(HI_MARK)) {
			fc_purge_cache();
		    }
		    result = 1;
		}
	    }
	}
    }

    return result;
}

/*
 *  FontCacheSearchEntry()
 *
 *  Search an entry matched with the key from the hash.
 */

int
FontCacheSearchEntry(FCCBPtr this, int key, FontCacheEntryPtr *value)
{
    FontCacheHeadPtr head;
    FontCacheEntryPtr entry;
    int index;

    index = key & this->mask;
    head = &this->head[index];

    TAILQ_FOREACH(entry, head, c_hash) {
	if (entry->key == key) {
	    /* found, change position */
	    CacheStatistics.f.hits++;

	    TAILQ_REMOVE(InUseQueue, entry, c_lru);
	    TAILQ_INSERT_HEAD(InUseQueue, entry, c_lru);

	    TAILQ_REMOVE(head, entry, c_hash);
	    TAILQ_INSERT_HEAD(head, entry, c_hash);

	    /* purge least recentrly used cache entirs */
	    fc_purge_cache();

	    *value = entry;
	    return 1;
	}
    }

    /* purge least recentrly used cache entirs */
    fc_purge_cache();

    /* not found */
    CacheStatistics.f.misshits++;
    *value = NULL;
    return 0;
}

/*
 *  FontCacheInsertEntry()
 *
 *  Insert an entry into the cache pool.
 */

int
FontCacheInsertEntry(FCCBPtr this, int key, FontCacheEntryPtr entry)
{
    FontCacheHeadPtr head;
    int index;

    index = key & this->mask;
    head = &this->head[index];

    entry->key = key;
    entry->c_head = head;
    TAILQ_INSERT_HEAD(head, entry, c_hash);

    /* insert entry into in-use queue */
    TAILQ_INSERT_HEAD(InUseQueue, entry, c_lru);

    /* adjust cache in-use size */
    HashSize.used += sizeof (FontCacheEntry);
    if (fc_check_size(HI_MARK)) {
	fc_purge_cache();
    }

    return 1;
}

/*
 *  fc_assign_cache()
 *
 *  Assign cache size considered with cache balance rate.
 */

static void
fc_assign_cache()
{
    HashSize.hiMark = (CacheHiMark * CacheBalance) / 100;
    HashSize.lowMark = (CacheLowMark * CacheBalance) / 100;

    AllocSize.hiMark = (CacheHiMark * (100 - CacheBalance)) / 100;
    AllocSize.lowMark = (CacheLowMark * (100 - CacheBalance)) / 100;
}

/*
 *  fc_assign_entry()
 *
 *  Assign cache entry into free queue.
 *
 *  Returns 0, when memory allocation failed.  Otherwise 1.
 */

static int
fc_assign_entry()
{
    FontCacheEntryPtr entry;
    long used;
    int result = 1;

    used = 0;
    while ((used + sizeof (FontCacheEntry)) < HashSize.hiMark) {
	entry = (FontCacheEntryPtr) malloc(sizeof (FontCacheEntry));
	if (entry == NULL) {
	    fprintf(stderr, "fc_assign_entry: can't allocate memory.\n");
	    result = 0;
	    break;
	}
	TAILQ_INSERT_HEAD(FreeQueue, entry, c_lru);
	used += sizeof (FontCacheEntry);
	HashSize.allocated += sizeof (FontCacheEntry);
    }

    return result;
}

/*
 *  fc_get_bitmap_area()
 *
 *  Search allocated memory area from free bitmap hash pool.  If there
 *  is no entry, then allocate new bitmap area.
 *
 *  Returns 0, when memory allocation failed, otherwise 1.  And some
 *  sort of cache entry structure members were updated.
 */

static int
fc_get_bitmap_area(FontCacheEntryPtr this, int size)
{
    FontCacheBitmapHeadPtr head;
    FontCacheBitmapPtr bitmap;
    int index;
    int result = 0;

    index = size & FC_MEM_HASH_MASK;
    head = &FreeBitmap->head[index];
    TAILQ_FOREACH(bitmap, head, b_hash) {
	if (bitmap->key == size) {
	    TAILQ_REMOVE(head, bitmap, b_hash);
	    this->bmp = bitmap;
	    this->charInfo.bits = (char *) (bitmap + 1);
	    bitmap->b_entry = this;
	    result = 1;
	    CacheStatistics.v.hits++;
	    AllocSize.used += (size + sizeof (FontCacheBitmap));
#if defined(HASH_DEBUG) || defined(DEBUG)
fprintf(stderr, "fc_get_bitmap_area: bitmap entry found in pool\n");
#endif
	    break;
	}
    }

    if (result == 0) {
        CacheStatistics.v.misshits++;
	bitmap = (FontCacheBitmapPtr) malloc(size + sizeof (FontCacheBitmap));
	if (bitmap != NULL) {
	    bitmap->b_entry = this;
	    bitmap->size = size + sizeof (FontCacheBitmap);
	    bitmap->key = size;
	    this->bmp = bitmap;
	    this->charInfo.bits = (char *) (bitmap + 1);
	    AllocSize.allocated += (size + sizeof (FontCacheBitmap));
	    AllocSize.used += (size + sizeof (FontCacheBitmap));
	    result = 1;
#if defined(HASH_DEBUG) || defined(DEBUG)
fprintf(stderr, "fc_get_bitmap_area: bitmap entry allocated\n");
#endif
	} else {
	    this->bmp = NULL;
	    this->charInfo.bits = NULL;
	}
    }

    return result;
}

/*
 *  fc_free_bitmap_area()
 *
 *  Release allocated bitmap area into free hash pool.
 */

static void
fc_free_bitmap_area(FontCacheBitmapPtr this)
{
    FontCacheBitmapHeadPtr head;
    FontCacheEntryPtr entry;
    int index;

#if defined(HASH_DEBUG) || defined(DEBUG)
fprintf(stderr, "fc_free_bitmap_area: bitmap entry returns into pool\n");
#endif

    index = this->key & FC_MEM_HASH_MASK;
    head = &FreeBitmap->head[index];
    TAILQ_INSERT_HEAD(head, this, b_hash);

    AllocSize.used -= this->size;

    entry = this->b_entry;
    entry->bmp = NULL;
    entry->bitmapsize = 0;
}

/*
 *  fc_flush_cache_bitmap()
 *
 *  Flush all allocated bitmap area from the free hash pool.
 */

static void
fc_flush_cache_bitmap()
{
    FontCacheBitmapHeadPtr head;
    FontCacheBitmapPtr bitmap;
    int i;

    for (i = 0; i < FC_MEM_HASH_SIZE; i++) {
	head = &FreeBitmap->head[i];
	while (!TAILQ_EMPTY(head)) {
	    bitmap = TAILQ_FIRST(head);
	    TAILQ_REMOVE(head, bitmap, b_hash);

	    AllocSize.allocated -= bitmap->size;
	    free(bitmap);
	}
    }
}

/*
 *  fc_flush_cache_inuse()
 *
 *  Release all in-use cache entries.
 */

static void
fc_flush_cache_inuse()
{
    FontCacheEntryPtr entry;
    FontCacheHeadPtr head;

    while (!TAILQ_EMPTY(InUseQueue)) {
	/* remove this entry from in-use queue */
	entry = TAILQ_FIRST(InUseQueue);
	TAILQ_REMOVE(InUseQueue, entry, c_lru);

	/* remove this entry from hash */
	head = entry->c_head;
	TAILQ_REMOVE(head, entry, c_hash);

	/* release bitmap area */
	if (entry->bitmapsize > FC_SMALL_BITMAP_SIZE
	    && entry->charInfo.bits != NULL) {
	    fc_free_bitmap_area(entry->bmp);
	}
	entry->charInfo.bits = NULL;
	entry->bitmapsize = 0;

        /* release font-specific private area */
        if ( entry->vfuncs && entry->vfuncs->f_private_dispose )
            (*entry->vfuncs->f_private_dispose)(entry->f_private);
        entry->f_private = NULL;
        entry->vfuncs = NULL;

	/* add this entry to free queue */
	TAILQ_INSERT_HEAD(FreeQueue, entry, c_lru);

	/* adjust size */
	HashSize.used -= sizeof (FontCacheEntry);
    }
}

/*
 *  fc_flush_cache_free()
 *
 *  Flush all free cache entries from the free cache queue.
 */

static void
fc_flush_cache_free()
{
    FontCacheEntryPtr entry;

    /* release entire entries of the free queue */
    while (!TAILQ_EMPTY(FreeQueue)) {
	entry = TAILQ_FIRST(FreeQueue);
	TAILQ_REMOVE(FreeQueue, entry, c_lru);
	free(entry);
	HashSize.allocated -= sizeof (FontCacheEntry);
    }
}

/*
 *  fc_flush_cache()
 *
 *  Flush all cache entries and allocated bitmap area from the pool.
 */

static void
fc_flush_cache()
{
    fc_flush_cache_inuse();
    fc_flush_cache_bitmap();
    fc_flush_cache_free();

    memset(&CacheStatistics, 0, sizeof (CacheStatistics));
}

/*
 *  fc_check_size()
 *
 *  Check cache size, then return it's result.
 */

static int
fc_check_size(int mark)
{
    int result = 0;

    if (mark == LOW_MARK) {
	if (HashSize.used > HashSize.lowMark) {
	    result |= PURGE_ENTRY;
	}
	if (AllocSize.used > AllocSize.lowMark) {
	    result |= PURGE_BITMAP;
	}
    } else {
	if (HashSize.used > HashSize.hiMark) {
	    result |= PURGE_ENTRY;
	}
	if (AllocSize.used > AllocSize.hiMark) {
	    result |= PURGE_BITMAP;
	}
    }

    return result;
}

/*
 *  fc_purge_cache_entry()
 *
 *  Purge least recently used cache entry.
 */

static void
fc_purge_cache_entry()
{
    FontCacheHeadPtr head;
    FontCacheEntryPtr entry;
    int i;

    for (i = 0; i < FC_PURGE_PER_SCAN; i++) {
	/* get least recently used entry */
	entry = TAILQ_LAST(InUseQueue, FcInUseQueue);

#if defined(HASH_DEBUG) || defined(DEBUG)
fprintf(stderr, "fc_purge_cache_entry: purged: %p, %d\n",
	entry, entry->key);
#endif

	/* remove this entry from in-use queue */
	TAILQ_REMOVE(InUseQueue, entry, c_lru);

	/* remove this entry from the hash */
	head = entry->c_head;
	TAILQ_REMOVE(head, entry, c_hash);

	/* release bitmap area */
	if (entry->bitmapsize > FC_SMALL_BITMAP_SIZE
	    && entry->charInfo.bits != NULL) {
	    fc_free_bitmap_area(entry->bmp);
	    CacheStatistics.v.purged++;
	}
	entry->charInfo.bits = NULL;
	entry->bitmapsize = 0;

        /* release font-specific private area */
        if ( entry->vfuncs && entry->vfuncs->f_private_dispose )
            (*entry->vfuncs->f_private_dispose)(entry->f_private);
        entry->f_private = NULL;
        entry->vfuncs = NULL;

	/* add this entry to free queue */
	TAILQ_INSERT_HEAD(FreeQueue, entry, c_lru);

	HashSize.used -= sizeof (FontCacheEntry);
	CacheStatistics.f.purged++;
    }
}

/*
 *  fc_purge_cache_entry_pool()
 *
 *  Purge free cache entries, to adjust cache size.
 */

static void
fc_purge_cache_entry_pool()
{
    FontCacheEntryPtr entry;

    while (!TAILQ_EMPTY(FreeQueue)) {
	entry = TAILQ_LAST(FreeQueue, FcFreeQueue);
	TAILQ_REMOVE(FreeQueue, entry, c_lru);
#if defined(HASH_DEBUG) || defined(DEBUG)
fprintf(stderr, "fc_purge_cache_entry_pool: purged from free queue: %p\n",
	entry);
#endif
	HashSize.allocated -= sizeof (FontCacheEntry);
	free(entry);
	if (HashSize.allocated <= HashSize.hiMark) {
	    break;
	}
    }
}

/*
 *  fc_purge_bitmap()
 *
 *  Purge least recently used allocated bitmap area.
 */

static void
fc_purge_bitmap()
{
    FontCacheEntryPtr entry, first;
    int purged = 0;

    /* release used entry, if required */
    first = TAILQ_FIRST(InUseQueue);
    if (first != NULL) {
	entry = TAILQ_LAST(InUseQueue, FcInUseQueue);
	while (purged < FC_PURGE_PER_SCAN) {
	    if (entry->bmp != NULL) {
#if defined(HASH_DEBUG) || defined(DEBUG)
fprintf(stderr, "fc_purge_bitmap: purged from live queue: %p, %d(%d)\n",
	entry->bmp, entry->bmp->key, entry->bmp->size);
#endif
		fc_free_bitmap_area(entry->bmp);
		entry->charInfo.bits = NULL;
		CacheStatistics.v.purged++;
		purged++;
	    }
	    if (entry == first) {
		break;
	    }
	    entry = TAILQ_PREV(entry, FcInUseQueue, c_lru);
	}
    }
}

/*
 *  fc_purge_bitmap_pool()
 *
 *  Purge free bitmap area from pool, to adjust cache size.
 */

static void
fc_purge_bitmap_pool()
{
    int this, stop, quit;
    FontCacheBitmapHeadPtr head;
    FontCacheBitmapPtr bitmap;

    /* release free bitmap entry */
    this = FreeBitmap->index;
    stop = this;
    quit = 0;

    do {
	head = &FreeBitmap->head[this];
	while (!TAILQ_EMPTY(head)) {
	    bitmap = TAILQ_LAST(head, fcmem_head);
	    TAILQ_REMOVE(head, bitmap, b_hash);
#if defined(HASH_DEBUG) || defined(DEBUG)
fprintf(stderr, "fc_purge_bitmap_pool: purged from pool: %p, %d(%d)\n",
	bitmap, bitmap->key, bitmap->size);
#endif
	    AllocSize.allocated -= bitmap->size;
	    free(bitmap);
	    if (AllocSize.allocated <= AllocSize.hiMark) {
		quit = 1;
		break;
	    }
	}
	this++;
	this &= FC_MEM_HASH_MASK;
    } while (this != stop && quit == 0);

    FreeBitmap->index++;
    FreeBitmap->index &= FC_MEM_HASH_MASK;
}

/*
 *  fc_purge_cache()
 *
 *  Purge font cache, if required.
 */

static void
fc_purge_cache()
{
    int strategy;

    if (NeedPurgeCache) {
	strategy = fc_check_size(LOW_MARK);
	switch (strategy) {
	case PURGE_ENTRY :
	    CacheStatistics.purge_runs++;
	    fc_purge_cache_entry();
	    break;
	case PURGE_BITMAP :
	    CacheStatistics.purge_runs++;
	    fc_purge_bitmap();
	    break;
	case (PURGE_ENTRY | PURGE_BITMAP) :
	    CacheStatistics.purge_runs++;
	    fc_purge_cache_entry();
	    fc_purge_bitmap();
	    break;
	default :
	    NeedPurgeCache = 0;
	    break;
	}
    } else {
	strategy = fc_check_size(HI_MARK);
	switch (strategy) {
	case PURGE_ENTRY :
	    if ((CacheBalance + FC_BALANCE_DIFFS) <= FC_BALANCE_HI) {
		CacheBalance += FC_BALANCE_DIFFS;
#if defined(HASH_DEBUG) || defined(DEBUG)
fprintf(stderr, "fc_purge_cache: cache balance changed to %d\n", CacheBalance);
#endif
		fc_assign_cache();
		fc_purge_bitmap_pool();
	    } else {
		CacheStatistics.purge_runs++;
		NeedPurgeCache = 1;
		while (fc_check_size(HI_MARK) & PURGE_ENTRY) {
		    fc_purge_cache_entry();
		}
	    }
	    break;
	case PURGE_BITMAP :
	    if ((CacheBalance - FC_BALANCE_DIFFS) >= FC_BALANCE_LOW) {
		CacheBalance -= FC_BALANCE_DIFFS;
#if defined(HASH_DEBUG) || defined(DEBUG)
fprintf(stderr, "fc_purge_cache: cache balance changed to %d\n", CacheBalance);
#endif
		fc_assign_cache();
		fc_purge_cache_entry_pool();
	    } else {
		CacheStatistics.purge_runs++;
		NeedPurgeCache = 1;
		while (fc_check_size(HI_MARK) & PURGE_BITMAP) {
		    fc_purge_bitmap();
		}
	    }
	    break;
	case (PURGE_ENTRY | PURGE_BITMAP) :
	    CacheStatistics.purge_runs++;
	    NeedPurgeCache = 1;
	    while (fc_check_size(HI_MARK)) {
		fc_purge_cache_entry();
		fc_purge_bitmap();
	    }
	    break;
	default :
	    break;
	}
    }
}
