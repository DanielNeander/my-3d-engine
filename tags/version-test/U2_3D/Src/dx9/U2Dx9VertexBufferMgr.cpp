#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9VertexBufferMgr.h"



U2Dx9VertexBufferMgr::U2Dx9VertexBufferMgr(LPDIRECT3DDEVICE9 pD3DDev)
	:m_pvTempBuffer(NULL),
	m_uiTempBufferSize(0),
	m_pLockedBuffer(NULL),
	m_uLockedBufferSize(0)	
{

	m_pkD3DDevice9 = pD3DDev;
	m_pkD3DDevice9->AddRef();

}

U2Dx9VertexBufferMgr::~U2Dx9VertexBufferMgr()
{

	U2_FREE(m_pvTempBuffer);
	SAFE_RELEASE(m_pkD3DDevice9);
	
}



LPDIRECT3DVERTEXDECLARATION9 U2Dx9VertexBufferMgr::GenerateShaderVertexParams(const U2VertexAttributes& inputAttr,
																			  uint32& uVertexStride, bool bNebulaVertexDecl)	
{
	uint32 elementCnt = 0; 
	bool bPos = inputAttr.HasPosition();
	if(bPos) elementCnt++;		
	bool bBlendWeight = inputAttr.HasBlendWeight();
	if(bBlendWeight) elementCnt++;
	bool bNormal = inputAttr.HasNormal();
	if(bNormal) elementCnt++;

	// Wild Magic 과 Nebula Device2의 쉐이더 정점 포맷을 통일 

	int iUnit;
	for(iUnit = 0; iUnit < inputAttr.GetMaxColors(); iUnit++)
	{
		if(inputAttr.HasColor(iUnit))
		{
			elementCnt++;
		}
	}
	bool bFog = inputAttr.HasFog();
	if(bFog) elementCnt++;
	bool bPSize = inputAttr.HasPSize();
	if(bPSize) elementCnt++;

	for(iUnit = 0; iUnit < inputAttr.GetMaxTCoords(); iUnit++)
	{
		if(inputAttr.HasTCoord(iUnit))
		{
			elementCnt++;
		}
	}

	bool bBlendIncidices = inputAttr.HasBlendIndices();
	if(bBlendIncidices) elementCnt++;
	bool bTangent = inputAttr.HasTangent();
	if(bTangent) elementCnt++;
	bool bBitangent = inputAttr.HasBitangent();
	if(bBitangent) elementCnt++;

	if(elementCnt == 0)
	{
		FDebug("U2Dx9Renderer::PackMeshData- InputAtrr has no Attritues\n");
		return false;
	}

	uVertexStride = 0;

	uint32 uChannels; 
	

	D3DVERTEXELEMENT9 d3dVertElem;
	d3dVertElem.Stream = 0;
	d3dVertElem.Method = D3DDECLMETHOD_DEFAULT;

	std::vector<D3DVERTEXELEMENT9> vertElemArray;	
	uint32 uIdx = 0; 	

	if(bNebulaVertexDecl)
	{
		if(bPos)
		{
			uChannels = inputAttr.GetPositionChannels();
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += uChannels * sizeof(float);		
			d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
			d3dVertElem.Usage = D3DDECLUSAGE_POSITION;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
			uIdx++;
		}

		// 2D
		/*if(screenPos)
		{

		}*/

		if(bNormal)
		{
			uChannels = inputAttr.GetNormalChannels();
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += uChannels * sizeof(float);
			d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
			d3dVertElem.Usage = D3DDECLUSAGE_NORMAL;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
			uIdx++;
		}

		// UV0 ~ 4(Nebula Max Support : 4)
		for(iUnit = 0; iUnit < inputAttr.GetMaxTCoords(); iUnit++)
		{
			if(inputAttr.HasTCoord(iUnit))
			{
				uChannels = inputAttr.GetTCoordChannels(iUnit);
				d3dVertElem.Offset = (WORD)uVertexStride;
				uVertexStride += uChannels * sizeof(float);
				d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
				d3dVertElem.Usage= D3DDECLUSAGE_TEXCOORD;
				d3dVertElem.UsageIndex = (BYTE)iUnit;
				vertElemArray.push_back(d3dVertElem);
				uIdx++;
			}
		}

		// Color 
		for(iUnit = 0; iUnit < inputAttr.GetMaxColors(); iUnit++)
		{
			if(inputAttr.HasColor(iUnit))
			{	
				uChannels = inputAttr.GetColorChannels(iUnit);
				d3dVertElem.Offset = (WORD)uVertexStride;
				uVertexStride += uChannels * sizeof(float);				
				d3dVertElem.Type = D3DDECLTYPE_D3DCOLOR;
				d3dVertElem.Usage = D3DDECLUSAGE_COLOR;
				d3dVertElem.UsageIndex = (BYTE)iUnit;
				vertElemArray.push_back(d3dVertElem);
				uIdx++;
			}
		}

		if(bTangent)
		{
			uChannels = inputAttr.GetTangentChannels();
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += uChannels * sizeof(float);
			d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
			d3dVertElem.Usage = D3DDECLUSAGE_TANGENT;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
			uIdx++;
		}

		if(bBitangent)
		{
			uChannels = inputAttr.GetBitangentChannels();
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += uChannels * sizeof(float);
			d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
			d3dVertElem.Usage = D3DDECLUSAGE_BINORMAL;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
		}	

	
	

		if(bBlendWeight)
		{
			uChannels = inputAttr.GetBlendWeightChannels();
			d3dVertElem.Offset = (WORD)uVertexStride;		
			uVertexStride += uChannels * sizeof(float);
			d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
			d3dVertElem.Usage = D3DDECLUSAGE_BLENDWEIGHT;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
			uIdx++;
		}	

		if(bBlendIncidices)
		{
			uChannels = inputAttr.GetBlendIndicesChannels();
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += uChannels * sizeof(float);
			d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
			d3dVertElem.Usage= D3DDECLUSAGE_BLENDINDICES;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
		}

		// nebula에서는 나머지 아래 항목은 지원 안함..

		if(bFog)
		{
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += sizeof(float);
			d3dVertElem.Type = D3DDECLTYPE_FLOAT1;
			d3dVertElem.Usage= D3DDECLUSAGE_FOG;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
			uIdx++;
		}

		if(bPSize)
		{
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += sizeof(float);
			d3dVertElem.Type = D3DDECLTYPE_FLOAT1;
			d3dVertElem.Usage= D3DDECLUSAGE_PSIZE;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
			uIdx++;		
		}	
	}
	else 
	{
	
		if(bPos)
		{
			uChannels = inputAttr.GetPositionChannels();
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += uChannels * sizeof(float);		
			d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
			d3dVertElem.Usage = D3DDECLUSAGE_POSITION;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
			uIdx++;
		}

		if(bBlendWeight)
		{
			uChannels = inputAttr.GetBlendWeightChannels();
			d3dVertElem.Offset = (WORD)uVertexStride;		
			uVertexStride += uChannels * sizeof(float);
			d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
			d3dVertElem.Usage = D3DDECLUSAGE_BLENDWEIGHT;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
			uIdx++;
		}

		if(bNormal)
		{
			uChannels = inputAttr.GetNormalChannels();
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += uChannels * sizeof(float);
			d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
			d3dVertElem.Usage = D3DDECLUSAGE_NORMAL;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
			uIdx++;
		}


		for(iUnit = 0; iUnit < inputAttr.GetMaxColors(); iUnit++)
		{
			if(inputAttr.HasColor(iUnit))
			{
				uChannels = inputAttr.GetColorChannels(iUnit);
				d3dVertElem.Offset = (WORD)uVertexStride;				
				uVertexStride += uChannels * sizeof(float);				
				d3dVertElem.Type = D3DDECLTYPE_D3DCOLOR;
				d3dVertElem.Usage = D3DDECLUSAGE_COLOR;
				d3dVertElem.UsageIndex = (BYTE)iUnit;
				vertElemArray.push_back(d3dVertElem);
				uIdx++;
			}
		}

		if(bFog)
		{
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += sizeof(float);
			d3dVertElem.Type = D3DDECLTYPE_FLOAT1;
			d3dVertElem.Usage= D3DDECLUSAGE_FOG;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
			uIdx++;
		}

		if(bPSize)
		{
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += sizeof(float);
			d3dVertElem.Type = D3DDECLTYPE_FLOAT1;
			d3dVertElem.Usage= D3DDECLUSAGE_PSIZE;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
			uIdx++;		
		}

		if(bBlendIncidices)
		{
			uChannels = inputAttr.GetBlendIndicesChannels();
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += uChannels * sizeof(float);
			d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
			d3dVertElem.Usage= D3DDECLUSAGE_BLENDINDICES;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
		}

		for(iUnit = 0; iUnit < inputAttr.GetMaxTCoords(); iUnit++)
		{
			if(inputAttr.HasTCoord(iUnit))
			{
				uChannels = inputAttr.GetTCoordChannels(iUnit);
				d3dVertElem.Offset = (WORD)uVertexStride;
				uVertexStride += uChannels * sizeof(float);
				d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
				d3dVertElem.Usage= D3DDECLUSAGE_TEXCOORD;
				d3dVertElem.UsageIndex = (BYTE)iUnit;
				vertElemArray.push_back(d3dVertElem);
				uIdx++;
			}
		}

		if(bTangent)
		{
			uChannels = inputAttr.GetTangentChannels();
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += uChannels * sizeof(float);
			d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
			d3dVertElem.Usage = D3DDECLUSAGE_TANGENT;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
			uIdx++;
		}

		if(bBitangent)
		{
			uChannels = inputAttr.GetBitangentChannels();
			d3dVertElem.Offset = (WORD)uVertexStride;
			uVertexStride += uChannels * sizeof(float);
			d3dVertElem.Type = (BYTE)(D3DDECLTYPE_FLOAT1 + uChannels - 1);
			d3dVertElem.Usage = D3DDECLUSAGE_BINORMAL;
			d3dVertElem.UsageIndex = 0;
			vertElemArray.push_back(d3dVertElem);
		}
	}

	D3DVERTEXELEMENT9 sentinelElem = D3DDECL_END();
	vertElemArray.push_back(sentinelElem);

	HRESULT hr;
	LPDIRECT3DVERTEXDECLARATION9 PVertDecl = 0;
	hr = m_pkD3DDevice9->CreateVertexDeclaration(&vertElemArray[0], 
		&PVertDecl);
	U2ASSERT(SUCCEEDED(hr));
	

	return PVertDecl;	
}




void U2Dx9VertexBufferMgr::GenerateVertexParams(U2MeshData* pData,   unsigned int& uiVertexTypeDesc, 
										  unsigned int& uiVertexStride, 
					unsigned int& uiPosOffset, unsigned int& uiWeightOffset, 
					unsigned int& uiNormOffset, unsigned int& uiColorOffset, 
					unsigned int& uiTexOffset)
{
	
	uiVertexTypeDesc = 0;
	uiVertexStride =0;

	unsigned short usBones = 0;
	
	uiPosOffset = uiVertexStride;
	uiVertexStride += 3 * sizeof(float);
	uiWeightOffset = uiVertexStride;

	if(usBones == 0)
	{
		uiVertexTypeDesc |= D3DFVF_XYZ;		
	}
	else 
	{
		uiVertexTypeDesc = D3DFVF_XYZB3;
		uiVertexStride +=  3 * sizeof(float);
		

	}

	if(pData->GetNormals())
	{
		uiVertexTypeDesc |= D3DFVF_NORMAL;
		uiNormOffset = uiVertexStride;
		uiVertexStride += 3 * sizeof(float);
		
	}

	if(pData->GetColors())
	{
		uiVertexTypeDesc |= D3DFVF_DIFFUSE;
		uiColorOffset = uiVertexStride;
		uiVertexStride += sizeof(unsigned int);		
	}

	unsigned int uTextureSets = pData->GetTexCoordSetCnt();
	if(uTextureSets == 0)
	{
		uTextureSets = 1;
	}
	else if(uTextureSets > U2Dx9Renderer::GetMaxTextureBlendStages())
	{
		uTextureSets = U2Dx9Renderer::GetMaxTextureBlendStages();
	}

	if(uTextureSets)
	{
		uiVertexTypeDesc |= uTextureSets << D3DFVF_TEXCOUNT_SHIFT;
		uiTexOffset = uiVertexStride;
		uiVertexStride += 2 * sizeof(float) * uTextureSets;
		

	}	
	
}

void* U2Dx9VertexBufferMgr::LockVB(LPDIRECT3DVERTEXBUFFER9 pkVB, unsigned int uiOffset, 
			 unsigned int uiSize, unsigned int uiLockFlags)
{
	void* pvData = NULL;
	if(FAILED(pkVB->Lock(uiOffset, uiSize, &pvData, uiLockFlags)))
	{
		return NULL;
	}

	U2ASSERT(m_pLockedBuffer == NULL);
	m_pLockedBuffer = pvData;
	m_uLockedBufferSize = uiSize;

	if(m_uiTempBufferSize < uiSize)
	{
		U2_FREE(m_pvTempBuffer);
		m_pvTempBuffer = NULL;
		m_pvTempBuffer = U2_ALLOC(char, uiSize);
		m_uiTempBufferSize = uiSize;
	}

	if((uiLockFlags & (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)) == 0)
	{
		memcpy_s(m_pvTempBuffer, uiSize, pvData, uiSize);
	}

	return m_pvTempBuffer;
}


void U2Dx9VertexBufferMgr::PackVB(unsigned char* pucDestBuffer, unsigned char* pucSrcBuffer, 
			unsigned int uiVertexStride, unsigned int uiDataSize, 
			unsigned int uiNumVerts)
{
	for(uint32 i=0; i < uiNumVerts; ++i)	{
		
		memcpy_s(pucDestBuffer, uiDataSize, pucSrcBuffer, uiDataSize);
		pucDestBuffer += uiVertexStride;
		pucSrcBuffer += uiDataSize;
	}
}


bool U2Dx9VertexBufferMgr::UnlockVB(LPDIRECT3DVERTEXBUFFER9 pkVB)
{
	U2ASSERT(m_pLockedBuffer);

	memcpy_s(m_pLockedBuffer, m_uLockedBufferSize, m_pvTempBuffer, m_uLockedBufferSize);
	m_pLockedBuffer = NULL;
	m_uLockedBufferSize = 0;

	return SUCCEEDED(pkVB->Unlock());
}


LPDIRECT3DVERTEXBUFFER9 U2Dx9VertexBufferMgr::CreateVertexBuffer(
	unsigned int uiLength, unsigned int uiUsage, unsigned int uiFVF, 
	D3DPOOL ePool, D3DVERTEXBUFFER_DESC* pkDesc)
{
	LPDIRECT3DVERTEXBUFFER9 pkVB9 = NULL;

	U2ASSERT(m_pkD3DDevice9);

	HRESULT eD3dRet = m_pkD3DDevice9->CreateVertexBuffer(uiLength, uiUsage, 
		uiFVF, ePool, &pkVB9, NULL);
	if (SUCCEEDED(eD3dRet))
	{
		if (pkDesc)
		{
			pkVB9->GetDesc(pkDesc);
		}		
	}
	else
	{		
		pkVB9 = NULL;
	}

	return pkVB9;
}


void U2Dx9VertexBufferMgr::DestoryBuffers()
{

}
