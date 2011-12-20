/************************************************************************
module	:	U2RenderTarget
Author	:	Yun sangyong
Desc	:   
************************************************************************/
#pragma once
#ifndef U2_RENDERTARGET_H
#define U2_RENDERTARGET_H


#include "U2Dx9FrameBuffer.h"


class U2_3D U2RenderTargets : public U2RefObject
{

public:

	static U2RenderTargets* Create(uint32 numBuffers, 
		U2Dx9Renderer *pRenderer);

	static U2RenderTargets* Create(U2Dx9FrameBuffer* pBackBuffer, 
		U2Dx9Renderer *pRenderer, U2Dx9FrameBuffer* pDSBuffer);

	virtual ~U2RenderTargets();

	enum 
	{		
		MAX_FRAMEBUFFERS = 8	// 8 Back Buffer
	};

	unsigned short GetWidth(unsigned int uiIndex) const;
	unsigned short GetHeight(unsigned int uiIndex) const;

	unsigned short GetDepthStencilWidth() const;
	unsigned short GetDepthStencilHeight() const;

	bool AttachBackBuffer(U2Dx9FrameBuffer* pBackBuffer, uint32 idx);
	bool AttachDSBuffer(U2Dx9FrameBuffer* pDSBuffer);

	U2Dx9FrameBuffer* GetBuffer(unsigned int uiIndex) const;
	U2Dx9FrameBuffer* GetDepthStencilBuffer() const;

	unsigned int GetBackBufferCnt() const;


	
private:
	U2RenderTargets();

	U2Dx9FrameBufferPtr m_spBackBuffers[MAX_FRAMEBUFFERS];
	uint32 m_uNumBuffers;
	U2Dx9FrameBufferPtr m_spDSBuffer;
};

typedef U2SmartPtr<U2RenderTargets> U2RenderTargetPtr;

#endif