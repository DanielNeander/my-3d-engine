inline U2Triangle3::U2Triangle3()
{

}

//-------------------------------------------------------------------------------------------------
inline U2Triangle3::U2Triangle3(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, 
			const D3DXVECTOR3& v2)			
{
	v[0] = v0;
	v[1] = v1;
	v[2] = v2;
}

//-------------------------------------------------------------------------------------------------
inline U2Triangle3::U2Triangle3(const D3DXVECTOR3 v[3])
{
	memcpy_s(this->v, 3 * sizeof(D3DXVECTOR3), v, 
		3 * sizeof(D3DXVECTOR3));
}

//-------------------------------------------------------------------------------------------------
//inline float U2Triangle3::DistanceTo(const D3DXVECTOR3& q) const
//{
//	// e : edge 
//
//	D3DXVECTOR3 diff = v[0] - q;
//	D3DXVECTOR3 e0 = v[1] - v[0], e1 = v[2] - v[0];
//	float fE0LenSqr = D3DXVec3LenSqr(&e0);
//	float fE0DotE1 = D3DXVec3Dot(&e0, &e1);
//	float fE1LenSqr = D3DXVec3LenSqr(&e1);
//	float fDiffDotE0 = D3DXVec3Dot(&diff, &e0);
//	float fDiffDotE1 = D3DXVec3Dot(&diff, &e1);
//	float fDiffLenSqr = D3DXVec3LenSqr(&diff);
//	float fDet = U2Math::FAbs(fE0LenSqr * fE1LenSqr - fE0DotE1 * fE0DotE1);
//	float fS = fE0DotE1 * fDiffDotE1 - fE1LenSqr * fDiffDotE0;
//	float fT = fE0DotE1 * fDiffDotE0 - fE0LenSqr * fDiffDotE1;
//	float fSqrDist;
//
//	if(fS + fT <= fDet)
//	{
//		
//	}
//}