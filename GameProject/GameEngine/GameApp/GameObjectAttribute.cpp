#include "stdafx.h"
#include "AnimationComponent.h"
#include "AnimationStateComponent.h"
#include "GameObjectAttribute.h"
#include "GameObjectUtil.h"

noImplementRootRTTI(IGameObjectAttribute);
noImplementRTTI(ICharacterMovementAttribute,IGameObjectAttribute);


noImplementRTTI(GameObjectRotatorAttribute,IGameObjectAttribute);
GameObjectRotatorAttribute::GameObjectRotatorAttribute()
{
    m_yawRate	= 0;
    m_pitchRate	= 0;
    m_rollRate  = 0;	
    m_desiredYaw = noMath::INFINITY;
    m_desiredPitch = noMath::INFINITY;
    m_desiredRoll = noMath::INFINITY;

    m_yawRotateDir		= 0;
    m_pitchRotateDir	= 0;
    m_rollRotateDir		= 0;

    m_fCurrYaw			= 0;
    m_fCurrPitch		= 0;
    m_fCurrRoll			= 0;

    m_fMaxYaw			= m_fMinYaw = 0;
    m_fMaxPitch			= m_fMinPitch = 0;
    m_fMaxRoll			= m_fMinRoll = 0;
}

void GameObjectRotatorAttribute::SetDesiredYaw(float yaw)
{
    if( yaw==m_desiredYaw )
        return;

    m_desiredYaw = yaw;
}

void GameObjectRotatorAttribute::UpdateDesiredYaw(float fDelta)
{
    if( m_fCurrYaw==m_desiredYaw || m_desiredYaw==noMath::INFINITY )
        return;

    if( m_yawRate==0 )
    {
        SetYaw( m_desiredYaw );
        return;
    }

    float   change = m_yawRate * fDelta;

    if( m_desiredYaw < m_fCurrYaw )
        change  = -change;

    SetYaw( GetYaw()+change );
    if( change < 0 && GetYaw() < m_desiredYaw )
        SetYaw(m_desiredYaw);
    else if ( change > 0 && GetYaw() > m_desiredYaw )
        SetYaw(m_desiredYaw);
}

void GameObjectRotatorAttribute::UpdateDesiredPitch(float fDelta)
{
    if( m_fCurrPitch==m_desiredPitch || m_desiredPitch==noMath::INFINITY )
        return;

    if( m_pitchRate==0 )
    {
        SetPitch( m_desiredPitch );
        return;
    }

    float   change = m_pitchRate * fDelta;

    if( m_desiredPitch < m_fCurrPitch )
        change  = -change;

    SetPitch( GetPitch()+change );
    if( change < 0 && GetPitch() < m_desiredPitch )
        SetPitch(m_desiredPitch);
    else if ( change > 0 && GetPitch() > m_desiredPitch )
        SetPitch(m_desiredPitch);
}

void GameObjectRotatorAttribute::UpdateDesiredRoll(float fDelta)
{
    if( m_fCurrRoll==m_desiredRoll || m_desiredRoll==noMath::INFINITY )
        return;

    if( m_rollRate==0 )
    {
        SetRoll( m_desiredRoll );
        return;
    }

    float   change = m_rollRate * fDelta;

    if( m_desiredRoll < m_fCurrRoll )
        change  = -change;

    SetRoll( GetRoll()+change );
    if( change < 0 && GetRoll() < m_desiredRoll )
        SetRoll(m_desiredRoll);
    else if ( change > 0 && GetRoll() > m_desiredRoll )
        SetRoll(m_desiredRoll);
}


void GameObjectRotatorAttribute::UpdateYaw(float desiredYawPct, float fDelta)
{
    float   desiredYaw = m_fCurrYaw;

    if( desiredYawPct>0 )
    {
        desiredYaw = m_fMaxYaw * desiredYawPct;
    }
    else
    {
        desiredYaw = m_fMinYaw * desiredYawPct * -1.0f;
    }

    float   change = 0;

    change = m_yawRate * fDelta;

    if( desiredYaw < m_fCurrYaw )
        change = -change;

    SetYaw(GetYaw()+change);
    if( change < 0 && GetYaw() < desiredYaw )
        SetYaw(desiredYaw);
    else if ( change > 0 && GetYaw() > desiredYaw )
        SetYaw(desiredYaw);
}

void GameObjectRotatorAttribute::UpdatePitch(float desiredPitchPct, float fDelta)
{
    float   desiredPitch = m_fCurrPitch;

    if( desiredPitchPct>0 )
    {
        desiredPitch = m_fMaxPitch * desiredPitchPct;
    }
    else
    {
        desiredPitch = m_fMinPitch * desiredPitchPct * -1.0f;
    }

    float   change = 0;

    change = m_pitchRate * fDelta;

    if( desiredPitch < m_fCurrPitch )
        change = -change;

    SetPitch(GetPitch()+change);

    if( change < 0 && GetPitch() < desiredPitch )
        SetPitch(desiredPitch);
    else if ( change > 0 && GetPitch() > desiredPitch )
        SetPitch(desiredPitch);
}

void GameObjectRotatorAttribute::UpdateRoll(float desiredRollPct, float fDelta)
{
    float   desiredRoll = m_fCurrRoll;

    if( desiredRollPct>0 )
    {
        desiredRoll = m_fMaxRoll * desiredRollPct;
    }
    else
    {
        desiredRoll = m_fMinRoll * desiredRollPct * -1.0f;
    }

    float   change = 0;

    change = m_rollRate * fDelta;

    if( desiredRoll < m_fCurrRoll )
        change = -change;

    SetRoll(GetRoll()+change);

    if( change < 0 && GetRoll() < desiredRoll )
        SetRoll(desiredRoll);
    else if ( change > 0 && GetRoll() > desiredRoll )
        SetRoll(desiredRoll);
}

float GameObjectRotatorAttribute::GetPitchPct()					
{ 
    float pct = m_fCurrPitch / m_fMaxPitch;

    if ( m_fCurrPitch<0 )
    {
        pct = -(m_fCurrPitch / m_fMinPitch);
    }

    pct *=-1;

    return pct;
}

void GameObjectRotatorAttribute::SetYaw(float newYaw)
{
    if( m_fMinYaw==m_fMaxYaw )
    {
        m_fCurrYaw = newYaw;
    }
    else
    {
        if( newYaw < m_fMinYaw )
        {
            newYaw = m_fMinYaw;
        }
        else if ( newYaw > m_fMaxYaw )
        {
            newYaw = m_fMaxYaw;
        }

        m_fCurrYaw = newYaw;
    }

    if( m_fCurrYaw==m_desiredYaw )
        m_desiredYaw = noMath::INFINITY;
}

void GameObjectRotatorAttribute::SetPitch(float newPitch)
{
    if( m_fMinPitch==m_fMaxPitch )
    {
        m_fCurrPitch = newPitch;
    }
    else
    {
        if( newPitch < m_fMinPitch )
        {
            newPitch = m_fMinPitch;
        }
        else if ( newPitch > m_fMaxPitch )
        {
            newPitch = m_fMaxPitch;
        }
        m_fCurrPitch = newPitch;
    }

    if( m_desiredPitch==m_fCurrPitch )
    {
        m_desiredPitch = noMath::INFINITY;
    }
}

void GameObjectRotatorAttribute::SetRoll(float newRoll)
{
    if( m_fMinRoll==m_fMaxRoll )
    {
        m_fCurrRoll = newRoll;
        return;
    }
    else
    {
        if( newRoll < m_fMinRoll )
        {
            newRoll = m_fMinRoll;
        }
        else if ( newRoll > m_fMaxRoll )
        {
            newRoll = m_fMaxRoll;
        }

        m_fCurrRoll = newRoll;
    }

    if( m_desiredRoll==m_fCurrRoll )
        m_desiredRoll   = noMath::INFINITY;
}

noMat3 GameObjectRotatorAttribute::GetRotation()
{
	
    noMat3   rotateYaw(mat3_default);
    noMat3   rotatePitch(mat3_default);
    noMat3   rotateRoll(mat3_default);

    if( MathHelpers::IsZUp() )
    {
		noRotation roll, pitch, yaw;
		roll.Set(vec3_zero, noVec3(0.f, 1.f, 0.f), RAD2DEG(m_fCurrRoll));
		rotateRoll = roll.ToMat3();        
        pitch.Set(vec3_zero, noVec3(1.0f, 0.0f, 0.f), RAD2DEG(m_fCurrPitch));
		rotatePitch = pitch.ToMat3();
		yaw.Set(vec3_zero, noVec3(0.0f, 0.0f, 1.0f), RAD2DEG(m_fCurrYaw));		
		rotateYaw = yaw.ToMat3();        
    }
    else
    {        
		noRotation roll, pitch, yaw;
		roll.Set(vec3_zero, noVec3(0.f, 0.f, 1.f), RAD2DEG(m_fCurrRoll));
		rotateRoll = roll.ToMat3();        
		pitch.Set(vec3_zero, noVec3(1.0f, 0.0f, 0.f), RAD2DEG(m_fCurrPitch));
		rotatePitch = pitch.ToMat3();
		yaw.Set(vec3_zero, noVec3(0.0f, 1.0f, 0.0f), RAD2DEG(m_fCurrYaw));		
		rotateYaw = yaw.ToMat3();             
    }


    return (rotateRoll * rotatePitch * rotateYaw);
}

noMat3 GameObjectRotatorAttribute::GetCharacterRotation()
{
    noMat3   rotation;
	noRotation rot(vec3_zero, UP_VECTOR, m_fCurrYaw);
    rotation = rot.ToMat3();
    
    return rotation;
}


noMat3 GameObjectRotatorAttribute::GetCameraRotation()
{
    if( MathHelpers::IsZUp() )
    {
        noMat3   kYaw,kPitch,kRoll,kV;
        noMat3   kRotX, kRotZ;

        /*kRoll.MakeYRotation(m_fCurrRoll);
        kYaw.MakeZRotation(m_fCurrYaw);
        kPitch.MakeXRotation(-m_fCurrPitch);

        kRotX.MakeXRotation(-NI_HALF_PI);
        kRotZ.MakeZRotation(NI_HALF_PI);*/

        kV = kRotZ * kRotX;

        kV = kRoll * kYaw * kPitch * kV;

        return kV;
    }

    assert( MathHelpers::IsYUp() );

    noMat3   kYaw,kPitch,kRoll,kV;
    noMat3   kRotX, kRotZ;

	noRotation rollR, yawR, pitchR;
	noRotation xR, zR;
    rollR.Set(vec3_zero, noVec3(0,0,1), m_fCurrRoll);
	kRoll = rollR.ToMat3(); 
	yawR.Set(vec3_zero, noVec3(0, 1, 0), m_fCurrYaw);
	kYaw = yawR.ToMat3();
    pitchR.Set(vec3_zero, noVec3(1, 0, 0), -m_fCurrPitch - 1.5f);
	kPitch = pitchR.ToMat3();
	 
	xR.Set(vec3_zero, noVec3(1, 0, 0), noMath::HALF_PI);
	kRotX = xR.ToMat3();
	zR.Set(vec3_zero, noVec3(0, 0, 1), -noMath::HALF_PI);
	kRotZ = zR.ToMat3();

    kV = kRotZ * kRotX;
    kV = kRoll * kYaw * kPitch * kV;

    return kV;
}

void GameObjectRotatorAttribute::FaceDir2D(const noVec3 &dir)
{
    /*
    noVec3    temp = dir;
    MathHelpers::UP_COMPONENT(temp) = 0;
    temp.Normalize();
    float dot       = FORWARD_VECTOR * temp;
    float angle     = acosf(dot);

    if ( dir * RIGHT_VECTOR > 0 )
        angle *=-1;
        */

    noVec3    currentForward = MathHelpers::GetForwardVectorFromMatrix(GetRotation());

    float angle = MathHelpers::GetAngleBetweenActorDirections(dir, currentForward);

    angle = DEG2RAD(angle);

    float desiredYaw = m_fCurrYaw + angle;

    SetDesiredYaw(desiredYaw);
}

void GameObjectRotatorAttribute::FaceDir3D(const noVec3 &dir)
{
    FaceDir2D(dir);

    noVec3    temp = FORWARD_VECTOR;
    MathHelpers::UP_COMPONENT(temp)         = MathHelpers::UP_COMPONENT(dir);

    temp.Normalize();

    float dot = FORWARD_VECTOR * temp;
    float angle = acosf(dot);

    if( dir * UP_VECTOR < 0 )
        angle *= -1.0f;

    SetDesiredPitch( angle );

    /*
    float upAngle = MathHelpers::UP_COMPONENT(dir) * (MathHelpers::DegToRad(90.0f));
    SetDesiredPitch( upAngle );
    */
}

void GameObjectRotatorAttribute::ResetYaw(const noMat3 &rotation)
{
    float yaw = MathHelpers::GetCurrentYaw(rotation);
    SetYaw( yaw );
}


void GameObjectRotatorAttribute::UpdateYawRotate(float fDelta)
{
    float   yaw = GetYaw();

    yaw += (fDelta * m_yawRate * m_yawRotateDir);

    m_desiredYaw = yaw;
    SetYaw( m_desiredYaw );
}

void GameObjectRotatorAttribute::UpdatePitchRotate(float fDelta)
{
    float   pitch = GetPitch();

    pitch += (fDelta * m_pitchRate * m_pitchRotateDir );

    m_desiredPitch = pitch;
    SetPitch( m_desiredPitch );
}

void GameObjectRotatorAttribute::UpdateRollRotate(float fDelta)
{
    float   roll = GetRoll();

    roll += (fDelta * m_rollRate * m_rollRotateDir );

    m_desiredRoll = roll;
    SetRoll( m_desiredRoll );
}

void GameObjectRotatorAttribute::AddYaw(float delta)
{
    SetYaw( GetYaw() + delta );
}

void GameObjectRotatorAttribute::AddPitch(float delta)
{
    SetPitch( GetPitch() + delta );
}

void GameObjectRotatorAttribute::AddRoll(float delta)
{
    SetRoll( GetRoll() + delta );
}

bool GameObjectRotatorAttribute::HasDesiredPitch() const
{
    if( m_desiredPitch==noMath::INFINITY )
        return false;

    return true;
}

bool GameObjectRotatorAttribute::HasDesiredYaw() const
{
    if( m_desiredYaw==noMath::INFINITY )
        return false;

    return true;
}

bool GameObjectRotatorAttribute::HasDesiredRoll() const
{
    if( m_desiredRoll==noMath::INFINITY )
        return false;

    return true;
}

void GameObjectRotatorAttribute::ClearDesiredYaw()
{
    m_desiredYaw = noMath::INFINITY;
}

void GameObjectRotatorAttribute::ClearDesiredPitch()
{
    m_desiredPitch = noMath::INFINITY;
}

void GameObjectRotatorAttribute::ClearDesiredRoll()
{
    m_desiredRoll = noMath::INFINITY;
}

// -------------------------------------------------------------
// -------------------------------------------------------------
noImplementRTTI(AnimStateAttributeBase,IGameObjectAttribute);
AnimStateAttributeBase::AnimStateAttributeBase()
{
    m_pkAnimState               = NULL;
    m_pkAnimStateOverride       = NULL;
}

void AnimStateAttributeBase::SetAnimState(IAnimState *pkState)
{
    if (m_pkAnimState!=pkState )
    {
        m_pkAnimState = pkState;
    }
}

IAnimState *AnimStateAttributeBase::GetAnimState()
{
    if( m_pkAnimStateOverride )
        return m_pkAnimStateOverride;

    return m_pkAnimState;
}

void AnimStateAttributeBase::SetOverrideAnimState( IAnimState *pkAnimState )
{
    m_pkAnimStateOverride   = pkAnimState;
}



// -------------------------------------------------------------
// -------------------------------------------------------------
noImplementRTTI(BasicCharacterAnimStateAttribute,AnimStateAttributeBase);
BasicCharacterAnimStateAttribute::BasicCharacterAnimStateAttribute()
{
    m_pkIdle            = NULL;
    m_pkWalk            = NULL;
    m_pkWalkBack        = NULL;
    m_pkWalkLeft        = NULL;
    m_pkWalkRight       = NULL;

    m_pkRun             = NULL;
    m_pkRunLeft         = NULL;
    m_pkRunRight        = NULL;
    m_pkRunBack         = NULL;

    m_pkPunch           = NULL;
    m_pkKick            = NULL;

    m_pkPain            = NULL;
    m_pkDeath           = NULL;

    m_pkTurnLeft        = NULL;
    m_pkTurnRight       = NULL;
}

void BasicCharacterAnimStateAttribute::Idle()
{
    assert(m_pkIdle);
     SetAnimState(m_pkIdle);
}

void BasicCharacterAnimStateAttribute::Walk()
{
    assert(m_pkWalk);
    SetAnimState(m_pkWalk);
}

void BasicCharacterAnimStateAttribute::Run()
{
    assert(m_pkRun);
    SetAnimState(m_pkRun);
}

void BasicCharacterAnimStateAttribute::Punch()
{
    if( !m_pkPunch )
        return;

    SetAnimState(m_pkPunch);
}

void BasicCharacterAnimStateAttribute::Kick()
{
    if( !m_pkKick )
        return;

    SetAnimState(m_pkKick);
}

void BasicCharacterAnimStateAttribute::Pain()
{
    assert(m_pkPain);
    SetAnimState(m_pkPain);
}

void BasicCharacterAnimStateAttribute::Death()
{
    assert(m_pkDeath);
    SetAnimState(m_pkDeath);
}

void BasicCharacterAnimStateAttribute::WalkBack()
{
    if( !m_pkWalkBack )
    {
        Walk();
        return;
    }

    SetAnimState(m_pkWalkBack);
}

void BasicCharacterAnimStateAttribute::WalkLeft()
{
    if( !m_pkWalkLeft )
    {
        Walk();
        return;
    }

    SetAnimState( m_pkWalkLeft );
}

void BasicCharacterAnimStateAttribute::WalkRight()
{
    if( !m_pkWalkRight )
    {
        Walk();
        return;
    }

    SetAnimState( m_pkWalkRight );
}

void BasicCharacterAnimStateAttribute::RunLeft()
{
    if( !m_pkRunLeft )
    {
        Run();
        return;
    }

    SetAnimState( m_pkRunLeft );
}

void BasicCharacterAnimStateAttribute::RunRight()
{
    if( !m_pkRunRight )
    {
        Run();
        return;
    }

    SetAnimState( m_pkRunRight );
}

void BasicCharacterAnimStateAttribute::RunBack()
{
    if( !m_pkRunBack )
    {
        WalkBack();
        return;
    }

    SetAnimState(m_pkRunBack);
}


void BasicCharacterAnimStateAttribute::TurnLeft()
{
    if( !m_pkTurnLeft )
    {
        Idle();
        return;
    }

    SetAnimState( m_pkTurnLeft );
}

void BasicCharacterAnimStateAttribute::TurnRight()
{
    if( !m_pkTurnLeft )
    {
        Idle();
        return;
    }

    SetAnimState( m_pkTurnRight );
}


noImplementRTTI(BasicCharacterMovementAttribute,ICharacterMovementAttribute);
BasicCharacterMovementAttribute::BasicCharacterMovementAttribute( float walkSpeed,float runSpeed )
{
	m_walkSpeed     = walkSpeed;
	m_runSpeed      = runSpeed;
	m_moveDir       = vec3_zero;
	m_desiredSpeed  = 0;
}

void BasicCharacterMovementAttribute::Walk()
{
	m_desiredSpeed = m_runSpeed;

}

void BasicCharacterMovementAttribute::Run()
{
	m_desiredSpeed = m_walkSpeed;

}

void BasicCharacterMovementAttribute::Stop()
{
	m_moveDir       = vec3_zero;
	m_desiredSpeed  = 0;
}
