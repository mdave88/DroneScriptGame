/*
	This software is provided 'as-is', without any express or implied warranty.
	In no event will the author(s) be held liable for any damages arising from
	the use of this software.

	Permission is granted to anyone to use this software for any purpose, including
	commercial applications, and to alter it and redistribute it freely, subject to
	the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.

	Author: Stewart Lynch
	www.puredevsoftware.com
	slynch@puredevsoftware.com

	This code is released to the public domain, as explained at
	http://creativecommons.org/publicdomain/zero/1.0/

	MemProLib is the library that allows the MemPro application to communicate
	with your application.

	===========================================================
                             SETUP
	===========================================================

	* include MemPro.cpp and MemPro.hpp into your project.

	* Link with Dbghelp.lib and Ws2_32.lib - these are needed for the callstack trace and the network connection

	* Connect to your app with the MemPro
*/

// This file contains all cpp and hpp files in the MemProLib library.
// Use this file instead of the library for a quick and convenient way of adding MemPro to your project.

//------------------------------------------------------------------------
// MemPro.hpp
//------------------------------------------------------------------------
/*
	MemPro
	Version:	1.2.5.0
	Date:		14/01/12
*/
//------------------------------------------------------------------------
#ifndef MEMPRO_MEMPRO_H_INCLUDED
#define MEMPRO_MEMPRO_H_INCLUDED

//------------------------------------------------------------------------
#define ENABLE_MEMPRO				// **** enable/disable MemPro here! ****

//------------------------------------------------------------------------
// macros for tracking allocs that define to nothing if disabled
#ifdef ENABLE_MEMPRO
	#ifndef WAIT_FOR_CONNECT
		#define WAIT_FOR_CONNECT false
	#endif
	#define MEMPRO_TRACK_ALLOC(p, size) MemPro::TrackAlloc(p, size, WAIT_FOR_CONNECT)
	#define MEMPRO_TRACK_FREE(p) MemPro::TrackFree(p, WAIT_FOR_CONNECT)
#else
	#define MEMPRO_TRACK_ALLOC(p, size)
	#define MEMPRO_TRACK_FREE(p)
#endif

//------------------------------------------------------------------------
#ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
// Some platforms have problems initialising winsock from global constructors,
// to help get around this problem MemPro waits this amount of time before
// initialising. Allocs and freed that happen during this time are stored in
// a temporary buffer.
#define MEMPRO_INIT_DELAY 100

//------------------------------------------------------------------------
// MemPro waits this long before giving up on a connection after initialisation
#define MEMPRO_CONNECT_TIMEOUT 500

//------------------------------------------------------------------------
#include <stdlib.h>

//------------------------------------------------------------------------
//#define WRITE_DUMP _T("allocs.mempro_dump")

//------------------------------------------------------------------------
namespace MemPro
{
	//------------------------------------------------------------------------
	enum PageState
	{
		Invalid = -1,
		Free,
		Reserved,
		Committed
	};

	//------------------------------------------------------------------------
	enum PageType
	{
		page_Unknown = -1,
		page_Image,
		page_Mapped,
		page_Private
	};

	//------------------------------------------------------------------------

	// You don't need to call this directly, it is automatically called on the first allocation.
	// Only call this function if you want to be able to connect to your app before it has allocated any memory.
	// If wait_for_connect is true this function will block until the external MemPro app has connected,
	// this is useful to make sure that every single allocation is being tracked.
	void Initialise(bool wait_for_connect=false);

	void Disconnect();		// kick all current connections, but can accept more

	void Shutdown();		// free all resources, no more connections allowed

	void TrackAlloc(void* p, size_t size, bool wait_for_connect=false);

	void TrackFree(void* p, bool wait_for_connect=false);

	// this is used for the realtime memory graph.
	void SendPageState(void* p, size_t size, PageState page_state, PageType page_type, unsigned int page_protection, bool send_memory);

	void TakeSnapshot();

	// ignore these, for internal use only
	void IncRef();
	void DecRef();
}

//------------------------------------------------------------------------
#ifndef WRITE_DUMP
namespace
{
	// if we are using sockets we need to flush the sockets on global teardown
	// This class is a trick to attempt to get mempro to shutdown after all other
	// global objects.
	class MemProGLobalScope
	{
	public:
		MemProGLobalScope() { MemPro::IncRef(); }
		~MemProGLobalScope() { MemPro::DecRef(); }
	};
	static MemProGLobalScope g_MemProGLobalScope;
}
#endif

//------------------------------------------------------------------------
#ifdef OVERRIDE_NEW_DELETE

	#if defined(__APPLE__)
		// if you get linker errors about duplicatly defined symbols please add a unexport.txt
		// file to your build settings
		// see here: https://developer.apple.com/library/mac/technotes/tn2185/_index.html
		void* operator new(std::size_t size) throw(std::bad_alloc)
		{
			void* p = malloc(size);
			MEMPRO_TRACK_ALLOC(p, size);
			return p;
		}

		void* operator new(std::size_t size, const std::nothrow_t&) throw()
		{
			void* p = malloc(size);
			MEMPRO_TRACK_ALLOC(p, size);
			return p;
		}

		void  operator delete(void* p) throw()
		{
			MEMPRO_TRACK_FREE(p);
			free(p);
		}

		void  operator delete(void* p, const std::nothrow_t&) throw()
		{
			MEMPRO_TRACK_FREE(p);
			free(p);
		}

		void* operator new[](std::size_t size) throw(std::bad_alloc)
		{
			void* p = malloc(size);
			MEMPRO_TRACK_ALLOC(p, size);
			return p;
		}

		void* operator new[](std::size_t size, const std::nothrow_t&) throw()
		{
			void* p = malloc(size);
			MEMPRO_TRACK_ALLOC(p, size);
			return p;
		}

		void  operator delete[](void* p) throw()
		{
			MEMPRO_TRACK_FREE(p);
			free(p);
		}

		void  operator delete[](void* p, const std::nothrow_t&) throw()
		{
			MEMPRO_TRACK_FREE(p);
			free(p);
		}
	#else
		#include <malloc.h>

		void* operator new(size_t size)
		{
			void* p = malloc(size);
			MEMPRO_TRACK_ALLOC(p, size);
			return p;
		}

		void operator delete(void* p)
		{
			MEMPRO_TRACK_FREE(p);
			free(p);
		}

		void* operator new[](size_t size)
		{
			void* p = malloc(size);
			MEMPRO_TRACK_ALLOC(p, size);
			return p;
		}

		void operator delete[](void* p)
		{
			MEMPRO_TRACK_FREE(p);
			free(p);
		}
	#endif

#endif

//------------------------------------------------------------------------
#ifdef OVERRIDE_MALLOC_FREE
	
	#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
		
		// NOTE: for this to work, you will need to make sure you are linking STATICALLY to the crt. eg: /MTd

		__declspec(restrict) __declspec(noalias) void* malloc(size_t size)
		{
			void* p = HeapAlloc(GetProcessHeap(), 0, size);
			MEMPRO_TRACK_ALLOC(p, size);
			return p;
		}

		__declspec(restrict) __declspec(noalias) void* realloc(void *p, size_t new_size)
		{
			MEMPRO_TRACK_FREE(p);
			void* p_new = HeapReAlloc(GetProcessHeap(), 0, p, new_size);
			MEMPRO_TRACK_ALLOC(p_new, new_size);
			return p_new;
		}

		__declspec(noalias) void free(void *p)
		{
			HeapFree(GetProcessHeap(), 0, p);
			MEMPRO_TRACK_FREE(p);
		}
	#else
		void *malloc(int size)
		{
			void* (*ptr)(int);
			void* handle = (void*)-1;
			ptr = (void*)dlsym(handle, "malloc");
			if(!ptr) abort();
			void *p = (*ptr)(size);
			MEMPRO_TRACK_ALLOC(p, size);
			return p;
		}

		void *realloc(void *p, int size)
		{
			MEMPRO_TRACK_FREE(p);
			void * (*ptr)(void *, int);
			void * handle = (void*) -1;
			ptr = (void*)dlsym(handle, "realloc");
			if (!ptr) abort();
			void* p_new = (*ptr)(p, size);
			MEMPRO_TRACK_ALLOC(p_new, size);
			return p_new;
		}

		void free(void *p)
		{
			MEMPRO_TRACK_FREE(p);
			void* (*ptr)(void*);
			void* handle = (void*)-1;
			ptr = (void*)dlsym(handle, "free");
			if (!ptr == NULL) abort();
			(*ptr)(alloc);
		}
	#endif
#endif

//------------------------------------------------------------------------
#endif		// #ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
#endif		// #ifndef MEMPRO_MEMPRO_H_INCLUDED

