/**************************************************************************************************
module	:	U2MeshBuilder
Author	:	Yun sangyong
Desc	:	// m_iUsageFlags 정의 필요..
			nMeshGroup
*************************************************************************************************/
#pragma once
#ifndef U2_MESHBUILDER_H
#define U2_MESHBUILDER_H


#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>
#include <U2Lib/Src/U2THashTable.h>
#include <U2_3D/Src/collision/U2Aabb.h>

class U2SkinModifier;
class U2SkinController;
class U2TransformInterpController;
class U2Plane;
class U2MeshLoader;

class U2_3D U2MeshBuilder : public U2RefObject
{

public:
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



	class Vertex : public U2MemObj
	{
	public:
		enum Component
		{
			VERT			= (1 << 0),
			NORMAL			= (1 << 1),
			UV0				= (1 << 2),
			UV1				= (1 << 3),
			UV2				= (1 << 4),
			UV3				= (1 << 5),
			COLOR			= (1 << 6),
			TANGENT			= (1 << 7),
			BINORMAL		= (1 << 8),
			BONE_WEIGHTS	= (1 << 9),
			BONE_INDICES	= (1 << 10),

			MAX_VERTEX_COMPONENTS = 11,
		};

		enum VertexFlag
		{
			VF_REDUNDANT = (1<<0),
		};

		enum 
		{
			MAX_TEXTURE_LAYERS = 4,
		};

		Vertex();		

		void SetVert(const D3DXVECTOR3& v);
		/// get coord
		const D3DXVECTOR3& GetVert() const;
		/// set normal
		void SetNormal(const D3DXVECTOR3& v);
		/// get normal
		const D3DXVECTOR3& GetNormal() const;
		/// set tangent
		void SetTangent(const D3DXVECTOR3& v);
		/// get tangent
		const D3DXVECTOR3& GetTangent() const;
		/// set binormal
		void SetBinormal(const D3DXVECTOR3& v);
		/// get binormal
		const D3DXVECTOR3& GetBinormal() const;
		/// set color
		void SetColor(const D3DXVECTOR4& v);
		/// get color
		const D3DXVECTOR4& GetColor() const;
		/// set uv
		void SetUv(int layer, const D3DXVECTOR2& v);
		/// get uv
		const D3DXVECTOR2& GetUv(int layer) const;
		/// set joint weights
		void SetWeights(const D3DXVECTOR4& w);
		/// get joint weights
		const D3DXVECTOR4& GetWeights() const;
		/// set joint indices
		void SetBoneIndices(const D3DXVECTOR4& w);
		/// get joint indices
		const D3DXVECTOR4& GetBoneIndices() const;
		/// check if component exists
		bool HasComponent(Component c) const;

		// 특정 정점 요소만 메쉬로 내보낼 때 사용
		void SetComponent(Component c);

		/// initialize a component with zero
		void ZeroComponent(Component c);
		/// delete a component
		void DelComponent(Component c);
		/// get component mask
		int GetComponentMask() const;
		/// compare method, return -1, 0 or +1
		int Compare(const Vertex& rhs) const;
		/// equality operator
		bool operator==(const Vertex& rhs) const;
		/// set flag
		void SetFlag(VertexFlag f);
		/// clear flag
		void UnsetFlag(VertexFlag f);
		/// check flag
		bool CheckFlag(VertexFlag f) const;
		/// get vertex width (number of valid floats in vertex)
		int GetVertexStride() const;
		/// transform the vertex in place
		void Transform(const D3DXMATRIX& m44, const D3DXMATRIX& m33);
		/// fill vertex data with interpolated result (ignores weights and joint indices!)
		void Interpolate(const Vertex& v0, const Vertex& v1, float lerp);
		/// component-wise copy from source vertex
		void CopyComponentFromVertex(const Vertex& src, int compMask);
		/// copy component from component, only if source vertex component is valid
		void CopyComponentFromComponent(Component from, Component to);


		D3DXVECTOR3 m_vert;
		D3DXVECTOR3 m_normal;
		D3DXVECTOR3 m_tangent;
		D3DXVECTOR3 m_bitangent;
		D3DXVECTOR3 m_binormal;
		D3DXVECTOR4 m_color;
		D3DXVECTOR2 m_uv[MAX_TEXTURE_LAYERS];
		D3DXVECTOR4 m_boneWeights;
		D3DXVECTOR4 m_boneIndices;

		uint16 m_usCompMask;
		uint16 m_usFlags;
	};

	class Triangle : public U2MemObj
	{
	public:
		
		enum Component 
		{
			VERTEX_INDICES		= (1 << 0),
			GROUP_ID			= (1 << 1),
			MATERIAL_ID			= (1 << 2),
			NORMAL				= (1 << 3),
			TANGENT				= (1 << 4),
			BINORMAL			= (1 << 5)
		};

		Triangle();
		~Triangle();

		/// set vertex indices
		void SetVertexIndices(int i0, int i1, int i2);
		/// get vertex indices
		void GetVertexIndices(int& i0, int& i1, int& i2) const;
		/// set group id
		void SetGroupId(int i);
		/// get group id
		int GetGroupId() const;
		/// set usage flags (== combination of nMesh2::Usage flags)
		void SetUsageFlags(int f);
		/// get usage flags
		int GetUsageFlags() const;
		/// set material id
		void SetMaterialId(int i);
		/// get material id
		int GetMaterialId() const;
		/// check if component is valid
		bool HasComponent(Component c) const;
		/// set triangle normal
		void SetNormal(const D3DXVECTOR3& v);
		/// get triangle normal
		const D3DXVECTOR3& GetNormal() const;
		/// set triangle tangent
		void SetTangent(const D3DXVECTOR3& v);
		/// get triangle tangent
		const D3DXVECTOR3& GetTangent() const;
		/// set triangle binormal
		void SetBinormal(const D3DXVECTOR3& v);
		/// get triangle binormal
		const D3DXVECTOR3& GetBinormal() const;
		/// returns whether the vertexIndices equal those of @c other
		bool Equals(Triangle &other) const;		

		int m_iVertIdx[3];
		int m_iUsageFlags;
		int m_iGroupId;
		int m_iMaterialId;
		D3DXVECTOR3 m_normal;
		D3DXVECTOR3 m_tangent;
		D3DXVECTOR3 m_binormal;

		uint16 m_usCompMask;
	};

	//--- a triangle group ---
	class Group : public U2MemObj
	{
	public:
		/// default constructor
		Group();
		/// set group id
		void SetId(int i);
		/// get group id
		int GetId() const;
		/// set material id
		void SetMaterialId(int i);
		/// get material id
		int GetMaterialId() const;
		/// set first triangle index (optional)
		void SetFirstTriangle(int i);
		/// get first triangle index
		int GetFirstTriangle() const;
		/// set number of triangles
		void SetNumTriangles(int i);
		/// get number of triangles
		int GetNumTriangles() const;
		/// set usage flags (see nMesh2::Usage)
		void SetUsageFlags(int f);
		/// get usage flags
		int GetUsageFlags() const;

		void SetSkinModifier(U2SkinModifier* pSkinMod);
		U2SkinModifier* GetSkinModifier() const;
	private:
		int m_iId;
		int m_iUsageFlags;
		int m_iMaterialId;
		int m_iFirstTriangle;
		int m_iNumTriangles;

		U2SkinModifier* m_pSkinMod;
	};

	struct GroupedEdge : public U2MemObj
	{
		uint16 m_usVIdx[2];
		uint16 m_usFIdx[2];
		int m_iGroupId;
	};

	U2MeshBuilder();
	~U2MeshBuilder();

	bool Load(const TCHAR* szFilename);

	bool LoadFile(U2MeshLoader* pMeshLoader, const TCHAR* szFilename);

	bool Save(const TCHAR* szFilename);

	bool LoadXMesh(const TCHAR* szFilename);

	bool SaveXMesh(const TCHAR* szFilename);

	/// check if vertex component exists
	bool HasVertexComponent(Vertex::Component c) const;

	void SetComponent(Vertex::Component c);
	
	/// delete a vertex component
	void DelVertexComponent(Vertex::Component c);
	/// add a vertex
	void AddVertex(const Vertex& v);
	/// add a triangle
	void AddTriangle(const Triangle& t);
	/// get number of triangles
	int GetNumTriangles() const;
	/// get triangle at index
	Triangle& GetTriangleAt(int index) const;
	/// get num vertices
	int GetNumVertices() const;
	/// get vertex at index
	Vertex& GetVertexAt(int index) const;
	/// get number of edges
	int GetNumEdges() const;
	/// get edge at index
	GroupedEdge& GetEdgeAt(int index) const;
	/// sort triangles by group id and material id
	void SortTriangles();
	/// find the first triangle matching group id, material id and usage flags
	int GetFirstGroupTriangle(int groupId, int materialId, int usageFlags) const;
	/// count number of triangles matching group and material id starting at index
	int GetNumGroupTriangles(int groupId, int materialId, int usageFlags, int startTriangleIndex) const;
	/// get the minimum vertex index referenced by a group
	bool GetGroupVertexRange(int groupId, int& minVertexIndex, int& maxVertexIndex) const;
	/// get the minimum edge index referenced by a group
	bool GetGroupEdgeRange(int groupId, int& minEdgeIndex, int& maxEdgeIndex) const;
	/// build a group mapping array
	void BuildGroupMap(U2PrimitiveVec<Group*>& groupMap);
	/// update triangle group ids from a group map
	void UpdateTriangleIds(const U2PrimitiveVec<Group*>& groupMap);
	/// copy triangle with its vertices, do not generate redundant vertices
	void CopyTriangle(const U2MeshBuilder& srcMesh, int triIndex, U2PrimitiveVec<int>& indexMap);
	/// copy group from source mesh group
	//void CopyGroup(U2MeshBuilder& srcMesh, int groupId);
	/// change group id
	//void ChangeGroupId(int srcGroupId, int dstGroupId);
	/// @}

	/// @name mesh operations
	/// @{

	/// clear buffers
	void Clear();
	/// erase/duplicate vertex components
	void ForceVertexComponents(int compMask);
	/// copy vertex components to another
	void CopyVertexComponents(Vertex::Component from, Vertex::Component to);
	/// extend all vertices to have the same vertex components
	void ExtendVertexComponents();
	/// transform vertices
	void Transform(const D3DXMATRIX& m);
	
	/// remove redundant vertices
	void Cleanup(U2PrimitiveVec< U2PrimitiveVec<int>* >* collapseMap);

	/// GameBryo 2.6 Method
	//bool RemoveDegenerateTriangles();
	//
	//// Checks if a given triangle is degenerate.  Returns false if triangle
	//// *IS* degenerate, thus the name.	
	//bool NonDegenerateTriangle(unsigned int uI0, unsigned int uI1, unsigned int uI2, 
	//	const D3DXVECTOR3& p0, const D3DXVECTOR3& p1, const D3DXVECTOR3 p2);

	//void RemoveDegenerateVertices(unsigned int* pUNewToOld = NULL);
	


	/// make triangle-vertices unique, creating redundant vertices, opposite of Cleanup()
	void Inflate();
	/// build edge information (only works on clean meshes, and it not allowed to change the mesh later)
	void CreateEdges();
	/// optimize for t&l hardware vertex cache
	void Optimize();
	/// append mesh from mesh builder object
	int Append(const U2MeshBuilder& source);
	/// copy from mesh builder object
	void Copy(const U2MeshBuilder& source);
	/// compute the bounding box of the mesh, filtered by a group id
	U2Aabb GetGroupBBox(int groupId) const;
	/// compute the complete bounding box of the mesh
	U2Aabb GetBBox() const;
	/// count vertices in bounding box
	//int CountVerticesInBBox(const bbox3& box) const;
	/// split triangle group in place, using a clip plane, return 2 new group indices
	void Split(const U2Plane& clipPlane, int groupId, int posGroupId, int negGroupId, int& numPosTriangles, int& numNegTriangles);
	/// fill a vertex-triangle mapping array
	void BuildVertexTriangleMap(U2PrimitiveVec< U2PrimitiveVec<int>* >& vertexTriangleMap) const;
	/// create face normals and tangents (requires a valid uv-mapping at layer 0)
	void BuildTriangleNormals();
	/// generate averaged vertex tangents
	void BuildVertexTangents(bool allowVertexSplits);
	/// fix existing tangent/binormal directions (if generated by Maya)
	void FixVertexTangentDirections();
	/// generate averaged vertex normals
	void BuildVertexNormals();
	/// flip v texture coordinates
	void FlipUvs();
	/// checks the mesh for geometry errors
	//nArray<nString> CheckForGeometryError();
	/// checks the mesh for duplicated face
	//nArray<nString> CheckForDuplicatedFaces();
	/// @}
	bool ComputeTangent(	const D3DXVECTOR3& pos0, const D3DXVECTOR2& tcoord0,
		const D3DXVECTOR3& pos1, const D3DXVECTOR2& tcoord1,
		const D3DXVECTOR3& pos2, const D3DXVECTOR2& tcoord2,
		D3DXVECTOR3& tangent);	


private:
		struct TempEdge : public U2MemObj
		{
			uint16 m_usVIdx[2];
			uint16 m_usFIdx;
			uint16 m_usGroupId;
		};

		/// static user data pointer for qsort hook
		static U2MeshBuilder* ms_pQsortData;
		/// a qsort() hook for generating a sorted index array
		static int __cdecl VertexSorter(const void* elm0, const void* elm1);
		/// qsort hook for sorting triangles by their group index
		static int __cdecl TriangleGroupSorter(const void* elm0, const void* elm1);
		/// qsort hook for sorting temp edges by vertex indices
		static int __cdecl TempEdgeSorter(const void* elm0, const void* elm1);
		/// qsort hook for sorting temp edges by groupID and face indices
		static int __cdecl GroupedEdgeSorter(const void* elm0, const void* elm1);
		/// do an inflated component copy using a source mesh and a collapseMap
		void InflateCopyComponents(const U2MeshBuilder& src, 
			const U2PrimitiveVec< U2PrimitiveVec<int>* >& collapseMap, int compMask);
		/// searches for duplicated faces
		void SearchDuplicatedFaces(U2PrimitiveVec<int>& result);
		/// generate averaged vertex tangents using vertex splitting
		void BuildVertexTangentsWithSplits();
		/// generate averaged vertex tangents without vertex splitting
		void BuildVertexTangentsWithoutSplits();

public:
	U2PrimitiveVec<Vertex*> m_vertexArray;
	U2PrimitiveVec<Triangle*> m_triangleArray;
	U2PrimitiveVec<GroupedEdge*> m_groupEdgeArray;

	U2THashTable<int, U2SkinController*> m_skinCtrls;
	U2THashTable<int, U2TransformInterpController*> m_tmInterpCtrls;

	U2PrimitiveVec<Group*> groupMap;
};

#include "U2MeshBuilder.inl"

typedef U2SmartPtr<U2MeshBuilder> U2MeshBuilderPtr;

#endif