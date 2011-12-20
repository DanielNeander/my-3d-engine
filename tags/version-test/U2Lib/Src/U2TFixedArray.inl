template<class T, class TAlloc>
inline 
U2TFixedArray<T, TAlloc>::U2TFixedArray()
:m_uSize(0),
m_pBase(0)
{

}


template<class T, class TAlloc>
inline 
U2TFixedArray<T, TAlloc>::U2TFixedArray(uint32 size)
:m_uSize(0),
m_pBase(0)
{
	this->Allocate(size);
}


template<class T, class TAlloc>
inline 
U2TFixedArray<T, TAlloc>::U2TFixedArray(const U2TFixedArray<T, TAlloc>& rhs)
:m_uSize(0),
m_pBase(0)
{
	Copy(rhs);
}


template<class T, class TAlloc>
inline 
U2TFixedArray<T, TAlloc>::~U2TFixedArray()
{
	TAlloc::Deallocate(m_pBase);
	m_uSize = 0;
}


template<class T, class TAlloc>
inline 
void U2TFixedArray<T, TAlloc>::SetSize(uint32 s)
{	
	if(m_uSize != s)
	{
		this->Allocate(s);
	}
	
}


template<class T, class TAlloc>
inline 
uint32 U2TFixedArray<T, TAlloc>::Size() const
{
	return m_uSize;
}


template<class T, class TAlloc>
inline 
void U2TFixedArray<T, TAlloc>::Clear(T elem)
{
	if(m_pBase)
	{
		uint32 i;
		for(i=0; i < m_uSize; ++i)
		{
			m_pBase[i] = elem;
		}
	}
}


template<class T, class TAlloc>
inline 
void U2TFixedArray<T, TAlloc>::operator=(const U2TFixedArray<T, TAlloc>& rhs)
{
	Copy(rhs);
}


template<class T, class TAlloc>
inline 
T& U2TFixedArray<T, TAlloc>::operator[] (int i) const
{
	U2ASSERT(m_pBase && 0 <= i && i < (int)m_uSize);

	return m_pBase[i];	
}


template<class T, class TAlloc>
inline 
uint32 U2TFixedArray<T, TAlloc>::Find(const T& e) const
{
	int i=0;
	for(i=0; i < m_uSize; ++i)
	{
		if(e == m_pBase[i])
		{
			return i;
		}
	}

	return -1;
}


template<class T, class TAlloc>
inline 
void U2TFixedArray<T, TAlloc>::Delete()
{
	if(m_pBase)
	{
		TAlloc::Deallocate(m_pBase);
		m_pBase = 0;

	}
	m_uSize = 0;
}


template<class T, class TAlloc>
inline 
void U2TFixedArray<T, TAlloc>::Allocate(int s)
{
	this->Delete();
	if(s > 0)
	{
		m_pBase = TAlloc::Allocate(s);
		m_uSize = s;
	}
}


template<class T, class TAlloc>
inline 
void U2TFixedArray<T, TAlloc>::Copy(const U2TFixedArray<T, TAlloc>& src)
{
	if(this != &src)
	{
		TAlloc::Allocate(src.m_uSize);
		uint32 i;
		for(i=0; i < m_uSize; ++i)
		{
			m_pBase[i] = src.m_pBase[i];
		}
	}
}


//-------------------------------------------------------------------------------------------------
template<class T>
inline
U2FixedObjArray<T>::U2FixedObjArray() 
{

}


template<class T>
inline
U2FixedObjArray<T>::U2FixedObjArray(uint32 size)
:U2TFixedArray<T, U2NewAlloc<T> >(size)
{

}
template<class T>
inline
U2FixedObjArray<T>::U2FixedObjArray(const U2FixedObjArray<T>& rhs)
:U2TFixedArray<T, U2NewAlloc<T>>(rhs)
{

}

//-------------------------------------------------------------------------------------------------

template<class T>
inline
U2FixedPrimitiveArray<T>::U2FixedPrimitiveArray() 
{

}


template<class T>
inline
U2FixedPrimitiveArray<T>::U2FixedPrimitiveArray(uint32 size)
:U2TFixedArray<T, U2MallocAlloc<T> >(size)
{

}

template<class T>
inline
U2FixedPrimitiveArray<T>::U2FixedPrimitiveArray(const U2FixedPrimitiveArray<T>& rhs)
:U2TFixedArray<T, U2MallocAlloc<T>>(rhs)
{

}

