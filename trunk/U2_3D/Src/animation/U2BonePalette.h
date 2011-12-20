/**************************************************************************************************
module	:	U2BonePalette
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_BONEPALETTE_H
#define U2_BONEPALETTE_H


class U2BonePalette : public U2Object 
{
public:
	U2BonePalette();

	void BeginJoints(int num);

	void SetJointIdx(int paletteIdx, int jointIdx);

	void EndJoints();

	int GetNumJoints() const;

	int GetJointIdx(int paletteIdx) const;

private:
	U2PrimitiveVec<int> m_joindIdxArray;
};

#include "U2BonePalette.inl"

#endif
