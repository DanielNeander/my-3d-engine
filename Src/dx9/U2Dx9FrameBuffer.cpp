#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9FrameBuffer.h"


LPDIRECT3DSURFACE9 U2Dx9FrameBuffer::ms_pkCurrDS = NULL;
unsigned int U2Dx9FrameBuffer::ms_uNumRenerTargets = 1;
bool U2Dx9FrameBuffer::ms_bIndependentBitDepths = false;

U2TPointerList<U2Dx9FrameBuffer*> U2Dx9FrameBuffer::ms_renderTargetDSs;

U2Dx9FrameBuffer::U2Dx9FrameBuffer()
	:m_eMTSType(D3DMULTISAMPLE_NONE),
	m_dwMTSQuality(0),
	m_eD3DFmt(D3DFMT_FORCE_DWORD),
	m_eDepth(DT_DEPTH_NONE),
	m_eStencil(ST_STENCIL_NONE),	
	m_pRenderTexture(NULL),
	m_pSurface(NULL)
	,m_bDisplayable(false)
{

}

U2Dx9FrameBuffer::~U2Dx9FrameBuffer()
{

	Shutdown();	
}

void U2Dx9FrameBuffer::Terminate()
{
	SAFE_RELEASE(ms_pkCurrDS);
}


bool U2Dx9FrameBuffer::SetRenderTarget(
									   LPDIRECT3DDEVICE9 pD3DDev, uint32 uTarget)
{
	if(IsSurfaceExist())
	{
		HRESULT hr = S_OK;
		if(uTarget > ms_uNumRenerTargets)
			return false;

		hr = pD3DDev->SetRenderTarget(uTarget, m_pSurface);
		if(FAILED(hr))
			return false;

		return true;
	}

	return false;
}



void U2Dx9FrameBuffer::ClearRenderTarget( 
							  LPDIRECT3DDEVICE9 pD3DDev, uint32 uTarget)
{
	if(uTarget != 0 && uTarget < ms_uNumRenerTargets )
	{
		pD3DDev->SetRenderTarget(uTarget, 0);
	}
}


void U2Dx9FrameBuffer::Shutdown()
{
	
	SAFE_RELEASE(m_pSurface);

	if(m_pRenderTexture)
	{
		((LPDIRECT3DTEXTURE9)m_pRenderTexture)->Release();
		m_pRenderTexture = 0;
	}	
}


U2Dx9FrameBuffer* U2Dx9FrameBuffer::CreateRenderSurf(LPDIRECT3DTEXTURE9 pD3DTex, 
										 LPDIRECT3DDEVICE9 pD3Ddev)
{
	U2Dx9FrameBuffer* pThis = U2_NEW U2Dx9FrameBuffer;
	U2ASSERT(pD3DTex);
	pThis->m_pRenderTexture = pD3DTex;
	pThis->m_pRenderTexture->AddRef();

	HRESULT hr = pD3DTex->GetSurfaceLevel(0, &pThis->m_pSurface);
	if(FAILED(hr))
	{
		U2_DELETE pThis;
		return NULL;
	}

	D3DSURFACE_DESC surfDesc;
	hr = pThis->m_pSurface->GetDesc(&surfDesc);
	if(FAILED(hr))
	{
		U2_DELETE pThis;
		return NULL;
	}

	pThis->ResetSize(surfDesc.Width, surfDesc.Height);

	pThis->m_dwMTSQuality = surfDesc.MultiSampleQuality;
	pThis->m_eMTSType = surfDesc.MultiSampleType;
	pThis->m_eD3DFmt = surfDesc.Format;

	return pThis;
}

U2Dx9FrameBuffer* U2Dx9FrameBuffer::CreateCubeSurf(LPDIRECT3DCUBETEXTURE9 pD3DCubeTex, 
									   D3DCUBEMAP_FACES eFace, U2Dx9BaseTexture* pExistFace, 
									   LPDIRECT3DDEVICE9 pD3DDev)
{
	U2Dx9FrameBuffer* pThis = U2_NEW U2Dx9FrameBuffer;
	U2ASSERT(pD3DCubeTex);
	pThis->m_pRenderTexture = pD3DCubeTex;
	pThis->m_pRenderTexture->AddRef();

	HRESULT hr = ((LPDIRECT3DCUBETEXTURE9)(pThis->m_pRenderTexture))->
		GetCubeMapSurface((D3DCUBEMAP_FACES)eFace, 0, &pThis->m_pSurface);

	if(FAILED(hr))
	{
		U2_DELETE pThis;
		return NULL;
	}

	D3DSURFACE_DESC surfDesc;
	hr = pThis->m_pSurface->GetDesc(&surfDesc);
	if(FAILED(hr))
	{
		U2_DELETE pThis;
		return NULL;
	}

	pThis->m_eMTSType = surfDesc.MultiSampleType;
	pThis->m_dwMTSQuality = surfDesc.MultiSampleQuality;
	pThis->m_eD3DFmt = surfDesc.Format;
	return pThis;
}


U2Dx9FrameBuffer* U2Dx9FrameBuffer::CreateColorBuffer(LPDIRECT3DSURFACE9 pD3DSurface)
{
	U2Dx9FrameBuffer* pThis = U2_NEW U2Dx9FrameBuffer();

	pThis->m_pSurface = pD3DSurface;

	D3DSURFACE_DESC surfDesc;
	HRESULT hr = pThis->m_pSurface->GetDesc(&surfDesc);
	if(FAILED(hr))
	{
		U2_DELETE pThis;
		return NULL;
	}

	pThis->m_eMTSType = surfDesc.MultiSampleType;
	pThis->m_dwMTSQuality = surfDesc.MultiSampleQuality;
	pThis->m_eD3DFmt = surfDesc.Format;

	return pThis;
}


bool U2Dx9FrameBuffer::RecreateColorBuffer(LPDIRECT3DSURFACE9 pD3DSurface)
{
	m_pSurface = pD3DSurface;
	if(!m_pSurface)
		return false;

	D3DSURFACE_DESC surfDesc;
	HRESULT hr = m_pSurface->GetDesc(&surfDesc);
	if(FAILED(hr))
	{
		SAFE_RELEASE(m_pSurface);
		return false;
	}

	m_eD3DFmt = surfDesc.Format;

	return true;
}

void U2Dx9FrameBuffer::ReleseDepthStencilTarget()
{
	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	if(pRenderer)
	{
		LPDIRECT3DDEVICE9 pD3DDev = pRenderer->GetD3DDevice();
		U2ASSERT(pD3DDev);
		LPDIRECT3DSURFACE9 pCurrDS = NULL;
		HRESULT hr = pD3DDev->GetDepthStencilSurface(&pCurrDS);
		if(SUCCEEDED(hr))
		{
			U2ASSERT(pCurrDS);
			if(pCurrDS == m_pSurface)
				SetNULLDepthStencilTarget((pRenderer->GetD3DDevice()));
			pCurrDS->Release();
		}
	}
}



bool U2Dx9FrameBuffer::SetDepthStencilTarget(LPDIRECT3DDEVICE9 pD3DDev)
{	
	if(!IsSurfaceExist())
		return false;		
		
	if(m_pSurface != ms_pkCurrDS)
	{
		HRESULT hr = pD3DDev->SetDepthStencilSurface(m_pSurface);
		if(FAILED(hr))
			return false;

		ms_pkCurrDS = m_pSurface;
	}
	return true;
}


bool U2Dx9FrameBuffer::SetNULLDepthStencilTarget(LPDIRECT3DDEVICE9 pD3DDev)
{	
	if(!IsSurfaceExist())
		return false;		

	if(NULL != ms_pkCurrDS)
	{
		HRESULT hr = pD3DDev->SetDepthStencilSurface(NULL);
		if(FAILED(hr))
			return false;

		ms_pkCurrDS = NULL;
	}
	return true;
}



U2Dx9FrameBuffer* U2Dx9FrameBuffer::CreateDepthStencilBuffer(LPDIRECT3DDEVICE9 pD3DDev)
{
	U2Dx9FrameBuffer* pThis = U2_NEW U2Dx9FrameBuffer;

	HRESULT hr = pD3DDev->GetDepthStencilSurface(&pThis->m_pSurface);
	if(FAILED(hr))
	{
		U2_DELETE pThis;
		return NULL;
	}

	D3DSURFACE_DESC surfDesc;
	hr = pThis->m_pSurface->GetDesc(&surfDesc);
	if(FAILED(hr))
	{
		U2_DELETE pThis;
		return NULL;
	}

	pThis->m_eMTSType = surfDesc.MultiSampleType;
	pThis->m_dwMTSQuality = surfDesc.MultiSampleQuality;
	pThis->m_eD3DFmt = surfDesc.Format;

	pThis->SetDepthStencilType(surfDesc.Format);

	ms_pkCurrDS = pThis->m_pSurface;

	pThis->ResetSize(surfDesc.Width, surfDesc.Height);

	return pThis;
}


U2Dx9FrameBuffer* U2Dx9FrameBuffer::CreateManualDSBuffer(LPDIRECT3DSURFACE9 pDSSurface)
{
	U2Dx9FrameBuffer* pThis = U2_NEW U2Dx9FrameBuffer;

	pThis->m_pSurface = pDSSurface;

	D3DSURFACE_DESC surfDesc;
	HRESULT hr = pThis->m_pSurface->GetDesc(&surfDesc);
	if(FAILED(hr))
	{
		U2_DELETE pThis;
		return NULL;
	}

	pThis->m_eMTSType = surfDesc.MultiSampleType;
	pThis->m_dwMTSQuality = surfDesc.MultiSampleQuality;
	pThis->m_eD3DFmt = surfDesc.Format;

	pThis->SetDepthStencilType(surfDesc.Format);

	return pThis;
}


bool U2Dx9FrameBuffer::RecreateDepthStencilBuffer(LPDIRECT3DDEVICE9 pD3DDev)
{
	if(IsSurfaceExist())
		Shutdown();

	HRESULT hr = pD3DDev->GetDepthStencilSurface(&m_pSurface);
	if(FAILED(hr))
		return false;

	D3DSURFACE_DESC surfDesc;
	hr = m_pSurface->GetDesc(&surfDesc);
	if(FAILED(hr))
	{
		SAFE_RELEASE(m_pSurface);
		return false;
	}

	SetDepthStencilType(surfDesc.Format);

	return true;	
}

void U2Dx9FrameBuffer::ShutdownAddedDepthStencilAll()
{
	U2ListNode<U2Dx9FrameBuffer*>* pIter = ms_renderTargetDSs.GetHeadNode();
	while(pIter)
	{
		U2Dx9FrameBuffer* pCurrDS = 
		ms_renderTargetDSs.GetNextElem(pIter);
		U2ASSERT(pCurrDS);
		pCurrDS->Shutdown();
	}
}

void U2Dx9FrameBuffer::RecreateAddedDepthStencilAll(LPDIRECT3DDEVICE9 pD3DDev)
{
	
	U2ListNode<U2Dx9FrameBuffer*>* pIter = ms_renderTargetDSs.GetHeadNode();
	while(pIter)
	{
		U2Dx9FrameBuffer* pCurrDS = 
			ms_renderTargetDSs.GetNextElem(pIter);
		U2ASSERT(pCurrDS);
		pCurrDS->RecreateAddedDepthStencilBuffer(pD3DDev);
	}	
}

void U2Dx9FrameBuffer::RemoveNewDSFromList(U2Dx9FrameBuffer* pDSBuffer)
{
	ms_renderTargetDSs.Remove(pDSBuffer);
}



U2Dx9FrameBuffer* U2Dx9FrameBuffer::CreateAddedDepthStencilBuffer(unsigned short usWidth, 
													  unsigned short usHeight,
													LPDIRECT3DDEVICE9 pD3DDev, D3DFORMAT d3dFmt,
													  D3DMULTISAMPLE_TYPE eMultiSampleType,
													   unsigned int uiMultiSampleQuality)
{
	if(d3dFmt == D3DFMT_UNKNOWN)
		return NULL;

	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();
	const U2RenderTargets* pDefaultRT = pRenderer->GetDefaultRenderTarget();
	U2ASSERT(pDefaultRT);

	HRESULT hr = pRenderer->GetDirect3D()->CheckDeviceFormat(pRenderer->GetAdapter(), 
		pRenderer->GetDevType(), pRenderer->GetAdapterFormat(), 
		D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, d3dFmt);

	if(FAILED(hr))
	{
		U2_DXTrace(hr, _T("Failed to U2Dx9FrameBuffer::CreateNewDSBuffer"));
		return NULL;
	}

	U2Dx9FrameBuffer* pThis = U2_NEW U2Dx9FrameBuffer();
	U2ASSERT(pThis);	

	ms_renderTargetDSs.InsertToHead(pThis);

	pThis->m_usWidth = usWidth;
	pThis->m_usHeight = usHeight;

	 //contents of the depth stencil buffer will be invalid after calling either IDirect3DDevice9::	//Present or IDirect3DDevice9::SetDepthStencilSurface with a different depth surface
	BOOL bDiscard = true;

	hr = pD3DDev->CreateDepthStencilSurface(pThis->m_usWidth, 
		pThis->m_usHeight,
		d3dFmt, 
		eMultiSampleType, 
		uiMultiSampleQuality,
		bDiscard, &pThis->m_pSurface, NULL);

	if(FAILED(hr))
	{
		U2_DXTrace(hr, _T("Failed to IDirect3DDevice9::CreateDepthStencilSurface"));
		U2_DELETE pThis;
		return NULL;
	}

	D3DSURFACE_DESC surfDesc;
	HRESULT hr2 = pThis->m_pSurface->GetDesc(&surfDesc);
	if(FAILED(hr2))
	{
		U2_DELETE pThis;
		return NULL;
	}

	pThis->m_eMTSType = surfDesc.MultiSampleType;
	pThis->m_dwMTSQuality = surfDesc.MultiSampleQuality;
	pThis->m_eD3DFmt = surfDesc.Format;

	return pThis;
}

bool U2Dx9FrameBuffer::RecreateAddedDepthStencilBuffer(LPDIRECT3DDEVICE9 pD3DDev)
{
	SAFE_RELEASE(m_pSurface);

	if(m_eD3DFmt == D3DFMT_UNKNOWN)
		return false;

	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();
	const U2RenderTargets* pCurrRT = pRenderer->GetDefaultRenderTarget();

	HRESULT hr = pRenderer->GetDirect3D()->CheckDeviceFormat(pRenderer->GetAdapter(), 
		pRenderer->GetDevType(), pRenderer->GetAdapterFormat(), 
		D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, m_eD3DFmt);

	if(FAILED(hr))
	{
		U2_DXTrace(hr, _T("Failed to U2Dx9FrameBuffer::RecreateNewDSBuffer"));
		return NULL;
	}
	

	//contents of the depth stencil buffer will be invalid after calling either IDirect3DDevice9::	//Present or IDirect3DDevice9::SetDepthStencilSurface with a different depth surface
	BOOL bDiscard = true;

	hr = pD3DDev->CreateDepthStencilSurface(m_usWidth, 
		m_usHeight,
		m_eD3DFmt, 
		m_eMTSType, 
		m_dwMTSQuality,
		bDiscard, &m_pSurface, NULL);

	if(FAILED(hr))
	{
		U2_DXTrace(hr, _T("Failed to IDirect3DDevice9::CreateDepthStencilSurface"));
		SAFE_RELEASE(m_pSurface);
		return false;
	}

	D3DSURFACE_DESC surfDesc;
	HRESULT hr2 = m_pSurface->GetDesc(&surfDesc);
	if(FAILED(hr2))
	{
		SAFE_RELEASE(m_pSurface);
		return false;
	}	
	
	m_eD3DFmt = surfDesc.Format;
	return true;	
}



U2Dx9FrameBuffer* U2Dx9FrameBuffer::CreateBackBuffer(LPDIRECT3DDEVICE9 pD3DDev, 
										  const D3DPRESENT_PARAMETERS& d3dPP)
{
	U2Dx9FrameBuffer *pThis = U2_NEW U2Dx9FrameBuffer;

	memcpy_s(&pThis->m_d3dPP, sizeof(pThis->m_d3dPP), &d3dPP, sizeof(d3dPP));

	HRESULT hr = pD3DDev->GetRenderTarget(0, &pThis->m_pSurface);

	if(FAILED(hr))
	{
		U2_DELETE pThis;
		return NULL;
	}

	D3DSURFACE_DESC surfDesc;
	hr = pThis->m_pSurface->GetDesc(&surfDesc);
	if(FAILED(hr))
	{
		U2_DELETE pThis;
		return NULL;
	}

	if(ms_uNumRenerTargets == 0)
	{	
		U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
		ms_uNumRenerTargets = pRenderer->GetMaxRenderTargets();
		ms_bIndependentBitDepths = pRenderer->GetIndependentBufferBitDepths();
	}

	pThis->ResetSize(surfDesc.Width, surfDesc.Height);

	pThis->m_eMTSType = surfDesc.MultiSampleType;
	pThis->m_dwMTSQuality = surfDesc.MultiSampleQuality;
	pThis->m_eD3DFmt = surfDesc.Format;
	pThis->m_bDisplayable = true;

	return pThis;

}

bool U2Dx9FrameBuffer::RecreateBackBuffer(LPDIRECT3DDEVICE9 pD3Dev)
{
	if(IsSurfaceExist())
		Shutdown();

	ms_pkCurrDS = NULL;

	
	HRESULT hr = pD3Dev->GetRenderTarget(0, &(m_pSurface));
	if(FAILED(hr))
	{
		SAFE_RELEASE(m_pSurface);
		return false;
	}

	D3DSURFACE_DESC surfDesc;
	hr = m_pSurface->GetDesc(&surfDesc);
	if(FAILED(hr))
	{
		SAFE_RELEASE(m_pSurface);
		return false;
	}
	
	m_eD3DFmt = surfDesc.Format;

	ResetSize(surfDesc.Width, surfDesc.Height);


	return true;
}



void U2Dx9FrameBuffer::ResetSize(unsigned short width, unsigned short height)
{
	m_usWidth = width;
	m_usHeight = height;
}


unsigned short U2Dx9FrameBuffer::GetWidth() const
{
	return m_usWidth;
}


unsigned short U2Dx9FrameBuffer::GetHeight() const
{
	return m_usHeight;
}


const D3DFORMAT U2Dx9FrameBuffer::GetSurfaceFormat() const
{
	return m_eD3DFmt;
}


const D3DMULTISAMPLE_TYPE U2Dx9FrameBuffer::GetMultiSampleType() const
{
	return m_eMTSType;
}




void U2Dx9FrameBuffer::SetDepthStencilType(D3DFORMAT eFmt)
{
	switch(eFmt)
	{
	case D3DFMT_D16_LOCKABLE:
		m_eDepth = DT_DEPTH;
		m_eStencil = ST_STENCIL_NONE;
		break;
	case D3DFMT_D32:
		m_eDepth = DT_DEPTH;		
		m_eStencil = ST_STENCIL_NONE;
		break;
	case D3DFMT_D15S1:
		m_eDepth = DT_DEPTH;
		m_eStencil = ST_STENCIL;
		break;
	case D3DFMT_D24S8:
		m_eDepth = DT_DEPTH;
		m_eStencil = ST_STENCIL;		
		break;
	case D3DFMT_D24X8:
		m_eDepth = DT_DEPTH;		
		m_eStencil = ST_STENCIL_NONE;
		break;
	case D3DFMT_D24X4S4:
		m_eDepth = DT_DEPTH;
		m_eStencil = ST_STENCIL;			
		break;
	}
}


U2Dx9FrameBuffer::DepthType U2Dx9FrameBuffer::GetDepthType () const
{
	return m_eDepth;
}


U2Dx9FrameBuffer::StencilType U2Dx9FrameBuffer::GetStencilType () const
{
	return m_eStencil;
}