#include "U2MaxMeshExport.h"
#include "U2MaxAnimExport.h"
#include "U2MaxUtil.h"
#include "U2MaxNullView.h"

U2MaxMeshExport::U2MaxMeshExport()
:m_pGeomNode(0),
m_pRefMaker(0),
m_pMaxMesh(0),
m_bDeleteMesh(FALSE),
m_bSkinned(false),
m_bPhysique(false),
m_bBeginSkin(false),
m_pMeshBuilder(0)
{

}


U2MaxMeshExport::~U2MaxMeshExport()
{
	
}



//-----------------------------------------------------------------------------
/**
Retrieves a Mesh from given INode.

Call after ReferenceMaker::RenderBegin(0);

@param inode
@param needDelete true, if the primitive already has cached mesh.
(it should be deleted)
*/
Mesh* U2MaxMeshExport::GetMeshFromRenderMesh(INode* pINode, BOOL &bNeedDelete)
{
	U2ASSERT(pINode);

	Object* pObj = U2MaxUtil::GetBaseObject(pINode, m_animStartTime);

	GeomObject* pGeomObj = (GeomObject*)pObj;


	U2MaxNullView nullView;
	bNeedDelete = FALSE;

	Mesh* pMesh = 0;
	//MNMesh* pMNMesh = 0;

	//if (pGeomObj->ClassID() == polyObjectClassID)
	//{
	//	PolyObject* pPolyObj = (PolyObject*)pObj;
	//	U2ASSERT(pPolyObj);
	//	pMNMesh = &pPolyObj->GetMesh();		
	//}
	
	pMesh = pGeomObj->GetRenderMesh(m_animStartTime, pINode, nullView, bNeedDelete);
	U2ASSERT(pMesh);

	return pMesh;	
}


//-----------------------------------------------------------------------------
/**
Retrieves a TriObject from a given INode.

The following shows that how to get a Mesh from retrieved TriObject.

@param inode the node which we want to retrieve TriObject from
@param needDelete true, if the retrieved TriObject should be deleted.

@return pointer to the TriObject. Null, if the given node can not be
converted to TriObject type.
*/
Mesh* U2MaxMeshExport::GetMeshFromTriObject(INode* inode, BOOL &needDelete)
{
	U2ASSERT(inode);

	
	Object* obj = U2MaxUtil::GetBaseObject(inode, m_animStartTime);

	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
	{
		TriObject* tri = (TriObject*)obj->ConvertToType(m_animStartTime, 
			Class_ID(TRIOBJ_CLASS_ID, 0));

		needDelete = false;
		// if the pointer to the object is not equal to the retrieved pointer,
		// the TriObject should be deleted after.
		if (obj != tri)
			needDelete = true;

		Mesh* mesh = &(tri->mesh);
		this->m_pTriObj = tri;

		return mesh;
	}
	else
	{
		FILE_LOG(logDEBUG) << TSTR("Could not get mesh from TriObject of ") <<  inode->GetName();
		return NULL;
	}
}

//-------------------------------------------------------------------------------------------------
void U2MaxMeshExport::Preprocess(INode* pNode)
{	
	// Clear node flags

	U2MaxUtil::SetFlags(pNode, ND_CLEAR);

	for (int i = 0; i < pNode->NumberOfChildren(); i++)
		Preprocess(pNode->GetChildNode(i));

	ObjectState kOState = pNode->EvalWorldState(0);
	Object* pkEvalObj = kOState.obj;
	if (!pkEvalObj) 
		return;

	if (pkEvalObj->SuperClassID() == GEOMOBJECT_CLASS_ID)
		U2MaxUtil::SetFlags(pkEvalObj, ND_CLEAR);

	Object* pkObjectRef = pNode->GetObjectRef();
	// Disable Skin Modifier so that we can get the skin in the
	// initial pose
	Modifier* pkMod = U2MaxUtil::FindModifier(pkObjectRef, SKIN_CLASSID);

	if (pkMod)
		pkMod->DisableMod();

	// Disable Physique Modifier so that we can get the skin in the
	// initial pose
	pkMod = U2MaxUtil::FindModifier(pkObjectRef,
		Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B));

	if (pkMod)
		pkMod->DisableMod();
}

//-------------------------------------------------------------------------------------------------
void U2MaxMeshExport::Postprocess(INode* pNode)
{
	
	// Re-enable Skin Modifier 
	Object* pkObjectRef = pNode->GetObjectRef();
	Modifier* pkMod = U2MaxUtil::FindModifier(pkObjectRef, SKIN_CLASSID);
	if (pkMod)
		pkMod->EnableMod();

	// Re-enable Physique Modifier 
	pkMod = U2MaxUtil::FindModifier(pkObjectRef,
		Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B));

	if (pkMod)
		pkMod->EnableMod();

	for (int i = 0; i < pNode->NumberOfChildren(); i++)
		Postprocess(pNode->GetChildNode(i));
}

//-----------------------------------------------------------------------------
/**
Retrieves mesh from the given node.

@param inode 3dsmax node
@return pointer to Mesh, if there's no mesh, return NULL.
*/
Mesh* U2MaxMeshExport::LockMesh(INode* pINode)
{
	U2ASSERT(m_pRefMaker == NULL);

	m_pRefMaker = pINode;
	m_pRefMaker->RenderBegin(0);

	m_pMaxMesh = GetMeshFromRenderMesh(pINode, m_bDeleteMesh);

	return m_pMaxMesh;
}


//-----------------------------------------------------------------------------
/**
Delete retrieved mesh file is other than original one.
*/
void U2MaxMeshExport::UnlockMesh()
{
	if(m_bDeleteMesh)
	{
		m_pMaxMesh->DeleteThis();
	}

	m_pRefMaker->RenderEnd(0);
	m_pRefMaker = NULL;

	m_pTriObj = 0;
	m_pMaxMesh = 0;
	m_bDeleteMesh = false;
}


////-----------------------------------------------------------------------------
///**
//Retrieves mesh options from node custom attribute and create xml elements
//based on it. 
//
//@return 
//*/
//bool U2MaxMeshExport::


void U2MaxMeshExport::Export(U2Spatial** ppGeom,  INode* pMaxNode)
{
	m_pGeomNode = pMaxNode;

	U2Mesh *pTris = 0;

	Object* pObj = U2MaxUtil::GetBaseObject(pMaxNode, 0);


	// this is needed for an object which one is what any modifier is applied.
	if(pMaxNode->GetObjectRef() != pObj)
	{

	}
	
	BeginSkin(pMaxNode);

	if(!m_pMeshBuilder)
		m_pMeshBuilder = U2_NEW U2MeshBuilder;

	// Get the number of materials to check this mesh uses single material or multi material.
	// The reason to use GetNumMaterial instead of using nMaxMaterial::GetType() is that
	// there's multi-sub material type which only has one sub material.
	// In this case, we process this mesh as same way of a single material mesh.
	int numMaterials = U2MaxUtil::GetMaterialCnt(pMaxNode);
	if(numMaterials == 1)
	{
		// we have single material.
		int baseGroupIdx = GetMesh(pTris, pMaxNode, m_pMeshBuilder, -1, 1);
		U2ASSERT(pTris);
		
		m_tempGeoms.AddElem(pTris);
	

		if(baseGroupIdx >= 0)
		{
			//U2String nodeName(pINode->GetName());
			
			// Create engine type data
			SetSkinController(pMaxNode, pTris, numMaterials);
			
			
			// save group index for skin partitioning and mesh fragments.
			U2Aabb localBox;
			localBox = m_pMeshBuilder->GetGroupBBox(baseGroupIdx);

			float geomScale = U2MaxOptions::ms_fGeomScale;
			D3DXMATRIX scaleMat;
			D3DXMatrixScaling(&scaleMat, geomScale, geomScale, geomScale);
			localBox.Transform(scaleMat);


		}
	

		*ppGeom = pTris;

		(*ppGeom)->SetName(pMaxNode->GetName());

	}
	else 
	{

		// we have multi materials

		// Create nTransform node for a mesh has multi material.
		// Guess that, a multi material mesh has some other child mesh in its hierarchy.
		// The multi material mesh to be splited by its number of the material and create
		U2Node *pU2Node = U2_NEW U2Node;	
		pU2Node->SetName(U2DynString(TSTR("MultiMtl_Node")));

		//U2Aabb parentLocalBox;

		for(int matIdx=0; matIdx < numMaterials; ++matIdx)
		{
			int numVertices = m_pMeshBuilder->GetNumVertices();
			int baseGroupIdx = GetMesh(pTris, pMaxNode, m_pMeshBuilder, matIdx, numMaterials);
			U2ASSERT(pTris);			
				

			// if mesh with matIdx has no faces, ignore it at all.
			if(baseGroupIdx < 0)
			{
				U2_DELETE pTris;				
				continue;
			}

			pU2Node->AttachChild(pTris);
			pTris->SetName(pMaxNode->GetName());

			m_tempGeoms.AddElem(pTris);


			SetSkinController(pMaxNode, pTris, numMaterials);

			std::map<int, U2SkinController*> m_skinCtrlMap;

			if(numVertices == m_pMeshBuilder->GetNumVertices())
			{
				//HACK: this case might happen when we removed a material from slot.
				//      By the way, even inspite of the the material was removed, total number of 
				//      materials are not changed. I have NO idea that is a bug or intended.
				continue;
			}
		}

		*ppGeom = pU2Node;
		
	}


	if (this->IsSkinned() || this->IsPhysique())
	{
		EndSkin();
	}

}


//-----------------------------------------------------------------------------
/**
@return True, if the given 3dsmax geometry node has skin modifier.
*/
bool U2MaxMeshExport::IsSkinned()
{
	return m_bSkinned;
}
//-----------------------------------------------------------------------------
/**
@return True, if the given 3dsmax geometry node has physique modifier.
*/
bool U2MaxMeshExport::IsPhysique()
{
	return m_bPhysique;
}


//-----------------------------------------------------------------------------
/**
Determine given mesh's face has same material.

@param mesh point to the Mesh
@param faceIdx face index
@param matIdx material index
@param numMats number of material of the given Mesh.
*/
bool U2MaxMeshExport::HasSameMaterial(Mesh* mesh, int faceIdx, 
									  const int matIdx, const int numMats)
{
	// Retrieves the zero based material index of the 'i-th' face.
	MtlID matID = mesh->getFaceMtlIndex(faceIdx) % numMats;

	return ((matIdx == -1 && numMats == 1) || matID == matIdx);
}

//-----------------------------------------------------------------------------
/**
Negatively scaled node has inverted order of vertices. (counter clockwise)

For example, consider exporting mirrored objects. 
The mirrored objects has negative scale factor and the vertices of that 
should be exported with counter clockwise order not to be inverted.

@param m matrix of a node.
@return true if the given matrix has negative scaling.
*/
bool U2MaxMeshExport::HasNegativeScale(Matrix3& m)
{
	Point3 cp = CrossProd(m.GetRow(0),m.GetRow(1));
	if (DotProd(cp, m.GetRow(2)) < 0.0f)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
/**
'parity' of the matrix is set if one axis of the matrix is scaled negatively.

@return return parity of the given node's world space transform.
*/
bool U2MaxMeshExport::HasNegativeScale(INode* inode)
{
	U2ASSERT(inode);
	return (inode->GetNodeTM(0).Parity() ? true : false);
}



//-----------------------------------------------------------------------------
/**
Get mesh from given 3dsmax node.

@param inode 3dsmax's node.
@param meshBuilder mesh builder which to be used to save mesh.
@param matidx material index
@param numMats number of the material of given node
@param baseGroup
*/

int U2MaxMeshExport::GetMesh(U2Mesh*& pGeom, INode* pINode,  U2MeshBuilder* pMeshBuilder, const int matIdx, 
							 const int numMats, bool bWorldCoords)
{
	Mesh* pMaxMesh = LockMesh(pINode);
	if(!pMaxMesh)
	{
		FILE_LOG(logDEBUG) << TSTR("The node does not have mesh.") << pINode->GetName();

		return -1;
	}

	int numVerts = pMeshBuilder->GetNumVertices();
	int numTris = pMeshBuilder->GetNumTriangles();

	int numGroups;
	U2PrimitiveVec<U2MeshBuilder::Group*> groupMap;
	pMeshBuilder->BuildGroupMap(groupMap);
	numGroups = groupMap.FilledSize();

	int iVertCnt = 0;

	//TODO: check if we need vertex normal.
	// build vertex normal of the given mesh
	pMaxMesh->buildNormals();
	pMaxMesh->checkNormals(TRUE);

	m_pivotMat.IdentityMatrix();
	if(!IsPhysique() && !IsSkinned() && !bWorldCoords)
	{
		m_pivotMat = (pINode->GetObjectTM(0) * Inverse(pINode->GetNodeTM(0))) * m_pivotMat;
	}
	else 
		m_pivotMat = pINode->GetObjectTM(0);

	int i;
	int numFaces = pMaxMesh->getNumFaces();

	int v1, v2, v3;

	bool bHasFaces = false;

#ifdef _DEBUG
	FILE_LOG(logDEBUG) << TSTR("Num Faces : ") << numFaces << TSTR("NumVerts : ") <<numVerts 
		<< TSTR("NumTris : ") << numTris;
#endif

	// matIdx = -1은 모든 face들이 같은 material을 사용한다는 의미.
	for(i=0; i < numFaces; ++i)
	{
		 // skip any face that doesn't belong in this trishape
		if(!HasSameMaterial(pMaxMesh, i, matIdx, numMats))
			continue;

		bHasFaces = true;
		Face& face = pMaxMesh->faces[i];

		if(HasNegativeScale(pINode))
		{
			v1=0; v2= 2; v3 = 1;
		}
		else 
		{
			v1=0; v2= 1; v3= 2;
		}

		

		U2MeshBuilder::Vertex& vert1 = GetVertex(pMaxMesh, face, i, v1);
		U2MeshBuilder::Vertex& vert2 = GetVertex(pMaxMesh, face, i, v2);
		U2MeshBuilder::Vertex& vert3 = GetVertex(pMaxMesh, face, i, v3);

		pMeshBuilder->AddVertex(vert1);
		pMeshBuilder->AddVertex(vert2);
		pMeshBuilder->AddVertex(vert3);

		U2MeshBuilder::Triangle* tri = U2_NEW U2MeshBuilder::Triangle;
		int baseVertIdx = numVerts + iVertCnt;
		int baseGroupidx = numGroups;

		tri->SetVertexIndices(baseVertIdx, 
			baseVertIdx + 1, baseVertIdx + 2);

		if(IsPhysique() || IsSkinned())
		{
			tri->SetUsageFlags(U2MeshBuilder::WriteOnce | U2MeshBuilder::NeedsVertexShader);
		}
		else 
		{
			 tri->SetUsageFlags(U2MeshBuilder::WriteOnce);	
		}

		tri->SetGroupId(baseGroupidx);
		
		if(matIdx >= 0)
			tri->SetMaterialId(matIdx);
		
		pMeshBuilder->AddTriangle(*tri);

		iVertCnt += 3;		
	}

	UnlockMesh();

	U2TriListData *pData= U2_NEW U2TriListData();
	pGeom = U2_NEW U2TriList(pData);

#ifdef _DEBUG
	FILE_LOG(logDEBUG) << TSTR("Number of vertices of : ") << pINode->GetName() << TSTR(" node are ")
		 << pMeshBuilder->GetNumVertices();
#endif
	return bHasFaces ? numGroups : -1;
}

//-----------------------------------------------------------------------------
/**
Get per vertex component from 3dsmax's Mesh and specifies it to 
mesh builder's vertex.

@return mesh builder's vertex.
*/
U2MeshBuilder::Vertex& U2MaxMeshExport::GetVertex(Mesh* mesh, Face& face, int faceNo, int vIdx)
{
	U2MeshBuilder::Vertex* vertex = U2_NEW 	U2MeshBuilder::Vertex;
	U2ASSERT(vertex);

	// face.v[vIdx] : get the actual index of the vertex of the Mesh vertex array.
	D3DXVECTOR3 pos = GetVertexPosition(mesh, face.v[vIdx]);
	vertex->SetVert(pos);

	

	// FIXME: we don't use MAX's custom attributes for filtering each of vertex components
	//if( this->hasCustomAttr )
	//{
	//    // vertex normal.
	//    if (this->ExportNormals())
	//    {
	//        vector3 norm = GetVertexNormal(mesh, face, faceNo, vIdx);
	//        vertex.SetNormal(norm);
	//    }

	//    // vertex color.
	//    if (this->ExportColors())
	//    {
	//        vector4 col = GetVertexColor(mesh, faceNo, vIdx);
	//        vertex.SetColor(col);
	//    }

	//    // vertex uvs.
	//    if (this->ExportUvs())
	//    {
	//        //FIXME: is 'm' identical to map channel?
	//        int layer = 0;
	//        for (int m=1; m<MAX_MESHMAPS-1; m++)
	//        {
	//            if (mesh->mapSupport(m))
	//            {
	//                vector2 uvs = GetVertexUv(mesh, faceNo, vIdx, m);
	//                vertex.SetUv(layer++, uvs);
	//            }
	//        }
	//    }
	//}
	//else
	
	// vertex normal.
	D3DXVECTOR3 norm = GetVertexNormal(mesh, face, faceNo, vIdx);
	vertex->SetNormal(norm);
		

		// vertex color.
	D3DXVECTOR4 col = GetVertexColor(mesh, faceNo, vIdx);
	vertex->SetColor(col);
		

		// vertex uvs.
	//FIXME: is 'm' identical to map channel?
	int layer = 0;
	for (int m=1; m<MAX_MESHMAPS-1; m++)
	{
		if (mesh->mapSupport(m))
		{
			D3DXVECTOR2 uvs = GetVertexUv(mesh, faceNo, vIdx, m);
			vertex->SetUv(layer++, uvs);
		}
	}
	

	if (IsPhysique() || IsSkinned())
	{
		D3DXVECTOR4 joints, weights;

		// 내부 joints, weights 데이터 저장 미구현했으므로 추가 필요.
		this->GetVertexWeight(face.v[vIdx], joints, weights);

		vertex->SetBoneIndices(joints);
		vertex->SetWeights(weights);

#ifdef _DEBUG
		FILE_LOG(logDEBUG) << TSTR("pos : ") << pos.x << TSTR(" ") << pos.y <<
			TSTR(" ") << pos.z << TSTR(" ") << TSTR("Bone Indices : ") << 
			joints.x << TSTR(" ") << joints.y << TSTR(" ") << joints.z 
			<< TSTR(" ") << joints.w << TSTR(" ") << weights.x << TSTR(" ")
			<< weights.y << TSTR(" ") << weights.z << TSTR(" ") << weights.w;
#endif
	}
	return *vertex;
}



//-----------------------------------------------------------------------------
/**
Get vertex position.
*/
D3DXVECTOR3 U2MaxMeshExport::GetVertexPosition(Mesh* mesh, int index)
{
	D3DXVECTOR3 pos;

	Point3& v = mesh->verts[index] * m_pivotMat;

	pos = D3DXVECTOR3(-v.x, v.z, v.y);
	//pos = D3DXVECTOR3(v.x, v.z, v.y);

	return pos;
}

//-----------------------------------------------------------------------------
/**
Get vertex normal.
*/
D3DXVECTOR3 U2MaxMeshExport::GetVertexNormal(Mesh* mesh, Face& face, int faceNo, int vtxIdx)
{
	D3DXVECTOR3 normal;
	Point3& vn = GetVertexNormal(mesh, faceNo, mesh->getRVertPtr(face.getVert(vtxIdx)));
	normal = D3DXVECTOR3(-vn.x, vn.z, vn.y);

	return normal;
}

//-----------------------------------------------------------------------------
/**
Get vertex normal.
*/
Point3 U2MaxMeshExport::GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv)
{
	Face* f = &mesh->faces[faceNo];
	DWORD smGroup = f->smGroup;
	int numNormals;
	Point3 vertexNormal;

	// Is normal specified
	// SPECIFIED_NORMAL is not currently used, but may be used in future versions.
	if (rv->rFlags & SPECIFIED_NORMAL)
	{
		vertexNormal = rv->rn.getNormal();
	}
	// If normal is not specified it's only available if the face belongs
	// to a smoothing group
	else 
		if ((numNormals = rv->rFlags & NORCT_MASK) && smGroup) 
		{
			// If there is only one vertex is found in the rn member.
			if (numNormals == 1) 
			{
				vertexNormal = rv->rn.getNormal();
			}
			else
			{
				// If two or more vertices are there you need to step through them
				// and find the vertex with the same smoothing group as the current face.
				// You will find multiple normals in the ern member.
				for (int i = 0; i < numNormals; i++)
				{
					if (rv->ern[i].getSmGroup() & smGroup)
					{
						vertexNormal = rv->ern[i].getNormal();
					}
				}
			}
		}
		else
		{
			// Get the normal from the Face if no smoothing groups are there
			vertexNormal = mesh->getFaceNormal(faceNo);
		}

		return vertexNormal;
}

//-----------------------------------------------------------------------------
/**
Check the given mesh uses per vertex alpha or not.
*/
bool U2MaxMeshExport::UsePerVertexAlpha(Mesh* mesh)
{
	return (mesh->mapSupport(MAP_ALPHA) ? true : false);
}

//-----------------------------------------------------------------------------
/**
Get vertex color.
*/
D3DXVECTOR4 U2MaxMeshExport::GetVertexColor(Mesh* mesh, int faceNo, int vtxIdx)
{
	AColor color;

	if (mesh->getNumVertCol())
	{
		Point3 col = mesh->vertCol[mesh->vcFace[faceNo].t[vtxIdx]];

		color.r = col.x; color.g = col.y; color.b = col.z; color.a = 1.0f;
	}
	else
	{
		color.White();
	}

	if (UsePerVertexAlpha(mesh))
	{
		UVVert* uvVerts = mesh->mapVerts(MAP_ALPHA);
		if (uvVerts)
		{
			TVFace* tvFace = &mesh->mapFaces(MAP_ALPHA)[faceNo];
			color.a = uvVerts[tvFace->t[vtxIdx]].x;
		}
	}

	D3DXVECTOR4 vertexCol;
	vertexCol = D3DXVECTOR4(color.r, color.g, color.b, color.a);

	return vertexCol;
}


//-----------------------------------------------------------------------------
/**
Get vertex uv.
*/
D3DXVECTOR2 U2MaxMeshExport::GetVertexUv(Mesh* mesh, int faceNo, int vtxIdx, int m)
{
	D3DXVECTOR2 uvs;

	int numTV = mesh->getNumMapVerts(m);
	if (numTV)
	{
		int a = mesh->mapFaces(m)[faceNo].t[vtxIdx];

		Point3 pt1 = mesh->mapVerts(m)[a];

		uvs = D3DXVECTOR2(pt1.x, 1.0f - pt1.y);
	}
	else
	{
		switch(vtxIdx)
		{
		case 0:
			uvs.x = 0.0f, uvs.y = 0.0f;
			break;
		case 1:
			uvs.x =1.0f, uvs.y = 0.0f;
			break;
		case 2:
			uvs.x = .0f, uvs.y = 1.0f;
			break;
		}
	}

	return uvs;
}


//-----------------------------------------------------------------------------
/**
Extract vertex weight value from given vertex

@param vertexIdx vertex index.
*/
void U2MaxMeshExport::GetVertexWeight(int vertexIdx, D3DXVECTOR4 &boneIndices, 
									  D3DXVECTOR4 &boneWeights)
{
	for(uint32 i=0; i < m_weightBones.Size(); ++i)
	{
		U2_DELETE m_weightBones[i];
	}

	m_weightBones.RemoveAll();

	if(IsPhysique())
	{
		IPhyVertexExport* pPhyVertExp = (IPhyVertexExport*)m_pPhyCxtExport->
								GetVertexInterface(vertexIdx);
		if(pPhyVertExp)
		{
			int vertType = pPhyVertExp->GetVertexType();

			switch(vertType)
			{
			case RIGID_BLENDED_TYPE:
				{
					IPhyBlendedRigidVertex* pBlended;
					pBlended = (IPhyBlendedRigidVertex*)pPhyVertExp;

					for(int i=0; i < pBlended->GetNumberNodes(); ++i)
					{
						INode* pBone = pBlended->GetNode(i);
						float fWeight = pBlended->GetWeight(i);

						if(pBone)
						{
							//1. Search Bone 
							// 2. Save boneid and weight
							U2DynString boneName(pBone->GetName());

							int boneId = U2MaxAnimExport::Instance()->FindBoneIdByName(boneName);

							if(boneId >= 0)
							{
								AddBoneInfluence(boneId, fWeight);
							}
						}
						else 
						{
							FILE_LOG(logDEBUG) << 
								TSTR("No bone influence for this vertex: bone ") << pBone->GetName()
								<< TSTR(" vertex Index : ") << vertexIdx;
						}

					}

				}
				break;
			case RIGID_TYPE:
				{
					IPhyRigidVertex* pRigid;
					pRigid = (IPhyRigidVertex*)pPhyVertExp;

					INode* pBone = pRigid->GetNode();
					float fWeight = 1.0f;

					if(pBone)
					{
						//1. Search Bone 
						// 2. Save boneid and weight
						U2DynString boneName(pBone->GetName());

						int boneId = U2MaxAnimExport::Instance()->FindBoneIdByName(boneName);

						if(boneId >= 0)
						{
							AddBoneInfluence(boneId, fWeight);
						}
					}
					else 
					{
						FILE_LOG(logDEBUG) << 
							TSTR("No bone influence for this vertex: bone ") << pBone->GetName()
							<< TSTR(" vertex Index : ") << vertexIdx;
					}

				}
				break;
			}

			ReleaseVertexInterface(pPhyVertExp);
			pPhyVertExp = 0;
		}
		else 
		{

			FILE_LOG(logDEBUG) << TSTR("Invalid IPhyvertexExport");
			return;
		}
	}
	else if(IsSkinned())
	{
		ISkinContextData* pSkinCxtData = GetSkinContextData();
		
		const int numBones = pSkinCxtData->GetNumAssignedBones(vertexIdx);
		for(int i=0; i < numBones; ++i)
		{
			// retrieves the index of the bone that affecting the vertex.
			int boneIdx = pSkinCxtData->GetAssignedBone(vertexIdx, i);
			if(boneIdx < 0)
			{
				// bone index should not be less than zero.
				boneIdx = 0;
				continue;
			}

			INode* pBone = m_pSkin->GetBone(boneIdx);
			float fWeight = pSkinCxtData->GetBoneWeight(vertexIdx, i);


			if(pBone)
			{
				//1. Search Bone 
				// 2. Save boneid and weight
				U2DynString boneName(pBone->GetName());

				int boneId = U2MaxAnimExport::Instance()->FindBoneIdByName(boneName);

				if(boneId >= 0)
				{
					AddBoneInfluence(boneId, fWeight);
				}
			}
			else 
			{
				FILE_LOG(logDEBUG) << 
					TSTR("No bone influence for this vertex: bone ") << pBone->GetName()
					<< TSTR(" vertex Index : ") << vertexIdx;
			}

		}

	}
	else
	{
		// should not reach here!
		FILE_LOG(logDEBUG) << TSTR("Failed to get the physique or skin context for the given node : ") 
			 << m_pGeomNode->GetName();
	}
	
	 // calculate bone influences for most important four bones.
	float fWeightThreashold = U2MaxOptions::ms_fWeightThreshold;
	AdjustBonesInfluence(fWeightThreashold);

	boneIndices = GetBoneIndices();
	boneWeights = GetBoneWeights();
}


//-----------------------------------------------------------------------------
/**
Determine if the given mesh has physique or skin modifier.

@note
DO NOT FORGET TO CALL EndSkin()

@param node 3dsmax's node.
@return true, if the given mesh has physique or skin modifier
*/
bool U2MaxMeshExport::BeginSkin(INode* pINode)
{
	U2ASSERT(pINode);
	U2ASSERT(m_bBeginSkin == false);

	m_bBeginSkin = true;

	// Get object from given node.
	Object* pObj = pINode->GetObjectRef();
	if(!pObj)
	{
		return false;
	}

	// find physique first
	m_pModifier = U2MaxUtil::FindModifier(pObj, 
		Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B));
	if(m_pModifier)
	{
		m_pPhyExport = (IPhysiqueExport*)m_pModifier->GetInterface(I_PHYINTERFACE);
		if(NULL == m_pPhyExport)
		{
			return false;
		}

		m_pPhyCxtExport = (IPhyContextExport*)m_pPhyExport->GetContextInterface(pINode);
		if(NULL == m_pPhyCxtExport)
		{
			return false;
		}

		// we convert all vertices to rigid
		m_pPhyCxtExport->ConvertToRigid(TRUE);
		m_pPhyCxtExport->AllowBlending(TRUE);

		Matrix3 initTM;
		int res = m_pPhyExport->GetInitNodeTM(pINode, initTM);
		if(res != MATRIX_RETURNED)
		{
			initTM = pINode->GetObjectTM(0);
		}

		m_bPhysique = true;
		m_bSkinned = false;

		return true;		

	}
	else 
	{
		// no physique, so we try to find skin modifier.
		m_pModifier = U2MaxUtil::FindModifier(pObj, SKIN_CLASSID);
		if(!m_pModifier)
		{
			return false;
		}

		m_pSkin = (ISkin*)m_pModifier->GetInterface(I_SKIN);
		if(!m_pSkin)
		{
			return false;
		}

		m_pSkinContext = m_pSkin->GetContextInterface(pINode);

		m_bPhysique = false;
		m_bSkinned = true;

		return true;
	}

	m_pObj = pObj;

	// not physique and skin
	return false;

}

//-------------------------------------------------------------------------------------------------
void U2MaxMeshExport::AddBoneInfluence(int iBone, float fWeight)
{	
	bool bAdd = true;
	for(uint32 i=0; i < m_weightBones.Size(); ++i)
	{
		if(m_weightBones[i])
		{
			BoneInfluence& bi = *m_weightBones[i];
			if(bi.bone == iBone)
			{
				bi.weight+= fWeight;
				bAdd = false;
				break;
			}
		}

	}

	if(bAdd)
	{
		BoneInfluence *pNewBI = U2_NEW BoneInfluence;
		pNewBI->bone = iBone;
		pNewBI->weight = fWeight;		

		m_weightBones.AddElem(pNewBI);
	}
}

//-------------------------------------------------------------------------------------------------
void U2MaxMeshExport::AdjustBonesInfluence(float fWeightThreshold)
{
	uint32 i = 0;

	while(1)
	{
		if(i >= m_weightBones.Size())
			break;

		if(m_weightBones[i])
		{

			BoneInfluence& bi = *m_weightBones[i];

			if(bi.weight < fWeightThreshold)
			{
				U2_DELETE m_weightBones[i];
				m_weightBones.Remove(i);					
				m_weightBones.Refresh();
			}		
		}
		++i;
	}

	while(m_weightBones.FilledSize() > 4)
	{
		float fMinInfluence = 1.0f;
		int iMinInfluencePos;

		i=0;
		while(1)
		{
			if(i >= m_weightBones.FilledSize())
				break;

			if(m_weightBones[i])
			{	

				BoneInfluence& bi = *m_weightBones[i];

				if(bi.weight < fMinInfluence)
				{
					fMinInfluence = bi.weight;
					iMinInfluencePos = i;
				}
				
			}
			++i;
		}

		if(m_weightBones[iMinInfluencePos])
		{
			U2_DELETE m_weightBones[iMinInfluencePos];
			m_weightBones.Remove(iMinInfluencePos);			
			m_weightBones.Refresh();
		}

	}

	// normalize influences
	float fTotalWeight = 0.0f;
	for(i = 0; i < m_weightBones.Size(); ++i)
	{
		if(m_weightBones[i])
		{
			BoneInfluence& bi = *m_weightBones[i];
			fTotalWeight += bi.weight;
		}
	}

	if(fTotalWeight < ZERO_TOLERANCE && m_weightBones.Size() > 0)
	{
		fTotalWeight = 1.0f / (float)m_weightBones.Size();
	}

	for(i=0; i < m_weightBones.Size(); ++i)
	{
		if(m_weightBones[i])
		{		
			BoneInfluence& bi = *m_weightBones[i];
			bi.weight /= fTotalWeight;
		}
	}


}

D3DXVECTOR4 U2MaxMeshExport::GetBoneIndices()
{
	D3DXVECTOR4 v(VECTOR4_ZERO);
	for(uint32 i=0; i < U2Math::Min<uint32>(m_weightBones.Size(), 4); 
		++i)
	{
		if(m_weightBones[i])
		{		
			BoneInfluence& bi = *m_weightBones[i];
			switch(i)
			{
			case 0:
				v.x = (float)bi.bone;
				break;
			case 1:
				v.y = (float)bi.bone;
				break;
			case 2:
				v.z = (float)bi.bone;
				break;
			case 3:
				v.w = (float)bi.bone;
				break;
			}
		}
	}

	return v;
}

D3DXVECTOR4 U2MaxMeshExport::GetBoneWeights()
{
	D3DXVECTOR4 v(VECTOR4_ZERO);
	for(uint32 i=0; i < U2Math::Min<uint32>(m_weightBones.Size(), 4); 
		++i)
	{
		if(m_weightBones[i])
		{
			BoneInfluence& bi = *m_weightBones[i];
			switch(i)
			{
			case 0:
				v.x = (float)bi.weight;
				break;
			case 1:
				v.y = (float)bi.weight;
				break;
			case 2:
				v.z = (float)bi.weight;
				break;
			case 3:
				v.w = (float)bi.weight;
				break;
			}
		}
	}

	return v;

}



void U2MaxMeshExport::EndSkin() 
{
	U2ASSERT(m_bBeginSkin == true);

	if(m_bPhysique)
	{
		m_pPhyExport->ReleaseContextInterface(m_pPhyCxtExport);
		m_pModifier->ReleaseInterface(I_PHYINTERFACE, m_pPhyExport);
	}

	m_bBeginSkin = false;
	m_pModifier = NULL;
	m_pPhyExport = NULL;
	m_pPhyCxtExport = NULL;
}


//-----------------------------------------------------------------------------
/**
Release retrieved vertex interface.
*/
void U2MaxMeshExport::ReleaseVertexInterface(IPhyVertexExport* phyVertexExport)
{
	m_pPhyCxtExport->ReleaseVertexInterface(phyVertexExport);
}

//-----------------------------------------------------------------------------
/**
Retrieves interface of skin context data.
*/
ISkinContextData* U2MaxMeshExport::GetSkinContextData()
{
	return m_pSkinContext;
}

//-------------------------------------------------------------------------------------------------
void U2MaxMeshExport::SetBaseGroupIdx(int baseGroupIdx)
{
	if(IsSkinned() || IsPhysique())
	{
		if(U2MaxOptions::ms_eMeshType == Shape)
		{
			int numGeoms = m_tempGeoms.FilledSize();
			for(int i=0; i < numGeoms; ++i)
			{
				U2Mesh* pMesh = m_tempGeoms[i];

				U2MaxSkineMeshData* pGroupedSkinMesh = U2_NEW U2MaxSkineMeshData;
				memset(pGroupedSkinMesh, 0, sizeof(U2MaxSkineMeshData));
				pGroupedSkinMesh->m_iGroupIdx = baseGroupIdx + i;
				pGroupedSkinMesh->m_pMesh = pMesh;
				
				m_skinnedGroupMeshes.AddElem(pGroupedSkinMesh);							
			}
		}
	}
	else 
	{
		if(U2MaxOptions::ms_eMeshType == Shape)
		{
			int numGeoms = m_tempGeoms.FilledSize();
			for(int i=0; i < numGeoms; ++i)
			{
				U2Mesh* pMesh = m_tempGeoms[i];
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
void U2MaxMeshExport::SetSkinController(INode* pMaxNode, U2Mesh* pMesh, int numMaterials)
{
	if(IsSkinned() || IsPhysique())
	{
		U2SkinModifier* pSkinMod = U2_NEW U2SkinModifier;
		pMesh->SetSkinModifier(pSkinMod);

		int skelIdx = U2MaxAnimExport::Instance()->GetSkeletonForNode(pMaxNode);
		if(numMaterials > 1)
		{
			U2DynString szSkinCtrl(_T("multimaterials_skincontroller"));
			if(skelIdx != -1)
				szSkinCtrl.AppendInt(skelIdx);
			else 
				szSkinCtrl.AppendInt(0);

			std::map<int, U2SkinController*>::iterator iter = 
			U2MaxAnimExport::Instance()->m_skinCtrlMap.find(skelIdx);
			if(U2MaxAnimExport::Instance()->m_skinCtrlMap.end() !=
				iter)
			{
				iter->second->SetName(szSkinCtrl);
			}

		}
		else 
		{
			U2DynString szSkinCtrl(_T("skincontroller"));
			if(skelIdx != -1)
				szSkinCtrl.AppendInt(skelIdx);
			else 
				szSkinCtrl.AppendInt(0);
		}
	}
}

bool U2MaxMeshExport::BuildMeshTangentNormals(U2MeshBuilder& meshBuilder)
{
	const U2MeshBuilder::Vertex& v = meshBuilder.GetVertexAt(0);

	bool buildTangentNormals = true;

	// Check Export Option 

	if (buildTangentNormals)
	{
		if (false == v.HasComponent(U2MeshBuilder::Vertex::UV0))
		{
			return false;
		}

		meshBuilder.BuildTriangleNormals();

		if (false == v.HasComponent(U2MeshBuilder::Vertex::NORMAL))
		{
			meshBuilder.BuildVertexNormals();
		}

		if ( true == U2MaxOptions::ms_bTangent)
			meshBuilder.BuildVertexTangents(true);
	}

	return true;
}
