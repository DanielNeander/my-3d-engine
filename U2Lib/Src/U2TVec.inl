template<class T, class TAlloc> 
inline 
U2TVec<T, TAlloc>::U2TVec(uint32 maxSize, uint32 incrSize)
:m_pBase(0),
 m_uiSize(0),	
 m_uiFirstAvailEmpty(0),
 m_uiFilledSize(0),
 m_uiIncreSize(incrSize)
{
	Resize(maxSize);
}

template<class T, class TAlloc> 
inline 
U2TVec<T, TAlloc>::~U2TVec()
{
	TAlloc::Deallocate(m_pBase);
}

template<class T, class TAlloc>
inline 
uint32 U2TVec<T, TAlloc>::Size() const
{
	return m_uiSize;
}

template<class T, class TAlloc>
inline 
uint32 U2TVec<T, TAlloc>::FilledSize() const
{
	return m_uiFirstAvailEmpty;
}

template<class T, class TAlloc>
inline 
uint32 U2TVec<T, TAlloc>::NonzeroElemSize() const
{
	return m_uiFilledSize;
}


template<class T, class TAlloc>
inline 
T* U2TVec<T, TAlloc>::GetBase()
{
	return m_pBase;
}

template<class T, class TAlloc>
inline 
void U2TVec<T, TAlloc>::SetIncreSize(uint32 increSize)
{
	m_uiIncreSize = increSize;
}


template<class T, class TAlloc>
inline 
void	U2TVec<T, TAlloc>::Resize(uint32 reSize)
{	
	// �޸� ���� ������ �д�.
	U2ASSERT(reSize <= UINT_MAX);

	if( m_uiSize == reSize)
		return;

	// ���� �迭�� ������� �� �۰� ����.
	// T(0)�� Smart Pointer ȿ���� �̿�

	uint32 i;
	if(reSize < m_uiFirstAvailEmpty)
	{
		for(i = reSize; i < m_uiSize; ++i)
		{
			if(m_pBase[i] != T(0))
			{
				m_pBase[i] = T(0);
				m_uiFilledSize--;
			}
		}

		m_uiFirstAvailEmpty = reSize;
		
	}

	T* pTempBase = m_pBase;
	m_uiSize = reSize;

	if(m_uiSize > 0)
	{
		m_pBase = TAlloc::Allocate(m_uiSize);
		U2ASSERT(m_pBase);

		for(i = 0; i < m_uiFirstAvailEmpty; ++i)
		{
			m_pBase[i] = pTempBase[i];
		}

		for(i = m_uiFirstAvailEmpty ; i < m_uiSize ; ++i)
		{
			m_pBase[i] = T(0);
		}
	}
	else 
	{
		m_pBase = 0;
	}

	TAlloc::Deallocate(pTempBase);

}


template<class T, class TAlloc>
inline 
const	T&	U2TVec<T, TAlloc>::GetElem(uint32 index) const
{
	U2ASSERT(index < m_uiSize);
	return m_pBase[index];
}


template<class T, class TAlloc>
inline
T&	U2TVec<T, TAlloc>::GetElem(uint32 index)
{
	U2ASSERT(index < m_uiSize);
	return m_pBase[index];
}


template<class T, class TAlloc>
inline
const T&	U2TVec<T, TAlloc>::operator[](uint32 idx) const
{
	return GetElem(idx);
}

template<class T, class TAlloc>
inline
T&	U2TVec<T, TAlloc>::operator[](uint32 idx)
{
	return GetElem(idx);
}



template<class T, class TAlloc>
inline
void U2TVec<T, TAlloc>::SetElem(uint32 idx, const T& elem)
{
	U2ASSERT(idx < m_uiSize);

	if(idx >= m_uiFirstAvailEmpty)
	{
		m_uiFirstAvailEmpty = idx + 1;
		if(elem != T(0))
		{
			m_uiFilledSize++;
		}
	}
	else 
	{
		if(elem != T(0))
		{
			if(m_pBase[idx] == T(0))
			{
				m_uiFilledSize++;
			}
		}
		else 
		{
			if(m_pBase[idx] != T(0))
			{
				m_uiFilledSize--;
			}
		}
	}

	m_pBase[idx] = elem;
}

template<class T, class TAlloc>
inline
uint32 U2TVec<T, TAlloc>::AddElem(const T& elem)
{
	return SetSafeElem(m_uiFirstAvailEmpty, elem);
}



template<class T, class TAlloc>
inline
uint32	U2TVec<T, TAlloc>::SetSafeElem(unsigned int idx, const T& elem)
{
	U2ASSERT(idx <= UINT_MAX);
	if(idx >= m_uiSize)
	{
		Resize(idx + m_uiIncreSize);
	}

	SetElem(idx, elem);
	return idx;
}


template<class T, class TAlloc>
inline
uint32 U2TVec<T, TAlloc>::AddEmptySlot(const T& elem)
{
	if(elem == T(0))
	{
		return -1;
	}

	for(unsigned int i = 0; i < m_uiFirstAvailEmpty; ++i)
	{
		if(m_pBase[i] == T(0))
		{
			m_pBase[i] = elem;
			m_uiFilledSize++;
			return i;
		}
	}

	return SetSafeElem(m_uiFirstAvailEmpty, elem);
}





template<class T, class TAlloc>
inline
T U2TVec<T, TAlloc>::Remove(uint32 idx)
{
	if(idx >= m_uiFirstAvailEmpty)
	{
		return T(0);
	}

	// �����Ϸ��� �� ���ϰ����� ���
	T elem = m_pBase[idx];
	m_pBase[idx] = T(0);

	if(elem != T(0))
	{
		m_uiFilledSize--;
	}

	// �迭�� ������ ������..
	if(idx == (m_uiFirstAvailEmpty-1))
	{
		m_uiFirstAvailEmpty--;
	}
	
	return elem;
}

template<class T, class TAlloc>
inline
T U2TVec<T, TAlloc>::RemoveAndFill(unsigned int uiIdx)
{
	if(uiIdx >= m_uiFirstAvailEmpty)
	{
		return T(0);
	}

	m_uiFirstAvailEmpty--;
	T elem = m_pBase[uiIdx];
	
	m_pBase[uiIdx] = m_pBase[m_uiFirstAvailEmpty];
	m_pBase[m_uiFirstAvailEmpty] = T(0);

	if(elem != T(0))
	{
		m_uiFilledSize--;
	}

	return elem;
}
	
template<class T, class TAlloc>
inline
T U2TVec<T, TAlloc>::RemoveEnd()
{
	if(m_uiFirstAvailEmpty == 0)
		return T(0);

	m_uiFirstAvailEmpty--;
	T elem = m_pBase[m_uiFirstAvailEmpty];
	m_pBase[m_uiFirstAvailEmpty] = T(0);

	if(elem != T(0))
	{
		m_uiFilledSize--;
	}

	return elem;
}
	
template<class T, class TAlloc>
inline
void	U2TVec<T, TAlloc>::Refresh()
{
	if(m_uiFilledSize == m_uiFirstAvailEmpty)
		return;

	// �迭 ��ҵ��� �迭�� ���ۿ��� �������� �������� �̵���Ų��.
	if(m_uiFilledSize)
	{
		for(unsigned short i=0, j=0; j < m_uiFirstAvailEmpty; ++i)
		{
			if(m_pBase[i] != T(0))
			{
				if(m_pBase[j] != m_pBase[i])
					m_pBase[j] = m_pBase[i];
				j++;
			}
		}
	}

	T* pSaveBase = m_pBase;
	m_uiFirstAvailEmpty = m_uiFilledSize;
	m_uiSize = m_uiFirstAvailEmpty;
	if(m_uiSize > 0)
	{
		m_pBase = TAlloc::Allocate(m_uiSize);
		U2ASSERT(m_pBase);

		for(unsigned short i=0; i < m_uiFirstAvailEmpty; ++i)
		{
			m_pBase[i] = pSaveBase[i];
		}
	}
	else 
	{
		m_pBase = 0;
	}

	TAlloc::Deallocate(pSaveBase);
}


template<class T, class TAlloc>
inline
void U2TVec<T, TAlloc>::RemoveAll()
{
	for(uint32 i = 0; i < m_uiFirstAvailEmpty; ++i)
	{
		m_pBase[i] = T(0);
	}

	m_uiFirstAvailEmpty = 0;
	m_uiFilledSize = 0;

}

template<class T, class TAlloc>
inline
uint32	U2TVec<T, TAlloc>::Remove(const T& elem)
{
	if(elem != T(0))
	{
		for(uint32 i = 0; i < m_uiFirstAvailEmpty; ++i)
		{
			if(m_pBase[i] == elem)
			{
				m_pBase[i] = T(0);
				m_uiFilledSize--;

				if(i == m_uiFirstAvailEmpty - 1)
					m_uiFirstAvailEmpty--;

				return i;
			}
		}
	}

	return -1;
}

template<class T, class TAlloc>
inline
uint32	U2TVec<T, TAlloc>::Find(const T& elem, uint32 startIdx = 0)
{
	if(elem != T(0))
	{
		for(uint32 i = startIdx; i < m_uiFirstAvailEmpty; ++i)
		{
			if(m_pBase[i] == elem)
			{
				return i;
			}
		}
	}

	return -1;	
}


template<class T, class TAlloc>
inline
void	U2TVec<T, TAlloc>::UpdateFirstEmpty()
{
	while (m_uiFirstAvailEmpty > 0)
	{
		if (m_pBase[m_uiFirstAvailEmpty - 1] != T(0))
		{
			break;
		}

		m_uiFirstAvailEmpty--;
	}
}

template<class T>
inline 
U2ObjVec<T>::U2ObjVec(uint32 maxSize, uint32 increSize)
: U2TVec<T,  U2NewAlloc<T>>(maxSize, increSize)
{

}

template<class T>
inline 
U2PrimitiveVec<T>::U2PrimitiveVec(uint32 maxSize, uint32 increSize)
: U2TVec<T,  U2MallocAlloc<T>>(maxSize, increSize)
{

}

