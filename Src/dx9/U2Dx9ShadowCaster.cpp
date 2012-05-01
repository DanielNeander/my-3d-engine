#include <U2_3D/src/U23DLibPch.h>
#include "U2Dx9ShadowCaster.h"
#include "nMeshGroup.h"

//-------------------------------------------------------------------------------------------------
U2Dx9ShadowCaster::U2Dx9ShadowCaster()
:m_pIndicesStart(0),
m_pIndicesCurrent(0),
m_uiDrawNumIndices(0),
m_pSrcMesh(0),
m_pSrcIndices(0),
m_pSrcEdges(0),
m_ePrevLitType(U2Light::LT_MAX_COUNT),
m_spShadowVolume(0)
{
	
}

//-------------------------------------------------------------------------------------------------
U2Dx9ShadowCaster::~U2Dx9ShadowCaster()
{

}


//-------------------------------------------------------------------------------------------------
bool U2Dx9ShadowCaster::CreateShadowVolume(U2N2Mesh* pMesh)
{
	const nMeshGroup& meshGroup = pMesh->Group(m_uiMeshGroupIdx);
	
	m_faces.SetSize(meshGroup.GetNumIndices() / 3);
	for(unsigned int i=0; i < m_faces.Size(); ++i)
	{
		Face* pEmptyFace = U2_NEW Face;
		pEmptyFace->bLit = false;
		m_faces[i] = pEmptyFace;
	}

	// create the face normals
	UpdateFaceNormalsAndMidpoints(pMesh);

	// allocate the shared index buffer for the dark cap, light cap and shadow volume indices
	// we assume that in the worst case there will never be more then 4 times the original
	// indices required for rendering the shadow volume

	U2TriListData* pData = U2_NEW U2TriListData();
	m_spShadowVolume = U2_NEW U2N2Mesh(pData);
	U2ASSERT(m_spShadowVolume);

	m_spShadowVolume->SetUsages(U2N2Mesh::WriteOnly | U2N2Mesh::NeedsVertexShader);
	m_spShadowVolume->SetVertexComponents(0);	
	pData->SetVertexCount(0, 0);
	uint16 activeTriCnt =pMesh->GetActiveTriangleCount() * 4;
	pData->SetIndices(activeTriCnt, activeTriCnt, 0, 0, 1);
	m_spShadowVolume->SetNumEdges(0);
	m_spShadowVolume->CreateEmptyBuffers();
	
	return true;
}

//-------------------------------------------------------------------------------------------------
void U2Dx9ShadowCaster::DestroyShadowVolume()
{
	unsigned int size = m_faces.Size();
	for(unsigned int i=0; i < size ; ++i)
	{
		U2_DELETE m_faces[i];
	}
	
	m_faces.SetSize(0);

	m_spShadowVolume = 0;
}

//-------------------------------------------------------------------------------------------------
/// update the face normals and midpoints from a mesh object
void U2Dx9ShadowCaster::UpdateFaceNormalsAndMidpoints(U2N2Mesh* pMesh)
{
	const nMeshGroup& meshGroup = pMesh->Group(m_uiMeshGroupIdx);

	// mesh가 Vertex, Index 데이터를 공유하는 상황이다.. 
	// 이 방식으로 가는게 좋을지.. 
	// 현재는 max에서 이렇게 데이터를 export하지만 nMeshGroup을 생성하는 
	// 방법을 적용해볼 것도 연구해 볼 필요가있음.
	uint16 startIdx = meshGroup.GetFirstIndex();
	float* pVertices = pMesh->LockVertices();
	uint16* pIndices = pMesh->LockIndices();
	unsigned int numFaces = m_faces.Size();
	// GetVertexStride의 이름을 GetVertexChannlCnt로 변경할 부분이 있음..
	// Visaul Assist의 오류로 나중에 바꿈.
	// 여기서 GetVertexStride 함수는 U2MeshData의 함수와 혼동하지 않도록 주의
	unsigned int vertexChannelCnt = pMesh->GetVertexStride() / sizeof(float);
	U2ASSERT(meshGroup.GetNumIndices() == (numFaces * 3));
	uint32 faceIdx;

	// 임시 변수 제거..
	for(faceIdx = 0; faceIdx < numFaces; ++faceIdx)
	{
		uint32 idx = startIdx + faceIdx * 3;
		const D3DXVECTOR3& v0 = *(D3DXVECTOR3*)(pVertices + pIndices[idx] * vertexChannelCnt);
		const D3DXVECTOR3& v1 = *(D3DXVECTOR3*)(pVertices + pIndices[idx+1] * vertexChannelCnt);
		const D3DXVECTOR3& v2 = *(D3DXVECTOR3*)(pVertices + pIndices[idx+2] * vertexChannelCnt);
		//U2DynString szLog;
		//szLog.Format(_T("v0 : %f, %f, %f v1 : %f, %f, %f v2: %f, %f, %f\n"),
		//	v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
		//FILE_LOG(logDEBUG) << szLog.Str();

		const D3DXVECTOR3& diff10 = (v1 - v0);
		const D3DXVECTOR3& diff20 = (v2 - v0);
		D3DXVec3Cross(&m_faces[faceIdx]->normal, &diff10, &diff20);		
		D3DXVECTOR3 normal; 
		D3DXVec3Normalize(&normal,  &m_faces[faceIdx]->normal);
		m_faces[faceIdx]->normal = normal;		
		m_faces[faceIdx]->point = v0;
	}

	pMesh->UnlockIndices();
	pMesh->UnlockVertices();
}

//-------------------------------------------------------------------------------------------------
/**
This method checks if the lighting situation has been changed since the
last time the method was called. If yes, the method will return true,
and the render-buffer must be updated. If no, everything can remain
as is, and rendering can happen without computations.
*/
bool U2Dx9ShadowCaster::LightingChanged(const U2Light& light, const D3DXMATRIX& invModelLitMat)
{
	U2Light::LightType curLitType = light.m_eLitType;
	D3DXVECTOR3 curLitPosOrDir;

	if(U2Light::LT_DIRECTIONAL == curLitType)
	{
		curLitPosOrDir = *((D3DXVECTOR3*)&invModelLitMat.m[2]);
	}
	else 
	{
		curLitPosOrDir = *((D3DXVECTOR3*)&invModelLitMat.m[3]);
	}

	if((curLitType != m_ePrevLitType) || (!U2Math::Vec3Equal(curLitPosOrDir, m_prevLitPosOrDir, 0.001f)))
	{
		m_ePrevLitType = curLitType;
		m_prevLitPosOrDir = curLitPosOrDir;
		return true;
	}
	else 
	{
		return false;
	}
}

//-------------------------------------------------------------------------------------------------
/**
This method computes the lit/unlit status of faces and updates
the "lit" flag of all faces in the face array.
*/
void U2Dx9ShadowCaster::UpdateFaceLitFlags(const U2Light& light, const D3DXMATRIX& invModelLitMat)
{

	U2Light::LightType litType = light.m_eLitType;
	if(U2Light::LT_DIRECTIONAL == litType)
	{
		const D3DXVECTOR3& modelLitDir = *((D3DXVECTOR3*)&invModelLitMat.m[2]);		
		
		uint32 faceIdx;
		uint32 numFaces = m_faces.Size();

		int numLit = 0;
		for(faceIdx = 0; faceIdx < numFaces; ++faceIdx)
		{
			Face& curFace = *m_faces[faceIdx];
			if(D3DXVec3Dot(&curFace.normal, &modelLitDir) > 0.0f)
			{
				curFace.bLit = true;			
			
				++numLit;
			}
			else 
			{
				curFace.bLit = false;
			}
		}
		numLit = numLit;
	}
	else if(U2Light::LT_POINT == litType)
	{
		const D3DXVECTOR3& modelLitPos = *((D3DXVECTOR3*)&invModelLitMat.m[3]);
		uint32 faceIdx;
		uint32 numFaces = m_faces.Size();
		for(faceIdx = 0; faceIdx < numFaces; ++faceIdx)
		{
			Face& curFace = *m_faces[faceIdx];
			D3DXVECTOR3 litDir = curFace.point - modelLitPos;
			if(D3DXVec3Dot(&curFace.normal, &litDir) > 0.0f)
			{
				curFace.bLit = true;
			}
			else 
			{
				curFace.bLit = false;
			}
		}
	}
	else 
	{
		U2ASSERT(false);
		FDebug("U2Dx9ShadowCaster::UpdateFaceLitFlags(): Unsupported light type!");
	}
}

//-------------------------------------------------------------------------------------------------
/**
This method starts writing the indices for the dark cap, light cap
and/or shadow volume sides into the shadow volume index buffer.
*/
void U2Dx9ShadowCaster::BeginWriteIndices(U2N2Mesh* pMesh)
{
	U2ASSERT(0 == m_pSrcMesh);
	U2ASSERT(0 == m_pSrcIndices);
	U2ASSERT(0 == m_pSrcEdges);
	U2ASSERT(0 == m_pIndicesStart);
	U2ASSERT(0 == m_pIndicesCurrent);
	U2ASSERT(pMesh);

	m_pIndicesStart = m_spShadowVolume->LockIndices();
	m_pIndicesCurrent = m_pIndicesStart;
	m_uiDrawNumIndices = 0;
	m_pSrcMesh = pMesh;
	m_pSrcIndices = pMesh->LockIndices();
	m_pSrcEdges = pMesh->GetEdges();
}

//------------------------------------------------------------------------------
/**
This writes the indices to render the dark or light cap into the shadow
index buffer. If the 'lit' argument is true, the light cap will be
rendered, otherwise the dark cap.
*/
void U2Dx9ShadowCaster::WriteCapIndices(bool bLit)
{
	U2ASSERT(m_pIndicesCurrent);
	U2ASSERT(m_pSrcIndices);
	U2ASSERT(m_pSrcMesh);

	uint16 startIdx = m_pSrcMesh->Group(m_uiMeshGroupIdx).GetFirstIndex();

	if(bLit)
	{
		// write light cap indices, these get extruded away from the light source
		uint32 faceIdx;
		uint32 numFaces = m_faces.Size();
		for(faceIdx = 0; faceIdx < numFaces; ++faceIdx)
		{
			Face& curFace = *m_faces[faceIdx];
			if(!curFace.bLit)
			{
				uint32 idx = startIdx + faceIdx * 3;
				*m_pIndicesCurrent++ = m_pSrcIndices[idx] + 1;
				*m_pIndicesCurrent++ = m_pSrcIndices[idx+2] + 1;
				*m_pIndicesCurrent++ = m_pSrcIndices[idx+1] + 1;
			}
		}
	}
	else 
	{
		// write dark cap indices, these remain at their original position
		uint32 faceIdx;
		uint32 numFaces = m_faces.Size();
		for(faceIdx = 0; faceIdx < numFaces; ++faceIdx)
		{
			Face& curFace = *m_faces[faceIdx];
			if(curFace.bLit)
			{
				uint32 idx = startIdx + faceIdx * 3;
				*m_pIndicesCurrent++ = m_pSrcIndices[idx];
				*m_pIndicesCurrent++ = m_pSrcIndices[idx+2];
				*m_pIndicesCurrent++ = m_pSrcIndices[idx+1];
			}
		}
	}

	/*U2DynString szLog;
	szLog.Format(_T("WriteCapIndices currIndices : %d ,  Lit : %d\n"), 
		(m_pIndicesCurrent - m_pIndicesStart), bLit ? 1 : 0);
	FILE_LOG(logDEBUG) << szLog.Str();*/
}

//------------------------------------------------------------------------------
/**
This writes a quad made of 2 triangles into the index buffer. This
is used for rendering the sides of the shadow volume.
*/
void U2Dx9ShadowCaster::WriteQuad(uint16 vertexIdx0, uint16 vertexIdx1)
{
	uint16 extrudedIdx0 = vertexIdx0 + 1;
	uint16 extrudedIdx1 = vertexIdx1 + 1;

	// write first triangle of quad
	*m_pIndicesCurrent++ = vertexIdx0;
	*m_pIndicesCurrent++ = vertexIdx1;
	*m_pIndicesCurrent++ = extrudedIdx0;

	// write second triangle of quad
	*m_pIndicesCurrent++ = vertexIdx1;
	*m_pIndicesCurrent++ = extrudedIdx1;
	*m_pIndicesCurrent++ = extrudedIdx0;
}

//------------------------------------------------------------------------------
/**
This writes the side faces of the shadow volume into the shadow index
buffer. The method iterates through the edge array, and for each silhouette
edge 2 triangles forming a quad will be written.
*/
void U2Dx9ShadowCaster::WriteSideIndices()
{
	U2ASSERT(m_pIndicesCurrent);
	U2ASSERT(m_pSrcMesh);
	U2ASSERT(m_pSrcEdges);

	const nMeshGroup& meshGroup = m_pSrcMesh->Group(m_uiMeshGroupIdx);
	uint16 startEdge = meshGroup.GetFirstEdge();
	uint16 numEdges = meshGroup.GetNumEdges();
	uint16 startFace = meshGroup.GetFirstIndex() / 3;
	uint16 edgeIdx;
	for(edgeIdx = startEdge; edgeIdx < startEdge + numEdges; ++edgeIdx)
	{
		const U2N2Mesh::Edge& edge = m_pSrcEdges[edgeIdx];

		if(edge.faceIdx[0] != U2N2Mesh::InvalidIndex)
		{
			
			const Face& face0 = *m_faces[edge.faceIdx[0] - startFace];
			if(edge.faceIdx[1] != U2N2Mesh::InvalidIndex)
			{
				// this is a normal edge
				const Face& face1 = *m_faces[edge.faceIdx[1] - startFace];
				if(face0.bLit != face1.bLit)
				{
					// a normal sillhouette edge
					if(face0.bLit) WriteQuad(edge.vertexIdx[0], edge.vertexIdx[1]);
					else			WriteQuad(edge.vertexIdx[1], edge.vertexIdx[0]);
				}
			}
			else 
			{
				// a border edge, first case 
				if(face0.bLit) WriteQuad(edge.vertexIdx[0], edge.vertexIdx[1]);
				else			WriteQuad(edge.vertexIdx[1], edge.vertexIdx[0]);
			}
		}
		else 
		{
				// a border edge, second case 
			const Face& face1 = *m_faces[edge.faceIdx[1] - startFace];
			if(face1.bLit) WriteQuad(edge.vertexIdx[0], edge.vertexIdx[1]);
			else			WriteQuad(edge.vertexIdx[1], edge.vertexIdx[0]);
		}
	}

	//U2DynString szLog;
	//szLog.Format(_T("WriteSideIndices currIndices : %d \n"), 
	//	(m_pIndicesCurrent - m_pIndicesStart));
	//FILE_LOG(logDEBUG) << szLog.Str();
}

//------------------------------------------------------------------------------
/**
Finish writing indices to the shadow volume index buffer. This updated
the drawNumIndices member which indicates how many indices from the
index buffer should be drawn.
*/
void U2Dx9ShadowCaster::EndWriteIndices()
{
	U2ASSERT(0 != m_pSrcMesh);
	U2ASSERT(0 != m_pSrcIndices);
	U2ASSERT(0 != m_pSrcEdges);
	U2ASSERT(0 == m_uiDrawNumIndices);
	U2ASSERT(0 != m_pIndicesStart);
	U2ASSERT(0 != m_pIndicesCurrent);

	m_pSrcMesh->UnlockIndices();
	m_pSrcMesh = 0;
	m_pSrcEdges = 0;
	m_pSrcIndices = 0;
	
	m_spShadowVolume->UnlockIndices();
	m_uiDrawNumIndices = m_pIndicesCurrent - m_pIndicesStart;
	m_pIndicesStart = 0;
	m_pIndicesCurrent = 0;	
}




