//--------------------------------------------------------------------------------
// T_Array
//
// Copyright (C) 2003-2009 Jason Zink
//--------------------------------------------------------------------------------
template <class T>
T_Array<T>::T_Array()
{
	m_iQuantity = 0;
	m_iCapacity = 5;

	m_aData = new T[m_iCapacity];
}
//--------------------------------------------------------------------------------
template <class T>
T_Array<T>::T_Array(const T_Array& item)
{
	m_aData = NULL;
	*this = item;
}
//--------------------------------------------------------------------------------
template <class T>
T_Array<T>::~T_Array()
{
	delete[] m_aData;
}
//--------------------------------------------------------------------------------
template <class T>
T_Array<T>& T_Array<T>::operator=(const T_Array& item)
{
	m_iQuantity = item.m_iQuantity;
	m_iCapacity = item.m_iCapacity;

	delete[] m_aData;
	
	m_aData = new T[m_iCapacity];

	for (int i = 0; i < m_iCapacity; i++)
		m_aData[i] = item.m_aData[i];

	return *this;
}
//--------------------------------------------------------------------------------
template <class T>
void T_Array<T>::add(const T& element)
{
	if ( m_iQuantity == m_iCapacity )
	{
		m_iCapacity = m_iCapacity * 2 + 1;

		T* aNewArray = new T[m_iCapacity];
		for ( int i = 0; i < m_iQuantity; i++ )
			aNewArray[i] = m_aData[i];

		delete[] m_aData;
		m_aData = aNewArray;
	}

	m_aData[m_iQuantity] = element;
	m_iQuantity++;
}
//--------------------------------------------------------------------------------
template <class T>
void T_Array<T>::remove(int index)
{
	if ((index >= 0) && (index < m_iQuantity))
	{
		for ( int i = index; i < m_iQuantity-1; i++ )
		{
			m_aData[i] = m_aData[i+1];
		}
		m_iQuantity--;
	}
}
//--------------------------------------------------------------------------------
template <class T>
void T_Array<T>::empty( )
{
	m_iQuantity = 0;
}
//--------------------------------------------------------------------------------
template <class T>
int T_Array<T>::count()
{
	return( m_iQuantity );
}
//--------------------------------------------------------------------------------
template <class T>
bool T_Array<T>::contains( const T& element )
{
	for ( int i = 0; i < m_iQuantity; i++ )
	{
		if ( m_aData[i] == element )
			return true;
	}

	return false;
}
//--------------------------------------------------------------------------------
template <class T>
int T_Array<T>::find( const T& element )
{
	for ( int i = 0; i < m_iQuantity; i++ )
	{
		if ( m_aData[i] == element )
			return( i );
	}

	return( -1 );
}
//--------------------------------------------------------------------------------
template <class T>
T& T_Array<T>::operator[] (int i)
{
	if ( i < 0 )
		i = 0;

	if ( i > m_iQuantity )
		i = m_iQuantity - 1;

	return m_aData[i];
}
//--------------------------------------------------------------------------------
template <class T>
const T& T_Array<T>::operator[] (int i) const
{
	if ( i < 0 )
		i = 0;

	if ( i > m_iQuantity )
		i = m_iQuantity - 1;

	return m_aData[i];
}
//--------------------------------------------------------------------------------