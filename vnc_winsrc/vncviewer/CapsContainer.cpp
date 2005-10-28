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

// CapsContainer.cpp

#include "CapsContainer.h"

//
// The constructor.
//

CapsContainer::CapsContainer(int maxCaps)
{
	maxSize = maxCaps;
	listSize = 0;
	plist = new CARD32[maxSize];
}

//
// The destructor.
//

CapsContainer::~CapsContainer()
{
	delete[] plist;

	// Remove char[] strings allocated by the new[] operator.
	std::map<CARD32,char*>::const_iterator iter;
	for (iter = descMap.begin(); iter != descMap.end(); iter++) {
		delete[] iter->second;
	}
}

//
// Add information about a particular capability into the object. There are
// two functions to perform this task. These functions overwrite capability
// records with the same code.
//

void
CapsContainer::Add(CARD32 code, const char *vendor, const char *name,
				   const char *desc)
{
	// Fill in an rfbCapabilityInfo structure and pass it to the overloaded
	// function.
	rfbCapabilityInfo capinfo;
	capinfo.code = code;
	memcpy(capinfo.vendorSignature, vendor, sz_rfbCapabilityInfoVendor);
	memcpy(capinfo.nameSignature, name, sz_rfbCapabilityInfoName);
	Add(&capinfo, desc);
}

void
CapsContainer::Add(const rfbCapabilityInfo *capinfo, const char *desc)
{
	infoMap[capinfo->code] = *capinfo;
	enableMap[capinfo->code] = false;

	if (IsKnown(capinfo->code)) {
		delete[] descMap[capinfo->code];
	}
	char *desc_copy = NULL;
	if (desc != NULL) {
		desc_copy = new char[strlen(desc) + 1];
		strcpy(desc_copy, desc);
	}
	descMap[capinfo->code] = desc_copy;
}

//
// Check if a capability with the specified code was added earlier.
//

bool
CapsContainer::IsKnown(CARD32 code)
{
	return (descMap.find(code) != descMap.end());
}

//
// Fill in a rfbCapabilityInfo structure with contents corresponding to the
// specified code. Returns true on success, false if the specified code is
// not known.
//

bool
CapsContainer::GetInfo(CARD32 code, rfbCapabilityInfo *capinfo)
{
	if (IsKnown(code)) {
		*capinfo = infoMap[code];
		return true;
	}

	return false;
}

//
// Get a description string for the specified capability code. Returns NULL
// either if the code is not known, or if there is no description for this
// capability.
//

char *
CapsContainer::GetDescription(CARD32 code)
{
	return (IsKnown(code)) ? descMap[code] : NULL;
}

//
// Mark the specified capability as "enabled". This function checks "vendor"
// and "name" signatures in the existing record and in the argument structure
// and enables the capability only if both records are the same.
//

bool
CapsContainer::Enable(const rfbCapabilityInfo *capinfo)
{
	if (!IsKnown(capinfo->code))
		return false;

	const rfbCapabilityInfo *known = &(infoMap[capinfo->code]);
	if ( memcmp(known->vendorSignature, capinfo->vendorSignature,
				sz_rfbCapabilityInfoVendor) != 0 ||
		 memcmp(known->nameSignature, capinfo->nameSignature,
				sz_rfbCapabilityInfoName) != 0 ) {
		enableMap[capinfo->code] = false;
		return false;
	}

	enableMap[capinfo->code] = true;
	if (listSize < maxSize) {
		plist[listSize++] = capinfo->code;
	}
	return true;
}

//
// Check if the specified capability is known and enabled.
//

bool
CapsContainer::IsEnabled(CARD32 code)
{
	return (IsKnown(code)) ? enableMap[code] : false;
}

//
// Return the capability code at the specified index.
// If the index is not valid, return 0.
//

CARD32
CapsContainer::GetByOrder(int idx)
{
	return (idx < listSize) ? plist[idx] : 0;
}

