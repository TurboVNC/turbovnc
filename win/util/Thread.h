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

#ifndef __THREAD_H__
#define __THREAD_H__

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include "Error.h"


namespace vglutil
{
	// These classes implement Java-like threads in C++

	class Runnable
	{
		public:

			Runnable(void) {}
			virtual ~Runnable(void) {}

		protected:

			virtual void run()=0;
			unsigned long threadID;
			Error lastError;
			friend class Thread;
	};


	class Thread
	{
		public:

			Thread(Runnable *obj_) : obj(obj_), handle(0), detached(false) {}
			void start(void);
			void stop(void);
			// This allows a Unix thread to kill itself.  It has no effect on Windows.
			void detach(void);
			void setError(Error &e);
			void checkError(void);

			static unsigned long threadID(void)
			{
				#ifdef _WIN32
				return GetCurrentThreadId();
				#else
				return (unsigned long)pthread_self();
				#endif
			}

		protected:

			Runnable *obj;
			#ifdef _WIN32
			static DWORD WINAPI threadFunc(void *param);
			#else
			static void *threadFunc(void *param);
			#endif

			#ifdef _WIN32
			HANDLE handle;
			#else
			pthread_t handle;
			#endif
			bool detached;
	};
}

#endif // __THREAD_H__
