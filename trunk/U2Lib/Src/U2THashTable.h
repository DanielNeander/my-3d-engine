/************************************************************************
module	:	U2THashTable
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once 
#ifndef U2_THASHTABLE_H
#define U2_THASHTABLE_H

#include <U2Lib/Src/Memory/U2MemObj.h>

template<class KEY, class VAL>
class HashItem : public U2MemObj
{
public:
	KEY m_key;
	VAL m_val;
	HashItem* m_pNext;
};

template<class KEY, class VAL>
class  U2THashTable : public U2MemObj
{
public:
	
	
	U2THashTable(int tbleSize);
	~U2THashTable();

	unsigned int AvailSize() const;

	bool Insert(const KEY& key, const VAL& val);

	VAL  Find(const KEY& key) const;

	bool Remove(const KEY& key);
	void RemoveAll();

	VAL  FirstVal(KEY* pKey) const;
	VAL  NextVal(KEY* pKey) const;

	//HashItem<KEY,VAL>* FirstItem(KEY* pKey) const;
	//HashItem<KEY,VAL>* NextItem(KEY* pKey) const;



	int (*UserHashFunc)(const KEY&);

private:
	


	int KeyToIndexFunc(const KEY& key) const;

	unsigned int m_uTblSize;
	unsigned int m_uAvailSize;
	// U2_ALLOC 으로 이중포인터로 할당하면 에러... 
	// 이유는.. 
	HashItem<KEY, VAL>** m_pTbl;

	mutable unsigned int m_uIdx;
	mutable HashItem<KEY,VAL>* m_pItem;

	U2THashTable(const U2THashTable&);
	U2THashTable& operator= (const U2THashTable&);

};

#include "U2THashTable.inl"

#endif