/************************************************************************
module  : U2FixedAllocator
Author  : yun sangyong
Desc	: Gamebryo 2.6 �޸� �Ŵ��� 
		  ������ Modern C++ Design�� Loki�ȿ� SmllObj Allocator 
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
		unsigned char m_ucFirstAvailableBlock; // �� Chunk���� ���� �Ҵ���� �ʴ� ù ��° �ҷ��� index
		unsigned char m_ucBlocksAvailable;		// �� Chunk���� ���� �Ҵ���� �ʴ� ����� ��ü ����

	private:
		U2Allocator* m_pAllocator;
	};

	void	DoDeallocate(void* p);

	// �ּ� p�� ����Ű�� ����� �����ϴ� Chunk�� ã�� �Լ�
	// m_pDeallocChunk���� ������ ��, �� ���� �������� �˻��Ѵ�.
	Chunk*	VicnityFind(void* p);

	void	PushBack(Chunk& );
	void	PopBack();
	void	Reserve(size_t newSize);

	/// Fewest # of objects managed by a Chunk.
	static unsigned char MinObjectsPerChunk_;

	/// Most # of objects managed by a Chunk - never exceeds UCHAR_MAX.
	static unsigned char MaxObjectsPerChunk_;

	U2Allocator *m_pAllocator;
	size_t		m_blockSize;		// ��� ũ��
	unsigned char m_ucNumBlocks;	// Chunk�� ���� �����Ǵ� ����� ��

	Chunk* m_pChunks;			
	size_t m_numChunks;			// ���� Chunk ��
	size_t m_maxNumChunks;		// Chunk ������ �ִ� ��
	Chunk* m_pAllocChunk;		// ������ �Ǵ� ���� �Ҵ��� ���� Chunk�� ���� ������
	Chunk* m_pDeallocChunk;		// ������ �Ǵ� ���� ������ ���� Chunk�� ���� ������
	Chunk* m_pEmptyChunk;		// ���� �ϳ��� ���� �� �� Chunk �׷��� ������ NULL

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