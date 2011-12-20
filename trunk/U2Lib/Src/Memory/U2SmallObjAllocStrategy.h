/************************************************************************
module  : U2SmallObjAllocStrategy
Author  : yun sangyong
Desc	: Gamebryo 2.6 메모리 매니저 
원본은 Modern C++ Design의 Loki안에 SmllObj Allocator 
************************************************************************/


#ifndef U2_SMALLOBJSTRATEGY_H
#define U2_SMALLOBJSTRATEGY_H

#ifndef MAX_SMALL_OBJ_SIZE
#define MAX_SMALL_OBJ_SIZE 256
#endif

#include "U2FixedAllocator.h"
#include "U2MemoryDefine.h"

template<class TAllocator>
class U2SmallObjAllocStrategy : public U2Allocator
{

public:
	U2SmallObjAllocStrategy(size_t chunkSize = DEFAULT_CHUNK_SIZE);

	virtual void* Allocate(
		size_t& sizeInBytes, 
		size_t& alignment, 
		uint32 bMemFlags,
		U2MemType eMemType,
		const char* szFile,
		int line,
		const char* szFunction);

	virtual void Deallocate(
		void* pvMem,
		U2MemType eMemType,
		size_t sizeInBytes);

	virtual void* Reallocate(
		void* pvMem,
		size_t& sizeInBytes,
		size_t& alignment,
		uint32 bMemFlags,
		U2MemType eMemType,
		size_t sizeCurr,
		const char* szFile, int line,
		const char* szFunction);

	// Memory Tracking
	virtual bool TrackAllocate(
		const void* const pvMem,
		size_t sizeInBytes,
		uint32 bMemFlags,
		U2MemType eMemType,
		const char* szFile,
		int line, 
		const char* szFunction);

	virtual bool TrackDeallocate(
		const void* const pvMem,
		U2MemType eMemType);

	virtual bool SetMark(const char* szMarkType, const char* szClassfier, 
		const char* szStr);

	virtual void Init();
	virtual void Terminate();

	virtual bool VertifyPtr(const void* pvMem);
	
	void SetSizeOnDeallocateFlag(bool bSet);
	bool CompilerProvideSizeOnDeallocate() const;

private:
	U2SmallObjAllocStrategy(const U2SmallObjAllocStrategy&) {}
	U2SmallObjAllocStrategy& operator=(const U2SmallObjAllocStrategy&) { return *(this); }

	void* Allocate(size_t numBytes);
	void Deallocate(void* p, size_t size);
	U2FixedAllocator*	GetFixedAllocatorForSize(size_t numBytes);
	size_t GetSizeFromAddress(void* pvMem);
	void	SetSizeToAddress(void* pvMem, size_t size );

	TAllocator m_allocator;
	U2FixedAllocator	m_pool[MAX_SMALL_OBJ_SIZE];
	size_t	m_chunkSize;
};

#include "U2SmallObjAllocStrategy.inl"

#endif