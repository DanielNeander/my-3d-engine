#include "stdafx.h"
#include "AnimationComponent.h"
#include "AnimationStateComponent.h"
#include "GameObjectAttribute.h"
#include "GameObjectUtil.h"

AnimationGroup::AnimationGroup()
{
	m_lastAnim  = NULL;
	m_maxChance = 0;
}

AnimationGroup::~AnimationGroup()
{
	for (uint32 ui=0;ui<m_group.size();ui++)
	{
		delete m_group.at(ui);
	}
}

void AnimationGroup::Add(SeqId id, uint32 chance, bool bCanRepeat)
{
	if (id != INVALID_SEQ_ID)
	{
		AnimationGroup::AnimationGroupItem  *pkItem = new AnimationGroup::AnimationGroupItem(id,chance,bCanRepeat);
		m_group.push_back(pkItem);
		m_maxChance += pkItem->m_chance;
	}
}

void AnimationGroup::Remove(SeqId id)
{
	for (uint32 ui=0;ui<m_group.size();ui++)
	{
		AnimationGroup::AnimationGroupItem *pkItem = m_group.at(ui);
		if ( pkItem->m_id==id )
		{
			m_maxChance -= pkItem->m_chance;

			delete pkItem;
			m_group[ui] = NULL;
			return;
		}
	}
}

SeqId AnimationGroup::GetNext()
{
	AnimationGroup::AnimationGroupItem* pkItem = PickAnim();

	if( !pkItem )
		return INVALID_SEQ_ID;

	m_lastAnim = pkItem;

	return pkItem->m_id;
}

AnimationGroup::AnimationGroupItem* AnimationGroup::PickAnim()
{
	int random = MathHelpers::GetRandomIntegerRangeZeroTo(m_maxChance);
	int counter = 0;
	uint32 size = m_group.size();

	if( size==1 )
		return m_group.at(0);

	for (uint32 ui=0;ui<size;ui++)
	{
		AnimationGroup::AnimationGroupItem  *pkItem = m_group.at(ui);

		if( m_lastAnim==pkItem && !pkItem->m_bCanRepeat )
			continue;

		counter += pkItem->m_chance;

		if( random < counter )
			return pkItem;
	}

	AnimationGroup::AnimationGroupItem  *pkItem = m_group.at(size-1);

	if (pkItem==m_lastAnim && !m_lastAnim->m_bCanRepeat )
	{
		pkItem = m_group.at(0);
		assert(pkItem!=m_lastAnim);
	}

	return pkItem;
}

// -----------------------------------------------------------------
// -----------------------------------------------------------------
GroupedAnimBase::GroupedAnimBase() : m_seqId(INVALID_SEQ_ID)
{
}

void GroupedAnimBase::AddAnimation(SeqId id, uint32 chance, bool bCanRepeat)
{
	if (id == INVALID_SEQ_ID)
		return;
	m_group.Add(id,chance,bCanRepeat);
}

void GroupedAnimBase::OnActivated(AnimationComponent *pkAnimComponent)
{
	m_seqId = m_group.GetNext();
	pkAnimComponent->PlayAnim(m_seqId);
}

bool GroupedAnimBase::IsDone(AnimationComponent *pkAnimComponent)
{
	if( pkAnimComponent->IsAnimDone(m_seqId) )
		return true;

	return false;
}

// -----------------------------------------------------------------
// -----------------------------------------------------------------
void IdleAnimState::Update(AnimationComponent *pkAnimComponent)
{
	// if current anim is done, play the next one...
	if( pkAnimComponent->IsAnimDone(m_seqId) )
	{
		m_seqId = m_group.GetNext();
		pkAnimComponent->PlayAnim(m_seqId);
	}
}

bool IdleAnimState::IsDone(AnimationComponent *pkAnimComponent)
{
	// idle anim state never really ends....
	return false;
}


// -----------------------------------------------------------------
// -----------------------------------------------------------------
SingleAnimState::SingleAnimState() : m_seqId(INVALID_SEQ_ID)
{
}

bool SingleAnimState::Setup(SeqId seqId)
{
	m_seqId = seqId;

	if( m_seqId==INVALID_SEQ_ID )
		return false;

	return true;
}

void SingleAnimState::OnActivated(AnimationComponent *pkAnimComponent)
{
	assert(m_seqId!=INVALID_SEQ_ID);
	pkAnimComponent->PlayAnim(m_seqId);
}

bool SingleAnimState::IsDone(AnimationComponent *pkAnimComponent)
{
	if( pkAnimComponent->IsAnimDone(m_seqId) )
		return true;

	return false;
}

// -------------------------------------------------------------
// -------------------------------------------------------------
noImplementRTTI(AnimStateComponent,GameObjectComponentBase);
AnimStateComponent::AnimStateComponent(GameObject *pkGameObject) : GameObjectComponentBase(pkGameObject)
	, m_pkAnimComponent(NULL), m_pkAnimStateAttrib(NULL), m_pkCurrState(NULL)
{

}
bool AnimStateComponent::Initialize()
{
	// grab pointer to AnimationComponent.... required!
	m_pkAnimComponent = GetGameObjectComponent(GetGameObject(),AnimationComponent);
	assert(m_pkAnimComponent);

	return SetupAnimStates();
}

bool AnimStateComponent::SetupAnimStates()
{
	// can be overridden buy child class....
	return true;
}

bool AnimStateComponent::Update(float fDelta)
{
	if( !m_pkAnimStateAttrib )
	{
		m_pkAnimStateAttrib     = GetGameObjectAttribute(GetGameObject(),AnimStateAttributeBase);
	}

	assert(m_pkAnimStateAttrib);

	IAnimState  *pkAnimState = m_pkAnimStateAttrib->GetAnimState();

	if( pkAnimState!=m_pkCurrState )
		SetAnimState(pkAnimState);

	if (m_pkCurrState)
	{
		m_pkCurrState->Update(m_pkAnimComponent);
	}
	return true;
}

bool AnimStateComponent::SetAnimState(IAnimState *pkAnimState)
{
	// We allow this to be called multiple times with same anim state.
	// Only do all the activation if the anim state differs....
	if( pkAnimState==m_pkCurrState )
		return true;

	//if( m_pkCurrState )
	//	m_pkCurrState->OnDeactivated(m_pkAnimComponent);

	m_pkCurrState = pkAnimState;  

	//if (m_pkCurrState) 
	//	m_pkCurrState->OnActivated(m_pkAnimComponent);

	return true;
}

bool AnimStateComponent::IsAnimStateDone()
{
	if(m_pkCurrState)
		return m_pkCurrState->IsDone(m_pkAnimComponent);

	return true;
}

