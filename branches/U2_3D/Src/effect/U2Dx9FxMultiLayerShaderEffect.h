/**************************************************************************************************
module	:	U2Dx9FxShaderEffect
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_D3D9FX_MULTILAYER_EFFECT_H
#define U2_D3D9FX_MULTILAYER_EFFECT_H

#include "U2Dx9FxShaderEffect.h"
	

class U2_3D U2DX9FxMultiLayerShaderEffect : public U2Dx9FxShaderEffect
{
public:
	U2DX9FxMultiLayerShaderEffect(U2D3DXEffectShader* pShader, U2Dx9Renderer *pRenderer);
	virtual ~U2DX9FxMultiLayerShaderEffect();

	inline void SetUVStretch(int nr, float val)
	{
		U2ASSERT(nr >= 0 && nr < 6);
		m_afUVStretch[nr] = val;
	}
	inline void SetTextureCnt(int cnt) { m_textureCnt = cnt; }

	virtual void UpdateShader();

protected:
	float m_afUVStretch[6];
	float m_afDX7uvStretch[6];
	int m_textureCnt;
};

#endif
