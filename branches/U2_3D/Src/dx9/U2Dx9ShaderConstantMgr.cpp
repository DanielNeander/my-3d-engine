#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9ShaderConstantMgr.h"

U2Dx9ShaderConstantMgr::U2Dx9ShaderConstantMgr(U2Dx9Renderer* pRenderer, 
					   const D3DCAPS9& d3dCaps)
					   :m_uiFirstCleanIVSReg(0),
					   m_uiFirstDirtyIVSReg(0),					   
					   m_uiFirstCleanIPSReg(0),
					   m_uiFirstDirtyIPSReg(0),
					   m_uiFirstCleanFVSReg(0),
					   m_uiFirstDirtyFVSReg(0),
					   m_uiFirstCleanFPSReg(0),
					   m_uiFirstDirtyFPSReg(0),
					   m_uiFirstCleanBVSReg(0),
					   m_uiFirstDirtyBVSReg(0),
					   m_uiFirstCleanBPSReg(0),
					   m_uiFirstDirtyBPSReg(0),
					   m_pRenderer(pRenderer)
{

	if(m_pRenderer)
		m_pD3DDev = pRenderer->GetD3DDevice();
	if(m_pD3DDev)
		m_pD3DDev->AddRef();

	m_uNumFVSConstants = d3dCaps.MaxVertexShaderConst;
	uint32 uiVSVer = d3dCaps.VertexShaderVersion;

	uint32 uMajorVSVer = 
		D3DSHADER_VERSION_MAJOR(uiVSVer);

	if(uMajorVSVer == 1)
	{
		m_uNumBVSConstants = 0;
		m_uNumIVSConstants = 0;
	}
	else if(uMajorVSVer == 2)
	{
		if(d3dCaps.VS20Caps.DynamicFlowControlDepth == 0)
			m_uNumBPSConstants = 0;
		else 
			m_uNumBPSConstants = 16;
	}
	else if(uMajorVSVer == 3) 
	{
		m_uNumBVSConstants = 16;
		m_uNumIVSConstants = 16;
	}

	uint32 uiPSVer = d3dCaps.PixelShaderVersion;

	uint32 uMajorPSVer = 
		D3DSHADER_VERSION_MAJOR(uiPSVer);

	if(uMajorPSVer == 1)
	{
		m_uNumFPSConstants = 8;
		m_uNumBPSConstants = 0;
		m_uNumIPSConstants = 0;
	}
	else if(uMajorPSVer == 2)
	{
		m_uNumFPSConstants = 32;

		if(d3dCaps.PS20Caps.DynamicFlowControlDepth == 0)
			m_uNumBPSConstants = 0;
		else 
			m_uNumBPSConstants = 16;

		if(d3dCaps.PS20Caps.StaticFlowControlDepth == 0)
			m_uNumIPSConstants = 16;
		else 
			m_uNumIPSConstants = 16;
	}
	else if(uMajorPSVer == 3)
	{
		m_uNumFPSConstants = 224;
		m_uNumBPSConstants = 16;
		m_uNumIPSConstants = 16;
	}
	

	m_uiFirstDirtyBVSReg = m_uNumBVSConstants;
	m_uiFirstDirtyBPSReg = m_uNumBPSConstants;	
	m_uiFirstDirtyFVSReg = m_uNumFVSConstants;
	m_uiFirstDirtyFPSReg = m_uNumFPSConstants;		
	m_uiFirstDirtyIVSReg = m_uNumIVSConstants;
	m_uiFirstDirtyIPSReg = m_uNumIPSConstants;

	if(m_uNumFVSConstants > 0)
	{
		uint32 uNumCnt = m_uNumFVSConstants * 4;
		m_pFVSConstants = U2_ALIGNED_ALLOC(float, uNumCnt, 16);
		ZeroMemory(m_pFVSConstants, 
			uNumCnt * sizeof(*m_pFVSConstants));

		m_pSavedFVSConstants = U2_ALIGNED_ALLOC(float, uNumCnt, 16);
		ZeroMemory(m_pSavedFVSConstants, 
			uNumCnt * sizeof(*m_pSavedFVSConstants));
	}

	if(m_uNumFPSConstants > 0)
	{
		uint32 uNumCnt = m_uNumFPSConstants * 4;
		m_pFPSConstants = U2_ALIGNED_ALLOC(float, uNumCnt, 16);
		ZeroMemory(m_pFPSConstants, 
			uNumCnt * sizeof(*m_pFPSConstants));

		m_pSavedFPSConstants = U2_ALIGNED_ALLOC(float, uNumCnt, 16);
		ZeroMemory(m_pSavedFPSConstants, 
			uNumCnt * sizeof(*m_pSavedFPSConstants));
	}

	if(m_uNumBVSConstants > 0)
	{
		
		m_pBVSConstants = U2_ALLOC(BOOL, m_uNumBVSConstants);
		ZeroMemory(m_pFVSConstants, 
			m_uNumBVSConstants * sizeof(*m_pBVSConstants));

		m_pSavedBVSConstants = U2_ALLOC(BOOL, m_uNumBVSConstants);
		ZeroMemory(m_pSavedFVSConstants, 
			m_uNumBVSConstants * sizeof(*m_pSavedBVSConstants));
	}

	if(m_uNumBPSConstants > 0)
	{
		m_pBPSConstants = U2_ALLOC(BOOL, m_uNumBPSConstants);
		ZeroMemory(m_pBPSConstants, 
			m_uNumBPSConstants * sizeof(*m_pBPSConstants));

		m_pSavedBPSConstants = U2_ALLOC(BOOL, m_uNumBPSConstants);
		ZeroMemory(m_pSavedFPSConstants, 
			m_uNumBPSConstants * sizeof(*m_pSavedBPSConstants));
	}

	if(m_uNumIVSConstants > 0)
	{
		uint32 uNumCnt = m_uNumIVSConstants * 4;
		m_pIVSConstants = U2_ALIGNED_ALLOC(int, uNumCnt, 16);
		ZeroMemory(m_pIVSConstants, 
			uNumCnt * sizeof(*m_pIVSConstants));

		m_pSavedIVSConstants = U2_ALIGNED_ALLOC(int, uNumCnt, 16);
		ZeroMemory(m_pSavedIVSConstants, 
			uNumCnt * sizeof(*m_pSavedIVSConstants));
	}

	if(m_uNumIPSConstants > 0)
	{
		uint32 uNumCnt = m_uNumIPSConstants * 4;
		m_pIPSConstants = U2_ALIGNED_ALLOC(int, uNumCnt, 16);
		ZeroMemory(m_pIPSConstants, 
			uNumCnt * sizeof(*m_pIPSConstants));

		m_pSavedIPSConstants = U2_ALIGNED_ALLOC(int, uNumCnt, 16);
		ZeroMemory(m_pSavedIPSConstants, 
			uNumCnt * sizeof(*m_pSavedIPSConstants));
	}

}

U2Dx9ShaderConstantMgr::~U2Dx9ShaderConstantMgr()
{
	U2_ALIGNEDFREE(m_pFVSConstants);
	U2_ALIGNEDFREE(m_pSavedFVSConstants);
	U2_ALIGNEDFREE(m_pFPSConstants);
	U2_ALIGNEDFREE(m_pSavedFPSConstants);

	U2_ALIGNEDFREE(m_pIVSConstants);
	m_pIVSConstants = NULL;
	U2_ALIGNEDFREE(m_pSavedIVSConstants);
	m_pSavedIVSConstants = NULL;
	U2_ALIGNEDFREE(m_pIPSConstants);
	m_pIPSConstants = NULL;
	U2_ALIGNEDFREE(m_pSavedIPSConstants);
	m_pSavedIPSConstants = NULL;

	U2_FREE(m_pBVSConstants);
	m_pBVSConstants= NULL;
	U2_FREE(m_pSavedBVSConstants);
	m_pSavedBVSConstants = NULL;
	U2_FREE(m_pBPSConstants);
	m_pBPSConstants = NULL;
	U2_FREE(m_pSavedBPSConstants);
	m_pSavedBPSConstants = NULL;

	SAFE_RELEASE(m_pD3DDev);
}

void	U2Dx9ShaderConstantMgr::Commit()
{	
	if (m_uiFirstDirtyFVSReg < m_uiFirstCleanFVSReg)
	{	
		
		HRESULT hr = m_pD3DDev->SetVertexShaderConstantF(
			m_uiFirstDirtyFVSReg, 
			m_pFVSConstants + m_uiFirstDirtyFVSReg * 4,
			m_uiFirstCleanFVSReg - m_uiFirstDirtyFVSReg);
		U2ASSERT(SUCCEEDED(hr));

		
	}

	if (m_uiFirstDirtyFPSReg < m_uiFirstCleanFPSReg)
	{
		HRESULT hr = m_pD3DDev->SetPixelShaderConstantF(
			m_uiFirstDirtyFPSReg, 
			m_pFPSConstants + m_uiFirstDirtyFPSReg * 4,
			m_uiFirstCleanFPSReg - m_uiFirstDirtyFPSReg);
		U2ASSERT(SUCCEEDED(hr));
	}

	if (m_uiFirstDirtyBVSReg < m_uiFirstCleanBVSReg)
	{
		HRESULT hr = m_pD3DDev->SetVertexShaderConstantB(
			m_uiFirstDirtyBVSReg, 
			(BOOL*)(m_pBVSConstants + m_uiFirstDirtyBVSReg),
			m_uiFirstCleanBVSReg - m_uiFirstDirtyBVSReg);
		U2ASSERT(SUCCEEDED(hr));
	}

	if (m_uiFirstDirtyBPSReg < m_uiFirstCleanBPSReg)
	{
		HRESULT hr = m_pD3DDev->SetPixelShaderConstantB(
			m_uiFirstDirtyBPSReg, 
			(BOOL*)(m_pBPSConstants + m_uiFirstDirtyBPSReg),
			m_uiFirstCleanBPSReg - m_uiFirstDirtyBPSReg);
		U2ASSERT(SUCCEEDED(hr));
	}

	if (m_uiFirstDirtyIVSReg < m_uiFirstCleanIVSReg)
	{
		HRESULT hr = m_pD3DDev->SetVertexShaderConstantI(
			m_uiFirstDirtyIVSReg, 
			m_pIVSConstants + m_uiFirstDirtyIVSReg * 4,
			m_uiFirstCleanIVSReg - m_uiFirstDirtyIVSReg);
		U2ASSERT(SUCCEEDED(hr));
	}

	if (m_uiFirstDirtyIPSReg < m_uiFirstCleanIPSReg)
	{
		HRESULT hr = m_pD3DDev->SetPixelShaderConstantI(
			m_uiFirstDirtyIPSReg, 
			m_pIPSConstants + m_uiFirstDirtyIPSReg * 4,
			m_uiFirstCleanIPSReg - m_uiFirstDirtyIPSReg);
		U2ASSERT(SUCCEEDED(hr));
	}

	// Mark all registers as clean
	m_uiFirstDirtyFVSReg = m_uNumFVSConstants;
	m_uiFirstDirtyFPSReg = m_uNumFPSConstants;
	m_uiFirstDirtyIVSReg = m_uNumIVSConstants;
	m_uiFirstDirtyIPSReg = m_uNumIPSConstants;
	m_uiFirstDirtyBVSReg = m_uNumBVSConstants;
	m_uiFirstDirtyBPSReg = m_uNumBPSConstants;
	m_uiFirstCleanFVSReg = 0;
	m_uiFirstCleanFPSReg = 0;
	m_uiFirstCleanIVSReg = 0;
	m_uiFirstCleanIPSReg = 0;
	m_uiFirstCleanBVSReg = 0;
	m_uiFirstCleanBPSReg = 0;

}

U2Dx9ShaderConstantMgr* U2Dx9ShaderConstantMgr::Create(U2Dx9Renderer* pRenderer, 
									  const D3DCAPS9& d3dCaps)
{

	return U2_NEW U2Dx9ShaderConstantMgr(pRenderer, d3dCaps);

}