#pragma once 

#include "TestObjects.h"


class GameChar : public WowActor
{
public:

	void CalceCamera(float fDeltaTime, noVec3& CamLoc, noAngles& CamRot, float& FOV);

	noVec3 Location;
	noVec3 CamStart;
	noVec3 CurrentCameraOffset;
	noVec3 CamOffset;
	float CurrentCameraScale;	
	float CameraScale;
	float CameraZOffset;
};

void GameChar::CalceCamera( float fDeltaTime, noVec3& CamLoc, noAngles& CamRot, float& FOV )
{
	noVec3 CamStart, HitLocation, HitNormal, CamDirX, CamDirY, CamDirZ, CurrentCamOffset;
	float DesiredCameraZOffset;

	CamStart = Location;
	CurrentCamOffset = CamOffset;

	DesiredCameraZOffset = 0.0f;

	//DesiredCameraZOffset = (Health > 0) ? 1.2 * GetCollisionHeight() + Mesh.Translation.Z : 0.f;
	CameraZOffset = (fDeltaTime < 0.2) ? DesiredCameraZOffset * 5 * fDeltaTime + (1 - 5*fDeltaTime) * CameraZOffset : DesiredCameraZOffset;

	/*if ( Health <= 0 )
	{
		CurrentCamOffset = vect(0,0,0);
		CurrentCamOffset.X = GetCollisionRadius();
	}*/
	CamStart.z += CameraZOffset;

	noMat3 camMat = CamRot.ToMat3();
	camMat[0] = CamDirX; 
	camMat[1] = CamDirY; 
	camMat[2] = CamDirZ; 
	
	CamDirX *= CurrentCameraScale;


	//if ( (Health <= 0) || bFeigningDeath )
	//{
	//	// adjust camera position to make sure it's not clipping into world
	//	// @todo fixmesteve.  Note that you can still get clipping if FindSpot fails (happens rarely)
	//	FindSpot(GetCollisionExtent(),CamStart);
	//}
	if (CurrentCameraScale < CameraScale)
	{
		CurrentCameraScale = Min<float>(CameraScale, CurrentCameraScale + 5 * Max<float>(CameraScale - CurrentCameraScale, 0.3)*fDeltaTime);
	}
	else if (CurrentCameraScale > CameraScale)
	{
		CurrentCameraScale = Max<float>(CameraScale, CurrentCameraScale - 5 * Max<float>(CameraScale - CurrentCameraScale, 0.3)*fDeltaTime);
	}

	//if (CamDirX.Z > GetCollisionHeight())
	//{
	//	CamDirX *= Square<float>(cosf(out_CamRot.Pitch * 0.0000958738)); // 0.0000958738 = 2*PI/65536
	//}

	CamLoc = CamStart - CamDirX*CurrentCamOffset.x + CurrentCamOffset.y*CamDirY + CurrentCamOffset.z*CamDirZ;

	//if (Trace(HitLocation, HitNormal, out_CamLoc, CamStart, false, vect(12,12,12)) != None)
	{
	//	CamLoc = HitLocation;
	}
}





