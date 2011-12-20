#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9DynamicShadowCaster.h"
#include "U2Dx9ShadowMgr.h"

//-------------------------------------------------------------------------------------------------
U2Dx9DynamicShadowCaster::U2Dx9DynamicShadowCaster()
:m_pSkeleton(0),
m_bSkeletonDirty(false)
{

}

//-------------------------------------------------------------------------------------------------
U2Dx9DynamicShadowCaster::~U2Dx9DynamicShadowCaster()
{
	
}

//-------------------------------------------------------------------------------------------------
bool U2Dx9DynamicShadowCaster::LoadResource()
{
	U2ASSERT(!m_spShadowVolume);
	U2ASSERT(!m_spBindPoseMesh);
	U2ASSERT(!m_spSkinnedMesh);

	U2TriListData* pData = U2_NEW U2TriListData();
	m_spBindPoseMesh = U2_NEW U2N2Mesh(pData);


	return true;	
}

//-------------------------------------------------------------------------------------------------
void U2Dx9DynamicShadowCaster::UnloadResource()
{


}

//-------------------------------------------------------------------------------------------------
void U2Dx9DynamicShadowCaster::SetupShadowVolume(const U2Light& light, 
												 const D3DXMATRIX& invModelLitMat)
{


}

//-------------------------------------------------------------------------------------------------
void U2Dx9DynamicShadowCaster::RenderShadowVolume()
{

}

