/************************************************************************
module	:	U2IntectLineBox3D
Author	:	Yun sangyong
Desc	:	GT, Section 11.12.2: Linear Components and Axis-Aligned
Bounding Box
GT, Section 11.12.3: Linear Components and Oriented Bounding
Box
************************************************************************/
#pragma once
#ifndef U2_INTECTLINESPHERE3D_H
#define U2_INTECTLINESPHERE3D_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2_3D/src/Main/U2Line3D.h>
#include <U2_3D/src/collision/U2Sphere.h>

#include "U2Intersector.h"

class U2_3D U2IntectLineSphere3D : public U2Intersector
{
public:
	U2IntectLineSphere3D(const U2Line3D& line, const U2Sphere& sphere);

	inline const U2Line3D& GetLine() const { return *m_pLine; }
	inline const U2Sphere& GetSphere() const { return *m_pSphere; }

	virtual bool Test();
	virtual bool Find();

	uint32 GetCnt() const;
	const D3DXVECTOR3& GetPoint(int i) const;

	float GetLineT(int i) const;

private:

	const U2Line3D* m_pLine;
	const U2Sphere* m_pSphere;

	int m_iCnt;
	D3DXVECTOR3 m_aPoint[2];
	float m_afLineT[2];
};



#endif