/* $Xorg: checktree.c,v 1.4 2001/02/09 02:03:16 xorgcvs Exp $ */

/*

Copyright (c) 1993, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
*/
/* $XFree86: xc/config/util/checktree.c,v 1.4 2001/12/14 19:53:22 dawes Exp $ */

#include <X11/Xos.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>

#ifndef X_NOT_POSIX
#include <dirent.h>
#else
#ifdef SYSV
#include <dirent.h>
#else
#ifdef USG
#include <dirent.h>
#else
#include <sys/dir.h>
#ifndef dirent
#define dirent direct
#endif
#endif
#endif
#endif

#ifdef S_IFLNK
#define Stat lstat
#else
#define Stat stat
#endif

#define CHARSALLOWED \
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_."

#define fmode_bits_minset 0444
#define fmode_bits_maxset 0777
#define fmode_bits_write  0222
#define dmode_bits_minset 0775

int dorcs = 1;			/* check RCS file */
int do83 = 1;			/* check for 8+3 clash */
int doro = 1;			/* disallow writable (checked out) files */
int dodot = 1;			/* disallow .files */
int dotwiddle = 1;		/* disallow file~ */

int dontcare(fn)
    char *fn;
{
    char *cp;

    if (fn[strlen(fn) - 1] == '~')
	return 1;
    cp = strrchr(fn, '.');
    return cp && (!strcmp(cp + 1, "Z") || !strcmp(cp + 1, "PS"));
}

checkfile(fullname, fn, fs)
    char *fullname, *fn;
    struct stat *fs;
{
    char *cp;
    int maxlen = 12;
    int len, mode;

    if (dodot && fn[0] == '.') {
	printf("dot file: %s\n", fullname);
	return;
    }
    for (len = 0, cp = fn; *cp; len++, cp++) {
	if (!strchr(CHARSALLOWED, *cp)) {
	    if (dotwiddle || *cp != '~' || cp[1])
		printf ("bad character: %s\n", fullname);
	    break;
	}
    }
    if (len > maxlen && !dontcare(fn))
	printf("too long (%d): %s\n", len, fullname);
#ifdef S_IFLNK
    if ((fs->st_mode & S_IFLNK) == S_IFLNK) {
	printf("symbolic link: %s\n", fullname);
	return;
    }
#endif
    mode = fs->st_mode & (~S_IFMT);
    if ((fs->st_mode & S_IFDIR) == S_IFDIR) {
	maxlen = 14;
	if ((mode & dmode_bits_minset) != dmode_bits_minset)
	    printf("directory mode 0%o not minimum 0%o: %s\n",
		   mode, dmode_bits_minset, fullname);
    } else if ((fs->st_mode & S_IFREG) != S_IFREG)
	printf("not a regular file: %s\n", fullname);
    else {
	if ((mode & fmode_bits_minset) != fmode_bits_minset)
	    printf("file mode 0%o not minimum 0%o: %s\n",
		   fs->st_mode, fmode_bits_minset, fullname);
	if (fs->st_nlink != 1)
	    printf("%d links instead of 1: %s\n", fs->st_nlink, fullname);
	if (doro && (mode & fmode_bits_write) && !dontcare(fn))
	    printf("writable: %s\n", fullname);
    }
    if ((mode & ~fmode_bits_maxset) != 0)
	printf("mode 0%o outside maximum set 0%o: %s\n",
	       mode, fmode_bits_maxset, fullname);
}

void
checkrcs(dir, p)
    char *dir;
    char *p;
{
    DIR *df;
    struct dirent *dp;
    struct stat fs;
    int i;

    if (!(df = opendir(dir))) {
	fprintf(stderr, "cannot open: %s\n", dir);
	return;
    }
    while (dp = readdir(df)) {
	i = strlen(dp->d_name);
	if (dp->d_name[i - 1] == 'v' && dp->d_name[i - 2] == ',') {
	    strcpy(p, dp->d_name);
	    p[i - 2] = '\0';
	    if (Stat(dir, &fs) < 0) {
		strcpy(p, "RCS/");
		strcat(p, dp->d_name);
		printf("not used: %s\n", dir);
	    }
	}
    }
    closedir(df);
}

int
Strncmp(cp1, cp2, n)
    char *cp1, *cp2;
    int n;
{
    char c1, c2;

    for (; --n >= 0 && *cp1 && *cp2; cp1++, cp2++) {
	if (*cp1 != *cp2) {
	    c1 = *cp1;
	    c2 = *cp2;
	    if (c1 >= 'A' && c1 <= 'Z')
		c1 += 'a' - 'A';
	    else if (c1 == '-')
		c1 = '_';
	    if (c2 >= 'A' && c2 <= 'Z')
		c2 += 'a' - 'A';
	    else if (c2 == '-')
		c2 = '_';
	    if (c1 != c2)
		return (int)c1 - (int)c2;
	}
    }
    if (n < 0)
	return 0;
    return (int)*cp1 - (int)*cp2;
}

int
fncomp(n1, n2)
    char **n1, **n2;
{
    int i, res;
    char *cp1, *cp2;
    char c1, c2;

    i = Strncmp(*n1, *n2, 8);
    if (!i) {
	cp1 = strrchr(*n1, '.');
	cp2 = strrchr(*n2, '.');
	if (cp1 || cp2) {
	    if (!cp1)
		return -1;
	    if (!cp2)
		return 1;
	    i = Strncmp(cp1 + 1, cp2 + 1, 3);
	}
    }
    return i;
}

void
checkdir(dir)
    char *dir;
{
    DIR *df;
    struct dirent *dp;
    char *p;
    struct stat fs;
    char *s, **names;
    int i, max;

    if (!(df = opendir(dir))) {
	fprintf(stderr, "cannot open: %s\n", dir);
	return;
    }
    p = dir + strlen(dir);
    if (p[-1] != '/')
	*p++ = '/';
    i = 0;
    max = 0;
    names = NULL;
    while (dp = readdir(df)) {
	strcpy(p, dp->d_name);
	if (Stat(dir, &fs) < 0) {
	    perror(dir);
	    continue;
	}
	if ((fs.st_mode & S_IFDIR) == S_IFDIR) {
	    if (dp->d_name[0] == '.' &&
		(dp->d_name[1] == '\0' || (dp->d_name[1] == '.' &&
					   dp->d_name[2] == '\0')))
		continue;
	    if (!strcmp (dp->d_name, "RCS")) {
		if (dorcs)
		    checkrcs(dir, p);
		continue;
	    }
	    if (!strcmp (dp->d_name, "SCCS"))
		continue;
	    if (!strcmp (dp->d_name, "CVS.adm"))
		continue;
	    checkfile(dir, p, &fs);
	    checkdir(dir);
	    continue;
	}
	checkfile(dir, p, &fs);
	if (dorcs && !dontcare(dp->d_name)) {
	    strcpy(p, "RCS/");
	    strcat(p, dp->d_name);
	    strcat(p, ",v");
	    if (Stat(dir, &fs) < 0) {
		strcpy(p, dp->d_name);
		printf("no RCS: %s\n", dir);
	    }
	}
	if (do83) {
	    s = (char *)malloc(strlen(dp->d_name) + 1);
	    strcpy(s, dp->d_name);
	    if (i >= max) {
		max += 25;
		if (names)
		    names = (char **)realloc((char *)names,
					     (max + 1) * sizeof(char *));
		else
		    names = (char **)malloc((max + 1) * sizeof(char *));
	    }
	    names[i++] = s;
	}
    }
    closedir(df);
    if (do83) {
	qsort((char *)names, i, sizeof(char *), fncomp);
	max = i - 1;
	*p = '\0';
	for (i = 0; i < max; i++) {
	    if (!fncomp(&names[i], &names[i + 1]))
		printf("8+3 clash: %s%s and %s\n",
		       dir, names[i], names[i + 1]);
	    free(names[i]);
	}
	if (names) {
	    free(names[i]);
	    free((char *)names);
	}
    }
}

main(argc, argv)
    int argc;
    char **argv;
{
    char buf[2048];

    argc--;
    argv++;
    while (argc > 0) {
	if (!strcmp(*argv, "-rcs")) {
	    dorcs = 0;
	    argc--;
	    argv++;
	} else if (!strcmp(*argv, "-83")) {
	    do83 = 0;
	    argc--;
	    argv++;
	} else if (!strcmp(*argv, "-ro")) {
	    doro = 0;
	    argc--;
	    argv++;
	} else if (!strcmp(*argv, "-dot")) {
	    dodot = 0;
	    argc--;
	    argv++;
	} else if (!strcmp(*argv, "-twiddle")) {
	    dotwiddle = 0;
	    argc--;
	    argv++;
	} else
	    break;
    }
    if (!argc) {
	strcpy(buf, ".");
	checkdir(buf);
    } else
	while (--argc >= 0) {
	    strcpy(buf, *argv++);
	    checkdir(buf);
	}
}
