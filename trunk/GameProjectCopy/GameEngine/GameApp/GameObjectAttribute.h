#pragma once 

class IGameObjectAttribute : public RefCounter
{
public:
	noDeclareRootRTTI(IGameObjectAttribute);
	virtual ~IGameObjectAttribute() {}


};

class GameObjectAttributeBase : public IGameObjectAttribute
{
public:
};

// ---------------------------------------------------------------------------------
// This attribute is used for doing yaw,pitch,roll rotations on game objects
// It is required or used by the following components: 
//  GameObjectRotatorComponent
//  CharacterRotatorComponent
//  PlayerInputComponentBase
//  PhysXFirstPersonCamera
//  PhysXPlayerCharacter
// ---------------------------------------------------------------------------------
class GameObjectRotatorAttribute : public GameObjectAttributeBase
{
public:
	noDeclareRTTI;

	float			        m_yawRate;
	float			        m_pitchRate;
	float			        m_rollRate;

	float					m_fMaxPitch;
	float					m_fMinPitch;

	float					m_fMaxRoll;
	float					m_fMinRoll;

	float					m_fMaxYaw;
	float					m_fMinYaw;

	float					m_yawRotateDir;
	float					m_pitchRotateDir;
	float					m_rollRotateDir;


	GameObjectRotatorAttribute();


	void RotateYaw(float direction)		{ m_yawRotateDir = direction; }
	void RotatePitch(float direction)	{ m_pitchRotateDir = direction; }
	void RotateRoll(float direction)	{ m_rollRotateDir = direction; }

	float GetYaw()						{ return m_fCurrYaw; }
	float GetPitch()					{ return m_fCurrPitch; }
	float GetRoll()						{ return m_fCurrRoll; }

	void UpdateYawRotate(float fDelta);
	void UpdatePitchRotate(float fDelta);
	void UpdateRollRotate(float fDelta);

	float GetPitchPct();

	void SetYaw(float newYaw);
	void SetPitch(float newPitch);
	void SetRoll(float newRoll);

	void AddYaw(float delta);
	void AddPitch(float delta);
	void AddRoll(float delta);

	void UpdateYaw(float desiredYawPct, float fDelta);
	void UpdatePitch(float desiredPitchPct, float fDelta);
	void UpdateRoll(float desiredRollPct, float fDelta);

	void UpdateDesiredYaw(float fDelta);
	void UpdateDesiredPitch(float fDelta);
	void UpdateDesiredRoll(float fDelta);

	noMat3 GetRotation();
	noMat3 GetCameraRotation();
	noMat3 GetCharacterRotation();

	void ClearDesiredYaw();
	void ClearDesiredPitch();
	void ClearDesiredRoll();

	void SetDesiredYaw(float yaw);
	void SetDesiredPitch(float pitch)           { m_desiredPitch = pitch; }
	void SetDesiredRoll(float roll)             { m_desiredRoll = roll; }

	float GetDesiredYaw()                       { return m_desiredYaw; }
	float GetDesiredPitch()                     { return m_desiredPitch; }
	float GetDesiredRoll()                      { return m_desiredRoll; }

	void FaceDir2D(const noVec3 &dir);        // updates only the desired Yaw
	void FaceDir3D(const noVec3 &dir);        // updates desired Yaw and Pitch...

	void ResetYaw(const noMat3 &rotation);

	bool HasDesiredPitch() const;
	bool HasDesiredYaw() const;
	bool HasDesiredRoll() const;

protected:
	float					m_fCurrYaw;
	float					m_fCurrPitch;
	float					m_fCurrRoll;

	float			        m_desiredYaw;
	float			        m_desiredPitch;
	float			        m_desiredRoll;


};

// ---------------------------------------------------------------------------------
// This attribute can be used to set the current animation state.  The 
// AnimationStateComponent tracks this attribute and sets the animation state
// when it changes
// ---------------------------------------------------------------------------------
class AnimStateAttributeBase : public GameObjectAttributeBase
{
public:
	noDeclareRTTI;

	AnimStateAttributeBase();

	void SetAnimState(class IAnimState *pkState);
	IAnimState *GetAnimState();

	void SetOverrideAnimState( IAnimState *pkAnimState );

private:
	class IAnimState*               m_pkAnimState;
	class IAnimState*               m_pkAnimStateOverride;
};

// ---------------------------------------------------------------------------------
// this is a helper AnimStateAttribute that has animstates for most commonly used
// character animation states.  This attribute is used by the AI 
// behaviors to set the current animation state
// ---------------------------------------------------------------------------------
class BasicCharacterAnimStateAttribute : public AnimStateAttributeBase
{
public:

	noDeclareRTTI;

	IAnimState                  *m_pkIdle;

	IAnimState                  *m_pkWalk;
	IAnimState                  *m_pkWalkBack;
	IAnimState                  *m_pkWalkLeft;
	IAnimState                  *m_pkWalkRight;

	IAnimState                  *m_pkRun;
	IAnimState                  *m_pkRunLeft;
	IAnimState                  *m_pkRunRight;
	IAnimState                  *m_pkRunBack;

	IAnimState                  *m_pkPunch;
	IAnimState                  *m_pkKick;

	IAnimState                  *m_pkPain;
	IAnimState                  *m_pkDeath;

	IAnimState                  *m_pkTurnLeft;
	IAnimState                  *m_pkTurnRight;

	BasicCharacterAnimStateAttribute();

	void Idle();

	void Walk();
	void WalkBack();
	void WalkLeft();
	void WalkRight();

	void Run();
	void RunLeft();
	void RunRight();
	void RunBack();

	void TurnLeft();
	void TurnRight();

	void Punch();
	void Kick();

	void Pain();
	void Death();

};

//////////////////////////////////////////////////////////////////////////
// Interface class that different types of movement components subclass
// from.  The primary responsibility is for this attribute to provide
// the various game object mover components a movement delta from the
// previous frame.
//////////////////////////////////////////////////////////////////////////
class ICharacterMovementAttribute : public IGameObjectAttribute
{
public:
	noDeclareRTTI;

	virtual noVec3 GetMoveDelta(float fFrameTime) = 0;
};

// ---------------------------------------------------------------------------------
// This attribute contains basic character movement attributes.  This is used by 
// both the player input components and the AI behavior components
// ---------------------------------------------------------------------------------
class BasicCharacterMovementAttribute : public ICharacterMovementAttribute
{
public:
	noDeclareRTTI;

	BasicCharacterMovementAttribute(float walkSpeed,float runSpeed);

	void Walk();
	void Run();
	void Stop();

	float GetDesiredSpeed()                                     { return m_desiredSpeed; }

	void SetMoveDir(const noVec3 &moveDir)                    { m_moveDir = moveDir; }
	noVec3 GetMoveDir()                                       { return m_moveDir; }

	virtual noVec3 GetMoveDelta(float fFrameTime)             { return (m_moveDir * m_desiredSpeed) * fFrameTime; }

	void SetWalkSpeed(float speed)								{ m_walkSpeed = speed; }
	float GetWalkSpeed()										{ return m_walkSpeed; }

	void SetRunSpeed(float speed)								{ m_runSpeed = speed; }
	float GetRunSpeed()											{ return m_runSpeed; }

protected:
	float           m_runSpeed;
	float           m_walkSpeed;
	float           m_desiredSpeed;
	noVec3        m_moveDir;
};