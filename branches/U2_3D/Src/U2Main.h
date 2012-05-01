/**************************************************************************************************
module	:	U2Main
Author	:	Yun sangyong
Desc	:	�ѹ� �����ϸ� ������� �ʴ� �����͵��� ������ ����Ѵ�.
***************************************************************************************************/
#pragma once
#ifndef U2_MAIN_H
#define U2_MAIN_H

#include <U2_3D/src/U2_3DLibType.h>


class U2_3D U2Main
{
public:
	typedef void (*Initializer)(void);
	typedef U2PrimitiveVec<Initializer> InitializerArray;
	static void AddInitializer (Initializer oInitialize);
	static void Initialize ();

	typedef void (*Terminator)(void);
	typedef U2PrimitiveVec<Terminator> TerminatorArray;
	static void AddTerminator (Terminator oTerminate);
	static void Terminate ();

private:
	static InitializerArray* ms_Initializers;
	static TerminatorArray* ms_Terminators;	

	static U2Allocator* ms_pAllocator;

	static bool ms_bInitAllocator;
	
};

#endif

