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
#include "GeomObject.h"


pcstr GeometryObject::sm_kapszPropertyNames[] =
{
    "Position", "Orientation", "Scale",
};

const Properties::Property GeometryObject::sm_kaDefaultProperties[] =
{
    Properties::Property( sm_kapszPropertyNames[ Property_Position ],
                          Properties::Values::Vector3,
                          Properties::Flags::Valid,
                          NULL, NULL, NULL, NULL,
                          Math::Vector3::Zero ),
    Properties::Property( sm_kapszPropertyNames[ Property_Orientation ],
                          Properties::Values::Quaternion,
                          Properties::Flags::Valid,
                          NULL, NULL, NULL, NULL,
                          Math::Vector3::Zero ),
    Properties::Property( sm_kapszPropertyNames[ Property_Scale ],
                          Properties::Values::Vector3,
                          Properties::Flags::Valid,
                          NULL, NULL, NULL, NULL,
                          Math::Vector3::Zero ),
};


GeometryObject::GeometryObject(
    ISystemScene* pSystemScene
    )
    : ISystemObject( pSystemScene, NULL )
    , m_Position( Math::Vector3::Zero )
    , m_Orientation( Math::Quaternion::Zero )
    , m_Scale( Math::Vector3::One )
{
    ASSERT( Property_Count == sizeof sm_kapszPropertyNames / sizeof sm_kapszPropertyNames[ 0 ] );
    ASSERT( Property_Count == sizeof sm_kaDefaultProperties / sizeof sm_kaDefaultProperties[ 0 ] );
}


GeometryObject::~GeometryObject(
    void
    )
{
}


System::Type
GeometryObject::GetSystemType(
    void
    )
{
    return System::Types::Geometry;
}


Error
GeometryObject::Initialize(
    std::vector<Properties::Property> Properties
    )
{
    ASSERT( !m_bInitialized );

    //
    // Set this set as initialized.
    //
	m_bInitialized = True;

    //
    // Set the properties for this object.
    //
    SetProperties( Properties );

    return Errors::Success;
}


void
GeometryObject::GetProperties(
    Properties::Array& Properties
    )
{
    //
    // Get the index of our first item.
    //
    i32 iProperty = static_cast<i32>(Properties.size());

    //
    // Add all the properties.
    //
    Properties.reserve( Properties.size() + Property_Count );

    for ( i32 i=0; i < Property_Count; i++ )
    {
        Properties.push_back( sm_kaDefaultProperties[ i ] );
    }

    //
    // Modify the default values.
    //
    Properties[ iProperty+Property_Position ].SetValue( m_Position );
    Properties[ iProperty+Property_Orientation ].SetValue( m_Orientation );
    Properties[ iProperty+Property_Scale ].SetValue( m_Scale );
}


void
GeometryObject::SetProperties(
    Properties::Array Properties
    )
{
    ASSERT( m_bInitialized );

    //
    // Read in the properties.
    //
	for ( Properties::Iterator it=Properties.begin(); it != Properties.end(); it++ )
    {
        std::string sName = it->GetName();

        if ( it->GetFlags() & Properties::Flags::Valid )
        {
            if ( sName == sm_kapszPropertyNames[ Property_Position ] )
            {
                m_Position = it->GetVector3();
                PostChanges( System::Changes::Geometry::Position );
            }
            else if ( sName == sm_kapszPropertyNames[ Property_Orientation ] )
            {
                m_Orientation = it->GetQuaternion();
                PostChanges( System::Changes::Geometry::Orientation );
            }
            else if ( sName == sm_kapszPropertyNames[ Property_Scale ] )
            {
                m_Scale = it->GetVector3();
                PostChanges( System::Changes::Geometry::Scale );
            }
            else
            {
                ASSERT( False );
            }

            //
            // Set this property to invalid since it's already been read.
            //
            it->ClearFlag( Properties::Flags::Valid );
        }
    }
}


System::Types::BitMask
GeometryObject::GetDesiredSystemChanges(
    void
    )
{
    return System::Changes::Geometry::Position |
           System::Changes::Geometry::Orientation |
           System::Changes::Geometry::Scale;
}


Error
GeometryObject::ChangeOccurred(
	ISubject* pSubject,
	System::Changes::BitMask ChangeType
	)
{
    ASSERT( m_bInitialized );

    if ( ChangeType & System::Changes::Geometry::Position )
    {
        m_Position = *dynamic_cast<IGeometryObject*>(pSubject)->GetPosition();
    }

    if ( ChangeType & System::Changes::Geometry::Orientation )
    {
        m_Orientation = *dynamic_cast<IGeometryObject*>(pSubject)->GetOrientation();
    }

    if ( ChangeType & System::Changes::Geometry::Scale )
    {
        m_Scale = *dynamic_cast<IGeometryObject*>(pSubject)->GetScale();
    }

    return Errors::Success;
}


System::Changes::BitMask
GeometryObject::GetPotentialSystemChanges(
    void
    )
{
    return System::Changes::Geometry::All;
}


const Math::Vector3*
GeometryObject::GetPosition(
    void
    )
{
	return &m_Position;
}


const Math::Quaternion*
GeometryObject::GetOrientation(
    void
    )
{
	return &m_Orientation;
}


const Math::Vector3*
GeometryObject::GetScale(
    void
    )
{
	return &m_Scale;
}
