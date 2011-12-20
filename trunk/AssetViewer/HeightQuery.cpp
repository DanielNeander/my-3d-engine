#include "stdafx.h"
#include "HeightQuery.h"
#include <U2Application/U2WinApp.h>


HeightQuery::HeightQuery()
:m_spHeightShader(0),
m_spHeightField(0),
m_spTex(0),
m_pSysMemSurface(0),
m_spMesh(0),
m_spScreenCam(0)

{

}

HeightQuery::~HeightQuery()
{
	Terminate();
}

bool HeightQuery::OnReset()
{
	if (!InitShader())
		return false;
	
	if (!InitMesh())
		return false;

	if (!InitRenderTarget())
		return false;

	if (!InitCamera())
		return false;

	return true;

}

bool HeightQuery::Initialize()
{
	return OnReset();
}


void HeightQuery::Terminate()
{
	m_spMesh = 0;
	m_spHeightShader = 0;

}

bool HeightQuery::InitMesh()
{
	U2TriListData *pData = U2_NEW U2TriListData;	
	m_spMesh = U2_NEW U2N2Mesh(pData);
	m_spMesh->SetUsages(U2N2Mesh::WriteOnly);
	m_spMesh->SetVertexComponents(U2N2Mesh::Coord);
	pData->SetVertexCount(1, 1);

	return m_spMesh->CreateEmptyBuffers();
}


bool HeightQuery::InitShader()
{
	U2FrameShader& FrameShad= U2SceneMgr::Instance()->m_spFrame->GetShader(_T("heightquery"));
	
	m_spHeightShader = FrameShad.GetShader();
	U2ASSERT(m_spHeightShader);
	m_spHeightShader->IncRefCount();
		

	return true;
}

bool HeightQuery::InitRenderTarget()
{
	IsRendererOK2;

	m_spTex = U2Dx9Texture::CreateRendered(1, 1, pRenderer, D3DFMT_R32F);

	if (FAILED(pRenderer->GetD3DDevice()->CreateOffscreenPlainSurface(
		1, 1, D3DFMT_R32F, D3DPOOL_SYSTEMMEM, &m_pSysMemSurface, 0))) {
		return false;
	}

	U2RenderTargets* pDefaultRT = pRenderer->GetDefaultRenderTarget();

	m_spRT = U2RenderTargets::Create(1, pRenderer);
	m_spRT->AttachBackBuffer(m_spTex->m_sp2DBuffer, 0);
	// Compatiable with Default Depth Stencil Buffer
	m_spRT->AttachDSBuffer(pDefaultRT->GetDepthStencilBuffer());
	

	return true;

}

bool HeightQuery::InitCamera()
{
	IsRendererOK2;
	
	m_spScreenCam = U2_NEW U2Camera();

	D3DXVECTOR3 zero(0,0,0);
	D3DXVECTOR3 unitX(1,0,0);
	D3DXVECTOR3 unitY(0,1,0);
	D3DXVECTOR3 unitZ(0,0,1);
	
	m_spScreenCam->SetFrame(zero, unitZ, unitY, unitX);
	m_spScreenCam->SetFrustum(0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 10.0f, true);	


	D3DXMatrixTranslation(&m_matWorld, -1/2, -1/2, 0);

	m_spScreenCam->SetViewport(0.0f, 1.0f, 1.0f, 0.0f);
		
	return true;

}

bool HeightQuery::SetVertexData(float u, float v)
{
	if (!m_spMesh)
		return false;
	
	float *vPtr = m_spMesh->LockVertices();
	U2ASSERT(vPtr);

	*vPtr++ = u; *vPtr++ = 0; *vPtr++ = v;
	m_spMesh->UnlockVertices();

	return true;
}


void HeightQuery::Render()
{
	IsRendererOK;

	
	
	pRenderer->BeginScene();

	pRenderer->BeginRenderTarget(m_spRT, U2Dx9Renderer::CLEAR_ALL);

	pRenderer->SetCamera(m_spScreenCam);
	pRenderer->SetProjectionTM(m_spScreenCam);
	pRenderer->SetViewport(m_spScreenCam);

	U2SceneMgr* pSceneMgr = U2SceneMgr::Instance();
	
	//m_matProjOrig = pRenderer->PopTransform(U2Dx9Renderer::TMT_PROJ);
	//m_matViewOrig = pRenderer->PopTransform(U2Dx9Renderer::TMT_VIEW);
	//m_matWorldOrig = pRenderer->PopTransform(U2Dx9Renderer::TMT_WORLD);	

	unsigned int origStencil = pRenderer->GetStencilClear();
	float origDepth = pRenderer->GetDepthClear();
	D3DXCOLOR origBGColor;
	pRenderer->GetBackgroundColor(origBGColor);

	m_matView = pRenderer->GetViewMat();
	m_matProj = pRenderer->GetProjMat();

	pRenderer->PushTransform(U2Dx9Renderer::TMT_WORLD, m_matWorld);
	pRenderer->PushTransform(U2Dx9Renderer::TMT_VIEW, m_matView);
	pRenderer->PushTransform(U2Dx9Renderer::TMT_PROJ, m_matProj);
	pRenderer->SetupMesh(m_spMesh);
	pRenderer->SetVertexRange(0, 1);	

	pRenderer->SetCurrEffectShader(m_spHeightShader);

	// Setup Shader 
	//m_spHeightShader->Begin(false);

	pRenderer->DrawPrimitive(D3DPT_POINTLIST);
	pRenderer->SetupMesh(0);	// FIXME FLOH: find out why this is necessary! if not done mesh data will be broken...
	pRenderer->PopTransform(U2Dx9Renderer::TMT_PROJ);
	pRenderer->PopTransform(U2Dx9Renderer::TMT_VIEW);
	pRenderer->PopTransform(U2Dx9Renderer::TMT_WORLD);	


	pRenderer->EndRenderTarget();

	pRenderer->EndScene();

	pRenderer->SetStencilClear(origStencil);
	pRenderer->SetBackgroundColor(origBGColor);
	pRenderer->SetDepthClear(origDepth);


}


bool HeightQuery::QueryPoint(float u, float v, float *height)
{
	IsRendererOK2;

	if (!m_spTex || !m_pSysMemSurface) 
		return false;

	if (!SetVertexData(u, v))
		return false;

	Render();	

	// copy the render target to system memory

	if (pRenderer->GetD3DDevice()->GetRenderTargetData(m_spTex->m_sp2DBuffer->m_pSurface,
		m_pSysMemSurface))
		return false;

	// extract the queried height
	D3DLOCKED_RECT rect;
	if (FAILED(m_pSysMemSurface->LockRect(&rect, 0, 0))) {
		return false;
	}

	*height = ((float*)rect.pBits)[0];

	m_pSysMemSurface->UnlockRect();

	return true;

}