/**************************************************************************************************
module	:	nMeshGrup
Author	:	Yun sangyong
Desc	:	Frsutum Culling을 위해 바운딩 정보 필요..
			u2SkinModifier는 nMeshGroup 전체에 공유할 수 있으므로 
			삭제하지 말 것.. 
**************************************************************************************************/
#pragma once
#ifndef N_MESHGROUP_H
#define N_MESHGROUP_H
//------------------------------------------------------------------------------
/**
@class nMeshGroup
@ingroup Gfx2

A nMeshGroup object defines a group of vertices and indices inside
a mesh.

(C) 2002 RadonLabs GmbH
*/
//------------------------------------------------------------------------------
//#include <U2Lib/src/U2RefObject.h>
//#include <U2Lib/src/U2SmartPtr.h>
#include <U2_3D/Src/collision/U2Aabb.h>
#include <U2_3D/Src/dx9/U2Triangle.h>
#include <U2_3D/Src/Dx9/U2SkinModifier.h>
#include <U2_3D/Src/animation/U2SkinController.h>

class U2N2Mesh;
class U2SceneMgr;
class U2BonePalette;

class U2_3D nMeshGroup : public U2Triangle
{
public:
	DECLARE_RTTI;

	/// constructor
	nMeshGroup();
	/// destructor
	~nMeshGroup();

	virtual void GetTriangle(uint16 usTriangle, D3DXVECTOR3*& pt0, 
		D3DXVECTOR3*& pt1, D3DXVECTOR3*& pt2);

	nMeshGroup& operator=(const nMeshGroup& rhs);
	/// set first vertex
	void SetFirstVertex(int i);
	/// get index of first vertex
	int GetFirstVertex() const;
	/// set number of vertices
	void SetNumVertices(int i);
	/// get number of vertices
	int GetNumVertices() const;
	/// set first index
	void SetFirstIndex(int i);
	/// get index of first primitive index
	int GetFirstIndex() const;
	/// set number of indices
	void SetNumIndices(int i);
	/// get number of indices
	int GetNumIndices() const;
	/// set first edge
	void SetFirstEdge(int i);
	/// get index of first edge
	int GetFirstEdge() const;
	/// set number of edges
	void SetNumEdges(int i);
	/// get number of edges
	int GetNumEdges() const;
		

	void SetMesh(U2N2Mesh* pMesh);
	U2N2Mesh* GetMesh() const;

	void SetSkinModifier(U2SkinModifier *pSkinModifier);
	U2SkinModifier* GetSkinModifier() const;

	virtual void UpdateRenderStates(U2RenderStateCollectionPtr spRSC = 0, 
		U2ObjVec<U2LightPtr>* pLightArray = 0) {}
	
	virtual void UpdateRenderContext(U2RenderContext* pRCxt);
	
	virtual bool Render(U2SceneMgr* pSceneMgr, U2RenderContext* pCxt);

protected:	
	virtual void UpdateState(U2RenderStateCollectionPtr spRS , 
		U2ObjVec<U2LightPtr>* pLightArray) {}

	virtual void RenderShaderSkinning(int meshGroupIdx, U2BonePalette& bonePalette);
	 
private:
	int firstVertex;
	int numVertices;
	int firstIndex;
	int numIndices;
	int firstEdge;
	int numEdges;
	

	U2SkinModifier* m_pSkinMod;
	U2N2Mesh* m_pOwnerMesh;
};

typedef U2SmartPtr<nMeshGroup> U2MeshGroupPtr;

//------------------------------------------------------------------------------
/**
*/
inline
nMeshGroup::nMeshGroup() :
firstVertex(0),
numVertices(0),
firstIndex(0),
numIndices(0),
firstEdge(0),
numEdges(0),
m_pSkinMod(0),
m_pOwnerMesh(0)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
inline
nMeshGroup::~nMeshGroup()
{
	
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nMeshGroup::SetFirstVertex(int i)
{
	this->firstVertex = i;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
nMeshGroup::GetFirstVertex() const
{
	return this->firstVertex;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nMeshGroup::SetNumVertices(int i)
{
	this->numVertices = i;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
nMeshGroup::GetNumVertices() const
{
	return this->numVertices;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nMeshGroup::SetFirstIndex(int i)
{
	this->firstIndex = i;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
nMeshGroup::GetFirstIndex() const
{
	return this->firstIndex;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nMeshGroup::SetNumIndices(int i)
{
	this->numIndices = i;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
nMeshGroup::GetNumIndices() const
{
	return this->numIndices;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nMeshGroup::SetFirstEdge(int i)
{
	this->firstEdge = i;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
nMeshGroup::GetFirstEdge() const
{
	return this->firstEdge;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
nMeshGroup::SetNumEdges(int i)
{
	this->numEdges = i;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
nMeshGroup::GetNumEdges() const
{
	return this->numEdges;
}


//------------------------------------------------------------------------------
inline 
void 
nMeshGroup::SetSkinModifier(U2SkinModifier *pSkinModifier)
{
	m_pSkinMod = pSkinModifier;
}

//-------------------------------------------------------------------------------------------------
inline 
U2SkinModifier* nMeshGroup::GetSkinModifier() const
{
	return m_pSkinMod;
}

inline void nMeshGroup::SetMesh(U2N2Mesh* pMesh)
{
	m_pOwnerMesh = pMesh;
}

inline U2N2Mesh* nMeshGroup::GetMesh() const
{
	return m_pOwnerMesh;
}

inline void nMeshGroup::UpdateRenderContext(U2RenderContext* pCxt)
{
	//if(m_pSkinMod)
	//{
	//	m_pSkinMod->GetSkinController()->Initialize(pCxt);
	//}
	for(unsigned int i=0; i < GetNumControllers(); ++i)
	{
		if(!GetController(i)->IsInitialize())
			GetController(i)->Initialize(pCxt);
	}
}


#endif
