/************************************************************************
module	:	U2MemoryPool
Author	:	Yun	sangyong
Desc	:	http://www.codeguru.com/cpp/cpp/cpp_mfc/stl/article.php/c4079
			참조. 
************************************************************************/
#pragma once
#ifndef U2_MEMORYPOOL_H
#define U2_MEMORYPOOL_H


#include <U2Lib/src/Memory/U2MemoryMgr.h>

#include <exception>
#include <list>
#include <algorithm>
#include <iostream>  //for dump()


class U2LIB U2MemoryPool 
{	
public:	
	class MemExcep:public std::exception
	{
	public:
		MemExcep(const TCHAR *what):what_(what){}
		const TCHAR* what(){return what_;}
	private:
		const TCHAR *what_;
	};

	enum {
		INIT_SIZE = 0xffff,
		MIN_SIZE = 0xf
	};

	U2MemoryPool(size_t size = INIT_SIZE) : m_blockSize(size), m_pBaseBlock(0)
	{
		if(size - sizeof(Block) < MIN_SIZE) throw MemExcep(_T("Init Pool Size is too small"));
		m_pool.push_back(U2_ALLOC(TCHAR, m_blockSize));
		m_pBaseBlock = reinterpret_cast<Block*>(*(m_pool.begin()));
		m_pBaseBlock->pPrev = 0;
		m_pBaseBlock->pNext = 0;
		m_pBaseBlock->bFree = 1;
		m_pBaseBlock->size = m_blockSize - sizeof(Block);		
	}

	~U2MemoryPool()
	{
		std::for_each(m_pool.begin(), m_pool.end(), Deleter());
	}

	void*	allocate(size_t size)
	{
		if(size > size - sizeof(Block)) throw std::bad_alloc();
		Block *pTempBlock = m_pBaseBlock;
		while(1) {
			while(!pTempBlock->bFree) {
				if(!pTempBlock->pNext) CreateBlock(pTempBlock);
				pTempBlock = pTempBlock->pNext;
			}
			if(pTempBlock->size < size) continue;
			break;
		}

		if(pTempBlock->size - size < 2 * sizeof(Block)) {
			pTempBlock->bFree = 0;
			return reinterpret_cast<TCHAR*>(pTempBlock) + sizeof(Block);		
		}
		else 
		{
			Block* pNewblock = (reinterpret_cast<Block*>(reinterpret_cast<TCHAR*>(pTempBlock)
				+ size + sizeof(Block)));
			if(pNewblock->pNext) pTempBlock->pPrev = pNewblock;
			pNewblock->pNext = pTempBlock->pNext;
			pTempBlock->pNext = pNewblock;
			pNewblock->pPrev = pNewblock;
			pNewblock->bFree = 0;
			pNewblock->size = pTempBlock->size - size - sizeof(Block);
			pTempBlock->size = size;
			pNewblock->bFree = 1;
			return reinterpret_cast<TCHAR*>(pTempBlock) + sizeof(Block);
		}
	}

	void deallocate(void *p, size_t = 0)
	{
		if(!p) return ;
		Block *pBlock = reinterpret_cast<Block*>((TCHAR*)p - sizeof(Block));
		
		if(pBlock->pPrev && pBlock->pNext) {
			if(pBlock->pPrev->bFree && pBlock->pNext->bFree) {
				pBlock->pPrev->size += pBlock->size + pBlock->pNext->size
					+ 2 * sizeof(Block);
				pBlock->pPrev->pNext = pBlock->pNext->pNext;
				if(pBlock->pNext->pNext) 
					pBlock->pNext->pNext->pPrev = pBlock->pPrev;
				return;
			}		
		}
		
		if(pBlock->pPrev) 
		{
			if(pBlock->pPrev->bFree) {
				pBlock->pPrev->size += pBlock->size + sizeof(Block);
				pBlock->pNext = pBlock->pNext;
				if(pBlock->pNext) pBlock->pNext->pPrev = pBlock->pPrev;
				pBlock->bFree = 1;
				return;
			}
		}

		if(pBlock->pNext)
		{
			if(pBlock->pNext->bFree) {
				pBlock->size += pBlock->pNext->size + sizeof(Block);
				pBlock->pNext = pBlock->pNext->pNext;
				if(pBlock->pNext) pBlock->pNext->pPrev = pBlock;
				pBlock->bFree = 1;
				return;
			}
		}

		pBlock->bFree = 1;
	}
	

private:
	size_t	m_blockSize;
	std::list<TCHAR*> m_pool;
		
	struct Block {
		Block*	pPrev;
		Block*	pNext;
		size_t	size;
		int		bFree;

		Block(Block *_pPrev, Block *_pNext, size_t _Size, int _bFree)
			:pPrev(_pPrev), pNext(_pNext), size(_Size), bFree(_bFree)
		{
		}
		~Block() {}		
	};
	Block*	m_pBaseBlock;

	struct Deleter
	{
		void operator()(TCHAR *p) { U2_FREE(p); p = NULL; }
	};

	void	CreateBlock(Block* pPrevBlock)
	{
		Block *pNewBlock(0);
		TCHAR* pMem = U2_ALLOC(TCHAR, m_blockSize);
		m_pool.push_back(pMem);
		pNewBlock = reinterpret_cast<Block*>(pMem);
		
		pNewBlock->pPrev = pPrevBlock;
		pNewBlock->pNext = 0;
		pNewBlock->bFree = 1;
		pNewBlock->size = m_blockSize - sizeof(Block);
		pPrevBlock->pNext = pNewBlock;		
	}



	// Copy 방지
	U2MemoryPool(const U2MemoryPool&);
	U2MemoryPool&	operator=(const U2MemoryPool&);
};


#endif