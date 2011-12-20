#include "stdafx.h"
#include "U2Dictionary.h"

namespace Util {

template<class KEY, class VALUE> 
Dictionary<KEY, VALUE>::Dictionary()
:m_bBulkInsert(false)
{

}

template<class KEY, class VALUE> 
void Dictionary<KEY, VALUE>::Clear()
{
	m_hashItemArray.RemoveAll();
}

template<class KEY, class VALUE> 
bool Dictionary<KEY, VALUE>::IsEmpty() const
{
	return (0 == m_hashItemArray.Size());
}

template<class KEY, class VALUE> 
void Dictionary<KEY, VALUE>::Add(const HashItem<KEY, VALUE>& rhs)
{
	if(m_bBulkInsert)
	{
		m_hashItemArray.AddElem(&rhs);
	}

}

template<class KEY, class VALUE> 
void Dictionary<KEY, VALUE>::Reserve(UINT numElems)
{
	m_hashItemArray.Resize(numElems);
}

template<class KEY, class VALUE> 
void Dictionary<KEY, VALUE>::Add(const KEY& key, const VALUE& val)
{
	if(m_bBulkInsert)		
	{
		HashItem *pNew = U2_NEW HashItem;
		pNew->m_key = key;
		pNew->m_val = val;
		m_hashItemArray.AddElem(pNew);
	}
}

template<class KEY, class VALUE> 
void Dictionary<KEY, VALUE>::EndBulkAdd()
{
	m_bBulkInsert = true;
}



template<class KEY, class VALUE> 
void Dictionary<KEY, VALUE>::Erase(const KEY& key)
{

}

template<class KEY, class VALUE> 
void Dictionary<KEY, VALUE>::EraseAt(UINT index)
{

}

template<class KEY, class VALUE> 
UINT Dictionary<KEY, VALUE>::FindIndex(const KEY& key) const
{

}

template<class KEY, class VALUE> 
bool Dictionary<KEY, VALUE>::Contains(const KEY& key) const
{

}

template<class KEY, class VALUE> 
const KEY& Dictionary<KEY, VALUE>::KeyAt(UINT idx) const
{

}

template<class KEY, class VALUE> 
VALUE& Dictionary<KEY, VALUE>::ValueAt(UINT idx) const
{

}

template<class KEY, class VALUE> 
const VALUE& Dictionary<KEY, VALUE>::ValueAt(UINT idx) const
{

}

template<class KEY, class VALUE> 
HashItem<KEY, VALUE>* Dictionary<KEY, VALUE>::HashItemAt(UINT idx) const
{
	
}

//template<class RETURN> RETURN KeysAs() const;
//template<class RETURN> RETURN ValueAs() const;

}