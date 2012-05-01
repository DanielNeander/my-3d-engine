/************************************************************************
module	:	U2Dx9FrameBuffer
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once 
#ifndef U2_FRAMEBUFFER_H
#define U2_FRAMEBUFFER_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2_3D/src/Main/U2Object.h>
#include <U2Lib/Src/U2SmartPtr.h>

class U2_3D U2Dx9FrameBuffer : public U2Object
{

public:

	enum DepthType
	{
		DT_DEPTH_NONE,
		DT_DEPTH		
	};

	enum StencilType
	{
		ST_STENCIL_NONE,
		ST_STENCIL
	};

	virtual ~U2Dx9FrameBuffer();

	bool CanDisplayFrame() { return m_bDisplayable; }


	void Shutdown();	

	static U2Dx9FrameBuffer* CreateRenderSurf(LPDIRECT3DTEXTURE9 pD3DTex, 
		LPDIRECT3DDEVICE9 pD3Ddev);

	static U2Dx9FrameBuffer* CreateCubeSurf(LPDIRECT3DCUBETEXTURE9 pD3DCubeTex, 
		D3DCUBEMAP_FACES eFace, U2Dx9BaseTexture* pExistFace, 
		LPDIRECT3DDEVICE9 pD3DDev);

	static U2Dx9FrameBuffer* CreateColorBuffer(LPDIRECT3DSURFACE9 pD3DSurface);

	static U2Dx9FrameBuffer* CreateBackBuffer(LPDIRECT3DDEVICE9 pD3DDev, 
		const D3DPRESENT_PARAMETERS& pD3DPP);

	bool	RecreateColorBuffer(LPDIRECT3DSURFACE9 pD3DSurface);

	static void Terminate();
	

	static U2Dx9FrameBuffer* CreateDepthStencilBuffer(LPDIRECT3DDEVICE9 pD3DDev);
	static U2Dx9FrameBuffer* CreateManualDSBuffer(LPDIRECT3DSURFACE9 pSurface);
	static U2Dx9FrameBuffer* CreateAddedDepthStencilBuffer(unsigned short usWidth, 
		unsigned short usHeight, LPDIRECT3DDEVICE9 pD3DDev, D3DFORMAT d3dFmt,
		D3DMULTISAMPLE_TYPE eMultiSampleType = D3DMULTISAMPLE_NONE, unsigned int uiMultiSampleQuality = 0);
	
	
	bool IsSurfaceExist() { return m_pSurface ? true : false; }

	// device reset 
	bool Recreate(LPDIRECT3DDEVICE9 pD3Dev);
	bool RecreateDepthStencilBuffer(LPDIRECT3DDEVICE9 pD3DDev);
	bool RecreateAddedDepthStencilBuffer(LPDIRECT3DDEVICE9 pD3DDev);
	bool RecreateBackBuffer(LPDIRECT3DDEVICE9 pD3Dev);

	D3DPRESENT_PARAMETERS& GetPresentParams() 
	{ return m_d3dPP; }
	
	void ReleseDepthStencilTarget();

	void ResetSize(unsigned short width, unsigned short height);

	LPDIRECT3DSURFACE9 GetSurface() { return m_pSurface; }
	
	unsigned short GetWidth() const;
	unsigned short GetHeight() const;

	const D3DFORMAT GetSurfaceFormat() const;
	const D3DMULTISAMPLE_TYPE GetMultiSampleType() const;

	bool SetRenderTarget(LPDIRECT3DDEVICE9 pD3DDev, uint32 uTarget);
	static void ClearRenderTarget(LPDIRECT3DDEVICE9 pD3DDev, uint32 uTarget);
	bool SetDepthStencilTarget(LPDIRECT3DDEVICE9 pD3Dev);
	bool SetNULLDepthStencilTarget(LPDIRECT3DDEVICE9 pD3Dev);

	void SetDepthStencilType(D3DFORMAT eFmt);

	static void ShutdownAddedDepthStencilAll();
	static void	RecreateAddedDepthStencilAll(LPDIRECT3DDEVICE9 pD3DDev);

	static void RemoveNewDSFromList(U2Dx9FrameBuffer* pThis);	

	LPDIRECT3DSURFACE9 m_pSurface;	// Depth-Stencil 
	LPDIRECT3DBASETEXTURE9 m_pRenderTexture;

	D3DPRESENT_PARAMETERS m_d3dPP;	

	D3DMULTISAMPLE_TYPE m_eMTSType;
	DWORD		m_dwMTSQuality;
	D3DFORMAT	m_eD3DFmt;

	unsigned short m_usWidth;
	unsigned short m_usHeight;

	static LPDIRECT3DSURFACE9 ms_pkCurrDS;
	static unsigned int ms_uNumRenerTargets;
	static bool ms_bIndependentBitDepths;

	DepthType GetDepthType () const;
	StencilType GetStencilType () const;

	DepthType m_eDepth;
	StencilType m_eStencil;

	bool m_bDisplayable;

private:
	U2Dx9FrameBuffer();

	static U2TPointerList<U2Dx9FrameBuffer*> ms_renderTargetDSs;

};


typedef U2SmartPtr<U2Dx9FrameBuffer> U2Dx9FrameBufferPtr;


#endif // end