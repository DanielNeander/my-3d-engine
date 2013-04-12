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
#include "TaskManagerTBB.h"
#include "EnvironmentManager.h"
#include "PlatformManager.h"
#include "ServiceManager.h"

#include "Instrumentation.h"

#include <process.h>
//#include <windows.h>

#pragma warning (push)
#pragma warning (disable: 4100)
    #include <tbb/parallel_for.h>
    #include <tbb/blocked_range.h>
    #include <tbb/partitioner.h>
#pragma warning (pop)

//#include "trace.h"

// Macros that improve readability of the rest of the code by eliminating the necessity 
// of frequent checks if the USE_THREAD_PROFILER and STATISTICS_BY_JOB_TYPE are defined
#if defined(USE_THREAD_PROFILER)
    #define DECLARE_TP_EVENT_ARG( tpEvent ), __itt_event tpEvent
    #define PASS_TP_EVENT_ARG( tpEvent ), tpEvent
#else
    #define DECLARE_TP_EVENT_ARG( tpEvent )
    #define PASS_TP_EVENT_ARG( tpEvent )
#endif

#if defined(STATISTICS_BY_JOB_TYPE)

    #define DECLARE_JOB_AND_TP_EVENT_ARGS( jobType, tpEvent ) \
        , u32 jobType                                       \
        DECLARE_TP_EVENT_ARG( tpEvent )

    #define PASS_JOB_AND_TP_EVENT_ARGS( jobType, tpEvent )    \
        , jobType                                           \
        PASS_TP_EVENT_ARG( tpEvent )

    #define JOB_TASK_STARTED( jobType, tpEvent )      \
        __ITT_EVENT_START( tpEvent, PROFILE_TASKMANAGER );               \
        {                                           \
            i64 counter = __rdtsc();

    #define JOB_TASK_FINISHED( jobType, tpEvent )     \
            counter = __rdtsc() - counter;          \
            Singletons::Instrumentation.CaptureJobCounterTicks( jobType, counter ); \
        }                                           \
        __ITT_EVENT_END( tpEvent, PROFILE_TASKMANAGER );

#else /* !STATISTICS_BY_JOB_TYPE */

    #define DECLARE_JOB_AND_TP_EVENT_ARGS( jobType, tpEvent ) \
        DECLARE_TP_EVENT_ARG( tpEvent )

    #define PASS_JOB_AND_TP_EVENT_ARGS( jobType, tpEvent )    \
        PASS_TP_EVENT_ARG( tpEvent )

    #define JOB_TASK_STARTED( jobType, tpEvent )     \
        __ITT_EVENT_START( tpEvent, 0 );

    #define JOB_TASK_FINISHED( jobType, tpEvent )    \
        __ITT_EVENT_END( tpEvent, 0 );

#endif /* STATISTICS_BY_JOB_TYPE */


namespace local {

    /// <summary>
    /// Base class containing data common to classes handling different callbacks.
    /// </summary>
    class GenericCallbackData
    {
    public:
        GenericCallbackData( void* pParam
                             DECLARE_JOB_AND_TP_EVENT_ARGS( jobType, tpEvent )
        )
            : m_pParam( pParam )
            #if defined(STATISTICS_BY_JOB_TYPE)
			, m_jobType( jobType )
            #endif
            #if defined(USE_THREAD_PROFILER)
			, m_tpEvent( tpEvent )
            #endif
        {}

    protected:
        void* m_pParam;

        #if defined(STATISTICS_BY_JOB_TYPE)
		u32 m_jobType;
        #endif

        #if defined(USE_THREAD_PROFILER)
		__itt_event m_tpEvent;
        #endif

    }; // template class GenericCallbackTask


    /// <summary>
    /// This class is used by <c>TaskManagerTBB</c> to execute a generic callback.
    /// </summary>
    template<class Fptr>
    class GenericCallbackTask : public tbb::task, public GenericCallbackData
    {
    public:
        GenericCallbackTask( Fptr fFunc, void* pParam
                             DECLARE_JOB_AND_TP_EVENT_ARGS( jobType, tpEvent )
		) 
            : GenericCallbackData(pParam
                                  PASS_JOB_AND_TP_EVENT_ARGS( jobType, tpEvent) )
            , m_fFunc( fFunc )
        {}

        virtual 
        tbb::task* execute()
        {
            // Call the callback function, mark it for TP, and figure out how long it took.
            ASSERT( m_fFunc != NULL );
            JOB_TASK_STARTED( m_jobType, m_tpEvent );

            m_fFunc( m_pParam );
            
            JOB_TASK_FINISHED( m_jobType, m_tpEvent );
            return NULL;
        }

    protected:
        Fptr m_fFunc;

    }; // template class GenericCallbackTask

    typedef GenericCallbackTask<ITaskManager::JobFunction> SystemTask;
    typedef GenericCallbackTask<ITaskManager::JobFunction> JobTask;
    typedef GenericCallbackTask<ITaskManager::JobCompletionFunction> JobCompletionTask;

    /// <summary>
    /// This class is used by <c>TaskManagerTBB</c> to stall unused threads.
    /// </summary>
    /// <seealso cref="TaskManagerTBB::SetNumberOfThreads"/>
    class StallTask : public tbb::task
    {
    public:
        StallTask( TaskManagerTBB* pTaskManager, Handle hWaitFor )
            : m_pTaskManager( pTaskManager ),
			m_hWaitFor( hWaitFor )
        {}

        tbb::task *execute()
        {
			if( m_pTaskManager->IsPrimaryThread() )
			{
				// we can't stall, so stall somebody else
				m_pTaskManager->AddStallTask();
				// wait for a tenth of a second to encourage somebody else to grab the stall task
				tbb::this_tbb_thread::sleep( tbb::tick_count::interval_t( 0.1 ) );
			}
			else
			{
				ASSERT( m_hWaitFor != NULL );
				WaitForSingleObject( m_hWaitFor, INFINITE );
			}
	        return NULL;
        }

    protected:
		TaskManagerTBB* m_pTaskManager;
        Handle m_hWaitFor;

    }; // class StallTask

    /// <summary>
    /// This class is used by <c>TaskManagerTBB</c> to manage synchronized callbacks.
    /// </summary>
    /// <seealso cref="TaskManagerTBB::NonStandardPerThreadCallback"/>
    class SynchronizeTask: public tbb::task
    {
    public:
        SynchronizeTask() {}

        tbb::task *execute()
        {
	        ASSERT( m_fCallback != NULL );
	        ASSERT( m_hAllCallbacksInvokedEvent != NULL );
        	
	        m_fCallback( m_pCallbackParam );
            
			if( InterlockedDecrement( &m_lCallbacksCount ) == 0 )
	        {
		        // set all of the SynchronizeTasks free
		        SetEvent( m_hAllCallbacksInvokedEvent );
	        }
	        else
	        {
		        // wait for somebody else to finish up
		        WaitForSingleObject( m_hAllCallbacksInvokedEvent, INFINITE );
	        }
            
			return NULL;
        }

        static void PrepareCallback( 
            ITaskManager::JobFunction fFunc,
		    void* pParam,
		    u32 uCount
	    ) 
        {
            m_fCallback = fFunc;
		    m_pCallbackParam = pParam;
		    m_lCallbacksCount = uCount;
            ResetEvent( m_hAllCallbacksInvokedEvent );
        }

    protected:
        friend class TaskManagerTBB;
        static Handle m_hAllCallbacksInvokedEvent;
        static ITaskManager::JobFunction m_fCallback;
        static void* m_pCallbackParam;
        static volatile long m_lCallbacksCount;
    }; // class SynchronizeTask

    Handle SynchronizeTask::m_hAllCallbacksInvokedEvent = NULL;
    ITaskManager::JobFunction SynchronizeTask::m_fCallback = NULL;
    void* SynchronizeTask::m_pCallbackParam = NULL;
    volatile long SynchronizeTask::m_lCallbacksCount = 0;

    static TaskManagerTBB* g_pTaskManagerTBB = NULL;
} // namespace local

using namespace local;

void TaskManagerTBB::AddStallTask( void )
{
	ASSERT( m_pStallPoolParent != NULL );
	tbb::task *pStallTask = new( m_pStallPoolParent->allocate_additional_child_of( *m_pStallPoolParent ) )
		StallTask( this, m_hStallPoolSemaphore );
	ASSERT( pStallTask != NULL );
	m_pStallPoolParent->spawn( *pStallTask );
}

void TaskManagerTBB::SystemTaskCallback( void* pData )
{
    ISystemTask* pTask = static_cast<ISystemTask*>( pData );
    pTask->Update( g_pTaskManagerTBB->m_fDeltaTime );
}

void TaskManagerTBB::InitAffinityData( void* mgr )
{
    TaskManagerTBB* pThis = static_cast<TaskManagerTBB*>( mgr );

    SpinWait::Lock lock( pThis->m_spinMutex );
	pThis->m_affinityIDs.push_back( tbb::task::self().affinity() );
}


void
TaskManagerTBB::Init( void )
{
    // Call this from the primary thread before calling any other TaskManagerTBB methods.
	g_pTaskManagerTBB = this;

	m_uPrimaryThreadID = tbb::this_tbb_thread::get_id();

	m_bTimeToQuit = False;
	m_uRequestedNumberOfThreads = Singletons::EnvironmentManager.Variables().GetAsInt( "TaskManager::Threads", 0 );
    if( m_uRequestedNumberOfThreads == 0 )
	{
        // IMPLEMENTATION NOTE
		// The audio thread (which Thread Profiler shows as constantly working) 
        // has no negative impact on the performance when it causes apparent 
        // oversubscription. Shrinking the pool to avoid this apparent oversubscription
        // results in smaller FPS. So this is probably one of the cases when TP
        // misinterprets the behavior of threads created by some of the Windows 
        // subsystems (DirectX is historically its weak place).
        //
        m_uRequestedNumberOfThreads = tbb::task_scheduler_init::default_num_threads();
	}
    
    m_uMaxNumberOfThreads = 0;
	m_uNumberOfThreads = 0;
	m_uTargetNumberOfThreads = 0;
	
    m_pStallPoolParent = NULL;
	m_hStallPoolSemaphore = CreateSemaphore( NULL, 0, m_uRequestedNumberOfThreads, NULL );
	SynchronizeTask::m_hAllCallbacksInvokedEvent = CreateEvent( NULL, True, False, NULL );

#if defined(USE_THREAD_PROFILER)
	m_bTPEventsForTasks = Singletons::EnvironmentManager.Variables().GetAsBool( "TaskManagerTBB::TPEventsForTasks", False );
	m_bTPEventsForJobs = Singletons::EnvironmentManager.Variables().GetAsBool( "TaskManagerTBB::TPEventsForJobs", False );
	m_bTPEventsForSynchronize = Singletons::EnvironmentManager.Variables().GetAsBool( "TaskManagerTBB::TPEventsForSynchronize", False );

    if( m_bTPEventsForSynchronize )
	{
	    m_tSynchronizeTPEvent = __itt_event_createA( "Synchronize", 11 );
	}

    m_tpSystemTaskSpawn = __itt_event_createA( "SystemTaskSpawn", 15 );
#endif /* USE_THREAD_PROFILER */
    
    m_uMaxNumberOfThreads = m_uRequestedNumberOfThreads;
    m_uTargetNumberOfThreads = m_uRequestedNumberOfThreads;
	m_uNumberOfThreads = m_uRequestedNumberOfThreads;

    m_pTbbScheduler = new tbb::task_scheduler_init( m_uRequestedNumberOfThreads );
    m_pSystemTasksRoot = new( tbb::task::allocate_root() ) tbb::empty_task;

    NonStandardPerThreadCallback( InitAffinityData, this );
	
	// Cache the thread count for display.
	Singletons::Instrumentation.setActiveThreadCount( m_uTargetNumberOfThreads );
} // TaskManagerTBB::Init


void
TaskManagerTBB::Shutdown( void )
{
    // Call this from the primary thread as the last TaskManagerTBB call.
	ASSERT( IsPrimaryThread() );

	// get the callback thread to exit
    m_bTimeToQuit = True;

	// trigger the release of the stall pool
	ReleaseSemaphore( m_hStallPoolSemaphore, m_uMaxNumberOfThreads, NULL );

    m_pSystemTasksRoot->destroy( *m_pSystemTasksRoot );

    delete m_pTbbScheduler;

    // now get rid of all the events
	CloseHandle( m_hStallPoolSemaphore );
	m_hStallPoolSemaphore = NULL;
    CloseHandle( SynchronizeTask::m_hAllCallbacksInvokedEvent );

#ifdef USE_THREAD_PROFILER
	m_SupportForSystemTasks.clear();
#endif
} // TaskManagerTBB::Shutdown


// PerformanceHint is used to modify the order that tasks are sent to the scheduler.
// The order of definition is the order of scheduling.
enum PerformanceHint
{
    Task_LongSerial = 0,
    Task_LongParallel,
    Task_Short,
	Task_NoPerformanceHint,
    Task_MAX
};

inline
PerformanceHint GetPerformanceHint( ISystemTask* pTask ) 
{
	// The indices of this array need to match System::TypeIndices.
	// Custom type indices are added here for convenience, but they are not guaranteed
	// to match (they're created in code with System::Types::MakeCustom()).
	//
	// If you modify System::TypeIndices or System::Types, please modify this to match.
	static PerformanceHint hints[System::Types::MAX] =
    {
		Task_NoPerformanceHint,	// 0:  Null
		Task_NoPerformanceHint,	// 1:  Not used
		Task_NoPerformanceHint, // 2:  Generic
        Task_Short,             // 3:  Geometry 
        Task_LongSerial,        // 4:  Graphics
        Task_LongSerial,        // 5:  PhysicsCollision
        Task_NoPerformanceHint, // 6:  Audio
        Task_NoPerformanceHint, // 7:  Input
		Task_LongParallel,      // 8:  AI
		Task_NoPerformanceHint, // 9:  Animation
		Task_Short,             // 10: Scripting
		Task_NoPerformanceHint,	// 11: Explosion - test for ideal hint
		Task_NoPerformanceHint, // 12: Water - test for ideal hint
		Task_NoPerformanceHint, // 13: Not used
		Task_NoPerformanceHint, // 14: Not used
		Task_NoPerformanceHint, // 15: Not used
		Task_Short,             // 16: ProceduralTrees
		Task_NoPerformanceHint,	// 17: Not used
		Task_LongParallel,		// 18: ProceduralFire

		// Not used below here, initialized for safety.
		Task_NoPerformanceHint,	// 19: Not used
		Task_NoPerformanceHint,	// 20: Not used
		Task_NoPerformanceHint,	// 21: Not used
		Task_NoPerformanceHint,	// 22: Not used
		Task_NoPerformanceHint,	// 23: Not used
		Task_NoPerformanceHint,	// 24: Not used
		Task_NoPerformanceHint,	// 25: Not used
		Task_NoPerformanceHint,	// 26: Not used
		Task_NoPerformanceHint,	// 27: Not used
		Task_NoPerformanceHint,	// 28: Not used
		Task_NoPerformanceHint,	// 29: Not used
		Task_NoPerformanceHint,	// 30: Not used
		Task_NoPerformanceHint	// 31: Not used
    };

	u32 jobType = System::Types::GetIndex(pTask->GetSystemType());

    return hints[jobType];
}

void
TaskManagerTBB::IssueJobsForSystemTasks(
    ISystemTask** pTasks,
    u32 uTaskCount,
    f32 fDeltaTime
    )
{
    // Call this from the primary thread to schedule system work.
	ASSERT( IsPrimaryThread() );

	__ITT_EVENT_START( m_tpSystemTaskSpawn, PROFILE_TASKMANAGER );

    ASSERT( uTaskCount > 0 );

    m_fDeltaTime = fDeltaTime;

	UpdateThreadPoolSize();

	ASSERT( m_pSystemTasksRoot != NULL );

	// We'll be adding tasks to the m_pSystemTasksRoot, and we will eventually call wait_for_all on it.
	// Support the eventual wait_for_all by setting reference count to 1 now.
	m_pSystemTasksRoot->set_ref_count( 1 );

    // now schedule the tasks, based upon their PerformanceHint order
	tbb::task_list tTaskList;
    u32 uAffinityCount = (u32)m_affinityIDs.size();

	u32 h = 0;
	u32 i = 0;
	for( h = 0; h < Task_MAX; h++ )
	{
		for( i = 0; i < uTaskCount; i++ )
		{
			if( pTasks[i]->IsPrimaryThreadOnly() )
			{
				// put this task on the list of tasks to be run on the primary thread
				// only do this during the first outer loop
				if( h == 0)
				{
					m_primaryThreadSystemTaskList.push_back( pTasks[i] );
				}
			}
			else
			{
				// see if it's time to dispatch this task
				if( GetPerformanceHint( pTasks[i] ) == (PerformanceHint)h )
				{
					// this task can be run on an arbitrary thread -- allocate it 
					SystemTask *pSystemTask = new( m_pSystemTasksRoot->allocate_additional_child_of( *m_pSystemTasksRoot ) )
						SystemTask( SystemTaskCallback, pTasks[i]
									PASS_JOB_AND_TP_EVENT_ARGS( pTasks[i]->GetSystemType(), 
																GetSupportForSystemTask( pTasks[i] ).m_tpeSystemTask )
								   );
					// affinity will increase the chances that each SystemTask will be assigned
					// to a unique thread, regardless of PerformanceHint
					ASSERT( pSystemTask != NULL );
					pSystemTask->set_affinity( m_affinityIDs[i % uAffinityCount] );
					tTaskList.push_back( *pSystemTask );
				}
			}
		}
	}

    // We only spawn system tasks here. They in their turn will spawn descendant tasks.
    // Waiting for the whole bunch completion happens in WaitForSystemTasks.
	m_pSystemTasksRoot->spawn( tTaskList );
	
    __ITT_EVENT_END( m_tpSystemTaskSpawn, PROFILE_TASKMANAGER );
} // TaskManagerTBB::IssueJobsForSystemTasks


void
TaskManagerTBB::NonStandardPerThreadCallback(
    JobFunction pfnCallback,
    void* pData
    )
{
	// This method triggers a synchronized callback to be called once by each thread used
	// by the TaskManagerTBB.  This method waits until all callbacks have executed.
    
	// only one at a time here
    SpinWait::Lock tLock( m_tSynchronizedCallbackMutex );

    __ITT_EVENT_START( m_tSynchronizeTPEvent, PROFILE_TASKMANAGER );

    u32 uNumberOfThreads = m_uNumberOfThreads;
    if( uNumberOfThreads != m_uMaxNumberOfThreads )
    {
        m_uTargetNumberOfThreads = m_uMaxNumberOfThreads;
        UpdateThreadPoolSize();
    }
	
    SynchronizeTask::PrepareCallback( pfnCallback, pData, m_uMaxNumberOfThreads );

    tbb::task* pBroadcastParent = new( tbb::task::allocate_root() ) tbb::empty_task;
	ASSERT( pBroadcastParent != NULL );

	// we have one reference for each thread, plus one for the wait_for_all below
    pBroadcastParent->set_ref_count( m_uMaxNumberOfThreads + 1 );

    tbb::task_list tList;
    for( u32 i = 0; i < m_uMaxNumberOfThreads; i++ )
    {
        // Add a SynchronizedTasks for each thread in the TBB pool (workers + this master)
        tbb::task *pNewTask = new( pBroadcastParent->allocate_child() ) SynchronizeTask;
		ASSERT( pNewTask != NULL );
        tList.push_back( *pNewTask );
    }

    // get the synchronize tasks running
    pBroadcastParent->spawn_and_wait_for_all( tList );
    pBroadcastParent->destroy( *pBroadcastParent );

    if( uNumberOfThreads != m_uMaxNumberOfThreads )
    {
        m_uTargetNumberOfThreads = uNumberOfThreads;
        UpdateThreadPoolSize();
    }
    __ITT_EVENT_END( m_tSynchronizeTPEvent, PROFILE_TASKMANAGER );

} // TaskManagerTBB::NonStandardPerThreadCallback


u32
TaskManagerTBB::GetRecommendedJobCount(
    ITaskManager::JobCountInstructionHints Hints
    )
{
    // Call this method to determine the ideal number of tasks to submit to the TaskManagerTBB
	// for maximum performance.
    
	//
    // Ignoring hints for now and just returning the number of available threads.
    //
    UNREFERENCED_PARAM( Hints );

    return m_uNumberOfThreads;
}


class ParallelForBody : public GenericCallbackData
{
public:
    ParallelForBody( ITaskManager::ParallelForFunction pfn, void* pParam
                     DECLARE_JOB_AND_TP_EVENT_ARGS( jobType, tpEvent )
	) 
        : GenericCallbackData(pParam
                              PASS_JOB_AND_TP_EVENT_ARGS(jobType, tpEvent) )
        , m_pfnCallback( pfn )
    {}

    void operator ()( const tbb::blocked_range<u32>& r ) const
    {
        JOB_TASK_STARTED( m_jobType, m_tpEvent );

        m_pfnCallback( m_pParam, r.begin(), r.end() );
        
        JOB_TASK_FINISHED( m_jobType, m_tpEvent );
    }

private:
    ITaskManager::ParallelForFunction m_pfnCallback;

}; // class ParallelForBody


void 
TaskManagerTBB::ParallelFor(
    ISystemTask* pSystemTask,
    ParallelForFunction pfnJobFunction, 
    void* pParam, 
    u32 begin, 
    u32 end, 
    u32 minGrainSize )
{
#if defined(STATISTICS_BY_JOB_TYPE)
	// ??? How often does this fail over to NULL?
	u32 jobType = pSystemTask ? pSystemTask->GetSystemType() : System::Types::Null;
#endif
#if defined(USE_THREAD_PROFILER)
    __itt_event tpEvent = GetSupportForSystemTask( pSystemTask ).m_tpeSystemTaskJob;
#endif

    ParallelForBody body( pfnJobFunction, pParam
                           PASS_JOB_AND_TP_EVENT_ARGS(jobType, tpEvent) );

    if( m_uNumberOfThreads != 1 )
    {
        tbb::parallel_for( tbb::blocked_range<u32>( begin, end, minGrainSize ), body, tbb::auto_partitioner() );
    }
    else
    {
        body( tbb::blocked_range<u32>( begin, end, minGrainSize ) );
    }
}


void
TaskManagerTBB::WaitForSystemTasks(
    ISystemTask** pTasks,
    u32 uTaskCount
    )
{
    // Call this from the primary thread to wait until specified tasks spawned with IssueJobsForSystemTasks
	// and all of their subtasks are complete.
	ASSERT( IsPrimaryThread() );

	ASSERT( uTaskCount > 0 );
	ASSERT( uTaskCount <= System::Types::MAX );

	// if we are waiting for any of the tasks dedicated to the primary thread,
	// then execute them now

	// if we issued primary thread tasks that we're not waiting for this time,
	// then save them for the next time
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );

    SystemTasksList::iterator it = m_primaryThreadSystemTaskList.begin();
    for( ; it != m_primaryThreadSystemTaskList.end(); ++it )
    {
		// see if we are waiting for this task
        if( std::find( pTasks, pTasks + uTaskCount, *it ) )
        {
			// we are, so execute it now on the primary thread
            __ITT_EVENT_START( GetSupportForSystemTask( *it ).m_tpeSystemTask, PROFILE_TASKMANAGER );

            ( *it )->Update( m_fDeltaTime );

            __ITT_EVENT_END( GetSupportForSystemTask( *it ).m_tpeSystemTask, PROFILE_TASKMANAGER );
		}
        else
		{
			// save it for next time
            m_tmpTaskList.push_back( *it );
        }
    }
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL );
    m_primaryThreadSystemTaskList.clear();
    m_primaryThreadSystemTaskList.swap( m_tmpTaskList );

    // contribute to the parallel calculation, and when it completes, we're done
    m_pSystemTasksRoot->wait_for_all();
}


u32 TaskManagerTBB::GetNumberOfThreads(
	void
	)
{
	// Call this method to get the number of threads in the thread pool which are active for running work.
	return m_uNumberOfThreads;
}


void TaskManagerTBB::SetNumberOfThreads(
	u32 uNumberOfThreads
	)
{
	// This method constrains the number of threads used by the TaskManagerTBB.
    u32 uTargetNumberOfThreads = uNumberOfThreads;
	
    if( uNumberOfThreads > m_uMaxNumberOfThreads || uTargetNumberOfThreads == 0 )
	{
		uTargetNumberOfThreads = m_uMaxNumberOfThreads;
	}

	m_uTargetNumberOfThreads = uTargetNumberOfThreads;
}


#if defined(USE_THREAD_PROFILER)

#define InitTpEventsForSystem(base_name, base_len) \
    if( m_bTPEventsForTasks ) \
	{ \
	    support.m_tpeSystemTask = __itt_event_createA( base_name, base_len ); \
	} \
    if( m_bTPEventsForJobs ) \
	{ \
	    support.m_tpeSystemTaskJob = __itt_event_createA( base_name"Job", base_len + 3 );  \
	    support.m_tpeSystemTaskJobCompletion = __itt_event_createA( base_name"JobCompletion", base_len + 13 ); \
    }

TaskManagerTBB::SystemTaskSupport&
TaskManagerTBB::GetSupportForSystemTask( 
    ISystemTask* pTask
    )
{
    if( !pTask )
    {
        static SystemTaskSupport emptySupport;

        return emptySupport;
    }

	SystemTaskSupport& support = m_SupportForSystemTasks[pTask];

    if( !support.m_bInitialized )
	{
		// figure out the most appropriate Thread Profiler event to map to this System
		support.m_bInitialized = true;

		switch( pTask->GetSystemScene()->GetSystem()->GetSystemType() )
		{
		case( System::Types::Geometry ):
			InitTpEventsForSystem( "Geometry", 8 );
			break;
		case( System::Types::Graphics ):
			InitTpEventsForSystem( "Graphics", 8 );
			break;
		case( System::Types::PhysicsCollision ):
			InitTpEventsForSystem( "Physics", 7 );
			break;
		case( System::Types::Audio ):
			InitTpEventsForSystem( "Audio", 5 );
			break;
		case( System::Types::Input ):
			InitTpEventsForSystem( "Input", 5 );
			break;
		case( System::Types::AI ):
			InitTpEventsForSystem( "AI", 2 );
			break;
		case( System::Types::Animation ):
			InitTpEventsForSystem( "Animation", 9 );
			break;
		case( System::Types::Scripting ):
			InitTpEventsForSystem( "Scripting", 9 );
			break;

        // NOTE: these next cases mimic the inline call to System::Types::MakeCustom
		case( 1 << (16 + 0) ):
			InitTpEventsForSystem( "ProceduralTrees", 15 );
			break;
		case( 1 << (16 + 2) ):
			InitTpEventsForSystem( "ProceduralFire", 14 );
			break;
		
		default:
			break;
		}
	}
    return support;
}

#endif /* USE_THREAD_PROFILER */


Bool TaskManagerTBB::IsPrimaryThread( void )
{
	return ( tbb::this_tbb_thread::get_id() == m_uPrimaryThreadID );
}

Bool TaskManagerTBB::IsTBBThread( void )
{
	// This method is used to determine if the calling thread is an Intel Threading Building Blocks thread.
#ifdef _DEBUG
    // If called not from TBB thread task::self() will assert itself
    return &tbb::task::self() != NULL;
#else
    return True;
#endif
}

void TaskManagerTBB::UpdateThreadPoolSize( void )
{
	// This method is called within IssueJobsForSystemTasks to update the size of TBB thread pool
	// if necessary.  The thread pool is resized by a call to SetNumberOfThreads.
    
	// this is accomplished by creating some tasks which do not complete until signaled
	// see if we should adjust the WaitTasks in the stall pool
	if( m_uTargetNumberOfThreads != m_uNumberOfThreads )
	{
		u32 uNumThreadsToWait = ( m_uMaxNumberOfThreads - m_uTargetNumberOfThreads );
		u32 uNumThreadsToFree = ( m_uMaxNumberOfThreads - m_uNumberOfThreads );
		
		// free up all the threads
        if( m_pStallPoolParent )
        {
		    ReleaseSemaphore( m_hStallPoolSemaphore, uNumThreadsToFree, NULL );
            // Make sure there are no stall tasks from the previous cycle lingering 
            // out there and competing for the semaphore
            m_pStallPoolParent->wait_for_all();
        }
        else
        {
		    // make a new stall parent
		    m_pStallPoolParent = new( tbb::task::allocate_root() ) tbb::empty_task;
		}
		ASSERT( m_pStallPoolParent != NULL );
        m_pStallPoolParent->set_ref_count( uNumThreadsToWait + 1 );

        tbb::task_list tList;
		for( u32 i = 0; i < uNumThreadsToWait; i++ )
		{
            tbb::task *pStallTask = new( m_pStallPoolParent->allocate_child() ) StallTask( this, m_hStallPoolSemaphore );
            ASSERT( pStallTask != NULL );
			tList.push_back( *pStallTask );
		}

		m_pStallPoolParent->spawn( tList );

		m_uNumberOfThreads = m_uTargetNumberOfThreads;

		// Cache the thread count for display.
		Singletons::Instrumentation.setActiveThreadCount( m_uTargetNumberOfThreads );
	}
}
