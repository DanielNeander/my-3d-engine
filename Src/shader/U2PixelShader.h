/**************************************************************************************************
module	:	U2PixelShader
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_PIXELSHADER_H
#define U2_PIXELSHADER_H

#include "U2Shader.h"
#include "U2PixelProgram.h"

class U2_3D U2PixelShader : public U2Shader 
{

public:
	U2PixelShader(const U2DynString& shaderName);
	virtual ~U2PixelShader();

	U2PixelProgram* GetGPUProgram() const;

protected:
	U2PixelShader();

};

typedef U2SmartPtr<U2PixelShader> U2PixelShaderPtr;


#endif 