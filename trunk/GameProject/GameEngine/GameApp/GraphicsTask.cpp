#include "stdafx.h"
#include "ColorValue.h"
#include "Common.h"

#include ".\BaseTypes\BaseTypes.h"
#include ".\Interfaces\Interface.h"

// UPDATE: provides access to environment variables
extern ManagerInterfaces   g_GraphicsManagers;

#include "GraphicsScene.h"
#include "GraphicsTask.h"

#include "GraphicsObject.h"
#include "ObjectMesh.h"

#include "./Framework/IttNotify.h"

__ITT_DEFINE_STATIC_EVENT( g_tpeRendering, "Graphics: Rendering", 19 );


GraphicsTask::GraphicsTask( GraphicsScene* pScene )
	: ISystemTask( (ISystemScene*)pScene )
	, m_pScene( pScene )

{

}

GraphicsTask::~GraphicsTask( void )
{

}

System::Type GraphicsTask::GetSystemType( void )
{
	return System::Types::Graphics;
}

enum PolygonModes
{
	PolygonMode_Invalid=-1,
	PolygonMode_Solid,
	PolygonMode_WireFrame,
	PolygonMode_WireFrame_NoTex,
	PolygonMode_Points,
	PolygonMode_Points_NoTex,
	PolygonMode_Solid_NoTex,
	PolygonMode_Count
};

void GraphicsTask::Update( f32 DeltaTime )
{
	static const std::string sCaption = "Caption";
	static Bool bMaterialsSaved     = False;
	static i32  iPolygonModeOld     = 0;
	static Bool bShowNormalsOld     = False;
	static Bool bShowTangentsOld    = False;
	static Bool bShowCaptionsOld    = False;
	static Bool bShowBoundingBoxOld = False;

	// Get current state as updated by the input system.

	i32  iPolygonMode     = g_GraphicsManagers.pEnvironment->Variables().GetAsInt( "Graphics::PolygonMode" );
	Bool bShowNormals     = g_GraphicsManagers.pEnvironment->Variables().GetAsBool( "Graphics::ShowNormals" );
	Bool bShowTangents    = g_GraphicsManagers.pEnvironment->Variables().GetAsBool( "Graphics::ShowTangents" );
	Bool bShowBoundingBox = g_GraphicsManagers.pEnvironment->Variables().GetAsBool( "Graphics::ShowBoundingBox" );
	Bool bShowCaptions    = g_GraphicsManagers.pEnvironment->Variables().GetAsBool( "Graphics::ShowCaptions" );


	GraphicsScene::ObjectsList* pObjects = &(((GraphicsScene*)m_pScene)->m_Objects);
	GraphicsScene::ObjectsList::iterator it;
	ASSERT( pObjects );

	//
	// Only parse the object list if we have to make any Graphics state change
	//
	if( iPolygonMode  != iPolygonModeOld )
	{
		iPolygonModeOld = iPolygonMode;

		PolygonModes polygonMode = (PolygonModes)(iPolygonMode % (int)PolygonMode_Count);

		for (it = pObjects->begin(); it != pObjects->end(); it++ ) 
		{
			GraphicsObject* pObject = *it;

			if(GraphicsObject::Type_Camera == pObject->GetType())
			{

			}
			else if( ( GraphicsObject::Type_Mesh == pObject->GetType() ) ||
				( GraphicsObject::Type_MeshAnimated == pObject->GetType() ) ) 
			{
				GraphicsObjectMesh* pMesh = dynamic_cast<GraphicsObjectMesh*>( pObject );
				ASSERT( pMesh != NULL );

			}
		}
		
	}

	// Since rendering is a limiting serial stage in some (if not most) of the frames,
	// we do not want it to be preempted. So temporarily boost up its thread priority.
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	//
	// Render the scene
	//
	__ITT_EVENT_START(g_tpeRendering, PROFILE_RENDER);
	//m_pRoot->renderOneFrame();
	__ITT_EVENT_END(g_tpeRendering, PROFILE_RENDER);

	// Since it's the pool thread, we know that normally it runs at normal priority.
	// In more general case we would have needed to remember the initial priority 
	// before bringing it up.
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

	//
	// Update objects for next frame
	//
	m_pScene->Update( DeltaTime );
}

Bool GraphicsTask::IsPrimaryThreadOnly( void )
{
	return False;
}
