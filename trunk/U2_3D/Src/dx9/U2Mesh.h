/************************************************************************
module	:	U2Mesh
Author	:	Yun sangyong
Desc	:
************************************************************************/
#ifndef	U2_MESH_H
#define	U2_MESH_H
#pragma  once

#include <U2_3D/src/U2_3DLibType.h>
#include <U2_3D/src/Main/U2Spatial.h>
#include <U2Lib/Src/U2SmartPtr.h>
#include "U2MeshData.h"
#include <U2_3D/src/Effect/U2LightEffect.h>


//#include "nMeshGroup.h"

class U2SceneMgr;
U2SmartPointer(U2SkinModifier);

class U2_3D U2Mesh : public U2Spatial 
{
	DECLARE_RTTI;	
public:
	virtual ~U2Mesh();

	U2BoundingVolumePtr m_spModelBound;

	enum 
	{
		TANGENTSPACE_NONE = -2,
		TANGENTSPACE_USE_MESH  = -1
	};

	
	virtual void UpdateMeshState(bool bUpdateNormals = true, 
		int iTangentSpaceType = TANGENTSPACE_NONE);

	
	unsigned short GetVertexCount() const;
	D3DXVECTOR3* GetVertices() const;
	void SetActiveVertexCount(unsigned short usActive);
	unsigned short GetActiveVertexCount() const;


	void CreateNormals();
	D3DXVECTOR3* GetNormals() const;


	void CreateColors();
	D3DXCOLOR* GetColors() const;	

	void CreateTexCoords(unsigned short numTexCoorSets);
	D3DXVECTOR2* GetTexCoordSet(
		unsigned short texcoordIdx) const;
	D3DXVECTOR2* GetTexCoords() const;	
	void AppendTexCoords(D3DXVECTOR2* pAddedTexCoords);
	unsigned short GetTexCoordSetCnt() const;

	
	virtual void GetVisibleSet(U2Culler& culler, bool bNoCull);

	virtual void Render(U2Dx9Renderer* pRenderer);

	void SetModelData(U2MeshData* pMeshData);
	U2MeshData* GetMeshData() const;

	U2LightEffectPtr m_spLitEffect;

	virtual void UpdateWorldData (float fAppTime);

	void SetSkinModifier(U2SkinModifier* pSkinMod);
	U2SkinModifier* GetSkinModifier() const;

	// 나중에 구조 바뀌면 수정..	

	enum GeometryType 
	{
		GT_POLYPOINT, 
		GT_POLYLINE_OPEN, 
		GT_POLYLINE_CLOSED,
		GT_TRILIST,
		GT_TRISTRIP,
		ST_TRIFAN,
		GT_MAX_COUNT
	};
	
	D3DXMATRIX m_matHWorld;

	void LightOnOff(bool bOnOff);
	 bool GetLightOnOff() const;

	 // computes world space bounding box (8 corner points)
	 void CalculateAABB(void);

	 // object space bounding box
	U2Obb *m_pObb;
	// world space bounding box (use CalculateAABB to update)
	U2Aabb* m_pBBox;

	 
protected:
	U2Mesh(U2MeshData* pMeshData);
	U2Mesh();
	
	virtual void UpdateModelBound ();	
	virtual void UpdateModelNormals();
	virtual void UpdateModelTangentSpace(int iTangentSpaceType);
	virtual void UpdateWorldBound ();

	virtual void UpdateState(U2RenderStateCollectionPtr spRS , 
		U2ObjVec<U2LightPtr>* pLightArray);


	U2SkinModifierPtr m_spSkinMod;
	
	
	U2MeshDataPtr m_spModelData;

	bool m_bLightEnable; 
	unsigned int m_uNumGroups;
	unsigned int m_uiGroupIdx;

};

#include "U2Mesh.inl"

typedef U2SmartPtr<U2Mesh> U2MeshPtr;



#endif