/*
	Copyright 2010 - 2013 Stewart Lynch

	This file is part of VMem.

	VMem is dual licensed. For use in open source software VMem can
	be used under the GNU license. For use in commercial applications a
	license can be purchased from Stewart Lynch.

    If used under the GNU license VMem is free software: you can redistribute
	it and/or modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of the License,
	or (at your option) any later version.

    VMem is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with VMem. If not, see <http://www.gnu.org/licenses/>.

	If VMem is used under the commercial license please see the license
	file distributed with this software, or see http://www.puredevsoftware.com/VMme_EULA.htm
*/

// This file contains all cpp and hpp files in the VMem library.
// Use this file instead of the library for a quick and convenient way of adding VMem to your project.

//------------------------------------------------------------------------
// VMemAlloc.hpp
//------------------------------------------------------------------------
/*
	Version:	2.2
	Date:		20/04/14
*/

//------------------------------------------------------------------------
#ifndef VMEM_VMEMALLOC_H_INCLUDED
#define VMEM_VMEMALLOC_H_INCLUDED

//------------------------------------------------------------------------

//------------------------------------------------------------------------
// VMemCore.hpp

//------------------------------------------------------------------------
// VMemDefs.hpp
//
// *** If you are having memory corruption problems enable this define! ***
//
// Debug level 3 is designed to be used when tracking down problems,
// such as memory corruptions. It includes periodic integrity checking, memory
// guards, guard pages, guard allocs and trail guards. Full checking does
// have a small memory overhead and can degrade performance. If performance
// is degraded too much you can define the only the checks that you want,
// see the DEBUG DEFINES section.

//#define VMEM_DEBUG_LEVEL 3							// enable full debug here

//------------------------------------------------------------------------
//
// Debug Levels
//
//		0	- All debug code defined out. Use for production biulds.
//		1	- Minimal debugging, minimal slowdown. Use for internal test builds.
//		2	- Standard debug. All but the really slow debug features enabled.
//		3	- Full debugging - very slow. Only enable to help track down bugs
//
#ifndef VMEM_DEBUG_LEVEL
	#if defined(_DEBUG) || defined(DEBUG)
		#define VMEM_DEBUG_LEVEL 2
	#else
		#define VMEM_DEBUG_LEVEL 0
	#endif
#endif

//------------------------------------------------------------------------
// Define the platform here.
//
// possible platforms are:
//
//		VMEM_PLATFORM_WIN
//		MEMPRO_PLATFORM_GAMES_CONSOLE_1
//		MEMPRO_PLATFORM_GAMES_CONSOLE_2
//		VMEM_PLATFORM_UNIX
//
#if defined(WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
	#ifdef _XBOX
		#define MEMPRO_PLATFORM_GAMES_CONSOLE_1
		//#define MEMPRO_PLATFORM_GAMES_CONSOLE_2
	#else
		#define VMEM_PLATFORM_WIN
	#endif
#elif defined(__APPLE__)
	#define VMEM_PLATFORM_APPLE
#elif defined(PLATFORM_ANDROID)
	#define VMEM_PLATFORM_ANDROID
#else
	#define VMEM_PLATFORM_UNIX
#endif

//------------------------------------------------------------------------
// define the OS
#if defined(VMEM_PLATFORM_WIN) || defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1) || defined(MEMPRO_PLATFORM_GAMES_CONSOLE_2)
	#define VMEM_OS_WIN
#elif defined(VMEM_PLATFORM_UNIX) || defined(VMEM_PLATFORM_APPLE) || defined(VMEM_PLATFORM_ANDROID)
	#define VMEM_OS_UNIX
#else
	#error platform not defined
#endif

//------------------------------------------------------------------------
#if (defined(VMEM_OS_WIN) && defined(_WIN64)) || (defined(VMEM_PLATFORM_APPLE) && defined(__LP64__))
	#define VMEM64
#endif

//------------------------------------------------------------------------
// DEBUG DEFINES - these are enabled based on the debug level, but you can
// enable them individually here.

//#define VMEM_ASSERTS					// small overhead
//#define VMEM_MEMSET					// memset all memory when it is committed, allocated and freed - small overhead
//#define VMEM_MEMSET_ONLY_SMALL 256	// memset only the first n bytes of every allocation - minimal overhead
//#define VMEM_STATS					// record memory usage in all heaps - minimal overhead
//#define VMEM_COALESCE_GUARDS			// guard blocks at the end of each alloc - small overhead
#define VMEM_FSA_GUARDS				// guard blocks at the end of each alloc - medium overhead
//#define VMEM_INC_INTEG_CHECK 1000		// the frequency to do an incremental integrity check
//#define VMEM_TRAIL_GUARDS				// don't re-allocate the last n allocs to check for overwrite corruptions
//#define VMEM_ALTERNATE_PAGES			// only commit alternate pages in the page heaps
//#define VMEM_COALESCE_GUARD_PAGES 4	// always keep a page decommitted every n times the max alloc size. Can trap memory corruptions.
//#define COALESCE_HEAP_MARKER			// put a marker word at the start of each allocation
//#define VMEM_MEMORY_CORRUPTION_FINDER	// don't re-use freed pages, only decommit them. This can help catch memory corruptions.
//#define VMEM_PROTECTED_HEAP			// add specific allocations to the protected debug heap

//------------------------------------------------------------------------
#if VMEM_DEBUG_LEVEL != 0
	#define VMEM_ASSERTS
	#define VMEM_STATS
	#define VMEM_COALESCE_GUARDS
	#define COALESCE_HEAP_MARKER
#endif

//------------------------------------------------------------------------
#if VMEM_DEBUG_LEVEL >= 2
	#define VMEM_MEMSET
#elif VMEM_DEBUG_LEVEL == 1
	#define VMEM_MEMSET_ONLY_SMALL 256
#endif

//------------------------------------------------------------------------
#if VMEM_DEBUG_LEVEL >= 3
	#define VMEM_FSA_GUARDS
	#define VMEM_INC_INTEG_CHECK 1000
	#define VMEM_TRAIL_GUARDS
	#define VMEM_ALTERNATE_PAGES
	#define VMEM_COALESCE_GUARD_PAGES 4
#endif

//------------------------------------------------------------------------
#if VMEM_DEBUG_LEVEL >= 4
	#define VMEM_MEMORY_CORRUPTION_FINDER
	#define VMEM_PROTECTED_HEAP
#endif

//------------------------------------------------------------------------
// Enable this to delay the decommitting and releasing of system pages back to the
// system. In some situations when there are lots of large allocations of similar sizes,
// or a lot of memory churn, this is a significant performance optimisation.
// It can lead to temporary increases in memory usage, however, if VMem can't commit
// memory it will flush these delayed releases, so it will never cause an out of memory.
//
// VMem uses a worker thread to periodically flush. On unix systems you will need to link to pthread.
// The value of this define is how long to delay in ms.
// note: to disable the delay comment out this define rather than setting it to zero.

#define VMEM_DELAYED_RELEASE 1000

//------------------------------------------------------------------------
// define this for a _slight_ speed increase but more memory fragmentation
//#ifndef VMEM_DISABLE_BIASING

//------------------------------------------------------------------------
// enable marker in all configs for x86 because it doesn't take up any more space
#ifndef VMEM64
	#define COALESCE_HEAP_MARKER
#endif

//------------------------------------------------------------------------
#ifdef VMEM_MEMORY_CORRUPTION_FINDER
	#ifndef VMEM_PLATFORM_WIN
		#error					// you can use this on non-windows platforms but it will use up a huge amount of memory
	#endif
	#ifdef VMEM_TRAIL_GUARDS
		#undef VMEM_TRAIL_GUARDS
	#endif
	#ifndef VMEM_DISABLE_BIASING
		#define VMEM_DISABLE_BIASING
	#endif
	#ifdef VMEM_DELAYED_RELEASE
		#undef VMEM_DELAYED_RELEASE
	#endif
#endif

//------------------------------------------------------------------------
// the system page size
#if defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1) || defined(MEMPRO_PLATFORM_GAMES_CONSOLE_2)
	#error Please contact slynch@puredevsoftware.com for this platform
#elif defined(VMEM_PLATFORM_WIN)
	#define SYS_PAGE_SIZE 4096
#elif defined(VMEM_OS_UNIX)
	#define SYS_PAGE_SIZE 4096
#else
	#error platform not defined
#endif

//------------------------------------------------------------------------
#ifdef VMEM_PROTECTED_HEAP
#define VMEM_PROTECTED_HEAP_SIZE (100*1024*1024)

// use this function to decide whether an allocation should go via the protected debug heap.
// i is incremented every allocation, size is the size of the allocation.
// you can also use VMem::SetVMemShouldProtectFn to set your own function.
namespace VMem
{
	inline bool VMemShouldProtect(int i, size_t size)
	{
		return size > SYS_PAGE_SIZE/2 || (i % 100) == 0;
	}
}
#endif

//------------------------------------------------------------------------
// guard sizes
#define FSA_PRE_GUARD_SIZE	0
#define FSA_POST_GUARD_SIZE	4
#define COALESCE_GUARD_SIZE	16

//------------------------------------------------------------------------
// markers
#define FSA_PAGE_HEADER_MARKER	0xf5a0f5a0
#define FSA_MARKER				0xff55aa00
#define COALESCE_ALLOC_MARKER	0xabcdef12
#define LARGE_ALLOC_NODE_MARKER	0x1a1a1a1a

//------------------------------------------------------------------------
// memset codes
#define COMMITTED_MEM	0xcfcfcfcf		// committed but not yet allocated
#define DECOMMITTED_MEM	0xdededede		// decommitted memory (used in delayed decommit)
#define UNUSED_PAGE		0xaeaeaeae
#define ALLOCATED_MEM	0xcdcdcdcd		// allocated but not yet written to
#define FREED_MEM		0xdddddddd		// freed memory
#define GUARD_MEM		0xfdfdfdfd		// guards at ends of allocs
#define TRAIL_GUARD_MEM	0xbdbdbdbd
#define PROTECTED_PAGE	0xafafafaf

//------------------------------------------------------------------------
// misc
#define WRITE_ALLOCS_MAX 16				// max number of allocs WriteAllocs will output for each heap

#ifdef VMEM64
	#define VMEM_INVALID_SIZE ULLONG_MAX
#else
	#define VMEM_INVALID_SIZE UINT_MAX
#endif

#ifdef VMEM_OS_UNIX
	#ifndef _T
		#define _T(s) s
	#endif
	#define _TCHAR char
#endif
#include <limits.h>
#include <stdint.h>
#include <memory.h>

#ifdef VMEM_OS_WIN
	#include <tchar.h>
#else
    #include <stdio.h>
#endif

#ifdef VMEM_PLATFORM_ANDROID
	#include <stdarg.h>
#endif

//-----------------------------------------------------------------------------
#ifdef VMEM_OS_WIN
	#pragma warning(disable:4127)	// conditional expression is constant
#endif

//------------------------------------------------------------------------
#ifdef VMEM_OS_WIN
#define POINTER_PREFIX "0x"
#else
#define POINTER_PREFIX ""
#endif

//------------------------------------------------------------------------
#ifdef VMEM_OS_WIN
	#define VMEM_DEBUG_BREAK __debugbreak()
#else
	#define VMEM_DEBUG_BREAK __builtin_trap()
#endif

//------------------------------------------------------------------------
#define VMEM_START_MULTI_LINE_MACRO	do {

#ifdef VMEM_OS_WIN
	#define VMEM_END_MULTI_LINE_MACRO } while (true,false)		// stupid trick to eliminate the 'conditional expression is constant' warning
#else
	#define VMEM_END_MULTI_LINE_MACRO } while (false)
#endif

//------------------------------------------------------------------------
#define VMEM_UNREFERENCED_PARAM(p) (void)(p)

//------------------------------------------------------------------------
#ifdef VMEM_ASSERTS
	#define VMEM_ASSERT(b, m)				\
		VMEM_START_MULTI_LINE_MACRO			\
		if(!(b))							\
		{									\
			if(!VMem::Break(_T(m)_T("\n")))	\
				VMEM_DEBUG_BREAK;			\
		}									\
		VMEM_END_MULTI_LINE_MACRO
#else
	#define VMEM_ASSERT(b, m) VMEM_UNREFERENCED_PARAM(b)
#endif

//------------------------------------------------------------------------
#ifdef VMEM_ASSERTS
	#define VMEM_ASSERT2(b, m, a1, a2)					\
		VMEM_START_MULTI_LINE_MACRO						\
		if(!(b))										\
		{												\
			VMem::DebugWrite(_T(m)_T("\n"), a1, a2);	\
			if(!VMem::Break(_T("VMem Assert\n")))		\
				VMEM_DEBUG_BREAK;						\
		}												\
		VMEM_END_MULTI_LINE_MACRO
#else
	#define VMEM_ASSERT1(b, m, a) VMEM_UNREFERENCED_PARAM(b)
#endif

//------------------------------------------------------------------------
#ifdef VMEM_ASSERTS
	#define VMEM_MEM_CHECK(p_value, expected_value)	\
		VMEM_START_MULTI_LINE_MACRO					\
		if(*(p_value) != (expected_value))			\
		{											\
			VMem::DebugWrite(sizeof(expected_value) == 8 ? _T("Memory corruption at ")_T(POINTER_PREFIX)_T("%p : value ")_T(POINTER_PREFIX)_T("%p : expected value ")_T(POINTER_PREFIX)_T("%p\n") : _T("Memory corruption at ")_T(POINTER_PREFIX)_T("%p : value 0x%08x : expected value 0x%08x\n"), (p_value), *(p_value), (expected_value));	\
			if(!VMem::Break(_T("VMem Assert\n")))	\
				VMEM_DEBUG_BREAK;					\
		}											\
		VMEM_END_MULTI_LINE_MACRO
#else
	#define VMEM_MEM_CHECK(p_value, expected_value) VMEM_UNREFERENCED_PARAM(p_value)
#endif

//------------------------------------------------------------------------
#ifdef VMEM_ASSERTS
	#define VMEM_ASSERT_MEM(b, p_value)				\
		VMEM_START_MULTI_LINE_MACRO					\
		if(!(b))									\
		{											\
			VMem::DebugWrite(_T("Memory corruption at ")_T(POINTER_PREFIX)_T("%p : value ")_T(POINTER_PREFIX)_T("%p\n"), (p_value), *((void**)(p_value)));	\
			if(!VMem::Break(_T("VMem Assert\n")))	\
				VMEM_DEBUG_BREAK;					\
		}											\
		VMEM_END_MULTI_LINE_MACRO
#else
	#define VMEM_ASSERT_MEM(b, p_value) VMEM_UNREFERENCED_PARAM(b)
#endif

//------------------------------------------------------------------------
#ifdef VMEM_ASSERTS
	#define VMEM_BREAK(m)				\
		VMEM_START_MULTI_LINE_MACRO		\
		if(!VMem::Break(_T(m)))			\
			VMEM_DEBUG_BREAK;			\
		VMEM_END_MULTI_LINE_MACRO
#else
	#define VMEM_BREAK(m) VMEM_UNREFERENCED_PARAM(m)
#endif

//------------------------------------------------------------------------
// compile time assert
#define VMEM_STATIC_ASSERT(expr) typedef char __VMEM_STATIC_ASSERT_t[ (expr) ? 1 : -1 ]

//------------------------------------------------------------------------
// for code that is only needed when asserts are turned on
#ifdef VMEM_ASSERTS
	#define VMEM_ASSERT_CODE(e) e
#else
	#define VMEM_ASSERT_CODE(e)
#endif

//------------------------------------------------------------------------
// for code that is only needed when memsetting is turned on
#if defined(VMEM_MEMSET)
	#define VMEMSET(p, value, size) memset(p, value, size)
#elif defined(VMEM_MEMSET_ONLY_SMALL)
	#define VMEMSET(p, value, size) memset(p, value, Min((size_t)size, (size_t)VMEM_MEMSET_ONLY_SMALL))
#else
	#define VMEMSET(p, value, size) VMEM_UNREFERENCED_PARAM(p)
#endif

//------------------------------------------------------------------------
// for code that is only needed when stats are turned on
#ifdef VMEM_STATS
	#define VSTATS(e) e
#else
	#define VSTATS(e)
#endif

//------------------------------------------------------------------------
#ifdef VMEM64
	#define VMEM64_ONLY(s) s
#else
	#define VMEM64_ONLY(s)
#endif

//------------------------------------------------------------------------
#ifdef VMEM64
	VMEM_STATIC_ASSERT(sizeof(void*) == sizeof(long long));		// please undefine VMEM64 for x86 builds
#else
	VMEM_STATIC_ASSERT(sizeof(void*) == sizeof(unsigned int));		// please define VMEM64 for 64bit builds
#endif

//------------------------------------------------------------------------
VMEM_STATIC_ASSERT(sizeof(size_t) == sizeof(void*));

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	// used for MemPro stats
	enum VmemAllocatorType
	{
		vmem_BasicCoalesceHeap = 0,
		vmem_PageHeap,
		vmem_FSAHeap,
		vmem_CoalesceHeap,
		vmem_LargeHeap,
		vmem_End
	};

	//------------------------------------------------------------------------
	void InitialiseCore();

	void UninitialiseCore(bool clean);

	bool Break(const _TCHAR* p_message);

	void DebugWrite(const _TCHAR* p_message, ...);

	void SetDebugBreakFunction(void (*DebugBreakFn)(const _TCHAR* p_message));

	//------------------------------------------------------------------------
	typedef unsigned char byte;

	//------------------------------------------------------------------------
	#if defined(VMEM_OS_WIN)
		typedef unsigned __int64 uint64;
	#elif defined(VMEM_OS_UNIX)
		typedef uint64_t uint64;
	#else
		#error platform undefined
	#endif

	//------------------------------------------------------------------------
	typedef void (*MemProSendFn)(void*, int, void*);

	//------------------------------------------------------------------------
	template<typename T>
	inline T Min(T a, T b)
	{
		return a < b ? a : b;
	}

	//------------------------------------------------------------------------
	template<typename T>
	inline T Max(T a, T b)
	{
		return a > b ? a : b;
	}

	//-----------------------------------------------------------------------------
	template<typename T>
	inline void SendToMemPro(T& value, MemProSendFn send_fn, void* p_context)
	{
		send_fn(&value, sizeof(value), p_context);
	}

	//-----------------------------------------------------------------------------
	template<typename T>
	inline void SendEnumToMemPro(T value, MemProSendFn send_fn, void* p_context)
	{
		send_fn(&value, sizeof(value), p_context);
	}

	//-----------------------------------------------------------------------------
	inline bool IsPow2(unsigned int value)
	{
		return (value & (value-1)) == 0;
	}

	//------------------------------------------------------------------------
	inline int AlignUp(int i, int alignment)
	{
		return (((i-1) / alignment) + 1) * alignment;
	}

	//------------------------------------------------------------------------
	inline int AlignUpPow2(int i, int alignment)
	{
		VMEM_ASSERT(IsPow2(alignment), "non-pow2 value passed to AlignDown");
		int mask = alignment - 1;
		return (i + mask) & ~mask;
	}

	//------------------------------------------------------------------------
	inline size_t AlignSizeUpPow2(size_t i, int alignment)
	{
		VMEM_ASSERT(IsPow2(alignment), "non-pow2 value passed to AlignDown");
		int mask = alignment - 1;
		return (i + mask) & ~mask;
	}

	//------------------------------------------------------------------------
	inline void* AlignUpPow2(void* p, int alignment)
	{
		VMEM_ASSERT(IsPow2(alignment), "non-pow2 value passed to AlignDown");
		size_t i = (size_t)p;
		int mask = alignment - 1;
		return (void*)((i + mask) & ~mask);
	}

	//------------------------------------------------------------------------
	inline int AlignDownPow2(int i, int alignment)
	{
		VMEM_ASSERT(IsPow2(alignment), "non-pow2 value passed to AlignDown");
		return i & ~(alignment-1);
	}

	//------------------------------------------------------------------------
	inline void* AlignDownPow2(void* p, int alignment)
	{
		VMEM_ASSERT(IsPow2(alignment), "non-pow2 value passed to AlignDown");
		size_t i = (size_t)p;
		return (void*)(i & ~(alignment-1));
	}

	//------------------------------------------------------------------------
	void WriteAlloc(const void* p_alloc, size_t size);

	//------------------------------------------------------------------------
	inline void SetGuards(void* p, int size)
	{
		VMEM_ASSERT((((size_t)p) & 3) == 0, "guard pointer misaligned");
		VMEM_ASSERT((size & 3) == 0, "guard pointer misaligned");

		unsigned int* p_guard = (unsigned int*)p;
		unsigned int* p_end_guard = (unsigned int*)((byte*)p + size);
		while(p_guard != p_end_guard)
			*p_guard++ = GUARD_MEM;
	}

	//------------------------------------------------------------------------
	inline void CheckMemory(void* p, size_t size, unsigned int value)
	{
#ifdef VMEM_ASSERTS
		VMEM_ASSERT((((size_t)p) & 3) == 0, "CheckMemory pointer misaligned");
		VMEM_ASSERT((size & 3) == 0, "CheckMemory size misaligned");

		#ifdef VMEM_MEMSET_ONLY_SMALL
			size = Min(size, (size_t)VMEM_MEMSET_ONLY_SMALL);
		#endif
		VMEM_ASSERT(size % sizeof(unsigned int) == 0, "bad size");
		unsigned int* p_uint = (unsigned int*)p;
		unsigned int* p_end = (unsigned int*)((byte*)p + size);
		while(p_uint != p_end)
		{
			VMEM_MEM_CHECK(p_uint, value);
			++p_uint;
		}
#else
		VMEM_UNREFERENCED_PARAM(p);
		VMEM_UNREFERENCED_PARAM(size);
		VMEM_UNREFERENCED_PARAM(value);
#endif
	}

	//------------------------------------------------------------------------
	inline int ToInt(size_t size)
	{
		VMEM_ASSERT(size <= INT_MAX, "size out of range");
		return (int)size;
	}

	//------------------------------------------------------------------------
	inline size_t ToSizeT(int value)
	{
		VMEM_ASSERT(value >= 0, "value out of range while converting to size_t");
		return (size_t)value;
	}

	//------------------------------------------------------------------------
	#ifndef VMEM_OS_WIN
		inline void memcpy_s(void* p_dst, size_t dst_len, void* p_src, size_t size) { memcpy(p_dst, p_src, size); }
		inline int _vstprintf_s(char *buffer, size_t numberOfElements, const char *format, va_list argptr) { return vsprintf(buffer, format, argptr); }
	#endif
}

//------------------------------------------------------------------------
VMEM_STATIC_ASSERT(sizeof(VMem::byte) == 1);
VMEM_STATIC_ASSERT(sizeof(int) == 4);
VMEM_STATIC_ASSERT(sizeof(unsigned int) == 4);
VMEM_STATIC_ASSERT(sizeof(long long) == 8);
VMEM_STATIC_ASSERT(sizeof(VMem::uint64) == 8);

//------------------------------------------------------------------------
// VMemStats.hpp
#include <stddef.h>

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	class Stats
	{
	public:
		inline Stats();

		inline Stats& operator+=(const Stats& other);

		inline size_t GetCommittedBytes() const;

		//------------------------------------------------------------------------
		// data
	public:
		size_t m_Used;			// memory that the allocator has allocated (in use by the app)
		size_t m_Unused;		// memory that the allocator has committed but not currently in use,
		size_t m_Overhead;		// memory used internally by the allocator to manage data structures.
		size_t m_Reserved;		// reserved memory (includes all committed memory)
	};

	//------------------------------------------------------------------------
	Stats::Stats()
	:	m_Used(0),
		m_Unused(0),
		m_Overhead(0),
		m_Reserved(0)
	{
	}

	//------------------------------------------------------------------------
	struct VMemHeapStats
	{
		Stats m_FSAHeap1;
		Stats m_FSAHeap2;
		Stats m_CoalesceHeap1;
		Stats m_CoalesceHeap2;
		Stats m_LargeHeap;
		Stats m_Internal;
		Stats m_Total;
	};

	//------------------------------------------------------------------------
	Stats& Stats::operator+=(const Stats& other)
	{
		m_Used += other.m_Used;
		m_Unused += other.m_Unused;
		m_Overhead += other.m_Overhead;
		m_Reserved += other.m_Reserved;
		return *this;
	}

	//------------------------------------------------------------------------
	size_t Stats::GetCommittedBytes() const
	{
		return m_Used + m_Unused + m_Overhead;
	}

	//------------------------------------------------------------------------
	inline Stats operator+(const Stats& s1, const Stats& s2)
	{
		Stats stats;
		stats += s1;
		stats += s2;
		return stats;
	}
}

//------------------------------------------------------------------------
#ifdef VMEM_OS_WIN
	#include <tchar.h>
#endif

//------------------------------------------------------------------------
// Note: Calling GetSize or Realloc on an allocation that is currently in the process
// of being freed on another thread will cause undefined behavour. It may crash or
// return invalid results.
namespace VMem
{
	void* Alloc(size_t size);

	void Free(void* p);

	size_t GetSize(void* p);

	void Lock();

	void Release();

	void* Realloc(void* p, size_t new_size);

	void Initialise();

	void Destroy(bool clean=true);

	void CheckIntegrity();

	void SetDebugBreak(void (*DebugBreakFn)(const _TCHAR* p_message));

	typedef void (*LogFn)(const _TCHAR* p_message);
	void SetLogFunction(LogFn log_fn);

#ifdef VMEM_STATS
	VMemHeapStats GetStats();
	void WriteStats();
	size_t WriteAllocs();
	void SendStatsToMemPro(void (*send_fn)(void*, int, void*), void* p_context);
#endif

#ifdef VMEM_PROTECTED_HEAP
	typedef bool (*VMemSHouldProtectFn)(int i, size_t size);
	void SetVMemShouldProtectFn(VMemSHouldProtectFn should_protect_fn);
#endif
}

//------------------------------------------------------------------------
#if defined(OVERRIDE_MALLOC)

	#error // not supported, see comment below
	//
	// Overriding malloc and free is not supported by the C++ standard, and
	// is not officially supported by VMem.
	//
	// However, it is usually possible to replace the functions. 
	// If you do manage to override them VMem will continue to work. VMem
	// uses lower level memory allocation functions and doesn't call malloc itself.
	// If you do run into any problems with VMem after overriding malloc/free
	// please let me know.
	//
	// Linkers will usually look in the crt libs last for an implementation, so by adding
	// your own functions **that match the definitions exactly** you can override
	// them. However, be aware that it is very system dependent, and is not
	// guaranteed to work. The file that they are overridden in does matter.
	// (the code can't be in a header file with gcc for example).
	//
	// The best thing to do is to copy the following 4 functions and paste them
	// into your application main.cpp, ideally as one of the first things the
	// file does, before any standard headers.

	// *********************************************************
	// ******** Copy these functions into your main.cpp ********
	/*
	void* malloc(size_t	size)
	{
		return VMem::Alloc(size);
	}

	void* calloc(size_t size, size_t count)
	{
		int total_size = count * size;
		void* p = VMem::Alloc(total_size);
		memset(p, 0, total_size);
		return p;
	}

	void* realloc(void *p, size_t new_size)
	{
		return VMem::Realloc(p, new_size);
	}

	void free(void *p)
	{
		VMem::Free(p);
	}
	*/
	// *********************************************************
#endif

//------------------------------------------------------------------------
#ifdef OVERRIDE_NEW_DELETE

#ifdef MEMPRO
	#undef OVERRIDE_NEW_DELETE
	#include "MemPro.hpp"
	#define OVERRIDE_NEW_DELETE
	#define WAIT_FOR_CONNECTION true
	#define TRACK_ALLOC(p, size)	MemPro::TrackAlloc(p, size, WAIT_FOR_CONNECTION)
	#define TRACK_FREE(p)			MemPro::TrackFree(p, WAIT_FOR_CONNECTION)
#else
	#define TRACK_ALLOC(p, size)
	#define TRACK_FREE(p)
#endif

	#include <new>

#ifdef VMEM_PLATFORM_APPLE
	void* operator new(std::size_t size) throw(std::bad_alloc)
	{
		void* p = VMem::Alloc(size);
		TRACK_ALLOC(p, size);
		return p;
	}

	void* operator new(std::size_t size, const std::nothrow_t&) throw()
	{
		void* p = VMem::Alloc(size);
		TRACK_ALLOC(p, size);
		return p;
	}

	void  operator delete(void* p) throw()
	{
		TRACK_FREE(p);
		VMem::Free(p);
	}

	void  operator delete(void* p, const std::nothrow_t&) throw()
	{
		TRACK_FREE(p);
		VMem::Free(p);
	}

	void* operator new[](std::size_t size) throw(std::bad_alloc)
	{
		void* p = VMem::Alloc(size);
		TRACK_ALLOC(p, size);
		return p;
	}

	void* operator new[](std::size_t size, const std::nothrow_t&) throw()
	{
		void* p = VMem::Alloc(size);
		TRACK_ALLOC(p, size);
		return p;
	}

	void  operator delete[](void* p) throw()
	{
		TRACK_FREE(p);
		VMem::Free(p);
	}

	void  operator delete[](void* p, const std::nothrow_t&) throw()
	{
		TRACK_FREE(p);
		VMem::Free(p);
	}
#else
	void* operator new(size_t size)
	{
		void* p = VMem::Alloc(size);
		TRACK_ALLOC(p, size);
		return p;
	}

	void operator delete(void* p)
	{
		TRACK_FREE(p);
		VMem::Free(p);
	}

	void* operator new[](size_t size)
	{
		void* p = VMem::Alloc(size);
		TRACK_ALLOC(p, size);
		return p;
	}

	void operator delete[](void* p)
	{
		TRACK_FREE(p);
		VMem::Free(p);
	}
#endif

#endif

//------------------------------------------------------------------------
#endif		// #ifndef VMEM_VMEMALLOC_H_INCLUDED

