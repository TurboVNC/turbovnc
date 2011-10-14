/* $Xorg: ddxList.c,v 1.3 2000/08/17 19:53:46 cpqbld Exp $ */
/************************************************************
Copyright (c) 1995 by Silicon Graphics Computer Systems, Inc.

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Silicon Graphics not be 
used in advertising or publicity pertaining to distribution 
of the software without specific prior written permission.
Silicon Graphics makes no representation about the suitability 
of this software for any purpose. It is provided "as is"
without any express or implied warranty.

SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/
/* $XFree86: xc/programs/Xserver/xkb/ddxList.c,v 3.8 2003/07/16 01:39:05 dawes Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <ctype.h>
#define	NEED_EVENTS 1
#include <X11/X.h>
#include <X11/Xos.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include <X11/extensions/XKM.h>
#include "inputstr.h"
#include "scrnintstr.h"
#include "windowstr.h"
#define	XKBSRV_NEED_FILE_FUNCS
#include <X11/extensions/XKBsrv.h>
#include <X11/extensions/XI.h>

#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define	PATH_MAX MAXPATHLEN
#else
#define	PATH_MAX 1024
#endif
#endif

#ifdef WIN32
/* from ddxLoad.c */
extern const char* Win32TempDir();
extern int Win32System(const char *cmdline);
#undef System
#define System Win32System

#define W32_tmparg " '%s'"
#define W32_tmpfile ,tmpname
#define W32_tmplen strlen(tmpname)+3
#else
#define W32_tmparg
#define W32_tmpfile 
#define W32_tmplen 0
#endif 

/***====================================================================***/

static char *componentDirs[_XkbListNumComponents] = {
	"keymap", "keycodes", "types", "compat", "symbols", "geometry"
};

/***====================================================================***/

static Status
_AddListComponent(	XkbSrvListInfoPtr	list,
			int			what,
			unsigned		flags,
			char *			str,
			ClientPtr		client)
{
int		slen,wlen;
unsigned char *	wire8;
unsigned short *wire16;
char *		tmp;

    if (list->nTotal>=list->maxRtrn) {
	list->nTotal++;
	return Success;
    }
    tmp= strchr(str,')');
    if ((tmp==NULL)&&((tmp=strchr(str,'('))==NULL)) {
	slen= strlen(str);
	while ((slen>0) && isspace(str[slen-1])) {
	    slen--;
	}
    }
    else {
	slen= (tmp-str+1);
    }
    wlen= (((slen+1)/2)*2)+4;	/* four bytes for flags and length, pad to */
				/* 2-byte boundary */
    if ((list->szPool-list->nPool)<wlen) {
	if (wlen>1024)	list->szPool+= XkbPaddedSize(wlen*2);
	else		list->szPool+= 1024;
	list->pool= _XkbTypedRealloc(list->pool,list->szPool,char);
	if (!list->pool)
	    return BadAlloc;
    }
    wire16= (unsigned short *)&list->pool[list->nPool];
    wire8= (unsigned char *)&wire16[2];
    wire16[0]= flags;
    wire16[1]= slen;
    memcpy(wire8,str,slen);
    if (client->swapped) {
	register int n;
	swaps(&wire16[0],n);
	swaps(&wire16[1],n);
    }
    list->nPool+= wlen;
    list->nFound[what]++;
    list->nTotal++;
    return Success;
}

/***====================================================================***/
static Status
XkbDDXListComponent(	DeviceIntPtr 		dev,
			int			what,
			XkbSrvListInfoPtr	list,
			ClientPtr		client)
{
char 	*file,*map,*tmp,*buf=NULL;
FILE 	*in;
Status	status;
int	rval;
Bool	haveDir;
#ifdef WIN32
char	tmpname[PATH_MAX];
#endif

    if ((list->pattern[what]==NULL)||(list->pattern[what][0]=='\0'))
	return Success;
    file= list->pattern[what];
    map= strrchr(file,'(');
    if (map!=NULL) {
	char *tmp;
	map++;
	tmp= strrchr(map,')');
	if ((tmp==NULL)||(tmp[1]!='\0')) {
	    /* illegal pattern.  No error, but no match */
	    return Success;
	}
    }

    in= NULL;
    haveDir= True;
#ifdef WIN32
    strcpy(tmpname, Win32TempDir());
    strcat(tmpname, "\\xkb_XXXXXX");
    (void) mktemp(tmpname);
#endif
    if (XkbBaseDirectory!=NULL) {
	if ((list->pattern[what][0]=='*')&&(list->pattern[what][1]=='\0')) {
	    buf = Xprintf("%s/%s.dir",XkbBaseDirectory,componentDirs[what]);
	    in= fopen(buf,"r");
	    xfree (buf);
	    buf = NULL;
	}
	if (!in) {
	    haveDir= False;
	    buf = Xprintf(
		"'%s/xkbcomp' '-R%s/%s' -w %ld -l -vlfhpR '%s'" W32_tmparg,
                XkbBinDirectory,XkbBaseDirectory,componentDirs[what],(long)
		((xkbDebugFlags<2)?1:((xkbDebugFlags>10)?10:xkbDebugFlags)),
		file W32_tmpfile
                );
	}
    }
    else {
	if ((list->pattern[what][0]=='*')&&(list->pattern[what][1]=='\0')) {
	    buf = Xprintf("%s.dir",componentDirs[what]);
	    in= fopen(buf,"r");
	    xfree (buf);
	    buf = NULL;
	}
	if (!in) {
	    haveDir= False;
	    buf = Xprintf(
		"xkbcomp -R%s -w %ld -l -vlfhpR '%s'" W32_tmparg,
                componentDirs[what],(long)
		((xkbDebugFlags<2)?1:((xkbDebugFlags>10)?10:xkbDebugFlags)),
		file W32_tmpfile
                );
	}
    }
    status= Success;
    if (!haveDir)
    {  
#ifndef WIN32
	in= Popen(buf,"r");
#else
#ifdef DEBUG_CMD
	ErrorF("xkb executes: %s\n",buf);
#endif
	if (System(buf) < 0)
	    ErrorF("Could not invoke keymap compiler\n");
	else
	    in= fopen(tmpname, "r");
#endif
    }
    if (!in)
    {
        if (buf != NULL)
	    xfree (buf);
#ifdef WIN32
	unlink(tmpname);
#endif
	return BadImplementation;
    }
    list->nFound[what]= 0;
    while ((status==Success)&&((tmp=fgets(buf,PATH_MAX,in))!=NULL)) {
	unsigned flags;
	register unsigned int i;
	if (*tmp=='#') /* comment, skip it */
	    continue;
	if (!strncmp(tmp, "Warning:", 8) || !strncmp(tmp, "        ", 8))
	    /* skip warnings too */
	    continue;
	flags= 0;
	/* each line in the listing is supposed to start with two */
	/* groups of eight characters, which specify the general  */
	/* flags and the flags that are specific to the component */
	/* if they're missing, fail with BadImplementation	  */
	for (i=0;(i<8)&&(status==Success);i++) { /* read the general flags */
	   if (isalpha(*tmp))	flags|= (1L<<i);
	   else if (*tmp!='-')	status= BadImplementation;
	   tmp++;
	}
	if (status != Success)  break;
	if (!isspace(*tmp)) {
	     status= BadImplementation;
	     break;
	}
	else tmp++;
	for (i=0;(i<8)&&(status==Success);i++) { /* read the component flags */
	   if (isalpha(*tmp))	flags|= (1L<<(i+8));
	   else if (*tmp!='-')	status= BadImplementation;
	   tmp++;
	}
	if (status != Success)  break;
	if (isspace(*tmp)) {
	    while (isspace(*tmp)) {
		tmp++;
	    }
	}
	else {
	    status= BadImplementation;
	    break;
	}
	status= _AddListComponent(list,what,flags,tmp,client);
    }
#ifndef WIN32
    if (haveDir)
	fclose(in);
    else if ((rval=pclose(in))!=0) {
	if (xkbDebugFlags)
	    ErrorF("xkbcomp returned exit code %d\n",rval);
    }
#else
    fclose(in);
    unlink(tmpname);
#endif
    if (buf != NULL)
        xfree (buf);
    return status;
}

/***====================================================================***/

/* ARGSUSED */
Status
XkbDDXList(DeviceIntPtr	dev,XkbSrvListInfoPtr list,ClientPtr client)
{
Status	status;

    status= XkbDDXListComponent(dev,_XkbListKeymaps,list,client);
    if (status==Success)
	status= XkbDDXListComponent(dev,_XkbListKeycodes,list,client);
    if (status==Success)
	status= XkbDDXListComponent(dev,_XkbListTypes,list,client);
    if (status==Success)
	status= XkbDDXListComponent(dev,_XkbListCompat,list,client);
    if (status==Success)
	status= XkbDDXListComponent(dev,_XkbListSymbols,list,client);
    if (status==Success)
	status= XkbDDXListComponent(dev,_XkbListGeometry,list,client);
    return status;
}
