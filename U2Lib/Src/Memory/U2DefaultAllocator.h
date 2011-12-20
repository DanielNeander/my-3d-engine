/************************************************************************
module : NiDefaultAllocator
Author : yun sangyong
Desc   : GameBryo 2.6 메모리 매니저 
		 
************************************************************************/
#pragma  once
#ifndef U2_DEFAULTALLOCATOR_H
#define U2_DEFAULTALLOCATOR_H

#include <U2Lib/Src/U2Debug.h>
#include "U2Allocator.h"

#define USE_SMALL_OBJECT_ALLOCATOR

class U2LIB U2DefaultAllocator : public U2Allocator
{

public:
	virtual void* Allocate(
		size_t& sizeInBytes, 
		size_t& alignment, 
		uint32 bMemFlags,
		U2MemType eMemType,
		const char* szFile,
		int line,
		const char* szFunction);

	virtual void Deallocate(
		void* pvMem,
		U2MemType eMemType,
		size_t sizeInBytes);

	virtual void* Reallocate(
		void* pvMem,
		size_t& sizeInBytes,		
		size_t& alignment,
		uint32 bMemFlags,
		U2MemType eMemType,
		size_t sizeCurr,
		const char* szFile, int line,
		const char* szFunction);

	// Memory Tracking
	virtual bool TrackAllocate(
		const void* const pvMem,
		size_t sizeInBytes,
		uint32 bMemFlags,
		U2MemType eMemType,
		const char* szFile,
		int line, 
		const char* szFunction);

	virtual bool TrackDeallocate(
		const void* const pvMem,
		U2MemType eMemType);

	virtual bool SetMark(const char* szMarkType, const char* szClassfier, 
		const char* szStr);

	virtual void Init();
	virtual void Terminate();

	virtual bool VertifyPtr(const void* pvMem);

//#ifdef USE_SMALL_OBJECT_ALLOCATOR
//	static U2SmallObj

};


#include "U2DefaultAllocator.inl"

#endif