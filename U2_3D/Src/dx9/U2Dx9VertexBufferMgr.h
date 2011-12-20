/************************************************************************
module	:	U2Dx9VertexBufferMgr
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once 
#ifndef U2_DX9_VERTEXBUFFERMGR_H
#define U2_DX9_VERTEXBUFFERMGR_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2Lib/src/U2SmartPtr.h>
#include <U2Lib/src/U2refObject.h>
#include <U2Lib/src/U2THashTable.h>

class U2_3D U2Dx9VertexBufferMgr : public U2RefObject
{
public:
	enum { DX9_MAX_VBS = 5 };


	U2Dx9VertexBufferMgr(LPDIRECT3DDEVICE9 pD3DDev);
	~U2Dx9VertexBufferMgr();
	
	void GenerateVertexParams(U2MeshData* pData,   unsigned int& uiVertexTypeDesc, unsigned int& uiVertexSize, 
		unsigned int& uiPosOffset, unsigned int& uiWeightOffset, 
		unsigned int& uiNormOffset, unsigned int& uiColorOffset, 
		unsigned int& uiTexOffset);

	void* LockVB(LPDIRECT3DVERTEXBUFFER9 pkVB, unsigned int uiOffset, 
		unsigned int uiSize, unsigned int uiLockFlags);

	void PackVB(unsigned char* pucDestBuffer, unsigned char* pucSrcBuffer, 
		unsigned int uiVertexSize, unsigned int uiDataSize, 
		unsigned int uiNumVerts);

	 bool UnlockVB(LPDIRECT3DVERTEXBUFFER9 pkVB);
	 
	 LPDIRECT3DVERTEXBUFFER9 CreateVertexBuffer(unsigned int uiLength, 
		 unsigned int uiUsage, unsigned int uiFVF, D3DPOOL ePool, 
		 D3DVERTEXBUFFER_DESC* pkDesc = NULL);

	 void DestoryBuffers();


	 LPDIRECT3DVERTEXDECLARATION9 GenerateShaderVertexParams(const U2VertexAttributes& inputAttr,
		 uint32& uVertexStride, bool bNebulaVertexDecl = false); 												 	 	 

private:
	
	



	void* m_pvTempBuffer;
	unsigned int m_uiTempBufferSize;

	 LPDIRECT3DDEVICE9 m_pkD3DDevice9;

	void* m_pLockedBuffer;
	unsigned int m_uLockedBufferSize;
};

typedef U2SmartPtr<U2Dx9VertexBufferMgr> U2Dx9VertexBufferMgrPtr;


#endif
