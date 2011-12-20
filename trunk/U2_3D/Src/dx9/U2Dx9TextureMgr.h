/************************************************************************
module	:	U2Dx9TextureMgr
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_DX9_TEXTUREMGR_H
#define	U2_DX9_TEXTUREMGR_H


#include <d3dx9.h>
#include <U2Lib/Src/U2RefObject.h>
#include "U2Dx9BaseTexture.h"

class U2Dx9Renderer;

class U2_3D	U2Dx9TextureMgr : public U2RefObject
{
public:
	U2Dx9TextureMgr(U2Dx9Renderer *pRenderer);
	~U2Dx9TextureMgr();

	LPDIRECT3DBASETEXTURE9 GetTexture(U2Dx9BaseTexture* pNewTex, 
		bool& bChanged, bool& bMipmap, bool& bNonPow2);

private:		
	LPDIRECT3DDEVICE9 m_pD3DDev;
	U2Dx9Renderer*	m_pRenderer;
};



#endif