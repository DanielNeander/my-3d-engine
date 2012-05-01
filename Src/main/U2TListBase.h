/************************************************************************
module	:	U2TListBase
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once 
#ifndef	U2_TLISTBASE_H
#define	U2_TLISTBASE_H

#include <U2Lib/src/Memory/U2MemObj.h>


template<class T>
class U2ListNode : public U2MemObj
{
public:
	T m_elem;
	U2ListNode* m_pPrev;
	U2ListNode* m_pNext;
};


template<class T, class TAlloc>
class  U2TListBase : public U2MemObj
{

public:	
	virtual ~U2TListBase();

	uint32 Size() const;
	bool IsEmpty() const;

	void RemoveAll();

	void InsertToHead(const T& element);
	void InsertToTail(const T& element);
	
	U2ListNode<T>* GetHeadNode() const;
	U2ListNode<T>* GetTailNode() const;

	U2ListNode<T>* GetNextNode(U2ListNode<T>* pCurrNode) const;
	U2ListNode<T>* GetPrevNode(U2ListNode<T>* pCurrNode) const;

	U2ListNode<T>* InsertBefore(U2ListNode<T>* pCurrNode, const T& elem);
	U2ListNode<T>* InsertAfter(U2ListNode<T>* pCurrNode, const T& elem);

	const T& GetHeadElem() const;
	const T& GetTailElem() const;
	const T& GetElem(U2ListNode<T>* pNode) const;

	const T& GetNextElem(U2ListNode<T>*& pCurrNode) const;
	const T& GetPrevElem(U2ListNode<T>*& pCurrNode) const;

	T RemoveHead();
	T RemoveTail();
	T Remove(const T& elem);
	T RemoveNode(U2ListNode<T>*& );

	U2ListNode<T>* Find(const T& elem, U2ListNode<T>* pStart = 0) const;

protected:
	U2TListBase();
	
	void		AddNodeToHead(U2ListNode<T>* pNode);
	void		AddNodeToTail(U2ListNode<T>* pNode);
	U2ListNode<T>* AddNodeBefore(U2ListNode<T>* pCurrNode, U2ListNode<T>* pNewNode);
	U2ListNode<T>* AddNodeAfter(U2ListNode<T>* pCurrNode, U2ListNode<T>* pNewNode);
	
	uint32 		m_uNumListItems;

	U2ListNode<T>* m_pHead;
	U2ListNode<T>* m_pTail;		

	TAlloc		m_allocator;

	virtual U2ListNode<T>* NewNode();
	virtual void DeleteNode(U2ListNode<T>* pItem);

private:
	
	U2TListBase(const U2TListBase&);
	U2TListBase& operator=(const U2TListBase&);	
};

#include "U2TListBase.inl"

#endif