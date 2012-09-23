/**************************************************************************************************
module	:	U2SkinController
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_SKINCONTROLLER_H
#define U2_SKINCONTROLLER_H

#include "U2Controller.h"
#include "U2AnimData.h"
#include "U2SkinCharSet.h"

U2SmartPointer(U2Animation);
U2SmartPointer(U2AnimClip);
U2SmartPointer(U2SkinController);

class U2SkinModifier;

class U2_3D U2SkinController : public U2Controller
{
	DECLARE_RTTI;
public:
	U2SkinController();
	virtual ~U2SkinController();

	//virtual bool LoadResources();
	//virtual void UnloadResources();
	virtual void Initialize(U2RenderContext* pCxt);
	virtual void Terminate(U2RenderContext* pCxt);
	
	virtual bool Update(float fTime, U2RenderContext* pCxt);

	void BeginJoints(uint32 numJoints);

	void SetJoint(int idx, int parentJointIdx, const D3DXVECTOR3& poseTrans,
		const D3DXQUATERNION& poseRotate, const D3DXVECTOR3& poseScale, const U2DynString& name);

	void EndJoints();

	uint32 GetNumJoints() const;

	void GetJoint(int idx, int& parentJointIdx, D3DXVECTOR3& poseTrans,
		D3DXQUATERNION& poseRotate, D3DXVECTOR3& poseScale, U2DynString& name);

	void SetAnim(const U2DynString& szFilename);

	const U2DynString& GetAnim() const;

	void BeginClips(int numClips);

	void SetClip(int clipIDx, int animGroupIdx, const U2DynString& szClipname);	


	void EndClips();

	int GetNumClips() const;

	const U2AnimClip& GetClip(int clipIdx) const;

	int GetClipIdx(const U2DynString& name) const;

	double GetClipDuration(int idx) const;


	void BeginAnimKeyTracks(int clipIdx, int numTracks);

	void BeginAnimKeyTracks(int clipIdx, int trackIdx, const U2DynString& name, int numKeys);

	void SetAnimKey(int clipIdx, int trackIdx, int keyIDx, float time, 
		const D3DXVECTOR3& trans, const D3DXQUATERNION& rot, const D3DXVECTOR3& scale);

	void EndAnimKeyTrack(int clipIdx, int trackIdx);

	void EndAnimKeyTracks(int clipIdx);

	void SetAnimEnabled(bool b);

	bool IsAnimEnabled() const;

	int GetAnimSetIdxHandle() const;
	int GetAnimVarIdxHandle() const;

	

	int GetJoint(const TCHAR* szJointname);

protected:
	
	bool LoadAnim();

	void UnloadAnim();

	U2AnimData* m_pAnimData;	
	U2Animation* m_pAnim;	
	U2DynString m_szAnimname;
	U2ObjVec<U2AnimClipPtr> m_clips;
	bool m_bAnimEnabled;
	uint32 m_uFrameId;

	int m_iAnimVarIdx;
	int m_iAnimSetIdx;

};

#include "U2SkinController.inl"


#endif
