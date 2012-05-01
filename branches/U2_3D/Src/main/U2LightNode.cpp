#include <u2_3d/src/U23DLibPCH.h>
#include "U2LightNode.h"

#include <U2_3d/Src/Dx9/U2Dx9Renderer.h>
#include <U2_3D/Src/Dx9/U2Dx9LightMgr.h>
#include <U2_3D/Src/main/U2SceneMgr.h>
#include <U2_3D/Src/shader/U2D3DXEffectShader.h>

IMPLEMENT_RTTI(U2LightNode, U2Node);

//-------------------------------------------------------------------------------------------------
U2LightNode::U2LightNode(U2Light* pLight/* =0 */)
	:m_spLight(pLight)
{
	if(m_spLight)
	{
		m_tmLocal.SetTrans(m_spLight->m_vLitPos);
		D3DXMATRIX rotMat;
		U2Math::MakeMatrix(rotMat, m_spLight->m_vLitDir, m_spLight->m_vLitUp, 
			m_spLight->m_vLitRight, true);
		m_tmLocal.SetRot(rotMat);
	}
}

//-------------------------------------------------------------------------------------------------
U2LightNode::~U2LightNode()
{

}

//-------------------------------------------------------------------------------------------------
void U2LightNode::SetLight(U2Light* pLight)
{
	m_spLight = pLight;

	if(m_spLight)
	{
		m_tmLocal.SetTrans(m_spLight->m_vLitPos);
		D3DXMATRIX rotMat;
		U2Math::MakeMatrix(rotMat, m_spLight->m_vLitRight, m_spLight->m_vLitUp, 
			m_spLight->m_vLitDir, true);
		m_tmLocal.SetRot(rotMat);
		Update();
	}
}

//-------------------------------------------------------------------------------------------------
void U2LightNode::UpdateWorldData(float fAppTime)
{
	U2Node::UpdateWorldData(fAppTime);

	if(m_spLight)
	{
		m_spLight->m_vLitPos = m_tmWorld.GetTrans();
		m_spLight->m_vLitRight= 
			D3DXVECTOR3(m_tmWorld.GetRot()._11, m_tmWorld.GetRot()._21, 
			m_tmWorld.GetRot()._31);
		m_spLight->m_vLitUp = 
			D3DXVECTOR3(m_tmWorld.GetRot()._12,m_tmWorld.GetRot()._22,
			m_tmWorld.GetRot()._32);
		m_spLight->m_vLitDir =
			D3DXVECTOR3(m_tmWorld.GetRot()._13,m_tmWorld.GetRot()._23,
			m_tmWorld.GetRot()._33);
	}

}

//-------------------------------------------------------------------------------------------------
const U2Light& U2LightNode::ApplyLight(const D3DXVECTOR4& shadowLitMask)
{
	m_spLight->m_fLitRange = m_shaderAttb.GetArg(U2FxShaderState::LightRange).GetFloat();
	m_spLight->m_diffuseColor = (D3DXCOLOR)
		m_shaderAttb.GetArg(U2FxShaderState::LightDiffuse).GetVector4();
	m_spLight->m_specularColor = (D3DXCOLOR)
		m_shaderAttb.GetArg(U2FxShaderState::LightSpecular).GetVector4();
	m_spLight->m_ambientColor = (D3DXCOLOR)
		m_shaderAttb.GetArg(U2FxShaderState::LightAmbient).GetVector4();
	m_spLight->m_shadowLightMask = shadowLitMask;

	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	pRenderer->GetLightMgr()->SetupD3D9Light(*this);
	return *m_spLight;
}

//-------------------------------------------------------------------------------------------------
const U2Light& U2LightNode::UpdateLight()
{
	U2SceneMgr* pSceneMgr = U2SceneMgr::Instance();
	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);


	D3DXMATRIX matHWorld;
	m_tmWorld.GetHomogeneousMat(matHWorld);
	pRenderer->SetTransform(U2Dx9Renderer::TMT_LIGHT, matHWorld);
	U2D3DXEffectShader* pShader = pRenderer->GetCurrEffectShader();
	if(m_spLight->m_eLitType == U2Light::LT_DIRECTIONAL)
	{
		// for directional lights, the light pos shader attributes
		// actually hold the light direction
		if(pShader->IsParamUsed(U2FxShaderState::ModelLightPos))
		{
			pShader->SetVector3(U2FxShaderState::ModelLightPos, 
				(float*)pRenderer->m_aTM[U2Dx9Renderer::TMT_INVWORLDLight].m[2]);
		}
	}
	else 
	{
		if(pShader->IsParamUsed(U2FxShaderState::ModelLightPos))
		{
			pShader->SetVector3(U2FxShaderState::ModelLightPos, 
				(float*)pRenderer->m_aTM[U2Dx9Renderer::TMT_INVWORLDLight].m[3]);
		}
	}
	return *m_spLight;
}

