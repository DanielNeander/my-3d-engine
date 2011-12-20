/**************************************************************************************************
module	:	U2LightNode
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef	U2_LIGHTNODE_H
#define	U2_LIGHTNODE_H

#include "U2Node.h"
#include <U2_3D/Src/shader/U2ShaderAttribute.h>

class U2_3D U2LightNode : public U2Node 
{
	DECLARE_RTTI;
public:
	U2LightNode(U2Light* pLight=0);
	virtual ~U2LightNode();

	void SetLight(U2Light* pLight);

	U2Light* GetLight();
	const U2Light* GetLight() const;

	void SetCastShadows(bool b);
	bool GetCastShadows() const; 

	D3DXMATRIX CalculateCropMatrix();

	// D3DXEffectShader 지원 함수
	const U2Light& ApplyLight(const D3DXVECTOR4& shadowLitMask);
	const U2Light& UpdateLight();

	virtual void UpdateWorldData(float fAppTime);

	U2ShaderAttribute& GetShaderAttb(); 

protected:
	

	U2LightPtr m_spLight;
	U2ShaderAttribute m_shaderAttb;
};

#include "U2LightNode.inl"
typedef U2SmartPtr<U2LightNode> U2LightNodePtr;

#endif 