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
// ODE system includes
//
#include "TreeSystem.h"
#include "TreeScene.h"
#include "TreeTask.h"
#include "TreeObject.h"
//
//
//
//pcstr TreeScene::sm_kapszPropertyNames[] =
//{
//    "Gravity",
//};
//
//const Properties::Property TreeScene::sm_kaDefaultProperties[] =
//{
//    Properties::Property( sm_kapszPropertyNames[ Property_Gravity ],
//                          Properties::Values::Vector3,
//                          Properties::Flags::Valid,
//                          Math::Vector3::Zero ),
//};


TreeScene::TreeScene(
    ISystem* pSystem
    )
    : ISystemScene( pSystem )
{
    //ASSERT( Property_Count == sizeof sm_kapszPropertyNames / sizeof sm_kapszPropertyNames[ 0 ] );
    //ASSERT( Property_Count == sizeof sm_kaDefaultProperties / sizeof sm_kaDefaultProperties[ 0 ] );

    //
    // Create the task for simulating the physics.
    //
    m_pTask = new TreeTask( this );
    ASSERT( m_pTask != NULL );
}


TreeScene::~TreeScene(
    void
    )
{
    SAFE_DELETE( m_pTask );

}


System::Type
TreeScene::GetSystemType(
    void
    )
{
    return System::Types::MakeCustom( 0 );
}


Error 
TreeScene::Initialize( std::vector<Properties::Property> Properties )
{
    return Errors::Success;
}


void
TreeScene::GetProperties(
    Properties::Array& Properties
    )
{
    UNREFERENCED_PARAM( Properties );

    ////
    //// Get the index of our first item.
    ////
    //i32 iProperty = static_cast<i32>(Properties.size());

    ////
    //// Add all the properties.
    ////
    //Properties.reserve( Properties.size() + Property_Count );

    //for ( i32 i=0; i < Property_Count; i++ )
    //{
    //    Properties.push_back( sm_kaDefaultProperties[ i ] );
    //}

    ////
    //// Modify the default values.
    ////
}


void
TreeScene::SetProperties(
    Properties::Array Properties
    )
{

    //
    // Read in the properties.
    //
    //for ( Properties::Iterator it=Properties.begin(); it != Properties.end(); it++ )
    //{
    //    if ( it->Flags & Properties::Flags::Valid )
    //    {
    //        std::string sName = it->pszName;

    //        if ( sName == sm_kapszPropertyNames[ Property_X ] )
    //        {
    //            //
    //            // Set the Xprop.
    //            //
    //            Math::Vector3& Xprop = it->ValueVector3;
    //        }
    //        else
    //        {
    //            ASSERT( False );
    //        }

    //        //
    //        // Set this property to invalid since it's already been read.
    //        //
    //        it->Flags &= ~Properties::Flags::Valid;
    //    }
    //}
}

    
pcstr*
TreeScene::GetObjectTypes(
    void
    )
{
    return NULL;  //grammars 
}

///   
ISystemObject*
TreeScene::CreateObject(
    pcstr pszName,
    pcstr pszType  //grammar :: spiketree or plaintree
    )
{
    UNREFERENCED_PARAM( pszName );
    
    ASSERT( pszType != NULL );
    TreeObject * tree = new TreeObject( this, pszType );
    m_Forest.push_back(tree);
    return tree;
}


Error
TreeScene::DestroyObject(
    ISystemObject* pSystemObject
    )
{
    ASSERT( pSystemObject != NULL );

    //
    // Cast to a TreeObject so that the correct destructor will be
    //  called.
    //
    TreeObject* pObject =
        reinterpret_cast<TreeObject*>(pSystemObject);

    m_Forest.remove( pObject );

    SAFE_DELETE( pObject );

    return Errors::Success;
}


ISystemTask*
TreeScene::GetSystemTask(
    void
    )
{
    
    return m_pTask;
}


System::Changes::BitMask
TreeScene::GetPotentialSystemChanges(
    void
    )
{
    return System::Changes::None;
}


const void*
TreeScene::GetSystemChangeData(
    System::Change SystemChange
    )
{
    UNREFERENCED_PARAM( SystemChange );

    return NULL;
}

TreeObjectList
TreeScene::GetForest()
{
    return m_Forest;
}
