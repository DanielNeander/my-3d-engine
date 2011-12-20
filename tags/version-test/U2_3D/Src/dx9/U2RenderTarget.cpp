#include <U2_3D/src/U23DLibPCH.h>
#include "U2RenderTarget.h"


U2RenderTargets::U2RenderTargets()
	:m_uNumBuffers(0)
{

}

U2RenderTargets::~U2RenderTargets()
{
	for(unsigned int i = 0 ; i < MAX_FRAMEBUFFERS; ++i)
		m_spBackBuffers[i] = 0;
	m_spDSBuffer = 0;

	m_uNumBuffers = 0;
}


U2RenderTargets* U2RenderTargets::Create(uint32 numBuffers, 
							  U2Dx9Renderer *pRenderer)
{
	if(!pRenderer)
		return NULL;

	if(pRenderer->GetMaxRenderTargets() < numBuffers)
		return NULL;

	if(MAX_FRAMEBUFFERS < numBuffers)
		return NULL;

	U2RenderTargets* pRT = U2_NEW U2RenderTargets;
	pRT->m_uNumBuffers = numBuffers;
	return pRT;
}


U2RenderTargets* U2RenderTargets::Create(U2Dx9FrameBuffer* pBackBuffer, 
							  U2Dx9Renderer *pRenderer, 
							  U2Dx9FrameBuffer* pDSBuffer)
{
	if(!pRenderer)
		return NULL;

	if(!pBackBuffer)
		return NULL;
	
	if(pRenderer->GetMaxRenderTargets() < 1)
		return NULL;

	if(MAX_FRAMEBUFFERS < 1)
		return NULL;

	if(pDSBuffer &&  !pRenderer->IsDSBufferCompatible(pBackBuffer, pDSBuffer))
	{
		// Print Error
		return NULL;
	}

	U2RenderTargets* pRT = U2_NEW U2RenderTargets;
	pRT->m_uNumBuffers = 1;
	pRT->AttachBackBuffer(pBackBuffer, 0);

	if(pDSBuffer)
	{
		U2ASSERT(pDSBuffer->GetWidth() >= pBackBuffer->GetWidth());
		U2ASSERT(pDSBuffer->GetHeight() >= pBackBuffer->GetHeight());
		pRT->AttachDSBuffer(pDSBuffer);
	}
	return pRT;
}


bool U2RenderTargets::AttachBackBuffer(U2Dx9FrameBuffer* pBackBuffer, 
									  uint32 idx)
{
	U2ASSERT(m_uNumBuffers <= MAX_FRAMEBUFFERS);
	if(idx < m_uNumBuffers)
	{
		m_spBackBuffers[idx] = pBackBuffer;
		return true;
	}
	return false;
}


bool U2RenderTargets::AttachDSBuffer(U2Dx9FrameBuffer* pDSBuffer)
{
	m_spDSBuffer = pDSBuffer;
	return true;
}


U2Dx9FrameBuffer* U2RenderTargets::GetBuffer(unsigned int uiIndex) const
{
	if (uiIndex >= m_uNumBuffers)
		return NULL;

	return m_spBackBuffers[uiIndex];	
}

U2Dx9FrameBuffer* U2RenderTargets::GetDepthStencilBuffer() const
{
	return m_spDSBuffer;
}


unsigned int U2RenderTargets::GetBackBufferCnt() const
{
	return m_uNumBuffers;
}


unsigned short U2RenderTargets::GetWidth(unsigned int uiIndex) const
{
	if (uiIndex >= m_uNumBuffers)
		return 0;

	if (m_spBackBuffers[uiIndex])
		return m_spBackBuffers[uiIndex]->GetWidth();
	else
		return 0;
}
//---------------------------------------------------------------------------
unsigned short U2RenderTargets::GetHeight(unsigned int uiIndex) const
{
	if (uiIndex >= m_uNumBuffers)
		return 0;

	if (m_spBackBuffers[uiIndex])
		return m_spBackBuffers[uiIndex]->GetHeight();
	else
		return 0;
}
//---------------------------------------------------------------------------
unsigned short U2RenderTargets::GetDepthStencilWidth() const
{
	if (m_spDSBuffer)
		return m_spDSBuffer->GetWidth();
	else
		return 0;
}
//---------------------------------------------------------------------------
unsigned short U2RenderTargets::GetDepthStencilHeight() const
{
	if (m_spDSBuffer)
		return m_spDSBuffer->GetHeight();
	else
		return 0;
}