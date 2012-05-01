
/**************************************************************************************************
module	:	U2SkinCharSet
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_SKINCHARSET_H
#define U2_SKINCHARSET_H

class U2SkinCharSet : public U2Object
{
public:
	U2SkinCharSet();
	virtual ~U2SkinCharSet();

	void SetFadeInTime(float fTime);
	
	float GetFadeInTime() const;

	void AddClip(const U2DynString& clipName, float clipWeight);

	void RemoveClip(const U2DynString& clipName);

	void SetClipWeight(int idx, float clipWeight);

	int GetClipIdx(const U2DynString& clipName) const;

	const U2DynString& GetClipname(int idx) const;

	float GetClipWeight(int idx) const;

	int GetNumClips() const;

	void ClearClips(); 

	bool IsDirty() const;

	void SetDirty(bool b);

protected:
	bool m_bDirty;
	float m_fFadeInTime;

	
	U2PrimitiveVec<U2DynString> m_clipNames;
	U2PrimitiveVec<float> m_clipWeights;
};

#include "U2SkinCharSet.inl"


#endif