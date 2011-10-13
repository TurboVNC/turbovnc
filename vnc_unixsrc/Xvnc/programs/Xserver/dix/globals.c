/* $XdotOrg: xc/programs/Xserver/dix/globals.c,v 1.7 2005/07/03 08:53:38 daniels Exp $ */
/* $XFree86: xc/programs/Xserver/dix/globals.c,v 1.12tsi Exp $ */
/************************************************************

Copyright 1987, 1998  The Open Group

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

********************************************************/

/* $Xorg: globals.c,v 1.4 2001/02/09 02:04:40 xorgcvs Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include "misc.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "input.h"
#include "dixfont.h"
#include "site.h"
#include "dixstruct.h"
#include "os.h"

ScreenInfo screenInfo;
KeybdCtrl defaultKeyboardControl = {
	DEFAULT_KEYBOARD_CLICK,
	DEFAULT_BELL,
	DEFAULT_BELL_PITCH,
	DEFAULT_BELL_DURATION,
	DEFAULT_AUTOREPEAT,
	DEFAULT_AUTOREPEATS,
	DEFAULT_LEDS,
	0};

PtrCtrl defaultPointerControl = {
	DEFAULT_PTR_NUMERATOR,
	DEFAULT_PTR_DENOMINATOR,
	DEFAULT_PTR_THRESHOLD,
	0};

ClientPtr *clients;
ClientPtr  serverClient;
int  currentMaxClients;   /* current size of clients array */
long maxBigRequestSize = MAX_BIG_REQUEST_SIZE;

WindowPtr *WindowTable;

unsigned long globalSerialNumber = 0;
unsigned long serverGeneration = 0;

/* these next four are initialized in main.c */
CARD32 ScreenSaverTime;
CARD32 ScreenSaverInterval;
int  ScreenSaverBlanking;
int  ScreenSaverAllowExposures;

#ifdef DPMSExtension
# ifndef DEFAULT_STANDBY_TIME
#  define DEFAULT_STANDBY_TIME DEFAULT_SCREEN_SAVER_TIME * 2
# endif
# ifndef DEFAULT_SUSPEND_TIME
#  define DEFAULT_SUSPEND_TIME DEFAULT_SCREEN_SAVER_TIME * 3
# endif
# ifndef DEFAULT_OFF_TIME
#  define DEFAULT_OFF_TIME DEFAULT_SCREEN_SAVER_TIME * 4
# endif
# ifndef DEFAULT_DPMS_ENABLED
#  define DEFAULT_DPMS_ENABLED FALSE
# endif
CARD32 defaultDPMSStandbyTime = DEFAULT_STANDBY_TIME;
CARD32 defaultDPMSSuspendTime = DEFAULT_SUSPEND_TIME;
CARD32 defaultDPMSOffTime = DEFAULT_OFF_TIME;
CARD16 DPMSPowerLevel = 0;
Bool defaultDPMSEnabled = DEFAULT_DPMS_ENABLED;
Bool DPMSEnabledSwitch = FALSE;	  /* these denote the DPMS command line */
Bool DPMSDisabledSwitch = FALSE;  /*                      switch states */
Bool DPMSCapableFlag = FALSE;
CARD32 DPMSStandbyTime;
CARD32 DPMSSuspendTime;
CARD32 DPMSOffTime;
Bool DPMSEnabled;
#endif

CARD32 defaultScreenSaverTime = DEFAULT_SCREEN_SAVER_TIME;
CARD32 defaultScreenSaverInterval = DEFAULT_SCREEN_SAVER_INTERVAL;
int  defaultScreenSaverBlanking = DEFAULT_SCREEN_SAVER_BLANKING;
int  defaultScreenSaverAllowExposures = DEFAULT_SCREEN_SAVER_EXPOSURES;
#ifndef NOLOGOHACK
int  logoScreenSaver = DEFAULT_LOGO_SCREEN_SAVER;
#endif

char *defaultFontPath = COMPILEDDEFAULTFONTPATH;
char *defaultTextFont = COMPILEDDEFAULTFONT;
char *defaultCursorFont = COMPILEDCURSORFONT;
char *rgbPath = RGB_DB;
char *defaultDisplayClass = COMPILEDDISPLAYCLASS;
FontPtr defaultFont;   /* not declared in dix.h to avoid including font.h in
			every compilation of dix code */
Bool loadableFonts = FALSE;
CursorPtr rootCursor;
Bool blackRoot=FALSE;
ClientPtr requestingClient;	/* XXX this should be obsolete now, remove? */

TimeStamp currentTime;
TimeStamp lastDeviceEventTime;

Bool permitOldBugs = FALSE; /* turn off some error checking, to permit certain
			     * old broken clients (like R2/R3 xterms) to work
			     */

int defaultColorVisualClass = -1;
int monitorResolution = 0;

char *display;

CARD32 TimeOutValue = DEFAULT_TIMEOUT * MILLI_PER_SECOND;
int	argcGlobal;
char	**argvGlobal;

DDXPointRec dixScreenOrigins[MAXSCREENS];
