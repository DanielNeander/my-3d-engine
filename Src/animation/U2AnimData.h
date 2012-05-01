/**************************************************************************************************
module	:	U2AnimData
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_ANIMDATA_H
#define U2_ANIMDATA_H

#include "U2Skeleton.h"
#include "U2AnimStateInfo.h"


U2SmartPointer(U2SkinController);
class U2AnimKeyTrack;
class U2AnimKey;

class U2AnimData : public U2RefObject
{
public:
	U2AnimData();
	U2AnimData(const U2AnimData& src);

	virtual ~U2AnimData();

	U2Skeleton& GetSkeleton();

	void SetAnim(U2Animation* pAnim);
	const U2AnimationPtr& GetAnim() const;

	void SetSkinController(U2SkinController* pCtrl);
	U2SkinController* GetSkinController() const;

	void SetActivateState(const U2AnimStateInfo& newState);

	const U2AnimStateInfo& GetActiveState() const;

	void EvaluateSkeleton(float fTime);

	void EmitAnimEvents(float fStartTime, float fStopTime);

	void SetAnimEnabled(bool b);
	bool IsAnimEnabled() const;

	void SetLastEvalFrameId(uint32 id);
	uint32 GetLastEvalFrameId() const;

private:
	bool Sample(const U2AnimStateInfo& info, float fTime, 
		D3DXVECTOR4* pKeyArray, D3DXVECTOR4* pScratchKeyArray, int keyArraySize);

	void EmitAnimEvents(const U2AnimStateInfo& info, float fStartTime, float fEndTime);

	void BeginEmitEvents();

	void AddEmitEvent(const U2AnimKeyTrack& track, const U2AnimKeyData& eventKey, float fWeight);

	void EndEmitEvents();

	enum 
	{
		MAX_JOINTS = 1024,
		MAX_INTERPKEYS = MAX_JOINTS * 3,
	};

	U2Skeleton* m_pSkel;
	U2AnimationPtr m_spAnim;
	U2SkinControllerPtr m_spSkinCtrl;

	U2AnimStateInfo* m_pPrevStateInfo;
	U2AnimStateInfo* m_pCurStateInfo;

	static U2ObjVec<U2AnimKeyTrackPtr> ms_resultAnimKeyTacks;
	static D3DXVECTOR4 ms_scratchKeys[MAX_INTERPKEYS];
	static D3DXVECTOR4 ms_keys[MAX_INTERPKEYS];
	static D3DXVECTOR4 ms_trasnsitionKeys[MAX_INTERPKEYS];

	bool m_bAnimEnabled;
	uint32 m_uiLastEvalFrameId;
};

typedef U2SmartPtr<U2AnimData> U2AnimDataPtr;

#include "U2AnimData.inl"

#endif