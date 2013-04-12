#pragma once 

#include "Client.h"

//
//	ELevelViewportType
//

enum ELevelViewportType
{
	LVT_None = -1,
	LVT_OrthoXY = 0,
	LVT_OrthoXZ = 1,
	LVT_OrthoYZ = 2,
	LVT_Perspective = 3
};

struct FEditorLevelViewportClient : public FViewportClient
{
	FViewport*	Viewport;

	/** Viewport's horizontal field of view. */
	FLOAT					ViewFOV;

	FLOAT					OrthoZoom;
	void Tick(FLOAT DeltaTime);
	void UpdateCameraMovement( FLOAT DeltaTime );
	void UpdateMouseDelta();
};