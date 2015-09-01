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

#include "GameStdAfx.h"
#include "VMem.hpp"

//------------------------------------------------------------------------
// BasicCoalesceHeap.cpp

//------------------------------------------------------------------------
// VMem_PCH.hpp
#include <stdlib.h>
#include <new>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef VMEM_OS_WIN
	#include <tchar.h>
#endif

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
#ifndef VMEM_PLATFORM_ANDROID
	#include <memory>
#endif

//-----------------------------------------------------------------------------
#if defined(VMEM_PLATFORM_WIN)
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0501
	#endif						
	#define WINDOWS_LEAN_AND_MEAN
	#include <windows.h>
	#include <intrin.h>
#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1) || defined(MEMPRO_PLATFORM_GAMES_CONSOLE_2)
	#error Please contact slynch@puredevsoftware.com for this platform
#endif

//------------------------------------------------------------------------
// BasicCoalesceHeap.hpp



//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	// BasicCoalesceHeap is different from the standard Coalesce heap in that it
	// doesn't allocate seperate nodes for the free list and doesn't do any biasing.
	// BasicCoalesceHeap doesn't rely on any other allocators and gets its memory
	// directly from the system.
	// It only has a very small overhead and does a simple linear search for a big
	// enough block of memory for each alloc. BasicCoalesceHeap is only intended for
	// internal use by VMem. All allocations are guaranteed to be aligned to 4 bytes.
	// BasicCoalesceHeap is NOT thread safe.
	class BasicCoalesceHeap
	{
		// the header is located immediately before the allocation or at the start of the free block
		struct Header
		{
#ifdef VMEM_ASSERTS
			unsigned int m_Marker;
#endif
			Header* mp_Prev;		// the prev and next free block of memory.
			Header* mp_Next;		// if the header is allocated prev and next pointers will be null.
			int m_Size;				// the size includes the size of the header
			int m_PrevSize;
#ifdef VMEM_STATS
			int m_RequestedSize;
#endif
		};

		struct Region
		{
			Header m_FreeList;
			Region* mp_Next;
		};

		//------------------------------------------------------------------------
	public:
		BasicCoalesceHeap(int region_size);

		~BasicCoalesceHeap();

		void* Alloc(size_t size);

		void Free(void* p);

		void CheckIntegrity();

		void DebugWrite() const;

#ifdef VMEM_STATS
		inline const Stats& GetStats() const;
		void SendStatsToMemPro(MemProSendFn send_fn, void* p_context);
#endif

	private:
		Region* CreateRegion();

		void DestroyRegion(Region* p_region);

		void DebugWrite(Header* p_header) const;

		void ClearRegion(Region* p_region);

		inline void Unlink(Header* p_header);

		//------------------------------------------------------------------------
		// data
	private:
		Region* mp_RegionList;
		int m_RegionSize;

#ifdef VMEM_STATS
		Stats m_Stats;
#endif
	};

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	const Stats& BasicCoalesceHeap::GetStats() const
	{
		return m_Stats;
	}
#endif
}

//------------------------------------------------------------------------
// VirtualMem.hpp
#ifndef VIRTUALMEM_H_INCLUDED
#define VIRTUALMEM_H_INCLUDED

//------------------------------------------------------------------------



//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	namespace VirtualMem
	{
		void Initialise();

		void CreatePulseThread();

		void Destroy(bool clean);

		void* Reserve(size_t size, int align);

		void Release(void* p);

		bool Commit(void* p, size_t size);

		void Decommit(void* p, size_t size);

		Stats GetStats();

		void Lock();

		void Release();
	};
}

//------------------------------------------------------------------------
#endif		// #ifndef VIRTUALMEM_H_INCLUDED

//------------------------------------------------------------------------
// VMemMemProStats.hpp



//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	namespace MemProStats
	{
		//------------------------------------------------------------------------
		// make everything use uint64 to avoid packing issues and simplify endian byte swapping
		struct Stats
		{
			Stats& operator=(const VMem::Stats& other)
			{
				m_Used = other.m_Used;
				m_Unused = other.m_Unused;
				m_Overhead = other.m_Overhead;
				m_Reserved = other.m_Reserved;
				return *this;
			}

			uint64 m_Used;
			uint64 m_Unused;
			uint64 m_Overhead;
			uint64 m_Reserved;
		};

		//------------------------------------------------------------------------
		struct BasicCoalesceHeapStats
		{
			uint64 m_RegionCount;
			uint64 m_RegionSize;
			Stats m_Stats;
		};

		//------------------------------------------------------------------------
		struct PageHeapStats
		{
			uint64 m_PageSize;
			uint64 m_RegionSize;
			uint64 m_RegionCount;
			Stats m_Stats;
		};

		//------------------------------------------------------------------------
		struct FSAHeapStats
		{
			uint64 m_FSACount;
			Stats m_Stats;
		};

		//------------------------------------------------------------------------
		struct FSAStats
		{
			uint64 m_Size;
			uint64 m_SlotSize;
			uint64 m_Alignment;
			uint64 m_FreePageCount;
			uint64 m_FullPageCount;
			Stats m_Stats;
		};

		//------------------------------------------------------------------------
		struct CoalesceHeapStats
		{
			uint64 m_MinSize;
			uint64 m_MaxSize;
			uint64 m_RegionSize;
			uint64 m_RegionCount;
			Stats m_Stats;
		};

		//------------------------------------------------------------------------
		struct LageHeapStats
		{
			uint64 m_AllocCount;
			Stats m_Stats;
		};
	}
}

//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
BasicCoalesceHeap::BasicCoalesceHeap(int region_size)
:	mp_RegionList(NULL),
	m_RegionSize(AlignUpPow2(region_size, SYS_PAGE_SIZE))
{
}

//------------------------------------------------------------------------
BasicCoalesceHeap::~BasicCoalesceHeap()
{
	// ideally the region will be empty when the coalesce heap is destructed
	// but we will clean up properly anyway. We can't just release all the
	// virtual memory otherwise the committed pages count will be wrong and
	// the free nodes won't be deleted. So we go through all allocations and 
	// delete them using the normal method.
	while(mp_RegionList)
		ClearRegion(mp_RegionList);
}

//------------------------------------------------------------------------
BasicCoalesceHeap::Region* BasicCoalesceHeap::CreateRegion()
{
	// commit the region memory
	// the region struct is at the start of the committed memory
	Region* p_region = (Region*)VirtualMem::Reserve(m_RegionSize, SYS_PAGE_SIZE);
	if(!p_region)
		return NULL;		// out of memory

	if(!VirtualMem::Commit(p_region, m_RegionSize))
		return NULL;		// out of memory

	// initialise the region
	p_region->mp_Next = NULL;

	// initialise the free list header
	Header& free_list_head = p_region->m_FreeList;
	memset(&free_list_head, 0, sizeof(Header));

	// setup first header 'allocated' header. This header is never used
	// but makes the coalesce logic a simpler.
	Header* p_first_header = (Header*)(p_region + 1);
	VMEM_ASSERT_CODE(p_first_header->m_Marker = COALESCE_ALLOC_MARKER);
	p_first_header->mp_Prev = NULL;				// NULL prev and next pointers mean header is allocated
	p_first_header->mp_Next = NULL;
	p_first_header->m_Size = sizeof(Header);
	VSTATS(p_first_header->m_RequestedSize = 0);
	p_first_header->m_PrevSize = 0;

	// setup free header and link it to the first header
	Header* p_header = p_first_header + 1;
	VMEM_ASSERT_CODE(p_header->m_Marker = COALESCE_ALLOC_MARKER);
	p_header->mp_Prev = &free_list_head;		// circular linked list
	p_header->mp_Next = &free_list_head;
	int total_free_size = m_RegionSize - sizeof(Region) - 2 * sizeof(Header);
	p_header->m_Size = total_free_size;			// includes size of the header
	VSTATS(p_header->m_RequestedSize = 0);
	p_header->m_PrevSize = sizeof(Header);
	free_list_head.mp_Prev = p_header;
	free_list_head.mp_Next = p_header;

	// setup last header guard
	Header* p_last_header = (Header*)((byte*)p_header + p_header->m_Size);
	VMEM_ASSERT_CODE(p_last_header->m_Marker = COALESCE_ALLOC_MARKER);
	p_last_header->mp_Prev = NULL;				// NULL prev and next pointers mean header is allocated
	p_last_header->mp_Next = NULL;
	p_last_header->m_Size = sizeof(Header);
	VSTATS(p_last_header->m_RequestedSize = 0);
	p_last_header->m_PrevSize = total_free_size;

	// update stats
	VSTATS(m_Stats.m_Reserved += m_RegionSize);
	VSTATS(m_Stats.m_Unused += m_RegionSize - sizeof(Region) - 3*sizeof(Header));
	VSTATS(m_Stats.m_Overhead += sizeof(Region) + 3*sizeof(Header));

	return p_region;
}

//------------------------------------------------------------------------
// WARNING: this deletes p_region, don't use p_region after calling this function.
// This function calls the standard Free function on each alloc in the region until
// the region disappears. It's slightly messy, but avoids duplicating the code in
// the Free function.
void BasicCoalesceHeap::ClearRegion(Region* p_region)
{
	// get the first alloc that we can free
	Header* p_header = (Header*)(p_region + 1) + 1;

	// first we work out how many bytes have been allocated so that we
	// know when we are done. This is needed because the last Free call
	// will delete the region and all headers, so we need a simple way
	// of telling when this happens.
	int size = 0;
	Header* p_end_header = (Header*)((byte*)p_region + m_RegionSize) - 1;
	for(Header* p_h = p_header; p_h != p_end_header; p_h=(Header*)((byte*)p_h + p_h->m_Size))
	{
		VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_h->m_Marker, COALESCE_ALLOC_MARKER));

		if(!p_h->mp_Prev)
			size += p_h->m_Size;
	}

	// now keep deleting the first allocated block in the list until all
	// allocations are freed (which frees the region)
	while(size)
	{
		Header* p_alloc_header = p_header;

		// if p_header is free the next header is garanteed to be allocated
		if(p_alloc_header->mp_Prev)
			p_alloc_header = (Header*)((byte*)p_alloc_header + p_alloc_header->m_Size);
		VMEM_MEM_CHECK(&p_alloc_header->mp_Prev, NULL);

		size -= p_alloc_header->m_Size;
		VMEM_ASSERT_MEM(p_alloc_header->m_Size >= 0, &p_alloc_header->m_Size);

		// warning: this deletes p_region when it's empty, so be careful not to access p_region after last free
		void* p = p_alloc_header + 1;
#ifdef VMEM_COALESCE_GUARDS
		p = (byte*)p + COALESCE_GUARD_SIZE;
#endif
		Free(p);
	}
}

//------------------------------------------------------------------------
// releases the memory associated with the region. Do not use p_region after this.
void BasicCoalesceHeap::DestroyRegion(Region* p_region)
{
	// check it's empty
#ifdef VMEM_ASSERTS
	int total_free_size = m_RegionSize - sizeof(Region) - 2 * sizeof(Header);
	Header* p_header = (Header*)(p_region + 1) + 1;
	VMEM_ASSERT_MEM(p_header->m_Size == total_free_size, &p_header->m_Size);		// destroying non-empty region
#endif

	// release memory
	VirtualMem::Decommit(p_region, m_RegionSize);
	VirtualMem::Release(p_region);

	// update stats
	VSTATS(m_Stats.m_Reserved -= m_RegionSize);
	VSTATS(m_Stats.m_Unused -= m_RegionSize - sizeof(Region) - 3*sizeof(Header));
	VSTATS(m_Stats.m_Overhead -= sizeof(Region) + 3*sizeof(Header));
}

//------------------------------------------------------------------------
void* BasicCoalesceHeap::Alloc(size_t size)
{
	int i_size = ToInt(size);

	VMEM_ASSERT((i_size&3) == 0, "internal allocs must be aligned to 4 or stats go wrong");

	// work out the size of the block
	int total_size = i_size + sizeof(Header);
#ifdef VMEM_COALESCE_GUARDS
	total_size += 2*COALESCE_GUARD_SIZE;
#endif

#ifdef VMEM_ASSERTS
	int total_free_size = m_RegionSize - sizeof(Region) - 2 * sizeof(Header);
	VMEM_ASSERT(total_size < total_free_size, "allocation bigger than region size");
#endif

	// get the first region (or create one)
	Region* p_region = mp_RegionList;
	if(!p_region)
		p_region = mp_RegionList = CreateRegion();

	// linear search through the regions for a big enough free block
	while(p_region)
	{
		Header* p_list_head = &p_region->m_FreeList;
		
		// linear search through the free block list for a big enough block
		for(Header* p_header=p_list_head->mp_Next; p_header != p_list_head; p_header=p_header->mp_Next)
		{
			VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_header->m_Marker, COALESCE_ALLOC_MARKER));
			VMEM_ASSERT_CODE(VMEM_ASSERT_MEM(p_header->mp_Prev, &p_header->mp_Prev));

			// work out what the offcut would be
			int offcut_size = p_header->m_Size - total_size;

			// if we have found an allocation big enough
			if(offcut_size >= 0)
			{
				Header* p_prev_free_header = p_header->mp_Prev;
				Header* p_next_free_header = p_header->mp_Next;

				// ignore offcut if too small
				if(offcut_size < (int)sizeof(Header))
				{
					total_size += offcut_size;
					offcut_size = 0;
				}

				// get the next header
				Header* p_next_header = (Header*)((byte*)p_header + p_header->m_Size);
				VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_next_header->m_Marker, COALESCE_ALLOC_MARKER));

				if(offcut_size)
				{
					// create a new header for the offcut
					Header* p_offcut_header = (Header*)((byte*)p_header + total_size);
					VMEM_ASSERT_CODE(p_offcut_header->m_Marker = COALESCE_ALLOC_MARKER);
					p_offcut_header->m_Size = offcut_size;
					VSTATS(p_offcut_header->m_RequestedSize = 0);
					p_offcut_header->m_PrevSize = total_size;

					// link new offcut header into free list
					VMEM_ASSERT_MEM(p_header->mp_Prev, &p_header->mp_Prev);
					p_prev_free_header->mp_Next = p_offcut_header;
					p_offcut_header->mp_Prev = p_prev_free_header;
					p_offcut_header->mp_Next = p_next_free_header;
					p_next_free_header->mp_Prev = p_offcut_header;

					// update the prev size of the next header in memory
					p_next_header->m_PrevSize = offcut_size;

					VSTATS(m_Stats.m_Unused -= sizeof(Header));
					VSTATS(m_Stats.m_Overhead += sizeof(Header));
				}
				else
				{
					// no offcut, so remove the original header from the free list
					VMEM_ASSERT_MEM(p_header->mp_Prev, &p_header->mp_Prev);
					p_prev_free_header->mp_Next = p_next_free_header;
					p_next_free_header->mp_Prev = p_prev_free_header;

					// update the prev size of the next header in memory
					p_next_header->m_PrevSize = total_size;
				}

				// setup the header for this alloc
				p_header->mp_Prev = NULL;		// NULL prev and next pointers mean header is allocated
				p_header->mp_Next = NULL;
				p_header->m_Size = total_size;
				VSTATS(p_header->m_RequestedSize = i_size);

				void* p = p_header + 1;
				VMEMSET(p, ALLOCATED_MEM, i_size);

				// set guards
#ifdef VMEM_COALESCE_GUARDS
				SetGuards(p, COALESCE_GUARD_SIZE);
				SetGuards((byte*)p_header + total_size - COALESCE_GUARD_SIZE, COALESCE_GUARD_SIZE);
				p = (byte*)p + COALESCE_GUARD_SIZE;
#endif
				// update stats
				VSTATS(m_Stats.m_Unused -= total_size - sizeof(Header));
				VSTATS(m_Stats.m_Used += i_size);
				VSTATS(m_Stats.m_Overhead += total_size - sizeof(Header) - i_size);

				return p;
			}
		}

		// region full, try the next one
		if(!p_region->mp_Next)
			p_region->mp_Next = CreateRegion();
		p_region = p_region->mp_Next;
	}

	return NULL;	// out of memory
}

//------------------------------------------------------------------------
void BasicCoalesceHeap::Free(void* p)
{
	// find the region that contains this alloc
	Region* p_region = mp_RegionList;
	while(p_region && (p < p_region || p >= (byte*)p_region + m_RegionSize))
		p_region = p_region->mp_Next;
	VMEM_ASSERT(p_region, "failed to find region for allocation");

#ifdef VMEM_COALESCE_GUARDS
	p = (byte*)p - COALESCE_GUARD_SIZE;
#endif

	// check the header
	Header* p_header = (Header*)p - 1;
	VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_header->m_Marker, COALESCE_ALLOC_MARKER));
	VMEM_ASSERT_MEM(!p_header->mp_Prev, &p_header->mp_Prev);		// allocation already freed?
	VMEM_ASSERT_MEM(!p_header->mp_Next, &p_header->mp_Next);		// header already on free list?

	VMEM_ASSERT_MEM(p_header->m_Size, &p_header->m_Size);
	int size = p_header->m_Size;

	// check the guards
#ifdef VMEM_COALESCE_GUARDS
	CheckMemory(p, COALESCE_GUARD_SIZE, GUARD_MEM);
	CheckMemory((byte*)p_header + size - COALESCE_GUARD_SIZE, COALESCE_GUARD_SIZE, GUARD_MEM);
#endif
	// stats
	VSTATS(m_Stats.m_Unused += p_header->m_Size - sizeof(Header));
	VSTATS(m_Stats.m_Used -= p_header->m_RequestedSize);
	VSTATS(m_Stats.m_Overhead -= p_header->m_Size - sizeof(Header) - p_header->m_RequestedSize);

	// clear the memory
	VMEMSET(p, FREED_MEM, size - sizeof(Header));

	// get the prev and next headers
	VMEM_ASSERT_MEM(p_header->m_PrevSize, &p_header->m_PrevSize);
	Header* p_prev_header = (Header*)((byte*)p_header - p_header->m_PrevSize);
	Header* p_next_header = (Header*)((byte*)p_header + size);
	int merged_size = size;

	//----------------------------
	// merge with prev header if it's free (mp_Prev!=NULL means free)
	if(p_prev_header && p_prev_header->mp_Prev)
	{
		VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_prev_header->m_Marker, COALESCE_ALLOC_MARKER));

		// merge with prev header
		merged_size += p_prev_header->m_Size;

		p_prev_header->m_Size = merged_size;
		p_next_header->m_PrevSize = merged_size;

		VMEMSET(p_header, FREED_MEM, sizeof(Header));

		p_header = p_prev_header;

		// remove from free list
		Unlink(p_header);

		VSTATS(m_Stats.m_Unused += sizeof(Header));
		VSTATS(m_Stats.m_Overhead -= sizeof(Header));
	}

	//----------------------------
	// merge with next header if it's free (mp_Prev!=NULL means free)
	if(p_next_header->mp_Prev)
	{
		VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_next_header->m_Marker, COALESCE_ALLOC_MARKER));

		merged_size += p_next_header->m_Size;

		Header* p_next_next_header = (Header*)((byte*)p_next_header + p_next_header->m_Size);
		if(p_next_next_header != (Header*)((byte*)p_region + m_RegionSize))
		{
			VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_next_next_header->m_Marker, COALESCE_ALLOC_MARKER));
			p_next_next_header->m_PrevSize = merged_size;
		}

		p_header->m_Size = merged_size;

		// remove next header from free list
		Unlink(p_next_header);

		VMEMSET(p_next_header, FREED_MEM, sizeof(Header));

		VSTATS(m_Stats.m_Unused += sizeof(Header));
		VSTATS(m_Stats.m_Overhead -= sizeof(Header));
	}

	//----------------------------
	// add header to free list
	Header* p_prev = &p_region->m_FreeList;
	Header* p_next = p_prev->mp_Next;
	p_prev->mp_Next = p_header;
	p_header->mp_Prev = p_prev;
	p_header->mp_Next = p_next;
	p_next->mp_Prev = p_header;

	//----------------------------
	// remove the region if it's empty
	int total_free_size = m_RegionSize - sizeof(Region) - 2 * sizeof(Header);
	if(merged_size == total_free_size)
	{
		Region* p_prev_region = NULL;
		for(Region* p_srch_region = mp_RegionList; p_srch_region!=p_region; p_srch_region=p_srch_region->mp_Next)
			p_prev_region = p_srch_region;

		Region* p_next_region = p_region->mp_Next;
		if(p_prev_region)
			p_prev_region->mp_Next = p_next_region;
		else
			mp_RegionList = p_next_region;

		DestroyRegion(p_region);
	}
}

//------------------------------------------------------------------------
void BasicCoalesceHeap::Unlink(Header* p_header)
{
	// prev and next pointers always guaranteed to be valid because of the fixed start and end headers
	Header* p_prev = p_header->mp_Prev;
	Header* p_next = p_header->mp_Next;
	p_prev->mp_Next = p_next;
	p_next->mp_Prev = p_prev;
}

//------------------------------------------------------------------------
void BasicCoalesceHeap::CheckIntegrity()
{
#ifdef VMEM_ASSERTS
	for(Region* p_region = mp_RegionList; p_region!=NULL; p_region=p_region->mp_Next)
	{
		int total_size = sizeof(Region);

		//----------------------------
		// check all headers
		Header* p_last_header = (Header*)((byte*)p_region + m_RegionSize);
		Header* p_header = (Header*)(p_region + 1);
		bool last_free = false;
		int prev_size = 0;
		while(p_header != p_last_header)
		{
			// check marker
			VMEM_MEM_CHECK(&p_header->m_Marker, COALESCE_ALLOC_MARKER);

			int size = p_header->m_Size;
			total_size += size;

			// check prev size
			VMEM_ASSERT_MEM(p_header->m_PrevSize == prev_size, &p_header->m_PrevSize);
			prev_size = size;

			// check no two consecutive free blocks
			bool free = p_header->mp_Prev != NULL;
			VMEM_ASSERT(!(free && last_free), "two consecutive free blocks, they should have been coalesced");
			last_free = free;

			p_header = (Header*)((byte*)p_header + p_header->m_Size);
		}

		VMEM_ASSERT(total_size == m_RegionSize, "blocks don't add up to region size");

		//----------------------------
		// check free list
		Header* p_head = &p_region->m_FreeList;
		Header* p_prev_header = p_head;
		for(Header* p_header = p_head->mp_Next; p_header!=p_head; p_header=p_header->mp_Next)
		{
			// check marker
			VMEM_MEM_CHECK(&p_header->m_Marker, COALESCE_ALLOC_MARKER);

			// check prev header pointer
			VMEM_MEM_CHECK(&p_header->mp_Prev, p_prev_header);
			p_prev_header = p_header;
		}
	}
#endif
}

//------------------------------------------------------------------------
void BasicCoalesceHeap::DebugWrite(Header* p_header) const
{
	int size = p_header->m_Size;
	void* p_start = p_header;
	void* p_end = (byte*)p_header + size;

	VMem::DebugWrite(_T("0x%08x - 0x%08x\t%7d\t%s\n"), p_start, p_end, size, p_header->mp_Prev ? _T("free") : _T("alloc"));
}

//------------------------------------------------------------------------
void BasicCoalesceHeap::DebugWrite() const
{
	for(Region* p_region = mp_RegionList; p_region!=NULL; p_region=p_region->mp_Next)
	{
		VMem::DebugWrite(_T("--------------------------------\n"), p_region);
		VMem::DebugWrite(_T("Region 0x%08x\n"), p_region);

		// write all headers
		Header* p_last_header = (Header*)((byte*)p_region + m_RegionSize);
		Header* p_header = (Header*)(p_region + 1);
		while(p_header != p_last_header)
		{
			DebugWrite(p_header);
			p_header = (Header*)((byte*)p_header + p_header->m_Size);
		}

		// write free list
		VMem::DebugWrite(_T("\nFree list\n"));

		Header* p_list_head = &p_region->m_FreeList;
		for(Header* p_header=p_list_head->mp_Next; p_header!=p_list_head; p_header=p_header->mp_Next)
		{
			DebugWrite(p_header);
		}
	}
}

//------------------------------------------------------------------------
#ifdef VMEM_STATS
void BasicCoalesceHeap::SendStatsToMemPro(MemProSendFn send_fn, void* p_context)
{
	MemProStats::BasicCoalesceHeapStats stats;

	stats.m_RegionCount = 0;
	for(Region* p_region = mp_RegionList; p_region!=NULL; p_region=p_region->mp_Next)
		++stats.m_RegionCount;

	stats.m_RegionSize = m_RegionSize;

	stats.m_Stats = m_Stats;

	SendEnumToMemPro(vmem_BasicCoalesceHeap, send_fn, p_context);
	SendToMemPro(stats, send_fn, p_context);
}
#endif
//------------------------------------------------------------------------
// BasicFSA.cpp


//------------------------------------------------------------------------
// BasicFSA.hpp



//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	struct BasicFSAPage;

	//------------------------------------------------------------------------
	// This class is only designed to be used internally by VMem. It is simple,
	// fast and small, doesn't care about fragmentation, not thread safe, and
	// allocates memory a page at a time. It doesn't rely on any other allocators
	// and gets its memory directly from the system. Aligns to 4 bytes
	class BasicFSA
	{
	public:
		inline BasicFSA(int size);

		void* Alloc();

		void Free(void* p);

#ifdef VMEM_STATS
		inline const Stats& GetStats() const;
#endif

	private:
		BasicFSAPage* CreatePage();

		void UnlinkPage(BasicFSAPage* p_page);

		void DestroyPage(BasicFSAPage* p_page);

		//------------------------------------------------------------------------
		// data
	private:
		int m_Size;							// the size of each allocation
		BasicFSAPage* mp_FreePageList;

#ifdef VMEM_STATS
		Stats m_Stats;
#endif
	};

	//------------------------------------------------------------------------
	BasicFSA::BasicFSA(int size)
	:	m_Size(size),
		mp_FreePageList(NULL)
	{
	}

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	const Stats& BasicFSA::GetStats() const
	{
		return m_Stats;
	}
#endif

	//------------------------------------------------------------------------
	template<typename T>
	class BasicFSA_T
	{
	public:
		BasicFSA_T() : m_FSA(sizeof(T)) {}

		T* Alloc() { return (T*)m_FSA.Alloc(); }

		void Free(void* p) { m_FSA.Free(p); }

#ifdef VMEM_STATS
		const Stats& GetStats() const { return m_FSA.GetStats(); }
#endif
		//------------------------------------------------------------------------
		// data
	private:
		BasicFSA m_FSA;
	};
}



//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	struct BasicFSAPage
	{
		void* mp_FreeList;
		BasicFSAPage* mp_Next;
		int m_FreeSlotCount;
#ifdef VMEM_ASSERTS
		unsigned int m_Marker;
#endif
	};
}

//------------------------------------------------------------------------
void* BasicFSA::Alloc()
{
	// get a page
	BasicFSAPage* p_page = mp_FreePageList;
	if(!p_page)
	{
		p_page = mp_FreePageList = CreatePage();
		if(!p_page)
			return NULL;		// out of memory
	}

	// remove a slot from the free list
	void* p = p_page->mp_FreeList;
	p_page->mp_FreeList = *(void**)p;
	--p_page->m_FreeSlotCount;

	// if the page is full take it off the free page list
	if(!p_page->m_FreeSlotCount)
		mp_FreePageList = mp_FreePageList->mp_Next;

	// update stats
	VSTATS(m_Stats.m_Unused -= m_Size);
	VSTATS(m_Stats.m_Used += m_Size);

	return p;
}

//------------------------------------------------------------------------
void BasicFSA::Free(void* p)
{
	BasicFSAPage* p_page = (BasicFSAPage*)AlignDownPow2(p, SYS_PAGE_SIZE);
	VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_page->m_Marker, FSA_MARKER));

	// put the slot back onto the free list
	*(void**)p = p_page->mp_FreeList;
	p_page->mp_FreeList = p;
	++p_page->m_FreeSlotCount;

	int slots_per_page = (SYS_PAGE_SIZE - sizeof(BasicFSAPage)) / m_Size;
	if(p_page->m_FreeSlotCount == slots_per_page)
	{
		// page empty - destroy it
		UnlinkPage(p_page);
		DestroyPage(p_page);
	}
	else if(p_page->m_FreeSlotCount == 1)
	{
		// page was empty - put it back onto the free list
		p_page->mp_Next = mp_FreePageList;
		mp_FreePageList = p_page;
	}

	// update stats
	VSTATS(m_Stats.m_Unused += m_Size);
	VSTATS(m_Stats.m_Used -= m_Size);
}

//------------------------------------------------------------------------
BasicFSAPage* BasicFSA::CreatePage()
{
	// allocate a page
	BasicFSAPage* p_page = (BasicFSAPage*)VirtualMem::Reserve(SYS_PAGE_SIZE, SYS_PAGE_SIZE);
	if(!p_page)
		return NULL;		// out of memory
	
	if(!VirtualMem::Commit(p_page, SYS_PAGE_SIZE))
		return NULL;		// out of memory

	// setup the free list
	int slots_per_page = (SYS_PAGE_SIZE - sizeof(BasicFSAPage)) / m_Size;
	void* p_first_slot = p_page + 1;
	void* p_slot = p_first_slot;
	void* p_last_slot = (byte*)p_first_slot + (slots_per_page-1) * m_Size;
	while(p_slot != p_last_slot)
	{
		void* p_next_slot = (byte*)p_slot + m_Size;
		*(void**)p_slot = p_next_slot;
		p_slot = p_next_slot;
	}
	*(void**)p_slot = NULL;

	// initialise the page
	p_page->mp_FreeList = p_first_slot;
	p_page->mp_Next = NULL;
	p_page->m_FreeSlotCount = slots_per_page;
	VMEM_ASSERT_CODE(p_page->m_Marker = FSA_MARKER);

	// update stats
	VSTATS(m_Stats.m_Unused += slots_per_page * m_Size);
	VSTATS(m_Stats.m_Overhead += SYS_PAGE_SIZE - slots_per_page * m_Size);
	VSTATS(m_Stats.m_Reserved += SYS_PAGE_SIZE);

	return p_page;
}

//------------------------------------------------------------------------
void BasicFSA::UnlinkPage(BasicFSAPage* p_page)
{
	// find the page
	VMEM_ASSERT(mp_FreePageList, "unable to find page");
	BasicFSAPage* p_srch_page = mp_FreePageList;
	BasicFSAPage* p_prev_page = NULL;
	while(p_srch_page != p_page)
	{
		p_prev_page = p_srch_page;
		p_srch_page = p_srch_page->mp_Next;
		VMEM_ASSERT(p_srch_page, "unable to find page");
	}

	// unlink it
	VMEM_ASSERT(p_page, "unable to find page");
	BasicFSAPage* p_next_page = p_page->mp_Next;
	if(!p_prev_page)
		mp_FreePageList = p_next_page;
	else
		p_prev_page->mp_Next = p_next_page;
}

//------------------------------------------------------------------------
void BasicFSA::DestroyPage(BasicFSAPage* p_page)
{
#if defined(VMEM_ASSERTS) || defined(VMEM_STATS)
	int slots_per_page = (SYS_PAGE_SIZE - sizeof(BasicFSAPage)) / m_Size;
#endif

	// check page is empty
	VMEM_ASSERT_CODE(VMEM_ASSERT(p_page->m_FreeSlotCount == slots_per_page, "Destroying page that is not empty"));

	// release the memory
	VirtualMem::Decommit(p_page, SYS_PAGE_SIZE);
	VirtualMem::Release(p_page);

	// update stats
	VSTATS(m_Stats.m_Unused -= slots_per_page * m_Size);
	VSTATS(m_Stats.m_Overhead -= SYS_PAGE_SIZE - slots_per_page * m_Size);
	VSTATS(m_Stats.m_Reserved -= SYS_PAGE_SIZE);
}
//------------------------------------------------------------------------
// Bitfield.cpp


//------------------------------------------------------------------------
// Bitfield.hpp



//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	class InternalHeap;

	//------------------------------------------------------------------------
	// This bitfield is designed to be very fast at finding the lowest clear bit
	// in the array of bits. It uses a hint index that points to the lowest bit
	// that might be clear. See GetFirstClearBit for more details.
	class Bitfield
	{
	public:
		Bitfield(int size, InternalHeap& internal_heap);

		~Bitfield();

		inline bool Get(int index) const;

		inline void Set(int index);

		inline void Clear(int index);

		inline int GetFirstClearBit();

		inline int GetCount() const;

		inline int GetSizeInBytes() const;

	private:
		Bitfield& operator=(const Bitfield& other);

		//------------------------------------------------------------------------
		// data
	private:
		int m_Size;					// size in bits
		unsigned int* mp_Bits;		// the bit array
		int m_FirstClearBitHint;	// always garanteed to be <= the first clear bit index. Usually the exact index.
		int m_Count;				// number of set bits

		InternalHeap& m_InternalHeap;	// heap that is used to allocate the bit array
	};

	//------------------------------------------------------------------------
	bool Bitfield::Get(int index) const
	{
		VMEM_ASSERT(index >= 0 && index < m_Size, "Bitfield index out of range");
		unsigned int u32_index = index / 32;
		unsigned int bit_index = index & 0x1f;
		return (mp_Bits[u32_index] & (1 << bit_index)) != 0;
	}

	//------------------------------------------------------------------------
	void Bitfield::Set(int index)
	{
		VMEM_ASSERT(index >= 0 && index < m_Size, "Bitfield index out of range");

		unsigned int u32_index = index / 32;
		unsigned int bit_index = index & 0x1f;

		int bit_mask = 1 << bit_index;
		VMEM_ASSERT_MEM((mp_Bits[u32_index] & bit_mask) == 0, &mp_Bits[u32_index]);		// Bitfield bit already set
		mp_Bits[u32_index] |= bit_mask;

		++m_Count;
	}

	//------------------------------------------------------------------------
	void Bitfield::Clear(int index)
	{
		VMEM_ASSERT(index >= 0 && index < m_Size, "Bitfield index out of range");

		unsigned int u32_index = index / 32;
		unsigned int bit_index = index & 0x1f;

		int bit_mask = 1 << bit_index;
		VMEM_ASSERT_MEM((mp_Bits[u32_index] & bit_mask) != 0, &mp_Bits[u32_index]);		// Bitfield bit already clear
		mp_Bits[u32_index] &= ~bit_mask;

		if(index < m_FirstClearBitHint)
			m_FirstClearBitHint = index;

		VMEM_ASSERT_MEM(m_Count, &m_Count);		// can't claer bit, bitfield is empty
		--m_Count;
	}

	//------------------------------------------------------------------------
	int Bitfield::GetCount() const
	{
		return m_Count;
	}

	//------------------------------------------------------------------------
	int Bitfield::GetSizeInBytes() const
	{
		return AlignUpPow2(m_Size, 32) / 8;
	}

	//------------------------------------------------------------------------
	// NOTE: the returned bit must be set after this call to keep m_FirstClearButHint valid.
	// m_FirstClearButHint is incremented by this function to avoid having a conditional in the
	// Set function.
	int Bitfield::GetFirstClearBit()
	{
		VMEM_ASSERT_MEM(m_FirstClearBitHint >= 0, &m_FirstClearBitHint);
		VMEM_ASSERT_MEM(m_FirstClearBitHint < m_Size, &m_FirstClearBitHint);
		VMEM_ASSERT_MEM(m_Count >= 0 && m_Count < m_Size, &m_Count);

		unsigned int u32_index = m_FirstClearBitHint / 32;
		unsigned int bit_index = m_FirstClearBitHint & 0x1f;
		unsigned int* p_u32 = mp_Bits + u32_index;

		int index = m_FirstClearBitHint;

		// if m_FirstClearBitHint is in use look for the next one
		if((*p_u32 & (1 << bit_index)) != 0)
		{
			// look for the next u32 that is not full
#ifdef VMEM_ASSERTS
			unsigned int u32_count = (m_Size + 31) / 32;
#endif
			while(mp_Bits[u32_index] == 0xffffffff)
			{
				++u32_index;
				VMEM_ASSERT_CODE(VMEM_ASSERT_MEM(u32_index < u32_count, &mp_Bits[u32_index]));
			}

			// find the clear bit in the u32
			unsigned int bits = mp_Bits[u32_index];
			unsigned int bit_mask = 1;
			bit_index = 0;
			while(bits & bit_mask)
			{
				bit_mask <<= 1;
				++bit_index;
			}

			index = u32_index * 32 + bit_index;
		}

		// might be free, but at least always guaranteed to be <= the free index
		m_FirstClearBitHint = index + 1;

		return index;
	}

	//------------------------------------------------------------------------
#ifdef VMEM_MEMORY_CORRUPTION_FINDER
	// bitfield that will never re-use a bit once it has been cleared.
	class NoReleaseBitfield
	{
	public:
		NoReleaseBitfield(int size, InternalHeap& internal_heap) : m_Bitfield(size, internal_heap), m_Size(size), m_FirstFreeIndex(0) {}

		bool Get(int index) const { return m_Bitfield.Get(index); }

		void Set(int index) { m_Bitfield.Set(index); if(index+1 > m_FirstFreeIndex) m_FirstFreeIndex = index+1; }

		void Clear(int index) { m_Bitfield.Clear(index); }

		int GetFirstClearBit() { return m_FirstFreeIndex; }

		int GetCount() const { return m_FirstFreeIndex; }

		int GetSizeInBytes() const { return m_Bitfield.GetSizeInBytes(); }

	private:
		NoReleaseBitfield& operator=(const NoReleaseBitfield& other);

		//------------------------------------------------------------------------
		// data
	private:
		Bitfield m_Bitfield;
		int m_Size;
		int m_FirstFreeIndex;
	};
#endif
}

//------------------------------------------------------------------------
// InternalHeap.hpp




//------------------------------------------------------------------------
// VMemCriticalSection.hpp


//------------------------------------------------------------------------
// VMemSys.hpp


#if defined(VMEM_INC_INTEG_CHECK) && defined(VMEM_OS_WIN)
	#include <intrin.h>
#endif

//------------------------------------------------------------------------
namespace VMem
{
	void VMemSysDestroy();

	void* VirtualReserve(size_t size);

	void VirtualRelease(void* p, size_t size);

	bool VirtualCommit(void* p, size_t size);

	void VirtualDecommit(void* p, size_t size);

	size_t GetReservedBytes();

	size_t GetCommittedBytes();

	void Break();

	void DebugWrite(const _TCHAR* p_str, ...);

	void DumpSystemMemory();

	void VMemSysSetLogFunction(void (*LogFn)(const _TCHAR* p_message));

	//-----------------------------------------------------------------------------
#ifdef VMEM_ASSERTS
	bool Committed(void* p, size_t size);
	bool Reserved(void* p, size_t size);
#endif

	//-----------------------------------------------------------------------------
#ifdef VMEM_PLATFORM_APPLE
	#define ALIGN_8 __attribute__((aligned(8)))
#else
	#define ALIGN_8
#endif
	
	//-----------------------------------------------------------------------------
#ifdef VMEM_INC_INTEG_CHECK
	#if defined(VMEM_OS_WIN)
		inline void VMem_InterlockedIncrement(volatile long& value)
		{
			_InterlockedIncrement(&value);
		}
	#elif defined(VMEM_OS_UNIX)
		// can't gurantee platform has interlocked instructions, so use critical section.
		// Doesn't have to be particularly fast because it's only used in debug code
		class IncCritSec
		{
		public:
			IncCritSec()
			{
				pthread_mutexattr_t attr;
				pthread_mutexattr_init(&attr);
				pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
				pthread_mutex_init(&cs, &attr);
			}

			~IncCritSec() { pthread_mutex_destroy(&cs); }

			void Enter() { pthread_mutex_lock(&cs); }

			void Leave() { pthread_mutex_unlock(&cs); }

		private:
			pthread_mutex_t cs;
		};
		IncCritSec g_InterlockedAddCritSec;

		inline void VMem_InterlockedIncrement(volatile long& value)
		{
			g_InterlockedAddCritSec.Enter();
			++value;
			g_InterlockedAddCritSec.Leave();
		}
	#else
		#error platform not defined
	#endif
#endif
}

//-----------------------------------------------------------------------------
#ifdef VMEM_OS_UNIX
	#include <pthread.h>
#endif

//-----------------------------------------------------------------------------
namespace VMem
{
	//-----------------------------------------------------------------------------
	// This is NOT re-entrant. Avoids having to have an extra interlocked increment.
	// Yields after 40 checks.
	class CriticalSection
	{
	public:
		//-----------------------------------------------------------------------------
		CriticalSection()
		{
#if defined(VMEM_OS_WIN)
			InitializeCriticalSection(&m_CritSec);
#elif defined(VMEM_OS_UNIX)
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&cs, &attr);
#else
			#error platform not defined
#endif
		}

		//-----------------------------------------------------------------------------
		~CriticalSection()
		{
#if defined(VMEM_OS_WIN)
			DeleteCriticalSection(&m_CritSec);
#elif defined(VMEM_OS_UNIX)
			pthread_mutex_destroy(&cs);
#else
			#error platform not defined
#endif
		}

		//-----------------------------------------------------------------------------
		void Enter()
		{
#if defined(VMEM_OS_WIN)
			EnterCriticalSection(&m_CritSec);
#elif defined(VMEM_OS_UNIX)
			pthread_mutex_lock(&cs);
#else
			#error platform not defined
#endif
		}

		//-----------------------------------------------------------------------------
		void Leave()
		{
#if defined(VMEM_OS_WIN)
			LeaveCriticalSection(&m_CritSec);
#elif defined(VMEM_OS_UNIX)
			pthread_mutex_unlock(&cs);
#else
			#error platform not defined
#endif
		}

		//-----------------------------------------------------------------------------
		// data
	private:
#if defined(VMEM_OS_WIN)
		CRITICAL_SECTION m_CritSec;
#elif defined(VMEM_OS_UNIX)
		pthread_mutex_t cs;
#else
		#error platform not defined
#endif
	};

	//-----------------------------------------------------------------------------
	class CriticalSectionScope
	{
	public:
		CriticalSectionScope(CriticalSection& critical_section) : m_CriticalSection(critical_section) { critical_section.Enter(); }
		~CriticalSectionScope() { m_CriticalSection.Leave(); }

	private:
		CriticalSectionScope& operator=(const CriticalSectionScope& other);

		CriticalSection& m_CriticalSection;
	};
}

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	class InternalHeap
	{
	public:
		InternalHeap();

		inline void* Alloc(int size);

		inline void Free(void* p);

#ifdef VMEM_STATS
		inline const Stats& GetStats() const;
		inline const Stats& GetStatsNoLock() const;
		inline void SendStatsToMemPro(MemProSendFn send_fn, void* p_context);
#endif

		template<typename T>
		T* Alloc()
		{
			T* p = (T*)Alloc(sizeof(T));
			new (p)T();
			return p;
		}

		template<typename T>
		T* New()
		{
			T* p = (T*)Alloc(sizeof(T));
			new (p)T;
			return p;
		}

		template<typename T, typename TArg1>
		T* New(const TArg1& arg1)
		{
			T* p = (T*)Alloc(sizeof(T));
			new (p)T(arg1);
			return p;
		}

		template<typename T, typename TArg1, typename TArg2>
		T* New(const TArg1& arg1, TArg2& arg2)
		{
			T* p = (T*)Alloc(sizeof(T));
			new (p)T(arg1, arg2);
			return p;
		}

		template<typename T, typename TArg1, typename TArg2>
		T* New(const TArg1& arg1, const TArg2& arg2)
		{
			T* p = (T*)Alloc(sizeof(T));
			new (p)T(arg1, arg2);
			return p;
		}

		template<typename T, typename TArg1, typename TArg2, typename TArg3>
		T* New(const TArg1& arg1, const TArg2& arg2, const TArg3& arg3)
		{
			T* p = (T*)Alloc(sizeof(T));
			new (p)T(arg1, arg2, arg3);
			return p;
		}

		template<typename T>
		void Delete(T* p)
		{
			p->~T();
			Free(p);
		}

		//------------------------------------------------------------------------
		// data
	private:
		mutable CriticalSection m_CriticalSection;

		BasicCoalesceHeap m_BasicCoalesceHeap;
	};

	//------------------------------------------------------------------------
	void* InternalHeap::Alloc(int size)
	{
		CriticalSectionScope lock(m_CriticalSection);
		return m_BasicCoalesceHeap.Alloc(size);
	}

	//------------------------------------------------------------------------
	void InternalHeap::Free(void* p)
	{
		CriticalSectionScope lock(m_CriticalSection);
		m_BasicCoalesceHeap.Free(p);
	}

	//------------------------------------------------------------------------
	#ifdef VMEM_STATS
	const Stats& InternalHeap::GetStats() const
	{
		CriticalSectionScope lock(m_CriticalSection);
		return m_BasicCoalesceHeap.GetStats();
	}
	#endif

	//------------------------------------------------------------------------
	#ifdef VMEM_STATS
	const Stats& InternalHeap::GetStatsNoLock() const
	{
		return m_BasicCoalesceHeap.GetStats();
	}
	#endif

	//------------------------------------------------------------------------
	#ifdef VMEM_STATS
	void InternalHeap::SendStatsToMemPro(MemProSendFn send_fn, void* p_context)
	{
		m_BasicCoalesceHeap.SendStatsToMemPro(send_fn, p_context);
	}
	#endif
}
#include <memory.h>

//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
Bitfield::Bitfield(int size, InternalHeap& internal_heap)
:	m_Size(size),
	m_FirstClearBitHint(0),
	m_Count(0),
	m_InternalHeap(internal_heap)
{
	VMEM_ASSERT(size, "size must > 0");

	int size_in_bytes = GetSizeInBytes();

	mp_Bits = (unsigned int*)internal_heap.Alloc(size_in_bytes);
	memset(mp_Bits, 0, size_in_bytes);

	// if the size is not a multiple of 32 set all trailing bits to 1
	int left_over = size & 31;
	if(left_over)
	{
		int size_in_uints = size_in_bytes / sizeof(unsigned int);
		unsigned int& last_uint = mp_Bits[size_in_uints - 1];
		for(int i=left_over; i<32; ++i)
			last_uint |= (1 << i);
	}
}

//------------------------------------------------------------------------
Bitfield::~Bitfield()
{
	m_InternalHeap.Free(mp_Bits);
}
//------------------------------------------------------------------------
// CoalesceHeap.cpp


//------------------------------------------------------------------------
// CoalesceHeap.hpp




//------------------------------------------------------------------------
// TrailGuard.hpp



//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	class TrailGuard
	{
	public:
		TrailGuard();

		void Initialise(int size, int check_freq, int alloc_size=-1);

		void* Add(void* p) { return Add(p, m_FixedAllocSize); }

		void* Add(void* p, int size);

		int GetSize() const { return m_Size; }

		void* Shutdown();

		bool Contains(void* p) const;

		void CheckIntegrity() const;

		//------------------------------------------------------------------------
		// data
	private:
		int m_Size;
		int m_CurSize;
		int m_FixedAllocSize;

		void* mp_Head;
		void* mp_Tail;

		int m_FullCheckFreq;
		int m_FullCheckCounter;
	};
}


//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	namespace Coalesce
	{
		struct Region;
		struct Header;
		struct Node;
	}

	//------------------------------------------------------------------------
	// This coalesce heap used best fit and immediate coalesceing. It also decommits
	// free pages in and free blocks, and performs biasing to reduce fragmentation.
	// Unlike many coalesce heaps the free nodes are not embedded in the memory blocks
	// but are seperate allocations to increase cache coherency when looking up and sorting.
	// This allocator aligns to 16 bytes.
	class CoalesceHeap
	{
	public:
		CoalesceHeap(int region_size, int min_size, int max_size);

		~CoalesceHeap();

		void* Alloc(size_t size);

		bool Free(void* p);

		size_t GetSize(void* p) const;

		void CheckIntegrity() const;

		void DebugWrite() const;

#ifdef VMEM_STATS
		inline Stats GetStats() const;
		inline Stats GetStatsNoLock() const;
		size_t WriteAllocs() const;
		void SendStatsToMemPro(MemProSendFn send_fn, void* p_context);
#endif

#ifdef VMEM_TRAIL_GUARDS
		inline void InitialiseTrailGuard(int size, int check_freq);
#endif

		void Lock() const {}

		void Release() const {}

	private:
		Coalesce::Region* CreateRegion();

		void ClearRegion(Coalesce::Region* p_region);

		void DestroyRegion(Coalesce::Region* p_region);

		inline Coalesce::Node* AllocNode();

		inline void FreeNode(Coalesce::Node* p_node);

		Coalesce::Node* GetNode(int size, Coalesce::Region*& p_region);

		void InsertNode(Coalesce::Node* p_node, Coalesce::Region* p_region);

		void RemoveNode(Coalesce::Node* p_node);

		inline int GetFreeListIndex(int size) const;

		Coalesce::Region* GetRegion(void* p) const;

		inline bool CommitRange(void* p, int size);

		inline void DecommitRange(void* p, int size);

		inline Coalesce::Header* FindNextAllocatedHeader(Coalesce::Region* p_region, Coalesce::Header* p_header) const;

		void CheckIntegrity(Coalesce::Header* p_header) const;

#ifdef VMEM_COALESCE_GUARD_PAGES
		bool SetupGuardPages(Coalesce::Region* p_region);
		void RemoveGuardPages(Coalesce::Region* p_region);
#endif

		//------------------------------------------------------------------------
		// data
	private:
		int m_RegionSize;

		// the minimum and maximum size that this coalesce heap can allocate. This is
		// used to skew the freelist map indices, deciding what to do with offcuts and
		// asserting the allocation sizes are in range.
		int m_MinSize;
		int m_MaxSize;

		Coalesce::Region* mp_RegionList;		// linked list of regions

		BasicFSA_T<Coalesce::Node> m_NodeFSA;	// the allocator to use to allocate nodes

#ifdef VMEM_STATS
		Stats m_Stats;
#endif

#ifdef VMEM_TRAIL_GUARDS
		TrailGuard m_TrailGuard;
#endif
	};

	//------------------------------------------------------------------------
	// thread safe wrapper for CoalesceHeap
	class CoalesceHeapMT
	{
	public:
		inline CoalesceHeapMT(int region_size, int min_size, int max_size);

		inline void* Alloc(size_t size);

		inline bool Free(void* p);

		inline size_t GetSize(void* p) const;

		inline void CheckIntegrity() const;

		inline void DebugWrite() const;

#ifdef VMEM_STATS
		inline Stats GetStats() const;
		inline Stats GetStatsNoLock() const;
		inline size_t WriteAllocs() const;
		inline void SendStatsToMemPro(MemProSendFn send_fn, void* p_context);
#endif

#ifdef VMEM_TRAIL_GUARDS
		inline void InitialiseTrailGuard(int size, int check_freq);
#endif
		inline void Lock() const;

		inline void Release() const;

		//------------------------------------------------------------------------
		// data
	private:
		mutable CriticalSection m_CriticalSection;
		CoalesceHeap m_CoalesceHeap;
	};

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	Stats CoalesceHeap::GetStats() const
	{
		Stats fsa_stats = m_NodeFSA.GetStats();
		fsa_stats.m_Used = 0;	// FSA used stat is counted in m_Stats

		return m_Stats + fsa_stats;
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	Stats CoalesceHeap::GetStatsNoLock() const
	{
		return GetStats();
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_TRAIL_GUARDS
	void CoalesceHeap::InitialiseTrailGuard(int size, int check_freq)
	{
		m_TrailGuard.Initialise(size, check_freq);
	}
#endif

	//------------------------------------------------------------------------
	CoalesceHeapMT::CoalesceHeapMT(int region_size, int min_size, int max_size)
	:	m_CoalesceHeap(region_size, min_size, max_size)
	{
	}

	//------------------------------------------------------------------------
	void* CoalesceHeapMT::Alloc(size_t size)
	{
		CriticalSectionScope lock(m_CriticalSection);
		return m_CoalesceHeap.Alloc(size);
	}

	//------------------------------------------------------------------------
	bool CoalesceHeapMT::Free(void* p)
	{
		CriticalSectionScope lock(m_CriticalSection);
		return m_CoalesceHeap.Free(p);
	}

	//------------------------------------------------------------------------
	size_t CoalesceHeapMT::GetSize(void* p) const
	{
		CriticalSectionScope lock(m_CriticalSection);
		return m_CoalesceHeap.GetSize(p);
	}

	//------------------------------------------------------------------------
	void CoalesceHeapMT::CheckIntegrity() const
	{
		CriticalSectionScope lock(m_CriticalSection);
		m_CoalesceHeap.CheckIntegrity();
	}

	//------------------------------------------------------------------------
	void CoalesceHeapMT::DebugWrite() const
	{
		CriticalSectionScope lock(m_CriticalSection);
		m_CoalesceHeap.DebugWrite();
	}

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	Stats CoalesceHeapMT::GetStats() const
	{
		CriticalSectionScope lock(m_CriticalSection);
		return m_CoalesceHeap.GetStats();
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	Stats CoalesceHeapMT::GetStatsNoLock() const
	{
		return m_CoalesceHeap.GetStatsNoLock();
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	size_t CoalesceHeapMT::WriteAllocs() const
	{
		CriticalSectionScope lock(m_CriticalSection);
		return m_CoalesceHeap.WriteAllocs();
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	void CoalesceHeapMT::SendStatsToMemPro(MemProSendFn send_fn, void* p_context)
	{
		CriticalSectionScope lock(m_CriticalSection);
		m_CoalesceHeap.SendStatsToMemPro(send_fn, p_context);
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_TRAIL_GUARDS
	void CoalesceHeapMT::InitialiseTrailGuard(int size, int check_freq)
	{
		CriticalSectionScope lock(m_CriticalSection);
		m_CoalesceHeap.InitialiseTrailGuard(size, check_freq);
	}
#endif

	//------------------------------------------------------------------------
	void CoalesceHeapMT::Lock() const
	{
		m_CriticalSection.Enter();
	}

	//------------------------------------------------------------------------
	void CoalesceHeapMT::Release() const
	{
		m_CriticalSection.Leave();
	}

}



//------------------------------------------------------------------------
#ifdef VMEM_OS_WIN
	#pragma warning(push)
	#pragma warning(disable : 4100)
#endif

//------------------------------------------------------------------------
using namespace VMem;
using namespace Coalesce;

//------------------------------------------------------------------------
#ifdef COALESCE_HEAP_MARKER
	#define VMEM_ASSERT_COALESCE_MARKER(p_value) VMEM_MEM_CHECK(p_value, COALESCE_ALLOC_MARKER)
#else
	#define VMEM_ASSERT_COALESCE_MARKER(p_value)
#endif

//------------------------------------------------------------------------
//
// The free list map.
// -------------------
// Each free block of memory has a node. These nodes are stored in the free list map.
// 
// This is a simple example of a free node map. It has a min and max size of
// 0 and 50, and there are 6 buckets (in reality there are 256 buckets and they
// are skewed).
// There is an array of 6 lists L, each contain nodes in a size range.
// Each list L contains a linked list of nodes n.
// The nodes in each list are sorted by size and then by address, in an ascending order.
//
// to find a node for a specific size, it first works out the first list L that
// could contain such a node. It then iterates down the list to find a node that is
// big enough. If no such node is found it moves onto the next non-empty list
// and uses the first node.
//
//   L0(0-10)  L1(10-20)  L2(20-30)  L3(30-40)  L4(40-50)  L5(>50)
//       |          |          |          |                   |
//       n2         n14        n20        n33                 n55
//       |                     |                              |
//       n6                    n27                            n200
//       |
//       n10
//

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	const int g_FreeNodeMapSize = 255;
	const int g_CoalesceHeapAlignment = 16;

	//------------------------------------------------------------------------
	// The node pointer is aliased with this enum, so the enum values must not be
	// valid pointers. The order is important, fragment must come after allocated
	// so that we can do > allocated to test if the memory is free.
	enum ENodeType
	{
		nt_Fixed = 1,			// fixed memory block at start or end of memory that is never freed
		nt_Allocated = 2,		// allocated memory
		nt_Fragment = 3			// offcut that is too small to be re-used and so doesn't need a free node
								// > than nt_Fragment means that it is a pointer to a free node. We are
								// relying on the fact that 1,2,3 are invalid pointers.
	};

	//------------------------------------------------------------------------
	namespace Coalesce
	{
		//------------------------------------------------------------------------
		const size_t g_MaxCheckMemorySize = 128;

		//------------------------------------------------------------------------
		// each free block points to a node. Allocated blocks do not have nodes.
		// Instead of embedding this data in the memory block we store it as a
		// seperate object to allow for fast cache friendly iteration when sorting
		// and looking up nodes.
		struct Node
		{
			void* mp_Mem;
			Node* mp_Prev;
			Node* mp_Next;
			int m_Size;
			VMEM64_ONLY(unsigned int m_Padding);
		};

		//------------------------------------------------------------------------
		// A reserved range of memory from which we commit memory for the allocations
		struct Region
		{
			Node mp_FreeNodeLists[g_FreeNodeMapSize];	// essentially a map from alloc size to free list
			Region* mp_Next;
#ifdef VMEM64
			unsigned int m_Marker[2];		// pack to maintain alignment
#else
			unsigned int m_Marker[3];		// pack to maintain alignment
#endif
		};

		//------------------------------------------------------------------------
		// embedded before the start of the allocation. The size and prev size are
		// needed so that we can coalesce previous and next allocation when deallocating.
		struct Header
		{
#ifdef COALESCE_HEAP_MARKER
			unsigned int m_Marker;
			VMEM64_ONLY(unsigned int m_Padding[3]);		// unused
#endif

			int m_Size;					// includes the size of the header
			int m_PrevSize;				// size of the previous sequential block in memory

			// pointer to a node or one of the node flag defines
			union
			{
				size_t m_NodeType;		// ENodeType - use size_t to ensure sizeof(m_NodeType) == sizeof(Node*)
				Node* mp_Node;				
			};
		};

		//------------------------------------------------------------------------
		// check alignment
		VMEM_STATIC_ASSERT((sizeof(Header) & (g_CoalesceHeapAlignment-1)) == 0);
		VMEM_STATIC_ASSERT((sizeof(Region) & (g_CoalesceHeapAlignment-1)) == 0);
		VMEM_STATIC_ASSERT((COALESCE_GUARD_SIZE & (g_CoalesceHeapAlignment-1)) == 0);

		//------------------------------------------------------------------------
		const int g_RegionOverhead = sizeof(Region) + 2*sizeof(Header);

		//------------------------------------------------------------------------
		inline void SetupHeader(Header* p_header)
		{
#ifdef COALESCE_HEAP_MARKER
			p_header->m_Marker = COALESCE_ALLOC_MARKER;
			VMEM64_ONLY(p_header->m_Padding[0] = p_header->m_Padding[1] = p_header->m_Padding[2] = 0);
#endif
		}
	}
}

//------------------------------------------------------------------------
// The region size will be reserved, but not necessarily committed.
// All allocation sizes requested must be in this min/max range.
CoalesceHeap::CoalesceHeap(int region_size, int min_size, int max_size)
:	m_RegionSize(AlignUpPow2(region_size, SYS_PAGE_SIZE)),
	m_MinSize(min_size),
	m_MaxSize(max_size),
	mp_RegionList(NULL)
{
	VMEM_ASSERT(max_size + (int)sizeof(Coalesce::Header) <= m_RegionSize - g_RegionOverhead, "region size not big enough to hold dmax alloc (+overhead)");
}

//------------------------------------------------------------------------
CoalesceHeap::~CoalesceHeap()
{
#ifdef VMEM_TRAIL_GUARDS
	// free the trail allocs
	void* p_trail_alloc = m_TrailGuard.Shutdown();
	while(p_trail_alloc)
	{
		void* p_next = *(void**)p_trail_alloc;
		Free(p_trail_alloc);
		p_trail_alloc = p_next;
	}
#endif

	// ideally the region will be empty when the coalesce heap is destructed
	// but we will clean up properly anyway. We can't just release all the
	// virtual memory otherwise the committed pages count will be wrong and
	// the free nodes won't be deleted. So we go through all allocations and 
	// delete them using the normal method.
	while(mp_RegionList)
		ClearRegion(mp_RegionList);
}

//------------------------------------------------------------------------
void* CoalesceHeap::Alloc(size_t size)
{
	int i_size = ToInt(size);

	VMEM_ASSERT(i_size >= m_MinSize && i_size <= m_MaxSize, "size out of range for this coalesce heap");

	// work out the actual size of the memory block
	int aligned_size = AlignUpPow2(i_size, g_CoalesceHeapAlignment);
	int total_size = sizeof(Coalesce::Header) + aligned_size;
#ifdef VMEM_COALESCE_GUARDS
	total_size += 2*COALESCE_GUARD_SIZE;
#endif

	// find a node that will fit this allocation size
	Coalesce::Region* p_region = NULL;
	Coalesce::Node* p_node = GetNode(total_size, p_region);	// this also removes the node
	if(!p_node)
		return NULL;	// this means out of memory

	void* p_mem = p_node->mp_Mem;

	// work out the offcut size
	int offcut_size = p_node->m_Size - total_size;
	VMEM_ASSERT(offcut_size >= 0, "node returned from GetNode is not big enough");

	// if the offcut size is too small ignore it and add it on to the alloc
	if(offcut_size < (int)sizeof(Coalesce::Header))
	{
		total_size += offcut_size;
		offcut_size = 0;
	}

	// work out the range to commit
	void* p_commit_start = (byte*)p_node->mp_Mem + sizeof(Coalesce::Header);
	int commit_size = total_size - sizeof(Coalesce::Header);

	// setup the header for this allocation
	Coalesce::Header* p_header = (Coalesce::Header*)p_mem;
	Coalesce::Header* p_next_header = (Coalesce::Header*)((byte*)p_header + p_header->m_Size);
	VMEM_ASSERT_MEM(p_header->m_NodeType > nt_Fragment, &p_header->m_NodeType);		// GetNode returned an allocated node?
	VMEM_ASSERT_COALESCE_MARKER(&p_header->m_Marker);
	p_header->m_NodeType = nt_Allocated;
	p_header->m_Size = total_size;

	// deal with the offcut
	if(offcut_size)
	{
		// get the pointer to the end of the range to commit. The end pointer is exclusive.
		// +sizeof(Coalesce::Header) because we always need to commit the new header for the new offcut
		void* p_commit_end = (byte*)p_commit_start + commit_size + sizeof(Coalesce::Header);

		// Commit the page for the offcut header.
		// If the next block header is in the same page as the offcut then it will
		// already be committed, otherwise we need to commit it here
		void* p_last_page = AlignUpPow2(p_commit_end, SYS_PAGE_SIZE);
		VMEM_ASSERT_COALESCE_MARKER(&p_next_header->m_Marker);
		if(p_last_page <= p_next_header)
			p_commit_end = p_last_page;

		// need to commit the memory before writing to the new offcut header
		int offcut_commit_size = ToInt((byte*)p_commit_end - (byte*)p_commit_start);
		if(!CommitRange(p_commit_start, offcut_commit_size))
			return NULL;

		// set the offcut header
		void* p_offcut_mem = (byte*)p_mem + total_size;
		Coalesce::Header* p_offcut_header = (Coalesce::Header*)p_offcut_mem;
		SetupHeader(p_offcut_header);
		p_offcut_header->m_Size = offcut_size;
		p_offcut_header->m_PrevSize = total_size;

		p_next_header->m_PrevSize = offcut_size;

		if(offcut_size < m_MinSize)
		{
			p_offcut_header->m_NodeType = nt_Fragment;
			FreeNode(p_node);
		}
		else
		{
			// re-use the free node for the offcut
			p_offcut_header->mp_Node = p_node;

			// update the offcut node
			p_node->mp_Mem = p_offcut_mem;
			p_node->m_Size = offcut_size;

			// reinsert the offcut node
			InsertNode(p_node, p_region);
		}

		#ifdef VMEM_MEMSET_ONLY_SMALL
			if(offcut_size > sizeof(Coalesce::Header))
			{
				void* p_offcut_memset_mem = (byte*)p_offcut_mem + sizeof(Coalesce::Header);
				int memset_size = offcut_size - sizeof(Coalesce::Header);

				// only commit up to the end of the page if the next page is not committed
				void* p_start_page = AlignDownPow2((byte*)p_offcut_memset_mem-1, SYS_PAGE_SIZE);
				void* p_end_page = AlignDownPow2((byte*)p_offcut_mem+offcut_size, SYS_PAGE_SIZE);
				if((byte*)p_end_page - (byte*)p_start_page > SYS_PAGE_SIZE)
					memset_size = (byte*)p_start_page + SYS_PAGE_SIZE - (byte*)p_offcut_memset_mem;

				if(memset_size)
					VMEMSET(p_offcut_memset_mem, FREED_MEM, memset_size);
			}
		#endif

		VSTATS(m_Stats.m_Overhead += sizeof(Coalesce::Header));
		VSTATS(m_Stats.m_Unused -= sizeof(Coalesce::Header));
	}
	else
	{
		FreeNode(p_node);

		p_next_header->m_PrevSize = total_size;

		if(!CommitRange(p_commit_start, commit_size))
			return NULL;		// out of memory!
	}

	void* p = p_header + 1;
	CheckMemory(p, total_size-sizeof(Coalesce::Header), FREED_MEM);
	VMEMSET(p, ALLOCATED_MEM, total_size-sizeof(Coalesce::Header));

#ifdef VMEM_COALESCE_GUARDS
	SetGuards(p, COALESCE_GUARD_SIZE);
	SetGuards((byte*)p_header + total_size - COALESCE_GUARD_SIZE, COALESCE_GUARD_SIZE);
	p = (byte*)p + COALESCE_GUARD_SIZE;
#endif

#ifdef VMEM_STATS
	int allocated_size = p_header->m_Size - sizeof(Coalesce::Header);
#ifdef VMEM_COALESCE_GUARDS
	allocated_size -= 2*COALESCE_GUARD_SIZE;
#endif
	VSTATS(m_Stats.m_Used += allocated_size);
	VSTATS(m_Stats.m_Unused -= allocated_size);
#endif

	return p;
}

//------------------------------------------------------------------------
bool CoalesceHeap::Free(void* p)
{
	// find the region that owns this alloc
	Coalesce::Region* p_region = GetRegion(p);
	if(!p_region)
		return false;		// not allocated by this allocator

#ifdef VMEM_COALESCE_GUARDS
	p = (byte*)p - COALESCE_GUARD_SIZE;
#endif

	// get the header for this alloc
	Coalesce::Header* p_header = (Coalesce::Header*)p - 1;
	VMEM_ASSERT_COALESCE_MARKER(&p_header->m_Marker);
	int size = p_header->m_Size;

#ifdef VMEM_TRAIL_GUARDS
	if(m_TrailGuard.GetSize())
	{
		// push the alloc onto the tail list and pop another alloc off the end
		#ifdef VMEM_COALESCE_GUARDS
			p = (byte*)p + COALESCE_GUARD_SIZE;
			size -= 2*COALESCE_GUARD_SIZE;
		#endif
		size -= sizeof(Coalesce::Header);
		
		// move stats from used to overhead
		VSTATS(m_Stats.m_Used -= size);
		VSTATS(m_Stats.m_Overhead += size);

		// put the alloc onto the trail, and pop another alloc to free
		p = m_TrailGuard.Add(p, size);
		if(!p) return true;					// return here if TrailGuard didn't return an alloc to free

		// get the details of the new alloc to free
		#ifdef VMEM_COALESCE_GUARDS
			p = (byte*)p - COALESCE_GUARD_SIZE;
		#endif
		p_header = (Coalesce::Header*)p - 1;
		size = p_header->m_Size;

		// move stats back from overhead to used
		int stats_size = size - sizeof(Coalesce::Header);
		#ifdef VMEM_COALESCE_GUARDS
			stats_size -= 2*COALESCE_GUARD_SIZE;
		#endif
		VSTATS(m_Stats.m_Used += stats_size);
		VSTATS(m_Stats.m_Overhead -= stats_size);

		// the region could have changed
		p_region = GetRegion(p);
		VMEM_ASSERT(p_region, "unable to find region");
	}
#endif

#ifdef VMEM_COALESCE_GUARDS
	CheckMemory(p, COALESCE_GUARD_SIZE, GUARD_MEM);
	CheckMemory((byte*)p_header + size - COALESCE_GUARD_SIZE, COALESCE_GUARD_SIZE, GUARD_MEM);
#endif

	VMEMSET(p, FREED_MEM, size-sizeof(Coalesce::Header));

	// get the range of memory we can definitely try and decommit. The end pointer is exclusive.
	void* p_decommit_start = p;
	void* p_decommit_end = (byte*)p + size - sizeof(Coalesce::Header);

	// get the prev and next headers
	Coalesce::Header* p_prev_header = (Coalesce::Header*)((byte*)p_header - p_header->m_PrevSize);
	Coalesce::Header* p_next_header = (Coalesce::Header*)((byte*)p_header + size);

	VMEM_ASSERT_COALESCE_MARKER(&p_prev_header->m_Marker);
	VMEM_ASSERT_COALESCE_MARKER(&p_next_header->m_Marker);

	int merged_size = size;

	// see if we can merge with the prev block (a valid node pointer means it's free)
	Coalesce::Node* p_new_node = NULL;
	size_t prev_node_type = p_prev_header->m_NodeType;
	if(prev_node_type > nt_Allocated)	// if block is free
	{
		CheckIntegrity(p_prev_header);

		merged_size += p_prev_header->m_Size;

		Coalesce::Node* p_prev_node = p_prev_header->mp_Node;			// take the node pointer before we memset it

		VMEMSET(p_header, FREED_MEM, sizeof(Coalesce::Header));

		p_header = p_prev_header;

		p_next_header->m_PrevSize = merged_size;

		// if it's not a fragment update the free node
		if(prev_node_type != nt_Fragment)
		{
			RemoveNode(p_prev_node);
			p_new_node = p_prev_node;
		}

		// we can decommit the original header if we are merging with the prev block
		p_decommit_start = (byte*)p_decommit_start - sizeof(Coalesce::Header);

		// we can also decommit the current page if it doesn't contain the prev block header
		void* p_cur_page = AlignDownPow2(p_decommit_start, SYS_PAGE_SIZE);
		if(p_cur_page >= p_header+1)
			p_decommit_start = p_cur_page;

		VSTATS(m_Stats.m_Overhead -= sizeof(Coalesce::Header));
		VSTATS(m_Stats.m_Unused += sizeof(Coalesce::Header));
	}

	// see if we can merge with the next alloc
	size_t next_node_type = p_next_header->m_NodeType;
	if(next_node_type > nt_Allocated)	// if block is free
	{
		CheckIntegrity(p_next_header);

		merged_size += p_next_header->m_Size;

		Coalesce::Node* p_next_node = p_next_header->mp_Node;			// take the node pointer before we memset it

		VMEMSET(p_next_header, FREED_MEM, sizeof(Coalesce::Header));

		// see if we can re-use the next node
		if(next_node_type != nt_Fragment)
		{
			RemoveNode(p_next_node);

			if(p_new_node)
			{
				// we already have a node, so free the next node
				FreeNode(p_next_node);
			}
			else
			{
				// don't have a node so re-use the next node
				p_new_node = p_next_node;
				p_new_node->mp_Mem = p_header;
			}
		}

		// update the next header prev size
		Coalesce::Header* p_next_next_header = (Coalesce::Header*)((byte*)p_header + merged_size);
		p_next_next_header->m_PrevSize = merged_size;

		// we can decommit the next header
		p_decommit_end = (byte*)p_decommit_end + sizeof(Coalesce::Header);

		// we can also decommit the page that the next header is on if it doesn't contain the next next header
		void* p_next_page = AlignUpPow2(p_decommit_end, SYS_PAGE_SIZE);
		if(p_next_page <= p_next_next_header)
			p_decommit_end = p_next_page;

		VSTATS(m_Stats.m_Overhead -= sizeof(Coalesce::Header));
		VSTATS(m_Stats.m_Unused += sizeof(Coalesce::Header));
	}

	p_header->m_Size = merged_size;

	// didn't merge, so create a new free node
	if(!p_new_node)
	{
		p_new_node = AllocNode();
		p_new_node->mp_Mem = p_header;
	}

	// setup the new node
	p_new_node->m_Size = merged_size;
	p_new_node->mp_Next = NULL;
	p_new_node->mp_Prev = NULL;
	VMEM64_ONLY(p_new_node->m_Padding = 0);
	p_header->mp_Node = p_new_node;

	// (re)insert the new free node
	InsertNode(p_new_node, p_region);

#ifdef VMEM_STATS
	int alloc_size = size - sizeof(Coalesce::Header);
	#ifdef VMEM_COALESCE_GUARDS
		alloc_size -= 2*COALESCE_GUARD_SIZE;
	#endif
#endif
	VSTATS(m_Stats.m_Used -= alloc_size);
	VSTATS(m_Stats.m_Unused += alloc_size);

	// decommit the range
	int range_size = ToInt((byte*)p_decommit_end - (byte*)p_decommit_start);
	DecommitRange(p_decommit_start, range_size);

	// destroy region if empty
	int total_free_size = m_RegionSize - sizeof(Coalesce::Region) - 2*sizeof(Coalesce::Header);
	if(merged_size == total_free_size)
		DestroyRegion(p_region);

	return true;
}

//------------------------------------------------------------------------
size_t CoalesceHeap::GetSize(void* p) const
{
	// find the region that owns this alloc
	Coalesce::Region* p_region = GetRegion(p);
	if(!p_region)
		return VMEM_INVALID_SIZE;		// not allocated by this allocator

#ifdef VMEM_COALESCE_GUARDS
	p = (byte*)p - COALESCE_GUARD_SIZE;
#endif

	// get the header for this alloc
	Coalesce::Header* p_header = (Coalesce::Header*)p - 1;
	int size = p_header->m_Size - sizeof(Coalesce::Header);

#ifdef VMEM_COALESCE_GUARDS
	size -= 2*COALESCE_GUARD_SIZE;
#endif

	return size;
}

//------------------------------------------------------------------------
Coalesce::Region* CoalesceHeap::CreateRegion()
{
	// reserve region memory
	void* p_region_mem = VirtualMem::Reserve(m_RegionSize, SYS_PAGE_SIZE);
	if(!p_region_mem)
		return NULL;		// out of virtual memory!

	// we have a fixed header at the start and end of the region that are never
	// allocated or freed. This makes the logic for coalescing simpler because
	// we can always assume there is a prev and next header.

	// commit the first page to store the start fixed alloc header and the free node header
	int offset = sizeof(Coalesce::Region) + 2*sizeof(Coalesce::Header);
	int start_commit_size = AlignUpPow2(offset, SYS_PAGE_SIZE);
	if(!VirtualMem::Commit(p_region_mem, start_commit_size))
		return NULL;		// out of memory
	VMEMSET((byte*)p_region_mem + offset, FREED_MEM, start_commit_size - offset);
	VSTATS(m_Stats.m_Unused += start_commit_size);

	// commit the last page to store the end fixed alloc
	int end_commit_size = AlignUpPow2(sizeof(Coalesce::Header), SYS_PAGE_SIZE);
	void* p_last_page = (byte*)p_region_mem + m_RegionSize - end_commit_size;
	if(p_last_page >= (byte*)p_region_mem + start_commit_size)
	{
		if(!VirtualMem::Commit(p_last_page, end_commit_size))
			return NULL;		// out of memory		
		VSTATS(m_Stats.m_Unused += end_commit_size);
		VMEMSET(p_last_page, FREED_MEM, end_commit_size);
	}

	// allocate region object at the start of the region memory
	Coalesce::Region* p_region = (Coalesce::Region*)p_region_mem;
	void* p_mem = (byte*)p_region_mem + sizeof(Coalesce::Region);

	// clear free node list head nodes
	Coalesce::Node* p_free_node_lists = p_region->mp_FreeNodeLists;
	memset(p_free_node_lists, 0, sizeof(p_region->mp_FreeNodeLists));

	// the free lists are circular so setup the prev and next pointers for the list heads.
	for(int i=0; i<g_FreeNodeMapSize; ++i)
	{
		Coalesce::Node& node_list = p_free_node_lists[i];
		node_list.mp_Prev = &node_list;
		node_list.mp_Next = &node_list;
	}

	// we need to put a fixed allocation at the start and end of the memory so that
	// we dont have to check if we are the first or last allocation when coalescing.

	// setup the fixed start alloc
	Coalesce::Header* p_start_header = (Coalesce::Header*)p_mem;
	SetupHeader(p_start_header);
	p_start_header->m_Size = sizeof(Coalesce::Header);
	p_start_header->m_PrevSize = 0;
	p_start_header->m_NodeType = nt_Allocated;

	// setup the main free node
	Coalesce::Header* p_header = p_start_header + 1;
	int total_free_size = m_RegionSize - g_RegionOverhead;
	SetupHeader(p_header);
	p_header->m_Size = total_free_size;
	p_header->m_PrevSize = sizeof(Coalesce::Header);

	// setup the fixed end alloc
	Coalesce::Header* p_end_header = (Coalesce::Header*)((byte*)p_header + total_free_size);
	SetupHeader(p_end_header);
	p_end_header->m_Size = sizeof(Coalesce::Header);
	p_end_header->m_PrevSize = total_free_size;
	p_end_header->m_NodeType = nt_Allocated;

	// add a node for the entire free memory
	Coalesce::Node* p_node = AllocNode();
	p_node->mp_Mem = p_header;
	p_node->m_Size = total_free_size;
	p_header->mp_Node = p_node;

	// insert the node into the last free node list
	Coalesce::Node& node_list_head = p_free_node_lists[g_FreeNodeMapSize-1];
	node_list_head.mp_Next = p_node;
	node_list_head.mp_Prev = p_node;
	p_node->mp_Prev = &node_list_head;
	p_node->mp_Next = &node_list_head;

	p_region->mp_Next = NULL;

	for(int i=0; i<(int)(sizeof(p_region->m_Marker)/sizeof(unsigned int)); ++i)
		p_region->m_Marker[i] = COALESCE_ALLOC_MARKER;

	VSTATS(m_Stats.m_Unused -= sizeof(Coalesce::Region) + 3*sizeof(Coalesce::Header));
	VSTATS(m_Stats.m_Overhead += sizeof(Coalesce::Region) + 3*sizeof(Coalesce::Header));
	VSTATS(m_Stats.m_Reserved += m_RegionSize);

#ifdef VMEM_COALESCE_GUARD_PAGES
	if(!SetupGuardPages(p_region))
		return NULL;		// out of memory
#endif

	return p_region;
}

//------------------------------------------------------------------------
Coalesce::Header* CoalesceHeap::FindNextAllocatedHeader(Coalesce::Region* p_region, Coalesce::Header* p_header) const
{
	Coalesce::Header* p_end_header = (Coalesce::Header*)((byte*)p_region + m_RegionSize) - 1;

	// find the next allocated header by skipping over free nodes and fragment nodes
	while(p_header->m_NodeType != nt_Allocated)
	{
		p_header = (Coalesce::Header*)((byte*)p_header + p_header->m_Size);
		VMEM_ASSERT_COALESCE_MARKER(&p_header->m_Marker);
		VMEM_ASSERT((byte*)p_header >= (byte*)p_region && (byte*)p_header < (byte*)p_region + m_RegionSize, "p_header out of range");
	}
	return p_header != p_end_header ? p_header : NULL;
}

//------------------------------------------------------------------------
// warning: this deletes p_region, don't use p_region after calling this function
void CoalesceHeap::ClearRegion(Coalesce::Region* p_region)
{
	// get the first header
	Coalesce::Header* p_header = (Coalesce::Header*)(p_region + 1) + 1;		// +1 for the fixed start alloc

	// get the first allocated header
	p_header = FindNextAllocatedHeader(p_region, p_header);

	while(p_header)
	{
		// check the header
		VMEM_ASSERT_MEM(p_header->m_NodeType == nt_Allocated, &p_header->m_NodeType);
		VMEM_ASSERT((byte*)p_header >= (byte*)p_region && (byte*)p_header < (byte*)p_region + m_RegionSize, "p_header out of range");

		// find the next allocated header by skipping over free nodes and fragment nodes
		Coalesce::Header* p_next_header =  (Coalesce::Header*)((byte*)p_header + p_header->m_Size);
		p_next_header = FindNextAllocatedHeader(p_region, p_next_header);

		// free the alloc
		// warning: this deletes p_region when it's empty, so be careful not to access p_region after last free
		void* p = p_header + 1;
#ifdef VMEM_COALESCE_GUARDS
		p = (byte*)p + COALESCE_GUARD_SIZE;
#endif
		Free(p);

		p_header = p_next_header;
	}

#ifdef VMEM_COALESCE_GUARD_PAGES
	// region won't have been destroyed because it still contains the alloc guards
	RemoveGuardPages(p_region);
#endif
}

//------------------------------------------------------------------------
void CoalesceHeap::DestroyRegion(Coalesce::Region* p_region)
{
	CheckIntegrity();

#ifdef VMEM_ASSERTS
	int total_free_size = m_RegionSize - g_RegionOverhead;
	Coalesce::Header* p_header = (Coalesce::Header*)((byte*)p_region + sizeof(Coalesce::Region) + sizeof(Coalesce::Header));
	VMEM_ASSERT(p_header->m_Size == total_free_size, "trying to destrroy non-empty region");
#endif

	// take region off list
	if(p_region == mp_RegionList)
	{
		mp_RegionList = p_region->mp_Next;
	}
	else
	{
		Coalesce::Region* p_srch_region = mp_RegionList;
		while(p_srch_region)
		{
			Coalesce::Region* p_next = p_srch_region->mp_Next;
			VMEM_ASSERT(p_next, "unable to find region to remove");

			if(p_next == p_region)
			{
				p_srch_region->mp_Next = p_region->mp_Next;
				break;
			}

			p_srch_region = p_next;
		}
	}

	// free the last free node that was alloced when creating the region
	Coalesce::Header* p_free_header = (Coalesce::Header*)(p_region + 1) + 1;
	Coalesce::Node* p_node = p_free_header->mp_Node;
	VMEM_ASSERT_COALESCE_MARKER(&p_free_header->m_Marker);
	VMEM_ASSERT_MEM(p_free_header->m_NodeType != nt_Allocated && p_free_header->m_NodeType != nt_Fragment, &p_free_header->m_NodeType);
	VMEM_ASSERT_CODE(VMEM_ASSERT_MEM(p_free_header->m_Size == total_free_size, &p_free_header->m_Size));
	RemoveNode(p_node);
	FreeNode(p_node);

	VSTATS(m_Stats.m_Unused += sizeof(Coalesce::Region) + 3*sizeof(Coalesce::Header));
	VSTATS(m_Stats.m_Overhead -= sizeof(Coalesce::Region) + 3*sizeof(Coalesce::Header));

	// decommit the start fixed header and region struct
	Header* p_first_header = (Coalesce::Header*)((byte*)p_region + sizeof(Coalesce::Region) + sizeof(Coalesce::Header));
	int free_size = p_first_header->m_Size - sizeof(Coalesce::Header);

	int start_offset = sizeof(Coalesce::Region) + 2*sizeof(Coalesce::Header);
	int start_commit_size = AlignUpPow2(start_offset, SYS_PAGE_SIZE);
	int commit_size = start_commit_size - start_offset;

	int check_size = Min(free_size, commit_size);
	CheckMemory(p_first_header+1, check_size, FREED_MEM);
	VirtualMem::Decommit(p_region, start_commit_size);
	VSTATS(m_Stats.m_Unused -= start_commit_size);

	// decommit the end fixed header
	int end_commit_size = AlignUpPow2(sizeof(Coalesce::Header), SYS_PAGE_SIZE);
	void* p_last_header = (byte*)p_region + m_RegionSize - sizeof(Coalesce::Header);
	void* p_last_page = AlignDownPow2(p_last_header, SYS_PAGE_SIZE);
	if(p_last_page >= (byte*)p_region + start_commit_size)
	{
		#ifndef VMEM_MEMSET_ONLY_SMALL
			if(p_last_header > p_last_page)
				CheckMemory(p_last_page, (byte*)p_last_header - (byte*)p_last_page, FREED_MEM);
		#endif

		VirtualMem::Decommit(p_last_page, end_commit_size);
		VSTATS(m_Stats.m_Unused -= end_commit_size);
	}

	VirtualMem::Release(p_region);

	VSTATS(m_Stats.m_Reserved -= m_RegionSize);
}

//------------------------------------------------------------------------
// ensure all the complete pages in the range are committed
// |-------|---C---|-------|	<- only commit middle page, other pages will already be committed
//      xxxxxxxxxxxxxx			<- range
bool CoalesceHeap::CommitRange(void* p, int size)
{
	VMEM_ASSERT(p, "invalid pointer passed to CommitRange");
	VMEM_ASSERT(size > 0, "invalid size passed to CommitRange");

	void* p_page_start = AlignUpPow2(p, SYS_PAGE_SIZE);
	void* p_page_end = AlignDownPow2((byte*)p + size, SYS_PAGE_SIZE);

	if(p_page_start < p_page_end)
	{
		size_t commit_size = (byte*)p_page_end - (byte*)p_page_start;

		bool success = VirtualMem::Commit(p_page_start, commit_size);
		VSTATS(m_Stats.m_Unused += commit_size);
		VMEMSET(p_page_start, FREED_MEM, commit_size);
		return success;
	}

	return true;
}

//------------------------------------------------------------------------
// ensure all the complete pages in the range are decommitted
// |-------|---D---|-------|	<- decommit just the middle page
//      xxxxxxxxxxxxxx			<- range
void CoalesceHeap::DecommitRange(void* p, int size)
{
	VMEM_ASSERT(p, "invalid pointer passed to DecommitRange");
	VMEM_ASSERT(size > 0, "invalid size passed to DecommitRange");

	void* p_page_start = AlignUpPow2(p, SYS_PAGE_SIZE);
	void* p_page_end = AlignDownPow2((byte*)p + size, SYS_PAGE_SIZE);

	if(p_page_start < p_page_end)
	{
		size_t decommit_size = (byte*)p_page_end - (byte*)p_page_start;

		VirtualMem::Decommit(p_page_start, decommit_size);
		VSTATS(m_Stats.m_Unused -= decommit_size);
	}
}

//------------------------------------------------------------------------
Coalesce::Node* CoalesceHeap::AllocNode()
{
	VSTATS(m_Stats.m_Overhead += sizeof(Coalesce::Node));
	return m_NodeFSA.Alloc();
}

//------------------------------------------------------------------------
void CoalesceHeap::FreeNode(Coalesce::Node* p_node)
{
	VMEM_ASSERT_MEM(!p_node->mp_Prev, &p_node->mp_Prev);		// trying to free node that is still on a list?
	VMEM_ASSERT_MEM(!p_node->mp_Next, &p_node->mp_Next);		// trying to free node that is still on a list?
	m_NodeFSA.Free(p_node);
	VSTATS(m_Stats.m_Overhead -= sizeof(Coalesce::Node));
}

//------------------------------------------------------------------------
int CoalesceHeap::GetFreeListIndex(int size) const
{
	// fragments don't go into the free list
	VMEM_ASSERT(size >= m_MinSize, "size passed to GetFreeListIndex is < min size");

	int r = m_MaxSize - m_MinSize;	// the range of possible sizes
	long long s = size - m_MinSize;	// the size (in range space)
	if(s > r) s = r;				// coalesced blocks can be bigger than the max size.

	// the skewing loses resolution if the range is not big enough, we are better of with a linear mapping
	// 36 seems about right for 32 bit ints.
	int max_index = g_FreeNodeMapSize - 1;
	if(r > 36 * g_FreeNodeMapSize)
	{
		// the skew formula is
		// skew = r - (s*s*s) / (r*r)
		// where s = r - s
		// this skews it so that there are more indices for smaller sizes which is usually beneficial
		s = r - s;
		s = r - (s * ((s*s)/r) ) / r;
	}

	int index = (int)((s * max_index) / r);
	VMEM_ASSERT(index >= 0 && index < g_FreeNodeMapSize, "GetFreeListIndex returning index out of range");
	return index;
}

//------------------------------------------------------------------------
Coalesce::Node* CoalesceHeap::GetNode(int size, Coalesce::Region*& p_region)
{
	// get the first region
	p_region = mp_RegionList;
	if(!p_region)
		p_region = mp_RegionList = CreateRegion();

	// search for a free node list big enough to hold the alloc
	while(p_region)
	{
		Coalesce::Node* p_free_node_lists = p_region->mp_FreeNodeLists;

		// get the best list to start searching in
		int index = GetFreeListIndex(size);
		VMEM_ASSERT(index >= 0 && index < g_FreeNodeMapSize, "invalid index returned by GetFreeListIndex");
		Coalesce::Node* p_list_head = p_free_node_lists + index;
		Coalesce::Node* p_node = p_list_head->mp_Next;

		// Best Fit - find the smallest node that fits the request (the list is sorted by size, small to large)
		while(p_node != p_list_head && p_node->m_Size < size)
			p_node = p_node->mp_Next;

		// if we haven't found a node that is big enough in the list move on to the next
		// non-empty list. Note that if we move onto another list we don't need to iterate
		// down the list, we can just use the first node in the list because it is always
		// guaranteed to be big enough.
		while(p_node == p_list_head && index < g_FreeNodeMapSize)
		{
			++index;
			++p_list_head;
			p_node = p_list_head->mp_Next;
		}

		// if we've found a node return it
		if(index < g_FreeNodeMapSize && p_node != p_list_head)
		{
			RemoveNode(p_node);
			return p_node;
		}

		// otherwise try the next region
		if(!p_region->mp_Next)
			p_region->mp_Next = CreateRegion();

		p_region = p_region->mp_Next;
	}

	return NULL;		// out of memory
}

//------------------------------------------------------------------------
void CoalesceHeap::InsertNode(Coalesce::Node* p_node, Coalesce::Region* p_region)
{
	VMEM_ASSERT_MEM(!p_node->mp_Prev, &p_node->mp_Prev);		// node already inserted?
	VMEM_ASSERT_MEM(!p_node->mp_Next, &p_node->mp_Next);
	VMEM_ASSERT(p_region == GetRegion(p_node->mp_Mem) || !GetRegion(p_node->mp_Mem), "Inserting node from another region");

	// get the list for this node size
	int index = GetFreeListIndex(p_node->m_Size);
	Coalesce::Node* p_list_head = p_region->mp_FreeNodeLists + index;

	// find the best place to insert
	Coalesce::Node* p_prev = p_list_head;
	if(index == g_FreeNodeMapSize-1)
	{
#ifndef VMEM_DISABLE_BIASING
		// always sort the top bucket by address only. No point doing best fit on blocks
		// that are much larger than our maximum allocation size, just take the lowest
		// in memory. this biasing reduces fragmentation.
		for(Coalesce::Node* p_iter=p_prev->mp_Next; p_iter!=p_list_head; p_iter=p_iter->mp_Next)
		{
			if(p_node->mp_Mem < p_iter->mp_Mem)
				break;
			p_prev = p_iter;
		}
#endif
	}
	else
	{
		for(Coalesce::Node* p_iter=p_prev->mp_Next; p_iter!=p_list_head; p_iter=p_iter->mp_Next)
		{
			// sort by size first and then by address, smallest to largest (in both)
#ifdef VMEM_DISABLE_BIASING
			if(p_iter->m_Size > p_node->m_Size)
#else
			if(p_iter->m_Size > p_node->m_Size || 
				(p_iter->m_Size == p_node->m_Size && p_node->mp_Mem < p_iter->mp_Mem))
#endif
			{
				break;
			}
			p_prev = p_iter;
		}
	}

	// link the node in
	Coalesce::Node* p_next = p_prev->mp_Next;
	p_prev->mp_Next = p_node;
	p_node->mp_Prev = p_prev;
	p_node->mp_Next = p_next;
	p_next->mp_Prev = p_node;
}

//------------------------------------------------------------------------
// unlink the node form the list
void CoalesceHeap::RemoveNode(Coalesce::Node* p_node)
{
	Coalesce::Node* p_prev = p_node->mp_Prev;
	Coalesce::Node* p_next = p_node->mp_Next;

	VMEM_ASSERT(GetRegion(p_node->mp_Mem) == GetRegion(p_prev->mp_Mem) || !p_prev->mp_Mem, "Removing node from another region");
	VMEM_ASSERT(GetRegion(p_node->mp_Mem) == GetRegion(p_next->mp_Mem) || !p_next->mp_Mem, "Removing node from another region");

	p_prev->mp_Next = p_next;
	p_next->mp_Prev = p_prev;

	p_node->mp_Prev = NULL;
	p_node->mp_Next = NULL;
}

//------------------------------------------------------------------------
// return the region that contains the specified address.
Coalesce::Region* CoalesceHeap::GetRegion(void* p) const
{
	Coalesce::Region* p_region = mp_RegionList;
	while(p_region)
	{
		byte* p_mem = (byte*)p_region;
		if(p >= p_mem && p < p_mem + m_RegionSize)
			return p_region;

		p_region = p_region->mp_Next;
	}
	return NULL;
}

//------------------------------------------------------------------------
void CoalesceHeap::DebugWrite() const
{
	for(Coalesce::Region* p_region = mp_RegionList; p_region!=NULL; p_region=p_region->mp_Next)
	{
		VMem::DebugWrite(_T("--------------------------------\n"), p_region);
		VMem::DebugWrite(_T("Region 0x%08x\n"), p_region);

		// write the sequential memory blocks
		Coalesce::Header* p_header = (Coalesce::Header*)(p_region + 1) + 1;
		Coalesce::Header* p_last_header = (Coalesce::Header*)((byte*)p_region + m_RegionSize - sizeof(Coalesce::Header));
		while(p_header != p_last_header)
		{
			VMEM_ASSERT_COALESCE_MARKER(&p_header->m_Marker);

			int size = p_header->m_Size;
			void* p_start = p_header;
			void* p_end = (byte*)p_header + size;

			const _TCHAR* p_status;
			if(p_header->m_NodeType == nt_Fragment) p_status = _T("frag");
			else if(p_header->m_NodeType == nt_Fixed) p_status = _T("fixed");
			else if(p_header->m_NodeType == nt_Allocated) p_status = _T("alloc");
			else p_status = _T("free");

			VMem::DebugWrite(_T("\t0x%08x - 0x%08x %7d\t%s\n"), p_start, p_end, size, p_status);

			p_header = (Coalesce::Header*)((byte*)p_header + p_header->m_Size);
		}

		// write the free node map
		VMem::DebugWrite(_T("\n"));
		VMem::DebugWrite(_T("Free nodes:\n"));

		for(int i=0; i<g_FreeNodeMapSize; ++i)
		{
			Coalesce::Node* p_list_head = p_region->mp_FreeNodeLists + i;
			if(p_list_head->mp_Next != p_list_head)
			{
				VMem::DebugWrite(_T("free list %d\n"), i);

				for(Coalesce::Node* p_node = p_list_head->mp_Next; p_node!=p_list_head; p_node=p_node->mp_Next)
					VMem::DebugWrite(_T("\t%08x %d\n"), p_node->mp_Mem, p_node->m_Size);
			}
		}
	}
}

//------------------------------------------------------------------------
void CoalesceHeap::CheckIntegrity() const
{
#ifdef VMEM_ASSERTS
	for(Coalesce::Region* p_region = mp_RegionList; p_region!=NULL; p_region=p_region->mp_Next)
	{
		// check headers
		Coalesce::Header* p_header = (Coalesce::Header*)(p_region + 1) + 1;
		Coalesce::Header* p_last_header = (Coalesce::Header*)((byte*)p_region + m_RegionSize - sizeof(Coalesce::Header));
		size_t last_node_type = nt_Allocated;
		int prev_size = -1;
		while(p_header != p_last_header)
		{
			VMEM_ASSERT_COALESCE_MARKER(&p_header->m_Marker);
			VMEM_ASSERT(prev_size == -1 || p_header->m_PrevSize == prev_size, "header prev size is incorrect");
			prev_size = p_header->m_Size;

			VMEM_ASSERT_MEM(p_header->m_NodeType == nt_Allocated || p_header->m_NodeType != last_node_type, &p_header->m_NodeType);		// two consecutive free nodes of the same status
			last_node_type = p_header->m_NodeType;

			p_header = (Coalesce::Header*)((byte*)p_header + p_header->m_Size);
		}

		// check free nodes
		prev_size = 0;
		for(int i=0; i<g_FreeNodeMapSize; ++i)
		{
			Coalesce::Node* p_list_head = p_region->mp_FreeNodeLists + i;
			VMEM_ASSERT_MEM(p_list_head->mp_Next, &p_list_head->mp_Next);		// found a list in the free node map with a null next pointer

			void* p_prev_addr = NULL;

			for(Coalesce::Node* p_node = p_list_head->mp_Next; p_node!=p_list_head; p_node=p_node->mp_Next)
			{
				VMEM_ASSERT(p_node, "null node");
				Coalesce::Header* p_header = (Coalesce::Header*)p_node->mp_Mem;

#ifndef VMEM_DISABLE_BIASING
				if(p_node->m_Size == prev_size || i == g_FreeNodeMapSize-1)
					VMEM_ASSERT_MEM(p_node->mp_Mem > p_prev_addr, &p_node->mp_Mem);		// Coalesce heap free node map list out of order
				else
					VMEM_ASSERT_MEM(p_node->m_Size > prev_size, &p_node->m_Size);		// Coalesce heap free node map list out of order
#endif
				// check memory
				CheckIntegrity(p_header);

				// remember sizes
				prev_size = p_node->m_Size;
				p_prev_addr = p_node->mp_Mem;

				VMEM_ASSERT(p_node->m_Size == p_header->m_Size, "coalesce heap block size doens't match node size");
				VMEM_ASSERT_COALESCE_MARKER(&p_header->m_Marker);
				VMEM_ASSERT_MEM(p_header->mp_Node == p_node, &p_header->mp_Node);		// Header doesn't point back to free node
			}
		}
	}
#endif

#ifdef VMEM_TRAIL_GUARDS
	m_TrailGuard.CheckIntegrity();
#endif
}

//------------------------------------------------------------------------
void CoalesceHeap::CheckIntegrity(Coalesce::Header* p_header) const
{
	void* p_check_mem = p_header + 1;
	size_t mem_check_size = p_header->m_Size - sizeof(Coalesce::Header);
	void* p_start_page = AlignDownPow2((byte*)p_check_mem-1, SYS_PAGE_SIZE);
	void* p_end_page = AlignDownPow2((byte*)p_check_mem + mem_check_size, SYS_PAGE_SIZE);
	if((byte*)p_end_page - (byte*)p_start_page > SYS_PAGE_SIZE)
		mem_check_size = (byte*)p_start_page + SYS_PAGE_SIZE - (byte*)p_check_mem;
	mem_check_size = Min(mem_check_size, g_MaxCheckMemorySize);

	CheckMemory(p_check_mem, mem_check_size, FREED_MEM);
}

//------------------------------------------------------------------------
#ifdef VMEM_STATS
size_t CoalesceHeap::WriteAllocs() const
{
	VMem::DebugWrite(_T("CoalesceHeap %d-%d\n"), m_MinSize, m_MaxSize);

	int allocs_written = 0;
	int alloc_count = 0;
	size_t bytes_allocated = 0;

	for(Coalesce::Region* p_region = mp_RegionList; p_region!=NULL; p_region=p_region->mp_Next)
	{
		// check headers
		Coalesce::Header* p_header = (Coalesce::Header*)(p_region + 1) + 1;
		Coalesce::Header* p_last_header = (Coalesce::Header*)((byte*)p_region + m_RegionSize - sizeof(Coalesce::Header));
		while(p_header != p_last_header)
		{
			if(p_header->m_NodeType == nt_Allocated)
			{
				if(allocs_written != WRITE_ALLOCS_MAX)
				{
					const void* p_alloc = p_header + 1;
#ifdef VMEM_COALESCE_GUARDS
					p_alloc = (byte*)p_alloc + COALESCE_GUARD_SIZE;
#endif
					WriteAlloc(p_alloc, p_header->m_Size);
					++allocs_written;
				}
				++alloc_count;
				bytes_allocated += p_header->m_Size - sizeof(Coalesce::Header);
#ifdef VMEM_COALESCE_GUARDS
				bytes_allocated -= 2*COALESCE_GUARD_SIZE;
#endif
			}

			p_header = (Coalesce::Header*)((byte*)p_header + p_header->m_Size);
		}
	}

	if(allocs_written == WRITE_ALLOCS_MAX)
		VMem::DebugWrite(_T("only showing first %d allocs\n"), WRITE_ALLOCS_MAX);

	VMem::DebugWrite(_T("%d bytes allocated across %d allocations\n"), bytes_allocated, alloc_count);

	return bytes_allocated;
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
void CoalesceHeap::SendStatsToMemPro(MemProSendFn send_fn, void* p_context)
{
	MemProStats::CoalesceHeapStats stats;
	stats.m_MinSize = m_MinSize;
	stats.m_MaxSize = m_MaxSize;
	stats.m_RegionSize = m_RegionSize;

	stats.m_RegionCount = 0;
	for(Coalesce::Region* p_region = mp_RegionList; p_region!=NULL; p_region=p_region->mp_Next)
		++stats.m_RegionCount;

	stats.m_Stats = m_Stats;

	SendEnumToMemPro(vmem_CoalesceHeap, send_fn, p_context);
	SendToMemPro(stats, send_fn, p_context);
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_COALESCE_GUARD_PAGES
bool CoalesceHeap::SetupGuardPages(Coalesce::Region* p_region)
{
	Coalesce::Header* p_free_header = (Coalesce::Header*)(p_region+1) + 1;
	Coalesce::Header* p_last_header = (Coalesce::Header*)((byte*)p_free_header + p_free_header->m_Size);

	void* p_prev_committed_page = AlignDownPow2(p_free_header, SYS_PAGE_SIZE);
	void* p_last_committed_page = AlignDownPow2(p_last_header, SYS_PAGE_SIZE);

	// we put a guard page every # times the max allocation size
	while(p_free_header->m_Size > VMEM_COALESCE_GUARD_PAGES * m_MaxSize + 2*SYS_PAGE_SIZE)
	{
		//--------------------------------
		// create the new decommitted page block

		// find the page that we will leave decommitted
		void* p_page = AlignUpPow2((byte*)p_free_header + VMEM_COALESCE_GUARD_PAGES * m_MaxSize, SYS_PAGE_SIZE);
		VMEM_ASSERT((byte*)p_page + SYS_PAGE_SIZE <= (byte*)p_region + m_RegionSize, "guard page out of range");

		// get the guard header
		int guard_size = sizeof(Coalesce::Header) + SYS_PAGE_SIZE;
		Coalesce::Header* p_guard_header = (Coalesce::Header*)((byte*)p_page - sizeof(Coalesce::Header));

		// reduce the size of the prev free block
		int new_free_size = (int)((byte*)p_guard_header - (byte*)p_free_header);
		p_free_header->m_Size = new_free_size;
		RemoveNode(p_free_header->mp_Node);
		p_free_header->mp_Node->m_Size = new_free_size;
		InsertNode(p_free_header->mp_Node, p_region);

		// commit the page for the guard header.
		void* p_commit_page = AlignDownPow2(p_guard_header, SYS_PAGE_SIZE);
		if(p_commit_page != p_prev_committed_page)
		{
			if(!VirtualMem::Commit(p_commit_page, SYS_PAGE_SIZE))
				return false;		// out of memory
			VSTATS(m_Stats.m_Unused += SYS_PAGE_SIZE);
			VMEMSET(p_commit_page, FREED_MEM, SYS_PAGE_SIZE);
			p_prev_committed_page = p_commit_page;
		}
		VSTATS(m_Stats.m_Unused -= sizeof(Coalesce::Header));
		VSTATS(m_Stats.m_Overhead += sizeof(Coalesce::Header));

		// setup the guard header
		SetupHeader(p_guard_header);
		p_guard_header->m_Size = guard_size;
		p_guard_header->m_PrevSize = p_free_header->m_Size;
		p_guard_header->m_NodeType = nt_Fixed;

		//--------------------------------
		// setup the next free block

		// get the header for the next free block
		Coalesce::Header* p_next_free_header = (Coalesce::Header*)((byte*)p_guard_header + guard_size);
		int next_free_size = (int)((byte*)p_last_header - (byte*)p_next_free_header);
		
		// create a node
		Coalesce::Node* p_node = AllocNode();
		p_node->mp_Mem = p_next_free_header;
		p_node->m_Size = next_free_size;
		p_node->mp_Prev = p_node->mp_Next = NULL;
		InsertNode(p_node, p_region);

		// commit the page that contains the next free header
		void* p_next_free_commit_page = AlignDownPow2(p_next_free_header, SYS_PAGE_SIZE);
		if(p_next_free_commit_page != p_last_committed_page)
		{
			if(!VirtualMem::Commit(p_next_free_commit_page, SYS_PAGE_SIZE))
				return false;		// out of memory
			VMEMSET(p_next_free_commit_page, FREED_MEM, SYS_PAGE_SIZE);
		}
		p_prev_committed_page = p_next_free_commit_page;
		VSTATS(m_Stats.m_Unused += SYS_PAGE_SIZE - sizeof(Coalesce::Header));
		VSTATS(m_Stats.m_Overhead += sizeof(Coalesce::Header));

		// setup the header
		SetupHeader(p_next_free_header);
		p_next_free_header->m_Size = next_free_size;
		p_next_free_header->m_PrevSize = guard_size;
		p_next_free_header->mp_Node = p_node;

		p_last_header->m_PrevSize = next_free_size;

		// move to the next free header
		p_free_header = p_next_free_header;
	}

	return true;
}
#endif

//------------------------------------------------------------------------
// this function assumes that all other allocs have been removed and only
// the guard allocs remain. All guard allocs will be removed and the region
// destroyed.
#ifdef VMEM_COALESCE_GUARD_PAGES
void CoalesceHeap::RemoveGuardPages(Coalesce::Region* p_region)
{
	// if the region is too small no guard pages will have been setup and the region will alreddy have been decallocated
	if(m_RegionSize - g_RegionOverhead < VMEM_COALESCE_GUARD_PAGES * m_MaxSize + 2*SYS_PAGE_SIZE)
		return;

	Coalesce::Header* p_first_free_header = (Coalesce::Header*)(p_region+1) + 1;
	Coalesce::Header* p_last_header = (Coalesce::Header*)((byte*)p_region + m_RegionSize - sizeof(Coalesce::Header));

	void* p_first_committed_page = AlignDownPow2(p_first_free_header, SYS_PAGE_SIZE);

	Coalesce::Header* p_guard_header = (Coalesce::Header*)((byte*)p_first_free_header + p_first_free_header->m_Size);

	while(p_guard_header != p_last_header)
	{
		VMEM_MEM_CHECK(&p_guard_header->m_NodeType, nt_Fixed);		// expected guard node

		// get the next free header
		Coalesce::Header* p_free_header = (Coalesce::Header*)((byte*)p_guard_header + p_guard_header->m_Size);
		VMEM_ASSERT_MEM(p_free_header->m_NodeType > nt_Allocated, &p_free_header->m_NodeType);		// expected free node
		Coalesce::Header* p_next_guard_header = (Coalesce::Header*)((byte*)p_free_header + p_free_header->m_Size);
		VMEMSET(p_guard_header, FREED_MEM, sizeof(Coalesce::Header));

		// delete the free node
		RemoveNode(p_free_header->mp_Node);
		FreeNode(p_free_header->mp_Node);
		VMEMSET(p_free_header, FREED_MEM, sizeof(Coalesce::Header));

		// decommit the guard header page
		void* p_guard_header_page = AlignDownPow2(p_guard_header, SYS_PAGE_SIZE);
		void* p_free_header_page = AlignDownPow2(p_free_header, SYS_PAGE_SIZE);
		if(p_guard_header_page != p_free_header_page && p_guard_header_page != p_first_committed_page)
		{
			VirtualMem::Decommit(p_guard_header_page, SYS_PAGE_SIZE);
			VSTATS(m_Stats.m_Unused -= SYS_PAGE_SIZE);
		}
		VSTATS(m_Stats.m_Unused += sizeof(Coalesce::Header));
		VSTATS(m_Stats.m_Overhead -= sizeof(Coalesce::Header));

		// decommit the free header page
		void* p_next_guard_header_page = AlignDownPow2(p_next_guard_header, SYS_PAGE_SIZE);
		if(p_free_header_page != p_next_guard_header_page && p_free_header_page != p_first_committed_page)
		{
			VirtualMem::Decommit(p_free_header_page, SYS_PAGE_SIZE);
			VSTATS(m_Stats.m_Unused -= SYS_PAGE_SIZE);
		}
		VSTATS(m_Stats.m_Unused += sizeof(Coalesce::Header));
		VSTATS(m_Stats.m_Overhead -= sizeof(Coalesce::Header));

		p_guard_header = p_next_guard_header;
	}

	int total_free_size = m_RegionSize - g_RegionOverhead;
	p_first_free_header->m_Size = total_free_size;
	p_first_free_header->mp_Node->m_Size = total_free_size;
	p_last_header->m_PrevSize = total_free_size;

	CheckIntegrity();

	DestroyRegion(p_region);
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_OS_WIN
	#pragma warning(pop)
#endif
//------------------------------------------------------------------------
// FSA.cpp


//------------------------------------------------------------------------
// FSA.hpp






//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	class PageHeap;
	class FSA;

	//------------------------------------------------------------------------
	struct FSAPageHeader
	{
#ifdef VMEM_ASSERTS
		unsigned int m_Marker;
#endif
		FSA* mp_FSA;
		void* mp_FreeSlots;
		int m_UsedSlotCount;
		FSAPageHeader* mp_Next;

#ifdef VMEM_STATS
		FSAPageHeader* mp_Prev;
#endif
	};

	//------------------------------------------------------------------------
	// FSA is a Fixed Size Allocator. It is setup with an allocation size, alignment
	// and a page heap. It allocates pages from the page heap and divides them up
	// into slots of the allocation size. The pages are put onto a free page list.
	// The slots are linked onto a free list. When allocating a slot of popped off
	// the free list, and added back onto the free list when freeing. When a page is
	// full it is removed from the free page list. When a page is empty it is given
	// back to the page heap. When a slot is freed from a full page the page is put
	// back onto the free page list. The freee page list is sorted by address for
	// biasing to reduce fragmentation.
	// This class is not thread safe.
	class FSA
	{
	public:
		FSA(int size, int alignment, PageHeap* p_page_heap);	// allocation size, alignment of each allocation and the page heap.

		~FSA();

		void* Alloc();

		void Free(void* p);

		static FSA* GetFSA(void* p, int page_size);		// get the FSA object for the address. p must be from an FSA.

		inline int GetSize() const;						// the allocation size

		void CheckIntegrity() const;

#ifdef VMEM_STATS
		inline const Stats& GetStats() const;
		inline const Stats& GetStatsNoLock() const;
		size_t WriteAllocs(int* p_allocs_written_count=NULL, int max_allocs_to_write=WRITE_ALLOCS_MAX, bool write_header=true) const;
		void SendStatsToMemPro(MemProSendFn send_fn, void* p_context);
#endif

#ifdef VMEM_TRAIL_GUARDS
		inline void InitialiseTrailGuard(int size, int check_freq);
#endif

		// Lock and free do nothing, this class is not thread safe.
		void Lock() {}

		void Release() {}

	private:
		void CheckIntegrity(FSAPageHeader* p_page) const;

		FSAPageHeader* AllocPage();

		void FreePage(FSAPageHeader* p_page);

		void InsertPageInFreeList(FSAPageHeader* p_page);

		void RemovePageFromFreeList(FSAPageHeader* p_page);

		//------------------------------------------------------------------------
		// data
	private:
		int m_Size;				// the allocation size
		int m_SlotSize;			// the slot size can be bigger than the allocation size because of alignment and guards
		int m_SlotsPerPage;
		int m_Alignment;

		PageHeap* mp_PageHeap;

		FSAPageHeader* mp_FreePageList;		// only contains partially full pages.

#ifdef VMEM_ASSERTS
		unsigned int m_Marker;
#endif

#ifdef VMEM_STATS
		Stats m_Stats;
		FSAPageHeader m_FullPageList;		// keep track of the full pages so that we can output stats
#endif

#ifdef VMEM_TRAIL_GUARDS
		TrailGuard m_TrailGuard;
#endif
	};

	//------------------------------------------------------------------------
	// A thread safe wrapper for FSA (FSA Multi Threaded). Use inheritance rather
	// than containment  so that we can upcast in FSAMT::GetFSA.
	class FSAMT : protected FSA
	{
	public:
		inline FSAMT(int size, int alignment, PageHeap* p_page_heap);

		inline void* Alloc();

		inline void Free(void* p);

		static inline FSAMT* GetFSA(void* p, int page_size);

		inline int GetSize() const;

		inline void CheckIntegrity() const;

		inline void Lock();

		inline void Release();

#ifdef VMEM_STATS
		inline const Stats& GetStats() const;
		inline const Stats& GetStatsNoLock() const;
		inline size_t WriteAllocs(int* p_allocs_written_count=NULL, int max_allocs_to_write=WRITE_ALLOCS_MAX, bool write_header=true) const;
		inline void SendStatsToMemPro(MemProSendFn send_fn, void* p_context);
#endif

#ifdef VMEM_TRAIL_GUARDS
		inline void InitialiseTrailGuard(int size, int check_freq);
#endif
		//------------------------------------------------------------------------
		// data
	private:
		mutable CriticalSection m_CriticalSection;
	};

	//------------------------------------------------------------------------
	int FSA::GetSize() const
	{
		return m_Size;
	}

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	const Stats& FSA::GetStats() const
	{
		return m_Stats;
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	const Stats& FSA::GetStatsNoLock() const
	{
		return m_Stats;
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_TRAIL_GUARDS
	void FSA::InitialiseTrailGuard(int size, int check_freq)
	{
		m_TrailGuard.Initialise(size, check_freq, m_SlotSize);
	}
#endif

	//------------------------------------------------------------------------
	FSAMT::FSAMT(int size, int alignment, PageHeap* p_page_heap)
	:	FSA(size, alignment, p_page_heap)
	{
	}

	//------------------------------------------------------------------------
	void* FSAMT::Alloc()
	{
		CriticalSectionScope lock(m_CriticalSection);
		return FSA::Alloc();
	}

	//------------------------------------------------------------------------
	void FSAMT::Free(void* p)
	{
		CriticalSectionScope lock(m_CriticalSection);
		FSA::Free(p);
	}

	//------------------------------------------------------------------------
	FSAMT* FSAMT::GetFSA(void* p, int page_size)
	{
		return (FSAMT*)FSA::GetFSA(p, page_size);
	}

	//------------------------------------------------------------------------
	int FSAMT::GetSize() const
	{
		return FSA::GetSize();
	}

	//------------------------------------------------------------------------
	void FSAMT::CheckIntegrity() const
	{
		CriticalSectionScope lock(m_CriticalSection);
		FSA::CheckIntegrity();
	}

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	const Stats& FSAMT::GetStats() const
	{
		CriticalSectionScope lock(m_CriticalSection);
		return FSA::GetStats();
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	const Stats& FSAMT::GetStatsNoLock() const
	{
		return FSA::GetStats();
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	size_t FSAMT::WriteAllocs(int* p_allocs_written_count, int max_allocs_to_write, bool write_header) const
	{
		CriticalSectionScope lock(m_CriticalSection);
		return FSA::WriteAllocs(p_allocs_written_count, max_allocs_to_write, write_header);
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	void FSAMT::SendStatsToMemPro(MemProSendFn send_fn, void* p_context)
	{
		CriticalSectionScope lock(m_CriticalSection);
		FSA::SendStatsToMemPro(send_fn, p_context);
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_TRAIL_GUARDS
	void FSAMT::InitialiseTrailGuard(int size, int check_freq)
	{
		CriticalSectionScope lock(m_CriticalSection);
		FSA::InitialiseTrailGuard(size, check_freq);
	}
#endif

	//------------------------------------------------------------------------
	void FSAMT::Lock()
	{
		m_CriticalSection.Enter();
	}

	//------------------------------------------------------------------------
	void FSAMT::Release()
	{
		m_CriticalSection.Leave();
	}
}

//------------------------------------------------------------------------
// PageHeap.hpp




//------------------------------------------------------------------------
#define USE_LOCKFREE_REGION_ARRAY

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	struct PageHeapRegion;
	class InternalHeap;

	//------------------------------------------------------------------------
	// PageHeap is used to allocate pages of a specific size. The page size must
	// be an exact multiple of the system page size, or vice versa.
	//
	// Note that the regions array was added to allow the Owns function to be lock free
	// because this wsas a point of contention for Realloc. The reason the region list
	// is still used is because it is ordered with the oldest region first. This biases
	// the allocs to the oldest regions allowing the new regions to be cleaned up. The
	// regions array is not ordered, it simply uses the first empty element. The regions
	// array could not be sorted and still be lockfree, so both the array and list are needed.
	//
	class PageHeap
	{
	public:
		PageHeap(int page_size, int region_size, InternalHeap& internal_heap);

		~PageHeap();

		void* Alloc();

		void Free(void* p_page);

		bool Owns(void* p) const;

		inline bool Empty() const;

		inline int GetPageSize() const;

#ifdef VMEM_STATS
		inline const Stats& GetStats() const;
		inline const Stats& GetStatsNoLock() const;
		void SendStatsToMemPro(MemProSendFn send_fn, void* p_context);
#endif

	private:
		PageHeapRegion* CreateRegion();

		void DestroyRegion(PageHeapRegion* p_region);

		PageHeapRegion* GetRegion(void* p) const;

		inline bool SysPageClear(int index, PageHeapRegion* p_region) const;

		PageHeap& operator=(const PageHeap& other);

#ifdef USE_LOCKFREE_REGION_ARRAY
		void AddRegionToRegionsArray(PageHeapRegion* p_region);
		void RemoveRegionFromRegionsArray(PageHeapRegion* p_region);
#endif

		//------------------------------------------------------------------------
		// data
	private:
		mutable CriticalSection m_CriticalSection;

		int m_PageSize;
		int m_SysPageSize;
		int m_RegionSize;
		int m_PagesPerRegion;
		int m_PagesPerSysPage;

		PageHeapRegion* mp_RegionList;

		// the Regions array - see class comment and AddRegionToRegionsArray for more details
#ifdef USE_LOCKFREE_REGION_ARRAY
		int m_RegionsArrayCapacity;
		void** mp_Regions;
		void* mp_DiscardedRegionArrays;
#endif

		InternalHeap& m_InternalHeap;

#ifdef VMEM_STATS
		Stats m_Stats;
#endif
	};

	//------------------------------------------------------------------------
	bool PageHeap::Empty() const
	{
		return mp_RegionList == NULL;
	}

	//------------------------------------------------------------------------
	int PageHeap::GetPageSize() const
	{
		return m_PageSize;
	}

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	const Stats& PageHeap::GetStats() const
	{
		CriticalSectionScope lock(m_CriticalSection);
		return m_Stats;
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_STATS
	const Stats& PageHeap::GetStatsNoLock() const
	{
		return m_Stats;
	}
#endif
}


//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
#ifdef VMEM_FSA_GUARDS
	VMEM_STATIC_ASSERT((FSA_PRE_GUARD_SIZE & 3) == 0);
	VMEM_STATIC_ASSERT((FSA_POST_GUARD_SIZE & 3) == 0);
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_FSA_GUARDS
	// p points to the start of the slot
	inline void SetGuards(void* p, int size, int slot_size)
	{
		SetGuards(p, FSA_PRE_GUARD_SIZE);
		SetGuards((byte*)p + size, slot_size - size - FSA_PRE_GUARD_SIZE);
	}
#endif

	//------------------------------------------------------------------------
#ifdef VMEM_FSA_GUARDS
	inline void CheckGuards(void* p, int size, int slot_size)
	{
		CheckMemory((byte*)p, FSA_PRE_GUARD_SIZE, GUARD_MEM);
		CheckMemory((byte*)p + FSA_PRE_GUARD_SIZE + size, slot_size - size - FSA_PRE_GUARD_SIZE, GUARD_MEM);
	}
#endif
}

//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
FSA::FSA(int size, int alignment, PageHeap* p_page_heap)
:	m_Size(size),
	m_SlotSize(size),
	m_Alignment(alignment),
	mp_PageHeap(p_page_heap),
	mp_FreePageList(NULL)
{
#ifdef VMEM_ASSERTS
	m_Marker = FSA_MARKER;
#endif

#ifdef VMEM_FSA_GUARDS
	m_SlotSize += FSA_PRE_GUARD_SIZE + FSA_POST_GUARD_SIZE;
#endif
	m_SlotSize = AlignUp(m_SlotSize, alignment);

	m_SlotsPerPage = (p_page_heap->GetPageSize() - AlignUp(sizeof(FSAPageHeader), alignment)) / m_SlotSize;

#ifdef VMEM_STATS
	m_FullPageList.m_Marker = FSA_PAGE_HEADER_MARKER;
	m_FullPageList.mp_FSA = NULL;
	m_FullPageList.mp_FreeSlots = NULL;
	m_FullPageList.m_UsedSlotCount = 0;
	m_FullPageList.mp_Prev = &m_FullPageList;
	m_FullPageList.mp_Next = &m_FullPageList;
#endif
}

//------------------------------------------------------------------------
FSA::~FSA()
{
#ifdef VMEM_TRAIL_GUARDS
	void* p_trail_alloc = m_TrailGuard.Shutdown();
	while(p_trail_alloc)
	{
		void* p_next = *(void**)p_trail_alloc;
#ifdef VMEM_FSA_GUARDS
		SetGuards(p_trail_alloc, m_Size, m_SlotSize);	// re-apply our guards
#endif
		Free(p_trail_alloc);
		p_trail_alloc = p_next;
	}
#endif
}

//------------------------------------------------------------------------
void* FSA::Alloc()
{
	// get the first page with a free slot
	FSAPageHeader* p_page = mp_FreePageList;

	// allocate a page if we need to
	if(!p_page)
	{
		p_page = mp_FreePageList = AllocPage();
		if(!p_page)
			return NULL;		// out of memory
	}

	VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_page->m_Marker, FSA_PAGE_HEADER_MARKER));

	// take a slot off the page
	void* p = p_page->mp_FreeSlots;
	VMEM_ASSERT_MEM(p >= p_page+1 && p < (byte*)p_page + mp_PageHeap->GetPageSize(), &p_page->mp_FreeSlots);

	p_page->mp_FreeSlots = *(void**)p;
	VMEM_ASSERT_MEM(!p_page->mp_FreeSlots || (p_page->mp_FreeSlots >= p_page+1 && p_page->mp_FreeSlots < (byte*)p_page + mp_PageHeap->GetPageSize()), p);

	++p_page->m_UsedSlotCount;

	// if the page is full take it off the free list
	if(!p_page->mp_FreeSlots)
	{
		mp_FreePageList = p_page->mp_Next;
		p_page->mp_Next = NULL;

#ifdef VMEM_STATS
		// put the page onto the full list
		FSAPageHeader* p_prev = &m_FullPageList;
		FSAPageHeader* p_next = m_FullPageList.mp_Next;
		p_prev->mp_Next = p_page;
		p_page->mp_Prev = p_prev;
		p_page->mp_Next = p_next;
		p_next->mp_Prev = p_page;
#endif
	}

	if(m_Size > (int)sizeof(void*))
		CheckMemory((byte*)p + sizeof(void*), m_Size - sizeof(void*), FREED_MEM);
	VMEMSET(p, ALLOCATED_MEM, m_Size);

#ifdef VMEM_FSA_GUARDS
	SetGuards(p, m_Size, m_SlotSize);
	p = (byte*)p + FSA_PRE_GUARD_SIZE;
#endif

	VSTATS(m_Stats.m_Unused -= m_SlotSize);
	VSTATS(m_Stats.m_Used += m_Size);
	VSTATS(m_Stats.m_Overhead += m_SlotSize - m_Size);

	return p;
}

//------------------------------------------------------------------------
void FSA::Free(void* p)
{
#ifdef VMEM_FSA_GUARDS
	p = (byte*)p - FSA_PRE_GUARD_SIZE;
	CheckGuards(p, m_Size, m_SlotSize);
#endif

#ifdef VMEM_TRAIL_GUARDS
	if(m_Size > 4 && *((unsigned int*)p+1) == TRAIL_GUARD_MEM)
		VMEM_ASSERT2(!m_TrailGuard.Contains(p), "Double free of pointer %s%p", POINTER_PREFIX, p);

	if(m_TrailGuard.GetSize())
	{
		p = m_TrailGuard.Add(p);
		VSTATS(m_Stats.m_Used -= m_Size);		// move stats to overhead
		VSTATS(m_Stats.m_Overhead += m_Size);
		if(!p) return;							// return here if TrailGuard ddin't return an alloc to free

		VSTATS(m_Stats.m_Used += m_Size);		// move stats back to used
		VSTATS(m_Stats.m_Overhead -= m_Size);
#ifdef VMEM_FSA_GUARDS
		SetGuards(p, m_Size, m_SlotSize);	// re-apply our guards
#endif
	}
#endif

	// memset to FREED_MEM
#if defined(VMEM_MEMSET) || defined(VMEM_MEMSET_ONLY_SMALL)
	void* p_memset = p;
	#ifdef VMEM_FSA_GUARDS
		p_memset = (byte*)p_memset + FSA_PRE_GUARD_SIZE;
	#endif
	if(m_Size > (int)sizeof(void*))
		VMEMSET((byte*)p_memset + sizeof(void*), FREED_MEM, m_Size - sizeof(void*));
#endif

	// get the page that owns this alloc
	FSAPageHeader* p_page = (FSAPageHeader*)AlignDownPow2(p, mp_PageHeap->GetPageSize());
#ifdef VMEM_ASSERTS
	int header_size = AlignUp(sizeof(FSAPageHeader), m_Alignment);
	VMEM_ASSERT((size_t)p >= (size_t)p_page + header_size && (size_t)p < (size_t)p_page + header_size + m_SlotSize * m_SlotsPerPage, "Bad pointer passed to Free. Pointer not in free list range");
	VMEM_ASSERT(((size_t)p - ((size_t)p_page + header_size)) % m_SlotSize == 0, "Bad pointer passed to Free. Pointer not aligned to slot");
#endif

	--p_page->m_UsedSlotCount;

#if defined(VMEM_MEMSET) || defined(VMEM_MEMSET_ONLY_SMALL)
	if((unsigned int)m_Size > sizeof(void*) && *((unsigned int*)((byte*)p+sizeof(void*))) == FREED_MEM)
	{
		void* pc = p_page->mp_FreeSlots;
		while(pc)
		{
			VMEM_ASSERT2(pc != p, "Double free of pointer %s%p", POINTER_PREFIX, p);
			pc = *(void**)pc;
		}
	}
#endif

	// if the page was full put it back onto the free list
	if(!p_page->mp_FreeSlots)
	{
#ifdef VMEM_STATS
		// take page off the full list
		FSAPageHeader* p_prev = p_page->mp_Prev;
		FSAPageHeader* p_next = p_page->mp_Next;
		p_prev->mp_Next = p_next;
		p_next->mp_Prev = p_prev;
		p_page->mp_Prev = p_page->mp_Next = NULL;
#endif
		InsertPageInFreeList(p_page);
	}

	// put the slot back onto the free list
	*(void**)p = p_page->mp_FreeSlots;
	p_page->mp_FreeSlots = p;

	// if the page is empty free it
	if(!p_page->m_UsedSlotCount)
		FreePage(p_page);

	VSTATS(m_Stats.m_Unused += m_SlotSize);
	VSTATS(m_Stats.m_Used -= m_Size);
	VSTATS(m_Stats.m_Overhead -= m_SlotSize - m_Size);
}

//------------------------------------------------------------------------
FSAPageHeader* FSA::AllocPage()
{
	// allocate the page
	FSAPageHeader* p_page = (FSAPageHeader*)mp_PageHeap->Alloc();
	if(!p_page)
		return NULL;	// out of memory

	// put all slots onto the free list
	// note that slots are always aligned to slot size
	int aligned_header_size = AlignUp(sizeof(FSAPageHeader), m_Alignment);
	byte* p_free_slots = (byte*)p_page + aligned_header_size;
	byte* p = p_free_slots;
	byte* p_end = p_free_slots + (m_SlotsPerPage-1) * m_SlotSize;
	bool do_memset = m_Size > (int)sizeof(void*);
	while(p != p_end)
	{
		byte* p_next = p + m_SlotSize;
		*(byte**)p = p_next;
		if(do_memset) VMEMSET((byte*)p + sizeof(void*), FREED_MEM, m_Size - sizeof(void*));
		p = p_next;
	}
	if(do_memset) VMEMSET((byte*)p + sizeof(void*), FREED_MEM, m_Size - sizeof(void*));
	*(byte**)p = NULL;

	// setup the page
	p_page->mp_FSA = this;
	p_page->mp_FreeSlots = p_free_slots;
	p_page->m_UsedSlotCount = 0;
	p_page->mp_Next = NULL;

#ifdef VMEM_STATS
	p_page->mp_Prev = NULL;
#endif

#ifdef VMEM_ASSERTS
	p_page->m_Marker = FSA_PAGE_HEADER_MARKER;
#endif

	VSTATS(m_Stats.m_Unused += mp_PageHeap->GetPageSize() - aligned_header_size);
	VSTATS(m_Stats.m_Overhead += aligned_header_size);

	return p_page;
}

//------------------------------------------------------------------------
void FSA::FreePage(FSAPageHeader* p_page)
{
	VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_page->m_Marker, FSA_PAGE_HEADER_MARKER));

	VMEM_ASSERT_CODE(CheckIntegrity(p_page));

	RemovePageFromFreeList(p_page);

	mp_PageHeap->Free(p_page);

	VSTATS(m_Stats.m_Unused -= mp_PageHeap->GetPageSize() - AlignUp(sizeof(FSAPageHeader), m_Alignment));
	VSTATS(m_Stats.m_Overhead -= AlignUp(sizeof(FSAPageHeader), m_Alignment));
}

//------------------------------------------------------------------------
// keeps the free page list sorted by address for the biasing
void FSA::InsertPageInFreeList(FSAPageHeader* p_page)
{
	VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_page->m_Marker, FSA_PAGE_HEADER_MARKER));
	VMEM_ASSERT_MEM(!p_page->mp_Next, &p_page->mp_Next);		// FSAPageHeader already in list?

#ifdef VMEM_DISABLE_BIASING
	p_page->mp_Next = mp_FreePageList;
	mp_FreePageList = p_page;
#else
	// find the previous page < p_page
	FSAPageHeader* p_prev_page = NULL;
	FSAPageHeader* p_srch_page = mp_FreePageList;
	while(p_srch_page && p_srch_page < p_page)
	{
		p_prev_page = p_srch_page;
		p_srch_page = p_srch_page->mp_Next;
	}

	// insert the page
	if(p_prev_page)
	{
		p_page->mp_Next = p_prev_page->mp_Next;
		p_prev_page->mp_Next = p_page;
	}
	else
	{
		p_page->mp_Next = mp_FreePageList;
		mp_FreePageList = p_page;
	}
#endif
}

//------------------------------------------------------------------------
void FSA::RemovePageFromFreeList(FSAPageHeader* p_page)
{
	VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_page->m_Marker, FSA_PAGE_HEADER_MARKER));

	// find the previous page
	FSAPageHeader* p_prev_page = NULL;
	FSAPageHeader* p_srch_page = mp_FreePageList;
	while(p_srch_page != p_page)
	{
		p_prev_page = p_srch_page;
		p_srch_page = p_srch_page->mp_Next;
	}

	// remove it
	if(p_prev_page)
		p_prev_page->mp_Next = p_page->mp_Next;
	else
		mp_FreePageList = p_page->mp_Next;

	p_page->mp_Next = NULL;
}

//------------------------------------------------------------------------
FSA* FSA::GetFSA(void* p, int page_size)
{
	size_t page_mask = ~(page_size - 1);
	FSAPageHeader* p_page = (FSAPageHeader*)((size_t)p & page_mask);
	VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_page->m_Marker, FSA_PAGE_HEADER_MARKER));
	FSA* p_fsa = p_page->mp_FSA;
	VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_fsa->m_Marker, FSA_MARKER));
	return p_fsa;
}

//------------------------------------------------------------------------
void FSA::CheckIntegrity() const
{
	for(FSAPageHeader* p_page=mp_FreePageList; p_page!=NULL; p_page=p_page->mp_Next)
		CheckIntegrity(p_page);

#ifdef VMEM_TRAIL_GUARDS
	m_TrailGuard.CheckIntegrity();
#endif
}

//------------------------------------------------------------------------
void FSA::CheckIntegrity(FSAPageHeader* p_page) const
{
	VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_page->m_Marker, FSA_PAGE_HEADER_MARKER));
	VMEM_ASSERT_CODE(VMEM_MEM_CHECK(&p_page->mp_FSA->m_Marker, FSA_MARKER));

	// check p_slot pointers are valid and the free slot count matches
	int free_slot_count = 0;
	void* p_slot = p_page->mp_FreeSlots;
	VMEM_ASSERT_CODE(void* p_slot_addr = &p_page->mp_FreeSlots);
	while(p_slot && free_slot_count < m_SlotsPerPage)
	{
		VMEM_ASSERT_MEM(!p_slot || (p_slot >= p_page+1 && p_slot < (byte*)p_page + mp_PageHeap->GetPageSize()), p_slot_addr);
		++free_slot_count;
		VMEM_ASSERT_CODE(p_slot_addr = p_slot);
		p_slot = *(void**)p_slot;
	}
	VMEM_ASSERT(free_slot_count == m_SlotsPerPage-p_page->m_UsedSlotCount, "FSA free slot list or m_UsedSlotCount corrupted");

	// check the memory contents
	if(m_Size > (int)sizeof(void*))
	{
		int check_size = m_Size - sizeof(void*);
		void* p_slot = p_page->mp_FreeSlots;
		while(p_slot)
		{
			CheckMemory((void**)p_slot+1, check_size, FREED_MEM);
			p_slot = *(void**)p_slot;
		}
	}
}

//------------------------------------------------------------------------
#ifdef VMEM_STATS
size_t FSA::WriteAllocs(int* p_allocs_written_count, int max_allocs_to_write, bool write_header) const
{
	if(write_header)
	{
		DebugWrite(_T("-------------------------\n"));
		DebugWrite(_T("FSA %d\n"), m_Size);
	}

	bool empty = true;

	int aligned_header_size = AlignUp(sizeof(FSAPageHeader), m_Alignment);

	int allocs_written = p_allocs_written_count ? *p_allocs_written_count : 0;

	int alloc_count = 0;
	size_t bytes_allocated = 0;

	// write all allocs in the full pages
	for(FSAPageHeader* p_page=m_FullPageList.mp_Next; p_page!=&m_FullPageList; p_page=p_page->mp_Next)
	{
		const byte* p_slot = (byte*)p_page + aligned_header_size;
		const byte* p_end_slot = p_slot + m_SlotsPerPage*m_SlotSize;
		for(; p_slot!=p_end_slot; p_slot+=m_SlotSize)
		{
			if(allocs_written < max_allocs_to_write)
			{
				WriteAlloc(p_slot, m_Size);
				++allocs_written;
				empty = false;
			}
			++alloc_count;
			bytes_allocated += m_Size;
		}
	}

	// write any allocs in the particallly free pages
	for(const FSAPageHeader* p_page = mp_FreePageList; p_page!=NULL; p_page=p_page->mp_Next)
	{
		const byte* p_slot = (byte*)p_page + aligned_header_size;
		const byte* p_end_slot = p_slot + m_SlotsPerPage*m_SlotSize;
		for(; p_slot!=p_end_slot; p_slot+=m_SlotSize)
		{
			// find out if this slot if on the free list
			bool allocated = true;
			for(void* p_iter=p_page->mp_FreeSlots; p_iter!=NULL; p_iter=*(void**)p_iter)
			{
				if(p_iter == p_slot)
				{
					allocated = false;
					break;
				}
			}

			if(allocated)
			{
				if(allocs_written < max_allocs_to_write)
				{
					WriteAlloc(p_slot, m_Size);
					++allocs_written;
				}
				++alloc_count;
				bytes_allocated += m_Size;
			}
		}
	}

	if(!empty)
	{
		if(!p_allocs_written_count)
		{
			if(allocs_written == max_allocs_to_write)
				DebugWrite(_T("Only showing first %d allocs\n"), max_allocs_to_write);

			DebugWrite(_T("%d bytes allocated across %d allocations\n"), bytes_allocated, alloc_count);
		}
	}

	if(p_allocs_written_count)
		*p_allocs_written_count = allocs_written;

	return bytes_allocated;
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
void FSA::SendStatsToMemPro(MemProSendFn send_fn, void* p_context)
{
	MemProStats::FSAStats stats;
	stats.m_Size = m_Size;
	stats.m_SlotSize = m_SlotSize;
	stats.m_Alignment = m_Alignment;
	stats.m_Stats = m_Stats;

	stats.m_FreePageCount = 0;
	for(FSAPageHeader* p_page=mp_FreePageList; p_page!=NULL; p_page=p_page->mp_Next)
		++stats.m_FreePageCount;

	stats.m_FullPageCount = 0;
	for(FSAPageHeader* p_page=m_FullPageList.mp_Next; p_page!=&m_FullPageList; p_page=p_page->mp_Next)
		++stats.m_FullPageCount;

	SendToMemPro(stats, send_fn, p_context);
}
#endif
//------------------------------------------------------------------------
// FSAHeap.cpp


//------------------------------------------------------------------------
// FSAHeap.hpp






//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	class InternalHeap;

	//------------------------------------------------------------------------
	// note: this class doesn't need to be thread safe. Once it's setup it's read only.
	template<class TFSA>
	class FSAHeap
	{
	public:
		typedef TFSA FSA;

		FSAHeap(PageHeap* p_page_heap, InternalHeap& internal_heap);

		~FSAHeap();

		void InitialiseFSA(int size, int alignment);

		inline void* Alloc(size_t size);

		inline bool Free(void* p);

		inline size_t GetSize(void* p) const;

		void CheckIntegrity() const;

#ifdef VMEM_INC_INTEG_CHECK
		void IncIntegrityCheck();
#endif

#ifdef VMEM_STATS
		Stats GetStats() const;
		Stats GetStatsNoLock() const;
		size_t WriteAllocs() const;
		void SendStatsToMemPro(MemProSendFn send_fn, void* p_context);
#endif
		inline TFSA* GetFSA(int size);

		void Lock() const;

		void Release() const;

	private:
		FSAHeap& operator=(const FSAHeap& other);

		//------------------------------------------------------------------------
		// data
	private:
		PageHeap* mp_PageHeap;

		static const int m_MaxFSASize = 1024;

		TFSA* m_FSAMap[m_MaxFSASize/4];

#ifdef VMEM_INC_INTEG_CHECK
		int m_FSAIntegCheckIndex;		// for incremental integrity checking
#endif
		InternalHeap& m_InternalHeap;

		Bitfield m_AllocatedFSAs;
	};

	//------------------------------------------------------------------------
	template<class TFSA>
	void* FSAHeap<TFSA>::Alloc(size_t size)
	{
		int i_size = ToInt(size);

		VMEM_ASSERT(i_size >= 0, "invalid size");
		int aligned_size = AlignUpPow2(i_size, 4);
		int index = aligned_size / 4;
		VMEM_ASSERT(index >= 0 && index < m_MaxFSASize/4, "FSA index out of range");
		VMEM_ASSERT(m_FSAMap[index], "FSA not setup for the specified size");

		return m_FSAMap[index]->Alloc();
	}

	//------------------------------------------------------------------------
	template<class TFSA>
	bool FSAHeap<TFSA>::Free(void* p)
	{
		if(mp_PageHeap->Owns(p))
		{
			TFSA* p_fsa = TFSA::GetFSA(p, mp_PageHeap->GetPageSize());
			p_fsa->Free(p);
			return true;
		}
		return false;
	}

	//------------------------------------------------------------------------
	template<class TFSA>
	size_t FSAHeap<TFSA>::GetSize(void* p) const
	{
		if(mp_PageHeap->Owns(p))
		{
			TFSA* p_fsa = TFSA::GetFSA(p, mp_PageHeap->GetPageSize());
			return p_fsa->GetSize();
		}
		return VMEM_INVALID_SIZE;
	}

	//------------------------------------------------------------------------
	template<class TFSA>
	TFSA* FSAHeap<TFSA>::GetFSA(int size)
	{
		int aligned_size = AlignUpPow2(size, 4);
		int index = aligned_size / 4;
		VMEM_ASSERT(index >= 0 && index < m_MaxFSASize/4, "FSA index out of range");
		VMEM_ASSERT(m_FSAMap[index], "FSA not setup for the specified size");

		return m_FSAMap[index];
	}
}




//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
template<class TFSA>
FSAHeap<TFSA>::FSAHeap(PageHeap* p_page_heap, InternalHeap& internal_heap)
:	mp_PageHeap(p_page_heap),
#ifdef VMEM_INC_INTEG_CHECK
	m_FSAIntegCheckIndex(0),
#endif
	m_InternalHeap(internal_heap),
	m_AllocatedFSAs(m_MaxFSASize, internal_heap)
{
	memset(m_FSAMap, 0, sizeof(m_FSAMap));
}

//------------------------------------------------------------------------
template<class TFSA>
FSAHeap<TFSA>::~FSAHeap()
{
	for(int i=0; i<m_MaxFSASize/4; ++i)
	{
		if(m_AllocatedFSAs.Get(i))
			m_InternalHeap.Delete(m_FSAMap[i]);
	}
}

//------------------------------------------------------------------------
template<class TFSA>
void FSAHeap<TFSA>::InitialiseFSA(int size, int alignment)
{
	int index = size / 4;
	VMEM_ASSERT_MEM(!m_FSAMap[index], &m_FSAMap[index]);		// FSA size already setup?

	TFSA* p_fsa = m_InternalHeap.New<TFSA>(size, alignment, mp_PageHeap);

	m_FSAMap[index] = p_fsa;
	m_AllocatedFSAs.Set(index);

	// set all the lower indices that haven't been setup up in map to use this fsa
	int i = index - 1;
	while(i >= 0 && !m_AllocatedFSAs.Get(i))
	{
		m_FSAMap[i] = p_fsa;
		--i;
	}
}

//------------------------------------------------------------------------
#ifdef VMEM_STATS
template<class TFSA>
Stats FSAHeap<TFSA>::GetStats() const
{
	Stats stats;

	for(int i=0; i<m_MaxFSASize/4; ++i)
	{
		if(m_AllocatedFSAs.Get(i))
		{
			stats += m_FSAMap[i]->GetStats();
			stats.m_Overhead += sizeof(TFSA);
		}
	}

	stats.m_Overhead += m_AllocatedFSAs.GetSizeInBytes();

	return stats;
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
template<class TFSA>
Stats FSAHeap<TFSA>::GetStatsNoLock() const
{
	Stats stats;

	for(int i=0; i<m_MaxFSASize/4; ++i)
	{
		if(m_AllocatedFSAs.Get(i))
		{
			stats += m_FSAMap[i]->GetStatsNoLock();
			stats.m_Overhead += sizeof(TFSA);
		}
	}

	stats.m_Overhead += m_AllocatedFSAs.GetSizeInBytes();

	return stats;
}
#endif

//------------------------------------------------------------------------
template<class TFSA>
void FSAHeap<TFSA>::CheckIntegrity() const
{
	for(int i=0; i<m_MaxFSASize/4; ++i)
	{
		if(m_AllocatedFSAs.Get(i))
			m_FSAMap[i]->CheckIntegrity();
	}
}

//------------------------------------------------------------------------
#ifdef VMEM_INC_INTEG_CHECK
template<class TFSA>
void FSAHeap<TFSA>::IncIntegrityCheck()
{
	int fsa_count = sizeof(m_FSAMap) / sizeof(TFSA*);

	int i = 0;
	while(!m_FSAMap[m_FSAIntegCheckIndex] && i++ < fsa_count)
		m_FSAIntegCheckIndex = (m_FSAIntegCheckIndex + 1) % fsa_count;

	TFSA* p_fsa = m_FSAMap[m_FSAIntegCheckIndex];
	if(p_fsa)
		p_fsa->CheckIntegrity();

	m_FSAIntegCheckIndex = (m_FSAIntegCheckIndex + 1) % fsa_count;
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
template<class TFSA>
size_t FSAHeap<TFSA>::WriteAllocs() const
{
	size_t allocated_bytes = 0;
	int allocs_written = 0;

	for(int i=0; i<m_MaxFSASize/4; ++i)
	{
		if(m_AllocatedFSAs.Get(i))
			allocated_bytes += m_FSAMap[i]->WriteAllocs(&allocs_written, WRITE_ALLOCS_MAX, false);
	}

	if(allocs_written == WRITE_ALLOCS_MAX)
		DebugWrite(_T("Only showing first %d allocs\n"), WRITE_ALLOCS_MAX);

	return allocated_bytes;
}
#endif

//------------------------------------------------------------------------
template<class TFSA>
void FSAHeap<TFSA>::Lock() const
{
	for(int i=0; i<m_MaxFSASize/4; ++i)
	{
		if(m_AllocatedFSAs.Get(i))
			m_FSAMap[i]->Lock();
	}
}

//------------------------------------------------------------------------
template<class TFSA>
void FSAHeap<TFSA>::Release() const
{
	for(int i=0; i<m_MaxFSASize/4; ++i)
	{
		if(m_AllocatedFSAs.Get(i))
			m_FSAMap[i]->Release();
	}
}

//------------------------------------------------------------------------
#ifdef VMEM_STATS
template<class TFSA>
void FSAHeap<TFSA>::SendStatsToMemPro(MemProSendFn send_fn, void* p_context)
{
	SendEnumToMemPro(vmem_FSAHeap, send_fn, p_context);

	MemProStats::FSAHeapStats stats;

	stats.m_FSACount = 0;
	for(int i=0; i<m_MaxFSASize/4; ++i)
	{
		if(m_AllocatedFSAs.Get(i))
			++stats.m_FSACount;
	}
	stats.m_Stats = GetStats();
	SendToMemPro(stats, send_fn, p_context);

	for(int i=0; i<m_MaxFSASize/4; ++i)
	{
		if(m_AllocatedFSAs.Get(i))
			m_FSAMap[i]->SendStatsToMemPro(send_fn, p_context);
	}
}
#endif

//------------------------------------------------------------------------
// explicit instanciation of FSAHeap for FSA and FSAMT
namespace VMem
{
	template class FSAHeap<FSA>;
	template class FSAHeap<FSAMT>;
}
//------------------------------------------------------------------------
// InternalHeap.cpp



//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
namespace VMem
{
	const int g_InternalHeapRegionSize = 16*1024;
}

//------------------------------------------------------------------------
InternalHeap::InternalHeap()
:	m_BasicCoalesceHeap(g_InternalHeapRegionSize)
{
}
//------------------------------------------------------------------------
// LargeHeap.cpp


//------------------------------------------------------------------------
// LargeHeap.hpp





//------------------------------------------------------------------------
// VMemHashMap.hpp
#ifndef VMEMHASHMAP_H_INCLUDED
#define VMEMHASHMAP_H_INCLUDED

//-----------------------------------------------------------------



//-----------------------------------------------------------------
//#define PROFILE_HASHMAP

//-----------------------------------------------------------------
#ifdef PROFILE_HASHMAP
	#include <time.h>
#endif

//-----------------------------------------------------------------
namespace VMem
{
	//-----------------------------------------------------------------
	template<typename TKey, typename TValue>
	class HashMap
	{
	public:
		//-----------------------------------------------------------------
		struct Pair
		{
			TKey m_Key;
			TValue m_Value;
		};

		//-----------------------------------------------------------------
		// Iterator used to iterate over the items of the set.
		// Must call MoveNext to move to the first element. To be used like
		// this:
		//	HashMap<TKey, TValue>::Iteartor iter(set);
		//	while(iter.MoveNext())
		//	{
		//		const HashMap<TKey, TValue>::Pair& pair = iter.GetValue();
		//		//...
		//	}
		class Iterator
		{
			friend class HashMap;

		public:
			//-----------------------------------------------------------------
			Iterator(const HashMap& hash_map)
			{
				Initialise(&hash_map);
			}

			//-----------------------------------------------------------------
			// if this iterator has reached the end do nothing and return false
			// otherwise move to the next item and return true.
			// The first time this is called moves to the first element.
			bool MoveNext()
			{
				const int capacity = mp_HashMap->m_Capacity;
				if(m_Index == capacity)
				{
					return false;
				}

				// move to the next non-empty item
				++m_Index;
				++mp_Pair;

				if(m_Index == capacity)
				{
					mp_Pair = NULL;
					return false;
				}

				while(!mp_HashMap->IsItemInUse(m_Index))
				{
					++m_Index;
					++mp_Pair;

					if(m_Index == capacity)
					{
						mp_Pair = NULL;
						return false;
					}
				}

				return true;
			}

			//-----------------------------------------------------------------
			const TKey& GetKey() const
			{
				VMEM_ASSERT(mp_Pair, "");		// Iterator not at a valid location
				return (*mp_Pair)->m_Key;
			}

			//-----------------------------------------------------------------
			TValue& GetValue()
			{
				VMEM_ASSERT(mp_Pair, "");		// Iterator not at a valid location
				return (*mp_Pair)->m_Value;
			}

		private:
			//-----------------------------------------------------------------
			void Initialise(const HashMap* p_hash_map)
			{
				mp_HashMap = p_hash_map;

				if(p_hash_map->m_Capacity)
				{
					m_Index = -1;
					mp_Pair = p_hash_map->mp_Table - 1;
				}
				else
				{
					m_Index = 0;
					mp_Pair = NULL;
				}
			}

			//-----------------------------------------------------------------
			// data
		private:
			const HashMap<TKey, TValue>* mp_HashMap;
			int m_Index;
			Pair** mp_Pair;
		};

		//-----------------------------------------------------------------
		// The default capacity of the set. The capacity is the number
		// of elements that the set is expected to hold. The set will resized
		// when the item count is greater than the capacity;
		HashMap(const int capacity=m_DefaultCapacity, int alloc_size=m_DefaultAllocSize)
		:	m_Capacity(0),
			mp_Table(NULL),
			m_Count(0),
			m_TableAllocSize(alloc_size),
			mp_ItemPool(NULL),
			mp_FreePair(NULL),
			m_AllocedMemory(0)
#ifdef PROFILE_HASHMAP
			,m_IterAcc(0)
			,m_IterCount(0)
#endif
		{
			if(capacity)
				AllocTable(GetNextPow2((256 * capacity) / m_Margin));
		}

		//-----------------------------------------------------------------
		~HashMap()
		{
			Clear();
		}

		//-----------------------------------------------------------------
		void Clear()
		{
			for(int i=0; i<m_Capacity; ++i)
			{
				Pair* p_pair = mp_Table[i];
				if(p_pair)
					p_pair->~Pair();
			}

			InternalFree(mp_Table, m_Capacity * sizeof(Pair*));

			m_Capacity = 0;
			mp_Table = NULL;
			m_Count = 0;

			FreePools();
		}

		//-----------------------------------------------------------------
		// Add a value to this set.
		// If this set already contains the value does nothing.
		void Add(const TKey& key, const TValue& value)
		{
			if(m_Capacity == 0 || m_Count == (m_Margin * m_Capacity) / 256)
			{
				Grow();
			}

			const int index = GetItemIndex(key);
			
			VMEM_ASSERT(mp_Table, "mp_Table is null");

			if(IsItemInUse(index))
			{
				mp_Table[index]->m_Value = value;
			}
			else
			{
				// make a copy of the value
				Pair* p_pair = AllocPair();
				p_pair->m_Key = key;
				p_pair->m_Value = value;

				// add to table
				mp_Table[index] = p_pair;

				++m_Count;
			}
		}

		//-----------------------------------------------------------------
		// if this set contains the value set value to the existing value and
		// return true, otherwise set to the default value and return false.
		bool TryGetValue(const TKey& key, TValue& value) const
		{
			if(!mp_Table)
				return false;

			const int index = GetItemIndex(key);
			if(IsItemInUse(index))
			{
				value = mp_Table[index]->m_Value;
				return true;
			}
			else
			{
				return false;
			}
		}

		//-----------------------------------------------------------------
		// If this set contains the specifed value remove it
		// and return true, otherwise do nothing and return false.
		TValue Remove(const TKey& key)
		{
			VMEM_ASSERT(mp_Table, "null table");

			int remove_index = GetItemIndex(key);
			VMEM_ASSERT(IsItemInUse(remove_index), "item no in use");

			Pair* p_pair = mp_Table[remove_index];

			TValue value = p_pair->m_Value;

			// find first index in this array
			int srch_index = remove_index;
			int first_index = remove_index;
			if(!srch_index)
			{
				srch_index = m_Capacity;
			}
			--srch_index;
			while(IsItemInUse(srch_index))
			{
				first_index = srch_index;
				if(!srch_index)
				{
					srch_index = m_Capacity;
				}
				--srch_index;
			}

			bool found = false;
			for(;;)
			{
				// find the last item in the array that can replace the item being removed
				int srch_index = (remove_index + 1) & (m_Capacity-1);

				int swap_index = m_InvalidIndex;
				while(IsItemInUse(srch_index))
				{
					const unsigned int srch_hash_code = mp_Table[srch_index]->m_Key.GetHashCode();
					const int srch_insert_index = srch_hash_code & (m_Capacity-1);

					if(InRange(srch_insert_index, first_index, remove_index))
					{
						swap_index = srch_index;
						found = true;
					}

					srch_index = (srch_index + 1) & (m_Capacity-1);
				}

				// swap the item
				if(found)
				{
					mp_Table[remove_index] = mp_Table[swap_index];
					remove_index = swap_index;
					found = false;
				}
				else
				{
					break;
				}
			}

			// remove the last item
			mp_Table[remove_index] = NULL;

			// free this item
			FreePair(p_pair);

			--m_Count;
			return value;
		}

		//-----------------------------------------------------------------
		Iterator GetIterator() const
		{
			return Iterator(*this);
		}

		//-----------------------------------------------------------------
		int GetCount() const
		{
			return m_Count;
		}

		//-----------------------------------------------------------------
		const TValue& operator[](const TKey& key) const
		{
			const int index = GetItemIndex(key);
			VMEM_ASSERT(IsItemInUse(index), "");
			return mp_Table[index]->m_Value;
		}

		//-----------------------------------------------------------------
		void Resize(int new_capacity)
		{
			new_capacity = GetNextPow2(new_capacity);

			// keep a copy of the old table
			Pair** const p_old_table = mp_Table;
			const int old_capacity = m_Capacity;

			// allocate the new table
			AllocTable(new_capacity);

			// copy the values from the old to the new table
			Pair** p_old_pair = p_old_table;
			for(int i=0; i<old_capacity; ++i, ++p_old_pair)
			{
				Pair* p_pair = *p_old_pair;
				if(p_pair)
				{
					const int index = GetItemIndex(p_pair->m_Key);
					mp_Table[index] = p_pair;
				}
			}

			if(p_old_table)
				InternalFree(p_old_table, old_capacity * sizeof(Pair*));
		}

		//-----------------------------------------------------------------
		int GetAllocedMemory() const
		{
			return m_AllocedMemory;
		}

	private:
		//-----------------------------------------------------------------
		void* InternalAlloc(int size)
		{
			size = AlignUpPow2(size, SYS_PAGE_SIZE);
			void* p = VMem::VirtualReserve(size);
			VMem::VirtualCommit(p, size);
			m_AllocedMemory += size;
			return p;
		}

		//-----------------------------------------------------------------
		void InternalFree(void* p, int size)
		{
			size = AlignUpPow2(size, SYS_PAGE_SIZE);
			VMem::VirtualDecommit(p, size);
			VMem::VirtualRelease(p, size);
			m_AllocedMemory -= size;
		}

		//-----------------------------------------------------------------
		static int GetNextPow2(int value)
		{
			int p = 2;
			while(p < value)
				p *= 2;
			return p;
		}

		//-----------------------------------------------------------------
		void AllocTable(const int capacity)
		{
			VMEM_ASSERT(capacity < m_MaxCapacity, "");
			m_Capacity = capacity;

			// allocate a block of memory for the table
			if(capacity > 0)
			{
				const int size = capacity * sizeof(Pair*);
				mp_Table = (Pair**)InternalAlloc(size);
				memset(mp_Table, 0, size);
			}
		}

		//-----------------------------------------------------------------
		bool IsItemInUse(const int index) const
		{
			return mp_Table[index] != NULL;
		}

		//-----------------------------------------------------------------
		int GetItemIndex(const TKey& key) const
		{
			VMEM_ASSERT(mp_Table, "");
			const unsigned int hash = key.GetHashCode();
			int srch_index = hash & (m_Capacity-1);
			while(IsItemInUse(srch_index) && !(mp_Table[srch_index]->m_Key == key))
			{
				srch_index = (srch_index + 1) & (m_Capacity-1);
#ifdef PROFILE_HASHMAP
				++m_IterAcc;
#endif
			}

#ifdef PROFILE_HASHMAP
			++m_IterCount;
			double average = m_IterAcc / (double)m_IterCount;
			if(average > 2.0)
			{
				static int last_write_time = 0;
				int now = clock();
				if(now - last_write_time > 1000)
				{
					last_write_time = now;
					DebugWrite("WARNING: HashMap average: %f\n", (float)average);
				}
			}
#endif
			return srch_index;
		}

		//-----------------------------------------------------------------
		// Increase the capacity of the table.
		void Grow()
		{
			const int new_capacity = m_Capacity ? 2*m_Capacity : m_DefaultCapacity;
			Resize(new_capacity);
		}

		//-----------------------------------------------------------------
		static bool InRange(
			const int index,
			const int start_index,
			const int end_index)
		{
			return (start_index <= end_index) ?
				index >= start_index && index <= end_index :
				index >= start_index || index <= end_index;
		}

		//-----------------------------------------------------------------
		void FreePools()
		{
			byte* p_pool = mp_ItemPool;
			while(p_pool)
			{
				byte* p_next_pool = *(byte**)p_pool;
				InternalFree(p_pool, m_DefaultAllocSize);
				p_pool = p_next_pool;
			}
			mp_ItemPool = NULL;
			mp_FreePair = NULL;
		}

		//-----------------------------------------------------------------
		Pair* AllocPair()
		{
			if(!mp_FreePair)
			{
				// allocate a new pool and link to pool list
				byte* p_new_pool = (byte*)InternalAlloc(m_DefaultAllocSize);
				*(byte**)p_new_pool = mp_ItemPool;
				mp_ItemPool = p_new_pool;

				// link all items onto free list
				mp_FreePair = p_new_pool + sizeof(Pair);
				byte* p = (byte*)mp_FreePair;
				int item_count = m_DefaultAllocSize / sizeof(Pair) - 2;	// subtract 2 for pool pointer and last item
				VMEM_ASSERT(item_count, "");
				for(int i=0; i<item_count; ++i, p+=sizeof(Pair))
				{
					*(byte**)p = p + sizeof(Pair);
				}
				*(byte**)p = NULL;
			}

			// take item off free list
			Pair* p_pair = (Pair*)mp_FreePair;
			mp_FreePair = *(byte**)mp_FreePair;

			// construct the pair
			new (p_pair)Pair;

			return p_pair;
		}

		//-----------------------------------------------------------------
		void FreePair(Pair* p_pair)
		{
			p_pair->~Pair();

			*(byte**)p_pair = mp_FreePair;
			mp_FreePair = (byte*)p_pair;
		}

		//-----------------------------------------------------------------
		// data
	private:
		static const int m_DefaultCapacity = 1024;
		static const int m_InvalidIndex = 0xffffffff;
		static const int m_MaxCapacity = 0x7fffffff;
		static const int m_Margin = (40 * 256) / 100;
		static const int m_DefaultAllocSize = 4096;

		int m_Capacity;			// the current capacity of this set, will always be >= m_Margin*m_Count/256
		Pair** mp_Table;		// NULL for a set with capacity 0
		int m_Count;			// the current number of items in this set, will always be <= m_Margin*m_Count/256

		int m_TableAllocSize;		// size of the blocks of memory to allocate for pairs
		byte* mp_ItemPool;
		byte* mp_FreePair;

		int m_AllocedMemory;

#ifdef PROFILE_HASHMAP
		mutable uint64 m_IterAcc;
		mutable uint64 m_IterCount;
#endif
	};

	//------------------------------------------------------------------------
	struct AddrKey
	{
		AddrKey() : m_Addr(0) {}

		AddrKey(void* p)
		:	m_Addr(p),
			m_HashCode(CalculateHashCode())
		{
		}

		bool operator==(const AddrKey& other) const { return m_Addr == other.m_Addr; }

		void* GetAddr() const { return m_Addr; }

		unsigned int GetHashCode() const { return m_HashCode; }

	private:
		unsigned int CalculateHashCode() const
		{
#ifdef VMEM64
			uint64 key = (uint64)m_Addr;
			key = (~key) + (key << 18);
			key = key ^ (key >> 31);
			key = key * 21;
			key = key ^ (key >> 11);
			key = key + (key << 6);
			key = key ^ (key >> 22);
			return (unsigned int)key;
#else
			unsigned int a = (unsigned int)m_Addr;
			a = (a ^ 61) ^ (a >> 16);
			a = a + (a << 3);
			a = a ^ (a >> 4);
			a = a * 0x27d4eb2d;
			a = a ^ (a >> 15);
			return a;
#endif
		}

		//------------------------------------------------------------------------
		// data
	private:
		void* m_Addr;
		unsigned int m_HashCode;
	};
}

//-----------------------------------------------------------------
#endif		// #ifndef VMEMHASHMAP_H_INCLUDED

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	class LargeHeap
	{
	public:
		~LargeHeap();

		void* Alloc(size_t size);

		bool Free(void* p);

		size_t GetSize(void* p) const;

#ifdef VMEM_STATS
		Stats GetStats() const;
		Stats GetStatsNoLock() const;
		size_t WriteAllocs();
		void SendStatsToMemPro(MemProSendFn send_fn, void* p_context);
#endif

		void Lock() const;

		void Release() const;

		//------------------------------------------------------------------------
		// data
	private:
		mutable VMem::CriticalSection m_CriticalSection;

		HashMap<AddrKey, size_t> m_Allocs;

#ifdef VMEM_STATS
		Stats m_Stats;
#endif
	};
}



//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
LargeHeap::~LargeHeap()
{
	HashMap<AddrKey, size_t>::Iterator iter = m_Allocs.GetIterator();
	while(iter.MoveNext())
	{
		void* p = (void*)iter.GetKey().GetAddr();
		Free(p);
	}
}

//------------------------------------------------------------------------
void* LargeHeap::Alloc(size_t size)
{
	CriticalSectionScope lock(m_CriticalSection);

	size_t aligned_size = AlignSizeUpPow2(size, SYS_PAGE_SIZE);
	void* p = VirtualMem::Reserve(aligned_size, SYS_PAGE_SIZE);
	if(!p)
		return NULL;

	if(!VirtualMem::Commit(p, aligned_size))
		return NULL;

	m_Allocs.Add(p, size);

	VMEMSET(p, ALLOCATED_MEM, aligned_size);

	VSTATS(m_Stats.m_Used += size);
	VSTATS(m_Stats.m_Unused += aligned_size - size);
	VSTATS(m_Stats.m_Reserved += aligned_size);

	return p;
}

//------------------------------------------------------------------------
bool LargeHeap::Free(void* p)
{
	CriticalSectionScope lock(m_CriticalSection);

	size_t alloc_size = 0;
	AddrKey key(p);
	if(!m_Allocs.TryGetValue(key, alloc_size))
		return false;

	size_t aligned_size = AlignSizeUpPow2(alloc_size, SYS_PAGE_SIZE);

	VirtualMem::Decommit(p, aligned_size);
	VirtualMem::Release(p);

	VSTATS(m_Stats.m_Used -= alloc_size);
	VSTATS(m_Stats.m_Unused -= aligned_size - alloc_size);
	VSTATS(m_Stats.m_Reserved -= aligned_size);

	m_Allocs.Remove(key);

	return true;
}

//------------------------------------------------------------------------
size_t LargeHeap::GetSize(void* p) const
{
	CriticalSectionScope lock(m_CriticalSection);

	size_t size = 0;
	if(m_Allocs.TryGetValue(p, size))
		return size;

	return VMEM_INVALID_SIZE;
}

//------------------------------------------------------------------------
#ifdef VMEM_STATS
size_t LargeHeap::WriteAllocs()
{
	CriticalSectionScope lock(m_CriticalSection);

	DebugWrite(_T("-------------------------\n"));
	DebugWrite(_T("LargeHeap\n"));

	int written_allocs = 0;
	int alloc_count = 0;
	size_t allocated_bytes = 0;

	HashMap<AddrKey, size_t>::Iterator iter = m_Allocs.GetIterator();
	while(iter.MoveNext())
	{
		void* p = (void*)iter.GetKey().GetAddr();
		size_t size = iter.GetValue();

		if(written_allocs != WRITE_ALLOCS_MAX)
		{
			WriteAlloc(p, size);
			++written_allocs;
		}
		++alloc_count;
		allocated_bytes += size;
	}

	if(written_allocs == WRITE_ALLOCS_MAX)
		DebugWrite(_T("Only showing first %d allocations\n"), WRITE_ALLOCS_MAX);

	DebugWrite(_T("%d bytes allocated across %d allocations\n"), allocated_bytes, alloc_count);

	return allocated_bytes;
}
#endif

//------------------------------------------------------------------------
void LargeHeap::Lock() const
{
	m_CriticalSection.Enter();
}

//------------------------------------------------------------------------
void LargeHeap::Release() const
{
	m_CriticalSection.Leave();
}

//------------------------------------------------------------------------
#ifdef VMEM_STATS
void LargeHeap::SendStatsToMemPro(MemProSendFn send_fn, void* p_context)
{
	CriticalSectionScope lock(m_CriticalSection);

	MemProStats::LageHeapStats stats;

	stats.m_AllocCount = 0;
	HashMap<AddrKey, size_t>::Iterator iter = m_Allocs.GetIterator();
	while(iter.MoveNext())
		++stats.m_AllocCount;

	stats.m_Stats = m_Stats;

	SendEnumToMemPro(vmem_LargeHeap, send_fn, p_context);
	SendToMemPro(stats, send_fn, p_context);
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
Stats LargeHeap::GetStats() const
{
	CriticalSectionScope lock(m_CriticalSection);
	return GetStatsNoLock();
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
Stats LargeHeap::GetStatsNoLock() const
{
	int map_size = m_Allocs.GetAllocedMemory();

	Stats internal_stats;
	internal_stats.m_Reserved += map_size;
	internal_stats.m_Overhead += map_size;

	return m_Stats + internal_stats;
}
#endif
//------------------------------------------------------------------------
// PageHeap.cpp







//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
#ifdef VMEM_MEMORY_CORRUPTION_FINDER
	typedef NoReleaseBitfield PageBitfield;
#else
	typedef Bitfield PageBitfield;
#endif

	//------------------------------------------------------------------------
#ifdef USE_LOCKFREE_REGION_ARRAY
	enum { DefaultRegionsArrayCapacity = 4 };
	enum { RegionArrayEndMarker = 0xffffffff };
	enum { RegionsArrayGrowMultiplier = 2 };
#endif

	//------------------------------------------------------------------------
	struct PageHeapRegion
	{
		PageHeapRegion(int page_count, InternalHeap& internal_heap) : mp_Mem(NULL), m_PageBitfield(page_count, internal_heap), mp_Next(NULL) {}

		void* mp_Mem;
		PageBitfield m_PageBitfield;
		PageHeapRegion* mp_Next;
	};
}

//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
PageHeap::PageHeap(int page_size, int region_size, InternalHeap& internal_heap)
:	m_PageSize(page_size),
	m_SysPageSize(Max(page_size, SYS_PAGE_SIZE)),
	m_RegionSize(AlignUpPow2(region_size, m_PageSize)),
	m_PagesPerRegion(region_size / m_PageSize),
	m_PagesPerSysPage(m_SysPageSize / page_size),
	mp_RegionList(NULL),
#ifdef USE_LOCKFREE_REGION_ARRAY
	m_RegionsArrayCapacity(DefaultRegionsArrayCapacity),
	mp_DiscardedRegionArrays(NULL),
#endif
	m_InternalHeap(internal_heap)
{
	VMEM_ASSERT((m_SysPageSize % page_size) == 0, "page size must be a multple of the system page size or vice versa");
	VMEM_ASSERT((m_SysPageSize % SYS_PAGE_SIZE) == 0, "page size must be a multple of the system page size or vice versa");

#ifdef USE_LOCKFREE_REGION_ARRAY
	int array_size = m_RegionsArrayCapacity * sizeof(void*);
	mp_Regions = (void**)m_InternalHeap.Alloc(array_size);
	memset(mp_Regions, 0, array_size);
	mp_Regions[m_RegionsArrayCapacity-1] = (void*)RegionArrayEndMarker;
	VSTATS(m_Stats.m_Overhead += array_size);
#endif
}

//------------------------------------------------------------------------
PageHeap::~PageHeap()
{
	while(mp_RegionList)
		DestroyRegion(mp_RegionList);

#ifdef USE_LOCKFREE_REGION_ARRAY
	m_InternalHeap.Free(mp_Regions);
	VSTATS(m_Stats.m_Overhead -= m_RegionsArrayCapacity*sizeof(void*));

	void* p = mp_DiscardedRegionArrays;
	VSTATS(int size = DefaultRegionsArrayCapacity);		// keep track of each arrays size so that we can update stats
	while(p)
	{
		void* p_next = *(void**)p;
		m_InternalHeap.Free(p);
		p = p_next;

		VSTATS(m_Stats.m_Overhead -= size);
		VSTATS(size *= RegionsArrayGrowMultiplier);
	}
#endif
}

//------------------------------------------------------------------------
void* PageHeap::Alloc()
{
	CriticalSectionScope lock(m_CriticalSection);

	// find the first region that has a free page
	PageHeapRegion* p_region = mp_RegionList;
	PageHeapRegion* p_prev_region = NULL;
	while(p_region && p_region->m_PageBitfield.GetCount() == m_PagesPerRegion)
	{
		p_prev_region = p_region;
		p_region = p_region->mp_Next;
	}

	// no region found so create a new one and add to the end of the list
	if(!p_region)
	{
		p_region = CreateRegion();
		if(!p_region)
			return NULL;

		if(p_prev_region)
			p_prev_region->mp_Next = p_region;
		else
			mp_RegionList = p_region;
	}

	int page_index = p_region->m_PageBitfield.GetFirstClearBit();
	void* p_page = (byte*)p_region->mp_Mem + page_index * m_PageSize;

	// commit the sys page if it's clear (there can be multiple pages in a system page)
	if(SysPageClear(page_index, p_region))
	{
		if(!VirtualMem::Commit(AlignDownPow2(p_page, SYS_PAGE_SIZE), m_SysPageSize))
			return NULL;		// out of memory
		VSTATS(m_Stats.m_Unused += m_SysPageSize);

		for(int i=0; i<m_PagesPerSysPage; ++i)
			VMEMSET((byte*)p_page + i*m_PageSize, UNUSED_PAGE, m_PageSize);
	}

	CheckMemory(p_page, m_PageSize, UNUSED_PAGE);

	p_region->m_PageBitfield.Set(page_index);

	VSTATS(m_Stats.m_Unused -= m_PageSize);
	VSTATS(m_Stats.m_Used += m_PageSize);

	return p_page;
}

//------------------------------------------------------------------------
void PageHeap::Free(void* p_page)
{
	CriticalSectionScope lock(m_CriticalSection);

	// get the region
	PageHeapRegion* p_region = GetRegion(p_page);
	VMEM_ASSERT(p_region, "can't find region for page");

	// clear the bitfield bit
	int index = ToInt(((byte*)p_page - (byte*)p_region->mp_Mem) / m_PageSize);
	p_region->m_PageBitfield.Clear(index);

	VSTATS(m_Stats.m_Unused += m_PageSize);
	VSTATS(m_Stats.m_Used -= m_PageSize);

	// decommit the sys page if it's clear (there can be multiple pages in a system page)
	if(SysPageClear(index, p_region))
	{
		VirtualMem::Decommit(AlignDownPow2(p_page, SYS_PAGE_SIZE), m_SysPageSize);
		VSTATS(m_Stats.m_Unused -= m_SysPageSize);
	}
	else
	{
		VMEMSET(p_page, UNUSED_PAGE, m_PageSize);
	}

	if(!p_region->m_PageBitfield.GetCount())
		DestroyRegion(p_region);
}

//------------------------------------------------------------------------
PageHeapRegion* PageHeap::CreateRegion()
{
	PageHeapRegion* p_region = m_InternalHeap.New<PageHeapRegion>(m_PagesPerRegion, m_InternalHeap);
	if(!p_region)
		return NULL;

	p_region->mp_Mem = VirtualMem::Reserve(m_RegionSize, m_SysPageSize);
	if(!p_region->mp_Mem)
		return NULL;

	p_region->mp_Next = NULL;

	VSTATS(m_Stats.m_Reserved += m_RegionSize);
	VSTATS(m_Stats.m_Overhead += sizeof(PageHeapRegion));
	VSTATS(m_Stats.m_Overhead += p_region->m_PageBitfield.GetSizeInBytes());

#if defined(VMEM_ALTERNATE_PAGES) && !defined(VMEM_MEMORY_CORRUPTION_FINDER)
	int sys_pages_per_region = m_RegionSize / m_SysPageSize;
	for(int i=1; i<sys_pages_per_region; i+=2)		// start from 1 so that we always have at least one valid page
	{
		for(int a=0; a<m_PagesPerSysPage; ++a)
			p_region->m_PageBitfield.Set(i*m_PagesPerSysPage + a);
	}
#endif

#ifdef USE_LOCKFREE_REGION_ARRAY
	AddRegionToRegionsArray(p_region);
#endif

	return p_region;
}

//------------------------------------------------------------------------
void PageHeap::DestroyRegion(PageHeapRegion* p_region)
{
#ifdef USE_LOCKFREE_REGION_ARRAY
	RemoveRegionFromRegionsArray(p_region);
#endif

	// unlink from region list
	if(mp_RegionList == p_region)
	{
		mp_RegionList = p_region->mp_Next;
	}
	else
	{
		VMEM_ASSERT(mp_RegionList, "unable to find region");
		PageHeapRegion* p = mp_RegionList;
		while(p->mp_Next != p_region)
		{
			p = p->mp_Next;
			VMEM_ASSERT(p, "unable to find region");
		}
		p->mp_Next = p_region->mp_Next;
	}

#if defined(VMEM_ALTERNATE_PAGES) && !defined(VMEM_MEMORY_CORRUPTION_FINDER)
	int sys_pages_per_region = m_RegionSize / m_SysPageSize;
	for(int i=1; i<sys_pages_per_region; i+=2)		// remember to start from 1
	{
		for(int a=0; a<m_PagesPerSysPage; ++a)
			p_region->m_PageBitfield.Clear(i*m_PagesPerSysPage + a);
	}
#endif

	// decommit pages
	byte* p_page = (byte*)p_region->mp_Mem;
	for(int i=0; i<m_PagesPerRegion; i+=m_PagesPerSysPage, p_page+=m_SysPageSize)
	{
		if(!SysPageClear(i, p_region))
			VirtualMem::Decommit(p_page, m_SysPageSize);
	}

	// release the memory
	VirtualMem::Release(p_region->mp_Mem);

	VSTATS(m_Stats.m_Reserved -= m_RegionSize);
	VSTATS(m_Stats.m_Overhead -= sizeof(PageHeapRegion));
	VSTATS(m_Stats.m_Overhead -= p_region->m_PageBitfield.GetSizeInBytes());

	m_InternalHeap.Delete<PageHeapRegion>(p_region);
}

//------------------------------------------------------------------------
PageHeapRegion* PageHeap::GetRegion(void* p) const
{
	for(PageHeapRegion* p_region=mp_RegionList; p_region!=NULL; p_region=p_region->mp_Next)
	{
		void* p_mem = p_region->mp_Mem;
		if(p >= p_mem && p < (byte*)p_mem + m_RegionSize)
			return p_region;
	}
	return NULL;
}

//------------------------------------------------------------------------
bool PageHeap::Owns(void* p) const
{
#ifdef USE_LOCKFREE_REGION_ARRAY
	void** pp_region = mp_Regions;
	while(*pp_region != (void*)RegionArrayEndMarker)
	{
		void* p_mem = *pp_region;
		if(p_mem && p >= p_mem && p < (byte*)p_mem + m_RegionSize)
			return true;
		++pp_region;
	}
	return false;
#else
	CriticalSectionScope lock(m_CriticalSection);

	for(PageHeapRegion* p_region=mp_RegionList; p_region!=NULL; p_region=p_region->mp_Next)
	{
		void* p_mem = p_region->mp_Mem;
		if(p >= p_mem && p < (byte*)p_mem + m_RegionSize)
			return true;
	}
	return false;
#endif
}

//------------------------------------------------------------------------
// we can always assume that the index passed in is clear, which covers the
// case of m_PagesPerSysPage being zero.
bool PageHeap::SysPageClear(int index, PageHeapRegion* p_region) const
{
	const PageBitfield& bitfield = p_region->m_PageBitfield;

	int start_index = AlignDownPow2(index, m_PagesPerSysPage);
	for(int i=0; i<m_PagesPerSysPage; ++i)
	{
		if(bitfield.Get(start_index + i))
			return false;
	}

	return true;
}

//------------------------------------------------------------------------
// The mp_Regions list is designed to be lock free with respect to the Owns
// function. For this reason when resizing the array we do not delete the old
// array in case another thread is accessing it. It will be cleaned up though
// when this class destructs.
#ifdef USE_LOCKFREE_REGION_ARRAY
void PageHeap::AddRegionToRegionsArray(PageHeapRegion* p_region)
{
	int capacity = m_RegionsArrayCapacity;
	int i = 1;	// first region is never used becuase it is used as a link in the discarded list
	while(mp_Regions[i])
	{
		++i;
		if(i == capacity-1)
		{
			// create a new array
			int new_capacity = RegionsArrayGrowMultiplier * capacity;	
			int new_array_size = new_capacity * sizeof(void*);
			void** p_new_array = (void**)m_InternalHeap.Alloc(new_array_size);

			// copy the old array to this array
			int old_capacity = capacity-1;				// -1 so we don't copy end marker
			memcpy_s(p_new_array, new_array_size, mp_Regions, old_capacity*sizeof(void*));
			memset(p_new_array+old_capacity, 0, (new_capacity-1-old_capacity)*sizeof(void*));
			p_new_array[new_capacity-1] = (void*)RegionArrayEndMarker;

			// add to discarded list
			mp_Regions[0] = mp_DiscardedRegionArrays;
			mp_DiscardedRegionArrays = mp_Regions;

			// point to the new array
			mp_Regions = p_new_array;
			m_RegionsArrayCapacity = new_capacity;

			VSTATS(m_Stats.m_Overhead += new_array_size);
			break;
		}
	}
	mp_Regions[i] = p_region->mp_Mem;
}
#endif

//------------------------------------------------------------------------
#ifdef USE_LOCKFREE_REGION_ARRAY
void PageHeap::RemoveRegionFromRegionsArray(PageHeapRegion* p_region)
{
	int i = 0;
	void* p_mem = p_region->mp_Mem;
	while(mp_Regions[i] != p_mem)
	{
		++i;
		VMEM_ASSERT(i < m_RegionsArrayCapacity-1, "region not found");
	}
	mp_Regions[i] = NULL;
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
void PageHeap::SendStatsToMemPro(MemProSendFn send_fn, void* p_context)
{
	CriticalSectionScope lock(m_CriticalSection);

	MemProStats::PageHeapStats stats;
	stats.m_PageSize = m_PageSize;
	stats.m_RegionSize = m_RegionSize;

	stats.m_RegionCount = 0;
	for(PageHeapRegion* p_region = mp_RegionList; p_region!=NULL; p_region=p_region->mp_Next)
		++stats.m_RegionCount;

	stats.m_Stats = m_Stats;

	SendEnumToMemPro(vmem_PageHeap, send_fn, p_context);
	SendToMemPro(stats, send_fn, p_context);
}
#endif
//------------------------------------------------------------------------
// ProtectedHeap.cpp


//------------------------------------------------------------------------
// ProtectedHeap.hpp
#ifndef PROTECTED_HEAP_H_INCLUDED
#define PROTECTED_HEAP_H_INCLUDED

//------------------------------------------------------------------------


//------------------------------------------------------------------------
#ifdef VMEM_PROTECTED_HEAP





#ifdef VMEM_OS_UNIX
	#include <pthread.h>
	#include <sys/time.h>
#endif

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	class ProtectedHeap
	{
	public:
		ProtectedHeap();
		
		~ProtectedHeap();

		void* Alloc(size_t size);

		bool Free(void* p);

		size_t GetSize(void* p) const;

		void CheckIntegrity();

		bool FreeReservedMemory();

		void Lock() const { m_CriticalSection.Enter(); }

		void Release() const { m_CriticalSection.Leave(); }

#ifdef VMEM_STATS
		Stats GetStats() const;
		Stats GetStatsNoLock() const;
#endif
	private:
		void AddUnreleasedPage(void* p_page, size_t size);

		void* AllocInternal(size_t size, bool& out_of_memory);

		ProtectedHeap& operator=(const ProtectedHeap&);

		//------------------------------------------------------------------------
		// data
	private:
		HashMap<AddrKey, size_t> m_Allocs;

		mutable CriticalSection m_CriticalSection;

		int m_AllocIndex;

		size_t m_Overhead;

		struct UnreleasedAllocsPage
		{
			UnreleasedAllocsPage* mp_Prev;
			UnreleasedAllocsPage* mp_Next;
		};
		struct UnreleasedAlloc
		{
			void* mp_Page;
			size_t m_Size;
		};
		UnreleasedAllocsPage m_UnreleasedAllocsPageList;		// list of linked pages contains lists of unreleased memory allocs
		int m_UnreleasedAllocCount;
		const int m_UnreleasedAllocsPerPage;
		int m_UnreservedBlockIndex;

#ifdef VMEM_STATS
		Stats m_Stats;
#endif
	};
}

//------------------------------------------------------------------------
#endif		// #ifdef VMEM_PROTECTED_HEAP

//------------------------------------------------------------------------
#endif		// #ifndef PROTECTED_HEAP_H_INCLUDED

//------------------------------------------------------------------------
#ifdef VMEM_PROTECTED_HEAP

//------------------------------------------------------------------------


//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
namespace VMem
{
	const int g_ProtectedAllocAlignment = 16;

	extern size_t g_ReservedBytes;

	typedef bool (*VMemSHouldProtectFn)(int i, size_t size);
	extern VMemSHouldProtectFn g_VMemSHouldProtectFn;

	inline bool ShouldProtect(int i, size_t size)
	{
		if(g_VMemSHouldProtectFn)
			return g_VMemSHouldProtectFn(i, size);
		else
			return VMemShouldProtect(i, size);
	}

	//------------------------------------------------------------------------
	inline void CheckMemoryBytes(void* p, size_t size, unsigned int value)
	{
#ifdef VMEM_ASSERTS
		byte* p_check = (byte*)p;
		for(size_t i=0; i<size; ++i, ++p_check)
			VMEM_MEM_CHECK(p_check, (byte)value);
#else
		VMEM_UNREFERENCED_PARAM(p);
		VMEM_UNREFERENCED_PARAM(size);
		VMEM_UNREFERENCED_PARAM(value);
#endif
	}
}

//------------------------------------------------------------------------
ProtectedHeap::ProtectedHeap()
:	m_AllocIndex(0),
	m_Overhead(0),
	m_UnreleasedAllocCount(0),
	m_UnreleasedAllocsPerPage((SYS_PAGE_SIZE - sizeof(UnreleasedAllocsPage)) / sizeof(UnreleasedAlloc)),
	m_UnreservedBlockIndex(0)
{
	m_UnreleasedAllocCount = m_UnreleasedAllocsPerPage;

	m_UnreleasedAllocsPageList.mp_Prev = &m_UnreleasedAllocsPageList;
	m_UnreleasedAllocsPageList.mp_Next = &m_UnreleasedAllocsPageList;
}

//------------------------------------------------------------------------
ProtectedHeap::~ProtectedHeap()
{
	while(FreeReservedMemory())
		;
}

//------------------------------------------------------------------------
void* ProtectedHeap::Alloc(size_t size)
{
	CriticalSectionScope lock(m_CriticalSection);

	if(!ShouldProtect(m_AllocIndex, size))
		return NULL;

	bool out_of_memory = false;

	void* p = AllocInternal(size, out_of_memory);

	while(out_of_memory && FreeReservedMemory())
	{
		out_of_memory = false;
		p = AllocInternal(size, out_of_memory);
	}

	VMEM_ASSERT(!out_of_memory, "out of memory");

	return p;
}

//------------------------------------------------------------------------
void* ProtectedHeap::AllocInternal(size_t size, bool& out_of_memory)
{
	++m_AllocIndex;

	if(size == 0)
		size = 4;

	size_t aligned_size = AlignUpPow2(size, g_ProtectedAllocAlignment);

	size_t page_aligned_size = AlignUpPow2(aligned_size, SYS_PAGE_SIZE);
	if(m_Overhead + page_aligned_size - size >= VMEM_PROTECTED_HEAP_SIZE)
		return NULL;

	void* p_page = VMem::VirtualReserve(page_aligned_size);
	if(!p_page)
	{
		out_of_memory = true;
		return NULL;
	}
	VMem::VirtualCommit(p_page, page_aligned_size);

	size_t alloc_offset = page_aligned_size - aligned_size;
	void* p_alloc = (byte*)p_page + alloc_offset;
	if(alloc_offset)
		memset(p_page, PROTECTED_PAGE, alloc_offset);

	size_t end_guard_size = aligned_size - size;
	if(end_guard_size)
		memset((byte*)p_page + page_aligned_size - end_guard_size, PROTECTED_PAGE, end_guard_size);

	m_Allocs.Add(p_alloc, size);

	size_t overhead = page_aligned_size - size;
	m_Overhead += overhead;

#ifdef VMEM_STATS
	m_Stats.m_Reserved += page_aligned_size;
	m_Stats.m_Used += size;
	m_Stats.m_Overhead += overhead;
#endif

	return p_alloc;
}

//------------------------------------------------------------------------
bool ProtectedHeap::Free(void* p)
{
	CriticalSectionScope lock(m_CriticalSection);

	void* p_alloc = p;

	size_t size = 0;
	AddrKey key(p_alloc);
	if(!m_Allocs.TryGetValue(key, size))
		return false;

	m_Allocs.Remove(key);

	size_t aligned_size = AlignUpPow2(size, g_ProtectedAllocAlignment);
	size_t page_aligned_size = AlignUpPow2(aligned_size, SYS_PAGE_SIZE);

	size_t alloc_offset = page_aligned_size - aligned_size;

	void* p_page = (byte*)p_alloc - alloc_offset;

	if(alloc_offset)
		CheckMemory(p_page, alloc_offset, PROTECTED_PAGE);

	size_t end_guard_size = aligned_size - size;
	if(end_guard_size)
		CheckMemoryBytes((byte*)p_page + page_aligned_size - end_guard_size, end_guard_size, PROTECTED_PAGE);

	// we only decommit, do not release. This protects the page and
	// stops the virtual address being used again
	VMem::VirtualDecommit(p_page, page_aligned_size);

	size_t overhead = page_aligned_size - size;
	m_Overhead -= overhead;

	AddUnreleasedPage(p_page, page_aligned_size);

#ifdef VMEM_STATS
	m_Stats.m_Used -= size;
	m_Stats.m_Overhead -= overhead;
#endif
	return true;
}

//------------------------------------------------------------------------
size_t ProtectedHeap::GetSize(void* p) const
{
	CriticalSectionScope lock(m_CriticalSection);

	size_t size = 0;
	if(m_Allocs.TryGetValue(p, size))
		return size;

	return VMEM_INVALID_SIZE;
}

//------------------------------------------------------------------------
void ProtectedHeap::AddUnreleasedPage(void* p_page, size_t size)
{
	if(m_UnreleasedAllocCount == m_UnreleasedAllocsPerPage)
	{
		void* p_new_page = VMem::VirtualReserve(SYS_PAGE_SIZE);
		if(!p_new_page)
		{
			// in an out of memory situation release the memory immediatley
			VirtualRelease(p_page, size);
#ifdef VMEM_STATS
			m_Stats.m_Reserved -= size;
#endif
			return;
		}

		VMem::VirtualCommit(p_new_page, SYS_PAGE_SIZE);
		memset(p_new_page, 0, SYS_PAGE_SIZE);
		
		UnreleasedAllocsPage* p_block_page = (UnreleasedAllocsPage*)p_new_page;

		// add to the end of the list
		p_block_page->mp_Next = &m_UnreleasedAllocsPageList;
		p_block_page->mp_Prev = m_UnreleasedAllocsPageList.mp_Prev;
		p_block_page->mp_Prev->mp_Next = p_block_page;
		m_UnreleasedAllocsPageList.mp_Prev = p_block_page;

		m_UnreleasedAllocCount = 0;
		
		m_Overhead += SYS_PAGE_SIZE;

#ifdef VMEM_STATS
		m_Stats.m_Reserved += SYS_PAGE_SIZE;
		m_Stats.m_Overhead += SYS_PAGE_SIZE;
#endif
	}
	
	// add to the last block page
	UnreleasedAlloc* p_unreleased_locks = (UnreleasedAlloc*)((byte*)m_UnreleasedAllocsPageList.mp_Prev+sizeof(UnreleasedAllocsPage));
	UnreleasedAlloc* p_unreleased_block = p_unreleased_locks + m_UnreleasedAllocCount;
	
	p_unreleased_block->mp_Page = p_page;
	p_unreleased_block->m_Size = size;

	++m_UnreleasedAllocCount;
}

//------------------------------------------------------------------------
bool ProtectedHeap::FreeReservedMemory()
{
	CriticalSectionScope lock(m_CriticalSection);

	// check if there are any block pages
	if(m_UnreleasedAllocsPageList.mp_Next == &m_UnreleasedAllocsPageList)
		return false;

	// get the next block to release from the first block page
	UnreleasedAlloc* p_unreleased_locks = (UnreleasedAlloc*)((byte*)m_UnreleasedAllocsPageList.mp_Next+sizeof(UnreleasedAllocsPage));
	UnreleasedAlloc unreleased_block = p_unreleased_locks[m_UnreservedBlockIndex];

	// release the block
	if(unreleased_block.mp_Page)
	{
		VirtualRelease(unreleased_block.mp_Page, unreleased_block.m_Size);
#ifdef VMEM_STATS
		m_Stats.m_Reserved -= unreleased_block.m_Size;
#endif
	}

	++m_UnreservedBlockIndex;

	// if all blocks in the current page have been totally released release the page
	if(m_UnreservedBlockIndex == m_UnreleasedAllocsPerPage)
	{
		// unlink the page
		UnreleasedAllocsPage* p_page = m_UnreleasedAllocsPageList.mp_Next;
		p_page->mp_Prev->mp_Next = p_page->mp_Next;
		p_page->mp_Next->mp_Prev = p_page->mp_Prev;

		// free the page
		VMem::VirtualDecommit(p_page, SYS_PAGE_SIZE);
		VMem::VirtualRelease(p_page, SYS_PAGE_SIZE);

		// if this is the last page set m_UnreleasedAllocCount to the maximum
		// so that the next time we allocate a new page
		if(m_UnreleasedAllocsPageList.mp_Next == &m_UnreleasedAllocsPageList)
			m_UnreleasedAllocCount = m_UnreleasedAllocsPerPage;

		m_UnreservedBlockIndex = 0;

		m_Overhead -= SYS_PAGE_SIZE;

#ifdef VMEM_STATS
		m_Stats.m_Reserved -= SYS_PAGE_SIZE;
		m_Stats.m_Overhead -= SYS_PAGE_SIZE;
#endif
	}

	return true;
}

//------------------------------------------------------------------------
void ProtectedHeap::CheckIntegrity()
{
	CriticalSectionScope lock(m_CriticalSection);

	HashMap<AddrKey, size_t>::Iterator iter = m_Allocs.GetIterator();
	while(iter.MoveNext())
	{
		void* p_alloc = (void*)iter.GetKey().GetAddr();
		size_t size = iter.GetValue();

		size_t aligned_size = AlignUpPow2(size, g_ProtectedAllocAlignment);
		size_t page_aligned_size = AlignUpPow2(aligned_size, SYS_PAGE_SIZE);

		size_t alloc_offset = page_aligned_size - aligned_size;

		void* p_page = (byte*)p_alloc - alloc_offset;

		if(alloc_offset)
			CheckMemory(p_page, alloc_offset, PROTECTED_PAGE);

		size_t end_guard_size = aligned_size - size;
		if(end_guard_size)
			CheckMemoryBytes((byte*)p_page + page_aligned_size - end_guard_size, end_guard_size, PROTECTED_PAGE);
	}
}

//------------------------------------------------------------------------
#ifdef VMEM_STATS
Stats ProtectedHeap::GetStats() const
{
	CriticalSectionScope lock(m_CriticalSection);
	return GetStatsNoLock();
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
Stats ProtectedHeap::GetStatsNoLock() const
{
	int map_size = m_Allocs.GetAllocedMemory();

	Stats internal_stats;
	internal_stats.m_Reserved += map_size;
	internal_stats.m_Overhead += map_size;

	return m_Stats + internal_stats;
}
#endif

//------------------------------------------------------------------------
#endif		// #ifdef VMEM_DELAYED_RELEASE
//------------------------------------------------------------------------
// TrailGuard.cpp




//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	inline int GetAllocSize(void* p, int fixed_alloc_size)
	{
		return fixed_alloc_size != -1 ? fixed_alloc_size : *(int*)((VMem::byte*)p + sizeof(void*));
	}
}

//------------------------------------------------------------------------
TrailGuard::TrailGuard()
:	m_Size(0),
	m_CurSize(0),
	m_FixedAllocSize(0),
	mp_Head(NULL),
	mp_Tail(NULL),
	m_FullCheckFreq(0),
	m_FullCheckCounter(0)
{
}

//------------------------------------------------------------------------
void TrailGuard::Initialise(int size, int check_freq, int alloc_size)
{
	m_Size = size;
	m_FullCheckFreq = check_freq;
	m_FixedAllocSize = alloc_size;
}

//------------------------------------------------------------------------
void* TrailGuard::Shutdown()
{
	void* p_trail = mp_Head;
	mp_Head = NULL;
	mp_Tail = NULL;
	m_Size = 0;
	m_CurSize = 0;
	return p_trail;
}

//------------------------------------------------------------------------
void* TrailGuard::Add(void* p, int size)
{
	if(!m_Size) return NULL;

	// add alloc to tail
	if(mp_Tail)
	{
		*(void**)mp_Tail = p;
		mp_Tail = p;
	}
	else
	{
		mp_Head = mp_Tail = p;
	}
	*(void**)p = NULL;

	// set the guard
	int guard_size = size - sizeof(void*);
	void* p_guard = (byte*)p + sizeof(void*);
	if(m_FixedAllocSize == -1)
	{
		VMEM_ASSERT((unsigned int)size >= sizeof(void*) + sizeof(int), "Variable size guards must be >= sizeof(void*) + sizeof(int)");
		guard_size -= sizeof(int);
		*(int*)p_guard = size;
		p_guard = (byte*)p_guard + sizeof(int);

	}
	if(guard_size)
		memset(p_guard, TRAIL_GUARD_MEM, guard_size);

	// increase trail size
	m_CurSize += size;

	// check the entire trail
	if(m_FullCheckCounter == m_FullCheckFreq)
	{
		m_FullCheckCounter = 0;
		CheckIntegrity();
	}
	++m_FullCheckCounter;

	// return NULL if trail has not reached limit yet
	if(m_CurSize < m_Size)
		return NULL;

	// remove alloc from head
	void* new_p = mp_Head;
	mp_Head = *(void**)new_p;

	int new_alloc_size = GetAllocSize(new_p, m_FixedAllocSize);
	m_CurSize -= new_alloc_size;

	int new_guard_size = new_alloc_size - sizeof(void*);
	void* p_new_guard = (byte*)new_p + sizeof(void*);
	if(m_FixedAllocSize == -1)
	{
		new_guard_size -= sizeof(int);
		p_new_guard = (byte*)p_new_guard + sizeof(int);
	}

	// check that the guard is still intact
	CheckMemory(p_new_guard, new_guard_size, TRAIL_GUARD_MEM);

	return new_p;
}

//------------------------------------------------------------------------
void TrailGuard::CheckIntegrity() const
{
	VMEM_ASSERT_CODE(int total_trail_size = 0);
	void* p_alloc = mp_Head;

	while(p_alloc)
	{
		int size = GetAllocSize(p_alloc, m_FixedAllocSize);

		void* p_guard = (byte*)p_alloc + sizeof(void*);
		int guard_size = size - sizeof(void*);

		if(m_FixedAllocSize == -1)
		{
			guard_size -= sizeof(int);
			p_guard = (byte*)p_guard + sizeof(int);
		}

		CheckMemory(p_guard, guard_size, TRAIL_GUARD_MEM);

		VMEM_ASSERT_CODE(total_trail_size += size);
		VMEM_ASSERT_CODE(VMEM_ASSERT(total_trail_size <= m_CurSize, "Trail guard fail"));

		// get the next alloc
		p_alloc = *(void**)p_alloc;
	}

	VMEM_ASSERT_CODE(VMEM_ASSERT(total_trail_size == m_CurSize, "Trail guard fail"));
}

//------------------------------------------------------------------------
bool TrailGuard::Contains(void* p) const
{
	void* p_alloc = mp_Head;

	while(p_alloc)
	{
		if(p_alloc == p)
			return true;
		p_alloc = *(void**)p_alloc;
	}

	return false;
}
//------------------------------------------------------------------------
// VirtualMem.cpp




//------------------------------------------------------------------------
// VMemThread.hpp
#ifndef VMEMTHREAD_H_INCLUDED
#define VMEMTHREAD_H_INCLUDED

//------------------------------------------------------------------------


//------------------------------------------------------------------------
#ifdef VMEM_DELAYED_RELEASE

#ifdef VMEM_OS_UNIX
	#include <pthread.h>
	#include <sys/time.h>
#endif

//------------------------------------------------------------------------
#ifdef VMEM_OS_WIN
	#pragma warning(push)
	#pragma warning(disable : 4100)
#endif

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	typedef int (*ThreadMain)();

#if defined(VMEM_OS_WIN)
	typedef HANDLE ThreadHandle;
#elif defined(VMEM_OS_UNIX)
	typedef pthread_t ThreadHandle;
#else
	#error	// OS not defined
#endif

	//------------------------------------------------------------------------
	namespace Thread
	{
		ThreadHandle CreateThread(ThreadMain p_thread_main);

		void TerminateThread(ThreadHandle thread);

		void WaitForThreadToTerminate(ThreadHandle thread);
	}

	//------------------------------------------------------------------------
	class Event
	{
	public:
		//------------------------------------------------------------------------
		Event(bool initial_state, bool auto_reset)
		{
#ifdef VMEM_OS_WIN
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

		//------------------------------------------------------------------------
		~Event()
		{
#ifdef VMEM_OS_WIN
			CloseHandle(m_Handle);
#else
			pthread_mutex_destroy(&m_Mutex);
			pthread_cond_destroy(&m_Cond);
#endif
		}

		//------------------------------------------------------------------------
		void Set() const
		{
#ifdef VMEM_OS_WIN
			SetEvent(m_Handle);
#else
			pthread_mutex_lock(&m_Mutex);
			m_Signalled = true;
			pthread_mutex_unlock(&m_Mutex);
			pthread_cond_signal(&m_Cond);
#endif
		}

		//------------------------------------------------------------------------
		void Reset()
		{
#ifdef VMEM_OS_WIN
			ResetEvent(m_Handle);
#else
			pthread_mutex_lock(&m_Mutex);
			m_Signalled = false;
			pthread_mutex_unlock(&m_Mutex);
#endif
		}

		//------------------------------------------------------------------------
		int Wait(int timeout=-1) const
		{
#ifdef VMEM_OS_WIN
			#ifdef MEMPRO_PLATFORM_GAMES_CONSOLE_2
	#error Please contact slynch@puredevsoftware.com for this platform
			#else
				VMEM_STATIC_ASSERT(INFINITE == -1);
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
#ifdef VMEM_OS_WIN
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
#ifdef VMEM_OS_WIN
	#pragma warning(pop)
#endif

//------------------------------------------------------------------------
#endif		// #ifdef VMEM_DELAYED_RELEASE

//------------------------------------------------------------------------
#endif		// #ifndef VMEMTHREAD_H_INCLUDED



//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	const int m_ReleaseNodePoolBlockSize = 4096;

	volatile bool g_ProcessRunning = true;

	static const int g_VirtualMemNodeBucketCount = 256;

	//------------------------------------------------------------------------
	struct VirtualMemNode
	{
		size_t m_Size;
		size_t m_AlignedSize;
		void* mp_Mem;
		VirtualMemNode* mp_Prev;
		VirtualMemNode* mp_Next;
		int m_Align;
		int m_FlushFrame;
	};

	//------------------------------------------------------------------------
	struct DecommitNode
	{
		void* mp_Mem;
		size_t m_Size;
		DecommitNode* mp_Prev;
		DecommitNode* mp_Next;
		int m_FlushFrame;
	};

	//------------------------------------------------------------------------
	template<typename T>
	class NodePool
	{
	public:
		//------------------------------------------------------------------------
		NodePool()
		:	mp_FreeList(NULL),
			mp_Blocks(NULL),
			m_BlockCount(0)
		{
		}

		//------------------------------------------------------------------------
		~NodePool()
		{
			void* p_block = mp_Blocks;
			while(p_block)
			{
				void* p_next = *(void**)p_block;
				VMem::VirtualDecommit(p_block, m_ReleaseNodePoolBlockSize);
				VMem::VirtualRelease(p_block, m_ReleaseNodePoolBlockSize);
				p_block = p_next;
			}
		}

		//------------------------------------------------------------------------
		T* Alloc()
		{
			if(!mp_FreeList)
			{
				void* p_block = VMem::VirtualReserve(m_ReleaseNodePoolBlockSize);
				VMem::VirtualCommit(p_block, m_ReleaseNodePoolBlockSize);

				memset(p_block, 0, m_ReleaseNodePoolBlockSize);

				*(void**)p_block = mp_Blocks;
				mp_Blocks = p_block;
				++m_BlockCount;

				int block_count = m_ReleaseNodePoolBlockSize / sizeof(T) - 1;

				void* p_start = (byte*)p_block + sizeof(T);
				void* p_end = (byte*)p_block + block_count*sizeof(T);
				
				void* p = p_start;

				while(p != p_end)
				{
					void* p_next = (byte*)p + sizeof(T);
					*(void**)p = p_next;
					p = p_next;
				}

				mp_FreeList = p_start;
			}

			void* p_node = mp_FreeList;
			mp_FreeList = *(void**)p_node;
			return (T*)p_node;
		}

		//------------------------------------------------------------------------
		void Free(T* p_node)
		{
			*(void**)p_node = mp_FreeList;
			mp_FreeList = p_node;
		}

		//------------------------------------------------------------------------
		int GetMemory() const
		{
			return m_BlockCount * m_ReleaseNodePoolBlockSize;
		}

		//------------------------------------------------------------------------
		// data
	private:
		void* mp_FreeList;
		void* mp_Blocks;
		int m_BlockCount;
	};

	//------------------------------------------------------------------------
	template<class TNode>
	void InitialiseNodeList(TNode& node_list)
	{
		memset(&node_list, 0, sizeof(TNode));
		node_list.mp_Prev = &node_list;
		node_list.mp_Next = &node_list;
	}

	//------------------------------------------------------------------------
	template<class TNode>
	void UnlinkNode(TNode* p_node)
	{
		p_node->mp_Prev->mp_Next = p_node->mp_Next;
		p_node->mp_Next->mp_Prev = p_node->mp_Prev;
	}

	//------------------------------------------------------------------------
	template<class TNode>
	void InsertNode(TNode* p_prev_node, TNode* p_new_node)
	{
		TNode* p_next_node = p_prev_node->mp_Next;
		
		p_new_node->mp_Prev = p_prev_node;
		p_new_node->mp_Next = p_next_node;

		p_prev_node->mp_Next = p_new_node;
		p_next_node->mp_Prev = p_new_node;
	}

	//------------------------------------------------------------------------
	struct VirtualAlloc
	{
		size_t m_Size;
		size_t m_AlignedSize;
		void* mp_BaseAddr;
		int m_Align;
	};

	//------------------------------------------------------------------------
	struct VirtualMemImp
	{
	public:
		VirtualMemImp();

		void Destroy(bool clean);

		void CreatePulseThread();

		void* Reserve(size_t size, int align);

		void Release(void* p);

		bool Commit(void* p, size_t size);

		void Decommit(void* p, size_t size);

		Stats GetStats();

		void Lock();

		void Release();

	private:
		bool TryCommit(void*& p_commit, size_t& commit_size);

		static int PulseThreadMainStatic();
		void PulseThreadMain();

		void FlushMemory(bool force);

		DecommitNode* FindDecommitNode(void* p, size_t size) const;

		size_t GetPoolMemory() const { return m_ReleaseNodePool.GetMemory() + m_DecommitNodePool.GetMemory(); }

		void CheckIntegrity();

		//------------------------------------------------------------------------
		// data
	private:
		VirtualMemNode m_VirtualNodeBuckets[g_VirtualMemNodeBucketCount];

		size_t m_UnusedBytes;
		size_t m_ReservedBytesOverhead;
		size_t m_DelayedReservedBytes;

		CriticalSection m_CriticalSection;

		HashMap<AddrKey, VirtualAlloc> m_Allocs;

		NodePool<VirtualMemNode> m_ReleaseNodePool;
		NodePool<DecommitNode> m_DecommitNodePool;

		DecommitNode m_DecommitNodeList;		// sorted by p_node->mp_Mem

		int m_FlushFrame;

#ifdef VMEM_DELAYED_RELEASE
		ThreadHandle m_PulseThread;
		Event m_WakeEvent;
#endif
	};

	//------------------------------------------------------------------------
	byte g_VirtualMemImpMem[sizeof(VirtualMemImp)] ALIGN_8;
	VirtualMemImp* gp_VirtualMemImp = (VirtualMemImp*)g_VirtualMemImpMem;
}

//------------------------------------------------------------------------
VirtualMemImp::VirtualMemImp()
:	m_UnusedBytes(0),
	m_ReservedBytesOverhead(0),
	m_DelayedReservedBytes(0),
	m_Allocs(0),
	m_FlushFrame(0)
#ifdef VMEM_DELAYED_RELEASE
	,m_PulseThread(ThreadHandle())
	,m_WakeEvent(false, true)
#endif
{
	g_ProcessRunning = true;

	InitialiseNodeList(m_DecommitNodeList);

	for(int i=0; i<g_VirtualMemNodeBucketCount; ++i)
		InitialiseNodeList(m_VirtualNodeBuckets[i]);
}

//------------------------------------------------------------------------
void VirtualMemImp::Destroy(bool clean)
{
	if(clean)
	{
		m_CriticalSection.Enter();
		g_ProcessRunning = false;
		m_CriticalSection.Leave();

	#ifdef VMEM_DELAYED_RELEASE
		m_WakeEvent.Set();
		Thread::WaitForThreadToTerminate(m_PulseThread);
		m_PulseThread = ThreadHandle();
	#endif

		FlushMemory(true);
	}
	else
	{
	#ifdef VMEM_DELAYED_RELEASE
		Thread::TerminateThread(m_PulseThread);
	#endif
	}
}

//------------------------------------------------------------------------
void VirtualMemImp::CreatePulseThread()
{
#ifdef VMEM_DELAYED_RELEASE
	m_PulseThread = Thread::CreateThread(PulseThreadMainStatic);
#endif
}

//------------------------------------------------------------------------
void VirtualMem::CreatePulseThread()
{
	gp_VirtualMemImp->CreatePulseThread();
}

//------------------------------------------------------------------------
void VirtualMem::Initialise()
{
	VMEM_ASSERT(!VMem::GetCommittedBytes(), "VMem didn't clean up properly");
	VMEM_ASSERT(!VMem::GetReservedBytes(), "VMem didn't clean up properly");

	new (gp_VirtualMemImp)VirtualMemImp();
}

//------------------------------------------------------------------------
void VirtualMem::Destroy(bool clean)
{
	gp_VirtualMemImp->Destroy(clean);
	if(clean)
		gp_VirtualMemImp->~VirtualMemImp();
	else
		memset(gp_VirtualMemImp, 0, sizeof(VirtualMemImp));

	if(clean)
	{
		VMEM_ASSERT(!VMem::GetCommittedBytes(), "VMem didn't clean up properly");
		VMEM_ASSERT(!VMem::GetReservedBytes(), "VMem didn't clean up properly");
	}
}

//------------------------------------------------------------------------
void* VirtualMemImp::Reserve(size_t size, int align)
{
	CriticalSectionScope lock(m_CriticalSection);

	VMEM_ASSERT(size > 0 && (size & (SYS_PAGE_SIZE-1)) == 0, "Invalid size passed to VirtualReserve");

	void* p = NULL;

	size_t aligned_size = size;

	int bucket_index = (int)(size / (1024*1024));
	if(bucket_index < g_VirtualMemNodeBucketCount)
	{
		VirtualMemNode& node_list = m_VirtualNodeBuckets[bucket_index];
		VirtualMemNode* p_node = node_list.mp_Next;
		while(p_node != &node_list)
		{
			if(p_node->m_Size == size && p_node->m_Align == align)
			{
				p = p_node->mp_Mem;
				UnlinkNode(p_node);
				m_ReleaseNodePool.Free(p_node);
				m_DelayedReservedBytes -= size;
				aligned_size = p_node->m_AlignedSize;
				break;
			}

			p_node = p_node->mp_Next;
		}
	}

	bool needs_aligning = align != SYS_PAGE_SIZE;

	if(!p)
	{
		if(needs_aligning)
		{
			VMEM_ASSERT((align & (SYS_PAGE_SIZE-1)) == 0, "bad alignment");
			aligned_size = AlignSizeUpPow2(aligned_size + align-1, SYS_PAGE_SIZE);
		}

		p = VMem::VirtualReserve(aligned_size);

		if(!p)
		{
			FlushMemory(true);
			p = VMem::VirtualReserve(aligned_size);
			if(!p)
				return NULL;
		}

		m_ReservedBytesOverhead += aligned_size - size;
	}

	VirtualAlloc virtual_alloc;
	virtual_alloc.mp_BaseAddr = p;
	virtual_alloc.m_Align = align;
	virtual_alloc.m_Size = size;
	virtual_alloc.m_AlignedSize = aligned_size;

	if(needs_aligning)
		p = AlignUpPow2((byte*)p, align);

	m_Allocs.Add(p, virtual_alloc);

	return p;
}

//------------------------------------------------------------------------
void VirtualMemImp::Release(void* p)
{
	CriticalSectionScope lock(m_CriticalSection);

	VMEM_ASSERT(p != NULL && ((size_t)p & (SYS_PAGE_SIZE-1)) == 0, "Invalid address passed to VirtualRelease");

	VirtualAlloc virtual_alloc = m_Allocs.Remove(p);
	p = virtual_alloc.mp_BaseAddr;
	size_t size = virtual_alloc.m_AlignedSize;
	int align = virtual_alloc.m_Align;

	VMEM_ASSERT(size > 0 && (size & (SYS_PAGE_SIZE-1)) == 0, "Invalid size passed to VirtualRelease");

	int bucket_index = (int)(size / (1024*1024));
	if(bucket_index < g_VirtualMemNodeBucketCount)
	{
		VirtualMemNode* p_node = m_ReleaseNodePool.Alloc();
		p_node->mp_Mem = p;
		p_node->m_Size = virtual_alloc.m_Size;
		p_node->m_AlignedSize = virtual_alloc.m_AlignedSize;
		p_node->m_Align = align;
		p_node->mp_Next = NULL;
		p_node->mp_Prev = NULL;
		p_node->m_FlushFrame = m_FlushFrame + 1;

		InsertNode(&m_VirtualNodeBuckets[bucket_index], p_node);

		m_DelayedReservedBytes += virtual_alloc.m_Size;

		#ifndef VMEM_DELAYED_RELEASE
			FlushMemory(true);
		#endif
	}
	else
	{
		VMem::VirtualRelease(p, virtual_alloc.m_AlignedSize);

		m_ReservedBytesOverhead -= virtual_alloc.m_AlignedSize - virtual_alloc.m_Size;
	}
}

//------------------------------------------------------------------------
DecommitNode* VirtualMemImp::FindDecommitNode(void* p, size_t size) const
{
	void* p0 = p;
	void* p1 = (byte*)p + size;

	DecommitNode* p_node = m_DecommitNodeList.mp_Next;
	while(p_node != &m_DecommitNodeList)
	{
		void* n0 = p_node->mp_Mem;
		void* n1 = (byte*)n0 + p_node->m_Size;

		if((p0 >= n0 && p0 < n1) || (n0 >= p0 && n0 < p1))
			return p_node;

		p_node = p_node->mp_Next;
	}

	return NULL;
}

//------------------------------------------------------------------------
bool VirtualMemImp::Commit(void* p, size_t size)
{
	CriticalSectionScope lock(m_CriticalSection);

	VMEM_ASSERT(p != NULL && ((size_t)p & (SYS_PAGE_SIZE-1)) == 0, "Invalid address passed to VirtualCommit");
	VMEM_ASSERT(size >= 0 && (size & (SYS_PAGE_SIZE-1)) == 0, "Invalid size passed to VirtualCommit");

	void* p_commit = p;
	size_t commit_size = size;

	bool succeeded = TryCommit(p_commit, commit_size);
	if(!succeeded)
	{
		FlushMemory(true);
		succeeded = TryCommit(p_commit, commit_size);
	}

	if(succeeded)
	{
		VMEMSET(p, COMMITTED_MEM, size);
		VMEM_ASSERT_CODE(VMEM_ASSERT(VMem::Committed(p, size), ""));
	}

	return succeeded;
}

//------------------------------------------------------------------------
bool VirtualMemImp::TryCommit(void*& p_commit, size_t& commit_size)
{
	DecommitNode* p_node = FindDecommitNode(p_commit, commit_size);

	if(!p_node)
		return VMem::VirtualCommit(p_commit, commit_size);

	// handle the common case of re-committing the same memory
	if(p_node->mp_Mem == p_commit && p_node->m_Size == commit_size)
	{
		UnlinkNode(p_node);
		m_DecommitNodePool.Free(p_node);
		m_UnusedBytes -= commit_size;
		CheckMemory(p_commit, commit_size, DECOMMITTED_MEM);
		return true;
	}
	else
	{
		// handle overlap

		// handle the case where p_commit is in the middle of a node
		if(p_node->mp_Mem < p_commit)
		{
			CheckMemory(p_node->mp_Mem, p_node->m_Size, DECOMMITTED_MEM);

			// only commit as much of the node as we need
			void* p_node_end = (byte*)p_node->mp_Mem + p_node->m_Size;
			size_t size_to_commit = Min((size_t)((byte*)p_node_end - (byte*)p_commit), commit_size);

			p_node->m_Size = (byte*)p_commit - (byte*)p_node->mp_Mem;

			p_commit = (byte*)p_commit + size_to_commit;

			commit_size -= size_to_commit;
			m_UnusedBytes -= size_to_commit;

			// if we have some left over, create a new node
			if(p_commit < p_node_end)
			{
				DecommitNode* p_new_node = m_DecommitNodePool.Alloc();
				p_new_node->mp_Mem = p_commit;
				p_new_node->m_Size = (byte*)p_node_end - (byte*)p_commit;
				InsertNode(p_node, p_new_node);

				#ifdef VMEM_MEMSET_ONLY_SMALL
					VMEMSET(p_new_node->mp_Mem, DECOMMITTED_MEM, p_new_node->m_Size);
				#endif
			}

			p_node = p_node->mp_Next;
		}

		// keep committing nodes and the gaps until we have committed enough
		while(commit_size && p_node != &m_DecommitNodeList)
		{
			DecommitNode* p_next_node = p_node->mp_Next;

			CheckMemory(p_node->mp_Mem, p_node->m_Size, DECOMMITTED_MEM);

			void* p_node_start = p_node->mp_Mem;

			// if the node starts part way through the block commit the range before the node
			if(p_commit < p_node_start)
			{
				size_t size_to_commit = Min((size_t)((byte*)p_node_start - (byte*)p_commit), commit_size);
				if(!VMem::VirtualCommit(p_commit, size_to_commit))
					return false;

				p_commit = (byte*)p_commit + size_to_commit;
				commit_size -= size_to_commit;
			}

			if(commit_size)
			{
				// only commit as much of the node as we need
				size_t size_to_commit = Min(p_node->m_Size, commit_size);

				p_node->mp_Mem = (byte*)p_node->mp_Mem + size_to_commit;
				p_node->m_Size -= size_to_commit;

				p_commit = (byte*)p_commit + size_to_commit;
				commit_size -= size_to_commit;
				m_UnusedBytes -= size_to_commit;

				if(p_node->m_Size)
				{
					#ifdef VMEM_MEMSET_ONLY_SMALL
						VMEMSET(p_node->mp_Mem, DECOMMITTED_MEM, p_node->m_Size);
					#endif
				}
				else
				{
					UnlinkNode(p_node);
					m_DecommitNodePool.Free(p_node);
				}
			}
			p_node = p_next_node;
		}

		if(commit_size)
		{
			if(!VMem::VirtualCommit(p_commit, commit_size))
				return false;
		}

		return true;
	}
}

//------------------------------------------------------------------------
void VirtualMemImp::Decommit(void* p, size_t size)
{
	CriticalSectionScope lock(m_CriticalSection);

	VMEM_ASSERT(p != NULL && ((size_t)p & (SYS_PAGE_SIZE-1)) == 0, "Invalid address passed to VirtualDecommit");
	VMEM_ASSERT(size >= 0 && (size & (SYS_PAGE_SIZE-1)) == 0, "Invalid size passed to VirtualDecommit");
	VMEM_ASSERT_CODE(VMEM_ASSERT(VMem::Committed(p, size), "Can't decommit range because it is not committed"));
	
	VMEMSET(p, DECOMMITTED_MEM, size);

	void* p_end = (byte*)p + size;

	m_UnusedBytes += size;

	DecommitNode* p_node = m_DecommitNodeList.mp_Next;
	while(p_node != &m_DecommitNodeList)
	{
		if(p_node->mp_Mem >= p_end)
			break;

		p_node = p_node->mp_Next;
	}

	DecommitNode* p_new_node = m_DecommitNodePool.Alloc();
	p_new_node->mp_Mem = p;
	p_new_node->m_Size = size;
	p_new_node->m_FlushFrame = m_FlushFrame + 1;
	InsertNode(p_node->mp_Prev, p_new_node);

	#ifndef VMEM_DELAYED_RELEASE
		FlushMemory(true);
	#endif
}

//------------------------------------------------------------------------
int VirtualMemImp::PulseThreadMainStatic()
{
	gp_VirtualMemImp->PulseThreadMain();
	return 0;
}

//------------------------------------------------------------------------
void VirtualMemImp::PulseThreadMain()
{
#ifdef VMEM_DELAYED_RELEASE
	while(g_ProcessRunning)
	{
		m_WakeEvent.Wait(VMEM_DELAYED_RELEASE);
		FlushMemory(false);
		++m_FlushFrame;
	}
#endif
}

//------------------------------------------------------------------------
void VirtualMemImp::FlushMemory(bool force)
{
	CriticalSectionScope lock(m_CriticalSection);

	if(!m_FlushFrame)
		force = true;	// to handle wrap around

	// flush decommit
	DecommitNode* p_node = m_DecommitNodeList.mp_Next;
	while(p_node != &m_DecommitNodeList)
	{
		DecommitNode* p_next_node = p_node->mp_Next;

		if(m_FlushFrame >= p_node->m_FlushFrame || force)
		{
			size_t size = p_node->m_Size;

			VMem::VirtualDecommit(p_node->mp_Mem, size);

			m_UnusedBytes -= size;

			UnlinkNode(p_node);
			m_DecommitNodePool.Free(p_node);
		}

		p_node = p_next_node;
	}

	// flush release
	for(int i=0; i<g_VirtualMemNodeBucketCount; ++i)
	{
		VirtualMemNode& node_list = m_VirtualNodeBuckets[i];
		VirtualMemNode* p_node = node_list.mp_Next;
		while(p_node != &node_list)
		{
			VirtualMemNode* p_next = p_node->mp_Next;

			if(m_FlushFrame >= p_node->m_FlushFrame || force)
			{
				size_t size = p_node->m_Size;
				size_t aligned_size = p_node->m_AlignedSize;

				VMem::VirtualRelease(p_node->mp_Mem, aligned_size);

				m_DelayedReservedBytes -= size;
				m_ReservedBytesOverhead -= aligned_size - size;

				UnlinkNode(p_node);
				m_ReleaseNodePool.Free(p_node);
			}

			p_node = p_next;
		}
	}
}

//------------------------------------------------------------------------
Stats VirtualMemImp::GetStats()
{
	CriticalSectionScope lock(m_CriticalSection);

	size_t pool_mem = GetPoolMemory();
	size_t hashmap_mem = m_Allocs.GetAllocedMemory();

	Stats stats;
	stats.m_Overhead = pool_mem + hashmap_mem;
	stats.m_Reserved = m_ReservedBytesOverhead + stats.m_Overhead + m_DelayedReservedBytes;
	stats.m_Unused = m_UnusedBytes;
	return stats;
}

//------------------------------------------------------------------------
void VirtualMemImp::Lock()
{
	m_CriticalSection.Enter();
}

//------------------------------------------------------------------------
void VirtualMemImp::Release()
{
	m_CriticalSection.Leave();
}

//------------------------------------------------------------------------
void VirtualMemImp::CheckIntegrity()
{
#ifdef VMEM_ASSERTS
	DecommitNode* p_node = m_DecommitNodeList.mp_Next;
	while(p_node != &m_DecommitNodeList)
	{
		CheckMemory(p_node->mp_Mem, p_node->m_Size, DECOMMITTED_MEM);
		p_node = p_node->mp_Next;
	}
#endif
}

//------------------------------------------------------------------------
void* VirtualMem::Reserve(size_t size, int align)
{
	return gp_VirtualMemImp->Reserve(size, align);
}

//------------------------------------------------------------------------
void VirtualMem::Release(void* p)
{
	gp_VirtualMemImp->Release(p);
}

//------------------------------------------------------------------------
bool VirtualMem::Commit(void* p, size_t size)
{
	return gp_VirtualMemImp->Commit(p, size);
}

//------------------------------------------------------------------------
void VirtualMem::Decommit(void* p, size_t size)
{
	gp_VirtualMemImp->Decommit(p, size);
}

//------------------------------------------------------------------------
Stats VirtualMem::GetStats()
{
	return gp_VirtualMemImp->GetStats();
}

//------------------------------------------------------------------------
void VirtualMem::Lock()
{
	gp_VirtualMemImp->Lock();
}

//------------------------------------------------------------------------
void VirtualMem::Release()
{
	gp_VirtualMemImp->Release();
}
//------------------------------------------------------------------------
// VMemAlloc.cpp



//------------------------------------------------------------------------
// VMemHeap.hpp








#ifdef VMEM_PROTECTED_HEAP
	#include "ProtectedHeap.hpp"
#endif

//------------------------------------------------------------------------
#define VMEMHEAP_THREAD_SAFE
//#define VMEMHEAP_GLOBAL_LOCK

//------------------------------------------------------------------------
#ifdef VMEMHEAP_GLOBAL_LOCK
	#define GLOBAL_LOCK CriticalSectionScope lock(m_CriticalSection);
#else
	#define GLOBAL_LOCK
#endif

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
#ifdef VMEMHEAP_THREAD_SAFE
	typedef FSAHeap<FSAMT> FSAHeapT;
	typedef CoalesceHeapMT CoalesceHeapT;
#else
	typedef FSAHeap<FSA> FSAHeapT;
	typedef CoalesceHeap CoalesceHeapT;
#endif

	//------------------------------------------------------------------------
	class VMemHeap
	{
	public:
		//------------------------------------------------------------------------
		struct Settings
		{
			Settings();

			int m_FSAHeap1_PageSize;
			int m_FSAHeap1_RegionSize;
			int m_FSAHeap1_MaxSize;

			int m_FSAHeap2_PageSize;
			int m_FSAHeap2_RegionSize;
			int m_FSAHeap2_MaxSize;

			int m_CoalesceHeap1_RegionSize;
			int m_CoalesceHeap1_MinSize;
			int m_CoalesceHeap1_MaxSize;

			int m_CoalesceHeap2_RegionSize;
			int m_CoalesceHeap2_MinSize;
			int m_CoalesceHeap2_MaxSize;
		};

		//------------------------------------------------------------------------
		VMemHeap(Settings& settings);

		void* Alloc(size_t size);

		void Free(void* p);

		size_t GetSize(void* p);

		void CheckIntegrity();

#ifdef VMEM_STATS
		VMemHeapStats GetStats();
		void WriteStats();
		size_t WriteAllocs();
		void SendStatsToMemPro(MemProSendFn send_fn, void* p_context);
#endif

		void Lock() const;

		void Release() const;

	private:
		void InitialiseFSAHeap1();
		void InitialiseFSAHeap2();

		void InitialiseTrailGuards();

		void CheckStats() const;

#ifdef VMEM_STATS
		VMemHeapStats GetStatsInternal();
		VMemHeapStats GetStatsNoLock() const;
#endif

#ifdef VMEM_INC_INTEG_CHECK
		void IncIntegrityCheck();
#endif

		//------------------------------------------------------------------------
		// data
	private:
		Settings m_Settings;

		InternalHeap m_InternalHeap;

		PageHeap m_FSA1PageHeap;
		PageHeap m_FSA2PageHeap;

		FSAHeapT m_FSAHeap1;
		FSAHeapT m_FSAHeap2;

		CoalesceHeapT m_CoalesceHeap1;
		CoalesceHeapT m_CoalesceHeap2;

		LargeHeap m_LargeHeap;

#ifdef VMEMHEAP_GLOBAL_LOCK
		CriticalSection m_CriticalSection;
#endif

#ifdef VMEM_PROTECTED_HEAP
		ProtectedHeap m_ProtectedHeap;
#endif
	};
}


//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
// write each allocation and free out to a binary file. Each operation is of
// the form <pointer (4 bytes), size (4 bytes)>
// Free operations are identified by a size of VMEM_INVALID_SIZE
// Playing back the exact allocations and frees can be useful for tracking down bugs.
//#define RECORD_ALLOCS

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	// we do it this way to avoid order of static initialisation problems
	VMem::byte g_VMemHeapMem[sizeof(VMemHeap)] ALIGN_8;
	VMemHeap* gp_VMemHeap = NULL;

	//------------------------------------------------------------------------
	class TeardownIntegrityCheck
	{
	public:
		~TeardownIntegrityCheck() { CheckIntegrity(); }
	};

	TeardownIntegrityCheck g_TeardownIntegrityCheck;

	//------------------------------------------------------------------------
#ifdef VMEM_PROTECTED_HEAP
	VMemSHouldProtectFn g_VMemSHouldProtectFn = NULL;
#endif

	//------------------------------------------------------------------------
#ifdef RECORD_ALLOCS
	VMem::byte g_RecordCritSecMem[sizeof(CriticalSection)] ALIGN_8;
	CriticalSection& g_RecordCritSec = (CriticalSection&)g_RecordCritSecMem;
	FILE* gp_RecordFile = NULL;
	void InitialiseRecording()
	{
		new (&g_RecordCritSec)CriticalSection();
		fopen_s(&gp_RecordFile, "allocs.bin", "wb");
	}
#endif

	//------------------------------------------------------------------------
#ifdef RECORD_ALLOCS
	void UninitialiseRecording()
	{
		fclose(gp_RecordFile);
		g_RecordCritSec.~CriticalSection();
	}
#endif

	//------------------------------------------------------------------------
	void InitialiseVMem()
	{
#ifdef RECORD_ALLOCS
		InitialiseRecording();
#endif
		// only one thhread will make the lock, so when another thread gets
		// the lock it must check that it hasn't already been initialised
		if(!gp_VMemHeap)
		{
			InitialiseCore();

			VMemHeap::Settings settings;

			VMemHeap* p_heap = (VMemHeap*)g_VMemHeapMem;
			new (p_heap) VMemHeap(settings);

			// do this LAST
			gp_VMemHeap = p_heap;

			// must do this LAST because creating a thread can call malloc, and if malloc
			// is overridden we can get into an inifnite loop!
			// as long as gp_VMemHeap has been set we should avoid infinity.
			VirtualMem::CreatePulseThread();
		}
	}

	//------------------------------------------------------------------------
	// we need to initialise VMem in global startup because the initialise is
	// not thread safe. Global startup should happen before any threads are
	// created. If you are creating threads in global startup then the results
	// are undefined.
	class ForceInitialisationDuringGlobalStartup
	{
	public:
		ForceInitialisationDuringGlobalStartup() { if(!gp_VMemHeap) InitialiseVMem(); }
	} g_ForceInitialisationDuringGlobalStartup;

	//------------------------------------------------------------------------
#ifdef RECORD_ALLOCS
	void Record(void* p, size_t size)
	{
		g_RecordCritSec.Enter();
		fwrite(&p, sizeof(p), 1, gp_RecordFile);
		fwrite(&size, sizeof(size), 1, gp_RecordFile);
		g_RecordCritSec.Leave();
	}
	#define RECORD(addr, byte_count) Record(addr, byte_count);
#else
	#define RECORD(addr, byte_count)
#endif
}

//------------------------------------------------------------------------
void* VMem::Alloc(size_t size)
{
	if(!gp_VMemHeap)
		InitialiseVMem();

	void* p = gp_VMemHeap->Alloc(size);
	RECORD(p, size);
	return p;
}

//------------------------------------------------------------------------
void VMem::Free(void* p)
{
	VMEM_ASSERT(gp_VMemHeap, "heap not initialised. Alloc must be called at least once");
	RECORD(p, VMEM_INVALID_SIZE);
	gp_VMemHeap->Free(p);
}

//------------------------------------------------------------------------
size_t VMem::GetSize(void* p)
{
	VMEM_ASSERT(gp_VMemHeap, "heap not initialised. Alloc must be called at least once");
	return gp_VMemHeap->GetSize(p);
}

//------------------------------------------------------------------------
void VMem::Lock()
{
	VMEM_ASSERT(gp_VMemHeap, "heap not initialised. Alloc must be called at least once");
	return gp_VMemHeap->Lock();
}

//------------------------------------------------------------------------
void VMem::Release()
{
	VMEM_ASSERT(gp_VMemHeap, "heap not initialised. Alloc must be called at least once");
	return gp_VMemHeap->Release();
}

//------------------------------------------------------------------------
void* VMem::Realloc(void* p, size_t new_size)
{
	void* new_p = new_size ? Alloc(new_size) : NULL;
	if(p)
	{
		size_t old_size = GetSize(p);
		VMEM_ASSERT(old_size != VMEM_INVALID_SIZE, "trying to realloc invalid pointer");
		size_t copy_size = Min(old_size, new_size);
		if(new_p)
			memcpy_s(new_p, new_size, p, copy_size);
		Free(p);
	}
	return new_p;
}

//------------------------------------------------------------------------
void VMem::Initialise()
{
	if(!gp_VMemHeap)
		InitialiseVMem();
}

//------------------------------------------------------------------------
void VMem::Destroy(bool clean)
{
	if(gp_VMemHeap)
	{
		if(clean)
			gp_VMemHeap->~VMemHeap();
		gp_VMemHeap = NULL;
	}

	UninitialiseCore(clean);

#ifdef RECORD_ALLOCS
	UninitialiseRecording();
#endif
}

//------------------------------------------------------------------------
void VMem::CheckIntegrity()
{
	if(gp_VMemHeap)
		gp_VMemHeap->CheckIntegrity();
}

//------------------------------------------------------------------------
#ifdef VMEM_STATS
VMemHeapStats VMem::GetStats()
{
	VMEM_ASSERT(gp_VMemHeap, "heap not initialised. Alloc must be called at least once");
	return gp_VMemHeap->GetStats();
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
void VMem::WriteStats()
{
	if(gp_VMemHeap)
		gp_VMemHeap->WriteStats();
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
size_t VMem::WriteAllocs()
{
	VMEM_ASSERT(gp_VMemHeap, "heap not initialised. Alloc must be called at least once");
	return gp_VMemHeap->WriteAllocs();
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
void VMem::SendStatsToMemPro(void (*send_fn)(void*, int, void*), void* p_context)
{
	VMEM_ASSERT(gp_VMemHeap, "heap not initialised. Alloc must be called at least once");
	return gp_VMemHeap->SendStatsToMemPro(send_fn, p_context);
}
#endif

//------------------------------------------------------------------------
void VMem::SetDebugBreak(void (*DebugBreakFn)(const _TCHAR* p_message))
{
	VMem::SetDebugBreakFunction(DebugBreakFn);
}

//------------------------------------------------------------------------
void VMem::SetLogFunction(LogFn log_fn)
{
	VMem::VMemSysSetLogFunction(log_fn);
}

//------------------------------------------------------------------------
#ifdef VMEM_PROTECTED_HEAP
void VMem::SetVMemShouldProtectFn(bool (*should_protect_fn)(int i, size_t size))
{
	g_VMemSHouldProtectFn = should_protect_fn;
}
#endif
//------------------------------------------------------------------------
// VMemCore.cpp





#include <memory.h>

//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
namespace VMem
{
	void (*g_DebugBreakFn)(const _TCHAR* p_message) = NULL;

	void (*g_MemProDebugBreakFn)() = NULL;
}

//------------------------------------------------------------------------
void VMem::InitialiseCore()
{
	VirtualMem::Initialise();
}

//------------------------------------------------------------------------
void VMem::UninitialiseCore(bool clean)
{
	VirtualMem::Destroy(clean);
	VMemSysDestroy();
}

//------------------------------------------------------------------------
// return true if break handled
bool VMem::Break(const _TCHAR* p_message)
{
	if(g_MemProDebugBreakFn)
		g_MemProDebugBreakFn();

	if(g_DebugBreakFn)
	{
		g_DebugBreakFn(p_message);
		return true;
	}
	else
	{
		DebugWrite(p_message);
		return false;
	}
}

//------------------------------------------------------------------------
void VMem::SetDebugBreakFunction(void (*DebugBreakFn)(const _TCHAR* p_message))
{
	g_DebugBreakFn = DebugBreakFn;
}

//------------------------------------------------------------------------
void VMem::WriteAlloc(const void* p_alloc, size_t size)
{
	DebugWrite(_T("%6d bytes at 0x%08x "), size, p_alloc);

	DebugWrite(_T("  <"));

	const size_t max_byte_printout = 8;
	size_t byte_count = Min(max_byte_printout, size);

	// write hex memory
	const byte* p = (byte*)p_alloc;
	for(size_t i=0; i<byte_count; ++i)
	{
		byte c = *p++;
		if(c < 32 || c> 127)
			c = '.';
		DebugWrite(_T("%c"), (char)c);
	}
	DebugWrite(_T(">  "));

	// write chars
	p = (byte*)p_alloc;
	for(size_t i=0; i<byte_count; ++i)
		DebugWrite(_T("%02x "), *p++);
	if(byte_count < size)
		DebugWrite(_T("..."));

	DebugWrite(_T("\n"));
}
//------------------------------------------------------------------------
// VMemHeap.cpp






//------------------------------------------------------------------------
using namespace VMem;

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	void OutOfMemoryHandler()
	{
		DebugWrite(_T("Out of memory!\n"));
		DumpSystemMemory();
	}

	//------------------------------------------------------------------------
	void DebugWriteMem(size_t size)
	{
		float mb = (float)(size/1024.0/1024.0);
#ifdef VMEM64
		DebugWrite(_T("%10lld (%0.1fMB)"), size, mb);
#else
		DebugWrite(_T("%10d (%0.1fMB)"), size, mb);
#endif

		if(mb < 10.0f)			DebugWrite(_T("   "));
		else if(mb < 100.0f)	DebugWrite(_T("  "));
		else if(mb < 1000.0f)	DebugWrite(_T(" "));
	}

	//------------------------------------------------------------------------
	void WriteStats(const VMem::Stats& stats)
	{
		size_t committed_bytes = stats.GetCommittedBytes();
		int usage_percent = committed_bytes ? (int)((100 * (long long)stats.m_Used) / committed_bytes) : 0;
		DebugWrite(_T("%3d%% "), usage_percent);

		DebugWriteMem(stats.m_Used);
		DebugWrite(_T("  "));
		DebugWriteMem(stats.m_Unused);
		DebugWrite(_T("  "));
		DebugWriteMem(stats.m_Overhead);
		DebugWrite(_T("  "));
		DebugWriteMem(committed_bytes);
		DebugWrite(_T("  "));
		DebugWriteMem(stats.m_Reserved);
		DebugWrite(_T("\n"));
	}
}

//------------------------------------------------------------------------
VMemHeap::Settings::Settings()
:
// on windows we can reserve pages without committing them, this allows us
// to have larger region sizes which has a slight performance gain.
#ifdef VMEM_PLATFORM_WIN
	m_FSAHeap1_PageSize(4*1024),
	m_FSAHeap1_RegionSize(32*1024*1024),
	m_FSAHeap1_MaxSize(64),

	m_FSAHeap2_PageSize(32*1024),
	m_FSAHeap2_RegionSize(32*1024*1024),
	m_FSAHeap2_MaxSize(512),

	m_CoalesceHeap1_RegionSize(32*1024*1024),
	m_CoalesceHeap1_MinSize(512),
	m_CoalesceHeap1_MaxSize(10 * 1024),

	m_CoalesceHeap2_RegionSize(128*1024*1024),
	m_CoalesceHeap2_MinSize(10 * 1024),
	m_CoalesceHeap2_MaxSize(1024 * 1024)
#else
	m_FSAHeap1_PageSize(4*1024),
	m_FSAHeap1_RegionSize(1024*1024),
	m_FSAHeap1_MaxSize(64),

	m_FSAHeap2_PageSize(32*1024),
	m_FSAHeap2_RegionSize(4*1024*1024),
	m_FSAHeap2_MaxSize(512),

	m_CoalesceHeap1_RegionSize(128*1024),
	m_CoalesceHeap1_MinSize(512),
	m_CoalesceHeap1_MaxSize(10 * 1024),

	m_CoalesceHeap2_RegionSize(8*1024*1024),
	m_CoalesceHeap2_MinSize(10 * 1024),
	m_CoalesceHeap2_MaxSize(1024 * 1024)
#endif
{
}

//------------------------------------------------------------------------
VMemHeap::VMemHeap(Settings& settings)

:	m_Settings(settings),

	m_FSA1PageHeap(settings.m_FSAHeap1_PageSize, settings.m_FSAHeap1_RegionSize, m_InternalHeap),
	m_FSA2PageHeap(settings.m_FSAHeap2_PageSize, settings.m_FSAHeap2_RegionSize, m_InternalHeap),

	m_FSAHeap1(&m_FSA1PageHeap, m_InternalHeap),
	m_FSAHeap2(&m_FSA2PageHeap, m_InternalHeap),

	m_CoalesceHeap1(settings.m_CoalesceHeap1_RegionSize, settings.m_CoalesceHeap1_MinSize, settings.m_CoalesceHeap1_MaxSize),
	m_CoalesceHeap2(settings.m_CoalesceHeap2_RegionSize, settings.m_CoalesceHeap2_MinSize, settings.m_CoalesceHeap2_MaxSize)

{
	InitialiseFSAHeap1();
	InitialiseFSAHeap2();

	InitialiseTrailGuards();
}

//------------------------------------------------------------------------
void VMemHeap::InitialiseFSAHeap1()
{
#ifdef VMEM64
	const int alignment = 16;
	const int step = 16;

	for(int i=step; i<=m_Settings.m_FSAHeap1_MaxSize; i+=step)
	{
		m_FSAHeap1.InitialiseFSA(i, alignment);
	}
#else
	m_FSAHeap1.InitialiseFSA(4, 4);
	m_FSAHeap1.InitialiseFSA(8, 8);
	m_FSAHeap1.InitialiseFSA(12, 12);
	m_FSAHeap1.InitialiseFSA(16, 16);
	m_FSAHeap1.InitialiseFSA(20, 20);
	m_FSAHeap1.InitialiseFSA(24, 24);
	m_FSAHeap1.InitialiseFSA(28, 28);
	m_FSAHeap1.InitialiseFSA(32, 16);
	m_FSAHeap1.InitialiseFSA(36, 36);
	m_FSAHeap1.InitialiseFSA(40, 40);
	m_FSAHeap1.InitialiseFSA(44, 44);
	m_FSAHeap1.InitialiseFSA(48, 16);
	m_FSAHeap1.InitialiseFSA(64, 16);
#endif
}

//------------------------------------------------------------------------
void VMemHeap::InitialiseFSAHeap2()
{
	const int alignment = 16;
	const int step = 16;

	for(int i=m_Settings.m_FSAHeap1_MaxSize+step; i<=m_Settings.m_FSAHeap2_MaxSize; i+=step)
	{
		m_FSAHeap2.InitialiseFSA(i, alignment);
	}
}

//------------------------------------------------------------------------
void* VMemHeap::Alloc(size_t size)
{
	GLOBAL_LOCK

	VMEM_ASSERT(size >= 0, "can't allocate negative size");

#ifdef VMEM_INC_INTEG_CHECK
	IncIntegrityCheck();
#endif

	void* p = NULL;

#ifdef VMEM_PROTECTED_HEAP
	p = m_ProtectedHeap.Alloc(size);
	if(p)
		return p;
	do {
#endif

	if(size <= ToSizeT(m_Settings.m_FSAHeap1_MaxSize))
		p = m_FSAHeap1.Alloc(ToInt(size));

	else if(size <= ToSizeT(m_Settings.m_FSAHeap2_MaxSize))
		p = m_FSAHeap2.Alloc(ToInt(size));

	else if(size <= ToSizeT(m_Settings.m_CoalesceHeap1_MaxSize))
		p = m_CoalesceHeap1.Alloc(ToInt(size));

	else if(size <= ToSizeT(m_Settings.m_CoalesceHeap2_MaxSize))
		p = m_CoalesceHeap2.Alloc(ToInt(size));

	else
		p = m_LargeHeap.Alloc(size);

#ifdef VMEM_PROTECTED_HEAP
	} while(!p && m_ProtectedHeap.FreeReservedMemory());
#endif

	if(!p)
		OutOfMemoryHandler();

	return p;
}

//------------------------------------------------------------------------
void VMemHeap::Free(void* p)
{
	GLOBAL_LOCK

#ifdef VMEM_PROTECTED_HEAP
	if(m_ProtectedHeap.Free(p))
		return;
#endif

	if(m_FSAHeap1.Free(p))
		return;

	if(m_FSAHeap2.Free(p))
		return;

	if(!p)			// unusual case, so put after most common cases
		return;

	if(m_CoalesceHeap1.Free(p))
		return;

	if(m_CoalesceHeap2.Free(p))
		return;

	if(m_LargeHeap.Free(p))
		return;

	VMem::Break(_T("Allocation was not allocated by this heap"));
}

//------------------------------------------------------------------------
size_t VMemHeap::GetSize(void* p)
{
	GLOBAL_LOCK

	size_t size;

#ifdef VMEM_PROTECTED_HEAP
	size = m_ProtectedHeap.GetSize(p);
	if(size != VMEM_INVALID_SIZE) return size;
#endif

	size = m_FSAHeap1.GetSize(p);
	if(size != VMEM_INVALID_SIZE) return size;

	size = m_FSAHeap2.GetSize(p);
	if(size != VMEM_INVALID_SIZE) return size;

	size = m_CoalesceHeap1.GetSize(p);
	if(size != VMEM_INVALID_SIZE) return size;

	size = m_CoalesceHeap2.GetSize(p);
	if(size != VMEM_INVALID_SIZE) return size;

	size = m_LargeHeap.GetSize(p);
	if(size != VMEM_INVALID_SIZE) return size;

	return VMEM_INVALID_SIZE;
}

//------------------------------------------------------------------------
#ifdef VMEM_STATS
void VMemHeap::SendStatsToMemPro(MemProSendFn send_fn, void* p_context)
{
	GLOBAL_LOCK
	m_InternalHeap.SendStatsToMemPro(send_fn, p_context);
	m_FSA1PageHeap.SendStatsToMemPro(send_fn, p_context);
	m_FSA2PageHeap.SendStatsToMemPro(send_fn, p_context);
	m_FSAHeap1.SendStatsToMemPro(send_fn, p_context);
	m_FSAHeap2.SendStatsToMemPro(send_fn, p_context);
	m_CoalesceHeap1.SendStatsToMemPro(send_fn, p_context);
	m_CoalesceHeap2.SendStatsToMemPro(send_fn, p_context);
	m_LargeHeap.SendStatsToMemPro(send_fn, p_context);

	SendEnumToMemPro(vmem_End, send_fn, p_context);
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
VMemHeapStats VMemHeap::GetStats()
{
	GLOBAL_LOCK
	return GetStatsInternal();
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
VMemHeapStats VMemHeap::GetStatsInternal()
{
	VMemHeapStats stats;

	Stats page_heap1 = m_FSA1PageHeap.GetStats();
	page_heap1.m_Used = 0;												// this is tracked by the FSAHeap
	stats.m_FSAHeap1 = m_FSAHeap1.GetStats() + page_heap1;

	Stats page_heap2 = m_FSA2PageHeap.GetStats();
	page_heap2.m_Used = 0;												// this is tracked by the FSAHeap
	stats.m_FSAHeap2 = m_FSAHeap2.GetStats() + page_heap2;

	stats.m_CoalesceHeap1 = m_CoalesceHeap1.GetStats();
	stats.m_CoalesceHeap2 = m_CoalesceHeap2.GetStats();

	stats.m_LargeHeap = m_LargeHeap.GetStats();

	stats.m_Internal = m_InternalHeap.GetStats();
	stats.m_Internal.m_Used = 0;										// this is tracked by the allocator overheads

	stats.m_Total = 
		stats.m_FSAHeap1 +
		stats.m_FSAHeap2 +
		stats.m_CoalesceHeap1 +
		stats.m_CoalesceHeap2 +
		stats.m_LargeHeap +
		stats.m_Internal;

#ifdef VMEM_PROTECTED_HEAP
	stats.m_Total += m_ProtectedHeap.GetStats();
#endif

	return stats;
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
VMemHeapStats VMemHeap::GetStatsNoLock() const
{
	VMemHeapStats stats;

	Stats page_heap1 = m_FSA1PageHeap.GetStatsNoLock();
	page_heap1.m_Used = 0;												// this is tracked by the FSAHeap
	stats.m_FSAHeap1 = m_FSAHeap1.GetStatsNoLock() + page_heap1;

	Stats page_heap2 = m_FSA2PageHeap.GetStatsNoLock();
	page_heap2.m_Used = 0;												// this is tracked by the FSAHeap
	stats.m_FSAHeap2 = m_FSAHeap2.GetStatsNoLock() + page_heap2;

	stats.m_CoalesceHeap1 = m_CoalesceHeap1.GetStatsNoLock();
	stats.m_CoalesceHeap2 = m_CoalesceHeap2.GetStatsNoLock();

	stats.m_LargeHeap = m_LargeHeap.GetStatsNoLock();

	stats.m_Internal = m_InternalHeap.GetStatsNoLock();
	stats.m_Internal.m_Used = 0;										// this is tracked by the allocator overheads

	Stats virtual_mem_stats = VirtualMem::GetStats();
	stats.m_Internal += virtual_mem_stats;

	stats.m_Total = 
		stats.m_FSAHeap1 +
		stats.m_FSAHeap2 +
		stats.m_CoalesceHeap1 +
		stats.m_CoalesceHeap2 +
		stats.m_LargeHeap +
		stats.m_Internal;

#ifdef VMEM_PROTECTED_HEAP
	stats.m_Total += m_ProtectedHeap.GetStatsNoLock();
#endif

	return stats;
}
#endif

//------------------------------------------------------------------------
#ifdef VMEM_STATS
void VMemHeap::WriteStats()
{
	GLOBAL_LOCK

	VMemHeapStats stats = GetStatsInternal();

	DebugWrite(_T("                       Used                 Unused               Overhead                  Total               Reserved\n"));
	DebugWrite(_T("     FSA1:  "));	::WriteStats(stats.m_FSAHeap1);
	DebugWrite(_T("     FSA2:  "));	::WriteStats(stats.m_FSAHeap2);
	DebugWrite(_T("Coalesce1:  "));	::WriteStats(stats.m_CoalesceHeap1);
	DebugWrite(_T("Coalesce2:  "));	::WriteStats(stats.m_CoalesceHeap2);
	DebugWrite(_T("    Large:  "));	::WriteStats(stats.m_LargeHeap);
	DebugWrite(_T(" Internal:  "));	::WriteStats(stats.m_Internal);
	DebugWrite(_T("    TOTAL:  "));	::WriteStats(stats.m_Total);
}
#endif

//------------------------------------------------------------------------
void VMemHeap::CheckIntegrity()
{
	GLOBAL_LOCK

	m_FSAHeap1.CheckIntegrity();
	m_FSAHeap2.CheckIntegrity();

	m_CoalesceHeap1.CheckIntegrity();
	m_CoalesceHeap2.CheckIntegrity();

#ifdef VMEM_PROTECTED_HEAP
	m_ProtectedHeap.CheckIntegrity();
#endif

	CheckStats();
}

//------------------------------------------------------------------------
#ifdef VMEM_INC_INTEG_CHECK
void VMemHeap::IncIntegrityCheck()
{
	static volatile long i = 0;
	if(i == VMEM_INC_INTEG_CHECK)
	{
		m_FSAHeap1.IncIntegrityCheck();
	}
	else if(i == 2*VMEM_INC_INTEG_CHECK)
	{
		m_FSAHeap2.IncIntegrityCheck();
	}
	else if(i == 3*VMEM_INC_INTEG_CHECK)
	{
		m_CoalesceHeap1.CheckIntegrity();
	}
	else if(i == 4*VMEM_INC_INTEG_CHECK)
	{
		m_CoalesceHeap2.CheckIntegrity();
	}
	else if(i == 5*VMEM_INC_INTEG_CHECK)
	{
		CheckStats();
	}
	else if(i == 6*VMEM_INC_INTEG_CHECK)
	{
		i = -1;
	}

	VMem_InterlockedIncrement(i);
}
#endif

//------------------------------------------------------------------------
void VMemHeap::CheckStats() const
{
#ifdef VMEM_STATS
	Lock();
	VMemHeapStats stats = GetStatsNoLock();
	VMEM_ASSERT(stats.m_Total.m_Reserved == VMem::GetReservedBytes(), "total reserved bytes doesn't match");
	VMEM_ASSERT(stats.m_Total.GetCommittedBytes() == VMem::GetCommittedBytes(), "total committed bytes doesn't match");
	Release();
#endif
}

//------------------------------------------------------------------------
#ifdef VMEM_STATS
size_t VMemHeap::WriteAllocs()
{
	GLOBAL_LOCK

	size_t allocated_bytes = 0;

	allocated_bytes += m_FSAHeap1.WriteAllocs();
	allocated_bytes += m_FSAHeap2.WriteAllocs();

	allocated_bytes += m_CoalesceHeap1.WriteAllocs();
	allocated_bytes += m_CoalesceHeap2.WriteAllocs();

	allocated_bytes += m_LargeHeap.WriteAllocs();

	return allocated_bytes;
}
#endif

//------------------------------------------------------------------------
void VMemHeap::InitialiseTrailGuards()
{
#ifdef VMEM_TRAIL_GUARDS
	int start_size = sizeof(void*);
	for(int i=start_size; i<=m_Settings.m_FSAHeap1_MaxSize; i+=4)
	{
		FSAHeapT::FSA* p_fsa = m_FSAHeap1.GetFSA(i);
		p_fsa->InitialiseTrailGuard(100*1024, 1000);
	}

	for(int i=m_Settings.m_FSAHeap1_MaxSize+4; i<=m_Settings.m_FSAHeap2_MaxSize; i+=4)
	{
		FSAHeapT::FSA* p_fsa = m_FSAHeap2.GetFSA(i);
		p_fsa->InitialiseTrailGuard(100*1024, 1000);
	}

	m_CoalesceHeap1.InitialiseTrailGuard(1024*1024, 100);
	m_CoalesceHeap2.InitialiseTrailGuard(1024*1024, 100);
#endif
}

//------------------------------------------------------------------------
void VMemHeap::Lock() const
{
	m_FSAHeap1.Lock();
	m_FSAHeap2.Lock();
	m_CoalesceHeap1.Lock();
	m_CoalesceHeap2.Lock();
	m_LargeHeap.Lock();
	VirtualMem::Lock();
#ifdef VMEM_PROTECTED_HEAP
	m_ProtectedHeap.Lock();
#endif
}

//------------------------------------------------------------------------
void VMemHeap::Release() const
{
	VirtualMem::Release();
	m_LargeHeap.Release();
	m_CoalesceHeap2.Release();
	m_CoalesceHeap1.Release();
	m_FSAHeap2.Release();
	m_FSAHeap1.Release();
#ifdef VMEM_PROTECTED_HEAP
	m_ProtectedHeap.Release();
#endif
}
//------------------------------------------------------------------------
// VMemSys.cpp



#include <stdarg.h>

#ifndef VMEM_PLATFORM_APPLE
	#include <malloc.h>
#endif

#ifdef VMEM_OS_UNIX
	#include <sys/mman.h>
#endif

//-----------------------------------------------------------------------------
#ifdef VMEM_OS_WIN
	#pragma warning(disable:4100)
#endif

//-----------------------------------------------------------------------------
//#define USE_MALLOC

//-----------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
	size_t g_ReservedBytes = 0;
	size_t g_CommittedBytes = 0;

	void (*g_LogFn)(const _TCHAR* p_message) = NULL;

	//------------------------------------------------------------------------
	void Log(const _TCHAR* p_message)
	{
		if(g_LogFn)
		{
			g_LogFn(p_message);
		}
		else
		{
			#ifdef VMEM_OS_WIN
				OutputDebugString(p_message);
			#else
				printf("%s", p_message);
			#endif
				printf("%s", p_message);
		}
	}

	//------------------------------------------------------------------------
	const void PrintLastError()
	{
#ifdef VMEM_OS_WIN
		_TCHAR* p_buffer = NULL;
		va_list args;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			::GetLastError(),
			0,
			(_TCHAR*)&p_buffer,
			4 * 1024,
			&args);
		Log(p_buffer);
#endif
	}
}

//------------------------------------------------------------------------
void VMem::VMemSysDestroy()
{
	g_ReservedBytes = 0;
	g_CommittedBytes = 0;
}

//------------------------------------------------------------------------
void* VMem::VirtualReserve(size_t size)
{
	VMEM_ASSERT((size & (SYS_PAGE_SIZE-1)) == 0, "reserve size not aligned to page size");

#if defined(USE_MALLOC)

	// ====================================================
	//					MALLOC reserve
	// ====================================================
	void* p = _aligned_malloc(size, SYS_PAGE_SIZE);

#elif defined(VMEM_OS_WIN)

	// ====================================================
	//					WIN reserve
	// ====================================================
	unsigned int flags = MEM_RESERVE;

	// only zero memory if memset defined
	#if !defined(VMEM_MEMSET) && defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1)
	#error Please contact slynch@puredevsoftware.com for this platform
	#endif

	// large pages more effecient on this platform
	#if defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1) || defined(MEMPRO_PLATFORM_GAMES_CONSOLE_2)
	#error Please contact slynch@puredevsoftware.com for this platform
	#endif

	void* p = ::VirtualAlloc(NULL, size, flags, PAGE_NOACCESS);

#else

	// ====================================================
	//					UNIX reserve
	// ====================================================
	void* p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
	if(p == MAP_FAILED)
		p = NULL;

#endif

	if(p)
		g_ReservedBytes += size;

	VMEM_ASSERT(((size_t)p & (SYS_PAGE_SIZE-1)) == 0, "Memory not aligned!");

	return p;
}

//------------------------------------------------------------------------
void VMem::VirtualRelease(void* p, size_t size)
{
#if defined(USE_MALLOC)

	// ====================================================
	//					MALLOC release
	// ====================================================
	_aligned_free(p);

#elif defined(VMEM_OS_WIN)

	// ====================================================
	//					WIN release
	// ====================================================
	BOOL b = ::VirtualFree(p, 0, MEM_RELEASE);
	if(!b)
		PrintLastError();
	VMEM_ASSERT(b, "VirtualFree failed");

#else

	// ====================================================
	//					UNIX release
	// ====================================================
	int ret = munmap(p, size);
	VMEM_ASSERT(ret == 0, "munmap failed");

#endif

	g_ReservedBytes -= size;
}

//------------------------------------------------------------------------
bool VMem::VirtualCommit(void* p, size_t size)
{
	bool success = false;

#if defined(USE_MALLOC)

	// ====================================================
	//					MALLOC commit
	// ====================================================
	VMEMSET(p, 0, size);
	success = true;

#elif defined(VMEM_OS_WIN)

	// ====================================================
	//					WIN commit
	// ====================================================
	unsigned int flags = MEM_COMMIT;

	// only zero memory if memset defined
	#if !defined(VMEM_MEMSET) && defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1)
	#error Please contact slynch@puredevsoftware.com for this platform
	#endif

	// large pages more effecient on this platform
	#ifdef MEMPRO_PLATFORM_GAMES_CONSOLE_1
	#error Please contact slynch@puredevsoftware.com for this platform
	#endif

	success = ::VirtualAlloc(p, size, flags, PAGE_READWRITE) != NULL;

#else

	// ====================================================
	//					UNIX commit
	// ====================================================
	VMEMSET(p, 0, size);
	success = true;

#endif

	if(success)
		g_CommittedBytes += size;

	return success;
}

//------------------------------------------------------------------------
void VMem::VirtualDecommit(void* p, size_t size)
{
#if defined(USE_MALLOC)

	// ====================================================
	//					MALLOC decommit
	// ====================================================
	// do nothing

#elif defined(VMEM_OS_WIN)

	// ====================================================
	//					WIN decommit
	// ====================================================
	BOOL b = ::VirtualFree(p, size, MEM_DECOMMIT);
	if(!b)
		PrintLastError();
	VMEM_ASSERT(b, "VirtualFree failed");

#else

	// ====================================================
	//					UNIX decommit
	// ====================================================
	// do nothing

#endif

	g_CommittedBytes -= size;
}

//------------------------------------------------------------------------
size_t VMem::GetReservedBytes()
{
	return g_ReservedBytes;
}

//------------------------------------------------------------------------
size_t VMem::GetCommittedBytes()
{
	return g_CommittedBytes;
}

//------------------------------------------------------------------------
// this does not need to be implemented, it is only for debug purposes. You can just return true.
#ifdef VMEM_ASSERTS
bool VMem::Committed(void* p, size_t size)
{
#ifdef VMEM_OS_WIN
	MEMORY_BASIC_INFORMATION mem_info;

	byte* p_check_p = (byte*)p;
	size_t check_size = size;
	while(check_size)
	{
		memset(&mem_info, 0, sizeof(mem_info));
		VirtualQuery(p_check_p, &mem_info, sizeof(mem_info));
		if(mem_info.State != MEM_COMMIT)
			return false;
		size_t reserve_size = Min(check_size, (size_t)mem_info.RegionSize);
		check_size -= reserve_size;
		p_check_p += reserve_size;
	}

	return true;
#else
	return true;
#endif
}
#endif

//------------------------------------------------------------------------
// this does not need to be implemented, it is only for debug purposes. You can just return true.
#ifdef VMEM_ASSERTS
bool VMem::Reserved(void* p, size_t size)
{
#ifdef VMEM_OS_WIN
	MEMORY_BASIC_INFORMATION mem_info;

	byte* p_check_p = (byte*)p;
	size_t check_size = size;
	while(check_size)
	{
		memset(&mem_info, 0, sizeof(mem_info));
		VirtualQuery(p_check_p, &mem_info, sizeof(mem_info));
		if(mem_info.State != MEM_RESERVE)
			return false;
		size_t reserve_size = Min(check_size, (size_t)mem_info.RegionSize);
		check_size -= reserve_size;
		p_check_p += reserve_size;
	}

	return true;
#else
	return true;
#endif
}
#endif

//------------------------------------------------------------------------
void VMem::DebugWrite(const _TCHAR* p_str, ...)
{
#ifdef VMEM_STATS
	va_list args;
	va_start(args, p_str);

	static _TCHAR g_TempString[1024];
	_vstprintf_s(g_TempString, sizeof(g_TempString)/sizeof(_TCHAR), p_str, args);
	Log(g_TempString);

	va_end(args);
#endif
}

//------------------------------------------------------------------------
void VMem::Break()
{
#ifdef VMEM_OS_WIN
	DebugBreak();
#else
	assert(false);
#endif
}

//------------------------------------------------------------------------
void VMem::DumpSystemMemory()
{
#if defined(VMEM_PLATFORM_WIN)
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);

	DebugWrite(_T("There is  %7ld percent of memory in use.\n"), status.dwMemoryLoad);
	DebugWrite(_T("There are %7I64d total Kbytes of physical memory.\n"), status.ullTotalPhys/1024);
	DebugWrite(_T("There are %7I64d free Kbytes of physical memory.\n"), status.ullAvailPhys/1024);
	DebugWrite(_T("There are %7I64d total Kbytes of paging file.\n"), status.ullTotalPageFile/1024);
	DebugWrite(_T("There are %7I64d free Kbytes of paging file.\n"), status.ullAvailPageFile/1024);
	DebugWrite(_T("There are %7I64d total Kbytes of virtual memory.\n"), status.ullTotalVirtual/1024);
	DebugWrite(_T("There are %7I64d free Kbytes of virtual memory.\n"), status.ullAvailVirtual/1024);
	DebugWrite(_T("There are %7I64d free Kbytes of extended memory.\n"), status.ullAvailExtendedVirtual/1024);
#elif defined(MEMPRO_PLATFORM_GAMES_CONSOLE_1)
	#error Please contact slynch@puredevsoftware.com for this platform
#elif defined(VMEM_OS_UNIX)
	// not possible on Unix
#else
	#error platform not defined
#endif
}

//------------------------------------------------------------------------
void VMem::VMemSysSetLogFunction(void (*LogFn)(const _TCHAR* p_message))
{
	g_LogFn = LogFn;
}
//------------------------------------------------------------------------
// VMemThread.cpp



//------------------------------------------------------------------------
#ifdef VMEM_DELAYED_RELEASE

//------------------------------------------------------------------------
namespace VMem
{
	//------------------------------------------------------------------------
#if defined(VMEM_OS_WIN)
	unsigned long WINAPI PlatformThreadMain(void* p_param)
	{
		ThreadMain p_thread_main = (ThreadMain)p_param;
		return p_thread_main();
	}
#elif defined(VMEM_OS_UNIX)
	void* PlatformThreadMain(void* p_param)
	{
		ThreadMain p_thread_main = (ThreadMain)p_param;
		p_thread_main();
		return NULL;
	}
#else
	#error	// OS not defined
#endif
}

//------------------------------------------------------------------------
VMem::ThreadHandle VMem::Thread::CreateThread(ThreadMain p_thread_main)
{
#if defined(VMEM_OS_WIN)
	return ::CreateThread(NULL, 0, PlatformThreadMain, p_thread_main, 0, NULL);
#elif defined(VMEM_OS_UNIX)
	pthread_t thread;
	pthread_create(&thread, NULL, PlatformThreadMain, (void*)p_thread_main);
	return thread;
#else
	#error	// OS not defined
#endif
}

//------------------------------------------------------------------------
// only used in dire circumstances when VMem is trying to shut down after a crash (in the VMem tests)
void VMem::Thread::TerminateThread(ThreadHandle thread)
{
#if defined(VMEM_OS_WIN)
	::TerminateThread(thread, 0);
#elif defined(VMEM_OS_UNIX) && !defined(VMEM_PLATFORM_ANDROID)
	pthread_cancel(thread);
#else
	VMem::Break("pthread_cancel not implemented on this platform");
#endif

	WaitForThreadToTerminate(thread);
}

//------------------------------------------------------------------------
void VMem::Thread::WaitForThreadToTerminate(ThreadHandle thread)
{
#if defined(VMEM_OS_WIN)
	WaitForSingleObject(thread, INFINITE);
#elif defined(VMEM_OS_UNIX)
	pthread_join(thread, NULL);
#else
	#error	// OS not defined
#endif
}

//------------------------------------------------------------------------
#endif		// #ifdef VMEM_DELAYED_RELEASE
//------------------------------------------------------------------------
// VMem_PCH.cpp
