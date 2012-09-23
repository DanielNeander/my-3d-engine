//-----------------------------------------------------------------------------
// File: GBufferMRT.h
//
// Desc: Header file GBufferMRT class (implements MRT version of a G-Buffer)
//-----------------------------------------------------------------------------
#pragma once

#include "GBuffer.h"

#define NUMBER_CHANNELS_PER_SURFACE 4
#define NUMBER_PARAMETER_SURFACES (NUMBER_PARAMETERS_CHANNELS / NUMBER_CHANNELS_PER_SURFACE)

class GBufferMRT : public GBuffer
{
public:
	GBufferMRT( LPDIRECT3D9 pD3D, LPDIRECT3DDEVICE9 pD3DDevice );
	~GBufferMRT();

	// method the derived class is using
	MRT_TYPE GetType() { return MT_MRT; }

	bool CreateBuffers( unsigned int width, unsigned int height );

	// number of render requires for all parameter buffers to be filled
	unsigned int GetRenderPassCount();

	unsigned int GetWidth(){ return m_Width; };
	unsigned int GetHeight(){ return m_Height; };

	void SelectBuffersAsRenderTarget( unsigned int name );
	void SelectBufferAsTexture( FFB_BUFFER_NAME name, unsigned int samplerNum );
	LPDIRECT3DTEXTURE9 GetBufferTexture( FFB_BUFFER_NAME buffer);
	LPDIRECT3DSURFACE9 GetBufferSurface( FFB_BUFFER_NAME buffer);

	void Clear(	unsigned int buffers, unsigned int colour = 0, float ZVal = 1.f, unsigned int StencilVal = 0);

	// noD3DResource
	HRESULT OnDeviceLoss(void);
	HRESULT OnDeviceRelease(void);
	HRESULT OnDeviceRestore(void);

	static D3DFORMAT		m_FatFormat;			// format (for simplicity and compatiblity all RT's are the same)

protected:
	unsigned int			m_numSimulRT;			// how many Render target we output to a the same time
	unsigned int			m_Width, m_Height;
	LPDIRECT3D9				m_pD3D;					// D3D object
	LPDIRECT3DDEVICE9		m_pD3DDevice;			// D3D Device object

	LPDIRECT3DTEXTURE9		m_pFatTexture[NUMBER_PARAMETER_SURFACES];	// the textures that make up the fat framebuffer
	LPDIRECT3DSURFACE9		m_pFatSurface[NUMBER_PARAMETER_SURFACES];	// the surface interface of the textures above
	LPDIRECT3DSURFACE9		m_pFatDepth;								// the depth stencil surface
};