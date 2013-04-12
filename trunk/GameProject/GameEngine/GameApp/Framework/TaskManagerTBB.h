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

#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4189 4244 4512 4634 )
#include <tbb/task.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/tbb_thread.h>
#pragma warning( pop )

#include "../BaseTypes/BaseTypes.h"
#include "../Interfaces/Interface.h"
#include "TaskManager.h"

#include "IttNotify.h"

// The current mechanism of by-job statistics does not work correctly in case of 
// nested parallelism, since work-stealing TBB scheduler may (and does) interleave
// different jobs on the same thread. To make it work correctly the statistics 
// should be accumulated on a per-thread basis. Besides with the fine grained job
// tasks (which are necessary to achieve acceptable load balancing) the overhead 
// of statistics tracking may become noticeable.
#define STATISTICS_BY_JOB_TYPE

/// <summary>
/// This class uses Intel Threading Building Blocks to run tasks.
/// </summary>
class TaskManagerTBB: public TaskManager
{
public:
    /// <summary cref="TaskManagerTBB::Init">
	/// Call this from the primary thread before calling any other <c>TaskManagerTBB</c> methods.
	/// </summary>
	void Init( void );
    
    /// <summary cref="TaskManagerTBB::Shutdown">
	/// Call this from the primary thread as the last <c>TaskManagerTBB</c> call.
	/// </summary>
	void Shutdown( void );

    /// <summary cref="TaskManagerTBB::IssueJobsForSystemTasks">
	/// Call this from the primary thread to schedule system work.
	/// </summary>
	/// <param name="pTasks">an array of <c>ISystemTask</c> objects which should have their
	/// <c>Update()</c> methods called asynchronously</param>
	/// <param name="uCount">the size of the <paramref name="pTasks"/> array</param>
	/// <param name="fDeltaTime">amount of time to be passed to each system's <c>Update</c> method</param>
	/// <seealso cref="TaskManagerTBB::WaitForSystemTasks"/>
	/// <seealso cref="ISystemTask::Update"/>
	void IssueJobsForSystemTasks( ISystemTask** pTasks, u32 uCount, f32 fDeltaTime );

    /// <summary cref="TaskManagerTBB::NonStandardPerThreadCallback">
    /// This method triggers a synchronized callback to be called once by each thread used by the <c>TaskManagerTBB</c>.
    /// This method waits until all callbacks have executed.
    /// </summary>
    /// <remarks>Unlike the general contract for this method, this method can be safely called at any time,
    /// as long as the call is made from the primary thread.</remarks>
    /// <param name="pfnCallback">the function callback to execute</param>
    /// <param name="pData">a pointer to data that is passed to the callback</param>
    virtual void NonStandardPerThreadCallback( JobFunction pfnCallback, void* pData );

    /// <summary cref="TaskManagerTBB::GetRecommendedJobCount">
	/// Call this method to determine the ideal number of tasks to submit to the <c>TaskManagerTBB</c>
	/// for maximum performance.
    /// </summary>
    /// <remarks><paramref name="Hints"/> is ignored and the number of active threads is always returned.</remarks>
	/// <param name="Hints">guidance on the type of work done in the jobs about to be submitted</param>
	/// <returns>the number of jobs which is optimal for the type of work specified by <paramref name="Hints"/>
	/// </returns>
	virtual u32 GetRecommendedJobCount( ITaskManager::JobCountInstructionHints Hints );

    virtual void ParallelFor( ISystemTask* pSystemTask,
                              ParallelForFunction pfnJobFunction, void* pParam, u32 begin, u32 end, u32 minGrainSize = 1 );

    /// <summary cref="TaskManagerTBB::WaitForSystemTasks">
	/// Call this from the primary thread to wait until specified tasks spawned with <c>IssueJobsForSystemTasks</c>
	/// and all of their subtasks are complete.
	/// </summary>
	/// <param name="pTasks">an array of <c>ISystemTask</c> objects</param>
	/// <param name="uCount">the length of the <paramref name="pTasks"/> array</param>
	/// <seealso cref="TaskManagerTBB::IssueJobsForSystemTasks"/>
	void WaitForSystemTasks( ISystemTask** pTasks, u32 uCount );

    /// <summary cref="TaskManagerTBB::GetNumberOfThreads">
	/// Call this method to get the number of threads in the thread pool which are active for running work.
	/// </summary>
	/// <returns>the number of threads being used</returns>
	u32 GetNumberOfThreads( void );
	
	/// <summary cref="TaskManagerTBB::SetNumberOfThreads">
	/// This method constrains the number of threads used by the <c>TaskManagerTBB</c>.
    /// </summary>
	/// <param name="uNumberOfThreads">the limit of the number of threads to use</param>
	void SetNumberOfThreads( u32 uNumberOfThreads );

	Bool IsPrimaryThread( void );

	void AddStallTask( void );

    static void SystemTaskCallback( void* pData );

private:
	tbb::tbb_thread::id m_uPrimaryThreadID;

	Handle m_hStallPoolSemaphore;
    tbb::task* m_pStallPoolParent;
	SpinWait m_tSynchronizedCallbackMutex;

    tbb::task_scheduler_init* m_pTbbScheduler;

#if USE_MAIN_THREAD_FOR_SERIAL_TASKS_ONLY
    // IMPLEMENTATION NOTE
    // Experiments showed that temporary oversubscription during the execution 
    // of primary thread bound system tasks is even beneficial rather than adverse.
    //
    // Therefore the TBB pool is created with 1 extra thread, and the master thread 
    // after completing primary thread bound tasks is blocked on the Windows 
    // synchronization primitive instead instead of entering wait_for_all method
    // (to prevent oversubscription during the execution of the main bulk of work)
    // 
    // Doing so requires two "root" tasks instead of 1.
    tbb::empty_task* m_pRootTask;
#endif
    tbb::task* m_pSystemTasksRoot;

    Bool m_bTimeToQuit;
	
	// requested, maximum and current number of threads in use
	u32 m_uRequestedNumberOfThreads;
	u32 m_uMaxNumberOfThreads;
	u32 m_uNumberOfThreads;
	u32 m_uTargetNumberOfThreads;

#if defined(USE_THREAD_PROFILER)
	Bool m_bTPEventsForTasks;
	Bool m_bTPEventsForJobs;
	Bool m_bTPEventsForSynchronize;

	__itt_event m_tSynchronizeTPEvent;
	__itt_event m_tpSystemTaskSpawn;

    /// <summary cref="TaskManagerTBB::SystemTaskSupport">
    /// This class is used by <c>TaskManagerTBB</c> to keep <c>SystemTasks</c> associated with <c>ISystemTasks</c>.
    /// </summary>
    /// <seealso cref="TaskManagerTBB::SystemTask"/>
    struct SystemTaskSupport
	{
        SystemTaskSupport()
            : m_bInitialized( False )
            , m_tpeSystemTask( 0 )
            , m_tpeSystemTaskJob( 0 )
            , m_tpeSystemTaskJobCompletion( 0 )
        {}

        Bool m_bInitialized;
		__itt_event m_tpeSystemTask;
		__itt_event m_tpeSystemTaskJob;
		__itt_event m_tpeSystemTaskJobCompletion;
	};

    std::map<ISystemTask*, SystemTaskSupport> m_SupportForSystemTasks;
#endif /* USE_THREAD_PROFILER */

    
    typedef std::vector<ISystemTask*> SystemTasksList;

    // Holds system tasks that should be run on the primary thread
    SystemTasksList m_primaryThreadSystemTaskList;
    SystemTasksList m_tmpTaskList;

    f32 m_fDeltaTime;

    typedef std::vector<tbb::task::affinity_id> AffinityIDsList;
    AffinityIDsList m_affinityIDs;

    SpinWait m_spinMutex;

    static void InitAffinityData( void* mgr );

#if defined(USE_THREAD_PROFILER)
    /// <summary cref="TaskManagerTBB::GetSupportForSystemTask">
    /// This method returns the associated <c>SystemTaskSupport</c> for the <c>ISystemTask</c>.
    /// </summary>
    /// <param name="pTask">an <c>ISystemTask</c>
    /// <returns>the associated <c>SystemTaskSupport</c> object
    SystemTaskSupport& GetSupportForSystemTask( ISystemTask* pTask );
#endif /* USE_THREAD_PROFILER */

	/// <summary cref="TaskManagerTBB::IsTBBThread">
    /// This method is used to determine if the calling thread is an Intel Threading Building Blocks thread.
    /// </summary>
    /// <remarks>Due to the difficulty using the Intel Threading Building Blocks API to determine this, and
    /// since no thread local storage is used as a workaround, this method always returns true.</remarks>
    /// <returns>true if the calling thread is managed by Intel Theading Building Blocks, false otherwise</returns>
    static Bool IsTBBThread( void );

    /// <summary cref="TaskManagerTBB::UpdateThreadPoolSize">
	/// This method is called within <c>IssueJobsForSystemTasks</c> to update the size of TBB thread pool
	/// if necessary.  The thread pool is resized by a call to <c>SetNumberOfThreads</c>.
    /// </summary>
    /// <seealso cref="TaskManagerTBB::SetNumberOfThreads"/>
	/// <seealso cref="TaskManagerTBB::IssueJobsForSystemTasks"/>
	void UpdateThreadPoolSize( void );
};
