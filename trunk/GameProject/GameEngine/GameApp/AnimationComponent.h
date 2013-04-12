#pragma once 

#include "GameObjectComponent.h"
#include "GameObject.h"


typedef unsigned int SeqId;
#define INVALID_SEQ_ID		(uint32)-1

// ---------------------------------------------------------------------------------
// The AnimationComponent is responsible for playing animations and tracking when
// they have finished
// ---------------------------------------------------------------------------------
class AnimationComponent : public GameObjectComponentBase
{
public:	
	noDeclareRTTI;

	AnimationComponent(GameObject *pkGameObject);
	~AnimationComponent();

	virtual bool Initialize();
	virtual bool SetupAnimations() { return true; }   // this is done on a per-character level...

	virtual bool Update(float fDelta);

	virtual bool PlayAnim(SeqId id);
	virtual bool IsAnimDone(SeqId id);

	virtual SeqId GetSequenceId(const char *animName, int id);
	virtual float GetAnimLength(SeqId seqId);
	virtual void ResetAnim(SeqId seqId);

	 bool PlayAnim(SeqId id, short forceLoop);

	SeqId GetCurrSeq() { return m_currSeq; }
	
	class AnimationSystem* AnimSys() const { return animsys_; }

protected:
	SeqId			m_currSeq;

	AnimationSystem* animsys_;

};
