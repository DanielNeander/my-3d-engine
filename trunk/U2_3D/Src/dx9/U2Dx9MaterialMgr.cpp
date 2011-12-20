#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9MaterialMgr.h"



U2Dx9MaterialMgr::MaterialEntry::MaterialEntry(const U2MaterialState& mat)
{
	m_uRevID = 0;

	bool bUpdated = Update(mat);
	U2ASSERT(bUpdated);
}


bool U2Dx9MaterialMgr::MaterialEntry::Update(const U2MaterialState& mat)
{
	uint32 uRevID = mat.GetRevisionID();

	if(m_uRevID != uRevID)
	{
		m_uRevID = uRevID;

		const D3DXCOLOR& emissive = mat.GetEmittance();
		const D3DXCOLOR& ambient = mat.GetAmbientColor();
		const D3DXCOLOR& diffuse = mat.GetDiffuseColor();
		const D3DXCOLOR& specular = mat.GetSpecularColor();
		float fAlpha = mat.GetAlpha();

		m_d3dMat.Emissive.r = emissive.r;
		m_d3dMat.Emissive.g = emissive.g;
		m_d3dMat.Emissive.b = emissive.b;
		m_d3dMat.Emissive.a = fAlpha;
		m_d3dMat.Ambient.r = ambient.r;
		m_d3dMat.Ambient.g = ambient.g;
		m_d3dMat.Ambient.b = ambient.b;
		m_d3dMat.Ambient.a = fAlpha;
		m_d3dMat.Diffuse.r = diffuse.r;
		m_d3dMat.Diffuse.g = diffuse.g;
		m_d3dMat.Diffuse.b = diffuse.b;
		m_d3dMat.Diffuse.a = fAlpha;
		m_d3dMat.Specular.r = specular.r;
		m_d3dMat.Specular.g = specular.g;
		m_d3dMat.Specular.b = specular.b;
		m_d3dMat.Specular.a = fAlpha;

		m_d3dMat.Power = mat.GetShineness();

		return true;

	}
	return false;
}


U2Dx9MaterialMgr::U2Dx9MaterialMgr()
{

}


U2Dx9MaterialMgr::~U2Dx9MaterialMgr()
{


}


D3DMATERIAL9* U2Dx9MaterialMgr::GetCurrMaterial()
{
	return m_pCurrMat->GetD3DMat();
}

void U2Dx9MaterialMgr::SetCurrMaterial(U2MaterialState& mat)
{
	MaterialEntry* pNewEntry = (MaterialEntry*)mat.GetRendererData();

	if(!pNewEntry)
	{
		pNewEntry = U2_NEW MaterialEntry(mat);
		mat.SetRendererData(pNewEntry);
		m_bChanged = true;
	}

	U2ASSERT(pNewEntry);

	if(pNewEntry->Update(mat))
		m_bChanged = true;
	if(pNewEntry != m_pCurrMat)
	{
		D3DMATERIAL9*pMat = pNewEntry->GetD3DMat();
		m_bChanged = true;
		m_pCurrMat = pNewEntry;
	}
	
}

void U2Dx9MaterialMgr::UpdateToDevice(LPDIRECT3DDEVICE9 pD3DDev)
{
	if(m_bChanged)
	{
		U2ASSERT(m_pCurrMat);
		m_bChanged = false;
		pD3DDev->SetMaterial(m_pCurrMat->GetD3DMat());
	}

}


void U2Dx9MaterialMgr::DestoryMaterialData(U2MaterialState* pMat)
{
	if(pMat)
	{
		MaterialEntry* pEntry = 
			(MaterialEntry*)pMat->GetRendererData();
		if(pEntry)
		{
			pMat->SetRendererData(NULL);
			if(pEntry == m_pCurrMat)
				m_pCurrMat = NULL;

			U2_DELETE pEntry;
		}
	}
	
}

void U2Dx9MaterialMgr::Init()
{
	m_pCurrMat = NULL;	
	SetCurrMaterial(*SmartPtrCast(U2MaterialState, U2RenderState::
		ms_aspDefault[U2RenderState::MATERIAL]));
	m_bChanged = true;
}