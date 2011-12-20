#ifndef	U2_INTERLOCKEDOP_H
#define	U2_INTERLOCKEDOP_H
#pragma  once 

#include "U2DataType.h"

inline int32 U2AtomicInc(int32& iVal)
{
	return ::InterlockedIncrement((LONG*)&iVal);
}

inline int32 U2AtomicDec(int32& iVal)
{
	return ::InterlockedIncrement((LONG*)&iVal);
}

inline uint32 U2AtomicInc(uint32& uVal)
{
	return ::InterlockedIncrement((LONG*)&uVal);
}

inline uint32 U2AtomicDec(uint32& uVal)
{	
	return ::InterlockedDecrement((LONG*)&uVal);
}

#endif 