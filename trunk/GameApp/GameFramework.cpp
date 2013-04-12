#include "stdafx.h"

#pragma warning( push )
#pragma warning( disable : 4635 6011 )
#include "TinyXML/TinyXml.h"
#pragma warning( pop )

#include ".\BaseTypes\BaseTypes.h"
#include ".\Interfaces\Interface.h"

#include ".\Framework\Universal.h"
#include ".\Framework\DebugWindow.h"
#include ".\Framework\MemoryManager.h"
#include ".\Framework\SystemManager.h"
#include ".\Framework\PlatformManager.h"
#include ".\Framework\ChangeControlManager.h"
#include ".\Framework\EnvironmentManager.h"
#include ".\Framework\ServiceManager.h"
#include ".\Framework\Scheduler.h"
#include ".\Framework\TaskManager.h"
#include ".\Framework\TaskManagerTP.h"
//#include ".\Framework\TaskManagerTBB.h"
#include ".\Framework\Instrumentation.h"


#include "GameFramework.h"

#include "GameApp/GameApp.h"

TaskManager*     g_pTaskManager = NULL;


void
	ExecuteGDF(
	pcstr pszGDF
	)
{
#ifndef _DEBUG
	try
#endif
	{
		GameFramework	Framework;

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

Error GameFramework::Initialize( pcstr pszGDF )
{
	Framework::Initialize( pszGDF);

	return Errors::Success;

}

void GameFramework::Shutdown( void )
{
	Framework::Shutdown();
}

Error GameFramework::Execute( void ) throw ( ... )
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
