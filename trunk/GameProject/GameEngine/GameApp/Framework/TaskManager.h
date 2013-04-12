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
#include "../BaseTypes/BaseTypes.h"
#include "../Interfaces/Interface.h"

/// <summary>
/// This abstract class provides a ways for Systems to supply work to run asynchronously.
/// Specifically, each <c>ISystemTask</c> uses a <c>TaskManager</c> to execute its <c>Update</c> method.
/// Only one <c>TaskManager</c> should be created for a process.
/// </summary>
/// <seealso cref="TaskManagerTP"/>
/// <seealso cref="TaskManagerTBB"/>
class TaskManager: public ITaskManager
{

public:
	/// <summary cref="TaskManager::Init">
	/// Call this from the primary thread before calling any other <c>TaskManager</c> methods.
	/// </summary>
	virtual void Init( void ) = 0;
	
	/// <summary cref="TaskManager::Shutdown">
	/// Call this from the primary thread as the last <c>TaskManager</c> call.
	/// </summary>
	virtual void Shutdown( void )=0;

	/// <summary cref="TaskManager::IssueJobsForSystemTasks">
	/// Call this from the primary thread to schedule system work.
	/// </summary>
	/// <param name="pTasks">an array of <c>ISystemTask</c> objects which should have their
	/// <c>Update</c> methods called asynchronously</param>
	/// <param name="uCount">the size of the <paramref name="pTasks"/> array</param>
	/// <param name="fDeltaTime">amount of time to be passed to each system's <c>Update</c> method</param>
	/// <seealso cref="TaskManager::WaitForSystemTasks"/>
	/// <seealso cref="ISystemTask::Update"/>
	virtual void IssueJobsForSystemTasks( ISystemTask** pTasks, u32 uCount, f32 fDeltaTime )=0;
	
	/// <summary cref="TaskManager::WaitForSystemTasks">
	/// Call this from the primary thread to wait until all tasks spawned with <c>IssueJobsForSystemTasks</c>
	/// and all of their subtasks are complete.
	/// </summary>
	/// <param name="pTasks">an array of <c>ISystemTask</c> objects</param>
	/// <param name="uCount">the length of the <paramref name="pTasks"/> array</param>
	/// <seealso cref="TaskManager::IssueJobsForSystemTasks"/>
	virtual void WaitForSystemTasks( ISystemTask** pTasks, u32 uCount )=0;
};
