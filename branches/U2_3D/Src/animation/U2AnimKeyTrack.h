/**************************************************************************************************
module	:	U2AnimKeyTrack
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_ANIMKEYTRACK_H
#define U2_ANIMKEYTRACK_H

#include "U2AnimKeyData.h"

class U2AnimKeyTrack : public U2RefObject
{
public:
	U2AnimKeyTrack();

	void SetName(const U2DynString& name);
	const U2DynString& GetName() const;
	void SetNumKeyDatas(uint32 num);
	uint32 GetNumKeyDatas() const;

	void SetKeyData(uint32 idx, const U2AnimKeyData& keyData);

	void AddKeyData(const U2AnimKeyData& keyData);

	const U2AnimKeyData& GetKeyData(uint32 idx) const;

	U2PrimitiveVec<U2AnimKeyData*>& KeyDataArray();

private:
	U2DynString m_szName;
	U2PrimitiveVec<U2AnimKeyData*> m_keyDataArray;
};

typedef U2SmartPtr<U2AnimKeyTrack> U2AnimKeyTrackPtr;

#include "U2AnimKeyTrack.inl"

#endif
