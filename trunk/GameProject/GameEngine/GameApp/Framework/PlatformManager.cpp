// Copyright ?2008-2009 Intel Corporation
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

// Prevent other headers from including any platform specific ones
#define __PLATFORM_MANAGER

#include "stdafx.h"
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

#include "PlatformManager.h"
#include "SystemManager.h"
#include "EnvironmentManager.h"
#include "ServiceManager.h"
#include "TaskManager.h"

#include "..\Geometry\\GeomSystem.h"
#include "..\GraphicSystem.h"
#include "..\ProceduralFire\FireSystem.h"
#include "..\ProceduralTrees\TreeSystem.h"
#include "..\Input\InputSystem.h"



extern TaskManager*     g_pTaskManager;

PlatformManager::PlatformManager(
    void
    )
{
}


PlatformManager::~PlatformManager(
    void
    )
{
}


#if defined ( WIN32 ) || defined ( WIN64 )
namespace Windows
{
    #define _WIN32_WINNT	0x0400 
    #include <windows.h>
}


PlatformManager::FileSystem::FileSystem(
    void
    )
{
}


PlatformManager::FileSystem::~FileSystem(
    void
    )
{
    //
    // Iterate through all the loaded libraries.
    //
    std::vector<SystemLib>::iterator it;
    for ( it=m_SystemLibs.begin(); it!=m_SystemLibs.end(); it++ )
    {
        //::HMODULE hLib = reinterpret_cast<::HMODULE>(it->hLib);

        ////
        //// Get the system destruction function.
        ////
        //DestroySystemFunction fnDestroySystem =
        //    reinterpret_cast<DestroySystemFunction>(
        //        ::GetProcAddress( hLib, "DestroySystem" )
        //        );

        //if ( fnDestroySystem != NULL )
        //{
        //    fnDestroySystem( it->pSystem );
        //}

        //::FreeLibrary( hLib );
			SAFE_DELETE(it->pSystem);
    }

    m_SystemLibs.clear();
}

Error
	PlatformManager::FileSystem::LoadSystem(
	const char* pszSysLib,
	ISystem** ppSystem
	)
{
	Error   Err = Errors::Failure;

	//
	// Load the dll.
	//	
	{
		//
		// Get the system initialization function.
		//
						
		
			ManagerInterfaces Managers;
			Managers.pEnvironment = &Singletons::EnvironmentManager;
			Managers.pService     = &Singletons::ServiceManager;
			Managers.pTask        = g_pTaskManager;

			extern ManagerInterfaces g_FireManagers;			
			extern ManagerInterfaces g_InputManagers;
			extern ManagerInterfaces g_Managers;
						
			idStr libname = pszSysLib;
			if (libname == "SystemProceduralFire") 
				g_FireManagers = Managers;			
			else if (libname == "SystemInput") 
				g_InputManagers = Managers;
			else if (libname == "SystemGraphics")
				g_GraphicsManagers = Managers;							
					
		{
			//
			// Create the system.
			//
			ISystem* pSystem = NULL;
			if (libname == "SystemGeometry") 
				pSystem = new GeometrySystem();
			if (libname == "SystemProceduralFire") 
				pSystem = new FireSystem();
			else if (libname == "SystemProceduralTrees") 
				pSystem = new TreeSystem();
			else if (libname == "SystemInput") 
				pSystem = new InputSystem();
			else if (libname == "SystemGraphics")
				pSystem = new GraphicsSystem();


			if ( pSystem != NULL )
			{
				//
				// Verify that there's no duplicate system type.
				//
				System::Type SystemType = pSystem->GetSystemType();

				ISystem* pCurrSystem =
					Singletons::SystemManager.Get( SystemType );

				if ( pCurrSystem == NULL )
				{
					//
					// Add the system to the collection.
					//
					Singletons::SystemManager.Add( pSystem );

					//SystemLib sl = { reinterpret_cast<Handle>(hLib), pSystem };
					SystemLib sl = { std::string(pszSysLib), pSystem };
					m_SystemLibs.push_back( sl );

					*ppSystem = pSystem;
				}
			}
		}
	}

	return Err;
}

Error
PlatformManager::FileSystem::LoadSystemLibrary(
    const char* pszSysLib,
    ISystem** ppSystem
    )
{
    Error   Err = Errors::Failure;

    //
    // Load the dll.
    //
    ::HMODULE hLib = ::LoadLibraryA( pszSysLib );

    if ( hLib != NULL )
    {
        //
        // Get the system initialization function.
        //
        InitializeSystemLibFunction fnInitSystemLib =
            reinterpret_cast<InitializeSystemLibFunction>(
                ::GetProcAddress( hLib, "InitializeSystemLib" )
                );

        if ( fnInitSystemLib != NULL )
        {
            ManagerInterfaces Managers;
            Managers.pEnvironment = &Singletons::EnvironmentManager;
            Managers.pService     = &Singletons::ServiceManager;
            Managers.pTask        = g_pTaskManager;

            fnInitSystemLib( &Managers );
        }

        //
        // Get the system creation function.
        //
        CreateSystemFunction fnCreateSystem =
            reinterpret_cast<CreateSystemFunction>(
                ::GetProcAddress( hLib, "CreateSystem" )
                );

        if ( fnCreateSystem != NULL )
        {
            //
            // Create the system.
            //
			ISystem* pSystem = fnCreateSystem( Debug::GetDebugger() );

            if ( pSystem != NULL )
            {
                //
                // Verify that there's no duplicate system type.
                //
                System::Type SystemType = pSystem->GetSystemType();

                ISystem* pCurrSystem =
                    Singletons::SystemManager.Get( SystemType );

                if ( pCurrSystem == NULL )
                {
                    //
                    // Add the system to the collection.
                    //
                    Singletons::SystemManager.Add( pSystem );

                    //SystemLib sl = { reinterpret_cast<Handle>(hLib), pSystem };
                    //m_SystemLibs.push_back( sl );

                    *ppSystem = pSystem;
                }
            }
        }
    }

    return Err;
}


Bool
PlatformManager::FileSystem::FileExists(
    In pcstr pszFileName
    )
{
    Bool bFound = False;

    //
    // Open the file for read access.
    //
    ::HANDLE hFile = ::CreateFileA( pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

    {
        using namespace Windows;

        if ( hFile != INVALID_HANDLE_VALUE )
        {
            //
            // Close it since we're just checking if it exists.
            //
            ::CloseHandle( hFile );

            bFound = True;
        }
    }

    return bFound;
}


Bool
PlatformManager::FileSystem::SetCurrentDirToFileLocation(
    In pcstr pszFileName,
    In pcstr apszLocations[],
    Out pstr pszCurrentDir,
    u32 BufferSize
    )
{
    Bool bDirectorySet = False;

    //
    // Check the current directory.
    //
    if ( !FileExists( pszFileName ) )
    {
        //
        // Save the original current directory.
        //
        char szCurrentDir[ _MAX_PATH ];
        ::GetCurrentDirectoryA( _MAX_PATH, szCurrentDir );

        for ( int i=0; apszLocations[ i ] != NULL; i++ )
        {
            //
            // Switch to the directory.
            //
            if ( ::SetCurrentDirectoryA( apszLocations[ i ] ) )
            {
                //
                // If it worked checked for file existence.
                //
                if ( FileExists( pszFileName ) )
                {
                    //
                    // We found it.  The break will keep the original directory from being reset.
                    //
                    bDirectorySet = True;
                    break;
                }
            }

            //
            // Restore the original directory for the next loop iteration.
            //
            ::SetCurrentDirectoryA( szCurrentDir );
        }
    }
    else
    {
        bDirectorySet = True;
    }

    //
    // Return the just set current directory if the user requested it.
    //
    if ( bDirectorySet && pszCurrentDir != NULL && BufferSize > 0 )
    {
        ::GetCurrentDirectoryA( BufferSize, pszCurrentDir );
    }

    return bDirectorySet;
}


PlatformManager::WindowSystem::WindowSystem(
    void
    )
{
}


void
PlatformManager::WindowSystem::ProcessMessages(
    void
    )
{
    //
    // Process all messages in the queue.
    //
    ::MSG    Msg;

    while ( ::PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) )
    {
        ::TranslateMessage( &Msg );
        ::DispatchMessage( &Msg );
    }
}


struct WindowsTimerData
{
    ::HANDLE			hTimer;
    ::LARGE_INTEGER	Interval;
    ::LARGE_INTEGER  PreviousTick;
};

static f32                  sm_PerfCounterFrequency;

PlatformManager::Timers::Timers(
    void
    )
{
    ::LARGE_INTEGER Frequency;
	::QueryPerformanceFrequency( &Frequency );

    sm_PerfCounterFrequency = static_cast<f32>(Frequency.QuadPart);
}


f32
PlatformManager::Timers::GetGranularity(
    void
    )
{
    // Use ::GetSystemTimeAdjustment
    return static_cast<f32>(1000.0f) / 1000000.0f;		// Set to 1 ms
}


Handle
PlatformManager::Timers::Create(
    f32 Interval
    )
{
    WindowsTimerData* pTimerData = new WindowsTimerData;
    ASSERT( pTimerData != NULL );

    //
    // Create the timer, granularity is in 100ns steps (but timer accuracy is system dependent).
    //
    pTimerData->hTimer = ::CreateWaitableTimer( NULL, TRUE, NULL );
    pTimerData->Interval.QuadPart = static_cast<::ULONGLONG>(static_cast<double>(Interval) * -10000000.0);
    ::QueryPerformanceCounter( &pTimerData->PreviousTick );

    //
    // Start the timer.
    //
    ::SetWaitableTimer( pTimerData->hTimer, &pTimerData->Interval, 0, NULL, NULL, 0 );

    return reinterpret_cast<Handle>(pTimerData);
}


void
PlatformManager::Timers::Destroy(
    Handle hTimer
    )
{
    WindowsTimerData* pTimerData = reinterpret_cast<WindowsTimerData*>(hTimer);

    ::CloseHandle( pTimerData->hTimer );

    SAFE_DELETE( pTimerData );
}


f32
PlatformManager::Timers::Wait(
    Handle hTimer,
	Bool bWait
    )
{
    WindowsTimerData* pTimerData = reinterpret_cast<WindowsTimerData*>(hTimer);

    //
    // Wait for the timer to expire, then start it up again.
    //
	if ( bWait )
	{
		::WaitForSingleObject( pTimerData->hTimer, INFINITE );
		::SetWaitableTimer( pTimerData->hTimer, &pTimerData->Interval, 0, NULL, NULL, 0 );
	}

	//
	// Calculate the time delta in seconds. We are using the performance counter which varies by system.
	// The performance frequency is the number of performance counts per second.
	//
	::LARGE_INTEGER CurrentTick;
	::QueryPerformanceCounter(&CurrentTick);

	f32 DeltaTime = (f32)(CurrentTick.QuadPart - pTimerData->PreviousTick.QuadPart) /
                    sm_PerfCounterFrequency;

	ASSERT( DeltaTime >= static_cast<f32>(pTimerData->Interval.QuadPart) );

	pTimerData->PreviousTick = CurrentTick;

    return DeltaTime;
}


PlatformManager::Debugging::Debugging(
    void
    )
{
}
    
void
PlatformManager::Debugging::OutputMessage(
    pcstr pszMessage
    )
{
    ::OutputDebugStringA( pszMessage );
}


PlatformManager::Processor::Processor(
    void
    )
{
    //
    // Get the processor affinities for this process.
    //
    ::DWORD_PTR ProcessAffinityMask;
    ::DWORD_PTR SystemAffinityMask;

    ::GetProcessAffinityMask(
        ::GetCurrentProcess(), &ProcessAffinityMask, &SystemAffinityMask
        );

    //
    // Create a mask index for the processors.
    //
    ::SYSTEM_INFO si;
    ::GetSystemInfo( &si );

    m_ProcessorMasks.reserve( si.dwNumberOfProcessors );

	// Make sure we can use all processors.
	if ( SystemAffinityMask != ProcessAffinityMask ) {
		::MessageBox( NULL,
			TEXT ( "The Smoke process has been restricted, so it cannot use all processors in the system." ),
			TEXT ( "Smoke startup warning" ),
			MB_OK );
	}

    for ( u32 i=0; ProcessAffinityMask > 0; i++, ProcessAffinityMask >>= 1 )
    {
        if ( ProcessAffinityMask & 1 )
        {
            m_ProcessorMasks.push_back( i );
        }
    }
    ASSERT( m_ProcessorMasks.size() <= si.dwNumberOfProcessors );
}


u32
PlatformManager::Processor::GetNumProcessors(
    void
    )
{
    return static_cast<u32>(m_ProcessorMasks.size());
}


void
PlatformManager::Processor::AffinitizeThreadToProcessor(
    u32 ProcessorNumber
    )
{
    ASSERT( ProcessorNumber < m_ProcessorMasks.size() );

    //
    // Set the preferred processor for this thread.
    //
    ::SetThreadIdealProcessor( ::GetCurrentThread(),
                                      m_ProcessorMasks[ ProcessorNumber ] );
}
#endif
