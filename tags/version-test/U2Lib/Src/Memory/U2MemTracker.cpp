#include <src/U2LibPCH.h>
#include "U2MemTracker.h"


size_t U2MemTracker::ms_stBreakOnAllocID = INT_MAX;
void* U2MemTracker::ms_pvBreakOnAllocRangeStart = NULL;
void* U2MemTracker::ms_pvBreakOnAllocRangeEnd = NULL;
const TCHAR* U2MemTracker::ms_pcBreakOnFunctionName = _T("@*");
U2MemTracker* U2MemTracker::ms_pTracker = NULL;
bool U2MemTracker::ms_bOutputLeaksToDebugStream = true;
size_t U2MemTracker::ms_stBreakOnSizeRequested = INT_MAX;

//-------------------------------------------------------------------------------------------------
void  U2MemTracker::Init()
{
	m_StCurrAllocID = 0;
	ResetSummaryStats();

	if(m_pActualAllocator)
		m_pActualAllocator->Init();
}

//-------------------------------------------------------------------------------------------------
void  U2MemTracker::Terminate()
{
	if(m_pActualAllocator)
		m_pActualAllocator->Terminate();
}


void U2MemTracker::ResetSummaryStats()
{
		m_fPeakMemTime = 0.0f;
		m_fPeakAllocCntTime = 0.0f;
		m_stActiveMem = 0;
		m_stPeakMem = 0;
		m_stAccumMem = 0;
		m_stUnusedButAllocatedMem = 0;

		m_stActiveAllocCnt = 0;
		m_stPeakAllocCnt = 0;
		m_stAccumAllocCnt = 0;

		m_stActiveExternMem = 0; 
		m_stPeakExternMem = 0; 
		m_stAccumExternMem = 0; 

		m_stActiveExternAllocCnt = 0;
		m_stPeakExternAllocCnt = 0;
		m_stAccumExternAllocCnt = 0;
	
}

U2MemTracker::U2MemTracker(U2Allocator* pActualAllocator, bool bWriteToLog /* = true */, 
						   unsigned int uiInitSize /* = 65536 */, 
						   unsigned int uiGrowBy /* = 4096 */, 
						   bool bAlwaysValidateAll /* = false */, 
						   bool bCheckArrayOverruns /* = true */)
						   :m_fPeakMemTime(0.0f), m_fPeakAllocCntTime(0.0f),
						   m_bAlwaysValidateAll(bAlwaysValidateAll), 
						   m_stReservoirGrowBy(uiGrowBy),
						   m_bCheckArrayOverruns(bCheckArrayOverruns), m_ucFillChar(0xbd),
						   m_bWriteToLog(bWriteToLog)
{
	ms_pTracker = this;
	m_pActualAllocator = pActualAllocator;
	memset(m_pActiveMem, 0, ms_uiHashSize * sizeof(U2AllocUnit*));
	if(uiInitSize > 0)
	{
		m_stReservoirBuffSize = 1;
		m_pReservoir = (U2AllocUnit*)calloc(uiInitSize, sizeof(U2AllocUnit));

		for(unsigned int i=0; i < uiInitSize - 1; ++i)
		{
			m_pReservoir[i].m_pNext = &m_pReservoir[i+1];
		}
		m_pReservoir[uiInitSize - 1].m_pNext = NULL;

		m_ppReservoirBuff = (U2AllocUnit**)malloc(sizeof(U2AllocUnit*));

		m_ppReservoirBuff[0] = m_pReservoir;

		unsigned int uiInitBytes = uiInitSize * sizeof(U2AllocUnit) * sizeof(U2AllocUnit*);
		m_stActiveTrackerOverhead = uiInitBytes;
		m_stPeakTrackerOverhead = uiInitBytes;
		m_StAccumTrackerOverhead = uiInitBytes;
	}
	else 
	{
		m_stReservoirBuffSize = 0;
	}
}

U2MemTracker::~U2MemTracker()
{
	if(m_ppReservoirBuff)
	{
		for(unsigned int i = 0; i < m_stReservoirBuffSize; ++i)
		{
			free(m_ppReservoirBuff[i]);
		}
		free(m_ppReservoirBuff);
	}
	ms_pTracker = NULL;

	delete m_pActualAllocator;
}

void U2MemTracker::GrowReservoir()
{
	m_pReservoir = (U2AllocUnit*)malloc(m_stReservoirGrowBy * sizeof(U2AllocUnit));

	U2ASSERT(m_pReservoir);

	m_stActiveTrackerOverhead += sizeof(U2AllocUnit) * 
		m_stReservoirGrowBy + sizeof(U2AllocUnit*);

	if(m_stActiveTrackerOverhead > m_stPeakTrackerOverhead)
	{
		m_stPeakTrackerOverhead = m_stActiveTrackerOverhead;
	}

	m_StAccumTrackerOverhead += sizeof(U2AllocUnit) * m_stReservoirGrowBy + sizeof(U2AllocUnit*);

	for(unsigned int i=0; i < m_stReservoirGrowBy - 1; ++i)
	{
		m_pReservoir[i].m_pNext = &m_pReservoir[i+1];
	}

	m_pReservoir[m_stReservoirGrowBy-1].m_pNext = NULL;

	U2AllocUnit **ppTemp = (U2AllocUnit**)realloc(m_ppReservoirBuff, 
		(m_stReservoirBuffSize + 1) * sizeof(U2AllocUnit*));

	U2ASSERT(ppTemp);
	if(ppTemp)
	{
		m_ppReservoirBuff = ppTemp;
		m_ppReservoirBuff[m_stReservoirBuffSize] = m_pReservoir;
		m_stReservoirBuffSize++;
	}
}

//-------------------------------------------------------------------------------------------------
void* U2MemTracker::Allocate(
					   size_t& sizeInBytes, 
					   size_t& alignment, 
					   uint32 bMemFlags,
					   U2MemType eMemType,
					   const char* szFile,
					   int line,
					   const char* szFunction)
{
	size_t origSize = sizeInBytes;
	float fTime = GetCurrTimeInSec();

	if(m_bCheckArrayOverruns)
	{
		sizeInBytes = PadForArrayOverrun(alignment, sizeInBytes);
	}

	U2ASSERT(strcmp(ms_pcBreakOnFunctionName, szFunction) != 0);

	U2ASSERT(ms_stBreakOnAllocID != m_StCurrAllocID);

	U2ASSERT(ms_stBreakOnSizeRequested != origSize);

	void* pvMem = m_pActualAllocator->Allocate(sizeInBytes, alignment, 
		bMemFlags, eMemType, szFile, line, szFunction);

	if(!pvMem)
	{
		return NULL;
	}

	U2ASSERT(!FindAllocUnit(pvMem));

	// update summary statistics
	m_stActiveAllocCnt++;
	m_stAccumAllocCnt++;
	if(m_stActiveAllocCnt > m_stPeakAllocCnt)
	{
		m_stPeakAllocCnt = m_stActiveAllocCnt;
		m_fPeakAllocCntTime = fTime;
	}

	m_stActiveMem += sizeInBytes;
	m_stAccumMem += sizeInBytes;
	if(m_stActiveMem > m_stPeakMem)
	{
		m_stPeakMem = m_stActiveMem;
		m_fPeakMemTime = fTime;
	}


	// If you hit this NIMEMASSERT, your memory request result was smaller 
	// than the input.
	U2ASSERT(sizeInBytes >= origSize);

	// If you hit this NIMEMASSERT, you requested a breakpoint on a specific
	// allocation address range.
	U2ASSERT(!IsInsideBreakRange(pvMem, sizeInBytes));

	// Pad the start and end of the allocation with the pad character 
	// so that we can check for array under and overruns. Note that the
	// address is shifted to hide the padding before the allocation.
	if(m_bCheckArrayOverruns)
	{
		MemfillForArrayOverrun(pvMem, alignment, origSize);
	}

	{
		MemFillWithPattern(pvMem, origSize);
	}

	// If you hit this NIMEMASSERT, the somehow you have allocated a memory
	// unit to an address that already exists. This should never happen
	// and is an indicator that something has gone wrong in the sub-allocator.
	U2ASSERT(FindAllocUnit(pvMem) == NULL);

	// Grow the tracking unit reservoir if necessary

	if(!m_pReservoir)
		GrowReservoir();

	U2AllocUnit* pUnit = m_pReservoir;
	m_pReservoir = pUnit->m_pNext;

	pUnit->Reset();
	pUnit->m_stAllocationId = m_StCurrAllocID;
	pUnit->m_stAlignment = alignment;
	pUnit->m_ulAllocThreadId = (unsigned long)GetCurrentThreadId();
	pUnit->m_eAllocType = eMemType;
	pUnit->m_eAllocFlags = (U2MemFlag)bMemFlags;
	pUnit->m_fAllocTime = fTime;
	pUnit->m_pvMem = pvMem;
	pUnit->m_stRequestedSize = origSize;
	pUnit->m_stAllocatedSize = sizeInBytes;
	
	InsertAllocUnit(pUnit);

	if(m_bAlwaysValidateAll)
	{
		bool bValidateAllAllocUnits = ValidateAllAllocUnits();
	}

	++m_StCurrAllocID;
	
	return pvMem;
}

void U2MemTracker::InsertAllocUnit(U2AllocUnit* pUnit)
{
	U2ASSERT(pUnit && pUnit->m_pvMem);

	unsigned int uiHashIdx = AddressToHashIndex(pUnit->m_pvMem);

	if(m_pActiveMem[uiHashIdx])
	{
		m_pActiveMem[uiHashIdx]->m_pPrev = pUnit;
	}

	pUnit->m_pNext = m_pActiveMem[uiHashIdx];
	pUnit->m_pPrev = NULL;
	m_pActiveMem[uiHashIdx] = pUnit;
}

void U2MemTracker::RemoveAllocUnit(U2AllocUnit *pUnit)
{
	U2ASSERT(pUnit && pUnit->m_pvMem);

	unsigned int uiHashIdx = AddressToHashIndex(pUnit->m_pvMem);

	if(m_pActiveMem[uiHashIdx] == pUnit)
	{
		m_pActiveMem[uiHashIdx] = pUnit->m_pNext;

		if(m_pActiveMem[uiHashIdx])
		{
			m_pActiveMem[uiHashIdx]->m_pPrev = NULL;
		}
	}
	else 
	{
		if(pUnit->m_pPrev)
		{
			pUnit->m_pPrev->m_pNext = pUnit->m_pNext;
		}

		if(pUnit->m_pNext)
		{
			pUnit->m_pNext->m_pPrev = pUnit->m_pPrev;
		}
	}

}

void U2MemTracker::Deallocate(
						void* pvMem,
						U2MemType eMemType,
						size_t sizeInBytes)
{
	if(pvMem)
	{
		try {
			float fTime = GetCurrTimeInSec();

			U2AllocUnit* pUnit = FindAllocUnit(pvMem);

			if(pUnit == NULL)
			{
				throw _T("Request to deallocate RAM that was never allocated");
				return;
			}

			U2ASSERT(ms_stBreakOnAllocID != pUnit->m_stAllocationId);
			U2ASSERT(ms_stBreakOnSizeRequested != pUnit->m_stRequestedSize);

			U2ASSERT(!IsInsideBreakRange(pUnit->m_pvMem, pUnit->m_stRequestedSize));

			if(sizeInBytes != U2_MEM_DEALLOC_SIZE)
				U2ASSERT(sizeInBytes == pUnit->m_stRequestedSize);

			bool bValidateAllocUnit = ValidateAllocUnit(pUnit);
			U2ASSERT(bValidateAllocUnit);

			U2ASSERT(eMemType != MEM_UNKNOWN);

			U2MemType eDeallocType = eMemType;
			float fDeallocTime = fTime;

			size_t unusedSize = MemBytesWithPattern(pvMem, pUnit->m_stRequestedSize);
			m_stUnusedButAllocatedMem += unusedSize;

			if(m_bCheckArrayOverruns)
			{

				// If you hit this NIMEMASSERT, you have code that overwrites
				// either before or after the range of an allocation.
				// Check the pkUnit for information about which allocation
				// is being overwritten. 
				bool bCheckForArrayOverrun = 
					CheckForArrayOverrun(pvMem, pUnit->m_stAlignment, 
					pUnit->m_stRequestedSize);
				U2ASSERT(!bCheckForArrayOverrun);

				if(sizeInBytes != U2_MEM_DEALLOC_SIZE)
				{
					sizeInBytes = PadForArrayOverrun(pUnit->m_stAlignment, 
						sizeInBytes);
				}
			}

			m_pActualAllocator->Deallocate(pvMem, eMemType, sizeInBytes);

			RemoveAllocUnit(pUnit);

			--m_stActiveAllocCnt;
			m_stActiveMem -= pUnit->m_stAllocatedSize;

			if(m_bAlwaysValidateAll)
			{
				bool bValidateAllAllocUnits = ValidateAllAllocUnits();
				U2ASSERT(bValidateAllAllocUnits);					
			}
		
			pUnit->m_pNext = m_pReservoir;
			m_pReservoir = pUnit;

			if(m_bAlwaysValidateAll)
			{
				bool bValidateAllAllocUnits = 
					ValidateAllAllocUnits();

				U2ASSERT(bValidateAllAllocUnits);
			}			
		}
		catch(const TCHAR* err)
		{
			FDebug("Error : %s, (Line : %d, Function : %s)", err, __LINE__, __FUNCTION__);
			
		}
	}
}

//-------------------------------------------------------------------------------------------------
void* U2MemTracker::Reallocate(
						 void* pvMem,
						 size_t& sizeInBytes,		
						 size_t& alignment,
						 uint32 bMemFlags,
						 U2MemType eMemType,
						 size_t sizeCurr,
						 const char* szFile, int line,
						 const char* szFunction)
{
	return NULL;
}

//-------------------------------------------------------------------------------------------------
bool U2MemTracker::TrackAllocate(
						   const void* const pvMem,
						   size_t sizeInBytes,
						   uint32 bMemFlags,
						   U2MemType eMemType,
						   const char* szFile,
						   int line, 
						   const char* szFunction)
{
	return true;
}

//-------------------------------------------------------------------------------------------------
bool U2MemTracker::TrackDeallocate(
							 const void* const pvMem,
							 U2MemType eMemType)
{
	return true;
}

//-------------------------------------------------------------------------------------------------
bool  U2MemTracker::SetMark(const char* szMarkType, const char* szClassfier, 
					 const char* szStr) 
{
	return true;
}

//-------------------------------------------------------------------------------------------------
U2AllocUnit* U2MemTracker::FindAllocUnit(const void* pvMem) const
{
	U2ASSERT(pvMem);

	// Use the address to locate the hash index. Note that we shift off the 
	// lower four bits. This is because most allocated addresses will be on 
	// four-, eight- or even sixteen-byte boundaries. If we didn't do this, 
	// the hash index would not have very good coverage.

	unsigned int uiHashIndex = AddressToHashIndex(pvMem);

	U2AllocUnit* pkUnit = m_pActiveMem[uiHashIndex];
	while(pkUnit)
	{
		if (pkUnit->m_pvMem == pvMem) 
			return pkUnit;

		pkUnit = pkUnit->m_pNext;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------------------
bool U2MemTracker::VertifyPtr(const void *pvMem)
{
	U2AllocUnit *pUnit = FindAllocUnit(pvMem);
	if(pUnit)
	{
		if(m_bCheckArrayOverruns && pUnit->m_eAllocType != MEM_EXTERNALLOC)
		{
			void* pvMemStore = pUnit->m_pvMem;
			if(CheckForArrayOverrun(pvMemStore, pUnit->m_stAlignment, 
				pUnit->m_stRequestedSize))
			{
				FDebug("Mem overrun found at Allocation:\n");
				return false;
			}			
		}

		return true;
	}

	// The Xbox 360 inserts an alignment-sized header into the allocation
	// when a destructor needs to be called. This value holds the count of 
	// elements that were allocated. However, it will be unknown to this 
	// function if such a header were actually added since all we have is
	// an address. Therefore, we must search backwards to some reasonable 
	// degree of certainty to find the matching allocation unit.

	// 4 is the minimum offset/alignment for all supported compilers.
	size_t stOffset = 4;

	// The maximum offset that is supported by this function is 128. 
	// Anything larger is probably unreasonable on modern systems.
	while(stOffset <= 128)
	{
		unsigned char* pcAdjustMem = ((unsigned char*)pvMem) - stOffset;
		pUnit = FindAllocUnit(pcAdjustMem);

		// We must be careful that the the allocation unit exists and is 
		// big enough to actually encompass the address and isn't actually 
		// right in front of it.
		if(pUnit && (pcAdjustMem + pUnit->m_stRequestedSize) >= 
			(unsigned char*)pvMem)
		{
			if(m_bCheckArrayOverruns && pUnit->m_eAllocType != MEM_EXTERNALLOC)
			{
				void* pvMemStore = pUnit->m_pvMem;
				if(CheckForArrayOverrun(pvMemStore, pUnit->m_stAlignment, 
					pUnit->m_stRequestedSize))
				{
					FDebug("Mem overrun found at Allocation:\n");
					return false;
				}			

			}
			return true;
		}
		// If we've found an allocation unit that exists before the 
		// address we are looking up, there is no need to keep searching,
		// this address is unknown to us.
		else if(pUnit)
		{
			return false;
		}

		stOffset += 2;
	}

	return false;

}

//-------------------------------------------------------------------------------------------------
void* U2MemTracker::FindContainingAlloc(const void* pvMemory)
{
	for (unsigned int uiHash = 0; uiHash < ms_uiHashSize; uiHash++)
	{
		U2AllocUnit* pkUnit = m_pActiveMem[uiHash];
		while(pkUnit)
		{
			void* pvBottom = pkUnit->m_pvMem;
			void* pvTop = (void*)(
				((size_t)pvBottom) + pkUnit->m_stAllocatedSize);
			if (pvBottom <= pvMemory && pvMemory < pvTop) 
			{
				void* pvResult = pkUnit->m_pvMem;
				
				return pvResult;
			}

			pkUnit = pkUnit->m_pNext;
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------------------------------
void U2MemTracker::MemfillForArrayOverrun(void*& pvMem, size_t alignment, size_t origSize)
{
	char* pcMem = (char*)pvMem;
	pvMem = pcMem + alignment;
	MemFillWithPattern(pcMem, alignment);

	pcMem = pcMem + alignment + origSize;
	MemFillWithPattern(pcMem, alignment);
}

//-------------------------------------------------------------------------------------------------
void U2MemTracker::MemFillWithPattern(void* pvMem, size_t sizeInBytes)
{
	unsigned char* pcMem = (unsigned char*)pvMem;
	for(unsigned int ui=0; ui < sizeInBytes; ++ui)
	{
		pcMem[ui] = m_ucFillChar;
	}
}

//-------------------------------------------------------------------------------------------------
size_t U2MemTracker::MemBytesWithPattern(void* pvMem, size_t sizeInBytes) const 
{
	unsigned char* pcMem = (unsigned char*)pvMem;
	size_t numBytes = 0;
	for(unsigned int ui=0; ui < sizeInBytes; ++ui)
	{
		if(pcMem[ui] == m_ucFillChar)
		{
			numBytes++;
		}
	}

	return numBytes;
}

//-------------------------------------------------------------------------------------------------
bool U2MemTracker::CheckForArrayOverrun(void*& pvMem, size_t alignment, size_t origSize) const
{
	U2ASSERT(m_bCheckArrayOverruns);

	char* pcMem = (char*)pvMem;
	pcMem -= alignment;
	pvMem = pcMem;

	if(alignment != MemBytesWithPattern(pcMem, alignment))
		return true;

	pcMem = pcMem + alignment + origSize;
	if(alignment != MemBytesWithPattern(pcMem, alignment))
		return true;

	return false;
}

//-------------------------------------------------------------------------------------------------
size_t U2MemTracker::PadForArrayOverrun(size_t alignment, size_t sizeOrig)
{
	return sizeOrig + 2 * alignment;
}

bool U2MemTracker::ValidateAllocUnit(const U2AllocUnit* pkUnit) const
{
	if (pkUnit->m_stAllocationId > m_StCurrAllocID)
		return false;

	if (pkUnit->m_stAllocatedSize < pkUnit->m_stRequestedSize)
		return false;

	if (pkUnit->m_stAllocatedSize == 0 ||  pkUnit->m_stRequestedSize == 0)
		return false;

	if (pkUnit->m_pvMem == NULL)
		return false;

	if (pkUnit->m_pNext != NULL && pkUnit->m_pNext->m_pPrev != pkUnit)
		return false;

	return true;

}

bool U2MemTracker::ValidateAllAllocUnits() const
{
	unsigned int uiActiveCount = 0;
	for (unsigned int uiHashIndex = 0; uiHashIndex < ms_uiHashSize; 
		uiHashIndex++)
	{
		U2AllocUnit* pkUnit = m_pActiveMem[uiHashIndex];
		U2AllocUnit* pkPrev = NULL;

		while(pkUnit)
		{
			if (!ValidateAllocUnit(pkUnit))
				return false;

			if (pkUnit->m_pPrev != pkPrev)
				return false;

			if (m_bCheckArrayOverruns)
			{
				void* pvMemStore = pkUnit->m_pvMem;
				if(CheckForArrayOverrun(pvMemStore, pkUnit->m_stAlignment, 
					pkUnit->m_stRequestedSize))
				{
					FDebug("Mem overrun found at Allocation:\n");
					return false;
				}			
			}

			pkPrev = pkUnit;
			pkUnit = pkUnit->m_pNext;
			uiActiveCount++;
		}
	}

	if(uiActiveCount != this->m_stActiveAllocCnt)
		return false;

	return true;
}







