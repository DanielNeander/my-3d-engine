#pragma once 
/************************************************************************
module	:	U2String
Author	:	Yun sangyong 
Date	:	2010-03-10
Desc	:	
************************************************************************/
//#include <U2Lib/Src/U2MemObj.h>
#include <U2Lib/Src/U2THashTable.h>
#include <U2Lib/Src/U2TVec.h>

namespace Util 
{
template<class KEY, class VALUE> 
class Dictionary : public U2MemObj
{
	Dictionary();
	~Dictionary();

	VALUE operator[](const KEY& key) const;
	UINT Size() const; 
	void Clear();
	bool IsEmpty() const;

	void Reserve(UINT numElems);

	/// begin a bulk insert (array will be sorted at End)
	void BeginBulkAdd();
	void Add(const HashItem<KEY, VALUE>& rhs);
	void Add(const KEY& key, const VALUE& val);
	
	/// end a bulk insert (this will sort the internal array)
	void EndBulkAdd();

	void Erase(const KEY& key);
	void EraseAt(UINT index);
	UINT FindIndex(const KEY& key) const;
	bool Contains(const KEY& key) const;
	const KEY& KeyAt(UINT idx) const;
	VALUE& ValueAt(UINT idx) const;
	const VALUE& ValueAt(UINT idx) const;
	HashItem<KEY, VALUE>* HashItemAt(UINT idx) const;

	template<class RETURN> RETURN KeysAs() const;
	template<class RETURN> RETURN ValueAs() const;

protected:
	void SortIfDirty() const;

	U2PrimitiveVec<HashItem<KEY, VALUE>* > m_hashItemArray;
	bool m_bBulkInsert;	

private:
	Dictionary(const Dictionary<KEY, VALUE>& rhs);
	void operator= (const Dictionary<KEY, VALUE>& rhs);
};


}








