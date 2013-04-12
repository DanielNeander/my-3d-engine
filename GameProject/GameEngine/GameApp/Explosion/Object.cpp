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

// Explosion system includes
//
#include "Object.h"

///////////////////////////////////////////////////////////////////////////////
// ExplosionObject - Constructor
ExplosionObject::ExplosionObject( ISystemScene* pSystemScene, pcstr pszName ) : ISystemObject( pSystemScene, pszName )
{
}


///////////////////////////////////////////////////////////////////////////////
// ~ExplosionObject - Destructor
ExplosionObject::~ExplosionObject( void )
{
}


///////////////////////////////////////////////////////////////////////////////
// GetSystemType - Returns System type for this Object
System::Type ExplosionObject::GetSystemType( void )
{
    return System::Types::Explosion;
}


///////////////////////////////////////////////////////////////////////////////
// Initialize - Initializes this Object with the given properties
Error ExplosionObject::Initialize( std::vector<Properties::Property> Properties )
{
    // Set this set as initialized.
	m_bInitialized = True;

	// Set properties
	SetProperties( Properties );

    return Errors::Success;
}


///////////////////////////////////////////////////////////////////////////////
// GetProperties - Properties for this Object are returned in Properties
void ExplosionObject::GetProperties( Properties::Array& Properties )
{
    UNREFERENCED_PARAM( Properties );
}


///////////////////////////////////////////////////////////////////////////////
// SetProperties - Set properties for this Object
void ExplosionObject::SetProperties( Properties::Array Properties )
{
    ASSERT( m_bInitialized );
}


///////////////////////////////////////////////////////////////////////////////
// GetDesiredSystemChanges - Returns the systems changes this Object 
//                           is intrested in hearing about
System::Types::BitMask ExplosionObject::GetDesiredSystemChanges( void )
{
    return System::Changes::None;
}


///////////////////////////////////////////////////////////////////////////////
// ChangeOccurred - Notify this Object a change occured (change handler)
Error ExplosionObject::ChangeOccurred( ISubject* pSubject, System::Changes::BitMask ChangeType )
{
    ASSERT( m_bInitialized );

    if( ChangeType & System::Changes::Geometry::Position )
    {
        m_Position = *dynamic_cast<IGeometryObject*>(pSubject)->GetPosition();
    }

    if( ChangeType & System::Changes::Geometry::Orientation )
    {
        m_Orientation = *dynamic_cast<IGeometryObject*>(pSubject)->GetOrientation();
    }

    if( ChangeType & System::Changes::Geometry::Scale )
	{
        m_Scale = *dynamic_cast<IGeometryObject*>(pSubject)->GetScale();
	}

    return Errors::Success;
}


///////////////////////////////////////////////////////////////////////////////
// GetPotentialSystemChanges - Returns systems changes possible for this Object
System::Changes::BitMask ExplosionObject::GetPotentialSystemChanges( void )
{
    return ( System::Changes::Geometry::Position
           | System::Changes::Geometry::Orientation
		   | System::Changes::Physics::Velocity );
}


///////////////////////////////////////////////////////////////////////////////
// GetPosition - Returns the position of this Object
const Math::Vector3* ExplosionObject::GetPosition( void )
{
	return &m_Position;
}


///////////////////////////////////////////////////////////////////////////////
// GetOrientation - Returns the orientation of this Object
const Math::Quaternion* ExplosionObject::GetOrientation( void )
{
	return &m_Orientation;
}


///////////////////////////////////////////////////////////////////////////////
// GetScale - Returns the scale of this Object
const Math::Vector3* ExplosionObject::GetScale( void )
{
	return &m_Scale;
}


///////////////////////////////////////////////////////////////////////////////
// PreUpdate - PreUpdate processing
void ExplosionObject::PreUpdate( f32 DeltaTime )
{
	UNREFERENCED_PARAM( DeltaTime );
}


///////////////////////////////////////////////////////////////////////////////
// Update - Main update
void ExplosionObject::Update( f32 DeltaTime )
{
	m_State.UpdateTime( DeltaTime );
}


///////////////////////////////////////////////////////////////////////////////
// PostUpdate - PostUpdate processing
void ExplosionObject::PostUpdate( f32 DeltaTime )
{
	UNREFERENCED_PARAM( DeltaTime );
}
