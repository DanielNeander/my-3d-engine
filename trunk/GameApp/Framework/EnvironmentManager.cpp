// Copyright ?2008-2009 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

//#include <windows.h>
//#include <string.h>
#include "stdafx.h"

#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

#include "EnvironmentManager.h"


EnvironmentManager::EnvironmentManager(
	void
	)
    : m_RuntimeStatus( IEnvironment::IRuntime::Status::Unknown )
{
}


EnvironmentManager::~EnvironmentManager(
	void
	)
{
}


EnvironmentManager::Variables::Variables(
    void
    )
{
}


void
EnvironmentManager::Variables::Add(
	In pcstr pszName,
	In pcstr pszValue
	)
{
    Variables::Pair vp;
	vp.sName = pszName;
	vp.sValue = pszValue;

	m_Pairs.push_back( vp );
}

// Set the value of an environment variable that already exists.  
// Environment variables are defined in Smoke.gdf.
void
EnvironmentManager::Variables::SetValue(
	In pcstr pszName,
    In pcstr pszValue
	)
{
	for ( PairsIt it=m_Pairs.begin(); it != m_Pairs.end(); it++ )
	{
        Pair& vp = *it;

		if ( vp.sName == pszName )
		{
			vp.sValue = pszValue;
			break;
		}
	}

}


pcstr
EnvironmentManager::Variables::GetAsString(
	In pcstr pszName,
	In pcstr pszDefaultValue
	)
{
	pcstr pszValue = pszDefaultValue;

	GetValue( pszName, pszValue );

	return pszValue;
}


Bool
EnvironmentManager::Variables::IsString(
	In pcstr pszName,
	In pcstr pszCompareValue
	)
{
	Bool bValue = False;
	pcstr pszValue = NULL;

	GetValue( pszName, pszValue );
	if( pszValue != NULL )
	{
		bValue = ( _stricmp( pszValue, pszCompareValue ) == 0 );
	}

	return bValue;
}


Bool
EnvironmentManager::Variables::GetAsBool(
	In pcstr pszName,
	In Bool bDefaultValue
	)
{
	Bool bValue = bDefaultValue;

	pcstr pszValue;
	if ( GetValue( pszName, pszValue ) )
	{
		ASSERT( !_stricmp( pszValue, "True" ) || !_stricmp( pszValue, "False" ) );
		bValue = _stricmp( pszValue, "True" ) == 0;
	}

	return bValue;
}


i32
EnvironmentManager::Variables::GetAsInt(
	In pcstr pszName,
	In i32 DefaultValue
	)
{
	i32 Value = DefaultValue;

	pcstr pszValue;
	if ( GetValue( pszName, pszValue ) )
	{
		Value = atoi( pszValue );
	}

	return Value;
}


f32
EnvironmentManager::Variables::GetAsFloat(
	In pcstr pszName,
	In f32 DefaultValue
	)
{
	f32 Value = DefaultValue;

	pcstr pszValue;
	if ( GetValue( pszName, pszValue ) )
	{
		Value = static_cast<f32>(atof( pszValue ));
	}

	return Value;
}


Bool
EnvironmentManager::Variables::GetValue(
	In pcstr pszName,
	Out pcstr& pszValue
	)
{
	Bool bFound = False;

	for ( PairsIt it=m_Pairs.begin(); it != m_Pairs.end(); it++ )
	{
        const Pair& vp = *it;

		if ( vp.sName == pszName )
		{
			pszValue = vp.sValue.c_str();
			bFound = True;
			break;
		}
	}

	return bFound;
}


IEnvironment::IRuntime::Status
EnvironmentManager::GetStatus(
    void
    )
{
    return m_RuntimeStatus;
}


void
EnvironmentManager::SetStatus(
    IEnvironment::IRuntime::Status Status
    )
{
    ::InterlockedExchange( (LONG*)&m_RuntimeStatus, Status );
}
