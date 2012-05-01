#include <u2_3d/src/U23DLibPCH.h>
#include "U2StandardMesh.h"



U2StandardMesh::U2StandardMesh(U2VertexAttributes* attr,  bool bInside, 
							   const U2Transform* pXform )
	:m_pAttr(attr),
	m_bInside(bInside)
{
	if(pXform)
	{
		m_xForm = *pXform;
	}

}


U2StandardMesh::~U2StandardMesh()
{
	
}


U2Mesh* U2StandardMesh::Retangle(int iXSamples, int iYSamples, float fXExtent, 
				 float fYExtent)
{
	int numVertCnt = iXSamples * iYSamples;
	int iTriangleCnt = 2 *(iXSamples-1) * (iYSamples - 1);

	U2TriListData* pModelData = U2_NEW U2TriListData();
	U2ASSERT(pModelData);

	// 정점의 수가 0일때 
	// 다른 객체의 포인터가 깨질수 있으므로 조심...
	// 예)U2Dx9Renderer 포인터가 손상됨.
	pModelData->SetVertexAttbs(m_pAttr);
	
	pModelData->SetVertexCount(numVertCnt, numVertCnt);


	pModelData->CreateVertices();	

	if(m_pAttr->HasNormal())
	{
		pModelData->CreateNormals(true);
	}

	if(m_pAttr->GetMaxColors())
	{
		pModelData->CreateColors();
	}

	if(m_pAttr->GetMaxTCoords())
	{
		pModelData->CreateTexCoords(m_pAttr->GetMaxTCoords());
	}
	
	float fInv0 = 1.0f /(iXSamples - 1.0f);
	float fInv1 = 1.0f /(iYSamples - 1.0f);
	float fU, fV;
	int i, i0, i1;
	for(i1 = 0, i = 0; i1 < iYSamples; i1++)
	{
		fV = i1 * fInv1;
		D3DXVECTOR3 yTmp = ((2.0f * fV - 1.0f) * fYExtent) * UNIT_Y;
		for(i0 = 0; i0 < iXSamples; ++i0)
		{
			fU = i0 * fInv0;
			D3DXVECTOR3 xTmp = ((2.0f * fU - 1.f)*fXExtent) * UNIT_X;		
			pModelData->GetVertices()[i] = xTmp + yTmp;			

			if(m_pAttr->HasNormal())
			{
				(pModelData->GetNormals())[i] = UNIT_Z;
			}
			
			if(m_pAttr->GetMaxTCoords() > 0)
			{
				
				D3DXVECTOR2 tCoord(fU, fV);
				for(int tSet=0; tSet < m_pAttr->GetMaxTCoords(); ++tSet)
				{
					if(m_pAttr->HasTCoord(tSet))
					{
						pModelData->GetTexCoordSet(tSet)[i] = tCoord;
					}
				}
			}
			++i;
		}			
	}

	// generate connectivity
	uint16 *pusTrilist = U2_ALLOC(uint16, 3 *iTriangleCnt);
	for(i1=0, i=0; i1 < iYSamples-1; i1++)
	{
		for(i0=0; i0< iXSamples -1; i0++)
		{
			uint16 v0 = (uint16)(i0 + iXSamples * i1);
			uint16 v1 = v0 + 1;
			uint16 v2 = v1 + iXSamples;
			uint16 v3 = v0 + iXSamples;
			pusTrilist[i++] = v0;
			pusTrilist[i++] = v1;
			pusTrilist[i++] = v2;
			pusTrilist[i++] = v0;
			pusTrilist[i++] = v2;
			pusTrilist[i++] = v3;
		}
	}

	pModelData->SetIndices(iTriangleCnt, iTriangleCnt, pusTrilist,
		NULL, 1);

	TransformData(*pModelData);
	U2TriList* pMesh = U2_NEW U2TriList(pModelData);
	return pMesh;
}

U2TriList* U2StandardMesh::Torus(int iCircleSmaples, int iRadialSmaples, float fOuterRadius, 
				 float fInnerRadius)
{
	int numVertCnt = (iCircleSmaples+ 1) * (iRadialSmaples + 1);
	int iTriangleCnt = 2 *(iCircleSmaples) * iRadialSmaples;

	U2TriListData* pModelData = U2_NEW U2TriListData();
	U2ASSERT(pModelData);

	// 정점의 수가 0일때 
	// 다른 객체의 포인터가 깨질수 있으므로 조심...
	// 예)U2Dx9Renderer 포인터가 손상됨.

	pModelData->SetVertexAttbs(m_pAttr);

	pModelData->SetVertexCount(numVertCnt, numVertCnt);

	pModelData->CreateVertices();	


	if(m_pAttr->HasNormal())
	{
		pModelData->CreateNormals(true);
	}

	if(m_pAttr->GetMaxTCoords())
	{
		pModelData->CreateTexCoords(m_pAttr->GetMaxTCoords());
	}

	if(m_pAttr->GetMaxColors())
	{
		pModelData->CreateColors();
	}


	float fInvCS = 1.0f/(float)iCircleSmaples;
	float fInvRS = 1.0f/(float)iRadialSmaples;
	int iC, iR, i, iUnit;
	D3DXVECTOR2 tcoord;

	for(iC=0, i=0; iC < iCircleSmaples; ++iC)
	{
		float fCircleFraction = iC*fInvCS; // in [0, 1]
		float fTheta = U2_TWO_PI * fCircleFraction;
		float fCosTheta = U2Math::Cos(fTheta);
		float fSinTheta = U2Math::Sin(fTheta);
		D3DXVECTOR3 vRadial(fCosTheta, fSinTheta, 0.f);
		D3DXVECTOR3 vTorusMiddle = fOuterRadius * vRadial;

		int iSave = i;
		for(iR=0; iR < iRadialSmaples; ++iR)
		{
			float fRadialFraction = iR * fInvRS;
			float fPhi = U2_TWO_PI * fRadialFraction;
			float fCosPhi = U2Math::Cos(fPhi);
			float fSinPhi = U2Math::Sin(fPhi);
			D3DXVECTOR3 vNormal = fCosPhi* vRadial + fSinPhi * UNIT_Z;
			pModelData->GetVertices()[i] = vTorusMiddle + fInnerRadius * vNormal;
			if(m_pAttr->HasNormal())
			{
				if(m_bInside)
				{
					pModelData->GetNormals()[i] = -vNormal;
				}
				else 
				{
					pModelData->GetNormals()[i] = vNormal;
				}
			}

			if(m_pAttr->GetMaxTCoords() > 0)
			{
				tcoord = D3DXVECTOR2(fRadialFraction, fCircleFraction);
				for(iUnit=0; iUnit < (int)m_pAttr->GetMaxTCoords(); ++iUnit)
				{
					if(m_pAttr->HasTCoord(iUnit))
					{
						pModelData->GetTexCoordSet(iUnit)[i] = tcoord;
					}
				}
			}

			++i;

		}

		pModelData->GetVertices()[i] = pModelData->GetVertices()[iSave];
		if(m_pAttr->HasNormal())
		{
			pModelData->GetNormals()[i] = pModelData->GetNormals()[iSave];
		}

		if(m_pAttr->GetMaxTCoords())
		{
			tcoord = D3DXVECTOR2(1.f, fCircleFraction);
			for(iUnit=0; iUnit < m_pAttr->GetMaxTCoords(); ++iUnit)
			{
				if(m_pAttr->HasTCoord(iUnit))
				{
					pModelData->GetTexCoordSet(iUnit)[i] = tcoord;
				}
			}
		}
		++i;

	}

	// duplicate the cylinder ends to form a torus
	for(iR = 0; iR <= iRadialSmaples; ++iR, ++i)
	{
		pModelData->GetVertices()[i] = pModelData->GetVertices()[iR];
		if(m_pAttr->HasNormal())
		{
			pModelData->GetNormals()[i] = pModelData->GetNormals()[iR];
		}

		if(m_pAttr->GetMaxTCoords())
		{
			tcoord = D3DXVECTOR2(pModelData->GetTexCoordSet(0)[iR].x, 1.f);
			for(iUnit=0; iUnit < (int)m_pAttr->GetMaxTCoords(); ++iUnit)
			{			
				if(m_pAttr->HasTCoord(iUnit))
				{
					pModelData->GetTexCoordSet(iUnit)[i] = tcoord;
				}
			}			
		}
	}

	U2ASSERT(i == numVertCnt);
	
	// Generate Connectvity
	
	uint16 *pusTrilist = U2_ALLOC(uint16, 3 *iTriangleCnt);
	unsigned short iCStart = 0;
	int j;
	for(iC=0, j=0; iC < iCircleSmaples; ++iC, ++i)
	{
		unsigned short i0 = iCStart;
		unsigned short i1 = i0 + 1;
		iCStart += (unsigned short)(iRadialSmaples + 1);
		unsigned short i2 = iCStart;
		unsigned short i3 = i2 + 1;
		for(i=0; i < iRadialSmaples; ++i, j+= 6)
		{
			if(m_bInside)
			{
				pusTrilist[j] = i0++;
				pusTrilist[j+1] = i1;
				pusTrilist[j+2] = i2;
				pusTrilist[j+3] = i1++;
				pusTrilist[j+4] = i3++;
				pusTrilist[j+5] = i2++;
			}
			else 
			{
				pusTrilist[j] = i0++;				
				pusTrilist[j+1] = i2;
				pusTrilist[j+2] = i1;
				pusTrilist[j+3] = i1++;				
				pusTrilist[j+4] = i2++;				
				pusTrilist[j+5] = i3++;				
			}
		}
	}	

	pModelData->SetIndices(iTriangleCnt, iTriangleCnt, pusTrilist,
		NULL, 1);

	TransformData(*pModelData);
	U2TriList* pMesh = U2_NEW U2TriList(pModelData);
	pMesh->UpdateMeshState();
	return pMesh;
}

//-------------------------------------------------------------------------------------------------
U2TriList* U2StandardMesh::Box(float fXExtent, float fYExtent, float fZExtent)
{
	uint16 numVertCnt = 8;
	uint16 uTris = 12;

	U2TriListData* pModelData = U2_NEW U2TriListData();
	U2ASSERT(pModelData);

	// 정점의 수가 0일때 
	// 다른 객체의 포인터가 깨질수 있으므로 조심...
	// 예)U2Dx9Renderer 포인터가 손상됨.
	pModelData->SetVertexAttbs(m_pAttr);

	pModelData->SetVertexCount(numVertCnt, numVertCnt);

	pModelData->CreateVertices();	

	D3DXVECTOR3* pVerts =pModelData->GetVertices();

	pVerts[0] = D3DXVECTOR3(-fXExtent, -fYExtent, -fZExtent);
	pVerts[1] = D3DXVECTOR3(+fXExtent, -fYExtent, -fZExtent);
	pVerts[2] = D3DXVECTOR3(+fXExtent, +fYExtent, -fZExtent);
	pVerts[3] = D3DXVECTOR3(-fXExtent, +fYExtent, -fZExtent);
	pVerts[4] = D3DXVECTOR3(-fXExtent, -fYExtent, +fZExtent);
	pVerts[5] = D3DXVECTOR3(+fXExtent, -fYExtent, +fZExtent);
	pVerts[6] = D3DXVECTOR3(+fXExtent, +fYExtent, +fZExtent);
	pVerts[7] = D3DXVECTOR3(-fXExtent, +fYExtent, +fZExtent);

	if(m_pAttr->GetMaxTCoords())
	{
		pModelData->CreateTexCoords(m_pAttr->GetMaxTCoords());
	}

	for(int iUnit=0; iUnit < m_pAttr->GetMaxTCoords(); ++iUnit)
	{
		if(m_pAttr->HasTCoord(iUnit))
		{
			D3DXVECTOR2* pTCoord = pModelData->GetTexCoordSet(iUnit);
			pTCoord[0] =D3DXVECTOR2(0.25f,0.75f);
			pTCoord[1] =D3DXVECTOR2(0.75f,0.75f);
			pTCoord[2] =D3DXVECTOR2(0.75f,0.25f);
			pTCoord[3] =D3DXVECTOR2(0.25f,0.25f);
			pTCoord[4] =D3DXVECTOR2(0.0f,1.0f);
			pTCoord[5] =D3DXVECTOR2(1.0f,1.0f);
			pTCoord[6] =D3DXVECTOR2(1.0f,1.0f);
			pTCoord[7] =D3DXVECTOR2(0.0f,0.0f);
		}
	}

	uint16 *pusIndices = U2_ALLOC(uint16, 3 * uTris);
	pusIndices[ 0]= 0;	pusIndices[ 1]= 2;	pusIndices[ 2] = 1;
	pusIndices[ 3]= 0;	pusIndices[ 4]= 3;	pusIndices[ 5] = 2;
	pusIndices[ 6]= 0;	pusIndices[ 7]= 1;	pusIndices[ 8] = 5;
	pusIndices[ 9]= 0;	pusIndices[10]= 5;	pusIndices[11] = 4;
	pusIndices[12]= 0;	pusIndices[13]= 4;	pusIndices[14] = 7;
	pusIndices[15]= 0;	pusIndices[16]= 7;	pusIndices[17] = 3;
	pusIndices[18]= 6;	pusIndices[19]= 4;	pusIndices[20] = 5;
	pusIndices[21]= 6;	pusIndices[22]= 7;	pusIndices[23] = 4;
	pusIndices[24]= 6;	pusIndices[25]= 5;	pusIndices[26] = 1;
	pusIndices[27]= 6;	pusIndices[28]= 1;	pusIndices[29] = 2;
	pusIndices[30]= 6;	pusIndices[31]= 2;	pusIndices[32] = 3;
	pusIndices[33]= 6;	pusIndices[34]= 3;	pusIndices[35] = 7;

	pModelData->SetIndices(uTris, uTris, pusIndices, NULL, 1);

	if(m_bInside)
	{
		ReverseTriangleOrder(uTris, pusIndices);
	}
	
	TransformData(*pModelData);
	U2TriList* pMesh = U2_NEW U2TriList(pModelData);
	pMesh->UpdateMeshState(TANGENTSPACE_USE_MESH);
	return pMesh;
}

void U2StandardMesh::ReverseTriangleOrder(uint16 uTris, uint16* pusIndices)
{
	for(uint16 i=0; i < uTris; ++i)
	{
		uint16 j1 = 3 * i+1, j2 = j1 + 1;
		uint16 usSave = pusIndices[j1];
		pusIndices[j1] = pusIndices[j2];
		pusIndices[j2] = usSave;
	}
}


//-------------------------------------------------------------------------------------------------
U2TriList* U2StandardMesh::Sphere(int iZSamples, int iRadialSamples, float fRadius)
{
	int iZSm1 = iZSamples-1, iZSm2 = iZSamples -2, iZSm3 = iZSamples - 3;
	int iRSp1 = iRadialSamples + 1;
	uint32 numVertCnt = iZSm2 * iRSp1 + 2;
	uint32 triCnt = 2 * iZSm2 * iRadialSamples;

	U2TriListData* pModelData = U2_NEW U2TriListData();
	U2ASSERT(pModelData);

	// 정점의 수가 0일때 
	// 다른 객체의 포인터가 깨질수 있으므로 조심...
	// 예)U2Dx9Renderer 포인터가 손상됨.
	pModelData->SetVertexAttbs(m_pAttr);

	pModelData->SetVertexCount(numVertCnt, numVertCnt);

	pModelData->CreateVertices();	


	if(m_pAttr->HasNormal())
	{
		pModelData->CreateNormals(true);
	}

	if(m_pAttr->GetMaxTCoords())
	{
		pModelData->CreateTexCoords(m_pAttr->GetMaxTCoords());
	}
	
	float fInvRS = 1.f/(float)iRadialSamples;
	float fZFactor = 2.f/(float)iZSm1;
	int iR, iZ, iZStart, i, iUnit;
	D3DXVECTOR2 tcoord;

	float* pfSin = U2_ALLOC(float, iRSp1);
	float *pfCos = U2_ALLOC(float, iRSp1);
	for(iR=0; iR < iRadialSamples; ++iR)
	{
		float fAngle = U2_TWO_PI * fInvRS *iR;
		pfSin[iR] = U2Math::Sin(fAngle);
		pfCos[iR] = U2Math::Cos(fAngle);
	}

	pfSin[iRadialSamples] = pfSin[0];
	pfCos[iRadialSamples] = pfCos[0];

	for(iZ=1, i=0; iZ < iZSm1; iZ++)
	{
		float fZFraction = -1.f + fZFactor *iZ;
		float fZ = fRadius * fZFraction;

		D3DXVECTOR3 sliceCenter(0.f, 0.f, fZ);
		float fSliceRadius = U2Math::Sqrt(U2Math::FAbs(fRadius*fRadius - fZ*fZ));

		D3DXVECTOR3 vNormal;
		int iSave = i;
		for(iR=0; iR < iRadialSamples; ++iR)
		{
			float fRadialFraction = iR*fInvRS;
			D3DXVECTOR3 radial(pfCos[iR], pfSin[iR], 0.f);
			pModelData->GetVertices()[i] = sliceCenter + fSliceRadius * radial;
			DPrintf("Postion : Idx: %d, Value : %f, %f, %f\n", i,
				pModelData->GetVertices()[i].x, pModelData->GetVertices()[i].y,
				pModelData->GetVertices()[i].z);
			if(m_pAttr->HasNormal())
			{
				vNormal = pModelData->GetVertices()[i];
				D3DXVECTOR3 vNormalized;
				D3DXVec3Normalize(&vNormalized, &vNormal);
				if(m_bInside)
				{
					pModelData->GetNormals()[i] = -vNormalized;
				}
				else 
				{
					pModelData->GetNormals()[i] = vNormalized;
				}			
			}
		
			
			if(m_pAttr->GetMaxTCoords() > 0)
			{
				tcoord= D3DXVECTOR2(fRadialFraction, 
					0.5f*(fZFraction+1.f));
				for(iUnit=0; iUnit < m_pAttr->GetMaxTCoords(); ++iUnit)
				{
					if(m_pAttr->HasTCoord(iUnit))
					{
						pModelData->GetTexCoordSet(iUnit)[i] = tcoord;
					}
				}
			}

			++i;
		}	
		
		pModelData->GetVertices()[i] = pModelData->GetVertices()[iSave];
		DPrintf("Postion : Idx: %d, Value : %f, %f, %f\n", i,
			pModelData->GetVertices()[i].x, pModelData->GetVertices()[i].y,
			pModelData->GetVertices()[i].z);
		if(m_pAttr->HasNormal())
		{
			pModelData->GetNormals()[i] = pModelData->GetNormals()[iSave];
		}

		if(m_pAttr->GetMaxTCoords() > 0)
		{
			tcoord = D3DXVECTOR2(1.f, 0.5f * (fZFraction + 1.f));
			for(iUnit=0; iUnit < m_pAttr->GetMaxTCoords(); ++iUnit)
			{
				pModelData->GetTexCoordSet(iUnit)[i] = tcoord;
			}
		}

		i++;
	}
	// South Pole
	pModelData->GetVertices()[i] = -fRadius * UNIT_Z;
	DPrintf("Postion : Idx: %d, Value : %f, %f, %f\n",i, 
		pModelData->GetVertices()[i].x, pModelData->GetVertices()[i].y,
		pModelData->GetVertices()[i].z);
	if(m_pAttr->HasNormal())
	{
		if(m_bInside)
		{
			pModelData->GetNormals()[i] = UNIT_Z;
		}
		else 
		{
			pModelData->GetNormals()[i] = -UNIT_Z;
		}
	}

	if(m_pAttr->GetMaxTCoords() > 0)
	{
		tcoord = D3DXVECTOR2(0.5f, 0.5f);
		for(iUnit=0; iUnit < m_pAttr->GetMaxTCoords(); ++iUnit)
		{
			if(m_pAttr->HasTCoord(iUnit))
			{
				pModelData->GetTexCoordSet(iUnit)[i] = tcoord;
			}
		}
	}
	++i;

		// North Pole
		pModelData->GetVertices()[i] = fRadius * UNIT_Z;
		DPrintf("Postion : Idx: %d, Value : %f, %f, %f\n", i,
			pModelData->GetVertices()[i].x, pModelData->GetVertices()[i].y,
			pModelData->GetVertices()[i].z);
		if(m_pAttr->HasNormal())
		{
			if(m_bInside)
			{
				pModelData->GetNormals()[i] = UNIT_Z;
			}
			else 
			{
				pModelData->GetNormals()[i] = -UNIT_Z;
			}
		}

		if(m_pAttr->GetMaxTCoords() > 0)
		{
			tcoord = D3DXVECTOR2(0.5f, 1.f);
			for(iUnit=0; iUnit < m_pAttr->GetMaxTCoords(); ++iUnit)
			{
				if(m_pAttr->HasTCoord(iUnit))
				{
					pModelData->GetTexCoordSet(iUnit)[i] = tcoord;
				}
			}
		}
		++i;

		U2ASSERT(i == numVertCnt);

		// Generate Connectivity
		uint16 *pusTrilist = U2_ALLOC(uint16, 3 * triCnt);		
		int j;
		for(iZ=0, iZStart=0, j=0; iZ < iZSm3; ++iZ)
		{
			uint16 i0 = iZStart;
			uint16 i1= i0 +1;
			iZStart += iRSp1;
			uint16 i2 = iZStart;
			uint16 i3 = i2 + 1;
			for(i=0; i < iRadialSamples; ++i, j+=6)
			{
				if(m_bInside)
				{
					pusTrilist[j] = i0++;
					pusTrilist[j+1] = i2;
					pusTrilist[j+2] = i1;
					pusTrilist[j+3] = i1++;
					pusTrilist[j+4] = i2++;
					pusTrilist[j+5] = i3++;
				}
				else 
				{
					pusTrilist[j] = i0++;				
					pusTrilist[j+1] = i1;
					pusTrilist[j+2] = i2;
					pusTrilist[j+3] = i1++;				
					pusTrilist[j+4] = i3++;				
					pusTrilist[j+5] = i2++;				
				}
				
			}
		}

		int iVQm2 = numVertCnt - 2;
		for(i=0; i < iRadialSamples; ++i, j+=3)
		{
			if(m_bInside)
			{
				pusTrilist[j] = i;
				pusTrilist[j+1] = i+1;
				pusTrilist[j+2] = iVQm2;
			}
			else 
			{
				pusTrilist[j] = i;
				pusTrilist[j+1] = iVQm2;
				pusTrilist[j+2] = i+1;
			}
		}

		int iVQm1 = numVertCnt - 1, iOffSet = iZSm3 * iRSp1;
		for(i=0; i < iRadialSamples; ++i, j+=3)
		{
			if(m_bInside)
			{
				pusTrilist[j] = i + iOffSet;
				pusTrilist[j+1] = iVQm1;
				pusTrilist[j+2] = i + 1 + iOffSet;
			}
			else 
			{
				pusTrilist[j] = i + iOffSet;
				pusTrilist[j+1] = i + 1 + iOffSet;
				pusTrilist[j+2] = iVQm1;
			}
		}

		U2_FREE(pfCos);
		pfCos = NULL;
		U2_FREE(pfSin);
		pfSin = NULL;

		pModelData->SetIndices(triCnt, triCnt, pusTrilist,
			NULL, 1);

		TransformData(*pModelData);
		U2TriList* pMesh = U2_NEW U2TriList(pModelData);
		pMesh->UpdateMeshState();

		return pMesh;
}


void U2StandardMesh::TransformData (U2TriangleData& data)
{
	if(m_xForm.IsIdentity())
	{
		return;
	}

	uint32 vertexCnt = data.GetActiveVertexCount();
	uint32 i;
	for(i=0; i < vertexCnt; ++i)
	{
		data.GetVertices()[i] = m_xForm.Apply(data.GetVertices()[i]);
	}

	if(m_pAttr->HasNormal())
	{
		D3DXVECTOR3 save = m_xForm.GetTrans();
		m_xForm.SetTrans(VECTOR3_ZERO);
		D3DXVECTOR3 normal;
		for(i=0; i < vertexCnt; ++i)
		{
			data.GetNormals()[i] = m_xForm.Apply(data.GetNormals()[i]);
			D3DXVec3Normalize(&normal, &data.GetNormals()[i]);			
			data.GetNormals()[i] = normal;
		}
		m_xForm.SetTrans(save);
	}
}
