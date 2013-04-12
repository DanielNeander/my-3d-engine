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
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

#include "Universal.h"


////////////////////////////////////////////////////////////////////////////////////////////////
// UScene
////////////////////////////////////////////////////////////////////////////////////////////////

UScene::UScene(
    IChangeManager* pSceneCCM,
    IChangeManager* pObjectCCM

    )
    : m_pSceneCCM( pSceneCCM )
    , m_pObjectCCM( pObjectCCM )
{
}


UScene::~UScene(
    void
    )
{
    //
    // Send "pre-destroying objects" message to the scene extensions.
    //
    for ( SystemScenesConstIt it=m_SystemScenes.begin(); it != m_SystemScenes.end(); it++ )
    {
        it->second->GlobalSceneStatusChanged(
            ISystemScene::GlobalSceneStatus::PreDestroyingObjects
            );
    }

	//
	// Get rid of all the links.
	//
	for ( ObjectLinksIt it=m_ObjectLinks.begin(); it != m_ObjectLinks.end(); it++ )
	{
		const ObjectLinkData& old = *it;

		m_pObjectCCM->Unregister( old.pSubject, old.pObserver );
	}
	m_ObjectLinks.clear();

    //
    // Get rid of all the objects.
    //
    Objects Objs = m_Objects;
    for ( ObjectsIt it=Objs.begin(); it != Objs.end(); it++ )
    {
        DestroyObject( *it );
    }
    m_Objects.clear();

    //
    // Send "post-destroying objects" message to the scene extensions then delete the scene.
    //
    SystemScenes SysScenes = m_SystemScenes;
    for ( SystemScenesIt it=SysScenes.begin(); it != SysScenes.end(); it++ )
    {
        ISystemScene* pSystemScene = it->second;

        pSystemScene->GlobalSceneStatusChanged(
            ISystemScene::GlobalSceneStatus::PostDestroyingObjects
            );

        Unextend( pSystemScene );
    }
    m_SystemScenes.clear();
}


ISystemScene*
UScene::Extend(
    ISystem* pSystem
    )
{
    ASSERT( pSystem != NULL );

    //
    // Get the system's type.
    //
    System::Type SystemType = pSystem->GetSystemType();
    ASSERTMSG( m_SystemScenes.find( SystemType ) == m_SystemScenes.end(),
               "The new scene to create for the selected system type already exists." );

    //
    // Have the system create it's scene.
    //
    ISystemScene* pScene = pSystem->CreateScene();
    ASSERT( pScene != NULL );

    if ( pScene != NULL )
    {
        //
        // Add the scene to the collection.
        //
        m_SystemScenes[ SystemType ] = pScene;

        //
        // Register all changes made by the scene.
        //
        m_pSceneCCM->Register( pScene, System::Changes::Generic::All, this );
    }

    return pScene;
}


Error
UScene::Unextend(
    ISystemScene* pScene
    )
{
    ASSERT( pScene != NULL );

    //
    // Get the system.
    //
    ISystem* pSystem = pScene->GetSystem();
    ASSERT( pSystem != NULL );

    //
    // Get the system's type.
    //
    System::Type SystemType = pSystem->GetSystemType();

    //
    // Find the system scene in the collection and remove it.
    //
    SystemScenesIt it = m_SystemScenes.find( SystemType );
    ASSERTMSG( it != m_SystemScenes.end(),
               "The scene to delete for its system type doesn't exist." );

    m_SystemScenes.erase( it );

    //
    // Unregister the scene from the CCM.
    //
    m_pSceneCCM->Unregister( pScene, this );

    //
    // Call the system to delete it's scene.
    //
    pSystem->DestroyScene( pScene );

    return Errors::Success;
}


uObject*
UScene::CreateObject(
    pcstr pszName
    )
{
    //
    // Create the new object.
    //
    uObject* pObject = new uObject( this, pszName );
    ASSERT( pObject != NULL );

    pObject->m_pObjectCCM = m_pObjectCCM; 

    //
    // Add the object to the collection.
    //
    m_Objects.push_back( pObject );

    //
    // Register the object with the scene's CCM.
    //
    m_pSceneCCM->Register(
        pObject, System::Changes::Generic::All | System::Changes::Geometry::All, this
        );

    return pObject;
}


Error
UScene::DestroyObject(
    uObject* pObject
    )
{
    ASSERT( pObject != NULL );

    m_pSceneCCM->Unregister( pObject, this );
    m_Objects.remove( pObject );
    delete pObject;

    return Errors::Success;
}


uObject*
UScene::FindObject(
    pcstr pszName
    )
{
    uObject* pObject = NULL;

    for ( UScene::ObjectsConstIt it=m_Objects.begin(); it != m_Objects.end(); it++ )
    {
        if ( strcmp( pszName, (*it)->GetName() ) == 0 )
        {
            pObject = *it;
            break;
        }
    }

    return pObject;
}


void
UScene::CreateObjectLink(
    ISystemObject* pSubject,
    ISystemObject* pObserver
    )
{
	//
	// Register objects with the CCM.
	//
    System::Changes::BitMask Changes =
        pSubject->GetPotentialSystemChanges() & pObserver->GetDesiredSystemChanges();

    if ( Changes )
    {
        m_pObjectCCM->Register( pSubject, Changes, pObserver );

	    //
	    // Hold on to the list for unregistering later.
	    //
	    ObjectLinkData old = { pSubject, pObserver };
	    m_ObjectLinks.push_back( old );

        //
        // Inform the link requester that the link has been established.
        //
        pSubject->PostChanges( System::Changes::Link );
    }
}


void
UScene::CreateObjectLink(
    uObject* pSubject,
    ISystemObject* pObserver
    )
{
	//
	// Register objects with the CCM.
	//
    System::Changes::BitMask Changes =
        pSubject->GetPotentialSystemChanges() & pObserver->GetDesiredSystemChanges();

    if ( Changes ) {
        m_pObjectCCM->Register( pSubject, Changes, pObserver );

        //
        // Hold on to the list for unregistering later.
        //
        ObjectLinkData old = { pSubject, pObserver };
        m_ObjectLinks.push_back( old );

        //
        // Inform the link requester that the link has been established.
        //
        pSubject->PostChanges( System::Changes::ParentLink );
    }
}


Error
UScene::ChangeOccurred(
    ISubject* pSubject,
    System::Changes::BitMask ChangeType
    )
{
    switch ( ChangeType )
    {
    case System::Changes::Generic::CreateObject:
    {
        IGenericScene* pScene = dynamic_cast<IGenericScene*>(pSubject);

        IGenericScene::CreateObjectDataArray aObjectsToCreate;
        pScene->GetCreateObjects( aObjectsToCreate );

        for ( IGenericScene::CreateObjectDataArrayConstIt it=aObjectsToCreate.begin();
              it != aObjectsToCreate.end(); it++ )
        {
            ASSERT( FindObject( it->pszName ) == NULL );

            uObject* pObject = CreateObject( it->pszName );
            ASSERT( pObject != NULL );

            if ( pObject != NULL )
            {
                System::Types::BitMask Type = 1;
                for ( u32 i=0; i < System::Types::MAX; i++ )
                {
                    if ( it->Types & Type )
                    {
                        SystemScenesIt ssIt = m_SystemScenes.find( Type );
                        if ( ssIt != m_SystemScenes.end() )
                        {
                            pObject->Extend( ssIt->second, NULL );
                        }
                        else
                        {
                            ASSERT( False );
                        }
                    }

                    Type <<= 1;
                }
            }
        }
        break;
    }

    case System::Changes::Generic::DeleteObject:
    {
        IGenericScene* pScene = dynamic_cast<IGenericScene*>(pSubject);

        IGenericScene::DestroyObjectDataArray aObjectsToDestroy;
        pScene->GetDestroyObjects( aObjectsToDestroy );

        for ( IGenericScene::DestroyObjectDataArrayConstIt it=aObjectsToDestroy.begin();
              it != aObjectsToDestroy.end(); it++ )
        {
            uObject* pObject = FindObject( *it );
            DestroyObject( pObject );
        }
        break;
    }

    case System::Changes::Generic::ExtendObject:
    {
        IGenericScene* pScene = dynamic_cast<IGenericScene*>(pSubject);

        IGenericScene::ExtendObjectDataArray aObjectsToExtend;
        pScene->GetExtendObjects( aObjectsToExtend );

        for ( IGenericScene::ExtendObjectDataArrayConstIt it=aObjectsToExtend.begin();
              it != aObjectsToExtend.end(); it++ )
        {
            uObject* pObject = FindObject( it->pszName );

            if ( pObject != NULL )
            {
                pObject->Extend( pScene->ExtendObject( it->pszName, it->pUserData ) );
            }
        }
        break;
    }

    case System::Changes::Generic::UnextendObject:
    {
        IGenericScene* pScene = dynamic_cast<IGenericScene*>(pSubject);

        std::vector<pcstr> aObjectsToUnextend;
        pScene->GetUnextendObjects( aObjectsToUnextend );

        for ( std::vector<pcstr>::const_iterator it=aObjectsToUnextend.begin();
              it != aObjectsToUnextend.end(); it++ )
        {
            uObject* pObject = FindObject( *it );
            if ( pObject != NULL )
            {
                pObject->Unextend( pScene->UnextendObject( *it )->GetSystemScene() );
            }
        }
        break;
    }
    }

    return Errors::Success;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// UObject
////////////////////////////////////////////////////////////////////////////////////////////////

uObject::uObject(
    UScene* pScene,
    pcstr pszName
    )
    : m_pScene( pScene )
    , m_pGeometryObject( NULL )
    , m_pGraphicsObject( NULL )
{
    //
    // Copy the name.
    //
    SetName( pszName );
}


uObject::~uObject(
    void
    )
{
    //
    // Remove all the extensions.
    //
    SystemObjects SysObjs = m_ObjectExtensions;
    for ( SystemObjectsIt it=SysObjs.begin(); it != SysObjs.end(); it++ )
    {
        Unextend( it->second->GetSystemScene() );
    }
    m_ObjectExtensions.clear();
}


ISystemObject*
uObject::Extend(
    ISystemScene* pSystemScene,
    pcstr pszSystemObjectType
    )
{
    ASSERT( pSystemScene != NULL );
    ASSERT( m_ObjectExtensions.find( pSystemScene->GetSystemType() ) == m_ObjectExtensions.end() );

    ISystemObject* pSystemObject = NULL;

    //
    // Create the system object.
    //
    pSystemObject = pSystemScene->CreateObject( m_sName.c_str(), pszSystemObjectType );
    ASSERT( pSystemObject != NULL );

    Extend( pSystemObject );

    return pSystemObject;
}


Bool
uObject::Extend(
    ISystemObject* pSystemObject
    )
{
    Bool bSuccess = False;

    if ( m_ObjectExtensions.find( pSystemObject->GetSystemType() ) == m_ObjectExtensions.end() )
    {
        //
        // Set this as the parent.
        //
        pSystemObject->SetParentObject( this );

        //
        // Get the changes this object will make and is looking for.
        //
        System::Changes::BitMask SysObjPotentialChanges =
            pSystemObject->GetPotentialSystemChanges();
        System::Changes::BitMask SysObjDesiredChanges =
            pSystemObject->GetDesiredSystemChanges();;

        if ( SysObjPotentialChanges & (System::Changes::Geometry::All |
                                       System::Changes::Graphics::AABB) )
        {
            //
            // Have the UObject watch for all the geometry and AABB changes this system makes.
            //
            m_pObjectCCM->Register(
                pSystemObject,
                System::Changes::Geometry::All | System::Changes::Graphics::AABB,
                this );
        }

        //
        // Register each object with scenes that care about the object's changes.
        //
        UScene::SystemScenes pScenes = m_pScene->GetSystemScenes();
        for ( UScene::SystemScenesIt it = pScenes.begin(); it != pScenes.end(); it++ )
        {
            ISystemScene* pScene = it->second;
            if ( pSystemObject->GetPotentialSystemChanges() & pScene->GetDesiredSystemChanges() )
            {
                m_pObjectCCM->Register( pSystemObject, pScene->GetDesiredSystemChanges(), pScene );
            }
        }

        //
        // Register each of the systems with each other.
        //
        System::Changes::BitMask Changes = pSystemObject->GetDesiredSystemChanges();
        for ( std::map<System::Type, ISystemObject*>::iterator it=m_ObjectExtensions.begin();
              it != m_ObjectExtensions.end(); it++ )
        {
            ISystemObject* pObj = it->second;
            if ( pObj->GetPotentialSystemChanges() & SysObjDesiredChanges )
            {
                m_pObjectCCM->Register( pObj, Changes, pSystemObject );
            }
            if ( SysObjPotentialChanges & pObj->GetDesiredSystemChanges() )
            {
                m_pObjectCCM->Register( pSystemObject, pObj->GetDesiredSystemChanges(), pObj );
            }
        }

        //
        // Add the system object to the list.
        //
        System::Type SystemType = pSystemObject->GetSystemType();

        m_ObjectExtensions[ SystemType ] = pSystemObject;

        //
        // Set up the speed path for the geometry and graphics objects.
        //
        if ( SystemType == System::Types::Geometry )
        {
            m_pGeometryObject = dynamic_cast<IGeometryObject*>(pSystemObject);
            ASSERT( m_pGeometryObject != NULL );
        }
        else if ( SystemType == System::Types::Graphics )
        {
            m_pGraphicsObject = dynamic_cast<IGraphicsObject*>(pSystemObject);
        }

        bSuccess = True;
    }

    return bSuccess;
}


void
uObject::Unextend(
    ISystemScene* pSystemScene
    )
{
    ASSERT( pSystemScene != NULL );

    //
    // Get the iterator for the object.
    //
    System::Type SystemType = pSystemScene->GetSystem()->GetSystemType();

    SystemObjectsIt SysObjIt = m_ObjectExtensions.find( SystemType );
    ASSERTMSG( SysObjIt != m_ObjectExtensions.end(),
               "The object to delete doesn't exist in the scene." );

    ISystemObject* pSystemObject = SysObjIt->second;

    //
    // Go through all the other systems and unregister them with this as subject and observer.
    //  The CCM should know if the objects are registered or not, and if not won't do anything.
    //
    for ( std::map<System::Type, ISystemObject*>::iterator it=m_ObjectExtensions.begin();
          it != m_ObjectExtensions.end(); it++ )
    {
        ISystemObject* pObj = it->second;

        //
        // Make sure he system object is not unregistering as an observer of itself.
        //
        if ( pSystemObject != pObj )
        {
            m_pObjectCCM->Unregister( pObj, pSystemObject );
            m_pObjectCCM->Unregister( pSystemObject, pObj );
        }
    }

    //
    // Unregister each object with scenes that cared about the object's changes.
    //
    UScene::SystemScenes pScenes = m_pScene->GetSystemScenes();
    for ( UScene::SystemScenesIt it = pScenes.begin(); it != pScenes.end(); it++ )
    {
        ISystemScene* pScene = it->second;

        if ( pSystemObject->GetPotentialSystemChanges() & pScene->GetDesiredSystemChanges() )
        {
            m_pObjectCCM->Unregister( pSystemObject,  pScene );
        }
    }

    //
    // Unregister the UObject as an observer of this system object.
    //
    m_pObjectCCM->Unregister( pSystemObject, this );

    //
    // Destroy the object.
    //
    pSystemScene->DestroyObject( pSystemObject );

    //
    // Remove the object from the map.
    //
    m_ObjectExtensions.erase( SysObjIt );
}


const uObject::SystemObjects&
uObject::GetExtensions(
    void
    )
{
    return m_ObjectExtensions;
}


ISystemObject*
uObject::GetExtension(
    System::Type SystemType
    )
{
    ISystemObject* pSystemObject = NULL;

    SystemObjectsConstIt it = m_ObjectExtensions.find( SystemType );

    if ( it != m_ObjectExtensions.end() )
    {
        pSystemObject = it->second;
    }

    return pSystemObject;
}


System::Changes::BitMask
uObject::GetPotentialSystemChanges(
    void
    )
{
    return System::Changes::Generic::All |
           System::Changes::Geometry::All |
           System::Changes::Graphics::AABB;
}


Error
uObject::ChangeOccurred(
    ISubject* pSubject,
    System::Changes::BitMask ChangeType
    )
{
    UNREFERENCED_PARAM( pSubject );

    if ( ChangeType & (System::Changes::Generic::All |
                       System::Changes::Geometry::All |
                       System::Changes::Graphics::AABB) )
    {
        ChangeType &= (System::Changes::Generic::All |
                       System::Changes::Geometry::All |
                       System::Changes::Graphics::AABB);

        //
        // Post the pertinent changes made by the extension to the scene CCM.
        //
        PostChanges( ChangeType );
    }

    return Errors::Success;
}


const Math::Vector3*
uObject::GetPosition(
    void
    )
{
    ASSERT( m_pGeometryObject != NULL );

    return m_pGeometryObject->GetPosition();
}


const Math::Quaternion*
uObject::GetOrientation(
    void
    )
{
    ASSERT( m_pGeometryObject != NULL );

    return m_pGeometryObject->GetOrientation();
}


const Math::Vector3*
uObject::GetScale(
    void
    )
{
    ASSERT( m_pGeometryObject != NULL );

    return m_pGeometryObject->GetScale();
}


u32 
uObject::GetSubMeshCount(
    void
    )
{
    ASSERTMSG( False, "Not applicable for UObject." );
    return 0;
}


u32
uObject::GetIndexDeclaration(
    In  u16 nSubMeshIndex
    )
{
    UNREFERENCED_PARAM( nSubMeshIndex );
    ASSERTMSG( False, "Not applicable for UObject." );
    return 0;
}


u32
uObject::GetVertexDeclarationCount(
    In  u16 nSubMeshIndex
    )
{
    UNREFERENCED_PARAM( nSubMeshIndex );
    ASSERTMSG( False, "Not applicable for UObject." );
    return 0;
}


void
uObject::GetVertexDeclaration(
    Out VertexDecl::Element* pVertexDecl,
    In  u16 nSubMeshIndex
    )
{
    UNREFERENCED_PARAM( pVertexDecl );
    UNREFERENCED_PARAM( nSubMeshIndex );
    ASSERTMSG( False, "Not applicable for UObject." );
}


u32
uObject::GetIndexCount(
    In  u16 nSubMeshIndex
    )
{
    UNREFERENCED_PARAM( nSubMeshIndex );
    ASSERTMSG( False, "Not applicable for UObject." );
    return 0;
}


u32
uObject::GetVertexCount(
    In  u16 nSubMeshIndex
    )
{
    UNREFERENCED_PARAM( nSubMeshIndex );
    ASSERTMSG( False, "Not applicable for UObject." );
    return 0;
}


void
uObject::GetIndices(
    Out void* pIndices,
    In  u16 nSubMeshIndex
    )
{
    UNREFERENCED_PARAM( pIndices );
    UNREFERENCED_PARAM( nSubMeshIndex );
    ASSERTMSG( False, "Not applicable for UObject." );
}


void
uObject::GetVertices(
    Out void* pVertices,
    In  u16 nSubMeshIndex,
    In  u16 nStreamIndex,
    In  u32 nVertexDeclCount,
    In  VertexDecl::Element* pVertexDecl
    )
{
    UNREFERENCED_PARAM( pVertices );
    UNREFERENCED_PARAM( nSubMeshIndex );
    UNREFERENCED_PARAM( nStreamIndex );
    UNREFERENCED_PARAM( nVertexDeclCount );
    UNREFERENCED_PARAM( pVertexDecl );
    ASSERTMSG( False, "Not applicable for UObject." );
}


u32 
uObject::GetStreamsChanged(
    void
    )
{
    ASSERTMSG( False, "Not applicable for UObject." );
    return 0;
}


void
uObject::GetAABB(
    Out Math::Vector3& Min,
    Out Math::Vector3& Max
    )
{
    ASSERT( m_pGraphicsObject != NULL );

    return m_pGraphicsObject->GetAABB( Min, Max );
}
