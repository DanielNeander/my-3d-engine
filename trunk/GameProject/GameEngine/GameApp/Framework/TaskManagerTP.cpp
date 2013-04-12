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
//#include <Windows.h>

#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

#include "EnvironmentManager.h"
#include "ServiceManager.h"
#include "PlatformManager.h"

#include "ThreadPool.h"
#include "TaskManager.h"
#include "TaskManagerTP.h"

#include "Instrumentation.h"


void
SystemTaskWrapper::Process(
    void
    )
{
    // Call the function, and figure out how long it took.
	i64 counter = __rdtsc();
    pSystemTask->Update( deltaTime );
	counter = __rdtsc() - counter;

	// Log this job's time in instrumentation.
	Singletons::Instrumentation.CaptureJobCounterTicks( pSystemTask->GetSystemType(), counter );
}


void
SystemTaskWrapper::SetStatus(
    WorkItem::Status inStatus
    )
{
    WorkItem::SetStatus( inStatus );

    if( inStatus == WorkItem::Status::Completed )
    {
        if( pfnJobCompletionFunction != NULL )
        {
            pfnJobCompletionFunction( pCompletionUserData );
		}
		
		SetEvent( *hWorkCompleted );
    }
}


void
SystemParallelForWrapper::Process(
    void
    )
{
    // Call the function, and figure out how long it took.
	i64 counter = __rdtsc();
    pfnParallelForFunction( pUserData, uStart, uEnd );
	counter = __rdtsc() - counter;

	if( pParentTask != NULL )
	{
		// Log this job's time in instrumentation.
		Singletons::Instrumentation.CaptureJobCounterTicks( pParentTask->pSystemTask->GetSystemType(), counter );
	}

	// decrement the job wait counter
	::InterlockedDecrementRelease( lJobWaitCount );
}


void
TaskManagerTP::Init(
    void
    )
{
    m_cActiveSystemTasksInQ = 0;
    m_pThreadPool = new BasicThreadPool();
    ASSERT( m_pThreadPool != NULL );

    u32 uNumberOfThreads = Singletons::EnvironmentManager.Variables().GetAsInt( "TaskManager::Threads", 0 );
	if( uNumberOfThreads <= 0 ) 
    {
        uNumberOfThreads = Singletons::PlatformManager.Processor().GetNumProcessors();
    } 
    m_pThreadPool->Initialize( uNumberOfThreads - 1 );
	// Cache the thread count for display.
	Singletons::Instrumentation.setActiveThreadCount( uNumberOfThreads );

    for( u32 i=0; i < System::Types::MAX; i++ )
    {
        m_hSystemTaskEvents[i] = CreateEvent( NULL, False, False, NULL );
    }
}


void
TaskManagerTP::Shutdown(
    void
    )
{
    SAFE_DELETE( m_pThreadPool );	

    for( u32 i=0; i < System::Types::MAX; i++ )
    {
        CloseHandle( m_hSystemTaskEvents[ i ] );
    }
}


void
TaskManagerTP::IssueJobsForSystemTasks(
    ISystemTask** pTasks,
    u32 uCount,
    f32 DeltaTime
    )
{
    m_cActiveSystemTasksInQ = uCount;

    for ( u32 i=0; i < uCount; i++ )
    {
		u32 iTaskQ = System::Types::GetIndex( pTasks[ i ]->GetSystemType() );

        m_TaskQ[ iTaskQ ].hWorkCompleted			= &m_hSystemTaskEvents[ iTaskQ ];
        m_TaskQ[ iTaskQ ].pSystemTask				= pTasks[ i ];
        m_TaskQ[ iTaskQ ].deltaTime					= DeltaTime;
        m_TaskQ[ iTaskQ ].pfnJobCompletionFunction	= NULL;
		m_TaskQ[ iTaskQ ].pCompletionUserData		= NULL;

        m_pThreadPool->QueueUserWorkItem( &m_TaskQ[ iTaskQ ] );

        m_hActiveSystemTasksInQ[ i ] = m_hSystemTaskEvents[ iTaskQ ];
	}
}


void TaskManagerTP::WaitForAllSystemTasks( void )
{
	// do some work while we're waiting
    while( WaitForMultipleObjects( m_cActiveSystemTasksInQ, m_hActiveSystemTasksInQ, TRUE, 0 ) != WAIT_OBJECT_0 )
	{
		// pull something off of the job queue and execute it
		m_pThreadPool->PopAndProcessWorkItem();
	}
	
	m_cActiveSystemTasksInQ = 0;

    for( std::list<SystemParallelForWrapper*>::iterator it = m_Jobs.begin(); it != m_Jobs.end(); it++ )
    {
        delete *it;
    }
    m_Jobs.clear();
}


void TaskManagerTP::WaitForSystemTasks( ISystemTask** pTasks, u32 uCount )
{
    // NOTE: not properly implemented
    // currently waits for all tasks
    DBG_UNREFERENCED_PARAM( pTasks );
    DBG_UNREFERENCED_PARAM( uCount );
    WaitForAllSystemTasks();
}


void
TaskManagerTP::NonStandardPerThreadCallback(
    JobFunction pfnCallback,
    void* pData
    )
{
	m_pThreadPool->NonStandardThreadExecute( pfnCallback, pData );

	// call it for ourself, too
	pfnCallback( pData );
}


u32
TaskManagerTP::GetRecommendedJobCount(
    ITaskManager::JobCountInstructionHints Hints
    )
{
    //
    // Ignoring hints for now and just returning the number of available threads.
    //
    UNREFERENCED_PARAM( Hints );

    return m_pThreadPool->GetThreadCount();
}


u32
TaskManagerTP::GetNumberOfThreads(
	void)
{
	return m_pThreadPool->GetActiveThreadCount();
}


void
TaskManagerTP::SetNumberOfThreads(
	u32 uNumberOfThreads
)
{
	m_pThreadPool->SetActiveThreadCount( uNumberOfThreads - 1 );
	// Cache the thread count for display.
	Singletons::Instrumentation.setActiveThreadCount( uNumberOfThreads );
}


void 
TaskManagerTP::ParallelFor(
    ISystemTask* pSystemTask,
    ParallelForFunction pfnJobFunction, 
    void* pParam, 
    u32 begin, 
    u32 end, 
    u32 minGrainSize
	)
{
	u32 uThreads = ( m_pThreadPool->GetActiveThreadCount() + 1 );
	u32 uCount = ( end - begin );
	
	ASSERT( uThreads > 0 );
	ASSERT( uCount >= 0 );
	
	u32 uGrainSize = ( uCount / uThreads );

	if( uGrainSize < minGrainSize )
	{
		uGrainSize = minGrainSize;
		ASSERT( uGrainSize > 0 );

		uThreads = ( uCount / uGrainSize );
	}

	if( uThreads > 1 )
	{
		// this job count will be used to track completion of jobs put into the pool
		// using InterlockedDecrementRelease
		volatile long lJobWaitCount = ( uThreads - 1 );

		u32 uStart = 0;
		u32 uEnd = 0;
		
		SystemTaskWrapper *pParentTask = NULL;
		if( pSystemTask != NULL )
		{
			u32 iTaskQ = System::Types::GetIndex( pSystemTask->GetSystemType() );
			pParentTask = &m_TaskQ[ iTaskQ ];
		}
			
		// dispatch other jobs
		for( u32 t = 1; t < uThreads; t++ )
		{
			uEnd = ( uStart + uGrainSize );

			// this will get deallocated after the wrapped is marked Completed
			SystemParallelForWrapper *pWrapper = new SystemParallelForWrapper();
			ASSERT( pWrapper != NULL );
			pWrapper->pParentTask = pParentTask;
			pWrapper->pfnParallelForFunction = pfnJobFunction;
			pWrapper->pUserData = pParam;
			pWrapper->uStart = uStart;
			pWrapper->uEnd = uEnd;
			pWrapper->lJobWaitCount = &lJobWaitCount;
			
			m_pThreadPool->QueueUserWorkItem( pWrapper );
			
			uStart = uEnd;
		}

		// now do our job
		uEnd = uCount;
		
		// Call the function, and figure out how long it took.
		i64 counter = __rdtsc();
		pfnJobFunction( pParam, uStart, uEnd );
		counter = __rdtsc() - counter;

		// Log this job's time in instrumentation
		if( pSystemTask != NULL )
		{
			Singletons::Instrumentation.CaptureJobCounterTicks( pSystemTask->GetSystemType(), counter );
		}
	
		// now check up on progress of the dispatched jobs
		while( lJobWaitCount > 0 )
		{
			// pull something off of the job queue and execute it
			m_pThreadPool->PopAndProcessWorkItem();
		}
	}
	else
	{
		// just do it ourselves
		pfnJobFunction( pParam, begin, end );
	}
}
