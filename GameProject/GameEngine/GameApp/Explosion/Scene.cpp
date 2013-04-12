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
// core includes
//
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

//
// Explosion system includes
//
#include "Scene.h"
#include "Object.h"
#include "Task.h"
#include "POI.h"
#include "MeteorImpact.h"


//
// Externs
//
extern ManagerInterfaces   g_Managers;


///////////////////////////////////////////////////////////////////////////////
// ExplosionScene - Constructor
ExplosionScene::ExplosionScene( ISystem* pSystem ) : ISystemScene( pSystem ), m_pExplosionTask( NULL )
{
}


///////////////////////////////////////////////////////////////////////////////
// ~ExplosionScene - Destructor
ExplosionScene::~ExplosionScene( void )
{
	SAFE_DELETE( m_pExplosionTask );

	// Free all the remaining objects.
	for( std::vector<ExplosionObject*>::iterator it = m_Objects.begin(); it != m_Objects.end(); it++ )
	{
		delete *it;
	}

	m_Objects.clear();


	// Free all the remaining POI.
	for( std::list<POI*>::iterator it = m_POI.begin(); it != m_POI.end(); it++ )
	{
		delete *it;
	}

	m_POI.clear();
}


///////////////////////////////////////////////////////////////////////////////
// GetSystemType - Returns System type for this Scene
System::Type ExplosionScene::GetSystemType( void )
{
	return System::Types::Explosion;
}


///////////////////////////////////////////////////////////////////////////////
// Initialize - Initializes this Scene with the given properties
Error ExplosionScene::Initialize( std::vector<Properties::Property> Properties )
{
	ASSERT( !m_bInitialized );

	m_pExplosionTask = new ExplosionTask( this );
	ASSERT( m_pExplosionTask != NULL );

	m_bInitialized = True;

	return Errors::Success;
}


///////////////////////////////////////////////////////////////////////////////
// GetProperties - Properties for this Scene are returned in Properties
void ExplosionScene::GetProperties( Properties::Array& Properties )
{
	UNREFERENCED_PARAM( Properties );
}


///////////////////////////////////////////////////////////////////////////////
// SetProperties - Set properties for this Scene
void ExplosionScene::SetProperties( Properties::Array Properties )
{
	UNREFERENCED_PARAM( Properties );

	ASSERT( m_bInitialized );
}


///////////////////////////////////////////////////////////////////////////////
// GetObjectTypes - Get Object types for this Scene
pcstr* ExplosionScene::GetObjectTypes( void )
{
	return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// CreateObject - Create an Object with the given Name and Type
ISystemObject* ExplosionScene::CreateObject( pcstr pszName, pcstr pszType )
{
	ASSERT( m_bInitialized );

	// Create the Explosion object
	ExplosionObject* pObject = NULL;
	if( strcmp( pszType, "MeteorImpact" ) == 0 )
	{
		pObject = (ExplosionObject*)new MeteorImpact( this, pszName );
	}
	else
	{
		// Create a default Explosion object (this is probably an error, so assert)
		ASSERT( false );
		pObject = new ExplosionObject( this, pszName );
	}

	if ( pObject != NULL )
	{
		m_Objects.push_back( pObject );
	}

	return pObject;
}


///////////////////////////////////////////////////////////////////////////////
// DestroyObject - Destroys the given Object, removing it from the Scene
Error ExplosionScene::DestroyObject( ISystemObject* pSystemObject )
{
	ASSERT( m_bInitialized );
	ASSERT( pSystemObject != NULL );

	// Cast to a ExplosionObject so that the correct destructor will be called.
	ExplosionObject* pObject = reinterpret_cast<ExplosionObject*>(pSystemObject);

	// Remove the object from the list and delete it
	u32 index = 0;
	for( std::vector<ExplosionObject*>::iterator it=m_Objects.begin(); it != m_Objects.end(); it++ )
	{
		if( *it == pObject )
		{
			m_Objects.erase( m_Objects.begin() + index );
			break;
		}
		index++;
	}

	SAFE_DELETE( pSystemObject );

	return Errors::Success;
}


///////////////////////////////////////////////////////////////////////////////
// GetSystemTask - Returns the task associated with this Scene
ISystemTask* ExplosionScene::GetSystemTask( void )
{
	return m_pExplosionTask;
}


///////////////////////////////////////////////////////////////////////////////
// GetPotentialSystemChanges - Returns systems changes possible for this Scene
System::Changes::BitMask ExplosionScene::GetPotentialSystemChanges( void )
{
	return System::Changes::None;
}


///////////////////////////////////////////////////////////////////////////////
// GetDesiredSystemChanges - Returns systems this Scene is interested in
System::Changes::BitMask ExplosionScene::GetDesiredSystemChanges( void )
{
	return System::Changes::POI::Contact;
}


///////////////////////////////////////////////////////////////////////////////
// ChangeOccurred - Process changes that occured by other systems/objects
Error ExplosionScene::ChangeOccurred( ISubject* pSubject, System::Changes::BitMask ChangeType )
{
	if( ChangeType & System::Changes::POI::Contact )
	{
		IContactObject* pContactObject = dynamic_cast<IContactObject*>(pSubject);
		ASSERT( pContactObject );

        if( pContactObject )
        {
            const IContactObject::Info* pContactInfo = pContactObject->GetContact();
            ASSERT( pContactInfo );

            if( pContactInfo )
            {
                // Store the contact points so objects can process them later
                POIContact* pContact = new POIContact();
                ASSERT( pContact );

                pContact->SetPosition( pContactInfo->m_Position );
                pContact->SetImpact( pContactInfo->m_Impact );
                pContact->SetStatic( pContactInfo->m_Static );
                pContact->SetVelocityObjectA( pContactInfo->m_VelocityObjectA );
                pContact->SetVelocityObjectB( pContactInfo->m_VelocityObjectB );

                SCOPED_SPIN_LOCK(m_mutex);
                m_POI.push_back( pContact );
            }
        }
	}

	return Errors::Success;
}


///////////////////////////////////////////////////////////////////////////////
// Update - Main Update for the Explosion Scene
void ExplosionScene::Update(f32 DeltaTime)
{
    // Update all Explosion objects serially
    std::vector<ExplosionObject*>::iterator it;
    for ( it = m_Objects.begin(); it != m_Objects.end(); it++ )
    {
        ExplosionObject* pObject = *it;

        pObject->PreUpdate( DeltaTime );
        pObject->Update( DeltaTime );
        pObject->PostUpdate( DeltaTime );
    }

    // Call PostUpdate directly
    PostUpdate();
}


///////////////////////////////////////////////////////////////////////////////
// PostUpdate - PostUpdate processing
void ExplosionScene::PostUpdate( void )
{
    // Loop through and remove and expired POI
    std::list<POI*>::iterator it = m_POI.begin();
    while( it != m_POI.end() )
    {
        POI* pPOI = *it;
        if( pPOI->Expired() )
        {
            it = m_POI.erase( it );
            delete pPOI;
        }
        else
        {
            it++;
        }
    }
}
