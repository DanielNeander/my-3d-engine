/**************************************************************************************************
module	:	U2AnimClip
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_ANIMCLIP_H
#define U2_ANIMCLIP_H

#include "U2Animation.h"
#include "U2AnimKeyTrack.h"

class  U2AnimClip : public U2Object
{
public:
	U2AnimClip();

	U2AnimClip(const U2DynString& szClipname, int iAnimGroupIdx, int iNumInterpKeys);

	const U2DynString& GetClipname() const;

	int GetAnimGroupIdx() const;

	int GetNumInterpKeys() const;

	void SetNumAnimKeyTracks(uint32 num);
	uint32 GetNumAimKeyTracks() const;

	const U2AnimKeyTrack& GetAnimKeyTrack(int idx) const;
	
	U2FixedObjArray<U2AnimKeyTrack>& GetAnimKeyTracks();

private:
	U2DynString m_szClipname;
	int m_iAnimGroupIdx;
	int m_iNumInterpKeys;

	U2FixedObjArray<U2AnimKeyTrack> m_animKeyTracks;
};

typedef U2SmartPtr<U2AnimClip> U2AnimClipPtr;

#include "U2AnimClip.inl"

#endif