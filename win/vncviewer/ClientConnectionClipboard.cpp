//  Copyright (C) 2012, 2016 D. R. Commander. All Rights Reserved.
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

#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"
#include "Exception.h"


// This file contains the code for getting text from, and putting text into,
// the Windows clipboard.

// Called by ClientConnection::WndProc.
// We've been informed that the local clipboard has been updated.
// If it's text, then we want to send it to the server.

void ClientConnection::ProcessLocalClipboardChange()
{
  vnclog.Print(2, "Clipboard changed\n");

  HWND hOwner = GetClipboardOwner();
  if (hOwner == m_hwnd) {
    vnclog.Print(2, "We changed it - ignore!\n");
  } else if (!m_initialClipboardSeen) {
    vnclog.Print(2, "Don't send initial clipboard!\n");
    m_initialClipboardSeen = true;
  } else if (!m_opts.m_DisableClipboard) {

    // The clipboard should not be modified by more than one thread at once
    omni_mutex_lock l(m_clipMutex);

    if (OpenClipboard(m_hwnd)) {
      HGLOBAL hglb = GetClipboardData(CF_TEXT);
      if (hglb == NULL) {
        CloseClipboard();
      } else {
        LPSTR lpstr = (LPSTR)GlobalLock(hglb);

        char *contents = new char[strlen(lpstr) + 1];
        char *unixcontents = new char[strlen(lpstr) + 1];
        STRNCPY(contents, lpstr, strlen(lpstr) + 1);
        GlobalUnlock(hglb);
        CloseClipboard();

        // Translate to Unix EOL before sending
        int j = 0;
        for (int i = 0; contents[i] != '\0'; i++) {
          if (contents[i] != '\x0d')
            unixcontents[j++] = contents[i];
        }
        unixcontents[j] = '\0';
        try {
          SendClientCutText(unixcontents, strlen(unixcontents));
        } catch (WarningException &e) {
          vnclog.Print(0, "Exception while sending clipboard text : %s\n",
                       e.m_info);
          DestroyWindow(m_hwnd1);
        }
        delete[] contents;
        delete[] unixcontents;
      }
    }
  }
  // Pass the message to the next window in the clipboard viewer chain
  SendMessage(m_hwndNextViewer, WM_DRAWCLIPBOARD, 0, 0);
}


// We've read some text from the remote server, and we need to copy it into the
// local clipboard.  Called by ClientConnection::ReadServerCutText()

void ClientConnection::UpdateLocalClipboard(char *buf, size_t len)
{

  if (m_opts.m_DisableClipboard)
    return;

  // Copy to wincontents, replacing LF with CR-LF
  char *wincontents = new char[len * 2 + 1];
  int j = 0;
  for (int i = 0; m_netbuf[i] != 0; i++, j++) {
    if (buf[i] == '\x0a') {
      wincontents[j++] = '\x0d';
      len++;
    }
    wincontents[j] = buf[i];
  }
  wincontents[j] = '\0';

  // The clipboard should not be modified by more than one thread at once
  {
    omni_mutex_lock l(m_clipMutex);

    if (!OpenClipboard(m_hwnd)) {
      vnclog.Print(0, "Failed to open clipboard (error = %d)\n",
                   GetLastError());
      delete[] wincontents;
      return;
    }
    if (!EmptyClipboard()) {
      vnclog.Print(0, "Failed to empty clipboard (error = %d)\n",
                   GetLastError());
      delete[] wincontents;
      return;
    }

    // Allocate a global memory object for the text.
    HGLOBAL hglbCopy = GlobalAlloc(GMEM_DDESHARE, (len + 1) * sizeof(char));
    if (hglbCopy != NULL) {
      // Lock the handle and copy the text to the buffer.
      LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
      memcpy(lptstrCopy, wincontents, len * sizeof(char));
      lptstrCopy[len] = (char)0;      // null character
      GlobalUnlock(hglbCopy);         // Place the handle on the clipboard.
      SetClipboardData(CF_TEXT, hglbCopy);
    }

    delete[] wincontents;

    if (!CloseClipboard()) {
      vnclog.Print(0, "Failed to close clipboard (error = %d)\n",
                   GetLastError());
      return;
    }
  }
}
