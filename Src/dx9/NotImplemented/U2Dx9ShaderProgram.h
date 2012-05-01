/************************************************************************
module	:	U2Dx9ShaderProgram
Authro	:	Yun sangyong
Desc	:
************************************************************************/
#ifndef U2_DX9SHADERPROGRAM_H
#define U2_DX9SHADERPROGRAM_H

#include <U2Lib/Src/U2RefObject.h>

class U2_3D	U2Dx9ShaderProgram : public U2RefObject
{
public:
	U2Dx9ShaderProgram(U2Dx9Renderer* pRenderer);
	virtual ~U2Dx9ShaderProgram();

protected:
	LPDIRECT3DDEVICE9	m_pD3DDev;
	U2Dx9Renderer*		m_pRenderer;

	U2Dx9RenderStateMgr*	m_pRSMgr;
};


#endif