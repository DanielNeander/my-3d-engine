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
///   Abstraction class for OS specific functionality.
/// </summary>
////////////////////////////////////////////////////////////////////////////////////////////////////

class PlatformManager :	public IPlatform, public SimpleSingleton
{
public:

    /// <summary>
    ///   Constructor.
    /// </summary>
    PlatformManager( void );

    /// <summary>
    ///   Destructor.
    /// </summary>
    ~PlatformManager( void );


public:

    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>
    ///   Provides OS file system functionality.
    /// </summary>
    ////////////////////////////////////////////////////////////////////////////////////////////////

    class FileSystem
    {
        friend class PlatformManager;

    protected:

        /// <summary>
        ///   Constructor.
        /// </summary>
        /// <remarks>Only accessible via the PlatformMananger.</remarks>
        FileSystem( void );

        /// <summary>
        ///   Destructor.
        /// </summary>
        /// <remarks>Only accessible via the PlatformMananger.</remarks>
        ~FileSystem( void );


    public:

        /// <summary>
        ///   Loads a system library and returns pointers to the system.
        /// </summary>
        /// <param name="pszSysLib">Filename of the system library to load.</param>
        /// <param name="ppSystem">Returned pointer to the ISystem implementation.
        ///   Remains untouched on failure.</param>
        /// <returns>An error code.</returns>
        Error LoadSystemLibrary( const char* pszSysLib, ISystem** ppSystem );

		Error LoadSystem( const char* pszSysLib, ISystem** ppSystem );
        /// <summary>
        ///   Verfies the existence of a file for read access.
        /// </summary>
        /// <param name="pszFileName">The file name to look for.</param>
        /// <returns>True if successful, otherwise false.</returns>
        Bool FileExists( In pcstr pszFileName );

        /// <summary>
        ///   Sets the current directory to the location of a file.
        /// </summary>
        /// <param name="pszFileName">The file name to look for.</param>
        /// <param name="apszLocations">A null terminated array of relative search directories.</param>
        /// <param name="pszCurrentDir">Location to store the new current directory or NULL.</param>
        /// <param name="BufferSize">The size of <c>pszCurrentDir</c>.</param>
        /// <returns>True if successful, otherwise false.</returns>
        Bool SetCurrentDirToFileLocation( In pcstr pszFileName, In pcstr apszLocations[],
                                          Out pstr pszCurrentDir=NULL, u32 BufferSize=0 );

    protected:

        struct SystemLib
        {
            //Handle                              hLib;
			std::string							strLib;
            ISystem*                            pSystem;
        };
        std::vector<SystemLib>                  m_SystemLibs;
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>
    ///   Provides OS window system functionality.
    /// </summary>
    ////////////////////////////////////////////////////////////////////////////////////////////////

    class WindowSystem
    {
        friend class PlatformManager;

    protected:

        /// <summary>
        ///   Constructor.
        /// </summary>
        /// <remarks>Only accessible via the PlatformMananger.</remarks>
        WindowSystem( void );


    public:

        /// <summary>
        ///   Processes any waiting window messages.
        /// </summary>
        void ProcessMessages( void );
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>
    ///   Provides OS timer functionality.
    /// </summary>
    ////////////////////////////////////////////////////////////////////////////////////////////////

    class Timers
    {
        friend class PlatformManager;

    protected:

        /// <summary>
        ///   Constructor.
        /// </summary>
        /// <remarks>Only accessible via the PlatformMananger.</remarks>
        Timers( void );


    public:

        /// <summary>
        ///   Get the granularity of the timer in seconds.
        /// </summary>
        /// <returns>The timer granularity.</returns>
        f32 GetGranularity( void );

        /// <summary>
        ///   Creates a timer object.
        /// </summary>
        /// <param name="Interval">The interval that the timer object signals.</param>
        /// <returns>A handle to the timer object.</returns>
        Handle Create( f32 Interval );

        /// <summary>
        ///   Destroys a timer object.
        /// </summary>
        /// <param name="hTimer">A handle to the timer object.</param>
        void Destroy( Handle hTimer );

        /// <summary>
        ///   Waits for the timer object to signal.
        /// </summary>
        /// <param name="hTimer">A handle to the timer object.</param>
		/// <param name="bWait">Should the call wait for the timer or calculate time immediately. (default = True)</param>
        /// <returns>The time elapsed in seconds from the previous signal.</returns>
        f32 Wait( Handle hTimer, Bool bWait=True );
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>
    ///   Provides OS debugging functionality.
    /// </summary>
    ////////////////////////////////////////////////////////////////////////////////////////////////

    class Debugging
    {
        friend class PlatformManager;

    protected:

        /// <summary>
        ///   Constructor.
        /// </summary>
        /// <remarks>Only accessible via the PlatformMananger.</remarks>
        Debugging( void );


    public:

        /// <summary>
        ///   Outputs a message to the debugger.
        /// </summary>
        /// <param name="pszMessage">The message to output.</param>
        void OutputMessage( pcstr pszMessage );
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////
    /// <summary>
    ///   Provides OS file system functionality.
    /// </summary>
    ////////////////////////////////////////////////////////////////////////////////////////////////

    class Processor : public IPlatform::IProcessor
    {
        friend class PlatformManager;

    protected:

        /// <summary>
        ///   Constructor.
        /// </summary>
        /// <remarks>Only accessible via the PlatformMananger.</remarks>
        Processor( void );


    public:
        /// <summary cref="IPlatform::IProcessor::GetNumProcessors">
        ///   Implementation of IPlatform::IProcessor::GetNumProcessors.
        /// </summary>
        virtual u32 GetNumProcessors( void );

        /// <summary cref="IPlatform::IProcessor::AffinitizeThreadToProcessor">
        ///   Implementation of IPlatform::IProcessor::AffinitizeThreadToProcessor.
        /// </summary>
        virtual void AffinitizeThreadToProcessor( u32 ProcessorNumber );


    protected:

        u32                             m_TechnologyBitMask;
        std::vector<u32>                m_ProcessorMasks;
    };


protected:

    FileSystem                          m_FileSystem;
    WindowSystem                        m_WindowSystem;
    Timers                              m_Timers;
    Debugging                           m_Debugging;
    Processor                           m_Processor;


public:

    /// <summary>
    ///   Gets a reference to the FileSystem class.
    /// </summary>
    /// <returns>A reference to the FileSystem class.</returns>
    FileSystem& FileSystem( void )
    {
        return m_FileSystem;
    }

    /// <summary>
    ///   Gets a reference to the WindowSystem class.
    /// </summary>
    /// <returns>A reference to the WindowSystem class.</returns>
    WindowSystem& WindowSystem( void )
    {
        return m_WindowSystem;
    }

    /// <summary>
    ///   Gets a reference to the Timers class.
    /// </summary>
    /// <returns>A reference to the Timers class.</returns>
    Timers& Timers( void )
    {
        return m_Timers;
    }

    /// <summary>
    ///   Gets a reference to the Debugging class.
    /// </summary>
    /// <returns>A reference to the Debugging class.</returns>
    Debugging& Debugging( void )
    {
        return m_Debugging;
    }

    /// <summary cref="IPlatform::Processor">
    ///   Implementation of IPlatform::IProcessor::Processor.
    /// </summary>
    virtual IProcessor& Processor( void )
    {
        return m_Processor;
    }
};


DeclareSingleton( PlatformManager );
