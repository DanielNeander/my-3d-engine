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


#include "ThreadPool.h"
#include "TaskManager.h"


/// <summary>
/// This class is used by <c>TaskManagerTP</c> to run a <c>ISystemTask</c> <c>Update</c> method.
/// </summary>
/// <seealso cref="TaskManagerTP"/>
class SystemTaskWrapper : public WorkItem
{
public:

    /// <summary cref="SystemTaskWrapper::Process">
	///	This method calls the <c>Update</c> method of the associated <c>ISystemTask</c>
	/// with some instrumentation to record the time taken for the call.
	/// </summary>
	void Process( void );

	/// <summary cref="SystemTaskWrapper::SetStatus">
	/// <c>TaskManagerTP</c> calls this method to initiate the call to the associated
	/// completion fucntion.
	/// </summary>
	/// <param name="inStatus">the new status</param>
    void SetStatus( WorkItem::Status inStatus );

	Bool IsDynamicallyAllocated( void ) { return False; };

	Handle* hWorkCompleted;
    ISystemTask* pSystemTask;
    f32 deltaTime;

    ITaskManager::JobCompletionFunction pfnJobCompletionFunction;
    void* pCompletionUserData;
};


/// <summary>
/// This class is used by <c>TaskManagerTP</c> to run a parallel for method.
/// </summary>
/// <seealso cref="TaskManagerTP"/>
class SystemParallelForWrapper : public WorkItem
{
public:

    /// <summary cref="SystemParallelForWrapper::Process">
	///	This method calls the parallel for method
	/// with some instrumentation to record the time taken for the call.
	/// </summary>
	void Process( void );

	Bool IsDynamicallyAllocated( void ) { return True; };

    SystemTaskWrapper* pParentTask;
    ITaskManager::ParallelForFunction pfnParallelForFunction;
    void* pUserData;
    u32 uStart;
	u32 uEnd;
	volatile long* lJobWaitCount;
};


/// <summary>
/// This class uses a pool of native Windows threads to run tasks.
/// </summary>
class TaskManagerTP : public TaskManager
{
    Handle m_hSystemTaskEvents[ System::Types::MAX ];
    Handle m_hActiveSystemTasksInQ[ System::Types::MAX ];
    u32 m_cActiveSystemTasksInQ;
    SystemTaskWrapper m_TaskQ[ System::Types::MAX ];

    std::list<SystemParallelForWrapper*> m_Jobs;

    BasicThreadPool* m_pThreadPool;


public:

	/// <summary cref="TaskManagerTP::Init">
	/// Call this from the primary thread before calling any other <c>TaskManagerTP</c> methods.
	/// </summary>
	void Init( void );

	/// <summary cref="TaskManagerTP::Shutdown">
	/// Call this from the primary thread as the last <c>TaskManagerTP</c> call.
	/// </summary>
	void Shutdown( void );

    /// <summary cref="TaskManagerTP::IssueJobsForSystemTasks">
	/// Call this from the primary thread to schedule system work.
	/// </summary>
	/// <param name="pTasks">an array of <c>ISystemTask</c> objects which should have their
	/// <c>Update()</c> methods called asynchronously</param>
	/// <param name="uCount">the size of the <paramref name="pTasks"/> array</param>
	/// <param name="fDeltaTime">amount of time to be passed to each system's <c>Update</c> method</param>
	/// <seealso cref="TaskManagerTP::WaitForSystemTasks"/>
	/// <seealso cref="ISystemTask::Update"/>
	void IssueJobsForSystemTasks( ISystemTask** pTasks, u32 uCount, f32 fDeltaTime );
    
	/// <summary cref="TaskManagerTP::WaitForAllSystemTasks">
	/// Call this from the primary thread to wait until all tasks spawned with <c>IssueJobsForSystemTasks</c>
	/// and all of their subtasks are complete.
	/// </summary>
	/// <seealso cref="TaskManagerTP::IssueJobsForSystemTasks"/>
	void WaitForAllSystemTasks( void );
    
	/// <summary cref="TaskManagerTP::WaitForSystemTasks">
	/// Call this from the primary thread to wait until specified tasks spawned with <c>IssueJobsForSystemTasks</c>
	/// and all of their subtasks are complete.
	/// </summary>
	/// <remarks>
	/// This method ignores its parameters and waits for all system tasks.
	/// This is adequate if each system is designed to run exactly once per frame.
	/// </remarks>
	/// <param name="pTasks">an array of <c>ISystemTask</c> objects</param>
	/// <param name="uCount">the length of the <paramref name="pTasks"> array</param>
	/// <seealso cref="TaskManagerTP::IssueJobsForSystemTasks"/>
	void WaitForSystemTasks( ISystemTask** pTasks, u32 uCount );

    /// <summary cref="TaskManagerTP::GetNumberOfThreads">
	/// Call this method to get the number of threads in the thread pool which are active for running work.
	/// </summary>
	/// <returns>the number of threads being used</returns>
	u32 GetNumberOfThreads( void );
	
	/// <summary cref="TaskManagerTP::SetNumberOfThreads">
	/// This method constrains the number of threads used by the <c>TaskManagerTP</c>.
    /// </summary>
	/// <param name="uNumberOfThreads">the limit of the number of threads to use</param>
	void SetNumberOfThreads( u32 uNumberOfThreads );

	/// <summary cref="TaskManagerTP::NonStandardPerThreadCallback">
    /// This method triggers a synchronized callback to be called once by each thread used by the <c>TaskManagerTP</c>.
    /// This method which should only be called during initialization and shutdown of 
	/// the <c>TaskManagerTP</c>.  This method waits until all callbacks have executed.
    /// </summary>
    /// <param name="pfnCallback">the function callback to execute</param>
    /// <param name="pData">a pointer to data that is passed to the callback</param>
    virtual void NonStandardPerThreadCallback( JobFunction pfnCallback, void* pData );

    /// <summary cref="TaskManagerTP::GetRecommendedJobCount">
	/// Call this method to determine the ideal number of tasks to submit to the <c>TaskManagerTP</c>
	/// for maximum performance.
    /// </summary>
    /// <remarks><paramref name="Hints"/> is ignored and the number of active threads is always returned.</remarks>
	/// <param name="Hints">guidance on the type of work done in the jobs about to be submitted</param>
	/// <returns>the number of jobs which is optimal for the type of work specified by <paramref name="Hints"/>
	/// </returns>
	virtual u32 GetRecommendedJobCount( ITaskManager::JobCountInstructionHints Hints );

    virtual void ParallelFor( ISystemTask* pSystemTask,
                              ParallelForFunction pfnJobFunction, void* pParam, u32 begin, u32 end, u32 minGrainSize = 1 );
};
