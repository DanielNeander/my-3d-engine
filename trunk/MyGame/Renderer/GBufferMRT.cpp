//-----------------------------------------------------------------------------
// File: GBufferMRT.cpp
//
// Desc: GBufferMRT class manages to the MRT
//-----------------------------------------------------------------------------
#include "dxut.h"
#include "noRenderSystemDX9.h"
#include "GBufferMRT.h"

// NOTES :  16Bit INT format is a good comprimise between the size of FLOAT32 and the precision 
//		    needed to store positions etc
//			I might seem more worthwhile to have different buffers at different bitdepth but this has
//			compitibity issues that I don't want to address here (ATI Radeon 9700 MRT requires same bitdepths)
//D3DFORMAT GBufferMRT::m_FatFormat = D3DFMT_A16B16G16R16; // a 16bit INTEGER format
D3DFORMAT GBufferMRT::m_FatFormat = D3DFMT_A32B32G32R32F; // a 32bit FLOAT format


//-----------------------------------------------------------------------------
// Name: GBufferMRT ctor
//-----------------------------------------------------------------------------
GBufferMRT::GBufferMRT( LPDIRECT3D9 pD3D, LPDIRECT3DDEVICE9 pD3DDevice ) : 
	m_pD3D(pD3D),
	m_pD3DDevice(pD3DDevice)
{
	assert( m_pD3D != 0 );
	assert( m_pD3DDevice != 0 );

	m_pD3D->AddRef();
	m_pD3DDevice->AddRef();


	D3DCAPS9 d3dCaps;
	m_pD3DDevice->GetDeviceCaps( &d3dCaps );
	
	m_numSimulRT = min(d3dCaps.NumSimultaneousRTs,NUMBER_PARAMETER_SURFACES);

}

//-----------------------------------------------------------------------------
// Name: GBufferMRT dtor
//-----------------------------------------------------------------------------
GBufferMRT::~GBufferMRT()
{
	
}

//-----------------------------------------------------------------------------
// Name: GBufferMRT Create buffers
// Desc: Creates the buffers of the size requested
//-----------------------------------------------------------------------------
bool GBufferMRT::CreateBuffers( unsigned int width, unsigned int height )
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
// Name: GBufferMRT GetRenderPassCount
// Desc: How many passes to fill the GBuffer (for 4 simul MRT, only 1 pass)
//-----------------------------------------------------------------------------
unsigned int GBufferMRT::GetRenderPassCount()
{
	return NUMBER_PARAMETER_SURFACES / m_numSimulRT;
}


//-----------------------------------------------------------------------------
// Name: GBufferMRT SelectBuffersAsRenderTarget
// Desc: Which buffer to render to 
//-----------------------------------------------------------------------------
// you MUST select render targets starting at 0 (no gaps)
void GBufferMRT::SelectBuffersAsRenderTarget( unsigned int buffers )
{
	if(buffers == 0)
		return;

	assert( buffers & FFB_BUFFER0 ); // D3D requires RT0 to have a target

	if( buffers & FFB_DEPTH | buffers & FFB_STENCIL )
		m_pD3DDevice->SetDepthStencilSurface( m_pFatDepth );
	else
		m_pD3DDevice->SetDepthStencilSurface( 0 );

	unsigned int numTargets = 0;

	if( buffers & FFB_BUFFER0 )
	{
		m_pD3DDevice->SetRenderTarget( 0, m_pFatSurface[0] );
		numTargets++;

		if( buffers & FFB_BUFFER1 )
		{
			m_pD3DDevice->SetRenderTarget( 1, m_pFatSurface[1] );
			numTargets++;
		
			if( buffers & FFB_BUFFER2 )
			{
				m_pD3DDevice->SetRenderTarget( 2, m_pFatSurface[2] );
				numTargets++;
				if( buffers & FFB_BUFFER3 )
				{
					m_pD3DDevice->SetRenderTarget( 3, m_pFatSurface[3] );
					numTargets++;
				}
			}
		}
	}

	while( numTargets < 4 )
		m_pD3DDevice->SetRenderTarget( numTargets++, 0 );
}

//-----------------------------------------------------------------------------
// Name: GBufferMRT SelectBufferAsTexture
// Desc: Buffer is selected in the specified sampler
//-----------------------------------------------------------------------------
void GBufferMRT::SelectBufferAsTexture( FFB_BUFFER_NAME name, unsigned int samplerNum )
{
	// depth textures etc are largely not supported so I don't use them
	assert( !(name & FFB_DEPTH || name & FFB_STENCIL ) );

	m_pD3DDevice->SetTexture( samplerNum, GetBufferTexture(name) );
}

//-----------------------------------------------------------------------------
// Name: GBufferMRT GetBufferTexture
// Desc: return specified Buffer as a texture
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE9 GBufferMRT::GetBufferTexture( FFB_BUFFER_NAME name)
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
// Name: GBufferMRT GetBufferSurface
// Desc: return specified Buffer as a surface
//-----------------------------------------------------------------------------
LPDIRECT3DSURFACE9 GBufferMRT::GetBufferSurface( FFB_BUFFER_NAME name )
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
// Name: GBufferMRT Clear
// Desc: Clears the specified buffers (don't be too clever...)
//-----------------------------------------------------------------------------
void GBufferMRT::Clear(		unsigned int buffers, 
							unsigned int colour, 
							float ZVal, 
							unsigned int StencilVal )
{
	unsigned int flags = 0;

	if( buffers != 0 )
	{
		unsigned int curTarget = 0;
		// debug complains if we select the same buffer several times...
		for(unsigned int i=1;i < 4;i++)
			m_pD3DDevice->SetRenderTarget( i, 0 );

		if( buffers & FFB_BUFFER0 )
			m_pD3DDevice->SetRenderTarget( curTarget++, m_pFatSurface[0] );
		if( buffers & FFB_BUFFER1 )
			m_pD3DDevice->SetRenderTarget( curTarget++, m_pFatSurface[1] );
		if( buffers & FFB_BUFFER2 )
			m_pD3DDevice->SetRenderTarget( curTarget++, m_pFatSurface[2] );
		if( buffers & FFB_BUFFER3 )
			m_pD3DDevice->SetRenderTarget( curTarget++, m_pFatSurface[3] );

		if( curTarget != 0 )
		{
			flags |= D3DCLEAR_TARGET;
		}
		if( buffers & FFB_DEPTH || buffers & FFB_STENCIL )
		{
			m_pD3DDevice->SetDepthStencilSurface( m_pFatDepth );
			if( buffers & FFB_DEPTH )
				flags |= D3DCLEAR_ZBUFFER;
			if( buffers & FFB_STENCIL )
				flags |= D3DCLEAR_STENCIL;
		}
		
		m_pD3DDevice->Clear( 0L, NULL, flags, colour, ZVal, StencilVal );
	}
}

HRESULT GBufferMRT::OnDeviceLoss( void )
{
	return this->OnDeviceRelease();
}

HRESULT GBufferMRT::OnDeviceRestore( void )
{
	return S_OK;
}

HRESULT GBufferMRT::OnDeviceRelease( void )
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