#include <U2_3D/src/U23DLibPCH.h>
#include "U2ShadowNode.h"
#include "U2Dx9ShadowMgr.h"


IMPLEMENT_RTTI(U2ShadowNode, U2Spatial);

const float U2ShadowNode::ms_fMaxSmallObjDistance = 30.0f;

//-------------------------------------------------------------------------------------------------
U2ShadowNode::U2ShadowNode()
:m_iGroupIdx(0)
{

}

//-------------------------------------------------------------------------------------------------
U2ShadowNode::~U2ShadowNode()
{
	UnloadResources();
}

//-------------------------------------------------------------------------------------------------
bool U2ShadowNode::LoadResources()
{
	m_spStaticCaster = (U2Dx9StaticShadowCaster*)U2Dx9ShadowMgr::Instance()->CreateShadowCaster(
		U2Dx9ShadowCaster::SHADOW_STATIC, NULL);
	U2ASSERT(m_spStaticCaster);
	m_spStaticCaster->SetName(m_szName);		// Mesh file path
	m_spStaticCaster->SetMeshGroupIdx(m_iGroupIdx);
	//U2ASSERT(m_spStaticCaster->LoadResource());
	m_spStaticCaster->LoadResource();

	return true;
}

//-------------------------------------------------------------------------------------------------
void U2ShadowNode::UnloadResources()
{
	m_spStaticCaster = 0;
}

//-------------------------------------------------------------------------------------------------
bool U2ShadowNode::RenderShadowVolume(U2SceneMgr* pSceneMgr, U2RenderContext* pCxt)
{
	D3DXMATRIX worldMat;
	m_tmWorld.GetHomogeneousMat(worldMat);
	// HACK:
	// check distance for small objects < smaller 3 meters diagonal)
	// should be replaced by some proper LODing!
	if(GetBoundingBox().DiagonalSize() < 3.0f)
	{
		U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
		U2ASSERT(pRenderer);

		const D3DXVECTOR3& viewerPos = *((D3DXVECTOR3*)&pRenderer->
			m_aTM[U2Dx9Renderer::TMT_INVVIEW].m[3]);
		const D3DXVECTOR3& worldPos = *((D3DXVECTOR3*)&worldMat.m[3]);
		float fDist = D3DXVec3Length(&(viewerPos - worldPos));
		if(fDist > ms_fMaxSmallObjDistance)
		{
			return true;
		}
	}

	U2Dx9ShadowMgr::Instance()->RenderShadowCaster(m_spStaticCaster, worldMat);
	return true;	
}

//-------------------------------------------------------------------------------------------------
//bool U2ShadowNode::HasShadow()
//{
//	return true;
//}
