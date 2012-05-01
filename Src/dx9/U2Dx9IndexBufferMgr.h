/************************************************************************
module	:	U2Dx9FrameBufferMgr
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once 
#ifndef U2_FRAMEBUFFERMGR_H
#define U2_FRAMEBUFFERMGR_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>

class U2_3D U2Dx9IndexBufferMgr : public U2RefObject
{
public:

	U2Dx9IndexBufferMgr(LPDIRECT3DDEVICE9 pD3DDev);
	~U2Dx9IndexBufferMgr();

	LPDIRECT3DINDEXBUFFER9 PackIB(const unsigned short* pusTriList, 
		unsigned int uiLength, unsigned int uiMaxLength, 
		LPDIRECT3DINDEXBUFFER9 pkOldIB, unsigned int& uiIBSize, 
		D3DPOOL ePool, unsigned int uiUsage);


	LPDIRECT3DINDEXBUFFER9 ResizeTempPointConnect(unsigned int uiPoints, 
		bool bRefill = false, unsigned int uiUsage = 0, 
		D3DPOOL ePool = D3DPOOL_MANAGED);

	LPDIRECT3DINDEXBUFFER9 ResizeTempLineConnect(unsigned int uiPoints, 
		unsigned char* pbFlags, unsigned int& uiConn, unsigned int uiUsage = 0, 
		D3DPOOL ePool = D3DPOOL_MANAGED);

	LPDIRECT3DINDEXBUFFER9 CreateIndexBuffer(unsigned int uiLength, 
		unsigned int uiUsage, D3DFORMAT eFormat, D3DPOOL ePool, 
		D3DINDEXBUFFER_DESC* pkDesc = NULL);

	unsigned short* LockIB(LPDIRECT3DINDEXBUFFER9 pIB,unsigned int offset, unsigned int uiSize, 
		unsigned int uiLockFlags);

	bool UnlockIB(LPDIRECT3DINDEXBUFFER9 pIB);

	void DestoryBuffers();

private:
	LPDIRECT3DDEVICE9 m_pD3DDev;

	LPDIRECT3DINDEXBUFFER9 m_pTempPointConnIB;
	unsigned int m_uTempPointConnLen;

	LPDIRECT3DINDEXBUFFER9 m_pTempLineConnIB;
	unsigned int m_uTempLineConnLen;

};

#endif