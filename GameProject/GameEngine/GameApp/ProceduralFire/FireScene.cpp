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
#include "FireTask.h"
#include "FireObject.h"

extern ManagerInterfaces   g_FireManagers;

//
//
//
//pcstr FireScene::sm_kapszPropertyNames[] =
//{
//    "Gravity",
//};
//
//const Properties::Property FireScene::sm_kaDefaultProperties[] =
//{
//    Properties::Property( sm_kapszPropertyNames[ Property_Gravity ],
//                          Properties::Values::Vector3,
//                          Properties::Flags::Valid,
//                          Math::Vector3::Zero ),
//};


FireScene::FireScene(
    ISystem* pSystem
    )
    : ISystemScene( pSystem )
    , m_bParallelize(False)
{
    //ASSERT( Property_Count == sizeof sm_kapszPropertyNames / sizeof sm_kapszPropertyNames[ 0 ] );
    //ASSERT( Property_Count == sizeof sm_kaDefaultProperties / sizeof sm_kaDefaultProperties[ 0 ] );

    //
    // Create the task for simulating the physics.
    //
    m_pTask = new FireTask( this );
    ASSERT( m_pTask != NULL );
}


FireScene::~FireScene(
    void
    )
{
    SAFE_DELETE( m_pTask );

}


System::Type
FireScene::GetSystemType(
    void
    )
{
    return System::Types::MakeCustom( 2 );
}


Error 
FireScene::Initialize( std::vector<Properties::Property> Properties )
{
    m_bParallelize = g_FireManagers.pTask != NULL && 
        g_FireManagers.pEnvironment->Variables().GetAsBool( "ProceduralFire::Parallel", True );

    return Errors::Success;
}


void
FireScene::GetProperties(
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
FireScene::SetProperties(
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
FireScene::GetObjectTypes(
    void
    )
{
    return FireObject::sm_kapszTypeNames;  //grammars 
}

///   
ISystemObject*
FireScene::CreateObject(
    pcstr pszName,
    pcstr pszType  //grammar :: TreeFire or HouseFire
    )
{
    
    ASSERT( pszType != NULL );
#if NO_MUTUAL_FIRE_COLLISION_CHECKS
    FireObject * FireObj = new FireObject( this, pszType, pszName, m_FireObjects.size() );
#else
    FireObject * FireObj = new FireObject( this, pszType, pszName );
#endif
    m_FireObjects.push_back(FireObj);
    return FireObj;
}


Error
FireScene::DestroyObject(
    ISystemObject* pSystemObject
    )
{
    ASSERT( pSystemObject != NULL );

    //
    // Cast to a FireObject so that the correct destructor will be
    //  called.
    //
    FireObject* pObject =
        reinterpret_cast<FireObject*>(pSystemObject);

    SAFE_DELETE( pObject );

    return Errors::Success;
}


ISystemTask*
FireScene::GetSystemTask(
    void
    )
{
    //return NULL;
    return m_pTask;
}
System::Changes::BitMask
FireScene::GetPotentialSystemChanges(
    void
    )
{
    return System::Changes::None;
}


const void*
FireScene::GetSystemChangeData(
    System::Change SystemChange
    )
{
    UNREFERENCED_PARAM( SystemChange );

    return NULL;
}
FireObjectList&
FireScene::GetFires()
{
    return m_FireObjects;
}
