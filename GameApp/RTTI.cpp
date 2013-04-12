#include "stdafx.h"
#include "RTTI.h"

//---------------------------------------------------------------------------
noRTTI::noRTTI(const char* pcName, const noRTTI* pkBaseRTTI) :
m_pcName(pcName), m_pkBaseRTTI(pkBaseRTTI)
{
}
//---------------------------------------------------------------------------
bool noRTTI::CopyName(char* acNameBuffer, unsigned int uiMaxSize) const
{
	const char* pcName = GetName();
	if (!pcName || !acNameBuffer)
	{
		strcpy_s(acNameBuffer, uiMaxSize, "\0");
		return false;
	}

	strcpy_s(acNameBuffer, uiMaxSize, pcName);
	return true;
}
//---------------------------------------------------------------------------
