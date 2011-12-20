#include <src/U2LibPCH.h>
#include "U2MemoryDefine.h"


void _U2Free(void* pvMem)
{
	if(pvMem == NULL)
		return;

	U2MemoryMgr::Instance().Deallocate(pvMem, MEM_FREE_);

}

void _U2AlignedFree(void* pvMem)
{
	if(pvMem == NULL)
		return;

	U2MemoryMgr::Instance().Deallocate(pvMem, MEM_ALIGNEDFREE);
}

#ifdef USE_DEBUG_MEMORY
void*		U2_Malloc_(size_t sizeInBytes, uint32 uiMemFlag,
					   const char* szFile, int line, const char* szFunction)
#else 
void*		U2_Malloc_(size_t sizeInBytes, uint32 uiMemFlag)
#endif
{
	U2ASSERT(U2MemoryMgr::IsInit());

	if(sizeInBytes == 0)
		sizeInBytes = 1;

	void* pvMem = U2MemoryMgr::Instance().Allocate(sizeInBytes, 
		U2_MEM_ALIGNMENT, uiMemFlag, MEM_MALLOC 
#ifdef USE_DEBUG_MEMORY
		,szFile, line, szFunction
#endif
		);

	U2ASSERT(pvMem);

	return pvMem;
}


#ifdef USE_DEBUG_MEMORY
void*		U2_AllignedMalloc_(size_t sizeInBytes, size_t alignment, uint32 uiMemFlag,
							   const char* szFile, int line, const char* szFunction)
#else 
void*		U2_AllignedMalloc_(size_t sizeInBytes, size_t alignment, uint32 uiMemFlag)
#endif 
{
	U2ASSERT(U2MemoryMgr::IsInit());

	if(sizeInBytes == 0)
		sizeInBytes = 1;

	void* pvMem = U2MemoryMgr::Instance().Allocate(sizeInBytes, alignment, uiMemFlag,
		MEM_ALIGNEDMALLOC
#ifdef USE_DEBUG_MEMORY
		,szFile, line, szFunction
#endif
		);

	U2ASSERT(pvMem);

	return pvMem;

}

#ifdef USE_DEBUG_MEMORY
bool		U2VerifyPtr(const void* pvMem)
{
	if(!pvMem)
		return true;

	U2ASSERT(U2MemoryMgr::IsInit());
	return U2MemoryMgr::Instance().VertifyPtr(pvMem);
}
#endif 

