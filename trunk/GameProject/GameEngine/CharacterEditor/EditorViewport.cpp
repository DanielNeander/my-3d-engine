#include "EditorPCH.h"
#include "EditorViewport.h"


void FEditorLevelViewportClient::Tick(FLOAT DeltaTime)
{
	// Update any real-time camera movement
	UpdateCameraMovement( DeltaTime );

	UpdateMouseDelta();
}

void FEditorLevelViewportClient::UpdateCameraMovement( FLOAT DeltaTime )
{
	// We only want to move perspective cameras around like this
	if( Viewport != NULL && ViewportType == LVT_Perspective )
	{


	}
}

void FEditorLevelViewportClient::UpdateMouseDelta()
{


}