///////////////////////////////////////////////////////////////////////////////
// Config.cpp
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Include files.
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <assert.h>
#include <vector>
#include "Config.h"

using namespace Intel;

///////////////////////////////////////////////////////////////////////////////
// Public methods.
///////////////////////////////////////////////////////////////////////////////
Config g_ConfigFile;

Config::Config()
{
	m_EntryVector.clear();
}

Config::~Config()
{
	for( unsigned int uEntry=0; uEntry<m_EntryVector.size(); uEntry++ )
	{
		delete m_EntryVector.at( uEntry );
	}

	m_EntryVector.clear();
}

bool Config::Load( char* pszFileName )
{
	assert( NULL != pszFileName );

	FILE* pFile = fopen( pszFileName, "rt" );

	if( pFile )
	{
		char szLineBuf[MAX_PATH];

		while( fgets( szLineBuf, MAX_PATH, pFile ) )
		{
			ParseLine( szLineBuf );		
		}
		
		fclose( pFile );
		
		return true;
	}

	return false;
}

bool Config::GetEntryData( char* pszEntryName, unsigned int* puNumFloats, float* pfData )
{
	assert( NULL != pszEntryName );

	for( unsigned int uEntry=0; uEntry<m_EntryVector.size(); uEntry++ )
	{
		Entry* pEntry = m_EntryVector.at( uEntry );

		if( !strcmp( pEntry->m_szName, pszEntryName ) )
		{
			if( NULL != puNumFloats )
			{
				*puNumFloats = pEntry->m_uNumFloats;
			}

			if( NULL != pfData )
			{
				memcpy( pfData, pEntry->m_fData, sizeof(float) * pEntry->m_uNumFloats );
			}

			return true;
		}
	}

	return false;
}

float Intel::Config::GetEntryDataWithDefault( char* pszEntryName, float defValue ) 
{
	float temp[MAX_FLOATS];
	unsigned count;

	if (GetEntryData(pszEntryName, &count, temp) && count>0 )
	{
		return temp[0];
	}
	return defValue;
}

bool Intel::Config::GetEntryDataWithDefault( char* pszEntryName, float* pfData, float d1 ) 
{
	unsigned puNumFloats;

	pfData[0]=d1;
	return GetEntryData(pszEntryName, &puNumFloats, pfData );
}

bool Intel::Config::GetEntryDataWithDefault( char* pszEntryName, float* pfData, float d1, float d2 ) 
{
	pfData[1]=d2;
	return GetEntryDataWithDefault(pszEntryName, pfData, d1 );
}

bool Intel::Config::GetEntryDataWithDefault( char* pszEntryName, float* pfData, float d1, float d2, float d3 ) 
{
	pfData[2]=d3;
	return GetEntryDataWithDefault(pszEntryName, pfData, d1, d2 );
}

bool Intel::Config::GetEntryDataWithDefault( char* pszEntryName, float* pfData, float d1, float d2, float d3, float d4 ) 
{
	pfData[3]=d4;
	return GetEntryDataWithDefault(pszEntryName, pfData, d1, d2, d3 );
}


///////////////////////////////////////////////////////////////////////////////
// Private methods.
///////////////////////////////////////////////////////////////////////////////

void Config::ParseLine( char* pszLineBuf )
{
	assert( NULL != pszLineBuf );

	if( !strncmp( pszLineBuf, "//", 2 ) )
	{
		return;
	}

	if( !strncmp( pszLineBuf, " ", 1 ) || !strncmp( pszLineBuf, "\n", 1 ) 
		|| !strncmp( pszLineBuf, "\r", 1 ) )
	{
		return;
	}

	Entry* pEntry = new Entry;
	pEntry->m_uNumFloats = 0;

	char* pszEquals = strchr( pszLineBuf, '=' );

	*pszEquals = '\0';

	strcpy( pEntry->m_szName, pszLineBuf );

	*pszEquals = '=';

	char* pszData = pszEquals + 1;
	char* pszComma;
	
	bool bParse = true;
	while( bParse )
	{
	    pszComma = strchr( pszData, ',' );
		if( pszComma )
		{
			*pszComma = '\0';
			pEntry->m_fData[pEntry->m_uNumFloats++] = (float)atof( pszData );
			*pszComma = ',';
			pszData = pszComma + 1;
		}
		else if( strlen( pszData ) )
		{
			pEntry->m_fData[pEntry->m_uNumFloats++] = (float)atof( pszData );
			bParse = false;
		}
		else
		{
			bParse = false;
		}
	}

	m_EntryVector.push_back( pEntry );
}

///////////////////////////////////////////////////////////////////////////////
// EOF.
///////////////////////////////////////////////////////////////////////////////

