/*
 *  Copyright (C) 2009-2013 D. R. Commander.  All Rights Reserved.
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

"Tvncviewer.title: %s",


/*
 * Translations on the main window
 */

"Tvncviewer.translations:\
  <Enter>: SelectionToVNC()\\n\
  <Leave>: SelectionFromVNC()",


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
  Ctrl Alt Shift <Key>G: ToggleGrabKeyboard()\\n\
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
"*qualLabelLo.label: poor",
"*qualLabelHi.label: best",
"*qualBar.length: 100",
"*qualBar.width: 130",
"*qualBar.orientation: horizontal",
"*qualBar.translations: #override\\n\
  <Btn1Down>: StartScroll(Continuous) MoveThumb() NotifyThumb()\\n\
  <Btn1Motion>: MoveThumb() NotifyThumb()\\n\
  <Btn3Down>: StartScroll(Continuous) MoveThumb() NotifyThumb()\\n\
  <Btn3Motion>: MoveThumb() NotifyThumb()",

"*qualText.label: 000",

"*subsampLabel.label: JPEG Chrominance Subsampling",
"*subsampLabelLo.label: fast",
"*subsampLabelHi.label: best",
"*subsampGray.label: Grayscale",
"*subsamp4X.label: 4X",
"*subsamp2X.label: 2X",
"*subsamp1X.label: None",

"*enableJPEG.label: Enable JPEG Compression",

"*zlibCompressLabel.label: Zlib Compression Level",
"*compressLabel.label: Compression Level",
"*compressLabelLo.label: fast",
"*compressLabelHi.label: best",

"*popup.title: TurboVNC Viewer Options",
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

"*popupButtonCount: 16",

"*popup*button1.label: Dismiss menu",
"*popup*button1.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: HidePopup()",

"*popup*button2.label: Close viewer",
"*popup*button2.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: Quit()",

"*popup*button3.label: Request refresh (Ctrl-Alt-Shift-R)",
"*popup*button3.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: SendRFBEvent(fbupdate) HidePopup()",

"*popup*button4.label: Request lossless refresh (Ctrl-Alt-Shift-L)",
"*popup*button4.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: LosslessRefresh() HidePopup()",

"*popup*button5.label: Full screen (Ctrl-Alt-Shift-F)",
"*popup*button5.type: toggle",
"*popup*button5.translations: #override\\n\
  <Visible>: SetFullScreenState()\\n\
  <Btn1Down>,<Btn1Up>: ToggleFullScreen() HidePopup()",

"*popup*button6.label: Grab keyboard (Ctrl-Alt-Shift-G)",
"*popup*button6.type: toggle",
"*popup*button6.translations: #override\\n\
  <Visible>: SetGrabKeyboardState()\\n\
  <Btn1Down>,<Btn1Up>: ToggleGrabKeyboard() HidePopup()",

"*popup*button7.label: Send F8",
"*popup*button7.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: SendRFBEvent(key,F8) HidePopup()",

"*popup*button8.label: Send Ctrl-Alt-Del",
"*popup*button8.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: SendRFBEvent(keydown,Control_L)\
                       SendRFBEvent(keydown,Alt_L)\
                       SendRFBEvent(key,Delete)\
                       SendRFBEvent(keyup,Alt_L)\
                       SendRFBEvent(keyup,Control_L)\
                       HidePopup()",

"*popup*button9.label: View only",
"*popup*button9.type: toggle",
"*popup*button9.translations: #override\\n\
  <Visible>: SetViewOnlyState()\\n\
  <Btn1Down>,<Btn1Up>: ToggleViewOnly() HidePopup()",

"*popup*button10.label: Clipboard: local -> remote",
"*popup*button10.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: SelectionToVNC(always) HidePopup()",

"*popup*button11.label: Clipboard: local <- remote",
"*popup*button11.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: SelectionFromVNC(always) HidePopup()",

"*popup*button12.label: Encoding method: Tight + Perceptually Lossless JPEG (LAN)",
"*popup*button12.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: QualHigh()",

"*popup*button13.label: Encoding method: Tight + Medium-Quality JPEG",
"*popup*button13.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: QualMed()",

"*popup*button14.label: Encoding method: Tight + Low-Quality JPEG (WAN)",
"*popup*button14.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: QualLow()",

"*popup*button15.label: Encoding method: Lossless Tight (Gigabit)",
"*popup*button15.translations: #override\\n\
  <Btn1Down>,<Btn1Up>: QualLossless()",

"*popup*button16.label: Encoding method: Lossless Tight + Zlib (WAN)",
"*popup*button16.translations: #override\\n\
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
  {"shared", "Shared", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, shareDesktop), XtRImmediate, (XtPointer) True},

  {"viewOnly", "ViewOnly", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, viewOnly), XtRImmediate, (XtPointer) False},

  {"fullScreen", "FullScreen", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, fullScreen), XtRImmediate, (XtPointer) False},

  {"fsAltEnter", "FSAltEnter", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, fsAltEnter), XtRImmediate, (XtPointer) False},

  {"grabKeyboard", "GrabKeyboard", XtRString, sizeof(String),
   XtOffsetOf(AppData, grabKeyboardString), XtRImmediate, (XtPointer) "fs"},

  {"doubleBuffer", "DoubleBuffer", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, doubleBuffer), XtRImmediate, (XtPointer) True},

  {"sharedMemory", "SharedMemory", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, useShm), XtRImmediate, (XtPointer) True},

  {"raiseOnBeep", "RaiseOnBeep", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, raiseOnBeep), XtRImmediate, (XtPointer) True},

  {"8Bit", "8Bit", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, useBGR233), XtRImmediate, (XtPointer) False},

  {"nColors", "NColors", XtRInt, sizeof(int),
   XtOffsetOf(AppData, nColors), XtRImmediate, (XtPointer) 256},

  {"SharedColors", "SharedColors", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, useSharedColors), XtRImmediate, (XtPointer) True},

  {"forceOwnCmap", "ForceOwnCmap", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, forceOwnCmap), XtRImmediate, (XtPointer) False},

  {"forceTrueColor", "ForceTrueColor", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, forceTrueColor), XtRImmediate, (XtPointer) False},

  {"requestedDepth", "RequestedDepth", XtRInt, sizeof(int),
   XtOffsetOf(AppData, requestedDepth), XtRImmediate, (XtPointer) 0},

  {"encodings", "Encodings", XtRString, sizeof(String),
   XtOffsetOf(AppData, encodingsString), XtRImmediate, (XtPointer) 0},

  {"JPEG", "JPEG", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, enableJPEG), XtRImmediate, (XtPointer) True},

  {"quality", "Quality", XtRInt, sizeof(int),
   XtOffsetOf(AppData, qualityLevel), XtRImmediate, (XtPointer) 95},

  {"subsampling", "Subsampling", XtRString, sizeof(String),
   XtOffsetOf(AppData, subsampString), XtRImmediate, (XtPointer) "1x"},

  {"compressLevel", "CompressLevel", XtRInt, sizeof(int),
   XtOffsetOf(AppData, compressLevel), XtRImmediate, (XtPointer) -1},

  {"continuousUpdates", "ContinuousUpdates", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, continuousUpdates), XtRImmediate, (XtPointer) True},

  {"cursorShape", "CursorShape", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, cursorShape), XtRImmediate, (XtPointer) True},

  {"richCursor", "RichCursor", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, useRichCursor), XtRImmediate, (XtPointer) True},

  {"userLogin", "UserLogin", XtRString, sizeof(String),
   XtOffsetOf(AppData, userLogin), XtRImmediate, (XtPointer) 0},

  {"noUnixLogin", "NoUnixLogin", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, noUnixLogin), XtRImmediate, (XtPointer) False},

  {"passwordFile", "PasswordFile", XtRString, sizeof(String),
   XtOffsetOf(AppData, passwordFile), XtRImmediate, (XtPointer) 0},

  {"autoPass", "AutoPass", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, autoPass), XtRImmediate, (XtPointer) False},

  {"passwordDialog", "PasswordDialog", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, passwordDialog), XtRImmediate, (XtPointer) False},

  {"configFile", "ConfigFile", XtRString, sizeof(String),
   XtOffsetOf(AppData, configFile), XtRImmediate, (XtPointer) 0},

  {"wmDecorationWidth", "WmDecorationWidth", XtRInt, sizeof(int),
   XtOffsetOf(AppData, wmDecorationWidth), XtRImmediate, (XtPointer) 4},

  {"wmDecorationHeight", "WmDecorationHeight", XtRInt, sizeof(int),
   XtOffsetOf(AppData, wmDecorationHeight), XtRImmediate, (XtPointer) 24},

  {"bumpScrollTime", "BumpScrollTime", XtRInt, sizeof(int),
   XtOffsetOf(AppData, bumpScrollTime), XtRImmediate, (XtPointer) 25},

  {"bumpScrollPixels", "BumpScrollPixels", XtRInt, sizeof(int),
   XtOffsetOf(AppData, bumpScrollPixels), XtRImmediate, (XtPointer) 20},

  {"popupButtonCount", "PopupButtonCount", XtRInt, sizeof(int),
   XtOffsetOf(AppData, popupButtonCount), XtRImmediate, (XtPointer) 0},

  {"debug", "Debug", XtRBool, sizeof(Bool),
   XtOffsetOf(AppData, debug), XtRImmediate, (XtPointer) False},

  {"rawDelay", "RawDelay", XtRInt, sizeof(int),
   XtOffsetOf(AppData, rawDelay), XtRImmediate, (XtPointer) 0},

  {"copyRectDelay", "CopyRectDelay", XtRInt, sizeof(int),
   XtOffsetOf(AppData, copyRectDelay), XtRImmediate, (XtPointer) 0},
};


/*
 * The cmdLineOptions array specifies how certain app resource specs can be set
 * with command-line options.
 */

XrmOptionDescRec cmdLineOptions[] = {
  {"-shared",        "*shared",             XrmoptionNoArg,  "True"},
  {"-noshared",      "*shared",             XrmoptionNoArg,  "False"},
  {"-viewonly",      "*viewOnly",           XrmoptionNoArg,  "True"},
  {"-fullcontrol",   "*viewOnly",           XrmoptionNoArg,  "False"},
  {"-fullscreen",    "*fullScreen",         XrmoptionNoArg,  "True"},
  {"-nofullscreen",  "*fullScreen",         XrmoptionNoArg,  "False"},
  {"-fsaltenter",    "*fsAltEnter",         XrmoptionNoArg,  "True"},
  {"-nofsaltenter",  "*fsAltEnter",         XrmoptionNoArg,  "False"},
  {"-grabkeyboard",  "*grabKeyboard",       XrmoptionSepArg, 0},
  {"-raiseonbeep",   "*raiseOnBeep",        XrmoptionNoArg,  "True"},
  {"-noraiseonbeep", "*raiseOnBeep",        XrmoptionNoArg,  "False"},
  {"-passwd",        "*passwordFile",       XrmoptionSepArg, 0},
  {"-encodings",     "*encodings",          XrmoptionSepArg, 0},
  {"-8bit",          "*8Bit",               XrmoptionNoArg,  "True"},
  {"-no8bit",        "*8Bit",               XrmoptionNoArg,  "False"},
  {"-owncmap",       "*forceOwnCmap",       XrmoptionNoArg,  "True"},
  {"-truecolor",     "*forceTrueColor",     XrmoptionNoArg,  "True"},
  {"-depth",         "*requestedDepth",     XrmoptionSepArg, 0},
  {"-samp",          "*subsampling",        XrmoptionSepArg, 0},
  {"-compresslevel", "*compressLevel",      XrmoptionSepArg, 0},
  {"-quality",       "*quality",            XrmoptionSepArg, 0},
  {"-jpeg",          "*JPEG",               XrmoptionNoArg,  "True"},
  {"-nojpeg",        "*JPEG",               XrmoptionNoArg,  "False"},
  {"-cursorshape",   "*cursorShape",        XrmoptionNoArg,  "True"},
  {"-nocursorshape", "*cursorShape",        XrmoptionNoArg,  "False"},
  {"-richcursor",    "*richCursor",         XrmoptionNoArg,  "True"},
  {"-norichcursor",  "*richCursor",         XrmoptionNoArg,  "False"},
  {"-doublebuffer",  "*doubleBuffer",       XrmoptionNoArg,  "True"},
  {"-singlebuffer",  "*doubleBuffer",       XrmoptionNoArg,  "False"},
  {"-lowqual",       "*quality",            XrmoptionNoArg,  "-1"},
  {"-medqual",       "*quality",            XrmoptionNoArg,  "-2"},
  {"-highqual",      "*quality",            XrmoptionNoArg,  "-3"},
  {"-lossless",      "*quality",            XrmoptionNoArg,  "-4"},
  {"-losslesswan",   "*quality",            XrmoptionNoArg,  "-5"},
  {"-autopass",      "*autoPass",           XrmoptionNoArg,  "True"},
  {"-user",          "*userLogin",          XrmoptionSepArg,  0},
  {"-nounixlogin",   "*noUnixLogin",        XrmoptionNoArg,  "True"},
  {"-cu",            "*continuousUpdates",  XrmoptionNoArg,  "True"},
  {"-nocu",          "*continuousUpdates",  XrmoptionNoArg,  "False"},
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
  {"ToggleGrabKeyboard", ToggleGrabKeyboard},
  {"SetGrabKeyboardState", SetGrabKeyboardState},
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
  {"SetViewOnlyState", SetViewOnlyState},
  {"ToggleViewOnly", ToggleViewOnly}
};


/*
 * removeArgs() is used to remove some of the command-line arguments.
 */

void removeArgs(int *argc, char** argv, int idx, int nargs)
{
  int i;
  if ((idx + nargs) > *argc) return;
  for (i = idx + nargs; i < *argc; i++) {
    argv[i - nargs] = argv[i];
  }
  *argc -= nargs;
}


void usage(void)
{
  fprintf(stderr,
          "\nTurboVNC Viewer %d-bit v"__VERSION" (build "__BUILD")\n"
          "Copyright (C) "__COPYRIGHT_YEAR" "__COPYRIGHT"\n"
          __URLTEXT"\n"
          "\n"
          "Usage: %s [<OPTIONS>] [<HOST>][:<DISPLAY#>]\n"
          "       %s [<OPTIONS>] [<HOST>][::<PORT#>]\n"
          "       %s [<OPTIONS>] -listen [<DISPLAY#>]\n"
          "       %s -help\n"
          "\n"
          "<OPTIONS> are standard Xt options, or:\n"
          "        -ipv6\n"
          "        -shared (default) / -noshared\n"
          "        -viewonly / -fullcontrol (default)\n"
          "        -fullscreen / -nofullscreen (default)\n"
          "        -fsaltenter / -nofsaltenter (default)\n"
          "        -grabkeyboard <fs | always | manual> (default=fs)\n"
          "        -doublebuffer (default) / -singlebuffer\n"
          "        -raiseonbeep (default) / -noraiseonbeep\n"
          "        -8bit / -no8bit (default)\n"
          "        -owncmap\n"
          "        -truecolor\n"
          "        -depth <DEPTH>\n"
          "        -encodings <ENCODING-LIST> (example: \"tight copyrect\")\n"
          "        -jpeg (default) / -nojpeg\n"
          "        -quality <JPEG IMAGE QUALITY> (1..100, 1=low, 100=high, default=95)\n"
          "        -samp <JPEG CHROMINANCE SUBSAMPLING> <1x | 2x | 4x | gray> (default=1x)\n"
          "        -compresslevel <ZLIB COMPRESSION LEVEL>\n"
          "                       (0..1, 0=fast, 1=best, default=1)\n"
          "        -lowqual (preset for -jpeg -samp 4x -quality 30)\n"
          "        -medqual (preset for -jpeg -samp 2x -quality 80)\n"
          "        -highqual (preset for -jpeg -samp 1x -quality 95)\n"
          "        -lossless (preset for -nojpeg -compresslevel 0)\n"
          "        -losslesswan (preset for -nojpeg -compresslevel 1)\n"
          "        -cursorshape (default) / -nocursorshape\n"
          "        -user <USER NAME> (Unix login authentication)\n"
          "        -nounixlogin\n"
          "        -passwd <PASSWD-FILENAME> (standard VNC authentication)\n"
          "        -autopass\n"
          "        -via <GATEWAY>\n"
          "        -tunnel\n"
          "        -config <CONFIG-FILENAME>\n"
          "\n"
          "Option names may be abbreviated (for example: -q instead of -quality.)\n"
          "See the manual page for more information.\n"
          "\n", (int)sizeof(size_t) * 8, programName, programName, programName,
          programName);
  exit(1);
}


/*
 * Load connection info from file
 */

#define ReadConfigBool(str, var) {  \
  n = strlen(str);  \
  if (!strncasecmp(buf2, str, n)) {  \
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
  if (!strncasecmp(buf2, str, n)) {  \
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
Bool encryptedPasswordSet = FALSE;

void LoadConfigFile(char *filename)
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
    if (!strncasecmp(buf2, "host=", n)) {
      if (buf2[n] == '\0') {
        fprintf(stderr, "ERROR reading host name from %s\n", filename);
        exit(1);
      }
      strncpy(vncServerHost, &buf2[n], 255 - n);
      continue;
    }

    n = 5;
    if (!strncasecmp(buf2, "port=", n)) {
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
    if (!strncasecmp(buf2, "password=", n)) {
      if (buf2[n] != '\0') {
        strncpy(passwordString, &buf2[n], 255 - n);
        passwordString[16] = 0;
        if (strlen(passwordString) != 16) {
          fprintf(stderr, "Password stored in connection info file is invalid.\n");
          exit(1);
        }
        for (i = 0; i < 16; i += 2) {
          char temps[3];  int temp;
          strncpy(temps, &passwordString[i], 2);
          temps[2] = 0;
          if (sscanf(temps, "%x", &temp) == 1)
            encryptedPassword[i/2] = (char)temp;
          else break;
        }
        if (i == 16) encryptedPasswordSet = TRUE;
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
    ReadConfigBool("cursorshape=", appData.cursorShape);
    ReadConfigInt("grabkeyboard=", appData.grabKeyboard, 0, 2);
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

void GetArgsAndResources(int argc, char **argv)
{
  char *vncServerName = NULL, *colonPos;
  int len, portOffset;
  int disp;

  /* Turn app resource specs into our appData structure for the rest of the
     program to use */

  XtGetApplicationResources(toplevel, &appData, appDataResourceList,
                            XtNumber(appDataResourceList), 0, 0);

  appData.subsampLevel = -1;
  appData.grabKeyboard = TVNC_FS;

  if (appData.configFile) LoadConfigFile(appData.configFile);
  else if (argc > 1 && strlen(argv[1]) >= 4
    && !strncmp(&argv[1][strlen(argv[1])-4], ".vnc", 4)) {
    appData.configFile = argv[1];
    LoadConfigFile(appData.configFile);
  }

  if (appData.subsampString && strlen(appData.subsampString) > 0 &&
      appData.subsampLevel < 0) {
    switch(toupper(appData.subsampString[0])) {
      case 'G': case '0':  appData.subsampLevel = TVNC_GRAY;  break;
      case '1':  appData.subsampLevel = TVNC_1X;  break;
      case '2':  appData.subsampLevel = TVNC_2X;  break;
      case '4':  appData.subsampLevel = TVNC_4X;  break;
    }
  }

  if (appData.grabKeyboardString && strlen(appData.grabKeyboardString) > 0) {
    char *s = appData.grabKeyboardString;
    char first = toupper(s[0]);
    if (first == 'A') // "always"
      appData.grabKeyboard = TVNC_ALWAYS;
    else if (first == '0' ||
             (first == 'F' && strlen(s) > 1 && toupper(s[1]) != 'S') ||
             first == 'M')  // "0" or "false" or "manual"
      appData.grabKeyboard = TVNC_MANUAL;
  }

  /* -lowqual switch was used */
  if (appData.qualityLevel == -1) {
    appData.encodingsString = "tight copyrect";
    appData.enableJPEG = True;
    appData.qualityLevel = 30;
    appData.subsampLevel = TVNC_4X;
  }
  /* -medqual switch was used */
  else if (appData.qualityLevel == -2) {
    appData.encodingsString = "tight copyrect";
    appData.enableJPEG = True;
    appData.qualityLevel = 80;
    appData.subsampLevel = TVNC_2X;
  }
  /* -highqual switch was used */
  else if (appData.qualityLevel == -3) {
    appData.encodingsString = "tight copyrect";
    appData.enableJPEG = True;
    appData.qualityLevel = 95;
    appData.subsampLevel = TVNC_1X;
  }
  /* -lossless switch was used */
  else if (appData.qualityLevel == -4) {
    appData.encodingsString = "tight copyrect";
    appData.enableJPEG = False;
    appData.qualityLevel = 95;
    appData.compressLevel = 0;
  }
  /* -losslesswan switch was used */
  else if (appData.qualityLevel == -5) {
    appData.encodingsString = "tight copyrect";
    appData.enableJPEG = False;
    appData.qualityLevel = 95;
    appData.compressLevel = 1;
  }

  /* Add our actions to the actions table so they can be used in widget
     resource specs */

  XtAppAddActions(appContext, actions, XtNumber(actions));
  if (benchFile) return;

  /* Check any remaining command-line arguments.  If -listen was specified,
     there should be none.  Otherwise, the only argument should be the VNC
     server name.  If not given, then pop up a dialog box and wait for the
     server name to be entered. */

  if (listenSpecified) {
    if (argc != 1) {
      fprintf(stderr, "\n%s -listen: invalid command line argument: %s\n",
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
      fprintf(stderr, "VNC server name too long\n");
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
