#include <U2_3D/src/U23DLibPCH.h>
#include "U2LightEffect.h"


U2LightEffect::U2LightEffect() : U2ShaderEffect(1)
{

}

U2LightEffect::~U2LightEffect()
{
	m_lights.RemoveAll();
}


void U2LightEffect::AttachLight(U2Light *pLight)
{
	U2ASSERT(pLight);

	for(uint32 i=0; i < m_lights.Size(); ++i)
	{
		if(m_lights.GetElem(i) == pLight)
			return;
	}

	m_lights.AddElem(pLight);
}


void U2LightEffect::DetachLight(U2Light *pLight)
{
	
	for(uint32 i=0; i < m_lights.Size(); ++i)
	{
		if(pLight == m_lights.GetElem(i))
		{
			m_lights.Remove(i);
			return;
		}		
	}	
}

void U2LightEffect::DetachAllLights()
{
	m_lights.RemoveAll();
}


void U2LightEffect::SetRenderState(int pass, U2Dx9RenderStateMgr* pRSMgr, 
							bool bPrimaryEffect)
{
	U2ShaderEffect::SetRenderState(pass, pRSMgr, bPrimaryEffect);	

	U2MaterialState* pMS = U2MaterialState::GetDefaultMaterial();
	{
		if(pass == 0)
		{
			m_saveEmissive = pMS->GetEmittance();
				
		}
		else 
			pMS->SetEmittance(COLOR_BLACK);
	}
		
	

	if(0 <= pass && pass < m_lights.Size())
	{
		U2Dx9Renderer::GetRenderer()->SetLight(0, m_lights.GetElem(pass));	
	}
}


void U2LightEffect::RestoreRenderState(int iPAss, U2Dx9RenderStateMgr* pRSMgr,
								bool bPrimaryEffect)
{
	U2ShaderEffect::RestoreRenderState(iPAss, pRSMgr, bPrimaryEffect);

	U2Dx9Renderer::GetRenderer()->SetLight(0, 0);

	if(iPAss == m_uPassCnt - 1)
	{
		U2MaterialState* pMS = U2MaterialState::GetDefaultMaterial();
		pMS->SetEmittance(m_saveEmissive);	
	}
}


void U2LightEffect::Configure()
{
	if(m_lights.Size() == 0)
	{
		SetPassCnt(1);
		m_vsArray.GetElem(0) = U2_NEW U2VertexShader(
			_T("v_Material.dx9.wmsp"));
		m_psArray.GetElem(0) = U2_NEW U2PixelShader(
			_T("p_PassThrough4.dx9.wmsp"));
		return;		
	}

	U2ObjVec<U2LightPtr> m_aBucket[4];
	uint32 litCnt = m_lights.Size();
	U2LightPtr spLight;
	uint32 i, type;
	for(i=0; i < litCnt; ++i)
	{
		spLight = m_lights.GetElem(i);
		type = spLight->m_eLitType;
		m_aBucket[type].AddElem(spLight);
	}

	SetPassCnt(litCnt);
	U2AlphaState* pAS;
	for(i=1; i < litCnt; ++i)
	{
		pAS = m_alphaStateArray.GetElem(i);
		pAS->SetAlphaBlending(true);
		pAS->SetSrcBlendMode(U2AlphaState::BLEND_ONE);
		pAS->SetDestBlendMode(U2AlphaState::BLEND_ONE);
	}

	const TCHAR acType[4] = {_T('a'), _T('d'), _T('p'), _T('s')};
	U2DynString shaderName(_T("v_L1$.dx9.wmsp"));
	int litIdx = 0;
	for(type=0; type < 4;type++)
	{
		for(i=0; i < m_aBucket[type].Size(); ++i)
		{
			shaderName.SetAt(4, acType[type]);
			m_vsArray.GetElem(litIdx) = U2_NEW U2VertexShader(shaderName);
			m_psArray.GetElem(litIdx) = U2_NEW U2PixelShader(
				_T("p_PassThrough4.dx9.wmsp"));
			litIdx++;
		}
	}
}
