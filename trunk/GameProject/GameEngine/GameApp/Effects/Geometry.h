//----------------------------------------------------------------------------------
// File: Geometry.h
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
#include <vector>
namespace Utility
{

namespace Geometry
{

template<typename V>
class SimpleVertexBuffer
{
public:

	template<typename V> SimpleVertexBuffer(ID3D11Device* device, ID3D11DeviceContext* context, const std::vector<V>& vertices, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, UINT bindflags = D3D11_BIND_VERTEX_BUFFER, UINT cpu_flags  = 0)
		:  m_device(device), m_buffer(0), m_num_vertices(UINT(vertices.size()))
	{
		HRESULT hr;
		m_device->AddRef();
		m_context = context;
		m_context->AddRef();
		
		D3D11_BUFFER_DESC bd;
		bd.Usage = usage ;
		bd.ByteWidth = sizeof( V ) * UINT(vertices.size());
		bd.BindFlags = bindflags;
		bd.CPUAccessFlags = cpu_flags;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &vertices[0];

		V(device->CreateBuffer( &bd, &InitData, &m_buffer ));

		m_stride = sizeof( V );
		m_offset = 0;
	}

	~SimpleVertexBuffer()
	{
		m_buffer->Release();
		m_device->Release();
		m_context->Release();
	}

	UINT NumVertices() const
	{
		return m_num_vertices;
	}
	
	ID3D11Buffer*	Buffer()
	{
		return m_buffer;

	}

	void BindToInputAssembler()
	{
		m_context->IASetVertexBuffers( 0, 1, &m_buffer, &m_stride, &m_offset );
	}

	void BindToStreamOut()
	{
		UINT offset[1] = { 0 };
		m_context->SOSetTargets( 1,  &m_buffer, offset);
	}

protected:
private:
	UINT		m_num_vertices;
	

	ID3D11Device*	m_device;
	ID3D11DeviceContext*	m_context;

	ID3D11Buffer*	m_buffer;

	UINT			m_stride;
	UINT			m_offset;


};

ID3D11InputLayout* CreateInputLayout(ID3D11Device* device, const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout, ID3DX11EffectTechnique* technique);
	

} // Geometry

}// Utility

