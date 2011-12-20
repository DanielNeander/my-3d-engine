
template<class VAL>
inline 
U2TStringHashTable<VAL>::U2TStringHashTable(uint32 tblSize, uint32 growBy)
{	
	m_uTblSize = 0;
	m_uAvailSize = 0;
	m_uIdx = 0;
	m_pItem = 0;
	m_uGrowBy = growBy;
	m_ppTbl = 0;

	Resize(tblSize);
	//if(tblSize > 0)
	//{
	//	m_ppTbl = U2_ALLOC(HashItem*, m_uTblSize);
	//	memset(m_ppTbl, 0, m_uTblSize * sizeof(HashItem*));
	//}
	//else 
	//	m_ppTbl = 0;
}

template<class VAL>
inline 
U2TStringHashTable<VAL>::~U2TStringHashTable()
{
	RemoveAll();
	U2_FREE( m_ppTbl );
	m_ppTbl = NULL;
}

template<class VAL>
inline 
void U2TStringHashTable<VAL>::RemoveAll()
{
	if(m_uAvailSize > 0)
	{
		for(int idx=0; idx < m_uTblSize; ++idx)
		{
			while(m_ppTbl[idx])
			{
				HashItem* pSave = m_ppTbl[idx];
				m_ppTbl[idx] = m_ppTbl[idx]->m_pNext;
				U2_DELETE pSave;
				pSave = NULL;
				if(--m_uAvailSize == 0)
				{
					return;
				}
			}
		}
	}
}


template<class VAL>
inline 
unsigned int U2TStringHashTable<VAL>::AvailSize() const 
{
	return m_uAvailSize;
}

template<class VAL>
inline 
void  U2TStringHashTable<VAL>::Resize(uint32 tblSize)
{	
	if(tblSize == m_uTblSize)
		return;
	
	if(tblSize > m_uTblSize)
	{
		m_uTblSize = tblSize;

		HashItem** ppSaveTbl = m_ppTbl;
		m_uTblSize = tblSize+m_uGrowBy;
		m_ppTbl = U2_ALLOC(HashItem*, m_uTblSize);
		U2ASSERT(m_ppTbl);
		memset(m_ppTbl, 0, m_uTblSize * sizeof(HashItem*));

		if(ppSaveTbl)
		{		
			uint32 i;
			for(i=0; i < tblSize+1; ++i)
			{
				m_ppTbl[i] = ppSaveTbl[i];
			}	
			for(i =tblSize+1; i < m_uTblSize; ++i)
			{
				m_ppTbl[i] = 0;
			}
			U2_FREE(ppSaveTbl);
			ppSaveTbl = NULL;
		}
	}
}



template<class VAL>
inline 
bool U2TStringHashTable<VAL>::Insert(const U2DynString& key, const VAL& val)
{
	int idx = KeyToIndexFunc(key);

	Resize(idx);

	HashItem* pItem = m_ppTbl[idx];

	while(pItem)
	{
		if(key == pItem->m_key)
			return false;
		pItem = pItem->m_pNext;
	}

	pItem = U2_NEW HashItem;
	pItem->m_key = key;
	pItem->m_val = val;
	pItem->m_pNext = m_ppTbl[idx];
	m_ppTbl[idx] = pItem;
	m_uAvailSize++;

	return true;
}


template<class VAL>
inline 
VAL* U2TStringHashTable<VAL>::Find(const U2DynString& key) const
{
	int idx = KeyToIndexFunc(key);

	if(idx >= m_uTblSize)
		return 0;

	HashItem* pItem = m_ppTbl[idx];

	while(pItem)
	{
		if(key == pItem->m_key)
			return &pItem->m_val;
		pItem = pItem->m_pNext;
	}
	return 0;
}


template<class VAL>
inline 
bool U2TStringHashTable<VAL>::Remove(const U2DynString& key)
{
	int idx = KeyToIndexFunc(key);
	HashItem* pItem = m_ppTbl[idx];

	if(!pItem)
		return false;

	if(key == pItem->m_key)
	{
		HashItem* pSaveItem = pItem;
		m_ppTbl[idx] = pItem->m_pNext;
		U2_DELETE pSaveItem;
		m_uAvailSize--;
		return true;
	}

	HashItem* pPrevItem = pItem;
	HashItem* pCurrItem = pItem->m_pNext;
	while(pCurrItem & key != pCurrItem->m_key)
	{
		pPrevItem = pCurrItem;
		pCurrItem = pCurrItem->m_pNext;
	}
	
	if(pCurrItem)
	{
		pPrevItem->m_pNext = pCurrItem->m_pNext;
		U2_DELETE pCurrItem;
		m_uAvailSize--;
		return true;
	}

	return false;
}


template<class VAL>
inline 
VAL* U2TStringHashTable<VAL>::FirstVal(U2DynString* pKey) const 
{
	if(m_uAvailSize > 0)
	{
		for(m_uIdx = 0; m_uIdx < m_uTblSize; m_uIdx++)
		{
			if(m_ppTbl[m_uIdx])
			{
				m_pItem = m_ppTbl[m_uIdx];
				*pKey = m_pItem->m_key;
				return &m_pItem->m_val;
			}
		}

	}

	return 0;
}

template<class VAL>
inline 
VAL* U2TStringHashTable<VAL>::NextVal(U2DynString* pKey) const 
{
	if(m_uAvailSize > 0)
	{
		m_pItem = m_pItem->m_pNext;
		if(m_pItem)
		{
			*pKey = m_pItem->m_key;
			return &m_pItem->m_val;
		}

		for(m_uIdx++;m_uIdx < m_uTblSize; m_uIdx++)
		{
			if(m_ppTbl[m_uIdx])
			{
				m_pItem = m_ppTbl[m_uIdx];
				*pKey = m_pItem->m_key;
				return &m_pItem->m_val;
			}
		}

	}

	return 0;
}

template<class VAL>
inline 
int U2TStringHashTable<VAL>::KeyToIndexFunc(const U2DynString& key) const
{
	int hiKey = 0, loKey = 0;
	for(int i=0; i < (int)key.Length(); ++i)
	{
		if(i & 1)
			hiKey += (int)key[i];
		else
			loKey += (int)key[i];
	}
	return abs(loKey * hiKey) % m_uTblSize;
}