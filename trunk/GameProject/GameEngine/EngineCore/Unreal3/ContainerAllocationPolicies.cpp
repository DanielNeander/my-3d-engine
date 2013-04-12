#include "Core.h"
#include "ContainerAllocationPolicies.h"

/** Whether array slack is being tracked. */
#define TRACK_ARRAY_SLACK 0

#if TRACK_ARRAY_SLACK
FStackTracker* GSlackTracker = NULL;
#endif

INT DefaultCalculateSlack(INT NumElements,INT NumAllocatedElements,UINT BytesPerElement)
{
#if TRACK_ARRAY_SLACK 
	if( !GSlackTracker )
	{
		GSlackTracker = new FStackTracker();
	}
#define SLACK_TRACE_TO_SKIP 4
	GSlackTracker->CaptureStackTrace(SLACK_TRACE_TO_SKIP);
#endif

	if(NumElements < NumAllocatedElements)
	{
		// If the container has too much slack, shrink it to exactly fit the number of elements.
		const UINT CurrentSlackElements = NumAllocatedElements-NumElements;
		const UINT CurrentSlackBytes = (NumAllocatedElements-NumElements)*BytesPerElement;
		const UBOOL bTooManySlackBytes = CurrentSlackBytes >= 16384;
		const UBOOL bTooManySlackElements = 3*NumElements < 2*NumAllocatedElements;
		if(	(bTooManySlackBytes || bTooManySlackElements) && (CurrentSlackElements > 64 || !NumElements) )
		{
			return NumElements;
		}
		else
		{
			return NumAllocatedElements;
		}
	}
	else if(NumElements > 0)
	{
		// Allocate slack for the array proportional to its size.
		return NumElements + 3*NumElements/8 + 16;
	}
	else
	{
		return 0;
	}
}