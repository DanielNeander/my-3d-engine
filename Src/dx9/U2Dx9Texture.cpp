#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9Texture.h"



U2Dx9Texture::U2Dx9Texture(U2Dx9Renderer* pRenderer) 
	:U2Dx9BaseTexture(),
	//m_bStatic(true)
	m_sp2DBuffer(0)
	
{
		
	 
}

U2Dx9Texture* U2Dx9Texture::Create(const TCHAR* szFilename, 
							U2Dx9Renderer* pRenderer)
{
	U2Dx9Texture *pThis = U2_NEW U2Dx9Texture(pRenderer);

	pThis->m_usTextureType  |= TEXTURETYPE_NORMAL;
	pThis->SetName(szFilename);	

	bool bSuccess = pThis->LoadFile(szFilename, pRenderer);
	if(bSuccess)
		return pThis;

	U2_DELETE pThis;
	pThis = NULL;
	return NULL;
}


//void U2Dx9Texture::SetStatic(bool bStatic)
//{
//	m_bStatic = bStatic;
//}
//
//
//bool U2Dx9Texture::GetStatic() const
//{
//	return m_bStatic;
//}

//-------------------------------------------------------------------------------------------------
U2Dx9Texture* U2Dx9Texture::CreateRendered(unsigned short usWidth, 
									unsigned short usHeight, U2Dx9Renderer* pRenderer,
									D3DFORMAT pixelFormat)
{
	
	
	U2Dx9Texture *pThis = U2_NEW U2Dx9Texture(pRenderer);
	
	pThis->m_usWidth = usWidth;
	pThis->m_usHeight = usHeight;	

	pThis->m_usTextureType |= TEXTURETYPE_RENDERED;

	if(!pThis->CreateSurface(pixelFormat))
	{
		U2_DELETE pThis;
		pThis = NULL;
		return NULL;		
	}

	LPDIRECT3DBASETEXTURE9 pBaseTex = pThis->GetD3DTex();
	U2ASSERT(pBaseTex);
	LPDIRECT3DSURFACE9 pSurf = NULL;
	

	LPDIRECT3DTEXTURE9 pD3DTex = NULL;
	// Grab the Ni2DBuffer from the NiRenderedTexture. This will
	// hold the DX9 Surface that we are rendering to.
	HRESULT hr = pBaseTex->QueryInterface(IID_IDirect3DTexture9,  (LPVOID *)&pD3DTex);	

	if(FAILED(hr))
	{
		FDebug("U2Dx9Texture::CreateRendered> Failed QueryInterface ");
		U2_DELETE pThis;
		return NULL;		
	}

	pThis->m_sp2DBuffer = U2Dx9FrameBuffer::CreateRenderSurf(pD3DTex, 
		pRenderer->GetD3DDevice());

	// Decrement the refcount from the QueryInterface call or else we leak.
	pD3DTex->Release();

	if(!pThis->m_sp2DBuffer)
	{
		FDebug("U2Dx9Texture::CreateRendered> Failed U2Dx9FrameBuffer::CreateRenderTex ");
		U2_DELETE pThis;
		return NULL;
	}

	return pThis;	
}


bool U2Dx9Texture::CreateSurface(D3DFORMAT pixelFormat)
{
	m_usMipMapLevels = 1;
	
	U2ASSERT(IsRenderedTexture());

	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);

	if(!pRenderer->IsTextureFormatOk(pixelFormat, D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE))  
	{	
		U2ASSERT(false);

		FDebug("nD3D9Texture:CreateRendered Could not create render target surface!\n");
		return false;
	}

	HRESULT hr;

	LPDIRECT3DTEXTURE9 pD3DTex = NULL;
	hr = pRenderer->GetD3DDevice()->CreateTexture(
		m_usWidth, 
		m_usHeight, 
		m_usMipMapLevels, 
		D3DUSAGE_RENDERTARGET, 
		pixelFormat, 
		D3DPOOL_DEFAULT,		// must be default
		&pD3DTex,
		NULL);
	U2_DXTrace(hr, _T("CreateTexture() Failed in U2Dx9Texture::CreateSurface()"));

	m_pD3DTex = pD3DTex;
	U2ASSERT(m_pD3DTex);

	return true;	
}
