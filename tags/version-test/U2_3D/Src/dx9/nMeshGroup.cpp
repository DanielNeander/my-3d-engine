#include <U2_3D/Src/U23DLibPCH.h>
#include "nMeshGroup.h"
#include <U2_3D/Src/main/U2SceneMgr.h>
#include <U2_3D/src/animation/U2BonePalette.h>
#include <U2_3D/src/animation/U2Bone.h>

IMPLEMENT_RTTI(nMeshGroup, U2Triangle);


void nMeshGroup::GetTriangle(uint16 usTriangle, D3DXVECTOR3*& pt0, 	D3DXVECTOR3*& pt1, D3DXVECTOR3*& pt2)
{
	U2TriListData* pModelData = 
		SmartPtrCast(U2TriListData, m_spModelData);
	U2ASSERT(usTriangle < pModelData->GetTriangleCnt());
	D3DXVECTOR3* pVertex = pModelData->GetVertices();
	uint16* pusTriList = pModelData->GetTriList();

	uint32 uStart = 3 *usTriangle;
	pt0 = &pVertex[pusTriList[uStart++]];
	pt1 = &pVertex[pusTriList[uStart++]];
	pt2 = &pVertex[pusTriList[uStart]];
}


nMeshGroup& nMeshGroup::operator=(const nMeshGroup& rhs)
{
	firstVertex = rhs.GetFirstVertex();
	numVertices = rhs.GetNumVertices();
	firstIndex = rhs.GetFirstIndex();
	numIndices = rhs.GetNumIndices();
	firstEdge = rhs.GetFirstEdge();
	numEdges = rhs.GetNumEdges();
	m_bbox = rhs.m_bbox;

	m_pSkinMod = rhs.GetSkinModifier();
	m_pOwnerMesh = rhs.GetMesh();

	for(unsigned int i=0; i < rhs.GetNumControllers(); ++i)
	{
		AttachController(rhs.GetController(i));
	}

	return *this;
}

bool nMeshGroup::Render(U2SceneMgr* pSceneMgr, U2RenderContext* pCxt)
{
	IsRendererOK2;

	if(m_pSkinMod)
		pCxt->pUserData = m_pSkinMod;
	
	for(unsigned int i=0; i < GetNumControllers(); ++i)
	{		
		GetController(i)->Update(0.f, pCxt);
	}


	if(m_pSkinMod)		
	{
		


		// Set D3D VB /IB
		//for(uint32 i=0; i < GetEffectCnt(); ++i)
		//{
		//	U2Dx9FxShaderEffect* pEffect = 
		//		DynamicCast<U2Dx9FxShaderEffect>(GetEffect(i));
		//	if(pEffect)
		//	{
		//		pRenderer->SetupMesh(GetMesh());
		//	}
		//}		
		
		int numFragments = m_pSkinMod->GetNumFragments();
		int fragIdx;

		
		for(fragIdx = 0; fragIdx < numFragments; ++fragIdx)
		{
			U2SkinModifier::Fragment& fragment = 
				*m_pSkinMod->GetFragment(fragIdx);
			RenderShaderSkinning(fragment.GetMeshGroupIdx(), fragment.GetBonePalette());			
		}
	}
	else 
	{
		//// Set D3D VB /IB
		//for(uint32 i=0; i < GetEffectCnt(); ++i)
		//{
		//	U2Dx9FxShaderEffect* pEffect = 
		//		DynamicCast<U2Dx9FxShaderEffect>(GetEffect(i));
		//	if(pEffect)
		//	{
		//		pRenderer->SetupMesh(GetMesh());
		//	}
		//}

		pRenderer->SetVertexRange(GetFirstVertex(), GetNumVertices());
		pRenderer->SetIndexRange(GetFirstIndex(), GetNumIndices());
		pRenderer->DrawIndexedPrimitive(m_spModelData->GetPrimitiveType());
	}

	return true;
}


void nMeshGroup::RenderShaderSkinning(int meshGroupIdx, U2BonePalette& bonePalette)
{
	
	static const int maxBonePaletteSize = 72;
	static D3DXMATRIX boneMatArray[maxBonePaletteSize];

	int paletteSize = bonePalette.GetNumJoints();
	U2ASSERT(paletteSize <= maxBonePaletteSize);

	int paletteIdx;
	for(paletteIdx = 0; paletteIdx < paletteSize; ++paletteIdx)
	{
		const U2Bone& bone = m_pSkinMod->GetSkeleton()->GetJoint(
			bonePalette.GetJointIdx(paletteIdx));
		boneMatArray[paletteIdx] = bone.GetSKinMat44();
	}

	U2SceneMgr *pSceneMgr = U2SceneMgr::Instance();
	U2ASSERT(pSceneMgr);
	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);

	U2D3DXEffectShader *pCurrShader = pRenderer->GetCurrEffectShader();
	U2ASSERT(pCurrShader);
	if(pCurrShader->IsParamUsed(U2FxShaderState::JointPalette))
	{
		pCurrShader->SetMatrixArray(U2FxShaderState::JointPalette, boneMatArray, paletteSize);
	}

	pRenderer->SetVertexRange(GetFirstVertex(), GetNumVertices());
	pRenderer->SetIndexRange(GetFirstIndex(), GetNumIndices());
	pRenderer->DrawIndexedPrimitive(D3DPT_TRIANGLELIST);
}
