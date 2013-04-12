#pragma once 

#include "EngineCore/Types.h"
#include "EngineCore/Util/Str.h"
#include "EngineCore/Containers/List.h"
#include "EngineCore/Containers/StrList.h"

typedef enum {
	CPUID_NONE							= 0x00000,
	CPUID_UNSUPPORTED					= 0x00001,	// unsupported (386/486)
	CPUID_GENERIC						= 0x00002,	// unrecognized processor
	CPUID_INTEL							= 0x00004,	// Intel
	CPUID_AMD							= 0x00008,	// AMD
	CPUID_MMX							= 0x00010,	// Multi Media Extensions
	CPUID_3DNOW							= 0x00020,	// 3DNow!
	CPUID_SSE							= 0x00040,	// Streaming SIMD Extensions
	CPUID_SSE2							= 0x00080,	// Streaming SIMD Extensions 2
	CPUID_SSE3							= 0x00100,	// Streaming SIMD Extentions 3 aka Prescott's New Instructions
	CPUID_ALTIVEC						= 0x00200,	// AltiVec
	CPUID_HTT							= 0x01000,	// Hyper-Threading Technology
	CPUID_CMOV							= 0x02000,	// Conditional Move (CMOV) and fast floating point comparison (FCOMI) instructions
	CPUID_FTZ							= 0x04000,	// Flush-To-Zero mode (denormal results are flushed to zero)
	CPUID_DAZ							= 0x08000	// Denormals-Are-Zero mode (denormal source operands are set to zero)
} cpuid_t;

typedef struct sysMemoryStats_s {
	int memoryLoad;
	int totalPhysical;
	int availPhysical;
	int totalPageFile;
	int availPageFile;
	int totalVirtual;
	int availVirtual;
	int availExtendedVirtual;
} sysMemoryStats_t;

typedef unsigned long address_t;


// a decent minimum sleep time to avoid going below the process scheduler speeds
#define			SYS_MINSLEEP	20


// Sys_Milliseconds should only be used for profiling purposes,
// any game related timing information should come from event timestamps
int				Sys_Milliseconds( void );

// for accurate performance testing
double			Sys_GetClockTicks( void );
double			Sys_ClockTicksPerSecond( void );

// allow game to yield CPU time
// NOTE: due to SYS_MINSLEEP this is very bad portability karma, and should be completely removed
void			Sys_Sleep( int msec );

// sets Flush-To-Zero mode (only available when CPUID_FTZ is set)
void			Sys_FPU_SetFTZ( bool enable );

// sets Denormals-Are-Zero mode (only available when CPUID_DAZ is set)
void			Sys_FPU_SetDAZ( bool enable );

// returns amount of system ram
int				Sys_GetSystemRam( void );

// returns amount of video ram
int				Sys_GetVideoRam( void );


// returns amount of drive space in path
int				Sys_GetDriveFreeSpace( const char *path );

// returns memory stats
void			Sys_GetCurrentMemoryStatus( sysMemoryStats_t &stats );
void			Sys_GetExeLaunchMemoryStatus( sysMemoryStats_t &stats );

// lock and unlock memory
bool			Sys_LockMemory( void *ptr, int bytes );
bool			Sys_UnlockMemory( void *ptr, int bytes );


// DLL loading, the path should be a fully qualified OS path to the DLL file to be loaded
int				Sys_DLL_Load( const char *dllName );
void *			Sys_DLL_GetProcAddress( int dllHandle, const char *procName );
void			Sys_DLL_Unload( int dllHandle );
cpuid_t			Sys_GetCPUId( void );

typedef enum {
	NA_BAD,					// an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP
} netadrtype_t;

typedef struct {
	netadrtype_t	type;
	unsigned char	ip[4];
	unsigned short	port;
} netadr_t;


typedef unsigned int (*xthread_t)( void * );

typedef enum {
	THREAD_NORMAL,
	THREAD_ABOVE_NORMAL,
	THREAD_HIGHEST
} xthreadPriority;

typedef struct {
	const char *	name;
	int				threadHandle;
	unsigned long	threadId;
} xthreadInfo;

const int MAX_THREADS				= 10;
extern xthreadInfo *g_threads[MAX_THREADS];
extern int			g_thread_count;

void				Sys_CreateThread( xthread_t function, void *parms, xthreadPriority priority, xthreadInfo &info, const char *name, xthreadInfo *threads[MAX_THREADS], int *thread_count );
void				Sys_DestroyThread( xthreadInfo& info ); // sets threadHandle back to 0

// find the name of the calling thread
// if index != NULL, set the index in g_threads array (use -1 for "main" thread)
const char *		Sys_GetThreadName( int *index = 0 );

const int MAX_CRITICAL_SECTIONS		= 4;

enum {
	CRITICAL_SECTION_ZERO = 0,
	CRITICAL_SECTION_ONE,
	CRITICAL_SECTION_TWO,
	CRITICAL_SECTION_THREE
};

void				Sys_EnterCriticalSection( int index = CRITICAL_SECTION_ZERO );
void				Sys_LeaveCriticalSection( int index = CRITICAL_SECTION_ZERO );
void Sys_SetClipboardData( const char *string );
char *Sys_GetClipboardData( void );
int Sys_ListFiles( const char *directory, const char *extension, idStrList &list );