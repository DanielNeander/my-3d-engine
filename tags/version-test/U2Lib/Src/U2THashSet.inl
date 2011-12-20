
template<class KEY>
inline 
U2THashSet<KEY>::U2THashSet(int uTblSize) 
{
	if(uTblSize == 0)
		uTblSize= 1;

	m_uTblSize = uTblSize;
	m_uAvailSize = 0;
	m_uIdx = 0;
	m_pItem = 0;
	m_pTbl = U2_ALLOC(HashItem*, uTblSize);
	memset(m_pTbl, 0, m_uTblSize * sizeof(HashItem*));
	UserHashFunc = 0;
}


template<class KEY>
inline 
U2THashSet<KEY>::~U2THashSet() 
{
	RemoveAll();
	U2_FREE(m_pTbl);
	m_pTbl = NULL;
}

template<class KEY>
inline 
unsigned int U2THashSet<KEY>::AvailSize() const 
{
	return m_uAvailSize;
}

template<class KEY>
inline 
KEY* U2THashSet<KEY>::Insert(const KEY& key)
{
	int idx = KeyToIndexFunc(key);
	HashItem* pItem = m_pTbl[idx];

	while(pItem)
	{
		if(key == pItem->m_key)
			return &pItem->m_key;
		
		pItem = pItem->m_pNext;
	}

	pItem = U2_NEW HashItem;
	pItem->m_key = key;
	pItem->m_pNext = m_pTbl[idx];
	m_pTbl[idx] = pItem;
	m_uAvailSize++;
	
	return &pItem->m_key;
}


template<class KEY>
inline 
KEY* U2THashSet<KEY>::Get(const KEY& key) const 
{
	int idx = KeyToIndexFunc(key);
	HashItem* pItem = m_pTbl[idx];

	while(pItem)
	{
		if(key == pItem->m_key)
			return &pItem->m_key;

		pItem = pItem->m_pNext;
	}

	return 0;
}

template<class KEY>
inline 
bool U2THashSet<KEY>::Remove(const KEY& key)
{
	int idx = KeyToIndexFunc(key);
	HashItem* pItem = m_pTbl[idx];

	if(!pItem)
		return false;

	if(key == pItem->m_key)
	{
		HashItem* pSaveItem = pItem;
		m_pTbl[idx] = pItem->m_pNext;
		
		U2_DELETE pSaveItem;
		pSaveItem = NULL;
		m_uAvailSize--;
		return true;
	}

	HashItem* pPrevItem = pItem;
	HashItem* pCurrItem = pItem->m_pNext;
	while(pCurrItem && key != pCurrItem->m_key)
	{
		pPrevItem = pCurrItem;
		pCurrItem = pCurrItem->m_pNext;
	}

	if(pCurrItem)
	{
		pPrevItem->m_pNext = pCurrItem->m_pNext;
		U2_DELETE pCurrItem;
		pCurrItem = NULL;
		m_uAvailSize--;
		return true;
	}

	return false;
}


template<class KEY>
inline 
void U2THashSet<KEY>::RemoveAll()
{
	if(m_uAvailSize > 0)
	{
		for(unsigned int idx = 0; idx < m_uTblSize; idx++)
		{
			while(m_pTbl[idx])
			{
				HashItem* pSaveItem = m_pTbl[idx];
				m_pTbl[idx] = m_pTbl[idx]->m_pNext;
				U2_DELETE pSaveItem;
				if(--m_uAvailSize == 0)
					return;
			}
		}
	}
}



template<class KEY>
inline 
KEY* U2THashSet<KEY>::FirstKey() const 
{
	if(m_uAvailSize > 0)
	{
		for(m_uIdx = 0; m_uIdx < m_uTblSize; m_uIdx++)
		{
			if(m_pTbl[m_uIdx])
			{
				m_pItem = m_pTbl[m_uIdx];
				return &m_pItem->m_key;
			}
		}
	}
	return 0;
}


template<class KEY>
inline 
KEY* U2THashSet<KEY>::NextKey() const 
{
	if(m_uAvailSize > 0)
	{
		m_pItem = m_pItem->m_pNext;
		if(m_pItem)
			return &m_pItem->m_key;
	}

	for(m_uIdx++;m_uIdx < m_uTblSize; m_uIdx++)
	{
		if(m_pTbl[m_uIdx])
		{
			m_pItem = m_pTbl[m_uIdx];
			return &m_pItem->m_key;
		}
	}
	return 0;
}

template<class KEY>
inline 
int U2THashSet<KEY>::KeyToIndexFunc(const KEY& key) const 
{
	if(UserHashFunc)
		return (*UserHashFunc)(key);

	static double s_hashMultiplier = 0.5 * (sqrt(5.0)-1.0);
	unsigned int uKey;
	memcpy(&uKey, &key, sizeof(unsigned int));
	uKey %= m_uTblSize;
	double fraction = fmod(s_hashMultiplier*uKey, 1.0);
	return (int)floor(m_uTblSize * fraction);
}

