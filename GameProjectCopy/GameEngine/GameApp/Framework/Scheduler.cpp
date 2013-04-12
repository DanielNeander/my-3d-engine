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

#include "stdafx.h"
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

#include "EnvironmentManager.h"
#include "ServiceManager.h"
#include "Universal.h"
#include "PlatformManager.h"
#include "TaskManager.h"
#include "Scheduler.h"
#include "Instrumentation.h"

const f32 Scheduler::sm_DefaultClockFrequency = 1.0f / 120.0f;      // Set the timer to 120Hz

Scheduler::Scheduler( 
	TaskManager* pTaskManager )
    : m_pTaskManager( pTaskManager )
    , m_ClockFrequency( sm_DefaultClockFrequency )
    , m_hExecutionTimer( NULL )
    , m_bThreadingEnabled( True )
{
    m_hExecutionTimer = Singletons::PlatformManager.Timers().Create( m_ClockFrequency );

	m_bBenchmarkingEnabled =
		Singletons::EnvironmentManager.Variables().GetAsBool( "Scheduler::Benchmarking", False );
}


Scheduler::~Scheduler(
    void
    )
{
    Singletons::PlatformManager.Timers().Destroy( m_hExecutionTimer );
}


void
Scheduler::SetScene(
    const UScene* pScene
    )
{
    //
    // If we have a TaskManager, then we can thread things.
    //
	m_bThreadingEnabled = ( m_pTaskManager != NULL );
   
    //
    // Wait for any executing scenes to finish and clear out the list.
    //
	ISystemTask* aScenesToWaitFor[ System::Types::MAX ];
    u32 cScenesToWaitFor = 0;

    for ( SceneExecsIt it=m_SceneExecs.begin(); it != m_SceneExecs.end(); it++ )
    {
        ISystemScene* pSystemScene = it->second;

        ASSERT( cScenesToWaitFor < System::Types::MAX );
		aScenesToWaitFor[ cScenesToWaitFor++ ] = pSystemScene->GetSystemTask();
    }
    m_SceneExecs.clear();

    if ( cScenesToWaitFor > 0 )
    {
		m_pTaskManager->WaitForSystemTasks( aScenesToWaitFor, cScenesToWaitFor );
    }

    //
    // Copy over all the system scenes.
    //
    const UScene::SystemScenes& SystemScenes = pScene->GetSystemScenes();

    for ( UScene::SystemScenesConstIt it = SystemScenes.begin();
          it != SystemScenes.end(); it++ )
    {
        //
        // Make sure the system has a task.
        //
        if ( it->second->GetSystemTask() != NULL )
        {
            m_SceneExecs[ it->first ] = it->second;
        }
    }

    //
    // Re-create the timer as a scene load may have taken a long time.
    //
    Singletons::PlatformManager.Timers().Destroy( m_hExecutionTimer );
    m_hExecutionTimer = Singletons::PlatformManager.Timers().Create( m_ClockFrequency );
}


void
Scheduler::Execute(
    void
    )
{
    //
    // Get the delta time; seconds since last Execute call.
    //
    f32   DeltaTime = 0.0f;

	DeltaTime = Singletons::PlatformManager.Timers().Wait( m_hExecutionTimer, !m_bBenchmarkingEnabled );

    //
	// Update instrumentation for this frame.
    // If we do this here, there's no thread sync to worry about since we're single-threaded here.
    //
	Singletons::ServiceManager.Instrumentation().UpdatePeriodicData( DeltaTime );

    //
    // Check if the execution is paused, and set delta time to 0 if so.
    //
    if ( Singletons::EnvironmentManager.Runtime().GetStatus() ==
         IEnvironment::IRuntime::Status::Paused )
    {
        DeltaTime = 0.0f;
    }

    if ( m_bThreadingEnabled )
    {
        //
        // Schedule the scenes that are ready for execution.
        //
        ISystemTask* aScenesToExecute[ System::Types::MAX ];
        u32 cScenesToExecute = 0;

        for ( SceneExecsIt it=m_SceneExecs.begin(); it != m_SceneExecs.end(); it++ )
        {
            ASSERT( cScenesToExecute < System::Types::MAX );
            ISystemScene* pSystemScene = it->second;
		    aScenesToExecute[ cScenesToExecute++ ] = pSystemScene->GetSystemTask();
        }

        m_pTaskManager->IssueJobsForSystemTasks( aScenesToExecute, cScenesToExecute, DeltaTime );

        //
        // Wait for the scenes that will be completing execution in this frame.
        //
#if 0
        ISystemTask* aScenesToWaitFor[ System::Types::MAX ];
        u32 cScenesToWaitFor = 0;

        for ( SceneExecsIt it=m_SceneExecs.begin(); it != m_SceneExecs.end(); it++ )
        {
            ASSERT( cScenesToWaitFor < System::Types::MAX );
            ISystemScene* pSystemScene = it->second;
		    aScenesToWaitFor[ cScenesToWaitFor++ ] = pSystemScene->GetSystemTask();
        }

        m_pTaskManager->WaitForSystemTasks( aScenesToWaitFor, cScenesToWaitFor );
#endif /* 0 */
        m_pTaskManager->WaitForSystemTasks( aScenesToExecute, cScenesToExecute );
    }
    else
    {
        for ( SceneExecsIt it=m_SceneExecs.begin(); it != m_SceneExecs.end(); it++ )
        {
            ISystemScene* pSystemScene = it->second;
            pSystemScene->GetSystemTask()->Update( DeltaTime );
        }
    }
}
