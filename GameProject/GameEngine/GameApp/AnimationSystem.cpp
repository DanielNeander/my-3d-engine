#include "stdafx.h"
#include "EngineCore/Util/GameLog.h"
#include "ModelLib/database.h"
#include "ModelLib/Model_M2Format.h"
#include "ModelLib/M2Loader.h"
#include "ModelLib/M2AnimationManager.h"
#include "AnimationSystem.h"

#define	ANIM_STAND	0

M2AnimationManager* AnimationSystem::CreateAnimSystem( ModelAnimation *anim, int numAnims )
{
	return new AnimationSystem(anim, numAnims);
}

AnimationSystem::AnimationSystem( ModelAnimation *anim, int numAnims ) :M2AnimationManager(anim, numAnims)
	,IsPlaying_(false)
{

}

SeqId AnimationSystem::FindSeqId( const char* name, int subid )
{	
	for (int i = 0; i < numAnim_; ++i)
	{
		wxString strName = name;	
		strName += wxString::Format(wxT(" [%i]"), subid);		
		if(strcmp(Animations[i].name.c_str(), strName.wx_str())==0)		
		{
			//LOG_INFO << "Found Anim Name : " << strName.wx_str();
			return Animations[i].id;
		}
	}
	return INVALID_SEQ_ID;
}

SeqId AnimationSystem::FindSeqId( const char* name )
{
	for (int i = 0; i < numAnim_; ++i)
	{
		wxString strName = name;	
		strName += wxString::Format(wxT(" [%i]"), i);		
		if(strcmp(Animations[i].name.c_str(), strName.wx_str())==0)		
		{
			//LOG_INFO << "Found Anim Name : " << strName.wx_str();
			return Animations[i].id;
		}
	}
	return INVALID_SEQ_ID;
}

SeqId AnimationSystem::FindSeqIdAndBaseIndex( const char* name, int& x)
{	
	for (int i = 0; i < numAnim_; ++i)
	{
		wxString strName = name;	
		strName += wxString::Format(wxT(" [%i]"), i);		
		if(strcmp(Animations[i].name.c_str(), strName.wx_str())==0)		
		{
			x = i;
			//LOG_INFO << "Found Anim Name : " << strName.wx_str();
			return Animations[i].id;
		}
	}
	return INVALID_SEQ_ID;
}

int AnimationSystem::Tick( int time )
{
	if (currAnim == -1)
		return -1;

	if((Count < PlayIndex) )
		return -1;

			
	//Frame += ((anims[animList[PlayIndex].AnimID].timeEnd - anims[animList[PlayIndex].AnimID].timeStart) / 60) * 0.1f * Speed; 	
	Frame += int(time * Speed);

	// animate our mouth animation
	if (AnimIDMouth > -1) {
		FrameMouth += (time*mouthSpeed);

		if (FrameMouth >= anims[AnimIDMouth].timeEnd) {
			FrameMouth -= (anims[AnimIDMouth].timeEnd - anims[AnimIDMouth].timeStart);
		} else if (FrameMouth < anims[AnimIDMouth].timeStart) {
			FrameMouth += (anims[AnimIDMouth].timeEnd - anims[AnimIDMouth].timeStart);
		}
	}

	// animate our second (upper body) animation
	if (AnimIDSecondary > -1) {
		FrameSecondary += (time*Speed);

		if (FrameSecondary >= anims[AnimIDSecondary].timeEnd) {
			FrameSecondary -= (anims[AnimIDSecondary].timeEnd - anims[AnimIDSecondary].timeStart);
		} else if (FrameSecondary < anims[AnimIDSecondary].timeStart) {
			FrameSecondary += (anims[AnimIDSecondary].timeEnd - anims[AnimIDSecondary].timeStart);
		}
	}

	if (Frame >= anims[animList[PlayIndex].AnimID].timeEnd) {
		//Next();
		if(CurLoop == 1 ) {									
			currAnim = animList[PlayIndex].AnimID;
			Paused = false;
			// 리셋 필요
			Frame = anims[animList[PlayIndex].AnimID].timeStart;
			if (gameVersion >= VERSION_WOTLK)
				TotalFrames = GetFrameCount();
		}
		else {
			Paused = true;
			currAnim = -1;
			IsPlaying_ = false;
		}
		//}
		return 1;
	} else if (Frame < anims[animList[PlayIndex].AnimID].timeStart) {
		Prev();
		return 1;
	}

	return 0;
}

void AnimationSystem::Play()
{
	if (currAnim == (unsigned int)-1)
		return;

	PlayIndex = 0;
	//if (Frame == 0 && PlayID == 0) {
	CurLoop = animList[PlayIndex].Loops;
	if (!IsPlaying_)
		Frame = anims[animList[PlayIndex].AnimID].timeStart;
	TotalFrames = GetFrameCount();
	//}

	IsPlaying_ = true;

	Paused = false;
	AnimParticles = false;
}

void AnimationSystem::Stop()
{
	Paused = true;
	PlayIndex = 0;
	Frame = anims[animList[0].AnimID].timeStart;
	CurLoop = animList[0].Loops;
}

void AnimationSystem::SetAnim( unsigned int index, unsigned int id, short loop )
{
	// error check, we currently only support 4 animations.	
	if (index > 3)
		return;

	animList[index].AnimID = id;
	animList[index].Loops = loop;	

	currAnim = id;

	if (id == (unsigned int)-1)
		return;

	// Just an error check for our "auto animate"
	if (index == 0) {
		Count = 1;
		PlayIndex = index;

		if (!IsPlaying_)
			Frame = anims[id].timeStart;
		TotalFrames = anims[id].timeEnd - anims[id].timeStart;
	}

	if (index+1 > Count)
		Count = index+1;
}



void AnimationSystem::Reset()
{
	Count = 0;
	//Clear();
	Paused = false;
	IsPlaying_ = false;
	currAnim = -1;
	Frame = 0;

}

void AnimationSystem::Clear()
{
	//Stop();
	Paused = true;
	PlayIndex = 0;
	Count = 0;	
	Frame = 0;
	currAnim = -1;
	IsPlaying_ = false;

}

void AnimationSystem::DebugIds()
{
	for (int i = 0; i < numAnim_; ++i)
	{
		LOG_INFO <<"AnimName" <<  Animations[i].name.c_str() << "AnimLoops " <<  (Animations[i].bLooped ? 1 : 0) << "Play Spped : " << Animations[i].playSpeed 
			<< " Move Speed: " << Animations[i].moveSpeed;
	}
}

void AnimationSystem::InitStand(M2Loader* m, bool bLoop)
{
	ssize_t useanim = -1;

	if (m->animated && m->anims) {
		wxString strName;
		wxString strStand;
		int selectAnim = 0;
		for (size_t i=0; i< m->header.nAnimations; i++) {			
			try {
				AnimDB::Record rec = animdb.getByAnimID(m->anims[i].animID);
				strName = rec.getString(AnimDB::Name);
			} catch (AnimDB::NotFound) {
				strName = wxT("???");
			}

			//strName = name;
			//if ((useanim==-1) && (strName=="Stand"))
			//	useanim = i;

			strName += wxString::Format(wxT(" [%i]"), i);

			if (m->anims[i].animID == ANIM_STAND && useanim == -1) {
				strStand = strName;
				useanim = i;
			}

			//animCList->Append(strName);
			//if (g_selModel->charModelDetails.isChar) {
			//animCList2->Append(strName);
			//animCList3->Append(strName);
			//}
		}

		/*if (useanim != -1) {
			for(unsigned int i=0; i<animCList->GetCount(); i++) {
				strName = animCList->GetString(i);
				if (strName == strStand) {
					selectAnim = i;
					break;
				}
			}
		}*/

		if (useanim==-1)
			useanim = 0;
		//return;

		m->currAnim = useanim; // anim position in anims
		//animCList->Select(selectAnim); // anim position in selection
		//animCList->Show(true);

		//frameSlider->SetRange(g_selModel->anims[useanim].timeStart, g_selModel->anims[useanim].timeEnd);
		//frameSlider->SetTickFreq(g_selModel->anims[useanim].playSpeed, 1);

		m->animManager->SetAnim(0, useanim, 0);
		/*if (bNextAnims && g_selModel) {
			int NextAnimation = useanim;
			for(size_t i=1; i<4; i++) {
				NextAnimation = g_selModel->anims[NextAnimation].NextAnimation;
				if (NextAnimation >= 0)
					g_selModel->animManager->AddAnim(NextAnimation, loopList->GetSelection());
				else
					break;
			}
		}*/		
		m->animManager->Play();
		((AnimationSystem*)m->animManager)->SetLoop(bLoop);
	}
}

