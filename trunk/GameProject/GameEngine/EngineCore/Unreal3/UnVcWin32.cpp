#include "Core.h"
#include "UnVcWin32.h"
#include "MemoryBase.h"
#include "UnArc.h"
#include "Array.h"
#include "FMallocThreadSafeProxy.h"
/** Does PC specific initialization of timing information */
void appInitTiming(void)
{
	LARGE_INTEGER Frequency;
	assert( QueryPerformanceFrequency(&Frequency) );
	GSecondsPerCycle = 1.0 / Frequency.QuadPart;
	GStartTime = appSeconds();
}

void GCreateMalloc()
{
	GMalloc = new FMallocWindows;
	// if the allocator is already thread safe, there is no need for the thread safe proxy
	if (!GMalloc->IsInternallyThreadSafe())
	{
		GMalloc = new FMallocThreadSafeProxy( GMalloc );
	}
}