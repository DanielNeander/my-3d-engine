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
// Input system includes
//
#include "InputObject.h"
//#include "D3DX9Math.h"

pcstr InputObject::sm_kapszTypeNames[] =
{
    "Controlled", "Overlay",
    NULL
};

pcstr InputObject::sm_kapszPropertyNames[] =
{
    "FKey", "Orientation"
};

InputObject::InputObject(ISystemScene* pSystemScene) : ISystemObject( pSystemScene, NULL )
{
	m_Type = Type_Controlled;
}

const Properties::Property InputObject::sm_kaDefaultProperties[] =
{
    Properties::Property( sm_kapszPropertyNames[ Property_FKey ],
						  VALUE1( Properties::Values::Int32 ),
                          Properties::Flags::Valid,
						  NULL, NULL, NULL, NULL,
						  0),

	Properties::Property( sm_kapszPropertyNames[ Property_Orientation ],
						  VALUE1x3( Properties::Values::Float32 ),
						  Properties::Flags::Valid,
						  NULL, NULL, NULL, NULL,
						  0)
};

InputObject::InputObject(ISystemScene* pSystemScene, pcstr pszType, pcstr pszName) : ISystemObject( pSystemScene, NULL )
{
	ASSERT( Property_Count == sizeof sm_kapszPropertyNames / sizeof sm_kapszPropertyNames[ 0 ] );
    ASSERT( Property_Count == sizeof sm_kaDefaultProperties / sizeof sm_kaDefaultProperties[ 0 ] );

	if (strcmp(pszType, sm_kapszTypeNames[Type_Controlled]) == 0) {
        m_Type = Type_Controlled;
    } else if (strcmp(pszType, sm_kapszTypeNames[Type_GUI]) == 0) {
        m_Type = Type_GUI;
    }

	m_sName = pszName;
}


InputObject::~InputObject(
    void
    )
{
}


System::Type
InputObject::GetSystemType(
    void
    )
{
    return System::Types::Input;
}


Error
InputObject::Initialize(
    std::vector<Properties::Property> Properties
    )
{
    //
    // Set this set as initialized.
    //
	m_bInitialized = True;

	// fill out pitch and yaw 
	m_Pitch = 0.0f;
	m_Yaw = 0.0f;
	m_nFunctionKey = 0;

	memset(&m_LayoutData, 0, sizeof(m_LayoutData));

	SetProperties(Properties);

    return Errors::Success;
}


void
InputObject::GetProperties(
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
	Properties[iProperty + Property_FKey].SetValue(0, m_nFunctionKey);
}


void
InputObject::SetProperties(
    Properties::Array Properties
    )
{
    ASSERT( m_bInitialized );

	Error Err;
	Err = Errors::Failure;

    //
    // Read in the properties.
    //
    for ( Properties::Iterator it=Properties.begin(); it != Properties.end(); it++ )
    {
        if ( it->GetFlags() & Properties::Flags::Valid )
        {
            std::string sName = it->GetName();
 
            if ( sName == sm_kapszPropertyNames[Property_FKey] )
            {
				m_nFunctionKey = it->GetInt32(0);
			}
			else if ( sName == sm_kapszPropertyNames[Property_Orientation] )
			{
				m_Yaw   = it->GetFloat32(0);
				m_Pitch = it->GetFloat32(1);
				m_Roll  = it->GetFloat32(2);

				m_Orientation.x = m_Yaw;
				m_Orientation.y = m_Pitch;
				m_Orientation.z = 0;
				m_Orientation.w = 0;

				PostChanges(System::Changes::Geometry::Orientation);
			}
            else
            {
                ASSERT(False);
            }
            //
            // Set this property to invalid since it's already been read.
            //
            it->ClearFlag(Properties::Flags::Valid);
        }
    }
}


System::Types::BitMask
InputObject::GetDesiredSystemChanges(
    void
    )
{
    return System::Changes::None;
}


Error
InputObject::ChangeOccurred(
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

    return Errors::Success;
}


System::Changes::BitMask
InputObject::GetPotentialSystemChanges(
    void
    )
{
    return System::Changes::Input::Firehose |
           System::Changes::Geometry::Position |
           System::Changes::Geometry::Orientation |
		   System::Changes::Graphics::GUI;
}


const Math::Vector3*
InputObject::GetPosition(
    void
    )
{
	return &m_Position;
}


const Math::Quaternion*
InputObject::GetOrientation(
    void
    )
{
	return &m_Orientation;
}


const Math::Vector3*
InputObject::GetScale(
    void
    )
{
    ASSERT( False );
	return NULL;
}

const WindowData* InputObject::GetWindowData(void)
{
	m_LayoutData.nFlags |= WINDOW_SHOW;
	m_LayoutData.bShow = !m_LayoutData.bShow;

	return &m_LayoutData; 
}

