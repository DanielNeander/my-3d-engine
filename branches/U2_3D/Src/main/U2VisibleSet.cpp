#include <U2_3D/src/U23DLibPCH.h>
#include "U2VisibleSet.h"


U2VisibleSet::U2VisibleSet()
{
	m_pArray = 0;
	m_uCurrSize = 0;
	m_uAllocatedSize = 0;
	m_uGrowBy = DEFAULT_GROWBY;
	Resize(DEFAULT_MAX_COUNT, 0);
}


U2VisibleSet::U2VisibleSet(unsigned int uMaxSize, unsigned int uGrowBy)
{
	m_pArray = 0;
	m_uCurrSize = 0;
	m_uAllocatedSize = 0;
	m_uGrowBy = uGrowBy;
	if(m_uGrowBy == 0)
		m_uGrowBy = 1;
	Resize(uMaxSize, 0);
}

U2VisibleSet::~U2VisibleSet()
{
	U2_DELETE[] m_pArray;
	m_pArray = NULL;
}

void U2VisibleSet::Insert(U2Spatial* obj, U2Effect* globalEffect)
{
	if(m_uAllocatedSize == m_uCurrSize)
	{
		Resize(m_uAllocatedSize, m_uGrowBy);
	}

	m_pArray[m_uCurrSize].m_pObj = obj;
	m_pArray[m_uCurrSize].m_pGlobalEffect = globalEffect;
	m_uCurrSize++;
}

uint32 U2VisibleSet::GetCount() const
{
	return m_uCurrSize;
}



void U2VisibleSet::RemoveAll()
{
	m_uCurrSize = 0;
}


U2VisibleObject* U2VisibleSet::GetVisible()
{
	return m_pArray;
}


U2VisibleObject& U2VisibleSet::GetVisible(uint32 uIdx) 
{
	U2ASSERT(0 <= uIdx && uIdx < m_uCurrSize);
	return m_pArray[uIdx];
}


void U2VisibleSet::Resize(int maxSize, int growBy)
{
	uint32 newSize = maxSize + growBy;

	if(newSize == m_uAllocatedSize)
		return ;

	if(newSize < m_uCurrSize)
		m_uCurrSize = newSize;

	U2VisibleObject* pSavedArray = m_pArray;
	m_uAllocatedSize = newSize;
	if(m_uAllocatedSize > 0)
	{
		m_pArray = U2_NEW U2VisibleObject[m_uAllocatedSize];
		U2ASSERT(m_pArray);

		if(m_uCurrSize)
		{
			memcpy_s(m_pArray, m_uCurrSize * sizeof(U2VisibleObject), pSavedArray, 
				m_uCurrSize * sizeof(U2VisibleObject));
		}
	}
	else 
		m_pArray = 0;

	U2_DELETE[] pSavedArray;	
	pSavedArray = NULL;
}


