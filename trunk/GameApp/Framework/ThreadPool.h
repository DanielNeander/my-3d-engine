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

/// <summary>
/// This class is used by <c>TaskManagerTP</c> wrap tasks into objects that can be
/// enqueued and assigned to a thread.
/// </summary>
/// <seealso cref="TaskManagerTP"/>
class WorkItem
{
public:

	/// <summary cref="WorkItem::Status">
	/// This type represents the state of the <c>WorkItem</c>.
	/// </summary>
    enum Status
    {
        Undefined=-1, Executing, Completed
    };

protected:

    Status      m_Status;

public:

    /// <summary cref="WorkItem::WorkItem">
	/// This is the basic constructor for <c>WorkItem</c>.
	/// </summary>
    WorkItem( void )
        : m_Status( Status::Undefined )
        {}

    /// <summary cref="WorkItem::Process">
	/// This abstract method will be called from a thread in the thread pool managed by <c>TaskManagerTP</c>.
	/// </summary>
    virtual void Process( void ) = 0;

    /// <summary cref="WorkItem::GetStatus">
	/// This method returns the execution status of this <c>WorkItem</c>.
	/// </summary>
	/// <returns>current status of this <c>WorkItem</c></returns>
    Status GetStatus() const
    {
        return m_Status;
    }

	virtual Bool IsDynamicallyAllocated( void ) = 0;

    /// <summary cref="WorkItem::SetStatus">
	/// This method is called by <c>TaskManagerTP</c> to change the status of this <c>WorkItem</c>.
	/// </summary>
	/// <param name="inStatus">the new status</param>
	virtual void SetStatus( Status inStatus )
    {
        m_Status = inStatus;
    }
};  // WorkItem

typedef std::queue<WorkItem*>       WorkQueue;

class BasicThreadPool;

/// <summary>
/// This class wraps up all the events and status variables necessary to maintain a thread
/// in the thread pool managed by <c>TaskManagerTP</c>.  All members are public to simplify access.
/// </summary>
/// <seealso cref="TaskManagerTP"/>
class ThreadContext
{
public:

	/// <summary cref="WorkItem::Status">
	/// This type is used to indicate state of the <c>ThreadContext</c>.
	/// </summary>
	enum Status
    {
        Callback, Shutdown
    };

	/// <summary cref="ThreadContext::ThreadCallback">
	/// This type is used for arbitrary callbacks dispatched from the <c>BasicThreadPool</c>.
	/// </summary>
	/// <seealso cref="BasicThreadPool::NonStandardThreadExecute"/>
	typedef void (*ThreadCallback)( void* pThreadData );

    BasicThreadPool*	pThreadPool;
    Handle				hStatusEvent;
    Handle				hWorkQueueSemaphore;	
	Handle              hSleepEvent;
	Handle              hWakeEvent;

    Status              status;

    ThreadCallback      pfnThreadCallback;
    void*               pThreadData;
    u32*                pCallbackCountdown;
}; // ThreadContext

/// <summary>
/// This class handles the actual management of the thread pool for <c>TaskManagerTP</c>.
/// </summary>
/// <seealso cref="TaskManagerTP"/>
class BasicThreadPool
{
protected:

    Handle				m_hWorkQueueSemaphore;	
    Handle*			    m_hWorkerThreads;
    u32*			    m_hWorkerthreadIds;
    u32			        m_NumThreads;
	u32                 m_NumActiveThreads;
    ThreadContext*	    m_pThreadContext;
    Bool			    m_bInitialized;
    WorkQueue*          m_pWorkQueue;
    SpinWait		    m_QueueSW;

    u32                 m_CallbackCountdown;

public:

    /// <summary cref="BasicThreadPool::BasicThreadPool">
	/// This is the basic constructor for <c>BasicThreadPool</c>.
	/// </summary>
    BasicThreadPool( void )
        : m_hWorkerThreads( 0 )
        , m_hWorkerthreadIds( 0 )
        , m_NumThreads( 0 )
        , m_pThreadContext( 0 )
        , m_bInitialized( False )
        , m_NumActiveThreads( 0)
		{}

    /// <summary cref="BasicThreadPool::Initialize">
	/// Call this to create the threads for the thread pool.
	/// </summary>
	/// <param name="inNumThreads">the number of threads to create, or 0 to request
	/// an optimum number of threads, based upon the number of logical processors</param>
    void Initialize( u32 inNumThreads = 0 );
    
	/// <summary cref="BasicThreadPool::Shutdown">
	/// Call this to ensure that all of the threads in the thread pool exit properly.
	/// </summary>
	void Shutdown( void );

    /// <summary cref="BasicThreadPool::GetThreadCount">
	/// This method returns the number of threads in the thread pool.  Not all of these threads
	/// may be enabled to run <c>WorkItems</c>.
	/// </summary>
	/// <returns>the number of threads in the thread pool</returns>
	/// <seealso cref="BasicThreadPool::Initialize"/>
	/// <seealso cref="BasicThreadPool::GetActiveThreadCount"/>
	u32 GetThreadCount( void )
    {
        return m_NumThreads;
    }

	/// <summary cref="BasicThreadPool::GetActiveThreadCount">
	/// This method returns the number of threads in the thread pool which are actually enabled.
	/// </summary>
	/// <returns>the number of threads in the thread pool which are actually enabled</returns>
	u32 GetActiveThreadCount( void )
	{
		return m_NumActiveThreads;
	}

	/// <summary cref="BasicThreadPool::SetActiveThreadCount">
	/// Call this method to constrain the number of threads which are enabled to run <c>WorkItems</c>.
	/// </summary>
	/// <param name="uCount">the number of threads in the thread pool to enable</param>
	void SetActiveThreadCount( u32 uCount );

    /// <summary cref="BasicThreadPool::QueueUserWorkItem">
	/// Call this method to put a <c>WorkItem</c> on the queue for execution.
	/// </summary>
	/// <param name="pItem">the <c>WorkItem</c> to enqueue</param>
	void QueueUserWorkItem(WorkItem* pItem);

    /// <summary cref="BasicThreadPool::RemoveWorkItem">
	/// This method pulls and returns a <c>WorkItem</c> from the execution queue.
	/// </summary>
	/// <returns>a <c>WorkItem</c> on the execution queue</returns>
	WorkItem* RemoveWorkItem( void );	

    /// <summary cref="BasicThreadPool::NonStandardThreadExecute">
	/// Call this method from the primary thread to wait until each thread in the thread pool,
	/// including the disabled threads, call the specificied callback method.
	/// </summary>
	/// <param name="pfnCallback">the callback that each thread should call</param>
	/// <param name="pData">the data that is passed to the <paramref name="pfnCallback"/></param>
	/// <seealso cref="TaskManagerTP::NonStandardPerThreadCallback"/>
	void NonStandardThreadExecute( ThreadContext::ThreadCallback pfnCallback, void* pData );

	void PopAndProcessWorkItem( Bool bProcess = True );

private:

	/// <summary cref="BasicThreadPool::WorkerThreadProc">
	/// This method is called by threads in the thread pool, and will call the <c>Process</c> method
	/// of a <c>WorkItem</c>.
	/// </summary>
	/// <param name="pThreadContext">the <c>ThreadContext</c> for the calling thread</param>
	/// <returns>always 0</returns>
	/// <seealso cref="WorkItem::Process"/>
	static u32 __stdcall WorkerThreadProc( void* pThreadContext );

    friend class WorkItem;
}; // BasicThreadPool
