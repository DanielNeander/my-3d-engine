#include <Src/U2LibPch.h>

#include "U2MemoryDefine.h"


#include "U2FixedAllocator.h"
#include "U2Allocator.h"
#include <algorithm>


void	U2FixedAllocator::Chunk::Init(U2Allocator* pAllocator, 
									  size_t blockSize, 
									  unsigned char blocks)
{
	U2ASSERT(pAllocator);
	U2ASSERT(blockSize > 0);
	U2ASSERT(blocks > 0);
	U2ASSERT((blockSize * blocks) / blockSize == blocks);

	m_pAllocator = pAllocator;

	size_t sizeInBytes = sizeof(unsigned char) * blockSize * blocks;
	size_t alignment = U2_MEM_ALIGNMENT;

	//m_pucData = reinterpret_cast<unsigned char*>(m_pAllocator->Allocate(
	//	sizeInBytes,
	//	alignment, 
	//	MEMFLAG_NONE,
	//	MEM_ALIGNEDMALLOC,
	//	__FILE__, __LINE__, __FUNCTION__));
	m_pucData = (unsigned char*)U2ExternAlignedMalloc(sizeInBytes, 
		alignment);

#ifdef _DEBUG
	if(m_pucData == 0)
		LOG_INFO << _T("Mem is full");
#endif
	U2ASSERT(m_pucData);

	Reset(blockSize, blocks);	
}


// Clear an already allocated chunk
void	U2FixedAllocator::Chunk::Reset(size_t blockSize, unsigned char blocks)
{
	U2ASSERT(blockSize > 0);
	U2ASSERT(blocks > 0);
	U2ASSERT((blockSize * blocks) / blockSize == blocks);

	m_ucFirstAvailableBlock = 0;
	m_ucBlocksAvailable = blocks;

	unsigned char i = 0;
	for(unsigned char *p = m_pucData; i != blocks; p += blockSize)
		*p = ++i;
}


void	U2FixedAllocator::Chunk::Release()
{
	U2ASSERT(m_pAllocator);

	//m_pAllocator->Deallocate(m_pucData, MEM_ALIGNEDFREE, 0);
	U2ExternAlignedFree(m_pucData);
}


void*	U2FixedAllocator::Chunk::Allocate(size_t blockSize)
{
	if(!m_ucBlocksAvailable)
		return 0;

	U2ASSERT((m_ucFirstAvailableBlock * blockSize) / blockSize == 
		m_ucFirstAvailableBlock);

	unsigned char* pucResult =
		m_pucData + (m_ucFirstAvailableBlock * blockSize);
	m_ucFirstAvailableBlock = *pucResult;
	--m_ucBlocksAvailable;


	return pucResult;
}

void	U2FixedAllocator::Chunk::Deallocate(void *p, size_t blockSize)
{
	U2ASSERT(p >= m_pucData);

	unsigned char* toRelease = static_cast<unsigned char*>(p);	
	// 블록 크기에 따른 바이트 정렬 검사
	U2ASSERT((toRelease - m_pucData) % blockSize == 0);
	*toRelease = m_ucFirstAvailableBlock;
	m_ucFirstAvailableBlock = static_cast<unsigned char>((toRelease - m_pucData) / blockSize);
	U2ASSERT(m_ucFirstAvailableBlock == (toRelease - m_pucData) / blockSize);

	++m_ucBlocksAvailable;
}


bool	U2FixedAllocator::Chunk::HasAvailable(unsigned char numBlocks) const
{
	return m_ucBlocksAvailable == numBlocks;
}


bool U2FixedAllocator::Chunk::HasBlock(unsigned char *p, size_t chunkLength) const
{
	return (m_pucData <= p) && (p < m_pucData + chunkLength);
}

bool	U2FixedAllocator::Chunk::IsFilled(void) const 
{
	return (0 == m_ucBlocksAvailable);
}


unsigned char U2FixedAllocator::MinObjectsPerChunk_ = 8;
unsigned char U2FixedAllocator::MaxObjectsPerChunk_ = UCHAR_MAX;


U2FixedAllocator::U2FixedAllocator()
		:m_pAllocator(0)
		,m_blockSize(0)
		,m_pChunks(0)
		,m_numChunks(0)
		,m_maxNumChunks(0)
		,m_pAllocChunk(0)
		,m_pDeallocChunk(0)
		,m_pEmptyChunk(0)
{
}


U2FixedAllocator::~U2FixedAllocator()
{
	U2ASSERT(m_pAllocator);

	for(size_t i = 0; i < m_numChunks; ++i)
	{
		m_pChunks[i].Release();
	}

//	m_pAllocator->Deallocate(m_pChunks, MEM_ALIGNEDFREE, 0);
}


void	U2FixedAllocator::Init(U2Allocator *pAllocator, size_t blockSize)
{
	U2ASSERT(pAllocator);

	m_pAllocator = pAllocator;
	m_numChunks = 0;
	m_maxNumChunks = 0;
	m_pChunks = NULL;
	m_pAllocChunk = NULL;
	m_blockSize = blockSize;
	size_t numBlocks = DEFAULT_CHUNK_SIZE / blockSize;

	if( numBlocks > MaxObjectsPerChunk_)
		numBlocks = MaxObjectsPerChunk_;
	else if(numBlocks == 0)
		numBlocks =  MinObjectsPerChunk_ * blockSize;

	m_ucNumBlocks = static_cast<unsigned char>(numBlocks);
	U2ASSERT(m_ucNumBlocks == numBlocks);
}

void	U2FixedAllocator::PushBack(Chunk& chunk)
{
	size_t count = m_numChunks;
	Reserve(m_numChunks + 1);
	m_pChunks[count] = chunk;
	m_numChunks++;

}

void	U2FixedAllocator::PopBack()
{
	--m_numChunks;

}


void	U2FixedAllocator::Reserve(size_t newSize)
{
	U2ASSERT(m_pAllocator);

	if(newSize > m_maxNumChunks)
	{
		size_t newSizeInBytes = newSize * sizeof(Chunk);
		size_t currSizeInBytes = m_maxNumChunks * sizeof(Chunk);
		size_t alignment = U2_MEM_ALIGNMENT;

		m_pChunks = reinterpret_cast<Chunk*>(m_pAllocator->Reallocate(
			m_pChunks,
			newSizeInBytes,
			alignment,
			MEMFLAG_NONE, 
			MEM_ALIGNEDREALLOC,
			currSizeInBytes,
			__FILE__, __LINE__, __FUNCTION__));

		U2ASSERT(m_pChunks);

		m_maxNumChunks = newSize;
	}
}



void*	U2FixedAllocator::Allocate()
{
	U2ASSERT((NULL == m_pEmptyChunk) || 
		(m_pEmptyChunk->HasAvailable(m_ucNumBlocks)));

	if(m_pAllocChunk && m_pAllocChunk->IsFilled())
		m_pAllocChunk = NULL;

	if(NULL != m_pEmptyChunk)
	{
		m_pAllocChunk = m_pEmptyChunk;
		m_pEmptyChunk = NULL;
	}

	if(m_pAllocChunk == 0)
	{
		// 빈 Chunk 있는지 순차 검색
		for(size_t i = 0; i < m_numChunks; ++i)
		{
			if(!m_pChunks[i].IsFilled())
			{
				m_pAllocChunk = & m_pChunks[i];
				break;
			}
		}

		// 없으면 새로 추가
		if(NULL == m_pAllocChunk)
		{
			Reserve(m_numChunks + 1);
			Chunk newChunk;
			newChunk.Init(m_pAllocator, m_blockSize, m_ucNumBlocks);
			PushBack(newChunk);
			m_pAllocChunk = &m_pChunks[m_numChunks - 1];
			m_pDeallocChunk = &m_pChunks[0];	

		}

	}
	U2ASSERT(m_pAllocChunk != 0);
	U2ASSERT(!m_pAllocChunk->IsFilled());

	U2ASSERT( (NULL == m_pEmptyChunk) || 
		(m_pEmptyChunk->HasAvailable(m_ucNumBlocks)));

	void* pvMem = m_pAllocChunk->Allocate(m_blockSize);	


	return pvMem;
}



void	U2FixedAllocator::Deallocate(void *p)
{
	U2ASSERT(m_numChunks != 0);
	U2ASSERT(&m_pChunks[0] <= m_pDeallocChunk);
	U2ASSERT(&m_pChunks[m_numChunks - 1] >= m_pDeallocChunk);

	m_pDeallocChunk = VicnityFind(p);

	U2ASSERT(m_pDeallocChunk);

	DoDeallocate(p);
}

U2FixedAllocator::Chunk*	U2FixedAllocator::VicnityFind(void* p)
{
	U2ASSERT(m_numChunks != 0);
	U2ASSERT(m_pDeallocChunk);

	const size_t chunkLen = m_ucNumBlocks * m_blockSize;

	Chunk* lo = m_pDeallocChunk;
	Chunk* hi = m_pDeallocChunk + 1;
	Chunk* loBound = &m_pChunks[0];
	Chunk* hiBound = &m_pChunks[m_numChunks - 1] + 1;

	if(hi == hiBound)
		hi = NULL;

	for(;;)
	{
		if(lo)
		{
			if(lo->HasBlock((unsigned char*)p, chunkLen))
				return lo;
			if( lo == loBound )
			{
				lo = NULL;
				if( NULL == hi )
					break;
			}
			else 
				--lo;

		}

		if(hi)
		{
			if(hi->HasBlock((unsigned char*)p, chunkLen))
				return hi;
			if(++hi == hiBound)
			{
				hi = NULL;
				if(NULL == lo)
					break;
			}
		}
	}

	U2ASSERT(!"Could not find pointer p in NiFixedAllocator::VicinityFind()");
	return NULL;
}

void	U2FixedAllocator::DoDeallocate(void *p)
{
	// 조소 p에 있는 블록이 Chunk 안에 있는지 체크	
	U2ASSERT(m_pDeallocChunk->HasBlock(static_cast<unsigned char *>(p), m_ucNumBlocks * m_blockSize));

	U2ASSERT((NULL == m_pEmptyChunk) || 
		(m_pEmptyChunk->HasAvailable(m_ucNumBlocks)));

	m_pDeallocChunk->Deallocate(p, m_blockSize);

	if(m_pDeallocChunk->HasAvailable(m_ucNumBlocks))
	{
		U2ASSERT(m_pEmptyChunk != m_pDeallocChunk);

		if(NULL != m_pEmptyChunk)
		{
			Chunk* pLastChunk = &m_pChunks[m_numChunks - 1];
			if(pLastChunk == m_pDeallocChunk)
				m_pDeallocChunk = m_pEmptyChunk;
			else if(pLastChunk != m_pEmptyChunk) 
				std::swap(*m_pEmptyChunk, *pLastChunk);

			U2ASSERT(pLastChunk->HasAvailable(m_ucNumBlocks));
			pLastChunk->Release();
			PopBack();
			m_pAllocChunk = m_pDeallocChunk;
		}
		m_pEmptyChunk = m_pDeallocChunk;
	}

	U2ASSERT((NULL == m_pEmptyChunk) || (m_pEmptyChunk->HasAvailable(m_ucNumBlocks)));

}