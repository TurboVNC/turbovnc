//  Copyright (C) 2001 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
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


// vncClient.cpp

// The per-client object.  This object takes care of all per-client stuff,
// such as socket input and buffering of updates.

// vncClient class handles the following functions:
// - Recieves requests from the connected client and
//   handles them
// - Handles incoming updates properly, using a vncBuffer
//   object to keep track of screen changes
// It uses a vncBuffer and is passed the vncDesktop and
// vncServer to communicate with.

// Includes
#include "stdhdrs.h"
#include <omnithread.h>
#include "resource.h"

// Custom
#include "vncClient.h"
#include "VSocket.h"
#include "vncDesktop.h"
#include "vncRegion.h"
#include "vncBuffer.h"
#include "vncService.h"
#include "vncPasswd.h"
#include "vncAcceptDialog.h"
// #include "rfb.h"

// vncClient thread class

class vncClientThread : public omni_thread
{
public:

	// Init
	virtual BOOL Init(vncClient *client,
		vncServer *server,
		VSocket *socket,
		BOOL auth,
		BOOL shared);

	// Sub-Init routines
	virtual BOOL InitVersion();
	virtual BOOL InitAuthenticate();

	// The main thread function
	virtual void run(void *arg);

protected:
	virtual ~vncClientThread();

	// Fields
protected:
	VSocket *m_socket;
	vncServer *m_server;
	vncClient *m_client;
	BOOL m_auth;
	BOOL m_shared;
};

vncClientThread::~vncClientThread()
{
	// If we have a client object then delete it
	if (m_client != NULL)
		delete m_client;
}

BOOL
vncClientThread::Init(vncClient *client, vncServer *server, VSocket *socket, BOOL auth, BOOL shared)
{
	// Save the server pointer and window handle
	m_server = server;
	m_socket = socket;
	m_client = client;
	m_auth = auth;
	m_shared = shared;

	// Start the thread
	start();

	return TRUE;
}

BOOL
vncClientThread::InitVersion()
{
	// Generate the server's protocol version
	rfbProtocolVersionMsg protocolMsg;
	sprintf((char *)protocolMsg,
		rfbProtocolVersionFormat,
		rfbProtocolMajorVersion,
		rfbProtocolMinorVersion);

	// Send the protocol message
	if (!m_socket->SendExact((char *)&protocolMsg, sz_rfbProtocolVersionMsg))
		return FALSE;

	// Now, get the client's protocol version
	rfbProtocolVersionMsg protocol_ver;
	protocol_ver[12] = 0;
	if (!m_socket->ReadExact((char *)&protocol_ver, sz_rfbProtocolVersionMsg))
		return FALSE;

	// Check the protocol version
	int major, minor;
	sscanf((char *)&protocol_ver, rfbProtocolVersionFormat, &major, &minor);
	if (major != rfbProtocolMajorVersion) {
		vnclog.Print(LL_CONNERR, VNCLOG("protocol versions do not match\n"));
		return FALSE;
	}

	return TRUE;
}

BOOL
vncClientThread::InitAuthenticate()
{
	// Retrieve local passwords
	char password[MAXPWLEN];
	m_server->GetPassword(password);
	vncPasswd::ToText plain(password);
	m_server->GetPasswordViewOnly(password);
	vncPasswd::ToText plain_viewonly(password);

	// By default we disallow passwordless workstations!
	if ((strlen(plain) == 0) && m_server->AuthRequired())
	{
		vnclog.Print(LL_CONNERR, VNCLOG("no password specified for server - client rejected\n"));

		// Send an error message to the client
		CARD32 auth_val = Swap32IfLE(rfbConnFailed);
		char *errmsg =
			"This server does not have a valid password enabled.  "
			"Until a password is set, incoming connections cannot be accepted.";
		CARD32 errlen = Swap32IfLE(strlen(errmsg));

		if (!m_socket->SendExact((char *)&auth_val, sizeof(auth_val)))
			return FALSE;
		if (!m_socket->SendExact((char *)&errlen, sizeof(errlen)))
			return FALSE;
		m_socket->SendExact(errmsg, strlen(errmsg));

		return FALSE;
	}

	// By default we filter out local loop connections, because they're pointless
	if (!m_server->LoopbackOk())
	{
		char *localname = strdup(m_socket->GetSockName());
		char *remotename = strdup(m_socket->GetPeerName());

		// Check that the local & remote names are different!
		if ((localname != NULL) && (remotename != NULL))
		{
			BOOL ok = strcmp(localname, remotename) != 0;

			free(localname);
			free(remotename);

			if (!ok)
			{
				vnclog.Print(LL_CONNERR, VNCLOG("loopback connection attempted - client rejected\n"));
				
				// Send an error message to the client
				CARD32 auth_val = Swap32IfLE(rfbConnFailed);
				char *errmsg = "Local loop-back connections are disabled.";
				CARD32 errlen = Swap32IfLE(strlen(errmsg));

				if (!m_socket->SendExact((char *)&auth_val, sizeof(auth_val)))
					return FALSE;
				if (!m_socket->SendExact((char *)&errlen, sizeof(errlen)))
					return FALSE;
				m_socket->SendExact(errmsg, strlen(errmsg));

				return FALSE;
			}
		}
	}

	// Verify the peer host name against the AuthHosts string
	vncServer::AcceptQueryReject verified;
	if (m_auth) {
		verified = vncServer::aqrAccept;
	} else {
		verified = m_server->VerifyHost(m_socket->GetPeerName());
	}

	// If necessary, query the connection with a timed dialog
	BOOL skip_auth = FALSE;
	if (verified == vncServer::aqrQuery) {
		vncAcceptDialog *acceptDlg =
			new vncAcceptDialog(m_server->QueryTimeout(),
								m_server->QueryAccept(),
								m_server->QueryAllowNoPass(),
								m_socket->GetPeerName());
		if (acceptDlg == NULL) {
			if (m_server->QueryAccept()) {
				verified = vncServer::aqrAccept;
			} else {
				verified = vncServer::aqrReject;
			}
		} else {
			int action = acceptDlg->DoDialog();
			if (action > 0) {
				verified = vncServer::aqrAccept;
				if (action == 2)
					skip_auth = TRUE;	// accept without authentication
			} else {
				verified = vncServer::aqrReject;
			}
			delete acceptDlg;
		}
	}

	// The connection should be rejected, either due to AuthHosts settings,
	// or because of the "Reject" action performed in the query dialog
	if (verified == vncServer::aqrReject) {
		vnclog.Print(LL_CONNERR, VNCLOG("Client connection rejected\n"));
		CARD32 auth_val = Swap32IfLE(rfbConnFailed);
		char *errmsg = "Your connection has been rejected.";
		CARD32 errlen = Swap32IfLE(strlen(errmsg));
		if (!m_socket->SendExact((char *)&auth_val, sizeof(auth_val)))
			return FALSE;
		if (!m_socket->SendExact((char *)&errlen, sizeof(errlen)))
			return FALSE;
		m_socket->SendExact(errmsg, strlen(errmsg));
		return FALSE;
	}

	// Authenticate the connection, if required
	if (m_auth || strlen(plain) == 0 || skip_auth)
	{
		// Send no-auth-required message
		CARD32 auth_val = Swap32IfLE(rfbNoAuth);
		if (!m_socket->SendExact((char *)&auth_val, sizeof(auth_val)))
			return FALSE;
	}
	else
	{
		// Send auth-required message
		CARD32 auth_val = Swap32IfLE(rfbVncAuth);
		if (!m_socket->SendExact((char *)&auth_val, sizeof(auth_val)))
			return FALSE;

		BOOL auth_ok = FALSE;
		{
			// Now create a 16-byte challenge
			char challenge[16];
			char challenge_viewonly[16];

			vncRandomBytes((BYTE *)&challenge);
			memcpy(challenge_viewonly, challenge, 16);

			// Send the challenge to the client
			if (!m_socket->SendExact(challenge, sizeof(challenge)))
				return FALSE;

			// Read the response
			char response[16];
			if (!m_socket->ReadExact(response, sizeof(response)))
				return FALSE;

			// Encrypt the challenge bytes
			vncEncryptBytes((BYTE *)&challenge, plain);

			// Compare them to the response
			if (memcmp(challenge, response, sizeof(response)) == 0) {
				auth_ok = TRUE;
			} else {
				// Check against the view-only password
				vncEncryptBytes((BYTE *)&challenge_viewonly, plain_viewonly);
				if (memcmp(challenge_viewonly, response, sizeof(response)) == 0) {
					m_client->m_pointerenabled = FALSE;
					m_client->m_keyboardenabled = FALSE;
					auth_ok = TRUE;
				}
			}
		}

		// Did the authentication work?
		CARD32 authmsg;
		if (!auth_ok)
		{
			vnclog.Print(LL_CONNERR, VNCLOG("authentication failed\n"));

			authmsg = Swap32IfLE(rfbVncAuthFailed);
			m_socket->SendExact((char *)&authmsg, sizeof(authmsg));
			return FALSE;
		}
		else
		{
			// Tell the client we're ok
			authmsg = Swap32IfLE(rfbVncAuthOK);
			if (!m_socket->SendExact((char *)&authmsg, sizeof(authmsg)))
				return FALSE;
		}
	}

	// Read the client's initialisation message
	rfbClientInitMsg client_ini;
	if (!m_socket->ReadExact((char *)&client_ini, sz_rfbClientInitMsg))
		return FALSE;

	// If the client wishes to have exclusive access then remove other clients
	if (!client_ini.shared && !m_shared)
	{
		// Which client takes priority, existing or incoming?
		if (m_server->ConnectPriority() < 1)
		{
			// Incoming
			vnclog.Print(LL_INTINFO, VNCLOG("non-shared connection - disconnecting old clients\n"));
			m_server->KillAuthClients();
		} else if (m_server->ConnectPriority() > 1)
		{
			// Existing
			if (m_server->AuthClientCount() > 0)
			{
				vnclog.Print(LL_CLIENTS, VNCLOG("connections already exist - client rejected\n"));
				return FALSE;
			}
		}
	}

	// Tell the server that this client is ok
	return m_server->Authenticated(m_client->GetClientId());
}

void
ClearKeyState(BYTE key)
{
	// This routine is used by the VNC client handler to clear the
	// CAPSLOCK, NUMLOCK and SCROLL-LOCK states.

	BYTE keyState[256];
	
	GetKeyboardState((LPBYTE)&keyState);

	if(keyState[key] & 1)
	{
		// Simulate the key being pressed
		keybd_event(key, 0, KEYEVENTF_EXTENDEDKEY, 0);

		// Simulate it being release
		keybd_event(key, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
}

void
vncClientThread::run(void *arg)
{
	// All this thread does is go into a socket-recieve loop,
	// waiting for stuff on the given socket

	// IMPORTANT : ALWAYS call RemoveClient on the server before quitting
	// this thread.

	vnclog.Print(LL_CLIENTS, VNCLOG("client connected : %s (id %hd)\n"),
				 m_client->GetClientName(), m_client->GetClientId());

	// Save the handle to the thread's original desktop
	HDESK home_desktop = GetThreadDesktop(GetCurrentThreadId());
	
	// To avoid people connecting and then halting the connection, set a timeout
	if (!m_socket->SetTimeout(30000))
		vnclog.Print(LL_INTERR, VNCLOG("failed to set socket timeout, error=%d\n"), GetLastError());

	// Initially blacklist the client so that excess connections from it get dropped
	m_server->AddAuthHostsBlacklist(m_client->GetClientName());

	// LOCK INITIAL SETUP
	// All clients have the m_protocol_ready flag set to FALSE initially, to prevent
	// updates and suchlike interfering with the initial protocol negotiations.

	// GET PROTOCOL VERSION
	if (!InitVersion())
	{
		m_server->RemoveClient(m_client->GetClientId());
		return;
	}
	vnclog.Print(LL_INTINFO, VNCLOG("negotiated protocol version\n"));

	// AUTHENTICATE LINK
	if (!InitAuthenticate())
	{
		m_server->RemoveClient(m_client->GetClientId());
		return;
	}

	// Authenticated OK - remove from blacklist and remove timeout
	m_server->RemAuthHostsBlacklist(m_client->GetClientName());
	m_socket->SetTimeout(m_server->AutoIdleDisconnectTimeout()*1000);
	vnclog.Print(LL_INTINFO, VNCLOG("authenticated connection\n"));

	// INIT PIXEL FORMAT

	// Get the screen format
	m_client->m_fullscreen = m_client->m_buffer->GetSize();

	// Create the quarter-screen rectangle
	m_client->m_qtrscreen.left = 0;
	m_client->m_qtrscreen.top = 0;
	m_client->m_qtrscreen.right = m_client->m_fullscreen.right/2;
	m_client->m_qtrscreen.bottom = m_client->m_fullscreen.bottom/2;

	// Get the name of this desktop
	char desktopname[MAX_COMPUTERNAME_LENGTH+1];
	DWORD desktopnamelen = MAX_COMPUTERNAME_LENGTH + 1;
	if (GetComputerName(desktopname, &desktopnamelen))
	{
		// Make the name lowercase
		for (size_t x=0; x<strlen(desktopname); x++)
		{
			desktopname[x] = tolower(desktopname[x]);
		}
	}
	else
	{
		strcpy(desktopname, "WinVNC");
	}

	// Send the server format message to the client
	rfbServerInitMsg server_ini;
	server_ini.format = m_client->m_buffer->GetLocalFormat();

	// Endian swaps
	server_ini.framebufferWidth = Swap16IfLE(m_client->m_fullscreen.right);
	server_ini.framebufferHeight = Swap16IfLE(m_client->m_fullscreen.bottom);
	server_ini.format.redMax = Swap16IfLE(server_ini.format.redMax);
	server_ini.format.greenMax = Swap16IfLE(server_ini.format.greenMax);
	server_ini.format.blueMax = Swap16IfLE(server_ini.format.blueMax);

	server_ini.nameLength = Swap32IfLE(strlen(desktopname));
	if (!m_socket->SendExact((char *)&server_ini, sizeof(server_ini)))
	{
		m_server->RemoveClient(m_client->GetClientId());
		return;
	}
	if (!m_socket->SendExact(desktopname, strlen(desktopname)))
	{
		m_server->RemoveClient(m_client->GetClientId());
		return;
	}
	vnclog.Print(LL_INTINFO, VNCLOG("sent pixel format to client\n"));

	// UNLOCK INITIAL SETUP
	// Initial negotiation is complete, so set the protocol ready flag
	{	omni_mutex_lock l(m_client->m_regionLock);
		m_client->m_protocol_ready = TRUE;
	}
	
	// Add a fullscreen update to the client's update list
	m_client->UpdateRect(m_client->m_fullscreen);

	// Clear the CapsLock and NumLock keys
	if (m_client->m_keyboardenabled)
	{
		ClearKeyState(VK_CAPITAL);
		// *** JNW - removed because people complain it's wrong
		//ClearKeyState(VK_NUMLOCK);
		ClearKeyState(VK_SCROLL);
	}

	// MAIN LOOP

	BOOL connected = TRUE;
	while (connected)
	{
		rfbClientToServerMsg msg;

		// Ensure that we're running in the correct desktop
		if (!vncService::InputDesktopSelected())
			if (!vncService::SelectDesktop(NULL))
				break;

		// Try to read a message ID
		if (!m_socket->ReadExact((char *)&msg.type, sizeof(msg.type)))
		{
			connected = FALSE;
			break;
		}

		// What to do is determined by the message id
		switch(msg.type)
		{

		case rfbSetPixelFormat:
			// Read the rest of the message:
			if (!m_socket->ReadExact(((char *) &msg)+1, sz_rfbSetPixelFormatMsg-1))
			{
				connected = FALSE;
				break;
			}

			// Swap the relevant bits.
			msg.spf.format.redMax = Swap16IfLE(msg.spf.format.redMax);
			msg.spf.format.greenMax = Swap16IfLE(msg.spf.format.greenMax);
			msg.spf.format.blueMax = Swap16IfLE(msg.spf.format.blueMax);

			{	omni_mutex_lock l(m_client->m_regionLock);
			
				// Tell the buffer object of the change
				if (!m_client->m_buffer->SetClientFormat(msg.spf.format))
				{
					vnclog.Print(LL_CONNERR, VNCLOG("remote pixel format invalid\n"));

					connected = FALSE;
				}

				// Set the palette-changed flag, just in case...
				m_client->m_palettechanged = TRUE;
			}
			
			break;

		case rfbSetEncodings:
			// Read the rest of the message:
			if (!m_socket->ReadExact(((char *) &msg)+1, sz_rfbSetEncodingsMsg-1))
			{
				connected = FALSE;
				break;
			}

			m_client->m_buffer->SetQualityLevel(-1);
			m_client->m_buffer->SetCompressLevel(6);
			m_client->m_buffer->EnableXCursor(FALSE);
			m_client->m_buffer->EnableRichCursor(FALSE);
			m_client->m_buffer->EnableLastRect(FALSE);
			m_client->m_use_PointerPos = FALSE;

			m_client->m_cursor_update_pending = FALSE;
			m_client->m_cursor_update_sent = FALSE;
			m_client->m_cursor_pos_changed = FALSE;

			// Read in the preferred encodings
			msg.se.nEncodings = Swap16IfLE(msg.se.nEncodings);
			{
				int x;
				BOOL encoding_set = FALSE;
				BOOL shapeupdates_requested = FALSE;
				BOOL pointerpos_requested = FALSE;

				{	omni_mutex_lock l(m_client->m_regionLock);
					// By default, don't use copyrect!
					m_client->m_copyrect_use = FALSE;
				}

				for (x=0; x<msg.se.nEncodings; x++)
				{ omni_mutex_lock l(m_client->m_regionLock);
					CARD32 encoding;

					// Read an encoding in
					if (!m_socket->ReadExact((char *)&encoding, sizeof(encoding)))
					{
						connected = FALSE;
						break;
					}

					// Is this the CopyRect encoding (a special case)?
					if (Swap32IfLE(encoding) == rfbEncodingCopyRect)
					{
						// Client wants us to use CopyRect
						m_client->m_copyrect_use = TRUE;
						continue;
					}

					// Is this an XCursor encoding request?
					if (Swap32IfLE(encoding) == rfbEncodingXCursor) {
						m_client->m_buffer->EnableXCursor(TRUE);
						shapeupdates_requested = TRUE;
						vnclog.Print(LL_INTINFO, VNCLOG("X-style cursor shape updates enabled\n"));
						continue;
					}

					// Is this a RichCursor encoding request?
					if (Swap32IfLE(encoding) == rfbEncodingRichCursor) {
						m_client->m_buffer->EnableRichCursor(TRUE);
						shapeupdates_requested = TRUE;
						vnclog.Print(LL_INTINFO, VNCLOG("Full-color cursor shape updates enabled\n"));
						continue;
					}

					// Is this a CompressLevel encoding?
					if ((Swap32IfLE(encoding) >= rfbEncodingCompressLevel0) &&
						(Swap32IfLE(encoding) <= rfbEncodingCompressLevel9))
					{
						// Client specified encoding-specific compression level
						int level = (int)(Swap32IfLE(encoding) - rfbEncodingCompressLevel0);
						m_client->m_buffer->SetCompressLevel(level);
						vnclog.Print(LL_INTINFO, VNCLOG("compression level requested: %d\n"), level);
						continue;
					}

					// Is this a QualityLevel encoding?
					if ((Swap32IfLE(encoding) >= rfbEncodingQualityLevel0) &&
						(Swap32IfLE(encoding) <= rfbEncodingQualityLevel9))
					{
						// Client specified image quality level used for JPEG compression
						int level = (int)(Swap32IfLE(encoding) - rfbEncodingQualityLevel0);
						m_client->m_buffer->SetQualityLevel(level);
						vnclog.Print(LL_INTINFO, VNCLOG("image quality level requested: %d\n"), level);
						continue;
					}

					// Is this a PointerPos encoding request?
					if (Swap32IfLE(encoding) == rfbEncodingPointerPos) {
						pointerpos_requested = TRUE;
						continue;
					}

					// Is this a LastRect encoding request?
					if (Swap32IfLE(encoding) == rfbEncodingLastRect) {
						m_client->m_buffer->EnableLastRect(TRUE);
						vnclog.Print(LL_INTINFO, VNCLOG("LastRect protocol extension enabled\n"));
						continue;
					}

					// Have we already found a suitable encoding?
					if (!encoding_set)
					{	// omni_mutex_lock l(m_client->m_regionLock);

						// No, so try the buffer to see if this encoding will work...
						if (m_client->m_buffer->SetEncoding(Swap32IfLE(encoding)))
							encoding_set = TRUE;
					}
				}

				// Enable CursorPos encoding only if cursor shape updates were
				// requested by the client.
				if (shapeupdates_requested && pointerpos_requested) {
					m_client->m_use_PointerPos = TRUE;
					m_client->m_cursor_pos_changed = TRUE;
					vnclog.Print(LL_INTINFO, VNCLOG("PointerPos protocol extension enabled\n"));
				}

				// If no encoding worked then default to RAW!
				// FIXME: Protocol extensions won't work in this case.
				if (!encoding_set)
				{
					omni_mutex_lock l(m_client->m_regionLock);

					vnclog.Print(LL_INTINFO, VNCLOG("defaulting to raw encoder\n"));

					if (!m_client->m_buffer->SetEncoding(Swap32IfLE(rfbEncodingRaw)))
					{
						vnclog.Print(LL_INTERR, VNCLOG("failed to select raw encoder!\n"));

						connected = FALSE;
					}
				}
			}

			break;
			
		case rfbFramebufferUpdateRequest:
			// Read the rest of the message:
			if (!m_socket->ReadExact(((char *) &msg)+1, sz_rfbFramebufferUpdateRequestMsg-1))
			{
				connected = FALSE;
				break;
			}

			{
				RECT update;

				// Get the specified rectangle as the region to send updates for.
				update.left = Swap16IfLE(msg.fur.x);
				update.top = Swap16IfLE(msg.fur.y);
				update.right = update.left + Swap16IfLE(msg.fur.w);
				update.bottom = update.top + Swap16IfLE(msg.fur.h);

				{	omni_mutex_lock l(m_client->m_regionLock);

					// Set the update-wanted flag to true
					m_client->m_updatewanted = TRUE;

					// Clip the rectangle to the screen
					if (IntersectRect(&update, &update, &m_client->m_fullscreen))
					{
						// Is this request for an incremental region?
						if (msg.fur.incremental)
						{
							// Yes, so add it to the incremental region
							m_client->m_incr_rgn.AddRect(update);
						}
						else
						{
							// No, so add it to the full update region
							m_client->m_full_rgn.AddRect(update);

							// Disable any pending CopyRect
							m_client->m_copyrect_set = FALSE;
						}
					}

					// Trigger an update
					m_server->RequestUpdate();
				}
			}
			break;

		case rfbKeyEvent:
			// Read the rest of the message:
			if (m_socket->ReadExact(((char *) &msg)+1, sz_rfbKeyEventMsg-1))
			{				
				if (m_client->m_keyboardenabled)
				{
					msg.ke.key = Swap32IfLE(msg.ke.key);

					// Get the keymapper to do the work
					vncKeymap::keyEvent(msg.ke.key, msg.ke.down != 0);

					m_client->m_remoteevent = TRUE;
				}
			}
			break;

		case rfbPointerEvent:
			// Read the rest of the message:
			if (m_socket->ReadExact(((char *) &msg)+1, sz_rfbPointerEventMsg-1))
			{
				if (m_client->m_pointerenabled)
				{
					// Convert the coords to Big Endian
					msg.pe.x = Swap16IfLE(msg.pe.x);
					msg.pe.y = Swap16IfLE(msg.pe.y);

					// Remember cursor position for this client
					m_client->m_cursor_pos.x = msg.pe.x;
					m_client->m_cursor_pos.y = msg.pe.y;

					// Work out the flags for this event
					DWORD flags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
					if ( (msg.pe.buttonMask & rfbButton1Mask) != 
						(m_client->m_ptrevent.buttonMask & rfbButton1Mask) )
					{
					    if (GetSystemMetrics(SM_SWAPBUTTON))
						flags |= (msg.pe.buttonMask & rfbButton1Mask) 
						    ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
					    else
						flags |= (msg.pe.buttonMask & rfbButton1Mask) 
						    ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
					}
					if ( (msg.pe.buttonMask & rfbButton2Mask) != 
						(m_client->m_ptrevent.buttonMask & rfbButton2Mask) )
					{
						flags |= (msg.pe.buttonMask & rfbButton2Mask) 
						    ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
					}
					if ( (msg.pe.buttonMask & rfbButton3Mask) != 
						(m_client->m_ptrevent.buttonMask & rfbButton3Mask) )
					{
					    if (GetSystemMetrics(SM_SWAPBUTTON))
						flags |= (msg.pe.buttonMask & rfbButton3Mask) 
						    ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
					    else
						flags |= (msg.pe.buttonMask & rfbButton3Mask) 
						    ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
					}

					// Treat buttons 4 and 5 presses as mouse wheel events
					DWORD wheel_movement = 0;
					if ((msg.pe.buttonMask & rfbButton4Mask) != 0 &&
						(m_client->m_ptrevent.buttonMask & rfbButton4Mask) == 0)
					{
						flags |= MOUSEEVENTF_WHEEL;
						wheel_movement = (DWORD)+120;
					}
					else if ((msg.pe.buttonMask & rfbButton5Mask) != 0 &&
							 (m_client->m_ptrevent.buttonMask & rfbButton5Mask) == 0)
					{
						flags |= MOUSEEVENTF_WHEEL;
						wheel_movement = (DWORD)-120;
					}

					// Generate coordinate values
					unsigned long x = (msg.pe.x *  65535) / (m_client->m_fullscreen.right);
					unsigned long y = (msg.pe.y * 65535) / (m_client->m_fullscreen.bottom);

					// Do the pointer event
					::mouse_event(flags, (DWORD)x, (DWORD)y, wheel_movement, 0);
					// Save the old position
					m_client->m_ptrevent = msg.pe;

					// Flag that a remote event occurred
					m_client->m_remoteevent = TRUE;

					// Flag that the mouse moved
					// FIXME: Is it necessary?
					m_client->UpdateMouse();

					// Trigger an update
					m_server->RequestUpdate();
				}
			}
			break;

		case rfbClientCutText:
			// Read the rest of the message:
			if (m_socket->ReadExact(((char *) &msg)+1, sz_rfbClientCutTextMsg-1))
			{
				// Allocate storage for the text
				const UINT length = Swap32IfLE(msg.cct.length);
				char *text = new char [length+1];
				if (text == NULL)
					break;
				text[length] = 0;

				// Read in the text
				if (!m_socket->ReadExact(text, length)) {
					delete [] text;
					break;
				}

				// Get the server to update the local clipboard
				m_server->UpdateLocalClipText(text);

				// Free the clip text we read
				delete [] text;
			}
			break;

		default:
			// Unknown message, so fail!
			connected = FALSE;
		}
	}

	// Move into the thread's original desktop
	vncService::SelectHDESK(home_desktop);

	// Quit this thread.  This will automatically delete the thread and the
	// associated client.
	vnclog.Print(LL_CLIENTS, VNCLOG("client disconnected : %s (id %hd)\n"),
				 m_client->GetClientName(), m_client->GetClientId());

	// Remove the client from the server, just in case!
	m_server->RemoveClient(m_client->GetClientId());
}

// The vncClient itself

vncClient::vncClient()
{
	vnclog.Print(LL_INTINFO, VNCLOG("vncClient() executing...\n"));

	m_socket = NULL;
	m_client_name = 0;
	m_buffer = NULL;

	m_copyrect_use = FALSE;

	m_mousemoved = FALSE;
	m_ptrevent.buttonMask = 0;
	m_ptrevent.x = 0;
	m_ptrevent.y = 0;

	m_cursor_update_pending = FALSE;
	m_cursor_update_sent = FALSE;
	m_cursor_pos_changed = FALSE;
	m_cursor_pos.x = -1;
	m_cursor_pos.y = -1;

	m_thread = NULL;
	m_updatewanted = FALSE;

	m_palettechanged = FALSE;

	m_copyrect_set = FALSE;

	m_pollingcycle = 0;
	m_remoteevent = FALSE;

	// IMPORTANT: Initially, client is not protocol-ready.
	m_protocol_ready = FALSE;
}

vncClient::~vncClient()
{
	vnclog.Print(LL_INTINFO, VNCLOG("~vncClient() executing...\n"));

	// We now know the thread is dead, so we can clean up
	if (m_client_name != 0) {
		free(m_client_name);
		m_client_name = 0;
	}

	// If we have a socket then kill it
	if (m_socket != NULL)
	{
		vnclog.Print(LL_INTINFO, VNCLOG("deleting socket\n"));

		delete m_socket;
		m_socket = NULL;
	}

	// Kill the screen buffer
	if (m_buffer != NULL)
	{
		vnclog.Print(LL_INTINFO, VNCLOG("deleting buffer\n"));

		delete m_buffer;
		m_buffer = NULL;
	}
}

// Init
BOOL
vncClient::Init(vncServer *server,
				VSocket *socket,
				BOOL auth,
				BOOL shared,
				vncClientId newid)
{
	// Save the server id;
	m_server = server;

	// Save the socket
	m_socket = socket;

	// Save the name of the connecting client
	char *name = m_socket->GetPeerName();
	if (name != 0)
		m_client_name = strdup(name);
	else
		m_client_name = strdup("<unknown>");

	// Save the client id
	m_id = newid;

	// Spawn the child thread here
	m_thread = new vncClientThread;
	if (m_thread == NULL)
		return FALSE;
	return ((vncClientThread *)m_thread)->Init(this, m_server, m_socket, auth, shared);

	return FALSE;
}

void
vncClient::Kill()
{
	// Close the socket
	if (m_socket != NULL)
		m_socket->Close();
}

// Client manipulation functions for use by the server
void
vncClient::SetBuffer(vncBuffer *buffer)
{
	// Until authenticated, the client object has no access
	// to the screen buffer.  This means that there only need
	// be a buffer when there's at least one authenticated client.
	m_buffer = buffer;
}

// Update handling functions
void
vncClient::PollWindow(HWND hwnd)
{
	BOOL poll = TRUE;

	// Are we set to low-load polling?
	if (m_server->PollOnEventOnly())
	{
		// Yes, so only poll if the remote user has done something
		if (!m_remoteevent)
			poll = FALSE;
	}

	// Does the client want us to poll only console windows?
	if (m_server->PollConsoleOnly())
	{
		char classname[20];

		// Yes, so check that this is a console window...
		if (GetClassName(hwnd, classname, sizeof(classname)))
			if ((strcmp(classname, "tty") != 0) &&
				(strcmp(classname, "ConsoleWindowClass") != 0))
				poll = FALSE;
	}

	// Are we still wanting to poll this window?
	if (poll)
	{
		RECT rect;

		// Get the rectangle
		if (GetWindowRect(hwnd, &rect))
			m_changed_rgn.AddRect(rect);
	}
}

void
vncClient::TriggerUpdate()
{
	// Lock the updates stored so far
	omni_mutex_lock l(m_regionLock);
	if (!m_protocol_ready) return;

	if (m_updatewanted)
	{
		// Handle the three polling modes
		if (m_server->PollFullScreen())
		{
			RECT rect;
			rect.left = (m_pollingcycle % 2) * m_qtrscreen.right;
			rect.right = rect.left + m_qtrscreen.right;
			rect.top = (m_pollingcycle / 2) * m_qtrscreen.bottom;
			rect.bottom = rect.top + m_qtrscreen.bottom;

			m_changed_rgn.AddRect(rect);
			
			m_pollingcycle = (m_pollingcycle + 1) % 4;
		}

		if (m_server->PollForeground())
		{
			// Get the window rectangle for the currently selected window
			HWND hwnd = GetForegroundWindow();
			if (hwnd != NULL)
				PollWindow(hwnd);
		}

		if (m_server->PollUnderCursor())
		{
			// Find the mouse position
			POINT mousepos;
			if (GetCursorPos(&mousepos))
			{
				// Find the window under the mouse
				HWND hwnd = WindowFromPoint(mousepos);
				if (hwnd != NULL)
					PollWindow(hwnd);
			}
		}

		// Check if cursor shape update has to be sent
		m_cursor_update_pending = m_buffer->IsCursorUpdatePending();

		// Clear the remote event flag
		m_remoteevent = FALSE;

		// Send an update if one is waiting
		if (!m_changed_rgn.IsEmpty() ||
			!m_full_rgn.IsEmpty() ||
			m_copyrect_set ||
			m_cursor_update_pending ||
			m_cursor_pos_changed)
		{
			// Has the palette changed?
			if (m_palettechanged)
			{
				m_palettechanged = FALSE;
				if (!SendPalette())
					return;
			}

			// Now send the update
			m_updatewanted = !SendUpdate();
		}
	}
}

void
vncClient::UpdateMouse()
{
	if (!m_mousemoved && !m_cursor_update_sent) {
		omni_mutex_lock l(m_regionLock);

		if (IntersectRect(&m_oldmousepos, &m_oldmousepos, &m_fullscreen))
			m_changed_rgn.AddRect(m_oldmousepos);

		m_mousemoved = TRUE;
	} else if (m_use_PointerPos) {
		m_cursor_pos_changed = TRUE;
	}
}

void
vncClient::UpdateRect(RECT &rect)
{
	// Add the rectangle to the update region
	if (IsRectEmpty(&rect))
		return;

	if (IntersectRect(&rect, &rect, &m_fullscreen))
	{	omni_mutex_lock l(m_regionLock);
		
		m_changed_rgn.AddRect(rect);
	}
}

void
vncClient::UpdateRegion(vncRegion &region)
{
	// Merge our current update region with the supplied one
	if (region.IsEmpty())
		return;

	{	omni_mutex_lock l(m_regionLock);
		
		// Merge the two
		vncRegion dummy;
		dummy.AddRect(m_fullscreen);
		region.Intersect(dummy);

		m_changed_rgn.Combine(region);
	}
}

void
vncClient::CopyRect(RECT &dest, POINT &source)
{
	// If copyrect is disabled then just redraw the region!
	if (!m_copyrect_use)
	{
		UpdateRect(dest);
		return;
	}

	{	omni_mutex_lock l(m_regionLock);

		// Clip the destination to the screen
		RECT destrect;
		if (!IntersectRect(&destrect, &dest, &m_fullscreen))
			return;

		// Adjust the source correspondingly
		source.x = source.x + (destrect.left - dest.left);
		source.y = source.y + (destrect.top - dest.top);

		// Work out the source rectangle
		RECT srcrect;

		// Is this a continuation of an earlier window drag?
		if (m_copyrect_set &&
			((source.x == m_copyrect_rect.left) && (source.y == m_copyrect_rect.top)))
		{
			// Yes, so use the old source position
			srcrect.left = m_copyrect_src.x;
			srcrect.top = m_copyrect_src.y;
		}
		else
		{
			// No, so use this source position
			srcrect.left = source.x;
			srcrect.top = source.y;
		}

		// And fill out the right & bottom using the dest rect
		srcrect.right = destrect.right-destrect.left + srcrect.left;
		srcrect.bottom = destrect.bottom-destrect.top + srcrect.top;

		// Clip the source to the screen
		RECT srcrect2;
		if (!IntersectRect(&srcrect2, &srcrect, &m_fullscreen))
			return;

		// Correct the destination rectangle
		destrect.left += (srcrect2.left - srcrect.left);
		destrect.top += (srcrect2.top - srcrect.top);
		destrect.right = srcrect2.right-srcrect2.left + destrect.left;
		destrect.bottom = srcrect2.bottom-srcrect2.top + destrect.top;

		// Is there an existing CopyRect rectangle?
		if (m_copyrect_set)
		{
			// Yes, so compare their areas!
			if (((destrect.right-destrect.left) * (destrect.bottom-destrect.top))
				< ((m_copyrect_rect.right-m_copyrect_rect.left) * (m_copyrect_rect.bottom-m_copyrect_rect.top)))
				return;
		}

		// Set the copyrect...
		m_copyrect_rect = destrect;
		m_copyrect_src.x = srcrect2.left;
		m_copyrect_src.y = srcrect2.top;

		m_copyrect_set = TRUE;
	}
}

void
vncClient::UpdateClipText(LPSTR text)
{
	// Lock out any update sends and send clip text to the client
	omni_mutex_lock l(m_regionLock);
	if (!m_protocol_ready) return;

	rfbServerCutTextMsg message;
	message.length = Swap32IfLE(strlen(text));
	if (!SendRFBMsg(rfbServerCutText, (BYTE *) &message, sizeof(message)))
	{
		Kill();
		return;
	}
	if (!m_socket->SendQueued(text, strlen(text)))
	{
		Kill();
		return;
	}
}

void
vncClient::UpdatePalette()
{
	omni_mutex_lock l(m_regionLock);

	m_palettechanged = TRUE;
}

// Functions used to set and retrieve the client settings
const char*
vncClient::GetClientName()
{
	return m_client_name;
}

// Internal methods
BOOL
vncClient::SendRFBMsg(CARD8 type, BYTE *buffer, int buflen)
{
	// Set the message type
	((rfbServerToClientMsg *)buffer)->type = type;

	// Send the message
	if (!m_socket->SendQueued((char *) buffer, buflen))
	{
		vnclog.Print(LL_CONNERR, VNCLOG("failed to send RFB message to client\n"));

		Kill();
		return FALSE;
	}
	return TRUE;
}

// Check for each rect in the list, which rects actually need sending
inline void
vncClient::CheckRects(vncRegion &rgn, rectlist &rects)
{
	rectlist::iterator i;

	for (i = rects.begin(); i != rects.end(); i++)
	{
		// Get the buffer to check for changes in the rect
		m_buffer->GetChangedRegion(rgn, *i);
	}
}

// For each rectangle in the list, just copy the foreground buffer to the background,
// to avoid false updates in the future.
inline void
vncClient::ClearRects(vncRegion &rgn, rectlist &rects)
{
	rectlist::iterator i;

	for (i = rects.begin(); i != rects.end(); i++)
	{
		m_buffer->Clear(*i);
		rgn.AddRect(*i);
	}
}

// Grab the given rectangles using the smallest no of bands possible
inline void
vncClient::GrabRegion(vncRegion &rgn)
{
	rectlist::iterator i;
	rectlist rects;
	RECT grabRect;

	// Get the rectangles
	if (!rgn.Rectangles(rects))
		return;

	// Clear the DIB rectangle
	SetRectEmpty(&grabRect);

	// Sort the rectangles in order of height
	rects.sort();

	for (i = rects.begin(); i != rects.end(); i++)
	{
		RECT current = *i;

		// Check that this rectangle is part of this capture region
		if (current.top > grabRect.bottom)
		{
			// If the existing rect is non-null the capture it
			if (!IsRectEmpty(&grabRect))
				m_buffer->GrabRect(grabRect);

			grabRect = current;
		} else {
			// Enlarge the region to be captured
			UnionRect(&grabRect, &current, &grabRect);
		}
	}

	// If there are still some rects to be done then do them
	if (!IsRectEmpty(&grabRect))
		m_buffer->GrabRect(grabRect);
}

BOOL
vncClient::SendUpdate()
{
	vncRegion toBeSent;			// Region to actually be sent
	rectlist toBeSentList;		// List of rectangles to actually send
	vncRegion toBeDone;			// Region to check

	// Prepare to send cursor position update if necessary
	if (m_cursor_pos_changed) {
		POINT cursor_pos;
		if (!GetCursorPos(&cursor_pos)) {
			cursor_pos.x = 0;
			cursor_pos.y = 0;
		}
		if (cursor_pos.x == m_cursor_pos.x && cursor_pos.y == m_cursor_pos.y) {
			m_cursor_pos_changed = FALSE;
		} else {
			m_cursor_pos.x = cursor_pos.x;
			m_cursor_pos.y = cursor_pos.y;
		}
	}

	// If there is nothing to send then exit
	if (m_changed_rgn.IsEmpty() &&
		m_full_rgn.IsEmpty() &&
		!m_copyrect_set &&
		!m_cursor_update_pending &&
		!m_cursor_pos_changed)
		return FALSE;

	// Check that the copyrect region doesn't intersect the full update region
	if (!m_full_rgn.IsEmpty() && m_copyrect_set)
	{	vncRegion temp;
		temp.AddRect(m_copyrect_rect);
		
		temp.Intersect(m_full_rgn);
		if (!temp.IsEmpty())
		{
			m_changed_rgn.AddRect(m_copyrect_rect);
			m_copyrect_set = FALSE;
		}
	}

	// Handle the CopyRect region, if any
	if (m_copyrect_set)
		m_buffer->CopyRect(m_copyrect_rect, m_copyrect_src);

	// *** Currently, we only check for changes when there isn't a CopyRect to do
	if (!m_copyrect_set && (!m_changed_rgn.IsEmpty() || !m_full_rgn.IsEmpty()))
	{
		// GRAB THE SCREEN DATA

		// Get the region to be scanned and potentially sent
		toBeDone.Clear();
		toBeDone.Combine(m_incr_rgn);
		toBeDone.Subtract(m_full_rgn);
		toBeDone.Intersect(m_changed_rgn);

		// Get the region to grab
		vncRegion toBeGrabbed;
		toBeGrabbed.Clear();
		toBeGrabbed.Combine(m_full_rgn);
		toBeGrabbed.Combine(toBeDone);
		GrabRegion(toBeGrabbed);

		// CLEAR REGIONS THAT WON'T BE SCANNED

		// Get the region to definitely be sent
		toBeSent.Clear();
		if (!m_full_rgn.IsEmpty())
		{
			rectlist rectsToClear;

			// Retrieve and clear the rectangles
			if (m_full_rgn.Rectangles(rectsToClear))
				ClearRects(toBeSent, rectsToClear);
		}

		// SCAN INCREMENTAL REGIONS FOR CHANGES

		if (!toBeDone.IsEmpty())
		{
			rectlist rectsToScan;

			// Retrieve and scan the rectangles
			if (toBeDone.Rectangles(rectsToScan))
				CheckRects(toBeSent, rectsToScan);
		}

		// CLEAN UP THE MAIN REGIONS

		// Clear the bits we're about to deal with from the changed region
		m_changed_rgn.Subtract(m_incr_rgn);
		m_changed_rgn.Subtract(m_full_rgn);

		// Clear the full & incremental regions, since we've dealt with them
		if (!toBeSent.IsEmpty())
		{
			m_full_rgn.Clear();
			m_incr_rgn.Clear();
		}

		if (!m_cursor_update_sent && !m_cursor_update_pending) {
			if (!m_mousemoved) {
				vncRegion tmpMouseRgn;
				tmpMouseRgn.AddRect(m_oldmousepos);
				tmpMouseRgn.Intersect(toBeSent);
				if (!tmpMouseRgn.IsEmpty()) {
					m_mousemoved = true;
				}
			}
			if (m_mousemoved)
			{
				// Grab the mouse
				m_oldmousepos = m_buffer->GrabMouse();
				if (IntersectRect(&m_oldmousepos, &m_oldmousepos, &m_fullscreen))
					m_buffer->GetChangedRegion(toBeSent, m_oldmousepos);

				m_mousemoved = FALSE;
			}
		}
	}

	// Get the list of changed rectangles!
	int numrects = 0;
	if (toBeSent.Rectangles(toBeSentList))
	{
		// Find out how many rectangles this update will contain
		rectlist::iterator i;
		int numsubrects;
		for (i=toBeSentList.begin(); i != toBeSentList.end(); i++)
		{
			numsubrects = m_buffer->GetNumCodedRects(*i);

			// Skip rest rectangles if an encoder will use LastRect extension.
			if (numsubrects == 0) {
				numrects = 0xFFFF;
				break;
			}
			numrects += numsubrects;
		}
	}

	if (numrects != 0xFFFF) {
		// Count cursor shape and cursor position updates.
		if (m_cursor_update_pending)
			numrects++;
		if (m_cursor_pos_changed)
			numrects++;
		// Count the copyrect region
		if (m_copyrect_set)
			numrects++;
		// If there are no rectangles then return
		if (numrects == 0)
			return FALSE;
	}

	// Otherwise, send <number of rectangles> header
	rfbFramebufferUpdateMsg header;
	header.nRects = Swap16IfLE(numrects);
	if (!SendRFBMsg(rfbFramebufferUpdate, (BYTE *) &header, sz_rfbFramebufferUpdateMsg))
		return TRUE;

	// Send mouse cursor shape update
	if (m_cursor_update_pending) {
		if (!SendCursorShapeUpdate())
			return TRUE;
	}

	// Send cursor position update
	if (m_cursor_pos_changed) {
		if (!SendCursorPosUpdate())
			return TRUE;
	}

	// Encode & send the copyrect
	if (m_copyrect_set) {
		m_copyrect_set = FALSE;
		if(!SendCopyRect(m_copyrect_rect, m_copyrect_src))
			return TRUE;
	}

	// Encode & send the actual rectangles
	if (!SendRectangles(toBeSentList))
		return TRUE;

	// Send LastRect marker if needed.
	if (numrects == 0xFFFF) {
		if (!SendLastRect())
			return TRUE;
	}

	// Both lists should be empty when we exit
	_ASSERT(toBeSentList.empty());

	return TRUE;
}

// Send a set of rectangles
BOOL
vncClient::SendRectangles(rectlist &rects)
{
	RECT rect;

	// Work through the list of rectangles, sending each one
	while(!rects.empty())
	{
		rect = rects.front();
		if (!SendRectangle(rect))
			return FALSE;
		rects.pop_front();
	}
	rects.clear();

	return TRUE;
}

// Tell the encoder to send a single rectangle
BOOL
vncClient::SendRectangle(RECT &rect)
{
	// Get the buffer to encode the rectangle
	UINT bytes = m_buffer->TranslateRect(rect, m_socket);

	// Send the encoded data
	return m_socket->SendQueued((char *)(m_buffer->GetClientBuffer()), bytes);
}

// Send a single CopyRect message
BOOL
vncClient::SendCopyRect(RECT &dest, POINT &source)
{
	// Create the message header
	rfbFramebufferUpdateRectHeader copyrecthdr;
	copyrecthdr.r.x = Swap16IfLE(dest.left);
	copyrecthdr.r.y = Swap16IfLE(dest.top);
	copyrecthdr.r.w = Swap16IfLE(dest.right-dest.left);
	copyrecthdr.r.h = Swap16IfLE(dest.bottom-dest.top);
	copyrecthdr.encoding = Swap32IfLE(rfbEncodingCopyRect);

	// Create the CopyRect-specific section
	rfbCopyRect copyrectbody;
	copyrectbody.srcX = Swap16IfLE(source.x);
	copyrectbody.srcY = Swap16IfLE(source.y);

	// Now send the message;
	if (!m_socket->SendQueued((char *)&copyrecthdr, sizeof(copyrecthdr)))
		return FALSE;
	if (!m_socket->SendQueued((char *)&copyrectbody, sizeof(copyrectbody)))
		return FALSE;

	return TRUE;
}

// Send LastRect marker indicating that there are no more rectangles to send
BOOL
vncClient::SendLastRect()
{
	// Create the message header
	rfbFramebufferUpdateRectHeader hdr;
	hdr.r.x = 0;
	hdr.r.y = 0;
	hdr.r.w = 0;
	hdr.r.h = 0;
	hdr.encoding = Swap32IfLE(rfbEncodingLastRect);

	// Now send the message;
	if (!m_socket->SendQueued((char *)&hdr, sizeof(hdr)))
		return FALSE;

	return TRUE;
}

// Send the encoder-generated palette to the client
// This function only returns FALSE if the SendQueued fails - any other
// error is coped with internally...
BOOL
vncClient::SendPalette()
{
	rfbSetColourMapEntriesMsg setcmap;
	RGBQUAD *rgbquad;
	UINT ncolours = 256;

	// Reserve space for the colour data
	rgbquad = new RGBQUAD[ncolours];
	if (rgbquad == NULL)
		return TRUE;
					
	// Get the data
	if (!m_buffer->GetRemotePalette(rgbquad, ncolours))
	{
		delete [] rgbquad;
		return TRUE;
	}

	// Compose the message
	setcmap.type = rfbSetColourMapEntries;
	setcmap.firstColour = Swap16IfLE(0);
	setcmap.nColours = Swap16IfLE(ncolours);

	if (!m_socket->SendQueued((char *) &setcmap, sz_rfbSetColourMapEntriesMsg))
	{
		delete [] rgbquad;
		return FALSE;
	}

	// Now send the actual colour data...
	for (UINT i=0; i<ncolours; i++)
	{
		struct _PIXELDATA {
			CARD16 r, g, b;
		} pixeldata;

		pixeldata.r = Swap16IfLE(((CARD16)rgbquad[i].rgbRed) << 8);
		pixeldata.g = Swap16IfLE(((CARD16)rgbquad[i].rgbGreen) << 8);
		pixeldata.b = Swap16IfLE(((CARD16)rgbquad[i].rgbBlue) << 8);

		if (!m_socket->SendQueued((char *) &pixeldata, sizeof(pixeldata)))
		{
			delete [] rgbquad;
			return FALSE;
		}
	}

	// Delete the rgbquad data
	delete [] rgbquad;

	return TRUE;
}

BOOL
vncClient::SendCursorShapeUpdate()
{
	m_cursor_update_pending = FALSE;

	if (!m_buffer->SendCursorShape(m_socket)) {
		m_cursor_update_sent = FALSE;
		return m_buffer->SendEmptyCursorShape(m_socket);
	}

	m_cursor_update_sent = TRUE;
	return TRUE;
}

BOOL
vncClient::SendCursorPosUpdate()
{
	m_cursor_pos_changed = FALSE;

	rfbFramebufferUpdateRectHeader hdr;
	hdr.encoding = Swap32IfLE(rfbEncodingPointerPos);
	hdr.r.x = Swap16IfLE(m_cursor_pos.x);
	hdr.r.y = Swap16IfLE(m_cursor_pos.y);
	hdr.r.w = Swap16IfLE(0);
	hdr.r.h = Swap16IfLE(0);

	return m_socket->SendQueued((char *)&hdr, sizeof(hdr));
}

