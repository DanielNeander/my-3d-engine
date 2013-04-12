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
#include "InputSystem.h"
#include "InputScene.h"
#include "InputObject.h"

ManagerInterfaces   g_InputManagers;

pcstr InputSystem::sm_kapszPropertyNames[] =
{
    "MoveForward",
    "MoveBack",
    "MoveLeft",
    "MoveRight",
    "MoveUp",
    "MoveDown",
    "ResetView",
    "Pause",
    "Quit",
    "PolygonMode",
    "UseFireHose",
    "ShowNormals",
    "ShowTangents",
    "ShowBoundingBox",
    "ShowCaptions",
    "SetThreadCountTo1",
    "SetThreadCountTo2",
    "SetThreadCountTo4",
    "SetThreadCountToMax",
    "ToggleOverlay1",
    "ToggleOverlay2",
    "ToggleOverlay3",
    "ToggleOverlay4",
    "ToggleOverlay5"
};

const Properties::Property InputSystem::sm_kaDefaultProperties[] =
{
    Properties::Property( sm_kapszPropertyNames[ HotKey_MoveForward ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_MoveBack ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_MoveLeft ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_MoveRight ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_MoveUp ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_MoveDown ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_ResetView ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_Pause ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_Quit ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_PolygonMode ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_UseFireHose ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_ShowNormals ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_ShowTangents ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_ShowBoundingBox ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_ShowCaptions ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_SetThreadCountTo1 ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_SetThreadCountTo2 ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_SetThreadCountTo4 ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_SetThreadCountToMax ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_ToggleOverlay1 ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_ToggleOverlay2 ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_ToggleOverlay3 ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_ToggleOverlay4 ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0),
    Properties::Property( sm_kapszPropertyNames[ HotKey_ToggleOverlay5 ],
    VALUE2( Properties::Values::String, Properties::Values::Int32 ),
    Properties::Flags::Valid,
    "Key", "Ctrl", NULL, NULL,
    "", 0)
};


InputSystem::InputSystem(
    void
    )
    : ISystem()
{
}


InputSystem::~InputSystem(
    void
    )
{
}


pcstr
InputSystem::GetName(
    void
    )
{
    return System::Names::Input;
}


System::Type
InputSystem::GetSystemType(
    void
    )
{
    return System::Types::Input;
}


Error
InputSystem::Initialize(
    Properties::Array Properties
    )
{
    ASSERT( !m_bInitialized );

	m_bInitialized = True;

    SetProperties( Properties );

    return Errors::Success;
}


void
InputSystem::GetProperties(
    Properties::Array& Properties
    )
{
    //
    // Get the index of our first item.
    //
    i32 iProperty = static_cast<i32>(Properties.size());
    iProperty = 0;

    //
    // Add all the properties.
    //
    Properties.reserve( Properties.size() + HotKey_Count );

    for ( i32 i=0; i < HotKey_Count; i++ )
    {
        Properties.push_back( sm_kaDefaultProperties[ i ] );
    }
}


void
InputSystem::SetProperties(
    Properties::Array Properties
    )
{
    ASSERT( m_bInitialized );
    //
    // Read in the properties.
    //
    for ( Properties::Iterator it=Properties.begin(); it != Properties.end(); it++ )
    {
        if ( it->GetFlags() & Properties::Flags::Valid )
        {
            // property name
            pcstr sName = it->GetName();

            // HOT! key (may be a combo)
            HotKey* hotKey = new HotKey;
            hotKey->psKey = it->GetString( 0 );
            hotKey->bCtrl = it->GetBool( 1 );

            // add to HOT! keys map
            m_aHotKeys[sName] = hotKey;

            //
            // Set this property to invalid since it's already been read.
            //
            it->ClearFlag( Properties::Flags::Valid );
        }
    }
}

ISystemScene*
InputSystem::CreateScene(
    void
    )
{
    return new InputScene( this );
}


Error
InputSystem::DestroyScene(
    ISystemScene* pSystemScene
    )
{
    ASSERT( pSystemScene != NULL );

    InputScene* pScene = reinterpret_cast<InputScene*>(pSystemScene);
    SAFE_DELETE( pScene );

    return Errors::Success;
}

