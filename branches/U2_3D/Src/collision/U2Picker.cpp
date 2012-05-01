#include <U2_3D/Src/U23DLibPCH.h>
#include "U2Picker.h"
#include <U2_3D/src/collision/U2IntectLineTriangle3.h>

const U2PickRecord U2Picker::ms_invalid;

//-------------------------------------------------------------------------------------------------
U2Picker::U2Picker()
:m_origin(VECTOR3_ZERO),
m_dir(VECTOR3_ZERO)
{
	m_fTMin = 0.0f;
	m_fTMax = 0.0f;
}

//-------------------------------------------------------------------------------------------------
void U2Picker::Execute(U2Spatial *pScene, const D3DXVECTOR3 &origin, const D3DXVECTOR3 &dir, 
					   float fTMin, float fTMax)
{
#ifdef _DEBUG
	if(fTMin == -FLT_MAX)
	{
		U2ASSERT(fTMax == FLT_MAX);
	}
	else 
	{
		U2ASSERT(fTMin == 0.0f && fTMax > 0.0f);
	}
#endif 

	m_origin = origin;
	m_dir = dir;
	m_fTMin = fTMin;
	m_fTMax = fTMax;

	unsigned int i;
	for(i=0; i < records.FilledSize(); ++i)
	{
		U2_DELETE records[i];
	}

	records.RemoveAll();

	ExecuteRecursive(pScene);
}

//-------------------------------------------------------------------------------------------------
const U2PickRecord& U2Picker::GetClosestToZero() const
{
	if(0 == records.FilledSize())
	{
		return ms_invalid;
	}

	float fClosest = U2Math::FAbs(records[0]->fT);
	unsigned int uClosest = 0;	
	for(unsigned int i=0; i < records.FilledSize(); ++i)
	{
		float fTmp = U2Math::FAbs(records[i]->fT);
		if(fTmp < fClosest)
		{
			fClosest = fTmp;
			uClosest = i;
		}
	}
	return *records[uClosest];
}

//-------------------------------------------------------------------------------------------------
const U2PickRecord& U2Picker::GetClosestNonnegative() const 
{
	if(0 == records.FilledSize())
	{
		return ms_invalid;
	}


	float fClosest = FLT_MAX;
	unsigned int uClosest;
	for(uClosest = 0; uClosest < records.FilledSize(); ++uClosest)
	{
		if(records[uClosest]->fT >= 0.0f)
		{
			fClosest = records[uClosest]->fT;
			break;
		}
	}

	if(uClosest == records.FilledSize())
	{
		// All values are negative.
		return ms_invalid;
	}

	for(unsigned int ui = uClosest+1; ui < records.FilledSize(); ++ui)
	{
		if(0.0f <= records[ui]->fT && records[ui]->fT < fClosest)
		{
			fClosest = records[ui]->fT;
			uClosest = ui;
		}
	}

	return *records[uClosest];	
}

//-------------------------------------------------------------------------------------------------
const U2PickRecord& U2Picker::GetClosestNonpositive() const 
{
	if(0 == records.FilledSize())
	{
		return ms_invalid;
	}


	float fClosest = -FLT_MAX;
	unsigned int uClosest;
	for(uClosest = 0; uClosest < records.FilledSize(); ++uClosest)
	{
		if(records[uClosest]->fT <= 0.0f)
		{
			fClosest = records[uClosest]->fT;
			break;
		}
	}

	if(uClosest == records.FilledSize())
	{
		// All values are negative.
		return ms_invalid;
	}

	for(unsigned int ui = uClosest+1; ui < records.FilledSize(); ++ui)
	{
		if(fClosest < records[ui]->fT && records[ui]->fT <= 0.0f)
		{
			fClosest = records[ui]->fT;
			uClosest = ui;
		}
	}

	return *records[uClosest];	
}

//-------------------------------------------------------------------------------------------------
void U2Picker::ExecuteRecursive(U2Spatial *pObj)
{
	U2Triangle* pMesh = DynamicCast<U2Triangle>(pObj);
	if(pMesh)
	{
		if(pMesh->m_spWorldBound->TestIntersection(m_origin, m_dir, m_fTMin, m_fTMax))
		{
			U2Line3D* line = U2_NEW U2Line3D(pMesh->m_tmWorld.ApplyInverse(m_origin), 
				pMesh->m_tmWorld.InverseVector(m_dir));

			// Compute intersections with the model-space triangles
			unsigned int uActiveTriangles = pMesh->GetActiveTriangleCount();
			
			D3DXVECTOR3* pVert;

			if (DynamicCast<nMeshGroup>(pObj))
			{
				pVert = ((nMeshGroup*)pObj)->GetMesh()->GetVertices();
			}
			else 			
				pVert = pMesh->GetVertices();
			for(unsigned int u = 0; u < uActiveTriangles; ++u)
			{
				unsigned short v0, v1, v2;				
				pMesh->GetTriangleIndices(u, v0, v1, v2);

				U2Triangle3* triangle = U2_NEW U2Triangle3( 
					pVert[v0], 
					pVert[v1],
					pVert[v2]);

				line->IncRefCount();

				U2IntectLineTriangle3* intersect = U2_NEW U2IntectLineTriangle3(*line, *triangle);
				if(intersect->Find() && m_fTMin <= intersect->GetLineT()
					&& intersect->GetLineT() <= m_fTMax)
				{
					U2PickRecord* pRecord = U2_NEW U2PickRecord;
					pMesh->IncRefCount();
					pRecord->spInterected = pMesh;
					pRecord->fT = intersect->GetLineT();
					pRecord->uiTriangle = u;
					pRecord->fB0 = intersect->GetTriB0();
					pRecord->fB1 = intersect->GetTriB1();
					pRecord->fB2 = intersect->GetTriB2();
					records.AddElem(pRecord);
				}

				U2_DELETE intersect;				
			}
		}
		return;
	}

	U2SwitchNode* pSwitchNode = DynamicCast<U2SwitchNode>(pObj);
	if(pSwitchNode)
	{
		unsigned int uActiveChild = pSwitchNode->GetActiveChild();
		if(uActiveChild != U2SwitchNode::SN_INVALID_CHILD)
		{
			if(pSwitchNode->m_spWorldBound->TestIntersection(m_origin, 
				m_dir, m_fTMin, m_fTMax))
			{
				U2Spatial* pChild = pSwitchNode->GetChild(uActiveChild);
				if(pChild)
				{
					ExecuteRecursive(pChild);
				}
			}
		}
		return;
	}

	U2Node* pNode = DynamicCast<U2Node>(pObj);
	if(pNode)
	{
		if(pNode->m_spWorldBound->TestIntersection(m_origin, m_dir, 
			m_fTMin, m_fTMax))
		{
			for(unsigned int i=0; i < pNode->GetChildCnt(); ++i)
			{
				U2Spatial* pChild = pNode->GetChild(i);
				if(pChild)
				{
					ExecuteRecursive(pChild);
				}
			}
		}
	}
}

					


			


				
					


