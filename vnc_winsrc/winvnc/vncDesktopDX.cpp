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
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.

// *** JNW
// THIS VERSION OF VNCDESKTOP TRIES TO USE DIRECTX TO INCREASE PERFORMANCE.
// THE PERFORMANCE INCREASE IS NOT VERY IMPRESSIVE...

// vncDesktop implementation

// System headers
#include "stdhdrs.h"
#include <omnithread.h>

// *** JNW
#include <ddraw.h>

// Custom headers
#include "WinVNC.h"
#include "VNCHooks\VNCHooks.h"
#include "vncServer.h"
#include "vncRegion.h"
#include "rectlist.h"
#include "vncDesktop.h"
#include "vncService.h"

// Constants
const UINT RFB_SCREEN_UPDATE = RegisterWindowMessage("WinVNC.Update.DrawRect");
const UINT RFB_COPYRECT_UPDATE = RegisterWindowMessage("WinVNC.Update.CopyRect");
const UINT RFB_MOUSE_UPDATE = RegisterWindowMessage("WinVNC.Update.Mouse");
const char szDesktopSink[] = "WinVNC desktop sink";

// Atoms
const char *VNC_WINDOWPOS_ATOMNAME = "VNCHooks.CopyRect.WindowPos";
ATOM VNC_WINDOWPOS_ATOM = NULL;

// *** TIMING
DWORD captureticks = 0;
DWORD updateticks = 0;

// ***> JNW - DIRECTX SYSTEM!
LPDIRECTDRAW lpDD = NULL;
LPDIRECTDRAWSURFACE lpDDSPrime = NULL;
DDSURFACEDESC DDSdesc;

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
	// Attempt to initialise and return success or failure
	if (!m_desktop->Startup())
	{
		ReturnVal(FALSE);
		return NULL;
	}

	// Succeeded to initialise ok
	ReturnVal(TRUE);

	// START PROCESSING DESKTOP MESSAGES

	// Add the window into the clipboard system, so we get notified if it changes
	// *** This MUST be done AFTER ReturnVal, otherwise a deadlock can occur,
	// This is because when the SetClipboardViewer function is called, it causes
	// this thread to attempt to update any clients' clipboards, which involves
	// locking the vncServer object.  But the vncServer object is already locked
	// by the thread which is waiting for ReturnVal to be called...
	m_desktop->m_hnextviewer = SetClipboardViewer(m_desktop->Window());

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
			// *** TIMINGS
			DWORD ticks = GetTickCount();

			// Thread has gone idle.  Now would be a good time to send an update.
			// First, we must check that the screen hasnt changed too much.

			// Has the display resolution or desktop changed?
			if (m_desktop->m_displaychanged || !vncService::InputDesktopSelected())
			{
				rfbServerInitMsg oldscrinfo = m_desktop->m_scrinfo;
				m_desktop->m_displaychanged = FALSE;

				// Attempt to close the old hooks
				if (!m_desktop->Shutdown())
				{
					m_server->KillAll();
					break;
				}

				// Now attempt to re-install them!
				if (!m_desktop->Startup())
				{
					m_server->KillAll();
					break;
				}

				// Check that the screen info hasn't changed
				vnclog.Print(LL_INTINFO, VNCLOG("SCR: old screen format "
												"%dx%dx%d\n"),
							 oldscrinfo.framebufferWidth,
							 oldscrinfo.framebufferHeight,
							 oldscrinfo.format.bitsPerPixel);
				vnclog.Print(LL_INTINFO, VNCLOG("SCR: new screen format "
												"%dx%dx%d\n"),
							 m_desktop->m_scrinfo.framebufferWidth,
							 m_desktop->m_scrinfo.framebufferHeight,
							 m_desktop->m_scrinfo.format.bitsPerPixel);
				if (memcmp(&m_desktop->m_scrinfo, &oldscrinfo, sizeof(oldscrinfo)) != 0)
				{
					m_server->KillAll();
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

			// *** TIMINGS
			updateticks += GetTickCount() - ticks;
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

	vnclog.Print(LL_INTINFO, VNCLOG("quitting desktop server thread\n"));

	// Clear all the hooks and close windows, etc.
	m_desktop->Shutdown();

	// Clear the shift modifier keys, now that there are no remote clients
	vncKeymap::ClearShiftKeys();

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

}

vncDesktop::~vncDesktop()
{
	// *** TIMING
	vnclog.Print(LL_INTERR, VNCLOG("TIMINGS : captureticks = %d\n"), captureticks);
	vnclog.Print(LL_INTERR, VNCLOG("TIMINGS : updateticks = %d\n"), updateticks);

	vnclog.Print(LL_INTINFO, VNCLOG("killing screen server\n"));

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
	// Initialise the Desktop object

	KillScreenSaver();

	if (!InitDesktop())
		return FALSE;

	if (!InitBitmap())
		return FALSE;

	if (!ThunkBitmapInfo())
		return FALSE;

	if (!SetPixFormat())
		return FALSE;

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
//		return FALSE;
	}

	// Start a timer to handle Polling Mode.  The timer will cause
	// an "idle" event once every second, which is necessary if Polling
	// Mode is being used, to cause TriggerUpdate to be called.
	m_timerid = SetTimer(m_hwnd, 1, 1000, NULL);

	// Get hold of the WindowPos atom!
	if ((VNC_WINDOWPOS_ATOM = GlobalAddAtom(VNC_WINDOWPOS_ATOMNAME)) == 0)
		return FALSE;

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

	// *** DIRECTX CLEANUP
	if (lpDDSPrime != NULL)
	{
		lpDDSPrime->Release();
		lpDDSPrime = NULL;
	}
	if (lpDD != NULL)
	{
		lpDD->Release();
		lpDD = NULL;
	}
	// ***

	// Free the WindowPos atom!
	if (VNC_WINDOWPOS_ATOM != NULL)
		GlobalDeleteAtom(VNC_WINDOWPOS_ATOM);

	return TRUE;
}

// Routine to ensure we're on the correct NT desktop
// This routine calls the vncNTOnly class, which implements NT specific WinVNC functions

BOOL
vncDesktop::InitDesktop()
{
	// Ask for the current input desktop
	return vncService::SelectDesktop(NULL);
}

// Routine used to close the screen saver, if it's active...

BOOL CALLBACK
KillScreenSaverFunc(HWND hwnd, LPARAM lParam)
{
	PostMessage(hwnd, WM_CLOSE, 0, 0);
	return TRUE;
}

void
vncDesktop::KillScreenSaver()
{
	OSVERSIONINFO osversioninfo;
	osversioninfo.dwOSVersionInfoSize = sizeof(osversioninfo);

	// *** If we're running as a service then we don't kill the screensaver
	if (vncService::RunningAsService())
		return;

	// Get the current OS version
	if (!GetVersionEx(&osversioninfo))
		return;

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
				// Close all windows on the screen saver desktop
				EnumDesktopWindows(hDesk, (WNDENUMPROC) &KillScreenSaverFunc, 0);
				CloseDesktop(hDesk);
				// Pause long enough for the screen-saver to close
				Sleep(2000);
				// Reset the screen saver so it can run again
				SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, SPIF_SENDWININICHANGE); 
			}
			break;
		}
	}
}

BOOL
vncDesktop::InitBitmap()
{
	// *** DIRECTX INIT!
	try {
		HRESULT ddrval = DirectDrawCreate(0, &lpDD, 0);
		if (ddrval != DD_OK)
			throw "Failed to open DirectDraw!";

		ddrval = lpDD->SetCooperativeLevel(NULL, DDSCL_NORMAL);
		if (ddrval != DD_OK)
			throw "Failed to set cooperative level!";

		ZeroMemory(&DDSdesc, sizeof(DDSdesc));
		DDSdesc.dwSize = sizeof(DDSdesc);
		DDSdesc.dwFlags = DDSD_CAPS;
		DDSdesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		ddrval = lpDD->CreateSurface(&DDSdesc, &lpDDSPrime, 0);
		if (ddrval != DD_OK)
			throw "Failed to create primary surface!";
		ZeroMemory(&DDSdesc, sizeof(DDSdesc));
		DDSdesc.dwSize = sizeof(DDSdesc);
		DDSdesc.dwFlags = DDSD_ALL;
		ddrval = lpDDSPrime->GetSurfaceDesc(&DDSdesc);
		if (ddrval != DD_OK)
			throw "Failed to get surface description!";

		if (DDSdesc.dwFlags & DDSD_WIDTH)
			vnclog.Print(LL_INTERR, VNCLOG("surface_w = %d\n"), DDSdesc.dwWidth);
		if (DDSdesc.dwFlags & DDSD_HEIGHT)
			vnclog.Print(LL_INTERR, VNCLOG("surface_h = %d\n"), DDSdesc.dwHeight);
		if (DDSdesc.dwFlags & DDSD_PITCH)
			vnclog.Print(LL_INTERR, VNCLOG("surface_pitch = %d\n"), DDSdesc.lPitch);
		if (DDSdesc.dwFlags & DDSD_PIXELFORMAT)
		{
			vnclog.Print(LL_INTERR, VNCLOG("surface_rgb = %s\n"),
				DDSdesc.ddpfPixelFormat.dwFlags & DDPF_RGB ? "yes" : "no");
			vnclog.Print(LL_INTERR, VNCLOG("surface_rmask = %x\n"),
				DDSdesc.ddpfPixelFormat.dwRBitMask);
			vnclog.Print(LL_INTERR, VNCLOG("surface_gmask = %x\n"),
				DDSdesc.ddpfPixelFormat.dwGBitMask);
			vnclog.Print(LL_INTERR, VNCLOG("surface_bmask = %x\n"),
				DDSdesc.ddpfPixelFormat.dwBBitMask);
		}

		vnclog.Print(LL_INTERR, VNCLOG("DirectX started OK.\n"));
	} catch (const char *str) {
		vnclog.Print(LL_INTERR, VNCLOG("%s\n"), str);
		return FALSE;
	};

	// *** DIRECTX CONTD
	if ((DDSdesc.dwFlags & DDSD_WIDTH) && (DDSdesc.dwFlags & DDSD_HEIGHT))
	{
		m_bmrect.left = m_bmrect.top = 0;
		m_bmrect.right = DDSdesc.dwWidth;
		m_bmrect.bottom = DDSdesc.dwHeight;
	}
	else
	{
		vnclog.Print(LL_INTERR, VNCLOG("DX - surface dimensions not given\n"));
		return FALSE;
	}

	// Fake standard bitmap info for now
	*(DWORD *) &m_bminfo.bmi.bmiColors[0] = DDSdesc.ddpfPixelFormat.dwRBitMask;
	*(DWORD *) &m_bminfo.bmi.bmiColors[1] = DDSdesc.ddpfPixelFormat.dwGBitMask;
	*(DWORD *) &m_bminfo.bmi.bmiColors[2] = DDSdesc.ddpfPixelFormat.dwBBitMask;
	m_bminfo.bmi.bmiHeader.biCompression = BI_BITFIELDS;
	m_bminfo.bmi.bmiHeader.biBitCount = DDSdesc.ddpfPixelFormat.dwRGBBitCount;
	m_bminfo.truecolour = DDSdesc.ddpfPixelFormat.dwFlags & DDPF_RGB;

	// *** END DIRECTX

	return TRUE;
}

BOOL
vncDesktop::ThunkBitmapInfo()
{
	/*
	// Attempt to force the actual format into one we can handle
	// We can handle 8-bit-palette and 16/32-bit-truecolour modes
	switch (m_bminfo.bmi.bmiHeader.biBitCount)
	{
	case 1:
	case 4:
		// Correct the BITMAPINFO header to the format we actually want
		m_bminfo.bmi.bmiHeader.biClrUsed = 1 << m_bminfo.bmi.bmiHeader.biBitCount;
		m_bminfo.bmi.bmiHeader.biBitCount = 8;
		m_bminfo.bmi.bmiHeader.biSizeImage =
			abs((m_bminfo.bmi.bmiHeader.biWidth *
				m_bminfo.bmi.bmiHeader.biHeight *
				m_bminfo.bmi.bmiHeader.biBitCount)/ 8);
		m_bminfo.bmi.bmiHeader.biClrImportant = 0;
		m_bminfo.truecolour = FALSE;
		break;	
	case 24:
		// Update the bitmapinfo header
		m_bminfo.bmi.bmiHeader.biBitCount = 32;
		m_bminfo.bmi.bmiHeader.biSizeImage =
			abs((m_bminfo.bmi.bmiHeader.biWidth *
				m_bminfo.bmi.bmiHeader.biHeight *
				m_bminfo.bmi.bmiHeader.biBitCount)/ 8);
		break;
	}
*/
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
			redMask = 0xff0000; greenMask = 0xff00; blueMask = 0x00ff;
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
			vnclog.Print(LL_INTERR, "unsupported truecolour pixel format for setpixshifts\n");
			return FALSE;
		}
		return TRUE;
	}

	// Convert the data we just retrieved
	MaskToMaxAndShift(redMask, m_scrinfo.format.redMax, m_scrinfo.format.redShift);
	MaskToMaxAndShift(greenMask, m_scrinfo.format.greenMax, m_scrinfo.format.greenShift);
	MaskToMaxAndShift(blueMask, m_scrinfo.format.blueMax, m_scrinfo.format.blueShift);

	return TRUE;
}

BOOL
vncDesktop::SetPalette()
{
	// Lock the current display palette into the memory DC we're holding
	// *** CHECK THIS FOR LEAKS!

	// *** DIRECTX
	if (!m_bminfo.truecolour)
		return FALSE;

/*	
	if (!m_bminfo.truecolour)
	{
		LOGPALETTE *palette;
		UINT size = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 256);

		palette = (LOGPALETTE *) new char[size];
		if (palette == NULL)
			return FALSE;

		// Initialise the structure
		palette->palVersion = 0x300;
		palette->palNumEntries = 256;

		// Get the system colours
		if (GetSystemPaletteEntries(m_hrootdc,
			0, 256, palette->palPalEntry) == 0)
		{
			delete [] palette;
			return FALSE;
		}

		// Create a palette from those
		HPALETTE pal = CreatePalette(palette);
		if (pal == NULL)
		{
			delete [] palette;
			return FALSE;
		}

		// Select the palette into our memory DC
		HPALETTE oldpalette = SelectPalette(m_hmemdc, pal, FALSE);
		if (oldpalette == NULL)
		{
			delete [] palette;
			DeleteObject(pal);
			return FALSE;
		}

		// Worked, so realise the palette
		RealizePalette(m_hmemdc);

		// It worked!
		delete [] palette;
		DeleteObject(oldpalette);
		return TRUE;
	}
*/
	// Not a palette based local screen - forget it!
	return TRUE;
}

LRESULT CALLBACK DesktopWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

BOOL
vncDesktop::InitWindow()
{
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
	RegisterClassEx(&wndclass);

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

	if (m_hwnd == NULL)
		return FALSE;

	// Set the "this" pointer for the window
	SetWindowLong(m_hwnd, GWL_USERDATA, (long)this);

	return TRUE;
}

BOOL
vncDesktop::Init(vncServer *server)
{
	vnclog.Print(LL_INTINFO, VNCLOG("initialising desktop handler\n"));

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
	DWORD ticks = GetTickCount();

	// Protect the memory bitmap
	omni_mutex_lock l(m_bitbltlock);

	// *** DIRECTX
	DDSURFACEDESC surfdesc;
	ZeroMemory(&surfdesc, sizeof(surfdesc)); 
	surfdesc.dwSize = sizeof(surfdesc);
	HRESULT ddrval = lpDDSPrime->Lock(&rect, &surfdesc, DDLOCK_READONLY | DDLOCK_WAIT, 0);
	if (ddrval != DD_OK)
		return;

	// vnclog.Print(LL_INTERR, VNCLOG("surface pitch = %d\n"), surfdesc.lPitch);

	// Now copy the data into our buffer
	BYTE * destbuffpos, * srcbuffpos;

	// Calculate where in the output buffer to put the data
	DWORD bytesPerRow = (rect.right-rect.left) * m_scrinfo.format.bitsPerPixel/8;
	srcbuffpos = (BYTE *) surfdesc.lpSurface;
/*+
		(surfdesc.lPitch * rect.top) +
		(m_scrinfo.format.bitsPerPixel/8 * rect.left);
*/
	destbuffpos = scrBuff +
		(m_bytesPerRow * rect.top) +
		(m_scrinfo.format.bitsPerPixel/8 * rect.left);

	for (int ypos = rect.top; ypos < rect.bottom; ypos ++)
	{
		memcpy(destbuffpos, srcbuffpos, bytesPerRow);
		destbuffpos += m_bytesPerRow;
		srcbuffpos += surfdesc.lPitch;
	}

	// And unlock the primary surface!
	lpDDSPrime->Unlock(surfdesc.lpSurface);

	// *** TIMING CODE
	captureticks += GetTickCount() - ticks;
}

// Add the mouse pointer to the buffer
void
vncDesktop::CaptureMouse(BYTE *scrBuff, UINT scrBuffSize)
{
/*
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
	if ((m_oldbitmap = (HBITMAP) SelectObject(m_hmemdc, m_membitmap)) == NULL)
		return;

	// Draw the cursor
	DrawIconEx(
		m_hmemdc,									// handle to device context 
		CursorPos.x, CursorPos.y,
		m_hcursor,									// handle to icon to draw 
		0,0,										// width of the icon 
		0,											// index of frame in animated cursor 
		NULL,										// handle to background brush 
		DI_NORMAL | DI_COMPAT						// icon-drawing flags 
		);

	// Select the old bitmap back into the memory DC
	SelectObject(m_hmemdc, m_oldbitmap);

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
*/
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
					SHORT x = dest.left;
					SHORT y = dest.top;
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
				SHORT x = dest.left;
				SHORT y = dest.top;
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
		if(GetClipboardOwner() != _this->Window())
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
