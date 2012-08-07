/*
 *  Copyright (C) 2009-2012 D. R. Commander.  All Rights Reserved.
 *  Copyright (C) 2010 University Corporation for Atmospheric Research.
                       All Rights Reserved.
 *  Copyright (C) 2005-2008 Sun Microsystems, Inc.  All Rights Reserved.
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 */

/*
 * argsresources.c - deal with command-line args and resources.
 */

#include <ctype.h>
#include "vncviewer.h"
#include "vncauth.h"


/*
 * fallback_resources - these are used if there is no app-defaults file
 * installed in one of the standard places.  Don't forget to 'make res'
 * to regenerate Tvncviewer whenever you change these.
 */

char *fallback_resources[] = {
/*
 * Application defaults file for the TurboVNC Viewer
 */


/*
 * The title of the main window.  "%s" will be replaced by the desktop name.
 */

"Tvncviewer.title: TurboVNC: %s",


/*
 * Translations on the main window
 */

"Tvncviewer.translations:\
  <Enter>: SelectionToVNC()\\n\
  <Leave>: SelectionFromVNC()",


/*
 * Uncomment if you don't want the TurboVNC Viewer to grab the keyboard in
 * full-screen mode.
 */

/* Tvncviewer.grabKeyboard: False */


/*
 * Background color to use in full-screen mode if the remote desktop size is
 * smaller than the local desktop size
 */

"*form.background: black",


/*
 * If the remote desktop size exceeds the local desktop size, use scrollbars on
 * the right and bottom of the window.
 */

"*viewport.allowHoriz: True",
"*viewport.allowVert: True",
"*viewport.useBottom: True",
"*viewport.useRight: True",
"*viewport*Scrollbar*thumb: None",
"*viewport*Scrollbar.translations: #override\\n\
  <Btn1Down>: StartScroll(Continuous) MoveThumb() NotifyThumb()\\n\
  <Btn1Motion>: MoveThumb() NotifyThumb()\\n\
  <Btn3Down>: StartScroll(Continuous) MoveThumb() NotifyThumb()\\n\
  <Btn3Motion>: MoveThumb() NotifyThumb()",


/*
 * Default translations on desktop window
 */

"*desktop.baseTranslations:\
  <Key>F8: ShowPopup()\\n\
  Ctrl Alt Shift <Key>F: ToggleFullScreen()\\n\
  Ctrl Alt Shift <Key>L: LosslessRefresh()\\n\
  Ctrl Alt Shift <Key>R: SendRFBEvent(fbupdate)\\n\
  <ButtonPress>: SendRFBEvent()\\n\
  <ButtonRelease>: SendRFBEvent()\\n\
  <Motion>: SendRFBEvent()\\n\
  <KeyPress>: SendRFBEvent()\\n\
  <KeyRelease>: SendRFBEvent()",


/*
 * Dialog boxes
 */

"*serverDialog.title: New TurboVNC Connection",
"*serverDialog.dialog.label: VNC server:",
"*serverDialog.dialog.value:",
"*serverDialog.dialog.value.translations: #override\\n\
  <Key>Return: ServerDialogDone()",

"*passwordDialog.title: Standard VNC Authentication",
"*passwordDialog.dialog.label: Password:",
"*passwordDialog.dialog.value:",
"*passwordDialog.dialog.value.AsciiSink.echo: False",
"*passwordDialog.dialog.value.translations: #override\\n\
  <Key>Return: PasswordDialogDone()",

"*userPwdDialog.title: Unix Login Authentication",
"*userPwdDialog.form.background: grey",
"*userPwdDialog.form.resizable: true",

"*userPwdDialog*userLabel.label: User:",

"*userPwdDialog*userField.editType: edit",
"*userPwdDialog*userField.fromHoriz: userLabel",
"*userPwdDialog*userField.resize: width",
"*userPwdDialog*userField.textSource.editType: edit",
"*userPwdDialog*userField.textSource.string: ",
"*userPwdDialog*userField.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: UserPwdSetFocus()\\n\
  Ctrl<Key>O:    Nothing()\\n\
  Meta<Key>I:    Nothing()\\n\
  Ctrl<Key>N:    Nothing()\\n\
  Ctrl<Key>P:    Nothing()\\n\
  Ctrl<Key>Z:    Nothing()\\n\
  Meta<Key>Z:    Nothing()\\n\
  Ctrl<Key>V:    Nothing()\\n\
  Meta<Key>V:    Nothing()\\n\
  <Key>Tab: UserPwdNextField()\\n\
  Ctrl<Key>J: UserPwdNextField()\\n\
  Ctrl<Key>M: UserPwdNextField()\\n\
  <Key>Linefeed: UserPwdNextField()\\n\
  <Key>Return: UserPwdNextField()",

"*userPwdDialog*pwdLabel.label: Password:",
"*userPwdDialog*pwdLabel.fromVert: userLabel",

"*userPwdDialog*pwdField.editType: edit",
"*userPwdDialog*pwdField.fromHoriz: pwdLabel",
"*userPwdDialog*pwdField.fromVert: userField",
"*userPwdDialog*pwdField.resize: width",
"*userPwdDialog*pwdField.textSink.echo: False",
"*userPwdDialog*pwdField.textSource.editType: edit",
"*userPwdDialog*pwdField.textSource.string: ",
"*userPwdDialog*pwdField.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: UserPwdSetFocus()\\n\
  Ctrl<Key>O:    Nothing()\\n\
  Meta<Key>I:    Nothing()\\n\
  Ctrl<Key>N:    Nothing()\\n\
  Ctrl<Key>P:    Nothing()\\n\
  Ctrl<Key>Z:    Nothing()\\n\
  Meta<Key>Z:    Nothing()\\n\
  Ctrl<Key>V:    Nothing()\\n\
  Meta<Key>V:    Nothing()\\n\
  <Key>Tab: UserPwdNextField()\\n\
  Ctrl<Key>J: UserPwdDialogDone()\\n\
  Ctrl<Key>M: UserPwdDialogDone()\\n\
  <Key>Linefeed: UserPwdDialogDone()\\n\
  <Key>Return: UserPwdDialogDone()",


/*
 * Popup window appearance
 */

"*qualLabel.label: JPEG Image Quality",
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
  
"*viewOnly.label: View only",

"*enableJPEG.label: Enable JPEG Compression",

"*enableZlib.label: Enable Zlib Compression",

"*popup.title: TurboVNC popup",
"*popup*background: grey",
"*popup*font: -*-helvetica-bold-r-*-*-16-*-*-*-*-*-*-*",
"*popup.buttonForm.Command.borderWidth: 0",
"*popup.buttonForm.Toggle.borderWidth: 0",


/*
 * Translations on popup window
 */

"*popup.translations: #override <Message>WM_PROTOCOLS: HidePopup()",
"*popup.buttonForm.translations: #override\\n\
  <KeyPress>: SendRFBEvent() HidePopup()",


/*
 * Popup buttons
 */

"*popupButtonCount: 15",

"*popup*button1.label: Dismiss popup",
"*popup*button1.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: HidePopup()",

"*popup*button2.label: Quit viewer",
"*popup*button2.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: Quit()",

"*popup*button3.label: Full screen (CTRL-ALT-SHIFT-F)",
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

"*popup*button6.label: Request refresh (CTRL-ALT-SHIFT-R)",
"*popup*button6.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: SendRFBEvent(fbupdate) HidePopup()",

"*popup*button7.label: Request lossless refresh (CTRL-ALT-SHIFT-L)",
"*popup*button7.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: LosslessRefresh() HidePopup()",

"*popup*button8.label: Send ctrl-alt-del",
"*popup*button8.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: SendRFBEvent(keydown,Control_L)\
                       SendRFBEvent(keydown,Alt_L)\
                       SendRFBEvent(key,Delete)\
                       SendRFBEvent(keyup,Alt_L)\
                       SendRFBEvent(keyup,Control_L)\
                       HidePopup()",

"*popup*button9.label: Send F8",
"*popup*button9.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: SendRFBEvent(key,F8) HidePopup()",

"*popup*button10.label: Continuous updates",
"*popup*button10.type: toggle",
"*popup*button10.translations: #override\\n\
  <Visible>: SetCUState()\\n\
  <Btn1Down>,<Btn1Up>: toggle() ToggleCU()",

"*popup*button11.label: Encoding method: Tight + Perceptually Lossless JPEG (LAN)",
"*popup*button11.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: QualHigh()",

"*popup*button12.label: Encoding method: Tight + Medium Quality JPEG",
"*popup*button12.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: QualMed()",

"*popup*button13.label: Encoding method: Tight + Low Quality JPEG (WAN)",
"*popup*button13.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: QualLow()",

"*popup*button14.label: Encoding method: Lossless Tight (Gigabit)",
"*popup*button14.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: QualLossless()",

"*popup*button15.label: Encoding method: Lossless Tight + Zlib (WAN)",
"*popup*button15.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: QualLosslessWAN()",

NULL
};


/*
 * vncServerHost and vncServerPort are set either from the command line or
 * from a dialog box.
 */

char vncServerHost[256] = "";
int vncServerPort = 0;


/*
 * appData is our application-specific data which can be set by the user with
 * application resource specs.  The AppData structure is defined in the header
 * file.
 */

AppData appData;

static XtResource appDataResourceList[] = {
  {"userLogin", "UserLogin", XtRString, sizeof(String),
   XtOffsetOf(AppData, userLogin), XtRImmediate, (XtPointer) 0},

  {"noUnixLogin", "NoUnixLogin", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, noUnixLogin), XtRImmediate, (XtPointer) False},

  {"shareDesktop", "ShareDesktop", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, shareDesktop), XtRImmediate, (XtPointer) True},

  {"viewOnly", "ViewOnly", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, viewOnly), XtRImmediate, (XtPointer) False},

  {"fullScreen", "FullScreen", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, fullScreen), XtRImmediate, (XtPointer) False},

  {"fsAltEnter", "FSAltEnter", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, fsAltEnter), XtRImmediate, (XtPointer) False},

  {"raiseOnBeep", "RaiseOnBeep", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, raiseOnBeep), XtRImmediate, (XtPointer) True},

  {"passwordFile", "PasswordFile", XtRString, sizeof(String),
   XtOffsetOf(AppData, passwordFile), XtRImmediate, (XtPointer) 0},

  {"passwordDialog", "PasswordDialog", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, passwordDialog), XtRImmediate, (XtPointer) False},

  {"encodings", "Encodings", XtRString, sizeof(String),
   XtOffsetOf(AppData, encodingsString), XtRImmediate, (XtPointer) 0},

  {"useBGR233", "UseBGR233", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, useBGR233), XtRImmediate, (XtPointer) False},

  {"nColours", "NColours", XtRInt, sizeof(int),
   XtOffsetOf(AppData, nColours), XtRImmediate, (XtPointer) 256},

  {"useSharedColours", "UseSharedColours", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, useSharedColours), XtRImmediate, (XtPointer) True},

  {"forceOwnCmap", "ForceOwnCmap", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, forceOwnCmap), XtRImmediate, (XtPointer) False},

  {"forceTrueColour", "ForceTrueColour", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, forceTrueColour), XtRImmediate, (XtPointer) False},

  {"requestedDepth", "RequestedDepth", XtRInt, sizeof(int),
   XtOffsetOf(AppData, requestedDepth), XtRImmediate, (XtPointer) 0},

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

  {"compressLevel", "CompressionLevel", XtRInt, sizeof(int),
   XtOffsetOf(AppData, compressLevel), XtRImmediate, (XtPointer) -1},

  {"subsampling", "Subsampling", XtRString, sizeof(String),
   XtOffsetOf(AppData, subsampString), XtRImmediate, (XtPointer) "1x"},

  {"quality", "Quality", XtRInt, sizeof(int),
   XtOffsetOf(AppData, qualityLevel), XtRImmediate, (XtPointer) 95},

  {"enableJPEG", "EnableJPEG", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, enableJPEG), XtRImmediate, (XtPointer) True},

  {"useRemoteCursor", "UseRemoteCursor", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, useRemoteCursor), XtRImmediate, (XtPointer) True},

  {"useRichCursor", "UseRichCursor", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, useRichCursor), XtRImmediate, (XtPointer) True},

  {"grabKeyboard", "GrabKeyboard", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, grabKeyboard), XtRImmediate, (XtPointer) True},

  {"doubleBuffer", "DoubleBuffer", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, doubleBuffer), XtRImmediate, (XtPointer) True},

  {"autoPass", "AutoPass", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, autoPass), XtRImmediate, (XtPointer) False},

  {"continuousUpdates", "ContinuousUpdates", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, continuousUpdates), XtRImmediate, (XtPointer) False},

  {"configFile", "ConfigFile", XtRString, sizeof(String),
   XtOffsetOf(AppData, configFile), XtRImmediate, (XtPointer) 0},
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
  {"-fsaltenter",    "*fsAltEnter",         XrmoptionNoArg,  "True"},
  {"-noraiseonbeep", "*raiseOnBeep",        XrmoptionNoArg,  "False"},
  {"-passwd",        "*passwordFile",       XrmoptionSepArg, 0},
  {"-encodings",     "*encodings",          XrmoptionSepArg, 0},
  {"-bgr233",        "*useBGR233",          XrmoptionNoArg,  "True"},
  {"-owncmap",       "*forceOwnCmap",       XrmoptionNoArg,  "True"},
  {"-truecolor",     "*forceTrueColour",    XrmoptionNoArg,  "True"},
  {"-truecolour",    "*forceTrueColour",    XrmoptionNoArg,  "True"},
  {"-depth",         "*requestedDepth",     XrmoptionSepArg, 0},
  {"-samp",          "*subsampling",        XrmoptionSepArg, 0},
  {"-compresslevel", "*compressLevel",      XrmoptionSepArg, 0},
  {"-quality",       "*quality",            XrmoptionSepArg, 0},
  {"-nojpeg",        "*enableJPEG",         XrmoptionNoArg,  "False"},
  {"-nocursorshape", "*useRemoteCursor",    XrmoptionNoArg,  "False"},
  {"-norichcursor",  "*useRichCursor",      XrmoptionNoArg,  "False"},
  {"-singlebuffer",  "*doubleBuffer",       XrmoptionNoArg,  "False"},
  {"-lowqual",       "*quality",            XrmoptionNoArg,  "-1"},
  {"-medqual",       "*quality",            XrmoptionNoArg,  "-2"},
  {"-lossless",      "*quality",            XrmoptionNoArg,  "-3"},
  {"-losslesswan",   "*quality",            XrmoptionNoArg,  "-4"},
  {"-autopass",      "*autoPass",           XrmoptionNoArg,  "True"},
  {"-user",          "*userLogin",          XrmoptionSepArg,  0},
  {"-nounixlogin",   "*noUnixLogin",        XrmoptionNoArg,  "True"},
  {"-cu",            "*continuousUpdates",  XrmoptionNoArg,  "True"},
  {"-config",        "*configFile",         XrmoptionSepArg, 0}
};

int numCmdLineOptions = XtNumber(cmdLineOptions);


/*
 * actions[] specifies actions that can be used in widget resource specs.
 */

static XtActionsRec actions[] = {
    {"SendRFBEvent", SendRFBEvent},
    {"QualHigh", QualHigh},
    {"QualLow", QualLow},
    {"QualMed", QualMed},
    {"QualLossless", QualLossless},
    {"QualLosslessWAN", QualLosslessWAN},
    {"LosslessRefresh", LosslessRefresh},
    {"ShowPopup", ShowPopup},
    {"HidePopup", HidePopup},
    {"ToggleFullScreen", ToggleFullScreen},
    {"SetFullScreenState", SetFullScreenState},
    {"SelectionFromVNC", SelectionFromVNC},
    {"SelectionToVNC", SelectionToVNC},
    {"ServerDialogDone", ServerDialogDone},
    {"PasswordDialogDone", PasswordDialogDone},
    {"Nothing", NULL},
    {"UserPwdDialogDone", UserPwdDialogDone},
    {"UserPwdNextField", UserPwdNextField},
    {"UserPwdSetFocus", UserPwdSetFocus},
    {"Pause", Pause},
    {"RunCommand", RunCommand},
    {"Quit", Quit},
    {"SetCUState", SetCUState},
    {"ToggleCU", ToggleCU}
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
	  "TurboVNC Viewer %d-bit v"__VERSION" (build "__BUILD")\n"
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
          "        -fsaltenter\n"
	  "        -noraiseonbeep\n"
	  "        -passwd <PASSWD-FILENAME> (standard VNC authentication)\n"
	  "        -encodings <ENCODING-LIST> (example: \"tight copyrect\")\n"
	  "        -bgr233\n"
	  "        -owncmap\n"
	  "        -truecolour\n"
	  "        -depth <DEPTH>\n"
	  "        -compresslevel <ZLIB COMPRESSION LEVEL> (0..1: 0-fast, 1-best)\n"
	  "        -samp <JPEG CHROMINANCE SUBSAMPLING> (1x | 2x | 4x | gray)\n"
	  "        -quality <JPEG IMAGE QUALITY> (1..100: 1-low, 100-high)\n"
	  "        -nojpeg\n"
	  "        -nocursorshape\n"
	  "        -autopass\n"
	  "        -singlebuffer\n"
	  "        -lowqual (preset for -samp 4x -quality 30)\n"
	  "        -medqual (preset for -samp 2x -quality 80)\n"
	  "        -lossless (preset for -nojpeg -compresslevel 0)\n"
	  "        -losslesswan (preset for -nojpeg -compresslevel 1)\n"
	  "        -user <USER NAME> (Unix login authentication)\n"
	  "        -nounixlogin\n"
	  "        -cu\n"
	  "        -config <CONFIG-FILENAME>\n"
	  "        -ipv6\n"
	  "\n"
	  "Option names may be abbreviated, for example, -q instead of -quality.\n"
	  "See the manual page for more information."
	  "\n", (int)sizeof(size_t)*8, programName, programName, programName, programName);
  exit(1);
}


/*
 * Load connection info from file
 */

#define ReadConfigBool(str, var) {  \
  n = strlen(str);  \
  if (!strncmp(buf2, str, n)) {  \
    int temp = 0;  \
    if (sscanf(&buf2[n], "%d", &temp) == 1) {  \
      if (temp) var = True;  \
      else var = False;  \
    }  \
    continue;  \
  }  \
}

#define ReadConfigInt(str, var, min, max) {  \
  n = strlen(str);  \
  if (!strncmp(buf2, str, n)) {  \
    int temp = -1;  \
    if (sscanf(&buf2[n], "%d", &temp) == 1 && temp >= min && temp <= max)  \
      var = temp;  \
    continue;  \
  }  \
}

static char encodings[256] = "";
static const char *encodingString[17] = {
  "raw", "copyrect", "rre", "", "corre", "hextile", "zlib", "tight",
  "zlibhex", "", "", "", "", "", "", "", "zrle"
};

char encryptedPassword[9] = "";

void
LoadConfigFile(char *filename)
{
  FILE *fp;
  char buf[256], buf2[256];
  int  line, len, n, i, j, preferred_encoding = -1;
  Bool use_encoding[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  char passwordString[256] = "";

  if ((fp = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "ERROR: Cannot read connection info from %s\n", filename);
    exit(1);
  }

  for (line = 0; fgets(buf, sizeof(buf), fp) != NULL; line++) {
    len = strlen(buf) - 1;
    if (buf[len] != '\n' && strlen(buf) == 255) {
      fprintf(stderr, "ERROR in %s: line %d is too long!\n", filename, line+1);
      exit(1);
    }
    buf[len] = '\0';

    for (i = 0, j = 0; i < len; i++) {
      if (buf[i] != ' ' && buf[i] != '\t' && buf[i] != '\r' && buf[i] != '\n')
        buf2[j++] = buf[i];
    }
    len = j;
    buf2[len] = '\0';
    if (len < 1) continue;

    n = 5;
    if (!strncmp(buf2, "host=", n)) {
      if (buf2[n] == '\0') {
        fprintf(stderr, "ERROR reading host name from %s\n", filename);
        exit(1);
      }
      strncpy(vncServerHost, &buf2[n], 255-n);
      continue;
    }

    n = 5;
    if (!strncmp(buf2, "port=", n)) {
      if (buf2[n] == '\0' || sscanf(&buf2[n], "%d", &vncServerPort) < 1) {
        fprintf(stderr, "ERROR reading port number from %s\n", filename);
        exit(1);
      }
      if (vncServerPort < 1 || vncServerPort > 65535) {
        fprintf(stderr, "ERROR in %s: port number must be between 1 and 65535\n",
          filename);
        exit(1);
      }
      continue;
    }

    n = 9;
    if (!strncmp(buf2, "password=", n)) {
      if (buf2[n] != '\0') {
        strncpy(passwordString, &buf2[n], 255-n);
        passwordString[16] = 0;
        for (i = 0; i < strlen(passwordString); i += 2) {
          char temps[3];  int temp;
          strncpy(temps, &passwordString[i], 2);
          temps[2] = 0;
          if (sscanf(temps, "%x", &temp) == 1)
            encryptedPassword[i/2] = (char)temp;
          else break;
        }
        encryptedPassword[i/2] = 0;
      }
      continue;
    }

    for (i = 0; i < 17; i++) {
      char token[17];
      snprintf(token, 16, "use_encoding_%d=", i);
      token[16] = 0;
      ReadConfigBool(token, use_encoding[i]);
    }

    ReadConfigInt("preferred_encoding=", preferred_encoding, 0, 16);
    ReadConfigBool("viewonly=", appData.viewOnly);
    ReadConfigBool("fullscreen=", appData.fullScreen);
    ReadConfigBool("fsaltenter=", appData.fsAltEnter);
    ReadConfigBool("8bit=", appData.useBGR233);
    ReadConfigBool("doublebuffer=", appData.doubleBuffer);
    ReadConfigBool("shared=", appData.shareDesktop);
    ReadConfigBool("continuousupdates=", appData.continuousUpdates);
    ReadConfigBool("belldeiconify=", appData.raiseOnBeep);
    ReadConfigInt("compresslevel=", appData.compressLevel, 0, 9);
    ReadConfigInt("subsampling=", appData.subsampLevel, 0, 3);
    ReadConfigInt("quality=", appData.qualityLevel, 1, 100);
    ReadConfigBool("nounixlogin=", appData.noUnixLogin);
  }

  if (preferred_encoding >= 0 && preferred_encoding <= 16
    && use_encoding[preferred_encoding]) {
    strncat(encodings, encodingString[preferred_encoding],
      255 - strlen(encodings));
  }
  for (i = 0; i < 17; i++) {
    if (use_encoding[i] && i != preferred_encoding) {
      strncat(encodings, " ", 255 - strlen(encodings));
      strncat(encodings, encodingString[i], 255 - strlen(encodings));
    }
  }
  if (strlen(encodings) > 0) appData.encodingsString = encodings;

  if (strlen(vncServerHost) == 0) {
    fprintf(stderr, "ERROR reading host name from %s\n", filename);
    exit(1);
  }
  if (vncServerPort == 0) {
    fprintf(stderr, "ERROR reading port number from %s\n", filename);
    exit(1);
  }

  fclose(fp);
}


/*
 * GetArgsAndResources() deals with resources and any command-line arguments
 * not already processed by XtVaAppInitialize().  It sets vncServerHost and
 * vncServerPort and all the fields in appData.
 */

void
GetArgsAndResources(int argc, char **argv)
{
  char *vncServerName, *colonPos;
  int len, portOffset;
  int disp;

  /* Turn app resource specs into our appData structure for the rest of the
     program to use */

  XtGetApplicationResources(toplevel, &appData, appDataResourceList,
			    XtNumber(appDataResourceList), 0, 0);

  appData.subsampLevel = -1;

  if(appData.configFile) LoadConfigFile(appData.configFile);
  else if(argc > 1 && strlen(argv[1]) >= 4
    && !strncmp(&argv[1][strlen(argv[1])-4], ".vnc", 4)) {
    appData.configFile = argv[1];
    LoadConfigFile(appData.configFile);
  }

  if (appData.subsampString && appData.subsampLevel < 0) {
    switch(toupper(appData.subsampString[0])) {
      case 'G': case '0':  appData.subsampLevel=TVNC_GRAY;  break;
      case '1':  appData.subsampLevel=TVNC_1X;  break;
      case '2':  appData.subsampLevel=TVNC_2X;  break;
      case '4':  appData.subsampLevel=TVNC_4X;  break;
    }
  }

  /* -lowqual switch was used */
  if(appData.qualityLevel==-1) {
    appData.encodingsString="tight copyrect";
    appData.enableJPEG=True;
    appData.qualityLevel=30;
    appData.subsampLevel=TVNC_4X;
  }
  /* -medqual switch was used */
  else if(appData.qualityLevel==-2) {
    appData.encodingsString="tight copyrect";
    appData.enableJPEG=True;
    appData.qualityLevel=80;
    appData.subsampLevel=TVNC_2X;
  }
  /* -lossless switch was used */
  if(appData.qualityLevel==-3) {
    appData.encodingsString="tight copyrect";
    appData.enableJPEG=False;
    appData.qualityLevel=95;
    appData.compressLevel=0;
  }
  /* -losslesswan switch was used */
  if(appData.qualityLevel==-4) {
    appData.encodingsString="tight copyrect";
    appData.enableJPEG=False;
    appData.qualityLevel=95;
    appData.compressLevel=1;
  }

  if(appData.useBGR233) {
    if(appData.enableJPEG)
      fprintf(stderr, "WARNING: Disabling JPEG encoding because BGR233 is enabled.\n");
    appData.enableJPEG=False;
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

  if (strlen(vncServerHost) == 0 && vncServerPort < 1) {

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

    if (strlen(vncServerName) > 255) {
      fprintf(stderr,"VNC server name too long\n");
      exit(1);
    }

    colonPos = strrchr(vncServerName, ':');
    while (colonPos > vncServerName && *(colonPos - 1) == ':')
      colonPos--;
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
}
