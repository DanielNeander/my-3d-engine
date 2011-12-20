#undef V
#include "U2MaxSceneExport.h"
#include "U2MaxAnimExport.h"
#include "U2MaxMeshExport.h"
#include "U2MaxCameraExport.h"
#include "U2MaxLightExport.h"
#include "U2MaxSkinPartition.h"
#include "U2MaxUtil.h"

U2Filename U2MaxSceneExport::ms_szFullPath = TSTR("");

U2MaxSceneExport::U2MaxSceneExport()
:m_spScene(0)
{
	m_nonSkinnedMeshBuilder = U2_NEW U2MeshBuilder;
	m_skinnedMeshBuider = U2_NEW U2MeshBuilder;

}

U2MaxSceneExport::~U2MaxSceneExport()
{
	U2_DELETE m_nonSkinnedMeshBuilder;
	m_nonSkinnedMeshBuilder = NULL;
	U2_DELETE m_skinnedMeshBuider;
	m_skinnedMeshBuider = NULL;

	uint32 i;
	for(i=0; i < m_nonSkinnnedMeshs.FilledSize(); ++i)
	{
		U2_DELETE m_nonSkinnnedMeshs[i];		
	}
	m_nonSkinnnedMeshs.RemoveAll();

	for(i=0; i < m_skinnedMeshes.FilledSize(); ++i)
	{
		U2_DELETE m_skinnedMeshes[i];
	}
	m_skinnedMeshes.RemoveAll();

}


bool U2MaxSceneExport::ExportScene(Interface* pIf, const TCHAR* szFilename)
{
	m_animStart = pIf->GetAnimRange().Start();
	m_animEnd = pIf->GetAnimRange().End();

	//if (m_animStart > 0)
	//	m_animStart = 0;

	U2Filename tempPath(szFilename);
	ms_szFullPath.SetDrive(tempPath.GetDrive());
	ms_szFullPath.SetDir(tempPath.GetDir());
	ms_szFullPath.SetFilename(tempPath.GetFilename());
	ms_szFullPath.SetExt(tempPath.GetExt());
	

	INode* pMaxRoot = pIf->GetRootNode();

	TimeValue tempStart = TIME_PosInfinity;
	TimeValue tempEnd = TIME_NegInfinity;
	U2MaxAnimExport::CalculateGlobalTimeRange(tempStart, tempEnd);

	if (tempEnd < TIME_PosInfinity && 
		tempEnd > TIME_NegInfinity && tempEnd > m_animEnd)
		m_animEnd = tempEnd;

	U2MaxMeshExport::Preprocess(pMaxRoot);

	FILE_LOG(logDEBUG) << TSTR("Start to build bone list.");

	m_pAnimExport = U2MaxAnimExport::Instance();
	m_pAnimExport->Init(m_animStart, m_animEnd);
	if(!m_pAnimExport->BuildBones(pIf,pMaxRoot))
	{
		FILE_LOG(logDEBUG) << TSTR("Failed to build bone list.");
		return false;
	}

	m_nonSkinnedMeshBuilder->Clear();
	m_skinnedMeshBuider->Clear();

	if(U2MaxOptions::ms_eMode != EXPMODE_MESH)
	{	
		if(!ExportSkinAnimation())
			return false;
	}

	if(U2MaxOptions::ms_eMode == EXPMODE_ANIM)
		return true;

	ExportNodes(pIf, pMaxRoot, &m_spScene);
		

	U2MaxMeshExport::Postprocess(pMaxRoot);
	
	U2MaxMeshExport* pMeshExport;
	if(m_nonSkinnnedMeshs.FilledSize() > 0)
	{
		int numMeshs = m_nonSkinnnedMeshs.FilledSize();
		for(int i=0; i < numMeshs; ++i)
		{
			 pMeshExport = m_nonSkinnnedMeshs[i];

			 int baseGroupIdx = m_nonSkinnedMeshBuilder->Append(*pMeshExport->m_pMeshBuilder);			 
			 pMeshExport->SetBaseGroupIdx(baseGroupIdx);
		}
	}

	if(m_skinnedMeshes.FilledSize() > 0)
	{
		int numGeoms = m_skinnedMeshes.Size();
		for(int i=0; i < numGeoms; ++i)
		{
			pMeshExport = m_skinnedMeshes[i];

			int baseGroupIdx = m_skinnedMeshBuider->Append(*pMeshExport->m_pMeshBuilder);
			pMeshExport->SetBaseGroupIdx(baseGroupIdx);
		}
	}

	// if the global mesh has skinned animation, it might be needed to be partitioning. 
	U2PrimitiveVec<U2SkinModifier*> skinMods;

	if(m_pAnimExport->GetNumBones() > 0)
	{
		if(m_skinnedMeshBuider->GetNumVertices() > 0)
		{
			U2MaxSkinPartition skinPartition;
			skinPartition.Execute(m_skinnedMeshes, *m_skinnedMeshBuider, skinMods);
		}
	}

	
	// Save 
	if(m_nonSkinnedMeshBuilder->GetNumVertices() > 0)
	{
		TCHAR szMeshFilename[MAX_PATH];
		ms_szFullPath.SetExt(TSTR(".xmesh"));
		ms_szFullPath.FullPath(szMeshFilename, MAX_PATH);		

		ProcessMeshBuilder(m_nonSkinnnedMeshs[0], *m_nonSkinnedMeshBuilder, false,
			ms_szFullPath.GetFilename());		

		for(uint32 i = 0 ; m_nonSkinnedMeshBuilder->groupMap.FilledSize(); ++i)
		{
			U2_DELETE m_nonSkinnedMeshBuilder->groupMap[i];
		}
		m_nonSkinnedMeshBuilder->groupMap.RemoveAll();

		m_nonSkinnedMeshBuilder->SortTriangles();		
		m_nonSkinnedMeshBuilder->BuildGroupMap(m_nonSkinnedMeshBuilder->groupMap);

		m_nonSkinnedMeshBuilder->Save(szMeshFilename);
	}	

	if(m_skinnedMeshBuider->GetNumVertices() > 0)
	{
		TCHAR szMeshFilename[MAX_PATH];

		U2DynString skinMeshFullpath(ms_szFullPath.GetFilename());
		skinMeshFullpath.Concatenate(TSTR("_skin"));

		ms_szFullPath.SetFilename(skinMeshFullpath.Str());
		ms_szFullPath.SetExt(TSTR(".xmesh"));
		ms_szFullPath.FullPath(szMeshFilename, MAX_PATH);

		ProcessMeshBuilder(m_skinnedMeshes[0], *m_skinnedMeshBuider, false, 
			ms_szFullPath.GetFilename());


		m_skinnedMeshBuider->SortTriangles();		
		m_skinnedMeshBuider->BuildGroupMap(m_skinnedMeshBuider->groupMap);

		// Error
		// 현재는 SkinModifer의 Name으로 Group을 Mapping
		//U2ASSERT(skinMods.FilledSize() == m_skinnedMeshBuider->groupMap.FilledSize());

		if ( U2MaxOptions::ms_eMode != EXPMODE_MESH )
		{		
			for (uint32 i = 0; i < skinMods.FilledSize(); ++i )
			{
				U2MeshBuilder::Group& group = *m_skinnedMeshBuider->groupMap.GetElem(i);
				group.SetSkinModifier(skinMods.GetElem(i));		
				
			}	

			//for(uint32 i=0; i < m_skinnedMeshBuider->groupMap.FilledSize(); ++i)
			//{
			//	U2MeshBuilder::Group& group = *m_skinnedMeshBuider->groupMap.GetElem(i);
			//					
			//	if(skinMods.FilledSize() - 1 >= i)
			//		group.SetSkinModifier(skinMods.GetElem(i));				
			//	else 
			//		group.SetSkinModifier(NULL);				
			//}	
		}

		m_skinnedMeshBuider->Save(szMeshFilename);		
	}

	return true;
}

//-----------------------------------------------------------------------------
/**
Do some processing on the meshbuilder object as the followings:
- cleanup mesh data
- build tangent and vertex normal
- check geometry errors
- scale a mesh

Called in nMaxScene::Postprocess() function.

ProcessOnMeshBuilder() use some components but they are removed by ForceVertexComponents()
and it produces new components which is not needed. So ProcessOnMeshBuilder() should be 
called before calling ForceVertexComponents().

@param meshBuilder  - A reference to nMeshBuilder needed for building tangent, 
normal and edges if it is necessary.
@param isShadowMesh - true if the given mesh is used for shadow.
@param meshName     - mesh file name.

*/
//-------------------------------------------------------------------------------------------------
void U2MaxSceneExport::ProcessMeshBuilder(U2MaxMeshExport* pMeshExport, U2MeshBuilder& meshBlder, bool bIsShadowMesh,
										  const U2DynString& szMeshname)
{
	meshBlder.Cleanup(0);

	
	if(!bIsShadowMesh)
	{
		// Build tangent and normals 
		if ( 0 != pMeshExport && !pMeshExport->BuildMeshTangentNormals(meshBlder) )
		{
			// Error
		}
	}
	else 
	{
		meshBlder.CreateEdges();
	}

	// check the mesh for geometry error.
	float geomScale = U2MaxOptions::ms_fGeomScale;
	if(geomScale != 0.0f)
	{
		D3DXVECTOR3 scale;
		D3DXMATRIX m;

		scale = D3DXVECTOR3(geomScale, geomScale, geomScale);
		if(1.0f != D3DXVec3Length(&scale))
		{
			D3DXMatrixScaling(&m, geomScale, geomScale, geomScale);
			meshBlder.Transform(m);
		}
	}
}

unsigned int U2MaxSceneExport::CalculateTotalNodeCount(INode* pkNode)
{
	if(pkNode == NULL)
		return 0;

	unsigned int uiCount = (unsigned int) pkNode->NumberOfChildren();
	unsigned int childCount = uiCount;
	for(unsigned int ui = 0; ui < uiCount; ui++)
	{
		childCount += CalculateTotalNodeCount(pkNode->GetChildNode(ui));
	}

	return childCount;
}


bool U2MaxSceneExport::ExportNodes(Interface* pIf, INode* pMaxNode, U2NodePtr* pspResult)
{

	// Check Already Exported Node 
	// 플래그 상태 정확한 체크 필요.
	bool bSuccess = true;

	U2Node* pU2Node = U2_NEW U2Node;

	pU2Node->SetName(pMaxNode->GetName());

	*pspResult = pU2Node;

	ObjectState objState = pMaxNode->EvalWorldState(m_animStart);
	Object* pObj = objState.obj;

	int i;

	FILE_LOG(logDEBUG) << TSTR("Export Node Name : ") << pMaxNode->GetName();
	FILE_LOG(logDEBUG) << TSTR( "Num Child : ") << pMaxNode->NumberOfChildren();

	
	for(i=0; i < pMaxNode->NumberOfChildren(); ++i)
	{
		INode* pMaxChild;
		char* pChildName;

		pMaxChild = pMaxNode->GetChildNode(i);

		pChildName = pMaxChild->GetName();

		FILE_LOG(logDEBUG) << TSTR(" Export Node Child Name : ") << pChildName;

		U2NodePtr spNiChild;
		bSuccess = ExportNodes(pIf, pMaxChild, &spNiChild);
		if(spNiChild)
			pU2Node->AttachChild(spNiChild);

		if(bSuccess)
			return bSuccess;
	}


	if(pObj)
	{
		SClass_ID sID = U2MaxUtil::GetSuperClassID(pObj);
		while(sID == GEN_DERIVOB_CLASS_ID)
		{
			pObj = ((IDerivedObject*)pObj)->GetObjRef();
			sID = pObj->SuperClassID();
		}

		switch(sID)
		{
		case CAMERA_CLASS_ID:

			break;
			
		case LIGHT_CLASS_ID:
			{
				U2MaxLightExport lightExp(m_animStart, m_animEnd);
				lightExp.ExportLight(pIf, pMaxNode,pU2Node, pObj);
			}
			break;

		case GEOMOBJECT_CLASS_ID:
			ExportGeomObj(pMaxNode, pU2Node, pObj);
			
			break;

		//case SHAPE_CLASS_ID:
		//	break;

		case HELPER_CLASS_ID:
			break;

		default:
			break;

		}
	}	
	
	m_aspScenes.AddElem(*pspResult);
	return 0;
}


void U2MaxSceneExport::ExportGeomObj(INode* pMaxNode, U2Node* pCurNode,  Object* pObj)
{
	U2Spatial* pGeom;

	if(pObj->IsRenderable())
	{
		if(!pMaxNode->IsHidden() &&
			!U2MaxUtil::IsBone(pMaxNode) &&
			!U2MaxUtil::IsFootStep(pMaxNode))
		{
			U2MaxMeshExport *meshExp = U2_NEW U2MaxMeshExport;
			
			meshExp->Export(&pGeom, pMaxNode);
			
			if(pGeom)
			{		
				pCurNode->AttachChild(pGeom);
			}

			AddMeshByType(meshExp);
		}	
	}
}

void U2MaxSceneExport::AddMeshByType(U2MaxMeshExport* pMesh)
{
	if(!pMesh)
		return;

	switch(U2MaxOptions::ms_eMeshType)
	{
	case Shape:
		{
			if(pMesh->IsPhysique() || pMesh->IsSkinned())
				m_skinnedMeshes.AddElem(pMesh);
			else 
				m_nonSkinnnedMeshs.AddElem(pMesh);
		}
		break;	
	default:
		FILE_LOG(logDEBUG) << TSTR("Unknown mesh type : ");
		U2_DELETE pMesh;
		break;
	}
}

//---------------------------------------------------------------------------
//
// This implementation of NfGetLocalTM removes all scales from the returned
// Matrix3, but also stores them in the parameter pScaleOut (if specified)
// so that they can be pushed down to the geometry level.
//
Matrix3 U2MaxSceneExport::GetLocalTM(INode* pMaxNode, TimeValue t)
{
	Matrix3 localScaleTM;
	INode* pParent = pMaxNode->GetParentNode();


	if(pParent)
	{
		Matrix3 parentTM = pParent->GetNodeTM(t);
		Matrix3 thisTM = pMaxNode->GetNodeTM(t);

		localScaleTM = thisTM * Inverse(parentTM);		
	}
	else 
		localScaleTM.IdentityMatrix();

	Matrix3 localNoScaleTM;
	localNoScaleTM = UniformMatrix(localScaleTM);

	return localNoScaleTM;

}


//---------------------------------------------------------------------------
Matrix3 U2MaxSceneExport::UniformMatrix(Matrix3 orig_cur_mat)
{
	AffineParts   parts;  
	Matrix3       mat;   

	// Remove scaling from orig_cur_mat
	// 1) Decompose original and get decomposition info
	decomp_affine(orig_cur_mat, &parts); 

	// 2) construct 3x3 rotation from quaternion parts.q
	parts.q.MakeMatrix(mat);

	// 3) construct position row from translation parts.t  
	mat.SetRow(3, parts.t);

	return mat;
}

//-----------------------------------------------------------------------------
/**
Export skin animator.

This member function is called before calling ExportNodes which exports all
3dsmax nodes of the given scene.

Nebula3 assumes any skinanimator places fisrt at its scene hierarchy when it 
loads and parses .n2 legacy file. So we also need to change skinanimators location. 

@note
It should be called after nMaxScene::OpenNebula() member function otherwise 
it will be failed.

@param animOnly used for exporting only animation file.
*/
bool U2MaxSceneExport::ExportSkinAnimation(bool bAnimOnly)
{
	U2ASSERT(m_pAnimExport);
	if(m_pAnimExport->GetNumBones() > 0)
	{
		for(int skelIdx = 0; skelIdx < m_pAnimExport->GetNumSkeletons();
			++skelIdx)
		{
			// export .anim2 and skin animator, if the exported scene has skinned mesh.
			//Create filename
			U2DynString szAnimFilename = ms_szFullPath.GetFilename();
			szAnimFilename.AppendInt(skelIdx);	

			ms_szFullPath.SetFilename(szAnimFilename);
			ms_szFullPath.SetExt(TSTR(".xanim"));
			TCHAR szPath[MAX_PATH];
			ms_szFullPath.FullPath(szPath, MAX_PATH);
			szAnimFilename = szPath;
					


			if(!m_pAnimExport->Export(skelIdx, szAnimFilename.Str()))
			{
				FILE_LOG(logDEBUG) << TSTR("Failed to export animation ") ;					
				return false;
			}

			if(bAnimOnly)
				continue;

			U2SkinController* pNewSkinCtrl = NULL;
			U2DynString szCtrlName(TSTR("U2SkinCtrl"));
			szCtrlName.AppendInt(skelIdx);			


			pNewSkinCtrl = m_pAnimExport->ConvertSkinAnim(skelIdx, szCtrlName.Str(),
				szAnimFilename.Str());
			
			m_pAnimExport->m_skinCtrlMap.insert(std::make_pair(skelIdx, pNewSkinCtrl));

			m_skinnedMeshBuider->m_skinCtrls.Insert(skelIdx, pNewSkinCtrl);

		}
	}

	return true;
}




