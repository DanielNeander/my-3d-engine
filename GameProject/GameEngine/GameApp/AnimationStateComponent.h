#pragma once 


#include "GameObjectComponent.h"
#include "GameObject.h"


#pragma once

class AnimationComponent;

// ---------------------------------------------------------------------------------
// IAnimState is the abstract base class for the animation states.
// An animation state is responsible for setting the current animation.
// It could potentially playback multiple animations.
// ---------------------------------------------------------------------------------
class IAnimState : public RefCounter
{
public:
	virtual ~IAnimState() {}
	virtual void OnActivated(AnimationComponent *pkAnimComponent)=0;
	virtual void OnDeactivated(AnimationComponent *pkAnimComponent)=0;
	virtual void Update(AnimationComponent *pkAnimComponent)=0;
	virtual bool IsDone(AnimationComponent *pkAnimComponent)=0;

	virtual bool HasRotationAccumulation()=0;
	virtual bool HasTranslationAccumulation()=0;
};
MSmartPointer(IAnimState);

// ---------------------------------------------------------------------------------
// The AnimStateComponent is responsible for setting the current animation state and
// tracking when it is done.  This class relies heavily on the AnimationComponent 
// class
// ---------------------------------------------------------------------------------
class AnimStateComponent : public GameObjectComponentBase
{
public:
	noDeclareRTTI;

	AnimStateComponent(GameObject *pkGameObject);

	virtual bool Initialize();
	virtual bool SetupAnimStates();

	virtual bool Update(float fDelta);

	bool SetAnimState(IAnimState *pkAnimState);

	bool IsAnimStateDone();

protected:
	class AnimationComponent        *m_pkAnimComponent;
	class AnimStateAttributeBase    *m_pkAnimStateAttrib;

private:
	IAnimState                      *m_pkCurrState;
};

//////////////////////////////////////////////////////////////////////////
// This anim state serves as a base implementation of IAnimState
// Accumulation properties can be set and retrieved with this class.
// The other basic animation states derive from this.
//////////////////////////////////////////////////////////////////////////
class BaseAnimState : public IAnimState
{
public:
	BaseAnimState()
		: m_bAccumulateTranslation(false), m_bAccumulateRotation(false)
	{
	}
	void SetAccumulation(bool bAccumTranslation, bool bAccumRotation)
	{
		m_bAccumulateTranslation = bAccumTranslation;
		m_bAccumulateRotation = bAccumRotation;
	}

	virtual bool HasRotationAccumulation()                              { return m_bAccumulateRotation; }
	virtual bool HasTranslationAccumulation()                           { return m_bAccumulateTranslation; }

protected:
	bool                                    m_bAccumulateTranslation;
	bool                                    m_bAccumulateRotation;

};

//////////////////////////////////////////////////////////////////////////
// Use this class if your animation state only needs to play one animation.
//////////////////////////////////////////////////////////////////////////
class SingleAnimState : public BaseAnimState
{
public:
	SingleAnimState();

	bool Setup(SeqId seqId);

	virtual void OnActivated(AnimationComponent *pkAnimComponent);
	virtual void OnDeactivated(AnimationComponent *pkAnimComponent)     {}
	virtual void Update(AnimationComponent *pkAnimComponent)            {}
	virtual bool IsDone(AnimationComponent *pkAnimComponent);

	SeqId				GetSeqId() const { return m_seqId; }

private:
	SeqId              m_seqId;
};

// ---------------------------------------------------------------------------------
// This class can be used to store a group of animations that can be accessed 
// randomly.  Each animation in the group can be assigned a "chance" that it will
// play next.
// ---------------------------------------------------------------------------------
class AnimationGroup
{
public:
	AnimationGroup();
	~AnimationGroup();

	void Add(SeqId id, uint32 chance = 100, bool bCanRepeat = true);
	void Remove(SeqId id);
	SeqId GetNext();

private:
	class AnimationGroupItem 
	{
	public:
		SeqId          m_id;
		uint32                            m_chance;
		bool                                m_bCanRepeat;

		AnimationGroupItem(SeqId id, uint32 chance, bool bCanRepeat)
			: m_id(id)
			, m_chance(chance)
			, m_bCanRepeat(bCanRepeat)
		{
		}
	};

	std::vector<AnimationGroupItem*>	 m_group;
	AnimationGroupItem*                     m_lastAnim;
	uint32                                m_maxChance;

	AnimationGroupItem* PickAnim();
};

// ---------------------------------------------------------------------------------
// Use this class if your animation state could potentially play multiple animations
// ---------------------------------------------------------------------------------
class GroupedAnimBase : public BaseAnimState
{
public:
	GroupedAnimBase();

	void AddAnimation(SeqId id, uint32 chance=100, bool bCanRepeat=true);
	virtual void OnActivated(AnimationComponent *pkAnimComponent);

	virtual void OnDeactivated(AnimationComponent *pkAnimComponent)     {}
	virtual void Update(AnimationComponent *pkAnimComponent)            {}
	virtual bool IsDone(AnimationComponent *pkAnimComponent);

protected:
	AnimationGroup                      m_group;
	SeqId          m_seqId;
};

// ---------------------------------------------------------------------------------
// Idle Animation State will continually play all the animations in its group.
// ---------------------------------------------------------------------------------
class IdleAnimState : public GroupedAnimBase
{
public:
	virtual void Update(AnimationComponent *pkAnimComponent);
	virtual bool IsDone(AnimationComponent *pkAnimComponent);
};

