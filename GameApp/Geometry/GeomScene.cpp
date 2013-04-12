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
#include "GeomScene.h"
#include "GeomObject.h"


GeometryScene::GeometryScene(
    ISystem* pSystem
    )
    : ISystemScene( pSystem )
{
}


GeometryScene::~GeometryScene(
    void
    )
{
    //
    // Free all the remaining objects.
    //
    for ( std::list<GeometryObject*>::iterator it=m_Objects.begin();
          it != m_Objects.end(); it++ )
    {
        delete *it;
    }

    m_Objects.clear();
}


System::Type
GeometryScene::GetSystemType(
    void
    )
{
    return System::Types::Geometry;
}


Error
GeometryScene::Initialize(
    std::vector<Properties::Property> Properties
    )
{
    ASSERT( !m_bInitialized );

	m_bInitialized = True;

    return Errors::Success;
}


void
GeometryScene::GetProperties(
    Properties::Array& Properties
    )
{
    UNREFERENCED_PARAM( Properties );
}


void
GeometryScene::SetProperties(
    Properties::Array Properties
    )
{
    UNREFERENCED_PARAM( Properties );

    ASSERT( m_bInitialized );
}

    
pcstr*
GeometryScene::GetObjectTypes(
    void
    )
{
    return NULL;
}


ISystemObject*
GeometryScene::CreateObject(
    pcstr pszName,
    pcstr pszType
    )
{
    UNREFERENCED_PARAM( pszName );
    UNREFERENCED_PARAM( pszType );

    ASSERT( m_bInitialized );
    ASSERT( pszType == NULL );

    //
    // Create the object and add it to the object list.
    //
    GeometryObject* pObject = new GeometryObject( this );

    if ( pObject != NULL )
    {
        m_Objects.push_back( pObject );
    }

    return pObject;
}


Error
GeometryScene::DestroyObject(
    ISystemObject* pSystemObject
    )
{
    ASSERT( m_bInitialized );
    ASSERT( pSystemObject != NULL );

    //
    // Cast to a GeometryObject so that the correct destructor will be called.
    //
    GeometryObject* pObject = reinterpret_cast<GeometryObject*>(pSystemObject);

    //
    // Remove the object from the list and delete it.
    //
    m_Objects.remove( pObject );

    SAFE_DELETE( pSystemObject );

    return Errors::Success;
}


ISystemTask*
GeometryScene::GetSystemTask(
    void
    )
{
    return NULL;
}


System::Changes::BitMask
GeometryScene::GetPotentialSystemChanges(
    void
    )
{
    return System::Changes::None;
}


const void*
GeometryScene::GetSystemChangeData(
    System::Change SystemChange
    )
{
    UNREFERENCED_PARAM( SystemChange );

    return NULL;
}
