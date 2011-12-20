template<class T, class TAlloc>
inline 
U2TListBase<T, TAlloc>::U2TListBase()
{
	m_pHead = NULL;
	m_pTail = NULL;
	m_uNumListItems = 0;		
}

template<class T, class TAlloc>
inline 
U2TListBase<T, TAlloc>::~U2TListBase()
{
	//RemoveAll();
}

template<class T, class TAlloc>
inline 
uint32 U2TListBase<T, TAlloc>::Size() const 
{
	return m_uNumListItems;
}


template<class T, class TAlloc>
inline 
bool U2TListBase<T, TAlloc>::IsEmpty() const
{
	return (m_uNumListItems == 0);
}

template<class T, class TAlloc>
inline 
void U2TListBase<T, TAlloc>::RemoveAll()
{
	U2ListNode<T>* pCurrNode = m_pHead;
	while(pCurrNode)
	{
		U2ListNode<T>* pDelNode = pCurrNode;
		pCurrNode = pCurrNode->m_pNext;
		DeleteNode(pDelNode);
	}
	
	m_uNumListItems = 0;
	m_pHead = NULL;
	m_pTail = NULL;
}


template<class T, class TAlloc>
inline 
U2ListNode<T>* U2TListBase<T, TAlloc>::GetHeadNode() const
{
	return m_pHead;
}

template<class T, class TAlloc>
inline 
U2ListNode<T>* U2TListBase<T, TAlloc>::GetTailNode() const
{
	return m_pTail;
}


template<class T, class TAlloc>
inline 
U2ListNode<T>* U2TListBase<T, TAlloc>::GetNextNode(U2ListNode<T>* pCurrNode) const
{
	return pCurrNode ? pCurrNode->m_pNext : 0;
}


template<class T, class TAlloc>
inline 
U2ListNode<T>* U2TListBase<T, TAlloc>::GetPrevNode(U2ListNode<T>* pCurrNode) const
{
	return pCurrNode ? pCurrNode->m_pPrev : 0;
}

template<class T, class TAlloc>
inline 
U2ListNode<T>*  U2TListBase<T, TAlloc>::NewNode()
{
	return (U2ListNode<T>*)m_allocator.Allocate();
}

template<class T, class TAlloc>
inline 
void  U2TListBase<T, TAlloc>::DeleteNode(U2ListNode<T>* pItem)
{
	pItem->m_elem = 0;
	m_allocator.Deallocate(pItem);
}


template<class T, class TAlloc>
inline 
const T& U2TListBase<T, TAlloc>::GetHeadElem() const
{
	return m_pHead->m_elem;
}


template<class T, class TAlloc>
inline 
const T& U2TListBase<T, TAlloc>::GetTailElem() const
{
	return m_pTail->m_elem;
}


template<class T, class TAlloc>
inline 
const T& U2TListBase<T, TAlloc>::GetElem(U2ListNode<T>* pNode) const
{
	U2ASSERT(pNode);
	return pNode->m_elem;
}


template<class T, class TAlloc>
inline 
const T& U2TListBase<T, TAlloc>::GetNextElem(U2ListNode<T>*& pCurrNode) const
{
	U2ASSERT(pCurrNode != 0);

	const T& elem = pCurrNode->m_elem;
	pCurrNode = pCurrNode->m_pNext;
	return elem;
}


template<class T, class TAlloc>
inline 
const T& U2TListBase<T, TAlloc>::GetPrevElem(U2ListNode<T>*& pCurrNode) const
{
	U2ASSERT(pCurrNode != 0);

	const T& elem = pCurrNode->m_elem;
	pCurrNode = pCurrNode->m_pPrev;
	return elem;

}


template<class T, class TAlloc>
inline 
void U2TListBase<T, TAlloc>::AddNodeToHead(U2ListNode<T>* pNode)
{
	pNode->m_pPrev = 0;
	pNode->m_pNext = m_pHead;

	if(m_pHead)
		m_pHead->m_pPrev = pNode;
	else 
		m_pTail = pNode;

	m_pHead = pNode;
	m_uNumListItems++;
}


template<class T, class TAlloc>
inline 
void U2TListBase<T, TAlloc>::AddNodeToTail(U2ListNode<T>* pNode)
{
	pNode->m_pNext = 0;
	pNode->m_pPrev = m_pTail;

	if(m_pTail)
		m_pTail->m_pNext = pNode;
	else 
		m_pHead = pNode;

	m_pTail = pNode;
	m_uNumListItems++;		
}


template<class T, class TAlloc>
inline 
U2ListNode<T>* U2TListBase<T, TAlloc>::AddNodeBefore(U2ListNode<T>* pCurrNode, 
													 U2ListNode<T>* pNewNode)
{
	pNewNode->m_pNext = pCurrNode;
	pNewNode->m_pPrev = pCurrNode->m_pPrev;

	if(pCurrNode->m_pPrev)
		pCurrNode->m_pPrev->m_pNext = pNewNode;
	else 
		m_pHead = pNewNode;

	pCurrNode->m_pPrev = pNewNode;

	m_uNumListItems++;
	return pNewNode;	
}


template<class T, class TAlloc>
inline 
U2ListNode<T>* U2TListBase<T, TAlloc>::AddNodeAfter(U2ListNode<T>* pCurrNode, 
													U2ListNode<T>* pNewNode)
{
	pNewNode->m_pPrev = pCurrNode;
	pNewNode->m_pNext = pCurrNode->m_pNext;

	if(pCurrNode->m_pNext)
		pCurrNode->m_pNext->m_pPrev = pNewNode;
	else 
		m_pTail = pNewNode;

	pCurrNode->m_pNext = pNewNode;

	m_uNumListItems++;
	return pNewNode;
}

template<class T, class TAlloc>
inline 
void U2TListBase<T, TAlloc>::InsertToHead(const T& element)
{
	U2ListNode<T>* pNewNode = NewNode();
	pNewNode->m_elem = element;

	AddNodeToHead(pNewNode);
}


template<class T, class TAlloc>
inline 
void U2TListBase<T, TAlloc>::InsertToTail(const T& element)
{

	U2ListNode<T>* pNewNode = NewNode();
	pNewNode->m_elem = element;

	AddNodeToTail(pNewNode);
}


template<class T, class TAlloc>
inline 
U2ListNode<T>* U2TListBase<T, TAlloc>::InsertBefore(U2ListNode<T>* pCurrNode, 
												 const T& elem)
{
	U2ASSERT(pCurrNode);

	U2ListNode<T>* pNewNode = NewNode();
	pNewNode->m_elem = elem;
	return AddNodeBefore(pCurrNode, pNewNode);
}


template<class T, class TAlloc>
inline 
U2ListNode<T>* U2TListBase<T, TAlloc>::InsertAfter(U2ListNode<T>* pCurrNode, 
												const T& elem)
{
	U2ASSERT(pCurrNode);

	U2ListNode<T>* pNewNode = NewNode();
	pNewNode->m_elem = elem;
	return AddNodeAfter(pCurrNode, pNewNode);
}


template<class T, class TAlloc>
inline 
T U2TListBase<T, TAlloc>::RemoveHead()
{
	U2ASSERT(m_pHead);

	U2ListNode<T>* pNode = m_pHead;

	m_pHead = m_pHead->m_pNext;
	
	if(m_pHead)
		m_pHead->m_pPrev = 0;
	else 
		m_pTail = 0;

	T elem = pNode->m_elem;

	DeleteNode(pNode);

	m_uNumListItems--;

	return elem;
}


template<class T, class TAlloc>
inline 
T U2TListBase<T, TAlloc>::RemoveTail()
{
	U2ASSERT(m_pTail);

	U2ListNode<T>* pNode = m_pTail;
	m_pTail = m_pTail->m_pPrev;

	if(m_pTail)
		m_pTail->m_pNext = 0;
	else 
		m_pHead = 0;

	T elem = pNode->m_elem;
	DeleteNode(pNode);

	m_uNumListItems--;

	return elem;
}


template<class T, class TAlloc>
inline 
T U2TListBase<T, TAlloc>::Remove(const T& elem)
{
	U2ListNode<T>* pNode = Find(elem);

	return pNode ? RemoveNode(pNode) : elem;	
}


template<class T, class TAlloc>
inline 
T U2TListBase<T, TAlloc>::RemoveNode(U2ListNode<T>*& pNode)
{
	U2ASSERT(pNode);

	if(pNode == m_pHead)
	{
		pNode = pNode->m_pNext;
		return RemoveHead();
	}
	if(pNode == m_pTail)
	{
		pNode = NULL;
		return RemoveTail();
	}
	
	U2ListNode<T>* pPrev = pNode->m_pPrev;
	U2ListNode<T>* pNext = pNode->m_pNext;

	pNode = pNext;

	if(pPrev)
		pPrev->m_pNext = pNext;
	if(pNext)
		pNext->m_pPrev = pPrev;

	T elem = pNode->m_elem;

	DeleteNode(pNode);

	m_uNumListItems--;

	return elem;
}


template<class T, class TAlloc>
inline 
U2ListNode<T>* U2TListBase<T, TAlloc>::Find(const T& elem, U2ListNode<T>* pStart) const
{
	if(!pStart)
		pStart = GetHeadNode();

	while(pStart)
	{
		U2ListNode<T>* pNode = pStart;
		if(elem == GetNextElem(pStart))
			return pNode;
	}

	return NULL;
}