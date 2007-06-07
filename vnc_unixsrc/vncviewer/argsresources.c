/*
 *  Copyright (C) 2005-2007 Sun Microsystems, Inc.  All Rights Reserved.
 *  Copyright (C) 2002-2006 Constantin Kaplinsky.  All Rights Reserved.
 *  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 *  USA.
 */

/*
 * argsresources.c - deal with command-line args and resources.
 */

#include "vncviewer.h"

/*
 * fallback_resources - these are used if there is no app-defaults file
 * installed in one of the standard places.
 */

char *fallback_resources[] = {

  "Vncviewer.title: TurboVNC: %s",

  "Vncviewer.translations:\
    <Enter>: SelectionToVNC()\\n\
    <Leave>: SelectionFromVNC()",

  "*form.background: black",

  "*viewport.allowHoriz: True",
  "*viewport.allowVert: True",
  "*viewport.useBottom: True",
  "*viewport.useRight: True",
  "*viewport*Scrollbar*thumb: None",

  "*desktop.baseTranslations:\
     <Key>F8: ShowPopup()\\n\
     <ButtonPress>: SendRFBEvent()\\n\
     <ButtonRelease>: SendRFBEvent()\\n\
     <Motion>: SendRFBEvent()\\n\
     <KeyPress>: SendRFBEvent()\\n\
     <KeyRelease>: SendRFBEvent()",

  "*serverDialog.dialog.label: VNC server:",
  "*serverDialog.dialog.value:",
  "*serverDialog.dialog.value.translations: #override\\n\
     <Key>Return: ServerDialogDone()",

  "*passwordDialog.dialog.label: Password:",
  "*passwordDialog.dialog.value:",
  "*passwordDialog.dialog.value.AsciiSink.echo: False",
  "*passwordDialog.dialog.value.translations: #override\\n\
     <Key>Return: PasswordDialogDone()",

  "*qualLabel.label: JPEG Quality",
  "*qualBar.length: 100",
  "*qualBar.width: 130",
  "*qualBar.orientation: horizontal",
  "*qualBar.translations: #override\\n\
     <Btn1Down>: StartScroll(Continuous) MoveThumb() NotifyThumb()\\n\
     <Btn1Motion>: MoveThumb() NotifyThumb()\\n\
     <Btn3Down>: StartScroll(Continuous) MoveThumb() NotifyThumb()\\n\
     <Btn3Motion>: MoveThumb() NotifyThumb()",
    
  "*qualText.label: 000",

  "*subsampLabel.label: JPEG Chrominance Subsampling\\n[4X = fastest]\\n[None = best quality]",
  "*subsampGray.label: Grayscale",
  "*subsamp4X.label: 4X",
  "*subsamp2X.label: 2X",
  "*subsamp1X.label: None",

  "*compressLabel.label: Image Compression Type",
  "*compressRGB.label: None (RGB)",
  "*compressJPEG.label: JPEG",

  "*wanopt.label: Optimize for High-Latency Network",

  "*popup.title: TurboVNC popup",
  "*popup*background: grey",
  "*popup*font: -*-helvetica-bold-r-*-*-16-*-*-*-*-*-*-*",
  "*popup.buttonForm.Command.borderWidth: 0",
  "*popup.buttonForm.Toggle.borderWidth: 0",

  "*popup.translations: #override <Message>WM_PROTOCOLS: HidePopup()",
  "*popup.buttonForm.translations: #override\\n\
     <KeyPress>: SendRFBEvent() HidePopup()",

  "*popupButtonCount: 11",

  "*popup*button1.label: Dismiss popup",
  "*popup*button1.translations: #override\\n\
     <Btn1Down>,<Btn1Up>: HidePopup()",

  "*popup*button2.label: Quit viewer",
  "*popup*button2.translations: #override\\n\
     <Btn1Down>,<Btn1Up>: Quit()",

  "*popup*button3.label: Full screen",
  "*popup*button3.type: toggle",
  "*popup*button3.translations: #override\\n\
     <Visible>: SetFullScreenState()\\n\
     <Btn1Down>,<Btn1Up>: toggle() ToggleFullScreen() HidePopup()",

  "*popup*button4.label: Clipboard: local -> remote",
  "*popup*button4.translations: #override\\n\
     <Btn1Down>,<Btn1Up>: SelectionToVNC(always) HidePopup()",

  "*popup*button5.label: Clipboard: local <- remote",
  "*popup*button5.translations: #override\\n\
     <Btn1Down>,<Btn1Up>: SelectionFromVNC(always) HidePopup()",

  "*popup*button6.label: Request refresh",
  "*popup*button6.translations: #override\\n\
     <Btn1Down>,<Btn1Up>: SendRFBEvent(fbupdate) HidePopup()",

  "*popup*button7.label: Send ctrl-alt-del",
  "*popup*button7.translations: #override\\n\
     <Btn1Down>,<Btn1Up>: SendRFBEvent(keydown,Control_L)\
                          SendRFBEvent(keydown,Alt_L)\
                          SendRFBEvent(key,Delete)\
                          SendRFBEvent(keyup,Alt_L)\
                          SendRFBEvent(keyup,Control_L)\
                          HidePopup()",

  "*popup*button8.label: Send F8",
  "*popup*button8.translations: #override\\n\
     <Btn1Down>,<Btn1Up>: SendRFBEvent(key,F8) HidePopup()",

  "*popup*button9.label: Preset: High-Speed Network (default)",
  "*popup*button9.translations: #override\\n\
     <Btn1Down>,<Btn1Up>: QualHigh()",

  "*popup*button10.label: Preset: Broadband (favor performance)",
  "*popup*button10.translations: #override\\n\
     <Btn1Down>,<Btn1Up>: QualLow()",

  "*popup*button11.label: Preset: Broadband (favor image quality)",
  "*popup*button11.translations: #override\\n\
     <Btn1Down>,<Btn1Up>: QualWAN()",

  NULL
};


/*
 * vncServerHost and vncServerPort are set either from the command line or
 * from a dialog box.
 */

char vncServerHost[256];
int vncServerPort = 0;


/*
 * appData is our application-specific data which can be set by the user with
 * application resource specs.  The AppData structure is defined in the header
 * file.
 */

AppData appData;

static XtResource appDataResourceList[] = {
  {"shareDesktop", "ShareDesktop", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, shareDesktop), XtRImmediate, (XtPointer) True},

  {"viewOnly", "ViewOnly", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, viewOnly), XtRImmediate, (XtPointer) False},

  {"fullScreen", "FullScreen", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, fullScreen), XtRImmediate, (XtPointer) False},

  {"raiseOnBeep", "RaiseOnBeep", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, raiseOnBeep), XtRImmediate, (XtPointer) True},

  {"passwordFile", "PasswordFile", XtRString, sizeof(String),
   XtOffsetOf(AppData, passwordFile), XtRImmediate, (XtPointer) 0},

  {"passwordDialog", "PasswordDialog", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, passwordDialog), XtRImmediate, (XtPointer) False},

  {"compressType", "CompressType", XtRString, sizeof(String),
   XtOffsetOf(AppData, encodingsString), XtRImmediate, (XtPointer) "JPEG"},

  {"useCopyRect", "UseCopyRect", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, useCopyRect), XtRImmediate, (XtPointer) True},

  {"useSharedMemory", "UseSharedMemory", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, useShm), XtRImmediate, (XtPointer) True},

  {"wmDecorationWidth", "WmDecorationWidth", XtRInt, sizeof(int),
   XtOffsetOf(AppData, wmDecorationWidth), XtRImmediate, (XtPointer) 4},

  {"wmDecorationHeight", "WmDecorationHeight", XtRInt, sizeof(int),
   XtOffsetOf(AppData, wmDecorationHeight), XtRImmediate, (XtPointer) 24},

  {"popupButtonCount", "PopupButtonCount", XtRInt, sizeof(int),
   XtOffsetOf(AppData, popupButtonCount), XtRImmediate, (XtPointer) 0},

  {"debug", "Debug", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, debug), XtRImmediate, (XtPointer) False},

  {"rawDelay", "RawDelay", XtRInt, sizeof(int),
   XtOffsetOf(AppData, rawDelay), XtRImmediate, (XtPointer) 0},

  {"copyRectDelay", "CopyRectDelay", XtRInt, sizeof(int),
   XtOffsetOf(AppData, copyRectDelay), XtRImmediate, (XtPointer) 0},

  {"bumpScrollTime", "BumpScrollTime", XtRInt, sizeof(int),
   XtOffsetOf(AppData, bumpScrollTime), XtRImmediate, (XtPointer) 25},

  {"bumpScrollPixels", "BumpScrollPixels", XtRInt, sizeof(int),
   XtOffsetOf(AppData, bumpScrollPixels), XtRImmediate, (XtPointer) 20},

  {"subsampling", "Subsampling", XtRString, sizeof(String),
   XtOffsetOf(AppData, subsampString), XtRImmediate, (XtPointer) "1X"},

  {"quality", "Quality", XtRInt, sizeof(int),
   XtOffsetOf(AppData, qualityLevel), XtRImmediate, (XtPointer) 95},

  {"useRemoteCursor", "UseRemoteCursor", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, useRemoteCursor), XtRImmediate, (XtPointer) True},

  {"useX11Cursor", "UseX11Cursor", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, useX11Cursor), XtRImmediate, (XtPointer) False},

  {"grabKeyboard", "GrabKeyboard", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, grabKeyboard), XtRImmediate, (XtPointer) False},

  {"doubleBuffer", "DoubleBuffer", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, doubleBuffer), XtRImmediate, (XtPointer) True},

  {"optimizeForWAN", "OptimizeForWAN", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, optimizeForWAN), XtRImmediate, (XtPointer) False},

  {"autoPass", "AutoPass", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, autoPass), XtRImmediate, (XtPointer) False}
};


/*
 * The cmdLineOptions array specifies how certain app resource specs can be set
 * with command-line options.
 */

XrmOptionDescRec cmdLineOptions[] = {
  {"-shared",        "*shareDesktop",       XrmoptionNoArg,  "True"},
  {"-noshared",      "*shareDesktop",       XrmoptionNoArg,  "False"},
  {"-viewonly",      "*viewOnly",           XrmoptionNoArg,  "True"},
  {"-fullscreen",    "*fullScreen",         XrmoptionNoArg,  "True"},
  {"-noraiseonbeep", "*raiseOnBeep",        XrmoptionNoArg,  "False"},
  {"-passwd",        "*passwordFile",       XrmoptionSepArg, 0},
  {"-compress",      "*compressType",       XrmoptionSepArg, 0},
  {"-nocopyrect",    "*useCopyRect",        XrmoptionNoArg,  "False"},
  {"-samp",          "*subsampling",        XrmoptionSepArg, 0},
  {"-quality",       "*quality",            XrmoptionSepArg, 0},
  {"-nocursorshape", "*useRemoteCursor",    XrmoptionNoArg,  "False"},
  {"-x11cursor",     "*useX11Cursor",       XrmoptionNoArg,  "True"},
  {"-singlebuffer",  "*doubleBuffer",       XrmoptionNoArg,  "False"},
  {"-broadband",     "*qualityLevel",       XrmoptionNoArg,  "-1"},
  {"-autopass",      "*autoPass",           XrmoptionNoArg,  "True"},
  {"-wan",           "*optimizeForWAN",     XrmoptionNoArg,  "True"},

};

int numCmdLineOptions = XtNumber(cmdLineOptions);


/*
 * actions[] specifies actions that can be used in widget resource specs.
 */

static XtActionsRec actions[] = {
    {"SendRFBEvent", SendRFBEvent},
    {"QualHigh", QualHigh},
    {"QualLow", QualLow},
    {"QualWAN", QualWAN},
    {"ShowPopup", ShowPopup},
    {"HidePopup", HidePopup},
    {"ToggleFullScreen", ToggleFullScreen},
    {"SetFullScreenState", SetFullScreenState},
    {"SelectionFromVNC", SelectionFromVNC},
    {"SelectionToVNC", SelectionToVNC},
    {"ServerDialogDone", ServerDialogDone},
    {"PasswordDialogDone", PasswordDialogDone},
    {"Pause", Pause},
    {"RunCommand", RunCommand},
    {"Quit", Quit},
};


/*
 * removeArgs() is used to remove some of command line arguments.
 */

void
removeArgs(int *argc, char** argv, int idx, int nargs)
{
  int i;
  if ((idx+nargs) > *argc) return;
  for (i = idx+nargs; i < *argc; i++) {
    argv[i-nargs] = argv[i];
  }
  *argc -= nargs;
}

/*
 * usage() prints out the usage message.
 */

void
usage(void)
{
  fprintf(stderr,
	  "TurboVNC Viewer version 0.4\n"
	  "\n"
	  "Usage: %s [<OPTIONS>] [<HOST>][:<DISPLAY#>]\n"
	  "       %s [<OPTIONS>] [<HOST>][::<PORT#>]\n"
	  "       %s [<OPTIONS>] -listen [<DISPLAY#>]\n"
	  "       %s -help\n"
	  "\n"
	  "<OPTIONS> are standard Xt options, or:\n"
	  "        -via <GATEWAY>\n"
	  "        -shared (set by default)\n"
	  "        -noshared\n"
	  "        -viewonly\n"
	  "        -fullscreen\n"
	  "        -noraiseonbeep\n"
	  "        -passwd <PASSWD-FILENAME> (standard VNC authentication)\n"
	  "        -nocopyrect\n"
	  "        -compress <IMAGE COMPRESSION TYPE> (jpeg | rgb)\n"
	  "        -samp <JPEG CHROMINANCE SUBSAMPLING> (1X | 2X | 4X | gray)\n"
	  "        -quality <JPEG IMAGE QUALITY> (1..100: 1-low, 100-high)\n"
	  "        -nocursorshape\n"
	  "        -x11cursor\n"
	  "        -autopass\n"
	  "        -singlebuffer\n"
	  "        -wan\n"
	  "        -broadband (preset for -wan -samp 4 -quality 30)\n"
	  "\n"
	  "Option names may be abbreviated, e.g. -q instead of -quality.\n"
	  "See the manual page for more information."
	  "\n", programName, programName, programName, programName);
  exit(1);
}


/*
 * GetArgsAndResources() deals with resources and any command-line arguments
 * not already processed by XtVaAppInitialize().  It sets vncServerHost and
 * vncServerPort and all the fields in appData.
 */

void
GetArgsAndResources(int argc, char **argv)
{
  int i;
  char *vncServerName, *colonPos;
  int len, portOffset;
  int disp;

  /* Turn app resource specs into our appData structure for the rest of the
     program to use */

  XtGetApplicationResources(toplevel, &appData, appDataResourceList,
			    XtNumber(appDataResourceList), 0, 0);

  /* -broadband switch was used */

  if(appData.qualityLevel==-1) {
    appData.qualityLevel=30;
    appData.compressLevel=TVNC_4X;
    appData.optimizeForWAN=1;
  }

  /* Translate compression parameters */
  if (appData.encodingsString && (toupper(appData.encodingsString[0]) == 'R'
    || appData.encodingsString[0] == '0'))
    appData.compressType = TVNC_RGB;
  else
    appData.compressType = TVNC_JPEG;

  if (appData.subsampString) {
    switch(toupper(appData.subsampString[0])) {
      case 'G': case '0':  appData.compressLevel=TVNC_GRAY;  break;
      case '1':  appData.compressLevel=TVNC_1X;  break;
      case '2':  appData.compressLevel=TVNC_2X;  break;
      case '4':  appData.compressLevel=TVNC_4X;  break;
    }
  }

  /* Add our actions to the actions table so they can be used in widget
     resource specs */

  XtAppAddActions(appContext, actions, XtNumber(actions));

  /* Check any remaining command-line arguments.  If -listen was specified
     there should be none.  Otherwise the only argument should be the VNC
     server name.  If not given then pop up a dialog box and wait for the
     server name to be entered. */

  if (listenSpecified) {
    if (argc != 1) {
      fprintf(stderr,"\n%s -listen: invalid command line argument: %s\n",
	      programName, argv[1]);
      usage();
    }
    return;
  }

  if (argc == 1) {
    vncServerName = DoServerDialog();
    appData.passwordDialog = True;
  } else if (argc != 2) {
    usage();
  } else {
    vncServerName = argv[1];

    if (!isatty(0))
      appData.passwordDialog = True;
    if (vncServerName[0] == '-')
      usage();
  }

  if (appData.doubleBuffer)
    appData.useX11Cursor = True;

  if (strlen(vncServerName) > 255) {
    fprintf(stderr,"VNC server name too long\n");
    exit(1);
  }

  colonPos = strchr(vncServerName, ':');
  if (colonPos == NULL) {
    /* No colon -- use default port number */
    strcpy(vncServerHost, vncServerName);
    vncServerPort = SERVER_PORT_OFFSET;
  } else {
    memcpy(vncServerHost, vncServerName, colonPos - vncServerName);
    vncServerHost[colonPos - vncServerName] = '\0';
    len = strlen(colonPos + 1);
    portOffset = SERVER_PORT_OFFSET;
    if (colonPos[1] == ':') {
      /* Two colons -- interpret as a port number */
      colonPos++;
      len--;
      portOffset = 0;
    }
    if (!len || strspn(colonPos + 1, "0123456789") != len) {
      usage();
    }
    disp = atoi(colonPos + 1);
    if (portOffset != 0 && disp >= 100)
      portOffset = 0;
    vncServerPort = disp + portOffset;
  }
}
