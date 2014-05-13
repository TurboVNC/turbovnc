/* Copyright (C)2004 Landmark Graphics Corporation
 * Copyright (C)2005 Sun Microsystems, Inc.
 * Copyright (C)2014 D. R. Commander
 *
 * This library is free software and may be redistributed and/or modified under
 * the terms of the wxWindows Library License, Version 3.1 or (at your option)
 * any later version.  The full license is in the LICENSE.txt file included
 * with this distribution.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * wxWindows Library License for more details.
 */

#ifndef __ERROR_H__
#define __ERROR_H__

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <new>


namespace vglutil
{
	class Error
	{
		public:

			Error(const char *method, char *message)
			{
				init(method, message, -1);
			}

			Error(const char *method, const char *message)
			{
				init(method, (char *)message, -1);
			}

			Error(const char *method, char *message, int line)
			{
				init(method, message, line);
			}

			Error(const char *method, const char *message, int line)
			{
				init(method, (char *)message, line);
			}

			void init(const char *method_, char *message_, int line)
			{
				message[0]=0;
				if(line>=1) sprintf(message, "%d: ", line);
				if(!method_) method_="(Unknown error location)";
				method=method_;
				if(message_)
					strncpy(&message[strlen(message)], message_, MLEN-strlen(message));
			}

			Error(void) : method(NULL) { message[0]=0; }

			operator bool() { return (method!=NULL && message[0]!=0); }

			const char *getMethod(void) { return method; }
			char *getMessage(void) { return message; }

		protected:

			static const int MLEN=256;
			const char *method;  char message[MLEN+1];
	};
}


#if defined(sgi) || defined(sun)
#define __FUNCTION__ __FILE__
#endif
#define _throw(m) throw(vglutil::Error(__FUNCTION__, m, __LINE__))
#define _errifnot(f) { if(!(f)) _throw("Unexpected NULL condition"); }
#define _newcheck(f)  \
	try {  \
		if(!(f)) _throw("Memory allocation error");  \
	} catch(std::bad_alloc& e) { _throw(e.what()); }


#ifdef _WIN32

namespace vglutil
{
	class W32Error : public Error
	{
		public:

			W32Error(const char *method) : Error(method, (char *)NULL)
			{
				if(!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, MLEN, NULL))
					strncpy(message, "Error in FormatMessage()", MLEN);
			}

			W32Error(const char *method, int line) :
				Error(method, (char *)NULL, line)
			{
				if(!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &message[strlen(message)],
					MLEN-(DWORD)strlen(message), NULL))
					strncpy(message, "Error in FormatMessage()", MLEN);
			}
	};
}

#define _throww32() throw(vglutil::W32Error(__FUNCTION__, __LINE__))
#define _w32(f) { if(!(f)) _throww32(); }

#endif // _WIN32


namespace vglutil
{
	class UnixError : public Error
	{
		public:

			UnixError(const char *method) : Error(method, strerror(errno)) {}
			UnixError(const char *method, int line) :
				Error(method, strerror(errno), line) {}
	};
}

#define _throwunix() throw(vglutil::UnixError(__FUNCTION__, __LINE__))
#define _unix(f) { if((f)==-1) _throwunix(); }


#define _fbx(f) {  \
	if((f)==-1)  \
		throw(vglutil::Error("FBX", fbx_geterrmsg(), fbx_geterrline()));  \
}
#define _fbxv(f) {  \
	if((f)==-1)  \
		throw(vglutil::Error("FBXV", fbxv_geterrmsg(), fbxv_geterrline()));  \
}
#define _tj(f) {  \
	if((f)==-1)  \
		throw(vglutil::Error(__FUNCTION__, tjGetErrorStr(), __LINE__));  \
}

#endif // __ERROR_H__
