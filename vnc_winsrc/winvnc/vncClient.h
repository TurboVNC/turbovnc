//  Copyright (C) 2001 Constantin Kaplinsky. All Rights Reserved.
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.


// vncClient.h

// vncClient class handles the following functions:
// - Recieves requests from the connected client and
//   handles them
// - Handles incoming updates properly, using a vncBuffer
//   object to keep track of screen changes
// It uses a vncBuffer and is passed the vncDesktop and
// vncServer to communicate with.

class vncClient;
typedef SHORT vncClientId;

#if (!defined(_WINVNC_VNCCLIENT))
#define _WINVNC_VNCCLIENT

#include <list>

typedef std::list<vncClientId> vncClientList;

// Includes
#include "stdhdrs.h"
#include "VSocket.h"
#include <omnithread.h>

// Custom
#include "rectlist.h"
#include "vncDesktop.h"
#include "vncRegion.h"
#include "vncBuffer.h"
#include "vncKeymap.h"

// The vncClient class itself

class vncClient
{
public:
	// Constructor/destructor
	vncClient();
	~vncClient();

	// Allow the client thread to see inside the client object
	friend class vncClientThread;

	// Init
	virtual BOOL Init(vncServer *server,
						VSocket *socket,
						BOOL auth,
						BOOL shared,
						vncClientId newid);

	// Kill
	// The server uses this to close the client socket, causing the
	// client thread to fail, which in turn deletes the client object
	virtual void Kill();

	// Client manipulation functions for use by the server
	virtual void SetBuffer(vncBuffer *buffer);

	// Update handling functions
	virtual void TriggerUpdate();
	virtual void UpdateMouse();
	virtual void UpdateRect(RECT &rect);
	virtual void UpdateRegion(vncRegion &region);
	virtual void CopyRect(RECT &dest, POINT &source);
	virtual void UpdateClipText(LPSTR text);
	virtual void UpdatePalette();

	// Functions for setting & getting the client settings
	virtual void SetTeleport(BOOL teleport) {m_teleport = teleport;};
	virtual void EnableKeyboard(BOOL enable) {m_keyboardenabled = enable;};
	virtual void EnablePointer(BOOL enable) {m_pointerenabled = enable;};
	virtual void SetCapability(int capability) {m_capability = capability;};

	virtual BOOL IsTeleport() {return m_teleport;};
	virtual BOOL IsKeyboardEnabled() {return m_keyboardenabled;};
	virtual BOOL IsPointerEnabled() {return m_pointerenabled;};
	virtual int GetCapability() {return m_capability;};
	virtual const char *GetClientName();
	virtual vncClientId GetClientId() {return m_id;};

	// Update routines
protected:
	BOOL SendUpdate();
	BOOL SendRFBMsg(CARD8 type, BYTE *buffer, int buflen);
	void CheckRects(vncRegion &rgn, rectlist &rects);
	void ClearRects(vncRegion &rgn, rectlist &rects);
	void GrabRegion(vncRegion &rgn);
	BOOL SendRectangles(rectlist &rects);
	BOOL SendRectangle(RECT &rect);
	BOOL SendCopyRect(RECT &dest, POINT &source);
	BOOL SendCursorShapeUpdate();
	BOOL SendCursorPosUpdate();
	BOOL SendLastRect();
	BOOL SendPalette();

	void PollWindow(HWND hwnd);

	// Internal stuffs
protected:
	// Per-client settings
	BOOL			m_teleport;
	BOOL			m_keyboardenabled;
	BOOL			m_pointerenabled;
	int				m_capability;
	BOOL			m_copyrect_use;
	vncClientId		m_id;

	// The screen buffer
	vncBuffer		*m_buffer;

	// The server
	vncServer		*m_server;

	// The socket
	VSocket			*m_socket;
	char			*m_client_name;

	// The client thread
	omni_thread		*m_thread;

	// Flag to indicate whether the client is ready for RFB messages
	BOOL			m_protocol_ready;

	// User input information
	RECT			m_oldmousepos;
	BOOL			m_mousemoved;
	rfbPointerEventMsg	m_ptrevent;
	vncKeymap		m_keymap;

	// Support for cursor shape updates (XCursor, RichCursor encodings)
	BOOL			m_cursor_update_pending;
	BOOL			m_cursor_update_sent;
	BOOL			m_cursor_pos_changed;
	HCURSOR			m_hcursor;
	POINT			m_cursor_pos;

	// Region structures used when preparing updates
	// - region of rects which may have changed since last update
	// - region for which incremental data is requested
	// - region for which full data is requested
	vncRegion		m_changed_rgn;
	vncRegion		m_incr_rgn;
	vncRegion		m_full_rgn;
	omni_mutex		m_regionLock;

	BOOL			m_copyrect_set;
	RECT			m_copyrect_rect;
	POINT			m_copyrect_src;

	BOOL			m_updatewanted;
	RECT			m_fullscreen;

	// When the local display is palettized, it sometimes changes...
	BOOL			m_palettechanged;

	// Information used in polling mode!
	RECT			m_qtrscreen;
	UINT			m_pollingcycle;
	BOOL			m_remoteevent;

	BOOL			m_use_PointerPos;
};

#endif
