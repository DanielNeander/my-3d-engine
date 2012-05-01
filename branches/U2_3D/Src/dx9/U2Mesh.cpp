#include <U2_3D/src/U23DLibPCH.h>
#include "U2Mesh.h"
#include <U2_3D/Src/Main/U2SceneMgr.h>

IMPLEMENT_RTTI(U2Mesh, U2Spatial);

//-------------------------------------------------------------------------------------------------
U2Mesh::U2Mesh(U2MeshData* pMeshData)
	:m_spModelData(pMeshData)
	,m_spLitEffect(0),
	m_spModelBound(U2BoundingVolume::Create()),
	m_uNumGroups(0),	
	m_bLightEnable(true),
	m_uiGroupIdx(0),
	m_pBBox(U2_NEW U2Aabb),
	m_pObb(U2_NEW U2Aabb)

{
	D3DXMatrixIdentity(&m_matHWorld);

}

//-------------------------------------------------------------------------------------------------
U2Mesh::U2Mesh()
	:m_spModelData(0),
	m_spLitEffect(0),
	m_spModelBound(U2BoundingVolume::Create()),
	m_uNumGroups(0),	
	m_bLightEnable(true),
	m_pBBox(U2_NEW U2Aabb),
	m_pObb(U2_NEW U2Aabb)

{
	D3DXMatrixIdentity(&m_matHWorld);
}

//-------------------------------------------------------------------------------------------------
U2Mesh::~U2Mesh()
{
	
	m_spModelData = 0;

	U2_DELETE m_pObb;
	m_pObb = NULL;

	U2_DELETE m_pBBox;
	m_pBBox = NULL;
}

//-------------------------------------------------------------------------------------------------
void U2Mesh::UpdateMeshState(bool bUpdateNormals /* = true */, 
							 int iTangentSpaceType /* = TANGENTSPACE_NONE */)
{
	UpdateModelBound();

	if(!m_spModelData)
		return;

	if(bUpdateNormals)
	{
		// Update Model Normals				
		UpdateModelNormals();
	}

	if(iTangentSpaceType > (int)TANGENTSPACE_NONE)
	{
		// UpdateModelTangentSpace
		UpdateModelTangentSpace(iTangentSpaceType);
	}
}

//-------------------------------------------------------------------------------------------------
void U2Mesh::UpdateModelBound ()
{
	m_spModelBound->ComputeFromData(m_spModelData);
	// Generate BBox 
	
	//m_pBBox->BeginExtend();
	//uint32 numVertices = m_spModelData->GetVertexCount();
	//uint32 vertexIdx;
	//for(vertexIdx = 0; vertexIdx < numVertices; ++vertexIdx)
	//{
	//	m_pBBox->Extend(m_spModelData->GetVertices()[vertexIdx]);
	//}
	//m_pBBox->EndExtend();	
	m_pBBox->Set(m_spModelData->GetVertices(), 
		m_spModelData->GetVertexCount(), m_spModelData->GetVertexStride(0));
}

//-------------------------------------------------------------------------------------------------
void U2Mesh::UpdateWorldBound ()
{
	m_spModelBound->TransformBy(m_tmWorld, m_spWorldBound);
	// Transform Axis Aligned Bounding Box 
	//m_pBBox->Transform(m_matHWorld);	
	CalculateAABB();
}


//-------------------------------------------------------------------------------------------------
void U2Mesh::UpdateState(U2RenderStateCollectionPtr spRSC, 
								U2ObjVec<U2LightPtr>* pLightArray)
{		

	if(GetLightOnOff())
	{	
		uint32 i;
		uint32 LitCnt = pLightArray->Size();
		if(LitCnt > 0)
		{
			if(m_spLitEffect)
			{
				m_spLitEffect->DetachAllLights();
			}
			else 
			{
				m_spLitEffect = U2_NEW U2LightEffect;
				m_aspEffects.AddElem(SmartPtrCast(U2Effect, m_spLitEffect));
			}

			for(i=0; i < LitCnt; ++i)
			{
				m_spLitEffect->AttachLight(SmartPtrCast(U2Light, pLightArray->GetElem(i)));
			}

			m_spLitEffect->Configure();
		}
		else 
		{
			if(m_spLitEffect)
			{
				if(m_aspEffects.Size() > 0)
				{
					m_aspEffects.Remove(m_aspEffects.GetElem(0));
				}
				m_spLitEffect = 0;			
			}
		}
	}

}

//-------------------------------------------------------------------------------------------------
unsigned short U2Mesh::GetVertexCount() const
{
	return m_spModelData->GetVertexCount();
}

//-------------------------------------------------------------------------------------------------
D3DXVECTOR3* U2Mesh::GetVertices() const
{
	return m_spModelData->GetVertices();
}

//-------------------------------------------------------------------------------------------------
void U2Mesh::SetActiveVertexCount(unsigned short usActive)
{
	if(!m_spModelData)
		return;
	m_spModelData->SetActiveVertexCount(usActive);
}

//-------------------------------------------------------------------------------------------------
unsigned short U2Mesh::GetActiveVertexCount() const
{
	if(!m_spModelData)
		return 0;
	return m_spModelData->GetActiveVertexCount();
}

//-------------------------------------------------------------------------------------------------
void U2Mesh::CreateNormals()
{
	m_spModelData->CreateNormals(true);
}


//-------------------------------------------------------------------------------------------------
D3DXVECTOR3* U2Mesh::GetNormals() const
{
	if(!m_spModelData)
		return 0;
	return m_spModelData->GetNormals();
}

//-------------------------------------------------------------------------------------------------
void U2Mesh::CreateColors()
{
	m_spModelData->CreateColors();
}

//-------------------------------------------------------------------------------------------------
D3DXCOLOR* U2Mesh::GetColors() const	
{
	if(!m_spModelData)
		return 0;
	return m_spModelData->GetColors();
}

//-------------------------------------------------------------------------------------------------
void U2Mesh::CreateTexCoords(unsigned short numTexCoorSets)
{
	return m_spModelData->CreateTexCoords(numTexCoorSets);
}

//-------------------------------------------------------------------------------------------------
D3DXVECTOR2* U2Mesh::GetTexCoordSet(unsigned short texCoordIdx) const
{
	if(!m_spModelData)
		return 0;
	return m_spModelData->GetTexCoordSet(texCoordIdx);
}

//-------------------------------------------------------------------------------------------------
D3DXVECTOR2* U2Mesh::GetTexCoords() const	
{
	if(!m_spModelData)
		return 0;
	return m_spModelData->GetTexCoords();
}

//-------------------------------------------------------------------------------------------------
void U2Mesh::AppendTexCoords(D3DXVECTOR2* pAddedTexCoords)
{
	m_spModelData->AppendTexCoords(pAddedTexCoords);
}

//-------------------------------------------------------------------------------------------------
unsigned short U2Mesh::GetTexCoordSetCnt() const
{
	if(!m_spModelData)
		return 0;
	return m_spModelData->GetTexCoordSetCnt();
}

//-------------------------------------------------------------------------------------------------
void U2Mesh::Render(U2Dx9Renderer* pRenderer)
{	
	
}

//-------------------------------------------------------------------------------------------------
void U2Mesh::SetModelData(U2MeshData* pMeshData)
{
	U2ASSERT(pMeshData);
	if(!pMeshData)
		return;

	m_spModelData = pMeshData;
}

//-------------------------------------------------------------------------------------------------
U2MeshData* U2Mesh::GetMeshData() const
{
	if(!m_spModelData)
		return 0;
	return m_spModelData;
}

//-------------------------------------------------------------------------------------------------
void U2Mesh::GetVisibleSet(U2Culler& culler, bool bNoCull)
{
	culler.Insert(this, 0);
}

//-------------------------------------------------------------------------------------------------
void U2Mesh::UpdateWorldData(float fAppTime)
{
	U2Spatial::UpdateWorldData(fAppTime);
	m_tmWorld.GetHomogeneousMat(m_matHWorld);
	
}

//-------------------------------------------------------------------------------------------------
void U2Mesh::CalculateAABB(void)
{
	// transform OOBB points to world space 
	D3DXVECTOR4 vTransformed[8];

	D3DXMATRIX transposeTM;
	D3DXMatrixTranspose(&transposeTM, &m_matHWorld);

	for (unsigned int i=0; i < 8; i++)
	{
		D3DXVec3TransformCoord((D3DXVECTOR3*)&vTransformed[i] , 
			(D3DXVECTOR3*)&vTransformed[i] , &transposeTM);		 
	}
	// set new AABB
	m_pBBox->Set(vTransformed, 8, sizeof(D3DXVECTOR4));
}





