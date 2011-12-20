
template<class T>
inline	
U2SmartPtr<T>::U2SmartPtr()
	:m_pObj(0)
{
	
}


template<class T>
inline	
U2SmartPtr<T>::U2SmartPtr(T* pObj)
{
	m_pObj = pObj;
	if(m_pObj)
		m_pObj->IncRefCount();
}

template<class T>
inline
U2SmartPtr<T>::U2SmartPtr(const U2SmartPtr<T>& rhs)
{
	m_pObj = rhs.m_pObj;
	if(m_pObj)
		m_pObj->IncRefCount();
}


template<class T>
inline 
U2SmartPtr<T>::~U2SmartPtr()
{
	if(m_pObj)
		m_pObj->DecRefCount();

}


template<class T>
inline
U2SmartPtr<T>::operator	T*() const
{
	return m_pObj;
}


//template<class T>
//inline
//U2SmartPtr<T>::operator	void*()
//{
//	return m_pObj;
//}

template<class T>
inline
T& U2SmartPtr<T>::operator*() const
{
	return *(m_pObj);
}
//
//template<class T>
//inline
//const T& U2SmartPtr<T>::operator*() const
//{
//	return (*m_pObj);
//}

template<class T>
inline
T*	U2SmartPtr<T>::operator->() const
{
	return (m_pObj);
}

//template<class T>
//inline
//const T* U2SmartPtr<T>::operator->() const
//{
//	return (m_pObj);
//}



template<class T>
inline
U2SmartPtr<T>&	U2SmartPtr<T>::operator=(const U2SmartPtr<T>& rhs)
{
	if(m_pObj != rhs.m_pObj)
	{
		if(m_pObj)
			m_pObj->DecRefCount();	
		m_pObj = rhs.m_pObj;
		if(m_pObj)
			m_pObj->IncRefCount();
	}
	return *this;
}

template<class T>
inline
U2SmartPtr<T>&	U2SmartPtr<T>::operator=(T* pObj)
{
	if(m_pObj != pObj)
	{
		if(m_pObj)
			m_pObj->DecRefCount();	
		m_pObj = pObj;
		if(m_pObj)
			m_pObj->IncRefCount();
	}
	return *this;	
}

template<class T>
inline
bool	U2SmartPtr<T>::operator==(const U2SmartPtr<T>& rhs) const
{
	return m_pObj == (rhs.m_pObj);
}

template<class T>
inline
bool	U2SmartPtr<T>::operator!=(const U2SmartPtr<T>& rhs) const
{
	return m_pObj != (rhs.m_pObj);	
}


template<class T>
inline
bool	U2SmartPtr<T>::operator==(T* pObj) const
{
	return m_pObj == pObj;
}


template<class T>
inline
bool	U2SmartPtr<T>::operator!=(T* pObj) const
{
	return m_pObj != pObj;
}


//template<class T,class U>
//bool operator==(const U2SmartPtr<T>& lhs, U* rhs)
//{
//	return lhs.m_pObj == rhs;
//}
//
//template<class T, class U>
//bool operator!=(const U2SmartPtr<T>& lhs, U* rhs)
//{
//	return lhs.m_pObj != rhs;
//}
//
//
//template<class T, class U>
//bool operator==(U* pObj, const U2SmartPtr<T>& rhs)
//{
//	return pObj == rhs.m_pObj;
//}
//
//
//template<class T, class U>
//bool operator!=(U* pObj, const U2SmartPtr<T>& rhs)
//{
//	return pObj != rhs.m_pObj;
//}



