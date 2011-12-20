/************************************************************************
module	: U2ListBase
Author	: Yun sangyong
Desc	: 																	  
************************************************************************/
#pragma once 
#ifndef U2_LISTBASE_H
#define U2_LISTBASE_H


#include "U2ContainerAlloc.h"


template<class T>
class U2ListNode : public U2MemObj
{
public:
	U2ListNode* m_pNext;
	U2ListNode* m_pPrev;
	T m_data;
};

template<class T, class TAlloc>
class U2ListBase : public U2MemObj
{
public:
	uint32	Size() const;
	bool	Empty() const;
	
	void	RemoveAll();
	
	U2ListNode<T>* m_pHead;
	U2ListNode<T>* m_pTail;

protected:
	U2ListBase();
	virtual ~U2ListBase();

	virtual	U2ListNode<T>* CreateNode() = 0;
	virtual void RemoveNode(U2ListNode<T>* pNode) = 0;
	


private:

	U2ListBase(const U2ListBase&);
	U2ListBase& operator=(const U2ListBase&);
	
};



#endif