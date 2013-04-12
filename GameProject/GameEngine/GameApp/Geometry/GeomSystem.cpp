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

#include "stdafx.h"
//
// core includes
//
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

//
// geometry system includes
//
#include "GeomSystem.h"
#include "GeomScene.h"
#include "GeomObject.h"


GeometrySystem::GeometrySystem(
    void
    )
    : ISystem()
{
}


GeometrySystem::~GeometrySystem(
    void
    )
{
}


pcstr
GeometrySystem::GetName(
    void
    )
{
    return System::Names::Geometry;
}


System::Type
GeometrySystem::GetSystemType(
    void
    )
{
    return System::Types::Geometry;
}


Error
GeometrySystem::Initialize(
    Properties::Array Properties
    )
{
    ASSERT( !m_bInitialized );

	m_bInitialized = True;

    return Errors::Success;
}


void
GeometrySystem::GetProperties(
    Properties::Array& Properties
    )
{
    UNREFERENCED_PARAM( Properties );
}


void
GeometrySystem::SetProperties(
    Properties::Array Properties
    )
{
    ASSERT( m_bInitialized );
}


ISystemScene*
GeometrySystem::CreateScene(
    void
    )
{
    return new GeometryScene( this );
}


Error
GeometrySystem::DestroyScene(
    ISystemScene* pSystemScene
    )
{
    ASSERT( pSystemScene != NULL );

    GeometryScene* pScene = reinterpret_cast<GeometryScene*>(pSystemScene);
    SAFE_DELETE( pScene );

    return Errors::Success;
}
