/* $XConsortium: atom.c,v 1.4 94/04/17 20:17:31 gildea Exp $ */

/*

Copyright (c) 1990, 1994  X Consortium

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

/*
 * Author:  Keith Packard, MIT X Consortium
 */

/* lame atom replacement routines for font applications */

#include "fontmisc.h"

typedef struct _AtomList {
    char		*name;
    int			len;
    int			hash;
    Atom		atom;
} AtomListRec, *AtomListPtr;

static AtomListPtr  *hashTable;

static int	    hashSize, hashUsed;
static int	    hashMask;
static int	    rehash;

static AtomListPtr  *reverseMap;
static int	    reverseMapSize;
static Atom	    lastAtom;

static
Hash(string, len)
    char    *string;
{
    int	h;

    h = 0;
    while (len--)
	h = (h << 3) ^ *string++;
    if (h < 0)
	return -h;
    return h;
}

static
ResizeHashTable ()
{
    int		newHashSize;
    int		newHashMask;
    AtomListPtr	*newHashTable;
    int		i;
    int		h;
    int		newRehash;
    int		r;

    if (hashSize == 0)
	newHashSize = 1024;
    else
	newHashSize = hashSize * 2;
    newHashTable = (AtomListPtr *) xalloc (newHashSize * sizeof (AtomListPtr));
    if (!newHashTable)
	return FALSE;
    bzero ((char *) newHashTable, newHashSize * sizeof (AtomListPtr));
    newHashMask = newHashSize - 1;
    newRehash = (newHashMask - 2);
    for (i = 0; i < hashSize; i++)
    {
	if (hashTable[i])
	{
	    h = (hashTable[i]->hash) & newHashMask;
	    if (newHashTable[h])
	    {
		r = hashTable[i]->hash % newRehash | 1;
		do {
		    h += r;
		    if (h >= newHashSize)
			h -= newHashSize;
		} while (newHashTable[h]);
	    }
	    newHashTable[h] = hashTable[i];
	}
    }
    xfree (hashTable);
    hashTable = newHashTable;
    hashSize = newHashSize;
    hashMask = newHashMask;
    rehash = newRehash;
    return TRUE;
}

static
ResizeReverseMap ()
{
    if (reverseMapSize == 0)
	reverseMapSize = 1000;
    else
	reverseMapSize *= 2;
    reverseMap = (AtomListPtr *) xrealloc (reverseMap, reverseMapSize * sizeof (AtomListPtr));
    if (!reverseMap)
	return FALSE;
}

static
NameEqual (a, b, l)
    char    *a, *b;
{
    while (l--)
	if (*a++ != *b++)
	    return FALSE;
    return TRUE;
}

Atom 
MakeAtom(string, len, makeit)
    char *string;
    unsigned len;
    int makeit;
{
    AtomListPtr	a;
    int		hash;
    int		h;
    int		r;

    hash = Hash (string, len);
    if (hashTable)
    {
    	h = hash & hashMask;
	if (hashTable[h])
	{
	    if (hashTable[h]->hash == hash && hashTable[h]->len == len &&
	    	NameEqual (hashTable[h]->name, string, len))
	    {
	    	return hashTable[h]->atom;
	    }
	    r = (hash % rehash) | 1;
	    for (;;)
	    {
		h += r;
		if (h >= hashSize)
		    h -= hashSize;
		if (!hashTable[h])
		    break;
		if (hashTable[h]->hash == hash && hashTable[h]->len == len &&
		    NameEqual (hashTable[h]->name, string, len))
		{
		    return hashTable[h]->atom;
		}
	    }
    	}
    }
    if (!makeit)
	return None;
    a = (AtomListPtr) xalloc (sizeof (AtomListRec) + len + 1);
    a->name = (char *) (a + 1);
    a->len = len;
    strncpy (a->name, string, len);
    a->name[len] = '\0';
    a->atom = ++lastAtom;
    a->hash = hash;
    if (hashUsed >= hashSize / 2)
    {
	ResizeHashTable ();
	h = hash & hashMask;
	if (hashTable[h])
	{
	    r = (hash % rehash) | 1;
	    do {
		h += r;
		if (h >= hashSize)
		    h -= hashSize;
	    } while (hashTable[h]);
	}
    }
    hashTable[h] = a;
    hashUsed++;
    if (reverseMapSize <= a->atom)
	ResizeReverseMap();
    reverseMap[a->atom] = a;
    return a->atom;
}

ValidAtom(atom)
    Atom atom;
{
    return (atom != None) && (atom <= lastAtom);
}

char *
NameForAtom(atom)
    Atom atom;
{
    if (atom != None && atom <= lastAtom)
	return reverseMap[atom]->name;
    return 0;
}
