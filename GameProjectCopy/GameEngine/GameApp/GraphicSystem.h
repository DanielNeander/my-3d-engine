#pragma once 

class GraphicsScene;
class GraphicsTask;

///////////////////////////////////////////////////////////////////////////////
/// <summary>
///   Implementation of the ISystem interface for graphics.
///   See Interfaces\System.h for a definition of the class and its functions.
/// </summary>
///////////////////////////////////////////////////////////////////////////////

class GraphicsSystem : public ISystem	//, public Ogre::WindowEventListener 
{
	friend GraphicsScene;
public:
	GraphicsSystem( void );
	virtual ~GraphicsSystem( void );


protected:
	/// <summary cref="OGREGraphicsSystem::GetName">
	///   Implementation of the <c>ISystem::GetName</c> function.
	///   Gets the name of the system.  Only custom systems can return a custom name.
	/// </summary>
	/// <returns>pcstr - The name of the system.</returns>
	/// <seealso cref="ISystem::GetName"/>
	virtual pcstr GetName( void );

	/// <summary cref="OGREGraphicsSystem::GetSystemType">
	///   Implementation of the <c>ISystem::GetSystemType</c> function.
	///   Gets the system type for this system.
	/// </summary>
	/// <returns>System::Type - The type of the system.</returns>
	/// <seealso cref="ISystem::GetSystemType"/>
	virtual System::Type GetSystemType( void );

	/// <summary cref="OGREGraphicsSystem::Initialize">
	///   Implementation of the <c>ISystem::Initialize</c> function.
	///   One time initialization function for the system.
	/// </summary>
	/// <param name="Properties">Property structure array to initialize.</param>
	/// <returns>Error - Any error codes.</returns>
	/// <seealso cref="ISystem::Initialize"/>
	virtual Error Initialize( Properties::Array Properties );

	/// <summary cref="OGREGraphicsSystem::GetProperties">
	///   Implementation of the <c>ISystem::GetProperties</c> function.
	///   Gets the properties of this system.
	/// </summary>
	/// <param name="Properties">Property structure array to fill</param>
	/// <seealso cref="ISystem::GetProperties"/>
	virtual void GetProperties( Properties::Array& Properties );

	/// <summary cref="OGREGraphicsSystem::SetProperties">
	///   Implementation of the <c>ISystem::SetProperties</c> function.
	///   Sets the properties for this system.
	/// </summary>
	/// <param name="Properties">Properties to set in the system.</param>
	/// <seealso cref="ISystem::SetProperties"/>
	virtual void SetProperties( Properties::Array Properties );

	/// <summary cref="OGREGraphicsSystem::CreateScene">
	///   Implementation of the <c>ISystem::CreateScene</c> function.
	///   Creates a system scene for containing system objects.
	/// </summary>
	/// <returns>ISystemScene* - The newly create system scene.</returns>
	/// <seealso cref="ISystem::CreateScene"/>
	virtual ISystemScene* CreateScene( void );

	/// <summary cref="OGREGraphicsSystem::DestroyScene">
	///   Implementation of the <c>ISystem::DestroyScene</c> function.
	///   Destroys a system scene.
	/// </summary>
	/// <param name="pSystemScene">The scene to destroy. Any objects within are destroyed.</param>
	/// <returns>Error - Any error codes.</returns>
	/// <seealso cref="ISystem::DestroyScene"/>
	virtual Error DestroyScene( ISystemScene* pSystemScene );

protected:

	enum PropertyTypes
	{
		Property_ResourceLocation,
		Property_WindowName,
		Property_Resolution,
		Property_ShadowTextureSize,
		Property_ShadowTextureCount,
		Property_FullScreen,
		Property_VerticalSync,
		Property_FSAntiAliasing,
		Property_Count
	};

	static pcstr                        sm_kapszPropertyNames[];
	static const Properties::Property   sm_kaDefaultProperties[];

	u16                                 m_uShadowTextureCount;
	u16                                 m_uShadowTextureSize;

	Direct3D11Renderer*					m_pRenderer;
};

extern ManagerInterfaces       g_GraphicsManagers;
