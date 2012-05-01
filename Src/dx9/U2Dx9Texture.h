/************************************************************************
module	:	U2Dx9Texture
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_DX9_TEXTURE_H
#define U2_DX9_TEXTURE_H

#include "U2Dx9BaseTexture.h"
#include "U2Dx9FrameBuffer.h"

class U2_3D	U2Dx9Texture : public U2Dx9BaseTexture
{

public:
	static U2Dx9Texture* Create(const TCHAR* szFilename, 
		U2Dx9Renderer* pRenderer);
	
	static U2Dx9Texture* CreateRendered(unsigned short usWidth, 
		unsigned short usHeight, U2Dx9Renderer* pRenderer, D3DFORMAT pixelFormat);

	// File Load 
	// Copy Surface 	
	//void SetStatic(bool bStatic);
	//bool GetStatic() const;

	// RenderTexture
	U2Dx9FrameBufferPtr m_sp2DBuffer;
	
protected:
	U2Dx9Texture(U2Dx9Renderer* pRenderer);	

	bool CreateSurface(D3DFORMAT pixelFormat);

	
	
	//bool	m_bStatic;	

};


typedef U2SmartPtr<U2Dx9Texture>	U2Dx9TexturePtr;

#endif