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
// core includes
//
#include "stdafx.h"
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

#include "FireSystem.h"
#include "FireScene.h"

ManagerInterfaces	g_FireManagers;


FireSystem::FireSystem(
    void
    )
    : ISystem()
{
}


FireSystem::~FireSystem(
    void
    )
{

}


pcstr
FireSystem::GetName(
    void
    )
{
    return "ProceduralFire";
}


System::Type
FireSystem::GetSystemType(
    void
    )
{
    return System::Types::MakeCustom(2);  // 2  claimed for Procedural Fire
}


Error
FireSystem::Initialize(
    Properties::Array Properties
    )
{
	m_bInitialized = True;

    return Errors::Success;
}


void
FireSystem::GetProperties(
    Properties::Array& Properties
    )
{
    UNREFERENCED_PARAM( Properties );
}


void
FireSystem::SetProperties(
    Properties::Array Properties
    )
{
    UNREFERENCED_PARAM( Properties );

    ASSERT( m_bInitialized );
}



ISystemScene*
FireSystem::CreateScene(
    void
    )
{
    return new FireScene( this );
}


Error
FireSystem::DestroyScene(
    ISystemScene* pSystemScene
    )
{
    ASSERT( pSystemScene != NULL );

    FireScene* pScene = reinterpret_cast<FireScene*>(pSystemScene);
    SAFE_DELETE( pScene );

    return Errors::Success;
}
