/**************************************************************************************************
module	:	U2AnimStateArray
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_ANIMSTATEARRAY_H
#define U2_ANIMSTATEARRAY_H

#include "U2AnimStateInfo.h"

class U2AnimStateArray : public U2MemObj
{
public:
	U2AnimStateArray();

	~U2AnimStateArray();

	void Begin(uint32 num);

	void SetState(int idx, const U2AnimStateInfo& state);

	void End();

	uint32 GetNumStates() const;

	U2AnimStateInfo& GetAnimStateInfo(int idx) const;

	U2AnimStateInfo* FindAnimStateInfo(const U2DynString& str) const;

	int FindStateIdx(const U2DynString& str) const;

private:
	U2PrimitiveVec<U2AnimStateInfo*> m_stateArray;

};

#include "U2AnimStateArray.inl"

#endif