/************************************************************************
module	:	U2TriangleData
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_TRIANGLEDATA_H
#define	U2_TRIANGLEDATA_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>
#include "U2MeshData.h"

class U2_3D U2TriangleData : public U2MeshData
{
	DECLARE_RTTI;
public:		
	virtual ~U2TriangleData();
	unsigned short GetTriangleCnt() const;

	
	virtual void SetActiveTriangleCount(unsigned short usActive);
	unsigned short GetActiveTriangleCount() const;

	virtual void GetTriangleIndices(unsigned short i, unsigned short& i0,
		unsigned short& i1, unsigned short& i2) const;

	void CreateBinormals(bool bInit);
	void CreateTangents(bool bInit);

	D3DXVECTOR3* GetBinormals() { return m_pBinormals; }
	D3DXVECTOR3* GetTangents() { return m_pTangents; }

	void GenerateNormals();
	void GenerateTangents(int iTCoordUnit);

	D3DXVECTOR3 ComputeTangent(
		const D3DXVECTOR3& p0, const D3DXVECTOR2& tcoord0,
		const D3DXVECTOR3& p1, const D3DXVECTOR2& tcoord1,
		const D3DXVECTOR3& p2, const D3DXVECTOR2& tcoord2 );

protected:
	U2TriangleData(unsigned int uiNumVerts, D3DXVECTOR3* pVert, 
		D3DXVECTOR3 *pNormal, D3DXCOLOR* pColor, D3DXVECTOR2* pTexCoords,
		unsigned short numTexCoordSets, unsigned short numTriangles);
	U2TriangleData();

	virtual void SetTriangleCount(unsigned short usTriangles);

	void UpdateModelTangentSpace(int iTangentSpaceType);	
	virtual void UpdateModelNormals();
	 

private:
	void UpdateTangentSpaceUseGeometry();
	void UpdateTangetSpaceUseTCoord(int iTCoordUnit);
	 
	
	friend class U2Triangle;
	
	D3DXVECTOR3* m_pBinormals;
	D3DXVECTOR3* m_pTangents;

	
};

typedef U2SmartPtr<U2TriangleData> U2TriangleDataPtr;

#endif 