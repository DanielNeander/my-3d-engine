///////////////////////////////////////////////////////////////////////  
//  Memory.h
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

#include "Allocator.h"
#include <new>

#ifdef _XBOX
    #include <malloc.h>
#endif
#ifdef __CELLOS_LV2__
    #include <stdlib.h>
#endif


///////////////////////////////////////////////////////////////////////  
//  Packing

#ifdef _WIN32 // Windows or 360
    #pragma warning(push)
    #pragma warning(disable : 4103)
    #pragma pack(push, 8)   
#endif


namespace SpeedTree
{
    #ifdef SPEEDTREE_ALLOCATORS_DECLARED
        ST_STORAGE_CLASS SpeedTree::CAllocator *g_pAllocator = NULL;
        ST_STORAGE_CLASS size_t g_sHeapMemoryUsed = 0;
    #else // SPEEDTREE_ALLOCATORS_DECLARED
        #ifdef SPEEDTREE_USE_SDK_AS_DLLS
            __declspec(dllimport) SpeedTree::CAllocator* g_pAllocator;
            __declspec(dllimport) size_t g_sHeapMemoryUsed;
        #else
            extern SpeedTree::CAllocator* g_pAllocator;
            extern size_t g_sHeapMemoryUsed;
        #endif
    #endif // SPEEDTREE_ALLOCATORS_DECLARED
}


///////////////////////////////////////////////////////////////////////
//  struct SHeapHandle
//
//  Necessary overhead for overriding array allocation.

namespace SpeedTree
{
    struct SHeapHandle
    {
        size_t  m_sNumElements;
    };
}


///////////////////////////////////////////////////////////////////////
//  placement new

inline void* operator new(size_t sBlockSize, const char* pDescription)
{
    void* pBlock = SpeedTree::g_pAllocator ? SpeedTree::g_pAllocator->Alloc(sBlockSize) : malloc(sBlockSize);

    if (pBlock)
    {
        // track all SpeedTree memory allocation
        SpeedTree::g_sHeapMemoryUsed += sBlockSize;
        #ifdef SPEEDTREE_MEMORY_STATS
            SpeedTree::CAllocator::TrackAlloc(pDescription ? pDescription : "Unknown", pBlock, sBlockSize);
        #else
            pDescription = pDescription; // avoid unreferenced parameter warning
        #endif
    }

    return pBlock;
}


///////////////////////////////////////////////////////////////////////
//  must have a delete call to match the placement new

inline void operator delete(void* pBlock, const char*)
{
    if (pBlock)
    {
        if (SpeedTree::g_pAllocator)
            SpeedTree::g_pAllocator->Free(pBlock);
        else
            free(pBlock);
    }
}


///////////////////////////////////////////////////////////////////////
//  st_new_array (equivalent of C++ new[])

template <typename TYPE> inline TYPE* st_new_array(size_t sNumElements, const char* pDescription)
{
    size_t sTotalSize = sizeof(SpeedTree::SHeapHandle) + sNumElements * sizeof(TYPE);

    char* pRawBlock = (char*) (SpeedTree::g_pAllocator ? SpeedTree::g_pAllocator->Alloc(sTotalSize) : malloc(sTotalSize));
    if (pRawBlock)
    {
        // get the main part of the data, after the heap handle
        TYPE* pBlock = (TYPE*) (pRawBlock + sizeof(SpeedTree::SHeapHandle));

        // record the number of elements
        *((size_t*) pRawBlock) = sNumElements;

        // manually call the constructors using placement new
        TYPE* pElements = pBlock;
        for (size_t i = 0; i < sNumElements; ++i)
            (void) new (static_cast<void*>(pElements + i)) TYPE;

        // track all SpeedTree memory allocation
        SpeedTree::g_sHeapMemoryUsed += sTotalSize;
#ifdef SPEEDTREE_MEMORY_STATS
        SpeedTree::CAllocator::TrackAlloc(pDescription ? pDescription : "Unknown", pRawBlock, sTotalSize);
#else
        pDescription = pDescription; // avoid unreferenced parameter warning
#endif

        return pBlock;
    }
    
    return NULL;
}


///////////////////////////////////////////////////////////////////////
//  st_delete (equivalent of C++ delete)

template <typename TYPE> inline void st_delete(TYPE*& pBlock, const char* pDescription)
{
#ifdef SPEEDTREE_MEMORY_STATS
    if (pBlock)
        SpeedTree::CAllocator::TrackFree(pDescription ? pDescription : "Unknown", pBlock, pBlock ? sizeof(TYPE) : 0);
#else
    pDescription = pDescription; // avoid unreferenced parameter warning
#endif

    if (pBlock)
    {
        pBlock->~TYPE( );
        if (SpeedTree::g_pAllocator)
            SpeedTree::g_pAllocator->Free(pBlock);
        else
            free(pBlock);
        pBlock = NULL;

        // track all SpeedTree memory allocation
        SpeedTree::g_sHeapMemoryUsed -= sizeof(TYPE);
    }
}


///////////////////////////////////////////////////////////////////////
//  st_delete_array (equivalent of C++ delete[])

template <typename TYPE> inline void st_delete_array(TYPE*& pRawBlock, const char* pDescription)
{
    if (pRawBlock)
    {
        // extract the array size
        SpeedTree::SHeapHandle* pHeapHandle = (SpeedTree::SHeapHandle*) ( ((char*) pRawBlock) - sizeof(SpeedTree::SHeapHandle) );

        if (pHeapHandle)
        {
            // point to the elements
            TYPE* pElements = pRawBlock; 

            if (pElements)
            {
                // track all SpeedTree memory allocation
                SpeedTree::g_sHeapMemoryUsed -= sizeof(SpeedTree::SHeapHandle) + pHeapHandle->m_sNumElements * sizeof(TYPE);

                for (size_t i = 0; i < pHeapHandle->m_sNumElements; ++i)
                    (pElements + i)->~TYPE( );

#ifdef SPEEDTREE_MEMORY_STATS
                SpeedTree::CAllocator::TrackFree(pDescription ? pDescription : "Unknown", pHeapHandle, pHeapHandle->m_sNumElements * sizeof(TYPE) + sizeof(SpeedTree::SHeapHandle));
#else
                pDescription = pDescription; // avoid unreferenced parameter warning
#endif
                if (SpeedTree::g_pAllocator)
                    SpeedTree::g_pAllocator->Free(pHeapHandle);
                else
                    free(pHeapHandle);
                pRawBlock = NULL;
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  Packing

#ifdef _WIN32
    #pragma pack(pop)   
    #pragma warning(pop)    
#endif
