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
*/

// This file contains all cpp and hpp files in the MemProLib library.
// Use this file instead of the library for a quick and convenient way of adding MemPro to your project.

#include "GameStdAfx.h"
#include "MemPro.hpp"

//------------------------------------------------------------------------
// CallstackSet.cpp

//------------------------------------------------------------------------
// MemProLib.hpp
#ifndef MEMPRO_MEMPROLIB_H_INCLUDED
#define MEMPRO_MEMPROLIB_H_INCLUDED

//------------------------------------------------------------------------

//------------------------------------------------------------------------
#ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
// **** The Target Platform ****

// define ONE of these

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
	#define MEMPRO_PLATFORM_WIN
#elif defined(__APPLE__)
	#define MEMPRO_PLATFORM_APPLE
#else
	#define MEMPRO_PLATFORM_UNIX
	//#define MEMPRO_PLATFORM_GAMES_CONSOLE_1
	//#define MEMPRO_PLATFORM_GAMES_CONSOLE_2
#endif

//------------------------------------------------------------------------
#if defined(MEMPRO_PLATFORM_WIN) || defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1) || defined(MEMPRO_PLATFORM_GAMES_CONSOLE_2)
	#define MEMPRO_WIN_BASED_PLATFORM
#endif

//------------------------------------------------------------------------
#if defined(MEMPRO_PLATFORM_UNIX) || defined(MEMPRO_PLATFORM_APPLE)
	#define MEMPRO_UNIX_BASED_PLATFORM
#endif

//------------------------------------------------------------------------
#if defined(MEMPRO_PLATFORM_WIN)
	#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0600
	#endif
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#ifndef WRITE_DUMP
		#if defined(UNICODE) && !defined(_UNICODE)
			#error for unicode builds please define both UNICODE and _UNICODE. See the FAQ for more details.
		#endif
		#if defined(AF_IPX) && !defined(_WINSOCK2API_)
			#error winsock already defined. Please include winsock2.h before including windows.h or use WIN32_LEAN_AND_MEAN. See the FAQ for more info.
		#endif
		#include <winsock2.h>
		#include <ws2tcpip.h>
	#endif
#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1)
	#error Please contact slynch@puredevsoftware.com for this platform
#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_2)
	#error Please contact slynch@puredevsoftware.com for this platform
#elif defined(MEMPRO_UNIX_BASED_PLATFORM)
    #include <execinfo.h>
	#include <inttypes.h>
	#include <stdlib.h>
	#include <string.h>
	#include <pthread.h>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>
#else
	#error platform not defined
#endif

//------------------------------------------------------------------------
#if defined(_WIN64) || defined(__LP64__) || defined(__x86_64__) || defined(__ppc64__)
	#define MEMPRO64
#endif

//------------------------------------------------------------------------
#ifdef MEMPRO_PLATFORM_GAMES_CONSOLE_2
	#error Please contact slynch@puredevsoftware.com for this platform
#endif

//------------------------------------------------------------------------
#if defined(MEMPRO_WIN_BASED_PLATFORM)
	#define MEMPRO_ASSERT(b) if(!(b)) DebugBreak()
#elif defined(MEMPRO_UNIX_BASED_PLATFORM)
	#define MEMPRO_ASSERT(b) if(!(b)) __builtin_trap()
#else
	#error		// platform not defined
#endif

//------------------------------------------------------------------------
#define MEMPRO_STATIC_ASSERT(expr) typedef char STATIC_ASSERT_TEST[ (expr) ]

//------------------------------------------------------------------------
namespace MemPro
{
	//------------------------------------------------------------------------
	class Allocator
	{
	public:
#ifdef MEMPRO_WIN_BASED_PLATFORM
		static void* Alloc(int size) { return ::VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE); }
		static void Free(void* p, int size) { VirtualFree(p, size, MEM_RELEASE); }
#else
		static void* Alloc(int size) { return malloc(size); }
		static void Free(void* p, int size) { free(p); }
#endif
	};

	//------------------------------------------------------------------------
	#if defined(MEMPRO_WIN_BASED_PLATFORM)
		typedef __int64 int64;
		typedef unsigned __int64 uint64;
	#elif defined(MEMPRO_UNIX_BASED_PLATFORM)
		typedef int64_t int64;
		typedef uint64_t uint64;
	#else
		#error
	#endif

	//------------------------------------------------------------------------
	// platform specific stuff
	#if defined(MEMPRO_WIN_BASED_PLATFORM)
		#define MEMPRO_FORCEINLINE FORCEINLINE
	#else
		#define MEMPRO_FORCEINLINE inline
		void memcpy_s(void* p_dst, int dst_len, void* p_src, int copy_len) { memcpy(p_dst, p_src, copy_len); }
		void Sleep(int ms) { usleep(1000 * ms); }
		typedef int SOCKET;
		typedef int DWORD;
		enum SocketValues { INVALID_SOCKET = -1 };
		#ifndef UINT_MAX
			enum MaxValues { UINT_MAX = 0xffffffff };
		#endif
		void OutputDebugString(const char*) {}
		#define _T(s) s
		enum SocketErrorCodes { SOCKET_ERROR = -1 };
		typedef sockaddr_in SOCKADDR_IN;
		enum SystemDefines { MAX_PATH = 256 };
	#endif
}

//------------------------------------------------------------------------
#endif		// #ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
#endif		// #ifndef MEMPRO_MEMPROLIB_H_INCLUDED

//------------------------------------------------------------------------
// CallstackSet.hpp
#ifndef MEMPRO_CALLSTACKSET_H_INCLUDED
#define MEMPRO_CALLSTACKSET_H_INCLUDED

//------------------------------------------------------------------------

//------------------------------------------------------------------------
#ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
namespace MemPro
{
	//------------------------------------------------------------------------
	struct Callstack
	{
		uint64* mp_Stack;
		int m_ID;
		int m_Size;
		unsigned int m_Hash;
	};

	//------------------------------------------------------------------------
	// A hash set collection for Callstack structures. Callstacks are added and
	// retreived using the stack address array as the key.
	// This class only allocates memory using virtual alloc/free to avoid going
	// back into the mian allocator.
	class CallstackSet
	{
	public:
		CallstackSet();

		~CallstackSet();

		Callstack* Get(uint64* p_stack, int stack_size, unsigned int hash);

		Callstack* Add(uint64* p_stack, int stack_size, unsigned int hash);

		void Clear();

	private:
		void Grow();

		void Add(Callstack* p_callstack);

		//------------------------------------------------------------------------
		// data
	private:
		Callstack** mp_Data;
		unsigned int m_CapacityMask;
		int m_Count;
		int m_Capacity;
	};
}

//------------------------------------------------------------------------
#endif		// #ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
#endif		// #ifndef MEMPRO_CALLSTACKSET_H_INCLUDED

//------------------------------------------------------------------------
// BlockAllocator.hpp
#ifndef MEMPRO_MEMPRO_SPINLOCK_H_INCLUDED
#define MEMPRO_MEMPRO_SPINLOCK_H_INCLUDED

//------------------------------------------------------------------------

//------------------------------------------------------------------------
// disable some warnings we are not interested in so that we can compile at warning level4
#ifdef MEMPRO_WIN_BASED_PLATFORM
	#pragma warning(disable : 4100)
#endif

//------------------------------------------------------------------------
#ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
namespace MemPro
{
	//------------------------------------------------------------------------
	// a very simple allocator tat allocated blocks of 64k of memory using the
	// templatized allocator.
	template<class TAllocator>
	class BlockAllocator
	{
	public:
		inline BlockAllocator();

		inline void* Alloc(int size);

		inline void Free(void* p);

		//------------------------------------------------------------------------
		// data
	private:
		static const int m_BlockSize = 64*1024;
		void* mp_CurBlock;
		int m_CurBlockUsage;
	};

	//------------------------------------------------------------------------
	template<class TAllocator>
	BlockAllocator<TAllocator>::BlockAllocator()
	:	mp_CurBlock(NULL),
		m_CurBlockUsage(0)
	{
	}

	//------------------------------------------------------------------------
	template<class TAllocator>
	void* BlockAllocator<TAllocator>::Alloc(int size)
	{
		MEMPRO_ASSERT(size < m_BlockSize);

		if(!mp_CurBlock || m_BlockSize - m_CurBlockUsage < size)
		{
			mp_CurBlock = TAllocator::Alloc(m_BlockSize);
			m_CurBlockUsage = 0;
		}

		void* p = (char*)mp_CurBlock + m_CurBlockUsage;
		m_CurBlockUsage += size;

		return p;
	}

	//------------------------------------------------------------------------
	template<class TAllocator>
	void BlockAllocator<TAllocator>::Free(void* p)
	{
		// do nothing
	}
}

//------------------------------------------------------------------------
#endif		// #ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
#endif		// #ifndef MEMPRO_MEMPRO_SPINLOCK_H_INCLUDED

//------------------------------------------------------------------------
#ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
namespace MemPro
{
	//------------------------------------------------------------------------
	const int g_InitialCapacity = 4096;		// must be a power of 2

	MemPro::BlockAllocator<Allocator> g_BlockAllocator;

	//------------------------------------------------------------------------
	inline bool StacksMatch(MemPro::Callstack* p_callstack, uint64* p_stack, int stack_size)
	{
		if(p_callstack->m_Size != stack_size)
			return false;

		for(int i=0; i<stack_size; ++i)
			if(p_callstack->mp_Stack[i] != p_stack[i])
				return false;

		return true;
	}
}

//------------------------------------------------------------------------
MemPro::CallstackSet::CallstackSet()
:	mp_Data((Callstack**)Allocator::Alloc(g_InitialCapacity*sizeof(Callstack*))),
	m_CapacityMask(g_InitialCapacity-1),
	m_Count(0),
	m_Capacity(g_InitialCapacity)
{
	memset(mp_Data, 0, g_InitialCapacity*sizeof(Callstack*));
}

//------------------------------------------------------------------------
MemPro::CallstackSet::~CallstackSet()
{
	Clear();
}

//------------------------------------------------------------------------
void MemPro::CallstackSet::Grow()
{
	int old_capacity = m_Capacity;
	Callstack** p_old_data = mp_Data;

	// allocate a new set
	m_Capacity *= 2;
	m_CapacityMask = m_Capacity - 1;
	int size = m_Capacity * sizeof(Callstack*);
	mp_Data = (Callstack**)Allocator::Alloc(size);
	memset(mp_Data, 0, size);

	// transfer callstacks from old set
	m_Count = 0;
	for(int i=0; i<old_capacity; ++i)
	{
		Callstack* p_callstack = p_old_data[i];
		if(p_callstack)
			Add(p_callstack);
	}

	// release old buffer
	Allocator::Free(p_old_data, old_capacity*sizeof(Callstack*));
}

//------------------------------------------------------------------------
MemPro::Callstack* MemPro::CallstackSet::Get(uint64* p_stack, int stack_size, unsigned int hash)
{
	int index = hash & m_CapacityMask;

	while(mp_Data[index] && !StacksMatch(mp_Data[index], p_stack, stack_size))
		index = (index + 1) & m_CapacityMask;

	return mp_Data[index];
}

//------------------------------------------------------------------------
MemPro::Callstack* MemPro::CallstackSet::Add(uint64* p_stack, int stack_size, unsigned int hash)
{
	// grow the set if necessary
	if(m_Count > (2*m_Capacity)/3)
		Grow();

	// create a new callstack
	Callstack* p_callstack = (Callstack*)g_BlockAllocator.Alloc(sizeof(Callstack));
	p_callstack->m_ID = m_Count;
	p_callstack->m_Size = stack_size;
	p_callstack->mp_Stack = (uint64*)g_BlockAllocator.Alloc(stack_size*sizeof(uint64));
	p_callstack->m_Hash = hash;
	memcpy_s(p_callstack->mp_Stack, stack_size*sizeof(uint64), p_stack, stack_size*sizeof(uint64));

	Add(p_callstack);

	return p_callstack;
}

//------------------------------------------------------------------------
void MemPro::CallstackSet::Add(Callstack* p_callstack)
{
	// find a clear index
	int index = p_callstack->m_Hash & m_CapacityMask;
	while(mp_Data[index])
		index = (index + 1) & m_CapacityMask;

	mp_Data[index] = p_callstack;

	++m_Count;
}

//------------------------------------------------------------------------
void MemPro::CallstackSet::Clear()
{
	for(int i=0; i<m_Capacity; ++i)
	{
		if(mp_Data[i])
			g_BlockAllocator.Free(mp_Data[i]);
	}

	Allocator::Free(mp_Data, m_Capacity*sizeof(Callstack*));

	mp_Data = (Callstack**)Allocator::Alloc(g_InitialCapacity*sizeof(Callstack*));
	m_CapacityMask = g_InitialCapacity-1;
	m_Count = 0;
	m_Capacity = g_InitialCapacity;
}

//------------------------------------------------------------------------
#endif		// #ifdef ENABLE_MEMPRO
//------------------------------------------------------------------------
// MemPro.cpp



//------------------------------------------------------------------------
// RingBuffer.hpp
#ifndef MEMPRO_RINGBUFFER_H_INCLUDED
#define MEMPRO_RINGBUFFER_H_INCLUDED

//------------------------------------------------------------------------

//------------------------------------------------------------------------
// CriticalSection.hpp
#ifndef MEMPRO_CRITICALSECTION_H_INCLUDED
#define MEMPRO_CRITICALSECTION_H_INCLUDED

//------------------------------------------------------------------------


//------------------------------------------------------------------------
#ifdef MEMPRO_UNIX_BASED_PLATFORM
	#include <pthread.h>
#endif

//------------------------------------------------------------------------
#ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
namespace MemPro
{
	//------------------------------------------------------------------------
	class CriticalSection
	{
	public:
		CriticalSection()
		:	m_Count(0)
		{
#ifdef MEMPRO_WIN_BASED_PLATFORM
			InitializeCriticalSection(&cs);
#else
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&cs, &attr);
#endif
		}

		~CriticalSection()
		{
#ifdef MEMPRO_WIN_BASED_PLATFORM
			DeleteCriticalSection(&cs);
#else
			pthread_mutex_destroy(&cs);
#endif
		}

		void Enter()
		{
#ifdef MEMPRO_WIN_BASED_PLATFORM
			EnterCriticalSection(&cs);
#else
			pthread_mutex_lock(&cs);
#endif
			++m_Count;
		}

		void Leave()
		{
#ifdef MEMPRO_WIN_BASED_PLATFORM
			LeaveCriticalSection(&cs);
#else
			pthread_mutex_unlock(&cs);
#endif
			MEMPRO_ASSERT(m_Count > 0);
		}
	private:

		//------------------------------------------------------------------------
		// data
	private:
#ifdef MEMPRO_WIN_BASED_PLATFORM
		CRITICAL_SECTION cs;
#else
		pthread_mutex_t cs;
#endif

		int m_Count;
	};

	//------------------------------------------------------------------------
	class CriticalSectionScope
	{
	public:
		CriticalSectionScope(CriticalSection& in_cs) : cs(in_cs) { cs.Enter(); }
		~CriticalSectionScope() { cs.Leave(); }
	private:
		CriticalSectionScope(const CriticalSectionScope&);
		CriticalSectionScope& operator=(const CriticalSectionScope&);
		CriticalSection& cs;
	};
}

//------------------------------------------------------------------------
#endif		// #ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
#endif		// #ifndef MEMPRO_CRITICALSECTION_H_INCLUDED

//------------------------------------------------------------------------
#ifdef MEMPRO_WIN_BASED_PLATFORM
	// USE_INTRINSIC can also be enabled on 32bit platform, but I left it disabled because it doesn't work on XP
	#ifdef MEMPRO64
		#define USE_INTRINSIC
	#endif
#endif

#ifdef USE_INTRINSIC
#include <intrin.h>
#pragma intrinsic(_InterlockedCompareExchange64)
#pragma intrinsic(_InterlockedExchangeAdd64)
#endif

//------------------------------------------------------------------------
#ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
#if defined(VMEM_PLATFORM_WIN32)
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0501
	#endif						
	#define WINDOWS_LEAN_AND_MEAN
	#include <windows.h>
	#include <intrin.h>
#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1)
	#error Please contact slynch@puredevsoftware.com for this platform
#endif

//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Event.hpp
#ifndef MEMPRO_EVENT_H_INCLUDED
#define MEMPRO_EVENT_H_INCLUDED

//------------------------------------------------------------------------

//------------------------------------------------------------------------
#ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
namespace MemPro
{
	//--------------------------------------------------------------------
	class Event
	{
	public:
		//--------------------------------------------------------------------
		Event(bool initial_state, bool auto_reset)
		{
#ifdef MEMPRO_WIN_BASED_PLATFORM
			m_Handle = CreateEvent(NULL, !auto_reset, initial_state, NULL);
#else
			pthread_cond_init(&m_Cond, NULL);
			pthread_mutex_init(&m_Mutex, NULL);
			m_Signalled = false;
			m_AutoReset = auto_reset;
	
			if(initial_state)
				Set();
#endif
		}

		//--------------------------------------------------------------------
		~Event()
		{
#ifdef MEMPRO_WIN_BASED_PLATFORM
			CloseHandle(m_Handle);
#else
			pthread_mutex_destroy(&m_Mutex);
			pthread_cond_destroy(&m_Cond);
#endif
		}

		//--------------------------------------------------------------------
		void Set() const
		{
#ifdef MEMPRO_WIN_BASED_PLATFORM
			SetEvent(m_Handle);
#else
			pthread_mutex_lock(&m_Mutex);
			m_Signalled = true;
			pthread_mutex_unlock(&m_Mutex);
			pthread_cond_signal(&m_Cond);
#endif
		}

		//--------------------------------------------------------------------
		void Reset()
		{
#ifdef MEMPRO_WIN_BASED_PLATFORM
			ResetEvent(m_Handle);
#else
			pthread_mutex_lock(&m_Mutex);
			m_Signalled = false;
			pthread_mutex_unlock(&m_Mutex);
#endif
		}

		//--------------------------------------------------------------------
		int Wait(int timeout=-1) const
		{
#ifdef MEMPRO_WIN_BASED_PLATFORM
			#ifdef MEMPRO_PLATFORM_GAMES_CONSOLE_2
	#error Please contact slynch@puredevsoftware.com for this platform
			#else
				MEMPRO_STATIC_ASSERT(INFINITE == -1);
				return WaitForSingleObject(m_Handle, timeout) == WAIT_OBJECT_0;
			#endif
#else
			pthread_mutex_lock(&m_Mutex);
	
			if(m_Signalled)
			{
				m_Signalled = false;
				pthread_mutex_unlock(&m_Mutex);
				return true;
			}
	
			if(timeout == -1)
			{
				while(!m_Signalled)
					pthread_cond_wait(&m_Cond, &m_Mutex);
		
				if(!m_AutoReset)
					m_Signalled = false;

				pthread_mutex_unlock(&m_Mutex);
		
				return true;
			}
			else
			{		
				timeval curr;
				gettimeofday(&curr, NULL);
		
				timespec time;
				time.tv_sec  = curr.tv_sec + timeout / 1000;
				time.tv_nsec = (curr.tv_usec * 1000) + ((timeout % 1000) * 1000000);
		
				pthread_cond_timedwait(&m_Cond, &m_Mutex, &time);
		
				if(m_Signalled)
				{
					if(!m_AutoReset)
						m_Signalled = false;

					pthread_mutex_unlock(&m_Mutex);
					return true;
				}
		
				pthread_mutex_unlock(&m_Mutex);
				return false;
			}
#endif
		}

		//------------------------------------------------------------------------
		// data
	private:
#ifdef MEMPRO_WIN_BASED_PLATFORM
		HANDLE m_Handle;
#else
		mutable pthread_cond_t  m_Cond;
		mutable pthread_mutex_t m_Mutex;
		mutable volatile bool m_Signalled;
		bool m_AutoReset;
#endif
	};
}

//------------------------------------------------------------------------
#endif		// #ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
#endif		// #ifndef MEMPRO_EVENT_H_INCLUDED

//------------------------------------------------------------------------
//#define USE_CRITICAL_SECTIONS

//------------------------------------------------------------------------
namespace MemPro
{
#ifdef MEMPRO_WIN_BASED_PLATFORM
	#ifndef USE_INTRINSIC
		//------------------------------------------------------------------------
		MEMPRO_FORCEINLINE int64 ssInterlockedCompareExchange64(int64 volatile *dest, int64 exchange, int64 comperand)
		{
			__asm
			{
				lea esi,comperand;
				lea edi,exchange;
				mov eax,[esi];
				mov edx,4[esi];
				mov ebx,[edi];
				mov ecx,4[edi];
				mov esi,dest;
				lock CMPXCHG8B [esi];
			}
		}

		//------------------------------------------------------------------------
		MEMPRO_FORCEINLINE int64 ssInterlockedExchangeAdd64(__inout int64 volatile *Addend, __in int64 Value)
		{
			int64 Old;
			do
			{
				Old = *Addend;
			} while (ssInterlockedCompareExchange64(Addend, Old + Value, Old) != Old);
			return Old;
		}

		//------------------------------------------------------------------------
		#define _InterlockedCompareExchange64 ssInterlockedCompareExchange64
		#define _InterlockedExchangeAdd64 ssInterlockedExchangeAdd64
	#endif
#else
	// no interlocked functions, so just use a critical section
	CriticalSection g_CASCritSec;
	MEMPRO_FORCEINLINE int64 _InterlockedCompareExchange64(int64 volatile *dest, int64 exchange, int64 comperand)
	{
		g_CASCritSec.Enter();
		int64 old_value = *dest;
		if(*dest == comperand)
			*dest = exchange;
		g_CASCritSec.Leave();
		return old_value;
	}

	MEMPRO_FORCEINLINE int64 _InterlockedExchangeAdd64(int64 volatile *Addend, int64 Value)
	{
		g_CASCritSec.Enter();
		int64 old_value = *Addend;
		*Addend += Value;
		g_CASCritSec.Leave();
		return old_value;
	}
#endif

	//------------------------------------------------------------------------
	// This ring buffer is a lockless buffer, designed to be accessed by no more
	// than two threads, one thread adding to the buffer and one removing. The
	// threads first request data and then add or remove tat data. The threads
	// will sleep if there is no space to add or no data to remove. Once the
	// threads have space on the buffer the data can be added or removed.
	class RingBuffer
	{
	public:
		//------------------------------------------------------------------------
		struct Range
		{
			Range() {}
			Range(void* p, int s) : mp_Buffer(p), m_Size(s) {}

			void* mp_Buffer;
			int m_Size;
		};

		//------------------------------------------------------------------------
		RingBuffer(char* p_buffer, int size)
		:	m_Size(size),
			mp_Buffer(p_buffer),
			m_UsedRange(0),
			m_BytesRemovedEvent(false, true),
			m_BytesAddedEvent(false, true)
		{
			MEMPRO_ASSERT(IsPow2(size));

#ifdef MEMPRO_WIN_BASED_PLATFORM
			MEMPRO_ASSERT((((int64)&m_UsedRange) & 7) == 0);
#endif

#ifdef USE_CRITICAL_SECTIONS
			InitializeCriticalSection(&m_CriticalSection);
#endif
		}

		//------------------------------------------------------------------------
		inline bool IsPow2(int value)
		{
			return (value & (value-1)) == 0;
		}

		//------------------------------------------------------------------------
		int GetSize() const
		{
			return m_Size;
		}

		//------------------------------------------------------------------------
		void Lock() const
		{
#ifdef USE_CRITICAL_SECTIONS
			EnterCriticalSection(&m_CriticalSection);
#endif
		}

		//------------------------------------------------------------------------
		void Release() const
		{
#ifdef USE_CRITICAL_SECTIONS
			LeaveCriticalSection(&m_CriticalSection);
#endif
		}

		//------------------------------------------------------------------------
		int64 GetRangeAtomic() const
		{
#ifdef USE_CRITICAL_SECTIONS
			Lock();
			int64 range = m_UsedRange;
			Release();
#else
			// there must be a better way to atomically read a 64 bit value.
			int64 range = _InterlockedExchangeAdd64(const_cast<int64*>(&m_UsedRange), 0);
#endif
			return range;
		}

		//------------------------------------------------------------------------
		// return the largest free range possible
		Range GetFreeRange(int timeout=-1) const
		{
			int64 range = GetRangeAtomic();
			int size = (int)(range & 0xffffffff);

			// wait until there is some space
			while(size == m_Size)
			{
				if(!m_BytesRemovedEvent.Wait(timeout))
					return Range(NULL, 0);

				range = GetRangeAtomic();
				size = (int)(range & 0xffffffff);
			}

			int start = (int)((range >> 32) & 0xffffffff);

			// calculate the size
			int free_start = (start + size) & (m_Size-1);
			int free_size = free_start < start ? start - free_start : m_Size - free_start;

			return Range(mp_Buffer + free_start, free_size);
		}

		//------------------------------------------------------------------------
		// return the largest used range
		Range GetAllocatedRange(int timeout=-1) const
		{
			int64 range = GetRangeAtomic();
#ifdef _XBOX
			__lwsync();		// ensure that the allocated data has finished writing
#endif
			int size = (int)(range & 0xffffffff);

			// wait until there is some data
			while(!size)
			{
				if(!m_BytesAddedEvent.Wait(timeout))
					return Range(NULL, 0);

				range = GetRangeAtomic();
				size = (int)(range & 0xffffffff);
			}

			int start = (int)((range >> 32) & 0xffffffff);

			// calculate the size
			int max_size = m_Size - start;
			if(size > max_size)
				size = max_size;

			return Range(mp_Buffer + start, size);
		}

		//------------------------------------------------------------------------
		// tells the ring buffer how many bytes have been copied to the allocated range
		void Add(int size)
		{
			Lock();

			MEMPRO_ASSERT(size >= 0);

			volatile int64 old_range;
			int64 new_range;

			do
			{
				old_range = GetRangeAtomic();
				
				int64 used_size = (old_range) & 0xffffffff;
				used_size += size;
				new_range = (old_range & 0xffffffff00000000) | used_size;

			} while(_InterlockedCompareExchange64(&m_UsedRange, new_range, old_range) != old_range);

			m_BytesAddedEvent.Set();

			Release();
		}

		//------------------------------------------------------------------------
		// tells the ring buffer how many bytes have been removed from the allocated range
		void Remove(int size)
		{
			Lock();

			MEMPRO_ASSERT(size >= 0);

			volatile int64 old_range;
			int64 new_range;
			int mask = m_Size - 1;

			do
			{
				old_range = GetRangeAtomic();
				
				int64 used_start = (old_range >> 32) & 0xffffffff;
				int64 used_size = (old_range) & 0xffffffff;
				used_start = (used_start + size) & mask;
				used_size -= size;
				new_range = (used_start << 32) | used_size;

			} while(_InterlockedCompareExchange64(&m_UsedRange, new_range, old_range) != old_range);

			m_BytesRemovedEvent.Set();

			Release();
		}

		//------------------------------------------------------------------------
		int GetUsedBytes() const
		{
			return (int)(m_UsedRange & 0xffffffff);
		}

		//------------------------------------------------------------------------
		void Clear()
		{
			m_UsedRange = 0;
			m_BytesRemovedEvent.Reset();
			m_BytesAddedEvent.Reset();
		}

		//------------------------------------------------------------------------
		// data
	private:
		int m_Size;
		char* mp_Buffer;

#ifdef MEMPRO_WIN_BASED_PLATFORM
		// NOTE: this MUST be 64bit aligned
		__declspec(align(8)) int64 m_UsedRange;		// start index is the high int, size is the low int
#else
		int64 m_UsedRange;
#endif

#ifdef USE_CRITICAL_SECTIONS
		mutable CRITICAL_SECTION m_CriticalSection;
#endif
		Event m_BytesRemovedEvent;
		Event m_BytesAddedEvent;
	};
}

//------------------------------------------------------------------------
#endif		// #ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
#endif		// #ifndef MEMPRO_RINGBUFFER_H_INCLUDED

//------------------------------------------------------------------------
// Packets.hpp
#ifndef MEMPRO_PACKETS_H_INCLUDED
#define MEMPRO_PACKETS_H_INCLUDED

//------------------------------------------------------------------------

//------------------------------------------------------------------------
// MemProMisc.hpp
#ifndef MEMPRO_MEMPROMISC_H_INCLUDED
#define MEMPRO_MEMPROMISC_H_INCLUDED

//------------------------------------------------------------------------

#include <stdlib.h>

//------------------------------------------------------------------------
// disable some warnings we are not interested in so that we can compile at warning level4
#ifdef MEMPRO_WIN_BASED_PLATFORM
	#pragma warning(disable : 4127)
#endif

//------------------------------------------------------------------------
#ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
#define MEMPRO_SPINLOCK_FREE_VAL 0
#define MEMPRO_SPINLOCK_LOCKED_VAL 1
#define MEMPRO_YIELD_SPIN_COUNT 40
#define MEMPRO_SLEEP_SPIN_COUNT 200

//------------------------------------------------------------------------
namespace MemPro
{
	//------------------------------------------------------------------------
	inline int Min(int a, int b) { return a < b ? a : b; }

	//------------------------------------------------------------------------
	inline void SwapEndian(unsigned int& value)
	{
#ifdef MEMPRO_WIN_BASED_PLATFORM
		value = _byteswap_ulong(value);
#else
		value = __builtin_bswap32(value);
#endif
	}

	//------------------------------------------------------------------------
	inline void SwapEndian(uint64& value)
	{
#ifdef MEMPRO_WIN_BASED_PLATFORM
		value = _byteswap_uint64(value);
#else
		value = __builtin_bswap64(value);
#endif
	}

	//------------------------------------------------------------------------
	inline void SwapEndian(int64& value)
	{
		SwapEndian((uint64&)value);
	}

	//------------------------------------------------------------------------
	template<typename T>
	inline void SwapEndian(T& value)
	{
		MEMPRO_ASSERT(sizeof(T) == sizeof(unsigned int));
		SwapEndian((unsigned int&)value);
	}

	//------------------------------------------------------------------------
	inline void SwapEndianUInt64Array(void* p, int size)
	{
		MEMPRO_ASSERT(size % 8 == 0);
		uint64* p_uint64 = (uint64*)p;
		uint64* p_end = p_uint64 + size/8;
		while(p_uint64 != p_end)
			SwapEndian(*p_uint64++);
	}

	//------------------------------------------------------------------------
	// hi-res timer
#if defined(MEMPRO_PLATFORM_WIN)
	inline uint64 GetRDTSC()
	{
#ifdef MEMPRO64
		return __rdtsc();
#else
	   __asm
	   {
			; Flush the pipeline
			XOR eax, eax
			CPUID
			; Get RDTSC counter in edx:eax
			RDTSC
		}
#endif
	}
	#define GET_CLOCK_COUNT(time) time = GetRDTSC();
#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1)
	#error Please contact slynch@puredevsoftware.com for this platform
#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_2)
	#error Please contact slynch@puredevsoftware.com for this platform
#endif

	//------------------------------------------------------------------------
	inline int64 GetTime()
	{
		int64 time;

#ifdef MEMPRO_WIN_BASED_PLATFORM
		GET_CLOCK_COUNT(time);
#else
		timeval curr;
		gettimeofday(&curr, NULL);
		time = ((int64)curr.tv_sec) * 1000000 + curr.tv_usec;
#endif
		return time;
	}

	//------------------------------------------------------------------------
	inline int64 GetTickFrequency()
	{
		Sleep(100);
		int64 start = GetTime();
		Sleep(1000);
		int64 end = GetTime();
		return end - start;
	}

	//------------------------------------------------------------------------
	inline void SetThreadName(unsigned int thread_id, const char* p_name)
	{
#ifdef MEMPRO_WIN_BASED_PLATFORM
		 // see http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
		const unsigned int MS_VC_EXCEPTION=0x406D1388;

		struct THREADNAME_INFO
		{
			unsigned int dwType;		// Must be 0x1000.
			LPCSTR szName;		// Pointer to name (in user addr space).
			unsigned int dwThreadID;	// Thread ID (-1=caller thread).
			unsigned int dwFlags;		// Reserved for future use, must be zero.
		};

		// on the xbox setting thread names messes up the XDK COM API that UnrealConsole uses so check to see if they have been
		// explicitly enabled
		Sleep(10);
		THREADNAME_INFO ThreadNameInfo;
		ThreadNameInfo.dwType		= 0x1000;
		ThreadNameInfo.szName		= p_name;
		ThreadNameInfo.dwThreadID	= thread_id;
		ThreadNameInfo.dwFlags		= 0;

		__try
		{
			RaiseException( MS_VC_EXCEPTION, 0, sizeof(ThreadNameInfo)/sizeof(ULONG_PTR), (ULONG_PTR*)&ThreadNameInfo );
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
#else
		// not supported
#endif
	}

	//------------------------------------------------------------------------
	inline void SmallFastMemCpy(void* p_dst, void* p_src, int size)
	{
		MEMPRO_ASSERT((((size_t)p_dst) & 3) == 0);
		MEMPRO_ASSERT((((size_t)p_src) & 3) == 0);
		MEMPRO_ASSERT((size & 3) == 0);

		unsigned int uint_count = size / sizeof(unsigned int);
		unsigned int* p_uint_dst = (unsigned int*)p_dst;
		unsigned int* p_uint_src = (unsigned int*)p_src;
		for(unsigned int i=0; i<uint_count; ++i)
			*p_uint_dst++ = *p_uint_src++;
	}
}

//------------------------------------------------------------------------
#endif		// #ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
#endif		// #ifndef MEMPRO_MEMPROMISC_H_INCLUDED

//------------------------------------------------------------------------
#ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
namespace MemPro
{
	//------------------------------------------------------------------------
	// This file contains all of te packets that can be sent to the MemPro app.

	//------------------------------------------------------------------------
	enum PacketType
	{
		EInvalid = 0xabcd,
		EAllocPacket,
		EFreePacket,
		ECallstackPacket,
		EPageStatePacket,
		EPageStateStartPacket,	// for backwards compatibility
		EPageStateEndPacket_OLD,
		EVirtualMemStats,
		ETakeSnapshot,
		EVMemStats,
		EPageStateEndPacket,
		EDataStoreEndPacket,
		EPulsePacket,
		ERequestShutdown
	};

	//------------------------------------------------------------------------
	enum MemProVersion
	{
		Version = 10
	};

	//------------------------------------------------------------------------
	enum MemProClientFlags
	{
		SendPageData = 0,
		SendPageDataWithMemory,
		EShutdownComplete
	};

	//------------------------------------------------------------------------
	// value that is sent immediatley after connection to detect big endian
	enum EEndianKey
	{
		EndianKey = 0xabcdef01
	};

	//------------------------------------------------------------------------
	enum Platform
	{
		Platform_Windows,
		Platform_Unix
	};

	//------------------------------------------------------------------------
	struct PacketHeader
	{
		PacketType m_PacketType;
		int64 m_Time;

		void SwapEndian()
		{
			MemPro::SwapEndian(m_PacketType);
			MemPro::SwapEndian(m_Time);
		}
	};

	//------------------------------------------------------------------------
	struct ConnectPacket
	{
		uint64 m_Padding;			// for backwards compatibility

		int64 m_ConnectTime;
		int64 m_TickFrequency;

		int m_Version;
		int m_PtrSize;

		Platform m_Platform;
		int m_Padding2;

		void SwapEndian()
		{
			MemPro::SwapEndian(m_Version);
			MemPro::SwapEndian(m_ConnectTime);
			MemPro::SwapEndian(m_TickFrequency);
			MemPro::SwapEndian(m_PtrSize);
		}
	};

	//------------------------------------------------------------------------
	struct AllocPacket
	{
		uint64 m_Addr;
		uint64 m_Size;
		int m_CallstackID;
		int m_Padding;

		void SwapEndian()
		{
			MemPro::SwapEndian(m_Addr);
			MemPro::SwapEndian(m_Size);
			MemPro::SwapEndian(m_CallstackID);
		}
	};

	//------------------------------------------------------------------------
	struct FreePacket
	{
		uint64 m_Addr;
		int m_CallstackID;
		int m_Padding;

		void SwapEndian()
		{
			MemPro::SwapEndian(m_Addr);
			MemPro::SwapEndian(m_CallstackID);
		}
	};

	//------------------------------------------------------------------------
	struct PageStatePacket
	{
		uint64 m_Addr;
		uint64 m_Size;
		PageState m_State;
		PageType m_Type;
		unsigned int m_Protection;
		int m_SendingMemory;

		void SwapEndian()
		{
			MemPro::SwapEndian(m_Addr);
			MemPro::SwapEndian(m_Size);
			MemPro::SwapEndian(m_State);
			MemPro::SwapEndian(m_Type);
			MemPro::SwapEndian(m_Protection);
			MemPro::SwapEndian(m_SendingMemory);
		}
	};

	//------------------------------------------------------------------------
	struct VirtualMemStatsPacket
	{
		uint64 m_Reserved;
		uint64 m_Committed;

		void SwapEndian()
		{
			MemPro::SwapEndian(m_Reserved);
			MemPro::SwapEndian(m_Committed);
		}
	};

	//------------------------------------------------------------------------
	struct IgnoreMemRangePacket
	{
		uint64 m_Addr;
		uint64 m_Size;

		void SwapEndian()
		{
			MemPro::SwapEndian(m_Addr);
			MemPro::SwapEndian(m_Size);
		}
	};
}

//------------------------------------------------------------------------
#endif		// #ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
#endif		// #ifndef MEMPRO_PACKETS_H_INCLUDED

//------------------------------------------------------------------------
// Socket.hpp
#ifndef MEMPRO_SOCKET_H_INCLUDED
#define MEMPRO_SOCKET_H_INCLUDED

//------------------------------------------------------------------------

//------------------------------------------------------------------------
#if defined(ENABLE_MEMPRO) && !defined(WRITE_DUMP)

//------------------------------------------------------------------------
#ifdef MEMPRO_WIN_BASED_PLATFORM
	#pragma warning(push)
	#pragma warning(disable : 4100)
#endif

//------------------------------------------------------------------------
namespace MemPro
{
	//------------------------------------------------------------------------
	class SocketImp;

	//------------------------------------------------------------------------
	class Socket
	{
	public:
		inline Socket();

		inline ~Socket();

		void Disconnect();

		bool Bind(const char* p_port);

		bool StartListening();

		bool Accept(Socket& client_socket);

		int Receive(void* p_buffer, int size);

		bool Send(void* p_buffer, int size);

		inline bool IsValid() const { return m_Socket != INVALID_SOCKET; }

	private:
		bool InitialiseWSA();

		void CleanupWSA();

		void HandleError();

		//------------------------------------------------------------------------
		// data
		SOCKET m_Socket;
	};

	//------------------------------------------------------------------------
	Socket::Socket()
	:	m_Socket(INVALID_SOCKET)
	{
	}

	//------------------------------------------------------------------------
	Socket::~Socket()
	{
		CleanupWSA();
	}
}

//------------------------------------------------------------------------
#ifdef MEMPRO_WIN_BASED_PLATFORM
	#pragma warning(pop)
#endif

//------------------------------------------------------------------------
#endif		// #if defined(ENABLE_MEMPRO) && !defined(WRITE_DUMP)

//------------------------------------------------------------------------
#endif		// #ifndef MEMPRO_SOCKET_H_INCLUDED



//------------------------------------------------------------------------
// Thread.hpp
#ifndef MEMPRO_THREAD_H_INCLUDED
#define MEMPRO_THREAD_H_INCLUDED

//------------------------------------------------------------------------
#ifdef MEMPRO_WIN_BASED_PLATFORM
	#pragma warning(push)
	#pragma warning(disable : 4100)
#endif

//------------------------------------------------------------------------
#ifndef MEMPRO_WIN_BASED_PLATFORM
	#include <pthread.h>
#endif

//------------------------------------------------------------------------
namespace MemPro
{
	//------------------------------------------------------------------------
	typedef int (*ThreadMain)(void*);

	//------------------------------------------------------------------------
	class Thread
	{
	public:
		Thread();

		void CreateThread(ThreadMain p_thread_main, void* p_param=NULL);

		bool IsAlive() const;

	private:
		#ifdef MEMPRO_WIN_BASED_PLATFORM
			static unsigned long WINAPI PlatformThreadMain(void* p_param);
		#else
			static void* PlatformThreadMain(void* p_param);
		#endif

		//------------------------------------------------------------------------
		// data
	private:
#ifdef MEMPRO_WIN_BASED_PLATFORM
		mutable HANDLE m_Handle;
#else
		mutable pthread_t m_Thread;
		mutable bool m_Alive;
#endif

		mutable ThreadMain mp_ThreadMain;
		mutable void* mp_Param;
	};
}

//------------------------------------------------------------------------
#ifdef MEMPRO_WIN_BASED_PLATFORM
	#pragma warning(pop)
#endif

//------------------------------------------------------------------------
#endif		// #ifndef MEMPRO_THREAD_H_INCLUDED
#include <new>
#include <stdio.h>
#include <time.h>
#include <limits.h>

#ifdef MEMPRO_WIN_BASED_PLATFORM
	#include <tchar.h>
#endif

//------------------------------------------------------------------------
// disable some warnings we are not interested in so that we can compile at warning level4
#ifdef MEMPRO_WIN_BASED_PLATFORM
	#pragma warning(disable : 4127)
	#pragma warning(disable : 4100)
#endif

//------------------------------------------------------------------------
#ifdef ENABLE_MEMPRO

//------------------------------------------------------------------------
#if !defined(WRITE_DUMP) && defined(MEMPRO_WIN_BASED_PLATFORM)
	#pragma comment(lib, "Ws2_32.lib")
#endif

//------------------------------------------------------------------------
// if you are having problems compiling this on your platform undefine ENUMERATE_ALL_MODULES and it send info for just the main module
#ifdef MEMPRO_WIN_BASED_PLATFORM
	#define ENUMERATE_ALL_MODULES
#endif

#ifdef ENUMERATE_ALL_MODULES
	#include <Dbghelp.h>
	#pragma comment(lib, "Dbghelp.lib")
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
namespace VMem { void SendStatsToMemPro(void (*send_fn)(void*, int, void*), void* p_context); }
#endif

//------------------------------------------------------------------------
//#define TEST_ENDIAN

//#define PACKET_START_END_MARKERS

#ifdef TEST_ENDIAN
	#define ENDIAN_TEST(a) a
#else
	#define ENDIAN_TEST(a)
#endif

//------------------------------------------------------------------------
// if both of these options are commented out it will use CaptureStackBackTrace (or backtrace on linux)
//#define USE_STACKWALK64		// much slower but possibly more reliable. USE_STACKWALK64 only implemented for x86 builds.
//#define USE_RTLVIRTUALUNWIND	// reported to be faster than StackWalk64 - only available on x64 builds

//------------------------------------------------------------------------
#ifdef MEMPRO_PLATFORM_GAMES_CONSOLE_2
	#error Please contact slynch@puredevsoftware.com for this platform
#endif

//------------------------------------------------------------------------
#ifdef USE_RTLVIRTUALUNWIND

#ifndef MEMPRO64
	#error USE_RTLVIRTUALUNWIND only available on x64 builds. Please use a different stack walk function.
#endif

namespace MemPro
{
	//------------------------------------------------------------------------
	__declspec(noinline) VOID VirtualUnwindStackWalk(void** stack, int max_stack_size)
	{
		CONTEXT context;
		memset(&context, 0, sizeof(context));
		RtlCaptureContext(&context);

		UNWIND_HISTORY_TABLE unwind_history_table;
		RtlZeroMemory(&unwind_history_table, sizeof(UNWIND_HISTORY_TABLE));

		int frame = 0;
		for (; frame < max_stack_size-1; ++frame)
		{
			stack[frame] = (void*)context.Rip;

			ULONG64 image_base;
			PRUNTIME_FUNCTION runtime_function = RtlLookupFunctionEntry(context.Rip, &image_base, &unwind_history_table);

			if (!runtime_function)
			{
				// If we don't have a RUNTIME_FUNCTION, then we've encountered
				// a leaf function. Adjust the stack approprately.
				context.Rip = (ULONG64)(*(PULONG64)context.Rsp);
				context.Rsp += 8;
			}
			else
			{
				// Otherwise, call upon RtlVirtualUnwind to execute the unwind for us.
				KNONVOLATILE_CONTEXT_POINTERS nv_context;
				RtlZeroMemory(&nv_context, sizeof(KNONVOLATILE_CONTEXT_POINTERS));

				PVOID handler_data;
				ULONG64 establisher_frame;

				RtlVirtualUnwind(
					0/*UNW_FLAG_NHANDLER*/,
					image_base,
					context.Rip,
					runtime_function,
					&context,
					&handler_data,
					&establisher_frame,
					&nv_context);
			}

			// If we reach an RIP of zero, this means that we've walked off the end
			// of the call stack and are done.
			if (!context.Rip)
				break;
		}

		stack[frame] = 0;
	}
}
#endif

//------------------------------------------------------------------------
namespace MemPro
{
	//------------------------------------------------------------------------
	int g_MemProRefs = 0;

	//------------------------------------------------------------------------
	const int PAGE_SIZE = 4096;

	//------------------------------------------------------------------------
	void InitialiseInternal();

	//------------------------------------------------------------------------
	// port number
#if defined(MEMPRO_PLATFORM_WIN)
	const char* g_DefaultPort = "27016";
#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1)
	#error Please contact slynch@puredevsoftware.com for this platform
#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE2)
	#error Please contact slynch@puredevsoftware.com for this platform
#elif defined(MEMPRO_UNIX_BASED_PLATFORM)
	const char* g_DefaultPort = "27016";
#else
	#error platform not defined
#endif

	//------------------------------------------------------------------------
	#if defined(MEMPRO_PLATFORM_WIN)
		#if (NTDDI_VERSION > NTDDI_WINXP)
			#define STACK_TRACE_SIZE 128
		#else
			#define STACK_TRACE_SIZE 62
		#endif
	#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1)
	#error Please contact slynch@puredevsoftware.com for this platform
	#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_2)
	#error Please contact slynch@puredevsoftware.com for this platform
	#elif defined(MEMPRO_UNIX_BASED_PLATFORM)
		#define STACK_TRACE_SIZE 128
	#else
		#error platform not defined
	#endif

	//------------------------------------------------------------------------
	// globals
	const int g_RingBufferSize = 32*1024;

#ifdef MEMPRO64
	uint64 g_MaxAddr = ULLONG_MAX;
#else
	uint64 g_MaxAddr = UINT_MAX;
#endif

	//------------------------------------------------------------------------
#ifdef WRITE_DUMP
	FILE* gp_DumpFile = NULL;
#endif

	//------------------------------------------------------------------------
	struct DataStorePageHeader
	{
		int m_Size;
		DataStorePageHeader* mp_Next;
	};

	//------------------------------------------------------------------------
	void BaseAddressLookupFunction()
	{
	}

	//------------------------------------------------------------------------
	class CMemPro
	{
	public:
		CMemPro();

		bool Initialise();

		void Shutdown();

		void Disconnect(bool listen_for_new_connection);

		void TrackAlloc(void* p, size_t size, bool wait_for_connect);

		void TrackFree(void* p, bool wait_for_connect);

		void SendPageState(void* p, size_t size, PageState page_state, PageType page_type, unsigned int page_protection, bool send_memory);

		void TakeSnapshot();

		int SendThreadMain(void* p_param);

#ifndef WRITE_DUMP
		int ReceiveThreadMain(void* p_param);
#endif

		int WaitForConnectionThreadMain(void* p_param);

		void Lock() { m_CriticalSection.Enter(); }

		void Release() { m_CriticalSection.Leave(); }

		void WaitForConnectionOnInitialise();

	private:
		void GetStackTrace(void* stack[STACK_TRACE_SIZE], int& stack_size, unsigned int& hash);

		void SendModuleInfo();

		void SendExtraModuleInfo(int64 ModuleBase);

		void SendString(const char* p_str);

#ifdef ENUMERATE_ALL_MODULES
		#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
			static BOOL CALLBACK EnumerateLoadedModulesCallback(__in PCSTR ModuleName,__in DWORD64 ModuleBase,__in ULONG ModuleSize,__in_opt PVOID UserContext);
		#else
			static BOOL CALLBACK EnumerateLoadedModulesCallback(__in PCSTR ModuleName,__in ULONG ModuleBase,  __in ULONG ModuleSize,__in_opt PVOID UserContext);
		#endif
#endif
		void StoreData(const void* p_data, int size);

		void BlockUntilSendThreadEmpty();

		void SendStoredData();

		void ClearStoreData();

		inline bool SendThreadStillAlive() const;

		void FlushRingBufferForShutdown();

		void SendData(const void* p_data, int size);

		bool SocketSendData(const void* p_data, int size);

		static void StaticSendVMemStatsData(void* p_data, int size, void* p_context);

		void SendVMemStatsData(void* p_data, int size);

		void SendData(unsigned int value);

		inline void SendPacketHeader(PacketType value);

		void SendStartMarker();

		void SendEndMarker();

		inline void Send(bool value);

		template<typename T> void Send(T& value) { SendData(&value, sizeof(value)); }

		void Send(unsigned int value) { SendData(value); }

		void SendPageState(bool send_memory);

		void SendVMemStats();

		void SendVirtualMemStats();

		int SendCallstack();

		bool WaitForConnection();

		bool WaitForConnectionIfListening();

		static int SendThreadMainStatic(void* p_param);

		static int ReceiveThreadMainStatic(void* p_param);

		static int WaitForConnectionThreadMainStatic(void* p_param);

		static int PulseThreadMainStatic(void* p_param);

		void PulseThreadMain();

		void BlockUntilReadyToSend();

		//------------------------------------------------------------------------
		// data
#ifndef WRITE_DUMP
		Socket m_ListenSocket;
		Socket m_ClientSocket;
#endif

#ifdef MEMPRO_WIN_BASED_PLATFORM
		HANDLE m_Process;
#endif

		// general buffer for used for optaining callstacks
		void* m_Stack[STACK_TRACE_SIZE];
		int m_StackSize;

		CallstackSet m_CallstackSet;

		RingBuffer m_RingBuffer;
		char m_RingBufferMem[g_RingBufferSize];

		volatile bool m_Connected;
		volatile bool m_ReadyToSend;

		volatile bool m_InEvent;

		Event m_StartedListeningEvent;
		Event m_WaitForConnectThreadFinishedEvent;
		Event m_SendThreadFinishedEvent;
		Event m_ReceiveThreadFinishedEvent;
		Event m_MemProReadyToShutdownEvent;
		Event m_PulseThreadFinished;

		volatile bool m_StartedListening;
		bool m_InitialConnectionTimedOut;

		int m_LastPageStateSend;
		int m_PageStateInterval;

		int m_LastVMemStatsSend;
		int m_VMemStatsSendInterval;

		bool m_WaitForConnect;

		static const int m_DataStorePageSize = 4096;
		DataStorePageHeader* mp_DataStoreHead;		// used to store allocs before initialised
		DataStorePageHeader* mp_DataStoreTail;

		Thread m_SendThread;
		Thread m_ReceiveThread;
		Thread m_PulseThread;
		Thread m_WaitForConnectionThread;

		bool m_FlushedRingBufferForShutdown;

		CriticalSection m_CriticalSection;
		CriticalSection m_DisconnectCriticalSection;

		int m_ModulesSent;

		volatile bool m_ShuttingDown;
	};

	//------------------------------------------------------------------------
	char g_MemProMem[sizeof(CMemPro)];
	CMemPro* gp_MemPro = NULL;
	volatile bool g_ShuttingDown = false;

	//------------------------------------------------------------------------
	inline CMemPro* GetMemPro()
	{
		if(!gp_MemPro)
			InitialiseInternal();

		return gp_MemPro;
	}

	//------------------------------------------------------------------------
	CMemPro::CMemPro()
	:
#ifdef MEMPRO_WIN_BASED_PLATFORM
		m_Process(0),
#endif
		m_RingBuffer(m_RingBufferMem, g_RingBufferSize),
		m_Connected(false),
		m_ReadyToSend(false),
		m_InEvent(false),
		m_StartedListeningEvent(false, false),
		m_WaitForConnectThreadFinishedEvent(false, false),
		m_SendThreadFinishedEvent(true, false),
		m_ReceiveThreadFinishedEvent(true, false),
		m_MemProReadyToShutdownEvent(false, false),
		m_PulseThreadFinished(true, false),
		m_StartedListening(false),
		m_InitialConnectionTimedOut(false),
		m_LastPageStateSend(0),
		m_PageStateInterval(1000),
		m_LastVMemStatsSend(0),
		m_VMemStatsSendInterval(5000),
		m_WaitForConnect(false),
		mp_DataStoreHead(NULL),
		mp_DataStoreTail(NULL),
		m_FlushedRingBufferForShutdown(false),
		m_ModulesSent(0),
		m_ShuttingDown(false)
	{
	}

	//------------------------------------------------------------------------
	inline unsigned int GetHash(void** p_stack, int stack_size)
	{
#ifdef MEMPRO64
		const unsigned int prime = 0x01000193;
		unsigned int hash = prime;
		void** p = p_stack;
		for(int i=0; i<stack_size; ++i)
		{
			uint64 key = (uint64)*p++;
			key = (~key) + (key << 18);
			key = key ^ (key >> 31);
			key = key * 21;
			key = key ^ (key >> 11);
			key = key + (key << 6);
			key = key ^ (key >> 22);
			hash = hash ^ (unsigned int)key;
		}

		return hash;
#else
		const unsigned int prime = 0x01000193;
		unsigned int hash = prime;
		for(int i=0; i<stack_size; ++i)
			hash = (hash * prime) ^ (unsigned int)p_stack[i];

		return hash;
#endif
	}

	//------------------------------------------------------------------------
	inline unsigned int GetHashAndStackSize(void** p_stack, int& stack_size)
	{
#ifdef MEMPRO64
		const unsigned int prime = 0x01000193;
		unsigned int hash = prime;
		stack_size = 0;
		void** p = p_stack;
		while(*p)
		{
			uint64 key = (uint64)*p++;
			key = (~key) + (key << 18);
			key = key ^ (key >> 31);
			key = key * 21;
			key = key ^ (key >> 11);
			key = key + (key << 6);
			key = key ^ (key >> 22);
			hash = hash ^ (unsigned int)key;
			++stack_size;
		}

		return hash;
#else
		const unsigned int prime = 0x01000193;
		unsigned int hash = prime;
		stack_size = 0;
		while(p_stack[stack_size])
		{
			hash = (hash * prime) ^ (unsigned int)p_stack[stack_size];
			++stack_size;
		}

		return hash;
#endif
	}

	//------------------------------------------------------------------------
	void CMemPro::GetStackTrace(void* stack[STACK_TRACE_SIZE], int& stack_size, unsigned int& hash)
	{
#if defined(MEMPRO_PLATFORM_WIN)
	#if defined(USE_STACKWALK64)

		#ifdef MEMPRO64
			#error USE_STACKWALK64 only works in x86 builds. Please use a different stack walk funtion.
		#endif

		// get the context
		CONTEXT context;
		memset(&context, 0, sizeof(context));
		RtlCaptureContext(&context);

		// setup the stack frame
		STACKFRAME64 stack_frame;
		memset(&stack_frame, 0, sizeof(stack_frame));
		stack_frame.AddrPC.Mode = AddrModeFlat;
		stack_frame.AddrFrame.Mode = AddrModeFlat;
		stack_frame.AddrStack.Mode = AddrModeFlat;
		#ifdef MEMPRO64
			DWORD machine = IMAGE_FILE_MACHINE_IA64;
			stack_frame.AddrPC.Offset = context.Rip;
			stack_frame.AddrFrame.Offset = context.Rsp;
			stack_frame.AddrStack.Offset = context.Rbp;
		#else
			DWORD machine = IMAGE_FILE_MACHINE_I386;
			stack_frame.AddrPC.Offset = context.Eip;
			stack_frame.AddrFrame.Offset = context.Ebp;
			stack_frame.AddrStack.Offset = context.Esp;
		#endif
		HANDLE thread = GetCurrentThread();

		stack_size = 0;
		while(StackWalk64(
			machine,
			m_Process,
			thread,
			&stack_frame,
			&context,
			NULL,
			SymFunctionTableAccess64,
			SymGetModuleBase64,
			NULL) && stack_size < STACK_TRACE_SIZE)
		{
			void* p = (void*)(stack_frame.AddrPC.Offset);
			stack[stack_size++] = p;
		}
		hash = GetHash(stack, stack_size);
	#elif defined(USE_RTLVIRTUALUNWIND)
		MemPro::VirtualUnwindStackWalk(stack, STACK_TRACE_SIZE);
		hash = GetHashAndStackSize(stack, stack_size);
	#else
		CaptureStackBackTrace(0, STACK_TRACE_SIZE, stack, (PDWORD)&hash);
		for(stack_size = 0; stack_size<STACK_TRACE_SIZE; ++stack_size)
			if(!stack[stack_size])
				break;
	#endif
#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1)
	#error Please contact slynch@puredevsoftware.com for this platform
#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_2)
	#error Please contact slynch@puredevsoftware.com for this platform
#elif defined(MEMPRO_UNIX_BASED_PLATFORM)
		stack_size = backtrace(stack, STACK_TRACE_SIZE);
		hash = GetHashAndStackSize(stack, stack_size);
#else
		#error platform not defined
#endif
	}

	//------------------------------------------------------------------------
	void CMemPro::StaticSendVMemStatsData(void* p_data, int size, void* p_context)
	{
		CMemPro* p_this = (CMemPro*)p_context;
		p_this->SendVMemStatsData(p_data, size);
	}

	//------------------------------------------------------------------------
	void CMemPro::SendVMemStatsData(void* p_data, int size)
	{
		static char buffer[256];
		MEMPRO_ASSERT(size <= (int)sizeof(buffer));
		memcpy_s(buffer, sizeof(buffer), p_data, size);
		ENDIAN_TEST(SwapEndianUInt64Array(buffer, size));
		SendData(buffer, size);
	}

	//------------------------------------------------------------------------
	void CMemPro::StoreData(const void* p_data, int size)
	{
		MEMPRO_ASSERT(size < m_DataStorePageSize - (int)sizeof(DataStorePageHeader));

		if(!mp_DataStoreTail || mp_DataStoreTail->m_Size + size > m_DataStorePageSize)
		{
			DataStorePageHeader* p_new_page = (DataStorePageHeader*)Allocator::Alloc(m_DataStorePageSize);
			p_new_page->m_Size = sizeof(DataStorePageHeader);
			p_new_page->mp_Next = NULL;

			if(mp_DataStoreTail)
				mp_DataStoreTail->mp_Next = p_new_page;
			else
				mp_DataStoreHead = p_new_page;

			mp_DataStoreTail = p_new_page;
		}

		memcpy((char*)mp_DataStoreTail + mp_DataStoreTail->m_Size, p_data, size);
		mp_DataStoreTail->m_Size += size;
	}

	//------------------------------------------------------------------------
	void CMemPro::BlockUntilSendThreadEmpty()
	{
		// wait for the send thread to have sent all of the stored data
		while(m_Connected && m_RingBuffer.GetAllocatedRange(100).m_Size)
			Sleep(100);
	}

	//------------------------------------------------------------------------
	void CMemPro::SendStoredData()
	{
		if(!m_Connected)
			return;

		DataStorePageHeader* p_page = mp_DataStoreHead;

		if(p_page)
		{
			while(p_page)
			{
				DataStorePageHeader* p_next = p_page->mp_Next;

				SendData((char*)p_page + sizeof(DataStorePageHeader), p_page->m_Size - sizeof(DataStorePageHeader));
				Allocator::Free(p_page, m_DataStorePageSize);

				p_page = p_next;
			}

			SendPacketHeader(EDataStoreEndPacket);
			SendEndMarker();
		}

#ifndef WRITE_DUMP
		BlockUntilSendThreadEmpty();
#endif

		mp_DataStoreHead = mp_DataStoreTail = NULL;
	}

	//------------------------------------------------------------------------
	void CMemPro::ClearStoreData()
	{
		DataStorePageHeader* p_page = mp_DataStoreHead;
		while(p_page)
		{
			DataStorePageHeader* p_next = p_page->mp_Next;
			Allocator::Free(p_page, m_DataStorePageSize);
			p_page = p_next;
		}

		mp_DataStoreHead = mp_DataStoreTail = NULL;

		m_CallstackSet.Clear();
	}

	//------------------------------------------------------------------------
	void CMemPro::Send(bool value)
	{
		unsigned int uint_value = value ? 1 : 0;
		Send(uint_value);
	}

	//------------------------------------------------------------------------
	bool CMemPro::SendThreadStillAlive() const
	{
		return m_SendThread.IsAlive();
	}

	//------------------------------------------------------------------------
	void CMemPro::FlushRingBufferForShutdown()
	{
		if(m_FlushedRingBufferForShutdown)
			return;
		m_FlushedRingBufferForShutdown = true;

		RingBuffer::Range range = m_RingBuffer.GetAllocatedRange(100);
		while(range.m_Size)
		{
			SocketSendData(range.mp_Buffer, range.m_Size);
			range = m_RingBuffer.GetAllocatedRange(100);
		}
	}

	//------------------------------------------------------------------------
	void CMemPro::SendData(const void* p_data, int size)
	{
		MEMPRO_ASSERT((size & 3) == 0);

		if(!m_Connected)
		{
			StoreData(p_data, size);
			return;
		}

		if(!SendThreadStillAlive())
		{
			FlushRingBufferForShutdown();
			SocketSendData(p_data, size);
#ifdef WRITE_DUMP
			fflush(gp_DumpFile);
#endif
		}
		else
		{
			int bytes_to_copy = size;
			char* p_src = (char*)p_data;
			while(bytes_to_copy)
			{
				RingBuffer::Range range;
				do {
					range = m_RingBuffer.GetFreeRange(100);
					if(!m_Connected)
						return;
				} while(!range.m_Size);
				if(!m_Connected)
					return;

				int copy_size = Min(range.m_Size, bytes_to_copy);
				SmallFastMemCpy(range.mp_Buffer, p_src, copy_size);
				p_src += copy_size;
				bytes_to_copy -= copy_size;

				m_RingBuffer.Add(copy_size);
			}
		}
	}

	//------------------------------------------------------------------------
	// slightly more optimal version for sending a single uint. Because all ringbuffer
	// operations are 4 byte aligned we can be guaranteed that the uint won't be split
	// between the end and start of the buffer, we will always get it in one piece.
	void CMemPro::SendData(unsigned int value)
	{
		if(!m_Connected)
		{
			StoreData(&value, sizeof(value));
			return;
		}

		if(!SendThreadStillAlive())
		{
			FlushRingBufferForShutdown();
			SocketSendData(&value, sizeof(value));
#ifdef WRITE_DUMP
			fflush(gp_DumpFile);
#endif
		}
		else
		{
			RingBuffer::Range range;
			do {
				range = m_RingBuffer.GetFreeRange(100);
				if(!m_Connected)
					return;
			} while(!range.m_Size);
			if(!m_Connected)
				return;

			MEMPRO_ASSERT(range.m_Size >= (int)sizeof(unsigned int));
			MEMPRO_ASSERT((((size_t)range.mp_Buffer) & 3) == 0);
			*(unsigned int*)range.mp_Buffer = value;

			m_RingBuffer.Add(sizeof(value));
		}
	}

	//------------------------------------------------------------------------
	void CMemPro::SendPacketHeader(PacketType value)
	{
		SendStartMarker();

		PacketHeader header;
		header.m_PacketType = value;
		header.m_Time = GetTime();

		Send(header);
	}

	//------------------------------------------------------------------------
	void CMemPro::SendStartMarker()
	{
#ifdef PACKET_START_END_MARKERS
		unsigned int start_marker = 0xabcdef01;
		ENDIAN_TEST(SwapEndian(start_marker));
		Send(start_marker);
#endif
	}

	//------------------------------------------------------------------------
	void CMemPro::SendEndMarker()
	{
#ifdef PACKET_START_END_MARKERS
		unsigned int end_marker = 0xaabbccdd;
		ENDIAN_TEST(SwapEndian(end_marker));
		Send(end_marker);
#endif
	}

	//------------------------------------------------------------------------
	void CMemPro::SendPageState(bool send_memory)
	{
		CriticalSectionScope lock(m_CriticalSection);

		SendPacketHeader(EPageStateStartPacket);
		SendEndMarker();

#ifdef MEMPRO_WIN_BASED_PLATFORM
		MEMORY_BASIC_INFORMATION info;
		memset(&info, 0, sizeof(info));

		uint64 addr = 0;

		HANDLE process = GetCurrentProcess();

		bool found_page = false;

		while(addr < g_MaxAddr)
		{
			uint64 last_addr = addr;

			if(VirtualQueryEx(process, (void*)addr, &info, sizeof(info)) != 0)
			{
				if(info.State != MEM_FREE)
				{
					PageState page_state;
					switch(info.State)
					{
						case MEM_RESERVE: page_state = MemPro::Reserved; break;
						case MEM_COMMIT: page_state = MemPro::Committed; break;
						default: page_state = MemPro::Committed; MEMPRO_ASSERT(false); break;
					}

					#if defined(MEMPRO_PLATFORM_WIN)
						PageType page_type;
						switch(info.Type)
						{
							case MEM_IMAGE: page_type = page_Image; break;
							case MEM_MAPPED: page_type = page_Mapped; break;
							case MEM_PRIVATE: page_type = page_Private; break;
							default: page_type = page_Unknown; break;
						}
					#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1)
	#error Please contact slynch@puredevsoftware.com for this platform
					#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_2)
	#error Please contact slynch@puredevsoftware.com for this platform
					#else
						#error platform not defined
					#endif
					SendPageState(info.BaseAddress, info.RegionSize, page_state, page_type, info.Protect, send_memory);
				}

				addr += info.RegionSize;
				found_page = true;
			}
			else
			{
				if(!found_page)
					addr += PAGE_SIZE;
				else
					break;		// VirtualQueryEx should only fail when it gets to the end, assuming it has found at least one page
			}

			if(addr < last_addr)		// handle wrap around
				break;
		}
#endif
		SendPacketHeader(EPageStateEndPacket);

		IgnoreMemRangePacket range_packet;
		range_packet.m_Addr = (uint64)m_RingBufferMem;
		range_packet.m_Size = sizeof(m_RingBufferMem);
		Send(range_packet);

		SendEndMarker();
	}

	//------------------------------------------------------------------------
	void CMemPro::SendVMemStats()
	{
#ifdef VMEM_STATS
		Send(EVMemStats);

		int64 time = GetTime();
		Send(time);

		VMem::SendStatsToMemPro(StaticSendVMemStatsData, this);
#endif
	}

	//------------------------------------------------------------------------
	void CMemPro::SendVirtualMemStats()
	{
#ifdef MEMPRO_WIN_BASED_PLATFORM
		MEMORY_BASIC_INFORMATION info;
		memset(&info, 0, sizeof(info));

		const int PAGE_SIZE = 4096;

		uint64 addr = 0;
		size_t reserved = 0;
		size_t committed = 0;

		HANDLE process = GetCurrentProcess();

		bool started = false;

		while(addr < g_MaxAddr)
		{
			uint64 last_addr = addr;

			if(VirtualQueryEx(process, (void*)addr, &info, sizeof(info)) != 0)
			{
				switch(info.State)
				{
					case MEM_RESERVE: reserved += info.RegionSize; break;
					case MEM_COMMIT: committed += info.RegionSize; break;
				}

				addr += info.RegionSize;

				started = true;
			}
			else
			{
				if(started)
					break;

				addr = (addr & (~((size_t)PAGE_SIZE-1))) + PAGE_SIZE;
			}

			if(addr < last_addr)		// handle wrap around
				break;
		}

#ifdef MEMPRO_PLATFORM_GAMES_CONSOLE_1
	#error Please contact slynch@puredevsoftware.com for this platform
#endif
		reserved += committed;

		SendPacketHeader(EVirtualMemStats);

		VirtualMemStatsPacket packet;
		packet.m_Reserved = reserved;
		packet.m_Committed = committed;
		ENDIAN_TEST(packet.SwapEndian());
		Send(packet);
#else
		Send(EVirtualMemStats);

		VirtualMemStatsPacket packet;
		packet.m_Reserved = 0;
		packet.m_Committed = 0;
		packet.m_Time = GetTime();
		ENDIAN_TEST(packet.SwapEndian());
		Send(packet);
#endif
		SendEndMarker();
	}

	//------------------------------------------------------------------------
	int CMemPro::SendCallstack()
	{
		memset(m_Stack, 0, sizeof(m_Stack));
		unsigned int hash = 0;
		GetStackTrace(m_Stack, m_StackSize, hash);

		const int ignore_count = 2;
		int stack_size = m_StackSize - ignore_count;
		if(stack_size <= 0)
		{
			m_Stack[0] = (void*)0xffffffff;
			stack_size = 1;
		}

		static uint64 stack64[STACK_TRACE_SIZE];
		for(int i=0; i<stack_size; ++i)
			stack64[i] = (uint64)m_Stack[i];

		Callstack* p_callstack = m_CallstackSet.Get(stack64, stack_size, hash);
		if(!p_callstack)
		{
			p_callstack = m_CallstackSet.Add(stack64, stack_size, hash);

			SendPacketHeader(ECallstackPacket);

			int send_stack_size = stack_size;
#ifdef TEST_ENDIAN
			for(int i=0; i<stack_size; ++i) SwapEndian(stack64[i]);
			SwapEndian(send_stack_size);
#endif
			Send(send_stack_size);
			SendData(stack64, stack_size*sizeof(uint64));

			SendEndMarker();
		}

		return p_callstack->m_ID;
	}

	//------------------------------------------------------------------------
	void CMemPro::TakeSnapshot()
	{
		CriticalSectionScope lock(m_CriticalSection);
		SendPacketHeader(ETakeSnapshot);
	}

	//------------------------------------------------------------------------
	int CMemPro::SendThreadMainStatic(void* p_param)
	{
		return gp_MemPro->SendThreadMain(p_param);
	}

	//------------------------------------------------------------------------
	bool CMemPro::SocketSendData(const void* p_data, int size)
	{
#ifdef WRITE_DUMP
		MEMPRO_ASSERT(gp_DumpFile);
		size_t result = fwrite(p_data, size, 1, gp_DumpFile);
		MEMPRO_ASSERT(result == 1);
		return true;
#else
		return m_ClientSocket.Send((void*)p_data, size);
#endif
	}

	//------------------------------------------------------------------------
	int CMemPro::SendThreadMain(void* p_param)
	{
		while(m_Connected)
		{
			RingBuffer::Range range;
			do {
				range = m_RingBuffer.GetAllocatedRange(100);	// timeout: check for disconnect every 100 ms
				if(!m_Connected)
				{
					m_SendThreadFinishedEvent.Set();
					return 0;
				}
			} while(!range.m_Size);

			if(!SocketSendData(range.mp_Buffer, range.m_Size))
			{
				m_SendThreadFinishedEvent.Set();
				Disconnect(true);
				return 0;
			}

			m_RingBuffer.Remove(range.m_Size);
		}

		m_SendThreadFinishedEvent.Set();
		return 0;
	}

	//------------------------------------------------------------------------
#ifndef WRITE_DUMP
	int CMemPro::ReceiveThreadMainStatic(void* p_param)
	{
		return gp_MemPro->ReceiveThreadMain(p_param);
	}
#endif

	//------------------------------------------------------------------------
#ifndef WRITE_DUMP
	int CMemPro::ReceiveThreadMain(void* p_param)
	{
		while(m_Connected)
		{
			unsigned int flag = 0;

			if(m_ClientSocket.Receive(&flag, sizeof(flag)) != sizeof(flag))
			{
				m_ReceiveThreadFinishedEvent.Set();
				Disconnect(true);
				return 0;
			}

			switch(flag)
			{
				case SendPageData: SendPageState(false/*send_memory*/); break;
				case SendPageDataWithMemory: SendPageState(true/*send memory*/); break;
				case EShutdownComplete: m_MemProReadyToShutdownEvent.Set(); break;
			}
		}

		m_ReceiveThreadFinishedEvent.Set();
		return 0;
	}
#endif

	//------------------------------------------------------------------------
	// http://www.debuginfo.com/articles/debuginfomatch.html

#ifdef MEMPRO_WIN_BASED_PLATFORM
	struct CV_HEADER
	{
	  DWORD Signature;
	  DWORD Offset;
	};

	struct CV_INFO_PDB20
	{
		CV_HEADER CvHeader;
		DWORD Signature;
		DWORD Age;
		char PdbFileName[MAX_PATH];
	};

	struct CV_INFO_PDB70
	{
		DWORD  CvSignature;
		GUID Signature;
		DWORD Age;
		char PdbFileName[MAX_PATH];
	};
#endif

	void CMemPro::SendExtraModuleInfo(int64 ModuleBase)
	{
#ifdef MEMPRO_WIN_BASED_PLATFORM
		IMAGE_DOS_HEADER* p_dos_header = (IMAGE_DOS_HEADER*)ModuleBase;
		IMAGE_NT_HEADERS* p_nt_header = (IMAGE_NT_HEADERS*)((char*)ModuleBase + p_dos_header->e_lfanew);
		IMAGE_OPTIONAL_HEADER& optional_header = p_nt_header->OptionalHeader;
		IMAGE_DATA_DIRECTORY& image_data_directory = optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
		IMAGE_DEBUG_DIRECTORY* p_debug_info_array = (IMAGE_DEBUG_DIRECTORY*)(ModuleBase + image_data_directory.VirtualAddress);
		int count = image_data_directory.Size / sizeof(IMAGE_DEBUG_DIRECTORY);
		for(int i=0; i<count; ++i)
		{
			if(p_debug_info_array[i].Type == IMAGE_DEBUG_TYPE_CODEVIEW)
			{
				char* p_cv_data = (char*)(ModuleBase + p_debug_info_array[i].AddressOfRawData);
				if(strncmp(p_cv_data, "RSDS", 4) == 0)
				{
					CV_INFO_PDB70* p_cv_info = (CV_INFO_PDB70*)p_cv_data;
					Send(true);				// sending info
					Send(p_cv_info->Age);
					Send(p_cv_info->Signature);
					SendString(p_cv_info->PdbFileName);
					return;									// returning here
				}
				else if(strncmp(p_cv_data, "NB10", 4) == 0)
				{
					Send(true);				// sending info
					CV_INFO_PDB20* p_cv_info = (CV_INFO_PDB20*)p_cv_data;
					Send(p_cv_info->Age);
					Send(p_cv_info->Signature);
					SendString(p_cv_info->PdbFileName);
					return;									// returning here
				}
			}
		}
#endif
		// failed to find info
		Send(false);				// not sending info
	}

	//------------------------------------------------------------------------
	void CMemPro::SendString(const char* p_str)
	{
		const int max_path_len = 1024;
		int len = (int)strlen(p_str) + 1;
		MEMPRO_ASSERT(len <= max_path_len);

		// round up to 4 bytes
		static char temp[max_path_len];
		memset(temp, 0, sizeof(temp));
		memcpy(temp, p_str, len);

		int rounded_len = ((int)len + 3) & ~3;
		Send(rounded_len);

		SendData(temp, rounded_len);
	}

	//------------------------------------------------------------------------
#ifdef ENUMERATE_ALL_MODULES
#if !defined(_IMAGEHLP_SOURCE_) && defined(_IMAGEHLP64)
	// depending on your platform you may need to change PCSTR to PSTR for ModuleName
	BOOL CALLBACK CMemPro::EnumerateLoadedModulesCallback(__in PCSTR ModuleName,__in DWORD64 ModuleBase,__in ULONG ModuleSize,__in_opt PVOID UserContext)
#else
	BOOL CALLBACK CMemPro::EnumerateLoadedModulesCallback(__in PCSTR ModuleName,__in ULONG ModuleBase,__in ULONG ModuleSize,__in_opt PVOID UserContext)
#endif
	{
		CMemPro* p_this = (CMemPro*)UserContext;

		int64 module_base = ModuleBase;
		p_this->Send(module_base);

		p_this->SendString(ModuleName);

		p_this->SendExtraModuleInfo(ModuleBase);

		++p_this->m_ModulesSent;

		return TRUE;
	}
#endif

	//------------------------------------------------------------------------
	void CMemPro::SendModuleInfo()
	{
		Send(true);

		// indicate we are going to be sending module signatures - for backwards compatibility
		uint64 extra_module_info = 0xabcdef;
		Send(extra_module_info);

		m_ModulesSent = 0;

		// if you are having problems compiling this on your platform undefine ENUMERATE_ALL_MODULES and it send info for just the main module
#ifdef ENUMERATE_ALL_MODULES
		EnumerateLoadedModules(GetCurrentProcess(), EnumerateLoadedModulesCallback, this);
#endif
		if(m_ModulesSent == 0)
		{
			int64 module_base = 0;

			char char_filename[MAX_PATH];

			// EnumerateLoadedModules failed so fall back to just getting the exe module handle and name
#ifdef MEMPRO_WIN_BASED_PLATFORM
			// get the module base address
			static int this_module = 0;
			HMODULE module = 0;
			BOOL get_module_result = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)&this_module, &module);
			MEMPRO_ASSERT(get_module_result);
			module_base = (int64)module;

			// get the module name
			TCHAR tchar_filename[MAX_PATH] = { 0 };
			GetModuleFileName(NULL, tchar_filename, MAX_PATH);

			#ifdef UNICODE
				size_t chars_converted = 0;
				wcstombs_s(&chars_converted, char_filename, tchar_filename, MAX_PATH);
				SendString(char_filename);
			#else
				strcpy_s(char_filename, tchar_filename);
			#endif
#else
			// get the module base address
			module_base = (int64)BaseAddressLookupFunction;		// use th address of the BaseAddressLookupFunction function so that we can work it out later

			// get the module name
			char arg1[20];
			sprintf(arg1, "/proc/%d/exe", getpid());
			memset(char_filename, 0, MAX_PATH);
			readlink(arg1, char_filename, MAX_PATH-1);
#endif
			Send(module_base);

			SendString(char_filename);

			SendExtraModuleInfo(module_base);
		}

		uint64 terminator = 0;
		Send(terminator);

		unsigned int magic = 1234;
		Send(magic);
	}

	//------------------------------------------------------------------------
	bool CMemPro::WaitForConnection()
	{
		m_CriticalSection.Enter();

#ifdef WRITE_DUMP
		OutputDebugString(_T("MemPro writing to dump file " WRITE_DUMP _T("\n")));
		_tfopen_s(&gp_DumpFile, WRITE_DUMP, _T("wb"));
		MEMPRO_ASSERT(gp_DumpFile);

		m_Connected = true;

		m_SendThreadFinishedEvent.Reset();

		// start the sending thread
		DWORD thread_id = 0;
		m_SendThread.CreateThread(SendThreadMainStatic, &thread_id);
		SetThreadName(thread_id, "MemPro write thread");
#else
		// start listening for connections
		if(m_ListenSocket.IsValid() && !m_ListenSocket.StartListening())
		{
			m_WaitForConnectThreadFinishedEvent.Set();		// do this before Shutdown
			Shutdown();
			m_CriticalSection.Leave();
			return false;
		}

		m_StartedListening = true;
		m_StartedListeningEvent.Set();

		// Accept a client socket
		bool accepted = false;
		if(m_ListenSocket.IsValid())
		{
			m_CriticalSection.Leave();
			accepted = m_ListenSocket.Accept(m_ClientSocket);

			if(!accepted)
			{
				bool shutting_down = m_ShuttingDown;
				m_WaitForConnectThreadFinishedEvent.Set();		// do this before Shutdown
				if(!shutting_down)			// check shutting down here in case CMemPro has been destructed
				{
					m_CriticalSection.Enter();
					Shutdown();
					m_CriticalSection.Leave();
				}
				return false;
			}
		}

		m_CriticalSection.Enter();

		m_Connected = true;

		m_SendThreadFinishedEvent.Reset();
		m_ReceiveThreadFinishedEvent.Reset();

		// start the sending thread
		DWORD send_thread_id = 0;
		m_SendThread.CreateThread(SendThreadMainStatic, &send_thread_id);
		SetThreadName(send_thread_id, "MemPro send thread");

		// start the receiving thread
		DWORD receive_thread_id = 0;
		m_ReceiveThread.CreateThread(ReceiveThreadMainStatic, &receive_thread_id);
		SetThreadName(receive_thread_id, "MemPro receive thread");
#endif
		// send the connect key
		unsigned int endian_key = (unsigned int)EndianKey;
		ENDIAN_TEST(SwapEndian(endian_key));
		Send(endian_key);

		// send the connect packet
		ConnectPacket connect_packet;
		connect_packet.m_Padding = 0xabcdabcd;
		connect_packet.m_Version = MemPro::Version;
		connect_packet.m_TickFrequency = GetTickFrequency();
		connect_packet.m_ConnectTime = GetTime();

		connect_packet.m_PtrSize = sizeof(void*);

#ifdef MEMPRO_WIN_BASED_PLATFORM
		connect_packet.m_Platform = Platform_Windows;
#else
		connect_packet.m_Platform = Platform_Unix;
#endif

		ENDIAN_TEST(connect_packet.SwapEndian());
		Send(connect_packet);

		SendModuleInfo();

#if defined(MEMPRO_PLATFORM_WIN)
		MemoryBarrier();
#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1)
	#error Please contact slynch@puredevsoftware.com for this platform
#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_2)
	#error Please contact slynch@puredevsoftware.com for this platform
#elif defined(MEMPRO_UNIX_BASED_PLATFORM)
		__sync_synchronize();
#else
		#error platform not defined
#endif

		SendStoredData();

		m_ReadyToSend = true;

		m_WaitForConnectThreadFinishedEvent.Set();
		m_CriticalSection.Leave();

		// start the pulse thread
		DWORD pulse_thread_id = 0;
		m_PulseThreadFinished.Reset();
		m_PulseThread.CreateThread(PulseThreadMainStatic, &pulse_thread_id);
		SetThreadName(pulse_thread_id, "MemPro pulse thread");

		return true;
	}

	//------------------------------------------------------------------------
	int CMemPro::WaitForConnectionThreadMainStatic(void* p_param)
	{
		return gp_MemPro->WaitForConnectionThreadMain(p_param);
	}

	//------------------------------------------------------------------------
	int CMemPro::PulseThreadMainStatic(void* p_param)
	{
		gp_MemPro->PulseThreadMain();
		return 0;
	}

	//------------------------------------------------------------------------
	void CMemPro::PulseThreadMain()
	{
		while(m_Connected)
		{
			{
				CriticalSectionScope lock(m_CriticalSection);
				if(!m_Connected)
					break;

				SendPacketHeader(EPulsePacket);
				SendEndMarker();
			}

			Sleep(1000);
		}

		m_PulseThreadFinished.Set();
	}

	//------------------------------------------------------------------------
	int CMemPro::WaitForConnectionThreadMain(void* p_param)
	{
#ifdef WRITE_DUMP
		Sleep(MEMPRO_INIT_DELAY);
#else
		if(!m_ListenSocket.IsValid())
		{
			Sleep(MEMPRO_INIT_DELAY);
			
			bool bind_result = m_ListenSocket.Bind(g_DefaultPort);
			
			if(!bind_result)
				OutputDebugString(_T("MemPro ERROR: Failed to bind port. This usually means that another process is already running with MemPro enabled.\n"));
			MEMPRO_ASSERT(bind_result);
			if(!bind_result)
				return 0;
		}
#endif
		WaitForConnection();

		return 0;
	}

	//------------------------------------------------------------------------
	bool CMemPro::Initialise()
	{
#ifdef MEMPRO_WIN_BASED_PLATFORM
		m_Process = GetCurrentProcess();
#endif
		m_WaitForConnectionThread.CreateThread(WaitForConnectionThreadMainStatic, NULL);

		return true;
	}

	//------------------------------------------------------------------------
	void CMemPro::Shutdown()
	{
		m_ShuttingDown = true;

		// wait for MemPro to have handled all data
		if(m_WaitForConnectionThread.IsAlive())
		{
			SendPacketHeader(ERequestShutdown);
			SendEndMarker();
			m_MemProReadyToShutdownEvent.Wait(10 * 1000);

			// do this so that we don't start listening after the listen socket has been shutdown and deadlock
			m_CriticalSection.Leave();
			m_StartedListeningEvent.Wait();
			m_CriticalSection.Enter();

			if(m_WaitForConnect)
			{
				BlockUntilReadyToSend();
				BlockUntilSendThreadEmpty();
			}
		}

		Disconnect(false/*listen_for_new_connection*/);

		m_CriticalSection.Leave();
		m_PulseThreadFinished.Wait();
		m_CriticalSection.Enter();

#ifndef WRITE_DUMP
		m_ListenSocket.Disconnect();
	
		if(m_WaitForConnectionThread.IsAlive())
			m_WaitForConnectThreadFinishedEvent.Wait();

		#ifdef MEMPRO_WIN_BASED_PLATFORM
			WSACleanup();
		#endif
#endif
	}

	//------------------------------------------------------------------------
	void CMemPro::Disconnect(bool listen_for_new_connection)
	{
		CriticalSectionScope lock(m_DisconnectCriticalSection);

		if(m_Connected)
		{
			m_ReadyToSend = false;
			m_Connected = false;

			// wait for the send thread to shutdown
			m_SendThreadFinishedEvent.Wait();
			m_SendThreadFinishedEvent.Reset();

#ifdef WRITE_DUMP
			fclose(gp_DumpFile);
			gp_DumpFile = NULL;
#else
			// close the client socket
			m_ClientSocket.Disconnect();

			// wait for the receive thread to shutdown
			m_ReceiveThreadFinishedEvent.Wait();
			m_ReceiveThreadFinishedEvent.Reset();
#endif
			// clear stuff
			m_CallstackSet.Clear();

			m_RingBuffer.Clear();

#ifndef WRITE_DUMP
			if(listen_for_new_connection)
			{
				CriticalSectionScope lock(m_CriticalSection);

				// start listening for another connection
				m_ListenSocket.Disconnect();
				m_StartedListeningEvent.Reset();
				m_StartedListening = false;
				m_InitialConnectionTimedOut = false;
				m_WaitForConnectionThread.CreateThread(WaitForConnectionThreadMainStatic, NULL);
			}
#endif
		}
	}

	//------------------------------------------------------------------------
	void CMemPro::BlockUntilReadyToSend()
	{
#ifndef WRITE_DUMP
		if(m_ListenSocket.IsValid())
		{
			OutputDebugString(_T("Waiting for connection to MemPro...\n"));

			int64 start_time = GetTime();
			while(!m_ReadyToSend && m_ListenSocket.IsValid() &&
				(m_WaitForConnect || ((GetTime() - start_time) / (double)GetTickFrequency()) * 1000 < MEMPRO_CONNECT_TIMEOUT))
			{
				m_CriticalSection.Leave();
				Sleep(100);
				m_CriticalSection.Enter();
			}

			if(m_ReadyToSend)
			{
				OutputDebugString(_T("Connected to MemPro!\n"));
			}
			else
			{
				m_InitialConnectionTimedOut = true;
				ClearStoreData();
				OutputDebugString(_T("Failed to connect to MemPro\n"));
			}
		}
#endif
	}

	//------------------------------------------------------------------------
	// return true to continue processing event (either connected or before started listening)
	bool CMemPro::WaitForConnectionIfListening()
	{
#ifdef WRITE_DUMP
		return true;
#else
		if(!m_ReadyToSend && !m_InitialConnectionTimedOut)
		{
			// store data until we have started listening
			if(!m_StartedListening)
				return true;

			BlockUntilReadyToSend();
		}

		return m_ReadyToSend;
#endif
	}

	//------------------------------------------------------------------------
	void CMemPro::TrackAlloc(void* p, size_t size, bool wait_for_connect)
	{
		CriticalSectionScope lock(m_CriticalSection);

		m_WaitForConnect = wait_for_connect;

		if(!WaitForConnectionIfListening())
			return;

#ifndef WRITE_DUMP
		#ifdef MEMPRO_WIN_BASED_PLATFORM
			if(m_ListenSocket.IsValid())
			{
				int now = GetTickCount();
				if(now - m_LastPageStateSend > m_PageStateInterval)
				{
					SendVirtualMemStats();
					m_LastPageStateSend = now;
				}

				if(now - m_LastVMemStatsSend > m_VMemStatsSendInterval)
				{
					SendVMemStats();
					m_LastVMemStatsSend = now;
				}
			}
		#endif
#endif
		if(m_InEvent)
			return;
		m_InEvent = true;

		int callstack_id = SendCallstack();

		SendPacketHeader(EAllocPacket);

		AllocPacket packet;
		packet.m_Addr = (uint64)p;
		packet.m_Size = size;
		packet.m_CallstackID = callstack_id;
		packet.m_Padding = 0xef12ef12;
		ENDIAN_TEST(packet.SwapEndian());
		Send(packet);

		SendEndMarker();

		m_InEvent = false;
	}

	//------------------------------------------------------------------------
	void CMemPro::TrackFree(void* p, bool wait_for_connect)
	{
		CriticalSectionScope lock(m_CriticalSection);

		m_WaitForConnect = wait_for_connect;

		if(!WaitForConnectionIfListening())
			return;

		if(m_InEvent)
			return;
		m_InEvent = true;

		int callstack_id = SendCallstack();

		SendPacketHeader(EFreePacket);

		FreePacket packet;
		packet.m_Addr = (uint64)p;
		packet.m_CallstackID = callstack_id;
		ENDIAN_TEST(packet.SwapEndian());
		Send(packet);

		SendEndMarker();

		m_InEvent = false;
	}

	//------------------------------------------------------------------------
	void CMemPro::SendPageState(void* p, size_t size, PageState page_state, PageType page_type, unsigned int page_protection, bool send_memory)
	{
#ifdef MEMPRO_WIN_BASED_PLATFORM
		if(!WaitForConnectionIfListening())
			return;

		SendPacketHeader(EPageStatePacket);

		bool send_page_mem = send_memory && page_state == Committed && (page_protection & (PAGE_NOACCESS | PAGE_EXECUTE | PAGE_GUARD)) == 0;

		PageStatePacket packet;
		packet.m_Addr = (uint64)p;
		packet.m_Size = size;
		packet.m_State = page_state;
		packet.m_Type = page_type;
		packet.m_Protection = page_protection;
		packet.m_SendingMemory = send_page_mem;
		ENDIAN_TEST(packet.SwapEndian());
		Send(packet);

		if(send_page_mem)
		{
			MEMPRO_ASSERT(!(size % PAGE_SIZE));
			char* p_page = (char*)p;
			char* p_end_page = p_page + size;
			while(p_page != p_end_page)
			{
				SendData(p_page, PAGE_SIZE);
				p_page += PAGE_SIZE;
			}
		}

		SendEndMarker();
#endif
	}

	//------------------------------------------------------------------------
	void CMemPro::WaitForConnectionOnInitialise()
	{
		m_WaitForConnect = true;

		m_StartedListeningEvent.Wait();

		CriticalSectionScope lock(m_CriticalSection);
		BlockUntilReadyToSend();
	}
}

//------------------------------------------------------------------------
void MemPro::InitialiseInternal()
{
	if(!gp_MemPro && !g_ShuttingDown)
	{
		gp_MemPro = (CMemPro*)g_MemProMem;
		new (gp_MemPro)CMemPro();
		gp_MemPro->Initialise();
	}
}

//------------------------------------------------------------------------
void MemPro::IncRef()
{
	++g_MemProRefs;
}

//------------------------------------------------------------------------
void MemPro::DecRef()
{
	if(--g_MemProRefs == 0)
		Shutdown();
}

//------------------------------------------------------------------------
// called by the APP (not internally)
void MemPro::Initialise(bool wait_for_connect)
{
	InitialiseInternal();

	if(wait_for_connect)
		gp_MemPro->WaitForConnectionOnInitialise();
}

//------------------------------------------------------------------------
void MemPro::Disconnect()
{
	if(gp_MemPro)
	{
		gp_MemPro->Lock();
		gp_MemPro->Disconnect(true);
		gp_MemPro->Release();
	}
}

//------------------------------------------------------------------------
void MemPro::Shutdown()
{
	if(!g_ShuttingDown)
	{
		g_ShuttingDown = true;
		if(gp_MemPro)
		{
			gp_MemPro->Lock();
			gp_MemPro->Shutdown();
			gp_MemPro->Release();
			gp_MemPro->~CMemPro();
			memset(gp_MemPro, 0, sizeof(CMemPro));
			gp_MemPro = NULL;
		}
	}
}

//------------------------------------------------------------------------
void MemPro::TrackAlloc(void* p, size_t size, bool wait_for_connect)
{
	CMemPro* p_mempro = GetMemPro();
	if(p_mempro)
		p_mempro->TrackAlloc(p, size, wait_for_connect);
}

//------------------------------------------------------------------------
void MemPro::TrackFree(void* p, bool wait_for_connect)
{
	CMemPro* p_mempro = GetMemPro();
	if(p_mempro)
		p_mempro->TrackFree(p, wait_for_connect);
}

//------------------------------------------------------------------------
void MemPro::SendPageState(void* p, size_t size, PageState page_state, PageType page_type, unsigned int page_protection, bool send_memory)
{
	CMemPro* p_mempro = GetMemPro();
	if(p_mempro)
		p_mempro->SendPageState(p, size, page_state, page_type, page_protection, send_memory);
}

//------------------------------------------------------------------------
void MemPro::TakeSnapshot()
{
	if(gp_MemPro) gp_MemPro->TakeSnapshot();
}

//------------------------------------------------------------------------
#endif		// #ifdef ENABLE_MEMPRO
//------------------------------------------------------------------------
// Socket.cpp


#include <stdlib.h>
#include <new>

#ifdef MEMPRO_WIN_BASED_PLATFORM
	#include <tchar.h>
#endif

//------------------------------------------------------------------------
#if defined(ENABLE_MEMPRO) && !defined(WRITE_DUMP)

//------------------------------------------------------------------------
namespace MemPro
{
	volatile int g_InitialiseCount = 0;
}

//------------------------------------------------------------------------
bool MemPro::Socket::InitialiseWSA()
{
	if(g_InitialiseCount == 0)
	{
#ifdef MEMPRO_PLATFORM_GAMES_CONSOLE_1
	#error Please contact slynch@puredevsoftware.com for this platform
#endif

#ifdef MEMPRO_WIN_BASED_PLATFORM
		// Initialize Winsock
		WSADATA wsaData;
		if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
		{
			HandleError();
			return false;
		}
#endif
	}

	++g_InitialiseCount;

	return true;
}

//------------------------------------------------------------------------
void MemPro::Socket::CleanupWSA()
{
	--g_InitialiseCount;

	if(g_InitialiseCount == 0)
	{
#ifdef MEMPRO_WIN_BASED_PLATFORM
		if(WSACleanup() == SOCKET_ERROR)
			HandleError();
#endif

#ifdef MEMPRO_PLATFORM_GAMES_CONSOLE_1
	#error Please contact slynch@puredevsoftware.com for this platform
#endif
	}
}

//------------------------------------------------------------------------
void MemPro::Socket::Disconnect()
{
	if(m_Socket != INVALID_SOCKET)
	{
#ifdef MEMPRO_WIN_BASED_PLATFORM
		if(shutdown(m_Socket, SD_BOTH) == SOCKET_ERROR)
			HandleError();
#else
		if(shutdown(m_Socket, SHUT_RDWR) == SOCKET_ERROR)
			HandleError();
#endif

		// loop until the socket is closed to ensure all data is sent
		unsigned int buffer = 0;
		size_t ret = 0;
		do { ret = recv(m_Socket, (char*)&buffer, sizeof(buffer), 0); } while(ret != 0 && ret != (size_t)SOCKET_ERROR);

#ifdef MEMPRO_WIN_BASED_PLATFORM
	    if(closesocket(m_Socket) == SOCKET_ERROR)
			HandleError();
#else
		close(m_Socket);
#endif
		m_Socket = INVALID_SOCKET;
	}
}

//------------------------------------------------------------------------
bool MemPro::Socket::StartListening()
{
	MEMPRO_ASSERT(m_Socket != INVALID_SOCKET);

	if (listen(m_Socket, SOMAXCONN) == SOCKET_ERROR)
	{
		HandleError();
		return false;
	}
	return true;
}

//------------------------------------------------------------------------
bool MemPro::Socket::Bind(const char* p_port)
{
	MEMPRO_ASSERT(m_Socket == INVALID_SOCKET);

	if(!InitialiseWSA())
		return false;

#ifdef MEMPRO_PLATFORM_WIN
	// setup the addrinfo struct
	addrinfo info;
	ZeroMemory(&info, sizeof(info));
	info.ai_family = AF_INET;
	info.ai_socktype = SOCK_STREAM;
	info.ai_protocol = IPPROTO_TCP;
	info.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	addrinfo* p_result_info;
	HRESULT result = getaddrinfo(NULL, p_port, &info, &p_result_info);
	if (result != 0)
	{
		HandleError();
		return false;
	}

	m_Socket = socket(
		p_result_info->ai_family,
		p_result_info->ai_socktype, 
		p_result_info->ai_protocol);
#else
	m_Socket = socket(
		AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);
#endif

	if (m_Socket == INVALID_SOCKET)
	{
#ifdef MEMPRO_PLATFORM_WIN
		freeaddrinfo(p_result_info);
#endif
		HandleError();
		return false;
	}

	// Setup the TCP listening socket
#ifdef MEMPRO_PLATFORM_WIN
	result = ::bind(m_Socket, p_result_info->ai_addr, (int)p_result_info->ai_addrlen);
	freeaddrinfo(p_result_info);
#else
	// Bind to INADDR_ANY
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	int iport = atoi(p_port);
	sa.sin_port = htons(iport);
	int result = ::bind(m_Socket, (const sockaddr*)(&sa), sizeof(SOCKADDR_IN));
#endif

	if (result == SOCKET_ERROR)
	{
		HandleError();
		Disconnect();
		return false;
	}

	return true;
}

//------------------------------------------------------------------------
bool MemPro::Socket::Accept(Socket& client_socket)
{
	MEMPRO_ASSERT(client_socket.m_Socket == INVALID_SOCKET);
	client_socket.m_Socket = accept(m_Socket, NULL, NULL);
	return client_socket.m_Socket != INVALID_SOCKET;
}

//------------------------------------------------------------------------
bool MemPro::Socket::Send(void* p_buffer, int size)
{
	int bytes_to_send = size;
	while(bytes_to_send != 0)
	{
		int bytes_sent = (int)send(m_Socket, (char*)p_buffer, bytes_to_send, 0);
		if(bytes_sent == SOCKET_ERROR)
		{
			HandleError();
			Disconnect();
			return false;
		}
		p_buffer = (char*)p_buffer + bytes_sent;
		bytes_to_send -= bytes_sent;
	}

	return true;
}

//------------------------------------------------------------------------
int MemPro::Socket::Receive(void* p_buffer, int size)
{
	int bytes_received = 0;
	while(size)
	{
		bytes_received += (int)recv(m_Socket, (char*)p_buffer, size, 0);
		if(bytes_received == 0)
		{
			Disconnect();
			return bytes_received;
		}
		else if(bytes_received == SOCKET_ERROR)
		{
			HandleError();
			Disconnect();
			return bytes_received;
		}

		size -= bytes_received;
		p_buffer = (char*)p_buffer + size;
	}

	return bytes_received;
}

//------------------------------------------------------------------------
void MemPro::Socket::HandleError()
{
#ifdef MEMPRO_PLATFORM_WIN
	if(WSAGetLastError() == WSAEADDRINUSE)
	{
		OutputDebugString(_T("MemPro: Network connection conflict. Please make sure that other MemPro enabled applications are shut down, or change the port in the the MemPro lib and MemPro settings.\n"));
		return;
	}

	TCHAR* p_buffer = NULL;
	va_list args;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		0,
		(TCHAR*)&p_buffer,
		4 * 1024,
		&args);

	OutputDebugString(p_buffer);

	LocalFree(p_buffer);
#endif
}

//------------------------------------------------------------------------
#endif		// #if defined(ENABLE_MEMPRO) && !defined(WRITE_DUMP)
//------------------------------------------------------------------------
// Thread.cpp



#ifdef MEMPRO_UNIX_BASED_PLATFORM
	#include <pthread.h>
#endif

//------------------------------------------------------------------------
MemPro::Thread::Thread()
#ifdef MEMPRO_WIN_BASED_PLATFORM
	:	m_Handle(0)
#else
	:	m_Alive(false)
#endif
{
}

//------------------------------------------------------------------------
void MemPro::Thread::CreateThread(ThreadMain p_thread_main, void* p_param)
{
	mp_ThreadMain = p_thread_main;
	mp_Param = p_param;

#ifdef MEMPRO_WIN_BASED_PLATFORM
	m_Handle = ::CreateThread(NULL, 0, PlatformThreadMain, this, 0, NULL);
#else
	pthread_create(&m_Thread, NULL, PlatformThreadMain, this);
#endif
}

//------------------------------------------------------------------------
bool MemPro::Thread::IsAlive() const
{
#ifdef MEMPRO_WIN_BASED_PLATFORM
	DWORD result = WaitForSingleObject(m_Handle, 0);
	return result != WAIT_OBJECT_0;
#else
	return m_Alive;
#endif
}

//------------------------------------------------------------------------
#ifdef MEMPRO_WIN_BASED_PLATFORM
unsigned long WINAPI MemPro::Thread::PlatformThreadMain(void* p_param)
{
	Thread* p_thread = (Thread*)p_param;
	return (unsigned long)p_thread->mp_ThreadMain(p_thread->mp_Param);
}
#else
void* MemPro::Thread::PlatformThreadMain(void* p_param)
{
	Thread* p_thread = (Thread*)p_param;
	p_thread->m_Alive = true;
	p_thread->mp_ThreadMain(p_thread->mp_Param);
	p_thread->m_Alive = false;
	return NULL;
}
#endif
