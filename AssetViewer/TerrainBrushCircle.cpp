#include "TerrainBrushCircle.h"


void TerrainBrushCircle::SetupBrushOverlay()
{


	U2VertexAttributes* attr = U2_NEW U2VertexAttributes;
	attr->SetPositionChannels(3);	
	attr->SetColorChannels(0, 1);	

	uint32 uVertCnt = this->ms_usVerts;
	D3DXVECTOR3* pVert = U2_ALLOC(D3DXVECTOR3, uVertCnt);
	D3DXCOLOR* pColor = U2_ALLOC(D3DXCOLOR, uVertCnt);

	unsigned char* pConn = U2_ALLOC(unsigned char, ms_usVerts);
		
	
	float fOuterAngle;
	for (uint32 ui = 0; ui < ms_usVerts; ui++)
	{
		pConn[ui] = 1;

		if (ui <= ms_usInnerSegments)
		{
			pColor[ui] = m_baseColor;
			pVert[ui] = D3DXVECTOR3(m_fSizeInner * U2Math::Cos((float)i/
				float(ms_usInnerSegments)*U2_TWO_PI), 				
				0.0f,
				m_fSizeInner * U2Math::Sin((float)i / float(ms_usInnerSegments) * U2_TWO_PI));

			if (ui == ms_usInnerSegments)
				pConn[ui] = 0;
		}
		else
		{
			pColor[ui] = m_baseColor * 0.5f;
			fOuterAngle = ((float)ui - (ms_usInnerSegments + 1.0f)) /(float)ms_usOuterSegments * U2_TWO_PI;
			
			pVert[ui] = D3DXVECTOR3(m_fSizeInner * U2Math::Cos(fOuterAngle), 0.0f,
				m_fSizeOuter * U2Math::Sin(fOuterAngle));

			if (ui == ms_usVerts - 1)
				pConn[ui] = 0;
		}		
	}

	U2PolyLineData* pLineData = U2_NEW U2PolyLineData(uVertCnt, pVert, NULL, pColor, 
		NULL, 0, NULL);

	pLineData->SetVertexAttbs(attr);

	m_spBrushOverlay = U2_NEW U2PolyLine(pLineData);

	m_spBrushOverlay->m_spModelBound->SetCenter(D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_spBrushOverlay->m_spModelBound->SetRadius(m_fSizeOuter);
	m_spBrushOverlay->Update(0.0f);
}


bool TerrainBrushCircle::IsInBrushBound(float fActiveRadiusSqr,
							float fOuterRadiusSqr,						
							D3DXVECTOR3& origin,
							D3DXVECTOR3& point, 
							bool& bInOuterRadius,
							float& fdistSqr
							)
{
	fdistSqr = U2Math::Sqr(origin.x - point.x) + U2Math::Sqr(origin.z - point.z);
	bInOuterRadius = fdistSqr <= fOuterRadiusSqr;
	return fdistSqr <= fActiveRadiusSqr;'
}