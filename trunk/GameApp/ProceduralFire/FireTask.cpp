//Writen by Hugh Smith summer and fall 2007
// Part of Smoke Framework implementation for procedural Fire system.

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

//
// extern includes
//


//
// core includes
//
#include "stdafx.h"
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"
#include "..\Interfaces\Environment.h"

//
// Ogre system includes
//
#include "FireScene.h"
#include "FireObject.h"
#include "FireTask.h"

extern ManagerInterfaces   g_FireManagers;


// Grain sizes of the parallel jobs. The less is the grainsize the better is 
// the load balance, and the higher is the parallel overhead. 
static const u32    FireSystemTaskGrainSize = 8;

#if FIRETASK_PARALLEL_PREBUILD_VERTICES
    static const u32   BuildVetrexBuffersGrainSize = 40;
#endif


FireTask::FireTask(
    FireScene* pScene
    )
	: ISystemTask( pScene )
    , m_pScene( pScene )
{
}


FireTask::~FireTask(
    void
    )
{
}


System::Type
FireTask::GetSystemType(
    void
    )
{
    return System::Types::MakeCustom( 2 );
}


void
FireTask::Update(
    f32 fDeltaTime
    )
{
    if ( fDeltaTime <= 0.0f )
    {
        return;
    }
    m_fDeltaTime = fDeltaTime;
 
    u32 size = (u32)m_pScene->m_FireObjects.size();

	if (
		m_pScene->m_bParallelize &&
		( g_FireManagers.pTask != NULL ) &&
		( FireSystemTaskGrainSize < size )
	)
    {
        g_FireManagers.pTask->ParallelFor( this, UpdateCallback, this, 0, size, FireSystemTaskGrainSize );
	}
	else
	{
        UpdateRange( 0, size );
    }

#if FIRETASK_PARALLEL_PREBUILD_VERTICES
	if (
		m_pScene->m_bParallelize &&
		( g_FireManagers.pTask != NULL ) &&
		( BuildVetrexBuffersGrainSize < size )
	)
    {
        g_FireManagers.pTask->ParallelFor( this, BuildVertexBuffersCallback, this, 0, size, BuildVetrexBuffersGrainSize );
	}
	else
	{
        BuildVertexBuffersRange( 0, size );
    }
#endif /* FIRETASK_PARALLEL_PREBUILD_VERTICES */
}


void FireTask::UpdateCallback( void *param, u32 begin, u32 end )
{
    FireTask *pThis = static_cast<FireTask*>(param);
    pThis->UpdateRange( begin, end );
}


void FireTask::UpdateRange( u32 begin, u32 end )
{
    FireObjectList &fireObjects = m_pScene->m_FireObjects;
    ASSERT (end <= fireObjects.size());
    for ( u32 i = begin; i < end; ++i )
    {
        fireObjects[i]->update(m_fDeltaTime, fireObjects);
    }
}

#if FIRETASK_PARALLEL_PREBUILD_VERTICES
void FireTask::BuildVertexBuffersCallback( void *param, u32 begin, u32 end )
{
    FireTask *pThis = static_cast<FireTask*>(param);
    pThis->BuildVertexBuffersRange( begin, end );
}

void FireTask::BuildVertexBuffersRange( u32 begin, u32 end )
{
    FireObjectList &fireObjects = m_pScene->m_FireObjects;
    ASSERT (end <= fireObjects.size());
    for ( u32 i = begin; i < end; ++i )
    {
        fireObjects[i]->BuildVertexBuffer();
    }
}
#endif /* FIRETASK_PARALLEL_PREBUILD_VERTICES */
