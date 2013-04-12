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

#pragma once


///////////////////////////////////////////////////////////////////////////////
/// <summary>
///   <c>InputSystem</c> Implementation of the ISystem interface for generic 
///   Input functionality.
/// </summary>
///////////////////////////////////////////////////////////////////////////////

class InputSystem : public ISystem
{
protected:

    /// <summary cref="InputSystem::GetName">
    ///   Implementation of the <c>ISystem::GetName</c> function.
    ///   Gets the name of the system.  Only custom systems can return a custom name.
    /// </summary>
    /// <returns>pcstr - The name of the system.</returns>
    /// <seealso cref="ISystem::GetName"/>
    virtual pcstr GetName( void );

    /// <summary cref="InputSystem::GetSystemType">
    ///   Implementation of the <c>ISystem::GetSystemType</c> function.
    ///   Gets the system type for this system.
    /// </summary>
    /// <returns>System::Type - The type of the system.</returns>
    /// <seealso cref="ISystem::GetSystemType"/>
    virtual System::Type GetSystemType( void );

    /// <summary cref="InputSystem::Initialize">
    ///   Implementation of the <c>ISystem::Initialize</c> function.
    ///   One time initialization function for the system.
    /// </summary>
    /// <param name="Properties">Property structure array to initialize.</param>
    /// <returns>Error - Any error codes.</returns>
    /// <seealso cref="ISystem::Initialize"/>
    virtual Error Initialize( Properties::Array Properties );

    /// <summary cref="InputSystem::GetProperties">
    ///   Implementation of the <c>ISystem::GetProperties</c> function.
    ///   Gets the properties of this system.
    /// </summary>
    /// <param name="Properties">Property structure array to fill</param>
    /// <seealso cref="ISystem::GetProperties"/>
    virtual void GetProperties( Properties::Array& Properties );

    /// <summary cref="InputSystem::SetProperties">
    ///   Implementation of the <c>ISystem::SetProperties</c> function.
    ///   Sets the properties for this system.
    /// </summary>
    /// <param name="Properties">Properties to set in the system.</param>
    /// <seealso cref="ISystem::SetProperties"/>
    virtual void SetProperties( Properties::Array Properties );

    /// <summary cref="InputSystem::CreateScene">
    ///   Implementation of the <c>ISystem::CreateScene</c> function.
    ///   Creates a system scene for containing system objects.
    /// </summary>
    /// <returns>ISystemScene* - The newly create system scene.</returns>
    /// <seealso cref="ISystem::CreateScene"/>
    virtual ISystemScene* CreateScene( void );

    /// <summary cref="InputSystem::DestroyScene">
    ///   Implementation of the <c>ISystem::DestroyScene</c> function.
    ///   Destroys a system scene.
    /// </summary>
    /// <param name="pSystemScene">The scene to destroy. Any objects within are destroyed.</param>
    /// <returns>Error - Any error codes.</returns>
    /// <seealso cref="ISystem::DestroyScene"/>
    virtual Error DestroyScene( ISystemScene* pSystemScene );

    enum HotKeyTypes
    {
        HotKey_MoveForward,
        HotKey_MoveBack,
        HotKey_MoveLeft,
        HotKey_MoveRight,
        HotKey_MoveUp,
        HotKey_MoveDown,
        HotKey_ResetView,
        HotKey_Pause,
        HotKey_Quit,
        HotKey_PolygonMode,
        HotKey_UseFireHose,
        HotKey_ShowNormals,
        HotKey_ShowTangents,
        HotKey_ShowBoundingBox,
        HotKey_ShowCaptions,
        HotKey_SetThreadCountTo1,
        HotKey_SetThreadCountTo2,
        HotKey_SetThreadCountTo4,
        HotKey_SetThreadCountToMax,
        HotKey_ToggleOverlay1,
        HotKey_ToggleOverlay2,
        HotKey_ToggleOverlay3,
        HotKey_ToggleOverlay4,
        HotKey_ToggleOverlay5,
        HotKey_Count
    };

    static pcstr sm_kapszPropertyNames[];
    static const Properties::Property sm_kaDefaultProperties[];
public:
    InputSystem( void );
    virtual ~InputSystem( void );

    typedef struct _HotKey {
        std::string psKey;
        Bool  bCtrl;
    } HotKey;
    std::map<pcstr, HotKey*> m_aHotKeys;
    typedef std::map<pcstr, HotKey*>::iterator hotKeyIt;
};

extern ManagerInterfaces   g_InputManagers;
