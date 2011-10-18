/* $XConsortium: AuLock.c,v 1.15 94/04/17 20:15:43 rws Exp $ */
/* $XFree86: xc/lib/Xau/AuLock.c,v 3.0 1994/10/20 06:04:31 dawes Exp $ */

/*

Copyright (c) 1988  X Consortium

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

#include <X11/Xauth.h>
#include <X11/Xos.h>
#include <sys/stat.h>
#include <errno.h>
#if defined(X_NOT_STDC_ENV)
extern int errno;
#define Time_t long
extern Time_t time ();
#else
#include <time.h>
#define Time_t time_t
#endif
#ifndef X_NOT_POSIX
#include <unistd.h>
#else
#ifndef WIN32
extern unsigned	sleep ();
#else
#define link rename
#endif
#endif
#ifdef __EMX__
#define link rename
#endif

#if NeedFunctionPrototypes
int
XauLockAuth (
_Xconst char *file_name,
int	retries,
int	timeout,
long	dead)
#else
int
XauLockAuth (file_name, retries, timeout, dead)
char	*file_name;
int	retries;
int	timeout;
long	dead;
#endif
{
    char	creat_name[1025], link_name[1025];
    struct stat	statb;
    Time_t	now;
    int		creat_fd = -1;

    if (strlen (file_name) > 1022)
	return LOCK_ERROR;
    (void) strcpy (creat_name, file_name);
    (void) strcat (creat_name, "-c");
    (void) strcpy (link_name, file_name);
    (void) strcat (link_name, "-l");
    if (stat (creat_name, &statb) != -1) {
	now = time ((Time_t *) 0);
	/*
	 * NFS may cause ctime to be before now, special
	 * case a 0 deadtime to force lock removal
	 */
	if (dead == 0 || now - statb.st_ctime > dead) {
	    (void) unlink (creat_name);
	    (void) unlink (link_name);
	}
    }
    
    while (retries > 0) {
	if (creat_fd == -1) {
	    creat_fd = creat (creat_name, 0666);
	    if (creat_fd == -1) {
		if (errno != EACCES)
		    return LOCK_ERROR;
	    } else
		(void) close (creat_fd);
	}
	if (creat_fd != -1) {
	    if (link (creat_name, link_name) != -1)
		return LOCK_SUCCESS;
	    if (errno == ENOENT) {
		creat_fd = -1;	/* force re-creat next time around */
		continue;
	    }
	    if (errno != EEXIST)
		return LOCK_ERROR;
	}
	(void) sleep ((unsigned) timeout);
	--retries;
    }
    return LOCK_TIMEOUT;
}
