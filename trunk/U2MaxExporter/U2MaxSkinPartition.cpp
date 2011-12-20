#include "U2MaxSkinPartition.h"
#include "U2MaxMeshExport.h"
#include "U2MaxOptions.h"
#include <U2_3D/Src/Tools/U2SkinPartition.h>


//-------------------------------------------------------------------------------------------------
U2MaxSkinPartition::U2MaxSkinPartition()
{

}

//-------------------------------------------------------------------------------------------------
U2MaxSkinPartition::~U2MaxSkinPartition()
{

}

//-------------------------------------------------------------------------------------------------
void U2MaxSkinPartition::Execute(U2PrimitiveVec<U2MaxMeshExport*>& srcMeshArray, 
								 U2MeshBuilder& srcMeshBuilder, 
								 U2PrimitiveVec<U2SkinModifier*>& skinMods)
{
	U2PrimitiveVec<U2MaxMeshFragement*> meshFragments;

	U2MeshBuilder tmpMeshBuilder;
	U2SkinPartition skinPartition;

	int maxBonePaletteSize = U2MaxOptions::ms_iMaxBonePaletteSize;

	if(skinPartition.Execute(srcMeshBuilder, tmpMeshBuilder, maxBonePaletteSize))
	{
		FILE_LOG(logDEBUG) << TSTR("The number of partitions : ") << skinPartition.GetNumPartitions();

		for(uint32 i=0; i < srcMeshArray.Size(); ++i)
		{
			U2MaxMeshExport* pMeshExport = srcMeshArray[i];

			for(int j=0; j < pMeshExport->GetNumGroupMeshes(); ++j)
			{					
				const U2MaxSkineMeshData& groupMesh = pMeshExport->GetGroupMesh(j);
				
				U2SkinModifier* pSkinMod = groupMesh.m_pMesh->GetSkinModifier();
				U2ASSERT(pSkinMod);

				int groupIdx = groupMesh.m_iGroupIdx;

				U2MaxMeshFragement* pMeshFragment = U2_NEW U2MaxMeshFragement;
				pMeshFragment->m_pSkinMod = pSkinMod;
				pSkinMod->SetName(groupMesh.m_pMesh->GetName());
				
				
				if(groupIdx >= 0)
				{
					const U2PrimitiveVec<int>& groupMaps = skinPartition.GetGroupMappings();

					for(int k=0; k < groupMaps.Size(); ++k)
					{
						if(groupMaps[k] == groupIdx)
						{
							U2PrimitiveVec<int>* bonePalettes = (U2PrimitiveVec<int>*)&skinPartition.GetBonePalette(k);

							if(bonePalettes->Size() > 0)
							{
								U2MaxMeshFragement::Fragement *pFrag = 
									U2_NEW U2MaxMeshFragement::Fragement;

								pFrag->groupMapIdx = k;
								
								for(int l=0; l < bonePalettes->FilledSize(); ++l)
								{
									FILE_LOG(logDEBUG) << TSTR("BonePalettes : ") << bonePalettes->GetElem(i);
									pFrag->bonePalettes.SetSafeElem(l, bonePalettes->GetElem(l));
								}

								pMeshFragment->m_fragments.AddElem(pFrag);
							}
						}
					}
				}
				meshFragments.AddElem(pMeshFragment);
			}
		}
	}

	srcMeshBuilder.Copy(tmpMeshBuilder);

//#ifdef DEBUG
//	int numtriangles = tmpMeshBuilder.GetNumTriangles();
//
//	for (int triangleIndex = 0; triangleIndex < numtriangles; triangleIndex++)
//	{
//
//		int i0, i1, i2;
//		tmpMeshBuilder.GetTriangleAt(triangleIndex).GetVertexIndices(i0, i1, i2);
//
//		FILE_LOG(logDEBUG) << _T("tmp Triangle : ") << i0 << _T(" ") << i1 << _T(" ") << i2;
//
//	}
//#endif	

	// build skin shape node's fragments.
	BuildMeshFragments(meshFragments, skinMods);	

}

//-----------------------------------------------------------------------------
/**
Specifies fragments and joint palette of the skin mesh based on 
the given nMaxMeshFragment

@param meshFragmentArray Array which contains skinned shape node and its 
mesh fragments of previously partitioned mesh.
*/
void U2MaxSkinPartition::BuildMeshFragments(U2PrimitiveVec<U2MaxMeshFragement*>& meshFragments,
											U2PrimitiveVec<U2SkinModifier*>& skinMods)
{
	for(int i=0; i < meshFragments.FilledSize(); ++i)
	{
		U2MaxMeshFragement& meshFragment = *meshFragments[i];

		uint32 numFragments = meshFragment.m_fragments.FilledSize();
		if(numFragments > 0)
		{
			U2SkinModifier* pSkinMod = meshFragment.m_pSkinMod;

			skinMods.AddElem(pSkinMod);
			pSkinMod->IncRefCount();

			pSkinMod->BeginFragments(numFragments);

			for(int j=0; j < numFragments; ++j)
			{
				U2MaxMeshFragement::Fragement& frag = *meshFragment.m_fragments[j];
				
				pSkinMod->SetFragGroupIndex(j, frag.groupMapIdx);

				uint32 numBonePaletteSize = frag.bonePalettes.Size();
				pSkinMod->BeginJointPalette(j, numBonePaletteSize);

				for(int k=0; k < numBonePaletteSize; ++k)
				{
					pSkinMod->SetJointIdx(j, k, frag.bonePalettes[k]);
				}

				pSkinMod->EndJointPalette(j);
			}
			pSkinMod->EndFragments();
		}
	}
}