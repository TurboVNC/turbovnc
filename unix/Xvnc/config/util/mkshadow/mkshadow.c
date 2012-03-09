/* $Xorg: mkshadow.c,v 1.3 2000/08/17 19:41:53 cpqbld Exp $ */
/* mkshadow.c - make a "shadow copy" of a directory tree with symlinks.
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

/*
 * Usage: mkshadow [-X exclude_file] [-x exclude_pattern] ... MASTER [SHADOW]
 * Makes SHADOW be a "shadow copy" of MASTER.  SHADOW defaults to the current
 * directory. Sort of like a recursive copy of MASTER to SHADOW.
 * However, symbolic links are used instead of actually
 * copying (non-directory) files.
 * Also, directories named RCS or SCCS are shared (with a symbolic link).
 * Warning messages are printed for files (and directories) in .
 * that don't match a corresponding file in MASTER (though
 * symbolic links are silently removed).
 * Also, a warning message is printed for non-directory files
 * under SHADOW that are not symbolic links.
 *
 * Files and directories can be excluded from the sharing
 * with the -X and -x flags. The flag `-x pattern' (or `-xpattern')
 * means that mkshadow should ignore any file whose name matches
 * the pattern. The pattern is a "globbing" pattern, i.e. the
 * characters *?[^-] are interpreted as by the shell.
 * If the pattern contains a '/' is is matched against the complete
 * current path (relative to '.'); otherwise, it is matched
 * against the last component of the path.
 * A `-X filename' flag means to read a set of exclusion patterns
 * from the named file, one pattern to a line.
 *
 * Originally written by Per Bothner at University of Wisconsin-Madison,
 * inspired by the lndir script distributed with X11.
 * Modified by Per Bothner <bothner@cygnus.com> November 1993
 * to more-or-less follow Posix.
 */

#include <sys/types.h>
#include <stdio.h>
#ifdef BSD
#include <strings.h>
#define strchr index
#else
#include <string.h>
#endif
#include <sys/stat.h>
#if defined(S_IFDIR) && !defined(S_ISDIR)
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif
#if defined(S_IFLNK) && !defined(S_ISLNK)
#define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)
#endif
#ifndef S_ISLNK
#define lstat stat
#endif
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
#include <errno.h>
#ifndef errno
extern int errno;
#endif

extern char * savedir();

fatal(msg)
     char *msg;
{
    if (errno) perror(msg ? msg : "");
    else if (msg) fprintf(stderr, "mkshadow: %s\n", msg);
    exit(-1);
}

/* When handling symbolic links to relative directories,
 * we need to prepend "../" to the "source".
 * We preallocate MAX_DEPTH repetations of "../" using a simple trick.
 */
#define MAX_DEPTH 20
#define PREPEND_BUFFER_SIZE (MAX_DEPTH*3)
char master_buffer[MAXPATHLEN+PREPEND_BUFFER_SIZE] =
    "../../../../../../../../../../../../../../../../../../../../";
/* The logical start of the master_buffer is defined by
 * master_start, which skips the fixed prepend area.
 */
#define master_start (master_buffer+PREPEND_BUFFER_SIZE)
char shadow_buffer[MAXPATHLEN];

void bad_args(msg)
{
    if (msg) fprintf(stderr, "%s\n", msg);
    fprintf (stderr, "usage: mkshadow [-X exclude_file] [-x exclude_pattern]");
    fprintf (stderr, " master [shadow]\n");
    exit(-1);
}

int exclude_count = 0;
char **exclude_patterns = NULL;
int exclude_limit = 0;

void add_exclude(pattern)
    char *pattern;
{
    if (exclude_limit == 0) {
	exclude_limit = 100;
	exclude_patterns = (char**)malloc(exclude_limit * sizeof(char*));
    } else if (exclude_count + 1 >= exclude_limit) {
	exclude_limit += 100;
	exclude_patterns = (char**)realloc(exclude_patterns, 
					   exclude_limit * sizeof(char*));
    }
    exclude_patterns[exclude_count] = pattern;
    exclude_count++;
}

void add_exclude_file(name)
     char *name;
{
    char buf[MAXPATHLEN];
    FILE *file = fopen(name, "r");
    if (file == NULL) fatal("failed to find -X (exclude) file");
    for (;;) {
	int len;
	char *str = fgets(buf, MAXPATHLEN, file);
	if (str == NULL) break;
	len = strlen(str);
	if (len && str[len-1] == '\n') str[--len] = 0;
	if (!len) continue;
	str = (char*)malloc(len+1);
	strcpy(str, buf);
	add_exclude(str);
    }
    fclose(file);
}

main(argc, argv)
     char **argv;
{
    char *master_name = NULL;
    char *shadow_name = NULL;
    int i;
    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-') {
	    switch(argv[i][1]) {
	      case 'X':
		if (argv[i][2]) add_exclude_file(&argv[i][2]);
		else if (++i >= argc) bad_args(NULL);
		else add_exclude_file(argv[i]);
		break;
	      case 'x':
		if (argv[i][2]) add_exclude(&argv[i][2]);
		else if (++i >= argc) bad_args(NULL);
		else add_exclude(argv[i]);
		break;
	      default:
		bad_args(NULL);
	    }
	} else if (master_name == NULL)
	    master_name = argv[i];
	else if (shadow_name == NULL)
	    shadow_name = argv[i];
	else bad_args (NULL);
    }

    if (master_name == NULL) bad_args(NULL);
    if (shadow_name == NULL)
	shadow_name = ".";
    else if ((shadow_name[0] != '.' || shadow_name[1])
	     && master_name[0] != '/') {
	fprintf(stderr, "Shadowing a relative directory pathname to a \n");
	fprintf(stderr, "shadow other than '.' is not supported!\n");
	exit(-1);
    }
    strcpy(shadow_buffer, shadow_name);
    strcpy(master_start, master_name);
    DoCopy(master_start, shadow_buffer, 0);
    return 0;
}

int compare_strings(ptr1, ptr2)
     char **ptr1, **ptr2;
{
    return strcmp(*ptr1, *ptr2);
}

void MakeLink(master, current, depth)
     char *master;
     char *current;
     int depth;
{
    if (master[0] != '/') {
	/* Source directory was specified with a relative pathname. */
	if (master != master_start) {
	    fatal("Internal bug: bad string buffer use");
	}
	/* Pre-pend "../" depth times. This compensates for
	 * the directories we've entered. */
	master -= 3 * depth;
    }
    if (symlink(master, current)) {
	fprintf(stderr, "Failed to create symbolic link %s->%s\n",
		current, master);
	exit (-1);
    }
}


/* Get a sorted NULL_terminator array of (char*) using 'names'
 * (created by save_dir) as data.
 */
char ** get_name_pointers(names)
     char *names;
{
    int n_names = 0;
    int names_buf_size = 64;
    char *namep;
    char ** pointers = (char**)malloc(names_buf_size * sizeof(char*));
    if (!names || !pointers) fatal("virtual memory exhausted");

    for (namep = names; *namep; namep += strlen(namep) + 1) {
	if (n_names + 1 >= names_buf_size) {
	    names_buf_size *= 2;
	    pointers = (char**)realloc(pointers,
				       names_buf_size * sizeof(char*));
	    if (!pointers) fatal("virtual memory exhausted");
	}
	pointers[n_names++] = namep;
    }
    pointers[n_names] = 0;
    qsort(pointers, n_names, sizeof(char*), compare_strings);
    return pointers;
}

/* Recursively shadow the directory whose name is in MASTER
 * (which is == MASTER_START) into the destination directory named CURRENT.
 */

DoCopy(master, current, depth)
     char *master; /* The source directory. */
     char *current; /* The destination directory. */
     int depth;
{
    struct stat stat_master, stat_current;
    char **master_pointer, **current_pointer;
    char **master_names, **current_names;
    char *master_end, *current_end;
    char *master_name_buf, *current_name_buf;
    master_end = master + strlen(master);
    current_end = current + strlen(current);

    /* Get rid of terminal '/' */
    if (master_end[-1] == '/' && master != master_end - 1)
	*--master_end = 0;
    if (current_end[-1] == '/' && current != current_end - 1)
	*--current_end = 0;

    if (depth >= MAX_DEPTH) {
	fprintf(stderr,
		"Nesting too deep (depth %d at %s). Probable circularity.\n",
		depth, master);
	exit(-1);
    }

    master_name_buf = savedir(master, 500);
    if (master_name_buf == NULL) {
	fprintf(stderr, "Not enough memory or no such directory: %s\n",
		master);
	exit(-1);
    }
    current_name_buf = savedir(current, 500);
    if (current_name_buf == NULL) {
	fprintf(stderr, "Not enough memory or no such directory: %s\n",
		current);
	exit(-1);
    }

    master_names = get_name_pointers(master_name_buf);
    current_names = get_name_pointers(current_name_buf);

    master_pointer = master_names;
    current_pointer = current_names;
    for (;;) {
	int cmp, ipat;
	int in_master, in_current;
	char *cur_name;
	if (*master_pointer == NULL && *current_pointer == NULL)
	    break;
	if (*master_pointer == NULL) cmp = 1;
	else if (*current_pointer == NULL) cmp = -1;
	else cmp = strcmp(*master_pointer, *current_pointer);
	if (cmp < 0) { /* file only exists in master directory */
	    in_master = 1; in_current = 0;
	} else if (cmp == 0) { /* file exists in both directories */
	    in_master = 1; in_current = 1;
	} else { /* file only exists in current directory */
	    in_current = 1; in_master = 0;
	}
	cur_name = in_master ? *master_pointer : *current_pointer;
	sprintf(master_end, "/%s", cur_name);
	sprintf(current_end, "/%s", cur_name);
	for (ipat = 0; ipat < exclude_count; ipat++) {
	    char *pat = exclude_patterns[ipat];
	    char *cur;
	    if (strchr(pat, '/')) cur = current + 2; /* Skip initial "./" */
	    else cur = cur_name;
	    if (wildmat(cur, pat)) goto skip;
	}
	if (in_master)
	    if (lstat(master, &stat_master) != 0) fatal("stat failed");
	if (in_current)
	    if (lstat(current, &stat_current) != 0) fatal("stat failed");
	if (in_current && !in_master) {
	    if (S_ISLNK(stat_current.st_mode))
		if (unlink(current)) {
		    fprintf(stderr, "Failed to remove symbolic link %s.\n",
			    current);
		}
		else
		    fprintf(stderr, "Removed symbolic link %s.\n",
			    current);
	    else {
		fprintf(stderr,
			"The file %s does not exist in the master tree.\n",
			current);
	    }
	}
	else if (S_ISDIR(stat_master.st_mode)
		 && strcmp(cur_name, "RCS") != 0
		 && strcmp(cur_name, "SCCS") != 0) {
	    if (!in_current) {
		if (mkdir(current, 0775)) fatal("mkdir failed");
	    }
	    else if (stat(current, &stat_current)) fatal("stat failed");
	    if (!in_current || stat_current.st_dev != stat_master.st_dev
		|| stat_current.st_ino != stat_master.st_ino)
		DoCopy(master, current, depth+1);
	    else
		fprintf(stderr, "Link %s is the same as directory %s.\n",
			current, master);
	}
	else {
	    if (!in_current)
		MakeLink(master, current, depth);
	    else if (!S_ISLNK(stat_current.st_mode)) {
		fprintf(stderr, "Existing file %s is not a symbolc link.\n",
			current);
	    } else {
		if (stat(current, &stat_current) || stat(master, &stat_master))
		    fatal("stat failed");
		if (stat_current.st_dev != stat_master.st_dev
		    || stat_current.st_ino != stat_master.st_ino) {
		    fprintf(stderr, "Fixing incorrect symbolic link %s.\n",
			    current);
		    if (unlink(current)) {
			fprintf(stderr, "Failed to remove symbolic link %s.\n",
				current);
		    }
		    else
			MakeLink(master, current, depth);
		}
	    }
	}
      skip:
	if (in_master) master_pointer++;
	if (in_current) current_pointer++;
    }

    free(master_names); free(current_names);
    free(master_name_buf); free(current_name_buf);
}
