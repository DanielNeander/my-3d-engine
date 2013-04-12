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

//
// extern includes
//
#include <windows.h>

//
// core includes
//
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

//
// system includes
//
#include "Scene.h"
#include "Object.h"
#include "Task.h"


///////////////////////////////////////////////////////////////////////////////
// ExplosionTask - Constructor
ExplosionTask::ExplosionTask( ExplosionScene* pScene ) : ISystemTask( pScene ) , m_pScene( pScene )
{
    ASSERT( m_pScene != NULL );
}


///////////////////////////////////////////////////////////////////////////////
// ~ExplosionTask - Destructor
ExplosionTask::~ExplosionTask( void )
{
}


///////////////////////////////////////////////////////////////////////////////
// GetSystemType - Returns System type for this Task
System::Type ExplosionTask::GetSystemType( void )
{
    return System::Types::Explosion;
}


///////////////////////////////////////////////////////////////////////////////
// Update - Update the Explosion system (this is where all the work gets done)
void ExplosionTask::Update( f32 DeltaTime )
{
    // Make sure DeltaTime isn't too large
    if ( DeltaTime > 0.04f )
	{
		DeltaTime = 0.04f;
	}

    // Update - process Explosions
    m_pScene->Update( DeltaTime );
}

