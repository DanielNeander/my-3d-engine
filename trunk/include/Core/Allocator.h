///////////////////////////////////////////////////////////////////////  
//  Allocator.h
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
//
//  *** Release version 5.0 ***


///////////////////////////////////////////////////////////////////////  
//  Preprocessor

#pragma once
#include <cstdlib>
#include "Core/ExportBegin.h"


// enable to get a complete report of the heap allocations used by the SpeedTree
// SDK; the reference application write this file next to the executable
//#define SPEEDTREE_MEMORY_STATS


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    ///////////////////////////////////////////////////////////////////////  
    //  class CAllocator

    class ST_STORAGE_CLASS CAllocator
    {
    public:
    virtual             ~CAllocator( )      { }

    virtual void*       Alloc(size_t BlockSize) = 0;
    virtual void        Free(void* pBlock) = 0;

    static  void        TrackAlloc(const char* pDescription, void* pBlock, size_t sAmount);
    static  void        TrackFree(const char* pDescription, void* pBlock, size_t sAmount);
    static  bool        Report(const char* pFilename = 0, bool bFreeTrackingData = true);
    };


} // end namespace SpeedTree


#include "Core/ExportEnd.h"
