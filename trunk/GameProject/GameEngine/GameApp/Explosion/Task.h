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


class ExplosionScene;


/*******************************************************************************
* CLASS: ExplosionTask
*
* DESCRIPTION:
* Implementation of the ISystemTask interface for OGRE graphics.
* See Interfaces\System.h for a definition of the class and its functions.
*******************************************************************************/

/// <summary>
/// ExplosionTask class: Implementation of the ISystemTask interface for OGRE graphics. 
/// </summary>
class ExplosionTask : public ISystemTask
{
    friend class ExplosionScene;


protected:

    ExplosionTask( ExplosionScene* pScene );
    ~ExplosionTask( void );

    /// <summary cref="ISystemScene::GetSystemType">
    ///   Implementation of the ISystemScene GetSystemType function.
    /// </summary>
    virtual System::Type GetSystemType( void );

    virtual void Update( f32 DeltaTime );
    virtual Bool IsPrimaryThreadOnly( void ) { return False; } 


private:

    ExplosionScene*                     m_pScene;
};

