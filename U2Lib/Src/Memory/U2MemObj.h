/************************************************************************
module  : U2MemObj
Author  : Yun sangyong 
Desc	: ������ ��� ��ü���� �� ��ü�� ��ӹ޾� �޸𸮸� ���ӹް� �ȴ�.
************************************************************************/
#ifndef U2_MEMOBJ_H
#define U2_MEMOBJ_H

#include <U2Lib/Src/U2LibType.h>
#include "U2MemoryDefine.h"


class U2LIB U2MemObj
{
#ifdef USE_DEBUG_MEMORY
private:
	static void* operator new(size_t size, uint32 uiMemFlag = MEMFLAG_NONE);
	static void* operator new[](size_t size, uint32 uiMemFlag = MEMFLAG_NONE);	
public:
	static void* operator new(
		size_t size, 
		uint32 uiMemFlag,
		const char* szFile,
		int line, const char* szFunction);

	static void* operator new[](
		size_t size, 
		uint32 uiMemFlag,
		const char* szFile, 
		int line,
		const char* szFuction);

	static void operator delete(
		void*, uint32, const char*, int, const char*){}
	static void operator delete[](
		void*, uint32, const char*, int, const char*) {}
#else 
public:
	static void* operator new(size_t size, uint32 uiMemFlag = MEMFLAG_NONE);
	static void* operator new[](size_t size, uint32 uiMemFlag = MEMFLAG_NONE);
#endif

public:
	static void operator delete(void *pvMem, size_t stElementSize);
	static void operator delete[](void *pvMem, size_t stElementSize);

	// We don't want to hide the placement new functions:
	static void* operator new( size_t, void* p ) { return p; }
	static void* operator new[]( size_t, void* p ) { return p; }

	// Conversely we also then need a placement delete function:
	static void operator delete( void *, void* ) {}
	static void operator delete[]( void *, void* ) {}	
};


#endif