/**************************************************************************************************
module	:	U2AnimStateInfo
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_ANIMSTATEINFO_H
#define U2_ANIMSTATEINFO_H

#include "U2AnimClip.h"

class U2AnimStateInfo : public U2Object
{
public:
	U2AnimStateInfo();

	void SetFadeInTime(float fTime);

	float GetFadeInTime() const;

	void SetStateStarted(float fTime);

	float GetStateStarted() const;

	void SetStateOffset(float i);

	float GetStateOffset() const;

	 bool IsValid() const;

	 void BeginClips(int num);
	 
	 void SetClip(int idx, const U2AnimClip& clip, float weight);

	 void EndClips();

	 int GetNumClips() const;

	 U2AnimClip& GetClip(int idx) const;
	 float GetClipWeight(int idx) const;

	 U2PrimitiveVec<U2AnimClip*> m_clipArray;
	 U2PrimitiveVec<float> m_clipWeights;
	 float m_fFadeInTime;
	 float m_fStateStarted;
	 float m_fStateOffset;
};

#include "U2AnimStateInfo.inl"


#endif