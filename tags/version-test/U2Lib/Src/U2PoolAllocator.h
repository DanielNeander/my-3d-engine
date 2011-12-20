/************************************************************************
module	:	U2PoolAllocator
Author	:	Yun sangyong
Desc	:   http://www.codeguru.com/cpp/cpp/cpp_mfc/stl/article.php/c4079
			ÂüÁ¶. 
************************************************************************/
#pragma once 
#ifndef U2_POOLALLOCATOR_H
#define U2_POOLALLOCATOR_H

#include "U2MemoryPool.h"

template<class T>class U2PoolAllocator;	

template<>
class U2LIB U2PoolAllocator<void>
{
public:
	typedef	void*		pointer;
	typedef const void* const_pointer;
	typedef void		value_type;
	template<class U>
	struct rebind { typedef U2PoolAllocator<U> other; };
};

namespace PoolAlloc 
{
	inline void destruct(char *){}
	inline void destruct(wchar_t*){}
	template <typename T> 
	inline void destruct(T *t){t->~T();}
} // namespace

template<class T>
class U2LIB U2PoolAllocator
{
public:
	typedef size_t		size_type;
	typedef ptrdiff_t	difference_type;
	typedef T*			pointer;
	typedef const T*	const_pointer;
	typedef T&			reference;
	typedef const T&	const_reference;
	typedef T			value_type;

	template<class U>
	struct rebind { typedef U2PoolAllocator<U> other; };
	
	U2PoolAllocator();
	pointer address(reference x) const;
	const_pointer address(const_reference x) const;
	pointer allocate(size_type size, U2PoolAllocator<void>::const_pointer hint = 0);
	

	template<class U> U2PoolAllocator(const U2PoolAllocator<U>&) {}
	void deallocate(pointer p, size_type n);	
	void deallocate(void *p, size_type n);
	

	size_type max_size() const throw();
	void	construct(pointer P, const T& val);	

	void construct(pointer p);

	void destroy(pointer p);

private:
	static U2MemoryPool m_mem;
};



template<class T, class U>
inline bool operator==(const U2PoolAllocator<T>&, const U2PoolAllocator<U>) 
{ return true; }

template<class T, class U>
inline bool operator!=(const U2PoolAllocator<T>&, const U2PoolAllocator<T>)
{	return false; }








#endif