/************************************************************************
module	:	U2MeshLoader
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_MESHLOADER_H
#define U2_MESHLOADER_H

#include <U2_3D/src/U2_3DLibType.h>
//#include <U2Lib/src/U2RefObject.h>
//#include <U2Lib/src/U2SmartPtr.h>
#include <U2Lib/Src/U2TVec.h>
#include <U2Lib/Src/U2TString.h>
#include <U2_3D/src/Dx9/nMeshGroup.h>
#include <U2_3D/src/collision/U2Aabb.h>

U2SmartPointer(U2SkinModifier);
U2SmartPointer(U2Controller);
class U2WinFile;

class  U2_3D U2MeshLoader : public U2RefObject
{
public:
	/// index types
	enum IndexType
	{
		Index16,
		Index32,
	};

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


	U2MeshLoader();
	virtual ~U2MeshLoader();

	//void SetFilename(const U2DynString& filename);
	void SetFilename(const TCHAR* filename);
	//const U2DynString& GetFilename() const;
	const char* GetFilename() const;

	void SetIndexType(IndexType t);
	/// get index type
	IndexType GetIndexType() const;

	void SetValidVertexComponents(unsigned int components);
	unsigned int GetValidVertexComponents();

	virtual bool Open();

	virtual void Close();

	unsigned int GetNumGroups() const;
	const nMeshGroup& GetMeshGroup(uint32 idx) const;

	unsigned int GetNumVertices() const;
	unsigned int GetVertexChannelCnt() const;

	unsigned int GetNumTriangles() const;

	unsigned int GetNumIndices() const;

	unsigned int GetNumEdges() const;

	unsigned int GetVertexComponents() const;

	virtual bool ReadVertices(void* pvBuffer, int buffSize);
	virtual bool ReadIndices(void* pvBuffer, int buffSize);
	virtual bool ReadEdges(void* pvBuffer, int buffSize);

	static unsigned int GetVertexChannelCntFromMask(int compMask);

	const U2Aabb& GetBoundingBox() const;

protected:
	U2DynString m_szFilename;
	IndexType m_eIndexType;
	U2WinFile* m_pFile;

	unsigned int m_uNumGroups;	
	unsigned int m_uNumVertices;
	unsigned int m_uVertexStride;
	unsigned int m_uFileVertexStride;
	unsigned int m_uNumTriangles;
	unsigned int m_uNumIndices;
	unsigned int m_uNumEdges;
	unsigned int m_uVertexComponents;
	unsigned int m_uFileVertexComponents;
	unsigned int m_uValidVertexComponents;
	U2ObjVec<U2MeshGroupPtr> m_meshGroups;
	U2ObjVec<U2SkinModifierPtr> m_skinMods;
	U2PrimitiveVec<U2Controller*> m_controllers;

	U2Aabb m_bbox;
	
};

#include "U2MeshLoader.inl"

#endif
