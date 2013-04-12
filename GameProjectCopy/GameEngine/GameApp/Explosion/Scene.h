// Copyright © 2008-2009 Intel Corporation
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


#pragma once

#include "..\BaseTypes\TbbSpinMutex.h"
class ExplosionSystem;
class ExplosionTask;
class ExplosionObject;

#include "POI.h"


/// <summary>
/// ExplosionScene class: Implementation of the ISystemScene interface. 
/// </summary>
class ExplosionScene : public ISystemScene
{
    friend class ExplosionSystem;
    friend class ExplosionTask;

public:
	inline std::vector<ExplosionObject*> GetObjects( void ) { return m_Objects; }
    inline std::list<POI*> GetPOI( void ) { return m_POI; }

    void PostUpdate( void );

protected:

    ExplosionScene( ISystem* pSystem );
    ~ExplosionScene( void );

    /// <summary cref="ISystemScene::GetSystemType">
    ///   Implementation of the ISystemScene GetSystemType function.
    /// </summary>
    virtual System::Type GetSystemType( void );

	virtual void Update( f32 DeltaTime );

	virtual Error Initialize( std::vector<Properties::Property> Properties );

    virtual void GetProperties( Properties::Array& Properties );
    virtual void SetProperties( Properties::Array Properties );

    virtual pcstr* GetObjectTypes( void );
    virtual ISystemObject* CreateObject( pcstr pszName, pcstr pszType );
    virtual Error DestroyObject( ISystemObject* pSystemObject );

    virtual ISystemTask* GetSystemTask( void );

    virtual System::Changes::BitMask GetPotentialSystemChanges( void );
    virtual System::Changes::BitMask GetDesiredSystemChanges( void );

    virtual Error ChangeOccurred( ISubject* pSubject, System::Changes::BitMask ChangeType );

protected:

    ExplosionTask*                 m_pExplosionTask;   // Main task for this scene
    std::vector<ExplosionObject*>  m_Objects;          // Scene objects
    std::list<POI*>         m_POI;                     // Scene points of interest
    DEFINE_SPIN_MUTEX(m_mutex);
};

