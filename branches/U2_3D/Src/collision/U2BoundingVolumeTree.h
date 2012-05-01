/************************************************************************
module	:	U2BoundingVolumeTree
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_BOUNDINGVOLUMETREE_H
#define	U2_BOUNDINGVOLUMETREE_H

#include <U2_3D/Src/U2_3DLibType.h>
#include <U2_3D/Src/Main/U2BoundingVolume.h>
#include <U2_3D/Src/Main/U2Line3D.h>
#include <U2_3D/Src/dx9/U2TriList.h>

class U2_3D U2BoundingVolumeTree : public U2RefObject
{
public:
	virtual ~U2BoundingVolumeTree();

	U2BoundingVolumeTree* GetLChild();
	U2BoundingVolumeTree* GetRChild();
	bool IsInteriorNode() const;
	bool IsLeafNode() const;

	const U2TriList* GetMesh() const;
	const U2BoundingVolume* GetWorldBound() const;
	unsigned short GetTriangleCnt() const;
	unsigned short GetTriangle(unsigned short i) const;
	const unsigned short* GetTriangles()const;

	void UpdateWorldBound();

protected:
	U2BoundingVolumeTree(const U2TriList* pMesh);
	U2BoundingVolumeTree(int eBVType, const U2TriList* pMesh, 
		unsigned short usMaxTrisPerLeaf= 1,bool bStoreInteriorTris = false);

	void BuildTree(int eBVType, unsigned short usMaxTrisPerLeaf, bool bStoreInteriorTris,
		const D3DXVECTOR3* pCentroid, uint16 i0, uint16 i1, uint16* piOSplit, uint16 *piISplit);
	static void SplitTriangles(const D3DXVECTOR3* pCentroid, uint16 i0,uint16 i1, 
		uint16* piISplit, uint16& rj0,uint16& rj1, uint16* piOSplit, const U2Line3D& line);

	// for quick-sort of centroid(center of mass) projections on axes
	class U2_3D ProjectionInfo : public U2MemObj
	{
	public:
		uint16 m_usTriangle;
		float m_fProjection;
	};

	static int Compare(const void*pvElement0, const void* pvElement);

	// model bounding volume factory
	typedef U2BoundingVolume* (*CreatorModel)(const U2TriList*, uint16,uint16,uint16*, 
		U2Line3D&);
	static CreatorModel ms_afnCreateModelBound[U2BoundingVolume::BV_QUANTITY];

	// world bounding volume factory
	typedef U2BoundingVolume* (*CreatorWorld)(void);
	static CreatorWorld ms_afnCreateWorldBound[U2BoundingVolume::BV_QUANTITY];

	const U2TriList* m_pMesh;
	U2BoundingVolumePtr m_spModelBound;
	U2BoundingVolumePtr m_spWorldBound;

	// binary tree representation
	U2BoundingVolumeTree* m_pLChild;
	U2BoundingVolumeTree* m_pRChild;

	// If bStoreInteriorTris is set to 'false' in the constructor, the
	// interior nodes set the triangle quantity to zero and the array to null.
	// Leaf nodes set the quantity to the number of triangles at that node (1
	// if iMaxTrianglesPerLeaf was set to 1) and allocate an array of
	// triangle indices that are relative to the input mesh of the top level
	// constructor.
	//
	// If bStoreInteriorTris is set to 'true', the interior nodes also save
	// the triangle quantity and array of triangle indices for the mesh that
	// the node represents.
	uint16 m_usTrisCnt;
	uint16* m_pusTriangle;
};

#include "U2BoundingVolumeTree.inl"

#endif