//----------------------------------------------------------------------------------
// File:  LightningRenderer.cpp
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

#include "stdafx.h"
#include "GameApp/Util/EffectUtil.h"
#include "LightningRenderer.h"
using namespace std;



namespace LightningDemo
{
	DXGI_FORMAT LightningRenderer::BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	LightningRenderer::LightningRenderer(ID3D11Device* device, ID3D11DeviceContext* context,  DXGI_SAMPLE_DESC back_buffer_sample_desc ):
	m_device(device),	
	m_effect(LoadEffect(device, "Data/Shaders/MainEffect.fx")),
	m_back_buffer_sample_desc (back_buffer_sample_desc),
	m_tech_bolt_out(m_effect->GetTechniqueByName("BoltOut")),	
	m_tech_lines_out(m_effect->GetTechniqueByName("ShowLines")),

	m_tech_add_buffer(m_effect->GetTechniqueByName("AddBuffer")),	
	m_tech_blur_buffer_horizontal(m_effect->GetTechniqueByName("BlurBufferHorizontal")),	
	m_tech_blur_buffer_vertical(m_effect->GetTechniqueByName("BlurBufferVertical")),	


	m_tech_down_sample_2x2(m_effect->GetTechniqueByName("DownSample2x2")),	
	
	m_constants_lightning_appearance(m_effect, "LightningAppearance"),
	m_constants_lightning_structure(m_effect, "LightningStructure"),
	
	m_world(m_effect,"world"),
	m_view(m_effect,"view"),
	m_projection(m_effect,"projection"),

	m_world_view(m_effect,"world_view"),
	m_world_view_projection(m_effect,"world_view_projection"),

	m_time(m_effect,"time"),

	m_fork(m_effect, "Fork"),
	m_subdivision_level(m_effect,"SubdivisionLevel"),


	m_animation_speed(m_effect,"AnimationSpeed"),
	m_charge(m_effect,"Charge"),

	m_buffer(m_effect, "buffer"),
	m_gradient(m_effect, "gradient"),
	m_buffer_texel_size(m_effect, "buffer_texel_size"),
	

	m_blur_sigma(m_effect,"BlurSigma"),
 	m_subdivide_buffer0(0),
	m_subdivide_buffer1(0),
	m_subdivide_layout(0),

	m_lightning_buffer0(m_device,BackBufferFormat,back_buffer_sample_desc),
	m_lightning_buffer1(m_device,BackBufferFormat,back_buffer_sample_desc),

	m_small_lightning_buffer0(m_device,BackBufferFormat,Utility::SampleDesc(1,0)),
	m_small_lightning_buffer1(m_device,BackBufferFormat,Utility::SampleDesc(1,0)),

	m_original_lightning_buffer(m_device,BackBufferFormat,Utility::SampleDesc(1,0)),

	m_gradient_texture(0),
	m_gradient_texture_srv(0),

	m_scene_depth_stencil_view(0),
	m_scene_render_target_view(0),

	m_down_sample_buffer_sizes(DecimationLevels),
	m_down_sample_buffers(DecimationLevels -1),

	m_num_scissor_rects(0),
	m_num_viewports(0),

	m_max_vertices(0)
{
	
	m_device->AddRef();
	m_context = context;
	m_context->AddRef();
	m_subdivide_layout = Geometry::CreateInputLayout(m_device,SubdivideVertex::GetLayout(),m_tech_bolt_out);

	BuildGradientTexture();
	m_gradient = m_gradient_texture_srv;
}


LightningRenderer::~LightningRenderer()
{
	delete m_subdivide_buffer0;
	delete m_subdivide_buffer1;

	while(!m_down_sample_buffers.empty())
	{
		delete m_down_sample_buffers.back();
		m_down_sample_buffers.pop_back();
	}

	while(!m_lightning_seeds.empty())
	{
		DestroyLightning(*m_lightning_seeds.begin());
	}
	
	m_gradient_texture_srv->Release();
	m_gradient_texture->Release();

	m_effect->Release();
	m_subdivide_layout->Release();
	m_context->Release();
	m_device->Release();
}


PathLightning*		LightningRenderer::CreatePathLightning(const std::vector<LightningPathSegment>& segments, int pattern_mask, unsigned int subdivisions)
{
	PathLightning* result = new PathLightning(m_effect, segments, pattern_mask, subdivisions);
	AddLightningSeed(result);
	return result;
}

ChainLightning*		LightningRenderer::CreateChainLightning(int pattern_mask, unsigned int subdivisions)
{
	ChainLightning* result = new ChainLightning(m_effect, pattern_mask, subdivisions);
	AddLightningSeed(result);
	return result;
}

void LightningRenderer::DestroyLightning(LightningSeed* seed)
{
	RemoveLightningSeed(seed);
	delete seed;
}

void	LightningRenderer::AddLightningSeed(LightningSeed* seed)
{
	m_lightning_seeds.insert(seed);
}
void	LightningRenderer::RemoveLightningSeed(LightningSeed* seed)
{
	m_lightning_seeds.erase(seed);
}

void LightningRenderer::SetTime(float time)
{
	m_time = time;
}

void LightningRenderer::SetMatrices(const D3DXMATRIX& world, const D3DXMATRIX& view,const D3DXMATRIX& projection)
{
	m_world = world;
	m_projection = projection;
	m_view = view;

	m_world_view = world * view;
	m_world_view_projection = world * view * projection;

}

void LightningRenderer::OnRenderTargetResize(unsigned width, unsigned height, ID3D11RenderTargetView* scene_render_target_view, ID3D11DepthStencilView* scene_depth_stencil_view)
{
	m_lightning_buffer0.Resize(width, height);
	m_lightning_buffer1.Resize(width, height);
	
	m_original_lightning_buffer.Resize(width, height);

	BuildDownSampleBuffers(width, height);

	m_scene_depth_stencil_view = scene_depth_stencil_view;
	m_scene_render_target_view = scene_render_target_view;

	m_buffer_texel_size =  D3DXVECTOR2(1.0f /width, 1.0f /height);
}

Geometry::SimpleVertexBuffer<SubdivideVertex>* LightningRenderer::Subdivide(LightningSeed* seed) 
{
	Geometry::SimpleVertexBuffer<SubdivideVertex>* source = m_subdivide_buffer0;
	Geometry::SimpleVertexBuffer<SubdivideVertex>* target = m_subdivide_buffer1;
	Geometry::SimpleVertexBuffer<SubdivideVertex>* last_target = target;

	m_context->IASetInputLayout(m_subdivide_layout);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	
	target->BindToStreamOut();

	m_subdivision_level = 0;
	m_fork = (seed->GetPatternMask() & (1 << 0));

	seed->RenderFirstPass();
	{
		UINT offset[1] = {0};
		ID3D11Buffer* zero[1] = {0};
		m_context->SOSetTargets(1,zero,offset);
	}

	last_target = target;
	swap(source,target);
	
	for(unsigned int i = 1; i < seed->GetSubdivisions(); ++i)
	{
		source->BindToInputAssembler();
		target->BindToStreamOut();		
	
		m_subdivision_level = i;
		m_fork = (seed->GetPatternMask() & (1 << i)) ;

		seed->GetSubdivideTechnique()->GetPassByIndex(0)->Apply(0, m_context);
		m_context->Draw( seed->GetNumVertices(i),0);
		{
			UINT offset[1] = {0};
			ID3D11Buffer* zero[1] = {0};
			m_context->SOSetTargets(1,zero,offset);
		}
	
		last_target = target;
		swap(source,target);
	}

	{
		UINT offset[1] = {0};
		ID3D11Buffer* zero[1] = {0};
		m_context->SOSetTargets(1,zero,offset);
		m_context->IASetVertexBuffers(0,1,zero,offset,offset);

	}
	return last_target;
}

void LightningRenderer::Begin()
{
	m_context->ClearRenderTargetView(m_lightning_buffer0.RenderTargetView(),D3DXVECTOR4(0,0,0,0));
	{
		ID3D11RenderTargetView* views[] = {m_lightning_buffer0.RenderTargetView()};
		m_context->OMSetRenderTargets(1, views, m_scene_depth_stencil_view);
	}
	BuildSubdivisionBuffers();
}
void LightningRenderer::Render(LightningSeed* seed, const LightningAppearance& appearance, float charge, float animation_speed, bool as_lines)
{
	{
		UINT offset[1] = {0};
		ID3D11Buffer* zero[1] = {0};
		m_context->SOSetTargets(1,zero,offset);
	}

	m_charge = charge;
	m_animation_speed = animation_speed;
	m_constants_lightning_appearance = appearance;

	seed->SetConstants();
	
	Geometry::SimpleVertexBuffer<SubdivideVertex>* subdivided = Subdivide(seed);

	subdivided->BindToInputAssembler();
	m_subdivision_level = 0;
	
	m_context->IASetInputLayout(m_subdivide_layout);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	if(as_lines)
		m_tech_lines_out->GetPassByIndex(0)->Apply(0, m_context);
	else
		m_tech_bolt_out->GetPassByIndex(0)->Apply(0, m_context);
	
	m_context->Draw(seed->GetNumVertices(seed->GetSubdivisions()),0);
}

void LightningRenderer::End(bool glow, D3DXVECTOR3 blur_sigma)
{
	ID3D11RenderTargetView* views[] = {NULL};
	m_context->OMSetRenderTargets(1, views, NULL);	
	m_context->ResolveSubresource(m_original_lightning_buffer.Texture(),0,m_lightning_buffer0.Texture(),0,BackBufferFormat);

	if(glow)
	{
		m_blur_sigma = blur_sigma;
		SaveViewports();
		
		DownSample(&m_original_lightning_buffer);
		ResizeViewport(m_down_sample_buffer_sizes.back().cx,m_down_sample_buffer_sizes.back().cy);
			Utility::RenderTargetPingPong ping_pong(&m_small_lightning_buffer0, &m_small_lightning_buffer1,0 , m_buffer);
			
			ping_pong.Apply(m_context, m_tech_blur_buffer_horizontal);						
			ping_pong.Apply(m_context, m_tech_blur_buffer_vertical);
		
		RestoreViewports();

		m_context->OMSetRenderTargets(1, &m_scene_render_target_view, m_scene_depth_stencil_view);
		m_buffer = ping_pong.LastTarget()->ShaderResourceView();
		DrawQuad(m_tech_add_buffer);
	}
	else
	{
		m_context->OMSetRenderTargets(1, &m_scene_render_target_view, m_scene_depth_stencil_view);
	}

	m_buffer = m_original_lightning_buffer.ShaderResourceView();
	DrawQuad(m_tech_add_buffer);
}

void LightningRenderer::BuildSubdivisionBuffers() 
{
	unsigned int max_segments = 0;

	for(std::set<LightningSeed*>::iterator it = m_lightning_seeds.begin(); it != m_lightning_seeds.end(); ++it)
	{
		max_segments = max(max_segments, (*it)->GetMaxNumVertices());
	}

	if( (0 != m_subdivide_buffer0) && (max_segments == m_subdivide_buffer0->NumVertices()))
		return;

	delete m_subdivide_buffer0;
	delete m_subdivide_buffer1;


	vector<SubdivideVertex> init_data(max_segments, SubdivideVertex());

	D3D11_USAGE usage =  D3D11_USAGE_DEFAULT;
	UINT flags = D3D11_BIND_VERTEX_BUFFER |  D3D11_BIND_STREAM_OUTPUT ;
	
	m_subdivide_buffer0 = new Geometry::SimpleVertexBuffer<SubdivideVertex>(m_device, m_context, init_data,usage,flags);
	m_subdivide_buffer1 = new Geometry::SimpleVertexBuffer<SubdivideVertex>(m_device, m_context, init_data,usage,flags);
	
}

void	LightningRenderer::SaveViewports()
{
	m_context->RSGetViewports(&m_num_viewports, 0);
	m_context->RSGetViewports(&m_num_viewports, m_viewports);

	m_context->RSGetScissorRects(&m_num_scissor_rects, 0);
	
	if( 0 != m_num_scissor_rects)
		m_context->RSGetScissorRects(&m_num_scissor_rects, m_scissor_rects);

}
void	LightningRenderer::ResizeViewport(unsigned int w, unsigned int h)
{
	D3D11_VIEWPORT viewport = {0, 0, w, h, 0.0f, 1.0f};
	D3D11_RECT	   scissor_rect = {0, 0, w, h};

	m_context->RSSetViewports(1, &viewport);
	m_context->RSSetScissorRects(1, &scissor_rect);

	
}
void	LightningRenderer::RestoreViewports()
{
	m_context->RSSetViewports(m_num_viewports, m_viewports);
	m_context->RSSetScissorRects(m_num_scissor_rects, m_scissor_rects);

}

void LightningRenderer::BuildGradientTexture()
{
	unsigned int w = 512;
	unsigned int h = 512;
	const unsigned int mip_levels = 4;
	D3D11_TEXTURE2D_DESC tex_desc = Utility::Texture2DDesc
	(
		w,
		h,
		mip_levels,
		1,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		Utility::SampleDesc(1,0),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
		0,
		D3D11_RESOURCE_MISC_GENERATE_MIPS
	);

	std::vector<unsigned char> data(4 * w * h);
	
	for(unsigned int y = 0; y < h; ++y)
	{
		unsigned int rowStart = y * w * 4;

		for(unsigned int x = 0; x < w; ++x)
		{
			float nx = x / float(w-1);
			float ny = y / float(h-1);

			float u = 2 * nx - 1;
			float v = 2 * ny - 1;

			float vv = max(0,min(1,1-sqrtf(u*u + v * v)));
			unsigned char value = (unsigned char)( max(0,min(255,vv * 255)));

			data[rowStart + x*4 + 0] = value; 
			data[rowStart + x*4 + 1] = value;
			data[rowStart + x*4 + 2] = value;
			data[rowStart + x*4 + 3]  = 255; 

		}

	}

	D3D11_SUBRESOURCE_DATA sr[mip_levels];
	for(unsigned int i = 0; i < mip_levels; ++i)
	{
		sr[i].pSysMem = &data[0];
		sr[i].SysMemPitch = w * 4;
		sr[i].SysMemSlicePitch = 4 * w * h;
	}

	HRESULT hr S_OK;
	DXCall(m_device->CreateTexture2D(&tex_desc,sr,&m_gradient_texture));
	DXCall(m_device->CreateShaderResourceView(m_gradient_texture,0,&m_gradient_texture_srv));
	
	m_context->GenerateMips(m_gradient_texture_srv);
	D3D11_TEXTURE2D_DESC result;
	m_gradient_texture->GetDesc(&result);
}

void	LightningRenderer::BuildDownSampleBuffers(unsigned int w, unsigned int h)
{
	int width = w;
	int height = h;
	for(size_t i = 0; i < DecimationLevels; ++i)
	{
			width  >>= 1;
			height >>= 1;
			SIZE s = {width, height};
			m_down_sample_buffer_sizes[i] = s;
	}
	
	for(size_t i = 0; i < m_down_sample_buffers.size(); ++i)
	{
		SIZE s = m_down_sample_buffer_sizes[i];
		if(0 == m_down_sample_buffers[i])
			m_down_sample_buffers[i] = new Utility::ColorRenderBuffer(m_device, s.cx, s.cy, BackBufferFormat,Utility::SampleDesc(1,0));
		else
			m_down_sample_buffers[i]->Resize(s.cx,s.cy);
	}

	m_small_lightning_buffer0.Resize(m_down_sample_buffer_sizes.back().cx, m_down_sample_buffer_sizes.back().cy);
	m_small_lightning_buffer1.Resize(m_down_sample_buffer_sizes.back().cx, m_down_sample_buffer_sizes.back().cy);

}

void	LightningRenderer::DownSample(Utility::ColorRenderBuffer* buffer)
{
	vector<Utility::ColorRenderBuffer*> sources;
	vector<Utility::ColorRenderBuffer*> targets;
	sources.push_back(buffer);

	copy(m_down_sample_buffers.begin(),m_down_sample_buffers.end(),back_inserter(sources));
	copy(m_down_sample_buffers.begin(),m_down_sample_buffers.end(),back_inserter(targets));

	targets.push_back(&m_small_lightning_buffer0);

	for(size_t i = 0; i < sources.size(); ++i)
	{
		ID3D11RenderTargetView* view[] = { targets[i]->RenderTargetView()};
		m_context->OMSetRenderTargets(1, const_cast<ID3D11RenderTargetView**> (view), 0);
		ResizeViewport(m_down_sample_buffer_sizes[i].cx, m_down_sample_buffer_sizes[i].cy );

		m_buffer = sources[i]->ShaderResourceView();
		DrawQuad(m_tech_down_sample_2x2);
	}
}

void LightningRenderer::DrawQuad(ID3DX11EffectTechnique* technique)
{
	ID3D11Buffer* zero = 0;
	UINT nought = 0;

	m_context->IASetVertexBuffers(0,1,&zero,&nought,&nought);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_context->IASetInputLayout(0);

	for(UINT n = 0; n < Effect::NumPasses(technique); ++n)
	{
		technique->GetPassByIndex(n)->Apply(0, m_context);
		m_context->Draw(4,0);
	}
}

}