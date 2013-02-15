//  Copyright (C) 2010-2013 D. R. Commander. All Rights Reserved.
//  Copyright (C) 2005-2006 Sun Microsystems, Inc. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
//  USA.


#ifndef VNCOPTIONS_H__
#define VNCOPTIONS_H__

#pragma once

#define LASTENCODING rfbEncodingZlibHex

#define NOCURSOR 0
#define DOTCURSOR 1
#define NORMALCURSOR 2
#define SMALLCURSOR 3
#define MAX_LEN_COMBO 5

#define SPAN_OPTS 3
enum { SPAN_PRIMARY = 0, SPAN_ALL, SPAN_AUTO };

#define KEY_VNCVIEWER_HISTORY "Software\\TurboVNC\\VNCviewer\\History"


struct COMBOSTRING {
  char NameString[41];
  int rfbEncoding;
  bool enableJpeg;
  int subsampLevel;
  int qualityLevel;
  int compressLevel;
};


class VNCOptions
{
  public:
    VNCOptions();
    VNCOptions& operator = (VNCOptions& s);
    virtual ~VNCOptions();

    // Save and load a set of options from a config file
    void Save(char *fname);
    void Load(char *fname);
    void VNCOptions::LoadOpt(char subkey[256], char keyname[256]);
    int VNCOptions::read(HKEY hkey, char *name, int retval);
    void VNCOptions::save(HKEY hkey ,char *name, int value);
    void VNCOptions::LoadGenOpt();
    void VNCOptions::SaveGenOpt();
    void VNCOptions::delkey(char subkey[256], char keyname[256]);
    void VNCOptions::SaveOpt(char subkey[256], char keyname[256]);

    // process options
    bool  m_listening;
    bool  m_ipv6;
    int   m_listenPort;
    char m_display[256];
    bool  m_toolbar;
    bool  m_skipprompt;
    int   m_historyLimit;
    bool  m_connectionSpecified;
    bool  m_configSpecified;
    char m_configFilename[_MAX_PATH];
    unsigned char m_encPasswd[8];
    bool  m_restricted;
    bool m_tunnel;
    char m_gatewayHost[256];

    // default connection options - can be set through Dialog
    bool  m_ViewOnly;
    bool  m_FullScreen;
    int   m_Span;
    bool  m_Use8Bit;
    bool  m_DoubleBuffer;
    int   m_PreferredEncoding;
    int   m_LastEncoding;
    bool  m_SwapMouse;
    bool  m_Emul3Buttons;
    int   m_Emul3Timeout;
    int   m_Emul3Fuzz;
    bool  m_Shared;
    bool  m_CU;
    bool  m_DeiconifyOnBell;
    bool  m_DisableClipboard;
    int   m_localCursor;
    bool  m_scaling;
    bool  m_FitWindow;
    int   m_scale_num, m_scale_den; // Numerator & denominator
    int   m_subsampLevel;
    int   m_compressLevel;
    bool  m_enableJpegCompression;
    int   m_jpegQualityLevel;
    bool  m_requestShapeUpdates;
    bool  m_ignoreShapeUpdates;

    // Keyboard can be specified on command line as 8-digit hex
    char m_kbdname[9];
    bool  m_kbdSpecified;

    // Connection options that can't be set through the dialog

    // Which encodings do we allow?
    bool  m_UseEnc[LASTENCODING + 1];

    char m_host[256];
    int   m_port;

    bool  m_FSAltEnter;
    int   m_GrabKeyboard;
    bool  m_noUnixLogin;
    char  m_user[256];

    bool  m_autoPass;

    // Logging
    int   m_logLevel;
    bool  m_logToFile, m_logToConsole;
    char m_logFilename[_MAX_PATH];

    // for debugging purposes
    int   m_delay;

    INT_PTR DoDialog(bool running = false);
    BOOL RaiseDialog();
    void CloseDialog();

    void SetFromCommandLine(LPTSTR szCmdLine);

    static BOOL CALLBACK DlgProc(HWND hwndDlg, UINT uMsg,
                                 WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK DlgProcConnOptions(HWND hwnd, UINT uMsg,
                                            WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK DlgProcGlobalOptions(HWND hwnd, UINT uMsg,
                                              WPARAM wParam, LPARAM lParam);
    static void Lim(HWND hwnd, int control, DWORD min, DWORD max);
    // Register() makes this viewer the default application for opening .vnc
    // files
    static void Register();
    HWND m_hPageConnection, m_hPageGeneral, m_hTab, m_hParent, m_hWindow;
    void FixScaling();

  private:
    void BrowseLogFile();
    void EnableCompress(HWND hwnd, bool enable);
    void EnableQuality(HWND hwnd, bool enable);
    void EnableSubsamp(HWND hwnd, bool enable);
    void EnableLog(HWND hwnd, bool enable);
    void SetSubsampSlider(HWND hwnd, int subsamp);
    void SetQualitySlider(HWND hwnd, int subsamp);
    void SetCompressSlider(HWND hwnd, int subsamp);
    void SetComboBox(HWND hwnd);

    // Just for temporary use
    bool m_running;

    void setHistoryLimit(int newLimit);
  };

#endif // VNCOPTIONS_H__
