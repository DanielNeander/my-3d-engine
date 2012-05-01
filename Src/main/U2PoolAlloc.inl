
template<class T>
inline 
U2PoolAlloc<T>::~U2PoolAlloc()
{
	//this->Terminate();
}


template<class T>
inline 
void U2PoolAlloc<T>::CreatePool()
{
	U2ASSERT(ms_pMemPool == 0);
	U2ASSERT(ms_uNumBlock >= 2);
	//U2ASSERT(!ms_pBlockHeader);

	ms_pMemPool = U2_NEW U2PoolAlloc<T>::Block[ms_uNumBlock];

	// 마지막 블록
	U2PoolAlloc<T>::Block* pCurr = ms_pMemPool + ms_uNumBlock - 1;
	pCurr->m_pNext = NULL;

	uint32 i = 1;
	do 
	{
		pCurr = ms_pMemPool + i;
		pCurr->m_pNext = pCurr + 1;
	} while(i++ < ms_uNumBlock - 2);

	if(ms_pBlockHeader)
	{
		ms_pMemPool->m_pNext = ms_pBlockHeader;
		ms_pBlockHeader = ms_pMemPool;
	}
	else 
	{	
		ms_pBlockHeader = ms_pMemPool;
		ms_pBlockHeader->m_pNext = 0;
	}

	ms_pMemPool = ms_pMemPool + 1;
}

template<class T>
inline 
void U2PoolAlloc<T>::Terminate()
{
	U2PoolAlloc<T>::Block* pCurr = ms_pBlockHeader;

	while(pCurr)
	{
		U2PoolAlloc<T>::Block* pNext = pCurr->m_pNext;
		U2_DELETE [] pCurr;
		pCurr = pNext;
	}

	ms_pBlockHeader = NULL;	
	ms_pMemPool = NULL;
}

template<class T>
inline 
void* U2PoolAlloc<T>::Allocate()
{
	if(!ms_pMemPool)
		U2PoolAlloc<T>::CreatePool();

	U2PoolAlloc<T>::Block* pTemp = ms_pMemPool;
	ms_pMemPool = ms_pMemPool->m_pNext;
	pTemp->m_elem = 0;
	pTemp->m_pNext = 0;
	pTemp->m_pvData = 0;
	return pTemp;	
}


template<class T>
inline 
void U2PoolAlloc<T>::Deallocate(void* ptr)
{
	U2PoolAlloc<T>::Block* pDel = 
		(U2PoolAlloc<T>::Block*)ptr;
	pDel->m_pvData = 0;
	pDel->m_pNext = ms_pMemPool;
	ms_pMemPool = pDel;
}