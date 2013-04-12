#pragma once 

#include "AnimationComponent.h"

class AnimationSystem : public M2AnimationManager
{
public:
	AnimationSystem( struct ModelAnimation *anim, int numAnims);

	SeqId	FindSeqId(const char* name);
	SeqId	FindSeqId(const char* name, int subid);
	SeqId FindSeqIdAndBaseIndex( const char* name, int& i);
	void DebugIds();
		
	static M2AnimationManager*	CreateAnimSystem(ModelAnimation *anim, int numAnims);
	
	virtual void SetAnim(unsigned int index, unsigned int id, short loop); // sets one of the 4 existing animations and changes it (not really used currently)

	virtual void Clear();
	
	virtual void Play(); // Players the animation, and reconfigures if nothing currently inputed
	virtual void Stop();
	int Tick( int time );

	bool  IsPlaying() const { return IsPlaying_; }
	int GetCurrSeq() const { return currAnim; }
	
	void Reset();
	static void InitStand(M2Loader* m, bool bLoop = false);
private:
	bool IsPlaying_;
	
};