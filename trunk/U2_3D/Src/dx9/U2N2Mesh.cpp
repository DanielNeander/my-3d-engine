#include <U2_3D/Src/U23DLibPCH.h>
#include "U2N2Mesh.h"
#include <U2_3D/Src/Dx9/U2Dx9Renderer.h>
#include <U2_3D/src/Dx9/U2Dx9VertexBufferMgr.h>

IMPLEMENT_RTTI(U2N2Mesh, U2Triangle);

//-------------------------------------------------------------------------------------------------
U2N2Mesh::U2N2Mesh(U2TriangleData *pData)
:U2Triangle(pData),
m_uIbSize(0),
m_uVbSize(0),
m_pvVretexBuffer(0),
m_pvIndexBuffer(0),
m_pVB(0),
m_iVertexUsage(WriteOnce),
m_iIndexUsage(WriteOnce),
m_iVertexCompMask(0),
m_dwD3DIBLockFlags(0),
m_dwD3DVBLockFlags(0),
m_meshGroups(0),
m_usNumEdges(0),
m_pEdgeBuffer(0),
m_uiEbSize(0)
{
	FILE_LOG(logINFO) << _T("N2Mesh Created"); 
}

//-------------------------------------------------------------------------------------------------
U2N2Mesh::~U2N2Mesh()
{	
	m_meshGroups.RemoveAll();
	SAFE_RELEASE(m_pVB);	
	FILE_LOG(logINFO) << _T("N2Mesh Destroyed"); 
}

//-------------------------------------------------------------------------------------------------
void U2N2Mesh::UpdateWorldData(float fAppTime)
{
	U2Mesh::UpdateWorldData(fAppTime);
	m_bbox.Transform(m_matHWorld);
}

//-------------------------------------------------------------------------------------------------
bool U2N2Mesh::CreatevertexBuffer()
{
	U2ASSERT(m_uVbSize > 0);
	U2ASSERT(0 == m_pvVretexBuffer);
	U2ASSERT(0 == m_pVB);

	if(ReadOnly & m_iVertexUsage)
	{
		// this is a read-only mesh which will never be rendered
		// and only read-accessed by the CPU, allocate private
		// vertex buffer
		m_pvVretexBuffer = U2_MALLOC(m_uVbSize);
		U2ASSERT(m_pvVretexBuffer);
		return true;
	}
	else 
	{
		U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
		U2ASSERT(pRenderer->GetD3DDevice());

		DWORD d3dUsage = D3DUSAGE_WRITEONLY;
		D3DPOOL d3dPool = D3DPOOL_MANAGED;
		m_dwD3DVBLockFlags = 0;
		if(WriteOnly & m_iVertexUsage)
		{
			d3dUsage = (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY);
			d3dPool = D3DPOOL_DEFAULT;
			m_dwD3DVBLockFlags = D3DLOCK_DISCARD;
		}
		if(ReadWrite & m_iVertexUsage)
		{
			d3dUsage = D3DUSAGE_DYNAMIC;
			d3dPool = D3DPOOL_SYSTEMMEM;
		}

		if(RTPatch & this->m_iVertexUsage)
		{
			d3dUsage |= D3DUSAGE_RTPATCHES;
		}
		if(PointSprite & this->m_iVertexUsage)
		{
			d3dUsage |= D3DUSAGE_POINTS;
		}

		m_pVB = pRenderer->GetVBMgr()->CreateVertexBuffer(m_uVbSize, d3dUsage, 
			0, d3dPool);
		/*pRenderer->GetD3DDevice()->CreateVertexBuffer(m_uVbSize, d3dUsage,
			0, d3dPool, &m_pVB, NULL);*/
		U2ASSERT(m_pVB);

		if(m_pVB)
			return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
bool U2N2Mesh::CreateIndexBuffer( )
{
	U2ASSERT(m_uIbSize > 0);
	U2ASSERT(0 == m_pvIndexBuffer);
	U2ASSERT(0 == m_spModelData->GetIB());

	if(ReadOnly & m_iVertexUsage)
	{
		// this is a read-only mesh which will never be rendered
		// and only read-accessed by the CPU, allocate private
		// vertex buffer
		m_pvIndexBuffer = U2_MALLOC(m_uIbSize);
		U2ASSERT(m_pvIndexBuffer);
		return true;
	}
	else 
	{
		U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
		U2ASSERT(pRenderer->GetD3DDevice());

		DWORD d3dUsage = D3DUSAGE_WRITEONLY;
		D3DPOOL d3dPool = D3DPOOL_MANAGED;
		m_dwD3DIBLockFlags = 0;
		if(WriteOnly & m_iIndexUsage)
		{
			d3dUsage = (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY);
			d3dPool = D3DPOOL_DEFAULT;
			m_dwD3DIBLockFlags = D3DLOCK_DISCARD;
		}
		if(ReadWrite & m_iIndexUsage)
		{
			d3dUsage = D3DUSAGE_DYNAMIC;
			d3dPool = D3DPOOL_SYSTEMMEM;
		}

		if(RTPatch & this->m_iIndexUsage)
		{
			d3dUsage |= D3DUSAGE_RTPATCHES;
		}
		if(PointSprite & this->m_iIndexUsage)
		{
			d3dUsage |= D3DUSAGE_POINTS;
		}

		IDirect3DIndexBuffer9* pIB = pRenderer->GetIBMgr()->CreateIndexBuffer(
			m_uIbSize, d3dUsage, D3DFMT_INDEX16, d3dPool);
		U2ASSERT(pIB);

		U2ASSERT(m_spModelData->GetTriCount() > 0);
		m_spModelData->SetIB(pIB, m_uIbSize, m_spModelData->GetTriCount() * 3);

		if(m_spModelData)
			return true;
	}

	return false;

}

bool U2N2Mesh::CreateEmptyBuffers()
{
	U2ASSERT(m_spModelData);
	
	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();

	if ( NULL == m_spModelData->GetVertexDeclaration() )
	{
		U2VertexAttributes* pVertexAttb = this->CreateVertexAttrib(m_iVertexCompMask);
		U2ASSERT(pVertexAttb);
		m_spModelData->SetVertexAttbs(pVertexAttb);
		
		// m_uiVertexStride는 SetVertexComponent 호출시 계산.
		// 쉐이더의 버텍스 포멧에 맞게 데이터를 잘 입력하도록 주의한다...
		// Nebula Device2 Vertex Decl Format 
		LPDIRECT3DVERTEXDECLARATION9 pVertDecl = pRenderer->GetVBMgr()->
			GenerateShaderVertexParams(*pVertexAttb, m_uiVertexStride, true);
		m_spModelData->SetVertexDeclaration(pVertDecl);
	}
	
	if(GetVertexCount() > 0)
	{	
		int vbBytesInSize = GetActiveVertexCount() * GetVertexStride();
		SetVertexBufferByteSize(vbBytesInSize);
		CreatevertexBuffer();
	}

	if(m_spModelData->GetTriCount() > 0)
	{	
		// trianglelist 만 현재 지원
		
		int ibBytesInSize;

		switch(m_spModelData->GetPrimitiveType())
		{
		case D3DPT_TRIANGLELIST:
			ibBytesInSize= GetActiveTriangleCount() * 3 * sizeof(uint16);
			break;
		case D3DPT_TRIANGLESTRIP:
			ibBytesInSize = m_spModelData->GetArrayLengths()[0] * sizeof(uint16);
			break;
		}			
		
				
		SetIndexBufferByteSize(ibBytesInSize);
		CreateIndexBuffer();
	}

	if(this->GetNumEdges() > 0)
	{
		int edgeSize = GetNumEdges() * sizeof(Edge);
		SetEdgeBufferSize(edgeSize);
		CreateEdgeBuffer();
	}

	return true;	
}



//-------------------------------------------------------------------------------------------------
bool U2N2Mesh::LoadFile(U2MeshLoader* pLoader)
{
	if(!pLoader)
		return false;

	if(!pLoader->Open())
	{
		U2ASSERT(false);
		FDebug("Failed to open file '%s'!\n", pLoader->GetFilename());
		return false;
	}

	//m_spModelData->SetMeshState(U2MeshData::DYNAMIC_MESH);

	// 현재는 nMeshGroup에 정보를 전달하는데 목적을 두고 있음.. 
	// 그러므로 그리지는 않음..	
	m_eCulling = CULL_ALWAYS;

	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();

	this->SetBoundingBox(pLoader->GetBoundingBox());

	CreateMeshGroups(pLoader->GetNumGroups());

	m_spModelData->SetVertexCount(pLoader->GetNumVertices(), pLoader->GetNumVertices());	
	m_spModelData->SetVertexStride(0, pLoader->GetVertexChannelCnt() * sizeof(float));
	m_iVertexCompMask = AllComponents & pLoader->GetVertexComponents();
	
	// 
	//m_spModelData->SetVertexStride(0, uiVertexStride);

	// Generate Vertex Attribute from vertex component
	U2VertexAttributes* pVertexAttb = CreateVertexAttrib(m_iVertexCompMask);
	U2ASSERT(pVertexAttb);
	m_spModelData->SetVertexAttbs(pVertexAttb);

	// 쉐이더의 버텍스 포멧에 맞게 데이터를 잘 입력하도록 주의한다...
	LPDIRECT3DVERTEXDECLARATION9 pVertDecl = pRenderer->GetVBMgr()->
		GenerateShaderVertexParams(*pVertexAttb, m_uiVertexStride, true);

	m_spModelData->SetVertexDeclaration(pVertDecl);

	uint32 groupIdx;
	uint32 numGroups = pLoader->GetNumGroups();

	// 차후 nMeshGroup는 변경 삭제 될 수 있음.	
	for(groupIdx = 0; groupIdx < numGroups; ++groupIdx)
	{
		nMeshGroup& group = Group(groupIdx);
		// MeshLoader로부터 정보를 읽어온다.
		group = pLoader->GetMeshGroup(groupIdx);
		group.SetName(pLoader->GetMeshGroup(groupIdx).GetName());
		group.SetMesh(this);
		U2TriListData *pModelData = U2_NEW U2TriListData();
		group.SetModelData(pModelData);				
		pModelData->SetVertexCount(group.GetNumVertices(), group.GetNumVertices());
		pModelData->CreateVertices(); // bounding volume generate		
		uint16 numTris = group.GetNumIndices() / 3;
		pModelData->SetActiveTriangleCount(numTris);		
		//FILE_LOG(logDEBUG) << pLoader->GetMeshGroup(groupIdx).GetName().Str();
	}

	

	m_spModelData->SetIndices(pLoader->GetNumTriangles(), pLoader->GetNumTriangles(), 
		NULL, NULL, 1);

	int vbSize = pLoader->GetNumVertices() * GetVertexStride();

	// TriList
	int numIndices= pLoader->GetNumTriangles() * 3;

	int ibSize = numIndices * sizeof(unsigned short);
	SetVertexBufferByteSize(vbSize);
	SetIndexBufferByteSize(ibSize);
	CreatevertexBuffer();
	CreateIndexBuffer();

	float* pVBData = (float*)pRenderer->GetVBMgr()->LockVB(
		m_pVB, 0, vbSize, m_dwD3DVBLockFlags);
	unsigned short* pIBData = pRenderer->GetIBMgr()->LockIB(
		m_spModelData->GetIB(), 0, ibSize, m_dwD3DIBLockFlags);

	bool res = pLoader->ReadVertices(pVBData, vbSize);
	U2ASSERT(res);


	// Calculate Vertex Channel count 
	unsigned int uiVertexChannelCnt = U2XMeshLoader::GetVertexChannelCntFromMask(m_iVertexCompMask);	
	U2ASSERT(uiVertexChannelCnt == pLoader->GetVertexChannelCnt());
	
	// 수정해야 할 코드 
	unsigned int i;
	m_spModelData->CreateVertices();	
	unsigned int numVerts = m_spModelData->GetVertexCount();
	for(i=0; i < numVerts; i++)
	{
		m_spModelData->GetVertices()[i].x = pVBData[0];
		m_spModelData->GetVertices()[i].y = pVBData[1];
		m_spModelData->GetVertices()[i].z = pVBData[2];
		pVBData += uiVertexChannelCnt;				
	}

	
	res = pLoader->ReadIndices(pIBData, ibSize);
	U2ASSERT(res);

	// Trinagle list
	m_spModelData->SetIndices(numIndices, numIndices,
		pIBData, 0, 1);

	// UpdateBoundingBox
	UpdateGroupBBoxes(pVBData, pIBData);
	
	pRenderer->GetVBMgr()->UnlockVB(m_pVB);
	pRenderer->GetIBMgr()->UnlockIB(m_spModelData->GetIB());

	// 현재 Bounding Volume을 추가하기 위해 필요함..
	for(i=0; i < GetNumGroups(); ++i)
	{
		nMeshGroup& group = Group(i);
		numVerts = group.GetMeshData()->GetActiveVertexCount();
		for(unsigned int j = group.GetFirstVertex(), index = 0;index < numVerts; ++index, j++)
		{
			group.GetMeshData()->GetVertices()[index] = m_spModelData->GetVertices()[j];
		}
		//int firstIndex = group.GetFirstIndex();
		uint16* pIndices = U2_ALLOC(uint16, numIndices);
		size_t uiByteSize = numIndices * sizeof(uint16);
		memcpy_s(pIndices, uiByteSize, pIBData, uiByteSize);
		group.GetMeshData()->SetIndexArray(pIndices);
	}

	pLoader->Close();


	return true;
}

float* U2N2Mesh::LockVertices()
{
	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();

	float* pVBData = 0;
	if(m_pVB)
	{
		pVBData = (float*)pRenderer->GetVBMgr()->LockVB(
			m_pVB, 0, m_uVbSize, m_dwD3DVBLockFlags);
		/*HRESULT hr = m_pVB->Lock(0, 0, &pvData, m_dwD3DVBLockFlags);
		pVBData = (float*)pvData;*/
	}
	
	return pVBData;
}

void U2N2Mesh::UnlockVertices()
{
	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();
	pRenderer->GetVBMgr()->UnlockVB(m_pVB);	
	//m_pVB->Unlock();
}

uint16* U2N2Mesh::LockIndices()
{
	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();
	unsigned short* pIBData = 0;
	if(m_spModelData->GetIB())
	{
		pIBData = pRenderer->GetIBMgr()->LockIB(
			m_spModelData->GetIB(), 0, m_uIbSize, m_dwD3DIBLockFlags);	
		/*VOID *pvData;
		HRESULT hr = m_spModelData->GetIB()->Lock(0, 0, &pvData, m_dwD3DIBLockFlags);
		U2ASSERT(pvData);

		pIBData = (uint16*)pvData;*/
	}

	return pIBData;
}

void U2N2Mesh::UnlockIndices()
{
	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();
	pRenderer->GetIBMgr()->UnlockIB(m_spModelData->GetIB());
}

//-------------------------------------------------------------------------------------------------
U2VertexAttributes* U2N2Mesh::CreateVertexAttrib(unsigned int compMask)
{
	U2VertexAttributes* attr = U2_NEW U2VertexAttributes;	

	if(compMask & U2MeshLoader::Coord)
	{	
		attr->SetPositionChannels(3);	
	}
	else if(compMask & U2MeshLoader::Coord4)
	{
		attr->SetPositionChannels(4);	
	}

	if(compMask & U2MeshLoader::Normal)
	{
		attr->SetNormalChannels(3);
	}	

	if(compMask & U2MeshLoader::Color)
	{
		attr->SetColorChannels(0, 4);
	}

	if(compMask & U2MeshLoader::Uv0)
	{
		attr->SetTCoordChannels(0, 2);	
	}

	if(compMask & U2MeshLoader::Uv1)
	{
		attr->SetTCoordChannels(1, 2);	
	}

	if(compMask & U2MeshLoader::Uv2)
	{
		attr->SetTCoordChannels(2, 2);	
	}

	if(compMask & U2MeshLoader::Uv3)
	{
		attr->SetTCoordChannels(2, 2);	
	}

	if(compMask & U2MeshLoader::Binormal)
	{
		attr->SetBitangentChannels(3);
	}

	if(compMask & U2MeshLoader::Tangent)
	{
		attr->SetTangentChannels(3);
	}

	if(compMask & U2MeshLoader::Weights)
	{
		attr->SetBlendWeightChannels(4);
	}

	if(compMask & U2MeshLoader::JIndices)
	{
		attr->SetBlendIndicesChannels(4);
	}

	return attr;
}

void U2N2Mesh::UpdateGroupBBoxes(float* pVertsData, uint16* pIndicesData)
{
	U2Aabb groupBox;
	uint32 groupIdx;
	for(groupIdx = 0; groupIdx < m_uNumGroups;  ++groupIdx)
	{
		groupBox.BeginExtend();
		nMeshGroup& group = Group(groupIdx);
		uint16* pIndicesBase =pIndicesData + group.GetFirstIndex();
		int i;
		for(i = 0; i < group.GetNumIndices(); ++i)
		{
			float* pVertsBase = pVertsData + (pIndicesBase[i] * 
				m_spModelData->GetVertexStride(0));
			groupBox.Extend(pVertsBase[0], pVertsBase[1], pVertsBase[2]);
		}
		group.SetBoundingBox(groupBox);
	}
}


//-------------------------------------------------------------------------------------------------
void U2N2Mesh::GetTriangle(uint16 usTriangle, D3DXVECTOR3*& pT0, 
						 D3DXVECTOR3*& pT1, D3DXVECTOR3*& pT2)
{
	U2TriListData* pModelData = 
		SmartPtrCast(U2TriListData, m_spModelData);
	U2ASSERT(usTriangle < pModelData->GetTriangleCnt());
	D3DXVECTOR3* pVertex = pModelData->GetVertices();
	uint16* pusTriList = pModelData->GetTriList();

	uint32 uStart = 3 *usTriangle;
	pT0 = &pVertex[pusTriList[uStart++]];
	pT1 = &pVertex[pusTriList[uStart++]];
	pT2 = &pVertex[pusTriList[uStart]];
}

//-------------------------------------------------------------------------------------------------
void U2N2Mesh::ApplyShader(U2SceneMgr* pSceneMgr)
{
	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);

	for(uint32 i=0; i < GetEffectCnt(); ++i)
	{
		U2Dx9FxShaderEffect* pEffect = DynamicCast<U2Dx9FxShaderEffect>(GetEffect(i));
		if(pEffect)
		{
			pEffect->GetEffectShader()->SetAttribute(pEffect->GetShaderAttb());

			pRenderer->SetCurrEffectShader(pEffect->GetEffectShader());			
			break;

		}
	}	
}

//-------------------------------------------------------------------------------------------------
void U2N2Mesh::ApplyMesh(U2SceneMgr* pSceneMgr, const nMeshGroup& curGroup)
{
	IsRendererOK;

	for(uint32 i=0; i < curGroup.GetEffectCnt(); ++i)
	{
		const U2Dx9FxShaderEffect* pEffect = DynamicCast<U2Dx9FxShaderEffect>(curGroup.GetEffect(i));
		if(pEffect)
		{
			pRenderer->SetupMesh(this);			
			//const nMeshGroup& curGroup = Group(m_uiGroupIdx);
			pRenderer->SetVertexRange(curGroup.GetFirstVertex(), curGroup.GetNumVertices());
			pRenderer->SetIndexRange(curGroup.GetFirstIndex(), curGroup.GetNumIndices());
		}
	}
}

//-------------------------------------------------------------------------------------------------
void U2N2Mesh::UpdateEffectShader(U2SceneMgr* pSceneMgr, U2RenderContext* renderContext)
{

	for(uint32 i=0; i < GetEffectCnt(); ++i)
	{
		U2Dx9FxShaderEffect* pEffect = DynamicCast<U2Dx9FxShaderEffect>(GetEffect(i));
		if(pEffect)
		{
			pEffect->GetEffectShader()->SetAttribute(renderContext->GetShaderOverides());
			break;
		}
	}
}

//-------------------------------------------------------------------------------------------------
void U2N2Mesh::Render(U2SceneMgr* pSceneMgr, U2RenderContext* pRenderContext)
{
	IsRendererOK;	
	pRenderer->DrawIndexedPrimitive(m_spModelData->GetPrimitiveType());
}

//-------------------------------------------------------------------------------------------------
U2N2Mesh::Edge* U2N2Mesh::GetEdges()
{
	return m_pEdgeBuffer;
}

//-------------------------------------------------------------------------------------------------
void U2N2Mesh::CreateEdgeBuffer()
{
	if(!m_pEdgeBuffer)
		m_pEdgeBuffer = U2_ALLOC(Edge, m_usNumEdges);
	U2ASSERT(m_pEdgeBuffer);	
}



