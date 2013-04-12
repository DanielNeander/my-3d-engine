//----------------------------------------------------------------------------------
// File: ColorRenderBuffer.h
// Author: Mathias Schott
// Email:  sdkfeedback@nvidia.com
// 
// Copyright (c) 2007 NVIDIA Corporation. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA OR ITS SUPPLIERS
// BE  LIABLE  FOR  ANY  SPECIAL,  INCIDENTAL,  INDIRECT,  OR  CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
//
//----------------------------------------------------------------------------------

#pragma once

#include "GameApp/Utility.h"

namespace Utility
{

class ColorRenderBuffer
{
public:
	ColorRenderBuffer(ID3D11Device* device, DXGI_FORMAT format, DXGI_SAMPLE_DESC sample_desc);
	ColorRenderBuffer(ID3D11Device* device, unsigned int w, unsigned int h, DXGI_FORMAT format, DXGI_SAMPLE_DESC sample_desc,unsigned int  cpu_access = 0, unsigned int  misc_flags = 0);
	~ColorRenderBuffer();

	void	Resize(unsigned int width, unsigned int height);

	D3D11_TEXTURE2D_DESC TextureDesc();
	ID3D11Texture2D*			Texture();
	ID3D11ShaderResourceView*	ShaderResourceView();
	ID3D11RenderTargetView*		RenderTargetView();

private:
	void						Create(unsigned int w, unsigned int h, DXGI_FORMAT format, DXGI_SAMPLE_DESC sample_desc,unsigned int cpu_access, unsigned int misc_flags);

	ID3D11Device*				m_device;

	ID3D11Texture2D*			m_texture;
	ID3D11ShaderResourceView*	m_shader_resource_view;
	ID3D11RenderTargetView*		m_render_target_view;
	
	DXGI_FORMAT					m_format;
	DXGI_SAMPLE_DESC			m_sample_desc;
};


class RenderTargetPingPong
{
public:
		RenderTargetPingPong(ColorRenderBuffer*	source,	ColorRenderBuffer*	target, ID3D11DepthStencilView* depth_stencil_view, ID3DX11EffectShaderResourceVariable* shader_resource_variable );

		void Apply(ID3D11DeviceContext* context, ID3DX11EffectTechnique* technique);
		ColorRenderBuffer*	LastTarget();
	
private:
	ColorRenderBuffer*	m_source;
	ColorRenderBuffer*	m_target;
	ColorRenderBuffer*	m_last_target;

	ID3DX11EffectShaderResourceVariable* m_shader_resource_variable;
	ID3D11DepthStencilView* m_depth_stencil_view;

};

}