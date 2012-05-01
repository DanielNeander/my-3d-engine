/************************************************************************
module	:	U2PoolAlloc
Author	:	Yun sangyong
Desc	:	GameBryo 2.3 버전.
************************************************************************/
#pragma once 
#ifndef U2_POOLALLOC_H
#define U2_POOLALLOC_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2Lib/src/Memory/U2MemObj.h>

// NiTObjectList is designed to hold compound objects efficiently. It
// is responsible for allocating and freeing the elements in the list. The
// interface does not support copy construction assignment of T directly.
// Instead, element access is provided via a pointer to the element.
// T is block allocated from a shared memory pool. The template class 
// requires that a default constructor for T exists, even if it does nothing.
//
// The pool uses some non-local static variables that must be declared using
// the NiAllocatorDeclareStatics macro. For example:
//   NiAllocatorDeclareStatics(T, size);
// where T is the template type T and size is the block size for the memory
// pool. To free this memory pool, use:
//   NiAllocatorShutdown(T);
// NiAllocatorShutdown assumes that any NiTObjectList<T> instances have been
// destroyed. Accessing a pre-existing NiTObjectList<T> after calling
// NiAllocatorShutdown(T) will cause a memory access violation. After calling
// NiAllocatorShutdown(T), new NiTObjectList<T> instances may be created, but
// of course they should be cleaned up with another call to
// NiAllocatorShutdown(T).
// 
// Remove and FindPos search for an element based on pointer equality. 
//
// Example of iteration from head to tail:
//
//   NiTObjectList<T> kList;
//   NiTListIterator kPos = kList.GetHeadPos();
//   while (kPos)
//   {
//       T* element = kList.GetNext(kPos);
//       <process element here>;
//   }

#define U2PoolShutdown(T)	U2PoolAlloc<T>::Terminate() 


#define U2AllocDeclareStatics(T, BLOCKSIZE) \
	template<> U2PoolAlloc<T>::Block* U2PoolAlloc<T>::ms_pBlockHeader = NULL;	\
	template<> U2PoolAlloc<T>::Block* U2PoolAlloc<T>::ms_pMemPool = NULL;	\
	template<> unsigned int  U2PoolAlloc<T>::ms_uNumBlock = BLOCKSIZE;	\


template<class T> 
class U2PoolAlloc  
{
public:

	~U2PoolAlloc();

	// 
	static void Terminate();		// U2PoolAlloc 사용시 반드시 Destroy 호출..
	static void CreatePool();
	

	
	class Block : public U2MemObj
	{
	public:
		Block* m_pNext;
		T m_elem;
		void* m_pvData;
	};
	
	static Block* ms_pBlockHeader;
	static Block* ms_pMemPool;
	static uint32 ms_uNumBlock;

	void *Allocate();
	void Deallocate(void* p);
};


#ifdef U2_3D_IMPORTS
#define EXPIMP_TEMPLATE extern
EXPIMP_TEMPLATE template class U2_3D U2PoolAlloc<size_t>;
#else 
#define EXPIMP_TEMPLATE 
#endif


#include "U2PoolAlloc.inl"



#endif