/**************************************************************************************************
module	:	U2VertexProgram
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_VERTEXPROGRAM_H
#define U2_VERTEXPROGRAM_H

#include "U2GpuProgram.h"

class U2_3D U2VertexProgram : public U2GpuProgram
{

public:
	U2VertexProgram(const U2DynString& strName);
	virtual ~U2VertexProgram();

	static U2VertexProgram* Load(const U2DynString& programName);


	IDirect3DVertexShader9 *m_pD3DVS;

protected:
	U2VertexProgram();
};

typedef U2SmartPtr<U2VertexProgram>	U2VertexProgramPtr;


#endif