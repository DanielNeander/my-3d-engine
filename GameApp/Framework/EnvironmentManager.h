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
///   A singleton for getting and setting information about the environment.
/// </summary>
////////////////////////////////////////////////////////////////////////////////////////////////////

class EnvironmentManager : public IEnvironment, public IEnvironment::IRuntime, public SimpleSingleton
{
public:

    /// <summary>
    ///   Constructor.
    /// </summary>
    EnvironmentManager( void );

    /// <summary>
    ///   Destructor.
    /// </summary>
    ~EnvironmentManager( void );


public:

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>
    ///   Environment variable functionality.
    /// </summary>
    ////////////////////////////////////////////////////////////////////////////////////////////////

    class Variables : public IEnvironment::IVariables
    {
        friend class EnvironmentManager;

    public:

        /// <summary>
        ///   Adds a new environment variable.
        /// </summary>
        /// <param name="pszName">The name of the variable.</param>
        /// <param name="pszValue">The value of the variable.</param>
        void Add( In pcstr pszName, In pcstr pszValue );

        /// <summary>
        ///   Sets new value for environment variable.
        /// </summary>
        /// <param name="pszName">The name of the variable.</param>
        /// <param name="pszValue">The new value of the variable.</param>
        void SetValue( In pcstr pszName, In pcstr pszValue );

        /// <summary cref="IEnvironment::IVariables::GetAsString">
        ///   Implementation of IEnvironment::IVariables::GetAsString.
        /// </summary>
        virtual pcstr GetAsString( In pcstr pszName, In pcstr pszDefaultValue="" );

        /// <summary cref="IEnvironment::IVariables::IsString">
        ///   Implementation of IEnvironment::IVariables::IsString.
        /// </summary>
        virtual Bool IsString( In pcstr pszName, In pcstr pszCompareValue );

        /// <summary cref="IEnvironment::IVariables::GetAsBool">
        ///   Implementation of IEnvironment::IVariables::GetAsBool.
        /// </summary>
        virtual Bool GetAsBool( In pcstr pszName, In Bool bDefaultValue=False );

        /// <summary cref="IEnvironment::IVariables::GetAsInt">
        ///   Implementation of IEnvironment::IVariables::GetAsInt.
        /// </summary>
        virtual i32 GetAsInt( In pcstr pszName, In i32 DefaultValue=0 );

        /// <summary cref="IEnvironment::IVariables::GetAsFloat">
        ///   Implementation of IEnvironment::IVariables::GetAsFloat.
        /// </summary>
        virtual f32 GetAsFloat( In pcstr pszName, In f32 DefaultValue=0.0f );


    protected:

        /// <summary>
        ///   Constructor.
        /// </summary>
        /// <remarks>Only accessible via the EnvironmentManager class.</remarks>
        Variables( void );

        /// <summary>
        ///   Gets the value of a variable.
        /// </summary>
        /// <param name="pszName">The name of the variable.</param>
        /// <param name="pszValue">The returned value of the variable.</param>
        /// <returns>True if the variable exists, false otherwise.</returns>
        Bool GetValue( In pcstr pszName, Out pcstr& pszValue );


    protected:

        struct Pair
        {
            std::string                             sName;
            std::string                             sValue;
        };

        typedef std::vector<Pair>                   Pairs;
        typedef Pairs::iterator                     PairsIt;

        Pairs                                       m_Pairs;
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // IEnvironment::IRuntime functionality.

    /// <summary cref="IEnvironment::IRuntime::GetStatus">
    ///   Implementation of IEnvironment::IRuntime::GetStatus.
    /// </summary>
    virtual IEnvironment::IRuntime::Status GetStatus( void );

    /// <summary cref="IEnvironment::IRuntime::SetStatus">
    ///   Implementation of IEnvironment::IRuntime::SetStatus.
    /// </summary>
    virtual void SetStatus( IEnvironment::IRuntime::Status Status );


protected:

    Variables                                       m_Variables;

    IEnvironment::IRuntime::Status                  m_RuntimeStatus;


public:

    /// <summary cref="IEnvironment::Variables">
    ///   Implementation of IEnvironment::Variables.
    /// </summary>
    virtual Variables& Variables( void )
    {
        return m_Variables;
    }

    /// <summary cref="IEnvironment::Runtime">
    ///   Implementation of IEnvironment::Runtime.
    /// </summary>
    virtual IRuntime& Runtime( void )
    {
        return *this;
    }
};


DeclareSingleton( EnvironmentManager );
