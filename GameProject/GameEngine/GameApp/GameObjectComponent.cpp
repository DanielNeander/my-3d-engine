#include "stdafx.h"
#include "GameObjectComponent.h"

#include "ModelLib/M2Loader.h"
#include "ModelLib/M2Particle.h"
#include "SceneNode.h"
#include "Mesh.h"
#include "M2Mesh.h"
#include "TestObjects.h"
#include "GameObjectAttribute.h"
#include "GameObjectUtil.h"
#include "GameObjectManager.h"
#include "TestObjects.h"
#include "GameApp.h"

noImplementRootRTTI(IGameObjectComponent);
noImplementRTTI(ITransformComponent,IGameObjectComponent);
noImplementRTTI(DefaultTransformComponent,ITransformComponent);
noImplementRTTI(NodeTransformComponent,ITransformComponent);
noImplementRTTI(ActorTransformComponent,NodeTransformComponent);



NodeTransformComponent::NodeTransformComponent( GameObject *pkGameObject )
	:m_offset(vec3_zero),
	m_pkGameObject(pkGameObject)
{

}

bool NodeTransformComponent::Update( float fDelta )
{
	return true;
}

bool NodeTransformComponent::PostUpdate( float fDelta )
{
	GetNode()->Update( m_pkGameObject->GetUpdateTime() );
	return true;
}

void NodeTransformComponent::SetTranslation( const noVec3 &position )
{
	GetNode()->SetTrans( position + m_offset );
}

void NodeTransformComponent::SetRotation( const noMat3 &rotation )
{
	GetNode()->SetRotate(rotation);
}

void NodeTransformComponent::SetHeight( float height )
{
	noVec3	pos = GetTranslation();
	pos.y = height;
	SetTranslation(pos);
}

void NodeTransformComponent::SetScale( float scale )
{
	GetNode()->SetScale( scale );
}

float NodeTransformComponent::GetScale()
{
	return GetNode()->GetScale();
}

noVec3 NodeTransformComponent::GetTranslation()
{
	return (GetNode()->GetTrans() - m_offset);

}

noMat3 NodeTransformComponent::GetRotation()
{
	return GetNode()->GetRotate();
}

SceneNode * NodeTransformComponent::GetNode()
{
	return m_pkGameObject->GetNode();

}

ActorTransformComponent::ActorTransformComponent( GameObject *pkGameObject )
	:NodeTransformComponent( pkGameObject)
{
	m_pkGameObject      = pkGameObject;

}

bool ActorTransformComponent::Update( float fDelta )
{
	return true;
}

bool ActorTransformComponent::PostUpdate( float fDelta )
{
//	WowActor* pActor = (WowActor*)m_pkGameObject;		
//	pActor->mesh_->Update(fDelta);

	return true;
}

noImplementRTTI(GameObjectComponentBase,IGameObjectComponent);
GameObjectComponentBase::GameObjectComponentBase( class GameObject *pkGameObject )
	: m_pkGameObject(pkGameObject)
	, m_bEnabled(true)
{

}

bool GameObjectComponentBase::Update( float fDelta )
{
	FSMObject::TickState();
	return true;
}

bool GameObjectComponentBase::PostUpdate( float fDelta )
{
	return false;
}

bool GameObjectComponentBase::ProcessInput()
{
	return FSMObject::TickInput();
}

noVec3 GameObjectComponentBase::GetTranslation()
{
	return m_pkGameObject->GetTranslation();

}

noMat3 GameObjectComponentBase::GetRotation()
{
	return m_pkGameObject->GetRotation();

}

noVec3 GameObjectComponentBase::GetVelocity()
{
	return m_pkGameObject->GetVelocity();
}

float GameObjectComponentBase::GetScale()
{
	return m_pkGameObject->GetScale();

}

void GameObjectComponentBase::SetTranslation( const noVec3 &translation )
{
	return m_pkGameObject->SetTranslation(translation);

}

void GameObjectComponentBase::SetRotation( const noMat3 &rotation )
{
	m_pkGameObject->SetRotation( rotation );

}

void GameObjectComponentBase::SetVelocity( const noVec3 &velocity )
{
	m_pkGameObject->SetVelocity(velocity);
}

void GameObjectComponentBase::SetScale( float fScale )
{
	m_pkGameObject->SetScale( fScale );

}

void GameObjectComponentBase::SetHeight( float height )
{
	m_pkGameObject->SetHeight(height);

}

noImplementRTTI(CharacterMoverComponent,GameObjectComponentBase);
CharacterMoverComponent::CharacterMoverComponent( GameObject *pkGameObject ) : GameObjectComponentBase(pkGameObject)
	, m_bMoveDest(false)
	, m_destination(vec3_zero)
{

}

bool CharacterMoverComponent::Initialize()
{
	m_pkMovementAttrib      = GetGameObjectAttribute(GetGameObject(),BasicCharacterMovementAttribute);
	assert(m_pkMovementAttrib);

	if( !m_pkMovementAttrib )
		return false;

	return true;
}

bool CharacterMoverComponent::Update( float fDelta )
{
	m_velocity = vec3_zero;

	if( m_bMoveDest )
		UpdateMoveTo(fDelta);
	else if( m_pkMovementAttrib->GetMoveDir()!=vec3_zero ) 
		UpdateMoveInDirection(fDelta);

	return true;
}

void CharacterMoverComponent::MoveTo( const noVec3 &destination )
{
	m_destination           = destination;
	m_bMoveDest             = true;
}

void CharacterMoverComponent::Stop()
{
	m_pkMovementAttrib->Stop();
	m_bMoveDest = false;
}

void CharacterMoverComponent::UpdateMoveTo( float fDelta )
{
	noVec3 currPos = GetTranslation();

	noVec3 dir = m_destination - currPos;

	MathHelpers::UP_COMPONENT(dir) = 0;

	dir.Normalize();

	m_velocity = (dir * m_pkMovementAttrib->GetDesiredSpeed());

	currPos += m_velocity * fDelta;

	SetTranslation(currPos);

	noVec3 newDir = m_destination - currPos;

	MathHelpers::UP_COMPONENT(newDir) = 0;

	newDir.Normalize();

	if( newDir * dir < 0 )
	{
		// we're done
		m_bMoveDest = false;
	}
}

void CharacterMoverComponent::UpdateMoveInDirection( float fDelta )
{
	noVec3 pos = GetTranslation();

	m_velocity = m_pkMovementAttrib->GetMoveDir() * m_pkMovementAttrib->GetDesiredSpeed();

	pos += m_pkMovementAttrib->GetMoveDelta(fDelta);

	SetTranslation(pos);
}

noImplementRTTI(FloorMoverComponent,GameObjectComponentBase);
FloorMoverComponent::FloorMoverComponent( GameObject *pkGameObject ) : GameObjectComponentBase( pkGameObject )
{
	m_ascendRate = 5.0f;
	m_descendRate = 5.0f;
	m_stepHeight = 0.5f;
	m_footOffset = 0;

	m_prevPosition = noVec3(1.f, 1.f, 1.f);
}

void FloorMoverComponent::Setup( float descendRate, float ascendRate, float stepHeight, float footOffset )
{
	m_descendRate = descendRate;
	m_ascendRate = ascendRate;
	m_stepHeight = stepHeight;
	m_footOffset = footOffset;
}

bool FloorMoverComponent::PostUpdate( float fDelta )
{
	return CheckForFloor(fDelta);
}

bool FloorMoverComponent::CheckForFloor( float fDelta )
{
	GameObjectManager   *pkGameObjectMan = GameObjectManager::Get();
	assert(pkGameObjectMan);

	float       height = 0;

	noVec3    currPos = GetTranslation();
	noVec3    point   = currPos;
	noVec3    checkPoint = currPos + (UP_VECTOR * m_stepHeight);

	if( !GetHeight(checkPoint,height) )
	{
		if (m_prevPosition!= noVec3(1.f, 1.f, 1.f))
		{
			GetGameObject()->ResetTranslation( m_prevPosition );
		}
		return true;
	}

	MathHelpers::UP_COMPONENT(point) = height + m_footOffset;
	MathHelpers::UP_COMPONENT(point) += ONE_METER * 0.01f;

	if( MathHelpers::UP_COMPONENT(currPos) > MathHelpers::UP_COMPONENT(point) )
	{
		MathHelpers::UP_COMPONENT(currPos) -= m_descendRate * fDelta;
		if( MathHelpers::UP_COMPONENT(currPos)<MathHelpers::UP_COMPONENT(point) )
			MathHelpers::UP_COMPONENT(currPos) = MathHelpers::UP_COMPONENT(point);
	}

	if( MathHelpers::UP_COMPONENT(currPos) < MathHelpers::UP_COMPONENT(point) )
	{
		MathHelpers::UP_COMPONENT(currPos) += m_ascendRate * fDelta;
		if( MathHelpers::UP_COMPONENT(currPos)>MathHelpers::UP_COMPONENT(point) )
			MathHelpers::UP_COMPONENT(currPos) = MathHelpers::UP_COMPONENT(point);
	}

	m_prevPosition  = currPos;

	SetHeight( MathHelpers::UP_COMPONENT(currPos) );

	return true;
}

void FloorMoverComponent::OnTeleported()
{
	m_prevPosition  = GetTranslation();

}

bool FloorMoverComponent::GetHeight( const noVec3 &pos, float &height )
{

	//// if there are walkables in the world, use them.  If not, then 
	//// just use the collision manager....

	//PickController *pkPickController = GameObjectManager::Get()->GetPickController();
	//if( pkPickController && pkPickController->HasWalkables() )
	//{
	//	return pkPickController->GetHeight(pos,height);
	//}

	//CollisionManager	*pkCollisionMan = CollisionManager::Get();
	//if( !pkCollisionMan )
	//	return false;

	//return pkCollisionMan->GetHeight(pos,height);
	return true;
}

noImplementRTTI(GameObjectRemoverComponent,GameObjectComponentBase);
GameObjectRemoverComponent::GameObjectRemoverComponent( GameObject *pkGameObject )
	:GameObjectComponentBase( pkGameObject )
	,m_removeTime(0)
{

}

bool GameObjectRemoverComponent::Update( float fDelta )
{
	if( GetTime() > m_removeTime && m_removeTime>0 )
	{
		GameObjectManager::Get()->DestroyGameObject( GetGameObject() );
		return true;
	}
	return true;
}


noImplementRTTI(CharacterRotatorComponent,GameObjectComponentBase);
CharacterRotatorComponent::CharacterRotatorComponent( GameObject *pkGameObject )
	: GameObjectComponentBase(pkGameObject)
{
	m_fTotalRotate      = 0;
	m_fCurrRotate       = 0;
	m_bHasDesiredYaw    = false;

	m_pkRotatorAttrib           = GetGameObjectAttribute(GetGameObject(),GameObjectRotatorAttribute);

	if( m_pkRotatorAttrib )
	{
		noMat3   rotate  = GetRotation();
		m_destinationYaw    = MathHelpers::GetCurrentYaw(rotate);

		m_pkRotatorAttrib->SetYaw( m_destinationYaw );
		m_pkRotatorAttrib->SetDesiredYaw( m_destinationYaw );
	}

	m_pkAnimStateAttrib         = GetGameObjectAttribute(GetGameObject(),AnimStateAttributeBase);
}

bool CharacterRotatorComponent::Initialize()
{
	m_pkRotatorAttrib           = GetGameObjectAttribute(GetGameObject(),GameObjectRotatorAttribute);

	if( !m_pkRotatorAttrib )
		return false;

	noMat3   rotate  = GetRotation();
	m_destinationYaw    = MathHelpers::GetCurrentYaw(rotate);

	m_pkRotatorAttrib->SetYaw( m_destinationYaw );
	m_pkRotatorAttrib->SetDesiredYaw( m_destinationYaw );

	return true;
}

bool CharacterRotatorComponent::Update( float fDelta )
{
	
	float rate          = m_pkRotatorAttrib->m_yawRate;
	float prevYaw       = m_pkRotatorAttrib->GetYaw();

	if (m_pkRotatorAttrib->m_yawRotateDir!=0)
	{
		IAnimState                      *pkAnimState = NULL;

		if( m_pkAnimStateAttrib )
			pkAnimState         = m_pkAnimStateAttrib->GetAnimState();

		if( pkAnimState && pkAnimState->HasRotationAccumulation() )
		{
			m_pkRotatorAttrib->ResetYaw( GetRotation() );
		}
		else
		{
			m_pkRotatorAttrib->UpdateYawRotate( fDelta );
		}
	}
	else 
	{
		SetDesiredYaw( m_pkRotatorAttrib->GetDesiredYaw() );

		if( m_bHasDesiredYaw )
		{
			if ( m_fTotalRotate >= 0.0f )
			{
				m_fCurrRotate	+= fDelta * rate;
				m_pkRotatorAttrib->AddYaw( fDelta * rate );

				if (m_fCurrRotate>=m_fTotalRotate)
				{
					m_fCurrRotate	= m_fTotalRotate;
					m_fTotalRotate	= 0.0f;
					m_pkRotatorAttrib->SetYaw( m_pkRotatorAttrib->GetDesiredYaw() );
					m_bHasDesiredYaw = false;
				}
			}
			else
			{
				m_fCurrRotate	-= fDelta * rate;
				m_pkRotatorAttrib->AddYaw( -(fDelta * rate) );

				if (m_fCurrRotate<=m_fTotalRotate)
				{
					m_fCurrRotate	 = m_fTotalRotate;
					m_fTotalRotate	 = 0.0f;
					m_pkRotatorAttrib->SetYaw( m_pkRotatorAttrib->GetDesiredYaw() );
					m_bHasDesiredYaw = false;
				}
			}
		}
	}

	if( prevYaw==m_pkRotatorAttrib->GetYaw() )
		return true;

	SetObjectRotation();
	return true;
}

void CharacterRotatorComponent::SetEnabled( bool enabled )
{
	if( !IsEnabled() && enabled )
	{
		noMat3   rotate  = GetRotation();
		m_pkRotatorAttrib->SetYaw( MathHelpers::GetCurrentYaw(rotate) );
		m_fTotalRotate      = 0;
		m_fCurrRotate       = 0;
		m_bHasDesiredYaw    = false;
		m_pkRotatorAttrib->RotateYaw( 0 );
		m_pkRotatorAttrib->SetDesiredYaw( m_pkRotatorAttrib->GetYaw() );
	}

	GameObjectComponentBase::SetEnabled( enabled );
}

void CharacterRotatorComponent::SetObjectRotation()
{
	SetRotation(m_pkRotatorAttrib->GetCharacterRotation());

}

void CharacterRotatorComponent::SetDesiredYaw( float desiredYaw )
{
	if( desiredYaw== noMath::INFINITY )
	{
		m_bHasDesiredYaw = false;
		return;
	}

	if( desiredYaw==m_pkRotatorAttrib->GetYaw() )
	{
		m_destinationYaw    = m_pkRotatorAttrib->GetYaw();
		m_bHasDesiredYaw    = false;
		return;
	}

	if( desiredYaw!=m_destinationYaw )
	{
		m_destinationYaw        = desiredYaw;
		m_bHasDesiredYaw        = true;
	}
	else
	{
		return;
	}

	noVec3    currentForward = MathHelpers::GetForwardVectorFromMatrix(GetRotation());
	noMat3   temp;	
	noRotation rotate(vec3_zero, UP_VECTOR, RAD2DEG(desiredYaw));
	temp = rotate.ToMat3();

	noVec3    destForward = MathHelpers::GetForwardVectorFromMatrix(temp);

	m_fTotalRotate   = MathHelpers::GetAngleBetweenActorDirections(destForward, currentForward);
	m_fTotalRotate   = DEG2RAD(m_fTotalRotate);
	m_fCurrRotate    = 0;
}

float CharacterRotatorComponent::GetTurnDir(const noVec3& targetpos)
{
	
	noVec3    targetDir = (targetpos - GetTranslation());
	noVec3    forwardDir = MathHelpers::GetForwardVectorFromMatrix(GetRotation());
	noVec3    rightDir = MathHelpers::GetRightVectorFromMatrix(GetRotation());

	MathHelpers::UP_COMPONENT(targetDir) = 0;
	MathHelpers::UP_COMPONENT(rightDir) = 0;
	MathHelpers::UP_COMPONENT(forwardDir) = 0;

	targetDir.Normalize();
	rightDir.Normalize();
	forwardDir.Normalize();

	float dot = targetDir * rightDir;

	if( dot < 0 )
	{
		return 1.0f;
	}

	return -1.0f;
}

void CharacterRotatorComponent::TurnToTarget(const noVec3& targetpos)
{	
	m_turnDir = GetTurnDir(targetpos);
	/*GameObjectRotatorAttribute* roateAtt = GetGameObjectAttribute(m_pkGameObject, GameObjectRotatorAttribute);
	if (roateAtt)
	{
	noVec3    targetDir = (targetpos - GetTranslation());
	roateAtt->m_yawRotateDir = 0;
	roateAtt->m_rollRotateDir = 0;
	targetDir.Normalize();
	roateAtt->FaceDir3D( targetDir );	
	}*/
}

void CharacterRotatorComponent::TickTurnToTarget(const noVec3 targetpos)
{	 

	noVec3    targetDir = (targetpos - GetTranslation());

	noVec3    forwardDir = MathHelpers::GetForwardVectorFromMatrix(GetRotation());
	noVec3    rightDir = MathHelpers::GetRightVectorFromMatrix(GetRotation());

	MathHelpers::UP_COMPONENT(targetDir) = 0;
	MathHelpers::UP_COMPONENT(rightDir) = 0;
	MathHelpers::UP_COMPONENT(forwardDir) = 0;

	targetDir.Normalize();

	float dot = targetDir * rightDir;

	float pct = 0.5f;

	float forwardDot = targetDir * forwardDir;

	if( forwardDot < 0 )
	{
		pct = 1.0f;
	}
	else if( forwardDot > 0.9f )
	{
		pct = 0.25f;
	}

	GameObjectRotatorAttribute* roateAtt = GetGameObjectAttribute(m_pkGameObject, GameObjectRotatorAttribute);
	if (roateAtt)
	{
		roateAtt->m_yawRotateDir = pct * m_turnDir;
		roateAtt->m_rollRotateDir = pct * m_turnDir;
	}

	if( GetTurnDir(targetpos)!=m_turnDir )
	{
		roateAtt->m_yawRotateDir = 0;
		roateAtt->m_rollRotateDir = 0;
		roateAtt->FaceDir2D( targetDir );		
	}
}

noImplementRTTI(GameObjectSlerpRotator,GameObjectComponentBase);
GameObjectSlerpRotator::GameObjectSlerpRotator(GameObject *pkGameObject) : GameObjectComponentBase(pkGameObject)
	, m_startRotTime(0)
	, m_totalRotTime(0)
{
}

bool GameObjectSlerpRotator::Initialize()
{
	return true;
}

bool GameObjectSlerpRotator::Update(float fDelta)
{
	if( m_totalRotTime==0 )
		return false;

	UpdateRotation();

	return true;
}

void GameObjectSlerpRotator::SlerpRotationTo(const noMat3 &destRot, float turnRate)
{
	noMat3       currRot = GetRotation();

	noVec3        vF,vNewF;

	if( GetTime() < m_startRotTime + 0.1f )
		return; // gotta give the slerp routine some time to do its work...

	vF = MathHelpers::GetForwardVectorFromMatrix(GetRotation());
	vNewF = MathHelpers::GetForwardVectorFromMatrix(destRot);

	m_startRotation = GetRotation().ToQuat();
	m_destRotation = destRot.ToQuat();

	float		angle	= MathHelpers::GetAngleBetweenVectors( vNewF, vF );

	m_startRotTime  = GetTime();
	m_totalRotTime  = fabs( angle / turnRate );

	if ( m_totalRotTime==0.0f )
	{
		// no rotation!
		m_startRotTime	= 0.0f;
		m_totalRotTime	= 0.0f;
	}
}

void GameObjectSlerpRotator::FaceDirection(const noVec3 &kDirection, float turnRate)
{
	noMat3   destRot;
	MathHelpers::FaceDirection(destRot,kDirection);

	SlerpRotationTo(destRot,turnRate);
}

void GameObjectSlerpRotator::UpdateRotation()
{
	if ( m_totalRotTime==0 )
		return;

	float		currTime	= GetTime();
	float		timeSoFar	= currTime - m_startRotTime;

	noMat3   destRot;
	destRot = m_destRotation.ToMat3();

	if ( timeSoFar >= m_totalRotTime )
	{
		// we're done
		m_totalRotTime	= 0.0f;
		m_startRotTime	= 0.0f;

		SetRotation( destRot );
		return;
	}

	float		t = timeSoFar / m_totalRotTime;

	noQuat    slerpedQ;
	MathHelpers::Slerp(slerpedQ,m_startRotation,m_destRotation,t);

	noMat3   newRot;
	newRot = slerpedQ.ToMat3();

	SetRotation( newRot );
}

void GameObjectSlerpRotator::Stop()
{
	if( m_totalRotTime>0 )
	{
		m_totalRotTime  = 0;
		m_startRotTime  = 0;
	}
}