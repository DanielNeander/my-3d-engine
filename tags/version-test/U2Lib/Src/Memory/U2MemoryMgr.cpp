#include <Src/U2LibPCH.h>
#include "U2DefaultAllocator.h"
#include "U2SmallObjAllocStrategy.h"
#include "U2MemTracker.h"

#include "U2MemoryMgr.h"

U2MemoryMgr* U2MemoryMgr::ms_pMemMgr = 0;

bool U2MemoryMgr::IsInit()
{
	return ms_pMemMgr != 0;	
}

void U2MemoryMgr::Init()
{
	ms_pMemMgr = new U2MemoryMgr();		
	//ms_pMemMgr->m_pAllocator = new U2SmallObjAllocStrategy<U2DefaultAllocator>();
	ms_pMemMgr->m_pAllocator = new U2MemTracker(
		new U2SmallObjAllocStrategy<U2DefaultAllocator>());
	U2ASSERT(ms_pMemMgr->m_pAllocator);
	ms_pMemMgr->m_pAllocator->Init();
}

void U2MemoryMgr::Terminate()
{
	U2ASSERT(ms_pMemMgr->m_pAllocator);
	ms_pMemMgr->m_pAllocator->Terminate();
	SAFE_DELETE(ms_pMemMgr->m_pAllocator);
	SAFE_DELETE(ms_pMemMgr)	;
}

bool U2MemoryMgr::VertifyPtr(const void* pvMem)
{
	U2ASSERT(ms_pMemMgr->m_pAllocator);
	return ms_pMemMgr->m_pAllocator->VertifyPtr(pvMem);
}