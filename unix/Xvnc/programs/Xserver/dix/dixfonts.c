/* $XFree86: xc/programs/Xserver/dix/dixfonts.c,v 3.29 2003/11/17 22:20:34 dawes Exp $ */
/************************************************************************
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

************************************************************************/

/* $Xorg: dixfonts.c,v 1.4 2000/08/17 19:48:18 cpqbld Exp $ */

#define NEED_REPLIES
#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "resource.h"
#include "dixstruct.h"
#include "cursorstr.h"
#include "misc.h"
#include "opaque.h"
#include "dixfontstr.h"
#include "closestr.h"

#ifdef DEBUG
#include	<stdio.h>
#endif

#ifdef PANORAMIX
#include "panoramiX.h"
#endif

#ifdef LBX
#include "lbxserve.h"
#endif

#ifdef XF86BIGFONT
#define _XF86BIGFONT_SERVER_
#include "xf86bigfont.h"
#endif

#define QUERYCHARINFO(pci, pr)  *(pr) = (pci)->metrics

extern pointer fosNaturalParams;
extern FontPtr defaultFont;

static FontPathElementPtr *font_path_elements = (FontPathElementPtr *) 0;
static int  num_fpes = 0;
FPEFunctions *fpe_functions = (FPEFunctions *) 0;
static int  num_fpe_types = 0;

static unsigned char *font_path_string;

static int  num_slept_fpes = 0;
static int  size_slept_fpes = 0;
static FontPathElementPtr *slept_fpes = (FontPathElementPtr *) 0;
static FontPatternCachePtr patternCache;

int
FontToXError(err)
    int         err;
{
    switch (err) {
    case Successful:
	return Success;
    case AllocError:
	return BadAlloc;
    case BadFontName:
	return BadName;
    case BadFontPath:
    case BadFontFormat:	/* is there something better? */
    case BadCharRange:
	return BadValue;
    default:
	return err;
    }
}


/*
 * adding RT_FONT prevents conflict with default cursor font
 */
Bool
SetDefaultFont(defaultfontname)
    char       *defaultfontname;
{
    int         err;
    FontPtr     pf;
    XID         fid;

    fid = FakeClientID(0);
    err = OpenFont(serverClient, fid, FontLoadAll | FontOpenSync,
		   (unsigned) strlen(defaultfontname), defaultfontname);
    if (err != Success)
	return FALSE;
    pf = (FontPtr) LookupIDByType(fid, RT_FONT);
    if (pf == (FontPtr) NULL)
	return FALSE;
    defaultFont = pf;
    return TRUE;
}

/*
 * note that the font wakeup queue is not refcounted.  this is because
 * an fpe needs to be added when it's inited, and removed when it's finally
 * freed, in order to handle any data that isn't requested, like FS events.
 *
 * since the only thing that should call these routines is the renderer's
 * init_fpe() and free_fpe(), there shouldn't be any problem in using
 * freed data.
 */
void
QueueFontWakeup(fpe)
    FontPathElementPtr fpe;
{
    int         i;
    FontPathElementPtr *new;

    for (i = 0; i < num_slept_fpes; i++) {
	if (slept_fpes[i] == fpe) {

#ifdef DEBUG
	    fprintf(stderr, "re-queueing fpe wakeup\n");
#endif

	    return;
	}
    }
    if (num_slept_fpes == size_slept_fpes) {
	new = (FontPathElementPtr *)
	    xrealloc(slept_fpes,
		     sizeof(FontPathElementPtr) * (size_slept_fpes + 4));
	if (!new)
	    return;
	slept_fpes = new;
	size_slept_fpes += 4;
    }
    slept_fpes[num_slept_fpes] = fpe;
    num_slept_fpes++;
}

void
RemoveFontWakeup(fpe)
    FontPathElementPtr fpe;
{
    int         i,
                j;

    for (i = 0; i < num_slept_fpes; i++) {
	if (slept_fpes[i] == fpe) {
	    for (j = i; j < num_slept_fpes; j++) {
		slept_fpes[j] = slept_fpes[j + 1];
	    }
	    num_slept_fpes--;
	    return;
	}
    }
}

/* ARGSUSED */
void
FontWakeup(data, count, LastSelectMask)
    pointer     data;
    int		count;
    pointer     LastSelectMask;
{
    int         i;
    FontPathElementPtr fpe;

    if (count < 0)
	return;
    /* wake up any fpe's that may be waiting for information */
    for (i = 0; i < num_slept_fpes; i++) {
	fpe = slept_fpes[i];
	(void) (*fpe_functions[fpe->type].wakeup_fpe) (fpe, LastSelectMask);
    }
}

/* XXX -- these two funcs may want to be broken into macros */
static void
UseFPE(FontPathElementPtr fpe)
{
    fpe->refcount++;
}

static void
FreeFPE (FontPathElementPtr fpe)
{
    fpe->refcount--;
    if (fpe->refcount == 0) {
	(*fpe_functions[fpe->type].free_fpe) (fpe);
	xfree(fpe->name);
	xfree(fpe);
    }
}

static Bool
doOpenFont(ClientPtr client, OFclosurePtr c)
{
    FontPtr     pfont = NullFont;
    FontPathElementPtr fpe = NULL;
    ScreenPtr   pScr;
    int         err = Successful;
    int         i;
    char       *alias,
               *newname;
    int         newlen;
    int		aliascount = 20;
    /*
     * Decide at runtime what FontFormat to use.
     */
    Mask FontFormat = 

	((screenInfo.imageByteOrder == LSBFirst) ?
	    BitmapFormatByteOrderLSB : BitmapFormatByteOrderMSB) |

	((screenInfo.bitmapBitOrder == LSBFirst) ?
	    BitmapFormatBitOrderLSB : BitmapFormatBitOrderMSB) |

	BitmapFormatImageRectMin |

#if GLYPHPADBYTES == 1
	BitmapFormatScanlinePad8 |
#endif

#if GLYPHPADBYTES == 2
	BitmapFormatScanlinePad16 |
#endif

#if GLYPHPADBYTES == 4
	BitmapFormatScanlinePad32 |
#endif

#if GLYPHPADBYTES == 8
	BitmapFormatScanlinePad64 |
#endif

	BitmapFormatScanlineUnit8;

    if (client->clientGone)
    {
	if (c->current_fpe < c->num_fpes)
	{
	    fpe = c->fpe_list[c->current_fpe];
	    (*fpe_functions[fpe->type].client_died) ((pointer) client, fpe);
	}
	err = Successful;
	goto bail;
    }
    while (c->current_fpe < c->num_fpes) {
	fpe = c->fpe_list[c->current_fpe];
	err = (*fpe_functions[fpe->type].open_font)
	    ((pointer) client, fpe, c->flags,
	     c->fontname, c->fnamelen, FontFormat,
	     BitmapFormatMaskByte |
	     BitmapFormatMaskBit |
	     BitmapFormatMaskImageRectangle |
	     BitmapFormatMaskScanLinePad |
	     BitmapFormatMaskScanLineUnit,
	     c->fontid, &pfont, &alias,
	     c->non_cachable_font && c->non_cachable_font->fpe == fpe ?
		 c->non_cachable_font :
		 (FontPtr)0);

	if (err == FontNameAlias && alias) {
	    newlen = strlen(alias);
	    newname = (char *) xrealloc(c->fontname, newlen);
	    if (!newname) {
		err = AllocError;
		break;
	    }
	    memmove(newname, alias, newlen);
	    c->fontname = newname;
	    c->fnamelen = newlen;
	    c->current_fpe = 0;
	    if (--aliascount <= 0)
		break;
	    continue;
	}
	if (err == BadFontName) {
	    c->current_fpe++;
	    continue;
	}
	if (err == Suspended) {
	    if (!c->slept) {
		c->slept = TRUE;
		ClientSleep(client, (ClientSleepProcPtr)doOpenFont, (pointer) c);
	    }
	    return TRUE;
	}
	break;
    }

    if (err != Successful)
	goto bail;
    if (!pfont) {
	err = BadFontName;
	goto bail;
    }
    if (!pfont->fpe)
	pfont->fpe = fpe;
    pfont->refcnt++;
    if (pfont->refcnt == 1) {
	UseFPE(pfont->fpe);
	for (i = 0; i < screenInfo.numScreens; i++) {
	    pScr = screenInfo.screens[i];
	    if (pScr->RealizeFont)
	    {
		if (!(*pScr->RealizeFont) (pScr, pfont))
		{
		    CloseFont (pfont, (Font) 0);
		    err = AllocError;
		    goto bail;
		}
	    }
	}
    }
    if (!AddResource(c->fontid, RT_FONT, (pointer) pfont)) {
	err = AllocError;
	goto bail;
    }
    if (patternCache && pfont != c->non_cachable_font)
	CacheFontPattern(patternCache, c->origFontName, c->origFontNameLen,
			 pfont);
bail:
    if (err != Successful && c->client != serverClient) {
	SendErrorToClient(c->client, X_OpenFont, 0,
			  c->fontid, FontToXError(err));
    }
    if (c->slept)
	ClientWakeup(c->client);
    for (i = 0; i < c->num_fpes; i++) {
	FreeFPE(c->fpe_list[i]);
    }
    xfree(c->fpe_list);
    xfree(c->fontname);
    xfree(c);
    return TRUE;
}

int
OpenFont(client, fid, flags, lenfname, pfontname)
    ClientPtr   client;
    XID         fid;
    Mask        flags;
    unsigned    lenfname;
    char       *pfontname;
{
    OFclosurePtr c;
    int         i;
    FontPtr     cached = (FontPtr)0;

#ifdef FONTDEBUG
    char *f;
    f = (char *)xalloc(lenfname + 1);
    memmove(f, pfontname, lenfname);
    f[lenfname] = '\0';
    ErrorF("OpenFont: fontname is \"%s\"\n", f);
    xfree(f);
#endif
    if (!lenfname || lenfname > XLFDMAXFONTNAMELEN)
	return BadName;
    if (patternCache)
    {

    /*
    ** Check name cache.  If we find a cached version of this font that
    ** is cachable, immediately satisfy the request with it.  If we find
    ** a cached version of this font that is non-cachable, we do not
    ** satisfy the request with it.  Instead, we pass the FontPtr to the
    ** FPE's open_font code (the fontfile FPE in turn passes the
    ** information to the rasterizer; the fserve FPE ignores it).
    **
    ** Presumably, the font is marked non-cachable because the FPE has
    ** put some licensing restrictions on it.  If the FPE, using
    ** whatever logic it relies on, determines that it is willing to
    ** share this existing font with the client, then it has the option
    ** to return the FontPtr we passed it as the newly-opened font.
    ** This allows the FPE to exercise its licensing logic without
    ** having to create another instance of a font that already exists.
    */

	cached = FindCachedFontPattern(patternCache, pfontname, lenfname);
	if (cached && cached->info.cachable)
	{
	    if (!AddResource(fid, RT_FONT, (pointer) cached))
		return BadAlloc;
	    cached->refcnt++;
	    return Success;
	}
    }
    c = (OFclosurePtr) xalloc(sizeof(OFclosureRec));
    if (!c)
	return BadAlloc;
    c->fontname = (char *) xalloc(lenfname);
    c->origFontName = pfontname;
    c->origFontNameLen = lenfname;
    if (!c->fontname) {
	xfree(c);
	return BadAlloc;
    }
    /*
     * copy the current FPE list, so that if it gets changed by another client
     * while we're blocking, the request still appears atomic
     */
    c->fpe_list = (FontPathElementPtr *)
	xalloc(sizeof(FontPathElementPtr) * num_fpes);
    if (!c->fpe_list) {
	xfree(c->fontname);
	xfree(c);
	return BadAlloc;
    }
    memmove(c->fontname, pfontname, lenfname);
    for (i = 0; i < num_fpes; i++) {
	c->fpe_list[i] = font_path_elements[i];
	UseFPE(c->fpe_list[i]);
    }
    c->client = client;
    c->fontid = fid;
    c->current_fpe = 0;
    c->num_fpes = num_fpes;
    c->fnamelen = lenfname;
    c->slept = FALSE;
    c->flags = flags;
    c->non_cachable_font = cached;

    (void) doOpenFont(client, c);
    return Success;
}

/*
 * Decrement font's ref count, and free storage if ref count equals zero
 */
/*ARGSUSED*/
int
CloseFont(value, fid)
    pointer	value;  /* must conform to DeleteType */
    XID		fid;
{
    int         nscr;
    ScreenPtr   pscr;
    FontPathElementPtr fpe;
    FontPtr     pfont = (FontPtr)value;

    if (pfont == NullFont)
	return (Success);
    if (--pfont->refcnt == 0) {
	if (patternCache)
	    RemoveCachedFontPattern (patternCache, pfont);
	/*
	 * since the last reference is gone, ask each screen to free any
	 * storage it may have allocated locally for it.
	 */
	for (nscr = 0; nscr < screenInfo.numScreens; nscr++) {
	    pscr = screenInfo.screens[nscr];
	    if (pscr->UnrealizeFont)
		(*pscr->UnrealizeFont) (pscr, pfont);
	}
	if (pfont == defaultFont)
	    defaultFont = NULL;
#ifdef LBX
	LbxFreeFontTag(pfont);
#endif
#ifdef XF86BIGFONT
	XF86BigfontFreeFontShm(pfont);
#endif
	fpe = pfont->fpe;
	(*fpe_functions[fpe->type].close_font) (fpe, pfont);
	FreeFPE(fpe);
    }
    return (Success);
}


/***====================================================================***/

 /*
  * \ Sets up pReply as the correct QueryFontReply for pFont with the first
  * nProtoCCIStructs char infos. \
  */

void
QueryFont(pFont, pReply, nProtoCCIStructs)
    FontPtr          pFont;
    xQueryFontReply *pReply;	/* caller must allocate this storage */
    int              nProtoCCIStructs;
{
    FontPropPtr      pFP;
    int              r,
                     c,
                     i;
    xFontProp       *prFP;
    xCharInfo       *prCI;
    xCharInfo       *charInfos[256];
    unsigned char    chars[512];
    int              ninfos;
    unsigned long    ncols;
    unsigned long    count;

    /* pr->length set in dispatch */
    pReply->minCharOrByte2 = pFont->info.firstCol;
    pReply->defaultChar = pFont->info.defaultCh;
    pReply->maxCharOrByte2 = pFont->info.lastCol;
    pReply->drawDirection = pFont->info.drawDirection;
    pReply->allCharsExist = pFont->info.allExist;
    pReply->minByte1 = pFont->info.firstRow;
    pReply->maxByte1 = pFont->info.lastRow;
    pReply->fontAscent = pFont->info.fontAscent;
    pReply->fontDescent = pFont->info.fontDescent;

    pReply->minBounds = pFont->info.ink_minbounds;
    pReply->maxBounds = pFont->info.ink_maxbounds;

    pReply->nFontProps = pFont->info.nprops;
    pReply->nCharInfos = nProtoCCIStructs;

    for (i = 0, pFP = pFont->info.props, prFP = (xFontProp *) (&pReply[1]);
	    i < pFont->info.nprops;
	    i++, pFP++, prFP++) {
	prFP->name = pFP->name;
	prFP->value = pFP->value;
    }

    ninfos = 0;
    ncols = (unsigned long) (pFont->info.lastCol - pFont->info.firstCol + 1);
    prCI = (xCharInfo *) (prFP);
    for (r = pFont->info.firstRow;
	    ninfos < nProtoCCIStructs && r <= (int)pFont->info.lastRow;
	    r++) {
	i = 0;
	for (c = pFont->info.firstCol; c <= (int)pFont->info.lastCol; c++) {
	    chars[i++] = r;
	    chars[i++] = c;
	}
	(*pFont->get_metrics) (pFont, ncols, chars, 
				TwoD16Bit, &count, charInfos);
	i = 0;
	for (i = 0; i < (int) count && ninfos < nProtoCCIStructs; i++) {
	    *prCI = *charInfos[i];
	    prCI++;
	    ninfos++;
	}
    }
    return;
}

static Bool
doListFontsAndAliases(ClientPtr client, LFclosurePtr c)
{
    FontPathElementPtr fpe;
    int         err = Successful;
    FontNamesPtr names = NULL;
    char       *name, *resolved=NULL;
    int         namelen, resolvedlen;
    int		nnames;
    int         stringLens;
    int         i;
    xListFontsReply reply;
    char	*bufptr;
    char	*bufferStart;
    int		aliascount = 0;

    if (client->clientGone)
    {
	if (c->current.current_fpe < c->num_fpes)
	{
	    fpe = c->fpe_list[c->current.current_fpe];
	    (*fpe_functions[fpe->type].client_died) ((pointer) client, fpe);
	}
	err = Successful;
	goto bail;
    }

    if (!c->current.patlen)
	goto finish;

    while (c->current.current_fpe < c->num_fpes) {
	fpe = c->fpe_list[c->current.current_fpe];
	err = Successful;

	if (!fpe_functions[fpe->type].start_list_fonts_and_aliases)
	{
	    /* This FPE doesn't support/require list_fonts_and_aliases */

	    err = (*fpe_functions[fpe->type].list_fonts)
		((pointer) c->client, fpe, c->current.pattern,
		 c->current.patlen, c->current.max_names - c->names->nnames,
		 c->names);

	    if (err == Suspended) {
		if (!c->slept) {
		    c->slept = TRUE;
		    ClientSleep(client,
			(ClientSleepProcPtr)doListFontsAndAliases,
			(pointer) c);
		}
		return TRUE;
	    }

	    err = BadFontName;
	}
	else
	{
	    /* Start of list_fonts_and_aliases functionality.  Modeled
	       after list_fonts_with_info in that it resolves aliases,
	       except that the information collected from FPEs is just
	       names, not font info.  Each list_next_font_or_alias()
	       returns either a name into name/namelen or an alias into
	       name/namelen and its target name into resolved/resolvedlen.
	       The code at this level then resolves the alias by polling
	       the FPEs.  */

	    if (!c->current.list_started) {
		err = (*fpe_functions[fpe->type].start_list_fonts_and_aliases)
		    ((pointer) c->client, fpe, c->current.pattern,
		     c->current.patlen, c->current.max_names - c->names->nnames,
		     &c->current.private);
		if (err == Suspended) {
		    if (!c->slept) {
			ClientSleep(client,
				    (ClientSleepProcPtr)doListFontsAndAliases,
				    (pointer) c);
			c->slept = TRUE;
		    }
		    return TRUE;
		}
		if (err == Successful)
		    c->current.list_started = TRUE;
	    }
	    if (err == Successful) {
		char    *tmpname;
		name = 0;
		err = (*fpe_functions[fpe->type].list_next_font_or_alias)
		    ((pointer) c->client, fpe, &name, &namelen, &tmpname,
		     &resolvedlen, c->current.private);
		if (err == Suspended) {
		    if (!c->slept) {
			ClientSleep(client,
				    (ClientSleepProcPtr)doListFontsAndAliases,
				    (pointer) c);
			c->slept = TRUE;
		    }
		    return TRUE;
		}
		if (err == FontNameAlias) {
		    if (resolved) xfree(resolved);
		    resolved = (char *) xalloc(resolvedlen + 1);
		    if (resolved)
			memmove(resolved, tmpname, resolvedlen + 1);
		}
	    }

	    if (err == Successful)
	    {
		if (c->haveSaved)
		{
		    if (c->savedName)
			(void)AddFontNamesName(c->names, c->savedName,
					       c->savedNameLen);
		}
		else
		    (void)AddFontNamesName(c->names, name, namelen);
	    }

	    /*
	     * When we get an alias back, save our state and reset back to
	     * the start of the FPE looking for the specified name.  As
	     * soon as a real font is found for the alias, pop back to the
	     * old state
	     */
	    else if (err == FontNameAlias) {
		char	tmp_pattern[XLFDMAXFONTNAMELEN];
		/*
		 * when an alias recurses, we need to give
		 * the last FPE a chance to clean up; so we call
		 * it again, and assume that the error returned
		 * is BadFontName, indicating the alias resolution
		 * is complete.
		 */
		memmove(tmp_pattern, resolved, resolvedlen);
		if (c->haveSaved)
		{
		    char    *tmpname;
		    int     tmpnamelen;

		    tmpname = 0;
		    (void) (*fpe_functions[fpe->type].list_next_font_or_alias)
			((pointer) c->client, fpe, &tmpname, &tmpnamelen,
			 &tmpname, &tmpnamelen, c->current.private);
		    if (--aliascount <= 0)
		    {
			err = BadFontName;
			goto ContBadFontName;
		    }
		}
		else
		{
		    c->saved = c->current;
		    c->haveSaved = TRUE;
		    if (c->savedName)
			xfree(c->savedName);
		    c->savedName = (char *)xalloc(namelen + 1);
		    if (c->savedName)
			memmove(c->savedName, name, namelen + 1);
		    c->savedNameLen = namelen;
		    aliascount = 20;
		}
		memmove(c->current.pattern, tmp_pattern, resolvedlen);
		c->current.patlen = resolvedlen;
		c->current.max_names = c->names->nnames + 1;
		c->current.current_fpe = -1;
		c->current.private = 0;
		err = BadFontName;
	    }
	}
	/*
	 * At the end of this FPE, step to the next.  If we've finished
	 * processing an alias, pop state back. If we've collected enough
	 * font names, quit.
	 */
	if (err == BadFontName) {
	  ContBadFontName: ;
	    c->current.list_started = FALSE;
	    c->current.current_fpe++;
	    err = Successful;
	    if (c->haveSaved)
	    {
		if (c->names->nnames == c->current.max_names ||
			c->current.current_fpe == c->num_fpes) {
		    c->haveSaved = FALSE;
		    c->current = c->saved;
		    /* Give the saved namelist a chance to clean itself up */
		    continue;
		}
	    }
	    if (c->names->nnames == c->current.max_names)
		break;
	}
    }

    /*
     * send the reply
     */
    if (err != Successful) {
	SendErrorToClient(client, X_ListFonts, 0, 0, FontToXError(err));
	goto bail;
    }

finish:

    names = c->names;
    nnames = names->nnames;
    client = c->client;
    stringLens = 0;
    for (i = 0; i < nnames; i++)
	stringLens += (names->length[i] <= 255) ? names->length[i] : 0;

    reply.type = X_Reply;
    reply.length = (stringLens + nnames + 3) >> 2;
    reply.nFonts = nnames;
    reply.sequenceNumber = client->sequence;

    bufptr = bufferStart = (char *) ALLOCATE_LOCAL(reply.length << 2);

    if (!bufptr && reply.length) {
	SendErrorToClient(client, X_ListFonts, 0, 0, BadAlloc);
	goto bail;
    }
    /*
     * since WriteToClient long word aligns things, copy to temp buffer and
     * write all at once
     */
    for (i = 0; i < nnames; i++) {
	if (names->length[i] > 255)
	    reply.nFonts--;
	else
	{
	    *bufptr++ = names->length[i];
	    memmove( bufptr, names->names[i], names->length[i]);
	    bufptr += names->length[i];
	}
    }
    nnames = reply.nFonts;
    reply.length = (stringLens + nnames + 3) >> 2;
    client->pSwapReplyFunc = ReplySwapVector[X_ListFonts];
    WriteSwappedDataToClient(client, sizeof(xListFontsReply), &reply);
    (void) WriteToClient(client, stringLens + nnames, bufferStart);
    DEALLOCATE_LOCAL(bufferStart);

bail:
    if (c->slept)
	ClientWakeup(client);
    for (i = 0; i < c->num_fpes; i++)
	FreeFPE(c->fpe_list[i]);
    xfree(c->fpe_list);
    if (c->savedName) xfree(c->savedName);
    FreeFontNames(names);
    xfree(c);
    if (resolved) xfree(resolved);
    return TRUE;
}

int
ListFonts(client, pattern, length, max_names)
    ClientPtr   client;
    unsigned char *pattern;
    unsigned int length;
    unsigned int max_names;
{
    int         i;
    LFclosurePtr c;

    /* 
     * The right error to return here would be BadName, however the
     * specification does not allow for a Name error on this request.
     * Perhaps a better solution would be to return a nil list, i.e.
     * a list containing zero fontnames.
     */
    if (length > XLFDMAXFONTNAMELEN)
	return BadAlloc;

    if (!(c = (LFclosurePtr) xalloc(sizeof *c)))
	return BadAlloc;
    c->fpe_list = (FontPathElementPtr *)
	xalloc(sizeof(FontPathElementPtr) * num_fpes);
    if (!c->fpe_list) {
	xfree(c);
	return BadAlloc;
    }
    c->names = MakeFontNamesRecord(max_names < 100 ? max_names : 100);
    if (!c->names)
    {
	xfree(c->fpe_list);
	xfree(c);
	return BadAlloc;
    }
    memmove( c->current.pattern, pattern, length);
    for (i = 0; i < num_fpes; i++) {
	c->fpe_list[i] = font_path_elements[i];
	UseFPE(c->fpe_list[i]);
    }
    c->client = client;
    c->num_fpes = num_fpes;
    c->current.patlen = length;
    c->current.current_fpe = 0;
    c->current.max_names = max_names;
    c->current.list_started = FALSE;
    c->current.private = 0;
    c->haveSaved = FALSE;
    c->slept = FALSE;
    c->savedName = 0;
    doListFontsAndAliases(client, c);
    return Success;
}

int
doListFontsWithInfo(client, c)
    ClientPtr   client;
    LFWIclosurePtr c;
{
    FontPathElementPtr fpe;
    int         err = Successful;
    char       *name;
    int         namelen;
    int         numFonts;
    FontInfoRec fontInfo,
               *pFontInfo;
    xListFontsWithInfoReply *reply;
    int         length;
    xFontProp  *pFP;
    int         i;
    int		aliascount = 0;
    xListFontsWithInfoReply finalReply;

    if (client->clientGone)
    {
	if (c->current.current_fpe < c->num_fpes)
 	{
	    fpe = c->fpe_list[c->current.current_fpe];
	    (*fpe_functions[fpe->type].client_died) ((pointer) client, fpe);
	}
	err = Successful;
	goto bail;
    }
    client->pSwapReplyFunc = ReplySwapVector[X_ListFontsWithInfo];
    if (!c->current.patlen)
	goto finish;
    while (c->current.current_fpe < c->num_fpes)
    {
	fpe = c->fpe_list[c->current.current_fpe];
	err = Successful;
	if (!c->current.list_started)
 	{
	    err = (*fpe_functions[fpe->type].start_list_fonts_with_info)
		(client, fpe, c->current.pattern, c->current.patlen,
		 c->current.max_names, &c->current.private);
	    if (err == Suspended)
 	    {
		if (!c->slept)
 		{
		    ClientSleep(client, (ClientSleepProcPtr)doListFontsWithInfo, c);
		    c->slept = TRUE;
		}
		return TRUE;
	    }
	    if (err == Successful)
		c->current.list_started = TRUE;
	}
	if (err == Successful)
 	{
	    name = 0;
	    pFontInfo = &fontInfo;
	    err = (*fpe_functions[fpe->type].list_next_font_with_info)
		(client, fpe, &name, &namelen, &pFontInfo,
		 &numFonts, c->current.private);
	    if (err == Suspended)
 	    {
		if (!c->slept)
 		{
		    ClientSleep(client,
		    	     (ClientSleepProcPtr)doListFontsWithInfo,
			     c);
		    c->slept = TRUE;
		}
		return TRUE;
	    }
	}
	/*
	 * When we get an alias back, save our state and reset back to the
	 * start of the FPE looking for the specified name.  As soon as a real
	 * font is found for the alias, pop back to the old state
	 */
	if (err == FontNameAlias)
 	{
	    /*
	     * when an alias recurses, we need to give
	     * the last FPE a chance to clean up; so we call
	     * it again, and assume that the error returned
	     * is BadFontName, indicating the alias resolution
	     * is complete.
	     */
	    if (c->haveSaved)
	    {
		char	*tmpname;
		int	tmpnamelen;
		FontInfoPtr tmpFontInfo;

	    	tmpname = 0;
	    	tmpFontInfo = &fontInfo;
	    	(void) (*fpe_functions[fpe->type].list_next_font_with_info)
		    (client, fpe, &tmpname, &tmpnamelen, &tmpFontInfo,
		     &numFonts, c->current.private);
		if (--aliascount <= 0)
		{
		    err = BadFontName;
		    goto ContBadFontName;
		}
	    }
	    else
	    {
		c->saved = c->current;
		c->haveSaved = TRUE;
		c->savedNumFonts = numFonts;
		if (c->savedName)
		  xfree(c->savedName);
		c->savedName = (char *)xalloc(namelen + 1);
		if (c->savedName)
		  memmove(c->savedName, name, namelen + 1);
		aliascount = 20;
	    }
	    memmove(c->current.pattern, name, namelen);
	    c->current.patlen = namelen;
	    c->current.max_names = 1;
	    c->current.current_fpe = 0;
	    c->current.private = 0;
	    c->current.list_started = FALSE;
	}
	/*
	 * At the end of this FPE, step to the next.  If we've finished
	 * processing an alias, pop state back.  If we've sent enough font
	 * names, quit.  Always wait for BadFontName to let the FPE
	 * have a chance to clean up.
	 */
	else if (err == BadFontName)
 	{
	  ContBadFontName: ;
	    c->current.list_started = FALSE;
	    c->current.current_fpe++;
	    err = Successful;
	    if (c->haveSaved)
 	    {
		if (c->current.max_names == 0 ||
			c->current.current_fpe == c->num_fpes)
 		{
		    c->haveSaved = FALSE;
		    c->saved.max_names -= (1 - c->current.max_names);
		    c->current = c->saved;
		}
	    }
	    else if (c->current.max_names == 0)
		break;
	}
 	else if (err == Successful)
 	{
	    length = sizeof(*reply) + pFontInfo->nprops * sizeof(xFontProp);
	    reply = c->reply;
	    if (c->length < length)
 	    {
		reply = (xListFontsWithInfoReply *) xrealloc(c->reply, length);
		if (!reply)
 		{
		    err = AllocError;
		    break;
		}
		c->reply = reply;
		c->length = length;
	    }
	    if (c->haveSaved)
 	    {
		numFonts = c->savedNumFonts;
		name = c->savedName;
		namelen = strlen(name);
	    }
	    reply->type = X_Reply;
	    reply->length = (sizeof *reply - sizeof(xGenericReply) +
			     pFontInfo->nprops * sizeof(xFontProp) +
			     namelen + 3) >> 2;
	    reply->sequenceNumber = client->sequence;
	    reply->nameLength = namelen;
	    reply->minBounds = pFontInfo->ink_minbounds;
	    reply->maxBounds = pFontInfo->ink_maxbounds;
	    reply->minCharOrByte2 = pFontInfo->firstCol;
	    reply->maxCharOrByte2 = pFontInfo->lastCol;
	    reply->defaultChar = pFontInfo->defaultCh;
	    reply->nFontProps = pFontInfo->nprops;
	    reply->drawDirection = pFontInfo->drawDirection;
	    reply->minByte1 = pFontInfo->firstRow;
	    reply->maxByte1 = pFontInfo->lastRow;
	    reply->allCharsExist = pFontInfo->allExist;
	    reply->fontAscent = pFontInfo->fontAscent;
	    reply->fontDescent = pFontInfo->fontDescent;
	    reply->nReplies = numFonts;
	    pFP = (xFontProp *) (reply + 1);
	    for (i = 0; i < pFontInfo->nprops; i++)
 	    {
		pFP->name = pFontInfo->props[i].name;
		pFP->value = pFontInfo->props[i].value;
		pFP++;
	    }
	    WriteSwappedDataToClient(client, length, reply);
	    (void) WriteToClient(client, namelen, name);
	    if (pFontInfo == &fontInfo)
 	    {
		xfree(fontInfo.props);
		xfree(fontInfo.isStringProp);
	    }
	    --c->current.max_names;
	}
    }
finish:
    length = sizeof(xListFontsWithInfoReply);
    bzero((char *) &finalReply, sizeof(xListFontsWithInfoReply));
    finalReply.type = X_Reply;
    finalReply.sequenceNumber = client->sequence;
    finalReply.length = (sizeof(xListFontsWithInfoReply)
		     - sizeof(xGenericReply)) >> 2;
    WriteSwappedDataToClient(client, length, &finalReply);
bail:
    if (c->slept)
	ClientWakeup(client);
    for (i = 0; i < c->num_fpes; i++)
	FreeFPE(c->fpe_list[i]);
    xfree(c->reply);
    xfree(c->fpe_list);
    if (c->savedName) xfree(c->savedName);
    xfree(c);
    return TRUE;
}

int
StartListFontsWithInfo(client, length, pattern, max_names)
    ClientPtr   client;
    int         length;
    unsigned char       *pattern;
    int         max_names;
{
    int		    i;
    LFWIclosurePtr  c;

    /* 
     * The right error to return here would be BadName, however the
     * specification does not allow for a Name error on this request.
     * Perhaps a better solution would be to return a nil list, i.e.
     * a list containing zero fontnames.
     */
    if (length > XLFDMAXFONTNAMELEN)
	return BadAlloc;

    if (!(c = (LFWIclosurePtr) xalloc(sizeof *c)))
	goto badAlloc;
    c->fpe_list = (FontPathElementPtr *)
	xalloc(sizeof(FontPathElementPtr) * num_fpes);
    if (!c->fpe_list)
    {
	xfree(c);
	goto badAlloc;
    }
    memmove(c->current.pattern, pattern, length);
    for (i = 0; i < num_fpes; i++)
    {
	c->fpe_list[i] = font_path_elements[i];
	UseFPE(c->fpe_list[i]);
    }
    c->client = client;
    c->num_fpes = num_fpes;
    c->reply = 0;
    c->length = 0;
    c->current.patlen = length;
    c->current.current_fpe = 0;
    c->current.max_names = max_names;
    c->current.list_started = FALSE;
    c->current.private = 0;
    c->savedNumFonts = 0;
    c->haveSaved = FALSE;
    c->slept = FALSE;
    c->savedName = 0;
    doListFontsWithInfo(client, c);
    return Success;
badAlloc:
    return BadAlloc;
}

#define TextEltHeader 2
#define FontShiftSize 5
static XID clearGC[] = { CT_NONE };
#define clearGCmask (GCClipMask)

int
doPolyText(client, c)
    ClientPtr   client;
    register PTclosurePtr c;
{
    register FontPtr pFont = c->pGC->font, oldpFont;
    Font	fid, oldfid;
    int err = Success, lgerr;	/* err is in X error, not font error, space */
    enum { NEVER_SLEPT, START_SLEEP, SLEEPING } client_state = NEVER_SLEPT;
    FontPathElementPtr fpe;
    GC *origGC = NULL;

    if (client->clientGone)
    {
	fpe = c->pGC->font->fpe;
	(*fpe_functions[fpe->type].client_died) ((pointer) client, fpe);

	if (c->slept)
	{
	    /* Client has died, but we cannot bail out right now.  We
	       need to clean up after the work we did when going to
	       sleep.  Setting the drawable pointer to 0 makes this
	       happen without any attempts to render or perform other
	       unnecessary activities.  */
	    c->pDraw = (DrawablePtr)0;
	}
	else
	{
	    err = Success;
	    goto bail;
	}
    }

    /* Make sure our drawable hasn't disappeared while we slept. */
    if (c->slept &&
	c->pDraw &&
	c->pDraw != (DrawablePtr)SecurityLookupIDByClass(client, c->did,
					RC_DRAWABLE, SecurityWriteAccess))
    {
	/* Our drawable has disappeared.  Treat like client died... ask
	   the FPE code to clean up after client and avoid further
	   rendering while we clean up after ourself.  */
	fpe = c->pGC->font->fpe;
	(*fpe_functions[fpe->type].client_died) ((pointer) client, fpe);
	c->pDraw = (DrawablePtr)0;
    }

    client_state = c->slept ? SLEEPING : NEVER_SLEPT;

    while (c->endReq - c->pElt > TextEltHeader)
    {
	if (*c->pElt == FontChange)
        {
	    if (c->endReq - c->pElt < FontShiftSize)
	    {
		 err = BadLength;
		 goto bail;
	    }

	    oldpFont = pFont;
	    oldfid = fid;

	    fid =  ((Font)*(c->pElt+4))		/* big-endian */
		 | ((Font)*(c->pElt+3)) << 8
		 | ((Font)*(c->pElt+2)) << 16
		 | ((Font)*(c->pElt+1)) << 24;
	    pFont = (FontPtr)SecurityLookupIDByType(client, fid, RT_FONT,
						    SecurityReadAccess);
	    if (!pFont)
	    {
		client->errorValue = fid;
		err = BadFont;
		/* restore pFont and fid for step 4 (described below) */
		pFont = oldpFont;
		fid = oldfid;

		/* If we're in START_SLEEP mode, the following step
		   shortens the request...  in the unlikely event that
		   the fid somehow becomes valid before we come through
		   again to actually execute the polytext, which would
		   then mess up our refcounting scheme badly.  */
		c->err = err;
		c->endReq = c->pElt;

		goto bail;
	    }

	    /* Step 3 (described below) on our new font */
	    if (client_state == START_SLEEP)
		pFont->refcnt++;
	    else
	    {
		if (pFont != c->pGC->font && c->pDraw)
		{
		    ChangeGC( c->pGC, GCFont, &fid);
		    ValidateGC(c->pDraw, c->pGC);
		    if (c->reqType == X_PolyText8)
			c->polyText = (PolyTextPtr) c->pGC->ops->PolyText8;
		    else
			c->polyText = (PolyTextPtr) c->pGC->ops->PolyText16;
		}

		/* Undo the refcnt++ we performed when going to sleep */
		if (client_state == SLEEPING)
		    (void)CloseFont(c->pGC->font, (Font)0);
	    }
	    c->pElt += FontShiftSize;
	}
	else	/* print a string */
	{
	    unsigned char *pNextElt;
	    pNextElt = c->pElt + TextEltHeader + (*c->pElt)*c->itemSize;
	    if ( pNextElt > c->endReq)
	    {
		err = BadLength;
		goto bail;
	    }
	    if (client_state == START_SLEEP)
	    {
		c->pElt = pNextElt;
		continue;
	    }
	    if (c->pDraw)
	    {
		lgerr = LoadGlyphs(client, c->pGC->font, *c->pElt, c->itemSize,
				   c->pElt + TextEltHeader);
	    }
	    else lgerr = Successful;

	    if (lgerr == Suspended)
	    {
		if (!c->slept) {
		    int len;
		    GC *pGC;
		    PTclosurePtr new_closure;

    /*  We're putting the client to sleep.  We need to do a few things
	to ensure successful and atomic-appearing execution of the
	remainder of the request.  First, copy the remainder of the
	request into a safe malloc'd area.  Second, create a scratch GC
	to use for the remainder of the request.  Third, mark all fonts
	referenced in the remainder of the request to prevent their
	deallocation.  Fourth, make the original GC look like the
	request has completed...  set its font to the final font value
	from this request.  These GC manipulations are for the unlikely
	(but possible) event that some other client is using the GC.
	Steps 3 and 4 are performed by running this procedure through
	the remainder of the request in a special no-render mode
	indicated by client_state = START_SLEEP.  */

		    /* Step 1 */
		    /* Allocate a malloc'd closure structure to replace
		       the local one we were passed */
		    new_closure = (PTclosurePtr) xalloc(sizeof(PTclosureRec));
		    if (!new_closure)
		    {
			err = BadAlloc;
			goto bail;
		    }
		    *new_closure = *c;
		    c = new_closure;

		    len = c->endReq - c->pElt;
		    c->data = (unsigned char *)xalloc(len);
		    if (!c->data)
		    {
			xfree(c);
			err = BadAlloc;
			goto bail;
		    }
		    memmove(c->data, c->pElt, len);
		    c->pElt = c->data;
		    c->endReq = c->pElt + len;

		    /* Step 2 */

		    pGC = GetScratchGC(c->pGC->depth, c->pGC->pScreen);
		    if (!pGC)
		    {
			xfree(c->data);
			xfree(c);
			err = BadAlloc;
			goto bail;
		    }
		    if ((err = CopyGC(c->pGC, pGC, GCFunction |
				      GCPlaneMask | GCForeground |
				      GCBackground | GCFillStyle |
				      GCTile | GCStipple |
				      GCTileStipXOrigin |
				      GCTileStipYOrigin | GCFont |
				      GCSubwindowMode | GCClipXOrigin |
				      GCClipYOrigin | GCClipMask)) !=
				      Success)
		    {
			FreeScratchGC(pGC);
			xfree(c->data);
			xfree(c);
			err = BadAlloc;
			goto bail;
		    }
		    origGC = c->pGC;
		    c->pGC = pGC;
		    ValidateGC(c->pDraw, c->pGC);
		    
		    c->slept = TRUE;
		    ClientSleep(client,
		    	     (ClientSleepProcPtr)doPolyText,
			     (pointer) c);

		    /* Set up to perform steps 3 and 4 */
		    client_state = START_SLEEP;
		    continue;	/* on to steps 3 and 4 */
		}
		return TRUE;
	    }
	    else if (lgerr != Successful)
	    {
		err = FontToXError(lgerr);
		goto bail;
	    }
	    if (c->pDraw)
	    {
		c->xorg += *((INT8 *)(c->pElt + 1));	/* must be signed */
		c->xorg = (* c->polyText)(c->pDraw, c->pGC, c->xorg, c->yorg,
		    *c->pElt, c->pElt + TextEltHeader);
	    }
	    c->pElt = pNextElt;
	}
    }

bail:

    if (client_state == START_SLEEP)
    {
	/* Step 4 */
	if (pFont != origGC->font)
	{
	    ChangeGC(origGC, GCFont, &fid);
	    ValidateGC(c->pDraw, origGC);
	}

	/* restore pElt pointer for execution of remainder of the request */
	c->pElt = c->data;
	return TRUE;
    }

    if (c->err != Success) err = c->err;
    if (err != Success && c->client != serverClient) {
#ifdef PANORAMIX
        if (noPanoramiXExtension || !c->pGC->pScreen->myNum)
#endif
	    SendErrorToClient(c->client, c->reqType, 0, 0, err);
    }
    if (c->slept)
    {
	ClientWakeup(c->client);
	ChangeGC(c->pGC, clearGCmask, clearGC);

	/* Unreference the font from the scratch GC */
	CloseFont(c->pGC->font, (Font)0);
	c->pGC->font = NullFont;

	FreeScratchGC(c->pGC);
	xfree(c->data);
	xfree(c);
    }
    return TRUE;
}

int
PolyText(client, pDraw, pGC, pElt, endReq, xorg, yorg, reqType, did)
    ClientPtr client;
    DrawablePtr pDraw;
    GC *pGC;
    unsigned char *pElt;
    unsigned char *endReq;
    int xorg;
    int yorg;
    int reqType;
    XID did;
{
    PTclosureRec local_closure;

    local_closure.pElt = pElt;
    local_closure.endReq = endReq;
    local_closure.client = client;
    local_closure.pDraw = pDraw;
    local_closure.xorg = xorg;
    local_closure.yorg = yorg;
    if ((local_closure.reqType = reqType) == X_PolyText8)
    {
	local_closure.polyText = (PolyTextPtr) pGC->ops->PolyText8;
	local_closure.itemSize = 1;
    }
    else
    {
	local_closure.polyText =  (PolyTextPtr) pGC->ops->PolyText16;
	local_closure.itemSize = 2;
    }
    local_closure.pGC = pGC;
    local_closure.did = did;
    local_closure.err = Success;
    local_closure.slept = FALSE;

    (void) doPolyText(client, &local_closure);
    return Success;
}


#undef TextEltHeader
#undef FontShiftSize

int
doImageText(client, c)
    ClientPtr   client;
    register ITclosurePtr c;
{
    int err = Success, lgerr;	/* err is in X error, not font error, space */
    FontPathElementPtr fpe;

    if (client->clientGone)
    {
	fpe = c->pGC->font->fpe;
	(*fpe_functions[fpe->type].client_died) ((pointer) client, fpe);
	err = Success;
	goto bail;
    }

    /* Make sure our drawable hasn't disappeared while we slept. */
    if (c->slept &&
	c->pDraw &&
	c->pDraw != (DrawablePtr)SecurityLookupIDByClass(client, c->did,
					RC_DRAWABLE, SecurityWriteAccess))
    {
	/* Our drawable has disappeared.  Treat like client died... ask
	   the FPE code to clean up after client. */
	fpe = c->pGC->font->fpe;
	(*fpe_functions[fpe->type].client_died) ((pointer) client, fpe);
	err = Success;
	goto bail;
    }

    lgerr = LoadGlyphs(client, c->pGC->font, c->nChars, c->itemSize, c->data);
    if (lgerr == Suspended)
    {
        if (!c->slept) {
	    GC *pGC;
	    unsigned char *data;
	    ITclosurePtr new_closure;

	    /* We're putting the client to sleep.  We need to
	       save some state.  Similar problem to that handled
	       in doPolyText, but much simpler because the
	       request structure is much simpler. */

	    new_closure = (ITclosurePtr) xalloc(sizeof(ITclosureRec));
	    if (!new_closure)
	    {
		err = BadAlloc;
		goto bail;
	    }
	    *new_closure = *c;
	    c = new_closure;

	    data = (unsigned char *)xalloc(c->nChars * c->itemSize);
	    if (!data)
	    {
		xfree(c);
		err = BadAlloc;
		goto bail;
	    }
	    memmove(data, c->data, c->nChars * c->itemSize);
	    c->data = data;

	    pGC = GetScratchGC(c->pGC->depth, c->pGC->pScreen);
	    if (!pGC)
	    {
		xfree(c->data);
		xfree(c);
		err = BadAlloc;
		goto bail;
	    }
	    if ((err = CopyGC(c->pGC, pGC, GCFunction | GCPlaneMask |
			      GCForeground | GCBackground | GCFillStyle |
			      GCTile | GCStipple | GCTileStipXOrigin |
			      GCTileStipYOrigin | GCFont |
			      GCSubwindowMode | GCClipXOrigin |
			      GCClipYOrigin | GCClipMask)) != Success)
	    {
		FreeScratchGC(pGC);
		xfree(c->data);
		xfree(c);
		err = BadAlloc;
		goto bail;
	    }
	    c->pGC = pGC;
	    ValidateGC(c->pDraw, c->pGC);

	    c->slept = TRUE;
            ClientSleep(client, (ClientSleepProcPtr)doImageText, (pointer) c);
        }
        return TRUE;
    }
    else if (lgerr != Successful)
    {
        err = FontToXError(lgerr);
        goto bail;
    }
    if (c->pDraw)
    {
	(* c->imageText)(c->pDraw, c->pGC, c->xorg, c->yorg,
	    c->nChars, c->data);
    }

bail:

    if (err != Success && c->client != serverClient) {
	SendErrorToClient(c->client, c->reqType, 0, 0, err);
    }
    if (c->slept)
    {
	ClientWakeup(c->client);
	ChangeGC(c->pGC, clearGCmask, clearGC);

	/* Unreference the font from the scratch GC */
	CloseFont(c->pGC->font, (Font)0);
	c->pGC->font = NullFont;

	FreeScratchGC(c->pGC);
	xfree(c->data);
	xfree(c);
    }
    return TRUE;
}

int
ImageText(client, pDraw, pGC, nChars, data, xorg, yorg, reqType, did)
    ClientPtr client;
    DrawablePtr pDraw;
    GC *pGC;
    int nChars;
    unsigned char *data;
    int xorg;
    int yorg;
    int reqType;
    XID did;
{
    ITclosureRec local_closure;

    local_closure.client = client;
    local_closure.pDraw = pDraw;
    local_closure.pGC = pGC;
    local_closure.nChars = nChars;
    local_closure.data = data;
    local_closure.xorg = xorg;
    local_closure.yorg = yorg;
    if ((local_closure.reqType = reqType) == X_ImageText8)
    {
	local_closure.imageText = (ImageTextPtr) pGC->ops->ImageText8;
	local_closure.itemSize = 1;
    }
    else
    {
	local_closure.imageText = (ImageTextPtr) pGC->ops->ImageText16;
	local_closure.itemSize = 2;
    }
    local_closure.did = did;
    local_closure.slept = FALSE;

    (void) doImageText(client, &local_closure);
    return Success;
}


/* does the necessary magic to figure out the fpe type */
static int
DetermineFPEType(char *pathname)
{
    int         i;

    for (i = 0; i < num_fpe_types; i++) {
	if ((*fpe_functions[i].name_check) (pathname))
	    return i;
    }
    return -1;
}


static void
FreeFontPath(FontPathElementPtr *list, int n, Bool force)
{
    int         i;

    for (i = 0; i < n; i++) {
	if (force) {
	    /* Sanity check that all refcounts will be 0 by the time
	       we get to the end of the list. */
	    int found = 1;	/* the first reference is us */
	    int j;
	    for (j = i+1; j < n; j++) {
		if (list[j] == list[i])
		    found++;
	    }
	    if (list[i]->refcount != found) {
		ErrorF("FreeFontPath: FPE \"%.*s\" refcount is %d, should be %d; fixing.\n",
		       list[i]->name_length, list[i]->name,
		       list[i]->refcount, found);
		list[i]->refcount = found; /* ensure it will get freed */
	    }
	}
	FreeFPE(list[i]);
    }
    xfree((char *) list);
}

static FontPathElementPtr
find_existing_fpe(FontPathElementPtr *list, int num, unsigned char *name, int len)
{
    FontPathElementPtr fpe;
    int         i;

    for (i = 0; i < num; i++) {
	fpe = list[i];
	if (fpe->name_length == len && memcmp(name, fpe->name, len) == 0)
	    return fpe;
    }
    return (FontPathElementPtr) 0;
}


static int
SetFontPathElements(int npaths, unsigned char *paths, int *bad, Bool persist)
{
    int         i, err = 0;
    int         valid_paths = 0;
    unsigned int len;
    unsigned char *cp = paths;
    FontPathElementPtr fpe = NULL, *fplist;

    fplist = (FontPathElementPtr *)
	xalloc(sizeof(FontPathElementPtr) * npaths);
    if (!fplist) {
	*bad = 0;
	return BadAlloc;
    }
    for (i = 0; i < num_fpe_types; i++) {
	if (fpe_functions[i].set_path_hook)
	    (*fpe_functions[i].set_path_hook) ();
    }
    for (i = 0; i < npaths; i++) 
    {
	len = (unsigned int) (*cp++);

	if (len == 0) 
	{
	    if (persist)
		ErrorF ("Removing empty element from the valid list of fontpaths\n");
	    err = BadValue;
	}
	else
	{
	    /* if it's already in our active list, just reset it */
	    /*
	     * note that this can miss FPE's in limbo -- may be worth catching
	     * them, though it'd muck up refcounting
	     */
	    fpe = find_existing_fpe(font_path_elements, num_fpes, cp, len);
	    if (fpe) 
	    {
		err = (*fpe_functions[fpe->type].reset_fpe) (fpe);
		if (err == Successful) 
		{
		    UseFPE(fpe);/* since it'll be decref'd later when freed
				 * from the old list */
		}
		else
		    fpe = 0;
	    }
	    /* if error or can't do it, act like it's a new one */
	    if (!fpe)
	    {
		fpe = (FontPathElementPtr) xalloc(sizeof(FontPathElementRec));
		if (!fpe) 
		{
		    err = BadAlloc;
		    goto bail;
		}
		fpe->name = (char *) xalloc(len + 1);
		if (!fpe->name) 
		{
		    xfree(fpe);
		    err = BadAlloc;
		    goto bail;
		}
		fpe->refcount = 1;
    
		strncpy(fpe->name, (char *) cp, (int) len);
		fpe->name[len] = '\0';
		fpe->name_length = len;
		fpe->type = DetermineFPEType(fpe->name);
		if (fpe->type == -1)
		    err = BadValue;
		else
		    err = (*fpe_functions[fpe->type].init_fpe) (fpe);
		if (err != Successful)
		{
		    if (persist)
		    {
			ErrorF("Could not init font path element %s, removing from list!\n",
			       fpe->name);
		    }
		    xfree (fpe->name);
		    xfree (fpe);
		}
	    }
	}
	if (err != Successful)
	{
	    if (!persist)
		goto bail;
	}
	else
	{
	    fplist[valid_paths++] = fpe;
	}
	cp += len;
    }

    FreeFontPath(font_path_elements, num_fpes, FALSE);
    font_path_elements = fplist;
    if (patternCache)
	EmptyFontPatternCache(patternCache);
    num_fpes = valid_paths;

    return Success;
bail:
    *bad = i;
    while (--valid_paths >= 0)
	FreeFPE(fplist[valid_paths]);
    xfree(fplist);
    return FontToXError(err);
}

/* XXX -- do we need to pass error down to each renderer? */
int
SetFontPath(client, npaths, paths, error)
    ClientPtr   client;
    int         npaths;
    unsigned char *paths;
    int        *error;
{
    int   err = Success;

    if (npaths == 0) {
	if (SetDefaultFontPath(defaultFontPath) != Success)
	    return BadValue;
    } else {
	err = SetFontPathElements(npaths, paths, error, FALSE);
    }
    return err;
}

int
SetDefaultFontPath(path)
    char       *path;
{
    unsigned char *cp,
               *pp,
               *nump,
               *newpath;
    int         num = 1,
                len,
                err,
                size = 0,
                bad;

    /* get enough for string, plus values -- use up commas */
    len = strlen(path) + 1;
    nump = cp = newpath = (unsigned char *) ALLOCATE_LOCAL(len);
    if (!newpath)
	return BadAlloc;
    pp = (unsigned char *) path;
    cp++;
    while (*pp) {
	if (*pp == ',') {
	    *nump = (unsigned char) size;
	    nump = cp++;
	    pp++;
	    num++;
	    size = 0;
	} else {
	    *cp++ = *pp++;
	    size++;
	}
    }
    *nump = (unsigned char) size;

    err = SetFontPathElements(num, newpath, &bad, TRUE);

    DEALLOCATE_LOCAL(newpath);

    return err;
}

unsigned char *
GetFontPath(count, length)
    int			*count;
    int			*length;
{
    int			i;
    unsigned char       *c;
    int			len;
    FontPathElementPtr	fpe;

    len = 0;
    for (i = 0; i < num_fpes; i++) {
	fpe = font_path_elements[i];
	len += fpe->name_length + 1;
    }
    font_path_string = (unsigned char *) xrealloc(font_path_string, len);
    if (!font_path_string)
	return NULL;

    c = font_path_string;
    *length = 0;
    for (i = 0; i < num_fpes; i++) {
	fpe = font_path_elements[i];
	*c = fpe->name_length;
	*length += *c++;
	memmove(c, fpe->name, fpe->name_length);
	c += fpe->name_length;
    }
    *count = num_fpes;
    return font_path_string;
}

int
LoadGlyphs(client, pfont, nchars, item_size, data)
    ClientPtr   client;
    FontPtr     pfont;
    unsigned    nchars;
    int         item_size;
    unsigned char *data;
{
    if (fpe_functions[pfont->fpe->type].load_glyphs)
	return (*fpe_functions[pfont->fpe->type].load_glyphs)
	    (client, pfont, 0, nchars, item_size, data);
    else
	return Successful;
}

void
DeleteClientFontStuff(client)
    ClientPtr	client;
{
    int			i;
    FontPathElementPtr	fpe;

    for (i = 0; i < num_fpes; i++)
    {
	fpe = font_path_elements[i];
	if (fpe_functions[fpe->type].client_died)
	    (*fpe_functions[fpe->type].client_died) ((pointer) client, fpe);
    }
}

void
InitFonts ()
{
    patternCache = MakeFontPatternCache();

#ifndef KDRIVESERVER
    if (screenInfo.numScreens > screenInfo.numVideoScreens) {
	PrinterFontRegisterFpeFunctions();
	FontFileCheckRegisterFpeFunctions();
	check_fs_register_fpe_functions();
    } else 
#endif
    {
#ifdef KDRIVESERVER
	BuiltinRegisterFpeFunctions();
#endif
	FontFileRegisterFpeFunctions();
#ifndef NOFONTSERVERACCESS
	fs_register_fpe_functions();
#endif
    }
}

int
GetDefaultPointSize ()
{
    return 120;
}


FontResolutionPtr
GetClientResolutions (num)
    int        *num;
{
    if (requestingClient && requestingClient->fontResFunc != NULL &&
	!requestingClient->clientGone)
    {
	return (*requestingClient->fontResFunc)(requestingClient, num);
    }
    else {
	static struct _FontResolution res;
	ScreenPtr   pScreen;

	pScreen = screenInfo.screens[0];
	res.x_resolution = (pScreen->width * 25.4) / pScreen->mmWidth;
	/*
	 * XXX - we'll want this as long as bitmap instances are prevalent 
	 so that we can match them from scalable fonts
	 */
	if (res.x_resolution < 88)
	    res.x_resolution = 75;
	else
	    res.x_resolution = 100;
	res.y_resolution = (pScreen->height * 25.4) / pScreen->mmHeight;
	if (res.y_resolution < 88)
	    res.y_resolution = 75;
	else
	    res.y_resolution = 100;
	res.point_size = 120;
	*num = 1;
	return &res;
    }
}

/*
 * returns the type index of the new fpe
 *
 * should be called (only once!) by each type of fpe when initialized
 */

int
RegisterFPEFunctions(NameCheckFunc name_func, 
		     InitFpeFunc init_func, 
		     FreeFpeFunc free_func, 
		     ResetFpeFunc reset_func, 
		     OpenFontFunc open_func, 
		     CloseFontFunc close_func, 
		     ListFontsFunc list_func, 
		     StartLfwiFunc start_lfwi_func, 
		     NextLfwiFunc next_lfwi_func, 
		     WakeupFpeFunc wakeup_func, 
		     ClientDiedFunc client_died, 
		     LoadGlyphsFunc load_glyphs, 
		     StartLaFunc start_list_alias_func, 
		     NextLaFunc next_list_alias_func, 
		     SetPathFunc set_path_func)
{
    FPEFunctions *new;

    /* grow the list */
    new = (FPEFunctions *) xrealloc(fpe_functions,
				 (num_fpe_types + 1) * sizeof(FPEFunctions));
    if (!new)
	return -1;
    fpe_functions = new;

    fpe_functions[num_fpe_types].name_check = name_func;
    fpe_functions[num_fpe_types].open_font = open_func;
    fpe_functions[num_fpe_types].close_font = close_func;
    fpe_functions[num_fpe_types].wakeup_fpe = wakeup_func;
    fpe_functions[num_fpe_types].list_fonts = list_func;
    fpe_functions[num_fpe_types].start_list_fonts_with_info =
	start_lfwi_func;
    fpe_functions[num_fpe_types].list_next_font_with_info =
	next_lfwi_func;
    fpe_functions[num_fpe_types].init_fpe = init_func;
    fpe_functions[num_fpe_types].free_fpe = free_func;
    fpe_functions[num_fpe_types].reset_fpe = reset_func;
    fpe_functions[num_fpe_types].client_died = client_died;
    fpe_functions[num_fpe_types].load_glyphs = load_glyphs;
    fpe_functions[num_fpe_types].start_list_fonts_and_aliases =
	start_list_alias_func;
    fpe_functions[num_fpe_types].list_next_font_or_alias =
	next_list_alias_func;
    fpe_functions[num_fpe_types].set_path_hook = set_path_func;

    return num_fpe_types++;
}

void
FreeFonts()
{
    if (patternCache) {
	FreeFontPatternCache(patternCache);
	patternCache = 0;
    }
    FreeFontPath(font_path_elements, num_fpes, TRUE);
    font_path_elements = 0;
    num_fpes = 0;
    xfree(fpe_functions);
    num_fpe_types = 0;
    fpe_functions = (FPEFunctions *) 0;
}

/* convenience functions for FS interface */

FontPtr
find_old_font(id)
    XID         id;
{
    return (FontPtr) SecurityLookupIDByType(NullClient, id, RT_NONE,
					    SecurityUnknownAccess);
}

Font
GetNewFontClientID()
{
    return FakeClientID(0);
}

int
StoreFontClientFont(pfont, id)
    FontPtr     pfont;
    Font        id;
{
    return AddResource(id, RT_NONE, (pointer) pfont);
}

void
DeleteFontClientID(id)
    Font        id;
{
    FreeResource(id, RT_NONE);
}

int
client_auth_generation(client)
    ClientPtr client;
{
    return 0;
}

static int  fs_handlers_installed = 0;
static unsigned int last_server_gen;

int
init_fs_handlers(fpe, block_handler)
    FontPathElementPtr fpe;
    BlockHandlerProcPtr block_handler;
{
    /* if server has reset, make sure the b&w handlers are reinstalled */
    if (last_server_gen < serverGeneration) {
	last_server_gen = serverGeneration;
	fs_handlers_installed = 0;
    }
    if (fs_handlers_installed == 0) {

#ifdef DEBUG
	fprintf(stderr, "adding FS b & w handlers\n");
#endif

	if (!RegisterBlockAndWakeupHandlers(block_handler,
					    FontWakeup, (pointer) 0))
	    return AllocError;
	fs_handlers_installed++;
    }
    QueueFontWakeup(fpe);
    return Successful;
}

void
remove_fs_handlers(fpe, block_handler, all)
    FontPathElementPtr fpe;
    BlockHandlerProcPtr block_handler;
    Bool        all;
{
    if (all) {
	/* remove the handlers if no one else is using them */
	if (--fs_handlers_installed == 0) {

#ifdef DEBUG
	    fprintf(stderr, "removing FS b & w handlers\n");
#endif

	    RemoveBlockAndWakeupHandlers(block_handler, FontWakeup,
					 (pointer) 0);
	}
    }
    RemoveFontWakeup(fpe);
}

#ifdef DEBUG
#define GLWIDTHBYTESPADDED(bits,nbytes) \
	((nbytes) == 1 ? (((bits)+7)>>3)        /* pad to 1 byte */ \
	:(nbytes) == 2 ? ((((bits)+15)>>3)&~1)  /* pad to 2 bytes */ \
	:(nbytes) == 4 ? ((((bits)+31)>>3)&~3)  /* pad to 4 bytes */ \
	:(nbytes) == 8 ? ((((bits)+63)>>3)&~7)  /* pad to 8 bytes */ \
	: 0)

#define GLYPH_SIZE(ch, nbytes)          \
	GLWIDTHBYTESPADDED((ch)->metrics.rightSideBearing - \
			(ch)->metrics.leftSideBearing, (nbytes))
dump_char_ascii(cip)
    CharInfoPtr cip;
{
    int         r,
                l;
    int         bpr;
    int         byte;
    static unsigned maskTab[] = {
	(1 << 7), (1 << 6), (1 << 5), (1 << 4),
	(1 << 3), (1 << 2), (1 << 1), (1 << 0),
    };

    bpr = GLYPH_SIZE(cip, 4);
    for (r = 0; r < (cip->metrics.ascent + cip->metrics.descent); r++) {
	pointer     row = (pointer) cip->bits + r * bpr;

	byte = 0;
	for (l = 0; l <= (cip->metrics.rightSideBearing -
			  cip->metrics.leftSideBearing); l++) {
	    if (maskTab[l & 7] & row[l >> 3])
		putchar('X');
	    else
		putchar('.');
	}
	putchar('\n');
    }
}

#endif
