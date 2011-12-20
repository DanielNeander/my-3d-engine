///////////////////////////////////////////////////////////////////////  
//  Array.h
//
//  This file is part of IDV's lightweight STL-like container classes.
//  Like STL, these containers are templated and portable, but are thin
//  enough not to have some of the portability issues of STL nor will
//  it conflict with an application's similar libraries.  These containers
//  also contain some mechanisms for mitigating the inordinate number of
//  of heap allocations associated with the standard STL.
//
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement
//
//      Copyright (c) 2003-2009 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      http://www.idvinc.com
//
//  This file is part of IDV's lightweight STL-like container classes.
//  Like STL, these containers are templated and portable, but are thin
//  enough not to have some of the portability issues of STL nor will
//  it conflict with an application's similar libraries.  These containers
//  also contain some mechanisms for mitigating the inordinate number of
//  of heap allocations associated with the standard STL.
//
//  *** Release version 5.0 ***


/////////////////////////////////////////////////////////////////////
// Preprocessor

#pragma once
#include "Core/ExportBegin.h"
#include <cstdlib>
#include <cassert>
#include <typeinfo>
#ifndef SPEEDTREE_NO_ALLOCATORS
    #include "Core/Memory.h"
#endif
#ifdef __CELLOS_LV2__
    #include <string.h>
#else
    #include <cstring>
#endif


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    /////////////////////////////////////////////////////////////////////
    // class CArray

    template <class T, bool bUseCustomAllocator = true>
    class ST_STORAGE_CLASS CArray
    {
    public:
                                CArray(void);
                                CArray(size_t uiSize, const T& tDefault = T( ));
                                CArray(const CArray& cCopy);
    virtual                     ~CArray(void);                                          

            // standard functions
            void                clear(void);                                            
            bool                empty(void) const;
            size_t              size(void) const;                                       
            size_t              capacity(void) const;
            void                resize(size_t uiSize);
            void                resize(size_t uiSize, const T& tDefault);                           
            void                reserve(size_t uiSize);                         
            void                push_back(const T& tNew);                               
            void                pop_back(void);
            void                clip(void);
            void                erase_all(const T& tErase);
            T&                  front(void);
            T&                  back(void);
            T&                  at(size_t uiIndex);
            const T&            at(size_t uiIndex) const;
            T&                  operator [] (size_t uiIndex);   
            const T&            operator [] (size_t uiIndex) const;
            CArray&             operator = (const CArray& cRight);                      

            // sorting          
            template <class CompareFunctor>
            void                sort(const CompareFunctor& Compare, bool bMemorySwap = false);
            void                sort(bool bMemorySwap = false);
            
            // iterator access
            typedef T*          iterator;   
            typedef T const*    const_iterator; 
            iterator            begin(void);                                            
            iterator            end(void);
            const_iterator      begin(void) const;                              
            const_iterator      end(void) const;
            iterator            erase(iterator iterWhere);
            iterator            insert(iterator iterWhere, const T& tData);

            // these only work correctly on a sorted array
            iterator            lower(const T& tData);
            iterator            higher(const T& tData);
            void                lower_and_higher(const T& tData, iterator& iterLower, iterator& iterHigher);
            iterator            insert_sorted(const T& tData);
            iterator            insert_sorted_unique(const T& tData);
        
    protected:
            T*                  Allocate(size_t uiSize);
            void                Deallocate(T* pData);
            
            T*                  m_pData;                                                
            size_t              m_uiSize;                                               
            size_t              m_uiDataSize;
    };


    /////////////////////////////////////////////////////////////////////
    // CArray sort functors
    
    class CDefaultArraySort
    {
    public:
        template <class T> bool operator () (const T& tLeft, const T& tRight) const { return (tLeft < tRight); }
    };
    
    class CReverseArraySort
    {
    public:
        template <class T> bool operator () (const T& tLeft, const T& tRight) const { return (tRight < tLeft); }
    };


    // include inline functions
    #include "Core/Array.inl"

} // end namespace SpeedTree


#include "Core/ExportEnd.h"



