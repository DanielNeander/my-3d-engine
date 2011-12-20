#include <Src/U2LibPCH.h>
#include <exception>
#include <new>

#include "U2Allocator.h"
#include "U2MemObj.h"

#include <U2Lib/Src/U2Debug.h>
#include "U2MemoryMgr.h"


#ifdef USE_DEBUG_MEMORY		
	void* U2MemObj::operator new(size_t size, uint32 uiMemFlag)
	{
		throw std::bad_alloc();
	}


	void* U2MemObj::operator new[](size_t size, uint32 uiMemFlag)
	{
		throw std::bad_alloc();		
	}

	void* U2MemObj::operator new(
			 size_t sizeInBytes,
			 uint32 uiMemFlag,
			 const char* szFile,
			 int line, 
			 const char* szFunction)
	{
		U2ASSERT(U2MemoryMgr::IsInit());
		if(sizeInBytes == 0)
			sizeInBytes = 1;

		uiMemFlag |= MEMFLAG_COMPILER_PROVIDE_SIZE_ON_DEALLOCATE;		
		void* p = U2MemoryMgr::Instance().Allocate(sizeInBytes, U2_MEM_ALIGNMENT, 
			uiMemFlag, MEM_NEW, szFile, line, szFunction);

		if(p == 0)
			throw std::bad_alloc();

		return p;
	}

	void* U2MemObj::operator new[](
			   size_t sizeInBytes,
			   uint32 uiMemFlag,
			   const char* szFile,
			   int line, 
			   const char* szFunction)
	{
		U2ASSERT(U2MemoryMgr::IsInit());
		if(sizeInBytes == 0)
			sizeInBytes = 1;

		void* p = U2MemoryMgr::Instance().Allocate(sizeInBytes, 
			U2_MEM_ALIGNMENT,uiMemFlag, MEM_NEW_ARRAY, szFile, line, szFunction);

		if(p == 0)
			throw std::bad_alloc();

		return p;
	}
#else 
	void* U2MemObj::operator new(size_t sizeInBytes, uint32 uiMemFlag)
	{
		U2ASSERT(U2MemoryMgr::IsInit());
		if(sizeInBytes == 0)
			sizeInBytes = 1;

		uiMemFlag |= MEMFLAG_COMPILER_PROVIDE_SIZE_ON_DEALLOCATE;		
		void* p = U2MemoryMgr::Instance().Allocate(sizeInBytes, U2_MEM_ALIGNMENT, 
			uiMemFlag, MEM_NEW);

		if(p == 0)
			throw std::bad_alloc();

		return p;

	}

	void* U2MemObj::operator new[](size_t sizeInBytes, uint32 uiMemFlag)
	{
		U2ASSERT(U2MemoryMgr::IsInit());
		if(sizeInBytes == 0)
			sizeInBytes = 1;

		void* p = U2MemoryMgr::Instance().Allocate(sizeInBytes, 
			U2_MEM_ALIGNMENT,uiMemFlag, MEM_NEW_ARRAY);

		if(p == 0)
			throw std::bad_alloc();

		return p;

	}
#endif 

void U2MemObj::operator delete(void* pvMem, size_t elemSize)
{
	if(pvMem)
	{
		U2ASSERT(U2MemoryMgr::IsInit());
		U2MemoryMgr::Instance().Deallocate(pvMem, MEM_DELETE, elemSize);
	}
}

void U2MemObj::operator delete[](void* pvMem, size_t)
{
	if(pvMem)
	{
		U2ASSERT(U2MemoryMgr::IsInit());
		U2MemoryMgr::Instance().Deallocate(pvMem, MEM_DELETE_ARRAY);
	}
}