//-----------------------------------------------------------------------------
// File: GBufferMRT.cpp
//
// Desc: GBufferSingle class manages with only 1 render target per render
//-----------------------------------------------------------------------------
#include "dxut.h"
#include "noRenderSystemDX9.h"
#include "GBufferSingle.h"

// NOTES :  16Bit INT format is a good comprimise between the size of FLOAT32 and the precision 
//		    needed to store positions etc
//			I might seem more worthwhile to have different buffers at different bitdepth but this has
//			compitibity issues that I don't want to address here (ATI Radeon 9700 MRT requires same bitdepths)
D3DFORMAT GBufferSingle::m_FatFormat = D3DFMT_A16B16G16R16; // a 16bit INTEGER format
//D3DFORMAT GBufferMRT::m_FatFormat = D3DFMT_A32B32G32R32F; // a 32bit FLOAT format

//-----------------------------------------------------------------------------
// Name: GBufferSingle ctor
//-----------------------------------------------------------------------------
GBufferSingle::GBufferSingle( LPDIRECT3D9 pD3D, LPDIRECT3DDEVICE9 pD3DDevice ) : 
	m_pD3D(pD3D),
	m_pD3DDevice(pD3DDevice)
{
	assert( m_pD3D != 0 );
	assert( m_pD3DDevice != 0 );

	m_pD3D->AddRef();
	m_pD3DDevice->AddRef();


	D3DCAPS9 d3dCaps;
	m_pD3DDevice->GetDeviceCaps( &d3dCaps );
	
	m_numSimulRT = 1;//d3dCaps.NumSimultaneousRTs;

}

//-----------------------------------------------------------------------------
// Name: GBufferSingle dtor
//-----------------------------------------------------------------------------
GBufferSingle::~GBufferSingle()
{

}

//-----------------------------------------------------------------------------
// Name: GBufferSingle Create buffers
// Desc: Creates the buffers of the size requested
//-----------------------------------------------------------------------------
bool GBufferSingle::CreateBuffers( unsigned int width, unsigned int height )
{
	HRESULT hRes;
	
	// create the fat framebuffer
	for(unsigned int i=0;i < NUMBER_PARAMETER_SURFACES; ++i)
	{
		hRes = m_pD3DDevice->CreateTexture( width,					// size should be >= to backbuffer
											height,					
											1,						// mipmaps TODO?
											D3DUSAGE_RENDERTARGET,	// render target obviously
											m_FatFormat,			// format our fat framebuffer is made up of
											D3DPOOL_DEFAULT,		// render target cannot be managed
											&m_pFatTexture[i],		//dest ptr
											0						// ignored
											);
		if( FAILED(hRes) )
			return false;
		m_pFatTexture[i]->GetSurfaceLevel( 0, &m_pFatSurface[i] );
	}
	// create the depth/stencil buffer we will use when rendering into the fat framebuffer
	// Discard? multisample?
	hRes = m_pD3DDevice->CreateDepthStencilSurface( width,				// size should be >= to backbuffer
													height, 
													D3DFMT_D24S8,		// format (TODO option for float depth?)
													D3DMULTISAMPLE_NONE,	// multisample
													0,					// Multisample quality
													FALSE,				// Discard TODO?
													&m_pFatDepth,		// dest
													0					// ignored
													);
	m_Width = width;
	m_Height = height;

	return true;
}

//-----------------------------------------------------------------------------
// Name: GBufferSingle GetRenderPassCount
// Desc: How many passes to fill the GBuffer (4 passes)
//-----------------------------------------------------------------------------
unsigned int GBufferSingle::GetRenderPassCount()
{
	return NUMBER_PARAMETER_SURFACES;
}

//-----------------------------------------------------------------------------
// Name: GBufferSingle SelectBuffersAsRenderTarget
// Desc: Which buffer to render to, only 1 non depth/stencil at a time 
//-----------------------------------------------------------------------------
void GBufferSingle::SelectBuffersAsRenderTarget( unsigned int buffers )
{
	if(buffers == 0)
		return;

	if( buffers & FFB_DEPTH || buffers & FFB_STENCIL )
		m_pD3DDevice->SetDepthStencilSurface( m_pFatDepth );
	else
		m_pD3DDevice->SetDepthStencilSurface( 0 );

	//remove stencil and depth flag to help debugger
	buffers &= ~(FFB_DEPTH|FFB_STENCIL);

	if( buffers & FFB_BUFFER0 )
	{
		m_pD3DDevice->SetRenderTarget( 0, m_pFatSurface[0] );
		buffers &= ~FFB_BUFFER0;
		assert(buffers == 0);
	}
	if( buffers & FFB_BUFFER1 )
	{
		m_pD3DDevice->SetRenderTarget( 0, m_pFatSurface[1] );
		buffers &= ~FFB_BUFFER1;
		assert(buffers == 0);
	}
	if( buffers & FFB_BUFFER2 )
	{
		m_pD3DDevice->SetRenderTarget( 0, m_pFatSurface[2] );
		buffers &= ~FFB_BUFFER2;
		assert(buffers == 0);
	}
	if( buffers & FFB_BUFFER3 )
	{
		m_pD3DDevice->SetRenderTarget( 0, m_pFatSurface[3] );
		buffers &= ~FFB_BUFFER3;
		assert(buffers == 0);
	}

}

//-----------------------------------------------------------------------------
// Name: GBufferSingle SelectBufferAsTexture
// Desc: Buffer is selected in the specified sampler
//-----------------------------------------------------------------------------
void GBufferSingle::SelectBufferAsTexture( FFB_BUFFER_NAME name, unsigned int samplerNum )
{
	m_pD3DDevice->SetTexture( samplerNum, GetBufferTexture(name) );
}

//-----------------------------------------------------------------------------
// Name: GBufferSingle GetBufferTexture
// Desc: return specified Buffer as a texture
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE9 GBufferSingle::GetBufferTexture( FFB_BUFFER_NAME name)
{
	// depth textures etc are largely not supported so I don't use them
	assert( !(name & FFB_DEPTH || name & FFB_STENCIL ) );

	if( name & FFB_BUFFER0 )
		return m_pFatTexture[0];
	else if( name & FFB_BUFFER1 )
		return m_pFatTexture[1];
	else if( name & FFB_BUFFER2 )
		return m_pFatTexture[2];
	else if( name & FFB_BUFFER3 )
		return m_pFatTexture[3];
	else return 0;
}

//-----------------------------------------------------------------------------
// Name: GBufferSingle GetBufferSurface
// Desc: return specified Buffer as a surface
//-----------------------------------------------------------------------------
LPDIRECT3DSURFACE9 GBufferSingle::GetBufferSurface( FFB_BUFFER_NAME name )
{
	if( (name & FFB_DEPTH || name & FFB_STENCIL ) )
		return m_pFatDepth;
	else
	if( name & FFB_BUFFER0 )
		return m_pFatSurface[0];
	else if( name & FFB_BUFFER1 )
		return m_pFatSurface[1];
	else if( name & FFB_BUFFER2 )
		return m_pFatSurface[2];
	else if( name & FFB_BUFFER3 )
		return m_pFatSurface[3];
	else return 0;
}

//-----------------------------------------------------------------------------
// Name: GBufferSingle Clear
// Desc: Clears the specified buffers (reasonable slow...)
//-----------------------------------------------------------------------------
void GBufferSingle::Clear(	unsigned int buffers, 
							unsigned int colour, 
							float ZVal, 
							unsigned int StencilVal )
{
	unsigned int flags = 0;

	if( buffers != 0 )
	{
		if( buffers & FFB_DEPTH || buffers & FFB_STENCIL )
		{
			m_pD3DDevice->SetDepthStencilSurface( m_pFatDepth );
			if( buffers & FFB_DEPTH )
				flags |= D3DCLEAR_ZBUFFER;
			if( buffers & FFB_STENCIL )
				flags |= D3DCLEAR_STENCIL;
		}

		if( buffers & FFB_BUFFER0 )
		{
			flags |= D3DCLEAR_TARGET;
			m_pD3DDevice->SetRenderTarget( 0, m_pFatSurface[0] );
			m_pD3DDevice->Clear( 0L, NULL, flags, colour, ZVal, StencilVal );
		}
		if( buffers & FFB_BUFFER1 )
		{
			flags |= D3DCLEAR_TARGET;
			m_pD3DDevice->SetRenderTarget( 0, m_pFatSurface[1] );
			m_pD3DDevice->Clear( 0L, NULL, flags, colour, ZVal, StencilVal );
		}
		if( buffers & FFB_BUFFER2 )
		{
			flags |= D3DCLEAR_TARGET;
			m_pD3DDevice->SetRenderTarget( 0, m_pFatSurface[2] );
			m_pD3DDevice->Clear( 0L, NULL, flags, colour, ZVal, StencilVal );
		}
		if( buffers & FFB_BUFFER3 )
		{
			flags |= D3DCLEAR_TARGET;
			m_pD3DDevice->SetRenderTarget( 0, m_pFatSurface[3] );
			m_pD3DDevice->Clear( 0L, NULL, flags, colour, ZVal, StencilVal );
		}
	}
}

HRESULT GBufferSingle::OnDeviceLoss( void )
{
	return this->OnDeviceRelease();
}

HRESULT GBufferSingle::OnDeviceRestore( void )
{
	return S_OK;
}

HRESULT GBufferSingle::OnDeviceRelease( void )
{
	SAFE_RELEASE( m_pFatDepth );

	for(unsigned int i=0;i < NUMBER_PARAMETER_SURFACES; ++i)
	{
		SAFE_RELEASE( m_pFatSurface[i] );
		SAFE_RELEASE( m_pFatTexture[i] );
	}

	SAFE_RELEASE( m_pD3D );
	SAFE_RELEASE( m_pD3DDevice );

	return S_OK;
}