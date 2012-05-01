/************************************************************************
module	:	U2Dx9ShadowCaster
Author	:	Yun sangyong
Desc	:	Light�� �޴� mesh�κ��� Shadow Volume�� �����.
			Static, Skinned Mesh�� �����Ͽ� ������ Ȯ�� Ŭ������ �����Ѵ�.
			m_spShadowVolume�� indexBuffer�� �����ϴµ� ������ ������.
************************************************************************/
#pragma once 
#ifndef U2_DX9_SHADOWCASTER_H
#define U2_DX9_SHADOWCASTER_H

#include <u2_3d/src/main/U2Object.h>

U2SmartPointer(U2N2Mesh);

class U2_3D U2Dx9ShadowCaster : public U2Object
{
public:

	enum ShadowType 
	{
		SHADOW_INVALID = -1,
		SHADOW_STATIC,
		SHADOW_DYNAMIC, 
		MAX_SHADOW_TYPES,		
	};

	U2Dx9ShadowCaster();
	virtual ~U2Dx9ShadowCaster();

	virtual void SetupShadowVolume(const U2Light& light, const D3DXMATRIX& invModelLitMat) {}
	virtual void RenderShadowVolume() {}
	void SetMeshGroupIdx(uint32 i);
	uint32 GetMeshGroupIdx() const;

protected:
	bool CreateShadowVolume(U2N2Mesh* pMesh); 
	void DestroyShadowVolume();

	/// update the face normals and midpoints from a mesh object
	void UpdateFaceNormalsAndMidpoints(U2N2Mesh* pMesh);
	
	/// update the face lit/unlit flags in the faces array	
	void UpdateFaceLitFlags(const U2Light& light, const D3DXMATRIX& invModelLitMat);

	void BeginWriteIndices(U2N2Mesh* pMesh);

	void WriteSideIndices();
	void WriteCapIndices(bool bLit);
	void EndWriteIndices();

	uint32 GetNumDrawIndices();

	bool LightingChanged(const U2Light& light, const D3DXMATRIX& invModelLitMat);

	struct Face : public U2MemObj
	{
		D3DXVECTOR3 normal;
		D3DXVECTOR3 point;
		bool bLit;
	};

	U2N2MeshPtr m_spShadowVolume;
	U2FixedPrimitiveArray<Face*> m_faces;
	uint32 m_uiMeshGroupIdx;

private:
	

	friend class U2Dx9ShadowMgr;

	// write a extruded quad to the index buffer
	void WriteQuad(uint16 idx0, uint16 idx1);

	U2Light::LightType m_ePrevLitType;
	D3DXVECTOR3 m_prevLitPosOrDir;

	uint16* m_pIndicesStart;
	uint16* m_pIndicesCurrent;
	uint16 m_uiDrawNumIndices;
	U2N2Mesh* m_pSrcMesh;
	uint16* m_pSrcIndices;
	U2N2Mesh::Edge* m_pSrcEdges;

};

//-------------------------------------------------------------------------------------------------
inline void U2Dx9ShadowCaster::SetMeshGroupIdx(uint32 i)
{
	m_uiMeshGroupIdx = i;
}

//-------------------------------------------------------------------------------------------------
inline uint32 U2Dx9ShadowCaster::GetMeshGroupIdx() const
{
	return m_uiMeshGroupIdx;
}

//-------------------------------------------------------------------------------------------------
inline uint32 U2Dx9ShadowCaster::GetNumDrawIndices()
{
	return m_uiDrawNumIndices;	
}


#endif