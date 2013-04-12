#pragma once

#include "GameObject.h"

class IEventListener
{
public:
	//    virtual const NiRTTI* GetRTTI()=0;
};

class IGameObjectComponent : public RefCounter
{
public:	
	noDeclareRootRTTI(IGameObjectComponent);
	virtual ~IGameObjectComponent() {}

	virtual bool Initialize()=0;
	virtual bool Update(float fDelta)=0;
	virtual bool PostUpdate(float fDelta)=0;
	virtual bool ProcessInput()=0;
	virtual bool IsEnabled()=0;
	virtual bool ShouldRemove()=0;
	virtual void OnTeleported()=0;      // Game object was teleported.  This is used by collision components...
	//virtual bool OnCollision(const GameObjectCollision &collisionArray)=0;

};

class ITransformComponent : public IGameObjectComponent
{
public:
	noDeclareRTTI;
	virtual void SetTranslation(const noVec3 &position)=0;
	virtual void SetRotation(const noMat3 &rotation)=0;
	virtual void SetHeight(float height)=0;

	virtual void SetScale( float scale )=0;
	virtual float GetScale()=0;

	virtual noVec3 GetTranslation()=0;
	virtual noMat3 GetRotation()=0;

	// Some objects are centered in the middle of the object.  Others are centered
	// at the bottom of the object.  This translation offset helps support this.
	virtual void SetTranslationOffset(const noVec3 &offset)=0;
	virtual noVec3 GetTranslationOffset()=0;

	//virtual bool OnCollision(const GameObjectCollision &collisionArray)   { return false; }

};

class DefaultTransformComponent : public ITransformComponent
{
public:
	noDeclareRTTI;


	DefaultTransformComponent()	: m_translation(vec3_zero)
		, m_rotation(mat3_default)
		, m_scale(1.0f)
	{
	}

	virtual bool Initialize()                                   { return true; }
	virtual bool Update(float fDelta)                           { return false; }
	virtual bool PostUpdate(float fDelta)                       { return false; }
	virtual bool ProcessInput()                                 { return false; }
	virtual bool IsEnabled()                                    { return true; }
	virtual bool ShouldRemove()                                 { return false; }
	virtual void OnTeleported()                                 {}
	//virtual bool OnCollision(const GameObjectCollision &collisionArray)   { return false; }

	virtual void SetTranslation(const noVec3 &position)       { m_translation = position; }
	virtual void SetRotation(const noMat3 &rotation)         { m_rotation = rotation; }
	virtual void SetHeight(float height)                        { }

	virtual void SetScale( float scale )                        { m_scale = scale; }
	virtual float GetScale()                                    { return m_scale; }

	virtual noVec3 GetTranslation()                           { return m_translation; }
	virtual noMat3 GetRotation()                             { return m_rotation; }

	virtual void SetTranslationOffset(const noVec3 &offset)   { }
	virtual noVec3 GetTranslationOffset()                     { return vec3_zero; }

private:
	noVec3        m_translation;
	noMat3		  m_rotation;
	float         m_scale;
};


class NodeTransformComponent : public ITransformComponent
{
public:
	noDeclareRTTI;
	
	NodeTransformComponent(GameObject *pkGameObject);

	virtual bool Initialize()								{ return true; }
	virtual bool ProcessInput()								{ return false; }
	virtual bool IsEnabled()								{ return true; }
	virtual bool ShouldRemove()								{ return false; }
	virtual void OnTeleported()                             { }

	virtual bool Update(float fDelta);
	virtual bool PostUpdate(float fDelta);

	virtual void SetTranslation(const noVec3 &position);
	virtual void SetRotation(const noMat3 &rotation);
	virtual void SetHeight(float height);
	virtual void SetScale( float scale );

	virtual float GetScale();
	virtual noVec3 GetTranslation();
	virtual noMat3 GetRotation();

	virtual void SetTranslationOffset(const noVec3 &offset)       { m_offset = offset; }
	virtual noVec3 GetTranslationOffset()                         { return m_offset; }



protected:
	GameObject					*m_pkGameObject;
	noVec3                    m_offset;

	SceneNode *GetNode();

};



class GameObjectComponentBase : public FSMObject, public IGameObjectComponent
{
public:
	noDeclareRTTI;

	GameObjectComponentBase(class GameObject *pkGameObject);

	virtual bool Update(float fDelta);
	virtual bool PostUpdate(float fDelta);
	virtual bool ProcessInput();

	inline GameObject *GetGameObject()                              { return m_pkGameObject; }

	virtual void SetEnabled(bool enabled)                           { m_bEnabled = enabled; }
	virtual bool IsEnabled()                                        { return m_bEnabled; }
	virtual bool ShouldRemove()                                     { return false; }
	virtual void OnTeleported()                                     { }
//	virtual bool OnCollision(const GameObjectCollision &collisionArray)   { return false; }


protected:
	GameObject      *m_pkGameObject;
	bool            m_bEnabled;

	// helper functions
	noVec3 GetTranslation();
	noMat3 GetRotation();
	noVec3 GetVelocity();
	float GetScale();

	void SetTranslation(const noVec3 &translation);
	void SetRotation(const noMat3 &rotation);
	void SetVelocity(const noVec3 &velocity);
	void SetScale(float fScale);
	void SetHeight(float height);
};

class ActorTransformComponent : public NodeTransformComponent
{
public:
	noDeclareRTTI;

	ActorTransformComponent(GameObject *pkGameObject);

	virtual bool Update(float fDelta);
	virtual bool PostUpdate(float fDelta);	

protected:
	GameObject					*m_pkGameObject;
};


// ---------------------------------------------------------------------------------
// This component is used to rotate a character around the up vector.  
// ---------------------------------------------------------------------------------
class CharacterRotatorComponent : public GameObjectComponentBase
{
public:
	noDeclareRTTI;

	CharacterRotatorComponent(GameObject *pkGameObject);

	virtual bool Initialize();
	virtual bool Update(float fDelta);


	void SetEnabled(bool enabled);

	void TurnToTarget(const noVec3& targetpos);
protected:
	float                       m_fTotalRotate;
	float                       m_fCurrRotate;
	float						m_turnDir;
	float                       m_destinationYaw;	
	bool                        m_bHasDesiredYaw;
	noVec3	                    m_moveDir;
	

	class GameObjectRotatorAttribute*           m_pkRotatorAttrib;
	class AnimStateAttributeBase*               m_pkAnimStateAttrib;

	virtual void SetObjectRotation();

	// radians / sec
	void SetDesiredYaw(float desiredYaw);
	void TickTurnToTarget(const noVec3 targetpos);
	float GetTurnDir(const noVec3& targetpos);	

};

class CharacterMoverComponent : public GameObjectComponentBase
{
public:
	noDeclareRTTI;

	CharacterMoverComponent(GameObject *pkGameObject);

	virtual bool Initialize();
	virtual bool Update(float fDelta);

	void MoveTo(const noVec3 &destination);
	bool IsMoveDone()                               { return !m_bMoveDest; }

	void Stop();

private:
	bool                                            m_bMoveDest;
	noVec3                                        m_destination;
	noVec3                                        m_velocity;
	class BasicCharacterMovementAttribute*          m_pkMovementAttrib;

	void UpdateMoveTo(float fDelta);
	void UpdateMoveInDirection(float fDelta);
};

// ---------------------------------------------------------------------------------
// This is a simplistic component which is responsible for checking with the PickController 
// to keep the character on the walk-able mesh.  
// ---------------------------------------------------------------------------------
class FloorMoverComponent : public GameObjectComponentBase
{
public:
	noDeclareRTTI;

	FloorMoverComponent(GameObject *pkGameObject);

	bool Initialize()                                   { return true; }

	void Setup(float descendRate, float ascendRate, float stepHeight, float footOffset);
	bool PostUpdate(float fDelta);
	bool CheckForFloor(float fDelta);

	virtual void OnTeleported();

protected:
	float           m_ascendRate;
	float           m_descendRate;
	float           m_stepHeight;
	float           m_footOffset;
	noVec3        m_prevPosition;

	bool GetHeight(const noVec3 &pos, float &height);

};

// ---------------------------------------------------------------------------------
// Simple component used to remove the gameobject after a specified period of time.
//
//  Once the Event system is fully implemented, we can replace this with a timed 
//  Event instead...
// ---------------------------------------------------------------------------------
class GameObjectRemoverComponent : public GameObjectComponentBase
{
public:
	noDeclareRTTI;
	GameObjectRemoverComponent(GameObject *pkGameObject);

	virtual bool Initialize()                           { return true; }
	virtual bool Update(float fDelta);
	virtual void SetRemoveTime(float fRemoveTime)       { m_removeTime = fRemoveTime; }

private:
	float           m_removeTime;

};

// ---------------------------------------------------------------------------------
// This component provides functionality for smooth object rotating. (using Slerp)
// ---------------------------------------------------------------------------------
class GameObjectSlerpRotator : public GameObjectComponentBase
{
public:
	noDeclareRTTI;
	GameObjectSlerpRotator(GameObject *pkGameObject);

	virtual bool Initialize();
	virtual bool Update(float fDelta);
	virtual void SlerpRotationTo(const noMat3 &rotation, float turnRate);
	virtual void FaceDirection(const noVec3 &kDirection, float turnRate);
	virtual void Stop();

protected:
	float                   m_startRotTime;
	float                   m_totalRotTime;
	noQuat					m_destRotation;
	noQuat				    m_startRotation;

	void UpdateRotation();
};
