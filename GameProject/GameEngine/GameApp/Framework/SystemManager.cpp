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

#include "stdafx.h"
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

#include "SystemManager.h"


typedef std::map<Id, ISystem*>::iterator SystemMapIter;
typedef std::map<Id, ISystem*>::const_iterator SystemMapConstIter;


SystemManager::SystemManager(
    void
    )
{
}


SystemManager::~SystemManager(
    void
    )
{
}


Error
SystemManager::Add(
    ISystem* pSystem
    )
{
    System::Type SystemType = pSystem->GetSystemType();    
    _ASSERT( m_Systems.find( SystemType ) == m_Systems.end() );

    m_Systems[ SystemType ] = pSystem;

    return Errors::Success;
}


Error
SystemManager::Remove(
    const System::Type SystemType
    )
{
    Error	Err = Errors::Success;

    SystemMapIter it = m_Systems.find( SystemType );

    if ( it != m_Systems.end() )
    {
        m_Systems.erase( it );
    }

    return Err;
}


ISystem*
SystemManager::Get(
    const System::Type SystemType
    )
{
    ISystem* pSystem = NULL;

    SystemMapConstIter it = m_Systems.find( SystemType );

    if ( it != m_Systems.end() )
    {
        pSystem = (*it).second;
    }


    return pSystem;
}


ISystem*
SystemManager::Get(
    pcstr pszName
    )
{
    ISystem* pSystem = NULL;

    for ( SystemMapConstIter it = m_Systems.begin();
          it != m_Systems.end(); it++ )
    {
        if ( strcmp( pszName, it->second->GetName() ) == 0 )
        {
            pSystem = it->second;
            break;
        }
    }

    return pSystem;
}


ISystem*
SystemManager::GetFirst(
    void
    )
{
    ISystem* pSystem = NULL;

    m_SystemIt = m_Systems.begin();

    if ( m_SystemIt != m_Systems.end() )
    {
        pSystem = (*m_SystemIt).second;
    }

    return pSystem;
}


ISystem*
SystemManager::GetNext(
    void
    )
{
    ISystem* pSystem = NULL;

    if ( ++m_SystemIt != m_Systems.end() )
    {
        pSystem = (*m_SystemIt).second;
    }

    return pSystem;
}
