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

// vncDesktop implementation

// System headers
#include "stdhdrs.h"
#include <omnithread.h>

// Custom headers
#include "WinVNC.h"
#include "VNCHooks\VNCHooks.h"
#include "vncServer.h"
#include "vncRegion.h"
#include "rectlist.h"
#include "vncDesktop.h"
#include "vncService.h"
#include <fstream.h>

// Constants
const UINT RFB_SCREEN_UPDATE = RegisterWindowMessage("WinVNC.Update.DrawRect");
const UINT RFB_COPYRECT_UPDATE = RegisterWindowMessage("WinVNC.Update.CopyRect");
const UINT RFB_MOUSE_UPDATE = RegisterWindowMessage("WinVNC.Update.Mouse");
const char szDesktopSink[] = "WinVNC desktop sink";

// Atoms
const char *VNC_WINDOWPOS_ATOMNAME = "VNCHooks.CopyRect.WindowPos";
ATOM VNC_WINDOWPOS_ATOM = NULL;

// The desktop handler thread
// This handles the messages posted by RFBLib to the vncDesktop window

class vncDesktopThread : public omni_thread
{
public:
	vncDesktopThread() {m_returnsig = NULL;};
protected:
	~vncDesktopThread() {if (m_returnsig != NULL) delete m_returnsig;};
public:
	virtual BOOL Init(vncDesktop *desktop, vncServer *server);
	virtual void *run_undetached(void *arg);
	virtual void ReturnVal(BOOL result);
protected:
	vncServer *m_server;
	vncDesktop *m_desktop;

	omni_mutex m_returnLock;
	omni_condition *m_returnsig;
	BOOL m_return;
	BOOL m_returnset;
};

BOOL
vncDesktopThread::Init(vncDesktop *desktop, vncServer *server)
{
	// Save the server pointer
	m_server = server;
	m_desktop = desktop;

	m_returnset = FALSE;
	m_returnsig = new omni_condition(&m_returnLock);

	// Start the thread
	start_undetached();

	// Wait for the thread to let us know if it failed to init
	{	omni_mutex_lock l(m_returnLock);

		while (!m_returnset)
		{
			m_returnsig->wait();
		}
	}

	return m_return;
}

void
vncDesktopThread::ReturnVal(BOOL result)
{
	omni_mutex_lock l(m_returnLock);

	m_returnset = TRUE;
	m_return = result;
	m_returnsig->signal();
}

void *
vncDesktopThread::run_undetached(void *arg)
{
	// Save the thread's "home" desktop, under NT (no effect under 9x)
	HDESK home_desktop = GetThreadDesktop(GetCurrentThreadId());

	// Attempt to initialise and return success or failure
	if (!m_desktop->Startup())
	{
		vncService::SelectHDESK(home_desktop);
		ReturnVal(FALSE);
		return NULL;
	}

	// Succeeded to initialise ok
	ReturnVal(TRUE);

	// START PROCESSING DESKTOP MESSAGES

	// We set a flag inside the desktop handler here, to indicate it's now safe
	// to handle clipboard messages
	m_desktop->SetClipboardActive(TRUE);

	// All UpdateRect messages are cached into a region cache object and are
	// only passed to clients immediately before TriggerUpdate is called!
	vncRegion rgncache;
	rgncache.Clear();

	BOOL unhandled;
	MSG msg;
	while (TRUE)
	{
		unhandled = TRUE;

		if (!PeekMessage(&msg, m_desktop->Window(), NULL, NULL, PM_NOREMOVE))
		{
			// Thread has gone idle.  Now would be a good time to send an update.
			// First, we must check that the screen hasnt changed too much.

			// While we're doing heavy processing, set the thread to normal priority
			omni_thread::set_priority(omni_thread::PRIORITY_NORMAL);

			// Has the display resolution or desktop changed?
			if (m_desktop->m_displaychanged || !vncService::InputDesktopSelected())
			{
				vnclog.Print(LL_STATE, VNCLOG("display resolution or desktop changed.\n"));

				rfbServerInitMsg oldscrinfo = m_desktop->m_scrinfo;
				m_desktop->m_displaychanged = FALSE;

				// Attempt to close the old hooks
				if (!m_desktop->Shutdown())
				{
					vnclog.Print(LL_INTERR, VNCLOG("failed to close desktop server.\n"));
					m_server->KillAuthClients();
					break;
				}

				// Now attempt to re-install them!
				if (!m_desktop->Startup())
				{
					vnclog.Print(LL_INTERR, VNCLOG("failed to re-start desktop server.\n"));
					m_server->KillAuthClients();
					break;
				}

				// Check that the screen info hasn't changed
				vnclog.Print(LL_INTINFO,
							 VNCLOG("SCR: old screen format %dx%dx%d\n"),
							 oldscrinfo.framebufferWidth,
							 oldscrinfo.framebufferHeight,
							 oldscrinfo.format.bitsPerPixel);
				vnclog.Print(LL_INTINFO,
							 VNCLOG("SCR: new screen format %dx%dx%d\n"),
							 m_desktop->m_scrinfo.framebufferWidth,
							 m_desktop->m_scrinfo.framebufferHeight,
							 m_desktop->m_scrinfo.format.bitsPerPixel);
				if (memcmp(&m_desktop->m_scrinfo, &oldscrinfo, sizeof(oldscrinfo)) != 0)
				{
					vnclog.Print(LL_CONNERR, VNCLOG("screen format has changed - disconnecting clients.\n"));
					m_server->KillAuthClients();
					break;
				}

				// Add a full screen update to all the clients
				RECT rect = m_desktop->m_bmrect;
				m_server->UpdateRect(rect);
				m_server->UpdatePalette();
			}
	
			// TRIGGER THE UPDATE

			// Update the mouse if required
			if (m_desktop->m_cursormoved)
			{
				m_desktop->m_cursormoved = FALSE;

				// Tell the server that the cursor has moved!
				m_server->UpdateMouse();
			}

			// Check for moved windows
			m_desktop->CalcCopyRects();

			// Flush the cached region data to all clients
			m_server->UpdateRegion(rgncache);
			rgncache.Clear();

			// Trigger an update to be sent
			m_server->TriggerUpdate();

			// Increase our priority while we process messages, to avoid "busy" applications
			// flooding us with messages
			omni_thread::set_priority(omni_thread::PRIORITY_HIGH);
		}

		// Now wait on further messages, or quit if told to
		if (!GetMessage(&msg, m_desktop->Window(), 0,0))
			break;

		// Now switch, dependent upon the message type recieved
		if (msg.message == RFB_SCREEN_UPDATE)
		{
			// An area of the screen has changed
			RECT rect;
			rect.left =	(SHORT) LOWORD(msg.wParam);
			rect.top = (SHORT) HIWORD(msg.wParam);
			rect.right = (SHORT) LOWORD(msg.lParam);
			rect.bottom = (SHORT) HIWORD(msg.lParam);
			if ((rect.left < 0) || (rect.top < 0) ||
				(rect.right > m_desktop->m_bmrect.right) || (rect.bottom > m_desktop->m_bmrect.bottom))
				vnclog.Print(50, VNCLOG("update:%d,%dx%d,%d\n"),
					rect.left, rect.top, rect.right, rect.bottom);

			rgncache.AddRect(rect);
//			m_server->UpdateRect(rect);

			unhandled = FALSE;
		}

		if (msg.message == RFB_MOUSE_UPDATE)
		{
			// Save the cursor ID
			m_desktop->SetCursor((HCURSOR) msg.wParam);
			m_desktop->m_cursormoved = TRUE;
			unhandled = FALSE;
		}

		if (unhandled)
			DispatchMessage(&msg);
	}

	m_desktop->SetClipboardActive(FALSE);
	
	vnclog.Print(LL_INTINFO, VNCLOG("quitting desktop server thread\n"));

	// Clear all the hooks and close windows, etc.
	m_desktop->Shutdown();

	// Clear the shift modifier keys, now that there are no remote clients
	vncKeymap::ClearShiftKeys();

	// Switch back into our home desktop, under NT (no effect under 9x)
	vncService::SelectHDESK(home_desktop);

	return NULL;
}

// Implementation

vncDesktop::vncDesktop()
{
	m_thread = NULL;

	m_hwnd = NULL;
	m_timerid = 0;
	m_hnextviewer = NULL;
	m_hcursor = NULL;
	m_cursormoved = TRUE;

	m_displaychanged = FALSE;

	m_hrootdc = NULL;
	m_hmemdc = NULL;
	m_membitmap = NULL;

	m_initialClipBoardSeen = FALSE;

	// Vars for Will Dean's DIBsection patch
	m_DIBbits = NULL;
	m_formatmunged = FALSE;

	m_clipboard_active = FALSE;
}

vncDesktop::~vncDesktop()
{
	vnclog.Print(LL_INTINFO, VNCLOG("killing desktop server\n"));

	// If we created a thread then here we delete it
	// The thread itself does most of the cleanup
	if(m_thread != NULL)
	{
		// Post a close message to quit our message handler thread
		PostMessage(Window(), WM_QUIT, 0, 0);

		// Join with the desktop handler thread
		void *returnval;
		m_thread->join(&returnval);
		m_thread = NULL;
	}

	// Let's call Shutdown just in case something went wrong...
	Shutdown();
}

// Routine to startup and install all the hooks and stuff
BOOL
vncDesktop::Startup()
{

	// Configure the display for optimal VNC performance.
	SetupDisplayForConnection();

	// Initialise the Desktop object
	if (!InitDesktop())
		return FALSE;

	if (!InitBitmap())
		return FALSE;

	if (!ThunkBitmapInfo())
		return FALSE;

	if (!SetPixFormat())
		return FALSE;

	EnableOptimisedBlits();

	if (!SetPixShifts())
		return FALSE;

	if (!SetPalette())
		return FALSE;

	if (!InitWindow())
		return FALSE;

	// Add the system hook
	if (!SetHook(
		m_hwnd,
		RFB_SCREEN_UPDATE,
		RFB_COPYRECT_UPDATE,
		RFB_MOUSE_UPDATE
		))
	{
		vnclog.Print(LL_INTERR, VNCLOG("failed to set system hooks\n"));
		// Switch on full screen polling, so they can see something, at least...
		m_server->PollFullScreen(TRUE);
	}

	// Start up the keyboard and mouse filters
	SetKeyboardFilterHook(m_server->LocalInputsDisabled());
	SetMouseFilterHook(m_server->LocalInputsDisabled());

	// Start a timer to handle Polling Mode.  The timer will cause
	// an "idle" event once every second, which is necessary if Polling
	// Mode is being used, to cause TriggerUpdate to be called.
	m_timerid = SetTimer(m_hwnd, 1, 1000, NULL);

	// Get hold of the WindowPos atom!
	if ((VNC_WINDOWPOS_ATOM = GlobalAddAtom(VNC_WINDOWPOS_ATOMNAME)) == 0) {
		vnclog.Print(LL_INTERR, VNCLOG("GlobalAddAtom() failed.\n"));
		return FALSE;
	}

	// Everything is ok, so return TRUE
	return TRUE;
}

// Routine to shutdown all the hooks and stuff
BOOL
vncDesktop::Shutdown()
{
	// If we created a timer then kill it
	if (m_timerid != NULL)
		KillTimer(NULL, m_timerid);

	// If we created a window then kill it and the hooks
	if(m_hwnd != NULL)
	{	
		// Remove the system hooks
		UnSetHook(m_hwnd);

		// The window is being closed - remove it from the viewer list
		ChangeClipboardChain(m_hwnd, m_hnextviewer);

		// Close the hook window
		DestroyWindow(m_hwnd);
		m_hwnd = NULL;
		m_hnextviewer = NULL;
	}

	// Now free all the bitmap stuff
	if (m_hrootdc != NULL)
	{
		// Release our device context
		if(ReleaseDC(NULL, m_hrootdc) == 0)
		{
			vnclog.Print(LL_INTERR, VNCLOG("failed to ReleaseDC\n"));
		}
		m_hrootdc = NULL;
	}
	if (m_hmemdc != NULL)
	{
		// Release our device context
		if (!DeleteDC(m_hmemdc))
		{
			vnclog.Print(LL_INTERR, VNCLOG("failed to DeleteDC\n"));
		}
		m_hmemdc = NULL;
	}
	if (m_membitmap != NULL)
	{
		// Release the custom bitmap, if any
		if (!DeleteObject(m_membitmap))
		{
			vnclog.Print(LL_INTERR, VNCLOG("failed to DeleteObject\n"));
		}
		m_membitmap = NULL;
	}

	// Free the WindowPos atom!
	if (VNC_WINDOWPOS_ATOM != NULL) {
		if (GlobalDeleteAtom(VNC_WINDOWPOS_ATOM) != 0) {
			vnclog.Print(LL_INTERR, VNCLOG("failed to delete atom!\n"));
		}
	}

	// Return display settings to previous values.
	ResetDisplayToNormal();

	return TRUE;
}

// Routine to ensure we're on the correct NT desktop

BOOL
vncDesktop::InitDesktop()
{
	if (vncService::InputDesktopSelected())
		return TRUE;

	// Ask for the current input desktop
	return vncService::SelectDesktop(NULL);
}

// Routine used to close the screen saver, if it's active...

BOOL CALLBACK
KillScreenSaverFunc(HWND hwnd, LPARAM lParam)
{
	char buffer[256];

	// - ONLY try to close Screen-saver windows!!!
	if ((GetClassName(hwnd, buffer, 256) != 0) &&
		(strcmp(buffer, "WindowsScreenSaverClass") == 0))
		PostMessage(hwnd, WM_CLOSE, 0, 0);
	return TRUE;
}

void
vncDesktop::KillScreenSaver()
{
	OSVERSIONINFO osversioninfo;
	osversioninfo.dwOSVersionInfoSize = sizeof(osversioninfo);

	// Get the current OS version
	if (!GetVersionEx(&osversioninfo))
		return;

	vnclog.Print(LL_INTINFO, VNCLOG("KillScreenSaver...\n"));

	// How to kill the screen saver depends on the OS
	switch (osversioninfo.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_WINDOWS:
		{
			// Windows 95

			// Fidn the ScreenSaverClass window
			HWND hsswnd = FindWindow ("WindowsScreenSaverClass", NULL);
			if (hsswnd != NULL)
				PostMessage(hsswnd, WM_CLOSE, 0, 0); 
			break;
		} 
	case VER_PLATFORM_WIN32_NT:
		{
			// Windows NT

			// Find the screensaver desktop
			HDESK hDesk = OpenDesktop(
				"Screen-saver",
				0,
				FALSE,
				DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS
				);
			if (hDesk != NULL)
			{
				vnclog.Print(LL_INTINFO, VNCLOG("Killing ScreenSaver\n"));

				// Close all windows on the screen saver desktop
				EnumDesktopWindows(hDesk, (WNDENUMPROC) &KillScreenSaverFunc, 0);
				CloseDesktop(hDesk);
				// Pause long enough for the screen-saver to close
				//Sleep(2000);
				// Reset the screen saver so it can run again
				SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, SPIF_SENDWININICHANGE); 
			}
			break;
		}
	}
}

void vncDesktop::ChangeResNow()
{
	BOOL settingsUpdated = false;
	int i = 0;
	lpDevMode = new DEVMODE; // *** create an instance of DEVMODE - Jeremy Peaks

	// *** WBB - Obtain the current display settings.
	if (! EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, lpDevMode)) {

		vnclog.Print(LL_INTINFO, VNCLOG("SCR-WBB: could not get "
										"current display settings!\n"));
		delete lpDevMode;
		lpDevMode = NULL;
		return;

	}

	vnclog.Print(LL_INTINFO, VNCLOG("SCR-WBB: current display: "
									"w=%d h=%d bpp=%d vRfrsh=%d.\n"),
				 lpDevMode->dmPelsWidth,
				 lpDevMode->dmPelsHeight,
				 lpDevMode->dmBitsPerPel,
				 lpDevMode->dmDisplayFrequency);

	origPelsWidth = lpDevMode->dmPelsWidth; // *** sets the original resolution for use later
	origPelsHeight = lpDevMode->dmPelsHeight; // *** - Jeremy Peaks

	// *** Open the registry key for resolution settings
	HKEY checkdetails = NULL;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
				"Software\\ORL\\WinVNC3",
				0,
				KEY_READ,
				&checkdetails);
	
	int slen=MAX_REG_ENTRY_LEN;
	int valType;
	char inouttext[MAX_REG_ENTRY_LEN];

	memset(inouttext, 0, MAX_REG_ENTRY_LEN);
	
	// *** Get the registry values for resolution change - Jeremy Peaks
	RegQueryValueEx(checkdetails,
		"ResWidth",
		NULL,
		(LPDWORD) &valType,
		(LPBYTE) &inouttext,
		(LPDWORD) &slen);

	
	if ((valType == REG_SZ) &&
		atol(inouttext)) { // *** if width is 0, then this isn't a valid resolution, so do nothing - Jeremy Peaks
		lpDevMode->dmPelsWidth = atol(inouttext);

		memset(inouttext, 0, MAX_REG_ENTRY_LEN);

		RegQueryValueEx(checkdetails,
			"ResHeight",
			NULL,
			(LPDWORD) &valType,
			(LPBYTE) &inouttext,
			(LPDWORD) &slen);
		
		lpDevMode->dmPelsHeight = atol(inouttext);
		if ((valType == REG_SZ ) &&
			(lpDevMode->dmPelsHeight > 0)) {

			vnclog.Print(LL_INTINFO, VNCLOG("SCR-WBB: attempting to change "
											"resolution w=%d h=%d\n"),
						 lpDevMode->dmPelsWidth, lpDevMode->dmPelsHeight);

			// *** make res change - Jeremy Peaks
			long resultOfResChange = ChangeDisplaySettings( lpDevMode, CDS_TEST);
			if (resultOfResChange == DISP_CHANGE_SUCCESSFUL) {
				ChangeDisplaySettings( lpDevMode, CDS_UPDATEREGISTRY);
				settingsUpdated = true;
			}
		} 
	}

	if (! settingsUpdated) {
		// Did not change the resolution.
		if ( lpDevMode != NULL ) {
			delete lpDevMode;
			lpDevMode = NULL;
		}
	}

	if (checkdetails != NULL) {
		RegCloseKey(checkdetails);
	}
}

// *** This method is a front-end for the SystemParameterInfo() call.
// *** First the HKLM\\Software\\ORL\\WinVNC\\<regName> string is checkted
// *** for presence, string type and Yes/yes/Y/y, if so, SystemParameterInfo
// *** is called.
void
vncDesktop::DisableIfRegSystemParameter(char *regName,
										int spiCommand,
										int spiParamInt,
										void* spiParamPtr,
										int spiUpdate)
{
	// *** Checks to see if reg key is set, then disables the
	// *** system parameter by invoking SystemParameterInfo().
	// *** This is placed here rather than vncHooks.cpp as it causes
	// *** VNC to crash if put there. - Jeremy Peaks

	HKEY checkdetails;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
				"Software\\ORL\\WinVNC3",
				0,
				KEY_READ,
				&checkdetails);

	int slen=MAX_REG_ENTRY_LEN;
	int valType;
	char inouttext[MAX_REG_ENTRY_LEN];

	memset(inouttext, 0, MAX_REG_ENTRY_LEN);

	// *** Get the registry value - Jeremy Peaks
	RegQueryValueEx(checkdetails,
		regName,
		NULL,
		(LPDWORD) &valType,
		(LPBYTE) &inouttext,
		(LPDWORD) &slen);

	// *** If regName = Yes/yes/Y/y then disable setting
	// *** - Jeremy Peaks & W. Brian Blevins
	if ((valType == REG_SZ) &&
		(inouttext[0] == 'Y' || inouttext[0] == 'y')) {

		vnclog.Print(LL_INTINFO, VNCLOG("SCR-WBB: attempting to %s.\n"), regName);

		SystemParametersInfo(spiCommand, spiParamInt, spiParamPtr, spiUpdate);
		// SystemParametersInfo(SPI_SETDESKPATTERN, 0, 0, 0);
	}

	if (checkdetails != NULL) {
		RegCloseKey(checkdetails);
	}
	
}

// *** Prepare the display settings for the new connection.
// *** The precise changes depend on a number of registry settings.
void
vncDesktop::SetupDisplayForConnection()
{

	KillScreenSaver();

	ChangeResNow(); // *** - Jeremy Peaks
	
	// If all optimizatations are not already enabled, then
	// check each one individually.
	if (! OptimizeDisplayForConnection())
	{
		DisableIfRegSystemParameter("DisablePattern",
									SPI_SETDESKPATTERN,
									0,
									0,
									SPIF_SENDCHANGE);

		DisableIfRegSystemParameter("DisableWallpaper",
									SPI_SETDESKWALLPAPER,
									0,
									"",
									SPIF_SENDCHANGE);

		DisableIfRegSystemParameter("DisableFontSmoothing",
									SPI_SETFONTSMOOTHING,
									FALSE,
									0,
									SPIF_SENDCHANGE);

		DisableIfRegSystemParameter("DisableFullWindowDrag",
									SPI_SETDRAGFULLWINDOWS,
									FALSE,
									0,
									SPIF_SENDCHANGE);
	}

}

// *** If configured, perform a set of display optimizations to
// *** improve performance over the remote connection.
BOOL
vncDesktop::OptimizeDisplayForConnection()
{

	BOOL result;
	HKEY checkdetails;

	result = FALSE;

	RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
				"Software\\ORL\\WinVNC3",
				0,
				KEY_READ,
				&checkdetails);

	int slen=MAX_REG_ENTRY_LEN;
	int valType;
	char inouttext[MAX_REG_ENTRY_LEN];

	memset(inouttext, 0, MAX_REG_ENTRY_LEN);

	// *** Get the DisablePattern value - Jeremy Peaks
	RegQueryValueEx(checkdetails,
		"OptimizeDesktop",
		NULL,
		(LPDWORD) &valType,
		(LPBYTE) &inouttext,
		(LPDWORD) &slen);

	// *** If Optimize = Yes/yes/Y/y then turn on all optimizations
	// *** - W. Brian Blevins
	if ((valType == REG_SZ) &&
		(inouttext[0] == 'Y' || inouttext[0] == 'y')) {

		vnclog.Print(LL_INTINFO, VNCLOG("SCR-WBB: desktop optimization enabled.\n"));
		result = TRUE;

		vnclog.Print(LL_INTINFO, VNCLOG("SCR-WBB: attempting to disable pattern.\n"));
		SystemParametersInfo(SPI_SETDESKPATTERN, 0, 0, SPIF_SENDCHANGE);

		vnclog.Print(LL_INTINFO, VNCLOG("SCR-WBB: attempting to disable wallpaper.\n"));
		SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, "", SPIF_SENDCHANGE);

		vnclog.Print(LL_INTINFO, VNCLOG("SCR-WBB: attempting to disable font smoothing.\n"));
		SystemParametersInfo(SPI_SETFONTSMOOTHING, FALSE, 0, SPIF_SENDCHANGE);

		vnclog.Print(LL_INTINFO, VNCLOG("SCR-WBB: attempting to disable full window drags.\n"));
		SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, FALSE, 0, SPIF_SENDCHANGE);

	}

	if (checkdetails != NULL) {
		RegCloseKey(checkdetails);
	}

	return result;
}

void
vncDesktop::ResetDisplayToNormal()
{

	int slen=MAX_REG_ENTRY_LEN;
	int valType;
	char inouttext[MAX_REG_ENTRY_LEN];

	if ( lpDevMode != NULL ) {

		// *** In case the resolution was changed, revert to original settings now
		lpDevMode->dmPelsWidth = origPelsWidth;
		lpDevMode->dmPelsHeight = origPelsHeight;

		long resultOfResChange = ChangeDisplaySettings( lpDevMode, CDS_TEST);
		if (resultOfResChange == DISP_CHANGE_SUCCESSFUL)
			ChangeDisplaySettings( lpDevMode, CDS_UPDATEREGISTRY);

		delete lpDevMode;
		lpDevMode = NULL;
	}

	// *** The following fixes a bug that existed in that WinNT users
	// *** wallpapers reverted back to default system settings, rather than
	// *** their own wallpaper.  The value is now taken from HKEY_CURRENT_USER

	// *** Open the registry key for current users settings
	HKEY checkdetails;
	RegOpenKeyEx(HKEY_CURRENT_USER, 
				"Control Panel\\Desktop",
				0,
				KEY_READ,
				&checkdetails);
	
	memset(inouttext, 0, MAX_REG_ENTRY_LEN);
	
	// *** Get the Wallpaper value - Jeremy Peaks
	RegQueryValueEx(checkdetails,
		"Wallpaper",
		NULL,
		(LPDWORD) &valType,
		(LPBYTE) &inouttext,
		(LPDWORD) &slen);

	// *** Disconnected now, so just in case the wallpaper was disabled
	// *** earlier, revert to saved wallpaper settings. - Jeremy Peaks
	SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, inouttext, SPIF_SENDCHANGE);

	memset(inouttext, 0, MAX_REG_ENTRY_LEN);

	// *** Get the FontSmoothing value - Jeremy Peaks
	RegQueryValueEx(checkdetails,
		"FontSmoothing",
		NULL,
		(LPDWORD) &valType,
		(LPBYTE) &inouttext,
		(LPDWORD) &slen);

	// *** Just in case font smoothing was disabled, re-enable it now
	// *** - Jeremy Peaks
	if (valType == REG_SZ && atoi(inouttext) > 0) {
		SystemParametersInfo(SPI_SETFONTSMOOTHING, TRUE, 0, SPIF_SENDCHANGE);
	}
	
	// *** Get the Pattern value - Jeremy Peaks
	long sllen = MAX_REG_ENTRY_LEN;

	memset(inouttext, 0, MAX_REG_ENTRY_LEN);

	RegQueryValueEx(checkdetails,
		"Pattern",
		NULL,
		(LPDWORD) &valType,
		(LPBYTE) &inouttext,
		(LPDWORD) &sllen);


	// *** Just in case the pattern was disabled, re-enable it now
	// *** - Jeremy Peaks
	SystemParametersInfo(SPI_SETDESKPATTERN, 0, inouttext, SPIF_SENDCHANGE);

	memset(inouttext, 0, MAX_REG_ENTRY_LEN);

	// *** Get the DragFullWindows value - W. Brian Blevins
	RegQueryValueEx(checkdetails,
		"DragFullWindows",
		NULL,
		(LPDWORD) &valType,
		(LPBYTE) &inouttext,
		(LPDWORD) &slen);

	// *** Just in case full window dragging was disabled, re-enable it now
	// *** - W. Brian Blevins
	if ((valType == REG_SZ) &&
		(inouttext[0] == '1')) {

		SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, TRUE, 0, SPIF_SENDCHANGE);

	}
	
	if (checkdetails != NULL) {
		RegCloseKey(checkdetails);
		RegCloseKey(HKEY_CURRENT_USER);
	}

}


BOOL
vncDesktop::InitBitmap()
{
	// Get the device context for the whole screen and find it's size
	m_hrootdc = ::GetDC(NULL);
	if (m_hrootdc == NULL) {
		vnclog.Print(LL_INTERR, VNCLOG("GetDC() failed, error=%d\n"), GetLastError());
		return FALSE;
	}

	m_bmrect.left = m_bmrect.top = 0;
	m_bmrect.right = GetDeviceCaps(m_hrootdc, HORZRES);
	m_bmrect.bottom = GetDeviceCaps(m_hrootdc, VERTRES);
	vnclog.Print(LL_INTINFO, VNCLOG("bitmap dimensions are %d x %d\n"),
				 m_bmrect.right, m_bmrect.bottom);

	// Create a compatible memory DC
	m_hmemdc = CreateCompatibleDC(m_hrootdc);
	if (m_hmemdc == NULL) {
		vnclog.Print(LL_INTERR, VNCLOG("CreateCompatibleDC() failed, error=%d\n"),
					 GetLastError());
		return FALSE;
	}

	// Check that the device capabilities are ok
	if ((GetDeviceCaps(m_hrootdc, RASTERCAPS) & RC_BITBLT) == 0)
	{
		vnclog.Print(LL_INTERR, VNCLOG("BitBlt() is not supported\n"));
		MessageBox(
			NULL,
			"vncDesktop : root device doesn't support BitBlt\n"
			"WinVNC cannot be used with this graphic device driver",
			szAppName,
			MB_ICONSTOP | MB_OK
			);
		return FALSE;
	}
	if ((GetDeviceCaps(m_hmemdc, RASTERCAPS) & RC_DI_BITMAP) == 0)
	{
		vnclog.Print(LL_INTERR, VNCLOG("GetDIBits() is not supported\n"));
		MessageBox(
			NULL,
			"vncDesktop : memory device doesn't support GetDIBits\n"
			"WinVNC cannot be used with this graphics device driver",
			szAppName,
			MB_ICONSTOP | MB_OK
			);
		return FALSE;
	}

	// Create the bitmap to be compatible with the ROOT DC!!!
	m_membitmap = CreateCompatibleBitmap(m_hrootdc, m_bmrect.right, m_bmrect.bottom);
	if (m_membitmap == NULL) {
		vnclog.Print(LL_INTERR, VNCLOG("failed to create memory bitmap, error=%d\n"),
					 GetLastError());
		return FALSE;
	}
	vnclog.Print(LL_INTINFO, VNCLOG("created memory bitmap\n"));

	// Get the bitmap's format and colour details
	m_bminfo.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bminfo.bmi.bmiHeader.biBitCount = 0;
	if (::GetDIBits(m_hmemdc, m_membitmap, 0, 1, NULL,
					&m_bminfo.bmi, DIB_RGB_COLORS) == 0) {
		vnclog.Print(LL_INTERR, VNCLOG("GetDIBits() failed, call #1\n"));
		return FALSE;
	}
	if (::GetDIBits(m_hmemdc, m_membitmap, 0, 1, NULL,
					&m_bminfo.bmi, DIB_RGB_COLORS) == 0) {
		vnclog.Print(LL_INTERR, VNCLOG("GetDIBits() failed, call #2\n"));
		return FALSE;
	}
	vnclog.Print(LL_INTINFO, VNCLOG("got bitmap format\n"));
	vnclog.Print(LL_INTINFO, VNCLOG("DBG:display context has %d planes!\n"), GetDeviceCaps(m_hrootdc, PLANES));
	vnclog.Print(LL_INTINFO, VNCLOG("DBG:memory context has %d planes!\n"), GetDeviceCaps(m_hmemdc, PLANES));
	if (GetDeviceCaps(m_hmemdc, PLANES) != 1)
	{
		vnclog.Print(LL_INTERR, VNCLOG("number of planes in memory context is more than 1\n"));
		MessageBox(
			NULL,
			"vncDesktop : current display is PLANAR, not CHUNKY!\n"
			"WinVNC cannot be used with this graphics device driver",
			szAppName,
			MB_ICONSTOP | MB_OK
			);
		return FALSE;
	}

	// Henceforth we want to use a top-down scanning representation
	m_bminfo.bmi.bmiHeader.biHeight = - abs(m_bminfo.bmi.bmiHeader.biHeight);

	// Is the bitmap palette-based or truecolour?
	m_bminfo.truecolour = (GetDeviceCaps(m_hmemdc, RASTERCAPS) & RC_PALETTE) == 0;

	return TRUE;
}

BOOL
vncDesktop::ThunkBitmapInfo()
{
	// If we leave the pixel format intact, the blits can be optimised (Will Dean's patch)
	m_formatmunged = FALSE;

	// HACK ***.  Optimised blits don't work with palette-based displays, yet
	if (!m_bminfo.truecolour) {
		m_formatmunged = TRUE;
	}

	// Attempt to force the actual format into one we can handle
	// We can handle 8-bit-palette and 16/32-bit-truecolour modes
	switch (m_bminfo.bmi.bmiHeader.biBitCount)
	{
	case 1:
	case 4:
		vnclog.Print(LL_INTINFO, VNCLOG("DBG:used/bits/planes/comp/size "
										"= %d/%d/%d/%d/%d\n"),
					 (int)m_bminfo.bmi.bmiHeader.biClrUsed,
					 (int)m_bminfo.bmi.bmiHeader.biBitCount,
					 (int)m_bminfo.bmi.bmiHeader.biPlanes,
					 (int)m_bminfo.bmi.bmiHeader.biCompression,
					 (int)m_bminfo.bmi.bmiHeader.biSizeImage);
		
		// Correct the BITMAPINFO header to the format we actually want
		m_bminfo.bmi.bmiHeader.biClrUsed = 0;
		m_bminfo.bmi.bmiHeader.biPlanes = 1;
		m_bminfo.bmi.bmiHeader.biCompression = BI_RGB;
		m_bminfo.bmi.bmiHeader.biBitCount = 8;
		m_bminfo.bmi.bmiHeader.biSizeImage =
			abs((m_bminfo.bmi.bmiHeader.biWidth *
				m_bminfo.bmi.bmiHeader.biHeight *
				m_bminfo.bmi.bmiHeader.biBitCount)/ 8);
		m_bminfo.bmi.bmiHeader.biClrImportant = 0;
		m_bminfo.truecolour = FALSE;

		// Display format is non-VNC compatible - use the slow blit method
		m_formatmunged = TRUE;
		break;	
	case 24:
		// Update the bitmapinfo header
		m_bminfo.bmi.bmiHeader.biBitCount = 32;
		m_bminfo.bmi.bmiHeader.biPlanes = 1;
		m_bminfo.bmi.bmiHeader.biCompression = BI_RGB;
		m_bminfo.bmi.bmiHeader.biSizeImage =
			abs((m_bminfo.bmi.bmiHeader.biWidth *
				m_bminfo.bmi.bmiHeader.biHeight *
				m_bminfo.bmi.bmiHeader.biBitCount)/ 8);
		// Display format is non-VNC compatible - use the slow blit method
		m_formatmunged = TRUE;
		break;
	}

	return TRUE;
}

BOOL
vncDesktop::SetPixFormat()
{
	// Examine the bitmapinfo structure to obtain the current pixel format
	m_scrinfo.format.trueColour = m_bminfo.truecolour;
	m_scrinfo.format.bigEndian = 0;

	// Set up the native buffer width, height and format
	m_scrinfo.framebufferWidth = (CARD16) (m_bmrect.right - m_bmrect.left);		// Swap endian before actually sending
	m_scrinfo.framebufferHeight = (CARD16) (m_bmrect.bottom - m_bmrect.top);	// Swap endian before actually sending
	m_scrinfo.format.bitsPerPixel = (CARD8) m_bminfo.bmi.bmiHeader.biBitCount;
	m_scrinfo.format.depth        = (CARD8) m_bminfo.bmi.bmiHeader.biBitCount;

	// Calculate the number of bytes per row
	m_bytesPerRow = m_scrinfo.framebufferWidth * m_scrinfo.format.bitsPerPixel / 8;

	return TRUE;
}

BOOL
vncDesktop::SetPixShifts()
{
	// Sort out the colour shifts, etc.
	DWORD redMask=0, blueMask=0, greenMask = 0;

	switch (m_bminfo.bmi.bmiHeader.biBitCount)
	{
	case 16:
		// Standard 16-bit display
		if (m_bminfo.bmi.bmiHeader.biCompression == BI_RGB)
		{
			// each word single pixel 5-5-5
			redMask = 0x7c00; greenMask = 0x03e0; blueMask = 0x001f;
		}
		else
		{
			if (m_bminfo.bmi.bmiHeader.biCompression == BI_BITFIELDS)
			{
				redMask =   *(DWORD *) &m_bminfo.bmi.bmiColors[0];
				greenMask = *(DWORD *) &m_bminfo.bmi.bmiColors[1];
				blueMask =  *(DWORD *) &m_bminfo.bmi.bmiColors[2];
			}
		}
		break;

	case 32:
		// Standard 24/32 bit displays
		if (m_bminfo.bmi.bmiHeader.biCompression == BI_RGB)
		{
			redMask = 0xff0000;
			greenMask = 0xff00;
			blueMask = 0x00ff;

			// The real color depth is 24 bits in this case. If the depth
			// is set to 32, the Tight encoder shows worse performance.
			m_scrinfo.format.depth = 24;
		}
		else
		{
			if (m_bminfo.bmi.bmiHeader.biCompression == BI_BITFIELDS)
			{
				redMask =   *(DWORD *) &m_bminfo.bmi.bmiColors[0];
				greenMask = *(DWORD *) &m_bminfo.bmi.bmiColors[1];
				blueMask =  *(DWORD *) &m_bminfo.bmi.bmiColors[2];
			}
		}
		break;

	default:
		// Other pixel formats are only valid if they're palette-based
		if (m_bminfo.truecolour)
		{
			vnclog.Print(LL_INTERR, "unsupported truecolour pixel format for SetPixShifts()\n");
			return FALSE;
		}
		vnclog.Print(LL_INTINFO, VNCLOG("DBG:palette-based desktop in SetPixShifts()\n"));
		return TRUE;
	}

	// Convert the data we just retrieved
	MaskToMaxAndShift(redMask, m_scrinfo.format.redMax, m_scrinfo.format.redShift);
	MaskToMaxAndShift(greenMask, m_scrinfo.format.greenMax, m_scrinfo.format.greenShift);
	MaskToMaxAndShift(blueMask, m_scrinfo.format.blueMax, m_scrinfo.format.blueShift);

	vnclog.Print(LL_INTINFO, VNCLOG("DBG:true-color desktop in SetPixShifts()\n"));
	return TRUE;
}

BOOL
vncDesktop::SetPalette()
{
	// Lock the current display palette into the memory DC we're holding
	// *** CHECK THIS FOR LEAKS!
	if (!m_bminfo.truecolour)
	{
		LOGPALETTE *palette;
		UINT size = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 256);

		palette = (LOGPALETTE *) new char[size];
		if (palette == NULL) {
			vnclog.Print(LL_INTERR, VNCLOG("error allocating palette\n"));
			return FALSE;
		}

		// Initialise the structure
		palette->palVersion = 0x300;
		palette->palNumEntries = 256;

		// Get the system colours
		if (GetSystemPaletteEntries(m_hrootdc, 0, 256, palette->palPalEntry) == 0)
		{
			vnclog.Print(LL_INTERR, VNCLOG("GetSystemPaletteEntries() failed.\n"));
			delete [] palette;
			return FALSE;
		}

		// Create a palette from those
		HPALETTE pal = CreatePalette(palette);
		if (pal == NULL)
		{
			vnclog.Print(LL_INTERR, VNCLOG("CreatePalette() failed.\n"));
			delete [] palette;
			return FALSE;
		}

		// Select the palette into our memory DC
		HPALETTE oldpalette = SelectPalette(m_hmemdc, pal, FALSE);
		if (oldpalette == NULL)
		{
			vnclog.Print(LL_INTERR, VNCLOG("SelectPalette() failed.\n"));
			delete [] palette;
			DeleteObject(pal);
			return FALSE;
		}

		// Worked, so realise the palette
		if (RealizePalette(m_hmemdc) == GDI_ERROR)
			vnclog.Print(LL_INTWARN, VNCLOG("warning - failed to RealizePalette\n"));

		// It worked!
		delete [] palette;
		DeleteObject(oldpalette);

		vnclog.Print(LL_INTINFO, VNCLOG("initialised palette OK\n"));
		return TRUE;
	}

	// Not a palette based local screen - forget it!
	vnclog.Print(LL_INTINFO, VNCLOG("no palette data for truecolour display\n"));
	return TRUE;
}

LRESULT CALLBACK DesktopWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

ATOM m_wndClass = 0;

BOOL
vncDesktop::InitWindow()
{
	if (m_wndClass == 0) {
		// Create the window class
		WNDCLASSEX wndclass;

		wndclass.cbSize			= sizeof(wndclass);
		wndclass.style			= 0;
		wndclass.lpfnWndProc	= &DesktopWndProc;
		wndclass.cbClsExtra		= 0;
		wndclass.cbWndExtra		= 0;
		wndclass.hInstance		= hAppInstance;
		wndclass.hIcon			= NULL;
		wndclass.hCursor		= NULL;
		wndclass.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
		wndclass.lpszMenuName	= (const char *) NULL;
		wndclass.lpszClassName	= szDesktopSink;
		wndclass.hIconSm		= NULL;

		// Register it
		m_wndClass = RegisterClassEx(&wndclass);
	}

	// And create a window
	m_hwnd = CreateWindow(szDesktopSink,
				"WinVNC",
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				400, 200,
				NULL,
				NULL,
				hAppInstance,
				NULL);

	if (m_hwnd == NULL) {
		vnclog.Print(LL_INTERR, VNCLOG("CreateWindow() failed.\n"));
		return FALSE;
	}

	// Set the "this" pointer for the window
	SetWindowLong(m_hwnd, GWL_USERDATA, (long)this);

	// Enable clipboard hooking
	m_hnextviewer = SetClipboardViewer(m_hwnd);

	return TRUE;
}

void
vncDesktop::EnableOptimisedBlits()
{
	vnclog.Print(LL_INTINFO, VNCLOG("attempting to enable DIBsection blits\n"));

	if (m_formatmunged) {
		vnclog.Print(LL_INTINFO, VNCLOG("unable to enable fast blits\n"));
		return;
	}

/*
	// Prepare the bitmapinfo palette if necessary (*** DOESN'T QUITE WORK!)
	if (!m_bminfo.truecolour) {
		LOGPALETTE *palette;
		UINT size = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 256);

		palette = (LOGPALETTE *) new char[size];
		if (palette == NULL) {
			vnclog.Print(LL_INTERR, VNCLOG("failed to create temp fast palette - disabling fast blits\n"));
			return;
		}

		// Initialise the structure
		palette->palVersion = 0x300;
		palette->palNumEntries = 256;

		// Get the system colours
		if (GetSystemPaletteEntries(m_hrootdc,
			0, 256, palette->palPalEntry) == 0) {
			delete [] palette;
			vnclog.Print(LL_INTERR, VNCLOG("failed to init fast palette - disabling fast blits\n"));
			return;
		}

		// Copy the system palette into the bitmap info
		for (int i=0; i<256; i++) {
			m_bminfo.bmi.bmiColors[i].rgbReserved = 0;
			m_bminfo.bmi.bmiColors[i].rgbRed = palette->palPalEntry[i].peRed;
			m_bminfo.bmi.bmiColors[i].rgbGreen = palette->palPalEntry[i].peGreen;
			m_bminfo.bmi.bmiColors[i].rgbBlue = palette->palPalEntry[i].peBlue;
		}

		// It worked!
		delete [] palette;

		vnclog.Print(LL_INTINFO, VNCLOG("initialised fast palette OK\n"));
	}
*/

	// Create a new DIB section ***
	HBITMAP tempbitmap = CreateDIBSection(m_hmemdc, &m_bminfo.bmi, DIB_RGB_COLORS, &m_DIBbits, NULL, 0);
	if (tempbitmap == NULL) {
		vnclog.Print(LL_INTWARN, VNCLOG("failed to build DIB section - reverting to slow blits\n"));
		m_DIBbits = NULL;
		return;
	}

	// Delete the old memory bitmap
	if (m_membitmap != NULL) {
		DeleteObject(m_membitmap);
		m_membitmap = NULL;
	}

	// Replace old membitmap with DIB section
	m_membitmap = tempbitmap;

	vnclog.Print(LL_INTINFO, VNCLOG("enabled fast DIBsection blits OK\n"));
}

BOOL
vncDesktop::Init(vncServer *server)
{
	vnclog.Print(LL_INTINFO, VNCLOG("initialising desktop server\n"));

	// Save the server pointer
	m_server = server;

	// Load in the arrow cursor
	m_hdefcursor = LoadCursor(NULL, IDC_ARROW);
	m_hcursor = m_hdefcursor;

	// Spawn a thread to handle that window's message queue
	vncDesktopThread *thread = new vncDesktopThread;
	if (thread == NULL)
		return FALSE;
	m_thread = thread;
	return thread->Init(this, m_server);
}

void
vncDesktop::RequestUpdate()
{
	PostMessage(m_hwnd, WM_TIMER, 0, 0);
}

int
vncDesktop::ScreenBuffSize()
{
	return m_scrinfo.format.bitsPerPixel/8 *
		m_scrinfo.framebufferWidth *
		m_scrinfo.framebufferHeight;
}

void
vncDesktop::FillDisplayInfo(rfbServerInitMsg *scrinfo)
{
	memcpy(scrinfo, &m_scrinfo, sz_rfbServerInitMsg);
}

// Function to capture an area of the screen immediately prior to sending
// an update.
void
vncDesktop::CaptureScreen(RECT &rect, BYTE *scrBuff, UINT scrBuffSize)
{
	// Protect the memory bitmap
	omni_mutex_lock l(m_bitbltlock);

	// Finish drawing anything in this thread 
	// Wish we could do this for the whole system - maybe we should
	// do something with LockWindowUpdate here.
	GdiFlush();

	// Select the memory bitmap into the memory DC
	HBITMAP oldbitmap;
	if ((oldbitmap = (HBITMAP) SelectObject(m_hmemdc, m_membitmap)) == NULL)
		return;

	// Capture screen into bitmap
	BOOL blitok = BitBlt(m_hmemdc, rect.left, rect.top,
		(rect.right-rect.left),
		(rect.bottom-rect.top),
		m_hrootdc, rect.left, rect.top, SRCCOPY);

	// Select the old bitmap back into the memory DC
	SelectObject(m_hmemdc, oldbitmap);

	if (blitok) {
		// Copy the new data to the screen buffer (CopyToBuffer optimises this if possible)
		CopyToBuffer(rect, scrBuff, scrBuffSize);
	}

}

// Add the mouse pointer to the buffer
void
vncDesktop::CaptureMouse(BYTE *scrBuff, UINT scrBuffSize)
{
	// Protect the memory bitmap
	omni_mutex_lock l(m_bitbltlock);

	POINT CursorPos;
	ICONINFO IconInfo;

	// If the mouse cursor handle is invalid then forget it
	if (m_hcursor == NULL)
		return;

	// Get the cursor position
	if (!GetCursorPos(&CursorPos))
		return;

	// Translate position for hotspot
	if (GetIconInfo(m_hcursor, &IconInfo))
	{
		CursorPos.x -= ((int) IconInfo.xHotspot);
		CursorPos.y -= ((int) IconInfo.yHotspot);
		if (IconInfo.hbmMask != NULL)
			DeleteObject(IconInfo.hbmMask);
		if (IconInfo.hbmColor != NULL)
			DeleteObject(IconInfo.hbmColor);
	}

	// Select the memory bitmap into the memory DC
	HBITMAP oldbitmap;
	if ((oldbitmap = (HBITMAP) SelectObject(m_hmemdc, m_membitmap)) == NULL)
		return;

	// Draw the cursor
	DrawIconEx(
		m_hmemdc,									// handle to device context 
		CursorPos.x, CursorPos.y,
		m_hcursor,									// handle to icon to draw 
		0,0,										// width of the icon 
		0,											// index of frame in animated cursor 
		NULL,										// handle to background brush 
		DI_NORMAL									// icon-drawing flags 
		);

	// Select the old bitmap back into the memory DC
	SelectObject(m_hmemdc, oldbitmap);

	// Save the bounding rectangle
	m_cursorpos.left = CursorPos.x;
	m_cursorpos.top = CursorPos.y;
	m_cursorpos.right = CursorPos.x + GetSystemMetrics(SM_CXCURSOR);
	m_cursorpos.bottom = CursorPos.y + GetSystemMetrics(SM_CYCURSOR);

	// Clip the bounding rect to the screen
	RECT screen;
	screen.left=0;
	screen.top=0;
	screen.right=m_scrinfo.framebufferWidth;
	screen.bottom=m_scrinfo.framebufferHeight;

	// Copy the mouse cursor into the screen buffer, if any of it is visible
	if (IntersectRect(&m_cursorpos, &m_cursorpos, &screen))
		CopyToBuffer(m_cursorpos, scrBuff, scrBuffSize);
}

// Obtain cursor image data in server's local format.
// The length of databuf[] should be at least (width * height * 4).
BOOL
vncDesktop::GetRichCursorData(BYTE *databuf, HCURSOR hcursor, int width, int height)
{
	// Protect the memory bitmap (is it really necessary here?)
	omni_mutex_lock l(m_bitbltlock);

	// Create bitmap, select it into memory DC
	HBITMAP membitmap = CreateCompatibleBitmap(m_hrootdc, width, height);
	if (membitmap == NULL) {
		return FALSE;
	}
	HBITMAP oldbitmap = (HBITMAP) SelectObject(m_hmemdc, membitmap);
	if (oldbitmap == NULL) {
		DeleteObject(membitmap);
		return FALSE;
	}

	// Draw the cursor
	DrawIconEx(m_hmemdc, 0, 0, hcursor, 0, 0, 0, NULL, DI_IMAGE);
	SelectObject(m_hmemdc, oldbitmap);

	// Prepare BITMAPINFO structure (copy most m_bminfo fields)
	BITMAPINFO *bmi = (BITMAPINFO *)calloc(1, sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
	memcpy(bmi, &m_bminfo.bmi, sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
	bmi->bmiHeader.biWidth = width;
	bmi->bmiHeader.biHeight = -height;

	// Clear data buffer and extract RGB data
	memset(databuf, 0x00, width * height * 4);
	int lines = GetDIBits(m_hmemdc, membitmap, 0, height, databuf, bmi, DIB_RGB_COLORS);

	// Cleanup
	free(bmi);
	DeleteObject(membitmap);

	return (lines != 0);
}

// Return the current mouse pointer position
RECT
vncDesktop::MouseRect()
{
	return m_cursorpos;
}

void
vncDesktop::SetCursor(HCURSOR cursor)
{
	if (cursor == NULL)
		m_hcursor = m_hdefcursor;
	else
		m_hcursor = cursor;
}

// Manipulation of the clipboard
void
vncDesktop::SetClipText(LPSTR text)
{
	// Open the system clipboard
	if (OpenClipboard(m_hwnd))
	{
		// Empty it
		if (EmptyClipboard())
		{
			HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, strlen(text)+1);

			if (hMem != NULL)
			{
				LPSTR pMem = (char*)GlobalLock(hMem);

				// Get the data
				strcpy(pMem, text);

				// Tell the clipboard
				GlobalUnlock(hMem);
				SetClipboardData(CF_TEXT, hMem);
			}
		}
	}

	// Now close it
	CloseClipboard();
}

// INTERNAL METHODS

inline void
vncDesktop::MaskToMaxAndShift(DWORD mask, CARD16 &max, CARD8 &shift)
{
	for (shift = 0; (mask & 1) == 0; shift++)
		mask >>= 1;
	max = (CARD16) mask;
}

// Copy data from the memory bitmap into a buffer
void
vncDesktop::CopyToBuffer(RECT &rect, BYTE *destbuff, UINT destbuffsize)
{
	// Are we being asked to blit from the DIBsection to itself?
	if (destbuff == m_DIBbits) {
		// Yes.  Ignore the request!
		return;
	}

	// Protect the memory bitmap
	omni_mutex_lock l(m_bitbltlock);

	int y_inv;
	BYTE * destbuffpos;

	// Calculate the scanline-ordered y position to copy from
	y_inv = m_scrinfo.framebufferHeight-rect.top-(rect.bottom-rect.top);

	// Calculate where in the output buffer to put the data
	destbuffpos = destbuff + (m_bytesPerRow * rect.top);

	// Set the number of bytes for GetDIBits to actually write
	// NOTE : GetDIBits pads the destination buffer if biSizeImage < no. of bytes required
	m_bminfo.bmi.bmiHeader.biSizeImage = (rect.bottom-rect.top) * m_bytesPerRow;

	// Get the actual bits from the bitmap into the bit buffer
	// If fast (DIBsection) blits are disabled then use the old GetDIBits technique
	if (m_DIBbits == NULL) {
		if (GetDIBits(m_hmemdc, m_membitmap, y_inv,
					(rect.bottom-rect.top), destbuffpos,
					&m_bminfo.bmi, DIB_RGB_COLORS) == 0)
		{
#ifdef _MSC_VER
			_RPT1(_CRT_WARN, "vncDesktop : [1] GetDIBits failed! %d\n", GetLastError());
			_RPT3(_CRT_WARN, "vncDesktop : thread = %d, DC = %d, bitmap = %d\n", omni_thread::self(), m_hmemdc, m_membitmap);
			_RPT2(_CRT_WARN, "vncDesktop : y = %d, height = %d\n", y_inv, (rect.bottom-rect.top));
#endif
		}
	} else {
		// Fast blits are enabled.  [I have a sneaking suspicion this will never get used, unless
		// something weird goes wrong in the code.  It's here to keep the function general, though!]

		int bytesPerPixel = m_scrinfo.format.bitsPerPixel / 8;
		BYTE *srcbuffpos = (BYTE*)m_DIBbits;

		srcbuffpos += (m_bytesPerRow * rect.top) + (bytesPerPixel * rect.left);
		destbuffpos += bytesPerPixel * rect.left;

		int widthBytes = (rect.right-rect.left) * bytesPerPixel;

		for(int y = rect.top; y < rect.bottom; y++)
		{
			memcpy(destbuffpos, srcbuffpos, widthBytes);
			srcbuffpos += m_bytesPerRow;
			destbuffpos += m_bytesPerRow;
		}
	}
}

// Callback routine used internally to catch window movement...
BOOL CALLBACK
EnumWindowsFn(HWND hwnd, LPARAM arg)
{
	HANDLE prop = GetProp(hwnd, (LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0));
	if (prop != NULL)
	{
		if (IsWindowVisible(hwnd))
		{
			RECT dest;
			POINT source;

			// Get the window rectangle
			if (GetWindowRect(hwnd, &dest))
			{
				// Old position
				source.x = (SHORT) LOWORD(prop);
				source.y = (SHORT) HIWORD(prop);

				// Got the destination position.  Now send to clients!
				if ((source.x != dest.left) || (source.y != dest.top))
				{
					// Update the property entry
					SHORT x = (SHORT) dest.left;
					SHORT y = (SHORT) dest.top;
					SetProp(hwnd,
						(LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0),
						(HANDLE) MAKELONG(x, y));

					// Notify all clients of the copyrect
					((vncServer*)arg)->CopyRect(dest, source);

					// Get the client to check for damage
					((vncServer*)arg)->UpdateRect(dest);
				}
			}
			else
				RemoveProp(hwnd, (LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0));
		}
		else
		{
			RemoveProp(hwnd, (LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0));
		}
	}
	else
	{
		// If the window has become visible then save its position!
		if (IsWindowVisible(hwnd))
		{
			RECT dest;

			if (GetWindowRect(hwnd, &dest))
			{
				SHORT x = (SHORT) dest.left;
				SHORT y = (SHORT) dest.top;
				SetProp(hwnd,
					(LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0),
					(HANDLE) MAKELONG(x, y));
			}
		}
	}

	return TRUE;
}

// Routine to find out which windows have moved
void
vncDesktop::CalcCopyRects()
{
	// Enumerate all the desktop windows for movement
	EnumWindows((WNDENUMPROC)EnumWindowsFn, (LPARAM) m_server);
}

// Window procedure for the Desktop window
LRESULT CALLBACK
DesktopWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	vncDesktop *_this = (vncDesktop*)GetWindowLong(hwnd, GWL_USERDATA);

	switch (iMsg)
	{

		// GENERAL

	case WM_DISPLAYCHANGE:
		// The display resolution is changing

		// We must kick off any clients since their screen size will be wrong
		_this->m_displaychanged = TRUE;
		return 0;

	case WM_SYSCOLORCHANGE:
	case WM_PALETTECHANGED:
		// The palette colours have changed, so tell the server

		// Get the system palette
		if (!_this->SetPalette())
			PostQuitMessage(0);

		// Update any palette-based clients, too
		_this->m_server->UpdatePalette();
		return 0;

		// CLIPBOARD MESSAGES

	case WM_CHANGECBCHAIN:
		// The clipboard chain has changed - check our nextviewer handle
		if ((HWND)wParam == _this->m_hnextviewer)
			_this->m_hnextviewer = (HWND)lParam;
		else
			if (_this->m_hnextviewer != NULL)
				SendMessage(_this->m_hnextviewer,
							WM_CHANGECBCHAIN,
							wParam, lParam);

		return 0;

	case WM_DRAWCLIPBOARD:
		// The clipboard contents have changed
		if((GetClipboardOwner() != _this->Window()) &&
		    _this->m_initialClipBoardSeen &&
			_this->m_clipboard_active)
		{
			LPSTR cliptext = NULL;

			// Open the clipboard
			if (OpenClipboard(_this->Window()))
			{
				// Get the clipboard data
				HGLOBAL cliphandle = GetClipboardData(CF_TEXT);
				if (cliphandle != NULL)
				{
					LPSTR clipdata = (LPSTR) GlobalLock(cliphandle);

					// Copy it into a new buffer
					if (clipdata == NULL)
						cliptext = NULL;
					else
						cliptext = strdup(clipdata);

					// Release the buffer and close the clipboard
					GlobalUnlock(cliphandle);
				}

				CloseClipboard();
			}

			if (cliptext != NULL)
			{
				int cliplen = strlen(cliptext);
				LPSTR unixtext = (char *)malloc(cliplen+1);

				// Replace CR-LF with LF - never send CR-LF on the wire,
				// since Unix won't like it
				int unixpos=0;
				for (int x=0; x<cliplen; x++)
				{
					if (cliptext[x] != '\x0d')
					{
						unixtext[unixpos] = cliptext[x];
						unixpos++;
					}
				}
				unixtext[unixpos] = 0;

				// Free the clip text
				free(cliptext);
				cliptext = NULL;

				// Now send the unix text to the server
				_this->m_server->UpdateClipText(unixtext);

				free(unixtext);
			}
		}

		_this->m_initialClipBoardSeen = TRUE;

		if (_this->m_hnextviewer != NULL)
		{
			// Pass the message to the next window in clipboard viewer chain.  
			return SendMessage(_this->m_hnextviewer, WM_DRAWCLIPBOARD, 0,0); 
		}

		return 0;

	default:
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}
}
