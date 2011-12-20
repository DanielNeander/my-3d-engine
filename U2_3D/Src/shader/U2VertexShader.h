/**************************************************************************************************
module	:	U2VertexShader
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_VERTEXSHADER_H
#define U2_VERTEXSHADER_H

#include "U2Shader.h"
#include "U2VertexProgram.h"

class U2_3D U2VertexShader : public U2Shader 
{

public:
	U2VertexShader(const U2DynString& shaderName);
	virtual ~U2VertexShader();


	U2VertexProgram* GetGPUProgram() const;

protected:
	U2VertexShader();

};


typedef U2SmartPtr<U2VertexShader> U2VertexShaderPtr;


#endif 