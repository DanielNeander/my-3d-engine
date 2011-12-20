/************************************************************************
module  : U2MemTracker
Author  : yun sangyong
Desc	: Gamebryo 2.6 메모리 할당자
			Reallocate, TrackAllocate, TrackDeallocate, SetMarker 미구현.
************************************************************************/
#ifndef	U2_MEMTRACKER_H
#define	U2_MEMTRACKER_H

#include <U2Lib/Src/U2LibType.h>
#include <U2Lib/src/U2Lib.h>
#include "U2Allocator.h"

class U2LIB U2AllocUnit
{
public :
	size_t m_stAllocationId;
	void* m_pvMem;
	size_t m_stAlignment;
	size_t m_stRequestedSize;
	size_t m_stAllocatedSize;

	// hash table support
	U2AllocUnit* m_pPrev;
	U2AllocUnit* m_pNext;

	unsigned long m_ulAllocThreadId;
	float m_fAllocTime;
	U2MemType m_eAllocType;
	U2MemFlag m_eAllocFlags;

	void Reset();
};

class U2LIB U2MemTracker : public U2Allocator
{
public:
	U2MemTracker(U2Allocator* pActualAllocator, 
		bool bWriteToLog = true, 
		unsigned int uiInitSize = 65536, 
		unsigned int uiGrowBy = 4096, 
		bool bAlwaysValidateAll = false,
		bool bCheckArrayOverruns = true);

	virtual ~U2MemTracker();

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
		const char* szFunction) ;

	// Memory Tracking
	virtual bool TrackAllocate(
		const void* const pvMem,
		size_t sizeInBytes,
		uint32 bMemFlags,
		U2MemType eMemType,
		const char* szFile,
		int line, 
		const char* szFunction) ;

	virtual bool TrackDeallocate(
		const void* const pvMem,
		U2MemType eMemType) ;

	virtual bool SetMark(const char* szMarkType, const char* szClassfier, 
		const char* szStr) ;

	virtual void Init() ;
	virtual void Terminate() ;

	virtual bool VertifyPtr(const void* pvMem) ;

	// Find the address of an allocation that contains pvMemory.
	virtual void* FindContainingAlloc(const void* pvMem);

	void ResetSummaryStats();


	size_t m_stActiveMem;
	size_t m_stPeakMem;
	size_t m_stAccumMem;
	size_t m_stUnusedButAllocatedMem;
	
	size_t m_stActiveAllocCnt;
	size_t m_stPeakAllocCnt;
	size_t m_stAccumAllocCnt;
	
	size_t m_stActiveTrackerOverhead;
	size_t m_stPeakTrackerOverhead;
	size_t m_StAccumTrackerOverhead;	

	size_t m_stActiveExternMem;
	size_t m_stPeakExternMem;
	size_t m_stAccumExternMem;

	size_t m_stActiveExternAllocCnt;
	size_t m_stPeakExternAllocCnt;
	size_t m_stAccumExternAllocCnt;

	float m_fPeakMemTime;
	float m_fPeakAllocCntTime;

	size_t GetCurrtAllocID() const;

	U2AllocUnit* FindAllocUnit(const void* pvMem) const;

	inline bool GetAlwaysValidateAll() const;
	inline void SetAlwaysValidateAll(bool bOn);

	inline bool GetCheckForArrayOverruns() const;

	bool ValidateAllocUnit(const U2AllocUnit* pkUnit) const;
	bool ValidateAllAllocUnits() const;

	static size_t ms_stBreakOnAllocID;
	static void* ms_pvBreakOnAllocRangeStart;
	static void* ms_pvBreakOnAllocRangeEnd;
	static const TCHAR* ms_pcBreakOnFunctionName;
	static bool ms_bOutputLeaksToDebugStream;
	static size_t ms_stBreakOnSizeRequested;

	static U2MemTracker* Instance();

protected:
	void GrowReservoir();
	void InsertAllocUnit(U2AllocUnit* pUnit);
	void RemoveAllocUnit(U2AllocUnit* punit);

	static unsigned int AddressToHashIndex(const void* pvAddress);
	void MemFillWithPattern(void* pvMem, size_t sizeInBytes);
	size_t MemBytesWithPattern(void* pvMem, size_t sizeInBytes) const;

	// Note that MemoryFillForArrayOverrun and CheckForArrayOverrun
	// modify the address passed in to reflect the sentinel characters
	// before the allocation. PadForArrayOverrun will adjust the size
	// of an allocation to reflect the sentinel characters.

	void MemfillForArrayOverrun(void*& pvMem, size_t alignment, size_t sizeOrig);
	bool CheckForArrayOverrun(void*& pvMem, size_t alignment, size_t sizeOrig) const ;
	size_t PadForArrayOverrun(size_t alignment, size_t sizeOrig);

	static bool IsInsideBreakRange(const void* pvMem, size_t size);

	static const unsigned int ms_uiHashBits = 12;
	static const unsigned int ms_uiHashSize = 1 << ms_uiHashBits;

	bool m_bAlwaysValidateAll;

	U2AllocUnit* m_pReservoir;
	size_t m_stReservoirGrowBy;
	U2AllocUnit** m_ppReservoirBuff;
	size_t m_stReservoirBuffSize;
	U2AllocUnit* m_pActiveMem[ms_uiHashSize];

	U2Allocator* m_pActualAllocator;
	size_t m_StCurrAllocID;

	bool m_bCheckArrayOverruns;
	unsigned char m_ucFillChar;
	bool m_bWriteToLog;

	static U2MemTracker* ms_pTracker;
//	U2CriticalSection m_cs;

};

#include "U2MemTracker.inl"

#endif 
