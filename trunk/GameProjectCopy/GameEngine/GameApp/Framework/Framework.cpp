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

#pragma warning( push )
#pragma warning( disable : 4635 6011 )
#include "TinyXML/TinyXml.h"
#pragma warning( pop )

#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"



#include "Universal.h"
#include "DebugWindow.h"
#include "MemoryManager.h"
#include "SystemManager.h"
#include "PlatformManager.h"
#include "ChangeControlManager.h"
#include "EnvironmentManager.h"
#include "ServiceManager.h"
#include "Scheduler.h"
#include "TaskManager.h"
#include "TaskManagerTP.h"
//#include "TaskManagerTBB.h"
#include "Instrumentation.h"
#include "Framework.h"

#include "GameApp/GameApp.h"

CreateSingleton( MemoryManager );
CreateSingleton( EnvironmentManager );
CreateSingleton( PlatformManager );
CreateSingleton( SystemManager );
CreateSingleton( ServiceManager );
CreateSingleton( Instrumentation );

//TaskManager*    g_pTaskManager = NULL;
extern TaskManager*    g_pTaskManager;


void
EngineExecuteGDF(
    pcstr pszGDF
    )
{
#ifndef _DEBUG
    try
#endif
    {
        Framework	Framework;

        pcstr GDF = (pszGDF != NULL) ? pszGDF : "engine.gdf";
        if ( Framework.Initialize( GDF ) == Errors::Success )
        {
            Framework.Execute();
            Framework.Shutdown();
        }
    }
#ifndef _DEBUG
    catch ( ... )
    {
        // Display an error message.
    }
#endif
}


Framework::Framework(
    void
    )
    : m_bExecuteLoop( True )
	, m_pScheduler( NULL )
	, m_pSceneCCM( NULL )
	, m_pObjectCCM( NULL )
#ifdef __ALLOW_DEBUG_WINDOW__
    , m_hDebugWindow( NULL )
#endif
{
	//
	// g_pTaskManager and m_pScheduler are instantiated after the environment variables
	// in the config file are parsed
	//
    m_pSceneCCM = new ChangeManager();
    ASSERT( m_pSceneCCM != NULL );

	m_pObjectCCM = new ChangeManager();
	ASSERT( m_pObjectCCM != NULL );
}


Framework::~Framework(
    void
    )
{
    SAFE_DELETE( m_pScheduler );
    SAFE_DELETE( g_pTaskManager );
    SAFE_DELETE( m_pSceneCCM );
    SAFE_DELETE( m_pObjectCCM );
}


Error
Framework::Initialize(
    pcstr pszGDF
    )
{
    //
    // Set the current directory to the location of the GDF.
    //
    static pcstr apszLocations[] =
    {
        ".\\definitions", "..\\.", "..\\definitions"
    };
    if ( !Singletons::PlatformManager.FileSystem().SetCurrentDirToFileLocation(
            pszGDF, apszLocations
            ) )
    {
        ASSERTMSG1( False, "Framework could not locate the GDF file %s.", pszGDF );
        return Errors::File::NotFound;
    }
    m_sGDF = pszGDF;

    //
    // Create the initial universal scene.
    //
    m_pScene = new UScene( m_pSceneCCM, m_pObjectCCM );
    ASSERT( m_pScene != NULL );

    if ( m_pScene == NULL )
    {
        return Errors::Memory::OutOfMemory;
    }

    //
    // Instantiate the parser, parse the environment variables in the GDF.
    //
    GDFParser Parser( m_pScene );
	Parser.ParseEnvironment( pszGDF );

    //
    // Register the framework as the system access provider.  The system access provider gives the
    //  ability for systems to set the properties in other systems.
    //
    Singletons::ServiceManager.RegisterSystemAccessProvider( this );

	//
	// Instantiate the task manager.
	//
    std::string sTaskManager =
        Singletons::EnvironmentManager.Variables().GetAsString( "TaskManager" );

	if ( sTaskManager == "TBB" || sTaskManager == "" )
	{
		//g_pTaskManager = new TaskManagerTBB();
	}
	else if ( sTaskManager == "TP" )
	{
		g_pTaskManager = new TaskManagerTP();
	}
	else if ( sTaskManager == "None" )
	{
		g_pTaskManager = NULL;
	}
	else
	{
        ASSERTMSG1( False, "Unknown TaskManager: %s", sTaskManager.c_str() );
	}

	if ( g_pTaskManager != NULL )
	{
		g_pTaskManager->Init();
	}

    //
    // Instantiate the scheduler.
    //
	m_pScheduler = new Scheduler( g_pTaskManager );
    ASSERT( m_pScheduler != NULL );

    if ( m_pScheduler == NULL )
    {
        return Errors::Memory::OutOfMemory;
    }

    //
	// Complete the parsing of the GDF and the initial scene.
    //
	m_sNextScene = Parser.Parse( pszGDF );
    m_sNextScene = Parser.ParseScene( pszGDF, m_sNextScene );

    //
    // Set the initial scene for the scheduler.
    //
    m_pScheduler->SetScene( m_pScene );

    return Errors::Success;
}


void
Framework::Shutdown(
    void
    )
{
    //
    // Get rid of the scene.
    //
    SAFE_DELETE( m_pScene );

#ifdef __ALLOW_DEBUG_WINDOW__
    //
    // Destroy the debug window.
    //
    if ( m_hDebugWindow != NULL )
    {
        DestroyDebugWindow( m_hDebugWindow );
    }
#endif

    //
    // De-register the framework as the system access provider.
    //
    Singletons::ServiceManager.UnregisterSystemAccessProvider( this );

    //
    // Free resources used for parallel execution by the change manager.
    //
    m_pObjectCCM->ResetTaskManager();
    m_pSceneCCM->ResetTaskManager();

    //
    // Free the task manager.
    //
	if ( g_pTaskManager != NULL )
	{
		g_pTaskManager->Shutdown();
        delete g_pTaskManager;
        g_pTaskManager = NULL;
	}
}


Error
Framework::Execute(
    void
    )
{
    //
    // Create the debug window.
    //
#ifdef __ALLOW_DEBUG_WINDOW__
    if ( Singletons::EnvironmentManager.Variables().GetAsBool( "Framework::DebugWindow", False ) )
    {
        m_hDebugWindow = CreateDebugWindow();
    }
#endif

    //
    // Process the link messages in the CCMs first, for both the object and scene CCMs.  The link
    //  needs to be established before any other messages come through.
    //
    m_pObjectCCM->DistributeQueuedChanges(
        System::Types::All, System::Changes::Link | System::Changes::ParentLink
        );
    m_pSceneCCM->DistributeQueuedChanges(
        System::Types::All, System::Changes::Link | System::Changes::ParentLink
        );

    //
    // Distribute changes for object and scene CCMs.  The UObject propagates some object messages
    //  up to the scene so it needs to go first.
    //
	m_pObjectCCM->DistributeQueuedChanges();
	m_pSceneCCM->DistributeQueuedChanges();

#ifdef __ALLOW_DEBUG_WINDOW__
    if ( m_hDebugWindow != NULL )
    {
        //
        // Initialize the debug window.
        //
        SetDebugWindowCCM( m_hDebugWindow, m_pSceneCCM, m_pObjectCCM );
        SetDebugWindowUScene( m_hDebugWindow, m_pScene );
    }
#endif

    //
    // Set the runtime status to running.
    //
    Singletons::EnvironmentManager.Runtime().SetStatus( IEnvironment::IRuntime::Status::Running );

    //
    // Run through the main game loop.
    //
	u32 StopAfterNFrames = Singletons::EnvironmentManager.Variables().GetAsInt( "StopAfterNFrames", 0 );
    u32 FrameCount = 0;


    //
    // Initialize resources necessary for parallel change distribution.
    //
    m_pObjectCCM->SetTaskManager(g_pTaskManager);
    m_pSceneCCM->SetTaskManager(g_pTaskManager);

    while ( m_bExecuteLoop )
    {
		::InvalidateRect(GetApp()->getWindow(), NULL, FALSE);

        //
        // Process any pending window messages.
        //
        Singletons::PlatformManager.WindowSystem().ProcessMessages();

        //
        // Call the scheduler to have the systems internally update themselves.
        //
        m_pScheduler->Execute();

        //
        // Set any properties that may have been issued for change.  Any propeties that correlate
        //  to system change notifications will be added to the change controller by the system.
        // NOTE: This is still untested as noone is using it.
        //
        IssuePendingSystemPropertyChanges();

        //
        // Distribute changes for object and scene CCMs.  The UObject propagates some object
        //  messages up to the scene CCM so it needs to go first.
        //
	    m_pObjectCCM->DistributeQueuedChanges();
	    m_pSceneCCM->DistributeQueuedChanges();

        //
        // Check with the environment manager if there is a change in the runtime status to quit.
        //
        if ( Singletons::EnvironmentManager.Runtime().GetStatus() ==
             IEnvironment::IRuntime::Status::Quit )
        {
            //
            // Time to quit looping.
            //
            m_bExecuteLoop = False;
        }

        //
        // Increment the frame count and max frame limit for exit.
        //
        FrameCount++;
        if ( StopAfterNFrames != 0 && FrameCount >= StopAfterNFrames )
        {
            //
            // Time to quit looping.
            //
            m_bExecuteLoop = False;
        }

		extern BOOL g_activeApp;
		if (g_activeApp) {

			GetApp()->updateTime();
			GetApp()->makeFrame();
		} else {
			::Sleep(100);
		}
    }

    return Errors::Success;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// IService::ISystemAccess Implementations.

Handle
Framework::GetSystem(
    pcstr pszSystemName
    )
{
    //
    // Get the pointer to the system from the system manager.  Handle is just a void* so it will
    //  convert with any problems.  If you're one of those guys that will think of just casting
    //  the handle back you'd better not as you'll break the threading when you try to make calls
    //  into this interface directly.
    //
    return Singletons::SystemManager.Get( pszSystemName );
}


Handle
Framework::GetSystem(
    System::Type Type
    )
{
    //
    // Get the pointer to the system from the system manager.  Handle is just a void* so it will
    //  convert with any problems.  If you're one of those guys that will think of just casting
    //  the handle back you'd better not as you'll break the threading when you try to make calls
    //  into this interface directly.
    //
    return Singletons::SystemManager.Get( Type );
}


Handle
Framework::GetScene(
    pcstr pszSystemName
    )
{
    Handle hScene = NULL;

    //
    // Get the system from the system manager to get the type.
    //
    ISystem* pSystem = Singletons::SystemManager.Get( pszSystemName );

    if ( pSystem != NULL )
    {
        //
        // Get the scene based on the type.
        //
        hScene = GetScene( pSystem->GetSystemType() );
    }

    return hScene;
}


Handle
Framework::GetScene(
    System::Type Type
    )
{
    Handle hScene = NULL;

    //
    // Find the scene extension in the universal scene.
    //
    UScene::SystemScenes::const_iterator it = m_pScene->GetSystemScenes().find( Type );

    if ( it != m_pScene->GetSystemScenes().end() )
    {
        hScene = it->second;
    }

    return hScene;
}


Handle
Framework::GetSystemObject(
    pcstr pszSystemName,
    pcstr pszName
    )
{
    Handle hObject = NULL;

    //
    // Get the system from the system manager to get the type.
    //
    ISystem* pSystem = Singletons::SystemManager.Get( pszSystemName );

    if ( pSystem != NULL )
    {
        hObject = GetSystemObject( pSystem->GetSystemType(), pszName );
    }

    return hObject;
}


Handle
Framework::GetSystemObject(
    System::Type Type,
    pcstr pszName
    )
{
    Handle hObject = NULL;

    //
    // Find the universal object in the scene.
    //
    uObject* pUObject = m_pScene->FindObject( pszName );

    if ( pUObject != NULL )
    {
        //
        // Get the system object extension of the universal object using the system type.
        //
        ISystemObject* pObject = pUObject->GetExtension( Type );

        if ( pObject != NULL )
        {
            hObject = pObject;
        }
    }

    return hObject;
}


void
Framework::GetSystemProperty(
    Handle hSystem,
    InOut Properties::Property& Property
    )
{
    std::string sPropertyName = Property.GetName();

    //
    // Reinterpret the handle as an ISystem.
    //
    ASSERT( hSystem != NULL );
    ISystem* pSystem = reinterpret_cast<ISystem*>(hSystem);

    //
    // Get the properties and find a match.
    //
    Properties::Array aProperties;
    pSystem->GetProperties( aProperties );

    Bool bFound = False;
    for ( Properties::ConstIterator it=aProperties.begin(); it != aProperties.end(); it++ )
    {
        if ( sPropertyName == it->GetName() )
        {
            Property = *it;
            bFound = True;
        }
    }
    ASSERTMSG( bFound, "The property does not exist." );
}


void
Framework::SetSystemProperty(
    Handle hSystem,
    In Properties::Property& Property
    )
{
    ASSERT( hSystem != NULL );
    ISystem* pSystem = reinterpret_cast<ISystem*>(hSystem);

#ifdef _DEBUG
    //
    // This will cause an assertion if the property doesn't exist.
    //
    Properties::Property TempProperty = Property;
    GetSystemProperty( hSystem, TempProperty );
#endif

    PropertyIssue pi = { System::System, pSystem->GetSystemType(), hSystem, Property };

    SpinWait::Lock Lock( m_PropertyIssuesSpinWait );
    m_PropertyIssues.push_back( pi );
}


void
Framework::GetSceneProperty(
    Handle hScene,
    InOut Properties::Property& Property
    )
{
    std::string sPropertyName = Property.GetName();

    //
    // Reinterpret the handle as an ISystemScene.
    //
    ASSERT( hScene != NULL );
    ISystemScene* pSystemScene = reinterpret_cast<ISystemScene*>(hScene);

    //
    // Get the properties and find a match.
    //
    Properties::Array aProperties;
    pSystemScene->GetProperties( aProperties );

    Bool bFound = False;
    for ( Properties::ConstIterator it=aProperties.begin(); it != aProperties.end(); it++ )
    {
        if ( sPropertyName == it->GetName() )
        {
            Property = *it;
            bFound = True;
        }
    }
    ASSERTMSG( bFound, "The property does not exist." );
}


void
Framework::SetSceneProperty(
    Handle hScene,
    In Properties::Property& Property
    )
{
    ASSERT( hScene != NULL );
    ISystemScene* pSystemScene = reinterpret_cast<ISystemScene*>(hScene);


#ifdef _DEBUG
    //
    // This will cause an assertion if the property doesn't exist.
    //
    Properties::Property TempProperty = Property;
    GetSceneProperty( hScene, TempProperty );
#endif

    PropertyIssue pi = { System::Scene, pSystemScene->GetSystemType(),
                         hScene, Property };

    SpinWait::Lock Lock( m_PropertyIssuesSpinWait );
    m_PropertyIssues.push_back( pi );
}


void
Framework::GetObjectProperty(
    Handle hObject,
    InOut Properties::Property& Property
    )
{
    std::string sPropertyName = Property.GetName();

    //
    // Reinterpret the handle as an ISystemScene.
    //
    ASSERT( hObject != NULL );
    ISystemObject* pSystemObject = reinterpret_cast<ISystemObject*>(hObject);

    //
    // Get the properties and find a match.
    //
    Properties::Array aProperties;
    pSystemObject->GetProperties( aProperties );

    Bool bFound = False;
    for ( Properties::ConstIterator it=aProperties.begin(); it != aProperties.end(); it++ )
    {
        if ( sPropertyName == it->GetName() )
        {
            Property = *it;
            bFound = True;
        }
    }
    ASSERTMSG( bFound, "The property does not exist." );
}


void
Framework::SetObjectProperty(
    Handle hObject,
    In Properties::Property& Property
    )
{
    ASSERT( hObject != NULL );
    ISystemObject* pSystemObject = reinterpret_cast<ISystemObject*>(hObject);

#ifdef _DEBUG
    //
    // This will cause an assertion if the property doesn't exist.
    //
    Properties::Property TempProperty = Property;
    GetObjectProperty( hObject, TempProperty );
#endif

    PropertyIssue pi = { System::Object, pSystemObject->GetSystemType(),
                         hObject, Property };

    SpinWait::Lock Lock( m_PropertyIssuesSpinWait );
    m_PropertyIssues.push_back( pi );
}


void
Framework::IssuePendingSystemPropertyChanges(
    System::Types::BitMask SystemTypes
    )
{
    //
    // Create an array used for setting the property.
    //
    Properties::Array aProperties( 1 );

    //
    // Iterate through all the properties to issue.
    //
    for ( PropertyIssuesIt it=m_PropertyIssues.begin(); it != m_PropertyIssues.end(); it++ )
    {
        //
        // Get the property to issue.
        //
        const PropertyIssue& pi = *it;

        //
        // Check if the schuler will allow issuing the property change.
        //
        if ( pi.SystemType & SystemTypes )
        {
            aProperties[ 0 ] = pi.Property;

            switch ( pi.SystemComponent )
            {
            case System::System:
                reinterpret_cast<ISystem*>(pi.hItem)->SetProperties( aProperties );
                break;

            case System::Scene:
                reinterpret_cast<ISystemScene*>(pi.hItem)->SetProperties( aProperties );
                break;

            case System::Object:
                reinterpret_cast<ISystemObject*>(pi.hItem)->SetProperties( aProperties );
                break;

            default:
                ASSERTMSG( False, "Unhandled case." );
                break;
            };
        }
    }

    m_PropertyIssues.clear();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Framework::GDFParser Implementations.

Framework::GDFParser::GDFParser(
    UScene* pScene
    )
    : m_pScene( pScene )
{
}


void
Framework::GDFParser::ParseEnvironment(
    std::string sGDF
    )
{
    //
    // Load the gdf xml file.
    //
    TiXmlDocument   XmlDoc( sGDF.c_str() );

    if ( XmlDoc.LoadFile() == False )
    {
        ASSERTMSG1( False, "Parser was unable to load GDF file %s.", sGDF.c_str() );
    }

    //
    // Find the "environment" element.
    //
    TiXmlElement* pElement = XmlDoc.FirstChildElement();
    ASSERT( pElement );
    ASSERTMSG4( strcmp( pElement->Value(), "GlobalDefinition" ) == 0,
                "Parser did not see <GlobablDefinition> as the root element" \
                " instead it was <%s>.  File (%s), row (%d) column (%d)",
                pElement->Value(), sGDF.c_str(), pElement->Row(), pElement->Column() );

    pElement = pElement->FirstChildElement();

    while ( pElement != NULL && strcmp( pElement->Value(), "Environment" ) != 0 )
    {
        pElement = pElement->NextSiblingElement();
    }

    if ( pElement != NULL )
    {
		m_GdfMarker = GDFM_Gdf;
        ParseElement( pElement, False );
    }
}


const std::string
Framework::GDFParser::Parse(
    std::string sGDF
    )
{
    //
    // Load the gdf xml file.
    //
    TiXmlDocument   XmlDoc( sGDF.c_str() );

    if ( XmlDoc.LoadFile() == False )
    {
        ASSERTMSG1( False, "Parser was unable to load GDF file %s.", sGDF.c_str() );
    }

    //
    // Set all the markers to false.
    //
    m_GdfMarker = GDFM_None;

    //
    // Get each element and attribute.
    //
    TiXmlElement* pElement = XmlDoc.FirstChildElement();
    ASSERTMSG( pElement != NULL, "Parser could not load the first element in the GDF." );

    ParseElement( pElement );

    return m_sNextScene;
}


const std::string
Framework::GDFParser::ParseScene(
    std::string sGDF,
    std::string sScene
    )
{
    ASSERTMSG( sScene.empty() == False, "Parser does not have a scene to parse." );

    //
    // Load the gdf xml file.
    //
    TiXmlDocument   XmlDoc( sGDF.c_str() );

    if ( XmlDoc.LoadFile() == False )
    {
        ASSERTMSG1( False, "Parser was unable to load GDF file %s.", sGDF.c_str() );
    }

    //
    // Find the "scenes" element.
    //
    TiXmlElement* pElement = XmlDoc.FirstChildElement();
    ASSERT( pElement );
    ASSERTMSG4( strcmp( pElement->Value(), "GlobalDefinition" ) == 0,
                "Parser did not see <GlobablDefinition> as the root element" \
                " instead it was <%s>.  File (%s), row (%d) column (%d)",
                pElement->Value(), sGDF.c_str(), pElement->Row(), pElement->Column() );

    pElement = pElement->FirstChildElement();

    while ( pElement != NULL && strcmp( pElement->Value(), "Scenes" ) != 0 )
    {
        pElement = pElement->NextSiblingElement();
    }

    if ( pElement != NULL )
    {
        m_FirstObjectsMarker = True;

        //
        // Find the selected scene.
        //
        pElement = pElement->FirstChildElement();
        ASSERT( pElement != NULL );
        ASSERTMSG4( strcmp( pElement->Value(), "Scene" ) == 0,
                    "Parser did not see the <Scene> element"\
                    " instead it was <%s>.  File (%s), row (%d) column (%d)",
                    pElement->Value(), sGDF.c_str(), pElement->Row(), pElement->Column() );

        while ( pElement != NULL )
        {
            pcstr pszName = pElement->Attribute( "Name" );

            if ( pszName != NULL && sScene == pszName )
            {
                m_GdfMarker = GDFM_Scenes;
                m_SceneLevel = 0;

                ParseElement( pElement );

                break;
            }

            pElement = pElement->NextSiblingElement();
        }
    }

    return m_sNextScene;
}


void
Framework::GDFParser::ParseElement(
    void* pElement,
	Bool bAllowProcessSiblings
    )
{
    TiXmlElement* pXmlElement = reinterpret_cast<TiXmlElement*>(pElement);

    //
    // Do beginning element sequence.
    //
    Bool bProcessChildren = BeginElement( pXmlElement );

    //
    // Get the element's attributes and parse them.
    //
    TiXmlAttribute* pXmlAttrib = pXmlElement->FirstAttribute();

    if ( pXmlAttrib != NULL )
    {
        ReadAttributes( pXmlElement, pXmlElement->FirstAttribute() );
    }

    if ( bProcessChildren )
    {
        //
        // Get the element's first child and parse it.
        //
        TiXmlElement* pXmlChildElement = pXmlElement->FirstChildElement();

        if ( pXmlChildElement != NULL )
        {
            ParseElement( pXmlChildElement );
        }
    }

    //
    // Do ending element sequence.
    //
    Bool bProcessSiblings = EndElement( pXmlElement );

    if ( bProcessSiblings && bAllowProcessSiblings )
    {
        //
        // Get the next element and parse it.
        //
        TiXmlElement* pXmlNextElement = pXmlElement->NextSiblingElement();

        if ( pXmlNextElement != NULL )
        {
            ParseElement( pXmlNextElement );
        }
    }
}


Bool
Framework::GDFParser::BeginElement(
    void* pElement
    )
{
    Bool bProcessChildren = True;

    TiXmlElement* pXmlElement = reinterpret_cast<TiXmlElement*>(pElement);

    pcstr pszName = pXmlElement->Value();

    if ( strcmp( pszName, "GlobalDefinition" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_None,
                    "Parser identified <GlobalDefinition> as not being at the root." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        m_GdfMarker = GDFM_Gdf;
    }
    else if ( strcmp( pszName, "Environment" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_Gdf,
                    "Parser identified <Environment> as not being under <GlobalDefinition>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        m_GdfMarker = GDFM_Environment;
    }
    else if ( strcmp( pszName, "Variable" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_Environment,
                    "Parser identified <Variable> as not being under <Environment>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        m_GdfMarker = GDFM_EnvVar;
    }
    else if ( strcmp( pszName, "Systems" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_Gdf,
                    "Parser identified <Systems> as not being under <GlobalDefinition>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        m_SystemLevel = 0;

        m_GdfMarker = GDFM_Systems;
    }
    else if ( strcmp( pszName, "System" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_Systems,
                    "Parser identified <System> as not being under <Systems>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        if ( m_SystemLevel == 0 )
        {
            m_pszSystemType = "";
            m_pSystem = NULL;
        }
        ASSERTMSG( m_pSystem == NULL, "Parser identified an error with using the CDF." );

        m_GdfMarker = GDFM_System;
    }
    else if ( strcmp( pszName, "Scenes" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_Gdf,
                    "Parser identified <Scenes> as not being under <GlobalDefinition>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        bProcessChildren = False;

        m_GdfMarker = GDFM_Scenes;
    }
    else if ( strcmp( pszName, "Scene" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_Scenes,
                    "Parser identified <Scene> as not being under <Scenes>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        if ( m_SceneLevel == 0 )
        {
            //
            // Create the initial scene for each system.
            //
            ISystem* pSystem = Singletons::SystemManager.GetFirst();

            while ( pSystem != NULL )
            {
                m_pScene->Extend( pSystem );

                pSystem = Singletons::SystemManager.GetNext();
            }

            //
            // Remove all the object properties.
            //
            m_AllObjectProperties.clear();
        }

        m_GdfMarker = GDFM_Scene;
    }
    else if ( strcmp( pszName, "Objects" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_Scene,
                    "Parser identified <Objects> as not being under <Scene>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        m_ObjectLevel = 0;

        m_GdfMarker = GDFM_Objects;

        //
        // Send "pre-loading objects" message to the scene extensions.
        //
        if ( m_FirstObjectsMarker )
        {
            m_FirstObjectsMarker = False;

            const UScene::SystemScenes Scenes = m_pScene->GetSystemScenes();
            for ( UScene::SystemScenesConstIt it=Scenes.begin(); it != Scenes.end(); it++ )
            {
                it->second->GlobalSceneStatusChanged(
                    ISystemScene::GlobalSceneStatus::PreLoadingObjects
                    );
            }
        }
    }
    else if ( strcmp( pszName, "Object" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_Objects,
                    "Parser identified <Object> as not being under <Objects>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        if ( m_ObjectLevel == 0 )
        {
            m_pszObjectName = NULL;

            //
            // Create the object and add the required geometry extension.
            //
            m_pUObject = m_pScene->CreateObject();
            ASSERT( m_pUObject != NULL );

            UScene::SystemScenesConstIt it = 
                m_pScene->GetSystemScenes().find( System::Types::Geometry );
            ASSERTMSG( it != m_pScene->GetSystemScenes().end(),
                       "The geometry system has to have already been loaded." );

            ISystemScene* pGeometryScene = it->second;
            ASSERT( pGeometryScene != NULL );

            m_pUObject->Extend( pGeometryScene, NULL );

            //
            // Ready the propeties map for setting the properties.
            //
            m_SetPropertiesMap.clear();

            m_SetPropertiesMap[ System::Types::Geometry ] = Properties::Array();
        }

        m_GdfMarker = GDFM_Object;
    }
    else if ( strcmp( pszName, "Properties" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_System || m_GdfMarker == GDFM_Scene || m_GdfMarker == GDFM_Object,
                    "Parser identified <Properties> as not being under <System> or <Scene> or <Object>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        if ( m_GdfMarker == GDFM_System )
        {
            ASSERTMSG( m_pSystem != NULL, "Parser could not find an instantiated system." );

            m_GdfMarker = GDFM_SystemProperties;
        }
        else if ( m_GdfMarker == GDFM_Scene )
        {
            ASSERTMSG( m_pSystem != NULL, "Parser could not find an instantiated system." );

            m_GdfMarker = GDFM_SceneProperties;
        }
        else if ( m_GdfMarker == GDFM_Object )
        {
            m_pszObjectType = NULL;
            m_pSystemObject = NULL;

            m_GdfMarker = GDFM_ObjectProperties;
        }
    }
    else if ( strcmp( pszName, "Property" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_SystemProperties || m_GdfMarker == GDFM_SceneProperties ||
                    m_GdfMarker == GDFM_ObjectProperties,
                    "Parser identified <Property> as not being under <Properties> for" \
                    " <System> or <Scene> or <Object>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        if ( m_GdfMarker == GDFM_SystemProperties )
        {
            m_GdfMarker = GDFM_SystemProperty;
        }
        else if ( m_GdfMarker == GDFM_SceneProperties )
        {
            m_GdfMarker = GDFM_SceneProperty;
        }
        else if ( m_GdfMarker == GDFM_ObjectProperties )
        {
            m_GdfMarker = GDFM_ObjectProperty;
        }
    }
    else if ( strcmp( pszName, "Links" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_Scene,
                    "Parser identified <Links> as not being under <Scene>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        m_GdfMarker = GDFM_Links;
    }
    else if ( strcmp( pszName, "Link" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_Links,
                    "Parser identified <Link> as not being under <Links>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        m_GdfMarker = GDFM_Link;
    }
    else if ( strcmp( pszName, "Include" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_Scene,
                    "Parser identified <Include> as not being under <Scene>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        m_GdfMarker = GDFM_Include;
    }
    else
    {
        ASSERTMSG4( False,
                    "Parser came across an unrecognized start marker <%s>." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );
    }


    return bProcessChildren;
}


Bool
Framework::GDFParser::EndElement(
    void* pElement
    )
{
    Bool bProcessSiblings = True;

    TiXmlElement* pXmlElement = reinterpret_cast<TiXmlElement*>(pElement);

    pcstr pszName = pXmlElement->Value();

    if ( strcmp( pszName, "GlobalDefinition" ) == 0 )
    {
        ASSERTMSG4( m_GdfMarker == GDFM_Gdf,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        m_GdfMarker = GDFM_None;
    }
    else if ( strcmp( pszName, "Environment" ) == 0 )
    {
        ASSERTMSG4( m_GdfMarker == GDFM_Environment,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        m_GdfMarker = GDFM_Gdf;
    }
    else if ( strcmp( pszName, "Variable" ) == 0 )
    {
		ASSERTMSG4( m_GdfMarker == GDFM_EnvVar,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        m_GdfMarker = GDFM_Environment;
    }
    else if ( strcmp( pszName, "Systems" ) == 0 )
    {
        ASSERTMSG4( m_GdfMarker == GDFM_Systems,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        m_GdfMarker = GDFM_Gdf;
    }
    else if ( strcmp( pszName, "System" ) == 0 )
    {
        ASSERTMSG4( m_GdfMarker == GDFM_System || m_GdfMarker == GDFM_SystemProperties,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        if ( m_SystemLevel == 0 )
        {
            m_pSystem->Initialize( m_SetProperties );
            m_GetProperties.clear();
            m_SetProperties.clear();
        }

        m_GdfMarker = GDFM_Systems;
    }
    else if ( strcmp( pszName, "Scenes" ) == 0 )
    {
        ASSERTMSG4( m_GdfMarker == GDFM_Scenes,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        m_GdfMarker = GDFM_Gdf;
    }
    else if ( strcmp( pszName, "Scene" ) == 0 )
    {
        ASSERTMSG4( m_GdfMarker == GDFM_Scene,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        bProcessSiblings = False;

        m_GdfMarker = GDFM_Scenes;

        //
        // Initialize all the objects and Send "post-loading objects" message to the scene extensions.
        //
        if ( !m_FirstObjectsMarker && m_SceneLevel < 2 )
        {
            m_FirstObjectsMarker = True;

            //
            // Initialize all the objects at once.
            //
            for ( std::map< uObject*,std::map<u32,Properties::Array> >::iterator
                   itAll=m_AllObjectProperties.begin();
                  itAll != m_AllObjectProperties.end(); itAll++ )
            {
                uObject* pUObject = itAll->first;

                for ( std::map<u32,Properties::Array>::iterator it = itAll->second.begin();
                      it != itAll->second.end(); it++ )
                {
                    //
                    // Get the extension.
                    //
                    ISystemObject* pObject = pUObject->GetExtension( it->first );
                    ASSERT( pObject != NULL );

                    //
                    // Initialize the extension.
                    //
                    if ( pObject != NULL )
                    {
                        pObject->Initialize( it->second );
                    }

                    //
                    // Remove all the properties.
                    //
                    it->second.clear();
                }
            }
            m_AllObjectProperties.clear();

            const UScene::SystemScenes Scenes = m_pScene->GetSystemScenes();
            for ( UScene::SystemScenesConstIt it=Scenes.begin(); it != Scenes.end(); it++ )
            {
                it->second->GlobalSceneStatusChanged(
                    ISystemScene::GlobalSceneStatus::PostLoadingObjects
                    );
            }
        }
    }
    else if ( strcmp( pszName, "SystemProperties" ) == 0 )
    {
        ASSERTMSG4( m_GdfMarker == GDFM_SceneProperties,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        m_GdfMarker = GDFM_Scene;
    }
    else if ( strcmp( pszName, "Objects" ) == 0 )
    {
        ASSERTMSG4( m_GdfMarker == GDFM_Objects,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        m_GdfMarker = GDFM_Scene;
    }
    else if ( strcmp( pszName, "Object" ) == 0 )
    {
        ASSERTMSG4( m_GdfMarker == GDFM_Object || m_GdfMarker == GDFM_ObjectProperties,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        if ( m_ObjectLevel == 0 )
        {
            ASSERTMSG( m_pUObject != NULL,
                       "A UObject should have already been created by this point." );

            //
            // Add this object's properties to the global collection.
            //
            m_AllObjectProperties[ m_pUObject ] = m_SetPropertiesMap;
            m_SetPropertiesMap.clear();

            //
            // We're done manipulating this object.
            //
            m_pUObject = NULL;
        }

        m_GdfMarker = GDFM_Objects;
    }
    else if ( strcmp( pszName, "Properties" ) == 0 )
    {
        ASSERTMSG4( m_GdfMarker == GDFM_SystemProperties || m_GdfMarker == GDFM_SceneProperties ||
                    m_GdfMarker == GDFM_ObjectProperties,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        if ( m_GdfMarker == GDFM_SystemProperties )
        {
            m_GdfMarker = GDFM_System;
        }
        else if ( m_GdfMarker == GDFM_SceneProperties )
        {
            ASSERTMSG3( m_SceneLevel < 2, "Scene properties are not valid for included CDFs." \
                        " File (%s), row (%d) column (%d).",
                        pXmlElement->GetDocument()->Value(),
                        pXmlElement->Row(), pXmlElement->Column() );

            ASSERT( m_pSystemScene != NULL );

            m_pSystemScene->Initialize( m_SetProperties );
            m_GetProperties.clear();
            m_SetProperties.clear();

            m_GdfMarker = GDFM_Scene;
        }
        else if ( m_GdfMarker == GDFM_ObjectProperties )
        {
            ASSERT( m_pSystemObject != NULL );

            m_GetProperties.clear();

            m_GdfMarker = GDFM_Object;
        }
    }
    else if ( strcmp( pszName, "Property" ) == 0 )
    {
        ASSERTMSG4( m_GdfMarker == GDFM_SystemProperty || m_GdfMarker == GDFM_SceneProperty ||
                    m_GdfMarker == GDFM_ObjectProperty,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        if ( m_GdfMarker == GDFM_SystemProperty )
        {
            m_GdfMarker = GDFM_SystemProperties;
        }
        else if ( m_GdfMarker == GDFM_SceneProperty )
        {
            m_GdfMarker = GDFM_SceneProperties;
        }
        else if ( m_GdfMarker == GDFM_ObjectProperty )
        {
            m_GdfMarker = GDFM_ObjectProperties;
        }
    }
    else if ( strcmp( pszName, "Links" ) == 0 )
    {
        ASSERTMSG4( m_GdfMarker == GDFM_Links,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        m_GdfMarker = GDFM_Scene;
    }
    else if ( strcmp( pszName, "Link" ) == 0 )
    {
        ASSERTMSG4( m_GdfMarker == GDFM_Link,
                    "Parser identified the <%s> end marker to be in the wrong location." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );

        m_GdfMarker = GDFM_Links;
    }
    else if ( strcmp( pszName, "Include" ) == 0 )
    {
        ASSERTMSG3( m_GdfMarker == GDFM_Include,
                    "Parser identified the <Include> end marker as not being under <Include>." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(), pXmlElement->Row(), pXmlElement->Column() );

        m_GdfMarker = GDFM_Scene;
    }
    else
    {
        ASSERTMSG4( False,
                    "Parser came across an unrecognized end marker <%s>." \
                    " File (%s), row (%d) column (%d).",
                    pszName, pXmlElement->GetDocument()->Value(), pXmlElement->Row(),
                    pXmlElement->Column() );
    }

    return bProcessSiblings;
}


void
Framework::GDFParser::ReadAttributes(
    void* pElement, 
    void* pAttributes
    )
{
    TiXmlElement* pXmlElement = reinterpret_cast<TiXmlElement*>(pElement);
    TiXmlAttribute* pXmlAttrib = reinterpret_cast<TiXmlAttribute*>(pAttributes);

    if ( m_GdfMarker == GDFM_System )
    {
        while ( pXmlAttrib != NULL )
        {
            pcstr pszName = pXmlAttrib->Name();

            if ( strcmp( pszName, "Type" ) == 0 )
            {
                ASSERTMSG4( m_SystemLevel == 0 ||
                            strcmp( m_pszSystemType, pXmlAttrib->Value() ) == 0,
                            "Parser identified an incorrect system type. It should be %s." \
                            " File (%s), row (%d) column (%d).",
                            pXmlAttrib->Value(), pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );

                //
                // Store the name of the library.
                //
                m_pszSystemType = pXmlAttrib->Value();
            }
            else if ( strcmp( pszName, "Lib" ) == 0 )
            {
                //
                // Load the system library.
                //
                Singletons::PlatformManager.FileSystem().LoadSystem(
                    pXmlAttrib->Value(), &m_pSystem
                    );
				

                ASSERTMSG4( m_pSystem != NULL,
                            "Parser could not load the system %s." \
                            " File (%s), row (%d) column (%d).",
                            pXmlAttrib->Value(), pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );

                //
                // Get the properties of the system.
                //
                m_pSystem->GetProperties( m_GetProperties );

                ASSERTMSG4( strcmp( m_pszSystemType, m_pSystem->GetName() ) == 0,
                            "Parser identified an incorrect system type. It should be %s." \
                            " File (%s), row (%d) column (%d).",
                            m_pszSystemType, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );
            }
            else if ( strcmp( pszName, "SDF" ) == 0 )
            {
                ASSERT( m_pSystem == NULL );

                //
                // Create a new xml document for the sdf.
                //
                char    pszSDF[ 256 ];

				strcpy_s( pszSDF, sizeof pszSDF, pXmlAttrib->Value() );
                strcat_s( pszSDF, sizeof pszSDF, ".sdf" );

                TiXmlDocument XmlDoc( pszSDF );

                if ( XmlDoc.LoadFile() == False )
                {
                    ASSERTMSG1( False, "Parser failed to load the SDF file %s.", pszSDF );
                }

                //
                // Parse the sdf.
                //
                m_GdfMarker = GDFM_Systems;
                m_SystemLevel++;

                ParseElement( XmlDoc.FirstChildElement() );

                m_SystemLevel--;
                m_GdfMarker = GDFM_System;
            }

            pXmlAttrib = pXmlAttrib->Next();
        }
    }
	else if ( m_GdfMarker == GDFM_Environment )
	{
        // <Environment> currently has no attributes so do nothing.
	}
	else if ( m_GdfMarker == GDFM_EnvVar )
	{
		pcstr pszVariableName = NULL;
		pcstr pszVariableValue = NULL;

        while ( pXmlAttrib != NULL )
        {
            pcstr pszName = pXmlAttrib->Name();

            if ( strcmp( pszName, "Name" ) == 0 )
            {
				ASSERTMSG4( pszVariableName == NULL,
							"Duplicate attribute found '%s'.",
                            pszName, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );

                pszVariableName = pXmlAttrib->Value();
            }
            else if ( strcmp( pszName, "Value" ) == 0 )
            {
				ASSERTMSG4( pszVariableValue == NULL,
							"Duplicate attribute found '%s'.",
                            pszName, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );

                pszVariableValue = pXmlAttrib->Value();
            }
            else
            {
                ASSERTMSG4( False,
                            "Parser encountered an unknown <Variable> attribute '%s'." \
                            " File (%s), row (%d) column (%d).",
                            pszName, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );
            }

            pXmlAttrib = pXmlAttrib->Next();
        }
		ASSERT( pszVariableName != NULL  &&  pszVariableValue != NULL );

		Singletons::EnvironmentManager.Variables().Add( pszVariableName, pszVariableValue );
	}
    else if ( m_GdfMarker == GDFM_SystemProperty )
    {
        ASSERTMSG3( m_GetProperties.empty() == False,
                    "Parser found an empty property element." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(),
                    pXmlElement->Row(), pXmlElement->Column() );

        ReadPropertyAttributes( pXmlElement, pXmlAttrib, m_GetProperties, m_SetProperties );
    }
    else if ( m_GdfMarker == GDFM_Scenes )
    {
        while ( pXmlAttrib != NULL )
        {
            pcstr pszName = pXmlAttrib->Name();

            if ( strcmp( pszName, "Startup" ) == 0 )
            {
                m_sNextScene = pXmlAttrib->Value();
            }
            else
            {
                ASSERTMSG4( False,
                            "Parser encountered an unknown <Scenes> attribute %s." \
                            " File (%s), row (%d) column (%d).",
                            pszName, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );
            }

            pXmlAttrib = pXmlAttrib->Next();
        }
    }
    else if ( m_GdfMarker == GDFM_Scene )
    {
        while ( pXmlAttrib != NULL )
        {
            pcstr pszName = pXmlAttrib->Name();
            pcstr pszValue = pXmlAttrib->Value();

            if ( strcmp( pszName, "Name" ) == 0 )
            {
                ASSERTMSG3( m_SceneLevel < 2,
                            "Scene names are only valid for non-included CDFs." \
                            " File (%s), row (%d) column (%d).",
                            pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );

                ASSERTMSG4( m_SceneLevel == 0 || strcmp( m_pszSceneName, pszValue ) == 0,
                            "Parser identified an incorrect scene name. It should be %s." \
                            " File (%s), row (%d) column (%d).",
                            m_pszSceneName, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );

                m_pszSceneName = pszValue;
            }
            else if ( strcmp( pszName, "CDF" ) == 0 )
            {
                ASSERTMSG3( m_SceneLevel < 1,
                            "Scene CDFs are only valid from the GDFs." \
                            " File (%s), row (%d) column (%d).",
                            pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );

                //
                // Create a new xml document for the cdf.
                //
                char    pszCDF[ 256 ];
                strcpy_s( pszCDF, sizeof pszCDF, pszValue );
                strcat_s( pszCDF, sizeof pszCDF, ".cdf" );

                TiXmlDocument XmlDoc( pszCDF );

                if ( XmlDoc.LoadFile() == False )
                {
                    ASSERTMSG1( False, "Parser was unable to load CDF file %s.", pszCDF );
                }

                //
                // Parse the sdf.
                //
                m_GdfMarker = GDFM_Scenes;
                m_SceneLevel++;

                ParseElement( XmlDoc.FirstChildElement() );

                m_SceneLevel--;
                m_GdfMarker = GDFM_Scene;
            }
            else if ( strcmp( pszName, "NextScene" ) == 0 )
            {
                ASSERTMSG4( m_SceneLevel == 0,
                            "Parser identified a %s attribute within an SDF." \
                            " This can only be set in the GDF." \
                            " File (%s), row (%d) column (%d).",
                            pszName, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );

                m_sNextScene = pszValue;
            }
            else
            {
                ASSERTMSG4( False,
                            "Parser encountered an unknown <Scene> attribute %s." \
                            " File (%s), row (%d) column (%d).",
                            pszName, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );
            }

            pXmlAttrib = pXmlAttrib->Next();
        }
    }
    else if ( m_GdfMarker == GDFM_SceneProperties )
    {
        while ( pXmlAttrib != NULL )
        {
            pcstr pszName = pXmlAttrib->Name();
            pcstr pszValue = pXmlAttrib->Value();

            if ( strcmp( pszName, "SystemType" ) == 0 )
            {
                m_pSystem = Singletons::SystemManager.Get( pszValue );
                ASSERTMSG4( m_pSystem != NULL, "Parser was unable to get system %s." \
                            " File (%s), row (%d) column (%d).",
                            pszValue, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );

                if ( m_pSystem != NULL )
                {
                    m_GetProperties.clear();
                    m_SetProperties.clear();

                    UScene::SystemScenesConstIt it =
                        m_pScene->GetSystemScenes().find( m_pSystem->GetSystemType() );
                    ASSERTMSG4( it != m_pScene->GetSystemScenes().end(),
                                "Parser was unable to find a scene for system %s." \
                                " File (%s), row (%d) column (%d).",
                                pszValue, pXmlElement->GetDocument()->Value(),
                                pXmlAttrib->Row(), pXmlAttrib->Column() );

                    m_pSystemScene = it->second;
                    ASSERT( m_pSystemScene != NULL );

                    m_pSystemScene->GetProperties( m_GetProperties );
                }
            }
            else
            {
                ASSERTMSG4( False,
                            "Parser encountered an unknown scene <Properties> attribute %s." \
                            " File (%s), row (%d) column (%d).",
                            pszName, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );
            }

            pXmlAttrib = pXmlAttrib->Next();
        }
    }
    else if ( m_GdfMarker == GDFM_SceneProperty )
    {
        ASSERTMSG3( m_GetProperties.empty() == False,
                    "Parser found an empty property element." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(),
                    pXmlElement->Row(), pXmlElement->Column() );

        ReadPropertyAttributes( pXmlElement, pXmlAttrib, m_GetProperties, m_SetProperties );
    }
    else if ( m_GdfMarker == GDFM_Object )
    {
        ASSERTMSG3( m_pUObject != NULL,
                    "Parser does not have a UObject to work with." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(),
                    pXmlElement->Row(), pXmlElement->Column() );

        while ( pXmlAttrib != NULL )
        {
            pcstr pszName = pXmlAttrib->Name();
            pcstr pszValue = pXmlAttrib->Value();

            if ( strcmp( pszName, "Name" ) == 0 )
            {
                if ( m_ObjectLevel == 0 )
                {
                    m_pszObjectName = pszValue;

                    if ( m_pszObjectName != NULL )
                    {
                        m_pUObject->SetName( m_pszObjectName );
                    }
                }
            }
            else if ( strcmp( pszName, "ODF" ) == 0 )
            {
                //
                // Create a new xml document for the sdf.
                //
                char    pszODF[ 256 ];
                strcpy_s( pszODF, sizeof pszODF, pszValue );
                strcat_s( pszODF, sizeof pszODF, ".odf" );

                TiXmlDocument XmlDoc( pszODF );

                if ( XmlDoc.LoadFile() == False )
                {
                    ASSERTMSG1( False, "Parser was unable to load ODF file %s.", pszODF );
                }

                //
                // Parse the sdf.
                //
                m_GdfMarker = GDFM_Objects;
                m_ObjectLevel++;

                ParseElement( XmlDoc.FirstChildElement() );

                m_ObjectLevel--;
                m_GdfMarker = GDFM_Object;
            }
            else
            {
                ASSERTMSG4( False,
                            "Parser encountered an unknown <Object> attribute %s." \
                            " File (%s), row (%d) column (%d).",
                            pszName, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );
            }

            pXmlAttrib = pXmlAttrib->Next();
        }
    }
    else if ( m_GdfMarker == GDFM_ObjectProperties )
    {
        ASSERTMSG3( m_pUObject != NULL,
                    "Parser does not have a UObject to work with." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(),
                    pXmlElement->Row(), pXmlElement->Column() );

        //
        // Clear the properties.
        //
        m_GetProperties.clear();
        m_SetProperties.clear();

        while ( pXmlAttrib != NULL )
        {
            pcstr pszName = pXmlAttrib->Name();
            pcstr pszValue = pXmlAttrib->Value();

            if ( strcmp( pszName, "SystemType" ) == 0 )
            {
                m_pSystem = Singletons::SystemManager.Get( pszValue );
                ASSERTMSG4( m_pSystem != NULL, "Parser was unable to get system %s." \
                            " File (%s), row (%d) column (%d).",
                            pszValue, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );

                if ( m_pSystem != NULL )
                {
                    m_pSystemObject = m_pUObject->GetExtension( m_pSystem->GetSystemType() );

                    if ( m_pSystemObject != NULL )
                    {
                        m_pSystemObject->GetProperties( m_GetProperties );
                    }
                }
            }
            else if ( strcmp( pszName, "ObjectType" ) == 0 )
            {
                m_pszObjectType = pszValue;
            }
            else
            {
                ASSERTMSG4( False,
                            "Parser encountered an unknown object <Properties> attribute %s." \
                            " File (%s), row (%d) column (%d).",
                            pszName, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );
            }

            pXmlAttrib = pXmlAttrib->Next();
        }

        //
        // Create the object if there isn't one yet.
        //
        if ( m_pSystemObject == NULL )
        {
            if ( m_pSystem != NULL )
            {
                UScene::SystemScenesConstIt it =
                    m_pScene->GetSystemScenes().find( m_pSystem->GetSystemType() );
                ASSERTMSG3( it != m_pScene->GetSystemScenes().end(),
                            "Parser was unable to find a scene for this system." \
                            " File (%s), row (%d) column (%d).",
                            pXmlElement->GetDocument()->Value(),
                            pXmlElement->Row(), pXmlElement->Column() );

                m_pSystemObject = m_pUObject->Extend( it->second, m_pszObjectType );
                ASSERT( m_pSystemObject != NULL );

                m_pSystemObject->GetProperties( m_GetProperties );

                //
                // Create an entry in the set properties map.
                //
                m_SetPropertiesMap[ m_pSystem->GetSystemType() ] = Properties::Array();
            }
        }
    }
    else if ( m_GdfMarker == GDFM_ObjectProperty )
    {
        ASSERTMSG3( m_GetProperties.empty() == False,
                    "Parser found an empty property element." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(),
                    pXmlElement->Row(), pXmlElement->Column() );

        //
        // Get the system object's type.
        //
        System::Type SystemType = m_pSystem->GetSystemType();

        //
        // If there isn't any entry in the properties map then create one.
        //
        std::map<u32,Properties::Array>::const_iterator it =
            m_SetPropertiesMap.find( SystemType );

        if ( it == m_SetPropertiesMap.end() )
        {
            m_SetPropertiesMap[ SystemType ] = Properties::Array();
        }

        //
        // Read in the properties.
        //
        ReadPropertyAttributes( pXmlElement, pXmlAttrib, m_GetProperties, m_SetPropertiesMap[ SystemType ] );
    }
    else if ( m_GdfMarker == GDFM_Links )
    {
        // <Links> currently has no attributes so do nothing.
    }
    else if ( m_GdfMarker == GDFM_Link )
    {
        uObject* pSubject = NULL;
        uObject* pObserver = NULL;
        std::string sSystemSubject;
        std::string sSystemObserver;

        while ( pXmlAttrib != NULL )
        {
            pcstr pszName = pXmlAttrib->Name();
            pcstr pszValue = pXmlAttrib->Value();

            if ( strcmp( pszName, "Subject" ) == 0 )
            {
                pSubject = m_pScene->FindObject( pszValue );
            }
            else if ( strcmp( pszName, "Observer" ) == 0 )
            {
                pObserver = m_pScene->FindObject( pszValue );
            }
            else if ( strcmp( pszName, "SubjectSystem" ) == 0 )
            {
                sSystemSubject = pszValue;
            }
            else if ( strcmp( pszName, "ObserverSystem" ) == 0 )
            {
                sSystemObserver = pszValue;
            }

            pXmlAttrib = pXmlAttrib->Next();
        }
        ASSERTMSG3( pSubject != NULL, "Parser did not find the UObject subject." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(),
                    pXmlElement->Row(), pXmlElement->Column() );
        ASSERTMSG3( pObserver != NULL, "Parser did not find the UObject observer." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(),
                    pXmlElement->Row(), pXmlElement->Column() );
        //ASSERTMSG3( !sSystemSubject.empty(), "Parser did not find the system subject." \
        //            " File (%s), row (%d) column (%d).",
        //            pXmlElement->GetDocument()->Value(),
        //            pXmlElement->Row(), pXmlElement->Column() );
        ASSERTMSG3( !sSystemObserver.empty(), "Parser did not find the system observer." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(),
                    pXmlElement->Row(), pXmlElement->Column() );

        ISystemObject* pSystemSubject = NULL;
        ISystemObject* pSystemObserver = NULL;

        ISystem* pSystem;

        //
        // Get the extension for the subject.
        //
        if ( !sSystemSubject.empty() )
        {
            pSystem = Singletons::SystemManager.Get( sSystemSubject.c_str() );
            ASSERTMSG4( pSystem != NULL,
                        "Parser identified system %s as not loaded." \
                        " File (%s), row (%d) column (%d).",
                        sSystemSubject.c_str(), pXmlElement->GetDocument()->Value(),
                        pXmlElement->Row(), pXmlElement->Column() );

            pSystemSubject = pSubject->GetExtension( pSystem->GetSystemType() );
            ASSERTMSG3( pSystemSubject != NULL,
                        "Parser identifed subject system as not extending subject object." \
                        " File (%s), row (%d) column (%d).",
                        pXmlElement->GetDocument()->Value(),
                        pXmlElement->Row(), pXmlElement->Column() );
        }

        //
        // Get the extension for the object.
        //
        pSystem = Singletons::SystemManager.Get( sSystemObserver.c_str() );
        ASSERTMSG4( pSystem != NULL,
                    "Parser identified system %s as not loaded." \
                    " File (%s), row (%d) column (%d).",
                    sSystemObserver.c_str(), pXmlElement->GetDocument()->Value(),
                    pXmlElement->Row(), pXmlElement->Column() );

        pSystemObserver = pObserver->GetExtension( pSystem->GetSystemType() );
        ASSERTMSG3( pSystemObserver != NULL,
                    "Parser identifed observer system as not extending observer object." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(),
                    pXmlElement->Row(), pXmlElement->Column() );

        //
        // Call the scene to register the links.
        //
        if ( pSystemSubject != NULL )
        {
            m_pScene->CreateObjectLink( pSystemSubject, pSystemObserver );
        }
        else
        {
            m_pScene->CreateObjectLink( pSubject, pSystemObserver );
        }
    }
    else if ( m_GdfMarker == GDFM_Include )
    {
        while ( pXmlAttrib != NULL )
        {
            pcstr pszName = pXmlAttrib->Name();
            pcstr pszValue = pXmlAttrib->Value();

            if ( strcmp( pszName, "CDF" ) == 0 )
            {
                //
                // Create a new xml document for the cdf.
                //
                char    pszCDF[ 256 ];
                strcpy_s( pszCDF, sizeof pszCDF, pszValue );
                strcat_s( pszCDF, sizeof pszCDF, ".cdf" );

                TiXmlDocument XmlDoc( pszCDF );

                if ( XmlDoc.LoadFile() == False )
                {
                    ASSERTMSG1( False, "Parser was unable to load CDF file %s.", pszCDF );
                }

                //
                // Parse the sdf.
                //
                m_GdfMarker = GDFM_Scenes;
                m_SceneLevel++;

                ParseElement( XmlDoc.FirstChildElement() );

                m_SceneLevel--;
                m_GdfMarker = GDFM_Include;
            }
            else
            {
                ASSERTMSG4( False,
                            "Parser encountered an unknown <Include> attribute %s." \
                            " File (%s), row (%d) column (%d).",
                            pszName, pXmlElement->GetDocument()->Value(),
                            pXmlAttrib->Row(), pXmlAttrib->Column() );
            }

            pXmlAttrib = pXmlAttrib->Next();
        }
    }
    else
    {
        ASSERTMSG3( False, "Parser encountered an unsupported marker." \
                    " File (%s), row (%d) column (%d).",
                    pXmlElement->GetDocument()->Value(),
                    pXmlElement->Row(), pXmlElement->Column() );
    }
}


void
Framework::GDFParser::ReadPropertyAttributes(
    void* pElement,
    void* pAttributes,
    std::vector<Properties::Property>& GetProperties,
    std::vector<Properties::Property>& SetProperties
    ) const
{
    TiXmlElement* pXmlElement = reinterpret_cast<TiXmlElement*>(pElement);
    TiXmlAttribute* pXmlAttrib = reinterpret_cast<TiXmlAttribute*>(pAttributes);

#ifdef NDEBUG
    UNREFERENCED_PARAM( pXmlElement );
#endif

    Properties::ConstIterator GetPropIt = GetProperties.end();
    size_t iProp = 0xFFFFFFFF;

    while ( pXmlAttrib != NULL )
    {
        pcstr pszName = pXmlAttrib->Name();

        if ( strcmp( pszName, "Name" ) == 0 )
        {
            //
            // Search for the name in the property array.
            //
            pcstr pszValue = pXmlAttrib->Value();

            for ( Properties::ConstIterator it=GetProperties.begin();
                  it != GetProperties.end(); it++ )
            {
                if ( strcmp( it->GetName(), pszValue ) == 0 )
                {
                    SetProperties.push_back( *it );
                    iProp = SetProperties.size() - 1;
                    GetPropIt = it;
                    break;
                }
            }

            ASSERTMSG4( GetPropIt != GetProperties.end(),
                        "Parser could not find the property named %s in the list given by the system." \
                        " File (%s), row (%d) column (%d).",
                        pszValue, pXmlElement->GetDocument()->Value(),
                        pXmlElement->Row(), pXmlElement->Column() );
        }
        else if ( _stricmp( pszName, GetPropIt->GetValueName( 0 ) ) == 0 ||
                  _stricmp( pszName, GetPropIt->GetValueName( 1 ) ) == 0 ||
                  _stricmp( pszName, GetPropIt->GetValueName( 2 ) ) == 0 ||
                  _stricmp( pszName, GetPropIt->GetValueName( 3 ) ) == 0 )
        {
            ASSERTMSG4( GetPropIt != GetProperties.end(),
                        "Parser encountered a property value %s that occured before the property name." \
                        " File (%s), row (%d) column (%d).",
                        pszName, pXmlElement->GetDocument()->Value(),
                        pXmlElement->Row(), pXmlElement->Column() );

            //
            // Determine the index of the value.
            //
            i32 iValue;
            for ( iValue=0; iValue < Properties::Values::Count; iValue++ )
            {
                if ( _stricmp( pszName, GetPropIt->GetValueName( iValue ) ) == 0 )
                {
                    break;
                }
            }
            ASSERTMSG4( iValue != Properties::Values::Count,
                        "Parser encoutered an unknown value '%s'." \
                        " File (%s), row (%d) column (%d).",
                        pszName, pXmlElement->GetDocument()->Value(),
                        pXmlElement->Row(), pXmlElement->Column() );

            //
            // Read in the property from the xml.
            //
            switch ( GetPropIt->GetValueType( iValue ) )
            {
            case Properties::Values::None:
            ASSERTMSG4( False,
                        "Parser encoutered a value '%s' with no type." \
                        " File (%s), row (%d) column (%d).",
                        pszName, pXmlElement->GetDocument()->Value(),
                        pXmlElement->Row(), pXmlElement->Column() );
                break;

            case Properties::Values::Boolean:
            case Properties::Values::Int32:
                SetProperties[ iProp ].SetValue( iValue, pXmlAttrib->IntValue() );
                break;

            case Properties::Values::Enum:
            {
                pcstr pszEnumOption = pXmlAttrib->Value();

                for ( i32 i=0; GetPropIt->GetEnumOption( i ) != NULL; i++ )
                {
                    if ( strcmp( pszEnumOption, GetPropIt->GetEnumOption( i ) ) == 0 )
                    {
                        SetProperties[ iProp ].SetValue( iValue, i );
                        break;
                    }
                }
                break;
            }

            case Properties::Values::Float32:
            case Properties::Values::Vector3 & Properties::Values::Mask:
            case Properties::Values::Vector4 & Properties::Values::Mask:
            case Properties::Values::Quaternion & Properties::Values::Mask:
            case Properties::Values::Color3 & Properties::Values::Mask:
            case Properties::Values::Color4 & Properties::Values::Mask:
                SetProperties[ iProp ].SetValue( iValue,
                                                 static_cast<f32>(pXmlAttrib->DoubleValue()) );
                break;

            case Properties::Values::Angle:
                SetProperties[ iProp ].SetValue(
                    iValue, Math::Angle::Deg2Rad( static_cast<f32>(pXmlAttrib->DoubleValue()) )
                    );
                break;

            case Properties::Values::String:
            case Properties::Values::Path:
                SetProperties[ iProp ].SetValue( iValue, pXmlAttrib->Value() );
                break;

            default:
                ASSERTMSG3( False,
                            "Parser encountered an unsupported property value." \
                            " File (%s), row (%d) column (%d).",
                            pXmlElement->GetDocument()->Value(),
                            pXmlElement->Row(), pXmlElement->Column() );
                break;
            }
        }
        else
        {
            ASSERTMSG4( False,
                        "Parser encountered an unknown property attribute '%s'." \
                        " File (%s), row (%d) column (%d).",
                        pszName, pXmlElement->GetDocument()->Value(),
                        pXmlAttrib->Row(), pXmlAttrib->Column() );
        }

        pXmlAttrib = pXmlAttrib->Next();
    }
}
