/************************************************************************
module  : U2FixedAllocator
Author  : yun sangyong
Desc	: Gamebryo 2.6 메모리 매니저 
		  원본은 Modern C++ Design의 Loki안에 SmllObj Allocator 
************************************************************************/

#ifndef U2_FIXEDALLOCATOR_H
#define U2_FIXEDALLOCATOR_H

#ifndef DEFAULT_CHUNK_SIZE
#define DEFAULT_CHUNK_SIZE 25600
#endif

#include <U2Lib/Src/U2LibType.h>

class U2Allocator;

class U2LIB U2FixedAllocator
{
private:
	struct U2LIB Chunk
	{
	public:
		void Init(U2Allocator* pAllocator, size_t blockSize,
			unsigned char ucBlocks);
		void*	Allocate(size_t blockSize);
		void	Deallocate(void* p, size_t blockSize);
		void	Reset(size_t blockSize, unsigned char ucBlocks);
		void	Release();
		bool	HasAvailable(unsigned char numBlocks) const;
		bool	HasBlock(unsigned char *p, size_t chunkLength) const;
		bool	IsFilled( void ) const;

		unsigned char* m_pucData;
		unsigned char m_ucFirstAvailableBlock; // 이 Chunk에서 아직 할당되지 않는 첫 번째 불록의 index
		unsigned char m_ucBlocksAvailable;		// 이 Chunk에서 아직 할당되지 않는 블록의 전체 개수

	private:
		U2Allocator* m_pAllocator;
	};

	void	DoDeallocate(void* p);

	// 주소 p를 가리키는 블록을 소유하는 Chunk를 찾는 함수
	// m_pDeallocChunk에서 시작해 앞, 뒤 양쪽 방향으로 검색한다.
	Chunk*	VicnityFind(void* p);

	void	PushBack(Chunk& );
	void	PopBack();
	void	Reserve(size_t newSize);

	/// Fewest # of objects managed by a Chunk.
	static unsigned char MinObjectsPerChunk_;

	/// Most # of objects managed by a Chunk - never exceeds UCHAR_MAX.
	static unsigned char MaxObjectsPerChunk_;

	U2Allocator *m_pAllocator;
	size_t		m_blockSize;		// 블록 크기
	unsigned char m_ucNumBlocks;	// Chunk에 의해 관리되는 블록의 수

	Chunk* m_pChunks;			
	size_t m_numChunks;			// 현재 Chunk 수
	size_t m_maxNumChunks;		// Chunk 모음의 최대 수
	Chunk* m_pAllocChunk;		// 마지막 또는 다음 할당을 위한 Chunk에 대한 포인터
	Chunk* m_pDeallocChunk;		// 마지막 또는 다음 해제를 위한 Chunk에 대한 포인터
	Chunk* m_pEmptyChunk;		// 만약 하나만 있을 때 빈 Chunk 그렇지 않으면 NULL

public:
	U2FixedAllocator();
	~U2FixedAllocator();

	void	Init(U2Allocator* pAllocator, size_t blockSize);
	void*	Allocate();

	void	Deallocate(void* p);

	size_t	BlockSize() const	{ return m_blockSize; }

	bool	operator<(size_t rhs) const { return BlockSize() < rhs; }

	size_t	GetNumChunks() const { return m_numChunks; }

};

#endif