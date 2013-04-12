#pragma once 


#include "Vector.h"
// clamp variables of any type
//template<typename Type>
//inline Type Clamp(const Type &A, const Type &Min, const Type &Max)
//{
//	if (A < Min) return Min;
//	if (A > Max) return Max;
//	return A;
//}

// normalize vector (returns length)
inline noVec3 Normalize(const noVec3 &vOther)
{
	float fLength = vOther.Length();
	if (fLength == 0.0f) return vOther;
	float fInvLength = 1.0f / fLength;
	return vOther * fInvLength;
}

// calculate cross product
inline noVec3 Cross(const noVec3 &vA, const noVec3 &vB)
{
	return noVec3( (vA.y * vB.z) - (vA.z * vB.y),
		(vA.z * vB.x) - (vA.x * vB.z),
		(vA.x * vB.y) - (vA.y * vB.x) );
}

// calculate dot product
inline float Dot(const noVec3 &vA, const noVec3 &vB)
{
	return (vA.x * vB.x) + (vA.y * vB.y) + (vA.z * vB.z);
}

// normalize vector (returns length)
inline noVec4 Normalize(const noVec4 &vOther)
{
	float fLength = vOther.Length();
	if (fLength == 0.0f) return vOther;
	float fInvLength = 1.0f / fLength;
	return vOther * fInvLength;
}

// calculate dot product
inline float Dot(const noVec4 &vA, const noVec4 &vB)
{
	return (vA.x * vB.x) + (vA.y * vB.y) + (vA.z * vB.z) + (vA.w * vB.w);
}

typedef struct Matrix
{
	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float m[4][4];		
	};

	inline Matrix() {}
	inline Matrix( float f11, float f12, float f13, float f14,
		float f21, float f22, float f23, float f24,
		float f31, float f32, float f33, float f34,
		float f41, float f42, float f43, float f44 ) :
	_11(f11), _12(f12), _13(f13), _14(f14),
		_21(f21), _22(f22), _23(f23), _24(f24),
		_31(f31), _32(f32), _33(f33), _34(f34),
		_41(f41), _42(f42), _43(f43), _44(f44) {}

	inline Matrix( float m[16] ) 
	{
		Set(m);
	}

	inline void Set(float m[16])
	{
		_11 = m[0];	_12 = m[1];	_13 = m[2];	_14 = m[3];
		_21 = m[4];	_22 = m[5];	_23 = m[6];	_24 = m[7];
		_31 = m[8];	_32 = m[9];	_33 = m[10];	_34 = m[11];
		_41 = m[12];	_42 = m[13];	_43 = m[14];	_44 = m[15];
	}

	// set matrix to identity
	inline void SetIdentity(void) { (*this) = Matrix(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1); }

	// set rotation from given euler angles (clears _11 to _33)
	inline void SetRotation(const noVec3 &vEuler);

	// set translation elements of matrix
	inline void SetTranslation(const noVec3 &vPos);

	// scale matrix by given multipliers
	inline void Scale(const noVec3 &vMultiplier);

	// multiply two matrices together
	inline Matrix operator * (const Matrix &B) const;

} Matrix;

// transforms vector with matrix
inline noVec4 Transform(const noVec4 &A, const Matrix &M)
{
	return noVec4((A.x * M._11) + (A.y * M._21) + (A.z * M._31) + (A.w * M._41),
		(A.x * M._12) + (A.y * M._22) + (A.z * M._32) + (A.w * M._42),
		(A.x * M._13) + (A.y * M._23) + (A.z * M._33) + (A.w * M._43),
		(A.x * M._14) + (A.y * M._24) + (A.z * M._34) + (A.w * M._44));
}

// transforms vector with matrix
inline noVec4 Transform(const noVec3 &A, const Matrix &M)
{
	return Transform(noVec4(A.x,A.y,A.z,1), M);
}

// multiply two matrices together
inline Matrix Matrix::operator * (const Matrix &B) const
{
	const Matrix &A = (*this);
	return Matrix(
		(A._11 * B._11) + (A._12 * B._21) + (A._13 * B._31) + (A._14 * B._41),
		(A._11 * B._12) + (A._12 * B._22) + (A._13 * B._32) + (A._14 * B._42),
		(A._11 * B._13) + (A._12 * B._23) + (A._13 * B._33) + (A._14 * B._43),
		(A._11 * B._14) + (A._12 * B._24) + (A._13 * B._34) + (A._14 * B._44),

		(A._21 * B._11) + (A._22 * B._21) + (A._23 * B._31) + (A._24 * B._41),
		(A._21 * B._12) + (A._22 * B._22) + (A._23 * B._32) + (A._24 * B._42),
		(A._21 * B._13) + (A._22 * B._23) + (A._23 * B._33) + (A._24 * B._43),
		(A._21 * B._14) + (A._22 * B._24) + (A._23 * B._34) + (A._24 * B._44),

		(A._31 * B._11) + (A._32 * B._21) + (A._33 * B._31) + (A._34 * B._41),
		(A._31 * B._12) + (A._32 * B._22) + (A._33 * B._32) + (A._34 * B._42),
		(A._31 * B._13) + (A._32 * B._23) + (A._33 * B._33) + (A._34 * B._43),
		(A._31 * B._14) + (A._32 * B._24) + (A._33 * B._34) + (A._34 * B._44),

		(A._41 * B._11) + (A._42 * B._21) + (A._43 * B._31) + (A._44 * B._41),
		(A._41 * B._12) + (A._42 * B._22) + (A._43 * B._32) + (A._44 * B._42),
		(A._41 * B._13) + (A._42 * B._23) + (A._43 * B._33) + (A._44 * B._43),
		(A._41 * B._14) + (A._42 * B._24) + (A._43 * B._34) + (A._44 * B._44));
}

// set translation elements of matrix
inline void Matrix::SetTranslation(const noVec3 &vPos)
{
	_41 = vPos.x;
	_42 = vPos.y;
	_43 = vPos.z;
}

// scale matrix by given multipliers
inline void Matrix::Scale(const noVec3 &vMultiplier)
{
	_11 *= vMultiplier.x;
	_22 *= vMultiplier.y;
	_33 *= vMultiplier.z;
}

// set rotation from given euler angles (clears _11 to _33)
inline void Matrix::SetRotation(const noVec3 &vEuler)
{
	// calculate from euler angles
	float ch = cosf(vEuler.x);
	float sh = sinf(vEuler.x);
	float cp = cosf(vEuler.y);
	float sp = sinf(vEuler.y);
	float cb = cosf(vEuler.z);
	float sb = sinf(vEuler.z);

	_11 = (cb * ch) + (sp * sh * sb);
	_12 = cp * sb;
	_13 = (-cb * sh) + (sp * ch * sb);
	_21 = (-sb * ch) + (sp * sh * cb);
	_22 = cp * cb;
	_23 = (sb * sh) + (sp * ch * cb);
	_31 = sh * cp;
	_32 = -sp;
	_33 = cp * ch;
}

// make a left-handed look-at matrix
inline Matrix MatrixLookAtLH(const noVec3 &vEye, const noVec3 &vAt, const noVec3 &vUp)
{
	noVec3 vZ = Normalize(vAt - vEye);
	noVec3 vX = Normalize(Cross(vUp, vZ));
	noVec3 vY = Cross(vZ, vX);

	return Matrix( vX.x, vY.x, vZ.x, 0,
		vX.y, vY.y, vZ.y, 0,
		vX.z, vY.z, vZ.z, 0,
		-Dot(vX, vEye), -Dot(vY, vEye), -Dot(vZ, vEye),  1);
}

// make a left-handed perspective projection matrix
inline Matrix MatrixPerspectiveFovLH(float fFov, float fAspect, float fNear, float fFar)
{
	float fHeight = 1.0f / tanf(fFov * 0.5f);
	float fWidth = fHeight / fAspect;

	return Matrix(  fWidth,        0,                          0,  0,
		0,  fHeight,                          0,  0,
		0,        0,        fFar/(fFar - fNear),  1,
		0,        0, -fNear*fFar/(fFar - fNear),  0   );

}

// make a left-handed orthographic projection matrix
inline Matrix MatrixOrthoLH(float fWidth, float fHeight, float fNear, float fFar)
{
	return Matrix(  2.0f/fWidth,             0,                     0,  0,
		0,  2.0f/fHeight,                     0,  0,
		0,             0,      1/(fFar - fNear),  0,
		0,             0, -fNear/(fNear - fFar),  1  );

}

template<typename type>
inline type MatrixOrthoOffCenterLH(float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar)
{
	return type(  2.0f/(fRight - fLeft),             0,                     0,  0,
		0,  2.0f/(fTop - fBottom),                     0,  0,
		0,             0,      1/(fFar - fNear),  0,
		(fLeft + fRight)/(fLeft - fRight),  (fTop + fBottom)/(fBottom - fTop), fNear/(fNear - fFar),  1  );

}

inline const Matrix Transpose(const Matrix& src)
{
	Matrix out;

	out._11 = src._11; out._12 = src._21; out._13 = src._31; out._14 = src._41; 
	out._21 = src._12; out._22 = src._22; out._23 = src._32; out._24 = src._42; 
	out._31 = src._13; out._32 = src._23; out._33 = src._33; out._34 = src._43; 
	out._41 = src._14; out._42 = src._24; out._43 = src._34; out._44 = src._44; 

	return out;
}

inline void  PlaneFrom3Points( float plane[4], const float a[3], const float b[3], const float c[3] )
{
	noVec3 va(a[0], a[1], a[2]), vb(b[0], b[1], b[2]), vc(c[0], c[1], c[2]);
	noVec3 vab, vac;
	
	vab = vb - va;
	vac = vc - va;

	noVec3 vPlane = vab.Cross(vac);
	vPlane.Normalize();

	plane[0] = vPlane.x;
	plane[1] = vPlane.y;
	plane[2] = vPlane.z;
	plane[3] = va * vPlane;
}