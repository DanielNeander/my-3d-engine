/************************************************************************
module	: U2ContainerAllocator
Author	: Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_CONTAINERALLOC_H
#define U2_CONTAINERALLOC_H


#include <U2Lib/Src/Memory/U2MemoryMgr.h>
#include <U2Lib/src/Memory/U2MemObj.h>

template<class T> 
class U2MallocAlloc
{
public:
	static T* Allocate(unsigned int numElems) 
	{
		return U2_ALLOC(T, numElems);
	}
	static void Deallocate(T* pArray)
	{
		U2_FREE (pArray); 
		pArray = NULL;
	}
};

template <class T> 
class U2NewAlloc
{
public:
	static T* Allocate(uint32 numElems)
	{
		return U2_NEW T[numElems];
	}

	static void Deallocate(T* pArray)
	{
		U2_DELETE [] pArray;
		pArray = NULL;
	}
};

#endif

