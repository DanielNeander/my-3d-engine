#include "stdafx.h"
#include "AnimationComponent.h"
#include <ModelLib/Model_M2.h>
#include <ModelLib/M2Loader.h>
#include "SceneNode.h"
#include "Mesh.h"
#include "M2Mesh.h"
#include "TestObjects.h"
#include "GameApp.h"
#include "AnimationSystem.h"


noImplementRTTI(AnimationComponent,GameObjectComponentBase);
AnimationComponent::AnimationComponent( GameObject *pkGameObject ) : GameObjectComponentBase(pkGameObject)
	,m_currSeq((SeqId)-1)
{

}

AnimationComponent::~AnimationComponent()
{

}

bool AnimationComponent::Initialize()
{
	WowActor* pActor = (WowActor*)m_pkGameObject;
	animsys_ = (AnimationSystem*)pActor->mesh_->m2_->animManager;

	return SetupAnimations();
}

bool AnimationComponent::Update( float fDelta )
{
	return false;
}

bool AnimationComponent::PlayAnim( SeqId id )
{	
		
	bool bLoop = false;
	//if (id != INVALID_SEQ_ID)
	//	bLoop = animsys_->GetAnim(id)->bLooped;		
	AnimSys()->Reset();
	animsys_->SetAnim(0, id, bLoop);
	animsys_->Play();
	m_currSeq = id;
	return true;
}

bool AnimationComponent::PlayAnim( SeqId id, short forceLoop )
{	
	AnimSys()->Reset();	
	animsys_->SetAnim(0, id, forceLoop);
	animsys_->Play();
	m_currSeq = id;
	return true;
}

bool AnimationComponent::IsAnimDone( SeqId id )
{
	if( id==m_currSeq )
		return  false;

	return true;
}

SeqId AnimationComponent::GetSequenceId( const char *animName, int id )
{	
	return animsys_->FindSeqId(animName, id);
}

float AnimationComponent::GetAnimLength( SeqId seqId )
{
	return animsys_->GetAnim(seqId)->duration;
}

void AnimationComponent::ResetAnim( SeqId seqId )
{
	animsys_->Clear();
	PlayAnim(seqId);
}
