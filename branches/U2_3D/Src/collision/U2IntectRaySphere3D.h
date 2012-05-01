/************************************************************************
module	:	U2IntectRaySphere3D
Author	:	Yun sangyong
Desc	:	GT, Section 7.7: The Method of Separating Axes			
************************************************************************/
#pragma once
#ifndef U2_INTECTRAYSPHERE3D_H
#define U2_INTECTRAYSPHERE3D_H

#include <U2_3D/src/U2_3DLibType.h>
#include "U2Intersector.h"
#include <U2_3D/src/Main/U2Ray3D.h>
#include "U2Sphere.h"

class U2_3D U2IntectRaySphere3D : public U2Intersector
{
public:
	U2IntectRaySphere3D(const U2Ray3D& ray, const U2Sphere& sphere);

	inline const U2Ray3D& GetRay() const { return *m_pRay; }
	inline const U2Sphere& GetSphere() const { return *m_pSphere; }

	virtual bool Test ();

	virtual bool Find ();
	inline int GetCnt() const { return m_iCnt; }
	inline const D3DXVECTOR3& GetPoint(int i) const 
	{
		U2ASSERT(0 <= i && i < m_iCnt);
		return m_aPoint[i];
	}
	inline float GetRayT(int i) const
	{
		U2ASSERT(0 <= i && i < m_iCnt);
		return m_afRayT[i];
	}

private:

	const U2Ray3D* m_pRay;
	const U2Sphere* m_pSphere;

	int m_iCnt;
	D3DXVECTOR3 m_aPoint[2];
	float m_afRayT[2];

};



#endif