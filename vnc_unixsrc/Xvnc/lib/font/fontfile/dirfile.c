/* $XFree86: xc/lib/font/fontfile/dirfile.c,v 3.3 1997/01/27 06:56:28 dawes Exp $ */
#ifndef lint
static char *rid=
    "$XConsortium: dirfile.c /main/12 1995/12/08 19:02:23 gildea $";
#endif /* lint */

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

/*
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * dirfile.c
 *
 * Read fonts.dir and fonts.alias files
 */

#include "fntfilst.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef X_NOT_STDC_ENV
extern int errno;
#endif

static int ReadFontAlias();

int
FontFileReadDirectory (directory, pdir)
    char		*directory;
    FontDirectoryPtr	*pdir;
{
    char        file_name[MAXFONTNAMELEN];
    char        font_name[MAXFONTNAMELEN];
    char        dir_file[MAXFONTNAMELEN];
#ifdef FONTDIRATTRIB
    char	dir_path[MAXFONTNAMELEN];
    char	*ptr;
#endif
    FILE       *file;
    int         count,
                i,
                status;
    struct stat	statb;

    FontDirectoryPtr	dir = NullFontDirectory;

#ifdef FONTDIRATTRIB
    /* Check for font directory attributes */
#ifndef __EMX__
    if (ptr = strchr(directory, ':')) {
#else
    /* OS/2 path might start with a drive letter, don't clip this */
    if (ptr = strchr(directory+2, ':')) {
#endif
	strncpy(dir_path, directory, ptr - directory);
	dir_path[ptr - directory] = '\0';
    } else {
	strcpy(dir_path, directory);
    }
    strcpy(dir_file, dir_path);
#else
    strcpy(dir_file, directory);
#endif
    if (dir_file[strlen(dir_file) - 1] != '/')
	strcat(dir_file, "/");
    strcat(dir_file, FontDirFile);
    file = fopen(dir_file, "r");
    if (file) {
	if (fstat (fileno(file), &statb) == -1)
	    return BadFontPath;
	count = fscanf(file, "%d\n", &i);
	if ((count == EOF) || (count != 1)) {
	    fclose(file);
	    return BadFontPath;
	}
	dir = FontFileMakeDir(directory, i);
	if (dir == NULL) {
	    fclose(file);
	    return BadFontPath;
	}
	dir->dir_mtime = statb.st_mtime;
	while ((count = fscanf(file, "%s %[^\n]\n", file_name, font_name)) != EOF) {
#ifdef __EMX__
	    /* strip any existing trailing CR */
	    for (i=0; i<strlen(font_name); i++) {
		if (font_name[i]=='\r') font_name[i] = '\0';
	    }
#endif
	    if (count != 2) {
		FontFileFreeDir (dir);
		fclose(file);
		return BadFontPath;
	    }
	    if (!FontFileAddFontFile (dir, font_name, file_name))
	    {
		FontFileFreeDir (dir);
		fclose(file);
		return BadFontPath;
	    }
	}
	fclose(file);
    } else if (errno != ENOENT) {
	return BadFontPath;
    }
#ifdef FONTDIRATTRIB
    status = ReadFontAlias(dir_path, FALSE, &dir);
#else
    status = ReadFontAlias(directory, FALSE, &dir);
#endif
    if (status != Successful) {
	if (dir)
	    FontFileFreeDir (dir);
	return status;
    }
    if (!dir) {
        /*** TJR - dirty hack - this variable is set in
	     programs/Xserver/dix/dixfont.c when setting default font path  */
	extern int settingDefaultFontPath;
	if (settingDefaultFontPath) {
	    fprintf(stderr,"Font directory '%s' not found - ignoring\n",
		    directory);
	    dir = FontFileMakeDir(directory, 0);
	}

	if (!dir)
	    return BadFontPath;
    }

    FontFileSortDir(dir);

    *pdir = dir;
    return Successful;
}

Bool
FontFileDirectoryChanged(dir)
    FontDirectoryPtr	dir;
{
    char	dir_file[MAXFONTNAMELEN];
    struct stat	statb;

    strcpy (dir_file, dir->directory);
    strcat (dir_file, FontDirFile);
    if (stat (dir_file, &statb) == -1)
    {
	if (errno != ENOENT || dir->dir_mtime != 0)
	    return TRUE;
	return FALSE;		/* doesn't exist and never did: no change */
    }
    if (dir->dir_mtime != statb.st_mtime)
	return TRUE;
    strcpy (dir_file, dir->directory);
    strcat (dir_file, FontAliasFile);
    if (stat (dir_file, &statb) == -1)
    {
	if (errno != ENOENT || dir->alias_mtime != 0)
	    return TRUE;
	return FALSE;		/* doesn't exist and never did: no change */
    }
    if (dir->alias_mtime != statb.st_mtime)
	return TRUE;
    return FALSE;
}
    
/*
 * Make each of the file names an automatic alias for each of the files.
 */

static Bool
AddFileNameAliases(dir)
    FontDirectoryPtr	dir;
{
    int		    i;
    char	    copy[MAXFONTNAMELEN];
    char	    *fileName;
    FontTablePtr    table;
    FontRendererPtr renderer;
    int		    len;
    FontNameRec	    name;

    table = &dir->nonScalable;
    for (i = 0; i < table->used; i++) {
	if (table->entries[i].type != FONT_ENTRY_BITMAP)
	    continue;
	fileName = table->entries[i].u.bitmap.fileName;
	renderer = FontFileMatchRenderer (fileName);
	if (!renderer)
	    continue;
	
	len = strlen (fileName) - renderer->fileSuffixLen;
	CopyISOLatin1Lowered (copy, fileName, len);
	copy[len] = '\0';
	name.name = copy;
	name.length = len;
	name.ndashes = FontFileCountDashes (copy, len);

	if (!FontFileFindNameInDir(table, &name)) {
	    if (!FontFileAddFontAlias (dir, copy, table->entries[i].name.name))
		return FALSE;
	}
    }
    return TRUE;
}

/*
 * parse the font.alias file.  Format is:
 *
 * alias font-name
 *
 * To imbed white-space in an alias name, enclose it like "font name"
 * in double quotes.  \ escapes and character, so
 * "font name \"With Double Quotes\" \\ and \\ back-slashes"
 * works just fine.
 *
 * A line beginning with a ! denotes a newline-terminated comment.
 */

/*
 * token types
 */

static int  lexAlias(), lexc();

#define NAME		0
#define NEWLINE		1
#define DONE		2
#define EALLOC		3

static int
ReadFontAlias(directory, isFile, pdir)
    char		*directory;
    Bool		isFile;
    FontDirectoryPtr	*pdir;
{
    char		alias[MAXFONTNAMELEN];
    char		font_name[MAXFONTNAMELEN];
    char		alias_file[MAXFONTNAMELEN];
    FILE		*file;
    FontDirectoryPtr	dir;
    int			token;
    char		*lexToken;
    int			status = Successful;
    struct stat		statb;

    dir = *pdir;
    strcpy(alias_file, directory);
    if (!isFile) {
	if (directory[strlen(directory) - 1] != '/')
	    strcat(alias_file, "/");
	strcat(alias_file, FontAliasFile);
    }
    file = fopen(alias_file, "r");
    if (!file)
	return ((errno == ENOENT) ? Successful : BadFontPath);
    if (!dir)
	*pdir = dir = FontFileMakeDir(directory, 10);
    if (!dir)
    {
	fclose (file);
	return AllocError;
    }
    if (fstat (fileno (file), &statb) == -1)
    {
	fclose (file);
	return BadFontPath;
    }
    dir->alias_mtime = statb.st_mtime;
    while (status == Successful) {
	token = lexAlias(file, &lexToken);
	switch (token) {
	case NEWLINE:
	    break;
	case DONE:
	    fclose(file);
	    return Successful;
	case EALLOC:
	    status = AllocError;
	    break;
	case NAME:
	    strcpy(alias, lexToken);
	    token = lexAlias(file, &lexToken);
	    switch (token) {
	    case NEWLINE:
		if (strcmp(alias, "FILE_NAMES_ALIASES"))
		    status = BadFontPath;
		else if (!AddFileNameAliases(dir))
		    status = AllocError;
		break;
	    case DONE:
		status = BadFontPath;
		break;
	    case EALLOC:
		status = AllocError;
		break;
	    case NAME:
		CopyISOLatin1Lowered((unsigned char *) alias,
				     (unsigned char *) alias,
				     strlen(alias));
		CopyISOLatin1Lowered((unsigned char *) font_name,
				     (unsigned char *) lexToken,
				     strlen(lexToken));
		if (!FontFileAddFontAlias (dir, alias, font_name))
		    status = AllocError;
		break;
	    }
	}
    }
    fclose(file);
    return status;
}

#define QUOTE		0
#define WHITE		1
#define NORMAL		2
#define END		3
#define NL		4
#define BANG		5

static int  charClass;

static int
lexAlias(file, lexToken)
    FILE       *file;
    char      **lexToken;
{
    int         c;
    char       *t;
    enum state {
	Begin, Normal, Quoted, Comment
    }           state;
    int         count;

    static char *tokenBuf = (char *) NULL;
    static int  tokenSize = 0;

    t = tokenBuf;
    count = 0;
    state = Begin;
    for (;;) {
	if (count == tokenSize) {
	    int         nsize;
	    char       *nbuf;

	    nsize = tokenSize ? (tokenSize << 1) : 64;
	    nbuf = (char *) xrealloc(tokenBuf, nsize);
	    if (!nbuf)
		return EALLOC;
	    tokenBuf = nbuf;
	    tokenSize = nsize;
	    t = tokenBuf + count;
	}
	c = lexc(file);
	switch (charClass) {
	case QUOTE:
	    switch (state) {
	    case Begin:
	    case Normal:
		state = Quoted;
		break;
	    case Quoted:
		state = Normal;
		break;
	    case Comment:
		break;
	    }
	    break;
	case WHITE:
	    switch (state) {
	    case Begin:
	    case Comment:
		continue;
	    case Normal:
		*t = '\0';
		*lexToken = tokenBuf;
		return NAME;
	    case Quoted:
		break;
	    }
	    /* fall through */
	case NORMAL:
	    switch (state) {
	    case Begin:
		state = Normal;
		break;
	    case Comment:
		continue;
	    }
	    *t++ = c;
	    ++count;
	    break;
	case END:
	case NL:
	    switch (state) {
	    case Begin:
	    case Comment:
		*lexToken = (char *) NULL;
		return charClass == END ? DONE : NEWLINE;
	    default:
		*t = '\0';
		*lexToken = tokenBuf;
		ungetc(c, file);
		return NAME;
	    }
	    break;
	case BANG:
	    switch (state) {
	    case Begin:
		state = Comment;
		break;
            case Comment:
		break;
            default:
		*t++ = c;
		++count;
	    }
	    break;
	}
    }
}

static int
lexc(file)
    FILE       *file;
{
    int         c;

    c = getc(file);
    switch (c) {
    case EOF:
	charClass = END;
	break;
    case '\\':
	c = getc(file);
	if (c == EOF)
	    charClass = END;
	else
	    charClass = NORMAL;
	break;
    case '"':
	charClass = QUOTE;
	break;
    case ' ':
    case '\t':
	charClass = WHITE;
	break;
    case '\r':
    case '\n':
	charClass = NL;
	break;
    case '!':
	charClass = BANG;
	break;
    default:
	charClass = NORMAL;
	break;
    }
    return c;
}
