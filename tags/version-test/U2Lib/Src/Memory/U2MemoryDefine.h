#ifndef U2_MEMORY_DEFINE_H
#define U2_MEMORY_DEFINE_H

#include <u2lib/src/U2DataType.h>

#ifdef _DEBUG 
#define USE_DEBUG_MEMORY
#endif


#define U2_MEM_FILE				0
#define U2_MEM_LINE				-1
#define U2_MEM_FUNCTION			0
#define U2_MEM_DEALLOC_SIZE		(size_t)-1

#define U2_MEM_ALIGNMENT		4

#define IS_POWER_OF_TWO(x)   (((x)&(x-1)) == 0)

enum U2MemFlag
{
	MEMFLAG_NONE = 0,
	MEMFLAG_COMPILER_PROVIDE_SIZE_ON_DEALLOCATE = 1,
	MEMFLAG_MASK = 0xffff
};


enum U2MemType 
{
	MEM_UNKNOWN = 0,
	MEM_NEW,
	MEM_NEW_ARRAY,
	MEM_DELETE,
	MEM_DELETE_ARRAY,
	MEM_MALLOC,
	MEM_REALLOC,
	MEM_ALIGNEDMALLOC,
	MEM_ALIGNEDREALLOC,
	MEM_FREE_,
	MEM_ALIGNEDFREE,
	MEM_EXTERNALLOC,
	MEM_EXTERNFREE,
};

U2LIB	void _U2Free(void* pvMem);
U2LIB	void _U2AlignedFree(void* pvMem);

#ifdef USE_DEBUG_MEMORY

	U2LIB  void*		U2_Malloc_(size_t sizeInBytes, uint32 uiMemFlag, 
								   const char* szFile, int line, const char* szFunction);
	U2LIB  void*		U2_AllignedMalloc_(size_t sizeInBytes, size_t alignment, uint32 uiMemFlag,
											   const char* szFile, int line, const char* szFunction);

	U2LIB bool			U2VerifyPtr(const void* pvMem);

	#define U2_ALLOC(T, count) ((T*)U2_Malloc_(sizeof(T)*(count), MEMFLAG_NONE, \
	__FILE__, __LINE__, __FUNCTION__))
	#define U2_ALIGNED_ALLOC(T, count, alignment ) ((T*)U2_AllignedMalloc_( \
			sizeof(T) *(count), alignment, MEMFLAG_NONE, __FILE__, __LINE__, __FUNCTION__))

	#define U2_ALIGNED_MALLOC(size, alignment ) (U2_AllignedMalloc_( \
		size, alignment, MEMFLAG_NONE, __FILE__, __LINE__, __FUNCTION__))


	#define U2_NEW					new(MEMFLAG_NONE, __FILE__, __LINE__, __FUNCTION__)					
	#define U2_DELETE				delete 

	#define U2_MALLOC(size)			(U2_Malloc_(size, MEMFLAG_NONE, __FILE__, __LINE__, __FUNCTION__))


	#define U2_FREE(p)			_U2Free(p)
	#define U2_ALIGNEDFREE(p)	_U2AlignedFree(p)
#else 
	U2LIB void*		U2_Malloc_(size_t sizeinBytes, uint32 uiMemFlag);
	U2LIB void*		U2_AllignedMalloc_(size_t sizeInBytes, size_t alignment, uint32 uiMemFlag);
	#define U2_NEW		new(MEMFLAG_NONE) 
	#define U2_DELETE	delete
	#define U2_ALLOC(T, count)	((T*)U2_Malloc_(sizeof(T) * (count), MEMFLAG_NONE))
	#define U2_ALIGNED_ALLOC(T, count, alignment) ((T*)U2_AllignedMalloc_(sizeof(T) * (count), alignment, MEMFLAG_NONE))
	#define U2_ALIGNED_MALLOC(size, alignment) (U2_AllignedMalloc(size, alignment, MEMFLAG_NONE)
	
	#define U2_MALLOC(size)			(U2_Malloc_(size, MEMFLAG_NONE))


	#define U2_FREE(p)			_U2Free(p)
	#define U2_ALIGNEDFREE(p)	_U2AlignedFree(p)
#endif 

#define U2ExternNew					new 
#define U2ExternDelete				delete
#define U2ExternAlloc(T, count)		((T*)malloc(sizeof(T)*(count)))
#define U2ExternMalloc				malloc
#define U2ExternRealloc				realloc 
#define U2ExternFree				free
#define U2ExternCalloc				calloc 

#define U2ExternAlignedMalloc		_aligned_malloc
#define U2ExternAlignedRealloc		_aligned_realloc
#define U2ExternAlignedFree			_aligned_free
#define U2AlignOf(T)				__alignof(T)

#endif

	