
template<class TAllocator>
U2SmallObjAllocStrategy<TAllocator>::U2SmallObjAllocStrategy(size_t chunkSize)
	: m_chunkSize(chunkSize)	
{
	for(size_t i = 1 ; i <= MAX_SMALL_OBJ_SIZE; ++i)
	{
		m_pool[i-1].Init(&m_allocator, i);
	}
}

template<class TAllocator>
void* U2SmallObjAllocStrategy<TAllocator>::Allocate(size_t numBytes)
{
	U2ASSERT(numBytes != 0);
	U2ASSERT(numBytes <= MAX_SMALL_OBJ_SIZE);
	U2ASSERT(m_pool[numBytes-1].BlockSize() == numBytes);


	return m_pool[numBytes-1].Allocate();
}

template<class TAllocator>
void U2SmallObjAllocStrategy<TAllocator>::Deallocate(void* p, size_t numBytes)
{
	U2ASSERT(numBytes != 0);
	U2ASSERT(numBytes <= MAX_SMALL_OBJ_SIZE);
	U2ASSERT(m_pool[numBytes-1].BlockSize() == numBytes);
	return m_pool[numBytes-1].Deallocate(p);
};

template<class TAllocator>
U2FixedAllocator* U2SmallObjAllocStrategy<TAllocator>::GetFixedAllocatorForSize(size_t numBytes)
{
	U2ASSERT(numBytes != 0);
	U2ASSERT(numBytes <= MAX_SMALL_OBJ_SIZE);
	U2ASSERT(m_pool[numBytes-1].BlockSize() == numBytes);
	return &m_pool[numBytes-1];		
}

template<class TAllocator>
size_t U2SmallObjAllocStrategy<TAllocator>::GetSizeFromAddress(void* pvMem)
{
	size_t size;
	char* pcMem = (char*)pvMem;
	char* pcSize = (char*)&size;
	U2ASSERT(sizeof(size_t) == 4);
	pcSize[0] = pcMem[0];
	pcSize[1] = pcMem[1];
	pcSize[2] = pcMem[2];
	pcSize[3] = pcMem[3];

	return size;
}

template<class TAllocator>
void U2SmallObjAllocStrategy<TAllocator>::SetSizeToAddress(void* pvMem, size_t size )
{
	char* pcMemory = (char*)pvMem;
	char* pcSize = (char*)&size;

	U2ASSERT(sizeof(size_t) == 4);
	pcMemory[0] = pcSize[0];
	pcMemory[1] = pcSize[1];
	pcMemory[2] = pcSize[2];
	pcMemory[3] = pcSize[3];
}


template<class TAllocator>
void* U2SmallObjAllocStrategy<TAllocator>::Allocate(size_t& sizeInBytes,
													size_t& alignment, 
													uint32 uiMemFlags,
													U2MemType eMemType, 
													const char* szFile, 
													int line, 
													const char* szFunction)
{
	U2ASSERT(IS_POWER_OF_TWO(alignment));
	
	bool bPrependSize = false;
	void* pvMemory = NULL;

	bool bAddAlignment = (uiMemFlags & MEMFLAG_COMPILER_PROVIDE_SIZE_ON_DEALLOCATE) != 0;

	// NiMemTracker의 m_bCheckArrayOverrruns 멤버 사용 확인.. 
	// 배열의 메모리 범위를 충분히 보장하기 위해.. 
	//sizeInBytes += 2 * alignment;

	if(alignment == U2_MEM_ALIGNMENT && 
		!bAddAlignment &&
		eMemType != MEM_ALIGNEDMALLOC && 
		eMemType != MEM_ALIGNEDREALLOC)
	{
		sizeInBytes += U2_MEM_ALIGNMENT;
		bPrependSize= true;
	}

	if(eMemType != MEM_ALIGNEDMALLOC && 
		eMemType != MEM_ALIGNEDREALLOC && 
		sizeInBytes <= MAX_SMALL_OBJ_SIZE && 
		alignment == U2_MEM_ALIGNMENT)
	{
		pvMemory = Allocate(sizeInBytes);

	}
	else 
	{
		/*pvMemory = m_allocator.Allocate(
			sizeInBytes,
			alignment,
			uiMemFlags,
			eMemType,
			szFile,
			line,
			szFunction);*/
		pvMemory = U2ExternAlignedMalloc(sizeInBytes, alignment);
	}
	
	//FILE_LOG(logDEBUG) << szFile << _T(" ") << line <<  _T(" ") << szFunction;

	if(pvMemory && bPrependSize)
	{
		U2ASSERT(U2_MEM_ALIGNMENT >= sizeof(size_t));
		SetSizeToAddress(pvMemory, sizeInBytes);
		pvMemory = ((char*)pvMemory) + U2_MEM_ALIGNMENT;
	}

	return pvMemory;
}

template<class TAllocator>
void U2SmallObjAllocStrategy<TAllocator>::Deallocate(void* pvMem, U2MemType eMemType, size_t sizeInBytes)
{
	if(pvMem == NULL)
		return;

	////Memtracker의 Deallocate 함수의 m_bCheckArrayOverrruns 확인..			
	//if(sizeInBytes != U2_MEM_DEALLOC_SIZE)
	//	sizeInBytes += 2 * U2_MEM_ALIGNMENT;

	if(eMemType != MEM_ALIGNEDFREE && eMemType != MEM_ALIGNEDREALLOC && 
		U2_MEM_DEALLOC_SIZE == sizeInBytes) 
	{
		

		pvMem = ((char*)pvMem) - U2_MEM_ALIGNMENT;
		U2ASSERT(U2_MEM_ALIGNMENT >= sizeof(size_t));

		sizeInBytes = GetSizeFromAddress(pvMem);
	}


	if(sizeInBytes <= MAX_SMALL_OBJ_SIZE)
	{
		Deallocate(pvMem, sizeInBytes);
		return;
	}

	//m_allocator.Deallocate(pvMem, eMemType, sizeInBytes);
	U2ExternAlignedFree(pvMem);
}


template<class TAllocator>
void* U2SmallObjAllocStrategy<TAllocator>::Reallocate(void *pvMemory, 
													  size_t &sizeInBytes, 
													  size_t &alignment, 
													  uint32 uiMemFlags,
													  U2MemType eMemType, 
													  size_t currSize, 
													  const char *szFile, 
													  int line, 
													  const char *szFunction)
{
	U2ASSERT(sizeInBytes != 0);

	if(eMemType != MEM_ALIGNEDFREE && eMemType != MEM_ALIGNEDREALLOC && 
		U2_MEM_DEALLOC_SIZE == currSize )
	{
		pvMemory = ((char*)pvMemory) - U2_MEM_ALIGNMENT;
		U2ASSERT(U2_MEM_ALIGNMENT >= sizeof(size_t));
		currSize= GetSizeFromAddress(pvMemory);
	}

	bool bPrependSize = false;
	if(alignment == U2_MEM_ALIGNMENT &&
		eMemType != MEM_ALIGNEDMALLOC &&
		eMemType != MEM_ALIGNEDREALLOC)
	{
		sizeInBytes += U2_MEM_ALIGNMENT;
		bPrependSize = true;
	}

	if(currSize != U2_MEM_ALIGNMENT &&
		sizeInBytes <= currSize && bPrependSize)
	{
		sizeInBytes = currSize;
		pvMemory = ((char*)pvMemory) + U2_MEM_ALIGNMENT;
		return pvMemory;
	}

	void*	pvNewMemory = NULL;
	if(eMemType != MEM_ALIGNEDMALLOC &&
		eMemType != MEM_ALIGNEDREALLOC &&
		(currSize <= MAX_SMALL_OBJ_SIZE) || (sizeInBytes <= MAX_SMALL_OBJ_SIZE))
	{
		U2ASSERT(alignment == U2_MEM_ALIGNMENT);
		U2ASSERT(currSize != U2_MEM_DEALLOC_SIZE);

		sizeInBytes -= U2_MEM_ALIGNMENT;


		pvNewMemory = Allocate(sizeInBytes, alignment, uiMemFlags, eMemType, szFile, line, szFunction);
		bPrependSize = false;

		if(currSize != U2_MEM_DEALLOC_SIZE)
		{
			char* pvMemToCopy = ((char*)pvNewMemory) + U2_MEM_ALIGNMENT;
			size_t sizeToCopy = currSize - U2_MEM_ALIGNMENT;

			memcpy_s(pvNewMemory, sizeInBytes, pvMemToCopy, sizeToCopy);
			Deallocate(pvMemory, eMemType, currSize);
		}
	}
	else
	{
		pvNewMemory = m_allocator.Reallocate(
			pvMemory, 
			sizeInBytes,
			alignment,
			uiMemFlags,
			eMemType,
			currSize,
			szFile,
			line,
			szFunction);
	}

	if(pvNewMemory && bPrependSize)
	{
		U2ASSERT(U2_MEM_ALIGNMENT >= sizeof(size_t));
		SetSizeToAddress(pvNewMemory, sizeInBytes);
		pvNewMemory = ((char*)pvNewMemory) + U2_MEM_ALIGNMENT;
	}
	
	return pvNewMemory;
}


template<class TAllocator>
bool U2SmallObjAllocStrategy<TAllocator>::TrackAllocate(const void* const pvMemory, 
														size_t sizeInBytes, 
														uint32 uiMemFlags,
														U2MemType eMemType, 														
														const char* szFile,
														int line, 
														const char* szFunction)
{
	return m_allocator.TrackAllocate(pvMemory,
		sizeInBytes,
		uiMemFlags,
		eMemType, 
		szFile,
		line,
		szFunction);
}


template<class TAllocator>
bool U2SmallObjAllocStrategy<TAllocator>::TrackDeallocate(const void* const pvMem, 
														  U2MemType eMemType)
{
	return m_allocator.TrackDeallocate(pvMem,
		eMemType);
}

template<class TAllocator>
bool U2SmallObjAllocStrategy<TAllocator>::SetMark(const char* szMarkType, 
												  const char* szClassfier,
												  const char* szStr)
{
	return m_allocator.SetMark(szMarkType, szClassfier, szStr);
}

template<class TAllocator>
void  U2SmallObjAllocStrategy<TAllocator>::Init()
{
	m_allocator.Init();
}

template<class TAllocator>
void U2SmallObjAllocStrategy<TAllocator>::Terminate()
{
	m_allocator.Terminate();
}

template<class TAllocator>
bool U2SmallObjAllocStrategy<TAllocator>::VertifyPtr(const void* pvMem)
{
	return m_allocator.VertifyPtr(pvMem);
}