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
#include "ServiceManager.h"


ServiceManager::ServiceManager(
    void
    )
    : m_pSystemAccess( NULL )
    , m_pCollision( NULL )
{
}


ServiceManager::~ServiceManager(
    void
    )
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// IService Implementations.

IService::ISystemAccess&
ServiceManager::SystemAccess(
    void
    )
{
    ASSERT( m_pSystemAccess != NULL );
    return *m_pSystemAccess;
}


void
ServiceManager::RegisterSystemAccessProvider(
    ISystemAccess* pSystemAccess
    )
{
    ASSERTMSG( m_pSystemAccess == NULL, "There is already an existing system access provider." );
    m_pSystemAccess = pSystemAccess;
}


void
ServiceManager::UnregisterSystemAccessProvider(
    ISystemAccess* pSystemAccess
    )
{
    ASSERTMSG( m_pSystemAccess == pSystemAccess,
               "You are not the registered system access provider." );

    if ( m_pSystemAccess == pSystemAccess )
    {
        m_pSystemAccess = NULL;
    }
}


IService::ICollision&
ServiceManager::Collision(
    void
    )
{
    ASSERT( m_pCollision != NULL );
    return *m_pCollision;
}


void
ServiceManager::RegisterCollisionProvider(
    ICollision* pCollision
    )
{
    ASSERTMSG( m_pCollision == NULL, "There is already an existing collision provider." );
    m_pCollision = pCollision;
}


void
ServiceManager::UnregisterCollisionProvider(
    ICollision* pCollision
    )
{
    ASSERTMSG( m_pCollision == pCollision, "You are not the registered collision provider." );

    if ( m_pCollision == pCollision )
    {
        m_pCollision = NULL;
    }
}


// Get the Instrumentation interface.
IService::IInstrumentation&
ServiceManager::Instrumentation(
	void
	)
{
	ASSERT( m_pInstrumentation != NULL );
	return *m_pInstrumentation;
}


// Register an instrumentation interface.
void
ServiceManager::RegisterInstrumentationProvider(
	IInstrumentation* pInstrumentation
	)
{
	ASSERTMSG( m_pInstrumentation == NULL, "There is already an existing instrumentation provider." );
	m_pInstrumentation = pInstrumentation;
}

// Unregister the instrumentation interface.
void
ServiceManager::UnregisterInstrumentationProvider(
	IInstrumentation* pInstrumentation
	)
{
	ASSERTMSG( m_pInstrumentation == pInstrumentation, "Trying to unregister an instrumentation provider that is not registered." );
	if( m_pInstrumentation == pInstrumentation )
	{
		m_pInstrumentation = NULL;
	}
}