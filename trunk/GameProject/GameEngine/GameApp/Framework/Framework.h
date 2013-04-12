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

#include "FrameworkAPI.h"

/*******************************************************************************
* CLASS: Framework
*
* DESCRIPTION:
* Responsible for tying in all the different managers and systems.  Also handles
*  loading and parsing of the global definition file (gdf).
*******************************************************************************/

class Scheduler;
class ChangeManager;
class UScene;
class uObject;

class Framework : public IService::ISystemAccess
{
public:

    //
    // Constructor.
    //
    Framework( void ) throw ( ... );

    //
    // Destructor.
    //
    ~Framework( void );

    virtual Error Initialize( pcstr pszGDF );

    virtual void Shutdown( void );

    //
    // Executes the passed in GDF.
    //  pszGDF - GDF file name.
    //   returns - an error code.
    //
    virtual Error Execute( void ) throw ( ... );


protected:

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // IService::ISystemAccess Implementations.

    /// <summary cref="IService::ISystemAccess::GetSystem(pcstr)">
    ///   Implementation of IService::ISystemAccess::GetSystem.
    /// </summary>
    virtual Handle GetSystem( pcstr pszSystemName );

    /// <summary>
    ///   Implementation of IService::ISystemAccess::GetSystem.
    /// </summary>
    virtual Handle GetSystem( System::Type Type );

    /// <summary cref="IService::ISystemAccess::GetScene(pcstr)">
    ///   Implementation of IService::ISystemAccess::GetScene.
    /// </summary>
    virtual Handle GetScene( pcstr pszSystemName );

    /// <summary>
    ///   Implementation of IService::ISystemAccess::GetScene.
    /// </summary>
    virtual Handle GetScene( System::Type Type );

    /// <summary cref="IService::ISystemAccess::GetSystemObject(pcstr, pcstr)">
    ///   Implementation of IService::ISystemAccess::GetSystemObject.
    /// </summary>
    virtual Handle GetSystemObject( pcstr pszSystemName, pcstr pszName );

    /// <summary>
    ///   Implementation of IService::ISystemAccess::GetSystemObject.
    /// </summary>
    virtual Handle GetSystemObject( System::Type Type, pcstr pszName );

    /// <summary cref="IService::ISystemAccess::GetSystemProperty">
    ///   Implementation of IService::ISystemAccess::GetSystemProperty.
    /// </summary>
    virtual void GetSystemProperty( Handle hSystem, InOut Properties::Property& Property );

    /// <summary cref="IService::ISystemAccess::SetSystemProperty">
    ///   Implementation of IService::ISystemAccess::SetSystemProperty.
    /// </summary>
    virtual void SetSystemProperty( Handle hSystem, In Properties::Property& Property );

    /// <summary cref="IService::ISystemAccess::GetSceneProperty">
    ///   Implementation of IService::ISystemAccess::GetSceneProperty.
    /// </summary>
    virtual void GetSceneProperty( Handle hScene, InOut Properties::Property& Property );

    /// <summary cref="IService::ISystemAccess::SetSceneProperty">
    ///   Implementation of IService::ISystemAccess::SetSceneProperty.
    /// </summary>
    virtual void SetSceneProperty( Handle hScene, In Properties::Property& Property );

    /// <summary cref="IService::ISystemAccess::GetObjectProperty">
    ///   Implementation of IService::ISystemAccess::GetObjectProperty.
    /// </summary>
    virtual void GetObjectProperty( Handle hObject, InOut Properties::Property& Property );

    /// <summary cref="IService::ISystemAccess::SetObjectProperty">
    ///   Implementation of IService::ISystemAccess::SetObjectProperty.
    /// </summary>
    virtual void SetObjectProperty( Handle hObject, In Properties::Property& Property );

    /// <summary>
    ///   Issues all the pending property changes.  This only occurs after the scheduler has
    ///    executed all the waiting items.
    /// </summary>
    /// <param name="SystemTypes">The system types of the system that can accept property
    ///  changes.</param>
    void IssuePendingSystemPropertyChanges( System::Types::BitMask SystemTypes=System::Types::All );


protected:

    struct PropertyIssue
    {
        System::Components      SystemComponent;
        System::Type            SystemType;
        Handle                  hItem;
        Properties::Property    Property;
    };
    typedef std::vector<PropertyIssue>      PropertyIssues;
    typedef PropertyIssues::iterator        PropertyIssuesIt;

    PropertyIssues                          m_PropertyIssues;
    SpinWait                                m_PropertyIssuesSpinWait;


protected:

    Scheduler*							    m_pScheduler;
    ChangeManager*                          m_pSceneCCM;
	ChangeManager*                          m_pObjectCCM;
    UScene*                                 m_pScene;

    Bool								    m_bExecuteLoop;

#ifdef __ALLOW_DEBUG_WINDOW__
    Handle                                  m_hDebugWindow;
#endif

private:

    std::string                             m_sGDF;
    std::string                             m_sNextScene;

    class GDFParser
    {
    public:

        GDFParser( UScene* pScene );

        void ParseEnvironment( std::string sGDF );

		const std::string Parse( std::string sGDF );

        const std::string ParseScene( std::string sGDF, std::string sScene );

        GDFParser& operator=( const GDFParser& ) { ASSERT( False ); }


    protected:

        void ParseElement( void* pElement, Bool bAllowProcessSiblings = True );

        Bool BeginElement( void* pElement );

        Bool EndElement( void* pElement );

        void ReadAttributes( void* pElement, void* pAttributes );

        void ReadPropertyAttributes(
                void* pElement, void* pAttributes,
                std::vector<Properties::Property>& GetProperties,
                std::vector<Properties::Property>& SetProperties
                ) const;


    protected:

        UScene*                                         m_pScene;

        std::string                                     m_sNextScene;

        enum GdfMarkers
        {
            GDFM_None, GDFM_Gdf,
			GDFM_Environment, GDFM_EnvVar,
            GDFM_Systems, GDFM_System, GDFM_SystemProperties, GDFM_SystemProperty,
            GDFM_Scenes, GDFM_Scene,
            GDFM_SceneProperties, GDFM_SceneProperty,
            GDFM_Objects, GDFM_Object, GDFM_ObjectProperties, GDFM_ObjectProperty,
            GDFM_Links, GDFM_Link,
            GDFM_Include,
        };
        GdfMarkers                                      m_GdfMarker;

        u32                                             m_SystemLevel;
        pcstr                                           m_pszSystemType;
        ISystem*                                        m_pSystem;

        u32                                             m_SceneLevel;
        pcstr                                           m_pszSceneName;
        ISystemScene*                                   m_pSystemScene;

        Bool                                            m_FirstObjectsMarker;

        u32                                             m_ObjectLevel;
        pcstr                                           m_pszObjectName;
        pcstr                                           m_pszObjectType;
        uObject*                                        m_pUObject;
        ISystemObject*                                  m_pSystemObject;

        Properties::Array                               m_GetProperties;
        Properties::Array                               m_SetProperties;

        std::map<u32,Properties::Array>                 m_SetPropertiesMap;
        std::map<uObject*,std::map<u32,Properties::Array> >  m_AllObjectProperties;
    };
};
