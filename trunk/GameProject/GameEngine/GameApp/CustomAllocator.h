///////////////////////////////////////////////////////////////////////  
//  CustomAllocator.h
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) CONFIDENTIAL AND PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization, Inc. and
//  may not be copied, disclosed, or exploited except in accordance with 
//  the terms of that agreement.
//
//      Copyright (c) 2003-2009 IDV, Inc.
//      All rights reserved in all media.
//
//      IDV, Inc.
//      http://www.idvinc.com


///////////////////////////////////////////////////////////////////////  
//  Preprocessor

#pragma once
#include <cstdlib>
#include <exception>
#include "EngineCore/Speedtree/Memory.h"


///////////////////////////////////////////////////////////////////////  
//  Overloaded global new

#ifdef SPEEDTREE_OVERRIDE_GLOBAL_NEW_AND_DELETE

void* operator new(size_t size)
{ 
    return malloc(size);
}


///////////////////////////////////////////////////////////////////////  
//  Overloaded global new[]

void* operator new[](size_t size)
{
    return malloc(size);
}


///////////////////////////////////////////////////////////////////////  
//  Overloaded global delete

void operator delete(void* p)
{
    if (p)
    {
        free(p);
    }
}


///////////////////////////////////////////////////////////////////////  
//  Overloaded global delete[]

void operator delete[](void* p)
{
    if (p)
    {
        free(p);
    }
}

#endif // SPEEDTREE_OVERRIDE_GLOBAL_NEW_AND_DELETE


///////////////////////////////////////////////////////////////////////  
//  class CCustomAllocator

#ifdef SPEEDTREE_USE_ALLOCATOR_INTERFACE

class CCustomAllocator : public SpeedTree::CAllocator
{
public:
    void* Alloc(size_t BlockSize)
    {
        return malloc(BlockSize);
    }

    void Free(void* pBlock)
    {
        if (pBlock)
            free(pBlock);
    }
};

#endif // SPEEDTREE_USE_ALLOCATOR_INTERFACE

