//----------------------------------------------------------------------------------
// File: EffectVariable.h
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
namespace Effect
{
class IntVariable 
{
public:
	
	IntVariable(ID3DX11Effect* effect, const std::string& name)
	{
		m_variable =  effect->GetVariableByName(name.c_str())->AsScalar();
	}

	IntVariable& operator = ( int i)
	{
		m_variable->SetInt( i);
		return *this;
	}
private:
	ID3DX11EffectScalarVariable*  m_variable;
};


class FloatVariable 
{
public:
	
	FloatVariable(ID3DX11Effect* effect, const std::string& name)
	{
		m_variable =  effect->GetVariableByName(name.c_str())->AsScalar();
	}

	FloatVariable& operator = ( float f)
	{
		m_variable->SetFloat( f);
		return *this;
	}

	operator float()
	{
		float result;
		m_variable->GetFloat(&result);
		return result;
	}
private:
	ID3DX11EffectScalarVariable*  m_variable;
};

class BoolVariable 
{
public:
	
	BoolVariable(ID3DX11Effect* effect, const std::string& name)
	{
		m_variable =  effect->GetVariableByName(name.c_str())->AsScalar();
	}

	BoolVariable& operator = ( BOOL b)
	{
		m_variable->SetBool( b);
		return *this;
	}
private:
	ID3DX11EffectScalarVariable*  m_variable;
};
class MatrixVariable 
{
public:
	
	MatrixVariable(ID3DX11Effect* effect, const std::string& name)
	{
		m_variable =  effect->GetVariableByName(name.c_str())->AsMatrix();
	}

	MatrixVariable& operator = ( const D3DXMATRIX& matrix)
	{
		m_variable->SetMatrix( D3DXMATRIX(matrix));
		return *this;
	}

	operator D3DXMATRIX()
	{
		D3DXMATRIX result;
		m_variable->GetMatrix(result);
		return result;
	}
private:
	ID3DX11EffectMatrixVariable*  m_variable;
};


class ShaderResourceVariable 
{
public:
	
	ShaderResourceVariable(ID3DX11Effect* effect, const std::string& name)
	{
		m_variable =  effect->GetVariableByName(name.c_str())->AsShaderResource();
	}
	ShaderResourceVariable& operator = ( const   ID3D11ShaderResourceView * shader_resource)
	{
		m_variable->SetResource(const_cast< ID3D11ShaderResourceView *>(shader_resource));
		return *this;
	}

	operator ID3DX11EffectShaderResourceVariable*()
	{
		return m_variable;
	}
private:
	ID3DX11EffectShaderResourceVariable*  m_variable;
};


class Vector2Variable 
{
public:
	
	Vector2Variable(ID3DX11Effect* effect, const std::string& name)
	{
		m_variable =  effect->GetVariableByName(name.c_str())->AsVector();
	}

	Vector2Variable& operator = ( const D3DXVECTOR2& v)
	{
		m_variable->SetFloatVector( D3DXVECTOR2(v));
		return *this;
	}
private:
	ID3DX11EffectVectorVariable*  m_variable;
};


class noVec3Variable 
{
public:
	
	noVec3Variable(ID3DX11Effect* effect, const std::string& name)
	{
		m_variable =  effect->GetVariableByName(name.c_str())->AsVector();
	}

	noVec3Variable& operator = ( const D3DXVECTOR3& v)
	{
		m_variable->SetFloatVector( D3DXVECTOR3(v));
		return *this;
	}
private:
	ID3DX11EffectVectorVariable*  m_variable;
};


class Vector4Variable 
{
public:
	
	Vector4Variable(ID3DX11Effect* effect, const std::string& name)
	{
		m_variable =  effect->GetVariableByName(name.c_str())->AsVector();
	}

	Vector4Variable& operator = ( const D3DXVECTOR4& v)
	{
		m_variable->SetFloatVector( D3DXVECTOR4(v));
		return *this;
	}
private:
	ID3DX11EffectVectorVariable*  m_variable;
};

class ConstantBuffer
{
public:
	
	ConstantBuffer(ID3DX11Effect* effect, const std::string& name)
	{
		m_buffer = effect->GetConstantBufferByName(name.c_str());
	}

	template<typename T>
	ConstantBuffer& operator = (const T& data)
	{
		m_buffer->SetRawValue(const_cast<T*>(&data), 0, sizeof(T));
		return *this;
	}

	operator ID3DX11EffectConstantBuffer*()
	{
		return m_buffer;
	}
private:
	ID3DX11EffectConstantBuffer*  m_buffer;
};
} // Effect
} // Utility