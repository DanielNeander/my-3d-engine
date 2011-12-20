/************************************************************************
module  : U2StandardAllocator
Author  : yun sangyong
Desc	: Gamebryo 2.6 메모리 매니저 
************************************************************************/
#pragma once 
#ifndef U2_STANDARDALLOCATOR_H
#define U2_STANDARDALLOCATOR_H

#include "U2DefaultAllocator.h"
#include "U2SmallObjAllocStrategy.h"

typedef U2SmallObjAllocStrategy<U2DefaultAllocator> 
	U2StandardAllocator;


#endif 