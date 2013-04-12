/*=============================================================================
	FMallocWindows.h: Windows optimized memory allocator.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __FMALLOCWINDOWS_H__
#define __FMALLOCWINDOWS_H__

#define MEM_TIME(st)

//
// Optimized Windows virtual memory allocator.
//
class FMallocWindows : public FMalloc
{
private:
	// Counts.
	enum {POOL_COUNT = 42     };
	enum {POOL_MAX   = 32768+1};

	// Forward declares.
	struct FFreeMem;
	struct FPoolTable;

	// Memory pool info. 32 bytes.
	struct FPoolInfo
	{
		DWORD	    Bytes;		// Bytes allocated for pool.
		DWORD		OsBytes;	// Bytes aligned to page size.
		DWORD       Taken;      // Number of allocated elements in this pool, when counts down to zero can free the entire pool.
		BYTE*       Mem;		// Memory base.
		FPoolTable* Table;		// Index of pool.
		FFreeMem*   FirstMem;   // Pointer to first free memory in this pool.
		FPoolInfo*	Next;
		FPoolInfo**	PrevLink;

		void Link( FPoolInfo*& Before )
		{
			if( Before )
			{
				Before->PrevLink = &Next;
			}
			Next     = Before;
			PrevLink = &Before;
			Before   = this;
		}
		void Unlink()
		{
			if( Next )
			{
				Next->PrevLink = PrevLink;
			}
			*PrevLink = Next;
		}
	};

	// Information about a piece of free memory. 8 bytes.
	struct FFreeMem
	{
		FFreeMem*	Next;		// Next or MemLastPool[], always in order by pool.
		DWORD		Blocks;		// Number of consecutive free blocks here, at least 1.
		FPoolInfo* GetPool()
		{
			return (FPoolInfo*)((INT)this & 0xffff0000);
		}
	};

	// Pool table.
	struct FPoolTable
	{
		FPoolInfo* FirstPool;
		FPoolInfo* ExhaustedPool;
		DWORD      BlockSize;
	};

	// Variables.
	FPoolTable  PoolTable[POOL_COUNT], OsTable;
	FPoolInfo*	PoolIndirect[32];
	FPoolTable* MemSizeToPoolTable[POOL_MAX];
	INT			OsCurrent,OsPeak,UsedCurrent,UsedPeak,CurrentAllocs,TotalAllocs;
	DOUBLE		MemTime;
	INT			PageSize;

	// Implementation. 
	void OutOfMemory()
	{
		//appErrorf( *LocalizeError("OutOfMemory",TEXT("Core")) );
		assert(false);
	}
	FPoolInfo* CreateIndirect()
	{
		FPoolInfo* Indirect = (FPoolInfo*)VirtualAlloc( NULL, 2048*sizeof(FPoolInfo), MEM_COMMIT, PAGE_READWRITE );
		if( !Indirect )
		{
			OutOfMemory();
		}
		return Indirect;
	}

public:
	// FMalloc interface.
	FMallocWindows()
	:	OsCurrent		( 0 )
	,	OsPeak			( 0 )
	,	UsedCurrent		( 0 )
	,	UsedPeak		( 0 )
	,	CurrentAllocs	( 0 )
	,	TotalAllocs		( 0 )
	,	MemTime			( 0.0 )
	,	PageSize		( 0 )
	{
		// Get OS page size.
		SYSTEM_INFO SI;
		GetSystemInfo( &SI );
		PageSize = SI.dwPageSize;
		assert(!(PageSize&(PageSize-1)));

		// Init tables.
		OsTable.FirstPool    = NULL;
		OsTable.ExhaustedPool = NULL;
		OsTable.BlockSize    = 0;

		PoolTable[0].FirstPool    = NULL;
		PoolTable[0].ExhaustedPool = NULL;
		PoolTable[0].BlockSize    = 8;
		for( DWORD i=1; i<5; i++ )
		{
			PoolTable[i].FirstPool    = NULL;
			PoolTable[i].ExhaustedPool = NULL;
			PoolTable[i].BlockSize    = (8<<((i+1)>>2)) + (2<<i);
		}
		for( DWORD i=5; i<POOL_COUNT; i++ )
		{
			PoolTable[i].FirstPool    = NULL;
			PoolTable[i].ExhaustedPool = NULL;
			PoolTable[i].BlockSize    = (4+((i+7)&3)) << (1+((i+7)>>2));
		}
		for( DWORD i=0; i<POOL_MAX; i++ )
		{
			DWORD Index;
			for( Index=0; PoolTable[Index].BlockSize<i; Index++ );
			assert(Index<POOL_COUNT);
			MemSizeToPoolTable[i] = &PoolTable[Index];
		}
		for( DWORD i=0; i<ARRAY_COUNT(PoolIndirect); i++ )
		{
			PoolIndirect[i] = NULL;
		}
		assert(POOL_MAX-1==PoolTable[POOL_COUNT-1].BlockSize);
	}
	
	virtual void* Malloc( DWORD Size, DWORD Alignment )
	{
		assert(Alignment == DEFAULT_ALIGNMENT && "Alignment currently unsupported in Windows");
		MEM_TIME(MemTime -= appSeconds());
		STAT(CurrentAllocs++);
		STAT(TotalAllocs++);
		FFreeMem* Free;
		if( Size<POOL_MAX )
		{
			// Allocate from pool.
			FPoolTable* Table = MemSizeToPoolTable[Size];
			assert(Size<=Table->BlockSize);
			FPoolInfo* Pool = Table->FirstPool;
			if( !Pool )
			{
				// Must create a new pool.
				DWORD Blocks  = 65536 / Table->BlockSize;
				DWORD Bytes   = Blocks * Table->BlockSize;
				assert(Blocks>=1);
				assert(Blocks*Table->BlockSize<=Bytes);

				// Allocate memory.
				Free = (FFreeMem*)VirtualAlloc( NULL, Bytes, MEM_COMMIT, PAGE_READWRITE );
				if( !Free )
				{
					OutOfMemory();
				}

				// Create pool in the indirect table.
				FPoolInfo*& Indirect = PoolIndirect[((DWORD)Free>>27)];
				if( !Indirect )
				{
					Indirect = CreateIndirect();
				}
				Pool = &Indirect[((DWORD)Free>>16)&2047];

				// Init pool.
				Pool->Link( Table->FirstPool );
				Pool->Mem            = (BYTE*)Free;
				Pool->Bytes		     = Bytes;
				Pool->OsBytes		 = Align(Bytes,PageSize);
				STAT(OsPeak = Max(OsPeak,OsCurrent+=Pool->OsBytes));
				Pool->Table		     = Table;
				Pool->Taken			 = 0;
				Pool->FirstMem       = Free;

				// Create first free item.
				Free->Blocks         = Blocks;
				Free->Next           = NULL;
			}

			// Pick first available block and unlink it.
			Pool->Taken++;
			assert(Pool->FirstMem);
			assert(Pool->FirstMem->Blocks>0);
			Free = (FFreeMem*)((BYTE*)Pool->FirstMem + --Pool->FirstMem->Blocks * Table->BlockSize);
			if( Pool->FirstMem->Blocks==0 )
			{
				Pool->FirstMem = Pool->FirstMem->Next;
				if( !Pool->FirstMem )
				{
					// Move to exhausted list.
					Pool->Unlink();
					Pool->Link( Table->ExhaustedPool );
				}
			}
			STAT(UsedPeak = Max(UsedPeak,UsedCurrent+=Table->BlockSize));
		}
		else
		{
			// Use OS for large allocations.
			INT AlignedSize = Align(Size,PageSize);
			Free = (FFreeMem*)VirtualAlloc( NULL, AlignedSize, MEM_COMMIT, PAGE_READWRITE );
			if( !Free )
			{
				OutOfMemory();
			}
			assert(!((SIZE_T)Free&65535));

			// Create indirect.
			FPoolInfo*& Indirect = PoolIndirect[((DWORD)Free>>27)];
			if( !Indirect )
			{
				Indirect = CreateIndirect();
			}

			// Init pool.
			FPoolInfo* Pool = &Indirect[((DWORD)Free>>16)&2047];
			Pool->Mem		= (BYTE*)Free;
			Pool->Bytes		= Size;
			Pool->OsBytes	= AlignedSize;
			Pool->Table		= &OsTable;
			STAT(OsPeak   = Max(OsPeak,  OsCurrent+=AlignedSize));
			STAT(UsedPeak = Max(UsedPeak,UsedCurrent+=Size));
		}
		MEM_TIME(MemTime += appSeconds());
		return Free;
	}
	
	virtual void* Realloc( void* Ptr, DWORD NewSize, DWORD Alignment )
	{
		assert(Alignment == DEFAULT_ALIGNMENT && "Alignment currently unsupported in Windows");
		MEM_TIME(MemTime -= appSeconds());
		void* NewPtr = Ptr;
		if( Ptr && NewSize )
		{
			FPoolInfo* Pool = &PoolIndirect[(DWORD)Ptr>>27][((DWORD)Ptr>>16)&2047];
			if( Pool->Table!=&OsTable )
			{
				// Allocated from pool, so grow or shrink if necessary.
				if( NewSize>Pool->Table->BlockSize || MemSizeToPoolTable[NewSize]!=Pool->Table )
				{
					NewPtr = Malloc( NewSize, Alignment );
					appMemcpy( NewPtr, Ptr, Min(NewSize,Pool->Table->BlockSize) );
					Free( Ptr );
				}
			}
			else
			{
				// Allocated from OS.
				assert(!((INT)Ptr&65535));
				if( NewSize>Pool->OsBytes || NewSize*3<Pool->OsBytes*2 )
				{
					// Grow or shrink.
					NewPtr = Malloc( NewSize, Alignment );
					appMemcpy( NewPtr, Ptr, Min(NewSize,Pool->Bytes) );
					Free( Ptr );
				}
				else
				{
					// Keep as-is, reallocation isn't worth the overhead.
					STAT(UsedCurrent+=NewSize-Pool->Bytes);
					STAT(UsedPeak=Max(UsedPeak,UsedCurrent));
					Pool->Bytes = NewSize;
				}
			}
		}
		else if( Ptr == NULL )
		{
			NewPtr = Malloc( NewSize, Alignment );
		}
		else
		{
			Free( Ptr );
			NewPtr = NULL;
		}
		MEM_TIME(MemTime += appSeconds());
		return NewPtr;
	}
	
	virtual void Free( void* Ptr )
	{
		if( !Ptr )
		{
			return;
		}
		MEM_TIME(MemTime -= appSeconds());
		STAT(CurrentAllocs--);

		// Windows version.
		FPoolInfo* Pool = &PoolIndirect[(DWORD)Ptr>>27][((DWORD)Ptr>>16)&2047];
		assert(Pool->Bytes!=0);
		if( Pool->Table!=&OsTable )
		{
			// If this pool was exhausted, move to available list.
			if( !Pool->FirstMem )
			{
				Pool->Unlink();
				Pool->Link( Pool->Table->FirstPool );
			}

			// Free a pooled allocation.
			FFreeMem* Free		= (FFreeMem *)Ptr;
			Free->Blocks		= 1;
			Free->Next			= Pool->FirstMem;
			Pool->FirstMem		= Free;
			STAT(UsedCurrent   -= Pool->Table->BlockSize);

			// Free this pool.
			assert(Pool->Taken>=1);
			if( --Pool->Taken == 0 )
			{
				// Free the OS memory.
				Pool->Unlink();
				assert( VirtualFree( Pool->Mem, 0, MEM_RELEASE )!=0 );
				STAT(OsCurrent -= Pool->OsBytes);
			}
		}
		else
		{
			// Free an OS allocation.
			assert(!((INT)Ptr&65535));
			STAT(UsedCurrent -= Pool->Bytes);
			STAT(OsCurrent   -= Pool->OsBytes);
			assert( VirtualFree( Ptr, 0, MEM_RELEASE )!=0 );
		}
		MEM_TIME(MemTime += appSeconds());
	}
	/**
	 * Gathers memory allocations for both virtual and physical allocations.
	 *
	 * @param Virtual	[out] size of virtual allocations
	 * @param Physical	[out] size of physical allocations	
	 */
	virtual void GetAllocationInfo( SIZE_T& Virtual, SIZE_T& Physical )
	{
		Virtual		= OsCurrent;
		Physical	= 0;
	}

	/*virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar ) 
	{ 
#if !SHIPPING_PC_GAME
		if( ParseCommand(&Cmd,TEXT("HEAPCHECK")) )
		{
			Heapassert();
			return TRUE;
		}
		else
#endif
		if( ParseCommand(&Cmd,TEXT("DUMPALLOCS")) )
		{
			const FString	Token			= ParseToken( Cmd, 0 );
			const UBOOL		bSummaryOnly	= Token == TEXT("SUMMARYONLY");
			DumpAllocs( bSummaryOnly, Ar );
			return TRUE;
		}

		return FALSE;
	}*/

	void Heapassert()
	{
		for( INT i=0; i<POOL_COUNT; i++ )
		{
			FPoolTable* Table = &PoolTable[i];
			for( FPoolInfo** PoolPtr=&Table->FirstPool; *PoolPtr; PoolPtr=&(*PoolPtr)->Next )
			{
				FPoolInfo* Pool=*PoolPtr;
				assert(Pool->PrevLink==PoolPtr);
				assert(Pool->FirstMem);
				for( FFreeMem* Free=Pool->FirstMem; Free; Free=Free->Next )
				{
					assert(Free->Blocks>0);
				}
			}
			for( FPoolInfo** PoolPtr=&Table->ExhaustedPool; *PoolPtr; PoolPtr=&(*PoolPtr)->Next )
			{
				FPoolInfo* Pool=*PoolPtr;
				assert(Pool->PrevLink==PoolPtr);
				assert(!Pool->FirstMem);
			}
		}
	}
};

#endif
