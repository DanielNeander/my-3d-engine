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

#include "Explosion.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
///   <c>MeteorImpact</c> Instance of an Explosion object.
/// </summary>
////////////////////////////////////////////////////////////////////////////////////////////////////

class MeteorImpact : public Explosion
{
public:

    /// <summary cref="MeteorImpact::Explosion">
    /// ctor
    /// </summary>
    /// <param name="pSystemScene">Pointer to the <c>ISystemScene</c> object.</param>
    /// <param name="pszName">Name of the MeteorImpact object.</param>
	MeteorImpact( ISystemScene* pSystemScene, pcstr pszName );
    
    /// <summary cref="MeteorImpact::~MeteorImpact">
    /// dtor
    /// </summary>
	virtual ~MeteorImpact( void );

    /// <summary cref="MeteorImpact::PreUpdate">
    /// This method is called before each <c>Update</c> call to perform pre-processing.
    /// </summary>
    /// <param name="DeltaTime">Elapsed time since the last frame.</param>
	virtual void PreUpdate( f32 DeltaTime );

    /// <summary cref="MeteorImpact::ChangeOccurred">
    /// Implementation of the <c>ISystemTask::Update</c> method.
    /// </summary>
    /// <param name="DeltaTime">Elapsed time since the last frame.</param>
    /// <seealso cref="ISystemTask::Update"/>
	virtual void Update( f32 DeltaTime );

    /// <summary cref="MeteorImpact::PostUpdate">
    /// This method is called after each <c>Update</c> call to perform post-processing.
    /// </summary>
    /// <param name="DeltaTime">Elapsed time since the last frame.</param>
	virtual void PostUpdate( f32 DeltaTime );

    /// <summary cref="MeteorImpact::GetPotentialSystemChanges">
    /// Implementation of the <c>ISubject::GetPotentialSystemChanges</c> method.
    /// </summary>
    /// <returns>Returns systems changes possible for this MeteorImpact
    /// </returns>
    /// <seealso cref="ISubject::GetPotentialSystemChanges"/>
    virtual System::Changes::BitMask GetPotentialSystemChanges( void );

    /// <summary cref="MeteorImpact::~MeteorImpact">
    /// Copy ctor
    /// </summary>
	MeteorImpact( const MeteorImpact& );

    /// <summary cref="MeteorImpact::~MeteorImpact">
    /// Assignment operator
    /// </summary>
	const MeteorImpact &operator=( const MeteorImpact &rhs );

protected:

    /// <summary >
    /// MeteorImpact indicator tha thte fragment was marked used in the pool
    /// </summary >
	Bool m_bFragmentUsed;

    /// <summary >
    /// MeteorImpact impact vector for fragment deflection angle calculations
    /// </summary >
	Math::Vector3 m_Impact; // Impact vector for fragment deflection angle calculations
};

