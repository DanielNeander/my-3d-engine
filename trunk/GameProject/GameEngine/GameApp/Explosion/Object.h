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

//
// forward declarations
//
class ExplosionSystem;
class ExplosionScene;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
///   <c>ExplosionState</c> Manages state for the Explosion system.
/// </summary>
////////////////////////////////////////////////////////////////////////////////////////////////////

class ExplosionState
{
#define STATEFLAG_NEW 0x00000001

public:
	ExplosionState( void ) 
	{ 
		m_State = -1;
		m_PreviousState = -1;
		m_Flags = 0;
		m_Duration = 0.0f; 
	}

	~ExplosionState( void ) {}

	inline void SetState( u32 State )
	{
		m_PreviousState = m_State;
		m_State = State;
		if( m_PreviousState != m_State )
		{
			m_Flags |= STATEFLAG_NEW;
		}
		m_Duration = 0.0f;
	}

	inline bool Triggered( void )
	{
		if( m_Flags & STATEFLAG_NEW )
		{
			m_Flags &= ~STATEFLAG_NEW;
			return True;
		}

		return False;
	}

	inline void InitState( i32 State ) { m_State = m_PreviousState = State; }
	inline i32 GetState( void ) { return m_State; }
	inline i32 GetPreviousState( void ) { return m_PreviousState; }
	inline i32 GetFlags( void ) { return m_Flags; }
	inline f32 GetTime( void ) { return m_Duration; }
	inline void UpdateTime( f32 DeltaTime ) { m_Duration += DeltaTime; }

private:
	i32 m_State;          // Current state
	i32 m_PreviousState;  // Previous state
	i32 m_Flags;          // Flags about this state
	f32 m_Duration;       // Time in the current state
};


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
///   <c>ExplosionObject</c>Explosion object created by the Explosion system.
/// </summary>
////////////////////////////////////////////////////////////////////////////////////////////////////

class ExplosionObject : public ISystemObject, public IGeometryObject
{
    friend class ExplosionScene;
    friend class ExplosionTask;

protected:

    ExplosionObject( ISystemScene* pSystemScene, pcstr pszName );
    virtual ~ExplosionObject( void );

    /// <summary cref="ExplosionObject::GetSystemType">
    ///   Implementation of the <c>ISystemObject::GetSystemType</c> function.
    /// </summary>
    /// <returns>Type of this system.
    /// </returns>
    /// <seealso cref="ISystemObject::GetSystemType"/>
    virtual System::Type GetSystemType( void );

    /// <summary cref="ExplosionObject::Initialize">
    ///   Implementation of the <c>ISystem::Initialize</c> function.
    /// </summary>
    /// <param name="Properties">Initializes the Explosion object with the properties specified by <paramref name="Properties"/>.</param>
    /// <returns>Error.
    /// </returns>
    /// <seealso cref="ISystem::Initialize"/>
    virtual Error Initialize( std::vector<Properties::Property> Properties );

    /// <summary cref="ExplosionObject::GetProperties">
    ///   Implementation of the <c>ISystem::GetProperties</c> function.
    /// </summary>
    /// <param name="Properties">Gets the properties of the Explosion object</param>
    /// <seealso cref="ISystem::GetProperties"/>
    virtual void GetProperties( Properties::Array& Properties );

    /// <summary cref="ExplosionObject::SetProperties">
    ///   Implementation of the <c>ISystem::SetProperties</c> function.
    /// </summary>
    /// <param name="Properties">Sets the properties of the Explosion object</param>
    /// <seealso cref="ISystem::SetProperties"/>
    virtual void SetProperties( Properties::Array Properties );

    /// <summary cref="ExplosionObject::GetDesiredSystemChanges">
    ///   Implementation of the <c>IGeometryObject::GetDesiredSystemChanges</c> function.
    /// </summary>
    /// <returns>System changes desired by the Explosion object.
    /// </returns>
    /// <seealso cref="ISystemObject::GetSystemType"/>
    virtual System::Types::BitMask GetDesiredSystemChanges( void );

    /// <summary cref="ExplosionObject::ChangeOccurred">
    ///   Implementation of the <c>IObserver::ChangeOccurred</c> function.
    /// </summary>
    /// <param name="pSubject">Subject of this notification.</param>
    /// <param name="ChangeType">Type of notification for this object.</param>
    /// <returns>Error.
    /// </returns>
    /// <seealso cref="IObserver::ChangeOccurred"/>
    virtual Error ChangeOccurred( ISubject* pSubject, System::Changes::BitMask ChangeType );

    /// <summary cref="ExplosionObject::GetPotentialSystemChanges">
    ///   Implementation of the <c>ISubject::GetPotentialSystemChanges</c> function.
    /// </summary>
    /// <returns>Returns systems changes possible for this Explosion.
    /// </returns>
    /// <seealso cref="ISubject::GetPotentialSystemChanges"/>
    virtual System::Changes::BitMask GetPotentialSystemChanges( void );

    /// <summary cref="ExplosionObject::GetOrientation">
    ///   Implementation of the <c>IGeometryObject::GetOrientation</c> function.
    /// </summary>
    /// <returns>Returns the orientation quaternion for this Explosion.
    /// </returns>
    /// <seealso cref="IGeometryObject::GetOrientation"/>
	virtual const Math::Quaternion* GetOrientation( void );

    /// <summary cref="ExplosionObject::GetScale">
    ///   Implementation of the <c>IGeometryObject::GetScale</c> function.
    /// </summary>
    /// <returns>Returns the scale for this Explosion.
    /// </returns>
    /// <seealso cref="IGeometryObject::GetScale"/>
	virtual const Math::Vector3* GetScale( void );

public:
    /// <summary cref="ExplosionObject::GetPosition">
    ///   Implementation of the <c>IGeometryObject::GetPosition</c> function.
    /// </summary>
    /// <returns>Returns the position for this Explosion.
    /// </returns>
    /// <seealso cref="IGeometryObject::GetPosition"/>
    virtual const Math::Vector3* GetPosition( void );

public:

    /// <summary cref="ExplosionObject::PreUpdate">
    /// This method is called before each <c>Update</c> call to perform pre-processing.
    /// </summary>
    /// <param name="DeltaTime">Elapsed time since the last frame.</param>
    virtual void PreUpdate( f32 DeltaTime );

    /// <summary cref="ExplosionObject::Update">
    /// Implementation of the <c>ISystemTask::Update</c> method.
    /// </summary>
    /// <param name="DeltaTime">Elapsed time since the last frame.</param>
    /// <seealso cref="ISystemTask::Update"/>
    virtual void Update( f32 DeltaTime );

    /// <summary cref="ExplosionObject::PostUpdate">
    /// This method is called after each <c>Update</c> call to perform post-processing.
    /// </summary>
    /// <param name="DeltaTime">Elapsed time since the last frame.</param>
	virtual void PostUpdate( f32 DeltaTime );

    /// <summary cref="ExplosionObject::GetState">
    ///   Returns the state of the Explosion object.
    /// </summary>
    /// <returns>Returns the state for this Explosion.
    /// </returns>
	inline i32 GetState( void ) { return m_State.GetState(); }

    Math::Vector3       m_Position;     // Position of Explosion object
    Math::Quaternion    m_Orientation;  // Orientation of Explosion object
    Math::Vector3       m_Scale;        // Scale of Explosion object

protected:
	ExplosionState             m_State;        // State of Explosion (internal use)
};

