#include <U2_3D/src/U23DLibPCH.h>
#include "U2IntectLineTriangle3.h"

//-------------------------------------------------------------------------------------------------
U2IntectLineTriangle3::U2IntectLineTriangle3(const U2Line3D& line, 
											 const U2Triangle3& triangle)
											 :m_spLine((U2Line3D*)&line),
											 m_pTriangle(&triangle)
{
}

U2IntectLineTriangle3::~U2IntectLineTriangle3()
{
	m_spLine = 0;
	U2_DELETE m_pTriangle;
}


//-------------------------------------------------------------------------------------------------
bool U2IntectLineTriangle3::Test()
{
	// compute the offset origin, edges, and normal
	D3DXVECTOR3 diff = m_spLine->m_vOrigin - m_pTriangle->v[0];
	D3DXVECTOR3 edge1 = m_pTriangle->v[1] - m_pTriangle->v[0];
	D3DXVECTOR3 edge2 = m_pTriangle->v[2] - m_pTriangle->v[0];
	D3DXVECTOR3 normal;
	D3DXVec3Cross(&normal, &edge1, &edge2);

	// Solve Q + t*D = b1*E1 + b2*E2 (Q = kDiff, D = line direction,
	// E1 = kEdge1, E2 = kEdge2, N = Cross(E1,E2)) by
	//   |Dot(D,N)|*b1 = sign(Dot(D,N))*Dot(D,Cross(Q,E2))
	//   |Dot(D,N)|*b2 = sign(Dot(D,N))*Dot(D,Cross(E1,Q))
	//   |Dot(D,N)|*t = -sign(Dot(D,N))*Dot(Q,N)
	float fDDotN = D3DXVec3Dot(&m_spLine->m_vDir, &normal);
	float fSign;
	if(fDDotN > ZERO_TOLERANCE)
	{
		fSign = 1.0f;
	}
	else if(fDDotN < -ZERO_TOLERANCE)
	{
		fSign = -1.0f;
		fDDotN = -fDDotN;
	}
	else 
	{
		// Line and triangle are parallel, call it a "no intersection"
		// even if the line does intersect.
		return false;
	}

	D3DXVECTOR3 qCrossE2;
	D3DXVec3Cross(&qCrossE2, &diff, &edge2);
	float fDDotQCrossE2 = fSign * D3DXVec3Dot(&m_spLine->m_vDir, &qCrossE2);
	if(fDDotQCrossE2 >= 0.0f)
	{
		D3DXVECTOR3 e1CrossQ;
		D3DXVec3Cross(&e1CrossQ, &edge1, &diff);
		float fDDotE1CrossQ = fSign * D3DXVec3Dot(&m_spLine->m_vDir, &e1CrossQ);
		if(fDDotE1CrossQ >= 0.0f)
		{
			if(fDDotQCrossE2 + fDDotE1CrossQ <= fDDotN)
			{
				// line intersects triangle
				return true;
			}
			// else: b1+b2 > 1, no intersection
		}
		// else: b2 < 0, no intersection
	}
	// else: b1 < 0, no intersection
	return false;
}

//-------------------------------------------------------------------------------------------------
bool U2IntectLineTriangle3::Find()
{
	// compute the offset origin, edges, and normal
	D3DXVECTOR3 diff = m_spLine->m_vOrigin - m_pTriangle->v[0];
	D3DXVECTOR3 edge1 = m_pTriangle->v[1] - m_pTriangle->v[0];
	D3DXVECTOR3 edge2 = m_pTriangle->v[2] - m_pTriangle->v[0];
	D3DXVECTOR3 normal;
	D3DXVec3Cross(&normal, &edge1, &edge2);

	// Solve Q + t*D = b1*E1 + b2*E2 (Q = kDiff, D = line direction,
	// E1 = kEdge1, E2 = kEdge2, N = Cross(E1,E2)) by
	//   |Dot(D,N)|*b1 = sign(Dot(D,N))*Dot(D,Cross(Q,E2))
	//   |Dot(D,N)|*b2 = sign(Dot(D,N))*Dot(D,Cross(E1,Q))
	//   |Dot(D,N)|*t = -sign(Dot(D,N))*Dot(Q,N)
	float fDDotN = D3DXVec3Dot(&m_spLine->m_vDir, &normal);
	float fSign;
	if(fDDotN > ZERO_TOLERANCE)
	{
		fSign = 1.0f;
	}
	else if(fDDotN < -ZERO_TOLERANCE)
	{
		fSign = -1.0f;
		fDDotN = -fDDotN;
	}
	else 
	{
		// Line and triangle are parallel, call it a "no intersection"
		// even if the line does intersect.
		return false;
	}

	D3DXVECTOR3 qCrossE2;
	D3DXVec3Cross(&qCrossE2, &diff, &edge2);
	float fDDotQCrossE2 = fSign * D3DXVec3Dot(&m_spLine->m_vDir, &qCrossE2);
	if(fDDotQCrossE2 >= 0.0f)
	{
		D3DXVECTOR3 e1CrossQ;
		D3DXVec3Cross(&e1CrossQ, &edge1, &diff);
		float fDDotE1CrossQ = fSign * D3DXVec3Dot(&m_spLine->m_vDir, &e1CrossQ);
		if(fDDotE1CrossQ >= 0.0f)
		{
			if(fDDotQCrossE2 + fDDotE1CrossQ <= fDDotN)
			{
				// line intersects triangle
				float fQDotN = -fSign * D3DXVec3Dot(&diff, &normal);
				float fInv = 1.0f / fDDotN;
				m_fLineT = fQDotN * fInv;
				m_fTriB1 = fDDotQCrossE2 * fInv;
				m_fTriB2 = fDDotE1CrossQ * fInv;
				m_fTriB0 = 1.0f - m_fTriB1 - m_fTriB2;
				return true;
			}
			// else: b1+b2 > 1, no intersection
		}
		// else: b2 < 0, no intersection
	}
	// else: b1 < 0, no intersection
	return false;

}


