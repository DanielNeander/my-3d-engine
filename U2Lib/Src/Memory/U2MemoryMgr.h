/************************************************************************
module  : U2MemoryMgr
Author  : yun sangyong
Desc	: Gamebryo 2.6 메모리 매니저 
************************************************************************/
#ifndef	U2_MEMORYMGR_H
#define	U2_MEMORYMGR_H

#include <U2Lib/Src/U2LibType.h>

#include "U2Allocator.h"
#include "U2MemoryDefine.h"
#include <U2Lib/src/U2Debug.h>
#include <U2Lib/src/U2Log.h>
#include <U2Lib/src/U2TString.h>


class U2LIB U2MemoryMgr 
{

public:
	static U2MemoryMgr& Instance() { return *(ms_pMemMgr); }
	static bool	IsInit(); 

	void* Allocate(
		size_t& sizeInBytes, 
		size_t alignment, 
		uint32 bMemFlags,
		U2MemType eMemType,
		const char* szFile = U2_MEM_FILE,
		int line = U2_MEM_LINE,
		const char* szFunction = U2_MEM_FUNCTION);

	void Deallocate(
		void* pvMem,
		U2MemType eMemType,
		size_t sizeInBytes  = U2_MEM_DEALLOC_SIZE);

	void* Reallocate(
		void* pvMem,
		size_t& sizeInBytes,
		size_t alignment,
		uint32 bMemFlags,		
		U2MemType eMemType,
		size_t sizeCurr = U2_MEM_DEALLOC_SIZE,
		const char* szFile = U2_MEM_FILE, 
		int line = U2_MEM_LINE,
		const char* szFunction = U2_MEM_FUNCTION);

	// Memory Tracking
	bool TrackAllocate(
		const void* const pvMem,
		size_t sizeInBytes,
		uint32 bMemFlags,
		U2MemType eMemType,
		const char* szFile = U2_MEM_FILE,
		int line = U2_MEM_LINE,  
		const char* szFunction = U2_MEM_FUNCTION);

	bool TrackDeallocate(
		const void* const pvMem,
		U2MemType eMemType);

	bool SetMark(const char* szMarkType, const char* szClassfier, 
		const char* szStr);

	static void Init();
	static void Terminate();

	static bool VertifyPtr(const void* pvMem);

protected:
	U2MemoryMgr() {}
	U2MemoryMgr(const U2MemoryMgr&) {}

	static U2MemoryMgr* ms_pMemMgr;

	U2Allocator* m_pAllocator;

};

#include "U2MemoryMgr.inl"


#endif