#include <U2_3D/src/U23DLibPCH.h>
#include "U2Object.h"


const U2Rtti U2Object::ms_rtti(_T("U2Object"), 0);
unsigned int U2Object::ms_uiNextId = 0;


U2Object::U2Object()
{	
	m_uiId = ms_uiNextId++;	
}

U2Object::~U2Object()
{
	DetachAllCotroller();	
}

//-------------------------------------------------------------------------------------------------
void U2Object::DetachController(U2Controller* pCtrl)
{
	for(unsigned int i=0; i < GetNumControllers(); ++i)
	{
		U2Controller* pCurrCtrl = GetController(i);
		if(pCurrCtrl == pCtrl)
		{
			U2_DELETE pCurrCtrl;
			m_controllers.SetElem(i, 0);
			return;
		}
	}	
}

//-------------------------------------------------------------------------------------------------
void U2Object::DetachAllCotroller()
{
	for(unsigned int i=0; i < GetNumControllers(); ++i)
	{
		U2_DELETE m_controllers[i];
	}
		
	m_controllers.RemoveAll();
}

//-------------------------------------------------------------------------------------------------
bool U2Object::UpdateControllers(float fAppTime, U2RenderContext* pCxt)
{
	bool bAnyUpdated = false;

	for(unsigned int i=0; i < GetNumControllers(); ++i)
	{
		U2Controller* pCurrCtrl = GetController(i);

		if(pCurrCtrl->Update(fAppTime, pCxt))
		{
			bAnyUpdated = true;
		}
	}

	return bAnyUpdated;	
}



