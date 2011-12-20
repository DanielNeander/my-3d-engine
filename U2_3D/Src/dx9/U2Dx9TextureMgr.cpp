#include <U2_3D/src/U23DLibPCH.h>

#include "U2Dx9TextureMgr.h"
#include "U2Dx9Renderer.h"
#include "U2Dx9Texture.h"



U2Dx9TextureMgr::U2Dx9TextureMgr(U2Dx9Renderer *pRenderer)
{
	U2ASSERT(pRenderer);
	m_pRenderer = pRenderer;
	m_pD3DDev = m_pRenderer->GetD3DDevice();
	m_pD3DDev->AddRef();
}

U2Dx9TextureMgr::~U2Dx9TextureMgr()
{
	SAFE_RELEASE(m_pD3DDev);
}


LPDIRECT3DBASETEXTURE9 U2Dx9TextureMgr::GetTexture(U2Dx9BaseTexture* pNewTex, 
								  bool& bChanged, bool& bMipmap, bool& bNonPow2)
{
	bChanged = false;
	bMipmap = false;
	bNonPow2 = false;

	if(!pNewTex)
		return NULL;

	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	
	unsigned short usWidth = pNewTex->GetWidth();
	unsigned short usHeight = pNewTex->GetHeight();

	if(!(U2Math::IsPowerOf2((int)usWidth) && U2Math::IsPowerOf2((int)usHeight)))
	{
		bNonPow2 = true;
	}

	if(pNewTex->IsNormalTexture())
	{
		U2Dx9Texture* pNormalTex = (U2Dx9Texture*)pNewTex;

		//if(!pNormalTex->GetStatic())
		//{
		//	// Update
		//}
	}

	bMipmap = (pNewTex->GetMipLevels() > 1) ? true : false;

	return pNewTex->GetD3DTex();
}


