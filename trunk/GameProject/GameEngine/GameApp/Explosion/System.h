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

extern ManagerInterfaces g_Managers;

/*******************************************************************************
* CLASS: ExplosionSystem
*
* DESCRIPTION:
* Implementation of the ISystem interface for generic Explosion functionality.
* See Interfaces\System.h for a definition of the class and its functions.
*******************************************************************************/

/// <summary>
/// ExplosionSystem class: Implementation of the ISystem interface for generic Explosion functionality. 
/// </summary>
class ExplosionSystem : public ISystem
{
public:

    ExplosionSystem( void );
    virtual ~ExplosionSystem( void );


protected:

    virtual pcstr GetName( void );
    virtual System::Type GetSystemType( void );

    virtual Error Initialize( Properties::Array Properties );

    virtual void GetProperties( Properties::Array& Properties );
    virtual void SetProperties( Properties::Array Properties );

    virtual ISystemScene* CreateScene( void );
    virtual Error DestroyScene( ISystemScene* pSystemScene );
};

