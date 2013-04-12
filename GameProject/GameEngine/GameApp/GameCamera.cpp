#include "stdafx.h"
#include "GameApp/GameApp.h"
#include "GameCamera.h"
#include "GameObjectUtil.h"

CameraRotator::CameraRotator()
{
	m_bHasDesiredYaw= false;
	m_fDesiredYaw   = 0;
	m_fRate         = DEG2RAD(360);
	m_fMinRate      = DEG2RAD(1);
	m_fCurrYaw      = noMath::INFINITY;
}

void CameraRotator::SetRotateRate( float fRate, float fMinRate/*=0.01f*/ )
{
	m_fRate     = fRate;
	m_fMinRate  = fMinRate;
}

void CameraRotator::SetDesiredYaw( float desiredYaw )
{
	if (m_fCurrYaw == noMath::INFINITY)
	{	
		m_fDesiredYaw = m_fCurrYaw = desiredYaw;
		m_bHasDesiredYaw = false;
		noRotation rot(vec3_zero, UP_VECTOR, RAD2DEG(m_fCurrYaw));
		m_matrix = rot.ToMat3();
		return;
	}

	if( desiredYaw==m_fCurrYaw )
	{
		m_fDesiredYaw       = m_fCurrYaw;
		m_bHasDesiredYaw    = false;
		return;
	}

	if( desiredYaw!=m_fDesiredYaw )
	{
		m_fDesiredYaw       = desiredYaw;
		m_bHasDesiredYaw    = true;
	}
	else
	{
		return;
	}

	noVec3    currentForward = MathHelpers::GetForwardVectorFromMatrix(m_matrix);
	noMat3   temp;
	temp = noRotation(vec3_zero,UP_VECTOR, RAD2DEG(desiredYaw)).ToMat3();

	noVec3    destForward = MathHelpers::GetForwardVectorFromMatrix(temp);

	m_fCurrYaw      = MathHelpers::GetCurrentYaw(m_matrix);

	m_fTotalRotate   = MathHelpers::GetAngleBetweenActorDirections(destForward, currentForward);
	m_fTotalRotate   = DEG2RAD(m_fTotalRotate);
	m_fCurrRotate    = 0;
}
	

void CameraRotator::FaceDir( const noVec3 &dir )
{
	float dot       = FORWARD_VECTOR * dir;
	float angle     = acosf(dot);

	if ( dir * RIGHT_VECTOR > 0 )
		angle *=-1;

	SetDesiredYaw(angle);
}

void CameraRotator::Update( float fDelta )
{
	if( !m_bHasDesiredYaw )
		return;

	float rate = GetRate();

	if ( m_fTotalRotate >= 0.0f )
	{
		m_fCurrRotate	+= fDelta * rate;
		m_fCurrYaw      += fDelta * rate;

		if (m_fCurrRotate>=m_fTotalRotate)
		{
			m_fCurrRotate	= m_fTotalRotate;
			m_fTotalRotate	= 0.0f;
			m_fCurrYaw      = m_fDesiredYaw;
			m_bHasDesiredYaw = false;
		}
	}
	else
	{
		m_fCurrRotate	-= fDelta * rate;
		m_fCurrYaw      -= fDelta * rate;

		if (m_fCurrRotate<=m_fTotalRotate)
		{
			m_fCurrRotate	 = m_fTotalRotate;
			m_fTotalRotate	 = 0.0f;
			m_fCurrYaw       = m_fDesiredYaw;
			m_bHasDesiredYaw = false;
		}
	}
	noRotation rot(vec3_zero, UP_VECTOR, RAD2DEG(m_fCurrYaw));
	m_matrix = rot.ToMat3();
}

// slows down the rotation as we get closer to the 
// destination
float CameraRotator::GetRate()
{
	float delta = m_fTotalRotate - m_fCurrRotate;
	float alteredRate = m_fRate * (fabs(delta) / noMath::PI);

	if( alteredRate< m_fMinRate )
		return m_fMinRate;

	return alteredRate;
}

void FollowCamera::Setup( GameObject *pkTarget, float zOffset )
{
	FocusCamera::Setup(pkTarget,zOffset);
	noVec3    dir = pkTarget->GetTranslation() - GetFrom();
	MathHelpers::UP_COMPONENT(dir) = 0;
	dir.Normalize();

	noMat3 temp;
	MathHelpers::FaceDirectionLight(temp, dir);
	m_rotator.SetMatrix(temp);
	m_rotator.SetRotateRate(noMath::TWO_PI);
}

void FollowCamera::SetupFollowCam( float cameraOffsetForward, float cameraOffsetUp )
{
	m_cameraOffsetForward = cameraOffsetForward;
	m_cameraOffsetUp = cameraOffsetUp;
}

void FollowCamera::SetRotateRate( float maxRate,float minRate/*=0*/ )
{
	m_rotator.SetRotateRate(maxRate,minRate);
}

void FollowCamera::UpdateCamera( float fDelta )
{
	if( !m_pkTarget )
		return;

	noVec3 currPos = this->GetFrom();
	noVec3 destPos = m_pkTarget->GetTranslation();
	noMat3 targetRotation = m_pkTarget->GetRotation();

	float targetYaw = MathHelpers::GetCurrentYaw(targetRotation);
	targetYaw += m_yawOffset;

	if (m_cameraOffsetForward < 0)
		targetYaw += noMath::TWO_PI;

	noRotation temp(vec3_zero, UP_VECTOR, RAD2DEG(targetYaw));
	targetRotation = temp.ToMat3();

	noVec3    targetForward = MathHelpers::GetForwardVectorFromMatrix(targetRotation);

	destPos += (targetForward * m_cameraOffsetForward);
	MathHelpers::UP_COMPONENT(destPos) += m_cameraOffsetUp;

	m_rotator.SetDesiredYaw( targetYaw );
	m_rotator.Update(fDelta);

	noVec3 forward = MathHelpers::GetForwardVectorFromMatrix(m_rotator.GetMatrix());

	destPos = m_pkTarget->GetTranslation();
	destPos += (forward * fabs(m_cameraOffsetForward));
	MathHelpers::UP_COMPONENT(destPos) += m_cameraOffsetUp;
	setFrom(destPos.ToFloatPtr());

	FaceTarget();
	computeModelView();
	ComputeProjection();		
}

void FollowCamera::InitVars()
{
	m_cameraOffsetForward   = 3.0f;
	m_cameraOffsetUp        = 2.0f;
	m_cameraMoveRate        = 4.0f;
	m_cameraDist            = 5.0f;
	m_yawOffset             = 0;
}

FollowCamera::FollowCamera( const float* from, const float* to, const float* up, float fov )
	: FocusCamera(from, to, up, fov)
{
	InitVars();
}

FocusCamera::~FocusCamera()
{

}

void FocusCamera::ResetPosition()
{
	m_from = m_originalPos;
}

void FocusCamera::Setup( GameObject *pkTarget, float zOffset )
{
	m_pkTarget = pkTarget;
	m_zOffset = zOffset;
}

void FocusCamera::SetTarget( GameObject *pkTarget )
{
	m_pkTarget = pkTarget;
}

void FocusCamera::FaceTarget()
{
	if (!m_pkTarget)
	{
		return;
	}

	noVec3  targetPos		= m_pkTarget->GetTranslation();
	targetPos				+= (UP_VECTOR * m_zOffset);
	setTo(targetPos.ToFloatPtr());
	setUp(UP_VECTOR.ToFloatPtr());
}

FocusCamera::FocusCamera( const float* from, const float* to, const float* up, float fov )
	:ShadowMapCamera(from ,to, up, fov)
{
	m_originalPos = GetFrom();
}

void FocusCamera::UpdateCamera( float fDelta )
{
	if (!m_pkTarget )
		return;

	FaceTarget();

	computeModelView();
	ComputeProjection();	
}

void FocusCamera::Initialize()
{
	BaseCamera::Initialize();

	CreateTransformComponent();
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static noMat3 MakeRotationMatrix(const float fHeading, const float fPitch)
{
	if( MathHelpers::IsZUp() )
	{
		noRotation kV;
		return kV.ToMat3();		
	}
	else
	{
		noMat3 kV;
		noRotation   kYaw,kPitch,kRoll;
		noRotation   kRotX, kRotZ;

		kYaw.Set(vec3_zero, UP_VECTOR, RAD2DEG(fHeading));
		kPitch.Set(vec3_zero, noVec3(1, 0, 0), RAD2DEG(-fPitch - 1.5f));
		kRoll.Set(vec3_zero, noVec3(0, 0, 1), RAD2DEG(0));
				
		kRotX.Set(vec3_zero, noVec3(1, 0, 0), 90);
		kRotZ.Set(vec3_zero, noVec3(0, 0, 1), -90);
				
		kV = kRotX.ToMat3() * kRotZ.ToMat3() ;
		//kV = kRoll.ToMat3() * kYaw.ToMat3() * kPitch.ToMat3() * kV;
		kV = kV * kPitch.ToMat3() * kYaw.ToMat3() * kRoll.ToMat3();
		return kV;
	}
}

noImplementRTTI(FirstPersonCameraAttrib,IGameObjectAttribute);
FirstPersonCameraAttrib::FirstPersonCameraAttrib()
	: m_fCamDist(0.0f),
	m_fCamHeight(0.0f),
	m_fCamPitch(0.0f), 
	m_fHeading(0.0f), 
	m_fFrameYaw(0.0f),
	m_fFramePitch(0.0f), 
	m_fForward(0.0f), 
	m_fStrafe(0.0f), 
	m_fVertical(0.0f), 
	m_fMaxCameraRaise(1000.0f),
	m_fMaxCameraDrop(1000.0f), 
	m_fWalkVelocity(5.0f),
	m_fRunVelocity(10.0f), 
	m_bInvert(false),
	m_eMoveState(FLYING)
{

}

void FirstPersonCameraAttrib::ToggleFly()
{
	if (m_eMoveState==FLYING)
	{
		m_eMoveState = LANDING;
	}
	else
	{
		m_eMoveState = FLYING;
	}
}

FirstPersonMouseAndKeyboardInput::FirstPersonMouseAndKeyboardInput( GameObject *pkGameObject ) 
	: GameObjectComponentBase(pkGameObject)
{
	m_pkCameraAttrib    = GetGameObjectAttribute(pkGameObject,FirstPersonCameraAttrib);
	assert(m_pkCameraAttrib);
}

bool FirstPersonMouseAndKeyboardInput::Initialize()
{
	return true;
}

bool FirstPersonMouseAndKeyboardInput::Update( float fDelta )
{
	int iX = 0;
	int iY = 0;

	float fVelocity = m_pkCameraAttrib->m_fWalkVelocity;
	float fVelocityXDelta = fVelocity * fDelta;

	float fHeadingDelta = 0.0f;
	float fPitchDelta = 0.0f;

	if (!iX && !iY)
	{  
		GetApp()->GetMouseDelta(iX, iY);

		if (m_pkCameraAttrib->m_bInvert)
			iY = -iY;

		// Compute the change to the heading of the character, based on the
		// horizontal mouse motion - 
		// we must factor in the size of the screen in pixels to avoid the
		// sensitivity changing with screen resolution
		unsigned int uiAppWidth = GetApp()->getWidth();
		if (uiAppWidth > 0)
		{
			fHeadingDelta = noMath::PI * 0.5f * (float)(iX) 
				/ (float)uiAppWidth;
		}
		unsigned int uiAppHeight = GetApp()->getHeight();
		if (uiAppHeight > 0)
		{
			fPitchDelta = noMath::PI * 0.375f * (float)(iY)
				/ (float)uiAppHeight;
		}

		bool shift = !!( ::GetKeyState(VK_SHIFT) & 0x8000 );
		if (shift)
		{
			// Running: Recalc.
			fVelocity = m_pkCameraAttrib->m_fRunVelocity;
			fVelocityXDelta = fVelocity * fDelta;
		}

		if (GetKeyDown('A'))
		{
			m_pkCameraAttrib->m_fStrafe = -fVelocityXDelta;
		}
		else if (GetKeyDown('D'))
		{
			 m_pkCameraAttrib->m_fStrafe = fVelocityXDelta;
		}

		if (GetKeyDown('S'))
		{
			m_pkCameraAttrib->m_fForward = -fVelocityXDelta;

		}
		else if(GetKeyDown('W'))
		{
			m_pkCameraAttrib->m_fForward = fVelocityXDelta;

		}
	}

		// Compute the change to the heading of the character, based on the
		// horizontal mouse motion - we must factor in the size of the screen
		// in pixels to avoid the sensitivity changing with screen resolution
		if (fHeadingDelta)
		{
			m_pkCameraAttrib->m_fHeading += fHeadingDelta;
			m_pkCameraAttrib->m_fHeading = (float)fmod(m_pkCameraAttrib->m_fHeading, noMath::TWO_PI);
		}   

		// Avoid making the frame heading go nuts if, for any reason, the
		// framerate is very high.  Otherwise (normal framerate), the frame
		// heading (which is used by external classes to render effects such
		// as head turning) is based on the change in heading induced by the 
		// controller this frame
		if (fDelta > 0.005f && fHeadingDelta)
			m_pkCameraAttrib->m_fFrameYaw = -fHeadingDelta / (noMath::PI * fDelta);
		else
			m_pkCameraAttrib->m_fFrameYaw = 0.0f;

		if (fPitchDelta)
			m_pkCameraAttrib->m_fCamPitch += fPitchDelta;

		// Avoid making the frame pitch go nuts if the framerate is very high
		// for any reason.  Otherwise (normal framerate), the frame pitch
		// (which is used by external classes to render effects such as head
		// tilting) is based on the change in pitch induced by the controller
		// this frame
		if (fDelta > 0.005f)
			m_pkCameraAttrib->m_fFramePitch = -fPitchDelta / (noMath::PI * fDelta);
		else
			m_pkCameraAttrib->m_fFramePitch = 0.0f;

		float fPitchMax = noMath::PI * 0.4f;

		// Compute an upward-looking pitch angle clamp.  Upward-looking clamp is
		// adjusted to be more restrictive as the camera zooms away.  This is 
		// adjusted to be <45deg, but quickly falls off to ensure that the camera
		// never goes below the ground height _at_the_character_ (may be 
		// different than the height at the camera).  We also check the camera
		// distance to be positive, so that the division in the asin doesn't give
		// NaN.  If the distance could cause asin to NaN then we return the fixed
		// 45 degree limit
		float fPitchMin = (m_pkCameraAttrib->m_fCamDist > 1e-5f) ? 
			-noMath::ASin(m_pkCameraAttrib->m_fCamHeight 
			/ (m_pkCameraAttrib->m_fCamHeight + m_pkCameraAttrib->m_fCamDist)) :
		-fPitchMax;

		// clamp the camera pitch angle
		// Downward-looking clamp is constant: 45deg
		if (m_pkCameraAttrib->m_fCamPitch < fPitchMin)
			m_pkCameraAttrib->m_fCamPitch = fPitchMin;
		else if (m_pkCameraAttrib->m_fCamPitch > fPitchMax)
			m_pkCameraAttrib->m_fCamPitch = fPitchMax;

		return true;
}

FirstPersonCameraMover::FirstPersonCameraMover( GameObject *pkGameObject )
	: GameObjectComponentBase(pkGameObject)
{
	m_pkCameraAttrib    = GetGameObjectAttribute(pkGameObject,FirstPersonCameraAttrib);
	assert(m_pkCameraAttrib);

	// Get height from camera
	BaseCamera* cam = (BaseCamera*)GetGameObject();

	noVec3    translation = cam->GetFrom(); //GetTranslation();
	m_pkCameraAttrib->m_fCamHeight = GetInitialHeight(translation); 

	//translation.y -= m_pkCameraAttrib->m_fCamHeight;
	//SetTranslation( translation );
	cam->setFrom(translation.ToFloatPtr());
}

bool FirstPersonCameraMover::Initialize()
{
	return true;
}

bool FirstPersonCameraMover::Update( float fDelta )
{
	BaseCamera* cam = (BaseCamera*)GetGameObject();
	noVec3	translation = cam->GetFrom();//GetTranslation();

	bool bSuccess = false;
	if (m_pkCameraAttrib->m_eMoveState != FirstPersonCameraAttrib::FLYING )
	{
		noVec3 kNewPosition;
		bool bSuccess = true; //TryPick(kNewPosition);

		if (bSuccess) // Success, the step is OK
		{
			// If Landing: Complete landing and start walking
			if (m_pkCameraAttrib->m_eMoveState == FirstPersonCameraAttrib::LANDING
				&& IsHeightInWalkingRange(MathHelpers::UP_COMPONENT(kNewPosition)))
			{
				m_pkCameraAttrib->m_eMoveState = FirstPersonCameraAttrib::WALKING;
			}

			translation.x = kNewPosition.x;
			translation.y = kNewPosition.y;
			MathHelpers::UP_COMPONENT(kNewPosition) += m_pkCameraAttrib->m_fCamHeight;
			// UP component is smoothly updated below:

			// This smooths the camera out when dropping levels...
			{
				float fDiff = MathHelpers::UP_COMPONENT(kNewPosition) - MathHelpers::UP_COMPONENT(translation);
				float fPercent = fDelta * 10.0f;
				fPercent = Min(1.0f, fPercent);

				MathHelpers::UP_COMPONENT(translation) += fDiff * fPercent;
			}
		}
	}

	// If we have not validated the location with walkables,
	// and we're not in a walking mode,
	// then fly
	//    if (!bSuccess && m_pkCameraAttrib->m_eMoveState != FirstPersonCameraAttrib::WALKING)
	{
		noVec3 kHeading = CalculateNewHeading(0, 1);
		translation += kHeading;
	}
		
	//SetTranslation( translation );
	cam->setFrom(translation.ToFloatPtr());

	UpdateCameraTransforms();
	return true;
}

float FirstPersonCameraMover::GetInitialHeight( const noVec3& kWorldTrans )
{	
	return  (MathHelpers::UP_COMPONENT(kWorldTrans));
}

#define NiLerp(T, START, FINISH) ((START) + ((T) * ((FINISH) - (START))))

bool FirstPersonCameraMover::TryPick( noVec3 &kNewPosition, const int iTry/*=0*/ )
{
	// Recursive function that attempts to return a valid position to move to.
	// 
	// pkPickController is used to form the picking samples against
	// the walkable polygons. 
	// 
	// If the function returns true,
	// kNewPosition is the best valid movement location.        
	//
	// The initial call to the function should specify 0 for iTry.
	//                                                          
	// If a pick fails, a series of additional picks are tried following a 
	// teardrop pattern. 
	// The samples alternate between the right and left side, 
	// and decreasing in distance. 
	// 
	// When walking into walls, or corners, this allows a sliding action.

	const int iTriesTotal = 30; // Max picks to attempt

	if (iTry >= iTriesTotal)
		return false;

	// fAngle & fMagnitude are used to modify how a new pick sample is found.
	// fAngle offsets the "forward" direction by that angle
	// fMagnitude shortens the magnitude of the heading vector
	float fAngle = 0;
	float fMagnitude = 1;
	{
		const float fTryNormalized = iTry / (float) iTriesTotal;
		const bool bOddTry = (iTry % 2) > 0;

		const float fMagnitudeStart = 1.0f;
		const float fMagnitudeFinish = 0.0f;
		fMagnitude = NiLerp(fTryNormalized, fMagnitudeStart, fMagnitudeFinish);

		const float fAngleStart = 0.0f;
		const float fAngleFinish = noMath::PI / 2;
		fAngle = NiLerp(fTryNormalized, fAngleStart, fAngleFinish);

		if (bOddTry)
			fAngle *= -1;
	}

	noVec3 kHeading = CalculateNewHeading(fAngle, fMagnitude);

	kNewPosition = GetTranslation() + kHeading;
	MathHelpers::UP_COMPONENT(kNewPosition) -= m_pkCameraAttrib->m_fCamHeight;

	noVec3 kPickPoint = 
	kNewPosition
		+ noVec3(0.0f, 0.0f, 
	Max(m_pkCameraAttrib->m_fCamHeight, m_pkCameraAttrib->m_fMaxCameraRaise));
		
	float fHeight;
	bool bSuccess;
	//	pkPickController->GetHeight(kPickPoint, fHeight);*/
	if (bSuccess)
	{
		// Is this a valid pick to transition to?
		if (m_pkCameraAttrib->m_eMoveState == FirstPersonCameraAttrib::LANDING 
			||
			(
			m_pkCameraAttrib->m_eMoveState == FirstPersonCameraAttrib::WALKING 
			&&
			IsHeightInWalkingRange(fHeight)
			)
			)
		{
			MathHelpers::UP_COMPONENT(kNewPosition) = fHeight;
			return true;
		}
	}

	// Failed, try again:
	return TryPick(kNewPosition, iTry + 1);
}

noVec3 FirstPersonCameraMover::CalculateNewHeading( const float fOffsetAngleRadians,const float fMagnitude )
{
	// Returns a vector (kHeading) that can be used to 
	// move the current camera position.
	//
	// The vector is modified by inputs:
	// fOffsetAngleRadians
	//   The direction considered forward is rotated by this angle
	// 
	// fMagnitude
	//   The magnitude of the resultant vector is multiplied by this value

	noVec3 kHeading = vec3_zero;
	noMat3 kR;
	// If walking, update heading to ignore pitch, only horizontal movement
	if (m_pkCameraAttrib->m_eMoveState == FirstPersonCameraAttrib::WALKING)
	{
		kR = MakeRotationMatrix(
			m_pkCameraAttrib->m_fHeading + fOffsetAngleRadians, 0);
	}
	else
	{
		kR = MakeRotationMatrix(
			m_pkCameraAttrib->m_fHeading + fOffsetAngleRadians, m_pkCameraAttrib->m_fCamPitch);
	}

	if (m_pkCameraAttrib->m_fStrafe)
	{
		noVec3 kStrafe = vec3_zero;
		kStrafe = kR[2];
		kStrafe *= m_pkCameraAttrib->m_fStrafe;
		kHeading = kStrafe;
	}

	if (m_pkCameraAttrib->m_fForward)
	{
		noVec3 kForward = vec3_zero;
		kForward = kR[0];
		kForward *= m_pkCameraAttrib->m_fForward;
		kHeading += kForward;
	}

	if (m_pkCameraAttrib->m_fVertical)
	{
		noVec3 kVertical = vec3_zero;
		kVertical = kR[1];
		kVertical *= m_pkCameraAttrib->m_fVertical;
		kHeading += kVertical;
	}

	kHeading *= fMagnitude;
	return kHeading;
}

void FirstPersonCameraMover::UpdateCameraTransforms()
{
	// The method used to position the camera is referred to as "camera on a
	// stick".  In other words, the camera is assumed to be attached to a 
	// long stick that comes out of the back of the character.  The length of
	// the stick and the angle of elevation of the stick as it comes out of
	// a fixed point on the back of the character may be adjusted.  As a
	// result, while the heading is implicit from the fact that the camera is
	// a child of the character's root node, we must add in the rotation of
	// the camera in pitch, and must also append that rotation to the "stick"
	// translation - thus, the stick rotation is applied to the camera's 
	// local transforms, and the rotation is also manually applied to the
	// stick's translation. 
	noMat3 kV = MakeRotationMatrix(m_pkCameraAttrib->m_fHeading, m_pkCameraAttrib->m_fCamPitch);
	//SetRotation(kV);	
	//if (m_pkCameraAttrib->m_fHeading != 0.0f) {
		BaseCamera* pCamera = (BaseCamera*)m_pkGameObject;	
		pCamera->computeModelView(kV);			
	//}
}

bool FirstPersonCameraMover::IsHeightInWalkingRange( const float fHeight )
{
	float fDiff = fHeight - MathHelpers::UP_COMPONENT(GetTranslation());

	bool bResult = fDiff >= -m_pkCameraAttrib->m_fMaxCameraDrop &&
		fDiff <= m_pkCameraAttrib->m_fMaxCameraRaise;

	return bResult;
}

FirstPersonCamera::FirstPersonCamera( const float* from, const float* to, const float* up, float fov )
	: ShadowMapCamera(from, to, up, fov)
{
	SetupCamera();
}

void FirstPersonCamera::SetRunSpeed( float fRunSpeed )
{
	m_pkCameraAttrib->m_fRunVelocity = fRunSpeed;

}

void FirstPersonCamera::SetWalkSpeed( float fWalkSpeed )
{
	m_pkCameraAttrib->m_fWalkVelocity = fWalkSpeed;

}

float FirstPersonCamera::GetRunSpeed()
{
	return m_pkCameraAttrib->m_fRunVelocity;

}

float FirstPersonCamera::GetWalkSpeed()
{
	return m_pkCameraAttrib->m_fWalkVelocity;

}

void FirstPersonCamera::SetupCamera()
{
	m_pkCameraAttrib    = new FirstPersonCameraAttrib;
	AttachAttribute(m_pkCameraAttrib);

	// we have to do these in the constructor for this class...
	//AttachComponent( new FirstPersonGamePadInput(this,0) );
	AttachComponent( new FirstPersonMouseAndKeyboardInput(this) );
	AttachComponent( new FirstPersonCameraMover(this) );

	m_pkCameraAttrib->m_eMoveState = FirstPersonCameraAttrib::FLYING;

	//noMat3 kCameraRot;
	//kCameraRot = GetRotation();

	//We want to preserve the "look at vector" 
	//Gamebryo camera looks down the positive X axis, 
	//so the 0th col is the lookat
	noVec3 kLookAtVector;
	kLookAtVector = GetDir();//kCameraRot[0];
	noVec3 kHeadingVector = noVec3(kLookAtVector[0], kLookAtVector[1], 0);
	kHeadingVector.Normalize();

	//The heading vector is the 'look at' vector projected into the x-y plane
	//so simple trig can get the heading
	m_pkCameraAttrib->m_fHeading = atan2(-kHeadingVector[0],-kHeadingVector[1]);

	//the pitch is the angle between the look at vector, and the x-y plane
	//we can find this through the dot product, and the z component of the 
	//'look at' vector for the sign
	m_pkCameraAttrib->m_fCamPitch = acos(kHeadingVector * kLookAtVector)
		* (kLookAtVector[2] >= 0 ? -1 : 1);

	//    SetTranslation( m_pkCameraAttrib->m_kTrans );

	// Set aspect ratio of cameras.
	{
		/*Ni2DBuffer* pkDefaultBackBuffer =
			NiRenderer::GetRenderer()->GetDefaultBackBuffer();
		NIASSERT(pkDefaultBackBuffer);
		float fAspectRatio = (float) pkDefaultBackBuffer->GetWidth() /
			pkDefaultBackBuffer->GetHeight();

		GetNiCamera()->AdjustAspectRatio(fAspectRatio);*/
	}
}

void FirstPersonCamera::UpdateCamera(float fDelta)
{
	// Clear out the variables used by our components....
	m_pkCameraAttrib->m_fStrafe = 0.0f;        
	m_pkCameraAttrib->m_fForward = 0.0f;          
	m_pkCameraAttrib->m_fVertical = 0.0f;          
}