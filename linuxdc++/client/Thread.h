/* 
 * Copyright (C) 2001-2004 Jacek Sieka, j_s at telia com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#if !defined(AFX_THREAD_H__3006956B_7C69_4DAD_9596_A49E1BD007D5__INCLUDED_)
#define AFX_THREAD_H__3006956B_7C69_4DAD_9596_A49E1BD007D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32
#include <pthread.h>
#include <sched.h>
#include <sys/resource.h>
#endif

#ifdef HAVE_ASM_ATOMIC_H
#include <asm/atomic.h>
#endif

#include "Exception.h"
STANDARD_EXCEPTION(ThreadException);

class Thread  
{
public:
#ifdef _WIN32
	enum Priority {
		IDLE = THREAD_PRIORITY_IDLE,
		LOW = THREAD_PRIORITY_LOWEST,
		NORMAL = THREAD_PRIORITY_NORMAL,
		HIGH = THREAD_PRIORITY_HIGHEST
	};

	Thread() throw() : threadHandle(NULL), threadId(0){ };
	virtual ~Thread() { 
		if(threadHandle)
			CloseHandle(threadHandle);
	};
	
	void start() throw(ThreadException);
	void join() throw(ThreadException) {
		if(threadHandle == NULL) {
			return;
		}

		WaitForSingleObject(threadHandle, INFINITE);
		CloseHandle(threadHandle);
		threadHandle = NULL;
	}

	void setThreadPriority(Priority p) { ::SetThreadPriority(threadHandle, p); };
	
	static void sleep(u_int32_t millis) { ::Sleep(millis); };
	static void yield() { ::Sleep(0); };
	static long safeInc(long* v) { return InterlockedIncrement(v); };
	static long safeDec(long* v) { return InterlockedDecrement(v); };
	static long safeExchange(long* target, long value) { return InterlockedExchange(target, value); };

#else

	enum Priority {
		IDLE = 1,
		LOW = 1,
		NORMAL = 0,
		HIGH = -1
	};
	Thread() throw() : threadHandle(0) { };
	virtual ~Thread() { 
		if (threadHandle) {
			pthread_detach(threadHandle);
		}
	};
	void start() throw(ThreadException);
	void join() throw() { 
		if (threadHandle) {
			pthread_join(threadHandle, 0);
			threadHandle = 0;
		}
	};

	void setThreadPriority(Priority p) { setpriority(PRIO_PROCESS, 0, p); };
	static void sleep(u_int32_t millis) { ::usleep(millis*1000); };
	static void yield() { ::sched_yield(); };
	static long safeInc(long* v) { 
#ifdef HAVE_ASM_ATOMIC_H
		atomic_t t = ATOMIC_INIT(*v);
		atomic_inc(&t);
		return (*v=t.counter);
#else
#warning FIXME
		(*v)++;
		return *v;
#endif
	};
	static long safeDec(long* v) { 
#ifdef HAVE_ASM_ATOMIC_H
		atomic_t t = ATOMIC_INIT(*v);
		atomic_dec(&t);
		return (*v=t.counter);
#else
#warning FIXME
		(*v)--;
		return *v;
#endif
	};
#endif

protected:
	virtual int run() = 0;
	
private:

#ifdef _WIN32
	HANDLE threadHandle;
	DWORD threadId;
	static DWORD WINAPI starter(void* p) {
		Thread* t = (Thread*)p;
		t->run();
		return 0;
	}
#else
	pthread_t threadHandle;
	static void* starter(void* p) {
		Thread* t = (Thread*)p;
		t->run();
		t->threadHandle = 0;
		return NULL;
	}
#endif
};

#endif // !defined(AFX_THREAD_H__3006956B_7C69_4DAD_9596_A49E1BD007D5__INCLUDED_)

/**
 * @file
 * $Id: Thread.h,v 1.3 2004/11/14 16:35:23 paskharen Exp $
 */

