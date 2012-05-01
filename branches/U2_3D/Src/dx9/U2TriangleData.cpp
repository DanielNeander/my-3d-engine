#include <U2_3D/src/U23DLibPCH.h>
#include "U2TriangleData.h"


IMPLEMENT_RTTI(U2TriangleData, U2MeshData);
U2TriangleData::U2TriangleData(unsigned int uiNumVerts, D3DXVECTOR3* pVert, 
							 D3DXVECTOR3 *pNormal, D3DXCOLOR* pColor, 
							 D3DXVECTOR2* pTexCoords,
							 unsigned short numTexCoordSets, 
							 unsigned short numTriangles 
							 ) : U2MeshData(uiNumVerts, 
							 pVert, pNormal, pColor, pTexCoords, 
							 numTexCoordSets),
							 m_pTangents(NULL),
							 m_pBinormals(NULL)
{
	m_uTriCnt = numTriangles;
	m_uMaxTriCnt = numTriangles;

}

U2TriangleData::U2TriangleData()
:m_pTangents(NULL),
m_pBinormals(NULL)
{


}

U2TriangleData::~U2TriangleData()
{
	U2_FREE(m_pBinormals);
	m_pBinormals = 0;
	U2_FREE(m_pTangents);
	m_pTangents = 0;
}



unsigned short U2TriangleData::GetTriangleCnt() const
{
	return m_uMaxTriCnt;
}


void U2TriangleData::SetTriangleCount(unsigned short usTriangles)
{	
	m_uMaxTriCnt = usTriangles;
	m_uTriCnt = usTriangles;
}


void U2TriangleData::SetActiveTriangleCount(
	unsigned short usActiveTriangle)
{
	m_uTriCnt = usActiveTriangle;
}
//---------------------------------------------------------------------------
unsigned short U2TriangleData::GetActiveTriangleCount() const
{
	return m_uTriCnt;
}

void U2TriangleData::GetTriangleIndices(unsigned short i, unsigned short& i0,
								unsigned short& i1, unsigned short& i2) const
{

}

//-------------------------------------------------------------------------------------------------
void U2TriangleData::CreateBinormals(bool bInit)
{
	U2ASSERT(m_uiNumVerts > 0);
	if(!m_pBinormals)
		m_pBinormals = U2_ALLOC(D3DXVECTOR3, m_uiNumVerts);

	if(bInit)
		memset(m_pBinormals, 0, sizeof(m_pBinormals[0]) * m_uiNumVerts);

}

//-------------------------------------------------------------------------------------------------
void U2TriangleData::CreateTangents(bool bInit)
{

	U2ASSERT(m_uiNumVerts > 0);
	if(!m_pTangents)
		m_pTangents = U2_ALLOC(D3DXVECTOR3, m_uiNumVerts);

	if(bInit)
		memset(m_pTangents, 0, sizeof(m_pTangents[0]) * m_uiNumVerts);

}

//-------------------------------------------------------------------------------------------------
void U2TriangleData::GenerateNormals()
{
	CreateNormals(true);

	UpdateModelNormals();

}

//-------------------------------------------------------------------------------------------------
void U2TriangleData::GenerateTangents(int iTcoordUnit)
{


}

void U2TriangleData::UpdateModelNormals()
{
	
}

//-------------------------------------------------------------------------------------------------
void U2TriangleData::UpdateModelTangentSpace(int iTangentSpaceType)
{
	if(iTangentSpaceType <= (int)U2Mesh::TANGENTSPACE_NONE 
		||!m_pVertsAttb 
		|| !m_pVertsAttb->HasNormal() 
		|| !m_pVertsAttb->HasTangent()
		|| !m_pVertsAttb->HasBitangent())
	{
		return;
	}

	if(iTangentSpaceType == (int)U2Mesh::TANGENTSPACE_USE_MESH)
	{
		UpdateTangentSpaceUseGeometry();
	}
	else 
	{
		UpdateTangetSpaceUseTCoord(iTangentSpaceType);
	}
}

//-------------------------------------------------------------------------------------------------
void U2TriangleData::UpdateTangentSpaceUseGeometry()
{
	const unsigned int uiActiveVerts = GetActiveVertexCount();
	
	D3DXMATRIX* pDNormal = U2_ALLOC(D3DXMATRIX, uiActiveVerts);
	D3DXMATRIX* pWWTM = U2_ALLOC(D3DXMATRIX, uiActiveVerts);
	D3DXMATRIX* pDWTM = U2_ALLOC(D3DXMATRIX, uiActiveVerts);

	memset(pDWTM, 0, sizeof(D3DXMATRIX) * uiActiveVerts);
	memset(pWWTM, 0, sizeof(D3DXMATRIX) * uiActiveVerts);

	const unsigned int uiActiveTris = GetActiveTriangleCount();
	uint32 i, row, col;
	for(i=0; i < uiActiveTris; ++i)
	{
		uint16 aiV[3];
		GetTriangleIndices(i, aiV[0], aiV[1], aiV[2]);

		for(uint32 j=0; j < 3; ++j)
		{
			uint16 iV0 = aiV[j];
			uint16 iV1 = aiV[(j+1) % 3];
			uint16 iV2 = aiV[(j+2) % 3];

			const D3DXVECTOR3& v0 = m_pVerts[iV0];
			const D3DXVECTOR3& v1 = m_pVerts[iV1];
			const D3DXVECTOR3& v2 = m_pVerts[iV2];
			const D3DXVECTOR3& n0 = m_pNorms[iV0];
			const D3DXVECTOR3& n1 = m_pNorms[iV1];
			const D3DXVECTOR3& n2 = m_pNorms[iV2];

			// Compute edge from V0 to V1, project to tangent plane of vertex,
			// and compute difference of adjacent normals.
			D3DXVECTOR3 vEdge = v1 - v0;
			D3DXVECTOR3 vW = vEdge - D3DXVec3Dot(&vEdge, &n0) * n0;
			D3DXVECTOR3 vD = n1 - n0;
			float* pafDataW = vW;
			float* pafDataD = vD;

			for(row = 0; row < 3; ++row)
			{
				for(col = 0; col < 3; ++col)
				{
					pWWTM[iV0].m[row][col] += pafDataW[row] * pafDataW[col];
					pDWTM[iV0].m[row][col] += pafDataD[row] * pafDataW[col];
				}
			}

			// Compute edge from V0 to V2, project to tangent plane of vertex,
			// and compute difference of adjacent normals.

			vEdge = v2 - v0;
			vW = vEdge - D3DXVec3Dot(&vEdge, &n0) * n0;
			vD = n2 - n0;
			pafDataW = vW;
			pafDataD = vD;
			for(row = 0; row < 3; ++row)
			{
				for(col = 0; col < 3; ++col)
				{
					pWWTM[iV0].m[row][col] += pafDataW[row] * pafDataW[col];
					pDWTM[iV0].m[row][col] += pafDataD[row] * pafDataW[col];
				}
			}
		}
	}

	// Add in N*N^T to W*W^T for numerical stability.  In theory 0*0^T gets
	// added to D*W^T, but of course no update needed in the implementation.
	// Compute the matrix of normal derivatives.
	for(i = 0; i < uiActiveVerts; ++i)
	{
		const D3DXVECTOR3& normal = m_pNorms[i];
		const float* pafData = normal;
		for(row = 0; row < 3; ++row)
		{
			for(col = 0; col < 3; ++col)
			{
				pWWTM[i].m[row][col] = 0.5f * pWWTM[i].m[row][col] +
					pafData[row] * pafData[col];
				pDWTM[i].m[row][col] *= 0.5f;
			}
		}		
	}

	U2_FREE(pWWTM);
	U2_FREE(pDWTM);

	// If N is a unit-length normal at a vertex, let U and V be unit-length
	// tangents so that {U, V, N} is an orthonormal set.  Define the matrix
	// J = [U | V], a 3-by-2 matrix whose columns are U and V.  Define J^T
	// to be the transpose of J, a 2-by-3 matrix.  Let dN/dX denote the
	// matrix of first-order derivatives of the normal vector field.  The
	// shape matrix is
	//   S = (J^T * J)^{-1} * J^T * dN/dX * J = J^T * dN/dX * J
	// where the superscript of -1 denotes the inverse.  (The formula allows
	// for J built from non-perpendicular vectors.) The matrix S is 2-by-2.
	// The principal curvatures are the eigenvalues of S.  If k is a principal
	// curvature and W is the 2-by-1 eigenvector corresponding to it, then
	// S*W = k*W (by definition).  The corresponding 3-by-1 tangent vector at
	// the vertex is called the principal direction for k, and is J*W.  The
	// principal direction for the minimum principal curvature is stored as
	// the mesh tangent.  The principal direction for the maximum principal
	// curvature is stored as the mesh bitangent.
	for(i=0; i < uiActiveVerts; ++i)
	{
		const D3DXVECTOR3& normal = m_pNorms[i];
		D3DXVECTOR3 vU, vV;
		U2Math::GenerateComplementBasis(vU, vV, normal);

		// Compute S = J^T * dN/dX * J.  In theory S is symmetric, but
		// because we have estimated dN/dX, we must slightly adjust our
		// calculations to make sure S is symmetric.
		
		float fS01 = D3DXVec3Dot(&vU, &U2Math::Vec3Transform(vV, pDNormal[i]));
		float fS10 = D3DXVec3Dot(&vV, &U2Math::Vec3Transform(vU, pDNormal[i]));
		float fSAvr = 0.5f * (fS01 + fS10);
		D3DXMATRIX sTM;
		memset(&sTM, 0, sizeof(D3DXMATRIX));
		// Make 2x2 Matrix 
		sTM.m[0][0] = D3DXVec3Dot(&vU, &U2Math::Vec3Transform(vU, pDNormal[i]));
		sTM.m[0][1] = fSAvr;
		sTM.m[1][0] = fSAvr;
		sTM.m[1][1] = D3DXVec3Dot(&vV, &U2Math::Vec3Transform(vV, pDNormal[i]));

		// Compute the eigenvalues of S (min and max curvatures).
		float fTrace = sTM.m[0][0] * sTM.m[1][1];
		float fDet = sTM.m[0][0] * sTM.m[1][1] - sTM.m[0][1] * sTM.m[1][0];
		float fDiscr = fTrace * fTrace - 4.0f * fDet;
		float fRootDiscr = U2Math::Sqrt(U2Math::FAbs(fDiscr));
		float fMinCurvature = 0.5f * (fTrace - fRootDiscr);
		float fMaxCurvature = 0.5f * (fTrace + fRootDiscr);
		
		// Compute the eigenvectors of S.
		D3DXVECTOR2 vW0(sTM.m[0][1], fMinCurvature - sTM.m[0][0]);
		D3DXVECTOR2 vW1(fMinCurvature - sTM.m[1][1], sTM.m[1][0]);
		if(U2Math::Vec2SquareLength(vW0) >= U2Math::Vec2SquareLength(vW1))
		{
			D3DXVECTOR2 tempNorm = vW0;
			D3DXVec2Normalize(&vW0, &tempNorm);
			m_pTangents[i] = vW0.x * vU + vW0.y * vV;
		}
		else 
		{
			D3DXVECTOR2 tempNorm = vW1;
			D3DXVec2Normalize(&vW1, &tempNorm);
			m_pTangents[i] = vW1.x * vU + vW1.y * vV;
		}

		vW0 = D3DXVECTOR2(sTM.m[0][1], fMaxCurvature - sTM.m[0][0]);
		vW1 = D3DXVECTOR2(fMaxCurvature - sTM.m[1][1], sTM.m[1][0]);
		if(U2Math::Vec2SquareLength(vW0) >= U2Math::Vec2SquareLength(vW1))
		{
			D3DXVECTOR2 tempNorm = vW0;
			D3DXVec2Normalize(&vW0, &tempNorm);
			m_pBinormals[i] = vW0.x * vU + vW0.y * vV;
		}
		else 
		{
			D3DXVECTOR2 tempNorm = vW1;
			D3DXVec2Normalize(&vW1, &tempNorm);
			m_pBinormals[i] = vW1.x * vU + vW1.y * vV;
		}
	}
	U2_FREE(pDNormal);
}

void U2TriangleData::UpdateTangetSpaceUseTCoord(int iTCoordUnit)
{
	// Each vertex can be visited multiple times, so compute the tangent
	// space only on the first visit.  Use the zero vector as a flag for the
	// tangent vector not being computed.
	const uint32 uiActiveVerts = GetActiveVertexCount();
	uint32 i;
	
	const uint32 uiActiveTris = GetActiveTriangleCount();
	for(uint16 uiTri = 0; uiTri < uiActiveTris; ++uiTri)
	{
		uint16 usV0, usV1, usV2;

		GetTriangleIndices(uiTri, usV0, usV1, usV2);

		D3DXVECTOR3* pPos[3] = {
			&m_pVerts[usV0], 
			&m_pVerts[usV1], 
			&m_pVerts[usV2]
		};
		D3DXVECTOR3* pNorm[3] = { 
			&m_pNorms[usV0], 
			&m_pNorms[usV1], 
			&m_pNorms[usV2]
		};
		D3DXVECTOR3* pTangent[3] = { 
			&m_pTangents[usV0], 
			&m_pTangents[usV1], 
			&m_pTangents[usV2]
		};
		D3DXVECTOR3* pBinormal[3] = { 
			&m_pBinormals[usV0], 
			&m_pBinormals[usV1], 
			&m_pBinormals[usV2]
		};
		D3DXVECTOR2* pTCoord[3] = { 
			&GetTexCoordSet(iTCoordUnit)[usV0], 
			&GetTexCoordSet(iTCoordUnit)[usV1], 
			&GetTexCoordSet(iTCoordUnit)[usV2]
		};

		for(i=0; i < 3; ++i)
		{
			if(*pTangent[i] != VECTOR3_ZERO)
			{
				// This vertex has already been visited.
				continue;
			}

			// Compute the tangent space at the vertex.
			D3DXVECTOR3 normal = *pNorm[i];
			int iP = (i == 0) ? 2 : i - 1;
			int iN = (i + 1) % 3;
			D3DXVECTOR3 tangent = ComputeTangent(
				*pPos[i ], *pTCoord[i ],
				*pPos[iN], *pTCoord[iN],
				*pPos[iP], *pTCoord[iP]);

			// Project T into the tangent plane by projecting out the surface
			// normal N, and then make it unit length.
			tangent -= D3DXVec3Dot(&normal, &tangent) * normal;
			D3DXVECTOR3 tempTangent = tangent;
			D3DXVec3Normalize(&tangent, &tempTangent);

			// Compute the bitangent B, another tangent perpendicular to T.
			D3DXVECTOR3 binormal;
			D3DXVec3Cross(&binormal, &normal, &tangent);

			*pTangent[i] = tangent;
			*pBinormal[i] = binormal;
		}
	}
}

D3DXVECTOR3 U2TriangleData::ComputeTangent(
	const D3DXVECTOR3& p0, const D3DXVECTOR2& tcoord0,
	const D3DXVECTOR3& p1, const D3DXVECTOR2& tcoord1,
	const D3DXVECTOR3& p2, const D3DXVECTOR2& tcoord2 )
{
	// Compute the change in positions at the vertex P0.
	D3DXVECTOR3 diffP1 = p1 - p0;
	D3DXVECTOR3 diffP2 = p2 - p0;

	if(U2Math::FAbs(D3DXVec3Length(&diffP1)) < ZERO_TOLERANCE ||
		U2Math::FAbs(D3DXVec3Length(&diffP2)) < ZERO_TOLERANCE)
	{
		return VECTOR3_ZERO;
	}

	// Compute the change in texture coordinates at the vertex P0 in the
	// direction of edge P1-P0.
	float fDiffU1 = tcoord1.x - tcoord0.x;
	float fDiffV1 = tcoord1.y - tcoord0.y;
	if(U2Math::FAbs(fDiffV1) < ZERO_TOLERANCE)
	{
		// The triangle effectively has no variation in the v texture
		// coordinate.
		if(U2Math::FAbs(fDiffU1) < ZERO_TOLERANCE)
		{
			// The triangle effectively has no variation in the u coordinate.
			// Since the texture coordinates do not vary on this triangle,
			// treat it as a degenerate parametric surface.
			return VECTOR3_ZERO;
		}

		// The variation is effectively all in u, so set the tangent vector
		// to be T = dP/du.
		return diffP1 / fDiffU1;
	}

	// Compute the change in texture coordinates at the vertex P0 in the
	// direction of edge P2-P0.
	float fDiffU2 = tcoord2.x - tcoord0.x;
	float fDiffV2 = tcoord2.y - tcoord0.y;
	float fDet = fDiffV1 * fDiffU2 - fDiffV2 * fDiffU1;
	if(U2Math::FAbs(fDet) < ZERO_TOLERANCE)
	{
		// The triangle vertices are collinear in parameter space, so treat
		// this as a degenerate parametric surface.
		return VECTOR3_ZERO;
	}

	// The triangle vertices are not collinear in parameter space, so choose
	// the tangent to be dP/du = (dv1*dP2-dv2*dP1)/(dv1*du2-dv2*du1)
	return (fDiffV1 * diffP2 - fDiffV2 * diffP1) / fDet;

}