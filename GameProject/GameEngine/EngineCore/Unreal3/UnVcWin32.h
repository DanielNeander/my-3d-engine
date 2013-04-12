#pragma once 

inline DOUBLE appSeconds()
{
	LARGE_INTEGER Cycles;
	QueryPerformanceCounter(&Cycles);
	// Add big number to make bugs apparent where return value is being passed to FLOAT
	return Cycles.QuadPart * GSecondsPerCycle + 16777216.0;
}