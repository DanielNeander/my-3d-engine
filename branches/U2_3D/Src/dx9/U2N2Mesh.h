/************************************************************************
module	:	U2N2Mesh
Author	:	Yun sangyong
Desc	:
************************************************************************/
#ifndef	U2_N2MESH_H
#define	U2_N2MESH_H
#pragma  once

#include "U2Triangle.h"
#include <U2_3D/src/collision/U2Aabb.h>
#include <U2_3d/src/object/U2XMeshLoader.h>
#include <U2_3d/src/dx9/nMeshGroup.h>
#include <U2Lib/src/U2TVec.h>

class U2_3D U2N2Mesh : public U2Triangle 
{
	DECLARE_RTTI;
public:

	virtual ~U2N2Mesh();

	enum VertexComponent
	{
		Coord    = (1<<0),
		Normal   = (1<<1),
		Uv0      = (1<<2),
		Uv1      = (1<<3),
		Uv2      = (1<<4),
		Uv3      = (1<<5),
		Color    = (1<<6),
		Tangent  = (1<<7),
		Binormal = (1<<8),
		Weights  = (1<<9),
		JIndices = (1<<10),
		Coord4   = (1<<11),

		NumVertexComponents = 12,
		AllComponents = ((1<<NumVertexComponents) - 1),
	};

	enum Usage
	{
		// read/write behavior (mutually exclusive)
		WriteOnce = (1<<0),     ///< (default) CPU only fills the vertex buffer once, and never touches it again
		ReadOnly  = (1<<1),     ///< CPU reads from the vertex buffer, which can never be rendered
		WriteOnly = (1<<2),     ///< CPU writes frequently to vertex buffer, but never read data back
		ReadWrite = (1<<3),     ///< CPU writes and reads the vertex buffer, which is also rendered

		// patch types (mutually exclusive)
		NPatch  = (1<<4),
		RTPatch = (1<<5),

		// use as point sprite buffer?
		PointSprite = (1<<6),

		// needs vertex shader?
		NeedsVertexShader = (1<<7),
	};

	enum OptimizationFlag
	{
		Faces       = (1<<0),
		Vertices    = (1<<1),
		AllOptimizations = Faces | Vertices
	};

	enum
	{
		InvalidIndex = 0xffff, // invalid index constant
	};

	struct Edge : public U2MemObj
	{
		uint16 faceIdx[2];	// face indices  the 2nd face index could be = InvalidIndex when the edge is a geometry border
		uint16 vertexIdx[2];	// vertex indices		
	};

	U2N2Mesh(U2TriangleData* pData);

	virtual void UpdateWorldData(float fAppTime);

	void SetVertexBufferByteSize(unsigned int sizeInBytes);
	unsigned int GetVertexBufferByteSize() const;

	void SetIndexBufferByteSize(unsigned int sizeInBytes);
	unsigned int GetIndexBufferByteSize() const;

	bool CreatevertexBuffer();
	float* LockVertices();
	void UnlockVertices();

	bool CreateIndexBuffer();	
	uint16* LockIndices();
	void UnlockIndices();

	// shadow volume relate
	void CreateEdgeBuffer();
	Edge* GetEdges();	
	void SetNumEdges(uint16 size);
	uint16 GetNumEdges() const;
	
	void SetEdgeBufferSize(uint32 sizeInBytes);
	uint32 GetEdgeBufferSize() const;


	void SetUsages(int useFlags);
	void SetVertexUsage(int useFlag);
	void SetIndexUsage(int useFlag);
	void SetVertexComponents(int compMask);

	/// optimize by reorganizing faces
	void OptimizeFaces(uint16* indices, int numFaces, int numVertices);
	/// optimize by reorganizing vertices
	void OptimizeVertices(float* vertices, uint16* indices, int numVertices, int numFaces);

	void UpdateGroupBBoxes(float* pVertsData, uint16* pIndicesData);

	bool CreateEmptyBuffers();
	bool LoadFile(U2MeshLoader* pLoader);

	U2VertexAttributes* CreateVertexAttrib(unsigned int compMask);

	virtual void GetTriangle(uint16 usTriangle, D3DXVECTOR3*& pt0, 
		D3DXVECTOR3*& pt1, D3DXVECTOR3*& pt2);

	LPDIRECT3DVERTEXBUFFER9 GetVB() const;

	virtual void ApplyMesh(U2SceneMgr* pSceneMgr, const nMeshGroup& curGroup);
	virtual void ApplyShader(U2SceneMgr* pSceneMgr);
	void UpdateEffectShader(U2SceneMgr* pSceneMgr, U2RenderContext* renderContext);
	
	// 2010/06/07 modified
	// primitive type 
	void Render(U2SceneMgr* pSceneMgr, U2RenderContext* pRenderContext);

	

	uint32 GetVertexStride() const;
	void SetVertexStride(uint32 uStride);

	// Nebula Device Support
	void CreateMeshGroups(uint32 num);
	int GetNumGroups() const;

	void SetGroupIdx(uint32 i);
	uint32 GetGroupIdx() const;

	nMeshGroup& Group(uint32 idx) const;
	
	// 2010/06/08 changed 
	// TerrainMeshGroup을 추가하기 위해 protected로 변경.
	nMeshGroup* GroupPtr(uint32 idx) const;

protected:
	U2ObjVec<U2MeshGroupPtr> m_meshGroups;


private:
	unsigned int m_uVbSize;
	unsigned int m_uIbSize;
	uint32 m_uiEbSize;

	int m_iVertexUsage;
	int m_iIndexUsage;
	int m_iVertexCompMask;
	DWORD m_dwD3DVBLockFlags;
	DWORD m_dwD3DIBLockFlags;
	IDirect3DVertexBuffer9* m_pVB;
	void* m_pvVretexBuffer;
	void* m_pvIndexBuffer;

	
	uint32 m_uiVertexStride;

	// shadow volume
	uint16 m_usNumEdges;	

	Edge* m_pEdgeBuffer;
	
		
};

typedef U2SmartPtr<U2N2Mesh> U2N2MeshPtr;

#include "U2N2Mesh.inl"

#endif 