#include <U2_3D/Src/U23DLibPCH.h>
#include "U2Dx9FramePass.h"
#include <U2_3D/Src/Main/U2SceneMgr.h>

//-------------------------------------------------------------------------------------------------
U2FramePass::U2FramePass()
:m_pOwnerFrame(0),
m_bFrameInBegin(false),
m_uiFrameShaderIdx(-1),
m_uiClearFlags(0),
m_eShadowTeq(NO_SHADOW),
m_bOcclusionQuery(false),
m_bDrawFullscreenQuad(false),
m_bDrawGui(false),
m_bShadowEnabled(true),
m_renderTargetNames(U2Dx9Renderer::GetRenderer()->GetMaxRenderTargets()),
m_spRenderTarget(0),
m_currRTWidth(0),
m_currRTHeight(0),
m_bDepthStencil(false),
m_bgColor(D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f)),
m_fClearDepth(1.0f),
m_iClearStencil(0)
{


}

//-------------------------------------------------------------------------------------------------
U2FramePass::~U2FramePass()
{

}

//-------------------------------------------------------------------------------------------------
void U2FramePass::Validate()
{
	uint32 i;
	uint32 numPhases = m_framePhases.FilledSize();
	for(i=0; i < numPhases; ++i)
	{
		m_framePhases[i]->SetFrame(m_pOwnerFrame);
		m_framePhases[i]->Validate();
	}

#if 1
	if(!m_spRenderTarget)
	{
		U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
		U2RenderTargets* pDefaultRT = pRenderer->GetDefaultRenderTarget();
		//U2Dx9FrameBuffer *pBackBuffer = pDefaultRT->GetBuffer(0);
		//U2ASSERT(pBackBuffer);

		// 임시 코드...
		uint32 numBuffers = 0;
		for(unsigned int i=0; i < m_renderTargetNames.Size(); ++i)
		{
			if(!m_renderTargetNames[i].IsEmpty())
			{
				++numBuffers;
			}
		}

		m_spRenderTarget = U2RenderTargets::Create(numBuffers, pRenderer);
		U2ASSERT(m_spRenderTarget);

		
		unsigned int width, height;
		
		for(unsigned int i=0; i < m_renderTargetNames.Size(); ++i)
		{
			if(!m_renderTargetNames[i].IsEmpty())
			{
				// Bug... frame에 저장된 렌더 타켓과 같은 것으로 설정..							
				unsigned int rtIdx = m_pOwnerFrame->FindRenderTargetIdx(m_renderTargetNames[i]);
				U2FrameRenderTargetPtr rt= m_pOwnerFrame->GetRenderTarget(rtIdx);
				width = rt->GetWidth(); height = rt->GetHeight();
				m_spRenderTarget->AttachBackBuffer(rt->GetTexture()->m_sp2DBuffer, i);
			}
		}

		// Compatiable with Default Depth Stencil Buffer
		if (m_bDepthStencil)
		{
			D3DFORMAT zFormat = D3DFMT_D24S8;

			U2Dx9FrameBuffer* pDSBuffer = U2Dx9FrameBuffer::CreateAddedDepthStencilBuffer
				(width, height, pRenderer->GetD3DDevice(), zFormat);
			m_spRenderTarget->AttachDSBuffer(pDSBuffer);
		}
		//else 
		//	m_spRenderTarget->AttachDSBuffer(pDefaultRT->GetDepthStencilBuffer());
	}
#endif

	if(((uint32)-1 == m_uiFrameShaderIdx )&& (!m_szShaderAlias.IsEmpty()))
	{
		m_uiFrameShaderIdx = m_pOwnerFrame->FindFrameShaderIdx(m_szShaderAlias);
		if((uint32)-1 == m_uiFrameShaderIdx)
		{
			U2ASSERT(false);
			FDebug("U2Dx9FramePass::Validate: couldn't find shader alias '%s'",
				m_szShaderAlias.Str());
			return;
		}
	}	
}


//-------------------------------------------------------------------------------------------------
void U2FramePass::CreateScreenQuad()
{
	if(!m_spScreenQuad)
	{
		// Create ScreenQuad		
		U2TriListData* pData = U2_NEW U2TriListData();


		m_spScreenQuad = U2_NEW U2N2Mesh(pData);
		m_spScreenQuad->SetUsages(U2N2Mesh::WriteOnly);
		m_spScreenQuad->SetVertexComponents(U2N2Mesh::Coord | U2N2Mesh::Uv0);
		pData->SetVertexCount(4, 4);
		// Triangle List
		pData->SetIndices(2, 2, 0, 0, 1);

		m_spScreenQuad->CreateEmptyBuffers();

		UpdateMeshCoords();
	}		
}

void U2FramePass::RecreateScreenQuad()
{
	CreateScreenQuad();
}


//-------------------------------------------------------------------------------------------------
void U2FramePass::UpdateMeshCoords()
{
	// Update Screen Quad
	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	int w, h;
	//w = pRenderer->GetCurrRenderTargets()->GetBuffer(0)->GetWidth();
	//h = pRenderer->GetCurrRenderTargets()->GetBuffer(0)->GetHeight();

	// RenderTarget은 하나만 존재한다는 가정..
	if (this->GetRenderTargetName(0).IsEmpty())
	{
		w = pRenderer->GetDefaultRenderTarget()->GetBuffer(0)->GetWidth();
		h = pRenderer->GetDefaultRenderTarget()->GetBuffer(0)->GetHeight();
	}
	else 
	{
		U2FrameRenderTargetPtr RT = m_pOwnerFrame->GetRenderTarget(
			m_pOwnerFrame->FindRenderTargetIdx(this->GetRenderTargetName(0).Str()));

		w = pRenderer->GetDefaultRenderTarget()->GetBuffer(0)->GetWidth() * 
			RT->GetRelativeSize();
		h = pRenderer->GetDefaultRenderTarget()->GetBuffer(0)->GetHeight() * 
			RT->GetRelativeSize();
	}
	

	//if(m_currRTWidth != w || m_currRTHeight != h)
	{
		D3DXVECTOR2 pixelSize(1.0f/ float(w), 1.0f / float(h));
		D3DXVECTOR2 halfPixelSize = pixelSize * 0.5f;

		float x0 = -1.0f;
		float x1 = +1.0f - pixelSize.x;
		float y0 = -1.0f + pixelSize.y;
		float y1 = +1.0f;

		float u0 = 0.0f + halfPixelSize.x;
		float u1 = 1.0f - halfPixelSize.x;
		float v0 = 0.0f + halfPixelSize.y;
		float v1 = 1.0f - halfPixelSize.y;

		float *vPtr = m_spScreenQuad->LockVertices();
		U2ASSERT(vPtr);

		// 0 ---- 2
		// |		 |
		// |		 |
		// 1 ---- 3

		*vPtr++ = x0; *vPtr++ = y1; *vPtr++ = 0.0f; *vPtr++ = u0; *vPtr++ = v0;
		*vPtr++ = x0; *vPtr++ = y0; *vPtr++ = 0.0f; *vPtr++ = u0; *vPtr++ = v1;
		*vPtr++ = x1; *vPtr++ = y1; *vPtr++ = 0.0f; *vPtr++ = u1; *vPtr++ = v0;
		*vPtr++ = x1; *vPtr++ = y0; *vPtr++ = 0.0f; *vPtr++ = u1; *vPtr++ = v1;

		m_spScreenQuad->UnlockVertices();
				

		uint16* iPtr = m_spScreenQuad->LockIndices();
		U2ASSERT(iPtr);

		*iPtr++ = 0; *iPtr++ = 1; *iPtr++ = 2;
		*iPtr++ = 1; *iPtr++ = 3; *iPtr++ = 2;

		m_spScreenQuad->UnlockIndices();	
		
	//	m_currRTWidth = w;
	//	m_currRTWidth = h;
	}
}

//-------------------------------------------------------------------------------------------------
unsigned int U2FramePass::Begin()
{
	
	// Only Render this pass if shadowing is enabled?
	//if(m_bShadowEnabled && check device state)
	//{
	//	return 0;
	//}

	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);

	
	U2SceneMgr *pSceneMgr = U2SceneMgr::Instance();

	if(!pSceneMgr->BeginScene(pRenderer->GetViewMat()))
		return 0;	

	//2010/08/25 Fixed
	pRenderer->SetBackgroundColor(m_bgColor);
	pRenderer->SetDepthClear(this->m_fClearDepth);
	pRenderer->SetStencilClear(this->m_iClearStencil);	
		
	// Set Default RenderTarget
	if(m_renderTargetNames[0].IsEmpty())
	{
		   
		// renderer에서 m_uiClearFlags가 0이면 Clear하지 않으므로 
		// 외부에서 체크할 필요 없음.
		pRenderer->BeginDefaultRenderTarget(m_uiClearFlags);
	}
	else 
	{
		//for(i=0 ; i < m_renderTargetNames.Size(); ++i)
		//{
		//	unsigned int renderTargetIdx = m_pOwnerFrame->FindRenderTargetIdx(
		//		m_renderTargetNames[i].Str());
		//	if((unsigned int)-1 == renderTargetIdx)
		//	{
		//		FDebug("Invalid Render target name %d", m_renderTargetNames[i].Str());
		//	}
		//}
		
		pRenderer->BeginRenderTarget(m_spRenderTarget, m_uiClearFlags);
	}

	pRenderer->UpdateScissorRect();

	

	pSceneMgr->UpdatePerSceneSharedShaderParams();
	
	// 
	U2D3DXEffectShader* pShader = GetEffectShader();
	if(pShader)
	{
		UpdateVariableShaderParams();
		if(!m_szTechnique.IsEmpty())
		{
#ifdef UNICODE 
			pShader->SetTechnique(ToUnicode(m_szTechnique.Str()));
#else 
			pShader->SetTechnique(m_szTechnique.Str());
#endif 
		}
		pShader->SetAttribute(m_shaderAttb);
		pRenderer->SetCurrEffectShader(pShader);		
		int numShaderPasses = pShader->Begin(true);
		U2ASSERT(1 == numShaderPasses);	 // assume 1-pass  for pass shaders
		pShader->BeginPass(0);
	}

	// Render GUI
	if(GetDrawGui())
	{
		// Render Gui
	}

	if(m_bDrawFullscreenQuad)
	{
		DrawFullScreenQuad();
	}

	m_bFrameInBegin = true;
	return m_framePhases.FilledSize();
}

//-------------------------------------------------------------------------------------------------
void U2FramePass::End()
{
	if(!m_bFrameInBegin)
	{
		return;
	}

	if((unsigned int)-1 != m_uiFrameShaderIdx)
	{
		U2D3DXEffectShader *pShader = m_pOwnerFrame->GetShader(m_uiFrameShaderIdx).GetShader();
		pShader->EndPass();
		pShader->End();
	}

	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	pRenderer->EndRenderTarget();
	
	U2SceneMgr *pSceneMgr = U2SceneMgr::Instance();	
	
	pSceneMgr->EndScene();

}

//------------------------------------------------------------------------------
/**
Renders a full-screen quad.
*/
void U2FramePass::DrawFullScreenQuad()
{
//	UpdateMeshCoords();

	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();

	U2SceneMgr* pSceneMgr = U2SceneMgr::Instance();
	D3DXMATRIX identMat;
	D3DXMatrixIdentity(&identMat);
	pRenderer->PushTransform(U2Dx9Renderer::TMT_WORLD, identMat);
	pRenderer->PushTransform(U2Dx9Renderer::TMT_VIEW, identMat);
	pRenderer->PushTransform(U2Dx9Renderer::TMT_PROJ, identMat);
	pRenderer->SetupMesh(m_spScreenQuad);
	pRenderer->SetVertexRange(0, 4);
	pRenderer->SetIndexRange(0, 6);
	pRenderer->DrawIndexedPrimitive(D3DPT_TRIANGLELIST);
	pRenderer->SetupMesh(0);	// FIXME FLOH: find out why this is necessary! if not done mesh data will be broken...
	pRenderer->PopTransform(U2Dx9Renderer::TMT_PROJ);
	pRenderer->PopTransform(U2Dx9Renderer::TMT_VIEW);
	pRenderer->PopTransform(U2Dx9Renderer::TMT_WORLD);	
}

//------------------------------------------------------------------------------
/**
This gathers the current global variable values from the render path
object and updates the shader parameter block with the new values.
*/
void U2FramePass::UpdateVariableShaderParams()
{
	unsigned int varIdx;
	unsigned int numVars = m_varContext.GetNumVariables();
	for(varIdx = 0; varIdx < numVars; ++varIdx)
	{
		const U2Variable& varParam = m_varContext.GetVariableAt(varIdx);

		U2FxShaderState::Param eShaderParam = (U2FxShaderState::Param)varParam.GetInt();

		const U2Variable* pVarVal = 
			U2VariableMgr::Instance()->GetGlobalVariable(varParam.GetHandle());
		U2ASSERT(pVarVal);
		U2ShaderArg shaderArg;
		switch(pVarVal->GetType())
		{
		case U2Variable::Int:
			shaderArg.SetInt(pVarVal->GetInt());
			break;
		case U2Variable::Float:
			shaderArg.SetFloat(pVarVal->GetFloat());
			break;
		case U2Variable::Float4:
			shaderArg.SetFloat4(pVarVal->GetFloat4());
			break;
		case U2Variable::Object:
			shaderArg.SetTexture((U2Dx9BaseTexture*)pVarVal->GetObj());
			break;
		case U2Variable::Matrix:
			shaderArg.SetMatrix44(&pVarVal->GetMatrix());
			break;
		case U2Variable::Vector4:
			shaderArg.SetVector4(pVarVal->GetVector4());
			break;

		default:
			U2ASSERT(!_T("U2Dx9FramePass : Invalid shader arg datatype"));
			break;
		}

		m_shaderAttb.SetArg(eShaderParam, shaderArg);
	}

}

//-------------------------------------------------------------------------------------------------
U2D3DXEffectShader* U2FramePass::GetEffectShader() const
{
	if((unsigned int)-1 != m_uiFrameShaderIdx)
	{
		const U2FrameShader& frameShader = m_pOwnerFrame->GetShader(m_uiFrameShaderIdx);
		U2D3DXEffectShader* pRealShader= frameShader.GetShader();
		return pRealShader;
	}
	else 
	{
		return NULL;
	}

}

