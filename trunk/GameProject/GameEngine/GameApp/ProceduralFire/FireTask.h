//Writen by Hugh Smith summer and fall 2007
// Part of Smoke Framework implementation for procedural Fire system.

// Copyright © 2008-2009 Intel Corporation
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


class FireScene;

// Disabled since currently parallel prebuild is done as part of the FireObject update
#define FIRETASK_PARALLEL_PREBUILD_VERTICES 0

///////////////////////////////////////////////////////////////////////////////
/// <summary>
///   Implementation of the ISystemTask interface for the fire system.
///   See Interfaces\System.h for a definition of the class and its functions.    
/// </summary>
///////////////////////////////////////////////////////////////////////////////

class FireTask : public ISystemTask
{
    friend FireScene;


protected:

    FireTask( FireScene* pScene );
    ~FireTask( void );

    /// <summary cref="ISystemScene::GetSystemType">
    ///   Implementation of the ISystemScene GetSystemType function.
    /// </summary>
    virtual System::Type GetSystemType( void );

    /// <summary cref="ISystemTask::Update">
    ///   Function informing the task to perform its updates.
    /// </summary>
    /// <param name="DeltaTime">The time delta from the last call.</param>
    virtual void Update( f32 fDeltaTime );

    virtual Bool IsPrimaryThreadOnly( void ) { return False; } 

private:

    f32     m_fDeltaTime;
    
    /// <summary cref="FireTask::UpdateCallback">
    ///   Invoked by ParalellFor algorithm to update a range of objects.
    /// </summary>
	static void UpdateCallback( void *param, u32 begin, u32 end );

    /// <summary cref="FireTask::UpdateCallback">
    ///   Updates the given range of fire objects.
    /// </summary>
	void UpdateRange( u32 begin, u32 end );

#if FIRETASK_PARALLEL_PREBUILD_VERTICES
    /// <summary cref="FireTask::UpdateCallback">
    ///   Invoked by ParalellFor algorithm to build a range of vertex buffers.
    /// </summary>
	static void BuildVertexBuffersCallback( void *param, u32 begin, u32 end );

    /// <summary cref="FireTask::UpdateCallback">
    ///   Builds a range of vertex buffers.
    /// </summary>
	void BuildVertexBuffersRange( u32 begin, u32 end );
#endif /* FIRETASK_PARALLEL_PREBUILD_VERTICES */

private:

    FireScene*      m_pScene;
};
