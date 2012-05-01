/************************************************************************
module	:	U2Aabb
Author	:	Yun sangyong
Date	:	2010/06/11
Desc	:	
************************************************************************/
#pragma once 

struct U2_3D U2Cone : public U2MemObj
{
	D3DXVECTOR3 direction;
	D3DXVECTOR3 apex;
	float       fovy;
	float       fovx;
	float       fNear;
	float       fFar;
	D3DXMATRIX  m_LookAt;

	U2Cone(): direction(0.f, 0.f, 1.f), apex(0.f, 0.f, 0.f), fovx(0.f), fovy(0.f), fNear(0.001f), fFar(1.f) { }
	U2Cone(const std::vector<U2Aabb>* boxes, const D3DXMATRIX* projection, const D3DXVECTOR3* _apex);
	U2Cone(const std::vector<U2Aabb>* boxes, const D3DXMATRIX* projection, const D3DXVECTOR3* _apex, const D3DXVECTOR3* _direction);	

};
