#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9StaticShadowCaster.h"
#include "U2Dx9ShadowMgr.h"

//-------------------------------------------------------------------------------------------------
U2Dx9StaticShadowCaster::U2Dx9StaticShadowCaster()
:m_spMesh(0)
{

}

//-------------------------------------------------------------------------------------------------
U2Dx9StaticShadowCaster::~U2Dx9StaticShadowCaster()
{

}

//-------------------------------------------------------------------------------------------------
bool U2Dx9StaticShadowCaster::LoadResource()
{
	U2ASSERT(!m_spShadowVolume);	
	
	U2TriListData* pData = U2_NEW U2TriListData();
	m_spMesh = U2_NEW U2N2Mesh(pData);
	
	TCHAR fullPath[MAX_PATH];
	_tcscpy_s(fullPath,  MAX_PATH, m_szName.Str());
	GetPath( fullPath, MAX_PATH);
	fullPath[MAX_PATH-1] = '\0';	

	// StackString Memory Leak...
	
	// 리소스 매니저가 필요하다.
	

	U2XMeshLoaderPtr spMeshLoader = U2_NEW U2XMeshLoader;

	spMeshLoader->SetFilename(fullPath);		

	if(spMeshLoader->Open())
	{
		m_spMesh->SetBoundingBox(spMeshLoader->GetBoundingBox());
		m_spMesh->SetVertexUsage(U2N2Mesh::WriteOnce | U2N2Mesh::NeedsVertexShader);
		m_spMesh->SetIndexUsage(U2N2Mesh::WriteOnly);
		m_spMesh->SetVertexComponents(U2N2Mesh::Coord4);
		pData->SetVertexCount(spMeshLoader->GetNumVertices() * 2, spMeshLoader->GetNumVertices() * 2);
		pData->SetIndices(spMeshLoader->GetNumTriangles(), spMeshLoader->GetNumTriangles(), 
			NULL, NULL, 1);
		m_spMesh->SetNumEdges(spMeshLoader->GetNumEdges());
		uint32 numGroups = spMeshLoader->GetNumGroups();
		m_spMesh->CreateMeshGroups(numGroups);

		uint32 groupIdx;
		// 차후 nMeshGroup는 변경 삭제 될 수 있음.	
		for(groupIdx = 0; groupIdx < numGroups; ++groupIdx)
		{
			nMeshGroup& group = m_spMesh->Group(groupIdx);			
			group = spMeshLoader->GetMeshGroup(groupIdx);
			group.SetFirstVertex(group.GetFirstVertex() * 2);
			group.SetNumVertices(group.GetNumVertices() * 2);
			group.SetName(spMeshLoader->GetMeshGroup(groupIdx).GetName());			
			//group.SetMesh(m_spMesh);
			//U2TriListData *pModelData = U2_NEW U2TriListData();
			//group.SetModelData(pModelData);				
			//pModelData->SetVertexCount(group.GetNumVertices() * 2, group.GetNumVertices() * 2);
			//pModelData->CreateVertices(); // bounding volume generate		
			//FILE_LOG(logDEBUG) << pLoader->GetMeshGroup(groupIdx).GetName().Str();
		}
		m_spMesh->CreateEmptyBuffers();

		 // read vertices into temporary buffer and expand them into mesh
		uint32 numVertices = spMeshLoader->GetNumVertices();
		uint32 vertexChannelCnt = spMeshLoader->GetVertexChannelCnt();
		uint32 srcVBSize = numVertices * vertexChannelCnt * sizeof(float);		
		float* pSrcVBData = U2_ALLOC(float, numVertices * vertexChannelCnt);

		spMeshLoader->ReadVertices(pSrcVBData, srcVBSize);		

		// write extruded and un-extruded vertices into destination vertex buffer
		float* pDstBuffer = m_spMesh->LockVertices();		
		
		uint32 vertexIdx;
		for(vertexIdx = 0; vertexIdx < numVertices; vertexIdx++)
		{
			uint32 curOffset = vertexIdx * vertexChannelCnt;

			// write unextruded vertex
			*pDstBuffer++ = pSrcVBData[curOffset];
			*pDstBuffer++ = pSrcVBData[curOffset + 1];
			*pDstBuffer++ = pSrcVBData[curOffset + 2];
			*pDstBuffer++ = 0.0f;

			// write extruded vertex
			*pDstBuffer++ = pSrcVBData[curOffset];
			*pDstBuffer++ = pSrcVBData[curOffset + 1];
			*pDstBuffer++ = pSrcVBData[curOffset + 2];
			*pDstBuffer++ = 1.0f;
		}
		m_spMesh->UnlockVertices();
		U2_FREE(pSrcVBData);
		pSrcVBData = NULL;						

		// read indices directly, and fix them for the new vertex layout
		uint16* pIndices = m_spMesh->LockIndices();
		spMeshLoader->ReadIndices(pIndices, m_spMesh->GetIndexBufferByteSize());
		uint32 idx;
		uint32 numIndices = m_spMesh->GetActiveTriangleCount() * 3;
		for(idx = 0; idx < numIndices; ++idx)
		{
			pIndices[idx] *= 2;
		}
		m_spMesh->UnlockIndices();
		pIndices = 0;

		U2N2Mesh::Edge* pEdges = m_spMesh->GetEdges();
		spMeshLoader->ReadEdges(pEdges, m_spMesh->GetEdgeBufferSize());
		uint32 edgeIdx;
		uint32 numEdges = m_spMesh->GetNumEdges();
		for(edgeIdx = 0; edgeIdx < numEdges; ++edgeIdx)
		{
			pEdges[edgeIdx].vertexIdx[0] *= 2;
			pEdges[edgeIdx].vertexIdx[1] *= 2;
		}		
		pEdges = 0;

		spMeshLoader->Close();
	}
	else 
	{
		U2ASSERT(false);
		FDebug("Error Loading Shadow Mesh");
		return false;
	}

	CreateShadowVolume(m_spMesh);

	return true;	
}

//-------------------------------------------------------------------------------------------------
void U2Dx9StaticShadowCaster::UnloadResource()
{
	m_spMesh = 0;

	DestroyShadowVolume();
}

//-------------------------------------------------------------------------------------------------
/**
Setup the shadow volume for this caster. This method is called by
nShadowServer2 right before rendering the shadow volume. Implement this
method in a derived class.
*/
void U2Dx9StaticShadowCaster::SetupShadowVolume(const U2Light& light, 
												 const D3DXMATRIX& invModelLitMat)
{
	U2Dx9ShadowMgr* pShadowMgr = U2Dx9ShadowMgr::Instance();

	 // split faces into lit and unlit
	if(LightingChanged(light, invModelLitMat))
	{
		UpdateFaceLitFlags(light, invModelLitMat);
	}

	if ( pShadowMgr->GetDepthFail())
	{
		BeginWriteIndices(m_spMesh);
		WriteSideIndices();
		WriteCapIndices(true);
		WriteCapIndices(false);
		EndWriteIndices();
	}
	else 
	{
		BeginWriteIndices(m_spMesh);
		WriteSideIndices();
		EndWriteIndices();
	}
	
	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	pRenderer->SetupMesh(m_spMesh, m_spShadowVolume);
	const nMeshGroup& meshGroup = m_spMesh->Group(m_uiMeshGroupIdx);
	pRenderer->SetVertexRange(meshGroup.GetFirstVertex(), meshGroup.GetNumVertices());
	pRenderer->SetIndexRange(0, GetNumDrawIndices());

}

//-------------------------------------------------------------------------------------------------
void U2Dx9StaticShadowCaster::RenderShadowVolume()
{
	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	pRenderer->DrawIndexedPrimitive(D3DPT_TRIANGLELIST);

}

