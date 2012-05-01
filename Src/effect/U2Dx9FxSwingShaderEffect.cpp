#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9FxSwingShaderEffect.h"
#include <U2_3D/Src/Main/U2SceneMgr.h>

U2DX9FxSwingShaderEffect::U2DX9FxSwingShaderEffect(
	U2D3DXEffectShader* pShader, U2Dx9Renderer *pRenderer)
	:U2Dx9FxShaderEffect(pShader, pRenderer),
	m_hTimeVar(U2Variable::InvalidHandle),
	m_hWindVar(U2Variable::InvalidHandle),
	m_fSwingAngle(45.0f),
	m_fSwingTime(5.0f)
{

}

U2DX9FxSwingShaderEffect::~U2DX9FxSwingShaderEffect()
{

}

void U2DX9FxSwingShaderEffect::LoadResoures(U2Dx9Renderer* pRenderer, U2Mesh* pMesh)
{
	U2Dx9FxShaderEffect::LoadResoures(pRenderer, pMesh);

	//pMesh->SetVertexUsage(U2N2Mesh::WriteOnce | U2N2Mesh::NeedsVertexShader);

	m_hTimeVar = U2VariableMgr::Instance()->GetVariableHandleByName("time");
	m_hWindVar = U2VariableMgr::Instance()->GetVariableHandleByName("wind");
}

void U2DX9FxSwingShaderEffect::ReleaseResources(U2Dx9Renderer* pRenderer, U2Mesh* pMesh)
{
	m_hTimeVar = U2Variable::InvalidHandle;
	m_hWindVar = U2Variable::InvalidHandle;

	U2Dx9FxShaderEffect::ReleaseResources(pRenderer, pMesh);
}

float U2DX9FxSwingShaderEffect::ComputeAngle(const D3DXVECTOR3 &pos, float time) const
{
	// add position offset to time to prevent that all trees swing in sync
	time += pos.x + pos.y + pos.z;

	// sinus wave swing value (between +1 and -1)
	float swing = (float) sin((time * DegToRad(360.0f)) / this->m_fSwingTime);

	// get a wind strength "swinging" angle, we want no swinging at
	// min and max wind strength, and max swinging at 0.5 wind strength
	return this->m_fSwingAngle * 0.3f + (this->m_fSwingAngle * swing * 0.7f);

}