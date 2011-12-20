//------------------------------------------------------------------------------
//  nrprendertarget.cc
//  (C) 2004 RadonLabs GmbH
//------------------------------------------------------------------------------
#include <U2_3D/Src/U23DLibPCH.h>
#include "U2FrameRenderTarget.h"


//-------------------------------------------------------------------------------------------------
U2FrameRenderTarget::U2FrameRenderTarget()
:m_ePixelFmt(U2Dx9BaseTexture::X8R8G8B8),
m_fRelSize(1.0f),
m_iWidth(0),
m_iHeight(0),
m_pRenderedTex(0)
{
	
}

//-------------------------------------------------------------------------------------------------
U2FrameRenderTarget::~U2FrameRenderTarget()
{
	U2_DELETE m_pRenderedTex;
	m_pRenderedTex = 0;
}

//-------------------------------------------------------------------------------------------------

void U2FrameRenderTarget::Validate()
{
	if(!m_pRenderedTex)
	{
		U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
		U2ASSERT(pRenderer);

		

		int w, h;
		if(m_iWidth > 0 && m_iHeight > 0)
		{
			w = m_iWidth;
			h = m_iHeight;
		}
		else 
		{			
			// 지금은 Default Render Target Group이라고 생각한다.

			U2RenderTargets* pCurrRT;
			
			pCurrRT = pRenderer->GetCurrRenderTargets();
			if (!pCurrRT)
				pCurrRT = pRenderer->GetDefaultRenderTarget();
			// use relative width, height of curr display screen width, height
			w = pCurrRT->GetWidth(0) * m_fRelSize;
			h = pCurrRT->GetHeight(0) * m_fRelSize;			
		}

		U2Dx9BaseTexture* pIter = U2Dx9BaseTexture::GetHead();
		while(pIter)
		{
			if(pIter->GetName() == this->GetName())
			{
				m_pRenderedTex = (U2Dx9Texture*)pIter;				
				break;
			}

			pIter = pIter->GetNext();
		}

		if(!m_pRenderedTex)
		{
		
			m_pRenderedTex = U2Dx9Texture::CreateRendered(w, h, pRenderer, 
				U2Dx9BaseTexture::GetD3DFmtFromU2PixelFormat(m_ePixelFmt));
			m_pRenderedTex->SetName(this->GetName());
		}

		// always create a 24 bit depth buffer
/*		D3DFORMAT depthFormat = D3DFMT_D24X8;
		U2Dx9FrameBuffer* pDepthStencil = U2Dx9FrameBuffer::
			CreateAddedDepthStencilBuffer(w, h, pRenderer->GetD3DDevice(), depthFormat);	*/	
		
	}
}
