///////////////////////////////////////////////////////////////////////////////
// Config.h
///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Include files.
///////////////////////////////////////////////////////////////////////////////

namespace Intel
{

///////////////////////////////////////////////////////////////////////////////
// The Config class definition.
///////////////////////////////////////////////////////////////////////////////
class Config
{
	const static int MAX_FLOATS = 4;

	class Entry
	{
	public:

		char m_szName[MAX_PATH];
		unsigned int m_uNumFloats;
		float m_fData[MAX_FLOATS];
	};

	std::vector<Entry*> m_EntryVector;

	void ParseLine( char* pszLineBuf );

public:

	Config();
	~Config();

	bool Load( char* pszFileName );

	bool GetEntryData( char* pszEntryName, unsigned int* puNumFloats, float* pfData );

	float GetEntryDataWithDefault( char* pszEntryName, float defValue  );
	bool GetEntryDataWithDefault( char* pszEntryName, float* pfData, float d1 );
	bool GetEntryDataWithDefault( char* pszEntryName, float* pfData, float d1, float d2 );
	bool GetEntryDataWithDefault( char* pszEntryName, float* pfData, float d1, float d2, float d3 );
	bool GetEntryDataWithDefault( char* pszEntryName, float* pfData, float d1, float d2, float d3, float d4 );

};

};

///////////////////////////////////////////////////////////////////////////////
// EOF.
///////////////////////////////////////////////////////////////////////////////
