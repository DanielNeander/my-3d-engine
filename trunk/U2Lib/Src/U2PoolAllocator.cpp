#include <src/U2LibPCH.h>
#include "U2PoolAllocator.h"




// static member
template<class T>U2MemoryPool U2PoolAllocator<T>::m_mem;

template<class  T>
U2PoolAllocator<T>::U2PoolAllocator() 
{

}

template<class  T>
typename U2PoolAllocator<T>::pointer U2PoolAllocator<T>::address(reference x) const 
{ return &x; }

template<class  T>
typename U2PoolAllocator<T>::const_pointer U2PoolAllocator<T>::address(const_reference x) const 
{ return &x; }


template<class  T>
typename U2PoolAllocator<T>::pointer U2PoolAllocator<T>::allocate(size_type size, 
									 U2PoolAllocator<void>::const_pointer hint)
{
	return static_cast<pointer>(m_mem.allocate(size * sizeof(T)));
}



template<class  T>
void U2PoolAllocator<T>::deallocate(pointer p, size_type n)
{
	m_mem.deallocate(p, n);
}


template<class  T>
void U2PoolAllocator<T>::deallocate(void *p, size_type n)
{
	m_mem.deallocate(p, n);
}


template<class  T>
typename U2PoolAllocator<T>::size_type U2PoolAllocator<T>::max_size() const throw() 
{ return size_t(-1) / sizeof(value_type); }



template<class  T>
void U2PoolAllocator<T>::construct(pointer P, const T& val)
{
	//new(static_cast<void*>(p)) T(val);
	U2_NEW T(val);
}


template<class  T>
void U2PoolAllocator<T>::construct(pointer p) 
{
	//new(static_cast<void*>(p)) T();		
	U2_NEW  T();		
}


template<class  T>
void U2PoolAllocator<T>::destroy(pointer p) 
{ PoolAlloc::destruct(p);}	









