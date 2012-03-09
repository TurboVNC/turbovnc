/* $Xorg: savedir.c,v 1.3 2000/08/17 19:41:53 cpqbld Exp $ */
/* savedir.c -- save the list of files in a directory in a string
   Copyright 1990, 1993 Free Software Foundation, Inc.

   Permission to use, copy, modify, and distribute this program for
   any purpose and without fee is hereby granted, provided that this
   copyright and permission notice appear on all copies, and that
   notice be given that copying and distribution is by permission of
   the Free Software Foundation.  The Free Software Foundation makes
   no representations about the suitability of this software for any
   purpose.  It is provided "as is" without expressed or implied
   warranty.

   (The FSF has modified its usual distribution terms, for this file,
   as a courtesy to the X project.)  */

/* $XFree86: xc/config/util/mkshadow/savedir.c,v 1.2 2001/07/25 15:04:41 dawes Exp $ */

/* Written by David MacKenzie <djm@ai.mit.edu>.
   Modified to use <dirent.h> by default.  Per Bothner <bothner@cygnus.com>. */

#include <sys/types.h>
#if !defined(DIRECT) && !defined(BSD)
#include <dirent.h>
#define NLENGTH(direct) (strlen((direct)->d_name))
#else
#undef dirent
#define dirent direct
#define NLENGTH(direct) ((direct)->d_namlen)
#ifdef BSD
#include <sys/dir.h>
#else
#ifdef SYSNDIR
#include <sys/ndir.h>
#else
#include <ndir.h>
#endif
#endif
#endif

#if defined(VOID_CLOSEDIR) || defined(BSD)
/* Fake a return value. */
#define CLOSEDIR(d) (closedir (d), 0)
#else
#define CLOSEDIR(d) closedir (d)
#endif

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

char *stpcpy ();

/* Return a freshly allocated string containing the filenames
   in directory DIR, separated by '\0' characters;
   the end is marked by two '\0' characters in a row.
   NAME_SIZE is the number of bytes to initially allocate
   for the string; it will be enlarged as needed.
   Return NULL if DIR cannot be opened or if out of memory. */

char *
savedir (dir, name_size)
     char *dir;
     unsigned name_size;
{
  DIR *dirp;
  struct dirent *dp;
  char *name_space;
  char *namep;

  dirp = opendir (dir);
  if (dirp == NULL)
    return NULL;

  name_space = (char *) malloc (name_size);
  if (name_space == NULL)
    {
      closedir (dirp);
      return NULL;
    }
  namep = name_space;

  while ((dp = readdir (dirp)) != NULL)
    {
      /* Skip "." and ".." (some NFS filesystems' directories lack them). */
      if (dp->d_name[0] != '.'
	  || (dp->d_name[1] != '\0'
	      && (dp->d_name[1] != '.' || dp->d_name[2] != '\0')))
	{
	  unsigned size_needed = (namep - name_space) + NLENGTH (dp) + 2;

	  if (size_needed > name_size)
	    {
	      char *new_name_space;

	      while (size_needed > name_size)
		name_size += 1024;

	      new_name_space = realloc (name_space, name_size);
	      if (new_name_space == NULL)
		{
		  closedir (dirp);
		  return NULL;
		}
	      namep += new_name_space - name_space;
	      name_space = new_name_space;
	    }
	  strcpy (namep, dp->d_name);
	  namep += strlen (namep) + 1;
	}
    }
  *namep = '\0';
  if (CLOSEDIR (dirp))
    {
      free (name_space);
      return NULL;
    }
  return name_space;
}
