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


// WinVNC.cpp

// 24/11/97		WEZ

// WinMain and main WndProc for the new version of WinVNC

////////////////////////////
// System headers
#include "stdhdrs.h"

////////////////////////////
// Custom headers
#include "VSocket.h"
#include "WinVNC.h"

#include "vncServer.h"
#include "vncMenu.h"
#include "vncInstHandler.h"
#include "vncService.h"

// Application instance and name
HINSTANCE	hAppInstance;
const char	*szAppName = "WinVNC";
DWORD		mainthreadId;

// WinMain parses the command line and either calls the main App
// routine or, under NT, the main service routine.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
#ifdef _DEBUG
	{
		// Get current flag
		int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

		// Turn on leak-checking bit
		tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

		// Set flag to the new value
		_CrtSetDbgFlag( tmpFlag );
	}
#endif

	// Save the application instance and main thread id
	hAppInstance = hInstance;
	mainthreadId = GetCurrentThreadId();

	// Initialise the VSocket system
	VSocketSystem socksys;
	if (!socksys.Initialised())
	{
		MessageBox(NULL, "Failed to initialise the socket system", szAppName, MB_OK);
		return 0;
	}
	vnclog.Print(LL_STATE, VNCLOG("sockets initialised\n"));

	// Make the command-line lowercase and parse it
	size_t i;
	for (i = 0; i < strlen(szCmdLine); i++)
	{
		szCmdLine[i] = tolower(szCmdLine[i]);
	}

	BOOL argfound = FALSE;
	for (i = 0; i < strlen(szCmdLine); i++)
	{
		if (szCmdLine[i] <= ' ')
			continue;
		argfound = TRUE;

		// Now check for command-line arguments
		if (strncmp(&szCmdLine[i], winvncRunServiceHelper, strlen(winvncRunServiceHelper)) == 0)
		{
			// NB : This flag MUST be parsed BEFORE "-service", otherwise it will match
			// the wrong option!  (This code should really be replaced with a simple
			// parser machine and parse-table...)

			// Run the WinVNC Service Helper app
			vncService::PostUserHelperMessage();
			return 0;
		}
		if (strncmp(&szCmdLine[i], winvncRunService, strlen(winvncRunService)) == 0)
		{
			// Run WinVNC as a service
			return vncService::WinVNCServiceMain();
		}
		if (strncmp(&szCmdLine[i], winvncRunAsUserApp, strlen(winvncRunAsUserApp)) == 0)
		{
			// WinVNC is being run as a user-level program
			return WinVNCAppMain();
		}
		if (strncmp(&szCmdLine[i], winvncInstallService, strlen(winvncInstallService)) == 0)
		{
			// Install WinVNC as a service
			vncService::InstallService();
			i+=strlen(winvncInstallService);
			continue;
		}
		if (strncmp(&szCmdLine[i], winvncReinstallService, strlen(winvncReinstallService)) == 0)
		{
			// Silently remove WinVNC, then re-install it
			vncService::ReinstallService();
			i+=strlen(winvncReinstallService);
			continue;
		}
		if (strncmp(&szCmdLine[i], winvncRemoveService, strlen(winvncRemoveService)) == 0)
		{
			// Remove the WinVNC service
			vncService::RemoveService();
			i+=strlen(winvncRemoveService);
			continue;
		}
		if (strncmp(&szCmdLine[i], winvncReload, strlen(winvncReload)) == 0)
		{
			// Reload Properties from the registry
			vncService::PostReloadMessage();
			i+=strlen(winvncReload);
			continue;
		}
		if (strncmp(&szCmdLine[i], winvncShowProperties, strlen(winvncShowProperties)) == 0)
		{
			// Show the Properties dialog of an existing instance of WinVNC
			vncService::ShowProperties();
			i+=strlen(winvncShowProperties);
			continue;
		}
		if (strncmp(&szCmdLine[i], winvncShowDefaultProperties, strlen(winvncShowDefaultProperties)) == 0)
		{
			// Show the Properties dialog of an existing instance of WinVNC
			vncService::ShowDefaultProperties();
			i+=strlen(winvncShowDefaultProperties);
			continue;
		}
		if (strncmp(&szCmdLine[i], winvncShowAbout, strlen(winvncShowAbout)) == 0)
		{
			// Show the About dialog of an existing instance of WinVNC
			vncService::ShowAboutBox();
			i+=strlen(winvncShowAbout);
			continue;
		}
		if (strncmp(&szCmdLine[i], winvncKillRunningCopy, strlen(winvncKillRunningCopy)) == 0)
		{
			// Kill any already running copy of WinVNC
			vncService::KillRunningCopy();
			i+=strlen(winvncKillRunningCopy);
			continue;
		}
		if (strncmp(&szCmdLine[i], winvncAddNewClient, strlen(winvncAddNewClient)) == 0)
		{
			// Add a new client to an existing copy of winvnc
			i+=strlen(winvncAddNewClient);

			// First, we have to parse the command line to get the hostname to use
			int start, end;
			start=i;
			while (szCmdLine[start] && szCmdLine[start] <= ' ') start++;
			end = start;
			while (szCmdLine[end] > ' ') end++;

			// Was there a hostname (and optionally a port number) given?
			if (end-start > 0) {
				char *name = new char[end-start+1];
				if (name != NULL) {
					strncpy(name, &(szCmdLine[start]), end-start);
					name[end-start] = 0;

					int port = INCOMING_PORT_OFFSET;
					char *portp = strchr(name, ':');
					if (portp != NULL) {
						*portp++ = '\0';
						if (*portp == ':') {
							port = atoi(++portp);
						} else {
							port += atoi(portp);
						}
					}

					VCard32 address = VSocket::Resolve(name);
					if (address != 0) {
						// Post the IP address to the server
						vncService::PostAddNewClient(address, port);
					}
					delete [] name;
				}
			} else {
				// Tell the server to show the Add New Client dialog
				vncService::PostAddNewClient(0, 0);
			}
			i = end;
			continue;
		}

		// Either the user gave the -help option or there is something odd on the cmd-line!

		// Show the usage dialog
		MessageBox(NULL, winvncUsageText, "WinVNC Usage", MB_OK | MB_ICONINFORMATION);
		break;
	};

	// If no arguments were given then just run
	if (!argfound)
		return WinVNCAppMain();

	return 0;
}

// This is the main routine for WinVNC when running as an application
// (under Windows 95 or Windows NT)
// Under NT, WinVNC can also run as a service.  The WinVNCServerMain routine,
// defined in the vncService header, is used instead when running as a service.

int WinVNCAppMain()
{
	// Set this process to be the last application to be shut down.
	SetProcessShutdownParameters(0x100, 0);
	
	// Check for previous instances of WinVNC!
	vncInstHandler instancehan;
	if (!instancehan.Init())
	{
		// We don't allow multiple instances!
		MessageBox(NULL, "Another instance of WinVNC is already running", szAppName, MB_OK);
		return 0;
	}

	// CREATE SERVER
	vncServer server;

	// Set the name and port number
	server.SetName(szAppName);
	vnclog.Print(LL_STATE, VNCLOG("server created ok\n"));

	// Create tray icon & menu if we're running as an app
	vncMenu *menu = new vncMenu(&server);
	if (menu == NULL)
	{
		vnclog.Print(LL_INTERR, VNCLOG("failed to create tray menu\n"));
		PostQuitMessage(0);
	}

	// Now enter the message handling loop until told to quit!
	MSG msg;
	while (GetMessage(&msg, NULL, 0,0) ) {
		vnclog.Print(LL_INTINFO, VNCLOG("message %d received\n"), msg.message);

		TranslateMessage(&msg);  // convert key ups and downs to chars
		DispatchMessage(&msg);
	}

	vnclog.Print(LL_STATE, VNCLOG("shutting down server\n"));

	if (menu != NULL)
		delete menu;

	return msg.wParam;
};
