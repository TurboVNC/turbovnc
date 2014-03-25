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

#include "Thread.h"

using namespace vglutil;


void Thread::start(void)
{
	if(!obj) throw (Error("Thread::start()", "Unexpected NULL pointer"));

	#ifdef _WIN32

	DWORD tid;
	if((handle=CreateThread(NULL, 0, threadFunc, obj, 0, &tid))==NULL)
		throw (W32Error("Thread::start()"));

	#else

	int err=0;
	if((err=pthread_create(&handle, NULL, threadFunc, obj))!=0)
		throw (Error("Thread::start()", strerror(err==-1? errno:err)));

	#endif
}


void Thread::stop(void)
{
	#ifdef _WIN32

	if(handle)
	{
		WaitForSingleObject(handle, INFINITE);  CloseHandle(handle);
	}

	#else

	if(handle && !detached) pthread_join(handle, NULL);

	#endif

	handle=0;
}


void Thread::detach(void)
{
	#ifndef _WIN32

	pthread_detach(handle);
	detached=true;

	#endif
}


void Thread::setError(Error &e)
{
	if(obj) obj->lastError=e;
}


void Thread::checkError(void)
{
	if(obj && obj->lastError) throw obj->lastError;
}


#ifdef _WIN32
DWORD WINAPI Thread::threadFunc(void *param)
#else
void *Thread::threadFunc(void *param)
#endif
{
	try
	{
		((Runnable *)param)->threadID=threadID();
		((Runnable *)param)->run();
	}
	catch(Error &e)
	{
		((Runnable *)param)->lastError=e;
	}
	return 0;
}
