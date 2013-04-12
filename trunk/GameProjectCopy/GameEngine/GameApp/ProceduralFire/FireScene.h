//Writen by Hugh Smith summer and fall 2007
// Part of Smoke Framework implementation for procedural Fire system.

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

#pragma once

// When NO_MUTUAL_FIRE_COLLISION_CHECKS is set, fire object with index i is
// only checked against fire objects in the range [i+1, last). This substantially
// reduces the amount of calculations.
//
// But since the effect of this setting is somewhat different from the result of
// the original total check, I've disabled it.
#define NO_MUTUAL_FIRE_COLLISION_CHECKS 0

#if NO_MUTUAL_FIRE_COLLISION_CHECKS
    // Since it looks like the check is not commutative (that is reciprocal),
    // I've implemented the tweak that alternates the checks against [i+1, last)
    // and [first, i-1). It delays the fire propagation by 1 frame, while reducing
    // the work almost by half. 
    //
    // If the check is commutative, then the code under this macro can be safely deleted.
    #define MUTUAL_CHECK_TWEAK 1
#endif


class FireSystem;
class FireTask;
class FireObject;

    typedef std::vector<FireObject*> FireObjectList;

///////////////////////////////////////////////////////////////////////////////
/// <summary>
///   Implementation of the ISystemScene interface.
///   See Interfaces\System.h for a definition of the class and its functions.    
/// </summary>
///////////////////////////////////////////////////////////////////////////////

class FireScene : public ISystemScene
{
    friend FireSystem;
    friend FireTask;


protected:

    FireScene( ISystem* pSystem );
    ~FireScene( void );

    /// <summary cref="ISystemScene::GetSystemType">
    ///   Implementation of the ISystemScene GetSystemType function.
    /// </summary>
    virtual System::Type GetSystemType( void );

    virtual Error Initialize( std::vector<Properties::Property> Properties );

    virtual void GetProperties( std::vector<Properties::Property>& Properties );
    virtual void SetProperties( std::vector<Properties::Property> Properties );

    virtual pcstr* GetObjectTypes( void );
    virtual ISystemObject* CreateObject( pcstr pszName,
                                         pcstr pszType );
    virtual Error DestroyObject( ISystemObject* pSystemObject );

    virtual ISystemTask* GetSystemTask( void );

    virtual System::Changes::BitMask GetPotentialSystemChanges( void );

	virtual const void* GetSystemChangeData( System::Change SystemChange );
public:

    virtual FireObjectList& GetFires();


protected:

    //enum PropertyTypes
    //{
    //    Property_Gravity,
    //    Property_Count
    //};

    //static pcstr                        sm_kapszPropertyNames[];
    //static const Properties::Property   sm_kaDefaultProperties[];

    FireTask*                     m_pTask;
    FireObjectList                m_FireObjects;

    Bool                            m_bParallelize;
};
