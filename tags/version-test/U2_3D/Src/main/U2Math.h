/**************************************************************************************************
module	:	U2Math
Author	:	Yun sangyong
Desc	:
***************************************************************************************************/
#pragma once

#ifdef VECTOR3_ZERO
#undef VECTOR3_ZERO
#endif

#ifdef UNIT_X
#undef UNIT_X
#endif 

#ifdef UNIT_Y
#undef UNIT_Y
#endif 

#ifdef UNIT_Z
#undef UNIT_Z
#endif 

#define VECTOR2_ZERO	D3DXVECTOR2(0.f, 0.f)
#define VECTOR3_ZERO	D3DXVECTOR3(0.f, 0.f, 0.f)
#define VECTOR4_ZERO	D3DXVECTOR4(0.f, 0.f, 0.F, 0.f)

#define UNIT_X		D3DXVECTOR3(1.f, 0.f, 0.f)
#define UNIT_Y		D3DXVECTOR3(0.f, 1.f, 0.f)
#define UNIT_Z		D3DXVECTOR3(0.f, 0.f, 1.f)


#define ZERO_TOLERANCE	1e-06f

#undef RadToDeg
#undef DegToRad

#ifndef U2_PI
#define U2_PI 3.1415926535897932384626433832795f
#endif
#ifndef DegToRad
#define DegToRad(D) ((D) * (U2_PI/180.0f))
#endif
#ifndef RadToDeg
#define RadToDeg(R) ((R) * (180.0f/U2_PI))
#endif

#define U2_TWO_PI 2 * U2_PI
#define U2_HALF_PI U2_PI * 0.5f


// Nvidia PracticalPSM Util.h
// Added 2010/06/11
#define DW_AS_FLT(DW) (*(FLOAT*)&(DW))
#define FLT_AS_DW(F) (*(DWORD*)&(F))
#define FLT_SIGN(F) ((FLT_AS_DW(F) & 0x80000000L))
#define ALMOST_ZERO(F) ((FLT_AS_DW(F) & 0x7f800000L)==0)
#define IS_SPECIAL(F)  ((FLT_AS_DW(F) & 0x7f800000L)==0x7f800000L)


//D3DXMATRIX* WINAPI D3DXMatrixRotationAxis(D3DXMATRIX *pout, CONST D3DXVECTOR3 *pv, FLOAT angle)
//{
//	    D3DXVECTOR3 v;
//	
//	    D3DXVec3Normalize(&v,pv);
//	    D3DXMatrixIdentity(pout);
//	   pout->m[0][0] = (1.0f - cos(angle)) * v.x * v.x + cos(angle);
//	    pout->m[1][0] = (1.0f - cos(angle)) * v.x * v.y - sin(angle) * v.z;
//	    pout->m[2][0] = (1.0f - cos(angle)) * v.x * v.z + sin(angle) * v.y;
//	    pout->m[0][1] = (1.0f - cos(angle)) * v.y * v.x + sin(angle) * v.z;
//	    pout->m[1][1] = (1.0f - cos(angle)) * v.y * v.y + cos(angle);
//	    pout->m[2][1] = (1.0f - cos(angle)) * v.y * v.z - sin(angle) * v.x;
//	    pout->m[0][2] = (1.0f - cos(angle)) * v.z * v.x - sin(angle) * v.y;
//	    pout->m[1][2] = (1.0f - cos(angle)) * v.z * v.y + sin(angle) * v.x;
//	    pout->m[2][2] = (1.0f - cos(angle)) * v.z * v.z + cos(angle);
//	    return pout;
//}

#ifdef _DEBUG 
U2_3D int DPrintf(const char* formatString, ...);
U2_3D void DPrintf(const D3DXMATRIX& matrix);
U2_3D void DPrintf(const char* description, const D3DXMATRIX& matrix);

#else 
inline int DPrintf(const char* formatString, ...) { return 0; }
inline void DPrintf(const D3DXMATRIX& matrix) {}
inline void DPrintf(const char* description, const D3DXMATRIX& matrix) {}
#endif



namespace U2Math {;



// calculate cross product
template<class TVector>
inline TVector Cross(const TVector &vA, const TVector &vB)
{
	return TVector( (vA.y * vB.z) - (vA.z * vB.y),
		(vA.z * vB.x) - (vA.x * vB.z),
		(vA.x * vB.y) - (vA.y * vB.x) );
}

// calculate dot product
template<class TVector>
inline float Dot(const TVector &vA, const TVector &vB)
{
	return (vA.x * vB.x) + (vA.y * vB.y) + (vA.z * vB.z);
}



 static inline int FastFloatToInt(float fValue)
{
	int iValue;
	_asm fld fValue
	_asm fistp iValue
	return iValue;
}


static inline DWORD F2DW(float fValue) 
{ 
	return *((DWORD*)&fValue); 
}



bool IsPowerOf2(unsigned int uiI);

float ACos(float fValue);
float ASin(float fValue);
float ATan(float fValue);
float ATan2(float fNumerator, float fDenominator);
float Exp(float fValue);
float Floor(float fValue);
float Fmod(float fNumerator, float fDenominator);
float Log(float fValue);
float Pow(float fBase, float fExponent);
float Sin(float fRadians);
float Cos(float fRadians);
float FAbs(float fValue);
float Tan(float fValue);
float Sqr(float fValue);
float Sqrt(float fValue);
float InvSqrt (float fValue);

bool FEqual(float f0, float f1, float tol);
bool Vec3Equal(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, float tol);

float Saturate(float val);

template<class T>T Min(T a, T b);
template<class T>T Max(T a, T b);
template<class T>T Clamp(T fValue, T fMin, T fMax);

U2_3D void Orthonormalize (D3DXVECTOR3& v0, D3DXVECTOR3& v1, D3DXVECTOR3& v2);

U2_3D D3DXQUATERNION FromRotationMatrix(const D3DXVECTOR3 col[3]);

void Orthonormalize(D3DXMATRIX& outMat, const D3DXMATRIX& inMat);


D3DXMATRIX TimesDiagonal(const D3DXMATRIX& mat, const D3DXVECTOR3& diag);


void ToRotationMatrix(const D3DXQUATERNION& quat,D3DXVECTOR3 rotColumn[3]);

void  MakeMatrix(D3DXMATRIX& output, const D3DXVECTOR3& u, const D3DXVECTOR3& v,
						const D3DXVECTOR3& w, bool bColumns);

D3DXVECTOR3 GetCol(const D3DXMATRIX& mat, int col);

D3DXVECTOR3 MatProductVec(const D3DXMATRIX& mat, const D3DXVECTOR3& v);




// Generate a random number in [0,1).  The random number generator may
// be seeded by a first call to UnitRandom with a positive seed.
float UnitRandom (unsigned int uiSeed = 0);


// Generate a random number in [-1,1).  The random number generator may
// be seeded by a first call to SymmetricRandom with a positive seed.
float RandomFloat(unsigned int uiSeed = 0);

// Generate a random number in [min,max).  The random number generator may
// be seeded by a first call to IntervalRandom with a positive seed.
float  IntervalRandom (float fMin, float fMax,
							unsigned int uiSeed = 0);
D3DCOLOR randomColor(void);


int Compare(const D3DXVECTOR2& v0, const D3DXVECTOR2& v1, float tol);
int Compare(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, float tol);
int Compare(const D3DXVECTOR4& v0, const D3DXVECTOR4& v1, float tol);

// Animation Builder Optimize
D3DXVECTOR4 Minimum(const D3DXVECTOR4& v0, const D3DXVECTOR4& v1);
D3DXVECTOR4 Maximum(const D3DXVECTOR4& v0, const D3DXVECTOR4& v1);


float U2Lerp(float x, float y, float l);

float Vec2SquareLength(const D3DXVECTOR2& v);
float Vec3SquareLength(const D3DXVECTOR3& v);
float Vec4SquareLength(const D3DXVECTOR4& v);



D3DXVECTOR3 Vec3Transform(const D3DXVECTOR3& vIn, const D3DXMATRIX& tm);


// Input W must be a unit-length vector.  The output vectors {U,V} are
// unit length and mutually perpendicular, and {U,V,W} is an orthonormal
// basis.
void GenerateComplementBasis (D3DXVECTOR3& rkU, D3DXVECTOR3& rkV,
									 const D3DXVECTOR3& rkW);


int RandomNumber(int iMin, int iMax);
float RandomNumber(float fMin, float fMax);

}

#include "U2Math.inl"


