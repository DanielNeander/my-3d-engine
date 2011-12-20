/************************************************************************
module	:	U2TStringHashTable
Author	:	Yun sangyong
Desc	:	inline 함수는 dll내보내기를 적용하면 안된다..
************************************************************************/
#pragma once 
#ifndef U2_THASHSET_H
#define U2_THASHSET_H

#include <U2Lib/src/U2ContainerAlloc.h>
#include <U2Lib/Src/Memory/U2MemObj.h>


template<class KEY>
class U2THashSet : public U2MemObj
{
public:
	U2THashSet(int tblSize);
	~U2THashSet();

	unsigned int AvailSize() const;

	KEY* Insert(const KEY& key);
	KEY* Get(const KEY& key) const;

	bool Remove(const KEY& key);
	void RemoveAll();

	KEY* FirstKey() const;
	KEY* NextKey() const;

	int (*UserHashFunc)(const KEY&);

private:
	class HashItem : public U2MemObj
	{
	public:
		KEY m_key;
		HashItem* m_pNext;
	};


	int KeyToIndexFunc(const KEY& key) const;

	unsigned int m_uTblSize;
	unsigned int m_uAvailSize;
	HashItem** m_pTbl;

	mutable unsigned int m_uIdx;
	mutable HashItem* m_pItem;

	U2THashSet(const U2THashSet&);
	U2THashSet operator=(const U2THashSet&);
};


#include "U2THashSet.inl"

#endif