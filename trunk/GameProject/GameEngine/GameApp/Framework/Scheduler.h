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

class TaskManager;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
///   Handles scheduling of all task execution.
/// </summary>
////////////////////////////////////////////////////////////////////////////////////////////////////

class Scheduler
{
public:

    /// <summary>
    ///   Constructor.
    /// </summary>
    /// <param name="pTaskManager">A pointer to the task manager.</param>
	Scheduler( 
		TaskManager* pTaskManager
	 )throw ( ... );

    /// <summary>
    ///   Destructor.
    /// </summary>
    ~Scheduler( void );

    /// <summary>
    ///   Turns benchmarking on or off.
    /// </summary>
    void EnableBenchmarking( Bool bEnable=True )
    {
        m_bBenchmarkingEnabled = bEnable;
    }

    /// <summary>
    ///   Turns threading on or off.
    /// </summary>
    void EnableThreading( Bool bEnable=True )
    {
        m_bThreadingEnabled = bEnable;
    }

    /// <summary>
    ///   Sets the UScene to schedule execution of.
    /// </summary>
    /// <param name="pScene">A pointer to the universal scene.</param>
    void SetScene( const UScene* pScene );

    /// <summary>
    ///   Execute the set UScene.
    /// </summary>
    void Execute( void );


protected:

    static const f32                sm_DefaultClockFrequency;
	TaskManager*                    m_pTaskManager;

    f32                             m_ClockFrequency;
    Handle                          m_hExecutionTimer;

    Bool                            m_bBenchmarkingEnabled;
    Bool                            m_bThreadingEnabled;

    typedef std::map<System::Type, ISystemScene*>   SceneExecs;
    typedef SceneExecs::iterator                    SceneExecsIt;

    SceneExecs                      m_SceneExecs;
};
