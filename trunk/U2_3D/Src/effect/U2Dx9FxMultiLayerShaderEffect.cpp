#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9FxMultiLayerShaderEffect.h"
#include <U2_3D/Src/Main/U2SceneMgr.h>

U2DX9FxMultiLayerShaderEffect::U2DX9FxMultiLayerShaderEffect(
	U2D3DXEffectShader* pShader, U2Dx9Renderer *pRenderer)
	:U2Dx9FxShaderEffect(pShader, pRenderer)
{

}

U2DX9FxMultiLayerShaderEffect::~U2DX9FxMultiLayerShaderEffect()
{

}

void U2DX9FxMultiLayerShaderEffect::UpdateShader()
{
	D3DXMATRIX uvStretchMatrix;
	for (int i = 0; i < this->m_textureCnt; i++)
		uvStretchMatrix.m[i/4][i%4] = this->m_afUVStretch[i];


	U2D3DXEffectShader* shd = this->GetEffectShader();
	if (shd->IsParamUsed(U2FxShaderState::MLPUVStretch))
	{
		shd->SetMatrix(U2FxShaderState::MLPUVStretch, uvStretchMatrix);
	}
}


