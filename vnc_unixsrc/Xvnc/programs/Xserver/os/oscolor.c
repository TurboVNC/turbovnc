/***********************************************************

Copyright (c) 1987  X Consortium

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: oscolor.c,v 1.23 94/04/17 20:27:04 dpw Exp $ */
/* $XFree86: xc/programs/Xserver/os/oscolor.c,v 3.2.4.1 1998/01/22 10:47:14 dawes Exp $ */

#ifndef USE_RGB_TXT

#ifdef NDBM
#include <ndbm.h>
#else
#ifdef SVR4
#include <rpcsvc/dbm.h>
#else
#include <dbm.h>
#endif
#endif
#include "rgb.h"
#include "os.h"
#include "opaque.h"

/* Note that we are assuming there is only one database for all the screens. */

#ifdef NDBM
DBM *rgb_dbm = (DBM *)NULL;
#else
int rgb_dbm = 0;
#endif

extern void CopyISOLatin1Lowered();

int
OsInitColors()
{
    if (!rgb_dbm)
    {
#ifdef NDBM
	rgb_dbm = dbm_open(rgbPath, 0, 0);
#else
	if (dbminit(rgbPath) == 0)
	    rgb_dbm = 1;
#endif
	if (!rgb_dbm) {
	    ErrorF( "Couldn't open RGB_DB '%s'\n", rgbPath );
	    return FALSE;
	}
    }
    return TRUE;
}

/*ARGSUSED*/
int
OsLookupColor(screen, name, len, pred, pgreen, pblue)
    int		screen;
    char	*name;
    unsigned	len;
    unsigned short	*pred, *pgreen, *pblue;

{
    datum		dbent;
    RGB			rgb;
    char		buf[64];
    char		*lowername;

    if(!rgb_dbm)
	return(0);

    /* we use xalloc here so that we can compile with cc without alloca
     * when otherwise using gcc */
    if (len < sizeof(buf))
	lowername = buf;
    else if (!(lowername = (char *)xalloc(len + 1)))
	return(0);
    CopyISOLatin1Lowered ((unsigned char *) lowername, (unsigned char *) name,
			  (int)len);

    dbent.dptr = lowername;
    dbent.dsize = len;
#ifdef NDBM
    dbent = dbm_fetch(rgb_dbm, dbent);
#else
    dbent = fetch (dbent);
#endif

    if (len >= sizeof(buf))
	xfree(lowername);

    if(dbent.dptr)
    {
	memmove((char *) &rgb, dbent.dptr, sizeof (RGB));
	*pred = rgb.red;
	*pgreen = rgb.green;
	*pblue = rgb.blue;
	return (1);
    }
    return(0);
}

#else /* USE_RGB_TXT */


/*
 * The dbm routines are a porting hassle. This implementation will do
 * the same thing by reading the rgb.txt file directly, which is much
 * more portable.
 */

#include <stdio.h>
#include "os.h"
#include "opaque.h"

#define HASHSIZE 511

typedef struct _dbEntry * dbEntryPtr;
typedef struct _dbEntry {
  dbEntryPtr     link;
  unsigned short red;
  unsigned short green;
  unsigned short blue;
  char           name[1];	/* some compilers complain if [0] */
} dbEntry;


extern void CopyISOLatin1Lowered();

static dbEntryPtr hashTab[HASHSIZE];


static dbEntryPtr
lookup(name, len, create)
     char *name;
     int  len;
     Bool create;
{
  unsigned int h = 0, g;
  dbEntryPtr   entry, *prev;
  char         *str = name;

  if (!(name = (char*)ALLOCATE_LOCAL(len +1))) return NULL;
  CopyISOLatin1Lowered(name, str, len);
  name[len] = '\0';

  for(str = name; *str; str++) {
    h = (h << 4) + *str;
    if ((g = h) & 0xf0000000) h ^= (g >> 24);
    h &= g;
  }
  h %= HASHSIZE;

  if ( entry = hashTab[h] )
    {
      for( ; entry; prev = (dbEntryPtr*)entry, entry = entry->link )
	if (! strcmp(name, entry->name) ) break;
    }
  else
    prev = &(hashTab[h]);

  if (!entry && create && (entry = (dbEntryPtr)xalloc(sizeof(dbEntry) +len)))
    {
      *prev = entry;
      entry->link = NULL;
      strcpy( entry->name, name );
    }

  DEALLOCATE_LOCAL(name);

  return entry;
}


Bool
OsInitColors()
{
  FILE       *rgb;
  char       *path;
  char       line[BUFSIZ];
  char       name[BUFSIZ];
  int        red, green, blue, lineno = 0;
  dbEntryPtr entry;

  static Bool was_here = FALSE;

  if (!was_here)
    {
#ifndef __EMX__
      path = (char*)ALLOCATE_LOCAL(strlen(rgbPath) +5);
      strcpy(path, rgbPath);
      strcat(path, ".txt");
#else
      char *tmp = (char*)__XOS2RedirRoot(rgbPath);
      path = (char*)ALLOCATE_LOCAL(strlen(tmp) +5);
      strcpy(path, tmp);
      strcat(path, ".txt");
#endif
      if (!(rgb = fopen(path, "r")))
        {
	   ErrorF( "Couldn't open RGB_DB '%s'\n", rgbPath );
	   DEALLOCATE_LOCAL(path);
	   return FALSE;
	}

      while(fgets(line, sizeof(line), rgb))
	{
	  lineno++;
#ifndef __EMX__
	  if (sscanf(line,"%d %d %d %[^\n]\n", &red, &green, &blue, name) == 4)
#else
	  if (sscanf(line,"%d %d %d %[^\n\r]\n", &red, &green, &blue, name) == 4)
#endif
	    {
	      if (red >= 0   && red <= 0xff &&
		  green >= 0 && green <= 0xff &&
		  blue >= 0  && blue <= 0xff)
		{
		  if (entry = lookup(name, strlen(name), TRUE))
		    {
		      entry->red   = (red * 65535)   / 255;
		      entry->green = (green * 65535) / 255;
		      entry->blue  = (blue  * 65535) / 255;
		    }
		}
	      else
		ErrorF("Value out of range: %s:%d\n", path, lineno);
	    }
	  else if (*line && *line != '#' && *line != '!')
	    ErrorF("Syntax Error: %s:%d\n", path, lineno);
	}
      
      fclose(rgb);
      DEALLOCATE_LOCAL(path);

      was_here = TRUE;
    }

  return TRUE;
}



Bool
OsLookupColor(screen, name, len, pred, pgreen, pblue)
    int		   screen;
    char	   *name;
    unsigned	   len;
    unsigned short *pred, *pgreen, *pblue;

{
  dbEntryPtr entry;

  if (entry = lookup(name, len, FALSE))
    {
      *pred   = entry->red;
      *pgreen = entry->green;
      *pblue  = entry->blue;
      return TRUE;
    }

  return FALSE;
}

#endif /* USE_RGB_TXT */
