/************************************************************************
module	:	U2StandardMesh
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_STANDARDMESH_H
#define	U2_STANDARDMESH_H

#include "U2Mesh.h"

class U2_3D U2StandardMesh : public U2Triangle
{

public:
	U2StandardMesh(U2VertexAttributes* attr, bool bInside = false, 
		const U2Transform* pXform = 0);
	~U2StandardMesh();


	virtual void GetTriangle(uint16 usTriangle, D3DXVECTOR3*& pt0, 
		D3DXVECTOR3*& pt1, D3DXVECTOR3*& pt2) {}

	U2Mesh* Retangle(int iXSamples, int iYSamples, float fXExtent, 
		float fYExtent);

	U2TriList* Box(float fXExtent, float fYExtent, float fZExtent);
	
	U2TriList* Torus(int iCircleSmaples, int iRadialSmaples, float fOuterRadius, 
		float fInnerRadius);

	U2TriList* Sphere(int iZSamples, int iRadialSamples, float fRadius);


private:
	void ReverseTriangleOrder(uint16 uTris, uint16* pusIndices);
	void TransformData (U2TriangleData& data);


	U2VertexAttributes* m_pAttr;
	bool m_bInside;
	U2Transform m_xForm;
};

typedef U2SmartPtr<U2StandardMesh> U2StandardMeshPtr;


#endif