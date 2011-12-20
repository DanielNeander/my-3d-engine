///////////////////////////////////////////////////////////////////////  
//  SpeedTreeTimer.h
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
//  *** Release version 5.0 ***


///////////////////////////////////////////////////////////////////////
// Preprocessor

#pragma once
#ifdef WIN32
    #ifndef _WINDOWS_
        #include <windows.h>
    #endif
#endif
#ifdef _XBOX
    #include <Xtl.h>
#endif
#ifdef __CELLOS_LV2__
    #include <sys/sys_time.h>
#endif
#include "Core/ExportBegin.h"


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    ///////////////////////////////////////////////////////////////////////  
    //  class CTimer
    /// CTimer is used like a stopwatch timer and can return the time in various units.

    class ST_STORAGE_CLASS CTimer
    {
    public:
                                CTimer(bool bStart = false);

            float               Stop(void) const;
            void                Start(void);

            float               GetSec(void) const;
            float               GetMilliSec(void) const;
            float               GetMicroSec(void) const;

    private:
    #if defined(WIN32) || defined(_XBOX)
            LARGE_INTEGER       m_liFreq;
            LARGE_INTEGER       m_liStart;
    mutable LARGE_INTEGER       m_liStop;
    #endif
    #ifdef __CELLOS_LV2__
            sys_time_sec_t      m_tStartSeconds;
            sys_time_nsec_t     m_tStartNanoseconds;
    mutable sys_time_sec_t      m_tStopSeconds;
    mutable sys_time_nsec_t     m_tStopNanoseconds;
    #endif
    };


    ///////////////////////////////////////////////////////////////////////  
    //  CTimer::CTimer
    /// CTimer constructor. If \a bStart is true, the timer will start by calling Start() internally.

    inline CTimer::CTimer(bool bStart)
    {
#if defined(WIN32) || defined(_XBOX)
        (void) QueryPerformanceFrequency(&m_liFreq);
        m_liStart.QuadPart = 0;
        m_liStop.QuadPart = 0;
#endif

        if (bStart)
            Start( );
    }


    ///////////////////////////////////////////////////////////////////////  
    //  CTimer::Start
    /// Starts the timer

    inline void CTimer::Start(void)
    {
#if defined(WIN32) || defined(_XBOX)
        m_liStart.QuadPart = 0;
        QueryPerformanceCounter(&m_liStart);
#endif
#ifdef __CELLOS_LV2__
        sys_time_get_current_time(&m_tStartSeconds, &m_tStartNanoseconds);
#endif
    }


    ///////////////////////////////////////////////////////////////////////  
    //  CTimer::Stop
    /// Stops the timer and returns the milliseconds elapsed since it was started.

    inline float CTimer::Stop(void) const
    {
#if defined(WIN32) || defined(_XBOX)
        QueryPerformanceCounter(&m_liStop);
#endif 
#ifdef __CELLOS_LV2__
        sys_time_get_current_time(&m_tStopSeconds, &m_tStopNanoseconds);
#endif

        return GetMilliSec( );
    }


    ///////////////////////////////////////////////////////////////////////  
    //  CTimer::GetMicroSec
    /// Returns the elapsed microseconds between the last Start() and Stop()

    inline float CTimer::GetMicroSec(void) const
    {
        return GetSec( ) * 1.0e6f;
    }


    ///////////////////////////////////////////////////////////////////////  
    //  CTimer::GetMilliSec
    /// Returns the elapsed milliseconds between the last Start() and Stop()

    inline float CTimer::GetMilliSec(void) const
    {
        return GetSec( ) * 1.0e3f;
    }


    ///////////////////////////////////////////////////////////////////////  
    //  CTimer::GetSec
    /// Returns the elapsed seconds between the last Start() and Stop()

    inline float CTimer::GetSec(void) const
    {
#if defined(WIN32) || defined(_XBOX)
        return (m_liStop.QuadPart - m_liStart.QuadPart) / float(m_liFreq.QuadPart);
#endif 
#ifdef __CELLOS_LV2__
        return (m_tStopSeconds - m_tStartSeconds) + (m_tStopNanoseconds - m_tStartNanoseconds) / 1.0e9f;
#endif
    }

} // end namespace SpeedTree

