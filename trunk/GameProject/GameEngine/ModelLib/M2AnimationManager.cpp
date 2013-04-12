#include "stdafx.h"
#include "EngineCore/Util/GameLog.h"
#include "M2Enums.h"
#include "Model_M2Format.h"
#include "M2AnimationManager.h"
#include "M2Loader.h"
#include "database.h"

extern int gameVersion;

void printfMat4(const mat4& mat)
{
	LOG_DEBUG << "00 " << mat.rows[0][0] << " 01 " << mat.rows[0][1] << " 02 " << mat.rows[0][2] << " 03 " << mat.rows[0][3];
	LOG_DEBUG << "10 " << mat.rows[1][0] << " 11 " << mat.rows[1][1] << " 12 " << mat.rows[1][2] << " 13 " << mat.rows[1][3];
	LOG_DEBUG << "20 " << mat.rows[2][0] << " 21 " << mat.rows[2][1] << " 22 " << mat.rows[2][2] << " 23 " << mat.rows[2][3];
	LOG_DEBUG << "30 " << mat.rows[3][0] << " 31 " << mat.rows[3][1] << " 32 " << mat.rows[3][2] << " 33 " << mat.rows[3][3];

}


M2AnimationManager::M2AnimationManager( ModelAnimation *anim, int numAnims )
	:numAnim_(numAnims)
{
	AnimIDSecondary = -1;
	SecondaryCount = UPPER_BODY_BONES;
	AnimIDMouth = -1;
	anims = anim;
	AnimParticles = false;
	currAnim = -1;
		
	Animations = new M2Animation[numAnims];

	wxString strName;
	for (int i = 0; i < numAnims; ++i)
	{
		Animations[i].id = i;
		Animations[i].duration = anims[i].timeEnd - anims[i].timeStart;
		
		try {
			AnimDB::Record rec = animdb.getByAnimID(anims[i].animID);
			strName = rec.getString(AnimDB::Name);
		} catch (AnimDB::NotFound) {
			strName = wxT("???");
		}
		strName += wxString::Format(wxT(" [%i]"), i);
		Animations[i].name = strName.wx_str();
		Animations[i].NextAnimation = anims[i].NextAnimation;
		Animations[i].bLooped = (anims[i].flags && ANIMATION_LOOPED) != 0;
		Animations[i].playSpeed = anims[i].playSpeed;
		Animations[i].moveSpeed = anims[i].moveSpeed;
	}

	Count = 1;
	PlayIndex = 0;
	CurLoop = 0;
	animList[0].AnimID = 0;
	animList[0].Loops = 0;

	if (anims != NULL) { 
		Frame = anims[0].timeStart;
		TotalFrames = anims[0].timeEnd - anims[0].timeStart;
	} else {
		Frame = 0;
		TotalFrames = 0;
	}

	Speed = 1.0f;
	mouthSpeed = 1.0f;

	Paused = false;

}

M2AnimationManager::~M2AnimationManager()
{
	delete Animations;
	Animations = 0;
	anims = NULL;
}

void M2AnimationManager::SetCount( int count )
{
	Count = count;

}

void M2AnimationManager::AddAnim( unsigned int id, short loops )
{
	if (Count > 3)
		return;

	animList[Count].AnimID = id;
	animList[Count].Loops = loops;
	Count++;
}

void M2AnimationManager::SetAnim( unsigned int index, unsigned int id, short loops )
{
	// error check, we currently only support 4 animations.	
	if (index > 3)
		return;

	animList[index].AnimID = id;
	animList[index].Loops = loops;

	currAnim = id;

	if (id == (unsigned int)-1)
		return;
	
	// Just an error check for our "auto animate"
	if (index == 0) {
		Count = 1;
		PlayIndex = index;
		Frame = anims[id].timeStart;
		TotalFrames = anims[id].timeEnd - anims[id].timeStart;
	}

	if (index+1 > Count)
		Count = index+1;
}

void M2AnimationManager::Play()
{
	if (currAnim == (unsigned int)-1)
		return;

	PlayIndex = 0;
	//if (Frame == 0 && PlayID == 0) {
	CurLoop = animList[PlayIndex].Loops;
	Frame = anims[animList[PlayIndex].AnimID].timeStart;
	TotalFrames = GetFrameCount();
	//}

	Paused = false;
	AnimParticles = false;
}

void M2AnimationManager::Stop()
{
	Paused = true;
	PlayIndex = 0;
	Frame = anims[animList[0].AnimID].timeStart;
	CurLoop = animList[0].Loops;
}

void M2AnimationManager::Pause( bool force /*= false*/ )
{
	if (Paused && force == false) {
		Paused = false;
		AnimParticles = !Paused;
	} else {
		Paused = true;
		AnimParticles = !Paused;
	}
}

void M2AnimationManager::Next()
{
	if(CurLoop == 1) {
		PlayIndex++;
		if (PlayIndex >= Count) {
			Stop();
			return;
		}

		CurLoop = animList[PlayIndex].Loops;
	} else if(CurLoop > 1) {
		CurLoop--;
	} else if(CurLoop == 0) {
		PlayIndex++;
		if (PlayIndex >= Count) {
			PlayIndex = 0;
		}
	}
	// don't change g_selModel->currentAnim in AnimManager
	//g_selModel->currentAnim = animList[PlayIndex].AnimID;

	Frame = anims[animList[PlayIndex].AnimID].timeStart;
	if (gameVersion >= VERSION_WOTLK)
		TotalFrames = GetFrameCount();
}

void M2AnimationManager::Prev()
{
	if(CurLoop >= animList[PlayIndex].Loops) {
		PlayIndex--;

		if (PlayIndex < 0) {
			Stop();
			return;
		}

		CurLoop = animList[PlayIndex].Loops;
	} else if(CurLoop < animList[PlayIndex].Loops) {
		CurLoop++;
	}

	Frame = anims[animList[PlayIndex].AnimID].timeEnd;
	if (gameVersion >= VERSION_WOTLK)
		TotalFrames = GetFrameCount();
}

int M2AnimationManager::Tick( int time )
{
	if((Count < PlayIndex) )
		return -1;

	Frame += ((anims[animList[PlayIndex].AnimID].timeEnd - anims[animList[PlayIndex].AnimID].timeStart) / 60); 
	//Frame += int(time*Speed);

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
		Next();
		return 1;
	} else if (Frame < anims[animList[PlayIndex].AnimID].timeStart) {
		Prev();
		return 1;
	}

	return 0;
}

size_t M2AnimationManager::GetFrameCount()
{
	return (anims[animList[PlayIndex].AnimID].timeEnd - anims[animList[PlayIndex].AnimID].timeStart);

}

void M2AnimationManager::SetFrame( size_t f )
{
	//TimeDiff = f - Frame;
	Frame = f;
}

void M2AnimationManager::PrevFrame()
{
	//AnimateParticles();
	ssize_t id = animList[PlayIndex].AnimID;
	Frame -= 1;//((anims[id].timeEnd - anims[id].timeStart) / 60);
	TimeDiff = ((anims[id].timeEnd - anims[id].timeStart) / 60) * -1;
}

void M2AnimationManager::NextFrame()
{
	//AnimateParticles();
	ssize_t id = animList[PlayIndex].AnimID;
	Frame += 1; //((anims[id].timeEnd - anims[id].timeStart) / 60);
	TimeDiff = ((anims[id].timeEnd - anims[id].timeStart) / 60);
}

void M2AnimationManager::Clear()
{
	Stop();
	Paused = true;
	PlayIndex = 0;
	Count = 0;
	CurLoop = 0;
	Frame = 0;
}

ssize_t M2AnimationManager::GetTimeDiff()
{
	ssize_t t = TimeDiff;
	TimeDiff = 0;
	return t;
}

void M2AnimationManager::SetTimeDiff( ssize_t i )
{
	TimeDiff = i;
}

void M2AnimationManager::ProcessAnimations(M2Loader* pLoader)
{	
	int oldGlobalTime = globalTime ;
	//for ( int i = 0; i < pLoader->header.nAnimations; i++)	
	int i = currAnim;
	{
		globalTime = oldGlobalTime;
		pLoader->animManager->SetAnim(0, i, 0);
		pLoader->animManager->Play();
		for ( ; GetFrame() < GetFrameCount(); )
		{
			globalTime += GetFrame();
			pLoader->calcBones(Animations[i], GetFrame());			
			//NextFrame();
			Frame += 1;

			/*ssize_t id = animList[PlayIndex].AnimID;
			if ((anims[id].timeEnd - anims[id].timeStart) / 60 == 0)
			{				
				Frame += 1; 
			}*/
		}
		for (int y = 0; y < Animations[i].GetNumFrames(); y++ )
		{
			noMat4* mat = Animations[i].GetFrame(y);
			mat4* frame = new mat4[pLoader->header.nBones];
			for (int x = 0; x < pLoader->header.nBones; ++x)
			{
				/*mat[x].mat[3][0] = mat[x].mat[0][3];
				mat[x].mat[3][1] = mat[x].mat[1][3];
				mat[x].mat[3][2] = mat[x].mat[2][3];
				mat[x].mat[0][3] = 0;
				mat[x].mat[1][3] = 0;
				mat[x].mat[2][3] = 0;*/
				noMat4 transpose = mat[x].Transpose();
				frame[x] = ToMat4(transpose);
				//printfMat4(frame[x]);			
				
			}
			Animations[i].frames2.push_back(frame);				
		}

		SetFrame(0);
	}

	globalTime = oldGlobalTime;
}

void M2AnimationManager::extractFrames()
{


}


M2Animation::M2Animation()
{
	 timeStep = 1/60.f;    // $$ 30fps animation interpolation off the curves
}

M2Animation::~M2Animation()
{
	while(frames.size())
	{
		delete [] frames.back();
		frames.pop_back();
	}

	while(frames2.size())
	{
		delete [] frames2.back();
		frames2.pop_back();
	}
}

noMat4* M2Animation::GetFrameAt( float time )
{
	return frames[GetFrameIndexAt(time)];

}

noMat4* M2Animation::GetFrame( int index )
{
	assert(index < (int)frames.size());
	return frames[index];
}

int M2Animation::GetFrameIndexAt( float time )
{
	// get a [0.f ... 1.f) value by allowing the percent to wrap around 1
	float percent = time / duration;
	int percentINT = (int)percent;
	percent = percent - (float)percentINT;

	return (int)((float)frames.size() * percent);
}

mat4* M2Animation::GetFrameAt2( float time )
{
	int index =GetFrameIndexAt2(time);
	LOG_DEBUG << "Frame Index " << index;
	return frames2[index];
}

int M2Animation::GetFrameIndexAt2( float time )
{
	// get a [0.f ... 1.f) value by allowing the percent to wrap around 1
	float percent = time / duration;
	int percentINT = (int)percent;
	percent = percent - (float)percentINT;

	return (int)((float)frames2.size() * percent);
}

mat4* M2Animation::GetFrame2( int index )
{	
	return frames2[index];
}

