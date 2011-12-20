#include <src/U2LibPCH.h>
#include "U2Command.h"

TCHAR U2Command::ms_acOptionNotFound[] = _T("option not found");
TCHAR U2Command::ms_acArgumentRequired[] = _T("option requires an argument");
TCHAR U2Command::ms_acArgumentOutOfRange[] = _T("argument out of range");
TCHAR U2Command::ms_acFilenameNotFound[] = _T("filename not found");

//----------------------------------------------------------------------------
U2Command::U2Command (int iQuantity, TCHAR** apcArgument)
{
	m_iQuantity = iQuantity;
	m_acCmdline = 0;
	m_abUsed = 0;

	if (m_iQuantity > 0)
	{
		m_apcArgument = U2_ALLOC(TCHAR*, m_iQuantity);
		for (int i = 0; i < m_iQuantity; i++)
		{
			m_apcArgument[i] = apcArgument[i];
		}
	}
	else
	{
		m_apcArgument = 0;
	}

	Initialize();
}
//----------------------------------------------------------------------------
U2Command::U2Command (TCHAR* acCmdline)
{
	class Argument : public U2MemObj
	{
	public:
		TCHAR* m_pcItem;
		Argument* m_pkNext;
	};

	m_iQuantity = 0;
	m_apcArgument = 0;
	m_acCmdline = 0;
	m_abUsed = 0;

	if (acCmdline == 0 || _tcslen(acCmdline) == 0)
	{
		return;
	}

	size_t uiSize = _tcslen(acCmdline) + 1;
	m_acCmdline = U2_ALLOC(TCHAR, uiSize);
	_tcscpy_s(m_acCmdline,uiSize,acCmdline);

	TCHAR* pcNextToken;
	TCHAR* pcToken = _tcstok_s(m_acCmdline,_T(" \t"),&pcNextToken);
	Argument* pkList = 0;

	while (pcToken)
	{
		m_iQuantity++;

		Argument* pkCurrent = U2_NEW Argument;
		pkCurrent->m_pcItem = pcToken;
		pkCurrent->m_pkNext = pkList;
		pkList = pkCurrent;

		pcToken = _tcstok_s(0,_T(" \t"),&pcNextToken);
	}

	m_iQuantity++;
	m_apcArgument = U2_ALLOC(TCHAR*, m_iQuantity);
	m_apcArgument[0] = m_acCmdline;
	int i = m_iQuantity-1;
	while (pkList)
	{
		m_apcArgument[i--] = pkList->m_pcItem;

		Argument* pkSave = pkList->m_pkNext;
		U2_DELETE pkList;
		pkList = pkSave;
	}

	Initialize();
}
//----------------------------------------------------------------------------
U2Command::~U2Command ()
{
	U2_FREE(m_abUsed);
	m_abUsed = NULL;
	U2_FREE( m_apcArgument);
	m_apcArgument = NULL;
	U2_FREE( m_acCmdline);
	m_acCmdline = NULL;
}
//----------------------------------------------------------------------------
void U2Command::Initialize ()
{
	m_abUsed = U2_ALLOC(bool, m_iQuantity);
	memset(m_abUsed,false,m_iQuantity*sizeof(bool));

	m_dSmall = 0.0;
	m_dLarge = 0.0;
	m_bMinSet = false;
	m_bMaxSet = false;
	m_bInfSet = false;
	m_bSupSet = false;

	m_acLastError = 0;
}
//----------------------------------------------------------------------------
int U2Command::ExcessArguments ()
{
	// checks to see if any U2Command line arguments were not processed
	for (int i = 1; i < m_iQuantity; i++)
	{
		if (!m_abUsed[i])
		{
			return i;
		}
	}

	return 0;
}
//----------------------------------------------------------------------------
U2Command& U2Command::Min (double dValue)
{
	m_dSmall = dValue;
	m_bMinSet = true;
	return *this;
}
//----------------------------------------------------------------------------
U2Command& U2Command::Max (double dValue)
{
	m_dLarge = dValue;
	m_bMaxSet = true;
	return *this;
}
//----------------------------------------------------------------------------
U2Command& U2Command::Inf (double dValue)
{
	m_dSmall = dValue;
	m_bInfSet = true;
	return *this;
}
//----------------------------------------------------------------------------
U2Command& U2Command::Sup (double dValue)
{
	m_dLarge = dValue;
	m_bSupSet = true;
	return *this;
}
//----------------------------------------------------------------------------
int U2Command::Boolean (TCHAR* acName)
{
	bool bValue = false;
	return Boolean(acName,bValue);
}
//----------------------------------------------------------------------------
int U2Command::Boolean (TCHAR* acName, bool& rbValue)
{
	int iMatchFound = 0;
	rbValue = false;
	for (int i = 1; i < m_iQuantity; i++)
	{
		TCHAR* pcTmp = m_apcArgument[i];
		if (!m_abUsed[i] && pcTmp[0] == _T('-') && _tcscmp(acName,++pcTmp) == 0)
		{
			m_abUsed[i] = true;
			iMatchFound = i;
			rbValue = true;
			break;
		}
	}

	if (iMatchFound == 0)
	{
		m_acLastError = ms_acOptionNotFound;
	}

	return iMatchFound;
}
//----------------------------------------------------------------------------
int U2Command::Integer (TCHAR* acName, int& riValue)
{
	int iMatchFound = 0;
	for (int i = 1; i < m_iQuantity; i++)
	{
		TCHAR* pcTmp = m_apcArgument[i];
		if (!m_abUsed[i] && pcTmp[0] == _T('-') && _tcscmp(acName,++pcTmp) == 0)
		{
			// get argument
			pcTmp = m_apcArgument[i+1];
			if (m_abUsed[i+1] || (pcTmp[0] == _T('-') && !isdigit(pcTmp[1])))
			{
				m_acLastError = ms_acArgumentRequired;
				return 0;
			}
			riValue = _tstoi(pcTmp);
			if ((m_bMinSet && riValue < m_dSmall)
				||  (m_bMaxSet && riValue > m_dLarge)
				||  (m_bInfSet && riValue <= m_dSmall)
				||  (m_bSupSet && riValue >= m_dLarge))
			{
				m_acLastError = ms_acArgumentOutOfRange;
				return 0;
			}
			m_abUsed[i] = true;
			m_abUsed[i+1] = true;
			iMatchFound = i;
			break;
		}
	}

	m_bMinSet = false;
	m_bMaxSet = false;
	m_bInfSet = false;
	m_bSupSet = false;

	if (iMatchFound == 0)
	{
		m_acLastError = ms_acOptionNotFound;
	}

	return iMatchFound;
}
//----------------------------------------------------------------------------
int U2Command::Float (TCHAR* acName, float& rfValue)
{
	int iMatchFound = 0;
	for (int i = 1; i < m_iQuantity; i++)
	{
		TCHAR* pcTmp = m_apcArgument[i];
		if (!m_abUsed[i] && pcTmp[0] == _T('-') && _tcscmp(acName,++pcTmp) == 0)
		{
			// get argument
			pcTmp = m_apcArgument[i+1];
			if (m_abUsed[i+1] || (pcTmp[0] == _T('-') && !isdigit(pcTmp[1])))
			{
				m_acLastError = ms_acArgumentRequired;
				return 0;
			}
			rfValue = (float)_tstof(pcTmp);
			if ((m_bMinSet && rfValue < m_dSmall)
				||  (m_bMaxSet && rfValue > m_dLarge)
				||  (m_bInfSet && rfValue <= m_dSmall)
				||  (m_bSupSet && rfValue >= m_dLarge))
			{
				m_acLastError = ms_acArgumentOutOfRange;
				return 0;
			}
			m_abUsed[i] = true;
			m_abUsed[i+1] = true;
			iMatchFound = i;
			break;
		}
	}

	m_bMinSet = false;
	m_bMaxSet = false;
	m_bInfSet = false;
	m_bSupSet = false;

	if (iMatchFound == 0)
	{
		m_acLastError = ms_acOptionNotFound;
	}

	return iMatchFound;
}
//----------------------------------------------------------------------------
int U2Command::Double (TCHAR* acName, double& rdValue)
{
	int iMatchFound = 0;
	for (int i = 1; i < m_iQuantity; i++)
	{
		TCHAR* pcTmp = m_apcArgument[i];
		if (!m_abUsed[i] && pcTmp[0] == _T('-') && _tcscmp(acName,++pcTmp) == 0)
		{
			// get argument
			pcTmp = m_apcArgument[i+1];
			if (m_abUsed[i+1] || (pcTmp[0] == _T('-') && !isdigit(pcTmp[1])))
			{
				m_acLastError = ms_acArgumentRequired;
				return 0;
			}
			rdValue = _tstof(pcTmp);
			if ((m_bMinSet && rdValue < m_dSmall)
				||  (m_bMaxSet && rdValue > m_dLarge)
				||  (m_bInfSet && rdValue <= m_dSmall)
				||  (m_bSupSet && rdValue >= m_dLarge))
			{
				m_acLastError = ms_acArgumentOutOfRange;
				return 0;
			}
			m_abUsed[i] = true;
			m_abUsed[i+1] = true;
			iMatchFound = i;
			break;
		}
	}

	m_bMinSet = false;
	m_bMaxSet = false;
	m_bInfSet = false;
	m_bSupSet = false;

	if (iMatchFound == 0)
	{
		m_acLastError = ms_acOptionNotFound;
	}

	return iMatchFound;
}
//----------------------------------------------------------------------------
int U2Command::String (TCHAR* acName, TCHAR*& racValue)
{
	int iMatchFound = 0;
	for (int i = 1; i < m_iQuantity; i++)
	{
		TCHAR* pcTmp = m_apcArgument[i];
		if (!m_abUsed[i] && pcTmp[0] == _T('-') && _tcscmp(acName,++pcTmp) == 0)
		{
			// get argument
			pcTmp = m_apcArgument[i+1];
			if (m_abUsed[i+1] || pcTmp[0] == _T('-'))
			{
				m_acLastError = ms_acArgumentRequired;
				return 0;
			}

			size_t uiSize = _tcslen(pcTmp) + 1;
			racValue = U2_ALLOC(TCHAR, uiSize);
			_tcscpy_s(racValue,uiSize,pcTmp);
			m_abUsed[i] = true;
			m_abUsed[i+1] = true;
			iMatchFound = i;
			break;
		}
	}

	if (iMatchFound == 0)
	{
		m_acLastError = ms_acOptionNotFound;
	}

	return iMatchFound;
}
//----------------------------------------------------------------------------
int U2Command::Filename (TCHAR*& racName)
{
	int iMatchFound = 0;
	for (int i = 1; i < m_iQuantity; i++)
	{
		TCHAR* pcTmp = m_apcArgument[i];
		if (!m_abUsed[i] && pcTmp[0] != '-')
		{
			size_t uiSize = _tcslen(pcTmp) + 1;
			racName = U2_ALLOC(TCHAR, uiSize);
			_tcscpy_s(racName,uiSize,pcTmp);
			m_abUsed[i] = true;
			iMatchFound = i;
			break;
		}
	}

	if (iMatchFound == 0)
	{
		m_acLastError = ms_acFilenameNotFound;
	}

	return iMatchFound;
}
//----------------------------------------------------------------------------
const TCHAR* U2Command::GetLastError ()
{
	return m_acLastError;
}
//----------------------------------------------------------------------------
