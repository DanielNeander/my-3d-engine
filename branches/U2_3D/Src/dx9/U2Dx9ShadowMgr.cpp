#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9ShadowMgr.h"
#include "U2ShadowTechnique.h"
#include "U2Dx9DynamicShadowCaster.h"
#include "U2Dx9StaticShadowCaster.h"
#include <U2_3D/src/main/U2SceneMgr.h>
#include <U2_3D/src/main/U2Culler.h>
#include "U2ShadowCommon.h"
#include <U2_3D/src/main/U2SphereBV.h>

U2Dx9ShadowMgr* U2Dx9ShadowMgr::ms_pSingleton = NULL;

//-------------------------------------------------------------------------------------------------
U2Dx9ShadowMgr::U2Dx9ShadowMgr(U2Camera* pSceneCam)
:m_bBeginShadow(false),
m_bBeginLit(false),
m_bUseDepthFail(true),
m_bShadowEnabled(true),
m_uiNumShaderPasses(0),
m_spShadowVolShader(0),
m_pCurLight(0),
m_spSceneCam(pSceneCam)
{
	ms_pSingleton = this;
}

//-------------------------------------------------------------------------------------------------
U2Dx9ShadowMgr::~U2Dx9ShadowMgr()
{
	Terminate();
	U2ASSERT(ms_pSingleton);
	ms_pSingleton = 0;
}

//-------------------------------------------------------------------------------------------------
U2Dx9ShadowCaster* U2Dx9ShadowMgr::CreateShadowCaster(U2Dx9ShadowCaster::ShadowType type, 
													  const TCHAR *szName)
{
	U2Dx9ShadowCaster* pNewShadowCaster = 0;

	switch(type)
	{
	case U2Dx9ShadowCaster::SHADOW_STATIC:
		pNewShadowCaster = U2_NEW U2Dx9StaticShadowCaster();
		break;
	case U2Dx9ShadowCaster::SHADOW_DYNAMIC:
		pNewShadowCaster = U2_NEW U2Dx9DynamicShadowCaster();
		break;
	}

	U2ASSERT(pNewShadowCaster);

	return pNewShadowCaster;
}

//-------------------------------------------------------------------------------------------------
bool U2Dx9ShadowMgr::Initialize()
{
	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);
	U2D3DXEffectShader* pShader = U2_NEW U2D3DXEffectShader(pRenderer);

	pShader->SetFilename(_T("shadow.fx"));

	if(!pShader->LoadResource())
	{
		U2ASSERT(FALSE);
		FDebug("Could not load shader file shadow.fx !\n");
		U2_DELETE pShader;
		pShader = NULL;
		return false;
	}

	m_spShadowVolShader = pShader;
	return true;
}

//-------------------------------------------------------------------------------------------------
void U2Dx9ShadowMgr::Terminate()
{
	m_spShadowVolShader = 0;
}

//-------------------------------------------------------------------------------------------------
bool U2Dx9ShadowMgr::BeginShadow()
{
	U2ASSERT(!m_bBeginShadow);

	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();

	D3DPRESENT_PARAMETERS d3dPP = 
		pRenderer->GetDefaultRenderTarget()->GetBuffer(0)->GetPresentParams();

	D3DFORMAT d3dFmt = d3dPP.AutoDepthStencilFormat;

	if(m_bShadowEnabled && (d3dFmt == D3DFMT_D24S8) || (d3dFmt == D3DFMT_D24X4S4))
	{

		if(m_bUseDepthFail)
		{
			m_spShadowVolShader->SetInt(U2FxShaderState::StencilFrontZFailOp, U2FxShaderState::DECR);
			m_spShadowVolShader->SetInt(U2FxShaderState::StencilFrontPassOp, U2FxShaderState::KEEP);
			m_spShadowVolShader->SetInt(U2FxShaderState::StencilFrontZFailOp, U2FxShaderState::INCR);
			m_spShadowVolShader->SetInt(U2FxShaderState::StencilBackPassOp, U2FxShaderState::KEEP);			
		}
		else 
		{
			m_spShadowVolShader->SetInt(U2FxShaderState::StencilFrontZFailOp, U2FxShaderState::KEEP);
			m_spShadowVolShader->SetInt(U2FxShaderState::StencilFrontPassOp, U2FxShaderState::INCR);
			m_spShadowVolShader->SetInt(U2FxShaderState::StencilBackZFailOp, U2FxShaderState::KEEP);
			m_spShadowVolShader->SetInt(U2FxShaderState::StencilBackPassOp, U2FxShaderState::DECR);
		}

		m_bBeginShadow = true;
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
void U2Dx9ShadowMgr::EndShadow()
{	
	U2ASSERT(m_bBeginShadow);
	U2ASSERT(!m_bBeginLit);
	m_bBeginShadow = false;
}

//-------------------------------------------------------------------------------------------------
void U2Dx9ShadowMgr::BeginLight(const U2Light &light)
{
	U2ASSERT(m_bBeginShadow);
	U2ASSERT(!m_bBeginLit);

	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);

	m_spShadowVolShader->SetInt(U2FxShaderState::LightType, light.m_eLitType);
	m_spShadowVolShader->SetFloat(U2FxShaderState::LightRange, light.m_fLitRange);
	pRenderer->SetCurrEffectShader(m_spShadowVolShader);
	m_uiNumShaderPasses = m_spShadowVolShader->Begin(true);
	if(1 == m_uiNumShaderPasses)
	{
		m_spShadowVolShader->BeginPass(0);
	}

	m_pCurLight = (U2Light*)&light;
	m_bBeginLit = true;
}

//-------------------------------------------------------------------------------------------------
void U2Dx9ShadowMgr::EndLight()
{
	U2ASSERT(m_bBeginShadow);
	U2ASSERT(m_bBeginLit);

	if(m_uiNumShaderPasses == 1)
	{
		m_spShadowVolShader->EndPass();
	}

	m_spShadowVolShader->End();
	m_bBeginLit = false;
}

//-------------------------------------------------------------------------------------------------
/**
Render a shadow caster with the current light.
*/
void U2Dx9ShadowMgr::RenderShadowCaster(U2Dx9ShadowCaster *pCaster, const D3DXMATRIX& worldMat)
{
	U2ASSERT(pCaster);

	U2SceneMgr* pSceneMgr = U2SceneMgr::Instance();

	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);
	
	
	pRenderer->SetTransform(U2Dx9Renderer::TMT_WORLD, worldMat);
	const D3DXMATRIX& invWorldMat = pRenderer->m_aTM[U2Dx9Renderer::TMT_INVWORLD];

	D3DXMATRIX litMat(
		m_pCurLight->m_vLitRight.x, m_pCurLight->m_vLitUp.x, m_pCurLight->m_vLitDir.x, 0.0f,
		m_pCurLight->m_vLitRight.y, m_pCurLight->m_vLitUp.y, m_pCurLight->m_vLitDir.y, 0.0f,
		m_pCurLight->m_vLitRight.z, m_pCurLight->m_vLitUp.z, m_pCurLight->m_vLitDir.z, 0.0f,
		m_pCurLight->m_vLitPos.x, m_pCurLight->m_vLitPos.y, m_pCurLight->m_vLitPos.z, 1.0f);

	D3DXMATRIX temp(0.23570225, 0.000000, -0.23570225, 0.000000,
		-0.1666666, 0.23477133, -0.166666, 0.000000,
		0.1666666, 0.23477133, 0.1666666, 0.000000,
		16.666665, 17.7777344, 16.66664, 1.0f);
		

	D3DXMATRIX invWorldLit = litMat * invWorldMat;

	invWorldLit = temp;
	if(m_pCurLight->m_eLitType == U2Light::LT_DIRECTIONAL)
	{
		m_spShadowVolShader->SetVector3(U2FxShaderState::ModelLightPos, 
			*((D3DXVECTOR3*)&invWorldLit.m[2]));
	}
	else 
	{
		m_spShadowVolShader->SetVector3(U2FxShaderState::ModelLightPos, 
			*((D3DXVECTOR3*)&invWorldLit.m[3]));
	}

	pCaster->SetupShadowVolume(*m_pCurLight, invWorldLit);

	if(1 == m_uiNumShaderPasses)
	{
		pCaster->RenderShadowVolume();
	}
	else 
	{
		uint32 pass;
		for(pass = 0; pass < m_uiNumShaderPasses; ++pass)
		{
			m_spShadowVolShader->BeginPass(pass);
			pCaster->RenderShadowVolume();
			m_spShadowVolShader->EndPass();
		}
	}
}

//-----------------------------------------------------------------------------
//  U2Dx9ShadowMgr::ComputeVirtualCameraParameters( )
//    computes the near & far clip planes for the virtual camera based on the
//    scene.
//
//    bounds the field of view for the virtual camera based on a swept-sphere/frustum
//    intersection.  if the swept sphere representing the extrusion of an object's bounding
//    sphere along the light direction intersects the view frustum, the object is added to
//    a list of interesting shadow casters.  the field of view is the minimum cone containing
//    all eligible bounding spheres.
//-----------------------------------------------------------------------------

void U2Dx9ShadowMgr::ComputeVirtualCameraParameters( )
{
	bool hit = false;

	//  frustum is in world space, so that bounding boxes are minimal size (xforming an AABB
	//  generally increases its volume).
	D3DXMATRIX modelView;
	D3DXMATRIX modelViewProjection;
	D3DXMatrixMultiply(&modelView, &m_World, &m_View);
	D3DXMatrixMultiply(&modelViewProjection, &modelView, &m_Projection);
	

	//U2Frustum sceneFrustum()

	m_ShadowCasterPoints.clear();
	m_aShadowCasters.RemoveAll();
	m_aShadowReceivers.RemoveAll();
	m_ShadowReceiverPoints.clear();

	Cull(m_spScene);

	//  add the biggest shadow receiver -- the ground!



	
}

void U2Dx9ShadowMgr::Cull(U2Spatial* pSpatial)
{
	
	bool hit = false;

	U2Node* pScene;
	if (pScene = DynamicCast<U2Node>(pSpatial))
	{	
		for(unsigned int i=0; i < pScene->GetChildCnt(); i++)
		{
			nMeshGroup* pChild = DynamicCast<nMeshGroup>(pScene->GetChild(i));
			if(!pChild)
				continue;		 

			int iFrustum = m_sceneCuller.TestAabb2(pChild->m_bbox);	

			switch (iFrustum)
			{
			case 0:	// fully inside frustum. so store large bounding box
				{
					hit = true;				
					m_ShadowReceiverPoints.push_back(pChild->m_bbox);
					m_aShadowCasters.AddElem(pChild);
					m_ShadowCasterPoints.push_back(pChild->m_bbox);
					m_aShadowReceivers.AddElem(pChild);
				}
				break;
			case 1:	// outside frustum -- test swept sphere for potential shadow caster
				{
					
					U2Sphere sphere(&pChild->m_bbox);
					U2SphereBV sphereBV(sphere);
					if (m_sceneCuller.TestSweptSphere(&sphereBV, &-m_lightDir))
					{
						hit = true;
						m_ShadowCasterPoints.push_back(pChild->m_bbox);
						m_aShadowCasters.AddElem(pChild);						
					}
					break;	
				}				

			case 2:
				//  big box intersects frustum.  test sub-boxen.  this improves shadow quality, since it allows
				//  a tighter bounding cone to be used.
				{
					//  only include objects in list once
					m_aShadowCasters.AddElem(pChild);
					m_aShadowReceivers.AddElem(pChild);		
					TestIntersectBox(m_spScene, hit);					
				}
			}
			
			if( pChild->IsExactly(U2Node::ms_rtti) )
				Cull(pChild);
		}
	}		

}

void U2Dx9ShadowMgr::TestIntersectBox(U2Spatial* pSpatial, bool& bHit)
{
	U2Node* pScene;
	if (pScene = DynamicCast<U2Node>(pSpatial))
	{	
		for(unsigned int i=0; i < pScene->GetChildCnt(); i++)
		{
			U2Spatial* pChild = pScene->GetChild(i);		

			if (0 != m_sceneCuller.TestAabb(pChild->m_bbox))
			{
				bHit = true;
				m_ShadowCasterPoints.push_back(pChild->m_bbox);
				m_ShadowReceiverPoints.push_back(pChild->m_bbox);
			}

			if( pChild->IsExactly(U2Node::ms_rtti) )
				TestIntersectBox(pChild, bHit);
		}
	}	
}


void U2Dx9ShadowMgr::RenderShadowMap()
{
	IsRendererOK;
	
	//  blast through cached shadow caster objects, and render them all
	
	//DrawScene

	//pRenderer->SetCameraData()

	m_pActiveShadowTec->UpdateShadowParams();
		// Set wrldVewProj, worldIT, textureMatrix

		//  blast through cached shadow caster objects, and render them all
	for (unsigned int i=0; i<m_aShadowCasters.Size(); i++)
	{
			
		nMeshGroup* pMesh = m_aShadowCasters.GetElem(i);
			
		if(pMesh)
		{
			m_pActiveShadowTec->BeginPass(*pMesh);
				//pMesh->Render(NULL);
			m_pActiveShadowTec->RenderShadowMap(pMesh);		

			m_pActiveShadowTec->EndPass();
		}				
	}

	m_pActiveShadowTec->DrawQuad();
}



