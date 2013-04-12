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

//#include <Windows.h>
#include "stdafx.h"
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

#include "PlatformManager.h"

#include "ThreadPool.h"


void
BasicThreadPool::Initialize(
    u32 inNumThreads
    )
{
    m_NumThreads = inNumThreads;
	m_NumActiveThreads = inNumThreads;
    
	if( inNumThreads > 0 )
	{
		m_hWorkQueueSemaphore = CreateSemaphore( NULL, 0, 1024, NULL );
		m_hWorkerThreads = new Handle[ m_NumThreads ];
		m_hWorkerthreadIds = new u32[ m_NumThreads ];
		m_pWorkQueue = new WorkQueue();

		//
		// initialize the thread context array
		//
		m_pThreadContext = new ThreadContext[ m_NumThreads ];
		for( u32 i=0; i < m_NumThreads; i++ )
		{
			m_pThreadContext[ i ].pThreadPool = this;
	    
			// Status event initialized to False
			m_pThreadContext[ i ].hStatusEvent = CreateEvent( NULL, TRUE, False, NULL );
	    
			m_pThreadContext[ i ].hWorkQueueSemaphore = m_hWorkQueueSemaphore;

			m_pThreadContext[ i ].hSleepEvent = CreateEvent( NULL, TRUE, False, NULL );

			m_pThreadContext[ i ].hWakeEvent = CreateEvent( NULL, TRUE, False, NULL );
		}

		for( u32 i = 0 ; i < m_NumThreads; i++ )
		{
			// NOTE: we should offset stack size of threads to avoid memory aliasing issues
			m_hWorkerThreads[i] = CreateThread( 
									NULL, 0,
									( LPTHREAD_START_ROUTINE )this->WorkerThreadProc,
									( LPVOID )&m_pThreadContext[ i ], 0,
									( LPDWORD )&m_hWorkerthreadIds[ i ]
									);
		}
	}

    m_bInitialized = True;
}


void
BasicThreadPool::Shutdown(
    void
    )
{
	// wake up all of our threads so they can shut down
    SetActiveThreadCount( m_NumThreads );

	//
    // send an event to shutdown the threads.
    //
    for( u32 i = 0; i < m_NumThreads; i++ )
    {
        m_pThreadContext[ i ].status = ThreadContext::Status::Shutdown;
        SetEvent( m_pThreadContext[ i ].hStatusEvent );
    }

    //
    // wait for all the threads to complete
    //
    WaitForMultipleObjects( m_NumThreads, m_hWorkerThreads, TRUE, INFINITE );

	// clean queue
	while( !m_pWorkQueue->empty() )
	{
		PopAndProcessWorkItem( False );
	}
    SAFE_DELETE( m_pWorkQueue );

    // clean up synch prims
    for( u32 i=0; i < m_NumThreads; i++ )
    {
        CloseHandle( m_pThreadContext[ i ].hStatusEvent );
    }
    CloseHandle( m_hWorkQueueSemaphore );
    SAFE_DELETE_ARRAY( m_pThreadContext );
    SAFE_DELETE_ARRAY( m_hWorkerThreads );
    SAFE_DELETE_ARRAY( m_hWorkerthreadIds );
    m_bInitialized = False;
}


void
BasicThreadPool::NonStandardThreadExecute(
    ThreadContext::ThreadCallback pfnCallback,
    void* pData
    )
{
	u32 uOldThreadCount = m_NumActiveThreads;

	SetActiveThreadCount( m_NumThreads );

    m_CallbackCountdown = m_NumThreads;

    for( u32 i=0; i < m_NumThreads; i++ )
    {
        m_pThreadContext[ i ].pfnThreadCallback = pfnCallback;
        m_pThreadContext[ i ].pThreadData = pData;
        m_pThreadContext[ i ].pCallbackCountdown = &m_CallbackCountdown;

        m_pThreadContext[ i ].status = ThreadContext::Status::Callback;
        ::SetEvent( m_pThreadContext[ i ].hStatusEvent );
    }

	SetActiveThreadCount( uOldThreadCount );

	while( m_CallbackCountdown > 0 )
    {
        Sleep( 0 );
    }
}


void
BasicThreadPool::QueueUserWorkItem(
    WorkItem* pWorkItem
    )
{
    ASSERT( pWorkItem != NULL );

	// intentionally creating a block to scope the lock
    {
        SpinWait::Lock Lock( m_QueueSW );
        m_pWorkQueue->push( pWorkItem );
    }

    if( ReleaseSemaphore( m_hWorkQueueSemaphore, 1, NULL ) == False )
    {
        ASSERT( False );
    }
}


WorkItem* BasicThreadPool::RemoveWorkItem( )
{
	WorkItem* pWorkItem = NULL;
	
	// create a block to scope a lock
	{
		SpinWait::Lock Lock( m_QueueSW );
		
		// pop off the queue
		if( !m_pWorkQueue->empty() )
		{
			pWorkItem = m_pWorkQueue->front();
			ASSERT( pWorkItem != NULL );
			m_pWorkQueue->pop();
		}
	}
	
	return pWorkItem;
}


u32 WINAPI BasicThreadPool::WorkerThreadProc( LPVOID pInThreadContext ) 
{
    Handle hSyncVector[3]; 
    u32 uEventStatus = 0;
    ThreadContext* pThreadContext = ( ThreadContext* )pInThreadContext;
    BasicThreadPool* pThreadPool = pThreadContext->pThreadPool;

    hSyncVector[0] = pThreadContext->hWorkQueueSemaphore;
    hSyncVector[1] = pThreadContext->hStatusEvent;
	hSyncVector[2] = pThreadContext->hSleepEvent;

#pragma warning (push)
#pragma warning (disable: 4127)
    while( True )
#pragma warning (pop)
    {
        uEventStatus = WaitForMultipleObjects( 3, hSyncVector, False, INFINITE ); 
        if( uEventStatus == WAIT_TIMEOUT )
        {
            continue;
        }
        
        switch( uEventStatus - WAIT_OBJECT_0 )
        { 
        case 0:
            // data available on the work queue
			pThreadPool->PopAndProcessWorkItem();
            break;

        case 1:
			// time to shutdown or do a callback
            if( pThreadContext->status == ThreadContext::Status::Shutdown )
            {
                return 0;
            }
            else if( pThreadContext->status == ThreadContext::Status::Callback )
            {
                pThreadContext->pfnThreadCallback( pThreadContext->pThreadData );
                ::InterlockedDecrement( ( PLONG )pThreadContext->pCallbackCountdown );
                ::ResetEvent( pThreadContext->hStatusEvent );
            }
			break;

		case 2:
			// time to sleep
			ResetEvent( pThreadContext->hSleepEvent );
			ResetEvent( pThreadContext->hWakeEvent );
			WaitForSingleObject( pThreadContext->hWakeEvent, INFINITE );
			break;

        default:
            ASSERT( False ); // problem here
            return 0;
        }
    }
}

void BasicThreadPool::PopAndProcessWorkItem( Bool bProcess )
{
	WorkItem* pWorkItem = RemoveWorkItem();
    if( pWorkItem != NULL )
	{
		if( bProcess )
		{
			pWorkItem->Process();
			pWorkItem->SetStatus( WorkItem::Status::Completed );
		}
		if( pWorkItem->IsDynamicallyAllocated() )
		{
			delete pWorkItem;
		}
	}
}

//-----------------------------------------------------------------------------
//
// BasicThreadPool::SetActiveThreadCount
//
// Sets the number of active threads.  We do this by "waking up" (signaling)
// the number of active threads we want and by putting the rest to sleep.
//
void BasicThreadPool::SetActiveThreadCount( u32 uCount )
{
	u32 numThreads = uCount;

	if( numThreads > m_NumThreads )
	{
		numThreads = m_NumThreads;
	}

	for( u32 i = 0; i < m_NumThreads; i++ )
    {
		if( ( i < numThreads ) && ( i >= m_NumActiveThreads ) )
		{
			//---------------------------------------------------------------------
			// Set the thread's sleep event to signaled which wakes it up
			//
			SetEvent( m_pThreadContext[ i ].hWakeEvent );
		}

		if( ( i < m_NumActiveThreads ) && ( i >= numThreads ) )
		{
			//---------------------------------------------------------------------
			// Set the thread's sleep event to nonsignaled which puts it to sleep
			//
			SetEvent( m_pThreadContext[ i ].hSleepEvent );
		}
	}

	m_NumActiveThreads = numThreads;
}
