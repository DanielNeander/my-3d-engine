/************************************************************************
                                                                     
************************************************************************/
#pragma once 
#ifndef U2_DX9_VERTEXSHADER_H
#define U2_DX9_VERTEXSHADER_H

#include "U2Dx9ShaderProgram.h"

class U2_3D U2Dx9VertexShader : public U2Dx9ShaderProgram
{
public:
	U2Dx9VertexShader(U2Dx9Renderer* pRenderer);
	virtual ~U2Dx9VertexShader();
	

protected:

	LPDIRECT3DVERTEXSHADER9		m_pVS;
	LPDIRECT3DVERTEXDECLARATION9 m_pDecl;
	
};


#endif
