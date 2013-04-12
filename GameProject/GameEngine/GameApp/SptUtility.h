///////////////////////////////////////////////////////////////////////  
//  Utility.h
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
#include <cstdio>
#include <cstdarg>
#ifdef SPEEDTREE_OPENGL
    #include "glew/glew.h"
    #include "Cg/cgGL.h"
#endif
#if defined(_WIN32) && !defined(_XBOX)
    #include <windows.h>
    #pragma warning (disable : 4996)
#endif
#ifdef _XBOX
    #include <Xtl.h>
#endif
#ifdef __CELLOS_LV2__
    #include <dirent.h>
#endif
#include "EngineCore/Speedtree/Core.h"
#include "EngineCore/Speedtree/String.h"


////////////////////////////////////////////////////////////
// UnreferencedParameter

#ifdef __CELLOS_LV2__
    #define UnreferencedParameter(x) (x) = (x)
#else
    #define UnreferencedParameter(x) (x)
#endif


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    ////////////////////////////////////////////////////////////
    // Interpolate

    template <class T> inline T Interpolate(const T& tStart, const T& tEnd, float fPercent)
    {
        return static_cast<T>((tStart + (tEnd - tStart) * fPercent));
    }


    ///////////////////////////////////////////////////////////////////////  
    //  FileExists

    inline bool FileExists(const char* pFilename)
    {
        FILE* pFile = fopen(pFilename, "r");
        bool bExists = (pFile != NULL);
        if (pFile)
            fclose(pFile);

        return bExists;
    }


    /////////////////////////////////////////////////////////////////////////////
    //  ColorToUInt

    inline unsigned int ColorToUInt(float fR, float fG, float fB, float fA = 1.0f)
    {
        unsigned int uiRed = unsigned int(fR * 255.0f);
        unsigned int uiGreen = unsigned int(fG * 255.0f);
        unsigned int uiBlue = unsigned int(fB * 255.0f);
        unsigned int uiAlpha = unsigned int(fA * 255.0f);

        return ((uiRed << 0) + (uiGreen << 8) + (uiBlue << 16) + (uiAlpha << 24));
    }


    /////////////////////////////////////////////////////////////////////////////
    //  ColorToUInt

    inline unsigned int ColorToUInt(const float afColor[4])
    {
        return ColorToUInt(afColor[0], afColor[1], afColor[2], afColor[3]);
    }


    /////////////////////////////////////////////////////////////////////////////
    //  ColorToFloats

    inline void ColorToFloats(unsigned int uiColor, float afColor[4])
    {
        afColor[0] = ((uiColor & 0x000000FF) >> 0) / 255.0f;
        afColor[1] = ((uiColor & 0x0000FF00) >> 8) / 255.0f;
        afColor[2] = ((uiColor & 0x00FF0000) >> 16) / 255.0f;
        afColor[3] = ((uiColor & 0xFF000000) >> 24) / 255.0f;
    }


    ////////////////////////////////////////////////////////////
    // Clamp

    template <class T> inline T Clamp(T tValue, T tMinValue, T tMaxValue)
    {
        T tReturn = st_min(tValue, tMaxValue);

        return st_max(tReturn, tMinValue);
    }


    ////////////////////////////////////////////////////////////
    // Swap

    template <class T> inline void Swap(T& tA, T& tB)
    {
        T tTemp = tA;
        tA = tB; 
        tB = tTemp;
    }


    ///////////////////////////////////////////////////////////////////////
    //  PrintSpeedTreeError

    inline void PrintSpeedTreeErrors(const char* pLocation = NULL)
    {
        const char* pError = SpeedTree::CCore::GetError( );
        while (pError)
        {
            if (pLocation)
                printf("SpeedTree Error (%s): %s", pLocation, pError);
            else
                printf("SpeedTree Error: %s", pError);

            pError = SpeedTree::CCore::GetError( );
        }
    }


    ///////////////////////////////////////////////////////////////////////  
    //  Strcmpi 

    inline bool Strcmpi(const SpeedTree::CString& strA, const SpeedTree::CString& strB)
    {
#ifdef __CELLOS_LV2__
        return strcasecmp(strA.c_str( ), strB.c_str( )) == 0;
#else
        return _strcmpi(strA.c_str( ), strB.c_str( )) == 0;
#endif
    }

} // end namespace SpeedTree
