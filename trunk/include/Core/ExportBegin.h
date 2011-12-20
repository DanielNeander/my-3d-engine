///////////////////////////////////////////////////////////////////////  
//  ExportBegin.h
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

// storage-class specification
#if (defined(_WIN32) || defined(_XBOX)) && defined(SPEEDTREE_BUILD_SDK_AS_DLLS) && !defined(ST_STORAGE_CLASS)
    #define ST_STORAGE_CLASS __declspec(dllexport)
#else
    #define ST_STORAGE_CLASS
#endif

// inline function export control; ST_ILEX stands for "SpeedTree Inline Export"
#if (defined(_WIN32) || defined(_XBOX)) && !defined(SPEEDTREE_BUILD_SDK_AS_DLLS) && !defined(ST_ILEX)
    #define ST_ILEX __declspec(dllexport)
#else
    #define ST_ILEX
#endif

// specify calling convention
#if (defined(_WIN32) || defined(_XBOX)) && !defined(ST_CALL_CONV)
    #define ST_CALL_CONV   __cdecl
#else
    #define ST_CALL_CONV
#endif

// mac export control
#ifdef __APPLE__
    #pragma export on
#endif
