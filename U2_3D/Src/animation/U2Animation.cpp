#include <U2_3D/src/U23DLibPCH.h>
#include "U2Animation.h"

U2Animation::U2Animation()
{

}

U2Animation::~U2Animation()
{
	UnloadResource();
}

bool U2Animation::LoadResource()
{
	return true;
}


void U2Animation::UnloadResource()
{
	unsigned int i;
	for(i=0; i < m_interpKeyGroupArray.Size(); ++i)
	{
		InterpKeyGroup* pGroup = m_interpKeyGroupArray.GetElem(i);
		U2_DELETE pGroup;
	}

	m_interpKeyGroupArray.RemoveAll();
}

void U2Animation::SampeInterpKeys(float fTime, int iInterpGroupIdx, 		
								int firstInterpKeyIdx, int numInterpKeys, D3DXVECTOR4* pKeyArray)
{

}


double U2Animation::GetDuration(int iInterpGroupIdx) const
{
	return m_interpKeyGroupArray.GetElem(iInterpGroupIdx)->GetDuration();
}


