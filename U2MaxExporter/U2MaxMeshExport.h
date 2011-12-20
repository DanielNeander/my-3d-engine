/**************************************************************************************************
module	:	U2MaxMeshExport
Author	:	Yun sangyong
Desc	:  Multi Sub Mesh Attach 미구현..
			SetBaseGroupIdx 미구현
*************************************************************************************************/
#pragma once
#ifndef U2_MAXMESHEXPORT_H
#define U2_MAXMESHEXPORT_H

#include "U2MaxLib.h"
#include "U2MaxOptions.h"

class U2SkinModifier;
class U2Mesh;

class U2MaxSkineMeshData : public U2MemObj
{
public:
	U2Mesh* m_pMesh;
	int m_iGroupIdx;
};


class U2MaxMeshExport : public U2MemObj
{

public:
	struct BoneInfluence : public U2MemObj
	{
		int bone;
		float weight;
	};


	U2MaxMeshExport();
	~U2MaxMeshExport();

	static void Preprocess(INode* pNode);
	static void Postprocess(INode* pNode);


	Mesh* GetMeshFromRenderMesh(INode* pINode, BOOL &bNeedDelete);
	Mesh* GetMeshFromTriObject(INode* inode, BOOL &needDelete);
	Mesh* LockMesh(INode* pINode);
	void UnlockMesh();


	void Export(U2Spatial** ppGeom, INode* pINode);

	bool IsSkinned();
	bool IsPhysique();
	bool HasSameMaterial(Mesh* mesh, int faceIdx, 
		const int matIdx, const int numMats);

	bool HasNegativeScale(Matrix3& m);
	bool HasNegativeScale(INode* inode);

	int GetMesh(U2Mesh*& pGeom, INode* pINode,  U2MeshBuilder* pMeshBuilder, 
		const int matIdx, const int numMats, bool bWorldCoords = false);

	U2MeshBuilder::Vertex& GetVertex(Mesh* mesh, Face& face, 
		int faceNo, int vIdx);

	D3DXVECTOR3 GetVertexPosition(Mesh* mesh, int index);
	D3DXVECTOR3 GetVertexNormal(Mesh* mesh, Face& face, int faceNo, int vtxIdx);

	Point3 GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);

	bool UsePerVertexAlpha(Mesh* mesh);

	D3DXVECTOR4 GetVertexColor(Mesh* mesh, int faceNo, int vtxIdx);

	D3DXVECTOR2 GetVertexUv(Mesh* mesh, int faceNo, int vtxIdx, int m);

	void GetVertexWeight(int vertexIdx, D3DXVECTOR4 &boneIndices, 
		D3DXVECTOR4 &boneWeights);

	void ReleaseVertexInterface(IPhyVertexExport* phyVertexExport);

	ISkinContextData* GetSkinContextData();

	bool BeginSkin(INode* pINode);

	void EndSkin();

	void SetBaseGroupIdx(int baseGroupIdx);

	uint32 GetNumGroupMeshes() const;

	const U2MaxSkineMeshData& GetGroupMesh(const uint32 idx);

	int GetGroupIdx(U2MeshBuilder* pMeshBuilder);

	bool BuildMeshTangentNormals(U2MeshBuilder& meshBuilder);



	void AddBoneInfluence(int iBone, float fWeight);
	void AdjustBonesInfluence(float fWeightThreshold);
	D3DXVECTOR4 GetBoneIndices();
	D3DXVECTOR4 GetBoneWeights();
	
	U2MeshBuilder* m_pMeshBuilder;


private:

	void SetSkinController(INode* pMaxNode, U2Mesh* pMesh, int numMaterials);


	// mesh's vertex options.
	enum
	{
		VertexNormal  = 0x01,
		VertexColor   = 0x02,
		VertexUvs     = 0x04,
		VertexTangent = 0x08,
	};	

	bool m_bSkinned;
	bool m_bPhysique;
	bool m_bBeginSkin;

	Modifier* m_pModifier;
	IPhysiqueExport* m_pPhyExport;
	IPhyContextExport* m_pPhyCxtExport;

	ISkin* m_pSkin;
	ISkinContextData* m_pSkinContext;
	Object* m_pObj;

	INode* m_pGeomNode;
	
	U2PrimitiveVec<U2Mesh*> m_tempGeoms;
	U2PrimitiveVec<U2MaxSkineMeshData*> m_skinnedGroupMeshes;

	ReferenceMaker* m_pRefMaker;		
	Mesh* m_pMaxMesh;
	BOOL m_bDeleteMesh;
	TriObject* m_pTriObj;

	Matrix3 m_pivotMat;

	U2String m_geomNodeName;

	TimeValue m_animStartTime, m_animEndTime;

	U2PrimitiveVec<BoneInfluence*> m_weightBones;
	
};

//-------------------------------------------------------------------------------------------------
inline uint32 U2MaxMeshExport::GetNumGroupMeshes() const
{
	return m_skinnedGroupMeshes.Size();
}

//-------------------------------------------------------------------------------------------------
inline const U2MaxSkineMeshData& U2MaxMeshExport::GetGroupMesh(const uint32 idx)
{
	return *m_skinnedGroupMeshes[idx];
}

//-----------------------------------------------------------------------------
/**
Retrieves group index of given mesh.
*/
inline int U2MaxMeshExport::GetGroupIdx(U2MeshBuilder* pMeshBuilder)
{
	U2ASSERT(pMeshBuilder);

	U2PrimitiveVec<U2MeshBuilder::Group*> groupMap;

	pMeshBuilder->BuildGroupMap(groupMap);

	int res = (int)groupMap.Size();

	for(int i=0; i < groupMap.Size(); ++i)
	{
		U2_DELETE groupMap[i];
	}
	groupMap.RemoveAll();

	return res;
}



#endif