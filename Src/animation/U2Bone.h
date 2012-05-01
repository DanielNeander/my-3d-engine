/**************************************************************************************************
module	:	U2Joint
Author	:	Yun sangyong
Desc	:	2010-03-08
			Nebula 2�� �ִϸ��̼��� �⺻������ Right Handed ��ǥ���̴�. 
			���� ������ ��ǥ��� Light Handed ��ǥ�踸 �����Ѵ�. �׷��Ƿ� 
			�� ���¿��� �ִϸ��̼��� �÷����ϸ� ��ǥ���� ���̷� ȭ���� �ְ�Ǵ� ������ �߻��Ѵ�. 
			1)�ذ��ϴ� ����� ��ǥ�踦 Right Handed ü��� �����ϰų�
			2)...�� �ִ�. 											
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

