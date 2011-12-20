//-------------------------------------------------------------------------------------------------
inline void U2AllocUnit::Reset()
{
	m_stAllocationId = (size_t)-1;
	m_stAlignment = 0;
	m_ulAllocThreadId = 0;
	m_pvMem = NULL;
	m_stRequestedSize = 0;
	m_stAllocatedSize = 0;
	m_eAllocType = MEM_UNKNOWN;
	m_eAllocFlags = MEMFLAG_NONE;
	m_fAllocTime = -FLT_MAX;
	m_pPrev = NULL;
	m_pNext = NULL;
}


//-------------------------------------------------------------------------------------------------
inline bool U2MemTracker::GetAlwaysValidateAll() const
{
	return m_bAlwaysValidateAll;
}

//-------------------------------------------------------------------------------------------------
inline void U2MemTracker::SetAlwaysValidateAll(bool bOn)
{
	m_bAlwaysValidateAll = bOn;
}

//-------------------------------------------------------------------------------------------------
inline bool U2MemTracker::GetCheckForArrayOverruns() const
{
	return m_bCheckArrayOverruns;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2MemTracker::AddressToHashIndex(const void* pvAddress)
{

	return (unsigned int)(reinterpret_cast<size_t>(pvAddress) >> 4) & (ms_uiHashSize - 1);
}

//-------------------------------------------------------------------------------------------------
inline U2MemTracker* U2MemTracker::Instance()
{
	return ms_pTracker;
}

//-------------------------------------------------------------------------------------------------
inline size_t U2MemTracker::GetCurrtAllocID() const
{
	return m_StCurrAllocID;
}

//-------------------------------------------------------------------------------------------------
inline bool U2MemTracker::IsInsideBreakRange(const void *pvMem, size_t size)
{
	return ((((const char*)pvMem) + size) >= ms_pvBreakOnAllocRangeStart && 
		(pvMem <= ms_pvBreakOnAllocRangeEnd));
}


