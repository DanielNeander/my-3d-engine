#pragma once 

//#include ".\BaseTypes\TbbSpinMutex.h"

class GraphicSystem;
class GraphicsScene;

class GraphicsObject : public ISystemObject {
	friend GraphicsScene;

public:
	enum Types
	{
		Type_Light,         // Light source (i.e. point light)
		Type_LightFire,     // Light source for a fire
		Type_Camera,        // Camera
		Type_Mesh,          // Model
		Type_MeshAnimated,  // A mesh with animation data
		Type_PointList,     // PointList particles
		Type_Window,        // A graphical window
		Type_StatWindow,	// Statistics window
		Type_Chart,         // A graphical chart
		Type_CPUChart,		// A graphical chart for showing CPU utilization
		Type_WorkloadWindow,	// Window for showing details on workload
		Type_ParticleSystem,    // Particle (Ogre3D particle system)
		Type_PagedGeometryLayer, // A layer on the paged geometry system
		Type_Count
	};

	Types GetType( void )
	{
		return m_Type;
	}

	/// <summary cref="OGREGraphicsObject::Update">
	///   Implementation of the <c>ISystemTask::Update</c> method.
	/// </summary>
	/// <param name="DeltaTime">Elapsed time since the last frame.</param>
	/// <seealso cref="ISystemTask::Update"/>
	virtual void Update( f32 DeltaTime );

protected:

	GraphicsObject( ISystemScene* pSystemScene, pcstr pszName );
	~GraphicsObject( void );

	/// <summary cref="OGREGraphicsObject::GetSystemType">
	///   Implementation of the <c>ISystemObject::GetSystemType</c> function.
	///   Lets this object know when a registered aspects of interest has changed 
	///   (this function will be called when other systems make changes this object should know about).
	/// </summary>
	/// <returns>System::Type - Type of this system.</returns>
	/// <seealso cref="ISystemObject::GetSystemType"/>
	virtual System::Type GetSystemType( void );

	/// <summary cref="OGREGraphicsObject::Initialize">
	///   Implementation of the <c>ISystem::Initialize</c> function.
	/// </summary>
	/// <param name="Properties">Initializes the object with the properties specified by <paramref name="Properties"/>.</param>
	/// <returns>Error.</returns>
	/// <seealso cref="ISystem::Initialize"/>
	virtual Error Initialize( std::vector<Properties::Property> Properties );

	/// <summary cref="OGREGraphicsObject::GetDesiredSystemChanges">
	///   Implementation of the <c>IGeometryObject::GetDesiredSystemChanges</c> function.
	/// </summary>
	/// <returns>System::Types::BitMask - System changes desired by the object.</returns>
	/// <seealso cref="ISystemObject::GetSystemType"/>
	virtual System::Types::BitMask GetDesiredSystemChanges( void );

	/// <summary cref="OGREGraphicsObject::ChangeOccurred">
	///   Implementation of the <c>IObserver::ChangeOccurred</c> function.
	/// </summary>
	/// <param name="pSubject">Subject of this notification.</param>
	/// <param name="ChangeType">Type of notification for this object.</param>
	/// <returns>Error.</returns>
	/// <seealso cref="IObserver::ChangeOccurred"/>
	virtual Error ChangeOccurred( ISubject* pSubject, System::Changes::BitMask ChangeType );

	/// <summary cref="OGREGraphicsObject::GetPotentialSystemChanges">
	///   Implementation of the <c>ISubject::GetPotentialSystemChanges</c> function.
	/// </summary>
	/// <returns>System::Changes::BitMask - Returns systems changes possible for this object.</returns>
	/// <seealso cref="ISubject::GetPotentialSystemChanges"/>
	virtual System::Changes::BitMask GetPotentialSystemChanges( void );

protected:

	Types                               m_Type;
	static pcstr                        sm_kapszTypeNames[];

	pcstr                               m_pszName;

	SceneNode*							m_pNode;

	DEFINE_SPIN_MUTEX(m_mutex);
};
