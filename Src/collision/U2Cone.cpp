#include <U2_3D/src/U23DLibPCH.h>
#include "U2Cone.h"

U2Cone::U2Cone(const std::vector<U2Aabb>* boxes, const D3DXMATRIX* projection, const D3DXVECTOR3* _apex) 
: apex(*_apex)
{
	const D3DXVECTOR3 yAxis(0.f, 1.f, 0.f);
	const D3DXVECTOR3 zAxis(0.f, 0.f, 1.f);
	const D3DXVECTOR3 negZAxis(0.f, 0.f, -1.f);
	switch (boxes->size())
	{
	case 0: 
		{
			direction = negZAxis;
			fovx = 0.f;
			fovy = 0.f;
			D3DXMatrixIdentity(&m_LookAt);
			break;
		}
	default:
		{
			unsigned int i, j;


			//  compute a tight bounding sphere for the vertices of the bounding boxes.
			//  the vector from the apex to the center of the sphere is the optimized view direction
			//  start by xforming all points to post-projective space
			std::vector<D3DXVECTOR3> ppPts;
			ppPts.reserve(boxes->size() * 8);

			for (i=0; i<boxes->size(); i++) 
			{
				for (j=0; j<8; j++)
				{
					D3DXVECTOR3 tmp = (*boxes)[i].CornerPoint(j);
					D3DXVec3TransformCoord(&tmp, &tmp, projection);

					ppPts.push_back(tmp);
				}
			}

			//  get minimum bounding sphere
			U2Sphere bSphere( &ppPts );

			float min_cosTheta = 1.f;

			direction = bSphere.m_center - apex;
			D3DXVec3Normalize(&direction, &direction);

			D3DXVECTOR3 axis = yAxis;

			if ( fabsf(D3DXVec3Dot(&yAxis, &direction)) > 0.99f )
				axis = zAxis;

			D3DXMatrixLookAtLH(&m_LookAt, &apex, &(apex+direction), &axis);

			fNear = 1e32f;
			fFar = 0.f;

			float maxx=0.f, maxy=0.f;
			for (i=0; i<ppPts.size(); i++)
			{
				D3DXVECTOR3 tmp;
				D3DXVec3TransformCoord(&tmp, &ppPts[i], &m_LookAt);
				maxx = max(maxx, fabsf(tmp.x / tmp.z));
				maxy = max(maxy, fabsf(tmp.y / tmp.z));
				fNear = min(fNear, tmp.z);
				fFar  = max(fFar, tmp.z);
			}

			fovx = atanf(maxx);
			fovy = atanf(maxy);
			break;
		}
	} // switch

}


U2Cone::U2Cone(const std::vector<U2Aabb>* boxes, const D3DXMATRIX* projection, const D3DXVECTOR3* _apex, 
			   const D3DXVECTOR3* _direction)
			   : apex(*_apex), direction(*_direction)
{
	const D3DXVECTOR3 yAxis(0.f, 1.f, 0.f);
	const D3DXVECTOR3 zAxis(0.f, 0.f, 1.f);
	D3DXVec3Normalize(&direction, &direction);

	D3DXVECTOR3 axis = yAxis;

	if ( fabsf(D3DXVec3Dot(&yAxis, &direction))>0.99f )
		axis = zAxis;


	D3DXMatrixLookAtLH(&m_LookAt, &apex, &(apex+direction), &axis);

	float maxx = 0.f, maxy = 0.f;
	fNear = 1e32f;
	fFar =  0.f;

	D3DXMATRIX concatMatrix;
	D3DXMatrixMultiply(&concatMatrix, projection, &m_LookAt);

	for (unsigned int i=0; i<boxes->size(); i++)
	{
		const U2Aabb& bbox = (*boxes)[i];
		for (int j=0; j<8; j++)
		{
			D3DXVECTOR3 vec = bbox.CornerPoint(j);
			D3DXVec3TransformCoord(&vec, &vec, &concatMatrix);
			maxx = max(maxx, fabsf(vec.x / vec.z));
			maxy = max(maxy, fabsf(vec.y / vec.z));
			fNear = min(fNear, vec.z);
			fFar  = max(fFar, vec.z);
		}
	}
	fovx = atanf(maxx);
	fovy = atanf(maxy);

}
