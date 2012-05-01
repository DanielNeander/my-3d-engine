/************************************************************************
module	:	U2FrameRenderTarget
Author	:	Yun sangyong
Desc	:	Nebula Device 2 
			Validate의 rendertarget은 default rendertarget이라고 가정..
************************************************************************/
#pragma once 
#ifndef U2_FRAMERENDERTARGET_H
#define U2_FRAMERENDERTARGET_H

class U2_3D U2FrameRenderTarget : public U2Object
{

public:
		U2FrameRenderTarget();
		virtual ~U2FrameRenderTarget();

		void SetRelativeSize(float size);
		float GetRelativeSize() const;

		/// set the render target's pixel format
		void SetFormat(U2Dx9BaseTexture::Format f);
		/// get the render target's pixel format
		U2Dx9BaseTexture::Format GetFormat() const;

		void SetWidth(int w);
		int GetWidth() const;

		void SetHeight(int h);
		int GetHeight() const;

		void Validate();


		const U2Dx9Texture* GetTexture() const;

		U2Dx9Texture* m_pRenderedTex;	// Rendered texture			

private:
	U2Dx9Texture::Format m_ePixelFmt;
	float m_fRelSize;
	int m_iWidth;
	int m_iHeight;
	
	
};

typedef U2SmartPtr<U2FrameRenderTarget> U2FrameRenderTargetPtr;

#include "U2FrameRenderTarget.inl"


#endif 