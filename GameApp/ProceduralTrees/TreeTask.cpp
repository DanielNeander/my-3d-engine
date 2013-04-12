//Writen by Hugh Smith summer and fall 2007
// Part of Smoke Framework implementation for procedural Tree system.

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
// core includes
//
#include "stdafx.h"
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

//
// Ogre system includes
//
#include "TreeScene.h"
#include "TreeObject.h"
#include "TreeTask.h"


TreeTask::TreeTask(
    TreeScene* pScene
    )
    : ISystemTask( pScene )
    , m_pScene( pScene )
{
}


TreeTask::~TreeTask(
    void
    )
{
}


System::Type
TreeTask::GetSystemType(
    void
    )
{
    return System::Types::MakeCustom( 0 );
}


void
TreeTask::Update(
    f32 DeltaTime
    )
{
    //DBG_UNREFERENCED_PARAM( DeltaTime );

    std::list<TreeObject*>::iterator i, iend;
    i = m_pScene->m_Forest.begin();
    iend = m_pScene->m_Forest.end();
    for ( i;i!=iend;i++){
        (*i)->update(DeltaTime);
    }
    
    if ( DeltaTime <= 0.0f )
    {
        return;
    }


}


//void
//TreeTask::UpdateChanges(
//    int SpaceId
//    )
//{
//}

