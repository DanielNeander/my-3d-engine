inline void* U2DefaultAllocator::Allocate(size_t& sizeInBytes, 
										  size_t& alignment, 
										  uint32 uiMemFlags,
										  U2MemType eMemType, 
										  const char* szFile, 
										  int line, 
										  const char* szFunction)
{
	if(!U2MemoryMgr::IsInit())
		return NULL;
	
	U2ASSERT(IS_POWER_OF_TWO(alignment));

	return _aligned_malloc(sizeInBytes, alignment);
}


inline void U2DefaultAllocator::Deallocate(void *pvMem, 
										   U2MemType eMemType, 
										   size_t sizeInBytes)
{
	if(NULL ==pvMem)
		return;

	_aligned_free(pvMem);	
}


inline void* U2DefaultAllocator::Reallocate(void* pvMem, 
											size_t& sizeInBytes, 
											size_t& alignment, 
											uint32 uiMemFlags,
											U2MemType eMemType, 
											size_t sizeCurr, 
											const char* szFile, 
											int line, 
											const char* szFunction)
{
	U2ASSERT(IS_POWER_OF_TWO(alignment));

	U2ASSERT(sizeInBytes != 0);

	return _aligned_realloc(pvMem, sizeInBytes, alignment);

}


inline bool U2DefaultAllocator::TrackAllocate(
	const void* const pvMem, 
	size_t sizeInBytes, 
	uint32 uiMemFlags,
	U2MemType eMemType, 
	const char* szFile, 
	int line, 
	const char* szFunction)
{
	return false;
}


inline bool U2DefaultAllocator::TrackDeallocate(const void *const pvMem, 
												U2MemType eMemType)
{
	return false;
}


inline bool U2DefaultAllocator::SetMark(const char*, const char*, const char*)
{
	return true;
}


inline void U2DefaultAllocator::Init()
{
	FILE_LOG(logDEBUG) << _T("U2DefaultAllocator::Init - Success...");
}


inline void U2DefaultAllocator::Terminate()
{

}


inline bool U2DefaultAllocator::VertifyPtr(const void *pvMem)
{
	return true;
}