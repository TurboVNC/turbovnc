//  Copyright (C) 2003 Constantin Kaplinsky. All Rights Reserved.
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

// vncCapsContainer.h

#if (!defined(_VNC_CAPSCONTAINER))
#define _VNC_CAPSCONTAINER

// Disable warnings about truncated names caused by #include <map>
#pragma warning(disable: 4786)

#include "stdhdrs.h"
#include "rfb.h"

#include <map>

class CapsContainer
{
public:
	CapsContainer(int maxCaps = 64);
	~CapsContainer();

	void Add(const rfbCapabilityInfo *capinfo, const char *desc = NULL);
	void Add(CARD32 code, const char *vendor, const char *name,
			 const char *desc = NULL);

	bool IsKnown(CARD32 code);
	bool GetInfo(CARD32 code, rfbCapabilityInfo *capinfo);
	char *GetDescription(CARD32 code);

	bool Enable(const rfbCapabilityInfo *capinfo);
	bool IsEnabled(CARD32 code);
	int NumEnabled() { return listSize; }
	CARD32 GetByOrder(int idx);

private:
	std::map<CARD32,rfbCapabilityInfo> infoMap;
	std::map<CARD32,char*> descMap;
	std::map<CARD32,bool> enableMap;

	int maxSize;
	int listSize;
	CARD32 *plist;
};

#endif // _VNC_CAPSCONTAINER

