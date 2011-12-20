/**************************************************************************************************
module	:	U2FixedArray
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_FIXEDARRAY_H
#define U2_FIXEDARRAY_H

#include <U2Lib/src/U2ContainerAlloc.h>
#include <U2Lib/src/Memory/U2MemObj.h>

template<class T, class TAlloc>
class  U2TFixedArray : public U2MemObj
{
public:
	U2TFixedArray();
	U2TFixedArray(uint32 size);
	U2TFixedArray(const U2TFixedArray<T, TAlloc>& rhs);
	virtual ~U2TFixedArray();

	void SetSize(uint32 s);
	uint32 Size() const;

	void Clear(T elem);
	void operator=(const U2TFixedArray<T, TAlloc>& rhs);
	T& operator[] (int i) const;
	uint32 Find(const T&e) const;

private:
	void Delete();
	void Allocate(int s);

	void Copy(const U2TFixedArray<T, TAlloc>& src);

	uint32 m_uSize;
	T* m_pBase;
};

template<class T>
class U2FixedObjArray : public U2TFixedArray<T, U2NewAlloc<T> >
{
public:
	U2FixedObjArray();
	U2FixedObjArray(uint32 size);
	U2FixedObjArray(const U2FixedObjArray<T>& rhs);
	
};


template<class T> 
class U2FixedPrimitiveArray : public U2TFixedArray<T, U2MallocAlloc<T> >
{
public:
	U2FixedPrimitiveArray();
	U2FixedPrimitiveArray(uint32 size);
	U2FixedPrimitiveArray(const U2FixedPrimitiveArray<T>& rhs);	
};



#include "U2TFixedArray.inl"




#endif 
