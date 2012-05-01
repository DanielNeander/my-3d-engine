#include <U2_3D/src/U23DLibPCH.h>
#include <U2_3D/Src/Main/U2SceneMgr.h>
#include "U2Dx9LightMgr.h"
#include "U2Dx9Renderer.h"


U2AllocDeclareStatics(U2Light, sizeof(U2Light) * 8)

//-------------------------------------------------------------------------------------------------
U2Dx9LightMgr::U2Dx9LightMgr(U2Dx9RenderStateMgr *pRS, 
							 LPDIRECT3DDEVICE9 pD3DDev)
{
	//U2ASSERT(pRS);
	U2ASSERT(pD3DDev);

	m_pRS = pRS;
	m_pD3DDev = pD3DDev;
	m_pD3DDev->AddRef();

	InitLightRenderStates();		
}

//-------------------------------------------------------------------------------------------------
U2Dx9LightMgr::~U2Dx9LightMgr()
{
	/*U2ListNode<U2Light*>* pLightNode = m_lights.GetHeadNode();	
	
	while(pLightNode)
	{
		U2Light* pLight = pLightNode->m_elem;
		U2_DELETE pLight;		
		pLight = m_lights.GetNextElem(pLightNode);
	}
	*/	
	ReleaseLights();
	SAFE_RELEASE(m_pD3DDev);
}

//-------------------------------------------------------------------------------------------------
void U2Dx9LightMgr::InitLightRenderStates()
{
	if(!m_pRS)
		return;

	D3DCOLOR d3dAmbientColor = 0x00000000;
	m_pRS->SetRenderState(D3DRS_AMBIENT, d3dAmbientColor);
	m_pRS->SetRenderState(D3DRS_LIGHTING, FALSE);

	m_pRS->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, 
		D3DMCS_MATERIAL);
	m_pRS->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, 
		D3DMCS_MATERIAL);
	m_pRS->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, 
		D3DMCS_MATERIAL);
}

//-------------------------------------------------------------------------------------------------
void U2Dx9LightMgr::DisableLightRenderStates()
{
	if(LT_FIXED_FUNC == m_eLitType)
	{		
		unsigned int numMaxLights = U2Dx9Renderer::GetMaxActiveLights();
		
		for(uint32 i=0; i < numMaxLights; ++i)
		{
			HRESULT hr = m_pD3DDev->LightEnable(i, FALSE);
			U2ASSERT(SUCCEEDED(hr));
		}
	}
}


//-------------------------------------------------------------------------------------------------
void U2Dx9LightMgr::SetState(U2Dx9TextureState *pTex)
{

}

//-------------------------------------------------------------------------------------------------
bool U2Dx9LightMgr::IsLit() const 
{
	return m_pRS->GetRenderState(D3DRS_LIGHTING) != FALSE;
}

void U2Dx9LightMgr::RemoveLight(U2Light* light)
{
	m_lights.Remove(light);
}

void U2Dx9LightMgr::ReleaseLights()
{
	unsigned int numLits = m_lights.Size();	

	for (uint32 i = 0; i < numLits; i++)
	{
		m_pD3DDev->LightEnable(i, FALSE);
	}
	
	U2PoolShutdown(U2Light);
}


void U2Dx9LightMgr::SetupD3D9Light(U2LightNode& lightNode)
{
	U2SceneMgr *pSceneMgr = U2SceneMgr::Instance();
	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);

	U2Light* pLight = lightNode.GetLight();

	if(!pSceneMgr)
	{
		FDebug("U2SceneManager::Instance() is not initialized.");
		return ;
	}

	unsigned int numLits = m_lights.Size();	

	lightNode.UpdateWorldData(0.f);
	D3DXMATRIXA16 matHWorld;
	lightNode.m_tmWorld.GetHomogeneousMat(matHWorld);

	switch(m_eLitType)
	{
	case LT_OFF:
		pRenderer->SetTransform(U2Dx9Renderer::TMT_LIGHT, matHWorld);
		break;
	case LT_FIXED_FUNC:
		{

			HRESULT hr;

			if(numLits == 1)
			{
				DWORD d3dAmbient = D3DCOLOR_COLORVALUE(pLight->m_ambientColor.r, 
					pLight->m_ambientColor.g, pLight->m_ambientColor.b, pLight->m_ambientColor.a);
				m_pRS->SetRenderState(D3DRS_AMBIENT, d3dAmbient);
			}

			D3DLIGHT9 d3dLight9;
			memset(&d3dLight9, 0, sizeof(d3dLight9));
			switch(pLight->m_eLitType)
			{
			case U2Light::LT_POINT:
				d3dLight9.Type = D3DLIGHT_POINT;
				break;
			case U2Light::LT_DIRECTIONAL:
				d3dLight9.Type = D3DLIGHT_DIRECTIONAL;
				break;
			case U2Light::LT_SPOT:
				d3dLight9.Type = D3DLIGHT_SPOT;
				break;
			}

			d3dLight9.Diffuse.r		= pLight->m_diffuseColor.r;
			d3dLight9.Diffuse.g		= pLight->m_diffuseColor.g;
			d3dLight9.Diffuse.b		= pLight->m_diffuseColor.b;
			d3dLight9.Diffuse.a		= pLight->m_diffuseColor.a;

			d3dLight9.Specular.r	= pLight->m_specularColor.r;
			d3dLight9.Specular.g	= pLight->m_specularColor.g;
			d3dLight9.Specular.b	= pLight->m_specularColor.b;
			d3dLight9.Specular.a	= pLight->m_specularColor.a;

			d3dLight9.Ambient.r		= pLight->m_ambientColor.r;
			d3dLight9.Ambient.g		= pLight->m_ambientColor.g;
			d3dLight9.Ambient.b		= pLight->m_ambientColor.b;
			d3dLight9.Ambient.a		= pLight->m_ambientColor.a;

			d3dLight9.Position.x	= pLight->m_vLitPos.x;
			d3dLight9.Position.y	= pLight->m_vLitPos.y;
			d3dLight9.Position.z	= pLight->m_vLitPos.z;
			d3dLight9.Range			= pLight->m_fLitRange;
			d3dLight9.Falloff		= 1.0f;

			// set the attenuation values so that at the maximum range,
			// the light intensity is at 20%
			d3dLight9.Attenuation0	= 0.0f;
			d3dLight9.Attenuation1	= 5.0f / pLight->m_fLitRange;
			d3dLight9.Attenuation2	= 0.0f;
			d3dLight9.Theta			= 0.0f;
			d3dLight9.Phi			= U2_PI;

			hr = m_pD3DDev->SetLight(numLits - 1, &d3dLight9);
			U2ASSERT(SUCCEEDED(hr));
			hr = m_pD3DDev->LightEnable(numLits - 1, TRUE);
			U2ASSERT(SUCCEEDED(hr));
		}
		break;
	case LT_SHADER:
		{
			pRenderer->SetTransform(U2Dx9Renderer::TMT_LIGHT, matHWorld);
			U2D3DXEffectShader* pSharedShader = pRenderer->m_spSharedShader;

			if(pLight->m_eLitType == U2Light::LT_DIRECTIONAL)
			{
				// for directional lights, the light pos shader attributes
				// actually hold the light direction
				if(pSharedShader->IsParamUsed(U2FxShaderState::LightPos))
				{
					pSharedShader->SetVector3(U2FxShaderState::LightPos, 
						(float*)pRenderer->m_aTM[U2Dx9Renderer::TMT_LIGHT].m[3]);
				}
			}
			else 
			{
				//point light position
				if(pSharedShader->IsParamUsed(U2FxShaderState::LightPos))
				{
					pSharedShader->SetVector3(U2FxShaderState::LightPos, 
						(float*)pRenderer->m_aTM[U2Dx9Renderer::TMT_LIGHT].m[3]);
				}

			}
			if(pSharedShader->IsParamUsed(U2FxShaderState::LightType))
			{
				pSharedShader->SetInt(U2FxShaderState::LightType, 
					pLight->m_eLitType);
			}
			if(pSharedShader->IsParamUsed(U2FxShaderState::LightRange))
			{
				pSharedShader->SetFloat(U2FxShaderState::LightRange, 
					pLight->m_fLitRange);
			}
			if(pSharedShader->IsParamUsed(U2FxShaderState::LightDiffuse))
			{
				pSharedShader->SetVector4(U2FxShaderState::LightDiffuse,
					(D3DXVECTOR4)pLight->m_diffuseColor);
			}
			if(pSharedShader->IsParamUsed(U2FxShaderState::LightSpecular))
			{
				pSharedShader->SetVector4(U2FxShaderState::LightSpecular, 
					(D3DXVECTOR4)pLight->m_specularColor);
			}
			if(pSharedShader->IsParamUsed(U2FxShaderState::LightAmbient))
			{
				pSharedShader->SetVector4(U2FxShaderState::LightAmbient, 
					(D3DXVECTOR4)pLight->m_ambientColor);
			}
			if(pSharedShader->IsParamUsed(U2FxShaderState::ShadowIndex))
			{
				pSharedShader->SetVector4(U2FxShaderState::ShadowIndex, 
					pLight->m_shadowLightMask);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
unsigned int U2Dx9LightMgr::AddLight(U2LightNode& lightNode)
{
	

	U2Light* pLight = lightNode.GetLight();
	m_lights.InsertToHead(pLight);

	unsigned int numLits = m_lights.Size();	
	return numLits;
}

//-------------------------------------------------------------------------------------------------
void U2Dx9LightMgr::SetLightingType(LightingType type)
{
	m_eLitType = type;
}

//-------------------------------------------------------------------------------------------------
U2Dx9LightMgr::LightingType U2Dx9LightMgr::GetLightingType() const
{
	return m_eLitType;
}
