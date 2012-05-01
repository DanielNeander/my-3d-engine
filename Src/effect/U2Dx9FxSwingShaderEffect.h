/**************************************************************************************************
module	:	U2Dx9FxSwingShaderEffect
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_D3D9FX_SWING_EFFECT_H
#define U2_D3D9FX_SWING_EFFECT_H

#include "U2Dx9FxShaderEffect.h"

class U2_3D U2DX9FxSwingShaderEffect : public U2Dx9FxShaderEffect
{
public:
	U2DX9FxSwingShaderEffect(U2D3DXEffectShader* pShader, U2Dx9Renderer *pRenderer);
	virtual ~U2DX9FxSwingShaderEffect();

	virtual void LoadResoures(U2Dx9Renderer* pRenderer, U2Mesh* pMesh);
	virtual void ReleaseResources(U2Dx9Renderer* pRenderer, U2Mesh* pMesh);

	/// set the max swing angle
	inline void SetSwingAngle(float f) { m_fSwingAngle = f; }
	/// get the swing angle
	inline float GetSwingAngle() const { return m_fSwingAngle; }
	/// set the swing time period
	inline void SetSwingTime(float t) { m_fSwingTime = t; }
	/// get the swing time period
	inline float GetSwingTime() const { return m_fSwingTime; }

private:

	// compute a permuted swing angle
	float ComputeAngle(const D3DXVECTOR3& pos, float time) const;

	U2Variable::Handle m_hTimeVar;
	U2Variable::Handle m_hWindVar;
	float m_fSwingAngle;
	float m_fSwingTime;

};

#endif 