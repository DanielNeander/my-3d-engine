template<class KEY, class VAL>
inline 
U2THashTable<KEY,VAL>::U2THashTable(int tbleSize) 
:m_uTblSize(tbleSize)
{
	U2ASSERT(m_uTblSize > 0);

	m_uAvailSize = 0;
	m_uIdx = 0;
	m_pItem = 0;

	uint32 byteInSize = sizeof(HashItem<KEY,VAL>*) * m_uTblSize;
	m_pTbl = (HashItem<KEY,VAL>**)U2_MALLOC(byteInSize);
	memset(m_pTbl, 0, m_uTblSize  * sizeof(HashItem<KEY,VAL>*));
	UserHashFunc = 0;
}


template<class KEY, class VAL>
inline 
U2THashTable<KEY,VAL>::~U2THashTable()
{
	RemoveAll();
	U2_FREE(m_pTbl);
	m_pTbl = NULL;
}


template<class KEY, class VAL>
inline 
unsigned int U2THashTable<KEY,VAL>::AvailSize() const
{
	return m_uAvailSize;
}

template<class KEY, class VAL>
inline 
bool U2THashTable<KEY,VAL>::Insert(const KEY& key, const VAL& val)
{
	int idx = KeyToIndexFunc(key);
	HashItem<KEY,VAL>* pItem = m_pTbl[idx];

	while(pItem)
	{
		if(key == pItem->m_key)
		{
			return false;
		}
		pItem = pItem->m_pNext;
	}

	pItem = U2_NEW HashItem<KEY,VAL>;
	pItem->m_key = key;
	pItem->m_val = val;
	pItem->m_pNext = m_pTbl[idx];
	m_pTbl[idx] = pItem;	
	++m_uAvailSize;

	return true;
}


template<class KEY, class VAL>
inline 
VAL U2THashTable<KEY,VAL>::Find(const KEY& key) const
{
	int idx = KeyToIndexFunc(key);
	HashItem<KEY,VAL>* pItem = m_pTbl[idx];

	while(pItem)
	{
		if(key == pItem->m_key)
		{
			return pItem->m_val;
		}
		pItem = pItem->m_pNext;
	}

	return 0;
}


template<class KEY, class VAL>
inline 
bool U2THashTable<KEY,VAL>::Remove(const KEY& key)
{
	int idx = KeyToIndexFunc(key);
	HashItem<KEY,VAL>* pItem = m_pTbl[idx];

	if(!pItem)
	{
		return false;
	}

	if(key == pItem->m_key)
	{
		HashItem<KEY,VAL>* pSaveItem = pItem;
		m_pTbl[idx]= pItem->m_pNext;		
		U2_DELETE pSaveItem;
		m_uAvailSize--;
		return true;		
	}

	HashItem<KEY,VAL>* pPrev = pItem;
	HashItem<KEY,VAL>* pCurr = pItem->m_pNext;
	while( pCurr && key != pCurr->m_key)
	{
		pPrev = pCurr;
		pCurr = pCurr->m_pNext;
	}

	if(pCurr)
	{
		pPrev->m_pNext = pCurr->m_pNext;
		U2_DELETE pCurr;
		m_uAvailSize--;
		return true;
	}

	return false;	
}


template<class KEY, class VAL>
inline 
void U2THashTable<KEY,VAL>::RemoveAll()
{
	if(m_uAvailSize > 0)
	{
		for(uint32 idx = 0; idx < m_uTblSize; idx++)
		{
			while(&m_pTbl[idx])
			{
				HashItem<KEY,VAL>* pSaveItem = m_pTbl[idx];
				m_pTbl[idx] = m_pTbl[idx]->m_pNext;				
				U2_DELETE pSaveItem;
				if(--m_uAvailSize == 0)
					return;				
			}
		}
	}
}


template<class KEY, class VAL>
inline 
VAL U2THashTable<KEY,VAL>::FirstVal(KEY* pKey) const
{
	if(m_uAvailSize > 0)
	{
		for(m_uIdx = 0; m_uIdx < m_uTblSize; ++m_uIdx)
		{
			if(&m_pTbl[m_uIdx])
			{
				m_pItem = m_pTbl[m_uIdx];
				*pKey = m_pItem->m_key;
				return m_pItem->m_val;
			}
		}
	}
	return 0;
}

template<class KEY, class VAL>
inline 
VAL U2THashTable<KEY,VAL>::NextVal(KEY* pKey) const
{
	if(m_uAvailSize > 0)
	{
		m_pItem = m_pItem->m_pNext;
		if(m_pItem)
		{
			*pKey = m_pItem->m_key;
			return m_pItem->m_val;
		}

		for(++m_uIdx; m_uIdx < m_uTblSize; ++m_uIdx)
		{
			if(m_pTbl[m_uIdx])
			{
				m_pItem = m_pTbl[m_uIdx];
				*pKey = m_pItem->m_key;
				return m_pItem->m_val;
			}
		}
	}

	return 0;
}

template<class KEY, class VAL>
inline 
int U2THashTable<KEY,VAL>::KeyToIndexFunc(const KEY& key) const
{
	if(UserHashFunc)
		return (*UserHashFunc)(key);

	// default hash function
	static double s_hashMultiplier = 0.5f * (sqrt(5.0) - 1.0);
	unsigned int uKey;
	memcpy(&uKey, &key, sizeof(unsigned int));
	uKey %= m_uTblSize;
	double fraction = fmod(s_hashMultiplier*uKey, 1.0);
	return (int)floor(m_uTblSize * fraction);
}
