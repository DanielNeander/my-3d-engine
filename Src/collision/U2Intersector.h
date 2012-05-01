/************************************************************************
module	:	U2IntectLineBox3D
Author	:	Yun sangyong
Desc	:	GT, Chapter 11: Intersection in 3D
************************************************************************/
#ifndef U2_INTERSECTOR_H
#define U2_INTERSECTOR_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2lib/src/U2RefObject.h>
#include <U2lib/src/U2SmartPtr.h>

#include "U2LineComp.h"

class U2_3D U2Intersector : public U2RefObject 
{
public:
	virtual ~U2Intersector();

	virtual bool Test();
	virtual bool Find();


	virtual bool Test(float fMax, const D3DXVECTOR3& velocity0, 
		const D3DXVECTOR3& velocity1);
	
	float GetContactTime() const;
	enum 
	{
		IT_EMPTY = U2LineComp::CT_EMPTY,
		IT_POINT = U2LineComp::CT_POINT,
		IT_SEGMENT = U2LineComp::CT_SEGMENT,
		IT_RAY = U2LineComp::CT_RAY,
		IT_LINE = U2LineComp::CT_LINE,
		IT_POLYGON,
		IT_PLANE,
		IT_POLYHEDRON,
		IT_OTHER
	};

	int GetIntersectionType () const;

protected:
	U2Intersector();

	float m_fContactTime;
	int m_eIntersectionType;

};

typedef U2SmartPtr<U2Intersector> U2IntersectorPtr;

#endif 