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


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Provides a single location for access to the different systems that are available for use.
/// </summary>
////////////////////////////////////////////////////////////////////////////////////////////////////

class SystemManager : public SimpleSingleton
{
public:

    /// <summary>
    ///   Constructor.
    /// </summary>
    SystemManager( void );

    /// <summary>
    ///   Destructor.
    /// </summary>
    ~SystemManager( void );

    /// <summary>
    ///   Adds a new system to the collection.  Called by the ISystem constructor.
    /// </summary>
    /// <param name="pSystem">A pointer to the system to add.</param>
    /// <returns>An error code.</returns>
    //
    Error Add( ISystem* pSystem );

    /// <summary>
    ///   Removes a system from the collection.  Called by the ISystem destructor.
    /// </summary>
    /// <param name="SystemType">The system type to remove.</param>
    /// <returns>An error code.</returns>
    Error Remove( const System::Type SystemType );

    /// <summary>
    ///   Gets a specific system from the collection based on its type.
    /// </summary>
    /// <param name="SystemType">The type of system to get.</param>
    /// <returns>A pointer to the system.</returns>
    ISystem* Get( const System::Type SystemType );

    /// <summary>
    ///   Gets a specific system from the collection base on its name.
    /// </summary>
    /// <param name="pszName">The name of system to get.</param>
    /// <returns>A pointer to the system or <c>NULL</c> of failure.</returns>
    ISystem* Get( pcstr pszName );

    //
    // TODO: These need to be defined better.  Maybe use something like the stl
    //  iterator. The owner of the Scheduler class needs to take care of this
    //  since it will be the primary consumer.
    //
    ISystem* GetFirst( void );
    ISystem* GetNext( void );


protected:

    std::map<System::Type, ISystem*>              m_Systems;
    std::map<System::Type, ISystem*>::iterator    m_SystemIt;
};



DeclareSingleton( SystemManager );
