/**************************************************************************************************
module	:	U2Joint
Author	:	Yun sangyong
Desc	:	2010-03-08
			Nebula 2의 애니메이션은 기본적으로 Right Handed 좌표계이다. 
			현재 엔진의 좌표계는 Light Handed 좌표계만 지원한다. 그러므로 
			이 상태에서 애니메이션을 플레이하면 좌표계의 차이로 화상이 왜곡되는 현상이 발생한다. 
			1)해결하는 방법은 좌표계를 Right Handed 체계로 변경하거나
			2)...가 있다. 											
***************************************************************************************************/
#pragma once
#ifndef U2_JOINT_H
#define U2_JOINT_H

#include <u2_3d/src/main/U2Object.h>
#include <d3dx9math.h>

class U2Bone : public U2Object 
{
public:
	U2Bone();
	~U2Bone();

	void SetParentJointIdx(int idx);
	
	int GetParentJointIdx() const;

	void SetParentJoint(U2Bone* pParent);

	U2Bone* GetParentJoint() const;

	void SetPose(const D3DXVECTOR3& trans, const D3DXQUATERNION& quat, const D3DXVECTOR3& scale);

	const D3DXVECTOR3& GetPoseTrans() const;
	const D3DXQUATERNION& GetPoseRot() const;
	const D3DXVECTOR3& GetPoseScale() const;

	void SetTrans(const D3DXVECTOR3& trans);
	const D3DXVECTOR3& GetTrans() const;

	void SetRot(D3DXQUATERNION quat);
	const D3DXQUATERNION& GetRot() const;

	void SetScale(const D3DXVECTOR3& scale);

	const D3DXVECTOR3& GetScale() const;

	void Evaluate();

	void SetLocalMat(const D3DXMATRIX& mat);
	
	const D3DXMATRIX& GetPoseMat() const;
	
	const D3DXMATRIX& GetInvPoseMat() const;

	void SetMat(const D3DXMATRIX& mat);

	const D3DXMATRIX& GetMat() const;

	const D3DXMATRIX& GetSKinMat44() const;

	const D3DXMATRIX& GetSkinMat33() const;

	void ClearUpTodateFlag();

	bool IsUpTodate() const;

private:
	D3DXVECTOR3 m_poseTrans;
	D3DXQUATERNION m_poseRot;
	D3DXVECTOR3 m_poseScale;

	D3DXVECTOR3 m_trans;
	D3DXQUATERNION m_quatRot;
	D3DXVECTOR3 m_scale;
	D3DXVECTOR3 m_variationScale;

	D3DXMATRIX m_poseMat;
	D3DXMATRIX m_invPoseMat;

	D3DXMATRIX m_localUnscaledMat;
	D3DXMATRIX m_localScaledMat;
	D3DXMATRIX m_worldUnscaledMat;
	D3DXMATRIX m_worldScaledMat;
	D3DXMATRIX m_skinMat44;
	D3DXMATRIX m_skinMat33;		// translation (0.f, 0.f, 0.f)

	int m_iParentJointIdx;
	U2Bone* m_pParentJoint;

	bool m_bMatDirty;
	bool m_bLockMat;
	bool m_bIsUpTodate;


};

#include "U2Bone.inl"

#endif

