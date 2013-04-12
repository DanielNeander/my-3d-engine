/*!
    \file TaskScheduler.cpp

    TaskScheduler is a class that manages the distribution of task among the worker
    threads and the lifetime of the worker threads. It is created and used by the 
    TaskMgrSS.

    Copyright 2011 Intel Corporation
    All Rights Reserved

    Permission is granted to use, copy, distribute and prepare derivative works of this
    software for any purpose and without fee, provided, that the above copyright notice
    and this statement appear in all copies.  Intel makes no representations about the
    suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED ""AS IS.""
    INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
    INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
    INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
    assume any responsibility for any errors which may appear in this software nor any
    responsibility to update it.

*/
#pragma once

#include "spin_mutex.h"

#define CACHE_ALIGN __declspec(align(64))

  // Forward Declarations
class Thread;

  // The custom Windows threads backend scheduler.  The TaskScheduler
  // class manages the Task Sets that the program gives to the TaskMgr
  // API and manages the lifetime of the worker threads.
class TaskScheduler
{
public:
      // Constant to pass to the Init method
    static const int MAX_THREADS = -1;

      // Sets up the threads and events for the scheduler
	VOID Init(int thread_count = MAX_THREADS);

      // Shuts down the scheduler and closes the threads
	VOID Shutdown();

    VOID AddTaskSet( TASKSETHANDLE hSet, INT iTaskCount );
    VOID DecrementTaskCount() { _InterlockedDecrement((LONG*)&miTaskCount); }
     
      // Yields the main thread to the scheduler 
      // when it needs to wait for a Task Set to be completed
	VOID WaitForFlag( volatile BOOL *pFlag );

private:
    static DWORD WINAPI ThreadMain(VOID* thread_instance);

      // Called by ThreadMain to execute tasks until the scheduler 
      // is shutdown
    VOID ExecuteTasks();

      // Number of worker threads that have bene created
    INT             miThreadCount;
      // Per thread data
    HANDLE*         mpThreadData;
      // Handle to a Windows Event
	HANDLE          mhTaskAvailable;
      // If the scheduler is alive, don't re-init
    BOOL            mbAlive;

      // Keep track of the number of avaiable tasks 
    CACHE_ALIGN volatile INT    miTaskCount;

    CACHE_ALIGN volatile INT    miWriter;
    CACHE_ALIGN UINT            muContextId;

      // Array that containing all tasks
    TASKSETHANDLE   mhActiveTaskSets[MAX_TASKSETS];
};
