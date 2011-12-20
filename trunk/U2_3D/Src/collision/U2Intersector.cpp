#include <U2_3D/src/U23DLibPCH.h>
#include "U2Intersector.h"


U2Intersector::U2Intersector()
{

}

U2Intersector::~U2Intersector()
{
	m_fContactTime = 0.f;
	m_eIntersectionType = IT_EMPTY;
}

bool U2Intersector::Test()
{
	U2ASSERT(false);
	return false;}

bool U2Intersector::Find()
{
	U2ASSERT(false);
	return false;
}

bool U2Intersector::Test(float fMax, const D3DXVECTOR3& velocity0, 
				  const D3DXVECTOR3& velocity1)
{
	U2ASSERT(false);
	return false;
}


float U2Intersector::GetContactTime() const
{
	return m_fContactTime;
}

int U2Intersector::GetIntersectionType () const
{
	return m_eIntersectionType;
}
