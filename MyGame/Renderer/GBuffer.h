//-----------------------------------------------------------------------------
// File: GBuffer.h
//
// Desc: Header file GBuffer base class 
//-----------------------------------------------------------------------------
#pragma once

// there are several ways of accessing multple render targets that make up the fat framebuffer
enum MRT_TYPE
{
	MT_SINGLE,	// single render target only
	MT_MRT,		// use multiple render targets
	MT_MET,		// use multiple element textures (TODO)
};

enum FFB_BUFFER_NAME
{
	FFB_BUFFER0 = (1 << 0),
	FFB_BUFFER1 = (1 << 1),
	FFB_BUFFER2 = (1 << 2),
	FFB_BUFFER3 = (1 << 3),

	FFB_DEPTH	= (1 << 4),	// depth and stencil are special in lots of ways...
	FFB_STENCIL	= (1 << 5),

	// buffer renaming
	// its easier to keep track of the buffers if there given meaningful name, so I just alias the buffer numbers
	FFB_POSITION = FFB_BUFFER0,
	FFB_NORMAL	 = FFB_BUFFER1,
	FFB_DIFFUSE	 = FFB_BUFFER2,
	FFB_SPECULAR = FFB_BUFFER3,

	MAX_FFV = 0xFFFFFFFF
};

// 16 seperate scalars (in 4 sets of 4)
#define NUMBER_PARAMETERS_CHANNELS		16

class GBuffer : public noD3DResource
{
public:
	virtual ~GBuffer() {};

	// method the derived class is using
	virtual MRT_TYPE GetType() = 0;

	virtual bool CreateBuffers( unsigned int width, unsigned int height ) = 0;

	// number of render requires for all parameter buffers to be filled
	virtual unsigned int GetRenderPassCount() = 0;

	virtual unsigned int GetWidth() = 0;
	virtual unsigned int GetHeight() = 0;

	// sets the specified buffers as the current render target
	virtual void SelectBuffersAsRenderTarget( unsigned int buffers ) = 0;

	// sets the specified buffer into the specified sampler for access in pixel shader
	virtual void SelectBufferAsTexture( FFB_BUFFER_NAME buffer, unsigned int samplerNum ) = 0;

	// get the texture of the specified buffer (not DEPTH or STENCIL)
	virtual LPDIRECT3DTEXTURE9 GetBufferTexture( FFB_BUFFER_NAME buffer) = 0;
	// get the surface with the specified buffer 
	virtual LPDIRECT3DSURFACE9 GetBufferSurface( FFB_BUFFER_NAME buffer) = 0;

	// clears the specified buffers (default to usual settings)
	virtual void Clear(	unsigned int buffers, unsigned int colour = 0, float ZVal = 1.0f,	unsigned int StencilVal = 0 ) = 0;

};
