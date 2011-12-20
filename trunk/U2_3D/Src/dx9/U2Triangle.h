/************************************************************************
module	:	U2Triangle
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_TRIANGLE_H
#define	U2_TRIANGLE_H

#include "U2Mesh.h"

class U2TriangleData;

class U2_3D U2Triangle : public U2Mesh 
{
	DECLARE_RTTI;

public:
	virtual ~U2Triangle();

	virtual void SetModelData(U2TriangleData* pMeshData);
	

	virtual void GetTriangle(uint16 usTriangle, D3DXVECTOR3*& pt0, 
		D3DXVECTOR3*& pt1, D3DXVECTOR3*& pt2) = 0;
	void SetActiveTriangleCount(unsigned short usActive);
	unsigned short GetActiveTriangleCount() const;


	void GetTriangleIndices(unsigned short i, unsigned short& i0,
		unsigned short& i1, unsigned short& i2) const;

protected:
	U2Triangle(U2TriangleData* pModelData);	
	U2Triangle();

	virtual void UpdateModelNormals();
	virtual void UpdateModelTangentSpace(int iTangentSpaceType);
	
private:


};

typedef U2SmartPtr<U2Triangle> U2TrianglePtr;

#endif