/************************************************************************
module : U2Allocator
Author : yun sangyong
Desc   : GameBryo 2.6 메모리 매니저 
		 커스텀 할당자를 제공할 수 있도록 만든 인터페이스
************************************************************************/

#ifndef U2_ALLOCATOR_H
#define U2_ALLOCATOR_H

#include <U2Lib/Src/U2LibType.h>
#include "U2MemoryDefine.h"

class U2LIB	U2Allocator
{
public:
	inline virtual ~U2Allocator() {};

	virtual void* Allocate(
		size_t& sizeInBytes, 
		size_t& alignment, 
		uint32 bMemFlags,
		U2MemType eMemType,
		const char* szFile,
		int line,
		const char* szFunction) = 0;
	
	virtual void Deallocate(
		void* pvMem,
		U2MemType eMemType,
		size_t sizeInBytes) = 0;

	virtual void* Reallocate(
		void* pvMem,
		size_t& sizeInBytes,		
		size_t& alignment,
		uint32 bMemFlags,
		U2MemType eMemType,
		size_t sizeCurr,
		const char* szFile, int line,
		const char* szFunction) = 0;

	// Memory Tracking
	virtual bool TrackAllocate(
		const void* const pvMem,
		size_t sizeInBytes,
		uint32 bMemFlags,
		U2MemType eMemType,
		const char* szFile,
		int line, 
		const char* szFunction) = 0;

	virtual bool TrackDeallocate(
		const void* const pvMem,
		U2MemType eMemType) = 0;

	virtual bool SetMark(const char* szMarkType, const char* szClassfier, 
		const char* szStr) = 0;

	virtual void Init() = 0;
	virtual void Terminate() = 0;

	virtual bool VertifyPtr(const void* pvMem) = 0;

};

#endif