// Copyright © 2008-2009 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

#pragma once

//
// This disables warning 4127 which is a warning about using a constant in a conditional. This
// is used for __ITT_EVENT_START below for which systems are being monitored.
//
#pragma warning( push, 0 )
#pragma warning( disable : 4127 )

#ifdef PROFILE
#ifndef USE_THREAD_PROFILER
#define USE_THREAD_PROFILER
#endif
#endif

#if defined(USE_THREAD_PROFILER)

    #include <libittnotify.h>

    //
    // Set the MONITORING define to the desired combination of system flags to enable
    // user events in Thread Profiler for specific systems.
    //
	#define PROFILE_AI             0x01
	#define PROFILE_RENDER         0x02
	#define PROFILE_CHANGEOCCURRED 0x04
	#define PROFILE_TASKMANAGER    0x08
    #define PROFILE_CHANGECONTROL  0x10
	#define PROFILE_MESH           0x20
    #define PROFILE_PARTICLES      0x40
    #define PROFILE_ALL            0xFF
	#define MONITORING             (PROFILE_AI | PROFILE_RENDER)

    #define __ITT_EVENT_START(evt, system) if (system & MONITORING) { __itt_event_start(evt); } else { (void)0; }
    #define __ITT_EVENT_END(evt, system)   if (system & MONITORING) { __itt_event_end(evt); } else { (void)0; }

    #define __ITT_DEFINE_STATIC_EVENT(evt, name, len) static __itt_event evt = __itt_event_createA( name, len )
    #define __ITT_CREATE_EVENT(evt, name, len) evt = __itt_event_createA( name, len )

#else
    #define __ITT_EVENT_START(evt, system) ((void)0)
    #define __ITT_EVENT_END(evt, system) ((void)0)

    #define __ITT_DEFINE_STATIC_EVENT(evt, name, len)
    #define __ITT_CREATE_EVENT(evt, name, len) ((void)0)
#endif
