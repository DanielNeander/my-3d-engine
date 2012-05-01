/**************************************************************************************************
module	:	U2AnimKeyData
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_ANIMKEYDATA_H
#define U2_ANIMKEYDATA_H


class U2AnimKeyData : public U2MemObj
{
public:
	U2AnimKeyData();
	void SetTime(float fTime);
	float GetTime() const;

	void SetTrans(const D3DXVECTOR3& t);
	const D3DXVECTOR3& GetTrans() const;

	void SetQuatRot(const D3DXQUATERNION& q);
	const D3DXQUATERNION& GetQuatRot() const;

	void SetScale(const D3DXVECTOR3& s);
	const D3DXVECTOR3& GetScale() const;

	D3DXMATRIX GetMatrix() const;
	void SetAccumWeight(float weight);
	float GetAccumWeight() const;

private:
	float m_fTime;
	D3DXVECTOR3 m_trans;
	D3DXQUATERNION m_quatRot;
	D3DXVECTOR3 m_scale;
	float m_fAccumWeight;	
};

#include "U2AnimKeyData.inl"

#endif 


