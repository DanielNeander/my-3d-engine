
namespace U2Math {;

inline bool IsPowerOf2(unsigned int uiI)
{
	if (!uiI)
		return false;

	return (uiI & (uiI - 1)) == 0;
}


//---------------------------------------------------------------------------
inline float ACos (float fValue)
{
	if ( -1.0f < fValue )
	{
		if ( fValue < 1.0f )
			return float(acos(fValue));
		else
			return 0.0f;
	}
	else
	{
		return U2_PI;
	}
}
//---------------------------------------------------------------------------
inline float ASin (float fValue)
{
	if ( -1.0f < fValue )
	{
		if ( fValue < 1.0f )
			return float(asin(fValue));
		else
			return U2_HALF_PI;
	}
	else
	{
		return -U2_HALF_PI;
	}
}
//---------------------------------------------------------------------------
inline float ATan (float fValue)
{
	return float(atan(fValue));
}
//---------------------------------------------------------------------------
inline float ATan2 (float fNumerator, float fDenominator)
{
	return float(atan2(fNumerator, fDenominator));
}
//---------------------------------------------------------------------------



inline float Floor (float fValue)
{
	return float(floor(fValue));
}

inline float Fmod (float fNumerator, float fDenominator)
{
	return float(fmod(fNumerator, fDenominator));
}

inline float Log (float fValue)
{
	return float(log(fValue));
}

inline float Cos (float fRadians)
{
	return float(cos(fRadians));
}

inline float Sin (float fRadians)
{
	return float(sin(fRadians));
}

inline float FAbs(float fValue)
{
	return (float)fabs(fValue);
}

inline float Tan(float fValue)
{
	return (float)tanf(fValue);
}

inline float Sqr (float fValue)
{
	return fValue*fValue;
}
//---------------------------------------------------------------------------
inline float Sqrt (float fValue)
{
	return float(sqrt(fValue));
}

inline float InvSqrt (float fValue)
{
	return 1.0f/sqrt(fValue);
}


template<class T>
inline T Min(T a, T b)
{
	return (a < b) ? a : b;
}
//---------------------------------------------------------------------------
template<class T>
inline T Max(T a, T b)
{
	return (a > b) ? a : b;
}

template<class T>
inline T Clamp(T fValue, T fMin, T fMax)
{
	U2ASSERT(fMin <= fMax);
	fValue = Max(fValue, fMin);
	fValue = Min(fValue, fMax);
	return fValue;
}

inline float Saturate(float val)
{
	if (val < 0.0f)      return 0.0f;
	else if (val > 1.0f) return 1.0f;
	else return val;
}

inline bool FEqual(float f0, float f1, float tol) {
	float f = f0-f1;
	if ((f>(-tol)) && (f<tol)) return true;
	else                       return false;
}

inline bool Vec3Equal(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, float tol)
{
	if(FAbs(v1.x - v0.x) > tol) return false;
	else if(FAbs(v1.y - v0.y) > tol) return false;
	else if(FAbs(v1.z - v0.z) > tol) return false;
	return true;	
}

inline D3DXMATRIX TimesDiagonal(const D3DXMATRIX& mat, const D3DXVECTOR3& diag)
{
	return D3DXMATRIX(mat._11 * diag.x, mat._12 * diag.y, mat._13 * diag.z,0.f,
		mat._21 * diag.x, mat._22 * diag.y, mat._13 * diag.z,0.f,
		mat._31 * diag.x, mat._32 * diag.y, mat._33 * diag.z,0.f,
		0.f, 0.f, 0.f, 0.f);
}




inline void ToRotationMatrix(const D3DXQUATERNION& quat,D3DXVECTOR3 rotColumn[3])
{
	float fAngle;
	D3DXQuaternionToAxisAngle(&quat, rotColumn, &fAngle);
}


inline void  MakeMatrix(D3DXMATRIX& output, const D3DXVECTOR3& u, const D3DXVECTOR3& v,
				  const D3DXVECTOR3& w, bool bColumns)
{
	if(bColumns)
	{
		output._11 = u.x;
		output._12 = v.x;
		output._13 = w.x;
		output._21 = u.y;
		output._22 = v.y;
		output._23 = w.y;
		output._31 = u.z;
		output._32 = v.z;
		output._33 = w.z;
	}
	else 
	{
		output._11 = u.x;
		output._12 = u.y;
		output._13 = u.z;
		output._21 = v.x;
		output._22 = v.y;
		output._23 = v.z;
		output._31 = w.x;
		output._32 = w.y;
		output._33 = w.z;

	}	

}


inline D3DXVECTOR3 GetCol(const D3DXMATRIX& mat, int col)
{
	return D3DXVECTOR3(mat.m[0][col], mat.m[1][col], mat.m[2][col]);
}


inline D3DXVECTOR3 MatProductVec(const D3DXMATRIX& mat, const D3DXVECTOR3& v)
{
	return D3DXVECTOR3(
		mat._11 * v.x + mat._21 * v.y + mat._31 * v.z,
		mat._12 * v.x + mat._22 * v.y + mat._32 * v.z,
		mat._13 * v.x + mat._23 * v.y + mat._33 * v.z);	
}


//-----------------------------------------------------------------------------
// 
// Compute a random color
//
//-----------------------------------------------------------------------------
inline D3DCOLOR randomColor(void)
{
	return(D3DCOLOR_RGBA(rand() & 0xFF, rand() & 0xFF, rand() & 0xFF, rand() & 0xFF));
}



inline float UnitRandom (unsigned int uiSeed )
{
	if (uiSeed > 0)
	{
		srand(uiSeed);
	}

	double dRatio = ((double)rand())/((double)(RAND_MAX));
	return (float)dRatio;
}




inline float RandomFloat (unsigned int uiSeed)
{
	if (uiSeed > 0.0)
	{
		srand(uiSeed);
	}
	double dRatio = ((double)rand())/((double)(RAND_MAX));
	return (float)(2.0*dRatio - 1.0);
}



inline float IntervalRandom (float fMin, float fMax, unsigned int uiSeed)
{
	if (uiSeed > 0)
	{
		srand(uiSeed);
	}

	double dRatio = ((double)rand())/((double)(RAND_MAX));
	return fMin+(fMax-fMin)*((float)dRatio);
}

inline D3DXVECTOR3 Vec3Transform(const D3DXVECTOR3& vIn, const D3DXMATRIX& tm)
{
	D3DXMATRIX transposeTM;
	D3DXMatrixTranspose(&transposeTM, &tm);

	D3DXVECTOR3 vOut;
	D3DXVec3TransformCoord(&vOut, &vIn, &transposeTM);

	return vOut;
}


inline void Orthonormalize(D3DXMATRIX& outMat, const D3DXMATRIX& inMat)
{
	// Algorithm uses Gram-Schmidt orthogonalization.  If 'this' matrix is
	// M = [m0|m1|m2], then orthonormal output matrix is Q = [q0|q1|q2],
	//
	//   q0 = m0/|m0|
	//   q1 = (m1-(q0*m1)q0)/|m1-(q0*m1)q0|
	//   q2 = (m2-(q0*m2)q0-(q1*m2)q1)/|m2-(q0*m2)q0-(q1*m2)q1|
	//
	// where |V| indicates length of vector V and A*B indicates dot
	// product of vectors A and B.

	// compute q0
	float fInvLen = InvSqrt(inMat._11 * inMat._11 + inMat._21 * inMat._21 +
		inMat._31 * inMat._31);

	outMat._11 = inMat._11 * fInvLen;
	outMat._21 = inMat._21 * fInvLen;
	outMat._31 = inMat._31 * fInvLen;

	float fDot0 = outMat._11 * inMat._12 + outMat._21 * inMat._22 + 
		outMat._31 * inMat._32;
	
	outMat._12 = inMat._12 - fDot0 * outMat._11;
	outMat._22 = inMat._22 - fDot0 * outMat._21;
	outMat._32 = inMat._32 - fDot0 * outMat._31;

	fInvLen = InvSqrt(outMat._12 * outMat._12 + 
		outMat._22 * outMat._22 + outMat._32 * outMat._32);

	outMat._12 *= fInvLen;
	outMat._22 *= fInvLen;
	outMat._32 *= fInvLen;

	float fDot1 = outMat._12 * inMat._13 + outMat._22 * inMat._23 +
		outMat._32 * inMat._33;

	fDot0 = outMat._11 * inMat._13 + outMat._21 * inMat._23 + outMat._31 *
		inMat._33;

	outMat._13 = inMat._13 - fDot0 * outMat._11 + fDot1 * outMat._12;
	outMat._23 = inMat._23 - fDot0 * outMat._21 + fDot1 * outMat._22;
	outMat._33 = inMat._33 - fDot0 * outMat._31 + fDot1 * outMat._32;

	fInvLen = InvSqrt(outMat._13 * outMat._13 + outMat._23 * outMat._23 +
		outMat._33 * outMat._33);

	outMat._13 *= fInvLen;	
	outMat._23 *= fInvLen;	
	outMat._33 *= fInvLen;	

	
}

inline int Compare(const D3DXVECTOR4& v0, const D3DXVECTOR4& v1, float tol)
{
	if (FAbs(v1.x - v0.x) > tol)      return (v1.x > v0.x) ? +1 : -1;
	else if (FAbs(v1.y - v0.y) > tol) return (v1.y > v0.y) ? +1 : -1;
	else if (FAbs(v1.z - v0.z) > tol) return (v1.z > v0.z) ? +1 : -1;
	else if (FAbs(v1.w - v0.w) > tol) return (v1.w > v0.w) ? +1 : -1;
	else                          return 0;
}

inline int Compare(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, float tol)
{
	if (FAbs(v1.x - v0.x) > tol)      return (v1.x > v0.x) ? +1 : -1;
	else if (FAbs(v1.y - v0.y) > tol) return (v1.y > v0.y) ? +1 : -1;
	else if (FAbs(v1.z - v0.z) > tol) return (v1.z > v0.z) ? +1 : -1;
	else                          return 0;
}

inline int Compare(const D3DXVECTOR2& v0, const D3DXVECTOR2& v1, float tol)
{
	if (FAbs(v1.x - v0.x) > tol)      return (v1.x > v0.x) ? +1 : -1;
	else if (FAbs(v1.y - v0.y) > tol) return (v1.y > v0.y) ? +1 : -1;	
	else                          return 0;
}


inline D3DXVECTOR4 Minimum(const D3DXVECTOR4& v0, const D3DXVECTOR4& v1)
{
	D3DXVECTOR4 res(v0);
	if (v1.x < v0.x) res.x = v1.x;
	if (v1.y < v0.y) res.y = v1.y;
	if (v1.z < v0.z) res.z = v1.z;
	if (v1.w < v0.w) res.w = v1.w;
	return res;
}

inline D3DXVECTOR4 Maximum(const D3DXVECTOR4& v0, const D3DXVECTOR4& v1)
{
	D3DXVECTOR4 res(v0);
	if (v1.x > v0.x) res.x = v1.x;
	if (v1.y > v0.y) res.y = v1.y;
	if (v1.z > v0.z) res.z = v1.z;
	if (v1.w > v0.w) res.w = v1.w;
	return res;
}

//------------------------------------------------------------------------------
/**
Linearly interpolate between 2 values: ret = x + l * (y - x)
*/
inline float U2Lerp(float x, float y, float l)
{
	return x + l * (y - x);
}

inline float Vec2SquareLength(const D3DXVECTOR2& v)
{
	return v.x * v.x + v.y * v.y;
}

inline float Vec3SquareLength(const D3DXVECTOR3& v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline float Vec4SquareLength(const D3DXVECTOR4& v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

inline void GenerateComplementBasis (D3DXVECTOR3& rkU, D3DXVECTOR3& rkV,
							  const D3DXVECTOR3& rkW)
{
	float fInvLen;

	if(FAbs(rkW.x) >= FAbs(rkW.y))
	{
		// W.x or W.z is the largest magnitude component, swap them
		fInvLen = InvSqrt(rkW.x * rkW.x + rkW.z * rkW.z);
		rkU.x = -rkW.z * fInvLen;
		rkU.y = 0.0f;
		rkU.z = rkW.x * fInvLen;
		rkV.x = rkW.y * rkU.z;
		rkV.y = rkW.z * rkU.x - rkW.x * rkU.z;
		rkV.z = -rkW.y * rkU.x;
	}
	else 
	{
		// W.y or W.z is the largest magnitude component, swap them
		fInvLen = InvSqrt(rkW.y * rkW.y + rkW.z * rkW.z);
		rkU.x = 0.0f;
		rkU.y = rkW.z * fInvLen;
		rkU.z = -rkW.y * fInvLen;
		rkV.x = rkW.y * rkU.z - rkW.z * rkU.y;
		rkV.y = -rkW.x * rkU.z;
		rkV.z = rkW.x * rkU.y;
	}
}


inline int RandomNumber(int iMin, int iMax)
{
	if (iMin == iMax) return iMin;
	return ((rand() % (abs(iMax-iMin) + 1)) + iMin);
}

inline float RandomNumber(float fMin, float fMax)
{
	if (fMin == fMax) return(fMin);
	float fRandom = (float)rand() / (float)RAND_MAX;
	return((fRandom * (float)fabs(fMax-fMin))+fMin);
}

} // namespace U2Math







