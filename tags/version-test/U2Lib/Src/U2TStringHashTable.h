/************************************************************************
module	:	U2TStringHashTable
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once 
#ifndef U2_TSTRINGHASHTABLE_H
#define U2_TSTRINGHASHTABLE_H

#include "U2LibType.h"
#include <U2Lib/Src/Memory/U2MemObj.h>

#include "U2Util.h"


template<class VAL>
class  U2TStringHashTable : public U2MemObj
{
public:
	U2TStringHashTable(uint32 tblSize = 0, uint32 growBy = 1);
	~U2TStringHashTable();

	unsigned int AvailSize() const;

	bool Insert(const U2DynString& Key, const VAL& val);
	VAL* Find(const U2DynString& Key) const;

	bool Remove(const U2DynString& key);
	void RemoveAll();

	VAL* FirstVal(U2DynString* pKey) const;
	VAL* NextVal(U2DynString* pKey) const;

	void Resize(uint32 tblSize);

private:
	
	class HashItem : public U2MemObj
	{
	public:
		HashItem() : m_key(_T("")) {}

		U2DynString m_key;
		VAL m_val;
		HashItem* m_pNext;
	};
	

	int KeyToIndexFunc(const U2DynString& key) const;

	unsigned int m_uTblSize;
	unsigned int m_uAvailSize;
	unsigned int m_uGrowBy;
	HashItem** m_ppTbl;

	mutable unsigned int m_uIdx;
	mutable HashItem* m_pItem;

	U2TStringHashTable(const U2TStringHashTable&);
	U2TStringHashTable& operator= (const U2TStringHashTable&);

};

#include "U2TStringHashTable.inl"

#endif


