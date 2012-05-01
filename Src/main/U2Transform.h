/************************************************************************
module	:	U2BoxBV
Author	:	Yun sangyong
Desc	:	Scale은 X, Y, Z축이 서로 동일하게 적용

D3DXTransformCoord의 소스 코드... 잘못사용하지 않도록.. 
In: VECTOR V, MATRIX M
Out: VECTOR V

float x,y,z,w;

x = V.x*M._11 + V.y*M._21 + V.z*M._31 + M._41;
y = V.x*M._12 + V.y*M._22 + V.z*M._32 + M._42;
z = V.x*M._13 + V.y*M._23 + V.z*M._33 + M._43;
w = V.x*M._14 + V.y*M._24 + V.z*M._34 + M._44;

V.x = x/w;
V.y = y/w;
V.z = z/w;
************************************************************************/
#ifndef U2_TRANSFORM_H
#define U2_TRANSFORM_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2Lib/src/U2Flags.h>
#include <U2Lib/src/Memory/U2MemObj.h>
#include <U2_3D/src/main/U2Plane.h>

class U2_3D U2Transform : public U2MemObj
{
public:	
	U2Transform();


	void				MakeIdentity();
	bool				operator ==(const U2Transform & xform)  const;
	U2Transform			operator*(const U2Transform &xform) const;	
	D3DXVECTOR3			operator*(const D3DXVECTOR3 &pt) const;	
	
	D3DXVECTOR3			Apply(const D3DXVECTOR3& vec3) const;
	U2Plane				Apply(const U2Plane& input) const;
	//void				Apply(int uVertCnt, const D3DXVECTOR);
	
	D3DXVECTOR3			ApplyInverse(const D3DXVECTOR3& vec3);
	/*void				ApplyInverse(int quantiy, const D3DXVECTOR3& inVec3,
						D3DXVECTOR3& outVec3 );*/

	D3DXVECTOR3			InverseVector(const D3DXVECTOR3& input);

	
	void				Inverse(U2Transform& out) const;						

	void Product (const U2Transform& rkA, const U2Transform& rkB);


	enum 
	{
		TM_IDENTITY = 0x2000,
		TM_ROTSCALE = 0x4000, 
		UNIFORM_SCALE = 0x8000,
		TRANSFORM_MASK = 0xffff
	};


	static D3DXMATRIX ZERO;
	static D3DXMATRIX IDENTITY;

	void				SetRot(const D3DXMATRIX& mat);
	void				SetTrans(const D3DXVECTOR3& trans);

	void				SetScale(const D3DXVECTOR3 vScale );
	void				SetUniformScale(float scale);


	bool U2Transform::IsIdentity() const;
	bool U2Transform::IsRSMatrix () const;
	bool U2Transform::IsUniformScale () const;



	const D3DXMATRIX&			GetRot() const;
	const D3DXVECTOR3&			GetTrans() const;
	const D3DXVECTOR3&			GetScale() const;

	float				GetNorm() const;

	void				GetHomogeneousMat(D3DXMATRIX& hMat) const;

		
private:
	D3DXMATRIX m_matRot;	
	D3DXVECTOR3 m_trans;
	D3DXVECTOR3 m_vScale;	

	U2DeclareFlags(unsigned short);
};



#endif