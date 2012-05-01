/**************************************************************************************************
module	:	U2PixelProgram
Author	:	Yun sangyong
Desc	:	
*************************************************************************************************/
#pragma once
#ifndef U2_PIXELPROGRAM_H
#define U2_PIXELPROGRAM_H

#include "U2GpuProgram.h"

class U2_3D U2PixelProgram : public U2GpuProgram
{

public:
	U2PixelProgram(const U2DynString& strName);
	virtual ~U2PixelProgram();

	static U2PixelProgram* Load(const U2DynString& programName);

	IDirect3DPixelShader9* m_pD3DPS;
protected:
	U2PixelProgram();
};

typedef U2SmartPtr<U2PixelProgram>	U2PixelProgramPtr;


#endif