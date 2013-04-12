#pragma once 

#include "GameObjectAttribute.h"
#include "ShadowCamera.h"

/**
 * Point of View Type
 */
struct FTPOV
{
	FVector		Location;
	noAngles	Rotation;
	FLOAT		FOV;

	FTPOV() {}
	FTPOV(FVector InLocation, noAngles InRotation, FLOAT InFOV): Location(InLocation), Rotation(InRotation), FOV(InFOV) {}
};

struct FTCameraCache
{
	FLOAT TimeStamp;
	FTPOV POV;

	/** Constructors */
	FTCameraCache() {}
	FTCameraCache(EEventParm)
	{
		appMemzero(this, sizeof(FTCameraCache));
	}
};

struct FTViewTarget
{
	class Actor* Target;
	//class AController* Controller;
	FTPOV POV;
	FLOAT AspectRatio;
	//class APlayerReplicationInfo* PRI;

	/** Constructors */
	FTViewTarget() {}
	FTViewTarget(EEventParm)
	{
		appMemzero(this, sizeof(FTViewTarget));
	}
};

struct FViewTargetTransitionParams
{
	FLOAT BlendTime;
	BYTE BlendFunction;
	FLOAT BlendExp;

	/*FViewTargetTransitionParams()
	{}*/
	FViewTargetTransitionParams()
	//	: BlendTime(0.f), BlendFunction(VTBlend_Cubic), BlendExp(2.f)
	{}

};

struct Camera_eventApplyCameraModifiers_Parms
{
	FLOAT DeltaTime;
	FTPOV OutPOV;
	Camera_eventApplyCameraModifiers_Parms()
	{
	}
};
struct Camera_eventUpdateCamera_Parms
{
	FLOAT DeltaTime;
	Camera_eventUpdateCamera_Parms()
	{
	}
};



// --------------------------------------------------------
// This class is used to perform smooth camera movement 
// by the follow cameras
// --------------------------------------------------------
class CameraRotator
{
public:
	CameraRotator();

	void SetMatrix(const noMat3 &matrix)     { m_matrix = matrix; }
	noMat3 GetMatrix()                       { return m_matrix; }

	void SetRotateRate(float maxRate,float minRate=0.01f);
	void SetDesiredYaw(float desiredYaw);

	void FaceDir(const noVec3 &dir);

	void Update(float fDelta);

private:
	float       m_fRate;
	float       m_fMinRate;
	float       m_fCurrYaw;
	float       m_fDesiredYaw;
	noMat3   m_matrix;
	bool        m_bHasDesiredYaw;

	float                       m_fTotalRotate;
	float                       m_fCurrRotate;


	float GetRate();
};



class FocusCamera : public ShadowMapCamera 
{
public:
	FocusCamera(const float* from, const float* to, const float* up, float fov);

	virtual ~FocusCamera();

	virtual void Initialize();

	virtual void ResetPosition();
	
	virtual void Setup(GameObject *pkTarget, float zOffset);
	virtual void SetTarget(GameObject *pkTarget);
	virtual void UpdateCamera(float fDelta);
protected:
	GameObjectWPtr        m_pkTarget;
	float                 m_zOffset;
	noVec3                m_originalPos;

	virtual void FaceTarget();
};

// --------------------------------------------------------
// Follow cam will:
//      1. always face the target + offset 
//      2. will attempt to stay a specified offset from the target
//      3. that offset will be from the facing position of the target
//  
// --------------------------------------------------------
class FollowCamera : public FocusCamera
{
public:
	FollowCamera(const float* from, const float* to, const float* up, float fov);


	virtual void Setup(GameObject *pkTarget, float zOffset);
	virtual void SetupFollowCam(float cameraOffsetForward, float cameraOffsetUp);
	virtual void SetRotateRate(float maxRate,float minRate=0);
	virtual void SetYawOffset(float offset)             { m_yawOffset = offset; }	
	virtual void UpdateCamera( float fDelta );


protected:
	float           m_cameraOffsetUp;
	float           m_cameraOffsetForward;
	float           m_cameraMoveRate;
	float           m_cameraDist;
	CameraRotator   m_rotator;
	float           m_yawOffset;

private:
	void InitVars();
};


class FirstPersonCameraAttrib : public GameObjectAttributeBase
{
public:
	enum EMoveState
	{
		FLYING,
		LANDING,
		WALKING
	};

	float m_fCamDist;  // follow-distance of the camera 
	float m_fCamHeight; // height of camera look-at point
	float m_fCamPitch; // pitch of the camera mounting
	float m_fHeading; // current absolute yaw angle
	float m_fFrameYaw; // angular velocity-related outputs
	float m_fFramePitch;
	float m_fForward;
	float m_fStrafe;
	float m_fVertical;

	float m_fMaxCameraRaise; // Maximum allowed to 'pop' up
	float m_fMaxCameraDrop; // Maximum allowed to drop down
	float m_fWalkVelocity;
	float m_fRunVelocity;
	//    NiPoint3 m_kTrans;

	bool m_bInvert;
	EMoveState m_eMoveState;

	FirstPersonCameraAttrib();

	void ToggleFly();	

	noDeclareRTTI;
};

class FirstPersonMouseAndKeyboardInput : public GameObjectComponentBase
{
public:
	FirstPersonMouseAndKeyboardInput(GameObject *pkGameObject);

	bool Initialize();
	bool Update(float fDelta);

protected:
	FirstPersonCameraAttrib    *m_pkCameraAttrib;
};


//---------------------------------------------------------------------------
// Implementation of a Mover component for use with the first person camera
// This class handles moving the camera based on the input components and
// uses the Pickcontroller to keep the camera on the walkable mesh.
//---------------------------------------------------------------------------
class FirstPersonCameraMover : public GameObjectComponentBase
{
public:
	FirstPersonCameraMover(GameObject *pkGameObject);

	bool Initialize();
	bool Update(float fDelta);

protected:
	FirstPersonCameraAttrib        *m_pkCameraAttrib;

	float GetInitialHeight(const noVec3& kWorldTrans);

	bool TryPick(noVec3 &kNewPosition, const int iTry=0);

	noVec3 CalculateNewHeading(const float fOffsetAngleRadians,const float fMagnitude);

	void UpdateCameraTransforms();

	bool IsHeightInWalkingRange(const float fHeight);
};

//---------------------------------------------------------------------------
// Implementation of the first person camera GameObject.  This is basically
// just a collection of attributes and components. (along with some accessor functions)
//---------------------------------------------------------------------------
class FirstPersonCamera : public ShadowMapCamera
{
public:
	FirstPersonCamera(const float* from, const float* to, const float* up, float fov);


	void SetRunSpeed(float fRunSpeed);
	void SetWalkSpeed(float fWalkSpeed);
	float GetRunSpeed();
	float GetWalkSpeed();

	void UpdateCamera(float fDelta);
protected:
	FirstPersonCameraAttrib* m_pkCameraAttrib;

private:

	void SetupCamera();
};

