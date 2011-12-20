


inline void* U2MemoryMgr::Allocate(size_t& sizeInBytes, size_t alignment, 
								   uint32 uiMemFlags,
								   U2MemType eMemType, 
								   const char* szFile, int line,
								   const char* szFunction)
{
	U2ASSERT(m_pAllocator);

	void *pvMem = m_pAllocator->Allocate(sizeInBytes,
		alignment, 
		uiMemFlags,
		eMemType,
		szFile,
		line,
		szFunction);

	
	return pvMem;
}


inline void* U2MemoryMgr::Reallocate(void *pvMem, 
									 size_t &sizeInBytes, 
									 size_t alignment, 
									 uint32 uiMemFlags,
									 U2MemType eMemType,
									 size_t currSize, 
									 const char *szFile, 
									 int line, 
									 const char *szFunction)
{
	U2ASSERT(m_pAllocator);

	void* pvNewMem = m_pAllocator->Reallocate(pvMem, 
		sizeInBytes, 
		alignment, 
		uiMemFlags,
		eMemType, 
		currSize,
		szFile,
		line,
		szFunction);

	return pvNewMem;
}


inline void	U2MemoryMgr::Deallocate(void* pvMem, U2MemType eMemType, size_t sizeInBytes )
{
	U2ASSERT(m_pAllocator);	
	m_pAllocator->Deallocate(pvMem, eMemType, sizeInBytes);
}


inline bool	U2MemoryMgr::TrackAllocate(const void* const pvMem, 
									   size_t sizeInBytes,
									   uint32 uiMemFlags,
									   U2MemType eMemType,
									   const char* szFile, 
									   int line, const char* szFunction)
{
	U2ASSERT(m_pAllocator);
	return m_pAllocator->TrackAllocate(pvMem, sizeInBytes,
		uiMemFlags,
		eMemType,
		szFile,
		line,
		szFunction);
}


inline bool U2MemoryMgr::TrackDeallocate(const void* const pvMem, U2MemType eMemType)
{
	U2ASSERT(m_pAllocator);
	return m_pAllocator->TrackDeallocate(pvMem, eMemType);
}


inline bool U2MemoryMgr::SetMark(const char* szMarkType,
								 const char* szClassfier, const char* szStr)
{
	U2ASSERT(m_pAllocator);
	return m_pAllocator->SetMark(szMarkType, szClassfier, szStr);
}
									   